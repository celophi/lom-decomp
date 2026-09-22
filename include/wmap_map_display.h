#ifndef WMAP_MAP_DISPLAY_H
#define WMAP_MAP_DISPLAY_H

#include "common.h"

/** @brief Resource record used by the world-map display cache. */
typedef struct
{
    u8 pad0[2];
    s16 resource_id;
    u8 pad4;
    u8 state;
    u8 pad6[0xA];
    s16 slot;
    u8 pad12[2];
    s32 busy;
    u8 pad18[0x14];
} WmapMapDisplayResource;

void func_80054A2C(void);
s32 func_80055BB0(WmapMapDisplayResource* resource);
void func_80056824(void);
void func_800571A4(void);
void func_80057C14(void);
void func_80058260(void);
void func_80058298(void);
s32 func_800582A0();
void func_800582E8(void);

#endif
