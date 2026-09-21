#include "common.h"

typedef struct { unsigned char b[8]; } WmapBlk8;

extern s32 D_80182DF0;
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B238;
extern s32 D_801B2E00;
extern s32 D_801B2E04;
extern void func_800A3548(void);

/** @brief World-map step handler: copy the source block, set the size cap, and advance. */
void func_800A4B4C(void)
{
    D_80182DF0 = 1;
    D_8013B238 = D_80139258;
    D_801B2E04 = 0x40;
    D_801B2E00 += 1;
    func_800A3548();
}
