import raijit

def fib(n):
    if n <= 1:
        return n
    else:
        return fib(n - 1) + fib(n - 2)

raijit.enable()
r = fib(11)
raijit.disable()
assert fib(11) == 89
