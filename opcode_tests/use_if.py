import raijit
raijit.enable()

def raijit_test_use_if_1(a):
    if a:
        return 1
    else:
        return 2

assert raijit_test_use_if_1(True) == 1
assert raijit_test_use_if_1(False) == 2

def raijit_test_use_if(a, b):
    if a > b:
        return a
    else:
        return b

assert raijit_test_use_if(33, 1) == 33
assert raijit_test_use_if(1, 33) == 33
