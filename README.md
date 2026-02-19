# CPython27X
一款基于CPython 2.7魔改的Python解释器，扩展了Python3的类型注解语法特性。

## 示例代码
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

## AST兼容性
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
