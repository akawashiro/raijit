#include "disasm.h"

// C++ includes
#include <chrono>
#include <fstream>
#include <iostream>

// Third party includes
#include <Zydis/Zydis.h>

// Local includes
#include "log.h"
#include "opcode_table.h"

void Disasm(const PyCodeObject *code, const PyObject *co_code,
            const char *code_buf, const uint8_t *code_mem,
            const uint8_t *code_ptr, const std::vector<uint8_t *> &code_addr,
            const size_t n_op) {
  const std::string func_name = PyUnicode_AsUTF8(code->co_name);
  const std::string now_str =
      std::to_string(duration_cast<std::chrono::milliseconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count());
  const auto disasm_file =
      std::filesystem::temp_directory_path() /
      (std::string("raijit_disasm_") + func_name + "_" + now_str);
  const auto disasm_sym = std::filesystem::temp_directory_path() /
                          (std::string("raijit_disasm_") + func_name);

  if(std::filesystem::exists(disasm_sym)) {
    std::filesystem::remove(disasm_sym);
  }
  std::filesystem::create_symlink(disasm_file, disasm_sym);

  ZyanU64 runtime_address = reinterpret_cast<ZyanU64>(code_mem);
  // Loop over the instructions in our buffer.
  ZyanUSize offset = 0;
  ZydisDisassembledInstruction instruction;
  size_t opcode_count = 0;

  FILE *fp = fopen(disasm_file.c_str(), "w");
  while (ZYAN_SUCCESS(ZydisDisassembleIntel(
      /* machine_mode:    */ ZYDIS_MACHINE_MODE_LONG_64,
      /* runtime_address: */ runtime_address,
      /* buffer:          */ reinterpret_cast<const ZyanU8 *>(code_mem) +
          offset,
      // /* length:          */ CODE_AREA_SIZE - offset,
      /* length:          */ code_ptr - code_mem - offset,
      /* instruction:     */ &instruction))) {
    if (code_addr[opcode_count] == code_mem + offset && opcode_count < n_op) {
      fprintf(fp, "; opcode=%s oprand=%d\n",
              OpcodeToString(code_buf[opcode_count * 2]).c_str(),
              code_buf[opcode_count * 2 + 1]);
      opcode_count++;
    }
    fprintf(fp, "%016lx ", runtime_address);
    const ZyanUSize instr_max = 16;
    const ZyanUSize instr_len = instruction.info.length;

    for (ZyanU8 i = 0; i < instr_len; ++i) {
      fprintf(fp, "%02X", code_mem[offset + i]);
    }
    for (ZyanUSize i = 0; i < instr_max - instr_len; ++i) {
      fprintf(fp, "  ");
    }
    fprintf(fp, " %s\n", instruction.text);
    offset += instruction.info.length;
    runtime_address += instruction.info.length;
  }
  fclose(fp);

  LOG(INFO) << "Disassembled code written to " << disasm_file;
  LOG(INFO) << "Printing disassembled code:";

  // Dump all contents of the file
  std::ifstream ifs(disasm_file);
  while (ifs.good()) {
    std::string line;
    std::getline(ifs, line);
    std::cout << line << std::endl;
  }
}
