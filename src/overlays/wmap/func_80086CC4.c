#include "common.h"

extern s32 D_801B28F8;
extern void func_80085950(void);
extern s32 D_801B28FC;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80086C8C(void)
{
    D_801B28FC = 0x40;
    D_801B28F8 += 1;
    func_80085950();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80086CC4(void)
{
    D_801B28F8 += 1;
}
