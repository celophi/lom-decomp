#include "common.h"

/**
 * @brief Thin stack-frame wrapper forwarding an offset pointer into func_800C1EC8.
 * @param arg0 Unused.
 * @param arg1 Base value; func_800C1EC8 is called with arg1 + 4.
 */
void func_800BD750(s32 arg0, s32 arg1)
{
    func_800C1EC8(0, arg1 + 4, 0x20);
}
