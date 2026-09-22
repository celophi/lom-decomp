#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 func_8006683C(s32);
extern s32 D_80139244;
extern s32 D_801ADAF4;
extern s32 D_801B2D58;
extern s32 D_801B2D5C;
extern void func_800A2058(void);
extern void func_800A3140(void);

/** @brief Register two callbacks around a color update and begin a 136-tick delay. */
void func_800A1B80(void)
{
    func_8006CAC0(&func_800A2058);
    D_80139244 = 0;
    func_8006683C(0x601550);
    D_801ADAF4 = 8;
    func_8006CAC0(&func_800A3140);
    D_801B2D5C = 0x88;
    D_801B2D58 += 1;
}
