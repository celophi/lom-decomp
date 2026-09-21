#include "common.h"

extern u32 D_801B2F88;
extern s32 D_801B2F8C;
extern void (*D_800D71EC[])(void);
extern void func_800B15E8(void);
extern u8 D_8011F538[];
extern u8* D_801399CC;
extern u8 D_800D93C8[];
extern void func_8006CC4C(u8* obj, u8* a1);
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800B14D8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2F88 = 1;
        D_801B2F8C = 1;
        return 1;
    }

    if (D_801B2F88 < 0x6)
    {
        D_800D71EC[D_801B2F88]();
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
void func_800B1550(void)
{
    D_801B2F88 = 1;
    D_801B2F8C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_800B1568(void)
{
    D_801399CC = D_8011F538;
    D_800D93C8[0x6] = 0xF;
    *(s16*)&D_800D93C8[0x10] = -1;
    *(s16*)&D_800D93C8[0x26] = 4;
    *(s16*)&D_800D93C8[0x22] = 0x81;
    *(s16*)&D_800D93C8[0x2] = 0;
    *(s16*)&D_800D93C8[0xE] = 0;
    *(s16*)&D_800D93C8[0x24] = 1;
    D_801B2F8C = 0x96;
    D_801B2F88 += 1;
    func_800B15E8();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800B15E8(void)
{
    func_8006CC4C(D_800D93C8, D_801399C8);
    func_80066F9C(D_800D93C8, D_8011CF4C, 0x11, 0x4, 0);
    if (--D_801B2F8C == 0)
    {
        D_801B2F88 += 1;
    }
}
