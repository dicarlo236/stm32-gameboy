#include <stdlib.h>
#define NDEBUG
#include <assert.h>
#include "video.h"
#include "mem.h"
#include "ntsc.h"

u32 frameCount = 0;

// the implementation of sprites is poor.
// pick a value for this that is between 0 and 256 and not equal to one of the colors
#define TRANSPARENT_SPRITE 37

#define BRIGHTEST_COLOR 6

VideoState globalVideoState;

// colors on screen (white, light gray, dark gray, black)
// the first one must be "BRIGHTEST_COLOR" - other code depends on this!
static const u8 colors[4] = {BRIGHTEST_COLOR, 4, 2, 0};




void initVideo(u8* frameBuffer) {
  globalVideoState.mode = 0;
  globalVideoState.modeClock = 0;
  globalVideoState.line = 0;
  globalVideoState.frameBuffer = frameBuffer; // <- todo change for STM32

//  for(u32 i = 0; i < 160*144; i++) {
//    globalVideoState.frameBuffer[i] = 255;
//  }

}

inline u32 xy2px(u8 x, u8 y) {
    return H_RES*(y+Y0s) + x + X0s;
}

void dumpVideo() {
  for(u16 i = 0x8000; i < 0x87ff; i++) {
    if((i%8) == 1) {
      printf("@ 0x%04x: ", i);
    }
    printf("0x%02x ", readByte(i));
    if(!(i%8)) printf("\n");
  }
  printf("\n");
  assert(false);
}

// read a pixel out of a tile and apply the given palette
u8 readTile(u16 tileAddr, u8 x, u8 y, u8 palette) {
  assert(x <= 8);
  assert(y <= 8);
  x = (7 - x);
  u16 loAddr = tileAddr + (y*(u16)2);
  u16 hiAddr = loAddr + (u16)1;
  u8 lo = readByte(loAddr);
  u8 hi = readByte(hiAddr);
  u8 loV = (lo >> x) & (u8)1;
  u8 hiV = (hi >> x) & (u8)1;
  //u8 result = loV * 120 + hiV * 60;
  u8 colorIdx = loV + 2 * hiV;
  u8 colorID = (palette >> (2 * colorIdx)) & 3;
  return colors[colorID];
}

u8 readTilePtr(u8* tileAddr, u8 x, u8 y, u8 palette) {
  assert(x <= 8);
  assert(y <= 8);
  x = (7 - x);
  u8* loAddr = tileAddr + (y*(u16)2);
  u8* hiAddr = loAddr + (u16)1;
  u8 lo = *(loAddr);
  u8 hi = *(hiAddr);
  u8 loV = (lo >> x) & (u8)1;
  u8 hiV = (hi >> x) & (u8)1;
  //u8 result = loV * 120 + hiV * 60;
  u8 colorIdx = loV + 2 * hiV;
  u8 colorID = (palette >> (2 * colorIdx)) & 3;
  return colors[colorID];
}

// read a pixel out of a tile and apply the given palette
// returns TRANSPARENT_SPRITE if the sprite should be transparent
u8 readSpriteTile(u16 tileAddr, u8 x, u8 y, u8 palette) {
  //tileAddr = 0x8180;
  assert(x <= 8);
  assert(y <= 8);
  x = (7 - x);
  u16 loAddr = tileAddr + (y*(u16)2);
  u16 hiAddr = loAddr + (u16)1;
  u8 lo = readByte(loAddr);
  u8 hi = readByte(hiAddr);
  u8 loV = (lo >> x) & (u8)1;
  u8 hiV = (hi >> x) & (u8)1;
  u8 colorIdx = loV + 2 * hiV;
  if(colorIdx == 0) {
    return TRANSPARENT_SPRITE;
  }
  u8 colorID = (palette >> (2 * colorIdx)) & 3;
  return colors[colorID];
}

u8 readSpriteTileAddr(u8* tileAddr, u8 x, u8 y, u8 palette) {
  //tileAddr = 0x8180;
  assert(x <= 8);
  assert(y <= 8);
  x = (7 - x);
  u8* loAddr = tileAddr + (y*(u16)2);
  u8* hiAddr = loAddr + (u16)1;
  u8 lo = *(loAddr);
  u8 hi = *(hiAddr);
  u8 loV = (lo >> x) & (u8)1;
  u8 hiV = (hi >> x) & (u8)1;
  u8 colorIdx = loV + 2 * hiV;
  if(colorIdx == 0) {
    return TRANSPARENT_SPRITE;
  }
  u8 colorID = (palette >> (2 * colorIdx)) & 3;
  return colors[colorID];
}

