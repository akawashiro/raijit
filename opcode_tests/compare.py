import raijit

def compare_gt(a, b):
    return a > b

def compare_eq(a, b):
    return a == b

def compare_le(a, b):
    return a <= b

import dis
dis.dis(compare_gt)
dis.dis(compare_eq)
dis.dis(compare_le)

raijit.enable()
assert compare_gt(1, 2) == False
assert compare_eq(1, 2) == False
assert compare_le(2, 2) == True
