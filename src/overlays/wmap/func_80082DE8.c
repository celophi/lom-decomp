#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern u32 D_801B2828;
extern s32 D_801B282C;
extern void (*D_800D57D0[])(void);
extern void func_80082E80(void);
extern u8 D_80127538[];
extern u8* D_801399CC;
extern u8 D_800D93C8[];
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80082D70(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2828 = 1;
        D_801B282C = 1;
        return 1;
    }

    if (D_801B2828 < 0x6)
    {
        D_800D57D0[D_801B2828]();
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
void func_80082DE8(void)
{
    D_801B2828 = 1;
    D_801B282C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80082E00(void)
{
    D_801399CC = D_80127538;
    D_800D93C8[0x6] = 0xF;
    *(s16*)&D_800D93C8[0x10] = -1;
    *(s16*)&D_800D93C8[0x26] = 8;
    *(s16*)&D_800D93C8[0x22] = 0x81;
    *(s16*)&D_800D93C8[0x2] = 0;
    *(s16*)&D_800D93C8[0xE] = 0;
    *(s16*)&D_800D93C8[0x24] = 1;
    D_801B282C = 0x20;
    D_801B2828 += 1;
    func_80082E80();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80082E80(void)
{
    func_8006CC4C(D_800D93C8, D_801399C8);
    func_80066F9C(D_800D93C8, D_8011CF4C, 0x8, 0xA, 0);
    if (--D_801B282C == 0)
    {
        D_801B2828 += 1;
    }
}
