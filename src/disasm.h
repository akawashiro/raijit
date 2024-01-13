// C++ includes
#include <filesystem>
#include <vector>

// Python related includes
#include <Python.h>

void Disasm(PyCodeObject *code, PyObject *co_code,
                             const char *code_buf, uint8_t *code_mem,
                             uint8_t *code_ptr,
                             std::vector<uint8_t *> code_addr,
                             const size_t n_op);
