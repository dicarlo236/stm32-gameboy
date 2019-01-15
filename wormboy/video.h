#ifndef GBC_VIDEO_H
#define GBC_VIDEO_H

#include "types.h"

struct VideoState {
  u32 mode;
  u32 modeClock;
  u32 line;
  u8* frameBuffer;
};

extern VideoState globalVideoState;

void initVideo(u8* frameBuffer);
void stepVideo(u32 cycles);
void renderLine();


#endif //GBC_VIDEO_H
