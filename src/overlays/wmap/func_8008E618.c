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
extern Blk8 D_801B2678;
extern Blk16 D_80182DC0;
extern Blk16 D_801B2660;
extern s32 D_801B25DC;
extern s32 D_801B2A48;
extern s32 D_801B2A4C;
extern void func_8008CFFC(void);

/** @brief World-map step handler: seed the actor block, arm the timer, and advance. */
void func_8008E618(void)
{
    D_801B2678 = D_80139258;
    D_801B2660 = D_80182DC0;
    D_801B25DC = 1;
    *(s32 *)((u8 *)&D_801B2660 + 0x8) = 0x7530;
    D_801B2A4C = 0x7C;
    D_801B2A48 += 1;
    func_8008CFFC();
}
