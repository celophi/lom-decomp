#include "common.h"

extern s32 func_800BD414(s32 arg0, s32 arg1);
extern void func_800AD194(s32 arg0);

/**
 * @brief Checks field state 0x2F08 and performs the corresponding update.
 */
void func_800C93B4(void)
{
    if (func_800BD414(0, 0x2F08) == 0x80)
    {
        func_800AD194(1);
    }
    else if (func_800BD414(0, 0x2F08) == 0xFF)
    {
        func_800AD194(0);
    }
}
