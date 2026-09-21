#include "common.h"
#include "sdk/libgte.h"

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

extern s32 D_800DBE70;
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
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
extern WmapConfigA D_800D9344;
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
extern s32 D_8011D500;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B20C;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern s32 D_801ADAF4;
extern VECTOR D_80182DC0;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
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
extern void func_8006683C(s32);
extern void func_8006CAC0(s32 (*callback)(s32));
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
extern void func_800C10B8(void);
extern void func_800C11A4(void);

void func_800C4388(void);
void func_800C451C(void);
s32 func_800C410C(s32);
s32 func_800C4260(s32);
s32 func_800C3A68(s32);
s32 func_800C3BBC(s32);
s32 func_800C3D10(s32);
s32 func_800C3E64(s32);
s32 func_800C3FB8(s32);
s32 func_800C3918(s32);
s32 func_800C43F4(s32);
s32 func_800C37BC(s32);
s32 func_800C2CD0(s32);
s32 func_800C2DF8(s32);
void func_800C302C(void);
void func_800C30F4(void);
void func_800C3298(void);
void func_800C3360(void);
void func_800C3504(void);
void func_800C35CC(void);

void func_800C2630(void)
{
    if (--D_801B3254 == 0)
    {
        D_801B3250++;
    }
}

void func_800C2664(void)
{
    func_8006CAC0(func_800C2CD0);
    D_800DBE70 = 0;
    D_801ADAF4 = 3;
    func_8006683C(0x801530);
    D_801B3254 = 0x64;
    D_801B3250++;
}

void func_800C26C0(void)
{
    if (--D_801B3254 == 0)
    {
        D_801B3250++;
    }
}

void func_800C26F4(void)
{
    func_8006CAC0(func_800C3918);
    D_801B3254 = 0x64;
    D_801B3250++;
}

void func_800C2730(void)
{
    if (--D_801B3254 == 0)
    {
        D_801B3250++;
    }
}

void func_800C2764(void)
{
    func_8006CAC0(func_800C3A68);
    D_801B3254 = 0x14;
    D_801B3250++;
}

void func_800C27A0(void)
{
    if (--D_801B3254 == 0)
    {
        D_801B3250++;
    }
}

void func_800C27D4(void)
{
    func_8006CAC0(func_800C3BBC);
    D_801B3254 = 0xF;
    D_801B3250++;
}

void func_800C2810(void)
{
    if (--D_801B3254 == 0)
    {
        D_801B3250++;
    }
}

void func_800C2844(void)
{
    func_8006CAC0(func_800C3D10);
    D_801B3254 = 0xF;
    D_801B3250++;
}

void func_800C2880(void)
{
    if (--D_801B3254 == 0)
    {
        D_801B3250++;
    }
}

void func_800C28B4(void)
{
    func_8006CAC0(func_800C3E64);
    D_801B3254 = 0xA;
    D_801B3250++;
}

void func_800C28F0(void)
{
    if (--D_801B3254 == 0)
    {
        D_801B3250++;
    }
}

void func_800C2924(void)
{
    func_8006CAC0(func_800C3FB8);
    D_801B3254 = 5;
    D_801B3250++;
}

void func_800C2960(void)
{
    if (--D_801B3254 == 0)
    {
        D_801B3250++;
    }
}

void func_800C2994(void)
{
    func_8006CAC0(func_800C410C);
    D_801ADAF4 = 0;
    D_801B3254 = 8;
    D_801B3250++;
}

void func_800C29D8(void)
{
    if (--D_801B3254 == 0)
    {
        D_801B3250++;
    }
}

void func_800C2A0C(void)
{
    func_8006CAC0(func_800C4260);
    D_801B3254 = 0x90;
    D_801B3250++;
}

void func_800C2A48(void)
{
    if (--D_801B3254 == 0)
    {
        D_801B3250++;
    }
}

void func_800C2A7C(void)
{
    if (--D_801B3254 == 0)
    {
        D_801B3250++;
    }
}

void func_800C2AB0(void)
{
    func_8006CAC0(func_800C2DF8);
    D_80139244 = 1;
    D_80139978 = -1;
    D_8013986C = -1;
    D_801B3254 = 2;
    D_801B3250++;
}

void func_800C2B0C(void)
{
    if (--D_801B3254 == 0)
    {
        D_801B3250++;
    }
}

void func_800C2B40(void)
{
    func_8006CAC0(func_800C43F4);
    D_801B3254 = 0xD;
    D_801B3250++;
}

void func_800C2B7C(void)
{
    if (--D_801B3254 == 0)
    {
        D_801B3250++;
    }
}

void func_800C2BB0(void)
{
    if (--D_801B3254 == 0)
    {
        D_801B3250++;
    }
}

void func_800C2BE4(void)
{
    func_8006CAC0(func_800C37BC);
    D_801B3254 = 0x7C;
    D_801B3250++;
}

void func_800C2C20(void)
{
    if (--D_801B3254 == 0)
    {
        D_801B3250++;
    }
}

