#include "common.h"

extern u8* D_801399AC;
extern u8 D_8011D538[];
extern u8 D_800D9318[];
extern s32 D_801B2628;
extern s32 D_801B262C;
extern void func_800784C8(void);

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80078450(void)
{
    D_801399AC = D_8011D538;
    D_800D9318[0x6] = 0xF;
    *(s16*)&D_800D9318[0x10] = -1;
    *(s16*)&D_800D9318[0x2] = 0;
    *(s16*)&D_800D9318[0xE] = 0;
    *(s16*)&D_800D9318[0x26] = 0;
    *(s16*)&D_800D9318[0x22] = 0x80;
    *(s16*)&D_800D9318[0x24] = 0x80;
    D_801B262C = 0x4A;
    D_801B2628 += 1;
    func_800784C8();
}
