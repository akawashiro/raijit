import raijit

def raijit_test_build_map():
    return {'a': 32}

def raijit_test_map_ref_in_func():
    return {'a': 32}['a']

def raijit_test_map_ref(d):
    return d['a']

def raijit_test_map_change(d):
    d['a'] = 33
    return d

def raijit_test_local_var_map():
    d = {'a': 32}
    return d

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

raijit.enable()

assert raijit_test_build_map() == {'a': 32}
assert raijit_test_map_ref_in_func() == 32
assert raijit_test_map_ref({'a': 32}) == 32
assert raijit_test_map_change({'a': 32}) == {'a': 33}
assert raijit_test_local_var_map() == {'a': 32}
assert raijit_test_map_setitem() == {'a': 32, 'b': 64}
assert raijit_test_build_map_2() == {'a': 32, 'b': 64}
assert raijit_test_build_map_3() == {'a': 32, 'b': 64, 'c': 128}
