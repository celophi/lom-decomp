#include "common.h"

typedef struct { unsigned char b[8]; } WmapBlk8;

extern s32 D_801B25D8;
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B238;
extern s32 D_80139234;
extern s32 D_801B2D10;
extern s32 D_801B2D14;
extern void func_8009DABC(void);

/**
 * @brief Arm the world-map sequence, seed its data block, and schedule the next step.
 */
void func_8009F6B8(void)
{
    D_801B25D8 = 1;
    D_8013B238 = D_80139258;
    D_80139234 = 0;
    D_801B2D14 = 0x12C;
    D_801B2D10 += 1;
    func_8009DABC();
}
