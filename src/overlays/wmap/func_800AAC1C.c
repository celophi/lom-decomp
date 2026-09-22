#include "common.h"

extern u8* D_801399AC;
extern u8 D_8011D538[];
extern u8 D_800D9318[];
extern u16 D_8011CF4C[];
extern s16 D_80182D58[];
extern s32 D_801B2E8C;
extern s32 D_801B2E88;
extern void func_800AC9AC(void);

/**
 * @brief Spawn a world-map wandering actor and randomize its start position.
 */
void func_800AAC1C(void)
{
    D_801399AC = D_8011D538;
    D_800D9318[0x6] = 0xF;
    *(s16*)&D_800D9318[0x10] = -1;
    *(s16*)&D_800D9318[0x26] = 0x2;
    *(s16*)&D_800D9318[0x2] = 0;
    *(s16*)&D_800D9318[0xE] = 0;
    *(s16*)&D_800D9318[0x22] = 0x81;
    *(s16*)&D_800D9318[0x24] = 0x81;
    D_80182D58[0] = D_8011CF4C[0] + ((rand() * 50) >> 15) - 10;
    D_80182D58[1] = D_8011CF4C[1] + ((rand() * 30) >> 15) + 15;
    D_801B2E8C = 0x24;
    D_801B2E88 += 1;
    func_800AC9AC();
}
