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

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern u8 D_8011D538;
extern WmapState* D_80139280;
extern void* D_80139A0C;
extern void* D_80139A14;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern s32 D_801B32B0;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800C1D60(void);
extern void func_800C1EC0(void);
extern void func_800C0CA4(void*);

void func_800C4388(void);
void func_800C451C(void);

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
