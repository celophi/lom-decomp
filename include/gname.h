#ifndef _GNAME_H
#define _GNAME_H

#include "common.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"

/**
 * @brief RGB lerp state.
 *
 * Used as a pair: `D_8014F818` is the *target* (final color + remaining
 * step count), `D_8014F828` is the *current* interpolated value (its
 * `steps` field is unused). Each tick @ref func_80140410 advances the
 * current toward the target by `(target - current) / steps` and decrements
 * `steps`. Channels are 0..0x100 with 0x100 meaning "no tint"; values
 * above 0x100 trigger an additive draw mode (GP0 0xE1 abr=2).
 */
typedef struct
{
    s32 r;     /* 0x0 — red channel,   0..0x100 normal, >0x100 = additive */
    s32 g;     /* 0x4 — green channel, 0..0x100 normal, >0x100 = additive */
    s32 b;     /* 0x8 — blue channel,  0..0x100 normal, >0x100 = additive */
    s32 steps; /* 0xC — frames remaining in the lerp (target struct only) */
} DataStruct;

// Structure for the argument object
typedef struct
{
    s32 unk0;             // offset 0x00
    char pad[0x4040 - 4]; // padding up to offset 0x4040
    void* unk4040;        // offset 0x4040
} ArgStruct;

typedef struct
{
    u8 _pad0[0x38];
    u32 unk38;
    u8 _pad1[0x4040 - (0x38 + sizeof(u32))];
    s32* unk4040;
    u8 _pad2[0x8];
    u32 unk404C;
} UnkStruct;

/* Object structure (offsets from target assembly) */
typedef struct
{
    u8 _pad0[0x28];
    u32 unk28; /* offset 0x28 */
    u8 _pad1[0x4040 - 0x28 - 4];
    u32* unk4040; /* offset 0x4040 */
    u8 _pad2[0x404C - 0x4040 - 4];
    u32 unk404C; /* offset 0x404C */
} Obj;

typedef struct
{
    u8 field0;
    u8 field1;
    u8 field2;
    u8 field3;
    u32 field4;
} TableEntry;

typedef struct
{
    u8 pad0[0x10];
    s16 unk10;
    u8 pad1[2];
} UnkStruct2;

extern s32 D_8014F840;
extern s32 D_8014F8CC;
extern DataStruct D_8014F818;
extern DataStruct D_8014F828;
extern s32 D_8014F880;
extern u8 D_80147494[];
extern s32 D_800F22AC;
extern s32 D_8014F8BC;
extern s32 D_8014F8A8;
extern s32 D_80122988;
extern s32 D_8014F844;
extern s32 D_8014F7E4;
extern s8 D_8014F8B8;
extern s8 D_8014F8B0;
extern s8 D_8014F850;
extern char D_8014F7E8;
extern s32 D_8014F848;
extern s32 D_8014F884;
extern s32 D_8014F888;
extern s32 D_8014F88C;
extern s32 D_8014F890;
extern s32 D_8014F894;
extern s32 D_8014F89C;
extern s32 D_8014F8A4;
extern s32 D_8014F8AC;
extern s32 D_8014F8B4;
extern s32 D_8014F8C0;
extern s32 D_8014F8C4;
extern s32 D_8014F8D0;
extern u8 D_8014F7B0;
extern u8* D_80142F04;
extern s32 D_8014F7E0;
extern s32 D_8014F83C;
extern u8* D_80142F00;
extern u32 D_80142E0C[];
extern s32 D_8014F8A0;
extern u8* D_80142EFC;
extern u32 D_80142E40[];
extern s32 D_8014F898;
extern u8* D_80142EF8;
extern u32 D_80142C98[];
extern u32 D_80142CAC[];
extern s32 D_8014F8C8;
extern s32 D_80142CA4;
extern void* D_8014F84C;
extern s32 D_8014F838;
extern u8 D_80142EF4[];
extern unsigned char D_80142CD4[];
extern s32 D_80142E14;
extern u32 D_8014F6B8[];

extern void func_800A3938(int, int);
extern void func_8014139C(void);
extern s32 func_80142720(s32);
extern s32 func_80142C50(s32);
extern s32 func_80140AB8(s32, s32);
extern void func_801428A4(s32, void*);

#endif