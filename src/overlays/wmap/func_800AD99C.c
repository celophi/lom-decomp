#include "common.h"

extern void func_800AD9DC(void);
extern s32 D_800DCEAC;
extern s32 D_801B2EEC;
extern s32 D_801B2EE8;

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void func_800AD99C(void)
{
    D_800DCEAC = 0;
    D_801B2EEC = 0x64;
    D_801B2EE8 += 1;
    func_800AD9DC();
}
