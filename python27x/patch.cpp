#include "./patch.h"
#include <iostream>
#include <unordered_map>
#include <string>
#include <cstring>

extern "C" {
    #include "PythonX.h"
}

static bool _INIT_PATCH_STATE = false;
// 强制启用UTF-8编码支持
static bool _FORCE_ENABLE_UTF8_SUPPORT = false;

// ============= 源码预处理工具函数 =============

static inline bool isHorizSpace(char c) {
    return c == ' ' || c == '\t';
}

static inline bool isIdentChar(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '_';
}

// 检查前两行是否包含编码声明 (PEP 263)
static bool hasEncodingDeclaration(const char* code, size_t len) {
    size_t i = 0;
    for (int line = 0; line < 2 && i < len; ++line) {
        size_t lineEnd = i;
        while (lineEnd < len && code[lineEnd] != '\n') ++lineEnd;

        // 在该行中查找 '#'
        size_t hashPos = i;
        while (hashPos < lineEnd && code[hashPos] != '#') ++hashPos;

        if (hashPos < lineEnd) {
            // 在 '#' 之后搜索 "coding" 模式
            for (size_t j = hashPos; j + 6 <= lineEnd; ++j) {
                if (std::memcmp(code + j, "coding", 6) == 0) {
                    size_t k = j + 6;
                    while (k < lineEnd && isHorizSpace(code[k])) ++k;
                    if (k < lineEnd && (code[k] == '=' || code[k] == ':')) {
                        return true;
                    }
                }
            }
        }

        i = (lineEnd < len) ? lineEnd + 1 : len;
    }
    return false;
}

// 向源码头部添加 UTF-8 编码声明
static void addUTF8Declaration(std::string& code) {
    if (hasEncodingDeclaration(code.c_str(), code.size())) return;

    // 如果第一行是 shebang, 插入到其后
    if (code.size() >= 2 && code[0] == '#' && code[1] == '!') {
        size_t nlPos = code.find('\n');
        if (nlPos != std::string::npos) {
            code.insert(nlPos + 1, "# -*- coding: utf-8 -*-\n");
        } else {
            code += "\n# -*- coding: utf-8 -*-\n";
        }
    } else {
        code.insert(0, "# -*- coding: utf-8 -*-\n");
    }
}

// 跳过水平空白
static inline size_t skipHSpace(const char* s, size_t pos, size_t len) {
    while (pos < len && isHorizSpace(s[pos])) ++pos;
    return pos;
}

// 跳过类型注解表达式, 在指定终止符(, ) = \n # ;)处停止
// 正确处理嵌套括号和字符串字面量
static size_t skipAnnotation(const char* s, size_t pos, size_t len) {
    int pd = 0, bd = 0, brd = 0;
    enum { SN, SSQ, SDQ, STSQ, STDQ } ss = SN;

    while (pos < len) {
        char c = s[pos];

        if (ss != SN) {
            if (c == '\\') { pos += 2; continue; }
            if (ss == STSQ && c == '\'' && pos + 2 < len && s[pos+1] == '\'' && s[pos+2] == '\'')
                { ss = SN; pos += 3; continue; }
            if (ss == STDQ && c == '"' && pos + 2 < len && s[pos+1] == '"' && s[pos+2] == '"')
                { ss = SN; pos += 3; continue; }
            if (ss == SSQ && c == '\'') ss = SN;
            if (ss == SDQ && c == '"') ss = SN;
            ++pos; continue;
        }

        if (c == '\'' || c == '"') {
            if (pos + 2 < len && s[pos+1] == c && s[pos+2] == c)
                { ss = (c == '\'') ? STSQ : STDQ; pos += 3; }
            else
                { ss = (c == '\'') ? SSQ : SDQ; ++pos; }
            continue;
        }

        if (c == '#') break;
        if (c == '(') { ++pd; ++pos; continue; }
        if (c == ')') { if (pd > 0) { --pd; ++pos; continue; } else break; }
        if (c == '[') { ++bd; ++pos; continue; }
        if (c == ']') { if (bd > 0) { --bd; ++pos; continue; } else break; }
        if (c == '{') { ++brd; ++pos; continue; }
        if (c == '}') { if (brd > 0) { --brd; ++pos; continue; } else break; }

        if (pd == 0 && bd == 0 && brd == 0) {
            if (c == ',' || c == '\n' || c == '\r' || c == ';') break;
            if (c == '=' && (pos + 1 >= len || s[pos + 1] != '=')) break;
        }

        ++pos;
    }
    return pos;
}

