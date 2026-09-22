#include "wmap_sequence_runtime.h"
#include "common.h"

extern void func_800994B8(void);
extern s16 D_800D944C[];
extern s32 D_801B2C3C;
extern s32 D_801B2C38;
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_801399E0[];
extern s32 D_80182D60;

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800993F0(void)
{
    func_8006CC4C(D_800D944C, D_801399E0);
    func_80066F9C(D_800D944C, D_80182D60, 0x13, 0x2, 0);
    if (--D_801B2C3C == 0)
    {
        D_801B2C38 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_8009946C(void)
{
    D_800D944C[17] = 0;
    D_800D944C[19] = 2;
    D_801B2C3C = 0x40;
    D_801B2C38 += 1;
    func_800994B8();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800994B8(void)
{
    func_8006CC4C(D_800D944C, D_801399E0);
    func_80066F9C(D_800D944C, D_80182D60, 0x13, 0x2, 0);
    if (--D_801B2C3C == 0)
    {
        D_801B2C38 += 1;
    }
}
