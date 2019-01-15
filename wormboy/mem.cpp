#define NDEBUG
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "mem.h"
#include "platform.h"
#include "rom.h"
#include "sound.h"

#define DANGER_MODE

MemState globalMemState;

void CartInfo::print() {
  printf("Gameboy Cartridge\n"
         "\ttitle: %s\n"
         "\tisColor: 0x%x\n"
         "\tSGB: 0x%x\n"
         "\tcartType: 0x%x\n"
         "\tromSize: 0x%x\n"
         "\tramSize: 0x%x\n"
         "\tnotJapan: 0x%x\n",
         title, isColor, SGB, (u8)cartType, (u8)romSize, (u8)ramSize, notJapan);
}

// number of banks for given cartridge types
static const u8 romSizeIDToNBanks[7] = {2,4,8,16,32,64,128};
static const u8 ramSizeIDToNBanks[5] = {0,1,1,4,4};
static u8* fileData = nullptr;

void saveGame() {
  FileLoadData fld;
  fld.size = 0x2000 * globalMemState.nRamBanks;
  fld.data = globalMemState.mappedRamAllocation;
  saveFile("savegame.gam", fld);
}

void loadGame() {
  FileLoadData fld = loadFile("savegame.gam");
  memcpy(globalMemState.mappedRamAllocation, fld.data, 0x2000 * globalMemState.nRamBanks);
}




