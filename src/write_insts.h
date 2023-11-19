#include <stdint.h>
#include <string.h>

uint8_t *WriteEndbr64(uint8_t *addr);
uint8_t *WriteMovRspToRbp(uint8_t *addr);
uint8_t *WriteMovRax(uint8_t *addr, uint64_t value);
uint8_t *WriteMovRdx(uint8_t *addr, uint64_t value);
uint8_t *WriteNop(uint8_t *addr);

// Pop
uint8_t *WritePopRax(uint8_t *addr);
uint8_t *WritePopRsi(uint8_t *addr);
uint8_t *WritePopRdi(uint8_t *addr);
uint8_t *WritePopRdx(uint8_t *addr);
uint8_t *WritePopR13(uint8_t *addr);
// Handy aliases
const auto WritePop1stArg = WritePopRdi;
const auto WritePop2ndArg = WritePopRsi;
const auto WritePop3rdArg = WritePopRdx;

// Push
uint8_t *WritePushRax(uint8_t *addr);
uint8_t *WritePushRbp(uint8_t *addr);
uint8_t *WritePushRdi(uint8_t *addr);
uint8_t *WritePushRsi(uint8_t *addr);
// Handy aliases
const auto WritePush1stArg = WritePushRdi;

uint8_t *WriteLeave(uint8_t *addr);
uint8_t *WriteRet(uint8_t *addr);
uint8_t *WriteCallRax(uint8_t *addr);
uint8_t *WriteCmpRaxImm8(uint8_t *addr, uint8_t imm8);
uint8_t *WriteJeRel32(uint8_t *addr, int32_t rel32);
uint8_t *WriteSoftwareBreakpoint(uint8_t *addr);
uint8_t *WriteMovRsi(uint8_t *addr, uint64_t value);
uint8_t *WriteMovToPtrRaxFromRdi(uint8_t *addr);
uint8_t *WriteMovRdi(uint8_t *addr, uint64_t value);
uint8_t *WriteMovToRaxFromPtrRdi(uint8_t *addr);
uint8_t *WriteMovToRdiFromRax(uint8_t *addr);
uint8_t *WriteMovToR12FromRet(uint8_t *addr);
uint8_t *WriteMovTo1stArgFromR13(uint8_t *addr);
uint8_t *WriteMovTo2ndArgFromImm(uint8_t *addr, uint64_t value);
uint8_t* WriteMovTo2ndArgFromRet(uint8_t* addr);
uint8_t* WriteMovTo1stArgFromR12(uint8_t* addr);
uint8_t* WritePushR12(uint8_t* addr);
uint8_t *WriteMovTo1stArgFromImm(uint8_t *addr, uint64_t value);
uint8_t *WriteMov2ndArgFromImm(uint8_t *addr, uint64_t value);
