import raijit

def raijit_test_identity(a):
    return a

raijit.enable()
assert raijit_test_identity(42) == 42
