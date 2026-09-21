#include "common.h"

extern u32 D_801B2E78;
extern s32 D_801B2E7C;
extern void (*D_800D6DEC[])(void);
extern s32 D_801398D0;
extern void func_800AB920(void);
extern void func_8006CAC0(void (*step)(void));
extern void func_800AB9C8(void);
extern void func_800AB964(void);
extern s32 D_8013B20C;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800AB850(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2E78 = 1;
        D_801B2E7C = 1;
        return 1;
    }

    if (D_801B2E78 < 0x6)
    {
        D_800D6DEC[D_801B2E78]();
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
void func_800AB8C8(void)
{
    D_801B2E78 = 1;
    D_801B2E7C = 1;
}

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void func_800AB8E0(void)
{
    if (D_801398D0 != 2)
    {
        D_801B2E78 += 1;
        func_800AB920();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_800AB920(void)
{
    func_8006CAC0(func_800AB9C8);
    D_8013B20C = 1;
    D_801B2E78 += 1;
    func_800AB964();
}