// 剔除 Python 3 类型注解 (参数注解、返回值注解、变量注解)
// 单遍 O(n) 扫描, 正确跳过字符串字面量和注释
static std::string stripTypeAnnotations(const char* code, size_t len) {
    std::string result;
    result.reserve(len);

    const char* s = code;
    size_t pos = 0;

    // 字符串状态
    enum StringState { StrNone, StrSQ, StrDQ, StrTSQ, StrTDQ };
    StringState strState = StrNone;
    bool inComment = false;

    // 嵌套深度
    int parenDepth = 0, bracketDepth = 0, braceDepth = 0;

    // def 跟踪状态
    enum DefState { DEF_NONE, DEF_SEEN, DEF_PARAMS, DEF_AFTER_CLOSE };
    DefState defState = DEF_NONE;
    int defParenBase = 0, defBracketBase = 0, defBraceBase = 0;

    // 关键字列表 (这些关键字后的 ':' 不是变量注解)
    static const char* const kws[] = {
        "if", "else", "elif", "for", "while", "with",
        "try", "except", "finally", "class", "def",
        "lambda", "return", "yield", "raise", "import",
        "from", "as", "pass", "break", "continue",
        "assert", "del", "exec", "print", "global", nullptr
    };

    while (pos < len) {
        char c = s[pos];

        // ======== 字符串内部: 原样输出 ========
        if (strState != StrNone) {
            if (c == '\\') {
                result += c;
                if (++pos < len) { result += s[pos]; ++pos; }
                continue;
            }
            if (strState == StrTSQ && c == '\'' && pos + 2 < len && s[pos+1] == '\'' && s[pos+2] == '\'') {
                result.append(s + pos, 3); pos += 3; strState = StrNone; continue;
            }
            if (strState == StrTDQ && c == '"' && pos + 2 < len && s[pos+1] == '"' && s[pos+2] == '"') {
                result.append(s + pos, 3); pos += 3; strState = StrNone; continue;
            }
            if ((strState == StrSQ && c == '\'') || (strState == StrDQ && c == '"'))
                strState = StrNone;
            if ((strState == StrSQ || strState == StrDQ) && c == '\n')
                strState = StrNone;
            result += c; ++pos;
            continue;
        }

        // ======== 注释内部: 原样输出 ========
        if (inComment) {
            result += c;
            if (c == '\n') inComment = false;
            ++pos;
            continue;
        }

        // ======== 注释起始 ========
        if (c == '#') {
            inComment = true;
            result += c; ++pos;
            continue;
        }

        // ======== 字符串起始 ========
        if (c == '\'' || c == '"') {
            if (pos + 2 < len && s[pos+1] == c && s[pos+2] == c) {
                strState = (c == '\'') ? StrTSQ : StrTDQ;
                result.append(s + pos, 3); pos += 3;
            } else {
                strState = (c == '\'') ? StrSQ : StrDQ;
                result += c; ++pos;
            }
            continue;
        }

        // ======== 换行 ========
        if (c == '\n') {
            result += c;
            if (parenDepth == 0 && bracketDepth == 0 && braceDepth == 0) {
                if (defState != DEF_AFTER_CLOSE)
                    defState = DEF_NONE;
            }
            ++pos;
            continue;
        }

        // ======== 括号追踪 ========
        if (c == '(') {
            ++parenDepth;
            if (defState == DEF_SEEN) {
                defState = DEF_PARAMS;
                defParenBase = parenDepth;
                defBracketBase = bracketDepth;
                defBraceBase = braceDepth;
            }
            result += c; ++pos; continue;
        }
        if (c == ')') {
            if (defState == DEF_PARAMS && parenDepth == defParenBase)
                defState = DEF_AFTER_CLOSE;
            --parenDepth;
            result += c; ++pos; continue;
        }
        if (c == '[') { ++bracketDepth; result += c; ++pos; continue; }
        if (c == ']') { --bracketDepth; result += c; ++pos; continue; }
        if (c == '{') { ++braceDepth;   result += c; ++pos; continue; }
        if (c == '}') { --braceDepth;   result += c; ++pos; continue; }

        // ======== 检测 "def" 关键字 ========
        if (c == 'd' && defState == DEF_NONE
            && pos + 3 <= len && s[pos+1] == 'e' && s[pos+2] == 'f'
            && (pos + 3 >= len || !isIdentChar(s[pos+3]))) {
            // 验证 def 处于语句起始位置 (兼容 async def)
            bool stmtStart = false;
            {
                size_t bk = result.size();
                while (bk > 0 && isHorizSpace(result[bk-1])) --bk;
                if (bk == 0 || result[bk-1] == '\n' || result[bk-1] == ';') {
                    stmtStart = true;
                } else if (bk >= 5
                    && result[bk-1] == 'c' && result[bk-2] == 'n'
                    && result[bk-3] == 'y' && result[bk-4] == 's'
                    && result[bk-5] == 'a'
                    && (bk < 6 || !isIdentChar(result[bk-6]))) {
                    // "async def" 模式
                    size_t bk2 = bk - 5;
                    while (bk2 > 0 && isHorizSpace(result[bk2-1])) --bk2;
                    stmtStart = (bk2 == 0 || result[bk2-1] == '\n' || result[bk2-1] == ';');
                }
            }
            if (stmtStart) defState = DEF_SEEN;
            result.append(s + pos, 3);
            pos += 3;
            continue;
        }

        // ======== 处理 "->" 返回值注解 ========
        if (defState == DEF_AFTER_CLOSE && c == '-' && pos + 1 < len && s[pos+1] == '>') {
            // 跳过 -> 及其后的返回值类型注解
            size_t p = pos + 2;
            p = skipHSpace(s, p, len);
            // 跳过类型表达式直到 ':' (函数体起始)
            int lp = 0, lb = 0, lbr = 0;
            enum { SN2, SSQ2, SDQ2, STSQ2, STDQ2 } ls = SN2;
            while (p < len) {
                char cc = s[p];
                if (ls != SN2) {
                    if (cc == '\\') { p += 2; continue; }
                    if (ls == STSQ2 && cc == '\'' && p+2<len && s[p+1]=='\'' && s[p+2]=='\'') { ls = SN2; p += 3; continue; }
                    if (ls == STDQ2 && cc == '"'  && p+2<len && s[p+1]=='"'  && s[p+2]=='"')  { ls = SN2; p += 3; continue; }
                    if (ls == SSQ2 && cc == '\'') ls = SN2;
                    if (ls == SDQ2 && cc == '"')  ls = SN2;
                    ++p; continue;
                }
                if (cc == '\'' || cc == '"') {
                    if (p+2<len && s[p+1]==cc && s[p+2]==cc) { ls = (cc=='\'') ? STSQ2 : STDQ2; p += 3; }
                    else { ls = (cc=='\'') ? SSQ2 : SDQ2; ++p; }
                    continue;
                }
                if (cc == '#') break;
                if (cc == '(') ++lp;
                else if (cc == ')') { if (lp>0) --lp; else break; }
                else if (cc == '[') ++lb;
                else if (cc == ']') { if (lb>0) --lb; else break; }
                else if (cc == '{') ++lbr;
                else if (cc == '}') { if (lbr>0) --lbr; else break; }
                if (lp == 0 && lb == 0 && lbr == 0 && (cc == ':' || cc == '\n' || cc == '\r')) break;
                ++p;
            }
            // 删除 result 尾部在 -> 之前的多余空白
            while (!result.empty() && isHorizSpace(result.back())) result.pop_back();
            pos = p;
            defState = DEF_NONE;
            continue;
        }

        // ======== def 结尾的 ':' (无返回值注解) ========
        if (defState == DEF_AFTER_CLOSE && c == ':'
            && parenDepth == 0 && bracketDepth == 0 && braceDepth == 0) {
            defState = DEF_NONE;
            result += c; ++pos;
            continue;
        }

        // ======== ':' 注解检测 ========
        if (c == ':') {
            bool handled = false;

            // --- 情况1: def 参数注解 ---
            if (defState == DEF_PARAMS
                && parenDepth == defParenBase
                && bracketDepth == defBracketBase
                && braceDepth == defBraceBase) {
                size_t bk = result.size();
                while (bk > 0 && isHorizSpace(result[bk-1])) --bk;
                if (bk > 0 && isIdentChar(result[bk-1])) {
                    result.resize(bk); // 修剪参数名后空白
                    size_t p = skipHSpace(s, pos + 1, len);
                    p = skipAnnotation(s, p, len);
                    pos = p;
                    handled = true;
                }
            }

            // --- 情况2: 变量注解 (语句级别) ---
            if (!handled && defState == DEF_NONE
                && parenDepth == 0 && bracketDepth == 0 && braceDepth == 0) {
                size_t bk = result.size();
                while (bk > 0 && isHorizSpace(result[bk-1])) --bk;
                size_t identEnd = bk;
                while (bk > 0 && (isIdentChar(result[bk-1]) || result[bk-1] == '.')) --bk;
                size_t identStart = bk;

                if (identStart < identEnd) {
                    size_t ck = identStart;
                    while (ck > 0 && isHorizSpace(result[ck-1])) --ck;
                    bool stmtStart = (ck == 0 || result[ck-1] == '\n' || result[ck-1] == ';');

                    if (stmtStart) {
                        // 提取标识符, 检查是否为关键字
                        std::string baseWord(result, identStart, identEnd - identStart);
                        auto dotPos = baseWord.find('.');
                        if (dotPos != std::string::npos) baseWord.resize(dotPos);

                        bool isKw = false;
                        for (auto kw = kws; *kw; ++kw) {
                            if (baseWord == *kw) { isKw = true; break; }
                        }

                        if (!isKw) {
                            result.resize(identEnd);
                            size_t p = skipHSpace(s, pos + 1, len);
                            p = skipAnnotation(s, p, len);
                            if (p < len && s[p] == '=') {
                                result += ' ';
                            }
                            pos = p;
                            handled = true;
                        }
                    }
                }
            }

            if (!handled) {
                result += c; ++pos;
            }
            continue;
        }

        // ======== 默认: 拷贝字符 ========
        result += c; ++pos;
    }

    return result;
}

