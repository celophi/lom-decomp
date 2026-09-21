#include "common.h"

typedef struct
{
    u8 pad00[0x4C];
    s32 tail_state;
} WmapState;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7CA4[];
extern WmapState* D_80139280;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800C0CA4(void*);

void func_800C451C(void);

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
