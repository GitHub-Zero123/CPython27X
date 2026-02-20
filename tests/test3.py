# -*- coding: utf-8 -*-

code = """
# -*- coding: utf-8 -*-

class FuckEvent:
    pass

def testFunc(event: FuckEvent, b: int) -> FuckEvent:
    pass

def testFunc2(event: FuckEvent | None) -> None:
    pass

aa = lambda x: x + 1
bb = lambda cc=aa: cc(10)

print bb()
print testFunc.__annotations__
print testFunc2.__annotations__
"""

codeObj = compile(code, "<test>", "exec")
exec(codeObj)