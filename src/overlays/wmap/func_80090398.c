#include "common.h"

extern s32 func_8006683C(s32);
extern s32 D_801ADAF4;
extern s32 D_801B2A70;
extern s32 D_801B2A74;

/** @brief Set the drawing color and world-map value, then start a 26-tick delay. */
void func_80090398(void)
{
    func_8006683C(0x808080);
    D_801ADAF4 = 0xF;
    D_801B2A74 = 0x1A;
    D_801B2A70 += 1;
}
