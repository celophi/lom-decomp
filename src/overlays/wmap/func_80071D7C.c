#include "common.h"

extern u8* D_801399C4;
extern u8 D_8011F538[];
extern u8 D_800D939C[];
extern s32 D_801B24D8;
extern s32 D_801B24DC;
extern void func_80071E00(void);

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80071D7C(void)
{
    D_801399C4 = D_8011F538;
    D_800D939C[0x6] = 0xF;
    *(s16*)&D_800D939C[0xE] = 2;
    *(s16*)&D_800D939C[0x10] = -1;
    *(s16*)&D_800D939C[0x26] = 0x10;
    *(s16*)&D_800D939C[0x22] = 0x81;
    *(s16*)&D_800D939C[0x2] = 0;
    *(s16*)&D_800D939C[0x24] = 1;
    D_801B24DC = 0x28;
    D_801B24D8 += 1;
    func_80071E00();
}
