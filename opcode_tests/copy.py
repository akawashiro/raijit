import dis

def raijit_test_copy():
    pos = [1.0, 2.0]
    pos[0] -= 1.0
    return pos

dis.dis(raijit_test_copy)

pos1 = raijit_test_copy()

import raijit
raijit.enable()
pos2 = raijit_test_copy()

assert pos1 == pos2
