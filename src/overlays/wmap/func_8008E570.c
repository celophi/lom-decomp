#include "common.h"

extern s32 D_801B2A40;
extern void func_8008CF04(void);
extern s32 D_801B2A44;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8008E538(void)
{
    D_801B2A44 = 0x40;
    D_801B2A40 += 1;
    func_8008CF04();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008E570(void)
{
    D_801B2A40 += 1;
}
