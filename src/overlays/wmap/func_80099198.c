#include "common.h"

extern u32 D_801B2C30;
extern s32 D_801B2C34;
extern void (*D_800D64C8[])(void);
extern void func_80099230(void);
extern u8 D_80121538[];
extern u8* D_801399DC;
extern u8 D_800D9420[];

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80099120(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2C30 = 1;
        D_801B2C34 = 1;
        return 1;
    }

    if (D_801B2C30 < 0x4)
    {
        D_800D64C8[D_801B2C30]();
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
void func_80099198(void)
{
    D_801B2C30 = 1;
    D_801B2C34 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_800991B0(void)
{
    D_801399DC = D_80121538;
    D_800D9420[0x6] = 0xF;
    *(s16*)&D_800D9420[0x10] = -1;
    *(s16*)&D_800D9420[0x26] = 4;
    *(s16*)&D_800D9420[0x22] = 0x81;
    *(s16*)&D_800D9420[0x2] = 0;
    *(s16*)&D_800D9420[0xE] = 0;
    *(s16*)&D_800D9420[0x24] = 1;
    D_801B2C34 = 0x3C;
    D_801B2C30 += 1;
    func_80099230();
}
