#include "wmap_effect_primitives.h"
#include "common.h"

extern s32 D_801B29E8;
extern s32 D_801B29EC;

/**
 * @brief Draw the world-map cursor via func_8006B6EC, then advance after the wait expires.
 */
void func_8008C260(void)
{
    func_8006B6EC(0x50, 0x58, 0x8, 0, 0x1C);
    if (--D_801B29EC == 0)
    {
        D_801B29E8 += 1;
    }
}