void initMem(FileLoadData file) {
  CartInfo* cartInfo = (CartInfo*)(file.data + CART_INFO_ADDR);
  cartInfo->print();
  fileData = file.data;
  printf("ROM size: 0x%x bytes\n", file.size);


  globalMemState.inBios = true;       // start in BIOS mode
  globalMemState.rom0 = file.data;    // ROM-bank 0 is the bottom of the cartridge

  // initialize everything to zero
  //globalMemState.mappedRom = nullptr;
  globalMemState.nRamBanks = 0;
  globalMemState.nRomBanks = 0;
  globalMemState.vram = nullptr;
  globalMemState.mappedRam = nullptr;
  globalMemState.disabledMappedRam = nullptr;
  globalMemState.mappedRamAllocation = nullptr;
  globalMemState.internalRam = nullptr;
  globalMemState.upperRam = nullptr;

  switch(cartInfo->cartType) {
    case ROM_ONLY:
      globalMemState.romBank = 1;
      //globalMemState.mappedRom = file.data + 0x4000; // maps in upper ROM by default
      break;

    case ROM_MBC1:
      globalMemState.romBank = 1;
      //globalMemState.mappedRom = file.data + 0x4000; // maps in upper ROM by default
      globalMemState.mbcType = 1;
      if((u8)cartInfo->romSize < 7) {
        globalMemState.nRomBanks = romSizeIDToNBanks[(u8)cartInfo->romSize]; // has ROM banks
      } else {
        printf("unknown number of rom banks\n");
        assert(false);
      }
      break;

    case ROM_MBC1_RAM:
      globalMemState.romBank = 1;
      //globalMemState.mappedRom = file.data + 0x4000; // maps in upper ROM by default
      globalMemState.mbcType = 1;
      if((u8)cartInfo->romSize < 7) {
        globalMemState.nRomBanks = romSizeIDToNBanks[(u8)cartInfo->romSize]; // has ROM banks
      } else {
        printf("unknown number of rom banks\n");
        assert(false);
      }

      if((u8)cartInfo->ramSize < 5) {
        globalMemState.nRamBanks = ramSizeIDToNBanks[(u8)cartInfo->ramSize]; // has RAM banks
      } else {
        printf("unknown number of ram banks\n");
        assert(false);
      }
      break;

    case ROM_MBC3_RAM_BATT:
    case ROM_MBC3_TIMER_RAM_BATT:
      globalMemState.romBank = 1;
      //globalMemState.mappedRom = file.data + 0x4000; // maps in upper ROM by default
      globalMemState.mbcType = 3;
      if((u8)cartInfo->romSize < 7) {
        globalMemState.nRomBanks = romSizeIDToNBanks[(u8)cartInfo->romSize]; // has ROM banks
      } else {
        printf("unknown number of rom banks\n");
        assert(false);
      }

      if((u8)cartInfo->ramSize < 5) {
        globalMemState.nRamBanks = ramSizeIDToNBanks[(u8)cartInfo->ramSize]; // has RAM banks
      } else {
        printf("unknown number of ram banks\n");
      }
      break;
    default:
      printf("unknown cart type 0x%x\n", (u8)cartInfo->cartType);
      assert(false);
      break;
  }

  printf("mbc%d\n", globalMemState.mbcType);
  printf("rom-banks: %d\n", globalMemState.nRomBanks);
  printf("ram-banks: %d\n", globalMemState.nRamBanks);

  // allocate cartridge RAM (8 KB * # of banks)
  if(globalMemState.nRamBanks) {
    globalMemState.mappedRamAllocation = (u8*)badalloc_check(0x2000 * globalMemState.nRamBanks, "mapped-ram");
    memset((void*)globalMemState.mappedRamAllocation, 0, 0x2000 * globalMemState.nRamBanks);
    globalMemState.disabledMappedRam = globalMemState.mappedRamAllocation;
  } else {

  }

  // allocate memories:
  // internal RAM
  globalMemState.internalRam = (u8*)badalloc_check(0x2000, "internal-ram");
  globalMemState.vram =        (u8*)badalloc_check(0x2000, "vram");
  globalMemState.upperRam    = (u8*)badalloc_check(0x80, "upperRam");
  globalMemState.ioRegs      = (u8*)badalloc_check(0x80, "ioRegs");

  // clear RAMs
  memset(globalMemState.internalRam, 0, 0x2000);
  memset(globalMemState.vram, 0, 0x2000);
  memset(globalMemState.upperRam, 0, 0x80);
  memset(globalMemState.ioRegs, 0, 0x80);

  // setup i/o regs and friends
  u8* io = globalMemState.ioRegs;
  io[IO_TIMA] = 0; // reset TIMER COUNT to 0
  io[IO_TMA] = 0;  // TIMER RELOAD
  io[IO_TAC] = 0;  // TIMER STOP
  io[IO_NR10] = 0x80;
  io[IO_NR11] = 0xbf;
  io[IO_NR12] = 0xf3;
  io[IO_NR14] = 0xbf;
  io[IO_NR21] = 0x3f;
  io[IO_NR22] = 0x00;
  io[IO_NR24] = 0xbf;
  io[IO_NR30] = 0x7f;
  io[IO_NR31] = 0xff;
  io[IO_NR32] = 0x9f;
  io[IO_NR34] = 0xbf;
  io[IO_NR41] = 0xff;
  io[IO_NR42] = 0x00;
  io[IO_NR43] = 0x00;
  io[IO_NR44] = 0xbf;
  io[IO_NR50] = 0x77;
  io[IO_NR51] = 0xf3;
  io[IO_NR52] = 0xf1;
  io[IO_LCDC] = 0x91;
  io[IO_SCROLLY] = 0x00;
  io[IO_SCROLLX] = 0x00;
  io[IO_LYC] = 0x00;
  io[IO_BGP] = 0xfc;
  io[IO_OBP0] = 0xff;
  io[IO_OBP1] = 0xff;
  io[IO_WINY] = 0x00;
  io[IO_WINX] = 0x00;

  // turn off interrupts
  globalMemState.upperRam[0x7f] = 0;
}



// boot ROM
static const u8 bios[256] = {   0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26, 0xFF, 0x0E,
	    0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3, 0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0,
	    0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B,
	    0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22, 0x23, 0x05, 0x20, 0xF9,
	    0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20,
	    0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04,
	    0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2,
	    0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62, 0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06,
	    0x7B, 0xE2, 0x0C, 0x3E, 0x87, 0xE2, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20,
	    0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17,
	    0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9, 0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
	    0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
	    0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
	    0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E, 0x3C, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C,
	    0x21, 0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x00, 0x00, 0x23, 0x7D, 0xFE, 0x34, 0x20,
	    0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x00, 0x00, 0x3E, 0x01, 0xE0, 0x50};
// handler for MBC0 switch
void mbc0Handler(u16 addr, u8 value) {

  // it looks like tetris tries to select ROM 1 for banked ROM, so we need to allow this:
  if(addr >= 0x2000 && addr < 0x3fff) {
    if(value == 0 || value == 1) {
      // nothing to do!
    } else {
      assert(false);
    }
  } else {
    assert(false);
  }

}

