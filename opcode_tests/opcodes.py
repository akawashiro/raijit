import raijit

def raijit_test_store():
    a = 42
    return a

def raijit_test_rot_two():
    a = 42
    b = 43
    (a, b) = (b, a)
    return a

def raijit_test_rot_three():
    a = 42
    b = 43
    c = 44
    (a, b, c) = (b, c, a)
    return a

def raijit_test_build_map():
    return {'a': 32}

def raijit_test_map_ref_in_func():
    return {'a': 32}['a']

def raijit_test_unary_pos(a):
    return +a

def raijit_test_unary_neg(a):
    return -a

def raijit_test_map_ref(d):
    return d['a']

def raijit_test_map_change(d):
    d['a'] = 33
    return d

def raijit_test_local_var_map():
    d = {'a': 32}
    return d

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

def raijit_test_inplace_pow(a, b):
    a **= b
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

def raijit_test_map_setitem():
    d = {}
    d['a'] = 32
    d['b'] = 64
    return d

def raijit_test_build_map_2():
    d = {'a': 32, 'b': 64}
    return d

def raijit_test_build_map_3():
    d = {'a': 32, 'b': 64, 'c': 128}
    return d

def raijit_test_build_list_0():
    return []

def raijit_test_build_list_1():
    return [1, 2]

def raijit_test_build_tuple_0():
    a = 10
    b = 20
    return (a, b)

def raijit_test_build_set_0():
    return set()

def raijit_test_contains_op():
    return 1 in [1, 2]

def raijit_test_is_op():
    a = 10
    b = 20
    return a is b

raijit.enable()
assert raijit_test_store() == 42
assert raijit_test_rot_two() == 43
assert raijit_test_rot_three() == 43
assert raijit_test_build_map() == {'a': 32}
assert raijit_test_map_ref_in_func() == 32
assert raijit_test_unary_pos(42) == 42
assert raijit_test_unary_neg(42) == -42
assert raijit_test_map_ref({'a': 32}) == 32
assert raijit_test_map_change({'a': 32}) == {'a': 33}
assert raijit_test_local_var_map() == {'a': 32}
assert raijit_test_binary_and(7, 3) == 3
assert raijit_test_binary_or(7, 3) == 7
assert raijit_test_binary_xor(7, 3) == 4
assert raijit_test_binary_lshift(7, 3) == 56
assert raijit_test_binary_rshift(7, 3) == 0
assert raijit_test_binary_modulo(7, 3) == 1
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
assert raijit_test_inplace_pow(7, 3) == 343
assert raijit_test_map_setitem() == {'a': 32, 'b': 64}
assert raijit_test_build_map_2() == {'a': 32, 'b': 64}
assert raijit_test_build_map_3() == {'a': 32, 'b': 64, 'c': 128}
assert raijit_test_build_list_0() == []
assert raijit_test_build_list_1() == [1, 2]
assert raijit_test_build_tuple_0() == (10, 20)
assert raijit_test_build_set_0() == set()
assert raijit_test_contains_op() == True
assert raijit_test_is_op() == False
