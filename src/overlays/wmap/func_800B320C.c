#include "common.h"

typedef struct { unsigned char b[8]; } WmapBlk8;

extern s32 D_80182DE8;
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern s32 D_80139240;
extern s32 D_801B2FD0;
extern s32 D_801B2FD4;
extern void func_800B1C78(void);

/**
 * @brief Arm the world-map sequence, seed its data block, and schedule the next step.
 */
void func_800B320C(void)
{
    D_80182DE8 = 1;
    D_801B24A0 = D_80139258;
    D_80139240 = 0;
    D_801B2FD4 = 0xE9;
    D_801B2FD0 += 1;
    func_800B1C78();
}
