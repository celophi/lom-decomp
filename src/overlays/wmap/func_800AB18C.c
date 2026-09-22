#include "common.h"

extern u8* D_801399DC;
extern u8 D_80121538[];
extern u8 D_800D9420[];
extern u16 D_8011CF4C[];
extern s16 D_80182D90[];
extern s32 D_801B2EBC;
extern s32 D_801B2EB8;
extern void func_800AD084(void);

/**
 * @brief Spawn a world-map wandering actor and randomize its start position.
 */
void func_800AB18C(void)
{
    D_801399DC = D_80121538;
    D_800D9420[0x6] = 0xF;
    *(s16*)&D_800D9420[0x10] = -1;
    *(s16*)&D_800D9420[0x26] = 0x2;
    *(s16*)&D_800D9420[0x2] = 0;
    *(s16*)&D_800D9420[0xE] = 0;
    *(s16*)&D_800D9420[0x22] = 0x81;
    *(s16*)&D_800D9420[0x24] = 0x81;
    D_80182D90[0] = D_8011CF4C[0] + ((rand() * 50) >> 15) - 10;
    D_80182D90[1] = D_8011CF4C[1] + ((rand() * 30) >> 15) + 15;
    D_801B2EBC = 0x24;
    D_801B2EB8 += 1;
    func_800AD084();
}
