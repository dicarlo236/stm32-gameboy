/*
 * cpu.h
 *
 *  Created on: Jan 13, 2019
 *      Author: jared
 */

#ifndef WORMBOY_CPU_H_
#define WORMBOY_CPU_H_

#include "types.h"

extern u32 frameNum;

// 16 bit register
union reg {
  u16 v;
  struct {
    u8 lo;
    u8 hi;
  };
};

struct CpuState {
  // registers
  reg bc, de, hl;
  u8 f;
  u8 a;
  u16 sp, pc;

  bool halt;
  uint64_t cycleCount;
  uint64_t divOffset;
  u8 ime;
  u32 timSubcount;
};

// external interface:
extern CpuState globalState;
void resetCpu(); // reinitialize the cpu
u32 cpuStep();   // step 1 instruction, returns number of clock cycles elapsed


bool getZeroFlag();
bool getSubtractFlag();
bool getHalfCarryFlag();
bool getCarryFlag();
void setZeroFlag();
void clearZeroFlag();
void setSubtractFlag();
void clearSubtractFlag();
void setHalfCarryFlag();
void clearHalfCarryFlag();
void setCarryFlag();
void clearCarryFlag();
void clearAllFlags();

#endif /* WORMBOY_CPU_H_ */