// compute the address of the tile from the tile's index
// this is confusing because depending on the tileData selected,
// the tileIdx is either signed or unsigned
u16 computeTileAddr(u8 tileIdx, bool tileData) {
  if(tileData) {
    return 0x8000 + 16 * tileIdx;
  } else {
    if(tileIdx <= 127) {
      return 0x9000 + 16 * tileIdx;
    } else {
      return 0x8000 + 16 * (tileIdx);
    }
  }
}

u8* computeTileAddrPtr(u8 tileIdx, bool tileData) {
  if(tileData) {
    return globalMemState.vram + 16 * tileIdx;
  } else {
    if(tileIdx <= 127) {
      return globalMemState.vram + 0x1000 + 16 * tileIdx;
    } else {
      return globalMemState.vram + 16 * (tileIdx);
    }
  }
}

// main function to render a line of the display.
// this implementation is missing a number of things, including (but not limited to)
//  -- proper position of the WINDOW
//  -- 16x8 sprites
//  -- sprite sorting
//  -- 10 sprite limit
u32 gameboyFrameCount = 0;
extern u32 frameCountNTSC;
u8 skipFrame = 0;
void renderLine() {
  if(frameCount % 8) return;
//	if(skipFrame || (globalVideoState.line == 0 && (frameCountNTSC - gameboyFrameCount) > 5)) {
//		skipFrame = 1;
//		return;
//	}
  //return;
  //if(drawing) return;
  //printf("%x %x\n", im_line_va, im_back);
  globalVideoState.frameBuffer = im_line_vas[bufferSelect];
  u8 lcdc = globalMemState.ioRegs[IO_LCDC];    // lcd control register
  bool lcdOn            = (lcdc >> 7) & (u8)1; // lcd display on?
  bool windowTileMap    = (lcdc >> 6) & (u8)1; // select tilemap source for window
  bool windowEnable     = (lcdc >> 5) & (u8)1; // draw window?
  bool tileData         = (lcdc >> 4) & (u8)1; // select tile data source
  bool bgTileMap        = (lcdc >> 3) & (u8)1; // select tilemap source for background
  bool objSize          = (lcdc >> 2) & (u8)1; // pick sprite size (nyi)
  bool objEnable        = (lcdc >> 1) & (u8)1; // enable sprite renderer
  bool bgWinEnable      = (lcdc >> 0) & (u8)1; // enable background and window renderer?

  u16 windowMapAddr = (u16)(windowTileMap ? 0x9c00 : 0x9800);
  u16 bgTileMapAddr = (u16)(bgTileMap     ? 0x9c00 : 0x9800);

  // background renderer
  if(lcdOn && bgWinEnable) {
    // render background onto framebuffer
    u8 pal = globalMemState.ioRegs[IO_BGP]; // color palette
    u16 tileMapRowAddr = (u16)(bgTileMapAddr + 32*((((u16)globalVideoState.line +
            globalMemState.ioRegs[IO_SCROLLY]) & (u16)255) >> 3)); // address of the row of the tilemap
    u8  tileMapColIdx  = globalMemState.ioRegs[IO_SCROLLX] >> 3;   // column index of the tilemap
    u8  yPixOffset     = ((u8)globalVideoState.line + globalMemState.ioRegs[IO_SCROLLY]) & (u8)7; // y-pixel of tile
    u8  xPixOffset     = globalMemState.ioRegs[IO_SCROLLX] & (u8)7;                               // x-pixel of tile
    //u8* linePtr        = globalVideoState.frameBuffer + 160 * globalVideoState.line;              // frame buffer pointer
    u8 tileIdx        = readByte(tileMapRowAddr + tileMapColIdx);                                 // tile index

    // loop over pixels in the line
    for(u8 px = 0; px < 160; px++) {
      globalVideoState.frameBuffer = im_line_vas[bufferSelect];
      globalVideoState.frameBuffer[xy2px(px,globalVideoState.line)] =
      readTilePtr(computeTileAddrPtr(tileIdx, tileData), xPixOffset, yPixOffset, pal);
      //readTile(computeTileAddr(tileIdx, tileData), xPixOffset, yPixOffset, pal); // set the frame buffer

      xPixOffset++; // increment tile pixel
      //linePtr++;    // increment frame buffer pixel
      if(xPixOffset == 8) { // if we have overflowed the tile
        xPixOffset = 0;     // go to the beginning
        tileMapColIdx = (tileMapColIdx + 1) & 31; // of the next tile (allow wraparound)
        tileIdx = readByte(tileMapRowAddr + tileMapColIdx); // and look up the tile index in the tile map
      }
    }
  }

  // window renderer
  if(windowEnable) {
    u8 pal = globalMemState.ioRegs[IO_BGP];   // palette
    u8 wx = globalMemState.ioRegs[IO_WINX];   // location of the window (nyi)
    u8 wy = globalMemState.ioRegs[IO_WINY];   // location of the window (nyi)
    if(wx > 166 || wy > 143) {
      // if the window is out of this range, it is disabled too.
    } else {
      u16 tileMapRowAddr = windowMapAddr + 32*((((u16)globalVideoState.line)) >> 3);  // address of the row of the tilemap
      u8 tileMapColIdx = 0; // column index of the tilemap
      u8 yPixOffset = ((u8)globalVideoState.line) & (u8)7; // y-pixel of tile
      u8 xPixOffset = 0; // x-pixel of tile
      //u8* linePtr        = globalVideoState.frameBuffer + 160 * globalVideoState.line;  // frame buffer pointer
      u8 tileIdx        = readByte(tileMapRowAddr + tileMapColIdx); // tile index

      // loop over pixels in the line
      for(u8 px = 0; px < 160; px++) {
        globalVideoState.frameBuffer[xy2px(px,globalVideoState.line)] =
                readTilePtr(computeTileAddrPtr(tileIdx, tileData), xPixOffset, yPixOffset, pal);
        //*linePtr = readTile(computeTileAddr(tileIdx, tileData), xPixOffset, yPixOffset, pal); // set the frame buffer

        xPixOffset++; // increment tile pixel
        //linePtr++;    // increment frame buffer pixel
        if(xPixOffset == 8) { // if we have overflowed the tile
          xPixOffset = 0;     // go to the beginning
          tileMapColIdx = (tileMapColIdx + 1) & 31;  // of the next tile (allow wraparound, but it shouldn't happen?)
          tileIdx = readByte(tileMapRowAddr + tileMapColIdx); // and look up the tile index in the tile map
        }
      }
    }
  }

  // sprite renderer
  if(objEnable) {
    for(u16 spriteID = 0; spriteID < 40; spriteID++) {
      u16 oamPtr = 0xfe00 + 4 * spriteID; // sprite information table
      u8 spriteY = readByte(oamPtr);      // y-coordinate of sprite
      u8 spriteX = readByte(oamPtr + 1);  // x-coordinate of sprite
      u8 patternIdx = readByte(oamPtr + 2); // sprite pattern
      u8 flags = readByte(oamPtr + 3);      // flag bits

      bool pri   = (flags >> 7) & (u8)1;    // priority (transparency stuff)
      bool yFlip = (flags >> 6) & (u8)1;    // flip around y?
      bool xFlip = (flags >> 5) & (u8)1;    // flip around x?
      bool palID = (flags >> 4) & (u8)1;    // palette ID (OBP0/OBP2)

      u8 pal = palID ? globalMemState.ioRegs[IO_OBP1] : globalMemState.ioRegs[IO_OBP0];


      if(spriteX | spriteY) {
        // the sprite coordinates have an offset
        u8 spriteStartY = spriteY - 16;
        u8 spriteLastY = spriteStartY + 8; // todo 16 row sprites

        // reject based on y if the sprite won't be visible in the current line
        if(globalVideoState.line < spriteStartY || globalVideoState.line >= spriteLastY) {
          continue;
        }

        // get y px relative to the sprite pattern
        u8 tileY = globalVideoState.line - spriteStartY;
        if(yFlip) {
          tileY = 7 - tileY;
        }

        assert(tileY < 8);

        // loop over the 8 pixels that the sprite is on:
        for(u8 tileX = 0; tileX < 8; tileX++) {

          u8 xPos = spriteX - 8 + tileX; // position on the screen
          if(xPos >= 160) continue; // reject if we go off the end, don't wrap around

          u32 fbIdx = xy2px(xPos, globalVideoState.line);

          globalVideoState.frameBuffer = im_line_vas[bufferSelect];


          // current color at the screen
          u8 old = globalVideoState.frameBuffer[fbIdx];

          // get the pixel from the sprite pattern data
          u8 tileLookupX = tileX;
          if(xFlip) {
            tileLookupX = 7 - tileX;
          }
          //u8 tileValue = readSpriteTile(0x8000 + patternIdx * 16, tileLookupX, tileY, pal);
          u8 tileValue = readSpriteTileAddr(globalMemState.vram + patternIdx * 16, tileLookupX, tileY, pal);
          //u8 tileValue = readSpriteTileAddr(globalMemState.vram + patternIdx*16, tileLookupX, tileY, pal);
          //u8 tileValue = 4;
          // don't draw transparent
          if(tileValue == TRANSPARENT_SPRITE) continue; // (transparent sprites)

          // not sure this is 100% right...
          if(!pri) {
            globalVideoState.frameBuffer[fbIdx] = tileValue;
          } else {
            if(old == BRIGHTEST_COLOR) {
              globalVideoState.frameBuffer[fbIdx] = tileValue;
            }
          }
        }
      }
    }
  }
}

