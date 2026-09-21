#include "common.h"

extern void func_8006CAC0(void (*callback)(void));
extern s32 D_801ADAE0;
extern s32 D_801B2C68;
extern s32 D_801B2C6C;
extern void func_8009C09C(void);
extern void func_8009C9F8(void);

/** @brief Register two callbacks around the sequence flag update and begin a 30-tick delay. */
void func_8009BDDC(void)
{
    func_8006CAC0(&func_8009C9F8);
    D_801ADAE0 = 1;
    func_8006CAC0(&func_8009C09C);
    D_801B2C6C = 0x1E;
    D_801B2C68 += 1;
}
