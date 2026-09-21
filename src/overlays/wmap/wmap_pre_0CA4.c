#include "common.h"
#include "sdk/libgte.h"

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad_04[2];
    u8 field_06;
    u8 pad_07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad_12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad_28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern s32 D_801ADAF4;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_8006CC4C(void*, void*);
extern void func_8006683C(s32);
extern void func_80066F9C(void*, s32, s32, s32, s32);
extern void func_8006CAC0(s32 (*callback)(s32));
extern void func_800BF4C0(void);
extern void func_800BF5D4(void);
extern void func_800BF6F8(void);
extern void func_800BF920(void);
extern void func_800BFA24(void);
extern void func_800BFB28(void);
extern void func_800BFC28(void);
extern void func_800C07DC(void);
extern void func_800C02FC(void);
extern s32 func_800C07F8(s32);
extern s32 func_800C09F4(s32);
extern void func_800C0960(void);
extern void func_800C0B00(void);
extern void func_800C0BC8(void);

void func_800C02C0(void)
{
    if (D_8013924C == 0)
    {
        D_801B3220++;
        func_800C02FC();
    }
}

void func_800C02FC(void)
{
    func_8006CAC0(func_800C07F8);
    D_801B3224 = 0xC;
    D_801B3220++;
}

void func_800C0338(void)
{
    if (--D_801B3224 == 0)
    {
        D_801B3220++;
    }
}

void func_800C036C(void)
{
    func_8006CAC0(func_800C09F4);
    D_801B3224 = 0x5A;
    D_801B3220++;
}

void func_800C03A8(void)
{
    if (--D_801B3224 == 0)
    {
        D_801B3220++;
    }
}

void func_800C03DC(void)
{
    func_8006683C(0x808080);
    D_801ADAF4 = 0x10;
    D_801B3224 = 0x3C;
    D_801B3220++;
}

void func_800C0424(void)
{
    if (--D_801B3224 == 0)
    {
        D_801B3220++;
    }
}

void func_800C0458(void)
{
    D_8013B20C = 0;
    D_801B3220++;
}

s32 func_800C0474(s32 reset)
{
    if (reset != 0)
    {
        D_801B3228 = 1;
        D_801B322C = 1;
        return 1;
    }

    if ((u32)D_801B3228 >= 8)
    {
        return 0;
    }

    D_800D7A64[D_801B3228]();
    return 1;
}

void func_800C04EC(void)
{
    D_801B3228 = 1;
    D_801B322C = 1;
}

void func_800C0504(void)
{
    D_801B24A0 = D_80139258;
    D_801B2650 = D_8011CF60;
    D_80182DE8 = 0;
    D_801B2650.vz = 0x1388;
    D_80139240 = 0;
    D_8013923C = 0;
    D_801B322C = 0x3C;
    D_801B3228++;
    func_800BF4C0();
}

void func_800C05C0(void)
{
    D_801B322C = 0xB4;
    D_801B3228++;
    func_800BF5D4();
}

void func_800C05F8(void)
{
    D_801B322C = 0x1E;
    D_801B3228++;
    func_800BF6F8();
}

void func_800C0630(void)
{
    D_8013924C = 0;
    D_801B3228++;
}

s32 func_800C064C(s32 reset)
{
    if (reset != 0)
    {
        D_801B3230 = 1;
        D_801B3234 = 1;
        return 1;
    }

    if ((u32)D_801B3230 >= 0xA)
    {
        return 0;
    }

    D_800D7A84[D_801B3230]();
    return 1;
}

void func_800C06C4(void)
{
    D_801B3230 = 1;
    D_801B3234 = 1;
}

void func_800C06DC(void)
{
    if (D_801398D0 != 2)
    {
        D_801B3230++;
        func_800BF920();
    }
}

void func_800C071C(void)
{
    if (D_801398D0 != 2)
    {
        D_801B3230++;
        func_800BFA24();
    }
}

