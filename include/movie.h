#ifndef _MOVIE_H
#define _MOVIE_H

#include "common.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"

typedef struct
{
    u_char unk0;
    u_char _pad1;
    u_short unk2;
    u_short unk4;
} SRC_801ED600;

typedef struct
{
    u_char _pad0[0x98];
    u_char unk98;
    u_char _pad99;
    u_char _pad9a;
    u_char _pad9b;
    u_char _pad9c;
    u_char unk9d;
    u_char _pad9e;
    u_char unk9f;
} SRC_801ED500;

typedef struct
{
    u8* unk0;
    u8* unk4;
    u8* unk8;
    u8* unkC;
    u8* unk10;
    u8* unk14;
    u8* unk18;
    u8* unk1C;
    RECT rects[3];
    u32 unk38;
    u32 unk3C;
    u8 pad_40[4];
    u32 unk44;
    u32 unk48;
    u32 unk4C;
    u32 unk50;
    u32 unk54;
    u32 unk58[9];
    u8 pad_7C[2];
    u16 unk7E;
    u32 unk80;
    u32 unk84;
    u32 unk88;
    u32 unk8C;
    s8 unk90;
    s8 unk91;
    s8 unk92;
    s8 unk93;
    s8 unk94;
    s8 unk95;
    s8 unk96;
    s8 unk97;
    s8 unk98;
    s8 unk99;
    s8 unk9A;
    s8 unk9B;
    s8 unk9C;
    s8 unk9D;
    s8 unk9E;
    s8 unk9F;
} UnkState;
typedef struct
{
    u8 pad[0x38];
    u32 unk38;
} AllocInfo;

extern u_char g_cdAudioReady;
extern void *D_80180014;
extern u8 D_801ED590;
extern void func_80140AC0(void);
extern void func_801416C4(void);
extern u32 *func_80140F04(s32 param_1, u32 param_2);

extern void CD_UpdateAndProcessQueue(void);
extern s32 CD_GetErrorStatus(void);
extern void CD_ResetSystem(void);
extern void func_800157DC(void);
extern void func_800157B0(u_long arg0);
extern void func_800158E0(void);
extern void func_80140358(s32 a0, s32 a1, s32 a2, s32 a3);
extern void func_801406E4(void);
extern void FUN_80140d48(void);
extern void func_80023030(s32 arg0);

#endif