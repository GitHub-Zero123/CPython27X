# CPython27X

基于 CPython 2.7 改造的 Python 解释器，支持 Python 3 风格的类型注解语法。

> 这是一个实验项目，用于探索老版本 Python 解释器中引入新语法的可行性，不保证稳定可靠，适合用于学习和测试目的。

## 简介

CPython27X 在 Python 2.7 的基础上额外引入了 Python 3 的类型注解语法支持。类型注解在解析阶段被忽略，不影响运行时行为，适用于需要在 Python 2.7 环境中使用类型提示的场景（一些老式项目仍在使用 Python 2.7）。

## 基础示例

通过以下示例展示了 CPython27X 中类型注解的支持情况：

```python
# -*- coding: utf-8 -*-

def testFunc(a: int = 0, b: int = 0) -> int:
    return a + b

def testFunc2(a: int, b: int):
    return a * b

testFunc2(3, b=4)

a: int | None = 1
b: int = 2
c = lambda aa=a: aa
print a
print b
print c()
print "细节Py2, 但是Py3类型注解"
print testFunc(3, 4)
```

## AST 兼容性

CPython27X 生成的 AST 结构与标准 Python 2.7 保持一致，确保现有的静态分析工具、代码检查器等基于 AST 的工具可以正常工作。

```python
# -*- coding: utf-8 -*-
import ast

testCode = """
def testFunc(a: int = 0, b: int = 0) -> int:
    return a + b

def testFunc2(a: int, b: int):
    return a * b

testFunc2(3, b=4)

a: int | None = 1
b: int = 2
c = lambda aa=a: aa
print a
print b
print c()
print "细节Py2, 但是Py3类型注解"
print testFunc(3, 4)
"""

tree = ast.parse(testCode)
print ast.dump(tree)
```

## 字节码兼容性

类型注解在解析阶段被完全剥离，生成的字节码与原生 Python 2.7 完全一致。这意味着带有类型注解的函数与不带注解的函数在执行效率上没有任何差异。

```python
# -*- coding: utf-8 -*-
import dis

def testFunc(a: int = 0, b: int = 0) -> int:
    return a + b

def testFunc2(a, b):
    return a + b

dis.dis(testFunc)
print ""
dis.dis(testFunc2)
```

输出结果：

```text
  5           0 LOAD_FAST                0 (a)
              3 LOAD_FAST                1 (b)
              6 BINARY_ADD
              7 RETURN_VALUE        

  8           0 LOAD_FAST                0 (a)
              3 LOAD_FAST                1 (b)
              6 BINARY_ADD
              7 RETURN_VALUE        
```
其字节码完全相同，不会因为引入类型注解而产生兼容问题。

## 核心变动

主要涉及以下核心文件的修改：

- `token.h`
- `parsermodule.c`
- `tokenizer.c`
- `ast.c`
- `graminit.c`