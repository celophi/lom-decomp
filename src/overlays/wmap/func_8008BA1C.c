#include "common.h"

extern u8* D_801399AC;
extern u8 D_8011D538[];
extern u8 D_800D9318[];
extern s32 D_801B29B8;
extern s32 D_801B29BC;
extern void func_8008BA94(void);

/**
 * @brief Populate a world-map actor control block and schedule its next step.
 */
void func_8008BA1C(void)
{
    D_801399AC = D_8011D538;
    D_800D9318[0x6] = 0xF;
    *(s16*)&D_800D9318[0x10] = -1;
    *(s16*)&D_800D9318[0x2] = 0;
    *(s16*)&D_800D9318[0xE] = 0;
    *(s16*)&D_800D9318[0x26] = 0;
    *(s16*)&D_800D9318[0x22] = 0x80;
    *(s16*)&D_800D9318[0x24] = 0x80;
    D_801B29BC = 0x48;
    D_801B29B8 += 1;
    func_8008BA94();
}
