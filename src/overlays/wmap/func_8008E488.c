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
extern Blk8 D_801B2670;
extern Blk16 D_80182DC0;
extern Blk16 D_80139898;
extern s32 D_801B25D8;
extern s32 D_801B2A40;
extern s32 D_801B2A44;
extern void func_8008CE08(void);

/** @brief World-map step handler: seed the actor block, arm the timer, and advance. */
void func_8008E488(void)
{
    D_801B2670 = D_80139258;
    D_80139898 = D_80182DC0;
    D_801B25D8 = 1;
    *(s32 *)((u8 *)&D_80139898 + 0x8) = 0x7530;
    D_801B2A44 = 0x7C;
    D_801B2A40 += 1;
    func_8008CE08();
}
