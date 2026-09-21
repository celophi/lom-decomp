#include "common.h"

extern u32 D_801B2D98;
extern s32 D_801B2D9C;
extern void (*D_800D69E4[])(void);
extern void func_800A2890(void);
extern u8 D_80123538[];
extern u8* D_801399CC;
extern u8 D_800D93C8[];

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A2784(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2D98 = 1;
        D_801B2D9C = 1;
        return 1;
    }

    if (D_801B2D98 < 0x4)
    {
        D_800D69E4[D_801B2D98]();
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
void func_800A27FC(void)
{
    D_801B2D98 = 1;
    D_801B2D9C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_800A2814(void)
{
    D_801399CC = D_80123538;
    D_800D93C8[0x6] = 0xF;
    *(s16*)&D_800D93C8[0x10] = -1;
    *(s16*)&D_800D93C8[0x26] = 8;
    *(s16*)&D_800D93C8[0x2] = 0;
    *(s16*)&D_800D93C8[0xE] = 0;
    *(s16*)&D_800D93C8[0x22] = 0;
    *(s16*)&D_800D93C8[0x24] = 0x81;
    D_801B2D9C = 0x10;
    D_801B2D98 += 1;
    func_800A2890();
}
