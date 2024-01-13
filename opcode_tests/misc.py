# TODO: tests in this file should be split into separate files.

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
assert raijit_test_build_list_0() == []
assert raijit_test_build_list_1() == [1, 2]
assert raijit_test_build_tuple_0() == (10, 20)
assert raijit_test_build_set_0() == set()
assert raijit_test_contains_op() == True
assert raijit_test_is_op() == False
