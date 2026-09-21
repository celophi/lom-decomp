#include "common.h"

extern s32 func_8006683C(s32);
extern void func_8006CAC0(void (*callback)(void));
extern s32 D_80139244;
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern s32 D_801B2DD8;
extern s32 D_801B2DDC;
extern void func_800A44CC(void);
extern void func_800A4810(void);

/** @brief Register callbacks, set effect flags and color, and begin a ten-tick delay. */
void func_800A3FA0(void)
{
    func_8006CAC0(&func_800A4810);
    D_80139244 = 1;
    D_801ADAE0 = 1;
    func_8006CAC0(&func_800A44CC);
    D_801ADAF4 = 1;
    func_8006683C(0x201010);
    D_801B2DDC = 0xA;
    D_801B2DD8 += 1;
}
