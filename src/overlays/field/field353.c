#include "common.h"

extern s32 D_8010AE78;
extern u8 *D_80122B78;
extern u8 *D_80123FB8;

/**
 * @brief Dispatch a resolved sequence action and latch a scene-state flag.
 *
 * Runs func_800C2B14 for @p arg1, resolves @p arg0 (0xFF reads the default from
 * @c D_80123FB8[0]), and forwards the pair to func_8006AD04. When @c D_8010AE78
 * is set it triggers func_80087FC0 and rewrites bits 17-19 of the scene flag
 * word at @c D_80122B78 + 0x400 to 0x20000. Always finishes with func_800BD520.
 *
 * @param arg0 Action index, or 0xFF to read the default from @c D_80123FB8[0].
 * @param arg1 Secondary parameter forwarded to the dispatched calls.
 * @see decomp.me (100%) TODO
 */
void func_800BC328(s32 arg0, s32 arg1)
{
    s32 var_a0;

    func_800C2B14(arg1);
    if (arg0 == 0xFF)
    {
        var_a0 = *D_80123FB8;
    }
    else
    {
        var_a0 = arg0;
    }
    func_8006AD04(var_a0, arg1, 0);
    if (D_8010AE78 != 0)
    {
        func_80087FC0(1, 2);
        *(s32 *)(D_80122B78 + 0x400) = (*(s32 *)(D_80122B78 + 0x400) & 0xFFF9FFFF) | 0x20000;
    }
    func_800BD520(0, 0x2F08, arg1);
}
