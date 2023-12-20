#include <sys/mman.h>

#include <cstddef>
#include <cstdint>

#include <iomanip>
#include <iostream>

// Python related includes
#include <Python.h>
#include <boolobject.h>
#include <frameobject.h>
#include <object.h>
#include <opcode.h>

#include <Zydis/Zydis.h>

#include "log.h"
#include "opcode_table.h"
#include "write_insts.h"

void PrintPyObject(PyObject *v) {
  if (PyLong_Check(v)) {
    LOG(INFO) << LOG_SHOW(PyLong_AsLong(v));
  } else if (PyDict_Check(v)) {
    LOG(INFO) << LOG_SHOW(PyDict_Size(v)) << LOG_SHOW(v->ob_refcnt);
    PyObject *key, *value;
    Py_ssize_t pos = 0;
    while (PyDict_Next(v, &pos, &key, &value)) {
      LOG(INFO) << LOG_SHOW(PyUnicode_AsUTF8(key)) << LOG_SHOW(value);
    }
  } else if (PyUnicode_Check(v)) {
    LOG(INFO) << LOG_SHOW(PyUnicode_AsUTF8(v));
  } else if (PyTuple_Check(v)) {
    LOG(INFO) << LOG_SHOW(PyTuple_Size(v));
  } else if (PyList_Check(v)) {
    LOG(INFO) << LOG_SHOW(PyList_Size(v));
  } else if (PyBool_Check(v)) {
    LOG(INFO) << LOG_SHOW(Py_IsTrue(v));
  } else if (PyFunction_Check(v)) {
    LOG(INFO) << LOG_SHOW(PyFunction_GetCode(v));
  } else {
    LOG(FATAL) << "UNKNOWN" << LOG_SHOW(v);
  }
}

PyObject *PyLongAdd(PyObject *v, PyObject *w) {
  if (PyLong_Check(v) && PyLong_Check(w)) {
    return PyLong_FromLong(PyLong_AsLong(v) + PyLong_AsLong(w));
  } else {
    LOG(FATAL) << "PyLongAdd";
  }
}

PyObject *PyLongSub(PyObject *v, PyObject *w) {
  if (PyLong_Check(v) && PyLong_Check(w)) {
    return PyLong_FromLong(PyLong_AsLong(v) - PyLong_AsLong(w));
  } else {
    LOG(FATAL) << "PyLongAdd";
  }
}

PyObject *PyLongGT(PyObject *v, PyObject *w) {
  if (PyLong_Check(v) && PyLong_Check(w)) {
    return PyBool_FromLong(PyLong_AsLong(v) > PyLong_AsLong(w));
  } else {
    LOG(FATAL) << "PyLongGT";
  }
}

PyObject *PyLongEQ(PyObject *v, PyObject *w) {
  if (PyLong_Check(v) && PyLong_Check(w)) {
    return PyBool_FromLong(PyLong_AsLong(v) == PyLong_AsLong(w));
  } else {
    LOG(FATAL) << "PyLongGT";
  }
}

PyObject *PyLongLE(PyObject *v, PyObject *w) {
  if (PyLong_Check(v) && PyLong_Check(w)) {
    return PyBool_FromLong(PyLong_AsLong(v) <= PyLong_AsLong(w));
  } else {
    LOG(FATAL) << "PyLongGT";
  }
}

PyObject *ConvertToPyBool(bool v) {
  if (v) {
    Py_RETURN_TRUE;
  } else {
    Py_RETURN_FALSE;
  }
}

uint64_t IsPyTrue(PyObject *v) {
  if (PyBool_Check(v)) {
    return Py_IsTrue(v);
  } else {
    LOG(FATAL) << "IsPyTrue";
  }
}

// TODO: Only for 32bits
struct RelocPatch {
  uint8_t *addr;
  size_t cur_op_index;
  size_t jump_op_index;
  int32_t addend;
};

std::ostream &operator<<(std::ostream &os, const RelocPatch &patch) {
  os << "RelocPatch{"
     << "addr=" << HexStringUint8Ptr(patch.addr) << ", "
     << "cur_op_index=" << patch.cur_op_index << ", "
     << "jump_op_index=" << patch.jump_op_index << ", "
     << "addend=" << patch.addend << "}";
  return os;
}

