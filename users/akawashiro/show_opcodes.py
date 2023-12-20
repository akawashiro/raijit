#!/usr/bin/env python3
import dis

def unary_not(a):
    return not a

def unary_invert(a):
    return ~a

print(dis.dis(unary_not))
print(dis.dis(unary_invert))
