// Implementation of OPCODES and cpu step function

#define NDEBUG
#include <assert.h>

#include "cpu.h"
#include "mem.h"


// cpu registers, halted state, and timers
CpuState globalState;

typedef void OpcodeHandler(u8 opcode);

// the GB has several invalid opcodes
void invHandler(u8 opcode) {
  printf("got invalid opcode 0x%x\n", opcode);
  assert(false);
}

////////////////////////////////
// CB Opcode Helper Functions //
////////////////////////////////

u8 rlcReg(u8 value, bool isA) {
  u8 result = value;
  clearAllFlags();
  if((result & 0x80) != 0) {
    setCarryFlag();
    result <<= 1;
    result |= 0x1;
  } else {
    result <<= 1;
  }
  if(!isA) {
    if((u8)result == 0) {
      setZeroFlag();
    }
  }
  return result;
}

u8 rlReg(u8 value, bool isA) {
  u8 carry = getCarryFlag() ? (u8)1 : (u8)0;
  u8 result = value;
  clearAllFlags();
  if((result & 0x80) != 0) {
    setCarryFlag();
  }
  result <<= 1;
  result |= carry;
  if(!isA) {
    if((u8)result == 0) {
      setZeroFlag();
    }
  }
  return result;
}

u8 rrc(u8 value, bool isA) {
  u8 result = value;
  clearAllFlags();
  if((result & 1) != 0) {
    setCarryFlag();
    result >>= 1;
    result |= 0x80;
  } else {
    result >>= 1;
  }
  if(!isA) {
    if((u8)result == 0) {
      setZeroFlag();
    }
  }
  return result;
}

u8 rr(u8 value, bool isA) {
  u8 carry = getCarryFlag() ? (u8)0x80 : (u8)0x00;
  u8 result = value;
  clearAllFlags();
  if((result & 1) != 0) {
    setCarryFlag();
  }
  result >>= 1;
  result |= carry;
  if(!isA) {
    if((u8)result == 0) {
      setZeroFlag();
    }
  }
  return result;
}

u8 srl(u8 value) {
  u8 result = value;
  clearAllFlags();
  if(result & 1) {
    setCarryFlag();
  }
  result >>= 1;
  if(result == 0) {
    setZeroFlag();
  }
  return result;
}

u8 sla(u8 value) {
  clearAllFlags();
  if(value & 0x80) {
    setCarryFlag();
  }
  u8 result = value << 1;
  if(result == 0) {
    setZeroFlag();
  }
  return result;
}

u8 sra(u8 value) {
  u8 result = value;
  clearAllFlags();
  if(result & 1) {
    setCarryFlag();
  }
  if((result & 0x80)) {
    result >>= 1;
    result |= 0x80;
  } else {
    result >>= 1;
  }
  if(result == 0) {
    setZeroFlag();
  }
  return result;
}

u8 swapRegister(u8 value) {
  u8 low = value & 0xf;
  u8 hi = (value >> 4) & 0xf;
  u8 result = (low << 4) + hi;
  clearAllFlags();
  if((u8)result == 0) {
    setZeroFlag();
  }
  return result;
}


////////////////////////////////
// CB Opcodes                 //
////////////////////////////////

void SLA_A(u8 opcode) { // 0x27
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.a = sla(globalState.a);
}

void SLA_B(u8 opcode) { // 0x20
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.bc.hi = sla(globalState.bc.hi);
}

void SLA_C(u8 opcode) { // 0x21
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.bc.lo = sla(globalState.bc.lo);
}

void SLA_D(u8 opcode) { // 0x22
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.de.hi = sla(globalState.de.hi);
}

void SLA_E(u8 opcode) { // 0x23
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.de.lo = sla(globalState.de.lo);
}

void SLA_H(u8 opcode) { // 0x24
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.hl.hi = sla(globalState.hl.hi);
}

void SLA_L(u8 opcode) { // 0x25
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.hl.lo = sla(globalState.hl.lo);
}

void SLA_DHL(u8 opcode) { // 0x26
  globalState.pc++;
  globalState.cycleCount += 16;
  writeByte(sla(readByte(globalState.hl.v)), globalState.hl.v);
}

void SRA_A(u8 opcode) { // 0x2f
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.a = sra(globalState.a);
}

void SRA_B(u8 opcode) { // 0x28
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.bc.hi = sra(globalState.bc.hi);
}

void SRA_C(u8 opcode) { // 0x29
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.bc.lo = sra(globalState.bc.lo);
}

void SRA_D(u8 opcode) { // 0x2a
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.de.hi = sra(globalState.de.hi);
}

void SRA_E(u8 opcode) { // 0x2b
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.de.lo = sra(globalState.de.lo);
}

void SRA_H(u8 opcode) { // 0x2c
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.hl.hi = sra(globalState.hl.hi);
}

void SRA_L(u8 opcode) { // 0x2d
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.hl.lo = sra(globalState.hl.lo);
}

void SRA_DHL(u8 opcode) { // 0x2e
  globalState.pc++;
  globalState.cycleCount += 16;
  writeByte(sra(readByte(globalState.hl.v)), globalState.hl.v);
}


void SRL_A(u8 opcode) { // 0x3f
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.a = srl(globalState.a);
}

void SRL_B(u8 opcode) { // 0x38
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.bc.hi = srl(globalState.bc.hi);
}

void SRL_C(u8 opcode) { // 0x39
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.bc.lo = srl(globalState.bc.lo);
}

void SRL_D(u8 opcode) { // 0x3a
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.de.hi = srl(globalState.de.hi);
}

void SRL_E(u8 opcode) { // 0x3b
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.de.lo = srl(globalState.de.lo);
}

void SRL_H(u8 opcode) { // 0x3c
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.hl.hi = srl(globalState.hl.hi);
}

void SRL_L(u8 opcode) { // 0x3d
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.hl.lo = srl(globalState.hl.lo);
}

void SRL_DHL(u8 opcode) { // 0x3e
  globalState.pc++;
  globalState.cycleCount += 16;
  writeByte(srl(readByte(globalState.hl.v)), globalState.hl.v);
}

void RR_A(u8 opcode) { // 0x1f
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.a = rr(globalState.a, false);
}

void RR_B(u8 opcode) { // 0x18
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.bc.hi = rr(globalState.bc.hi, false);
}

void RR_C(u8 opcode) { // 0x19
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.bc.lo = rr(globalState.bc.lo, false);
}

void RR_D(u8 opcode) { // 0x1a
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.de.hi = rr(globalState.de.hi, false);
}

void RR_E(u8 opcode) { // 0x1b
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.de.lo = rr(globalState.de.lo, false);
}

void RR_H(u8 opcode) { // 0x1c
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.hl.hi = rr(globalState.hl.hi, false);
}

void RR_L(u8 opcode) { // 0x1d
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.hl.lo = rr(globalState.hl.lo, false);
}

void RR_DHL(u8 opcode) { // 0x1e
  globalState.pc++;
  globalState.cycleCount += 16;
  writeByte(rr(readByte(globalState.hl.v), false), globalState.hl.v);
}

void RL_A(u8 opcode) { // 0x17
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.a = rlReg(globalState.a, false);
}

void RL_B(u8 opcode) { // 0x10
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.bc.hi = rlReg(globalState.bc.hi, false);
}

void RL_C(u8 opcode) { // 0x11
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.bc.lo = rlReg(globalState.bc.lo, false);
}

void RL_D(u8 opcode) { // 0x12
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.de.hi = rlReg(globalState.de.hi, false);
}

void RL_E(u8 opcode) { // 0x13
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.de.lo = rlReg(globalState.de.lo, false);
}

void RL_H(u8 opcode) { // 0x14
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.hl.hi = rlReg(globalState.hl.hi, false);
}

void RL_L(u8 opcode) { // 0x15
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.hl.lo = rlReg(globalState.hl.lo, false);
}

void RL_DHL(u8 opcode) { // 0x16
  globalState.pc++;
  globalState.cycleCount += 16;
  writeByte(rlReg(readByte(globalState.hl.v), false), globalState.hl.v);
}

void SWAP_A(u8 opcode) { // 37
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.a = swapRegister(globalState.a);
}

void SWAP_B(u8 opcode) { // 30
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.bc.hi = swapRegister(globalState.bc.hi);
}

void SWAP_C(u8 opcode) { // 31
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.bc.lo = swapRegister(globalState.bc.lo);
}

void SWAP_D(u8 opcode) { // 32
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.de.hi = swapRegister(globalState.de.hi);
}

void SWAP_E(u8 opcode) { // 33
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.de.lo = swapRegister(globalState.de.lo);
}

void SWAP_H(u8 opcode) { // 34
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.hl.hi = swapRegister(globalState.hl.hi);
}

void SWAP_L(u8 opcode) { // 35
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.hl.lo = swapRegister(globalState.hl.lo);
}

void SWAP_DHL(u8 opcode) { // 36
  globalState.pc++;
  globalState.cycleCount += 16;
  writeByte(swapRegister(readByte(globalState.hl.v)), globalState.hl.v);
}

void RLC_A(u8 opcode) { // 07
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.a = rlcReg(globalState.a, false);
}

void RLC_B(u8 opcode) { // 00
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.bc.hi = rlcReg(globalState.bc.hi, false);
}

void RLC_C(u8 opcode) { // 01
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.bc.lo = rlcReg(globalState.bc.lo, false);
}

void RLC_D(u8 opcode) { // 02
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.de.hi = rlcReg(globalState.de.hi, false);
}

void RLC_E(u8 opcode) { // 03
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.de.lo = rlcReg(globalState.de.lo, false);
}

void RLC_H(u8 opcode) { // 04
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.hl.hi = rlcReg(globalState.hl.hi, false);
}

void RLC_L(u8 opcode) { // 05
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.hl.lo = rlcReg(globalState.hl.lo, false);
}

void RLC_DHL(u8 opcode) { // 06
  globalState.pc++;
  globalState.cycleCount += 16;
  u8 result = rlcReg(readByte(globalState.hl.v), false);
  writeByte(result, globalState.hl.v);
}


void RRC_A(u8 opcode) { // 0f
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.a = rrc(globalState.a, false);
}

void RRC_B(u8 opcode) { // 08
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.bc.hi = rrc(globalState.bc.hi, false);
}

void RRC_C(u8 opcode) { // 09
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.bc.lo = rrc(globalState.bc.lo, false);
}

void RRC_D(u8 opcode) { // 0a
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.de.hi = rrc(globalState.de.hi, false);
}

void RRC_E(u8 opcode) { // 0b
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.de.lo = rrc(globalState.de.lo, false);
}