static u32 oldTics = 0;

float filteredFrameRate = 0.f;

// step the video by a number of clock cycles
void stepVideo(u32 cycles) {
  globalVideoState.modeClock += cycles;
  switch(globalVideoState.mode) {
    case 2: // OAM read, scanning

      if(globalVideoState.modeClock >= 80) {
        globalVideoState.modeClock = 0;
        globalVideoState.mode = 3; //  VRAM read, scanning
      }
      break;
    case 3: //  VRAM read, scanning
      if(globalVideoState.modeClock >= 172) {
        globalVideoState.modeClock = 0;
        globalVideoState.mode = 0; // hblank
        renderLine(); // draw line into framebuffer
      }
      break;
    case 0: // hblank
      if(globalVideoState.modeClock >= 204) {
        globalVideoState.modeClock = 0;
        globalVideoState.line++;

        if(globalVideoState.line == 143) {

          globalVideoState.mode = 1; // vblank
          globalMemState.ioRegs[IO_IF] |= 0x1; // set interrupt for vblank
          printf("lag: %d\r\n", frameCountNTSC - gameboyFrameCount);

          gameboyFrameCount++;
          while(gameboyFrameCount > frameCountNTSC) {

          }
          //if(!keyboard.turbo) // if we are in "turbo" mode, don't update graphics
            //updateGraphics(); // display framebuffer on screen
          //bufferSelect = !bufferSelect;

          skipFrame = 0;
          int aaaa = tics - oldTics;
          float frameTime = 0.0000064 * aaaa;
          filteredFrameRate = (0.94 * filteredFrameRate) + (0.06 * frameTime);
          //printf("%d %f %f\n", aaaa, 1.f/frameTime, 1.f/filteredFrameRate);
          frameCount++;
          oldTics = tics;

        } else {
          globalVideoState.mode = 2; // oam
        }
      }
      break;
    case 1: // vblank
      if(globalVideoState.modeClock >= 456) {
        globalVideoState.modeClock = 0;
        globalVideoState.line++;

        if(globalVideoState.line > 153) {
          globalVideoState.mode = 2;
          globalVideoState.line = 0;
        }
      }
      break;
    default:
      assert(false);
  }

  globalMemState.ioRegs[IO_LY] = (u8)globalVideoState.line; // update current line


  // this is likely somewhat wrong.
  u8 stat = globalMemState.ioRegs[IO_STAT]; // update STAT register (this is likely the source of issue on bubble bobble)
  stat &= ~7; // clear mode, coincidence
  stat += globalVideoState.mode; // set current mode
  if(globalMemState.ioRegs[IO_LYC] == globalMemState.ioRegs[IO_LY]) { // check coincidence
    stat += 4;
    if((stat >> 6) & 1) {
      globalMemState.ioRegs[IO_IF] |= 2; // stat interrupt
    }
  }
}
