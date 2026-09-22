#include "common.h"

typedef struct { unsigned char b[8]; } WmapBlk8;

extern s32 D_80182DEC;
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern s32 D_8013923C;
extern s32 D_801B2D18;
extern s32 D_801B2D1C;
extern void func_8009DC9C(void);

/**
 * @brief Arm the world-map sequence, seed its data block, and schedule the next step.
 */
void func_8009F814(void)
{
    D_80182DEC = 1;
    D_801B24A8 = D_80139258;
    D_8013923C = 0;
    D_801B2D1C = 0x3C;
    D_801B2D18 += 1;
    func_8009DC9C();
}
