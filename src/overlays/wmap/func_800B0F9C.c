#include "common.h"

extern u32 D_801B2F78;
extern s32 D_801B2F7C;
extern void (*D_800D71B4[])(void);
extern void func_800B1034(void);
extern u8 D_80121538[];
extern u8* D_801399D4;
extern u8 D_800D93F4[];

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800B0F24(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2F78 = 1;
        D_801B2F7C = 1;
        return 1;
    }

    if (D_801B2F78 < 0x6)
    {
        D_800D71B4[D_801B2F78]();
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
void func_800B0F9C(void)
{
    D_801B2F78 = 1;
    D_801B2F7C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_800B0FB4(void)
{
    D_801399D4 = D_80121538;
    D_800D93F4[0x6] = 0xF;
    *(s16*)&D_800D93F4[0x10] = -1;
    *(s16*)&D_800D93F4[0x26] = 4;
    *(s16*)&D_800D93F4[0x22] = 0x81;
    *(s16*)&D_800D93F4[0x2] = 0;
    *(s16*)&D_800D93F4[0xE] = 0;
    *(s16*)&D_800D93F4[0x24] = 1;
    D_801B2F7C = 0x78;
    D_801B2F78 += 1;
    func_800B1034();
}