// handler for MBC1 switch (doesn't handle everything yet...)
void mbc1Handler(u16 addr, u8 value) {
  if(addr >= 0x2000 && addr < 0x3fff) {
    // ROM bank switch
    if(value >= globalMemState.nRomBanks) {
      printf("\trequested rom bank %d when there are only %d banks!\n", value, globalMemState.nRomBanks);
      assert(false);
    }

    if(value == 0) value = 1;
    if(value == 0x21) value = 0x20;
    if(value == 0x41) value = 0x40;
    globalMemState.romBank = value;
    //globalMemState.mappedRom = fileData + 0x4000 * value;
  } else if(addr >= 0 && addr < 0x1fff) {
    // enable RAM
    if(value == 0) {
      globalMemState.disabledMappedRam = globalMemState.mappedRam;
      globalMemState.mappedRam = nullptr;
    } else if(value == 0xa) {
      globalMemState.mappedRam = globalMemState.disabledMappedRam;
    } else {
      assert(false);
    }
  } else {
    assert(false);
  }

}

// handler for MBC2 switch (doesn't handle anything yet...)
void mbc2Handler(u16 addr, u8 value) {
  assert(false);
}

// handler for MBC3 switch (doesn't handle anything yet...)
void mbc3Handler(u16 addr, u8 value) {

  if(addr >= 0x2000 && addr < 0x3fff) {
    // ROM bank switch
    if(value >= globalMemState.nRomBanks) {
      printf("\trequested rom bank %d when there are only %d banks!\n", value, globalMemState.nRomBanks);
      assert(false);
    }

    if(value == 0) value = 1;
    globalMemState.romBank = value;
    //globalMemState.mappedRom = fileData + 0x4000 * value;
  } else if(addr >= 0 && addr < 0x1fff) {
    // RAM enable/disable
    if(value == 0) {
      globalMemState.disabledMappedRam = globalMemState.mappedRam;
      globalMemState.mappedRam = nullptr;
    } else if(value == 0xa) {
      globalMemState.mappedRam = globalMemState.disabledMappedRam;
    } else {
      //assert(false);
    }
  } else if(addr >= 0x4000 && addr < 0x5fff) {
    // RAM bank switch
    if(value < globalMemState.nRamBanks) {
      globalMemState.mappedRam = globalMemState.mappedRamAllocation + 0x2000 * value;
    } else {
      //assert(false);
    }
  } else if(addr == 0x6000) {
    // ?? RTC latch nonsense
  } else {
    assert(false);
  }
}


// handler for all MBC switches
void mbcHandler(u16 addr, u8 value) {
  switch(globalMemState.mbcType) {
    case 0:
      mbc0Handler(addr, value);
      break;
    case 1:
      mbc1Handler(addr, value);
      break;
    case 2:
      mbc2Handler(addr, value);
      break;
    case 3:
      mbc3Handler(addr, value);
      break;
    default:
      assert(false);
      break;
  }
}

u32 hash(u32 key) {
  key = ~key + (key << 15); // key = (key << 15) - key - 1;
  key = key ^ (key >> 12);
  key = key + (key << 2);
  key = key ^ (key >> 4);
  key = key * 2057; // key = (key + (key << 3)) + (key << 11);
  key = key ^ (key >> 16);
  return key;
}


#define CACHE_ENTRY_SIZE_LG2 9
#define N_CACHE_ENTRIES_LG2 6
extern FILE* fp;
u8 romBankCache[(1 << CACHE_ENTRY_SIZE_LG2) * (1 << N_CACHE_ENTRIES_LG2)];
u32 blockIDs[(1<<N_CACHE_ENTRIES_LG2)] = {0};
u32 cacheBlock = 0xffffffff;

