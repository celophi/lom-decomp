#include "wmap_sequence_runtime.h"
#include "common.h"

extern u32 D_801B24E0;
extern s32 D_801B24E4;
extern void (*D_800D4E68[])(void);
extern void func_80071FA0(void);
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80071E94(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B24E0 = 1;
        D_801B24E4 = 1;
        return 1;
    }

    if (D_801B24E0 < 0x4)
    {
        D_800D4E68[D_801B24E0]();
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
void func_80071F0C(void)
{
    D_801B24E0 = 1;
    D_801B24E4 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80071F24(void)
{
    D_801399B4 = D_8011D538;
    D_800D9344[0x6] = 0xF;
    *(s16*)&D_800D9344[0x10] = -1;
    *(s16*)&D_800D9344[0x26] = 8;
    *(s16*)&D_800D9344[0x2] = 0;
    *(s16*)&D_800D9344[0xE] = 0;
    *(s16*)&D_800D9344[0x22] = 0x80;
    *(s16*)&D_800D9344[0x24] = 0;
    D_801B24E4 = 0x3D;
    D_801B24E0 += 1;
    func_80071FA0();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80071FA0(void)
{
    func_8006CC4C(D_800D9344, D_801399B0);
    func_80066F9C(D_800D9344, D_8011CF4C, 0x1C, 0x8, 0);
    if (--D_801B24E4 == 0)
    {
        D_801B24E0 += 1;
    }
}
