#include "wmap_sequence_runtime.h"
#include "common.h"

extern u32 D_801B2BF0;
extern s32 D_801B2BF4;
extern void (*D_800D6430[])(void);
extern void func_800984F8(void);
extern u8 D_8011F538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800983E8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2BF0 = 1;
        D_801B2BF4 = 1;
        return 1;
    }

    if (D_801B2BF0 < 0x4)
    {
        D_800D6430[D_801B2BF0]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80098460(void)
{
    D_801B2BF0 = 1;
    D_801B2BF4 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80098478(void)
{
    D_801399AC = D_8011F538;
    D_800D9318[0x6] = 0xF;
    *(s16*)&D_800D9318[0x10] = -1;
    *(s16*)&D_800D9318[0x26] = 0x10;
    *(s16*)&D_800D9318[0x22] = 0x81;
    *(s16*)&D_800D9318[0x2] = 0;
    *(s16*)&D_800D9318[0xE] = 0;
    *(s16*)&D_800D9318[0x24] = 1;
    D_801B2BF4 = 0x40;
    D_801B2BF0 += 1;
    func_800984F8();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800984F8(void)
{
    func_8006CC4C(D_800D9318, D_801399A8);
    func_80066F9C(D_800D9318, D_8011CF4C, 0x13, 0x2, 0);
    if (--D_801B2BF4 == 0)
    {
        D_801B2BF0 += 1;
    }
}