u8 readByteRomBank(u16 addr, u8 bank) {
  u16 bankedRomAddress = addr & 0x3fff;               // address relative to start of memory bank
  u32 fileAddress = 0x4000 * bank + bankedRomAddress; // address relative to start of cartridge
  return *(fileData + fileAddress);
//  u32 block = fileAddress >> CACHE_ENTRY_SIZE_LG2;    // block index (relative to start of cartridge)
//  u32 tableSlot = hash(block) & ((1 << N_CACHE_ENTRIES_LG2) - 1);
//  assert(tableSlot < (1 << N_CACHE_ENTRIES_LG2));
//
//  if(blockIDs[tableSlot] == block) {
//    // yeet
//    return romBankCache[tableSlot * (1 << CACHE_ENTRY_SIZE_LG2) + (fileAddress & ((1 << CACHE_ENTRY_SIZE_LG2) - 1))];
//  } else {
//    fseek(fp, (block << CACHE_ENTRY_SIZE_LG2), SEEK_SET);
//    fread(romBankCache + (tableSlot * (1<<CACHE_ENTRY_SIZE_LG2)), 1, (1 << CACHE_ENTRY_SIZE_LG2), fp);
//    //memcpy(romBankCache + (tableSlot * (1<<CACHE_ENTRY_SIZE_LG2)), fileData + (block << CACHE_ENTRY_SIZE_LG2), (1 << CACHE_ENTRY_SIZE_LG2));
//    blockIDs[tableSlot] = block;
//    return romBankCache[tableSlot * (1 << CACHE_ENTRY_SIZE_LG2) + (fileAddress & ((1 << CACHE_ENTRY_SIZE_LG2) - 1))];
//  }


}

// read a u16 from game memory
u16 readU16(u16 addr) {
  return (u16)readByte(addr) + ((u16)(readByte(addr+(u16)1)) << 8);
}

// write a u16 to game memory
void writeU16(u16 mem, u16 addr) {
  writeByte((u8)(mem & 0xff), addr);
  writeByte((u8)(mem >> 8),  addr + (u16)1);
}

