// C++ includes
#include <filesystem>
#include <vector>

// Python related includes
#include <Python.h>

void Disasm(const PyCodeObject *code, const PyObject *co_code,
                             const char *code_buf, const uint8_t *code_mem,
                             const uint8_t *code_ptr,
                             const std::vector<uint8_t *>& code_addr,
                             const size_t n_op);
