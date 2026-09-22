#include "common.h"

extern u8* D_801399AC;
extern u8 D_8011F538[];
extern u8 D_800D9318[];
extern s32 D_801B2B98;
extern s32 D_801B2B9C;
extern void func_8009665C(void);

/**
 * @brief Populate a world-map actor control block and schedule its next step.
 */
void func_800965E4(void)
{
    D_801399AC = D_8011F538;
    D_800D9318[0x6] = 0xF;
    *(s16*)&D_800D9318[0x10] = -1;
    *(s16*)&D_800D9318[0x2] = 0;
    *(s16*)&D_800D9318[0xE] = 0;
    *(s16*)&D_800D9318[0x26] = 0;
    *(s16*)&D_800D9318[0x22] = 0x81;
    *(s16*)&D_800D9318[0x24] = 0x81;
    D_801B2B9C = 0x4C;
    D_801B2B98 += 1;
    func_8009665C();
}
