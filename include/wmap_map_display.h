#ifndef WMAP_MAP_DISPLAY_H
#define WMAP_MAP_DISPLAY_H

#include "common.h"

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
} WmapResource;

typedef struct
{
    s16 unk0;
    s16 texture_index;
} WmapMarker;

typedef struct
{
    u8 _pad00[5];
    u8 phase_mode;
    u8 phase;
    u8 _pad07[7];
    s16 frame_index;
    s16 previous_frame_index;
    u8* animation_cursor;
    u8* animation_start;
    s8* quad_data;
    s16 frame_timer;
} WmapSpriteState;

void func_80054A2C();
void func_800551A8();
void func_8005536C();
void func_8005556C();
void func_80055830();
void func_80055AF4();
s32 func_80055BB0();
void func_80055E5C();
void func_800561F8();
void func_80056824();
void func_80056C30();
void func_800571A4();
void func_80057274();
void func_800574D0();
void func_8005784C();
void func_80057C14();
void func_80057D2C();
void func_80058014();
void func_80058110();
void func_800581A0();
void func_80058260();
void func_80058298();
s32 func_800582A0();
void func_800582E8();
void func_8005833C();
s32 func_80058400();
s32 func_80058488();
void func_80058490();
void func_80058498();

#endif
