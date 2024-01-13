# raijit
`raijit` is CPython JIT compiler.

## Note
`raijit` is under development. Any contribution is welcome.

## TODO
- Run [pyperformance](https://github.com/python/pyperformance).
- You can list up all unimplemented opcodes using `./scripts/list_up_unimplmented_ops.sh`.

## How to build
Because all scripts in this project assume `build` as the build directory, please do not change the build directory.
```console
$ cmake -G Ninja -S src -B build -D CMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_POSITION_INDEPENDENT_CODE=ON
$ cmake --build build
```

## How to use
```
$ cat id.py
import raijit

def identity(a):
    return a

raijit.enable()
assert identity(42) == 42
$ export PYTHONPATH=./build
$ python3 id.py
```

## How to implement opcode
1. Add a test under `./opcode_tests` directory.
2. Check it fails with `./scripts/run_opcode_tests.sh`.
3. Implement
4. Check it pass with `./scripts/run_opcode_tests.sh`.

## Run tests
```console
$ ./scripts/run_opcode_tests.sh
```

Run CI tests on your computer using
```console
$ ./scripts/run_ci_local.sh
```

## Directory structure
- `./src`: All C++ source code files
- `./scripts`: All shellscripts for development
- `./opcode_tests`: Test files
- `./users/<YOUR GITHUB ID>`: You can put anything in this directory
- `./ci`: All CI related files
