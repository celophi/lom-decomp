#include "common.h"

extern u8* D_801399AC;
extern u8 D_8011D538[];
extern u8 D_800D9318[];
extern s32 D_801B2F00;
extern s32 D_801B2F04;
extern void func_800AF6C8(void);

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_800AF644(void)
{
    D_801399AC = D_8011D538;
    D_800D9318[0x6] = 0xF;
    *(s16*)&D_800D9318[0xE] = 3;
    *(s16*)&D_800D9318[0x10] = -1;
    *(s16*)&D_800D9318[0x26] = 8;
    *(s16*)&D_800D9318[0x22] = 0x81;
    *(s16*)&D_800D9318[0x2] = 0;
    *(s16*)&D_800D9318[0x24] = 1;
    D_801B2F04 = 0x80;
    D_801B2F00 += 1;
    func_800AF6C8();
}