void func_800C2C54(void)
{
    D_8011D500 = 0xFE;
    D_801B3254 = 0x80;
    D_801B3250++;
}

void func_800C2C80(void)
{
    if (--D_801B3254 == 0)
    {
        D_801B3250++;
    }
}

void func_800C2CB4(void)
{
    D_8013B20C = 0;
    D_801B3250++;
}

s32 func_800C2CD0(s32 reset)
{
    if (reset != 0)
    {
        D_801B3258 = 1;
        D_801B325C = 1;
        return 1;
    }

    if ((u32)D_801B3258 >= 4)
    {
        return 0;
    }

    D_800D7B84[D_801B3258]();
    return 1;
}

void func_800C2D48(void)
{
    D_801B3258 = 1;
    D_801B325C = 1;
}

void func_800C2D60(void)
{
    D_801B2650 = D_80182DC0;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    D_801B325C = 0x20;
    D_801B3258++;
    func_800C10B8();
}

void func_800C2DE0(void)
{
    D_801B3258++;
}

s32 func_800C2DF8(s32 reset)
{
    if (reset != 0)
    {
        D_801B3260 = 1;
        D_801B3264 = 1;
        return 1;
    }

    if ((u32)D_801B3260 >= 4)
    {
        return 0;
    }

    D_800D7B94[D_801B3260]();
    return 1;
}

void func_800C2E70(void)
{
    D_801B3260 = 1;
    D_801B3264 = 1;
}

void func_800C2E88(void)
{
    D_801B2478 = D_80182DC0;
    D_80182DEC = 0x80;
    D_801B2478.vz = 0xAFC8;
    D_801B3264 = 0x80;
    D_801B3260++;
    func_800C11A4();
}

void func_800C2F04(void)
{
    D_801B3260++;
}

s32 func_800C2F1C(s32 reset)
{
    if (reset != 0)
    {
        D_801B3268 = 1;
        D_801B326C = 1;
        return 1;
    }

    if ((u32)D_801B3268 >= 6)
    {
        return 0;
    }

    D_800D7BA4[D_801B3268]();
    return 1;
}

void func_800C2F94(void)
{
    D_801B3268 = 1;
    D_801B326C = 1;
}

void func_800C2FAC(void)
{
    D_801399B4 = &D_80121538;
    D_800D9344.field_06 = 0xF;
    D_800D9344.field_10 = -1;
    D_800D9344.field_26 = 2;
    D_800D9344.field_22 = 0x81;
    D_800D9344.field_02 = 0;
    D_800D9344.field_0E = 0;
    D_800D9344.field_24 = 1;
    D_801B326C = 0x1F6;
    D_801B3268++;
    func_800C302C();
}

void func_800C302C(void)
{
    func_8006CC4C(&D_800D9344, &D_801399B0);
    func_80066F9C(&D_800D9344, D_8011CF4C, 8, 2, 0);
    if (--D_801B326C == 0)
    {
        D_801B3268++;
    }
}

void func_800C30A8(void)
{
    D_800D9344.field_26 = 8;
    D_800D9344.field_22 = 0;
    D_801B326C = 0x10;
    D_801B3268++;
    func_800C30F4();
}

void func_800C30F4(void)
{
    func_8006CC4C(&D_800D9344, &D_801399B0);
    func_80066F9C(&D_800D9344, D_8011CF4C, 8, 2, 0);
    if (--D_801B326C == 0)
    {
        D_801B3268++;
    }
}

void func_800C3170(void)
{
    D_801B3268++;
}

s32 func_800C3188(s32 reset)
{
    if (reset != 0)
    {
        D_801B3270 = 1;
        D_801B3274 = 1;
        return 1;
    }

    if ((u32)D_801B3270 >= 6)
    {
        return 0;
    }

    D_800D7BBC[D_801B3270]();
    return 1;
}

void func_800C3200(void)
{
    D_801B3270 = 1;
    D_801B3274 = 1;
}

void func_800C3218(void)
{
    s32 value;

    D_801399BC = &D_8011F538;
    D_800D9370.field_06 = 0xF;
    D_800D9370.field_0E = value = 1;
    D_800D9370.field_10 = -value;
    D_800D9370.field_26 = 0x80;
    D_800D9370.field_24 = value;
    D_800D9370.field_02 = 0;
    D_800D9370.field_22 = 0x81;
    D_801B3274 = 0x118;
    D_801B3270++;
    func_800C3298();
}

void func_800C3298(void)
{
    func_8006CC4C(&D_800D9370, &D_801399B8);
    func_80066F9C(&D_800D9370, D_8011CF4C, 0x19, 7, 0);
    if (--D_801B3274 == 0)
    {
        D_801B3270++;
    }
}

void func_800C3314(void)
{
    D_800D9370.field_26 = 0x80;
    D_800D9370.field_22 = 0;
    D_801B3274 = 1;
    D_801B3270++;
    func_800C3360();
}

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
