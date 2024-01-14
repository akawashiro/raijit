#include "write_insts.h"
#include <cstdint>

uint8_t *WriteEndbr64(uint8_t *addr) {
  uint8_t *p = (uint8_t *)addr;
  p[0] = 0xf3;
  p[1] = 0x0f;
  p[2] = 0x1e;
  p[3] = 0xfa;
  return p + 4;
}

uint8_t *WritePushRbp(uint8_t *addr) {
  uint8_t *p = (uint8_t *)addr;
  p[0] = 0x55;
  return p + 1;
}

uint8_t *WriteMovRspToRbp(uint8_t *addr) {
  uint8_t *p = (uint8_t *)addr;
  p[0] = 0x48;
  p[1] = 0x89;
  p[2] = 0xe5;
  return p + 3;
}

uint8_t *WriteMovRdx(uint8_t *addr, uint64_t value) {
  uint8_t *p = (uint8_t *)addr;
  p[0] = 0x48;
  p[1] = 0xba;
  memcpy(p + 2, &value, 8);
  return p + 10;
}

uint8_t *WriteMovRax(uint8_t *addr, uint64_t value) {
  uint8_t *p = (uint8_t *)addr;
  p[0] = 0x48;
  p[1] = 0xb8;
  memcpy(p + 2, &value, 8);
  return p + 10;
}

uint8_t *WriteMovRdi(uint8_t *addr, uint64_t value) {
  uint8_t *p = (uint8_t *)addr;
  p[0] = 0x48;
  p[1] = 0xbf;
  memcpy(p + 2, &value, 8);
  return p + 10;
}

uint8_t *WriteMovTo2ndArgFromImm(uint8_t *addr, uint64_t value) {
  uint8_t *p = (uint8_t *)addr;
  // mov rsi, imm64
  p[0] = 0x48;
  p[1] = 0xbe;
  memcpy(p + 2, &value, 8);
  return p + 10;
}

uint8_t *WriteMovTo1stArgFromImm(uint8_t *addr, uint64_t value) {
  uint8_t *p = (uint8_t *)addr;
  // mov rdi, imm64
  p[0] = 0x48;
  p[1] = 0xbf;
  memcpy(p + 2, &value, 8);
  return p + 10;
}

uint8_t *WriteMovTo2ndArgFromRet(uint8_t *addr) {
  uint8_t *p = (uint8_t *)addr;
  // mov rsi, rax
  p[0] = 0x48;
  p[1] = 0x89;
  p[2] = 0xc6;
  return p + 3;
}

uint8_t *WriteMovTo1stArgFromR12(uint8_t *addr) {
  uint8_t *p = (uint8_t *)addr;
  // mov rdi, r12
  p[0] = 0x4c;
  p[1] = 0x89;
  p[2] = 0xe7;
  return p + 3;
}

uint8_t *WritePushR12(uint8_t *addr) {
  uint8_t *p = (uint8_t *)addr;
  // push r12
  p[0] = 0x41;
  p[1] = 0x54;
  return p + 2;
}

uint8_t *WriteMovToRaxFromPtrRdi(uint8_t *addr) {
  uint8_t *p = (uint8_t *)addr;
  // mov rax, QWORD PTR [rdi]
  p[0] = 0x48;
  p[1] = 0x8b;
  p[2] = 0x07;
  return p + 3;
}

uint8_t *WriteMovToR12FromRet(uint8_t *addr) {
  uint8_t *p = (uint8_t *)addr;
  // mov r12, rax
  p[0] = 0x49;
  p[1] = 0x89;
  p[2] = 0xc4;
  return p + 3;
}

uint8_t *WritePopR13(uint8_t *addr) {
  uint8_t *p = (uint8_t *)addr;
  // pop r13
  p[0] = 0x41;
  p[1] = 0x5d;
  return p + 2;
}

uint8_t *WriteMovToPtrRaxFromRdi(uint8_t *addr) {
  uint8_t *p = (uint8_t *)addr;
  // mov QWORD PTR [rax],rdi
  p[0] = 0x48;
  p[1] = 0x89;
  p[2] = 0x38;
  return p + 3;
}

uint8_t *WriteMovTo1stArgFromR13(uint8_t *addr) {
  uint8_t *p = (uint8_t *)addr;
  // mov rdi, r13
  p[0] = 0x4c;
  p[1] = 0x89;
  p[2] = 0xef;
  return p + 3;
}