// 格式化源码并返回堆上的字符串地址
// 1. 剔除 Python 3 类型注解 (参数/返回值/变量注解)
// 2. 若启用 _FORCE_ENABLE_UTF8_SUPPORT, 自动添加 UTF-8 编码声明
static std::string* formatStringToHeapCString(const char* format) {
    if (format == nullptr) return nullptr;

    size_t len = std::strlen(format);
    if (len == 0) return new std::string();

    // 剔除 Python 3 类型注解
    std::string* result = new std::string(stripTypeAnnotations(format, len));

    // 按需添加 UTF-8 编码声明
    if (_FORCE_ENABLE_UTF8_SUPPORT) {
        addUTF8Declaration(*result);
    }

    return result;
}

namespace Python27X {
    static std::unordered_map<const char*, std::string*> stdStringMapping;

    static const char* moveSTDStringToCString(const std::string& str) {
        std::string* heapStr = new std::string(str);
        const char* cstr = heapStr->c_str();
        stdStringMapping[cstr] = heapStr;
        return cstr;
    }

    static const char* moveSTDStringToCString(std::string&& str) {
        std::string* heapStr = new std::string(std::move(str));
        const char* cstr = heapStr->c_str();
        stdStringMapping[cstr] = heapStr;
        return cstr;
    }

