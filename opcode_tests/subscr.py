import dis

def raijit_test_subscr():
    pos = []
    for i in range(3):
        pos.append(i)

    r = pos[1]
    return r

dis.dis(raijit_test_subscr)

answer = raijit_test_subscr()

import raijit
raijit.enable()
raijit_result = raijit_test_subscr()

print(f"{answer=}")
print(f"{raijit_result=}")
assert answer == raijit_result