void func_800C075C(void)
{
    if (D_801398D0 != 2)
    {
        D_801B3230++;
        func_800BFB28();
    }
}

void func_800C079C(void)
{
    if (D_801398D0 != 2)
    {
        D_801B3230++;
        func_800C07DC();
    }
}

void func_800C07DC(void)
{
    D_80139234 = 0;
    D_801B3230++;
}

s32 func_800C07F8(s32 reset)
{
    if (reset != 0)
    {
        D_801B3238 = 1;
        D_801B323C = 1;
        return 1;
    }

    if ((u32)D_801B3238 >= 6)
    {
        return 0;
    }

    D_800D7AAC[D_801B3238]();
    return 1;
}

void func_800C0870(void)
{
    D_801B3238 = 1;
    D_801B323C = 1;
}

void func_800C0888(void)
{
    D_801399B4 = &D_8011F538;
    D_800D9344.field_06 = 0xF;
    D_800D9344.field_0E = 1;
    D_800D9344.field_10 = -1;
    D_800D9344.field_02 = 0;
    D_800D9344.field_26 = 0;
    D_800D9344.field_22 = 0x80;
    D_800D9344.field_24 = 0x80;
    D_80182D60.field_00 = 0;
    D_80182D60.field_02 = 0;
    D_801B323C = 0x64;
    D_801B3238++;
    func_800BFC28();
}

void func_800C0914(void)
{
    D_800D9344.field_26 = 4;
    D_800D9344.field_22 = 0;
    D_801B323C = 0x20;
    D_801B3238++;
    func_800C0960();
}

void func_800C0960(void)
{
    func_8006CC4C(&D_800D9344, &D_801399B0);
    func_80066F9C(&D_800D9344, D_80182D58, 8, 8, 0);
    if (--D_801B323C == 0)
    {
        D_801B3238++;
    }
}

void func_800C09DC(void)
{
    D_801B3238++;
}

s32 func_800C09F4(s32 reset)
{
    if (reset != 0)
    {
        D_801B3240 = 1;
        D_801B3244 = 1;
        return 1;
    }

    if ((u32)D_801B3240 >= 6)
    {
        return 0;
    }

    D_800D7AC4[D_801B3240]();
    return 1;
}

void func_800C0A6C(void)
{
    D_801B3240 = 1;
    D_801B3244 = 1;
}

void func_800C0A84(void)
{
    D_801399BC = &D_8011F538;
    D_800D9370.field_06 = 0xF;
    D_800D9370.field_10 = -1;
    D_800D9370.field_26 = 0x10;
    D_800D9370.field_02 = 0;
    D_800D9370.field_0E = 0;
    D_800D9370.field_22 = 0x80;
    D_800D9370.field_24 = 0;
    D_801B3244 = 0x3E;
    D_801B3240++;
    func_800C0B00();
}

void func_800C0B00(void)
{
    func_8006CC4C(&D_800D9370, &D_801399B8);
    func_80066F9C(&D_800D9370, D_8011CF4C, 8, 8, 0);
    if (--D_801B3244 == 0)
    {
        D_801B3240++;
    }
}

void func_800C0B7C(void)
{
    D_800D9370.field_26 = 2;
    D_800D9370.field_22 = 0;
    D_801B3244 = 0x40;
    D_801B3240++;
    func_800C0BC8();
}

void func_800C0BC8(void)
{
    func_8006CC4C(&D_800D9370, &D_801399B8);
    func_80066F9C(&D_800D9370, D_8011CF4C, 8, 8, 0);
    if (--D_801B3244 == 0)
    {
        D_801B3240++;
    }
}

void func_800C0C44(void)
{
    D_801B3240++;
}

void func_800C0C5C(VECTOR* translation, SVECTOR* rotation)
{
    MATRIX matrix;

    RotMatrix(rotation, &matrix);
    TransMatrix(&matrix, translation);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
}
