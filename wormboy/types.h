#ifndef GBC_TYPES_H
#define GBC_TYPES_H

#include <stdint.h>
#include <stddef.h>

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define s8 int8_t
#define s16 int16_t
#define s32 int32_t
#define nullptr NULL

//using u8 = uint8_t;
//using u16 = uint16_t;
//using u32 = uint32_t;
//
//using s8 = int8_t;
//using s16 = int16_t;
//using s32 = int32_t;

struct KeyState {
  bool a,b,u,d,l,r,start,select,turbo, save, load;
};

extern KeyState keyboard;

#endif //GBC_TYPES_H
