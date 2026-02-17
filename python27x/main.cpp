// #include <iostream>
#include <string>
#include <filesystem>
// #include "./patch.h"
#include <Python.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// 获取当前exe所在路径
static std::filesystem::path getCurrentExePath() {
    wchar_t buf[MAX_PATH];
    GetModuleFileNameW(NULL, buf, MAX_PATH);

    std::filesystem::path exePath(buf);
    return exePath.parent_path();
}

int main(int argc, char** argv) {
    if(argc > 1) {
        // 对于非交互式的命令行参数，设置控制台输出为UTF-8编码
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
    }
    auto exePath = getCurrentExePath();
    Py_SetPythonHome(const_cast<char*>(exePath.string().c_str()));
    return Py_Main(argc, argv);
}

// int main() {
//     SetConsoleCP(CP_UTF8);
//     SetConsoleOutputCP(CP_UTF8);
//     // Python27X::InitializePatch();

//     Py_SetPythonHome(const_cast<char*>("D:\\Zero123\\CPP\\CMAKE\\cpython27x\\cpython"));
//     Py_Initialize();
//     PyRun_SimpleString(R"(
// def testFunc(a: int = 0, b: int = 0) -> int:
//     return a + b

// def testFunc2(a: int, b: int):
//     return a * b

// testFunc2(3, b=4)

// a: int | None = 1
// b: int = 2
// print a
// print b
// print "细节Py2, But Py3类型注解"
// print testFunc(3, 4)
// )");
//     Py_Finalize();
//     return 0;
// }