void RRC_H(u8 opcode) { // 0c
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.hl.hi = rrc(globalState.hl.hi, false);
}

void RRC_L(u8 opcode) { // 0d
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.hl.lo = rrc(globalState.hl.lo, false);
}

void RRC_DHL(u8 opcode) { // 0e
  globalState.pc++;
  globalState.cycleCount += 16;
  u8 result = rrc(readByte(globalState.hl.v), false);
  writeByte(result, globalState.hl.v);
}


void bit_B_set(u8 opcode) {
  u8 bitID = (opcode - (u8)0xC0) >> 3;
  assert(bitID < 8);
  globalState.bc.hi |= (1 << bitID);
  globalState.pc += 1;
  globalState.cycleCount += 8;
}

void bit_C_set(u8 opcode) {
  u8 bitID = (opcode - (u8)0xC1) >> 3;
  assert(bitID < 8);
  globalState.bc.lo |= (1 << bitID);
  globalState.pc += 1;
  globalState.cycleCount += 8;
}

void bit_D_set(u8 opcode) {
  u8 bitID = (opcode - (u8)0xC2) >> 3;
  assert(bitID < 8);
  globalState.de.hi |= (1 << bitID);
  globalState.pc += 1;
  globalState.cycleCount += 8;
}

void bit_E_set(u8 opcode) {
  u8 bitID = (opcode - (u8)0xC3) >> 3;
  assert(bitID < 8);
  globalState.de.lo |= (1 << bitID);
  globalState.pc += 1;
  globalState.cycleCount += 8;
}

void bit_H_set(u8 opcode) {
  u8 bitID = (opcode - (u8)0xC4) >> 3;
  assert(bitID < 8);
  globalState.hl.hi |= (1 << bitID);
  globalState.pc += 1;
  globalState.cycleCount += 8;
}

void bit_L_set(u8 opcode) {
  u8 bitID = (opcode - (u8)0xC5) >> 3;
  assert(bitID < 8);
  globalState.hl.lo |= (1 << bitID);
  globalState.pc += 1;
  globalState.cycleCount += 8;
}

void bit_DHL_set(u8 opcode) {
  u8 bitID = (opcode - (u8)0xC6) >> 3;
  assert(bitID < 8);
  u8 value = readByte(globalState.hl.v);
  value |= (1 << bitID);
  writeByte(value, globalState.hl.v);
  globalState.pc += 1;
  globalState.cycleCount += 16;
}

void bit_A_set(u8 opcode) {
  u8 bitID = (opcode - (u8)0xC7) >> 3;
  assert(bitID < 8);
  globalState.a |= (1 << bitID);
  globalState.pc += 1;
  globalState.cycleCount += 8;
}

void bit_B_res(u8 opcode) {
  u8 bitID = (opcode - (u8)0x80) >> 3;
  assert(bitID < 8);
  globalState.bc.hi &= ~(1 << bitID);
  globalState.pc += 1;
  globalState.cycleCount += 8;
}

void bit_C_res(u8 opcode) {
  u8 bitID = (opcode - (u8)0x81) >> 3;
  assert(bitID < 8);
  globalState.bc.lo &= ~(1 << bitID);
  globalState.pc += 1;
  globalState.cycleCount += 8;
}

void bit_D_res(u8 opcode) {
  u8 bitID = (opcode - (u8)0x82) >> 3;
  assert(bitID < 8);
  globalState.de.hi &= ~(1 << bitID);
  globalState.pc += 1;
  globalState.cycleCount += 8;
}

void bit_E_res(u8 opcode) {
  u8 bitID = (opcode - (u8)0x83) >> 3;
  assert(bitID < 8);
  globalState.de.lo &= ~(1 << bitID);
  globalState.pc += 1;
  globalState.cycleCount += 8;
}

void bit_H_res(u8 opcode) {
  u8 bitID = (opcode - (u8)0x84) >> 3;
  assert(bitID < 8);
  globalState.hl.hi &= ~(1 << bitID);
  globalState.pc += 1;
  globalState.cycleCount += 8;
}

void bit_L_res(u8 opcode) {
  u8 bitID = (opcode - (u8)0x85) >> 3;
  assert(bitID < 8);
  globalState.hl.lo &= ~(1 << bitID);
  globalState.pc += 1;
  globalState.cycleCount += 8;
}

void bit_DHL_res(u8 opcode) {
  u8 bitID = (opcode - (u8)0x86) >> 3;
  assert(bitID < 8);
  u8 value = readByte(globalState.hl.v);
  value &= ~(1 << bitID);
  writeByte(value, globalState.hl.v);
  globalState.pc += 1;
  globalState.cycleCount += 16;
}

void bit_A_res(u8 opcode) {
  u8 bitID = (opcode - (u8)0x87) >> 3;
  assert(bitID < 8);
  globalState.a &= ~(1 << bitID);
  globalState.pc += 1;
  globalState.cycleCount += 8;
}

void bit_A_test(u8 opcode) {
  u8 bitID = (opcode - (u8)0x47) >> 3;
  assert(bitID < 8);
  //printf("check bit %d of A\n", bitID);
  u8 val = globalState.a;
  if(((val >> bitID) & 1) == 0) {
    setZeroFlag();
  } else {
    clearZeroFlag();
  }
  setHalfCarryFlag();
  clearSubtractFlag();
  globalState.pc += 1;
  globalState.cycleCount += 8;
}

void bit_B_test(u8 opcode) {
  u8 bitID = (opcode - (u8)0x40) >> 3;
  //printf("B opcode 0x%x bitId %d\n", opcode, bitID);
  assert(bitID < 8);
  //printf("check bit %d of B\n", bitID);
  u8 val = globalState.bc.hi;
  if(((val >> bitID) & 1) == 0) {
    setZeroFlag();
  } else {
    clearZeroFlag();
  }
  setHalfCarryFlag();
  clearSubtractFlag();
  globalState.pc += 1;
  globalState.cycleCount += 8;
}


void bit_C_test(u8 opcode) {
  u8 bitID = (opcode - (u8)0x41) >> 3;
  assert(bitID < 8);
  //printf("check bit %d of C\n", bitID);
  u8 val = globalState.bc.lo;
  if(((val >> bitID) & 1) == 0) {
    setZeroFlag();
  } else {
    clearZeroFlag();
  }
  setHalfCarryFlag();
  clearSubtractFlag();
  globalState.pc += 1;
  globalState.cycleCount += 8;
}

void bit_D_test(u8 opcode) {
  u8 bitID = (opcode - (u8)0x42) >> 3;
  assert(bitID < 8);
  //printf("check bit %d of D\n", bitID);
  u8 val = globalState.de.hi;
  if(((val >> bitID) & 1) == 0) {
    setZeroFlag();
  } else {
    clearZeroFlag();
  }
  setHalfCarryFlag();
  clearSubtractFlag();
  globalState.pc += 1;
  globalState.cycleCount += 8;
}

void bit_E_test(u8 opcode) {
  u8 bitID = (opcode - (u8)0x43) >> 3;
  assert(bitID < 8);
  //printf("check bit %d of E\n", bitID);
  u8 val = globalState.de.lo;
  if(((val >> bitID) & 1) == 0) {
    setZeroFlag();
  } else {
    clearZeroFlag();
  }
  setHalfCarryFlag();
  clearSubtractFlag();
  globalState.pc += 1;
  globalState.cycleCount += 8;
}

void bit_H_test(u8 opcode) {
  u8 bitID = (opcode - (u8)0x44) >> 3;
  assert(bitID < 8);
  //printf("check bit %d of H\n", bitID);
  u8 val = globalState.hl.hi;
  if(((val >> bitID) & 1) == 0) {
    setZeroFlag();
  } else {
    clearZeroFlag();
  }
  setHalfCarryFlag();
  clearSubtractFlag();
  globalState.pc += 1;
  globalState.cycleCount += 8;
}

void bit_L_test(u8 opcode) {
  u8 bitID = (opcode - (u8)0x45) >> 3;
  assert(bitID < 8);
  //printf("check bit %d of L\n", bitID);
  u8 val = globalState.hl.lo;
  if(((val >> bitID) & 1) == 0) {
    setZeroFlag();
  } else {
    clearZeroFlag();
  }
  setHalfCarryFlag();
  clearSubtractFlag();
  globalState.pc += 1;
  globalState.cycleCount += 8;
}

void bit_DHL_test(u8 opcode) {
  u8 bitID = (opcode - (u8)0x45) >> 3;
  assert(bitID < 8);
  //printf("check bit %d of L\n", bitID);
  u8 val = readByte(globalState.hl.v);
  if(((val >> bitID) & 1) == 0) {
    setZeroFlag();
  } else {
    clearZeroFlag();
  }
  setHalfCarryFlag();
  clearSubtractFlag();
  globalState.pc += 1;
  globalState.cycleCount += 16;
}

