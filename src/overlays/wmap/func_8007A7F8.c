#include "common.h"

typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B26B0;
extern s32 D_801B26B4;
extern void (*D_800D5340[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B2678;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2660;
extern s32 D_80182DEC;
extern void func_80079384(void);

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_8007A788(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B26B0 = 1;
        D_801B26B4 = 1;
    }

    if (D_801B26B0 < 0x6)
    {
        D_800D5340[D_801B26B0]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007A7F8(void)
{
    D_801B26B0 = 1;
    D_801B26B4 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_8007A810(void)
{
    D_801B2678 = D_80139258;
    D_801B2660 = D_80182DC0;
    D_80182DEC = 0x1;
    D_801B2660.w[2] = 0xA410;
    D_801B26B4 = 0x3A;
    D_801B26B0 += 1;
    func_80079384();
}
