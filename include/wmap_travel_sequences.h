#ifndef WMAP_TRAVEL_SEQUENCES_H
#define WMAP_TRAVEL_SEQUENCES_H

#include "common.h"

typedef struct
{
    u8 pad_00[0xE];
    s16 sequence;
    s16 previous_sequence;
} WmapAnimation;

WmapAnimation * func_80099754();
void func_80099848();
void func_80099918();
s16 func_800999D0();
s32 func_80099B50();
s32 func_80099E84();
s32 func_80099F98();
void func_8009A114();
void func_8009A258();
void func_8009A328();
s32 func_8009A3E0();
s32 func_8009A420();
void func_8009A498();
void func_8009A4B0();
void func_8009A588();
void func_8009A5BC();
void func_8009A668();
void func_8009A6A8();
void func_8009A75C();
void func_8009A798();
void func_8009A7D0();
void func_8009A854();
void func_8009A8D0();
void func_8009A90C();
void func_8009A944();
void func_8009A9A8();
void func_8009A9F0();
void func_8009AA2C();
void func_8009AA6C();
void func_8009AB00();
s32 func_8009AB20();
void func_8009AB98();
void func_8009ABB0();
void func_8009ABF0();
void func_8009AC6C();
void func_8009ACA8();
void func_8009ACE0();
void func_8009AD44();
void func_8009AD8C();
void func_8009ADC8();
void func_8009AE08();
void func_8009AE9C();

#endif
