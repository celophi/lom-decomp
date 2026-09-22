#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern u32 D_801B2B20;
extern s32 D_801B2B24;
extern void (*D_800D6138[])(void);
extern void func_800932EC(void);
extern u8 D_80123538[];
extern u8* D_801399D4;
extern u8 D_800D93F4[];
extern u8 D_801399D0[];
extern s32 D_8011CF4C;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800931DC(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2B20 = 1;
        D_801B2B24 = 1;
        return 1;
    }

    if (D_801B2B20 < 0x6)
    {
        D_800D6138[D_801B2B20]();
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
void func_80093254(void)
{
    D_801B2B20 = 1;
    D_801B2B24 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_8009326C(void)
{
    D_801399D4 = D_80123538;
    D_800D93F4[0x6] = 0xF;
    *(s16*)&D_800D93F4[0x10] = -1;
    *(s16*)&D_800D93F4[0x26] = 2;
    *(s16*)&D_800D93F4[0x22] = 0x81;
    *(s16*)&D_800D93F4[0x2] = 0;
    *(s16*)&D_800D93F4[0xE] = 0;
    *(s16*)&D_800D93F4[0x24] = 1;
    D_801B2B24 = 0x88;
    D_801B2B20 += 1;
    func_800932EC();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800932EC(void)
{
    func_8006CC4C(D_800D93F4, D_801399D0);
    func_80066F9C(D_800D93F4, D_8011CF4C, 0x8, 0x1E, 0);
    if (--D_801B2B24 == 0)
    {
        D_801B2B20 += 1;
    }
}