// CB-prefixed opcode handler table
static OpcodeHandler* opcode_cbs[256] =
        {RLC_B,          RLC_C,          RLC_D,          RLC_E,          RLC_H,          RLC_L,          RLC_DHL,        RLC_A,          // 0x0 - 0x7
         RRC_B,          RRC_C,          RRC_D,          RRC_E,          RRC_H,          RRC_L,          RRC_DHL,        RRC_A,          // 0x8 - 0xf
         RL_B,           RL_C,           RL_D,           RL_E,           RL_H,           RL_L,           RL_DHL,         RL_A,           // 0x10 - 0x17
         RR_B,           RR_C,           RR_D,           RR_E,           RR_H,           RR_L,           RR_DHL,         RR_A,           // 0x18 - 0x1f
         SLA_B,          SLA_C,          SLA_D,          SLA_E,          SLA_H,          SLA_L,          SLA_DHL,        SLA_A,          // 0x20 - 0x27
         SRA_B,          SRA_C,          SRA_D,          SRA_E,          SRA_H,          SRA_L,          SRA_DHL,        SRA_A,          // 0x28 - 0x2f
         SWAP_B,         SWAP_C,         SWAP_D,         SWAP_E,         SWAP_H,         SWAP_L,         SWAP_DHL,       SWAP_A,         // 0x30 - 0x37
         SRL_B,          SRL_C,          SRL_D,          SRL_E,          SRL_H,          SRL_L,          SRL_DHL,        SRL_A,          // 0x38 - 0x3f
         bit_B_test,     bit_C_test,     bit_D_test,     bit_E_test,     bit_H_test,     bit_L_test,     bit_DHL_test,   bit_A_test,     // 0x40 - 0x47
         bit_B_test,     bit_C_test,     bit_D_test,     bit_E_test,     bit_H_test,     bit_L_test,     bit_DHL_test,   bit_A_test,     // 0x48 - 0x4f
         bit_B_test,     bit_C_test,     bit_D_test,     bit_E_test,     bit_H_test,     bit_L_test,     bit_DHL_test,   bit_A_test,     // 0x50 - 0x57
         bit_B_test,     bit_C_test,     bit_D_test,     bit_E_test,     bit_H_test,     bit_L_test,     bit_DHL_test,   bit_A_test,     // 0x58 - 0x5f
         bit_B_test,     bit_C_test,     bit_D_test,     bit_E_test,     bit_H_test,     bit_L_test,     bit_DHL_test,   bit_A_test,     // 0x60 - 0x67
         bit_B_test,     bit_C_test,     bit_D_test,     bit_E_test,     bit_H_test,     bit_L_test,     bit_DHL_test,   bit_A_test,     // 0x68 - 0x6f
         bit_B_test,     bit_C_test,     bit_D_test,     bit_E_test,     bit_H_test,     bit_L_test,     bit_DHL_test,   bit_A_test,     // 0x70 - 0x77
         bit_B_test,     bit_C_test,     bit_D_test,     bit_E_test,     bit_H_test,     bit_L_test,     bit_DHL_test,   bit_A_test,     // 0x78 - 0x7f
         bit_B_res,      bit_C_res,      bit_D_res,      bit_E_res,      bit_H_res,      bit_L_res,      bit_DHL_res,    bit_A_res,      // 0x80 - 0x8
         bit_B_res,      bit_C_res,      bit_D_res,      bit_E_res,      bit_H_res,      bit_L_res,      bit_DHL_res,    bit_A_res,      // 0x88 - 0x8f
         bit_B_res,      bit_C_res,      bit_D_res,      bit_E_res,      bit_H_res,      bit_L_res,      bit_DHL_res,    bit_A_res,      // 0x90 - 0x97
         bit_B_res,      bit_C_res,      bit_D_res,      bit_E_res,      bit_H_res,      bit_L_res,      bit_DHL_res,    bit_A_res,      // 0x98 - 0x9f
         bit_B_res,      bit_C_res,      bit_D_res,      bit_E_res,      bit_H_res,      bit_L_res,      bit_DHL_res,    bit_A_res,      // 0xa0 - 0xa7
         bit_B_res,      bit_C_res,      bit_D_res,      bit_E_res,      bit_H_res,      bit_L_res,      bit_DHL_res,    bit_A_res,      // 0xa8 - 0xaf
         bit_B_res,      bit_C_res,      bit_D_res,      bit_E_res,      bit_H_res,      bit_L_res,      bit_DHL_res,    bit_A_res,      // 0xb0 - 0xb7
         bit_B_res,      bit_C_res,      bit_D_res,      bit_E_res,      bit_H_res,      bit_L_res,      bit_DHL_res,    bit_A_res,      // 0xb8 - 0xbf
         bit_B_set,      bit_C_set,      bit_D_set,      bit_E_set,      bit_H_set,      bit_L_set,      bit_DHL_set,    bit_A_set     , // 0xc0 - 0xc7
         bit_B_set,      bit_C_set,      bit_D_set,      bit_E_set,      bit_H_set,      bit_L_set,      bit_DHL_set,    bit_A_set     , // 0xc8 - 0xcf
         bit_B_set,      bit_C_set,      bit_D_set,      bit_E_set,      bit_H_set,      bit_L_set,      bit_DHL_set,    bit_A_set     , // 0xd0 - 0xd7
         bit_B_set,      bit_C_set,      bit_D_set,      bit_E_set,      bit_H_set,      bit_L_set,      bit_DHL_set,    bit_A_set     , // 0xd8 - 0xdf
         bit_B_set,      bit_C_set,      bit_D_set,      bit_E_set,      bit_H_set,      bit_L_set,      bit_DHL_set,    bit_A_set     , // 0xe0 - 0xe7
         bit_B_set,      bit_C_set,      bit_D_set,      bit_E_set,      bit_H_set,      bit_L_set,      bit_DHL_set,    bit_A_set     , // 0xe8 - 0xef
         bit_B_set,      bit_C_set,      bit_D_set,      bit_E_set,      bit_H_set,      bit_L_set,      bit_DHL_set,    bit_A_set     , // 0xf0 - 0xf7
         bit_B_set,      bit_C_set,      bit_D_set,      bit_E_set,      bit_H_set,      bit_L_set,      bit_DHL_set,    bit_A_set     }; // 0xf8 - 0xff


////////////////////////////////
// Opcodes                    //
////////////////////////////////
void LD_B_n(u8 opocde) { // 06
  u8 imm = readByte(++globalState.pc);
  globalState.pc++;
  globalState.bc.hi = imm;
  globalState.cycleCount += 8;
}

void LD_C_n(u8 opocde) { // 0E
  u8 imm = readByte(++globalState.pc);
  globalState.pc++;
  globalState.bc.lo = imm;
  globalState.cycleCount += 8;
}

void LD_D_n(u8 opocde) { // 16
  u8 imm = readByte(++globalState.pc);
  globalState.pc++;
  globalState.de.hi = imm;
  globalState.cycleCount += 8;
}

void LD_E_n(u8 opocde) { // 1e
  u8 imm = readByte(++globalState.pc);
  globalState.pc++;
  globalState.de.lo = imm;
  globalState.cycleCount += 8;
}

void LD_H_n(u8 opocde) { // 26
  u8 imm = readByte(++globalState.pc);
  globalState.pc++;
  globalState.hl.hi = imm;
  globalState.cycleCount += 8;
}

void LD_L_n(u8 opocde) { // 2e
  u8 imm = readByte(++globalState.pc);
  globalState.pc++;
  globalState.hl.lo = imm;
  globalState.cycleCount += 8;
}

// REGISTER-REGISTER LOADS (REGISTER A)
void LD_A_A(u8 opcode) { // 0x7f
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.a = globalState.a;
}

void LD_A_B(u8 opcode) { // 0x78
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.a = globalState.bc.hi;
}

void LD_A_C(u8 opcode) { // 0x79
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.a = globalState.bc.lo;
}

void LD_A_D(u8 opcode) { // 0x7a
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.a = globalState.de.hi;
}

void LD_A_E(u8 opcode) { // 0x7b
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.a = globalState.de.lo;
}

void LD_A_H(u8 opcode) { // 0x7c
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.a = globalState.hl.hi;
}

void LD_A_L(u8 opcode) { // 0x7d
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.a = globalState.hl.lo;
}

void LD_A_DHL(u8 opcode) { // 0x7e
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.a = readByte(globalState.hl.v);
}



// REGISTER-REGISTER LOADS (REGISTER B)
void LD_B_B(u8 opcode) { // 0x40
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.bc.hi = globalState.bc.hi;
}

void LD_B_C(u8 opcode) { // 0x41
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.bc.hi = globalState.bc.lo;
}

void LD_B_D(u8 opcode) { // 0x42
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.bc.hi = globalState.de.hi;
}

void LD_B_E(u8 opcode) { // 0x43
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.bc.hi = globalState.de.lo;
}

void LD_B_H(u8 opcode) { // 0x44
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.bc.hi = globalState.hl.hi;
}

void LD_B_L(u8 opcode) { // 0x45
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.bc.hi = globalState.hl.lo;
}

void LD_B_DHL(u8 opcode) { // 0x46
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.bc.hi = readByte(globalState.hl.v);
}



// REGISTER-REGISTER LOADS (REGISTER C)
void LD_C_B(u8 opcode) { // 0x48
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.bc.lo = globalState.bc.hi;
}

void LD_C_C(u8 opcode) { // 0x49
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.bc.lo = globalState.bc.lo;
}

void LD_C_D(u8 opcode) { // 0x4a
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.bc.lo = globalState.de.hi;
}

void LD_C_E(u8 opcode) { // 0x4b
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.bc.lo = globalState.de.lo;
}

void LD_C_H(u8 opcode) { // 0x4c
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.bc.lo = globalState.hl.hi;
}

void LD_C_L(u8 opcode) { // 0x4d
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.bc.lo = globalState.hl.lo;
}

void LD_C_DHL(u8 opcode) { // 0x4e
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.bc.lo = readByte(globalState.hl.v);
}



// REGISTER-REGISTER LOADS (REGISTER D)
void LD_D_B(u8 opcode) { // 0x50
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.de.hi = globalState.bc.hi;
}

void LD_D_C(u8 opcode) { // 0x51
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.de.hi = globalState.bc.lo;
}

void LD_D_D(u8 opcode) { // 0x52
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.de.hi = globalState.de.hi;
}

void LD_D_E(u8 opcode) { // 0x53
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.de.hi = globalState.de.lo;
}

void LD_D_H(u8 opcode) { // 0x54
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.de.hi = globalState.hl.hi;
}

void LD_D_L(u8 opcode) { // 0x55
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.de.hi = globalState.hl.lo;
}

void LD_D_DHL(u8 opcode) { // 0x56
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.de.hi = readByte(globalState.hl.v);
}


// REGISTER-REGISTER LOADS (REGISTER E)
void LD_E_B(u8 opcode) { // 0x58
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.de.lo = globalState.bc.hi;
}

void LD_E_C(u8 opcode) { // 0x59
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.de.lo = globalState.bc.lo;
}

void LD_E_D(u8 opcode) { // 0x5a
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.de.lo = globalState.de.hi;
}

void LD_E_E(u8 opcode) { // 0x5b
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.de.lo = globalState.de.lo;
}

void LD_E_H(u8 opcode) { // 0x5c
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.de.lo = globalState.hl.hi;
}

void LD_E_L(u8 opcode) { // 0x5d
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.de.lo = globalState.hl.lo;
}

void LD_E_DHL(u8 opcode) { // 0x5e
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.de.lo = readByte(globalState.hl.v);
}

// REGISTER-REGISTER LOADS (REGISTER H)
void LD_H_B(u8 opcode) { // 0x60
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.hl.hi = globalState.bc.hi;
}

void LD_H_C(u8 opcode) { // 0x61
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.hl.hi = globalState.bc.lo;
}

void LD_H_D(u8 opcode) { // 0x62
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.hl.hi = globalState.de.hi;
}

void LD_H_E(u8 opcode) { // 0x63
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.hl.hi = globalState.de.lo;
}

void LD_H_H(u8 opcode) { // 0x64
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.hl.hi = globalState.hl.hi;
}

void LD_H_L(u8 opcode) { // 0x65
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.hl.hi = globalState.hl.lo;
}

void LD_H_DHL(u8 opcode) { // 0x66
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.hl.hi = readByte(globalState.hl.v);
}

