#include "common.h"

typedef struct
{
    u8 pad00[0x24];
    s32 state_24;
    u8 pad28[0x24];
    s32 tail_state;
} WmapState;

typedef struct
{
    u8 pad00[2];
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
} WmapConfigA;

typedef struct
{
    u8 pad00[2];
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
} WmapConfigB;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern u8 D_801399B8;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_80066F9C(void*, s32, s32, s32, s32);
extern void func_8006CC4C(void*, void*);
extern void func_800C1290(void);
extern void func_800C1390(void);
extern void func_800C1488(void);
extern void func_800C1588(void);
extern void func_800C1680(void);
extern void func_800C17E0(void);
extern void func_800C1940(void);
extern void func_800C1AA0(void);
extern void func_800C1C00(void);
extern void func_800C1D60(void);
extern void func_800C1EC0(void);
extern void func_800C0CA4(void*);

void func_800C4388(void);
void func_800C451C(void);
void func_800C3504(void);
void func_800C35CC(void);

/**
 * @see decomp.me (100%)
 */
void func_800C3360(void)
{
    func_8006CC4C(&D_800D9370, &D_801399B8);
    func_80066F9C(&D_800D9370, D_8011CF4C, 0x19, 7, 0);
    if (--D_801B3274 == 0)
    {
        D_801B3270++;
    }
}

/**
 * @see decomp.me (100%)
 */
void func_800C33DC(void)
{
    D_801B3270++;
}

/**
 * @see decomp.me (100%)
 */
