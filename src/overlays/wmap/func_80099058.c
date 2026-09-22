#include "common.h"

/** @brief Eight bytes of world-map effect state copied together. */
typedef struct
{
    u8 bytes[8];
} WmapBlk8;

extern void func_800979A8(void);
extern s32 D_8013923C;
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B238;
extern s32 D_80182DF0;
extern s32 D_801B2C28;
extern s32 D_801B2C2C;

/** @brief Restore effect data, clear two flags, and begin a 100-tick sequence step. */
void func_80099058(void)
{
    D_8013B238 = D_80139258;
    D_80182DF0 = 0;
    D_8013923C = 0;
    D_801B2C2C = 0x64;
    D_801B2C28 += 1;
    func_800979A8();
}
