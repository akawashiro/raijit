#!/usr/bin/env python3
import dis

def unary_not(a):
    return not a

def unary_invert(a):
    return ~a

def dup_top():
    a = 10
    return a, a

def dict_merge(a, b):
    return {**a, **b}

def dict_merge2(a, b):
    return dict(**a, **b)

def set_update(b):
    a = {1, 2, 3}
    a.add(b)
    return a

def define_lambda():
    return lambda x: x + 1

def define_class():
    class A:
        def __init__(self, a):
            self.a = a

    return A

print(dis.dis(unary_not))
print(dis.dis(unary_invert))
print(dis.dis(dup_top))
print(dis.dis(dict_merge))
print(dis.dis(dict_merge2))
print(dis.dis(set_update))
print(dis.dis(define_lambda))
# print(dis.dis(define_class))
