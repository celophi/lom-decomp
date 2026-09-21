#include "common.h"

typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2618;
extern s32 D_801B261C;
extern void (*D_800D51F0[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B2670;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_80139898;
extern s32 D_80182DE8;
extern void func_8007708C(void);

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_80078128(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2618 = 1;
        D_801B261C = 1;
    }

    if (D_801B2618 < 0x4)
    {
        D_800D51F0[D_801B2618]();
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
void func_80078198(void)
{
    D_801B2618 = 1;
    D_801B261C = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_800781B0(void)
{
    D_801B2670 = D_80139258;
    D_80139898 = D_80182DC0;
    D_80182DE8 = 0x80;
    D_80139898.w[2] = 0xAFC8;
    D_801B261C = 0x1E;
    D_801B2618 += 1;
    func_8007708C();
}