// REGISTER-REGISTER LOADS (REGISTER L)
void LD_L_B(u8 opcode) { // 0x60
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.hl.lo = globalState.bc.hi;
}

void LD_L_C(u8 opcode) { // 0x61
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.hl.lo = globalState.bc.lo;
}

void LD_L_D(u8 opcode) { // 0x62
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.hl.lo = globalState.de.hi;
}

void LD_L_E(u8 opcode) { // 0x63
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.hl.lo = globalState.de.lo;
}

void LD_L_H(u8 opcode) { // 0x64
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.hl.lo = globalState.hl.hi;
}

void LD_L_L(u8 opcode) { // 0x65
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.hl.lo = globalState.hl.lo;
}

void LD_L_DHL(u8 opcode) { // 0x66
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.hl.lo = readByte(globalState.hl.v);
}

// REGISTER-REGISTER LOADS (REGISTER (HL))
void LD_DHL_B(u8 opcode) { // 0x70
  globalState.pc++;
  globalState.cycleCount += 8;
  writeByte(globalState.bc.hi, globalState.hl.v);
}

void LD_DHL_C(u8 opcode) { // 0x71
  globalState.pc++;
  globalState.cycleCount += 8;
  writeByte(globalState.bc.lo, globalState.hl.v);
}

void LD_DHL_D(u8 opcode) { // 0x72
  globalState.pc++;
  globalState.cycleCount += 8;
  writeByte(globalState.de.hi, globalState.hl.v);
}

void LD_DHL_E(u8 opcode) { // 0x73
  globalState.pc++;
  globalState.cycleCount += 8;
  writeByte(globalState.de.lo, globalState.hl.v);
}

void LD_DHL_H(u8 opcode) { // 0x74
  globalState.pc++;
  globalState.cycleCount += 8;
  writeByte(globalState.hl.hi, globalState.hl.v);
}

void LD_DHL_L(u8 opcode) { // 0x75
  globalState.pc++;
  globalState.cycleCount += 8;
  writeByte(globalState.hl.lo, globalState.hl.v);
}

// Random
void LD_DHL_n(u8 opcode) { // 0x36
  u8 imm = readByte(++globalState.pc);
  globalState.pc++;
  globalState.cycleCount += 12;
  writeByte(imm, globalState.hl.v);
}

// Load Into Register A Specials
void LD_A_DBC(u8 opcode) { // 0A
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.a = readByte(globalState.bc.v);
}

void LD_A_DDE(u8 opcode) { // 1A
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.a = readByte(globalState.de.v);
}


void LD_A_Dnn(u8 opcode) { // FA
  globalState.pc++;
  globalState.cycleCount += 16;
  globalState.a = readByte(readU16(globalState.pc));
  globalState.pc += 2;
}

void LD_A_n(u8 opocde) { // 0x3e
  u8 imm = readByte(++globalState.pc);
  globalState.pc++;
  globalState.a = imm;
  globalState.cycleCount += 8;
}

// LOAD FROM REGISTER A
void LD_B_A(u8 opcode) { // 0x47
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.bc.hi = globalState.a;
}

void LD_C_A(u8 opcode) { // 0x4f
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.bc.lo = globalState.a;
}

void LD_D_A(u8 opcode) { // 0x57
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.de.hi = globalState.a;
}

void LD_E_A(u8 opcode) { // 0x5f
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.de.lo = globalState.a;
}

void LD_H_A(u8 opcode) { // 0x67
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.hl.hi = globalState.a;
}

void LD_L_A(u8 opcode) { // 0x6f
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.hl.lo = globalState.a;
}

void LD_DBC_A(u8 opcode) { // 0x02
  globalState.pc++;
  globalState.cycleCount += 8;
  writeByte(globalState.a, globalState.bc.v);
}

void LD_DDE_A(u8 opcode) { // 0x12
  globalState.pc++;
  globalState.cycleCount += 8;
  writeByte(globalState.a, globalState.de.v);
}

void LD_DHL_A(u8 opcode) { // 0x77
  globalState.pc++;
  globalState.cycleCount += 8;
  writeByte(globalState.a, globalState.hl.v);
}

void LD_Dnn_A(u8 opcode) { // 0xea
  globalState.pc++;
  globalState.cycleCount += 16;
  writeByte(globalState.a, readU16(globalState.pc));
  globalState.pc += 2;
}

// Load A with memory offset from 0xff00
void LD_A_FF00_C(u8 opcode) { //0xf2
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.a = readByte((u16)0xff00 + globalState.bc.lo);
}

// Store A with memory offset from 0xff00
void LD_FF00_C_A(u8 opcode) { // 0xe2
  globalState.pc++;
  writeByte(globalState.a, (u16)0xff00 + globalState.bc.lo);
  globalState.cycleCount += 8;
}

// Load (HL) into A, decrement HL
void LDD_A_DHL(u8 opcode) { // 0x3a
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.a = readByte(globalState.hl.v);
  globalState.hl.v--;
}

// Load A into (HL), decrement HL
void LDD_DHL_A(u8 opcode) { // 0x32
  writeByte(globalState.a, globalState.hl.v);
  globalState.hl.v--;
  globalState.pc += 1;
  globalState.cycleCount += 8;
}

// Load (HL) into A, increment HL
void LDI_A_DHL(u8 opcode){ // 0x2a
  globalState.pc += 1;
  globalState.cycleCount += 8;
  globalState.a = readByte(globalState.hl.v);
  globalState.hl.v++;
}

// Load A into (HL), increment HL
void LDI_DHL_A(u8 opcode) { // 0x22
  writeByte(globalState.a, globalState.hl.v);
  globalState.hl.v++;
  globalState.pc += 1;
  globalState.cycleCount += 8;
}
// page 75

// Store A into FF00 + n
void LD_FF00_n_A(u8 opcode) { //0xe0
  u8 imm = readByte(++globalState.pc);
  globalState.pc++;
  writeByte(globalState.a, (u16)0xff00 + imm);
  globalState.cycleCount += 12;
}

// Load A with FF00 + n
void LD_A_FF00_n(u8 opcode) { // 0xf0
  u8 imm = readByte(++globalState.pc);
  globalState.pc++;
  globalState.a = readByte((u16)0xff00 + imm);
  globalState.cycleCount += 12;
}

// 16-bit loads
void LD_BC_nn(u8 opcode) { // 0x01
  u16 imm = readU16(globalState.pc + (u16)1);
  globalState.bc.v = imm;
  globalState.pc += 3;
  globalState.cycleCount += 12;
}

void LD_DE_nn(u8 opcode) { // 0x11
  u16 imm = readU16(globalState.pc + (u16)1);
  globalState.de.v = imm;
  globalState.pc += 3;
  globalState.cycleCount += 12;
}

void LD_HL_nn(u8 opcode) { // 0x21
  u16 imm = readU16(globalState.pc + (u16)1);
  globalState.hl.v = imm;
  globalState.pc += 3;
  globalState.cycleCount += 12;
}

void LD_SP_nn(u8 opcode) { // 0x31
  u16 imm = readU16(globalState.pc + (u16)1);
  globalState.sp = imm;
  globalState.pc += 3;
  globalState.cycleCount += 12;
}

void LD_SP_HL(u8 opcode) { // 0xf9
  globalState.sp = globalState.hl.v;
  globalState.pc++;
  globalState.cycleCount += 8;
}

// load effective address relative to stack pointer
void LD_HL_SP_n(u8 opcode) { //0xf8
  globalState.pc++;
  globalState.cycleCount += 12;
  s8 imm = readByte(globalState.pc);
  globalState.pc++;
  clearAllFlags();
  u16 result = globalState.sp + imm;
  if(((globalState.sp ^ imm ^ result) & 0x100) == 0x100) {
    setCarryFlag();
  }
  if(((globalState.sp ^ imm ^ result) & 0x10) == 0x10) {
    setHalfCarryFlag();
  }
  globalState.hl.v = result;
}

// store stack pointer
void LD_Dnn_SP(u8 opcode) { // 08
  globalState.pc++;
  u16 addr = readU16(globalState.pc);
  globalState.pc+=2;
  globalState.cycleCount += 20;
  writeU16(globalState.sp, addr);
}

// push U16 register onto stack
void PUSH_AF(u8 opcode) { // 0xf5
  globalState.pc++;
  globalState.cycleCount += 16;
  writeU16(globalState.f + (((u16)globalState.a) << 8), globalState.sp - (u16)2);
  globalState.sp -= 2;
}

void PUSH_BC(u8 opcode) { // 0xc5
  globalState.pc++;
  globalState.cycleCount += 16;
  writeU16(globalState.bc.v, globalState.sp - (u16)2);
  globalState.sp -= 2;
}

void PUSH_DE(u8 opcode) { // 0xd5
  globalState.pc++;
  globalState.cycleCount += 16;
  writeU16(globalState.de.v, globalState.sp - (u16)2);
  globalState.sp -= 2;
}

void PUSH_HL(u8 opcode) { // 0xe5
  globalState.pc++;
  globalState.cycleCount += 16;
  writeU16(globalState.hl.v, globalState.sp - (u16)2);
  globalState.sp -= 2;
}

// POP U16 register from stack
void POP_AF(u8 opcode) { // 0xf1
  globalState.pc++;
  globalState.cycleCount += 12;
  u16 v = readU16(globalState.sp);
  globalState.sp += 2;
  globalState.a = (u8)(v >> 8);
  globalState.f = (u8)(v & 0xf0);
}

void POP_BC(u8 opcode) { // 0xc1
  globalState.pc++;
  globalState.cycleCount += 12;
  u16 v = readU16(globalState.sp);
  globalState.sp += 2;
  globalState.bc.v = v;
}

void POP_DE(u8 opcode) { // 0xd1
  globalState.pc++;
  globalState.cycleCount += 12;
  u16 v = readU16(globalState.sp);
  globalState.sp += 2;
  globalState.de.v = v;
}

void POP_HL(u8 opcode) { // 0xe1
  globalState.pc++;
  globalState.cycleCount += 12;
  u16 v = readU16(globalState.sp);
  globalState.sp += 2;
  globalState.hl.v = v;
}

void addToA(u8 number) {
  int result = globalState.a + number;
  int carryBits = globalState.a ^ number ^ result;
  globalState.a = (u8)result;
  clearAllFlags();

  if((u8)result == 0){
    setZeroFlag();
  }

  if((carryBits & 0x100) != 0) {
    setCarryFlag();
  }
  if((carryBits & 0x10) != 0) {
    setHalfCarryFlag();
  }
}

// ADD Opcodes
void ADD_A(u8 opcode) { // 0x87
  globalState.pc++;
  globalState.cycleCount += 4;
  addToA(globalState.a);
}

