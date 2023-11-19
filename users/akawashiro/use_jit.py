import dis
import raijit

def raijit_test_sum_range(n):
    s = 0
    for i in range(n):
        s += i
    return s

def raijit_test_build_list_0():
    return []

def raijit_test_build_list_1():
    return [1, 2]

def raijit_test_list_append():
    l = []
    l.append(1)
    l.append(2)
    return l

def raijit_test_build_list_2():
    return [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]

def raijit_test_build_set_0():
    return set()

def raijit_test_set_add():
    s = set()
    s.add(1)
    s.add(2)
    return s

def raijit_test_build_tuple_0():
    a = 10
    b = 20
    return (a, b)

def raijit_test_list_to_tuple():
    l = [1, 2]
    return tuple(l)

def raijit_test_is_op():
    a = 10
    b = 20
    return a is b

dis.dis(raijit_test_is_op)
raijit.enable()
r = raijit_test_is_op()
raijit.disable()
print(r)
