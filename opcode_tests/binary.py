import raijit

def raijit_test_binary_and(a, b):
    return a & b

def raijit_test_binary_or(a, b):
    return a | b

def raijit_test_binary_xor(a, b):
    return a ^ b

def raijit_test_binary_lshift(a, b):
    return a << b

def raijit_test_binary_rshift(a, b):
    return a >> b

def raijit_test_binary_modulo(a, b):
    return a % b

raijit.enable()

assert raijit_test_binary_and(7, 3) == 3
assert raijit_test_binary_or(7, 3) == 7
assert raijit_test_binary_xor(7, 3) == 4
assert raijit_test_binary_lshift(7, 3) == 56
assert raijit_test_binary_rshift(7, 3) == 0
assert raijit_test_binary_modulo(7, 3) == 1
