#include "common.h"

extern u8* D_801399C4;
extern u8 D_8011F538[];
extern u8 D_800D939C[];
extern u16 D_8011CF4C[];
extern s16 D_80182D84[];
extern s32 D_801B2EB4;
extern s32 D_801B2EB0;
extern void func_800ACF60(void);

/**
 * @brief Spawn a world-map wandering actor and randomize its start position.
 */
void func_800AB0A4(void)
{
    D_801399C4 = D_8011F538;
    D_800D939C[0x6] = 0xF;
    *(s16*)&D_800D939C[0x10] = -1;
    *(s16*)&D_800D939C[0x26] = 0x2;
    *(s16*)&D_800D939C[0x2] = 0;
    *(s16*)&D_800D939C[0xE] = 0;
    *(s16*)&D_800D939C[0x22] = 0x81;
    *(s16*)&D_800D939C[0x24] = 0x81;
    D_80182D84[0] = D_8011CF4C[0] + ((rand() * 50) >> 15) - 10;
    D_80182D84[1] = D_8011CF4C[1] + ((rand() * 30) >> 15) + 15;
    D_801B2EB4 = 0x24;
    D_801B2EB0 += 1;
    func_800ACF60();
}
