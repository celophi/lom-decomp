#include "common.h"

typedef struct { unsigned char b[8]; } WmapBlk8;

extern s32 D_801B25D8;
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B2670;
extern s32 D_80139234;
extern s32 D_801B2F38;
extern s32 D_801B2F3C;
extern void func_800AE12C(void);

/**
 * @brief Arm the world-map sequence, seed its data block, and schedule the next step.
 */
void func_800B0104(void)
{
    D_801B25D8 = 1;
    D_801B2670 = D_80139258;
    D_80139234 = 0;
    D_801B2F3C = 0x84;
    D_801B2F38 += 1;
    func_800AE12C();
}
