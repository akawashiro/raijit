# raijit

## TODO
- Run [pyperformance](https://github.com/python/pyperformance).
- You can list up all unimplemented opcodes using `./scripts/list_up_unimplmented_ops.sh`.

## How to build
Because all scripts in this project assume `build` as the build directory, please do not change the build directory.
```console
$ cmake -G Ninja -S src -B build -D CMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_POSITION_INDEPENDENT_CODE=ON
$ cmake --build build
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
