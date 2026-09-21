#include "common.h"

typedef struct { unsigned char b[8]; } WmapBlk8;

extern s32 D_80182DF4;
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B240;
extern s32 D_801B2F30;
extern s32 D_801B2F34;
extern void func_800ADF80(void);

/** @brief World-map step handler: copy the source block, set the size cap, and advance. */
void func_800AFFB0(void)
{
    D_80182DF4 = 1;
    D_8013B240 = D_80139258;
    D_801B2F34 = 0xAE;
    D_801B2F30 += 1;
    func_800ADF80();
}
