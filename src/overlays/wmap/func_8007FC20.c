#include "common.h"

extern s32 D_801B2790;
extern void func_8007EE18(void);
extern s32 D_801B2794;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007FBE8(void)
{
    D_801B2794 = 0x40;
    D_801B2790 += 1;
    func_8007EE18();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007FC20(void)
{
    D_801B2790 += 1;
}
