# -*- coding: utf-8 -*-

def createTestFunc():
    def _testFunc(a: int) -> int:
        pass
    return _testFunc

# 内存泄漏测试
while 1:
    createTestFunc().__annotations__