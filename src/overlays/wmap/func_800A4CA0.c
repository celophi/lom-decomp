#include "common.h"

typedef struct { unsigned char b[8]; } WmapBlk8;

extern s32 D_80182DF4;
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B240;
extern s32 D_801B2E08;
extern s32 D_801B2E0C;
extern void func_800A36F4(void);

/** @brief World-map step handler: copy the source block, set the size cap, and advance. */
void func_800A4CA0(void)
{
    D_80182DF4 = 1;
    D_8013B240 = D_80139258;
    D_801B2E0C = 0x14;
    D_801B2E08 += 1;
    func_800A36F4();
}
