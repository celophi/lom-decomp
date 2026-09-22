#include "common.h"

extern u8* D_801399DC;
extern u8 D_80127538[];
extern u8 D_800D9420[];
extern s32 D_801B2838;
extern s32 D_801B283C;
extern void func_8008335C(void);

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_800832D8(void)
{
    D_801399DC = D_80127538;
    D_800D9420[0x6] = 0xF;
    *(s16*)&D_800D9420[0xE] = 2;
    *(s16*)&D_800D9420[0x10] = -1;
    *(s16*)&D_800D9420[0x26] = 8;
    *(s16*)&D_800D9420[0x22] = 0x81;
    *(s16*)&D_800D9420[0x2] = 0;
    *(s16*)&D_800D9420[0x24] = 1;
    D_801B283C = 0x18;
    D_801B2838 += 1;
    func_8008335C();
}
