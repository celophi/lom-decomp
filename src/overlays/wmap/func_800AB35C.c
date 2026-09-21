#include "common.h"

extern u8* D_801399EC;
extern u8 D_80121538[];
extern u8 D_800D9478[];
extern u16 D_8011CF4C[];
extern s16 D_80182DB8[];
extern s32 D_801B2ECC;
extern s32 D_801B2EC8;
extern void func_800AD2CC(void);

/**
 * @brief Spawn a world-map wandering actor and randomize its start position.
 */
void func_800AB35C(void)
{
    D_801399EC = D_80121538;
    D_800D9478[0x6] = 0xF;
    *(s16*)&D_800D9478[0x10] = -1;
    *(s16*)&D_800D9478[0x26] = 0x2;
    *(s16*)&D_800D9478[0x2] = 0;
    *(s16*)&D_800D9478[0xE] = 0;
    *(s16*)&D_800D9478[0x22] = 0x81;
    *(s16*)&D_800D9478[0x24] = 0x81;
    D_80182DB8[0] = D_8011CF4C[0] + ((rand() * 50) >> 15) - 10;
    D_80182DB8[1] = D_8011CF4C[1] + ((rand() * 30) >> 15) + 15;
    D_801B2ECC = 0x24;
    D_801B2EC8 += 1;
    func_800AD2CC();
}
