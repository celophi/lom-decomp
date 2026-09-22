#include "common.h"

/** @brief Eight bytes copied together as world-map effect state. */
typedef struct
{
    u8 bytes[8];
} WmapBlk8;

extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern s32 D_801B24B4;
extern s32 D_801B2410;
extern s32 D_801B2414;
extern void func_8006D918(void);

/** @brief Restore the effect state and begin a one-tick sequence step. */
void func_8006EFA8(void)
{
    D_801B24B4 = 128;
    D_801B24A0 = D_80139258;
    D_801B2414 = 1;
    D_801B2410 += 1;
    func_8006D918();
}