void ADD_B(u8 opcode) { // 0x80
  globalState.pc++;
  globalState.cycleCount += 4;
  addToA(globalState.bc.hi);
}

void ADD_C(u8 opcode) { // 0x81
  globalState.pc++;
  globalState.cycleCount += 4;
  addToA(globalState.bc.lo);
}

void ADD_D(u8 opcode) { // 0x82
  globalState.pc++;
  globalState.cycleCount += 4;
  addToA(globalState.de.hi);
}

void ADD_E(u8 opcode) { // 0x83
  globalState.pc++;
  globalState.cycleCount += 4;
  addToA(globalState.de.lo);
}

void ADD_H(u8 opcode) { // 0x84
  globalState.pc++;
  globalState.cycleCount += 4;
  addToA(globalState.hl.hi);
}

void ADD_L(u8 opcode) { // 0x85
  globalState.pc++;
  globalState.cycleCount += 4;
  addToA(globalState.hl.lo);
}

void ADD_DHL(u8 opcode) { // 0x86
  globalState.pc++;
  globalState.cycleCount += 8;
  addToA(readByte(globalState.hl.v));
}

void ADD_n(u8 opcode) { // 0xC6
  u8 imm = readByte(++globalState.pc);
  globalState.pc++;
  globalState.cycleCount += 8;
  addToA(imm);
}

void adcToA(u8 number) {
  int carry = getCarryFlag() ? 1 : 0;
  int result = globalState.a + number + carry;
  clearAllFlags();
  if((u8)result == 0) {
    setZeroFlag();
  }
  if(result > 0xff) {
    setCarryFlag();
  }
  if(((globalState.a & 0xf) + (number & 0xf) + carry) > 0xf) {
    setHalfCarryFlag();
  }
  globalState.a = (u8) result;
}

// ADC Opcodes
void ADC_A(u8 opcode) { // 0x8f
  globalState.pc++;
  globalState.cycleCount += 4;
  adcToA(globalState.a);
}

void ADC_B(u8 opcode) { // 0x88
  globalState.pc++;
  globalState.cycleCount += 4;
  adcToA(globalState.bc.hi);
}

void ADC_C(u8 opcode) { // 0x89
  globalState.pc++;
  globalState.cycleCount += 4;
  adcToA(globalState.bc.lo);
}

void ADC_D(u8 opcode) { // 0x8a
  globalState.pc++;
  globalState.cycleCount += 4;
  adcToA(globalState.de.hi);
}

void ADC_E(u8 opcode) { // 0x8b
  globalState.pc++;
  globalState.cycleCount += 4;
  adcToA(globalState.de.lo);
}

void ADC_H(u8 opcode) { // 0x8c
  globalState.pc++;
  globalState.cycleCount += 4;
  adcToA(globalState.hl.hi);
}

void ADC_L(u8 opcode) { // 0x8d
  globalState.pc++;
  globalState.cycleCount += 4;
  adcToA(globalState.hl.lo);
}

void ADC_DHL(u8 opcode) { // 0x8e
  globalState.pc++;
  globalState.cycleCount += 8;
  adcToA(readByte(globalState.hl.v));
}

void ADC_n(u8 opcode) { // 0xCe
  u8 imm = readByte(++globalState.pc);
  globalState.pc++;
  globalState.cycleCount += 8;
  adcToA(imm);
}

void subToA(u8 number) {
  int result = globalState.a - number;
  int carryBits = globalState.a ^ number ^ result;
  globalState.a = (u8)result;
  clearAllFlags();
  setSubtractFlag();
  if((u8)result == 0) {
    setZeroFlag();
  }
  if((carryBits & 0x100) != 0) {
    setCarryFlag();
  }
  if((carryBits & 0x10) != 0) {
    setHalfCarryFlag();
  }
}

// SUB Opcodes
void SUB_A(u8 opcode) { // 0x97
  globalState.pc++;
  globalState.cycleCount += 4;
  subToA(globalState.a);
}

void SUB_B(u8 opcode) { // 0x90
  globalState.pc++;
  globalState.cycleCount += 4;
  subToA(globalState.bc.hi);
}

void SUB_C(u8 opcode) { // 0x91
  globalState.pc++;
  globalState.cycleCount += 4;
  subToA(globalState.bc.lo);
}

void SUB_D(u8 opcode) { // 0x92
  globalState.pc++;
  globalState.cycleCount += 4;
  subToA(globalState.de.hi);
}

void SUB_E(u8 opcode) { // 0x93
  globalState.pc++;
  globalState.cycleCount += 4;
  subToA(globalState.de.lo);
}

void SUB_H(u8 opcode) { // 0x94
  globalState.pc++;
  globalState.cycleCount += 4;
  subToA(globalState.hl.hi);
}

void SUB_L(u8 opcode) { // 0x95
  globalState.pc++;
  globalState.cycleCount += 4;
  subToA(globalState.hl.lo);
}

void SUB_DHL(u8 opcode) { // 0x96
  globalState.pc++;
  globalState.cycleCount += 8;
  subToA(readByte(globalState.hl.v));
}

void SUB_n(u8 opcode) { // 0xD6
  u8 imm = readByte(++globalState.pc);
  globalState.pc++;
  globalState.cycleCount += 8;
  subToA(imm);
}

void sbcToA(u8 number) {
  int carry = getCarryFlag() ? 1 : 0;
  int result = globalState.a - number - carry;
  clearAllFlags();
  setSubtractFlag();
  if((u8)result == 0) {
    setZeroFlag();
  }
  if(result < 0) {
    setCarryFlag();
  }
  if(((globalState.a & 0xf) - (number & 0xf) - carry) < 0) {
    setHalfCarryFlag();
  }
  globalState.a = (u8)result;
}

// SBC Opcodes
void SBC_A(u8 opcode) { // 0x9f
  globalState.pc++;
  globalState.cycleCount += 4;
  sbcToA(globalState.a);
}

void SBC_B(u8 opcode) { // 0x98
  globalState.pc++;
  globalState.cycleCount += 4;
  sbcToA(globalState.bc.hi);
}

void SBC_C(u8 opcode) { // 0x99
  globalState.pc++;
  globalState.cycleCount += 4;
  sbcToA(globalState.bc.lo);
}

void SBC_D(u8 opcode) { // 0x9a
  globalState.pc++;
  globalState.cycleCount += 4;
  sbcToA(globalState.de.hi);
}

void SBC_E(u8 opcode) { // 0x9b
  globalState.pc++;
  globalState.cycleCount += 4;
  sbcToA(globalState.de.lo);
}

void SBC_H(u8 opcode) { // 0x9c
  globalState.pc++;
  globalState.cycleCount += 4;
  sbcToA(globalState.hl.hi);
}

void SBC_L(u8 opcode) { // 0x9d
  globalState.pc++;
  globalState.cycleCount += 4;
  sbcToA(globalState.hl.lo);
}

void SBC_DHL(u8 opcode) { // 0x9e
  globalState.pc++;
  globalState.cycleCount += 8;
  sbcToA(readByte(globalState.hl.v));
}

void SBC_n(u8 opcode) { // 0xDe
  u8 imm = readByte(++globalState.pc);
  globalState.pc++;
  globalState.cycleCount += 8;
  sbcToA(imm);
}

void andtoA(u8 number) {
  u8 result = globalState.a & number;
  globalState.a = result;
  clearAllFlags();
  setHalfCarryFlag();
  if((u8)result == 0) {
    setZeroFlag();
  }
}

// AND Opcodes
void AND_A(u8 opcode) { // 0xa7
  globalState.pc++;
  globalState.cycleCount += 4;
  andtoA(globalState.a);
}

void AND_B(u8 opcode) { // 0xa0
  globalState.pc++;
  globalState.cycleCount += 4;
  andtoA(globalState.bc.hi);
}

void AND_C(u8 opcode) { // 0xa1
  globalState.pc++;
  globalState.cycleCount += 4;
  andtoA(globalState.bc.lo);
}

void AND_D(u8 opcode) { // 0xa2
  globalState.pc++;
  globalState.cycleCount += 4;
  andtoA(globalState.de.hi);
}

void AND_E(u8 opcode) { // 0xa3
  globalState.pc++;
  globalState.cycleCount += 4;
  andtoA(globalState.de.lo);
}

void AND_H(u8 opcode) { // 0xa4
  globalState.pc++;
  globalState.cycleCount += 4;
  andtoA(globalState.hl.hi);
}

void AND_L(u8 opcode) { // 0xa5
  globalState.pc++;
  globalState.cycleCount += 4;
  andtoA(globalState.hl.lo);
}

void AND_DHL(u8 opcode) { // 0xa6
  globalState.pc++;
  globalState.cycleCount += 8;
  andtoA(readByte(globalState.hl.v));
}

void AND_n(u8 opcode) { // 0xe6
  u8 imm = readByte(++globalState.pc);
  globalState.pc++;
  globalState.cycleCount += 8;
  andtoA(imm);
}

void xorToA(u8 number) {
  u8 result = globalState.a ^ number;
  globalState.a = result;
  clearAllFlags();
  if((u8)result == 0) {
    setZeroFlag();
  }
}

// XOR Opcodes
void XOR_A(u8 opcode) { // 0xaf
  globalState.pc++;
  globalState.cycleCount += 4;
  xorToA(globalState.a);
}

void XOR_B(u8 opcode) { // 0xa8
  globalState.pc++;
  globalState.cycleCount += 4;
  xorToA(globalState.bc.hi);
}

void XOR_C(u8 opcode) { // 0xa9
  globalState.pc++;
  globalState.cycleCount += 4;
  xorToA(globalState.bc.lo);
}

void XOR_D(u8 opcode) { // 0xaa
  globalState.pc++;
  globalState.cycleCount += 4;
  xorToA(globalState.de.hi);
}

void XOR_E(u8 opcode) { // 0xab
  globalState.pc++;
  globalState.cycleCount += 4;
  xorToA(globalState.de.lo);
}

void XOR_H(u8 opcode) { // 0xac
  globalState.pc++;
  globalState.cycleCount += 4;
  xorToA(globalState.hl.hi);
}

void XOR_L(u8 opcode) { // 0xad
  globalState.pc++;
  globalState.cycleCount += 4;
  xorToA(globalState.hl.lo);
}

void XOR_DHL(u8 opcode) { // 0xae
  globalState.pc++;
  globalState.cycleCount += 8;
  xorToA(readByte(globalState.hl.v));
}

void XOR_n(u8 opcode) { // 0xee
  u8 imm = readByte(++globalState.pc);
  globalState.pc++;
  globalState.cycleCount += 8;
  xorToA(imm);
}

