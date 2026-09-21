#include "common.h"

typedef struct
{
    u8 b[8];
} Blk8;

typedef struct
{
    s32 w[4];
} Blk16;

extern Blk8 D_80139258;
extern Blk8 D_8013B240;
extern Blk16 D_80182DC0;
extern Blk16 D_80139888;
extern s32 D_80182DF4;
extern s32 D_801B2A38;
extern s32 D_801B2A3C;
extern void func_8008CC14(void);

/** @brief World-map step handler: seed the actor block, arm the timer, and advance. */
void func_8008E2F8(void)
{
    D_8013B240 = D_80139258;
    D_80139888 = D_80182DC0;
    D_80182DF4 = 1;
    *(s32 *)((u8 *)&D_80139888 + 0x8) = 0xBB8;
    D_801B2A3C = 0x48;
    D_801B2A38 += 1;
    func_8008CC14();
}
