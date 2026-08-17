#include "common.h"

void func_800A710C(void);
void func_8006441C(void);
void func_800A8880(s32);
void func_80063194(void);

extern s32 D_800F229C[];

/**
 * @see decomp.me (100%) TODO
 */
void func_80067FB0(s32 arg0)
{
    if (D_800F229C[0] != 0)
    {
        func_800A710C();
        if (D_800F229C[0] != 0)
        {
            func_8006441C();
            if (D_800F229C[0] != 0)
            {
                func_800A8880(arg0);
            }
            func_80063194();
        }
    }
}
