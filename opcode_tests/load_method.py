import random
import dis

def raijit_test_load_method():
    random.seed(0)
    return random.random()

dis.dis(raijit_test_load_method)

r0 = raijit_test_load_method()

import raijit
raijit.enable()
r1 = raijit_test_load_method()

assert r0 == r1, (r0, r1)
