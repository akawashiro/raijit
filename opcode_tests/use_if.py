import raijit

def use_if(a, b):
    if a > b:
        return a
    else:
        return b

raijit.enable()
assert use_if(33, 1) == 33
assert use_if(1, 33) == 33