    static const char* moveSTDHeapStringToCString(std::string* str) {
        if(str == nullptr) {
            return nullptr;
        }
        const char* cstr = str->c_str();
        stdStringMapping[cstr] = str;
        return cstr;
    }

    static void freeCString(const char* cstr) {
        auto it = stdStringMapping.find(cstr);
        if (it != stdStringMapping.end()) {
            delete it->second;
            stdStringMapping.erase(it);
        }
    }

    static const char* customParserStringHandler(const char* s) {
        if(s == nullptr) {
            return nullptr;
        }
        auto heapStr = formatStringToHeapCString(s);
        if(heapStr == nullptr) {
            return nullptr;
        }
        return moveSTDHeapStringToCString(heapStr);
    }

    static void customParserStringFreeHandler(const char* s) {
        freeCString(s);
    }

    // 初始化补丁方法
    void InitializePatch() {
        if(_INIT_PATCH_STATE) {
            return;
        }
        _INIT_PATCH_STATE = true;
        // std::cout << python27x_get_PyParser_ParseStringFlagsFilenameEx_addr() << "\n";
        PyParser_SetCustomHandler(customParserStringHandler, customParserStringFreeHandler);
    }

    void ForceEnableUTF8Support(bool enable) {
        _FORCE_ENABLE_UTF8_SUPPORT = enable;
    }
}