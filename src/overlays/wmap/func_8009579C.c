#include "common.h"

extern u32 D_801B2B80;
extern s32 D_801B2B84;
extern void (*D_800D6290[])(void);
extern void func_80095834(void);
extern u8 D_8011D538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];
extern void func_8006CC4C(u8* obj, u8* a1);
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80095724(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2B80 = 1;
        D_801B2B84 = 1;
        return 1;
    }

    if (D_801B2B80 < 0x4)
    {
        D_800D6290[D_801B2B80]();
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
void func_8009579C(void)
{
    D_801B2B80 = 1;
    D_801B2B84 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_800957B4(void)
{
    D_801399BC = D_8011D538;
    D_800D9370[0x6] = 0xF;
    *(s16*)&D_800D9370[0xE] = 3;
    *(s16*)&D_800D9370[0x10] = -1;
    *(s16*)&D_800D9370[0x26] = 1;
    *(s16*)&D_800D9370[0x2] = 0;
    *(s16*)&D_800D9370[0x22] = 0x81;
    *(s16*)&D_800D9370[0x24] = 0x81;
    D_801B2B84 = 0x24;
    D_801B2B80 += 1;
    func_80095834();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80095834(void)
{
    func_8006CC4C(D_800D9370, D_801399B8);
    func_80066F9C(D_800D9370, D_8011CF4C, 0xF, 0x2, 0);
    if (--D_801B2B84 == 0)
    {
        D_801B2B80 += 1;
    }
}
