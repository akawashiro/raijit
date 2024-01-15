import raijit

def raijit_test_compare_gt(a, b):
    return a > b

def raijit_test_compare_eq(a, b):
    return a == b

def raijit_test_compare_le(a, b):
    return a <= b

def raijit_test_compare_lt(a, b):
    return a < b

def raijit_test_compare_ge(a, b):
    return a >= b

def raijit_test_compare_ne(a, b):
    return a != b

raijit.enable()
assert raijit_test_compare_gt(1, 2) == False
assert raijit_test_compare_eq(1, 2) == False
assert raijit_test_compare_le(2, 2) == True
assert raijit_test_compare_lt(2, 2) == False
assert raijit_test_compare_ge(2, 2) == True
assert raijit_test_compare_ne(2, 2) == False
