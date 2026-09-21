#include "common.h"

typedef struct { unsigned char b[8]; } WmapBlk8;

extern s32 D_801B25DC;
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B2678;
extern s32 D_8013923C;
extern s32 D_801B2F40;
extern s32 D_801B2F44;
extern void func_800AE30C(void);

/**
 * @brief Arm the world-map sequence, seed its data block, and schedule the next step.
 */
void func_800B0260(void)
{
    D_801B25DC = 1;
    D_801B2678 = D_80139258;
    D_8013923C = 0;
    D_801B2F44 = 0x84;
    D_801B2F40 += 1;
    func_800AE30C();
}
