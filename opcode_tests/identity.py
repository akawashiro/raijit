import raijit

def identity(a):
    return a

raijit.enable()
assert identity(42) == 42
