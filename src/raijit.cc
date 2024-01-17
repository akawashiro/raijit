#include <sys/mman.h>

// C
#include <cstddef>
#include <cstdint>

// C++
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <map>

// Python related includes
#include <Python.h>
#include <boolobject.h>
#include <frameobject.h>
#include <internal/pycore_frame.h>
#include <internal/pycore_intrinsics.h>
#include <object.h>
#include <opcode.h>

// Raijit includes
#include "disasm.h"
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

PyObject *RaijitEvalFrame(PyThreadState *ts,
                          _PyInterpreterFrame *interpreter_frame,
                          int throwflag) {
  PyCodeObject *code = reinterpret_cast<PyCodeObject *>(
      PyUnstable_InterpreterFrame_GetCode(interpreter_frame));
  PyObject *co_code = PyCode_GetCode(code);

  const std::string func_name = PyUnicode_AsUTF8(code->co_name);
  if (std::getenv("RAIJIT_TEST_MODE") != NULL &&
      !func_name.starts_with("raijit_test_")) {
    return _PyEval_EvalFrameDefault(ts, interpreter_frame, throwflag);
  }
  LOG(INFO) << "Compiling a function " << func_name;

  // PyFrameObject *f = PyThreadState_GetFrame(ts);
  const int CODE_AREA_SIZE = 4096 * 4;
  uint8_t *code_mem = reinterpret_cast<uint8_t *>(
      mmap(NULL, CODE_AREA_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC,
           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
  uint8_t *code_ptr = code_mem;
  code_ptr = WriteEndbr64(code_ptr);
  code_ptr = WritePushRbp(code_ptr);
  code_ptr = WriteMovRspToRbp(code_ptr);

  bool compile_success = true;

  const size_t n_byte = PyBytes_Size(co_code);
  const size_t n_op = n_byte / 2;
  std::vector<uint8_t *> code_addr;
  std::vector<RelocPatch> reloc_patch;
  const int32_t rel32_dummy_value = 0xdeadbeef;
  {
    const char *code_buf = PyBytes_AsString(co_code);
    for (size_t op_index = 0; op_index < n_op; op_index++) {
      CHECK_LE(code_ptr, code_mem + CODE_AREA_SIZE);

      code_addr.emplace_back(code_ptr);
      const uint8_t *code_op_head = code_ptr;
      const uint8_t opcode = code_buf[op_index * 2];
      const uint8_t oprand = code_buf[op_index * 2 + 1];
      switch (opcode) {
      case RESUME: {
        // https://github.com/python/cpython/blob/4259acd39464b292075f75b7604535cb6158c25b/Doc/library/dis.rst?plain=1#L1517-L1529
        code_ptr = WriteNop(code_ptr);
        break;
      }
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
        code_ptr =
            WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(PyObject_Not));
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
        code_ptr =
            WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(PyNumber_Invert));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case BINARY_SUBSCR: {
        code_ptr = WritePop2ndArg(code_ptr);
        code_ptr = WritePop1stArg(code_ptr);
        code_ptr =
            WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(PyObject_GetItem));
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
      case STORE_FAST: {
        // https://github.com/python/cpython/blob/6c2f34fa77f884bd79801a9ab8a117cab7d9c7ed/Python/ceval.c#L1879-L1884
        // code_ptr = WriteSoftwareBreakpoint(code_ptr);
        PyObject **local = &(interpreter_frame->localsplus[oprand]);
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
          code_ptr = WriteMovTo2ndArgFromImm(code_ptr, oprand - i - 1);
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
          code_ptr = WriteMovTo2ndArgFromImm(code_ptr, oprand - i - 1);
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
      case LOAD_FAST: {
        PyObject **local = &(interpreter_frame->localsplus[oprand]);
        code_ptr = WriteMovRdi(code_ptr, reinterpret_cast<uint64_t>(local));
        // code_ptr = WritePushRdi(code_ptr);
        code_ptr = WriteMovToRaxFromPtrRdi(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case LOAD_GLOBAL: {
        const auto name =
            PyTuple_GET_ITEM(interpreter_frame->f_code->co_names, oprand >> 1);
        LOG(INFO) << "LOAD_GLOBAL: " << PyUnicode_AsUTF8(name);
        PyObject *global = PyDict_GetItem(interpreter_frame->f_globals, name);
        if (global == NULL) {
          global = PyDict_GetItem(interpreter_frame->f_builtins, name);
        }
        CHECK_NOTNULL(global);
        LOG(INFO) << "LOAD_GLOBAL: " << PyUnicode_AsUTF8(name)
                  << LOG_SHOW(global);
        code_ptr = WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(global));
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case LOAD_CONST: {
        const auto const_ = PyTuple_GET_ITEM(code->co_consts, oprand);
        code_ptr = WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(const_));
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case LOAD_ATTR: {
        const auto name =
            PyTuple_GET_ITEM(interpreter_frame->f_code->co_names, oprand >> 1);
        LOG(INFO) << "LOAD_ATTR: " << PyUnicode_AsUTF8(name);
        code_ptr = WritePop1stArg(code_ptr);
        code_ptr =
            WriteMovTo2ndArgFromImm(code_ptr, reinterpret_cast<uint64_t>(name));
        code_ptr =
            WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(PyObject_GetAttr));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case RETURN_VALUE: {
        code_ptr = WritePopRax(code_ptr);
        code_ptr = WriteLeave(code_ptr);
        code_ptr = WriteRet(code_ptr);
        break;
      }
      case RETURN_CONST: {
        const auto const_ = PyTuple_GET_ITEM(code->co_consts, oprand);
        code_ptr = WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(const_));
        code_ptr = WriteLeave(code_ptr);
        code_ptr = WriteRet(code_ptr);
        break;
      }
      case CALL: {
        switch (oprand) {
        case 0: {
          code_ptr = WritePopRdi(code_ptr);
          code_ptr = WriteMovRax(
              code_ptr, reinterpret_cast<uint64_t>(PyObject_CallNoArgs));
          code_ptr = WriteCallRax(code_ptr);
          code_ptr = WritePushRax(code_ptr);
          break;
        }
        case 1: {
          code_ptr = WritePopRsi(code_ptr);
          code_ptr = WritePopRdi(code_ptr);
          code_ptr = WriteMovRax(
              code_ptr, reinterpret_cast<uint64_t>(PyObject_CallOneArg));
          code_ptr = WriteCallRax(code_ptr);
          code_ptr = WritePushRax(code_ptr);
          break;
        }
        default: {
          LOG(FATAL) << "Unsupported number of arguments for CALL: "
                     << LOG_SHOW(int(oprand));
          compile_success = false;
          break;
        }
        }
        break;
      }
      case CACHE: {
        // https://github.com/python/cpython/blob/4259acd39464b292075f75b7604535cb6158c25b/Doc/library/dis.rst?plain=1#L490-L505
        code_ptr = WriteNop(code_ptr);
        break;
      }
      case SWAP: {
        if (oprand == 2) {
          code_ptr = WritePopRax(code_ptr);
          code_ptr = WritePopRdi(code_ptr);
          code_ptr = WritePushRax(code_ptr);
          code_ptr = WritePushRdi(code_ptr);
          break;
        } else if (oprand == 3) {
          code_ptr = WritePopRax(code_ptr);
          code_ptr = WritePopRdi(code_ptr);
          code_ptr = WritePopRsi(code_ptr);
          code_ptr = WritePushRax(code_ptr);
          code_ptr = WritePushRdi(code_ptr);
          code_ptr = WritePushRsi(code_ptr);
          break;
        } else {
          LOG(FATAL) << "Unsupported number of arguments for SWAP: "
                     << LOG_SHOW(int(oprand));
          compile_success = false;
          break;
        }
      }
      case COPY: {
        code_ptr = WriteMovToRdiFromQwordPtrRspOffset(code_ptr, oprand * 8);
        code_ptr = WritePushRdi(code_ptr);
        break;
      }
      case FOR_ITER: {
        // https://github.com/python/cpython/blob/4259acd39464b292075f75b7604535cb6158c25b/Python/generated_cases.c.h#L3260-L3301
        code_ptr = WritePopRdi(code_ptr);
        code_ptr = WritePushRdi(code_ptr);
        code_ptr =
            WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(PyIter_Next));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        code_ptr = WriteCmpRaxImm8(code_ptr, 0);

        // Jump to END_FOR when iter_next returns NULL.
        code_ptr = WriteJzRel32(code_ptr, rel32_dummy_value);
        reloc_patch.emplace_back(RelocPatch{
            .addr = code_ptr - 4,
            .cur_op_index = op_index,
            .jump_op_index = op_index + oprand + 2, // TODO: Check this.
            .addend = static_cast<int32_t>(code_op_head - code_ptr),
        });
        break;
      }
      case END_FOR: {
        // https://github.com/python/cpython/blob/4259acd39464b292075f75b7604535cb6158c25b/Python/generated_cases.c.h#L271-L288
        // TODO: Decrease refcount of iter.
        code_ptr = WritePopRax(code_ptr);
        code_ptr = WritePopRax(code_ptr);
        break;
      }
      case JUMP_BACKWARD: {
        code_ptr = WriteJmpRel32(code_ptr, rel32_dummy_value);
        reloc_patch.emplace_back(RelocPatch{
            .addr = code_ptr - 4,
            .cur_op_index = op_index,
            .jump_op_index = op_index - oprand + 1,
            .addend = static_cast<int32_t>(code_op_head - code_ptr),
        });
        break;
      }
      case BINARY_OP_ADD_INT: {
        code_ptr = WritePopRsi(code_ptr);
        code_ptr = WritePopRdi(code_ptr);
        code_ptr = WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(PyLongAdd));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case BINARY_OP_SUBTRACT_INT: {
        code_ptr = WritePopRsi(code_ptr);
        code_ptr = WritePopRdi(code_ptr);
        code_ptr = WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(PyLongSub));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case COMPARE_OP: {
        CHECK_LE(oprand >> 4, Py_GE);

        code_ptr = WritePop2ndArg(code_ptr);
        code_ptr = WritePop1stArg(code_ptr);
        code_ptr = WriteMovTo3rdArgFromImm(code_ptr, oprand >> 4);
        code_ptr = WriteMovRax(
            code_ptr, reinterpret_cast<uint64_t>(PyObject_RichCompare));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case BINARY_OP: {
        static std::map<uint8_t, uint64_t> oprand_to_func = {
            {NB_ADD, reinterpret_cast<uint64_t>(PyNumber_Add)},
            {NB_AND, reinterpret_cast<uint64_t>(PyNumber_And)},
            {NB_OR, reinterpret_cast<uint64_t>(PyNumber_Or)},
            {NB_XOR, reinterpret_cast<uint64_t>(PyNumber_Xor)},
            {NB_TRUE_DIVIDE, reinterpret_cast<uint64_t>(PyNumber_TrueDivide)},
            {NB_FLOOR_DIVIDE, reinterpret_cast<uint64_t>(PyNumber_FloorDivide)},
            {NB_LSHIFT, reinterpret_cast<uint64_t>(PyNumber_Lshift)},
            {NB_RSHIFT, reinterpret_cast<uint64_t>(PyNumber_Rshift)},
            {NB_MULTIPLY, reinterpret_cast<uint64_t>(PyNumber_Multiply)},
            {NB_MATRIX_MULTIPLY,
             reinterpret_cast<uint64_t>(PyNumber_MatrixMultiply)},
            {NB_REMAINDER, reinterpret_cast<uint64_t>(PyNumber_Remainder)},
            {NB_SUBTRACT, reinterpret_cast<uint64_t>(PyNumber_Subtract)},
            {NB_INPLACE_ADD, reinterpret_cast<uint64_t>(PyNumber_InPlaceAdd)},
            {NB_INPLACE_AND, reinterpret_cast<uint64_t>(PyNumber_InPlaceAnd)},
            {NB_INPLACE_OR, reinterpret_cast<uint64_t>(PyNumber_InPlaceOr)},
            {NB_INPLACE_XOR, reinterpret_cast<uint64_t>(PyNumber_InPlaceXor)},
            {NB_INPLACE_TRUE_DIVIDE,
             reinterpret_cast<uint64_t>(PyNumber_InPlaceTrueDivide)},
            {NB_INPLACE_FLOOR_DIVIDE,
             reinterpret_cast<uint64_t>(PyNumber_InPlaceFloorDivide)},
            {NB_INPLACE_POWER,
             reinterpret_cast<uint64_t>(PyNumber_InPlacePower)},
            {NB_INPLACE_LSHIFT,
             reinterpret_cast<uint64_t>(PyNumber_InPlaceLshift)},
            {NB_INPLACE_RSHIFT,
             reinterpret_cast<uint64_t>(PyNumber_InPlaceRshift)},
            {NB_INPLACE_MULTIPLY,
             reinterpret_cast<uint64_t>(PyNumber_InPlaceMultiply)},
            {NB_INPLACE_MATRIX_MULTIPLY,
             reinterpret_cast<uint64_t>(PyNumber_InPlaceMatrixMultiply)},
            {NB_INPLACE_REMAINDER,
             reinterpret_cast<uint64_t>(PyNumber_InPlaceRemainder)},
            {NB_INPLACE_SUBTRACT,
             reinterpret_cast<uint64_t>(PyNumber_InPlaceSubtract)},
        };

        code_ptr = WritePop2ndArg(code_ptr);
        code_ptr = WritePop1stArg(code_ptr);
        if (oprand_to_func.contains(oprand)) {
          code_ptr = WriteMovRax(code_ptr, oprand_to_func[oprand]);
        } else if (oprand == NB_POWER) {
          code_ptr = WriteMovTo3rdArgFromImm(
              code_ptr, reinterpret_cast<uint64_t>(Py_None));
          code_ptr =
              WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(PyNumber_Power));
        } else {
          LOG(FATAL) << "UNKNOWN BINARY_OP" << LOG_SHOW(int(oprand));
          compile_success = false;
          break;
        }
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WritePushRax(code_ptr);
        break;
      }
      case POP_TOP: {
        code_ptr = WritePopRax(code_ptr);
        break;
      }
      case POP_JUMP_IF_FALSE:
      case POP_JUMP_IF_TRUE: {
        code_ptr = WritePopRdi(code_ptr);
        // code_ptr = WriteSoftwareBreakpoint(code_ptr);
        code_ptr = WriteMovRax(code_ptr, reinterpret_cast<uint64_t>(IsPyTrue));
        code_ptr = WriteCallRax(code_ptr);
        code_ptr = WriteCmpRaxImm8(code_ptr, 0);
        if (opcode == POP_JUMP_IF_TRUE) {
          code_ptr = WriteJnzRel32(code_ptr, rel32_dummy_value);
        } else {
          code_ptr = WriteJzRel32(code_ptr, rel32_dummy_value);
        }
        CHECK_LE(std::numeric_limits<int32_t>::min(), code_op_head - code_ptr);
        CHECK_LE(code_op_head - code_ptr, std::numeric_limits<int32_t>::max());
        reloc_patch.emplace_back(RelocPatch{
            .addr = code_ptr - 4,
            .cur_op_index = op_index,
            .jump_op_index = op_index + oprand + 1,
            .addend = static_cast<int32_t>(code_op_head - code_ptr),
        });
        break;
      }
      case CALL_INTRINSIC_1: {
        code_ptr = WritePop1stArg(code_ptr);
        switch (oprand) {
        case INTRINSIC_UNARY_POSITIVE: {
          code_ptr = WriteMovRax(code_ptr,
                                 reinterpret_cast<uint64_t>(PyNumber_Positive));
          break;
        }
        default: {
          LOG(FATAL) << "UNKNOWN" << LOG_SHOW(int(opcode))
                     << LOG_SHOW(int(oprand));
          compile_success = false;
        } break;
        }
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
    LOG(INFO) << LOG_SHOW(rp.jump_op_index) << LOG_SHOW(rp.cur_op_index)
              << LOG_SHOW(rp) << LOG_BITS(*p) << LOG_BITS(v);
    *p = v;
  }

  if (compile_success) {
    PyCodeObject *code = reinterpret_cast<PyCodeObject *>(
        PyUnstable_InterpreterFrame_GetCode(interpreter_frame));
    PyObject *co_code = PyCode_GetCode(code);
    const char *code_buf = PyBytes_AsString(co_code);

    Disasm(code, co_code, code_buf, code_mem, code_ptr, code_addr, n_op);
  }
  if (compile_success) {
    PyObject *(*func)() = reinterpret_cast<PyObject *(*)()>(code_mem);
    auto result = func();
    LOG(INFO) << "Result of execution of JITed function: " << result;
    munmap(code_mem, CODE_AREA_SIZE);
    // TODO: Delete this
    Py_INCREF(result);
    return result;
  } else {
    munmap(code_mem, CODE_AREA_SIZE);
    return _PyEval_EvalFrameDefault(ts, interpreter_frame, throwflag);
  }
}

PyObject *(*original_eval_frame_func)(PyThreadState *ts, _PyInterpreterFrame *f,
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
