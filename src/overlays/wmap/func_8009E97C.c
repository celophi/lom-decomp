#include "wmap_view_effects.h"
#include "common.h"

extern s32 D_801B2CD8;
extern s32 D_801B2CDC;

/** @brief World-map step: set fade colour then advance to the next handler. */
void func_8009E97C(void)
{
    func_8006683C(0x252035);
    D_801B2CDC = 0x38;
    D_801B2CD8 += 1;
}
