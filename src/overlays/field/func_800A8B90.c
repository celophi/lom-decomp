#include "common.h"

typedef struct
{
    u8 unk0;
    u8 unk1;
} StructEC;

extern StructEC D_800EC3E4;

s32 func_800A8DDC(u8 *arg0);
void func_800A8E28(u8 *dest, u8 *src);

/**
 * @brief Format a signed decimal value into the destination glyph buffer.
 * @param buf Destination buffer.
 * @param val Signed value to format.
 */
void func_800A8B90(u8 *buf, s32 val)
{
    u8 *dst;
    s32 value;
    s32 wide;
    u8 *minus;
    s32 low;
    s32 offset;
    s32 div;
    s32 started;
    s32 digit;

    dst = buf;
    value = val;
    wide = 0;
    if (value < 0)
    {
        value = -value;
        low = D_800EC3E4.unk0;
        offset = (D_800EC3E4.unk1 << 8) + (s32)((u8 *)&D_800EC3E4 - 0x20);
        minus = (u8 *)(low + offset);
        func_800A8E28(dst, minus);
        dst += func_800A8DDC(minus);
    }
    div = 10000000;
    started = 0;
    do
    {
        digit = value / div;
        if (digit != 0)
        {
            started = 1;
        }
        if (started || div == 1)
        {
            if (wide)
            {
                *dst++ = 0x1D;
                *dst = digit;
            }
            else
            {
                *dst = digit + '0';
            }
            dst++;
            value -= (value / div) * div;
        }
        div /= 10;
    } while (div != 0);
    *dst = 0;
}
