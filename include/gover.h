#ifndef _GOVER_H
#define _GOVER_H

#include "common.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"
#include "psyq/libetc.h"


// Structure for arg0 (the header)
typedef struct {
    u8  _pad0[8];       // offsets 0x00-0x07
    u32 offset;         // offset 0x08
    u8  _pad1[4];       // offsets 0x0C-0x0F
    u16 width;          // offset 0x10
    u16 height;         // offset 0x12
    u_long *image_data; // offset 0x14
} Header;

// Structure for the sub‑header (at arg0 + offset + 8)
typedef struct {
    u8  _pad0[8];       // offsets 0x00-0x07
    u16 w;              // offset 0x08
    u16 h;              // offset 0x0A
    u_long *data;       // offset 0x0C
} SubHeader;

// Structure for arg1 (the rectangle coordinates)
typedef struct {
    u16 x0;   // offset 0x00
    u16 y0;   // offset 0x02
    u16 x1;   // offset 0x04
    u16 y1;   // offset 0x06
} Arg1;

typedef struct
{
  u32 unk0;
  u32 unk4;
  u32 unk8;
  u8 unk12[1];
} D_80119F00_t;


extern void FUN_8002279c(undefined4 param_1, u_int param_2);
extern void FUN_80022aa8(void);
extern void FUN_80022ac8(void);
extern void func_800224D8(s32);
extern s32 func_800A368C(s32, s32);
extern s32 func_800A380C(void);
extern s32 func_800A39A8(s32, s32, s32, s32);
extern s32 func_801401F0(void);
extern s32 func_80140538(s32, s16*, s32);
extern s32 func_80140648(s32);
extern s32 D_8011588C;
extern s32 D_80140708;
extern s32 D_80141048;
extern u8 D_801407A0[];
extern s32 D_80140710[];
extern s32 D_80122988;
extern u32 D_8003EC90;
extern s32 D_8010D018;
extern D_80119F00_t D_80119F00;
extern s32 D_80180004;
extern u8 D_80180000[];

#endif