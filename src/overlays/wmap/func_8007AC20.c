#include "common.h"

extern u32 D_801B26C8;
extern s32 D_801B26CC;
extern void (*D_800D5378[])(void);
extern void func_8007ACB4(void);
extern u8 D_8011D538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_8007ABB0(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B26C8 = 1;
        D_801B26CC = 1;
    }

    if (D_801B26C8 < 0x4)
    {
        D_800D5378[D_801B26C8]();
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
void func_8007AC20(void)
{
    D_801B26C8 = 1;
    D_801B26CC = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_8007AC38(void)
{
    D_801399BC = D_8011D538;
    D_800D9370[0x6] = 0xF;
    *(s16*)&D_800D9370[0xE] = 3;
    *(s16*)&D_800D9370[0x10] = -1;
    *(s16*)&D_800D9370[0x2] = 0;
    *(s16*)&D_800D9370[0x26] = 0;
    *(s16*)&D_800D9370[0x22] = 0x81;
    *(s16*)&D_800D9370[0x24] = 0x81;
    D_801B26CC = 0x4D;
    D_801B26C8 += 1;
    func_8007ACB4();
}
