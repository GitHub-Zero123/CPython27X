#include <iostream>
#include "./patch.h"
#include <Python.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

int main() {
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    Python27X::InitializePatch();

    Py_SetPythonHome(const_cast<char*>("D:\\Zero123\\CPP\\CMAKE\\cpython27x\\cpython"));
    Py_Initialize();
    PyRun_SimpleString(R"(
def testFunc(a: int = 0, b: int = 0) -> int:
    return a + b

print "细节Py2, But Py3类型注解"
print testFunc(3, 4)
)");
    Py_Finalize();
    return 0;
}