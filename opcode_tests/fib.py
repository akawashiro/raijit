import raijit

def fib(n):
    if n <= 1:
        return n
    else:
        return fib(n - 1) + fib(n - 2)

raijit.enable()
assert fib(11) == 89
