#include "common.h"

extern u8* D_801399BC;
extern u8 D_80125538[];
extern u8 D_800D9370[];
extern s32 D_801B2E38;
extern s32 D_801B2E3C;
extern void func_800A5D68(void);

/**
 * @brief Populate a world-map actor control block and schedule its next step.
 */
void func_800A5CF0(void)
{
    D_801399BC = D_80125538;
    D_800D9370[0x6] = 0xF;
    *(s16*)&D_800D9370[0x10] = -1;
    *(s16*)&D_800D9370[0x2] = 0;
    *(s16*)&D_800D9370[0xE] = 0;
    *(s16*)&D_800D9370[0x26] = 0;
    *(s16*)&D_800D9370[0x22] = 0x80;
    *(s16*)&D_800D9370[0x24] = 0x80;
    D_801B2E3C = 0x82;
    D_801B2E38 += 1;
    func_800A5D68();
}
