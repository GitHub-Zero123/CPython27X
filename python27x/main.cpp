#include <string>
#include <filesystem>
#include <Python.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

static std::filesystem::path getCurrentExePath() {
    wchar_t buf[MAX_PATH];
    GetModuleFileNameW(NULL, buf, MAX_PATH);

    std::filesystem::path exePath(buf);
    return exePath.parent_path();
}

int main(int argc, char** argv) {
    if(argc > 1) {
        // 对于非交互式的命令行参数，设置控制台输出为UTF-8编码
        // 确保不再像CPython2原版那样使用系统默认编码（如GBK），以避免输出乱码问题
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
    }
    auto exePath = getCurrentExePath().string();
    Py_SetPythonHome(const_cast<char*>(exePath.c_str()));
    return Py_Main(argc, argv);
}