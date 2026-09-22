#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern u32 D_801B2728;
extern s32 D_801B272C;
extern void (*D_800D54A8[])(void);
extern void func_8007CDDC(void);
extern u8 D_8011D538[];
extern u8* D_801399D4;
extern u8 D_800D93F4[];
extern u8 D_801399D0[];
extern s32 D_8011CF4C;

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_8007CCD8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2728 = 1;
        D_801B272C = 1;
    }

    if (D_801B2728 < 0x6)
    {
        D_800D54A8[D_801B2728]();
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
void func_8007CD48(void)
{
    D_801B2728 = 1;
    D_801B272C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_8007CD60(void)
{
    D_801399D4 = D_8011D538;
    D_800D93F4[0x6] = 0xF;
    *(s16*)&D_800D93F4[0xE] = 3;
    *(s16*)&D_800D93F4[0x10] = -1;
    *(s16*)&D_800D93F4[0x2] = 0;
    *(s16*)&D_800D93F4[0x26] = 0;
    *(s16*)&D_800D93F4[0x22] = 0x80;
    *(s16*)&D_800D93F4[0x24] = 0x80;
    D_801B272C = 0x50;
    D_801B2728 += 1;
    func_8007CDDC();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8007CDDC(void)
{
    func_8006CC4C(D_800D93F4, D_801399D0);
    func_80066F9C(D_800D93F4, D_8011CF4C, 0x15, 0xB, 0);
    if (--D_801B272C == 0)
    {
        D_801B2728 += 1;
    }
}
