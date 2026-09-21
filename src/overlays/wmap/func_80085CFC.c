#include "common.h"

extern s32 D_801B28BC;
extern s32 D_801B28B8;
extern void func_8006CAC0(void (*step)(void));
extern void func_80086544(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80085CFC(void)
{
    if (--D_801B28BC == 0)
    {
        D_801B28B8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80085D30(void)
{
    func_8006CAC0(func_80086544);
    D_801B28BC = 0x1E;
    D_801B28B8 += 1;
}
