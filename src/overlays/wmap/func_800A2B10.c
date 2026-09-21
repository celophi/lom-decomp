#include "common.h"

/** @brief Eight bytes of world-map effect state copied together. */
typedef struct
{
    u8 bytes[8];
} WmapBlk8;

extern void func_800A0F84(void);
extern s32 D_80139234;
extern WmapBlk8 D_80139258;
extern s32 D_80139264;
extern s32 D_801B25D8;
extern WmapBlk8 D_801B2670;
extern s32 D_801B2DA8;
extern s32 D_801B2DAC;

/** @brief Reset effect state and begin a 96-tick sequence step. */
void func_800A2B10(void)
{
    D_801B25D8 = 1;
    D_801B2670 = D_80139258;
    D_80139234 = 0;
    D_80139264 = 0;
    D_801B2DAC = 0x60;
    D_801B2DA8 += 1;
    func_800A0F84();
}
