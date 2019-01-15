#ifndef GBC_PLATFORM_H
#define GBC_PLATFORM_H

#include "types.h"

struct FileLoadData {
  u8* data;
  u32 size;
};

void* badalloc_check(u32 size, const char* alloc_name);
FileLoadData loadFile(const char* name);
void saveFile(const char* name, FileLoadData info);
void updateKeyboard(KeyState* keys);

#endif //GBC_PLATFORM_H
