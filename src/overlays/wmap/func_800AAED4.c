#include "common.h"

extern u8* D_801399D4;
extern u8 D_8011F538[];
extern u8 D_800D93F4[];
extern u16 D_8011CF4C[];
extern s16 D_80182D6C[];
extern s32 D_801B2EA4;
extern s32 D_801B2EA0;
extern void func_800ACD18(void);

/**
 * @brief Spawn a world-map wandering actor and randomize its start position.
 */
void func_800AAED4(void)
{
    D_801399D4 = D_8011F538;
    D_800D93F4[0x6] = 0xF;
    *(s16*)&D_800D93F4[0x10] = -1;
    *(s16*)&D_800D93F4[0x26] = 0x2;
    *(s16*)&D_800D93F4[0x2] = 0;
    *(s16*)&D_800D93F4[0xE] = 0;
    *(s16*)&D_800D93F4[0x22] = 0x81;
    *(s16*)&D_800D93F4[0x24] = 0x81;
    D_80182D6C[0] = D_8011CF4C[0] + ((rand() * 50) >> 15) - 10;
    D_80182D6C[1] = D_8011CF4C[1] + ((rand() * 30) >> 15) + 15;
    D_801B2EA4 = 0x24;
    D_801B2EA0 += 1;
    func_800ACD18();
}
