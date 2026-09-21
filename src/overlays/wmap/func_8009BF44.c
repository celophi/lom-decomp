#include "common.h"

extern void func_8006CAC0(void (*callback)(void));
extern s32 D_80139244;
extern s32 D_801B2C68;
extern s32 D_801B2C6C;
extern void func_8009C240(void);

/** @brief Register a sequence callback, clear the world-map value, and start a 64-tick delay. */
void func_8009BF44(void)
{
    func_8006CAC0(&func_8009C240);
    D_80139244 = 0;
    D_801B2C6C = 0x40;
    D_801B2C68 += 1;
}
