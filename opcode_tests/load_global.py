import raijit

global_var = 12345
def load_global():
    return global_var

raijit.enable()
assert load_global() == 12345
