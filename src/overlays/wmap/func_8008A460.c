#include "wmap_sequence_runtime.h"
#include "common.h"

extern void func_8008A4AC(void);
extern s16 D_800D9268[];
extern s32 D_801B2994;
extern s32 D_801B2990;
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_80182D58;

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8008A3E4(void)
{
    func_8006CC4C(D_800D9370, D_801399B8);
    func_80066F9C(D_800D9370, D_80182D58, 0xF, 0x2, 0);
    if (--D_801B2994 == 0)
    {
        D_801B2990 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_8008A460(void)
{
    D_800D9268[151] = 8;
    D_800D9268[149] = 0;
    D_801B2994 = 0x10;
    D_801B2990 += 1;
    func_8008A4AC();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8008A4AC(void)
{
    func_8006CC4C(D_800D9370, D_801399B8);
    func_80066F9C(D_800D9370, D_80182D58, 0xF, 0x2, 0);
    if (--D_801B2994 == 0)
    {
        D_801B2990 += 1;
    }
}