// read byte from memory
u8 readByte(u16 addr) {
  switch(addr & 0xf000) {
    case 0x0000: // either BIOS or ROM 0:
      if(globalMemState.inBios) {
        if(addr < 0x100) {
          return bios[addr];
        } else if(addr == 0x100) {
          printf("EXIT BIOS ERROR\n");
          assert(false);
        } else {
          //return pkmn_red_rom_0[addr];
          return globalMemState.rom0[addr]; // todo <- change this for stm32
        }
      } else {
        //return pkmn_red_rom_0[addr];
        return globalMemState.rom0[addr];   // todo <- change this for stm32
      }

    case 0x1000: // ROM 0
    case 0x2000: // ROM 0
    case 0x3000: // ROM 0
      //return pkmn_red_rom_0[addr];
      return globalMemState.rom0[addr];    // todo <- change this for stm32

    case 0x4000: // banked ROM
    case 0x5000:
    case 0x6000:
    case 0x7000:
        return readByteRomBank(addr, globalMemState.romBank);
      //return globalMemState.mappedRom[addr & 0x3fff]; // todo <- change this for stm32

    case 0x8000: // VRAM
    case 0x9000:
      return globalMemState.vram[addr & 0x1fff];

    case 0xa000: // mapped RAM
    case 0xb000:
      if(!globalMemState.mappedRam) {
#ifndef DANGER_MODE
        assert(false);
#endif
        return 0xff;
      }
      return globalMemState.mappedRam[addr & 0x1fff];

    case 0xc000: // internal RAM
    case 0xd000:
      return globalMemState.internalRam[addr & 0x1fff];

    case 0xe000: // interal RAM copy
      return globalMemState.internalRam[addr & 0x1fff];

    case 0xf000: // either internal RAM copy or I/O or top-ram
      switch(addr & 0x0f00) {
        case 0x000:
        case 0x100:
        case 0x200:
        case 0x300:
        case 0x400:
        case 0x500:
        case 0x600:
        case 0x700:
        case 0x800:
        case 0x900:
        case 0xa00:
        case 0xb00:
        case 0xc00:
        case 0xd00:
        case 0xe00:
          return globalMemState.internalRam[addr & 0x1fff];


        case 0xf00:
          if(addr >= 0xff80) {
            return globalMemState.upperRam[addr & 0x7f];
          } else {
            u8 lowAddr = (u8)(addr & 0xff);
            switch(lowAddr) {
              case IO_LY:
              case IO_SCROLLX:
              case IO_SCROLLY:
              case IO_NR10: // nyi
              case IO_NR11: // nyi
              case IO_NR12: // nyi
              case IO_NR13: // nyi
              case IO_NR14: // nyi
              case IO_NR21: // nyi
              case IO_NR22: // nyi
              case IO_NR23: // nyi
              case IO_NR24: // nyi
              case IO_NR30: // nyi
              case IO_NR31: // nyi
              case IO_NR32: // nyi
              case IO_NR33: // nyi
              case IO_NR34: // nyi
              case IO_NR41: // nyi
              case IO_NR42: // nyi
              case IO_NR43: // nyi
              case IO_NR44: // nyi
              case IO_NR50: // nyi
              case IO_NR51: // nyi
              case IO_NR52: // nyi
              case IO_STAT: // nyi
              case IO_WAVE_PATTERN: // nyi
              case IO_LCDC: // nyi
              case IO_BGP:
              case IO_OBP0:
              case IO_OBP1:
              case IO_SERIAL_SB:
              case IO_SERIAL_SC:
              case IO_DIV:
              case IO_TIMA:
              case IO_TMA:
              case IO_TAC:
              case IO_WINY:
              case IO_WINX:
                return globalMemState.ioRegs[lowAddr];
                break;

              case IO_IF:
                printf("read if: 0x%x\n", globalMemState.ioRegs[lowAddr]);
                printf("timer value: 0x%x\n", globalMemState.ioRegs[IO_TIMA]);
                return globalMemState.ioRegs[lowAddr];
                break;


              case IO_P1:
              {
                u8 regP1 = globalMemState.ioRegs[IO_P1];
                //printf("ireg: 0x%x\n", regP1);
                u8 joypad_data = 0;
                if(regP1 & 0x10) {
                  if(keyboard.a) joypad_data += 0x1;
                  if(keyboard.b) joypad_data += 0x2;
                  if(keyboard.select) joypad_data += 0x4;
                  if(keyboard.start) joypad_data += 0x8;
                }
                if(regP1 & 0x20) {
                  if(keyboard.r) joypad_data += 0x1;
                  if(keyboard.l) joypad_data += 0x2;
                  if(keyboard.u) joypad_data += 0x4;
                  if(keyboard.d) joypad_data += 0x8;
                }

                regP1 = (regP1 & 0xf0);
                joypad_data = ~joypad_data;
                joypad_data = regP1 + (joypad_data & 0xf);
                //globalMemState.ioRegs[IO_P1] = joypad_data;
                //printf("jpd: 0x%x\n", joypad_data);
                return joypad_data;
              }

                break;

              case IO_GBCSPEED:
                return 0xff;

              case IO_LYC:
              case IO_DMA:

                printf("unhandled I/O read @ 0x%x\n", addr);
#ifndef DANGER_MODE
                assert(false);
#endif
                break;
              default:
                printf("unknown I/O read @ 0x%x\n", addr);
#ifndef DANGER_MODE
                assert(false);
#endif
                break;
            }

          }
        default:
#ifndef DANGER_MODE
          assert(false);
#endif
          break;

      }
      break;

    default:
#ifndef DANGER_MODE
      assert(false);
#endif
      break;
  }
  assert(false);
  return 0xff;
}


