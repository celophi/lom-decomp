#include "common.h"

extern void func_8006CAC0(void (*callback)(void));
extern s32 D_801ADAE0;
extern s32 D_801B2510;
extern s32 D_801B2514;
extern void func_80073848(void);
extern void func_800739C8(void);

/** @brief Set the sequence flag, register two callbacks, and begin a 48-tick delay. */
void func_8007316C(void)
{
    D_801ADAE0 = 1;
    func_8006CAC0(&func_80073848);
    func_8006CAC0(&func_800739C8);
    D_801B2514 = 0x30;
    D_801B2510 += 1;
}
