#include "common.h"

typedef struct { unsigned char b[8]; } WmapBlk8;

extern s32 D_80182DE4;
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B2498;
extern s32 D_8013923C;
extern s32 D_801B2FC8;
extern s32 D_801B2FCC;
extern void func_800B1A74(void);

/**
 * @brief Arm the world-map sequence, seed its data block, and schedule the next step.
 */
void func_800B30B0(void)
{
    D_80182DE4 = 1;
    D_801B2498 = D_80139258;
    D_8013923C = 0;
    D_801B2FCC = 0x159;
    D_801B2FC8 += 1;
    func_800B1A74();
}
