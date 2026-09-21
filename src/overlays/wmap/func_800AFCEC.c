#include "common.h"

typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2F20;
extern s32 D_801B2F24;
extern void (*D_800D70BC[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B238;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_80139870;
extern s32 D_80182DF0;
extern void func_800ADCC4(void);

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800AFC74(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2F20 = 1;
        D_801B2F24 = 1;
        return 1;
    }

    if (D_801B2F20 < 0x4)
    {
        D_800D70BC[D_801B2F20]();
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
void func_800AFCEC(void)
{
    D_801B2F20 = 1;
    D_801B2F24 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_800AFD04(void)
{
    D_8013B238 = D_80139258;
    D_80139870 = D_80182DC0;
    D_80182DF0 = 0x80;
    D_80139870.w[2] = 0xAFC8;
    D_801B2F24 = 0x20;
    D_801B2F20 += 1;
    func_800ADCC4();
}
