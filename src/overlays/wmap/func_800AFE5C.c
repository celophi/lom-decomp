#include "common.h"

typedef struct { unsigned char b[8]; } WmapBlk8;

extern s32 D_80182DE4;
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B2498;
extern s32 D_801B2F28;
extern s32 D_801B2F2C;
extern void func_800ADDD4(void);

/** @brief World-map step handler: copy the source block, set the size cap, and advance. */
void func_800AFE5C(void)
{
    D_80182DE4 = 1;
    D_801B2498 = D_80139258;
    D_801B2F2C = 0xB4;
    D_801B2F28 += 1;
    func_800ADDD4();
}