s32 func_800C33F4(s32 reset)
{
    if (reset != 0)
    {
        D_801B3278 = 1;
        D_801B327C = 1;
        return 1;
    }

    if ((u32)D_801B3278 >= 6)
    {
        return 0;
    }

    D_800D7BD4[D_801B3278]();
    return 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C346C(void)
{
    D_801B3278 = 1;
    D_801B327C = 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C3484(void)
{
    D_801399C4 = &D_8011F538;
    D_800D939C.field_06 = 0xF;
    D_800D939C.field_10 = -1;
    D_800D939C.field_26 = 2;
    D_800D939C.field_22 = 0x81;
    D_800D939C.field_02 = 0;
    D_800D939C.field_0E = 0;
    D_800D939C.field_24 = 1;
    D_801B327C = 0xF0;
    D_801B3278++;
    func_800C3504();
}

/**
 * @see decomp.me (100%)
 */
void func_800C3504(void)
{
    func_8006CC4C(&D_800D939C, &D_801399C0);
    func_80066F9C(&D_800D939C, D_8011CF4C, 0x19, 8, 0);
    if (--D_801B327C == 0)
    {
        D_801B3278++;
    }
}

/**
 * @see decomp.me (100%)
 */
void func_800C3580(void)
{
    D_800D939C.field_26 = 4;
    D_800D939C.field_22 = 0;
    D_801B327C = 0x20;
    D_801B3278++;
    func_800C35CC();
}

/**
 * @see decomp.me (100%)
 */
void func_800C35CC(void)
{
    func_8006CC4C(&D_800D939C, &D_801399C0);
    func_80066F9C(&D_800D939C, D_8011CF4C, 0x19, 8, 0);
    if (--D_801B327C == 0)
    {
        D_801B3278++;
    }
}

/**
 * @see decomp.me (100%)
 */
void func_800C3648(void)
{
    D_801B3278++;
}

/**
 * @see decomp.me (100%)
 */
s32 func_800C3660(s32 reset)
{
    if (reset != 0)
    {
        D_801B3280 = 1;
        D_801B3284 = 1;
        return 1;
    }

    if ((u32)D_801B3280 >= 6)
    {
        return 0;
    }

    D_800D7BEC[D_801B3280]();
    return 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C36D8(void)
{
    D_801B3280 = 1;
    D_801B3284 = 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C36F0(void)
{
    D_80182DF0 = 1;
    D_8013B238 = D_80139258;
    D_8013923C = 0;
    D_801B3284 = 0xF0;
    D_801B3280++;
    func_800C1290();
}

/**
 * @see decomp.me (100%)
 */
void func_800C376C(void)
{
    D_801B3284 = 0x20;
    D_801B3280++;
    func_800C1390();
}

/**
 * @see decomp.me (100%)
 */
void func_800C37A4(void)
{
    D_801B3280++;
}

/**
 * @see decomp.me (100%)
 */
s32 func_800C37BC(s32 reset)
{
    if (reset != 0)
    {
        D_801B3288 = 1;
        D_801B328C = 1;
        return 1;
    }

    if ((u32)D_801B3288 >= 6)
    {
        return 0;
    }

    D_800D7C04[D_801B3288]();
    return 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C3834(void)
{
    D_801B3288 = 1;
    D_801B328C = 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C384C(void)
{
    D_80182DF4 = 1;
    D_8013B240 = D_80139258;
    D_80139260 = 0;
    D_801B328C = 0xD0;
    D_801B3288++;
    func_800C1488();
}

/**
 * @see decomp.me (100%)
 */
void func_800C38C8(void)
{
    D_801B328C = 0x10;
    D_801B3288++;
    func_800C1588();
}

/**
 * @see decomp.me (100%)
 */
void func_800C3900(void)
{
    D_801B3288++;
}

/**
 * @see decomp.me (100%)
 */
s32 func_800C3918(s32 reset)
{
    if (reset != 0)
    {
        D_801B3290 = 1;
        D_801B3294 = 1;
        return 1;
    }

    if ((u32)D_801B3290 >= 4)
    {
        return 0;
    }

    D_800D7C1C[D_801B3290]();
    return 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C3990(void)
{
    D_801B3290 = 1;
    D_801B3294 = 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C39A8(void)
{
    D_801399CC = &D_8011D538;
    D_800D93C8.field_06 = 0xF;
    D_800D93C8.field_10 = -1;
    D_800D93C8.field_26 = 0x10;
    D_800D93C8.field_02 = 0;
    D_800D93C8.field_0E = 0;
    D_800D93C8.field_22 = 0x80;
    D_800D93C8.field_24 = 0;
    D_801AFC70.field_08 = 0x1964;
    D_801AFC70.field_02 = 0xBB8;
    D_801AFC70.field_04 = 0x64;
    D_801AFC70.field_0E = 0;
    D_801AFC70.field_0C = 0x5A;
    D_801B3294 = 0xBF;
    D_801B3290++;
    func_800C1680();
}

/**
 * @see decomp.me (100%)
 */
void func_800C3A50(void)
{
    D_801B3290++;
}

/**
 * @see decomp.me (100%)
 */
s32 func_800C3A68(s32 reset)
{
    if (reset != 0)
    {
        D_801B3298 = 1;
        D_801B329C = 1;
        return 1;
    }

    if ((u32)D_801B3298 >= 4)
    {
        return 0;
    }

    D_800D7C2C[D_801B3298]();
    return 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C3AE0(void)
{
    D_801B3298 = 1;
    D_801B329C = 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C3AF8(void)
{
    D_801399D4 = &D_8011D538;
    D_800D93F4.field_06 = 0xF;
    D_800D93F4.field_0E = 1;
    D_800D93F4.field_10 = -1;
    D_800D93F4.field_26 = 0x10;
    D_800D93F4.field_02 = 0;
    D_800D93F4.field_22 = 0x80;
    D_800D93F4.field_24 = 0;
    D_801AFC84.field_08 = 0x2710;
    D_801AFC84.field_02 = 0x200;
    D_801AFC84.field_04 = 0x64;
    D_801AFC84.field_0E = 0;
    D_801AFC84.field_0C = 0x3D;
    D_801B329C = 0xBF;
    D_801B3298++;
    func_800C17E0();
}

/**
 * @see decomp.me (100%)
 */
void func_800C3BA4(void)
{
    D_801B3298++;
}

/**
 * @see decomp.me (100%)
 */
s32 func_800C3BBC(s32 reset)
{
    if (reset != 0)
    {
        D_801B32A0 = 1;
        D_801B32A4 = 1;
        return 1;
    }

    if ((u32)D_801B32A0 >= 4)
    {
        return 0;
    }

    D_800D7C3C[D_801B32A0]();
    return 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C3C34(void)
{
    D_801B32A0 = 1;
    D_801B32A4 = 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C3C4C(void)
{
    D_801399DC = &D_8011D538;
    D_800D9420.field_06 = 0xF;
    D_800D9420.field_0E = 1;
    D_800D9420.field_10 = -1;
    D_800D9420.field_26 = 0x10;
    D_800D9420.field_02 = 0;
    D_800D9420.field_22 = 0x80;
    D_800D9420.field_24 = 0;
    D_801AFC98.field_08 = 0x2710;
    D_801AFC98.field_02 = 0x898;
    D_801AFC98.field_04 = 0x64;
    D_801AFC98.field_0E = 0;
    D_801AFC98.field_0C = 0x3D;
    D_801B32A4 = 0xBF;
    D_801B32A0++;
    func_800C1940();
}

/**
 * @see decomp.me (100%)
 */
void func_800C3CF8(void)
{
    D_801B32A0++;
}

/**
 * @see decomp.me (100%)
 */
s32 func_800C3D10(s32 reset)
{
    if (reset != 0)
    {
        D_801B32A8 = 1;
        D_801B32AC = 1;
        return 1;
    }

    if ((u32)D_801B32A8 >= 4)
    {
        return 0;
    }

    D_800D7C4C[D_801B32A8]();
    return 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C3D88(void)
{
    D_801B32A8 = 1;
    D_801B32AC = 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C3DA0(void)
{
    D_801399FC = &D_8011D538;
    D_800D94D0.field_06 = 0xF;
    D_800D94D0.field_0E = 2;
    D_800D94D0.field_10 = -1;
    D_800D94D0.field_26 = 0x10;
    D_800D94D0.field_02 = 0;
    D_800D94D0.field_22 = 0x80;
    D_800D94D0.field_24 = 0;
    D_801AFCE8.field_08 = 0x2710;
    D_801AFCE8.field_02 = 0x50;
    D_801AFCE8.field_04 = 0x64;
    D_801AFCE8.field_0E = 0;
    D_801AFCE8.field_0C = 0x26;
    D_801B32AC = 0xBF;
    D_801B32A8++;
    func_800C1AA0();
}

/**
 * @see decomp.me (100%)
 */
void func_800C3E4C(void)
{
    D_801B32A8++;
}

/**
 * @see decomp.me (100%)
 */
s32 func_800C3E64(s32 reset)
{
    if (reset != 0)
    {
        D_801B32B0 = 1;
        D_801B32B4 = 1;
        return 1;
    }

    if ((u32)D_801B32B0 >= 4)
    {
        return 0;
    }

    D_800D7C5C[D_801B32B0]();
    return 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C3EDC(void)
{
    D_801B32B0 = 1;
    D_801B32B4 = 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C3EF4(void)
{
    D_80139A04 = &D_8011D538;
    D_800D94FC.field_06 = 0xF;
    D_800D94FC.field_0E = 2;
    D_800D94FC.field_10 = -1;
    D_800D94FC.field_26 = 0x10;
    D_800D94FC.field_02 = 0;
    D_800D94FC.field_22 = 0x80;
    D_800D94FC.field_24 = 0;
    D_801AFCFC.field_08 = 0x1388;
    D_801AFCFC.field_02 = 0x6D6;
    D_801AFCFC.field_04 = 0x64;
    D_801AFCFC.field_0E = 0;
    D_801AFCFC.field_0C = 0x26;
    D_801B32B4 = 0xBF;
    D_801B32B0++;
    func_800C1C00();
}

/**
 * @see decomp.me (100%)
 */
void func_800C3FA0(void)
{
    D_801B32B0++;
}

/**
 * @see decomp.me (100%)
 */
s32 func_800C3FB8(s32 reset)
{
    if (reset != 0)
    {
        D_801B32B8 = 1;
        D_801B32BC = 1;
        return 1;
    }

    if ((u32)D_801B32B8 >= 4)
    {
        return 0;
    }

    D_800D7C6C[D_801B32B8]();
    return 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C4030(void)
{
    D_801B32B8 = 1;
    D_801B32BC = 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C4048(void)
{
    D_80139A0C = &D_8011D538;
    D_800D9528.field_06 = 0xF;
    D_800D9528.field_0E = 2;
    D_800D9528.field_10 = -1;
    D_800D9528.field_26 = 0x10;
    D_800D9528.field_02 = 0;
    D_800D9528.field_22 = 0x80;
    D_800D9528.field_24 = 0;
    D_801AFD10.field_08 = 0x2710;
    D_801AFD10.field_02 = 0xA8C;
    D_801AFD10.field_04 = 0x64;
    D_801AFD10.field_0E = 0;
    D_801AFD10.field_0C = 0x26;
    D_801B32BC = 0xBF;
    D_801B32B8++;
    func_800C1D60();
}

/**
 * @see decomp.me (100%)
 */
void func_800C40F4(void)
{
    D_801B32B8++;
}

/**
 * @see decomp.me (100%)
 */
s32 func_800C410C(s32 reset)
{
    if (reset != 0)
    {
        D_801B32C0 = 1;
        D_801B32C4 = 1;
        return 1;
    }

    if ((u32)D_801B32C0 >= 4)
    {
        return 0;
    }

    D_800D7C7C[D_801B32C0]();
    return 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C4184(void)
{
    D_801B32C0 = 1;
    D_801B32C4 = 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C419C(void)
{
    D_80139A14 = &D_8011D538;
    D_800D9554.field_06 = 0xF;
    D_800D9554.field_0E = 2;
    D_800D9554.field_10 = -1;
    D_800D9554.field_26 = 0x10;
    D_800D9554.field_02 = 0;
    D_800D9554.field_22 = 0x80;
    D_800D9554.field_24 = 0;
    D_801AFD24.field_08 = 0x2710;
    D_801AFD24.field_02 = 0x320;
    D_801AFD24.field_04 = 0x64;
    D_801AFD24.field_0E = 0;
    D_801AFD24.field_0C = 0x26;
    D_801B32C4 = 0xBF;
    D_801B32C0++;
    func_800C1EC0();
}

/**
 * @see decomp.me (100%)
 */
void func_800C4248(void)
{
    D_801B32C0++;
}

/**
 * @see decomp.me (100%)
 */
s32 func_800C4260(s32 reset)
{
    if (reset != 0)
    {
        D_801B32C8 = 1;
        D_801B32CC = 1;
        return 1;
    }

    if ((u32)D_801B32C8 >= 6)
    {
        return 0;
    }

    D_800D7C8C[D_801B32C8]();
    return 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C42D8(void)
{
    D_801B32C8 = 1;
    D_801B32CC = 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C42F0(void)
{
    func_800C0CA4(D_80139280);
    if (--D_801B32CC == 0)
    {
        D_801B32C8++;
    }
}

/**
 * @see decomp.me (100%)
 */
void func_800C4344(void)
{
    D_80139280->state_24 = 0;
    D_801B32CC = 0x10;
    D_801B32C8++;
    func_800C4388();
}

/**
 * @see decomp.me (100%)
 */
void func_800C4388(void)
{
    func_800C0CA4(D_80139280);
    if (--D_801B32CC == 0)
    {
        D_801B32C8++;
    }
}

/**
 * @see decomp.me (100%)
 */
void func_800C43DC(void)
{
    D_801B32C8++;
}

/**
 * @see decomp.me (100%)
 */
s32 func_800C43F4(s32 reset)
{
    if (reset != 0)
    {
        D_801B32D0 = 1;
        D_801B32D4 = 1;
        return 1;
    }

    if ((u32)D_801B32D0 >= 6)
    {
        return 0;
    }

    D_800D7CA4[D_801B32D0]();
    return 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C446C(void)
{
    D_801B32D0 = 1;
    D_801B32D4 = 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C4484(void)
{
    func_800C0CA4((u8*)D_80139280 + 0x28);
    if (--D_801B32D4 == 0)
    {
        D_801B32D0++;
    }
}

/**
 * @see decomp.me (100%)
 */
void func_800C44D8(void)
{
    D_80139280->tail_state = 0;
    D_801B32D4 = 0x20;
    D_801B32D0++;
    func_800C451C();
}

/**
 * @see decomp.me (100%)
 */
void func_800C451C(void)
{
    func_800C0CA4((u8*)D_80139280 + 0x28);
    if (--D_801B32D4 == 0)
    {
        D_801B32D0++;
    }
}

/**
 * @see decomp.me (100%)
 */
void func_800C4570(void)
{
    D_801B32D0++;
}
