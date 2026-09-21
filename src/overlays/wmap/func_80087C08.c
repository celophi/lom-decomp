#include "common.h"

extern u32 D_801B2918;
extern s32 D_801B291C;
extern void (*D_800D5AB8[])(void);
extern void func_80087C9C(void);
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80087B90(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2918 = 1;
        D_801B291C = 1;
        return 1;
    }

    if (D_801B2918 < 0x4)
    {
        D_800D5AB8[D_801B2918]();
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
void func_80087C08(void)
{
    D_801B2918 = 1;
    D_801B291C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80087C20(void)
{
    D_801399AC = D_8011D538;
    D_800D9318[0x6] = 0xF;
    *(s16*)&D_800D9318[0x10] = -1;
    *(s16*)&D_800D9318[0x26] = 2;
    *(s16*)&D_800D9318[0x2] = 0;
    *(s16*)&D_800D9318[0xE] = 0;
    *(s16*)&D_800D9318[0x22] = 0x80;
    *(s16*)&D_800D9318[0x24] = 0;
    D_801B291C = 0x7C;
    D_801B2918 += 1;
    func_80087C9C();
}