PyObject *RaijitEvalFrame(PyThreadState *ts, PyFrameObject *f, int throwflag) {
  const std::string func_name = PyUnicode_AsUTF8(f->f_code->co_name);
  if (std::getenv("RAIJIT_TEST_MODE") != NULL &&
      !func_name.starts_with("raijit_test_")) {
    return _PyEval_EvalFrameDefault(ts, f, throwflag);
  }
  LOG(INFO) << LOG_KEY_VALUE("f->f_code->co_name", func_name);

  const int CODE_AREA_SIZE = 4096;
  uint8_t *code_mem = reinterpret_cast<uint8_t *>(
      mmap(NULL, CODE_AREA_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC,
           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
  uint8_t *code_ptr = code_mem;
  code_ptr = WriteEndbr64(code_ptr);
  code_ptr = WritePushRbp(code_ptr);
  code_ptr = WriteMovRspToRbp(code_ptr);

  bool compile_success = true;

  const size_t n_byte = PyBytes_Size(f->f_code->co_code);
  const size_t n_op = n_byte / 2;
  std::vector<uint8_t *> code_addr;
  std::vector<RelocPatch> reloc_patch;
  const int32_t rel32_dummy_value = 0xdeadbeef;
  {
    const char *code_buf = PyBytes_AsString(f->f_code->co_code);
    for (size_t op_index = 0; op_index < n_op; op_index++) {
      code_addr.emplace_back(code_ptr);
      const uint8_t *code_op_head = code_ptr;
      const uint8_t opcode = code_buf[op_index * 2];
      const uint8_t oprand = code_buf[op_index * 2 + 1];
      switch (opcode) {
      case IS_OP: {
        code_ptr = WritePopRsi(code_ptr);
        code_ptr = WritePopRdi(code_ptr);
        code_ptr = WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(Py_Is));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        code_ptr = WritePop1stArg(code_ptr);
        code_ptr =
            WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(ConvertToPyBool));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case CONTAINS_OP: {
        code_ptr = WritePop1stArg(code_ptr);
        code_ptr = WritePop2ndArg(code_ptr);
        code_ptr = WriteMovRax(code_ptr,
                               reinterpret_cast<uint64_t>(PySequence_Contains));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        code_ptr = WritePop1stArg(code_ptr);
        code_ptr =
            WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(ConvertToPyBool));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case STORE_SUBSCR: {
        // TODO: Broken?
        code_ptr = WritePop2ndArg(code_ptr);
        code_ptr = WritePop1stArg(code_ptr);
        code_ptr = WritePop3rdArg(code_ptr);
        // PyDict_SetItem do not return dict. It just mutate dict.
        code_ptr = WritePush1stArg(code_ptr);
        code_ptr =
            WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(PyDict_SetItem));
        code_ptr = WriteCallRax(code_ptr);
        break;
      }
      case UNARY_POSITIVE: {
        code_ptr = WritePopRdi(code_ptr);
        code_ptr = WriteMovRax(code_ptr,
                               reinterpret_cast<uint64_t>(PyNumber_Positive));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case UNARY_NEGATIVE: {
        code_ptr = WritePopRdi(code_ptr);
        code_ptr = WriteMovRax(code_ptr,
                               reinterpret_cast<uint64_t>(PyNumber_Negative));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case UNARY_NOT: {
        code_ptr = WritePopRdi(code_ptr);
        code_ptr = WriteMovRax(code_ptr,
                               reinterpret_cast<uint64_t>(PyObject_Not));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        code_ptr = WritePop1stArg(code_ptr);
        code_ptr =
            WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(ConvertToPyBool));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case UNARY_INVERT: {
        code_ptr = WritePopRdi(code_ptr);
        code_ptr = WriteMovRax(code_ptr,
                               reinterpret_cast<uint64_t>(PyNumber_Invert));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case BINARY_SUBSCR: {
        code_ptr = WritePopRsi(code_ptr);
        code_ptr = WritePopRdi(code_ptr);
        code_ptr =
            WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(PyDict_GetItem));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case BUILD_CONST_KEY_MAP: {
        code_ptr =
            WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(PyDict_New));
        code_ptr = WriteCallRax(code_ptr);
        // R12 is dict.
        code_ptr = WriteMovToR12FromRet(code_ptr);
        // R13 is key.
        code_ptr = WritePopR13(code_ptr);

        for (size_t i = 0; i < oprand; i++) {
          code_ptr = WriteNop(code_ptr);
          code_ptr = WriteMovRax(code_ptr,
                                 reinterpret_cast<uint64_t>(PyTuple_GetItem));
          code_ptr = WriteMovTo1stArgFromR13(code_ptr);
          code_ptr = WriteMovTo2ndArgFromImm(code_ptr, oprand - 1 - i);
          code_ptr = WriteCallRax(code_ptr);

          code_ptr = WriteMovTo2ndArgFromRet(code_ptr);
          code_ptr = WriteMovTo1stArgFromR12(code_ptr);
          code_ptr = WritePop3rdArg(code_ptr);

          // 1st arg is dict.
          // 2nd arg is key.
          // 3rd arg is value.
          code_ptr =
              WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(PyDict_SetItem));
          code_ptr = WriteCallRax(code_ptr);
        }

        code_ptr = WritePushR12(code_ptr);
        break;
      }
      case BUILD_MAP: {
        code_ptr =
            WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(PyDict_New));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);

        for (size_t i = 0; i < oprand; i++) {
          code_ptr = WritePop1stArg(code_ptr);
          code_ptr = WritePop3rdArg(code_ptr);
          code_ptr = WritePop2ndArg(code_ptr);
          // PyDict_SetItem do not return dict. It just mutate dict.
          code_ptr = WritePush1stArg(code_ptr);
          code_ptr =
              WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(PyDict_SetItem));
          code_ptr = WriteCallRax(code_ptr);
        }
        break;
      }
      case ROT_TWO: {
        code_ptr = WritePopRax(code_ptr);
        code_ptr = WritePopRdi(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        code_ptr = WritePushRdi(code_ptr);
        break;
      }
      case ROT_THREE: {
        code_ptr = WritePopRax(code_ptr);
        code_ptr = WritePopRdi(code_ptr);
        code_ptr = WritePopRsi(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        code_ptr = WritePushRsi(code_ptr);
        code_ptr = WritePushRdi(code_ptr);
        break;
      }
      case CALL_FUNCTION: {
        if (oprand == 0) {
          code_ptr = WritePop1stArg(code_ptr);
          code_ptr = WriteMovRax(
              code_ptr, reinterpret_cast<uint64_t>(PyObject_CallNoArgs));
          code_ptr = WriteCallRax(code_ptr);
          code_ptr = WritePushRax(code_ptr);

        } else {
          CHECK_EQ(oprand, 1);
          code_ptr = WritePopRsi(code_ptr);
          code_ptr = WritePopRdi(code_ptr);
          code_ptr = WriteMovRax(
              code_ptr, reinterpret_cast<uint64_t>(PyObject_CallOneArg));
          code_ptr = WriteCallRax(code_ptr);
          code_ptr = WritePushRax(code_ptr);
        }
        break;
      }
      case LOAD_FAST: {
        PyObject **local = &(f->f_localsplus[oprand]);
        code_ptr = WriteMovRdi(code_ptr, reinterpret_cast<uint64_t>(local));
        code_ptr = WriteMovToRaxFromPtrRdi(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        // Breakpoint
        // code_ptr = WriteSoftwareBreakpoint(code_ptr);
        break;
      }
      case STORE_FAST: {
        // https://github.com/python/cpython/blob/6c2f34fa77f884bd79801a9ab8a117cab7d9c7ed/Python/ceval.c#L1879-L1884
        PyObject **local = &(f->f_localsplus[oprand]);
        code_ptr = WritePopRdi(code_ptr);
        code_ptr = WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(local));
        code_ptr = WriteMovToPtrRaxFromRdi(code_ptr);
        break;
      }
      case BUILD_LIST: {
        code_ptr = WriteMovTo1stArgFromImm(code_ptr, oprand);
        code_ptr =
            WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(PyList_New));
        code_ptr = WriteCallRax(code_ptr);
        // R12 is list.
        code_ptr = WriteMovToR12FromRet(code_ptr);

        for (size_t i = 0; i < oprand; i++) {
          code_ptr = WriteMovTo1stArgFromR12(code_ptr);
          code_ptr = WriteMov2ndArgFromImm(code_ptr, oprand - i - 1);
          code_ptr = WritePop3rdArg(code_ptr);
          code_ptr =
              WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(PyList_SetItem));
          code_ptr = WriteCallRax(code_ptr);
        }

        code_ptr = WritePushR12(code_ptr);
        break;
      }
      case BUILD_TUPLE: {
        code_ptr = WriteMovTo1stArgFromImm(code_ptr, oprand);
        code_ptr =
            WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(PyTuple_New));
        code_ptr = WriteCallRax(code_ptr);
        // R12 is tuple.
        code_ptr = WriteMovToR12FromRet(code_ptr);

        for (size_t i = 0; i < oprand; i++) {
          code_ptr = WriteMovTo1stArgFromR12(code_ptr);
          code_ptr = WriteMov2ndArgFromImm(code_ptr, oprand - i - 1);
          code_ptr = WritePop3rdArg(code_ptr);
          code_ptr = WriteMovRax(code_ptr,
                                 reinterpret_cast<uint64_t>(PyTuple_SetItem));
          code_ptr = WriteCallRax(code_ptr);
        }

        code_ptr = WritePushR12(code_ptr);
        break;
      }
      case GET_ITER: {
        code_ptr = WritePopRdi(code_ptr);
        code_ptr =
            WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(PyObject_GetIter));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case LOAD_GLOBAL: {
        const auto name = PyTuple_GET_ITEM(f->f_code->co_names, oprand);
        PyObject *global = PyDict_GetItem(f->f_globals, name);
        if (global == NULL) {
          global = PyDict_GetItem(f->f_builtins, name);
        }
        CHECK_NOTNULL(global);
        LOG(INFO) << LOG_SHOW(PyUnicode_AsUTF8(name)) << LOG_SHOW(global);
        code_ptr = WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(global));
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case LOAD_CONST: {
        const auto const_ = PyTuple_GET_ITEM(f->f_code->co_consts, oprand);
        code_ptr = WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(const_));
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case RETURN_VALUE: {
        code_ptr = WritePopRax(code_ptr);
        // code_ptr = WriteSoftwareBreakpoint(code_ptr);
        code_ptr = WriteLeave(code_ptr);
        code_ptr = WriteRet(code_ptr);
        break;
      }
      case BINARY_ADD: {
        code_ptr = WritePopRsi(code_ptr);
        code_ptr = WritePopRdi(code_ptr);
        code_ptr = WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(PyLongAdd));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case BINARY_SUBTRACT: {
        code_ptr = WritePopRsi(code_ptr);
        code_ptr = WritePopRdi(code_ptr);
        code_ptr = WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(PyLongSub));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case BINARY_OR: {
        code_ptr = WritePopRsi(code_ptr);
        code_ptr = WritePopRdi(code_ptr);
        code_ptr =
            WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(PyNumber_Or));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case BINARY_AND: {
        code_ptr = WritePopRsi(code_ptr);
        code_ptr = WritePopRdi(code_ptr);
        code_ptr =
            WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(PyNumber_And));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case BINARY_XOR: {
        code_ptr = WritePopRsi(code_ptr);
        code_ptr = WritePopRdi(code_ptr);
        code_ptr =
            WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(PyNumber_Xor));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case BINARY_LSHIFT: {
        code_ptr = WritePopRsi(code_ptr);
        code_ptr = WritePopRdi(code_ptr);
        code_ptr =
            WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(PyNumber_Lshift));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case BINARY_RSHIFT: {
        code_ptr = WritePopRsi(code_ptr);
        code_ptr = WritePopRdi(code_ptr);
        code_ptr =
            WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(PyNumber_Rshift));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case BINARY_MULTIPLY: {
        code_ptr = WritePopRsi(code_ptr);
        code_ptr = WritePopRdi(code_ptr);
        code_ptr = WriteMovRax(code_ptr,
                               reinterpret_cast<uint64_t>(PyNumber_Multiply));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case BINARY_TRUE_DIVIDE: {
        code_ptr = WritePopRsi(code_ptr);
        code_ptr = WritePopRdi(code_ptr);
        code_ptr = WriteMovRax(code_ptr,
                               reinterpret_cast<uint64_t>(PyNumber_TrueDivide));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case BINARY_FLOOR_DIVIDE: {
        code_ptr = WritePopRsi(code_ptr);
        code_ptr = WritePopRdi(code_ptr);
        code_ptr = WriteMovRax(
            code_ptr, reinterpret_cast<uint64_t>(PyNumber_FloorDivide));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case BINARY_MODULO: {
        code_ptr = WritePopRsi(code_ptr);
        code_ptr = WritePopRdi(code_ptr);
        code_ptr = WriteMovRax(code_ptr,
                               reinterpret_cast<uint64_t>(PyNumber_Remainder));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case BINARY_POWER: {
        code_ptr = WritePopRsi(code_ptr);
        code_ptr = WritePopRdi(code_ptr);
        code_ptr =
            WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(PyNumber_Power));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case BINARY_MATRIX_MULTIPLY: {
        code_ptr = WritePopRsi(code_ptr);
        code_ptr = WritePopRdi(code_ptr);
        code_ptr = WriteMovRax(
            code_ptr, reinterpret_cast<uint64_t>(PyNumber_MatrixMultiply));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case INPLACE_ADD: {
        code_ptr = WritePopRsi(code_ptr);
        code_ptr = WritePopRdi(code_ptr);
        code_ptr = WriteMovRax(code_ptr,
                               reinterpret_cast<uint64_t>(PyNumber_InPlaceAdd));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case INPLACE_SUBTRACT: {
        code_ptr = WritePopRsi(code_ptr);
        code_ptr = WritePopRdi(code_ptr);
        code_ptr = WriteMovRax(
            code_ptr, reinterpret_cast<uint64_t>(PyNumber_InPlaceSubtract));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case INPLACE_OR: {
        code_ptr = WritePopRsi(code_ptr);
        code_ptr = WritePopRdi(code_ptr);
        code_ptr = WriteMovRax(code_ptr,
                               reinterpret_cast<uint64_t>(PyNumber_InPlaceOr));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case INPLACE_AND: {
        code_ptr = WritePopRsi(code_ptr);
        code_ptr = WritePopRdi(code_ptr);
        code_ptr = WriteMovRax(code_ptr,
                               reinterpret_cast<uint64_t>(PyNumber_InPlaceAnd));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case INPLACE_XOR: {
        code_ptr = WritePopRsi(code_ptr);
        code_ptr = WritePopRdi(code_ptr);
        code_ptr = WriteMovRax(code_ptr,
                               reinterpret_cast<uint64_t>(PyNumber_InPlaceXor));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case INPLACE_LSHIFT: {
        code_ptr = WritePopRsi(code_ptr);
        code_ptr = WritePopRdi(code_ptr);
        code_ptr = WriteMovRax(
            code_ptr, reinterpret_cast<uint64_t>(PyNumber_InPlaceLshift));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case INPLACE_RSHIFT: {
        code_ptr = WritePopRsi(code_ptr);
        code_ptr = WritePopRdi(code_ptr);
        code_ptr = WriteMovRax(
            code_ptr, reinterpret_cast<uint64_t>(PyNumber_InPlaceRshift));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case INPLACE_MULTIPLY: {
        code_ptr = WritePopRsi(code_ptr);
        code_ptr = WritePopRdi(code_ptr);
        code_ptr = WriteMovRax(
            code_ptr, reinterpret_cast<uint64_t>(PyNumber_InPlaceMultiply));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case INPLACE_TRUE_DIVIDE: {
        code_ptr = WritePopRsi(code_ptr);
        code_ptr = WritePopRdi(code_ptr);
        code_ptr = WriteMovRax(
            code_ptr, reinterpret_cast<uint64_t>(PyNumber_InPlaceTrueDivide));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case INPLACE_FLOOR_DIVIDE: {
        code_ptr = WritePopRsi(code_ptr);
        code_ptr = WritePopRdi(code_ptr);
        code_ptr = WriteMovRax(
            code_ptr, reinterpret_cast<uint64_t>(PyNumber_InPlaceFloorDivide));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case INPLACE_MODULO: {
        code_ptr = WritePopRsi(code_ptr);
        code_ptr = WritePopRdi(code_ptr);
        code_ptr = WriteMovRax(
            code_ptr, reinterpret_cast<uint64_t>(PyNumber_InPlaceRemainder));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case INPLACE_POWER: {
        code_ptr = WritePopRsi(code_ptr);
        code_ptr = WritePopRdi(code_ptr);
        code_ptr = WriteMovRdx(code_ptr, reinterpret_cast<uint64_t>(Py_None));
        code_ptr = WriteMovRax(
            code_ptr, reinterpret_cast<uint64_t>(PyNumber_InPlacePower));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case INPLACE_MATRIX_MULTIPLY: {
        code_ptr = WritePopRsi(code_ptr);
        code_ptr = WritePopRdi(code_ptr);
        code_ptr = WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(
                                             PyNumber_InPlaceMatrixMultiply));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case COMPARE_OP: {
        code_ptr = WritePopRsi(code_ptr);
        code_ptr = WritePopRdi(code_ptr);
        switch (oprand) {
        case Py_GT: {
          code_ptr =
              WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(PyLongGT));
          break;
        }
        case Py_EQ: {
          code_ptr =
              WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(PyLongGT));
          break;
        }
        case Py_LE: {
          code_ptr =
              WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(PyLongLE));
          break;
        }
        default: {
          LOG(FATAL) << "UNKNOWN" << LOG_SHOW(int(opcode))
                     << LOG_SHOW(int(oprand));
          compile_success = false;
          break;
        }
        }
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case POP_JUMP_IF_FALSE: {
        code_ptr = WritePopRdi(code_ptr);
        code_ptr = WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(IsPyTrue));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WriteCmpRaxImm8(code_ptr, 0);
        code_ptr = WriteJeRel32(code_ptr, rel32_dummy_value);
        CHECK_LE(std::numeric_limits<int32_t>::min(), code_op_head - code_ptr);
        CHECK_LE(code_op_head - code_ptr, std::numeric_limits<int32_t>::max());
        reloc_patch.emplace_back(RelocPatch{
            .addr = code_ptr - 4,
            .cur_op_index = op_index,
            .jump_op_index = op_index + oprand / 2,
            .addend = static_cast<int32_t>(code_op_head - code_ptr),
        });
        break;
      }
      case NOP: {
        code_ptr = WriteNop(code_ptr);
        break;
      }
      case POP_TOP: {
        code_ptr = WritePopRax(code_ptr);
        break;
      }
      case MAKE_FUNCTION: {
        // https://github.com/python/cpython/blob/6c2f34fa77f884bd79801a9ab8a117cab7d9c7ed/Python/ceval.c#L4291-L4294
        // PyObject *qualname = POP();
        code_ptr = WritePopRdx(code_ptr);
        // PyObject *codeobj = POP();
        code_ptr = WritePopRdi(code_ptr);
        // Set f->f_globals
        WriteMovRsi(code_ptr, reinterpret_cast<uint64_t>(f->f_globals));

        WriteMovRax(code_ptr,
                    reinterpret_cast<uint64_t>(PyFunction_NewWithQualName));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      default: {
        LOG(FATAL) << "UNKNOWN"
                   << LOG_KEY_VALUE("opcode", OpcodeToString(opcode))
                   << LOG_KEY_VALUE("operand", uint64_t(oprand));
        compile_success = false;
        break;
      }
      }
    }
  }
  for (const auto &rp : reloc_patch) {
    int32_t *p = reinterpret_cast<int32_t *>(rp.addr);
    int32_t v =
        code_addr[rp.jump_op_index] - code_addr[rp.cur_op_index] + rp.addend;
    CHECK_EQ(*p, rel32_dummy_value);
    VLOG(3) << LOG_SHOW(rp) << LOG_BITS(*p) << LOG_BITS(v);
    *p = v;
  }

  if (compile_success) {
    const char *code_buf = PyBytes_AsString(f->f_code->co_code);
    ZyanU64 runtime_address = reinterpret_cast<ZyanU64>(code_mem);
    // Loop over the instructions in our buffer.
    ZyanUSize offset = 0;
    ZydisDisassembledInstruction instruction;
    size_t opcode_count = 0;
    while (ZYAN_SUCCESS(ZydisDisassembleIntel(
        /* machine_mode:    */ ZYDIS_MACHINE_MODE_LONG_64,
        /* runtime_address: */ runtime_address,
        /* buffer:          */ reinterpret_cast<ZyanU8 *>(code_mem) + offset,
        // /* length:          */ CODE_AREA_SIZE - offset,
        /* length:          */ code_ptr - code_mem - offset,
        /* instruction:     */ &instruction))) {
      if (code_addr[opcode_count] == code_mem + offset && opcode_count < n_op) {
        printf("; opcode=%s oprand=%d\n",
               OpcodeToString(code_buf[opcode_count * 2]).c_str(),
               code_buf[opcode_count * 2 + 1]);
        opcode_count++;
      }
      printf("%016lx ", runtime_address);
      const ZyanUSize instr_max = 16;
      const ZyanUSize instr_len = instruction.info.length;

      for (ZyanU8 i = 0; i < instr_len; ++i) {
        printf("%02X", code_mem[offset + i]);
      }
      for (ZyanUSize i = 0; i < instr_max - instr_len; ++i) {
        printf("  ");
      }
      printf(" %s\n", instruction.text);
      offset += instruction.info.length;
      runtime_address += instruction.info.length;
    }
  }
  if (compile_success) {
    PyObject *(*func)() = reinterpret_cast<PyObject *(*)()>(code_mem);
    auto result = func();
    munmap(code_mem, CODE_AREA_SIZE);
    // TODO: Delete this
    Py_INCREF(result);
    return result;
  } else {
    munmap(code_mem, CODE_AREA_SIZE);
    return _PyEval_EvalFrameDefault(ts, f, throwflag);
  }
}

PyObject *(*original_eval_frame_func)(PyThreadState *ts, PyFrameObject *f,
                                      int throwflag) = NULL;

extern "C" {

PyObject *enable(PyObject *self, PyObject *args) {
  LOG(INFO) << "enable";
  auto prev = _PyInterpreterState_GetEvalFrameFunc(PyInterpreterState_Main());
  _PyInterpreterState_SetEvalFrameFunc(PyInterpreterState_Main(),
                                       RaijitEvalFrame);
  if (prev == RaijitEvalFrame) {
    Py_RETURN_FALSE;
  }
  Py_RETURN_TRUE;
}

PyObject *disable(PyObject *self, PyObject *args) {
  LOG(INFO) << "disable";
  _PyInterpreterState_SetEvalFrameFunc(PyInterpreterState_Main(),
                                       original_eval_frame_func);
  Py_RETURN_NONE;
}

/* Define functions in module */
static PyMethodDef raijitMethods[] = {
    {"enable", enable, METH_NOARGS, "Enable JIT."},
    {"disable", disable, METH_NOARGS, "Disable JIT."},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

/* Create PyModuleDef stucture */
static struct PyModuleDef raijitStruct = {PyModuleDef_HEAD_INIT,
                                          "raijit",
                                          "",
                                          -1,
                                          raijitMethods,
                                          NULL,
                                          NULL,
                                          NULL,
                                          NULL};

/* Module initialization */
PyObject *PyInit_raijit(void) {
  original_eval_frame_func =
      _PyInterpreterState_GetEvalFrameFunc(PyInterpreterState_Main());
  return PyModule_Create(&raijitStruct);
}
}
