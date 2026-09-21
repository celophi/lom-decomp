#include "common.h"

extern u8* D_80139A0C;
extern u8 D_8011F538[];
extern u8 D_800D9528[];
extern s32 D_80182DEC;
extern s32 D_801B25A0;
extern s32 D_801B25A4;
extern void func_800745A4(void);

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_800760FC(void)
{
    D_80139A0C = D_8011F538;
    D_800D9528[0x6] = 0xF;
    *(s16*)&D_800D9528[0xE] = 1;
    *(s16*)&D_800D9528[0x10] = -1;
    D_80182DEC = 0;
    *(s16*)&D_800D9528[0x2] = 0;
    D_801B25A4 = 0x18;
    D_801B25A0 += 1;
    func_800745A4();
}
