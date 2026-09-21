#include "common.h"

extern u32 D_801B2FA0;
extern s32 D_801B2FA4;
extern void (*D_800D7274[])(void);
extern void func_800B2908(void);
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800B27F8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2FA0 = 1;
        D_801B2FA4 = 1;
        return 1;
    }

    if (D_801B2FA0 < 0x4)
    {
        D_800D7274[D_801B2FA0]();
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
void func_800B2870(void)
{
    D_801B2FA0 = 1;
    D_801B2FA4 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_800B2888(void)
{
    D_801399AC = D_8011D538;
    D_800D9318[0x6] = 0xF;
    *(s16*)&D_800D9318[0x10] = -1;
    *(s16*)&D_800D9318[0x26] = 0x10;
    *(s16*)&D_800D9318[0x22] = 1;
    *(s16*)&D_800D9318[0x2] = 0;
    *(s16*)&D_800D9318[0xE] = 0;
    *(s16*)&D_800D9318[0x24] = 0x81;
    D_801B2FA4 = 0x10;
    D_801B2FA0 += 1;
    func_800B2908();
}
