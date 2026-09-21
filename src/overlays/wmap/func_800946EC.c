#include "common.h"

extern u32 D_801B2B38;
extern s32 D_801B2B3C;
extern void (*D_800D61C8[])(void);
extern void func_80094784(void);
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern void func_8006CC4C(u8* obj, u8* a1);
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80094674(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2B38 = 1;
        D_801B2B3C = 1;
        return 1;
    }

    if (D_801B2B38 < 0x6)
    {
        D_800D61C8[D_801B2B38]();
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
void func_800946EC(void)
{
    D_801B2B38 = 1;
    D_801B2B3C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80094704(void)
{
    D_801399AC = D_8011D538;
    D_800D9318[0x6] = 0xF;
    *(s16*)&D_800D9318[0xE] = 2;
    *(s16*)&D_800D9318[0x10] = -1;
    *(s16*)&D_800D9318[0x26] = 8;
    *(s16*)&D_800D9318[0x2] = 0;
    *(s16*)&D_800D9318[0x22] = 0x81;
    *(s16*)&D_800D9318[0x24] = 0x81;
    D_801B2B3C = 0x60;
    D_801B2B38 += 1;
    func_80094784();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80094784(void)
{
    func_8006CC4C(D_800D9318, D_801399A8);
    func_80066F9C(D_800D9318, D_8011CF4C, 0xF, 0x2, 0);
    if (--D_801B2B3C == 0)
    {
        D_801B2B38 += 1;
    }
}
