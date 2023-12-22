import random

def load_method():
    random.seed(0)
    return random.random()

r0 = load_method()

import raijit
raijit.enable()
r1 = load_method()

assert r0 == r1