void writeByte(u8 byte, u16 addr) {
  switch(addr & 0xf000) {
    case 0x0000: // ROM 0, but possibly the BIOS area
      if(globalMemState.inBios) {
        printf("ERROR: tried to write into ROM0 or BIOS (@ 0x%04x) during BIOS!\n", addr);
#ifndef DANGER_MODE
        assert(false);
#endif
      } else {
        mbcHandler(addr, byte);
      }
      break;


    case 0x1000: // ROM 0
    case 0x2000: // ROM 0
    case 0x3000: // ROM 0
    case 0x4000: // ROM 1
    case 0x5000: // ROM 1
    case 0x6000: // ROM 1
    case 0x7000: // ROM 1
      mbcHandler(addr, byte);
      break;

    case 0x8000: // VRAM
    case 0x9000:
      globalMemState.vram[addr & 0x1fff] = byte;
      break;

    case 0xa000: // mapped RAM
    case 0xb000:
      if(!globalMemState.mappedRam) {
        //printf("write to unmapped ram @ 0x%x value 0x%x\n", addr, byte);
//#ifndef DANGER_MODE
//        assert(false);
//#endif
        break;
      }
      globalMemState.mappedRam[addr & 0x1fff] = byte;
      break;

    case 0xc000: // internal RAM
    case 0xd000:
      globalMemState.internalRam[addr & 0x1fff] = byte;
      break;

    case 0xe000: // interal RAM copy
      globalMemState.internalRam[addr & 0x1fff] = byte;
      break;

    case 0xf000: // either internal RAM copy or I/O or top-ram
      switch(addr & 0x0f00) {
        case 0x000:
        case 0x100:
        case 0x200:
        case 0x300:
        case 0x400:
        case 0x500:
        case 0x600:
        case 0x700:
        case 0x800:
        case 0x900:
        case 0xa00:
        case 0xb00:
        case 0xc00:
        case 0xd00:
        case 0xe00:
          globalMemState.internalRam[addr & 0x1fff] = byte;
          break;


        case 0xf00:
          if(addr >= 0xff80) {
            globalMemState.upperRam[addr & 0x7f] = byte;
            break;
          } else {
            u16 maskedAddress = addr & 0x7f;
            globalMemState.ioRegs[maskedAddress] = byte;
            u8 lowAddr = (u8)(addr & 0xff);
            switch(lowAddr) {

            case IO_NR11:
            	ch1_length_load();
            	break;
            case IO_NR14:
            	ch1_trigger_load();
            	break;

            case IO_NR13:
               ch1_update_freq();
               break;

            case IO_NR21:
            	ch2_length_load();
            	break;

            case IO_NR23:
            	ch2_update_freq();
            	break;

            case IO_NR24:
            	ch2_trigger_load();
            	break;

            case IO_NR31:
            	ch3_length_load();
            	break;
            case IO_NR33:
            	ch3_update_freq();
            	break;
            case IO_NR34:
            	ch3_trigger_load();
            	break;

            case IO_NR41:
            	ch4_length_load();
            	break;
            case IO_NR44:
            	ch4_trigger_load();

            case IO_NR12:
            case IO_NR10:

            case IO_NR22:


            case IO_NR30:

            case IO_NR32:



            case IO_NR42:
            case IO_NR43:
            case IO_NR50:
            case IO_NR51:
            case IO_NR52:
//            	printf("snd 0x%x 0x%x\r\n", addr, byte);
//            	break;


              case IO_WAVE_PATTERN:
              case 0x31:
              case 0x32:
              case 0x33:
              case 0x34:
              case 0x35:
              case 0x36:
              case 0x37:
              case 0x38:
              case 0x39:
              case 0x3a:
              case 0x3b:
              case 0x3c:
              case 0x3d:
              case 0x3e:
              case 0x3f:
              case IO_BGP:
              case IO_SCROLLX:
              case IO_SCROLLY:
              case IO_LCDC:
              case IO_STAT:
              case IO_OBP0:
              case IO_OBP1:
              case IO_P1:
              case IO_IF:
              case IO_TAC:
              case IO_TIMA:
              case IO_TMA:
              case IO_SERIAL_SB:
              case IO_SERIAL_SC:
              case IO_WINY:
              case IO_WINX:
              case IO_LYC:
                break;

              case IO_EXIT_BIOS:
                if(globalMemState.inBios) {
                  printf("EXIT BIOS by write 0x%x to 0x%x", byte, addr);
                  globalMemState.inBios = false;
                  break;
                } else {
                  printf("tried to write to 0xff50 when not in bios?\n");
                  break;
                }
                break;

              case IO_DMA:
              {
                u16 dmaAddr = ((u16)byte) << 8;
                for(u16 i = 0; i < 160; i++) {
                  writeByte(readByte(dmaAddr + i), (u16)0xfe00 + i);
                }
                break;
              }

              case 0x7f:
                printf("OOPS\n");
                break;


              case IO_LY:
                printf("unhandled I/O write @ 0x%x\n", addr);
#ifndef DANGER_MODE
                assert(false);
#endif
                break;
              default:
                printf("unknown I/O write @ 0x%x\n", addr);
#ifndef DANGER_MODE
                assert(false);
#endif
                break;
            }
            break;
          }
        default:
#ifndef DANGER_MODE
          assert(false);
#endif
          break;
      }
      break;
    default:
#ifndef DANGER_MODE
      assert(false);
#endif
      break;
  }
}