uint8_t *WriteMovRsi(uint8_t *addr, uint64_t value) {
  uint8_t *p = (uint8_t *)addr;
  p[0] = 0x48;
  p[1] = 0xbe;
  memcpy(p + 2, &value, 8);
  return p + 10;
}

uint8_t *WriteMovToRdiFromRax(uint8_t *addr) {
  uint8_t *p = (uint8_t *)addr;
  // mov rdi, rax
  p[0] = 0x48;
  p[1] = 0x89;
  p[2] = 0xc7;
  return p + 3;
}

uint8_t *WritePushRax(uint8_t *addr) {
  uint8_t *p = (uint8_t *)addr;
  p[0] = 0x50;
  return p + 1;
}

uint8_t *WritePushRsi(uint8_t *addr) {
  uint8_t *p = (uint8_t *)addr;
  p[0] = 0x56;
  return p + 1;
}

uint8_t *WritePushRdi(uint8_t *addr) {
  uint8_t *p = (uint8_t *)addr;
  p[0] = 0x57;
  return p + 1;
}

uint8_t *WriteNop(uint8_t *addr) {
  uint8_t *p = (uint8_t *)addr;
  p[0] = 0x90;
  return p + 1;
}

uint8_t *WritePopRax(uint8_t *addr) {
  uint8_t *p = (uint8_t *)addr;
  p[0] = 0x58;
  return p + 1;
}

uint8_t *WritePopRsi(uint8_t *addr) {
  uint8_t *p = (uint8_t *)addr;
  p[0] = 0x5e;
  return p + 1;
}

uint8_t *WritePopRdx(uint8_t *addr) {
  uint8_t *p = (uint8_t *)addr;
  p[0] = 0x5a;
  return p + 1;
}

uint8_t *WritePopRdi(uint8_t *addr) {
  uint8_t *p = (uint8_t *)addr;
  p[0] = 0x5f;
  return p + 1;
}

uint8_t *WriteLeave(uint8_t *addr) {
  uint8_t *p = (uint8_t *)addr;
  p[0] = 0xc9;
  return p + 1;
}

uint8_t *WriteRet(uint8_t *addr) {
  uint8_t *p = (uint8_t *)addr;
  p[0] = 0xc3;
  return p + 1;
}

uint8_t *WriteCallRax(uint8_t *addr) {
  uint8_t *p = (uint8_t *)addr;
  // push rsp
  p[0] = 0x54;
  // push   QWORD PTR [rsp]
  // ff 34 24
  p[1] = 0xff;
  p[2] = 0x34;
  p[3] = 0x24;
  // and    rsp,0xfffffffffffffff0
  // 48 83 e4 f0
  p[4] = 0x48;
  p[5] = 0x83;
  p[6] = 0xe4;
  p[7] = 0xf0;
  // call   rax
  // ff d0
  p[8] = 0xff;
  p[9] = 0xd0;
  // mov    rsp,QWORD PTR [rsp+0x8]
  // 48 8b 64 24 08
  p[10] = 0x48;
  p[11] = 0x8b;
  p[12] = 0x64;
  p[13] = 0x24;
  p[14] = 0x08;
  return p + 15;
}

uint8_t *WriteCmpRaxImm8(uint8_t *addr, uint8_t imm8) {
  uint8_t *p = (uint8_t *)addr;
  // cmp rax, imm8
  p[0] = 0x48;
  p[1] = 0x83;
  p[2] = 0xf8;
  p[3] = imm8;
  return p + 4;
}

uint8_t *WriteJzRel32(uint8_t *addr, int32_t rel32) {
  uint8_t *p = (uint8_t *)addr;
  // je rel32
  p[0] = 0x0f;
  p[1] = 0x84;
  memcpy(p + 2, &rel32, 4);
  return p + 6;
}

uint8_t *WriteJnzRel32(uint8_t *addr, int32_t rel32) {
  uint8_t *p = (uint8_t *)addr;
  // jnz rel32
  p[0] = 0x0f;
  p[1] = 0x85;
  memcpy(p + 2, &rel32, 4);
  return p + 6;
}

uint8_t *WriteJmpRel32(uint8_t *addr, int32_t rel32) {
  uint8_t *p = (uint8_t *)addr;
  // jmp rel32
  p[0] = 0xe9;
  memcpy(p + 1, &rel32, 4);
  return p + 5;
}

uint8_t *WriteSoftwareBreakpoint(uint8_t *addr) {
  uint8_t *p = (uint8_t *)addr;
  // int3
  p[0] = 0xcc;
  return p + 1;
}
