#include "wmap_view_effects.h"
#include "common.h"

extern s32 D_801B2400;
extern s32 D_801B2404;

/** @brief World-map step: set fade colour then advance to the next handler. */
void func_8006FF68(void)
{
    func_8006683C(0x808080);
    D_801B2404 = 0x54;
    D_801B2400 += 1;
}
