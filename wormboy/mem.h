#ifndef GBC_MEM_H
#define GBC_MEM_H

#include "types.h"
#include "stdio.h"
#include "platform.h"

// interrupt flags
#define INTERRUPT_KEY 0x10;

// interrupt vector locations
#define VBLANK_INTERRUPT    0x0040
#define LCDC_INTERRUPT      0x0048
#define TIMER_INTERRUPT     0x0050
#define SERIAL_INTERRUPT    0x0058
#define HIGH_TO_LOW_P10_P13 0x0060

// IO Regs (offset from 0xff00)
#define IO_P1        0x0  // joystick               (needs keyboard handler)
#define IO_SERIAL_SB 0x01 // serial transfer data   (don't support)
#define IO_SERIAL_SC 0x02 // serial control         (don't support)
#define IO_DIV       0x04 // div
#define IO_TIMA      0x05 // timer value            (nyi)
#define IO_TMA       0x06 // timer reload           (nyi)
#define IO_TAC       0x07 // TIMER CONTROL (enable, speed) (nyi)
#define IO_IF        0x0f

#define IO_NR10      0x10
#define IO_NR11      0x11
#define IO_NR12      0x12
#define IO_NR13      0x13
#define IO_NR14      0x14

#define IO_NR21      0x16
#define IO_NR22      0x17
#define IO_NR23      0x18
#define IO_NR24      0x19

#define IO_NR30      0x1a
#define IO_NR31      0x1b
#define IO_NR32      0x1c
#define IO_NR33      0x1d
#define IO_NR34      0x1e

#define IO_NR41      0x20
#define IO_NR42      0x21
#define IO_NR43      0x22
#define IO_NR44      0x23

#define IO_NR50      0x24
#define IO_NR51      0x25
#define IO_NR52      0x26

#define IO_WAVE_PATTERN 0x30 // this is 16 bytes

#define IO_LCDC      0x40 // yes, revisit
#define IO_STAT      0x41 // no
#define IO_SCROLLY   0x42
#define IO_SCROLLX   0x43
#define IO_LY        0x44
#define IO_LYC       0x45 // no

#define IO_DMA       0x46 // yes
#define IO_BGP       0x47 // ??
#define IO_OBP0      0x48 // ??
#define IO_OBP1      0x49 // ??
#define IO_WINY      0x4a // no
#define IO_WINX      0x4b // no

#define IO_GBCSPEED  0x4d

#define IO_EXIT_BIOS 0x50


#define CART_INFO_ADDR      0x0100


// cartridge types
enum CartType {
  ROM_ONLY = 0,
  ROM_MBC1 = 1,
  ROM_MBC1_RAM = 2,
  ROM_MBC1_RAM_BATT = 3,
  ROM_MBC2 = 5,
  ROM_MBC2_BATT = 6,
  ROM_RAM = 8,
  ROM_RAM_BATT = 9,
  ROM_MM01 = 0xb,
  ROM_MM01_SRAM = 0xc,
  ROM_MMM01_SRAM_BATT = 0xd,
  ROM_MBC3_TIMER_BATT = 0xf,
  ROM_MBC3_TIMER_RAM_BATT = 0x10, // pkmn crystal
  ROM_MBC3 = 0x11,
  ROM_MBC3_RAM = 0x12,
  ROM_MBC3_RAM_BATT = 0x13, // pkmn red
  ROM_MBC5 = 0x19,
  ROM_MBC_RAM = 0x1a,
  ROM_MBC5_RAM_BATT = 0x1b,
  ROM_MBC5_RUMBLE = 0x1c,
  ROM_MBC5_RUMBLE_SRAM = 0x1d,
  ROM_MBC5_RUMBLE_SRAM_BATT = 0x1e,
  POCKET_CAMERA = 0x1f,
  BANDAI_TAMA5 = 0xfd,
  HUDSON_HUC3 = 0xfe,
  HUDSON_HUC1 = 0xff
};

// in banks, of 16 KByte each
enum RomSize {
  BANK_2 = 0,
  BANK_4 = 1,
  BANK_8 = 2,
  BANK_16 = 3,
  BANK_32 = 4,
  BANK_64 = 5, // pkmn red: 1 MB
  BANK_128 = 6, // pkmn crystal
  BANK_72 = 0x52,
  BANK_80 = 0x53,
  BANK_96 = 0x54
};

enum RamSize {
  NONE = 0,
  SIZE_2KB = 1, // 1 bank
  SIZE_8KB = 2, // 1 bank
  SIZE_32KB = 3, // 4 banks, pkmn red, crystal
  SIZE_128KB = 4, // 16 banks
};

struct CartInfo {
  u8 beginExec[4];        // 0x0 -> 0x3
  u8 nintendoGraphic[48]; // 0x3 ->
  char title[15];
  u8 isColor; // compare against CARTRIDGE_IS_COLOR
  u8 license1;
  u8 license2;
  u8 SGB;
  CartType cartType;
  RomSize romSize;
  RamSize ramSize;
  u8 notJapan;
  u8 license3;
  u8 maskRomVersion;

  void print();
};

struct MemState {
  bool inBios;
  u8 mbcType;
  u8 romBank;
  u8 nRomBanks;
  u8 nRamBanks;
  u8* rom0;
  u8* mappedRom;
  u8* vram;
  u8* mappedRam;
  u8* disabledMappedRam;
  u8* internalRam;
  u8* mappedRamAllocation;
  u8* ioRegs;
  u8* upperRam;
  u8* spriteAttribute;
};

// external interface
extern MemState globalMemState;
void initMem(FileLoadData file); // intialize memory for a cartridge
u8 readByte(u16 addr);           // read a byte from memory
u16 readU16(u16 addr);           // read 16-bits from memory
void writeByte(u8 byte, u16 addr); // write a byte to memory
void writeU16(u16 mem, u16 addr); // read 16-bits from memory
void saveGame();
void loadGame();


#endif //GBC_MEM_H
