#include "common.h"

extern u8* D_801399BC;
extern u8 D_8011D538[];
extern u8 D_800D9370[];
extern s32 D_801B2FB0;
extern s32 D_801B2FB4;
extern void func_800B2C54(void);

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_800B2BD0(void)
{
    D_801399BC = D_8011D538;
    D_800D9370[0x6] = 0xF;
    *(s16*)&D_800D9370[0xE] = 2;
    *(s16*)&D_800D9370[0x10] = -1;
    *(s16*)&D_800D9370[0x26] = 0x10;
    *(s16*)&D_800D9370[0x22] = 1;
    *(s16*)&D_800D9370[0x2] = 0;
    *(s16*)&D_800D9370[0x24] = 0x81;
    D_801B2FB4 = 0x10;
    D_801B2FB0 += 1;
    func_800B2C54();
}
