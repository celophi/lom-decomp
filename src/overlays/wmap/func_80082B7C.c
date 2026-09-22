#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern u32 D_801B2820;
extern s32 D_801B2824;
extern void (*D_800D57B8[])(void);
extern void func_80082C14(void);
extern u8 D_8011F538[];
extern u8* D_801399C4;
extern u8 D_800D939C[];
extern u8 D_801399C0[];
extern s32 D_8011CF4C;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80082B04(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2820 = 1;
        D_801B2824 = 1;
        return 1;
    }

    if (D_801B2820 < 0x6)
    {
        D_800D57B8[D_801B2820]();
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
void func_80082B7C(void)
{
    D_801B2820 = 1;
    D_801B2824 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80082B94(void)
{
    D_801399C4 = D_8011F538;
    D_800D939C[0x6] = 0xF;
    *(s16*)&D_800D939C[0x10] = -1;
    *(s16*)&D_800D939C[0x26] = 2;
    *(s16*)&D_800D939C[0x22] = 0x81;
    *(s16*)&D_800D939C[0x2] = 0;
    *(s16*)&D_800D939C[0xE] = 0;
    *(s16*)&D_800D939C[0x24] = 1;
    D_801B2824 = 0x30;
    D_801B2820 += 1;
    func_80082C14();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80082C14(void)
{
    func_8006CC4C(D_800D939C, D_801399C0);
    func_80066F9C(D_800D939C, D_8011CF4C, 0x8, 0xA, 0);
    if (--D_801B2824 == 0)
    {
        D_801B2820 += 1;
    }
}
