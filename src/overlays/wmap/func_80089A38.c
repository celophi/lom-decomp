#include "wmap_sequence_runtime.h"
#include "common.h"

extern void func_80089A84(void);
extern s16 D_800D9268[];
extern s32 D_801B296C;
extern s32 D_801B2968;
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_80182D58;

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800899BC(void)
{
    func_8006CC4C(D_800D9318, D_801399A8);
    func_80066F9C(D_800D9318, D_80182D58, 0xF, 0x9, 0);
    if (--D_801B296C == 0)
    {
        D_801B2968 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_80089A38(void)
{
    D_800D9268[107] = 8;
    D_800D9268[105] = 0;
    D_801B296C = 0x10;
    D_801B2968 += 1;
    func_80089A84();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80089A84(void)
{
    func_8006CC4C(D_800D9318, D_801399A8);
    func_80066F9C(D_800D9318, D_80182D58, 0xF, 0x9, 0);
    if (--D_801B296C == 0)
    {
        D_801B2968 += 1;
    }
}
