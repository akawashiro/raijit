import raijit

def raijit_test_for():
    a = 0
    for i in [1, 2, 3]:
        a += i
    return a

raijit.enable()

import dis
dis.dis(raijit_test_for)

assert raijit_test_for() == 6
