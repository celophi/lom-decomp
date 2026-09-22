#include "common.h"

/** @brief Eight bytes of world-map effect state copied together. */
typedef struct
{
    u8 bytes[8];
} WmapBlk8;

extern void func_80091850(void);
extern s32 D_80139234;
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B240;
extern s32 D_80182DF4;
extern s32 D_801B2B08;
extern s32 D_801B2B0C;

/** @brief Restore effect state and begin a 66-tick sequence step. */
void func_80092BD8(void)
{
    D_80182DF4 = 1;
    D_8013B240 = D_80139258;
    D_80139234 = 0x200;
    D_801B2B0C = 0x42;
    D_801B2B08 += 1;
    func_80091850();
}
