#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern u32 D_801B2A78;
extern s32 D_801B2A7C;
extern void (*D_800D5F50[])(void);
extern void func_80090588(void);
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80090478(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2A78 = 1;
        D_801B2A7C = 1;
        return 1;
    }

    if (D_801B2A78 < 0x4)
    {
        D_800D5F50[D_801B2A78]();
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
void func_800904F0(void)
{
    D_801B2A78 = 1;
    D_801B2A7C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80090508(void)
{
    D_801399AC = D_8011D538;
    D_800D9318[0x6] = 0xF;
    *(s16*)&D_800D9318[0x10] = -1;
    *(s16*)&D_800D9318[0x26] = 8;
    *(s16*)&D_800D9318[0x22] = 0x81;
    *(s16*)&D_800D9318[0x2] = 0;
    *(s16*)&D_800D9318[0xE] = 0;
    *(s16*)&D_800D9318[0x24] = 1;
    D_801B2A7C = 0x28;
    D_801B2A78 += 1;
    func_80090588();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80090588(void)
{
    func_8006CC4C(D_800D9318, D_801399A8);
    func_80066F9C(D_800D9318, D_8011CF4C, 0x13, 0x1E, 0);
    if (--D_801B2A7C == 0)
    {
        D_801B2A78 += 1;
    }
}
