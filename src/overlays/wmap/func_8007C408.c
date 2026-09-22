#include "wmap_sequence_runtime.h"
#include "common.h"

extern u32 D_801B2700;
extern s32 D_801B2704;
extern void (*D_800D5440[])(void);
extern void func_8007C49C(void);
extern u8 D_8011D538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_8007C398(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2700 = 1;
        D_801B2704 = 1;
    }

    if (D_801B2700 < 0x6)
    {
        D_800D5440[D_801B2700]();
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
void func_8007C408(void)
{
    D_801B2700 = 1;
    D_801B2704 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_8007C420(void)
{
    D_801399BC = D_8011D538;
    D_800D9370[0x6] = 0xF;
    *(s16*)&D_800D9370[0x10] = -1;
    *(s16*)&D_800D9370[0x26] = 2;
    *(s16*)&D_800D9370[0x2] = 0;
    *(s16*)&D_800D9370[0xE] = 0;
    *(s16*)&D_800D9370[0x22] = 0x80;
    *(s16*)&D_800D9370[0x24] = 0x80;
    D_801B2704 = 0x56;
    D_801B2700 += 1;
    func_8007C49C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8007C49C(void)
{
    func_8006CC4C(D_800D9370, D_801399B8);
    func_80066F9C(D_800D9370, D_8011CF4C, 0x15, 0xB, 0);
    if (--D_801B2704 == 0)
    {
        D_801B2700 += 1;
    }
}
