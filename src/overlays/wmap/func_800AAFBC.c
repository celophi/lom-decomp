#include "common.h"

extern u8* D_801399CC;
extern u8 D_8011F538[];
extern u8 D_800D93C8[];
extern u16 D_8011CF4C[];
extern s16 D_80182D7C[];
extern s32 D_801B2EAC;
extern s32 D_801B2EA8;
extern void func_800ACE3C(void);

/**
 * @brief Spawn a world-map wandering actor and randomize its start position.
 */
void func_800AAFBC(void)
{
    D_801399CC = D_8011F538;
    D_800D93C8[0x6] = 0xF;
    *(s16*)&D_800D93C8[0x10] = -1;
    *(s16*)&D_800D93C8[0x26] = 0x2;
    *(s16*)&D_800D93C8[0x2] = 0;
    *(s16*)&D_800D93C8[0xE] = 0;
    *(s16*)&D_800D93C8[0x22] = 0x81;
    *(s16*)&D_800D93C8[0x24] = 0x81;
    D_80182D7C[0] = D_8011CF4C[0] + ((rand() * 50) >> 15) - 10;
    D_80182D7C[1] = D_8011CF4C[1] + ((rand() * 30) >> 15) + 15;
    D_801B2EAC = 0x24;
    D_801B2EA8 += 1;
    func_800ACE3C();
}
