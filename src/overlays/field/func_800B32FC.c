#include "common.h"

extern u8 *D_80122B74;

void func_800BD520(s32 arg0, s32 arg1, s32 arg2);

/**
 * @see decomp.me (100%)
 */
void func_800B32FC(s32 arg0)
{
    s32 chance;
    s32 offset;

    chance = D_80122B74[0xC06];
    if (((rand() * 100) / 0x8000) < chance)
    {
        func_800BD520(2, 0xD030, 0x81);
        func_800BD520(2, 0xD038, 0);
        func_800BD520(2, 0xD040, 0x64);
    }
    else if (arg0 < 0x24)
    {
        offset = arg0 * 4;
        func_800BD520(2, 0xD030, *(D_80122B74 + offset + 0x2A7C));
        func_800BD520(2, 0xD038, *(D_80122B74 + offset + 0x2A7D));
        func_800BD520(2, 0xD040, *(D_80122B74 + offset + 0x2A7E));
    }
    else
    {
        func_800BD520(2, 0xD030, 0x81);
        func_800BD520(2, 0xD038, 0);
        func_800BD520(2, 0xD040, 0x63);
    }
}
