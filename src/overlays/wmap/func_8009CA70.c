#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern u32 D_801B2CA0;
extern s32 D_801B2CA4;
extern void (*D_800D66A4[])(void);
extern void func_8009CB08(void);
extern u8 D_8011F538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009C9F8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2CA0 = 1;
        D_801B2CA4 = 1;
        return 1;
    }

    if (D_801B2CA0 < 0x6)
    {
        D_800D66A4[D_801B2CA0]();
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
void func_8009CA70(void)
{
    D_801B2CA0 = 1;
    D_801B2CA4 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_8009CA88(void)
{
    D_801399BC = D_8011F538;
    D_800D9370[0x6] = 0xF;
    *(s16*)&D_800D9370[0x10] = -1;
    *(s16*)&D_800D9370[0x26] = 4;
    *(s16*)&D_800D9370[0x22] = 0x81;
    *(s16*)&D_800D9370[0x2] = 0;
    *(s16*)&D_800D9370[0xE] = 0;
    *(s16*)&D_800D9370[0x24] = 1;
    D_801B2CA4 = 0x80;
    D_801B2CA0 += 1;
    func_8009CB08();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8009CB08(void)
{
    func_8006CC4C(D_800D9370, D_801399B8);
    func_80066F9C(D_800D9370, D_8011CF4C, 0x15, 0x5, 0);
    if (--D_801B2CA4 == 0)
    {
        D_801B2CA0 += 1;
    }
}
