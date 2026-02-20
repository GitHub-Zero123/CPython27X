# -*- coding: utf-8 -*-

class FuckEvent:
    _STATIC_VAR: int = 0

    def testMethod(self, a: int) -> str:
        pass

    @classmethod
    def testClassMethod(cls, a: int) -> str:
        pass

    @staticmethod
    def testStaticMethod(a: int) -> str:
        pass

print FuckEvent.testMethod.__annotations__
print FuckEvent.testStaticMethod.__annotations__
print FuckEvent.testClassMethod.__annotations__

def testFunc(event: FuckEvent, b: int) -> FuckEvent:
    pass

def testFunc2(event: FuckEvent | None) -> None:
    pass

def testFunc3(a: 'int', b: dict[str, int], c: list[tuple[float, float, float]]) -> int:
    pass

aa = lambda x: x + 1
bb = lambda cc=aa: cc(10)

print bb()
print testFunc.__annotations__
print testFunc2.__annotations__
print testFunc3.__annotations__

import dis
dis.dis(testFunc)
print ""
dis.dis(testFunc2)
print ""
dis.dis(testFunc3)