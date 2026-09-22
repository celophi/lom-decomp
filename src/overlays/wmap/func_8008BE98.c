#include "common.h"

typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B29D0;
extern s32 D_801B29D4;
extern void (*D_800D5D18[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern void func_8008A964(void);

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008BE20(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B29D0 = 1;
        D_801B29D4 = 1;
        return 1;
    }

    if (D_801B29D0 < 0x4)
    {
        D_800D5D18[D_801B29D0]();
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
void func_8008BE98(void)
{
    D_801B29D0 = 1;
    D_801B29D4 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_8008BEB0(void)
{
    D_801B24A8 = D_80139258;
    D_801B2478 = D_80182DC0;
    D_80182DEC = 0x80;
    D_801B2478.w[2] = 0xAFC8;
    D_801B29D4 = 0x100;
    D_801B29D0 += 1;
    func_8008A964();
}
