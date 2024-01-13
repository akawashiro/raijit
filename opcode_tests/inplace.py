import raijit

def raijit_test_inplace_add(a, b):
    a += b
    return a

def raijit_test_inplace_sub(a, b):
    a -= b
    return a

def raijit_test_inplace_mul(a, b):
    a *= b
    return a

def raijit_test_inplace_div(a, b):
    a /= b
    return a

def raijit_test_inplace_mod(a, b):
    a %= b
    return a

def raijit_test_inplace_lshift(a, b):
    a <<= b
    return a

def raijit_test_inplace_rshift(a, b):
    a >>= b
    return a

def raijit_test_inplace_and(a, b):
    a &= b
    return a

def raijit_test_inplace_xor(a, b):
    a ^= b
    return a

def raijit_test_inplace_or(a, b):
    a |= b
    return a

def raijit_test_inplace_true_div(a, b):
    a /= b
    return a

def raijit_test_inplace_floor_div(a, b):
    a //= b
    return a

# TODO: Test
def raijit_test_inplace_matrix_mul(a, b):
    a @= b
    return a

# TODO: Test
def raijit_test_inplace_pow(a, b):
    a **= b
    return a

raijit.enable()

assert raijit_test_inplace_add(7, 3) == 10
assert raijit_test_inplace_sub(7, 3) == 4
assert raijit_test_inplace_mul(7, 3) == 21
assert raijit_test_inplace_div(7, 3) == 2.3333333333333335
assert raijit_test_inplace_mod(7, 3) == 1
assert raijit_test_inplace_lshift(7, 3) == 56
assert raijit_test_inplace_rshift(7, 3) == 0
assert raijit_test_inplace_and(7, 3) == 3
assert raijit_test_inplace_xor(7, 3) == 4
assert raijit_test_inplace_or(7, 3) == 7
assert raijit_test_inplace_true_div(7, 3) == 2.3333333333333335
assert raijit_test_inplace_floor_div(7, 3) == 2
# assert raijit_test_inplace_pow(7, 3) == 343
