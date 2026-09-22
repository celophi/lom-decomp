#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern u32 D_801B2718;
extern s32 D_801B271C;
extern void (*D_800D5478[])(void);
extern void func_8007C9A4(void);
extern u8 D_8011D538[];
extern u8* D_801399CC;
extern u8 D_800D93C8[];
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8007C898(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2718 = 1;
        D_801B271C = 1;
        return 1;
    }

    if (D_801B2718 < 0x6)
    {
        D_800D5478[D_801B2718]();
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
void func_8007C910(void)
{
    D_801B2718 = 1;
    D_801B271C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_8007C928(void)
{
    D_801399CC = D_8011D538;
    D_800D93C8[0x6] = 0xF;
    *(s16*)&D_800D93C8[0xE] = 1;
    *(s16*)&D_800D93C8[0x10] = -1;
    *(s16*)&D_800D93C8[0x2] = 0;
    *(s16*)&D_800D93C8[0x26] = 0;
    *(s16*)&D_800D93C8[0x22] = 0x80;
    *(s16*)&D_800D93C8[0x24] = 0x80;
    D_801B271C = 0x10;
    D_801B2718 += 1;
    func_8007C9A4();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8007C9A4(void)
{
    func_8006CC4C(D_800D93C8, D_801399C8);
    func_80066F9C(D_800D93C8, D_8011CF4C, 0x15, 0x5, 0);
    if (--D_801B271C == 0)
    {
        D_801B2718 += 1;
    }
}
