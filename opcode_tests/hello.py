import raijit

def hello():
    print("Hello world")

import dis
dis.dis(hello)

raijit.enable()
hello()
