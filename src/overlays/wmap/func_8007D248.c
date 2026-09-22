#include "common.h"

extern u8* D_80139B3C;
extern u8 D_8011D538[];
extern u8 D_800D9BB0[];
extern s32 D_801B2738;
extern s32 D_801B273C;
extern void func_8007D2CC(void);

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_8007D248(void)
{
    D_80139B3C = D_8011D538;
    D_800D9BB0[0x6] = 0xF;
    *(s16*)&D_800D9BB0[0xE] = 4;
    *(s16*)&D_800D9BB0[0x10] = -1;
    *(s16*)&D_800D9BB0[0x26] = 8;
    *(s16*)&D_800D9BB0[0x22] = 0x81;
    *(s16*)&D_800D9BB0[0x2] = 0;
    *(s16*)&D_800D9BB0[0x24] = 1;
    D_801B273C = 0x40;
    D_801B2738 += 1;
    func_8007D2CC();
}