void orToA(u8 number) {
  u8 result = globalState.a | number;
  globalState.a = result;
  clearAllFlags();
  if((u8)result == 0) {
    setZeroFlag();
  }
}

// OR Opcodes
void OR_A(u8 opcode) { // 0xb7
  globalState.pc++;
  globalState.cycleCount += 4;
  orToA(globalState.a);
}

void OR_B(u8 opcode) { // 0xb0
  globalState.pc++;
  globalState.cycleCount += 4;
  orToA(globalState.bc.hi);
}

void OR_C(u8 opcode) { // 0xb1
  globalState.pc++;
  globalState.cycleCount += 4;
  orToA(globalState.bc.lo);
}

void OR_D(u8 opcode) { // 0xb2
  globalState.pc++;
  globalState.cycleCount += 4;
  orToA(globalState.de.hi);
}

void OR_E(u8 opcode) { // 0xb3
  globalState.pc++;
  globalState.cycleCount += 4;
  orToA(globalState.de.lo);
}

void OR_H(u8 opcode) { // 0xb4
  globalState.pc++;
  globalState.cycleCount += 4;
  orToA(globalState.hl.hi);
}

void OR_L(u8 opcode) { // 0xb5
  globalState.pc++;
  globalState.cycleCount += 4;
  orToA(globalState.hl.lo);
}

void OR_DHL(u8 opcode) { // 0xb6
  globalState.pc++;
  globalState.cycleCount += 8;
  orToA(readByte(globalState.hl.v));
}

void OR_n(u8 opcode) { // 0xf6
  u8 imm = readByte(++globalState.pc);
  globalState.pc++;
  globalState.cycleCount += 8;
  orToA(imm);
}

void cpToA(u8 number) {
  clearAllFlags();
  setSubtractFlag();
  if(globalState.a < number) {
    setCarryFlag();
  }
  if(globalState.a == number) {
    setZeroFlag();
  }
  if(((globalState.a - number) & 0xf) > (globalState.a & 0xf)) {
    setHalfCarryFlag();
  }
}

// CP Opcodes
void CP_A(u8 opcode) { // 0xbf
  globalState.pc++;
  globalState.cycleCount += 4;
  cpToA(globalState.a);
}

void CP_B(u8 opcode) { // 0xb8
  globalState.pc++;
  globalState.cycleCount += 4;
  cpToA(globalState.bc.hi);
}

void CP_C(u8 opcode) { // 0xb9
  globalState.pc++;
  globalState.cycleCount += 4;
  cpToA(globalState.bc.lo);
}

void CP_D(u8 opcode) { // 0xba
  globalState.pc++;
  globalState.cycleCount += 4;
  cpToA(globalState.de.hi);
}

void CP_E(u8 opcode) { // 0xbb
  globalState.pc++;
  globalState.cycleCount += 4;
  cpToA(globalState.de.lo);
}

void CP_H(u8 opcode) { // 0xbc
  globalState.pc++;
  globalState.cycleCount += 4;
  cpToA(globalState.hl.hi);
}

void CP_L(u8 opcode) { // 0xbd
  globalState.pc++;
  globalState.cycleCount += 4;
  cpToA(globalState.hl.lo);
}

void CP_DHL(u8 opcode) { // 0xbe
  globalState.pc++;
  globalState.cycleCount += 8;
  cpToA(readByte(globalState.hl.v));
}

void CP_n(u8 opcode) { // 0xfe
  u8 imm = readByte(++globalState.pc);
  globalState.pc++;
  globalState.cycleCount += 8;
  cpToA(imm);
}

void DecodeCB(u8 opcode) {
  globalState.pc += 1;
  u8 newOpcode = readByte(globalState.pc);
  opcode_cbs[newOpcode](newOpcode);
}

u8 increment(u8 in) {
  u8 result = in + (u8)1;
  if(getCarryFlag()) {
    clearAllFlags();
    setCarryFlag();
  } else {
    clearAllFlags();
  }
  if(result == 0) {
    setZeroFlag();
  }
  if((result & 0x0f) == 0x00) {
    setHalfCarryFlag();
  }
  return result;
}

void INC_A(u8 opcode) { // 0x3c
  globalState.a = increment(globalState.a);
  globalState.pc++;
  globalState.cycleCount += 4;
}

void INC_B(u8 opcode) { // 0x04
  globalState.bc.hi = increment(globalState.bc.hi);
  globalState.pc++;
  globalState.cycleCount += 4;
}

void INC_C(u8 opcode) { // 0x0c
  globalState.bc.lo = increment(globalState.bc.lo);
  globalState.pc++;
  globalState.cycleCount += 4;
}

void INC_D(u8 opcode) { // 0x14
  globalState.de.hi = increment(globalState.de.hi);
  globalState.pc++;
  globalState.cycleCount += 4;
}

void INC_E(u8 opcode) { // 0x1c
  globalState.de.lo = increment(globalState.de.lo);
  globalState.pc++;
  globalState.cycleCount += 4;
}

void INC_H(u8 opcode) { // 0x24
  globalState.hl.hi = increment(globalState.hl.hi);
  globalState.pc++;
  globalState.cycleCount += 4;
}

void INC_L(u8 opcode) { // 0x2c
  globalState.hl.lo = increment(globalState.hl.lo);
  globalState.pc++;
  globalState.cycleCount += 4;
}

void INC_DHL(u8 opcode) { // 0x34
  // todo this one is hard.
  u8 v = readByte(globalState.hl.v);
  writeByte(increment(v), globalState.hl.v);
  globalState.pc++;
  globalState.cycleCount += 12;
}

u8 decrement(u8 in) {
  u8 result = in - (u8)1;
  if(getCarryFlag()) {
    clearAllFlags();
    setCarryFlag();
  } else {
    clearAllFlags();
  }
  setSubtractFlag();
  if(result == 0) {
    setZeroFlag();
  }
  if((result & 0xf) == 0xf) {
    setHalfCarryFlag();
  }
  return result;
}

void DEC_A(u8 opcode) { // 0x3d
  globalState.a = decrement(globalState.a);
  globalState.pc++;
  globalState.cycleCount += 4;
}

void DEC_B(u8 opcode) { // 0x05
  globalState.bc.hi = decrement(globalState.bc.hi);
  globalState.pc++;
  globalState.cycleCount += 4;
}

void DEC_C(u8 opcode) { // 0x0d
  globalState.bc.lo = decrement(globalState.bc.lo);
  globalState.pc++;
  globalState.cycleCount += 4;
}

void DEC_D(u8 opcode) { // 0x15
  globalState.de.hi = decrement(globalState.de.hi);
  globalState.pc++;
  globalState.cycleCount += 4;
}

void DEC_E(u8 opcode) { // 0x1d
  globalState.de.lo = decrement(globalState.de.lo);
  globalState.pc++;
  globalState.cycleCount += 4;
}

void DEC_H(u8 opcode) { // 0x25
  globalState.hl.hi = decrement(globalState.hl.hi);
  globalState.pc++;
  globalState.cycleCount += 4;
}

void DEC_L(u8 opcode) { // 0x2d
  globalState.hl.lo = decrement(globalState.hl.lo);
  globalState.pc++;
  globalState.cycleCount += 4;
}

void DEC_DHL(u8 opcode) { // 0x35
  // todo this one is hard.
  u8 v = readByte(globalState.hl.v);
  writeByte(decrement(v), globalState.hl.v);
  globalState.pc++;
  globalState.cycleCount += 12;
}

void addToHl(u16 number) {
  int result = globalState.hl.v + number;
  if(getZeroFlag()) {
    clearAllFlags();
    setZeroFlag();
  } else {
    clearAllFlags();
  }
  if(result & 0x10000) {
    setCarryFlag();
  }
  if((globalState.hl.v ^ number ^ (result & 0xffff)) & 0x1000) {
    setHalfCarryFlag();
  }
  globalState.hl.v = (u16)result;
}

void ADD_HL_BC(u8 opcode) { // 09
  globalState.pc++;
  globalState.cycleCount += 8;
  addToHl(globalState.bc.v);
}

void ADD_HL_DE(u8 opcode) { // 19
  globalState.pc++;
  globalState.cycleCount += 8;
  addToHl(globalState.de.v);
}

void ADD_HL_HL(u8 opcode) { // 29
  globalState.pc++;
  globalState.cycleCount += 8;
  addToHl(globalState.hl.v);
}

void ADD_HL_SP(u8 opcode) { // 39
  globalState.pc++;
  globalState.cycleCount += 8;
  addToHl(globalState.sp);
}

void ADD_SP_n(u8 opcode) { // E8
  s8 number = readByte(++globalState.pc);
  globalState.pc++;
  globalState.cycleCount += 16;
  int result = globalState.sp + number;
  clearAllFlags();
  if(((globalState.sp ^ number ^ (result & 0xffff)) & 0x100) == 0x100) {
    setCarryFlag();
  }
  if(((globalState.sp ^ number ^ (result & 0xffff)) & 0x10) == 0x10) {
    setHalfCarryFlag();
  }
  globalState.sp = (u16)result;
}

// 16 bit incs
void INC_BC(u8 opcode) { // 03
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.bc.v++;
}

void INC_DE(u8 opcode) { // 13
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.de.v++;
}

void INC_HL(u8 opcode) { // 23
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.hl.v++;
}

void INC_SP(u8 opcode) { // 33
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.sp++;
}

// 16 bit decs
void DEC_BC(u8 opcode) { // 0B
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.bc.v--;
}

void DEC_DE(u8 opcode) { // 1B
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.de.v--;
}

void DEC_HL(u8 opcode) { // 2B
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.hl.v--;
}

void DEC_SP(u8 opcode) { // 3B
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.sp--;
}

void DAA(u8 opcode) { // 0x27
  globalState.pc++;
  globalState.cycleCount += 4;
  u8 a = globalState.a;

  if(!getSubtractFlag()) {
    if(getCarryFlag() || a > 0x99) {
      a += 0x60;
      setCarryFlag();
    }
    if(getHalfCarryFlag() || (a & 0x0f) > 0x09) {
      a += 0x6;
    }
  } else {
    if(getCarryFlag()) {
      a -= 0x60;
    }
    if(getHalfCarryFlag()) {
      a -= 0x6;
    }
  }
  clearZeroFlag();
  clearHalfCarryFlag();
  if(a == 0) {
    setZeroFlag();
  }
  globalState.a = (u8)a;
}

void CPL(u8 opcode) { // 0x2f
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.a = ~globalState.a;
  setHalfCarryFlag();
  setSubtractFlag();
}

