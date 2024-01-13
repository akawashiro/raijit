import raijit

def raijit_test_unary_pos(a):
    return +a

def raijit_test_unary_neg(a):
    return -a

def raijit_test_unary_not(a):
    return not a

def raijit_test_unary_invert(a):
    return ~a

raijit.enable()

assert raijit_test_unary_pos(42) == 42
assert raijit_test_unary_neg(42) == -42
assert raijit_test_unary_not(True) == False
assert raijit_test_unary_not(42) == False
assert raijit_test_unary_not(0) == True
assert raijit_test_unary_invert(42) == -43
