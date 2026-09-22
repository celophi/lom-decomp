#include "common.h"

typedef struct { unsigned char b[8]; } WmapBlk8;

extern s32 D_80182DE4;
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B2498;
extern s32 D_8013923C;
extern s32 D_801B2DA0;
extern s32 D_801B2DA4;
extern void func_800A0D80(void);

/**
 * @brief Arm the world-map sequence, seed its data block, and schedule the next step.
 */
void func_800A29B4(void)
{
    D_80182DE4 = 1;
    D_801B2498 = D_80139258;
    D_8013923C = 0;
    D_801B2DA4 = 0x7C;
    D_801B2DA0 += 1;
    func_800A0D80();
}
