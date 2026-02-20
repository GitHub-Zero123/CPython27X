# -*- coding: utf-8 -*-
import dis

def testFunc(a: int = 0, b: int = 0) -> int:
    return a + b

def testFunc2(a, b):
    return a + b

dis.dis(testFunc)
print ""
dis.dis(testFunc2)