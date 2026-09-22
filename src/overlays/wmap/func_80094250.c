#include "wmap_view_effects.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_80139244;
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern s32 D_801B2B30;
extern s32 D_801B2B34;
extern void func_80094674(void);

/** @brief Set world-map color and flags, register a callback, and begin a 20-tick delay. */
void func_80094250(void)
{
    func_8006683C(0x561030);
    func_8006CAC0(&func_80094674);
    D_80139244 = 1;
    D_801ADAF4 = 4;
    D_801ADAE0 = 1;
    D_801B2B34 = 0x14;
    D_801B2B30 += 1;
}
