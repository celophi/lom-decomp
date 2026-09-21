#include "common.h"

extern u32 D_801B24C8;
extern s32 D_801B24CC;
extern void (*D_800D4E38[])(void);
extern void func_80071AB4(void);
extern u8 D_8011F538[];
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
s32 func_800719A8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B24C8 = 1;
        D_801B24CC = 1;
        return 1;
    }

    if (D_801B24C8 < 0x4)
    {
        D_800D4E38[D_801B24C8]();
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
void func_80071A20(void)
{
    D_801B24C8 = 1;
    D_801B24CC = 1;
}

void func_80071A38(void)
{
    D_801399AC = D_8011F538;
    D_800D9318[0x6] = 0xF;
    *(s16*)&D_800D9318[0x10] = -1;
    *(s16*)&D_800D9318[0x26] = 8;
    *(s16*)&D_800D9318[0x2] = 0;
    *(s16*)&D_800D9318[0xE] = 0;
    *(s16*)&D_800D9318[0x22] = 0x80;
    *(s16*)&D_800D9318[0x24] = 0;
    D_801B24CC = 0x7C;
    D_801B24C8 += 1;
    func_80071AB4();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80071AB4(void)
{
    func_8006CC4C(D_800D9318, D_801399A8);
    func_80066F9C(D_800D9318, D_8011CF4C, 0xB, 0x9, 0);
    if (--D_801B24CC == 0)
    {
        D_801B24C8 += 1;
    }
}
