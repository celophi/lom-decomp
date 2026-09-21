#include "common.h"

typedef struct { unsigned char b[8]; } WmapBlk8;

extern s32 D_80182DF0;
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B238;
extern s32 D_80139234;
extern s32 D_801B2C90;
extern s32 D_801B2C94;
extern void func_8009B0BC(void);

/**
 * @brief Arm the world-map sequence, seed its data block, and schedule the next step.
 */
void func_8009C724(void)
{
    D_80182DF0 = 1;
    D_8013B238 = D_80139258;
    D_80139234 = 0;
    D_801B2C94 = 0x60;
    D_801B2C90 += 1;
    func_8009B0BC();
}
