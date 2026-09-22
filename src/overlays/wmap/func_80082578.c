#include "wmap_sequence_runtime.h"
#include "common.h"

extern u32 D_801B2800;
extern s32 D_801B2804;
extern void (*D_800D5778[])(void);
extern void func_8008260C(void);
extern u8 D_80121538[];
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
s32 func_80082500(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2800 = 1;
        D_801B2804 = 1;
        return 1;
    }

    if (D_801B2800 < 0x4)
    {
        D_800D5778[D_801B2800]();
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
void func_80082578(void)
{
    D_801B2800 = 1;
    D_801B2804 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80082590(void)
{
    D_801399B4 = D_80121538;
    D_800D9344[0x6] = 0xF;
    *(s16*)&D_800D9344[0x10] = -1;
    *(s16*)&D_800D9344[0x26] = 8;
    *(s16*)&D_800D9344[0x2] = 0;
    *(s16*)&D_800D9344[0xE] = 0;
    *(s16*)&D_800D9344[0x22] = 0x80;
    *(s16*)&D_800D9344[0x24] = 0x80;
    D_801B2804 = 0x5D;
    D_801B2800 += 1;
    func_8008260C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8008260C(void)
{
    func_8006CC4C(D_800D9344, D_801399B0);
    func_80066F9C(D_800D9344, D_8011CF4C, 0x18, 0x33, 0);
    if (--D_801B2804 == 0)
    {
        D_801B2800 += 1;
    }
}
