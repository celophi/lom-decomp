#include "common.h"

extern u32 D_801B24D0;
extern s32 D_801B24D4;
extern void (*D_800D4E48[])(void);
extern void func_80071C58(void);
extern u8 D_8011F538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];
extern void func_8006CC4C(u8* obj, u8* a1);
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80071B48(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B24D0 = 1;
        D_801B24D4 = 1;
        return 1;
    }

    if (D_801B24D0 < 0x4)
    {
        D_800D4E48[D_801B24D0]();
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
void func_80071BC0(void)
{
    D_801B24D0 = 1;
    D_801B24D4 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80071BD8(void)
{
    s32 one = 1;

    D_801399BC = D_8011F538;
    D_800D9370[0x6] = 0xF;
    *(s16*)&D_800D9370[0xE] = 1;
    *(s16*)&D_800D9370[0x10] = -one;
    *(s16*)&D_800D9370[0x26] = 2;
    *(s16*)&D_800D9370[0x2] = 0;
    *(s16*)&D_800D9370[0x22] = 0x81;
    *(s16*)&D_800D9370[0x24] = 1;
    D_801B24D4 = 0x7C;
    D_801B24D0 += one;
    func_80071C58();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80071C58(void)
{
    func_8006CC4C(D_800D9370, D_801399B8);
    func_80066F9C(D_800D9370, D_8011CF4C, 0xB, 0xA, 0);
    if (--D_801B24D4 == 0)
    {
        D_801B24D0 += 1;
    }
}
