import raijit

def add(a, b):
    return a + b

def sub(a, b):
    return a - b

raijit.enable()
assert add(42, 24) == 66
assert sub(42, 24) == 18
