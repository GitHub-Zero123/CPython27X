# -*- coding: utf-8 -*-

class FuckEvent:
    pass

def testFunc(event: FuckEvent, b: int) -> FuckEvent:
    pass

def testFunc2(event: FuckEvent | None) -> None:
    pass

def testFunc3(a: 'int') -> int:
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