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

extern u_char g_cdAudioReady;

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