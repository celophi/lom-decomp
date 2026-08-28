#include "common.h"

/**
 * @brief Look up a width/height pair for a field text-box style.
 *
 * Selects a fixed (@p arg1, @p arg2) size pair keyed by the style index
 * @p arg0 (0-6); out-of-range indices leave both outputs untouched.
 *
 * @param arg0 Style index (0-6).
 * @param arg1 Receives the first dimension.
 * @param arg2 Receives the second dimension.
 * @note Diff shows 99.70% in isolation; the only delta is the compiler-generated
 *       switch jump table being anonymous in the scratch vs the named
 *       jtbl_80051144 relocation, which resolves identically at link time.
 * @see decomp.me (100%) TODO
 */
void func_8009D95C(s32 arg0, s32 *arg1, s32 *arg2)
{
    switch (arg0)
    {
    case 0:
        *arg1 = 0x1E;
        *arg2 = 0x60;
        break;
    case 1:
        *arg1 = 0x40;
        *arg2 = 0x80;
        break;
    case 2:
        *arg1 = 0x10;
        *arg2 = 0x40;
        break;
    case 3:
        *arg1 = 0x1E;
        *arg2 = 0xC8;
        break;
    case 4:
        *arg1 = 0x1E;
        *arg2 = 0x40;
        break;
    case 5:
        *arg1 = 0;
        *arg2 = 0x64;
        break;
    case 6:
        *arg1 = 0x1E;
        *arg2 = 0x40;
        break;
    }
}