void CCF(u8 opcode) { // 0x3f
  globalState.pc++;
  globalState.cycleCount += 4;
  if(getCarryFlag()) {
    clearCarryFlag();
  } else {
    setCarryFlag();
  }
  clearHalfCarryFlag();
  clearSubtractFlag();
}

void SCF(u8 opcode) { // 0x37
  globalState.pc++;
  globalState.cycleCount += 4;
  clearHalfCarryFlag();
  clearSubtractFlag();
  setCarryFlag();
}

void NOP(u8 opcode) { // 00
  globalState.pc++;
  globalState.cycleCount += 4;
}


void HALT(u8 opcode) { // 76
  globalState.cycleCount += 4;
  globalState.pc++;
  //if(globalState.ime) {
    globalState.halt = true;
  //}

}

void STOP(u8 opcode) { // 10 (00)
  printf("stop not yet implemented\n");
  assert(false);
}

void DI(u8 opcode) { // F3
  // todo instruction after bs
  globalState.ime = 0;
  globalState.cycleCount += 4;
  globalState.pc++;
//  printf("di not yet implemented\n");
//  assert(false);
}

void EI(u8 opcode) { // FB
  globalState.ime = 1;
  globalState.cycleCount += 4;
  globalState.pc++;
}

void RLCA(u8 opcode) { // 07
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.a = rlcReg(globalState.a, true);
}

void RLA(u8 opcode) { // 17
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.a = rlReg(globalState.a, true);
}

void RRCA(u8 opcode) { // 0x0f
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.a = rrc(globalState.a, true);
}

void RRA(u8 opcode) { // 0x1f
  globalState.pc++;
  globalState.cycleCount += 4;
  globalState.a = rr(globalState.a, true);
}

void JP_nn(u8 opcodes) { // 0xc3
  globalState.pc++;
  u16 addr = readU16(globalState.pc);
  globalState.pc += 2;
  globalState.cycleCount += 12;
  globalState.pc = addr;
}

void JP_NZ_nn(u8 opcodes) { // 0xc2
  globalState.pc++;
  u16 addr = readU16(globalState.pc);
  globalState.pc += 2;
  if(!getZeroFlag()) {
    globalState.pc = addr;
  }
  globalState.cycleCount += 12;
}

void JP_Z_nn(u8 opcodes) { // 0xca
  globalState.pc++;
  u16 addr = readU16(globalState.pc);
  globalState.pc += 2;
  if(getZeroFlag()) {
    globalState.pc = addr;
  }
  globalState.cycleCount += 12;
}

void JP_NC_nn(u8 opcodes) { // 0xd2
  globalState.pc++;
  u16 addr = readU16(globalState.pc);
  globalState.pc += 2;
  if(!getCarryFlag()) {
    globalState.pc = addr;
  }
  globalState.cycleCount += 12;
}

void JP_C_nn(u8 opcodes) { // 0xda
  globalState.pc++;
  u16 addr = readU16(globalState.pc);
  globalState.pc += 2;
  if(getCarryFlag()) {
    globalState.pc = addr;
  }
  globalState.cycleCount += 12;
}

void JP_HL(u8 opcodes) { // 0xe9
  globalState.pc = globalState.hl.v;
  globalState.cycleCount += 4;
}

void JR_n(u8 opcode) { // 18
  s8 imm = readByte(++globalState.pc);
  globalState.pc++;
  globalState.pc += imm;
  globalState.cycleCount += 8;
}

void JR_NZ(u8 opcode) { // 20
  s8 imm = readByte(++globalState.pc);
  globalState.pc++;
  if(!getZeroFlag()){
    globalState.pc += imm;
  }
  globalState.cycleCount += 8;
}

void JR_Z(u8 opcode) { // 28
  s8 imm = readByte(++globalState.pc);
  globalState.pc++;
  if(getZeroFlag()){
    globalState.pc += imm;
  }
  globalState.cycleCount += 8;
}

void JR_NC(u8 opcode) {  // 30
  s8 imm = readByte(++globalState.pc);
  globalState.pc++;
  if(!getCarryFlag()){
    globalState.pc += imm;
  }
  globalState.cycleCount += 8;
}

void JR_C(u8 opcode) { // 38
  s8 imm = readByte(++globalState.pc);
  globalState.pc++;
  if(getCarryFlag()){
    globalState.pc += imm;
  }
  globalState.cycleCount += 8;
}


void CALL_nn(u8 opcode) { // 0xCD
  globalState.pc++;
  u16 addr = readU16(globalState.pc);
  //printf("call address 0x%x\n", addr);
  globalState.pc += 2;
  writeU16(globalState.pc, globalState.sp - (u16)2);
  globalState.sp -= 2;
  globalState.pc = addr;
  globalState.cycleCount += 12;
}

void CALL_NZ(u8 opcode) { // 0xC4
  globalState.pc++;
  u16 addr = readU16(globalState.pc);
  globalState.pc += 2;
  if(!getZeroFlag()) {
    writeU16(globalState.pc, globalState.sp - (u16)2);
    globalState.sp -= 2;
    globalState.pc = addr;
  }
  globalState.cycleCount += 12;
}

void CALL_Z(u8 opcode) { // 0xCc
  globalState.pc++;
  u16 addr = readU16(globalState.pc);
  globalState.pc += 2;
  if(getZeroFlag()) {
    writeU16(globalState.pc, globalState.sp - (u16)2);
    globalState.sp -= 2;
    globalState.pc = addr;
  }
  globalState.cycleCount += 12;
}


void CALL_NC(u8 opcode) { // 0d4
  globalState.pc++;
  u16 addr = readU16(globalState.pc);
  globalState.pc += 2;
  if(!getCarryFlag()) {
    writeU16(globalState.pc, globalState.sp - (u16)2);
    globalState.sp -= 2;
    globalState.pc = addr;
  }
  globalState.cycleCount += 12;
}

void CALL_C(u8 opcode) { // 0xdc
  globalState.pc++;
  u16 addr = readU16(globalState.pc);
  globalState.pc += 2;
  if(getCarryFlag()) {
    writeU16(globalState.pc, globalState.sp - (u16)2);
    globalState.sp -= 2;
    globalState.pc = addr;
  }
  globalState.cycleCount += 12;
}

void REST_00(u8 opcode) { // 0xc7
  globalState.pc++;
  writeU16(globalState.pc, globalState.sp - (u16)2);
  globalState.sp -= 2;
  globalState.pc = 0x0000;
  globalState.cycleCount += 32;
}

void REST_08(u8 opcode) { // 0xcf
  globalState.pc++;
  writeU16(globalState.pc, globalState.sp - (u16)2);
  globalState.sp -= 2;
  globalState.pc = 0x0008;
  globalState.cycleCount += 32;
}

void REST_10(u8 opcode) { // 0xd7
  globalState.pc++;
  writeU16(globalState.pc, globalState.sp - (u16)2);
  globalState.sp -= 2;
  globalState.pc = 0x0010;
  globalState.cycleCount += 32;
}

void REST_18(u8 opcode) { // 0xdf
  globalState.pc++;
  writeU16(globalState.pc, globalState.sp - (u16)2);
  globalState.sp -= 2;
  globalState.pc = 0x0018;
  globalState.cycleCount += 32;
}

void REST_20(u8 opcode) { // 0xe7
  globalState.pc++;
  writeU16(globalState.pc, globalState.sp - (u16)2);
  globalState.sp -= 2;
  globalState.pc = 0x0020;
  globalState.cycleCount += 32;
}

void REST_28(u8 opcode) { // 0xef
  globalState.pc++;
  writeU16(globalState.pc, globalState.sp - (u16)2);
  globalState.sp -= 2;
  globalState.pc = 0x0028;
  globalState.cycleCount += 32;
}

void REST_30(u8 opcode) { // 0xf7
  globalState.pc++;
  writeU16(globalState.pc, globalState.sp - (u16)2);
  globalState.sp -= 2;
  globalState.pc = 0x0030;
  globalState.cycleCount += 32;
}

void REST_38(u8 opcode) { // 0xff
  globalState.pc++;
  writeU16(globalState.pc, globalState.sp - (u16)2);
  globalState.sp -= 2;
  globalState.pc = 0x0038;
  globalState.cycleCount += 32;
}

void RET(u8 opcode) { // 0xc9
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.pc = readU16(globalState.sp);
  globalState.sp += 2;
}

void RET_NZ(u8 opcode) { // 0xc0
  globalState.pc++;
  globalState.cycleCount += 8;
  if(!getZeroFlag()) {
    globalState.pc = readU16(globalState.sp);
    globalState.sp += 2;
  }
}

void RET_Z(u8 opcode) { // 0xc8
  globalState.pc++;
  globalState.cycleCount += 8;
  if(getZeroFlag()) {
    globalState.pc = readU16(globalState.sp);
    globalState.sp += 2;
  }
}

void RET_NC(u8 opcode) { // 0xd0
  globalState.pc++;
  globalState.cycleCount += 8;
  if(!getCarryFlag()) {
    globalState.pc = readU16(globalState.sp);
    globalState.sp += 2;
  }
}

void RET_C(u8 opcode) { // 0xd8
  globalState.pc++;
  globalState.cycleCount += 8;
  if(getCarryFlag()) {
    globalState.pc = readU16(globalState.sp);
    globalState.sp += 2;
  }
}

void RETI(u8 opcode) { // 0xd9
  globalState.pc++;
  globalState.cycleCount += 8;
  globalState.pc = readU16(globalState.sp);
  globalState.sp += 2;
  globalState.ime = 1;
}

