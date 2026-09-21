#include "common.h"

extern void func_800652A8(s32, s32);
extern s32 func_8006683C(s32);
extern void func_8006CAC0(void (*callback)(void));
extern s32 D_801ADAF4;
extern s32 D_801B27F0;
extern s32 D_801B27F4;
extern void func_80082360(void);
extern void func_800826A0(void);

/** @brief Set world-map color, play sound 30, register two callbacks, and begin an eight-tick delay. */
void func_80081DD8(void)
{
    func_8006683C(0x122840);
    D_801ADAF4 = 4;
    func_800652A8(0x1E, 0x80);
    func_8006CAC0(&func_80082360);
    func_8006CAC0(&func_800826A0);
    D_801B27F4 = 8;
    D_801B27F0 += 1;
}
