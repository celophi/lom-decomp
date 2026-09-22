#include "common.h"

extern u8* D_801399B4;
extern u8 D_8011D538[];
extern u8 D_800D9344[];
extern u16 D_8011CF4C[];
extern s16 D_80182D60[];
extern s32 D_801B2E94;
extern s32 D_801B2E90;
extern void func_800ACAD0(void);

/**
 * @brief Spawn a world-map wandering actor and randomize its start position.
 */
void func_800AAD04(void)
{
    D_801399B4 = D_8011D538;
    D_800D9344[0x6] = 0xF;
    *(s16*)&D_800D9344[0x10] = -1;
    *(s16*)&D_800D9344[0x26] = 0x2;
    *(s16*)&D_800D9344[0x2] = 0;
    *(s16*)&D_800D9344[0xE] = 0;
    *(s16*)&D_800D9344[0x22] = 0x81;
    *(s16*)&D_800D9344[0x24] = 0x81;
    D_80182D60[0] = D_8011CF4C[0] + ((rand() * 50) >> 15) - 10;
    D_80182D60[1] = D_8011CF4C[1] + ((rand() * 30) >> 15) + 15;
    D_801B2E94 = 0x24;
    D_801B2E90 += 1;
    func_800ACAD0();
}
