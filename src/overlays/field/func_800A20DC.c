#include "common.h"

extern s32 D_80117E68;
extern s32 D_80117E7C;

/**
 * @brief Fill an output array with fixed-point offsets derived from field globals.
 * @param out Destination array for the generated 20.12 fixed-point values.
 */
void func_800A20DC(s32 *out)
{
    s32 value;
    s32 base;
    s32 bound;
    s32 i;

    i = 0;
    value = D_80117E7C;
    if (value > 0)
    {
        bound = value;
        value = D_80117E68;
        base = value - 1;
        do
        {
            *out++ = (i - base) << 12;
            i++;
        } while (i < bound);
    }
}
