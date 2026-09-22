#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern u32 D_801B2D80;
extern s32 D_801B2D84;
extern void (*D_800D69AC[])(void);
extern void func_800A2414(void);
extern u8 D_80121538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A2308(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2D80 = 1;
        D_801B2D84 = 1;
        return 1;
    }

    if (D_801B2D80 < 0x4)
    {
        D_800D69AC[D_801B2D80]();
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
void func_800A2380(void)
{
    D_801B2D80 = 1;
    D_801B2D84 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_800A2398(void)
{
    D_801399BC = D_80121538;
    D_800D9370[0x6] = 0xF;
    *(s16*)&D_800D9370[0x10] = -1;
    *(s16*)&D_800D9370[0x26] = 0x10;
    *(s16*)&D_800D9370[0x2] = 0;
    *(s16*)&D_800D9370[0xE] = 0;
    *(s16*)&D_800D9370[0x22] = 0x80;
    *(s16*)&D_800D9370[0x24] = 0;
    D_801B2D84 = 0x8C;
    D_801B2D80 += 1;
    func_800A2414();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800A2414(void)
{
    func_8006CC4C(D_800D9370, D_801399B8);
    func_80066F9C(D_800D9370, D_8011CF4C, 0x16, 0xA, 0);
    if (--D_801B2D84 == 0)
    {
        D_801B2D80 += 1;
    }
}
