#include "common.h"

typedef struct { unsigned char b[8]; } WmapBlk8;

extern s32 D_80182DF4;
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B240;
extern s32 D_80139260;
extern s32 D_801B2DB0;
extern s32 D_801B2DB4;
extern void func_800A11C4(void);

/**
 * @brief Arm the world-map sequence, seed its data block, and schedule the next step.
 */
void func_800A2C74(void)
{
    D_80182DF4 = 1;
    D_8013B240 = D_80139258;
    D_80139260 = 0;
    D_801B2DB4 = 0x6C;
    D_801B2DB0 += 1;
    func_800A11C4();
}
