import raijit

def load_const():
    return 12345

raijit.enable()
assert load_const() == 12345
