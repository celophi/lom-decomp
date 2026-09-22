#include "common.h"

typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2610;
extern s32 D_801B2614;
extern void (*D_800D51E0[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B240;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_80139888;
extern s32 D_80182DE4;
extern void func_80076FAC(void);

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_80077FD8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2610 = 1;
        D_801B2614 = 1;
    }

    if (D_801B2610 < 0x4)
    {
        D_800D51E0[D_801B2610]();
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
void func_80078048(void)
{
    D_801B2610 = 1;
    D_801B2614 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_80078060(void)
{
    D_8013B240 = D_80139258;
    D_80139888 = D_80182DC0;
    D_80182DE4 = 0x80;
    D_80139888.w[2] = 0xAFC8;
    D_801B2614 = 0x2A;
    D_801B2610 += 1;
    func_80076FAC();
}
