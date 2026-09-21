#include "common.h"

typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B28F0;
extern s32 D_801B28F4;
extern void (*D_800D5A20[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern void func_80085718(void);

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80086A38(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B28F0 = 1;
        D_801B28F4 = 1;
        return 1;
    }

    if (D_801B28F0 < 0x4)
    {
        D_800D5A20[D_801B28F0]();
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
void func_80086AB0(void)
{
    D_801B28F0 = 1;
    D_801B28F4 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_80086AC8(void)
{
    D_801B24A8 = D_80139258;
    D_801B2478 = D_80182DC0;
    D_80182DEC = 0x80;
    D_801B2478.w[2] = 0xAFC8;
    D_801B28F4 = 0x40;
    D_801B28F0 += 1;
    func_80085718();
}
