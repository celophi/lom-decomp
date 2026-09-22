#include "common.h"

extern u8* D_801399B4;
extern u8 D_8011D538[];
extern u8 D_800D9344[];
extern s32 D_801B2690;
extern s32 D_801B2694;
extern void func_8007A09C(void);

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_8007A018(void)
{
    D_801399B4 = D_8011D538;
    D_800D9344[0x6] = 0xF;
    *(s16*)&D_800D9344[0xE] = 2;
    *(s16*)&D_800D9344[0x10] = -1;
    *(s16*)&D_800D9344[0x26] = 0x10;
    *(s16*)&D_800D9344[0x22] = 0x81;
    *(s16*)&D_800D9344[0x2] = 0;
    *(s16*)&D_800D9344[0x24] = 1;
    D_801B2694 = 0x10;
    D_801B2690 += 1;
    func_8007A09C();
}