// normal opcode handler table
static OpcodeHandler* opcodes[256] =
        {NOP,          LD_BC_nn,     LD_DBC_A,     INC_BC,       INC_B,        DEC_B,        LD_B_n,       RLCA,         // 0x0 - 0x7
         LD_Dnn_SP,    ADD_HL_BC,    LD_A_DBC,     DEC_BC,       INC_C,        DEC_C,        LD_C_n,       RRCA,         // 0x8 - 0xf
         STOP,         LD_DE_nn,     LD_DDE_A,     INC_DE,       INC_D,        DEC_D,        LD_D_n,       RLA,          // 0x10 - 0x17
         JR_n,         ADD_HL_DE,    LD_A_DDE,     DEC_DE,       INC_E,        DEC_E,        LD_E_n,       RRA,          // 0x18 - 0x1f
         JR_NZ,        LD_HL_nn,     LDI_DHL_A,    INC_HL,       INC_H,        DEC_H,        LD_H_n,       DAA,          // 0x20 - 0x27
         JR_Z,         ADD_HL_HL,    LDI_A_DHL,    DEC_HL,       INC_L,        DEC_L,        LD_L_n,       CPL,          // 0x28 - 0x2f
         JR_NC,        LD_SP_nn,     LDD_DHL_A,    INC_SP,       INC_DHL,      DEC_DHL,      LD_DHL_n,     SCF,          // 0x30 - 0x37
         JR_C,         ADD_HL_SP,    LDD_A_DHL,    DEC_SP,       INC_A,        DEC_A,        LD_A_n,       CCF,          // 0x38 - 0x3f
         LD_B_B,       LD_B_C,       LD_B_D,       LD_B_E,       LD_B_H,       LD_B_L,       LD_B_DHL,     LD_B_A,       // 0x40 - 0x47
         LD_C_B,       LD_C_C,       LD_C_D,       LD_C_E,       LD_C_H,       LD_C_L,       LD_C_DHL,     LD_C_A,       // 0x48 - 0x4f
         LD_D_B,       LD_D_C,       LD_D_D,       LD_D_E,       LD_D_H,       LD_D_L,       LD_D_DHL,     LD_D_A,       // 0x50 - 0x57
         LD_E_B,       LD_E_C,       LD_E_D,       LD_E_E,       LD_E_H,       LD_E_L,       LD_E_DHL,     LD_E_A,       // 0x58 - 0x5f
         LD_H_B,       LD_H_C,       LD_H_D,       LD_H_E,       LD_H_H,       LD_H_L,       LD_H_DHL,     LD_H_A,       // 0x60 - 0x67
         LD_L_B,       LD_L_C,       LD_L_D,       LD_L_E,       LD_L_H,       LD_L_L,       LD_L_DHL,     LD_L_A,       // 0x68 - 0x6f
         LD_DHL_B,     LD_DHL_C,     LD_DHL_D,     LD_DHL_E,     LD_DHL_H,     LD_DHL_L,     HALT,         LD_DHL_A,     // 0x70 - 0x77
         LD_A_B,       LD_A_C,       LD_A_D,       LD_A_E,       LD_A_H,       LD_A_L,       LD_A_DHL,     LD_A_A      , // 0x78 - 0x7f
         ADD_B,        ADD_C,        ADD_D,        ADD_E,        ADD_H,        ADD_L,        ADD_DHL,      ADD_A,        // 0x80 - 0x87
         ADC_B,        ADC_C,        ADC_D,        ADC_E,        ADC_H,        ADC_L,        ADC_DHL,      ADC_A,        // 0x88 - 0x8f
         SUB_B,        SUB_C,        SUB_D,        SUB_E,        SUB_H,        SUB_L,        SUB_DHL,      SUB_A,        // 0x90 - 0x97
         SBC_B,        SBC_C,        SBC_D,        SBC_E,        SBC_H,        SBC_L,        SBC_DHL,      SBC_A,        // 0x98 - 0x9f
         AND_B,        AND_C,        AND_D,        AND_E,        AND_H,        AND_L,        AND_DHL,      AND_A,        // 0xa0 - 0xa7
         XOR_B,        XOR_C,        XOR_D,        XOR_E,        XOR_H,        XOR_L,        XOR_DHL,      XOR_A,        // 0xa8 - 0xaf
         OR_B,         OR_C,         OR_D,         OR_E,         OR_H,         OR_L,         OR_DHL,       OR_A,         // 0xb0 - 0xb7
         CP_B,         CP_C,         CP_D,         CP_E,         CP_H,         CP_L,         CP_DHL,       CP_A,         // 0xb8 - 0xbf
         RET_NZ,       POP_BC,       JP_NZ_nn,     JP_nn,        CALL_NZ,      PUSH_BC,      ADD_n,        REST_00,      // 0xc0 - 0xc7
         RET_Z,        RET,          JP_Z_nn,      DecodeCB,     CALL_Z,       CALL_nn,      ADC_n,        REST_08,      // 0xc8 - 0xcf
         RET_NC,       POP_DE,       JP_NC_nn,     invHandler,   CALL_NC,      PUSH_DE,      SUB_n,        REST_10,      // 0xd0 - 0xd7
         RET_C,        RETI,         JP_C_nn,      invHandler,   CALL_C,       invHandler,   SBC_n,        REST_18,      // 0xd8 - 0xdf
         LD_FF00_n_A,  POP_HL,       LD_FF00_C_A,  invHandler,   invHandler,   PUSH_HL,      AND_n,        REST_20,      // 0xe0 - 0xe7
         ADD_SP_n,     JP_HL,        LD_Dnn_A,     invHandler,   invHandler,   invHandler,   XOR_n,        REST_28,      // 0xe8 - 0xef
         LD_A_FF00_n,  POP_AF,       LD_A_FF00_C,  DI,           invHandler,   PUSH_AF,      OR_n,         REST_30,      // 0xf0 - 0xf7
         LD_HL_SP_n,   LD_SP_HL,     LD_A_Dnn,     EI,           invHandler,   invHandler,   CP_n,         REST_38};     // 0xf8 - 0xff


// reset the globalState structure, including registers and timers
void resetCpu() {
  globalState.f = 0x1; // or 0x11? (
  globalState.a = 0xb0; // maybe f, a swap??
  globalState.bc.v = 0x0013;
  globalState.de.v = 0x00d8;
  globalState.hl.v = 0x014d;
  globalState.sp = 0xfffe;
  globalState.pc = 0x0;
  globalState.halt = false;
  globalState.cycleCount = 0;
  globalState.divOffset = 0;
  globalState.timSubcount = 0;
}

// set the globalState so the next call to step() will run an ISR
void interrupt(u16 addr) {
  globalState.ime = 0;                                // disable interrupts
  writeU16(globalState.pc, globalState.sp - (u16)2);  // push pc to stack
  globalState.sp -= 2;                                // push pc to stack
  globalState.cycleCount += 12;                       // timing
  globalState.pc = addr;                              // jump to ISR
}

// timer speeds: once this number of clock cycles has elapsed, the timer ticks.
static u32 timReset[4] = {(1 << 10), (1 << 4), (1 << 6), (1 << 8)};

// step the CPU 1 instruction
// returns number of clock cycles
u32 cpuStep() {
  uint64_t oldCycleCount = globalState.cycleCount;

  // update div register
  globalMemState.ioRegs[IO_DIV] = (u8)((globalState.cycleCount - globalState.divOffset) >> 8);

  // execute, if we aren't halted.
  if(!globalState.halt) {
    // fetch opcode
    u8 opcode = readByte(globalState.pc);
    // execute opcode
    opcodes[opcode](opcode);
  }

    assert(false);
  // interrupts
  if(globalState.ime && globalMemState.upperRam[0x7f] && globalMemState.ioRegs[IO_IF]) {

    // mask interrupts with the interrupt enable register at 0xffff.
    u8 interrupts = globalMemState.upperRam[0x7f] & globalMemState.ioRegs[IO_IF];

    if(interrupts & 0x01) {
      globalMemState.ioRegs[IO_IF] &= ~1;
      interrupt(VBLANK_INTERRUPT);
      globalState.halt = false;
    } else if(interrupts & 0x02) {
      globalMemState.ioRegs[IO_IF] &= ~2;
      interrupt(LCDC_INTERRUPT);
      globalState.halt = false;
    } else if(interrupts & 0x04) {
      globalMemState.ioRegs[IO_IF] &= ~4;
      interrupt(TIMER_INTERRUPT);
      globalState.halt = false;
    } else if(interrupts & 0x08) {
      globalMemState.ioRegs[IO_IF] &= ~8;
      interrupt(SERIAL_INTERRUPT);
      globalState.halt = false;
    } else if(interrupts & 0x10) {
      globalMemState.ioRegs[IO_IF] &= ~0x10;
      interrupt(HIGH_TO_LOW_P10_P13);
      globalState.halt = false;
    }
  }

  // even if we have IME off, and we're halted, we're supposed to check IF and IE register
  // this won't fire an interrupt, but will get us out of halt
  // this behavior isn't well documented, but was required to pass cpu_instr.gb
  // (though none of the games seem to need it...)
  if(globalState.halt) {
    globalState.cycleCount += 200; // just to keep ticking the timer...
    u8 interrupts = globalMemState.upperRam[0x7f] & globalMemState.ioRegs[IO_IF];
    if(interrupts & 0x01) {
      globalState.halt = false;
    } else if(interrupts & 0x02) {
      globalState.halt = false;
    } else if(interrupts & 0x04) {
      globalState.halt = false;
    } else if(interrupts & 0x08) {
      globalState.halt = false;
    } else if(interrupts & 0x10) {
      globalState.halt = false;
    }
  }


  // cycle count
  uint64_t cyclesThisIteration = globalState.cycleCount - oldCycleCount;

  // update timer
  u8 tac = globalMemState.ioRegs[IO_TAC];
  bool ten = ((tac >> 2) & 1) != 0; // timer enable?
  if(ten) {
    u8 tclk = (tac & (u8)3);     // timer speed
    globalState.timSubcount += cyclesThisIteration;
    if(globalState.timSubcount >= timReset[tclk]) { // timer tick
      globalState.timSubcount = 0;
      u8 timv = globalMemState.ioRegs[IO_TIMA]; // check for overflow
      if(timv == 255) {
        globalMemState.ioRegs[IO_IF] |= 4; // set interrupt
        globalMemState.ioRegs[IO_TIMA] = globalMemState.ioRegs[IO_TMA]; // reset
      } else {
        globalMemState.ioRegs[IO_TIMA] = timv + (u8)1; // increment.
      }
    }
  }

  return (u32)cyclesThisIteration;
}

bool getZeroFlag() {
  return (globalState.f & 0x80) != 0;
}

bool getSubtractFlag() {
  return (globalState.f & 0x40) != 0;
}

bool getHalfCarryFlag() {
  return (globalState.f & 0x20) != 0;
}

bool getCarryFlag() {
  return (globalState.f & 0x10) != 0;
}

void setZeroFlag() {
  globalState.f = globalState.f | (u8)0x80;
}

void setSubtractFlag() {
  globalState.f = globalState.f | (u8)0x40;
}

void setHalfCarryFlag() {
  globalState.f = globalState.f | (u8)0x20;
}

void setCarryFlag() {
  globalState.f = globalState.f | (u8)0x10;
}

void clearZeroFlag(){
  globalState.f = globalState.f & ~((u8)0x80);
}

void clearSubtractFlag(){
  globalState.f = globalState.f & ~((u8)0x40);
}

void clearHalfCarryFlag(){
  globalState.f = globalState.f & ~((u8)0x20);
}

void clearCarryFlag(){
  globalState.f = globalState.f & ~((u8)0x10);
}

void clearAllFlags() {
  clearZeroFlag();
  clearSubtractFlag();
  clearHalfCarryFlag();
  clearCarryFlag();
}
