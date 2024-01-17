import dis

def raijit_test_unpack_sequence():
    (a, b) = (1.0, 2.0)
    return a

def raijit_test_copy():
    pos = [1.0, 2.0]
    pos[0] -= 1.0
    return pos

dis.dis(raijit_test_unpack_sequence)

answer = raijit_test_unpack_sequence()

import raijit

raijit.enable()
raijit_result = raijit_test_unpack_sequence()
raijit.disable()

assert answer == raijit_result

answer = raijit_test_copy()

raijit.enable()
raijit_result = raijit_test_copy()
raijit.disable()

assert answer == raijit_result
