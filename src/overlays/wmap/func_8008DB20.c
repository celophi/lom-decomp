#include "common.h"

extern s32 func_8006683C(s32);
extern s32 D_80139244;
extern s32 D_801B2A10;
extern s32 D_801B2A14;

/** @brief Clear the world-map value, set the drawing color, and start a 140-tick delay. */
void func_8008DB20(void)
{
    D_80139244 = 0;
    func_8006683C(0x403060);
    D_801B2A14 = 0x8C;
    D_801B2A10 += 1;
}
