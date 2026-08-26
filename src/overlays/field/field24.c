#include "common.h"

s32 func_80085FAC(s32 arg0, void *arg1, s32 digit, u16 *arg3);

/**
 * @brief Emit a right-aligned 3-digit decimal number followed by a trailing
 *        glyph, advancing the horizontal cursor 7 units per column.
 * @param arg0 Primitive-buffer cursor passed through the digit emitter.
 * @param arg1 Opaque draw context forwarded to func_80085FAC.
 * @param arg2 Value to render (hundreds/tens/units).
 * @param arg3 Pointer to the current X cursor, bumped by 7 after each column.
 * @note Leading zeros in the hundreds/tens columns are suppressed until the
 *       first non-zero digit is emitted.
 */
void func_80085E84(s32 arg0, void *arg1, s32 arg2, u16 *arg3)
{
    s32 emitted;

    emitted = 0;
    if (arg2 / 100 != 0)
    {
        emitted = 1;
        arg0 = func_80085FAC(arg0, arg1, arg2 / 100, arg3);
        arg2 -= 100;
    }
    *arg3 += 7;
    if (emitted || arg2 / 10 != 0)
    {
        s32 digit;
        digit = arg2 / 10;
        arg0 = func_80085FAC(arg0, arg1, digit, arg3);
        arg2 -= digit * 10;
    }
    *arg3 += 7;
    arg0 = func_80085FAC(arg0, arg1, arg2, arg3);
    *arg3 += 7;
    func_80085FAC(arg0, arg1, 10, arg3);
}
