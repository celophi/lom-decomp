#include "common.h"

typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2CF8;
extern s32 D_801B2CFC;
extern void (*D_800D67FC[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B238;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_80139870;
extern s32 D_80182DF0;
extern void func_8009D820(void);

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009F0C8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2CF8 = 1;
        D_801B2CFC = 1;
        return 1;
    }

    if (D_801B2CF8 < 0x4)
    {
        D_800D67FC[D_801B2CF8]();
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
void func_8009F140(void)
{
    D_801B2CF8 = 1;
    D_801B2CFC = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_8009F158(void)
{
    D_8013B238 = D_80139258;
    D_80139870 = D_80182DC0;
    D_80182DF0 = 0x80;
    D_80139870.w[2] = 0xAFC8;
    D_801B2CFC = 0x40;
    D_801B2CF8 += 1;
    func_8009D820();
}
