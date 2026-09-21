#include "common.h"

extern u8* D_801399B4;
extern u8 D_8011F538[];
extern u8 D_800D9344[];
extern s32 D_801B2D60;
extern s32 D_801B2D64;
extern void func_800A1E6C(void);

/**
 * @brief Populate a world-map actor control block and schedule its next step.
 */
void func_800A1DF4(void)
{
    D_801399B4 = D_8011F538;
    D_800D9344[0x6] = 0xF;
    *(s16*)&D_800D9344[0x10] = -1;
    *(s16*)&D_800D9344[0x2] = 0;
    *(s16*)&D_800D9344[0xE] = 0;
    *(s16*)&D_800D9344[0x26] = 0;
    *(s16*)&D_800D9344[0x22] = 0x7F;
    *(s16*)&D_800D9344[0x24] = 0x7F;
    D_801B2D64 = 0x159;
    D_801B2D60 += 1;
    func_800A1E6C();
}
