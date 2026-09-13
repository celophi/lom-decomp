#include "common.h"
#include "gpu_packet.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

/**
 * @brief Two-byte descriptor used to locate the "minus" glyph string.
 * @note Local to this TU; a distinct name avoids clashing with other files.
 */
typedef struct
{
    u8 unk0;
    u8 unk1;
} StructEC;

extern StructEC D_800EC3E4;

s32 field_text_build_sprites(SPRT* prim, u8* text, s32 style);
SPRT* func_800AD658(s32* ot, SPRT* sprite_cursor, s32 count);
s32 func_800A8DDC(u8 *arg0);
void func_800A8E28(u8 *dest, u8 *src);

/**
 * @brief Build and enqueue text-glyph sprites for a line of immediate text.
 * @param sprite_cursor Sprite scratch buffer to fill and enqueue.
 * @param ot Ordering table to add primitives to.
 * @param text Glyph string to render.
 * @param text_color Text color/style selector passed to the glyph builder.
 * @param x Starting x coordinate (adjusted for center/right alignment).
 * @param y Starting y coordinate.
 * @param flags Alignment bits (0x7F) plus 0x80 post-processing flag.
 * @return Pointer just past the trailing DR_TPAGE primitive.
 * @see decomp.me (100%) TODO
 */
void* func_800A88A0(SPRT* sprite_cursor, s32* ot, u8* text, s32 text_color, s32 x, s32 y, s32 flags)
{
    s32 n, count, i, acc;
    SPRT* sprite;
    DR_TPAGE* tpage;

    if (*text == 0)
    {
        return sprite_cursor;
    }

    n = field_text_build_sprites(sprite_cursor, text, text_color);
    count = n;

    if ((flags & 0x7F) != 1)
    {
        if ((flags & 0x7F) == 2)
        {
            sprite = sprite_cursor;
            for (i = 0; i < count; i++)
            {
                x -= sprite[i].w >> 1;
            }
        }
    }
    else
    {
        sprite = sprite_cursor;
        for (i = 0; i < count; i++)
        {
            x -= sprite[i].w;
        }
    }

    acc = 0;

    if (count != 0)
    {
        do
        {
            sprite = sprite_cursor;
            SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
            setSprt(sprite);
            sprite->x0 = x + acc;
            sprite->y0 = y;
            acc += sprite->w;

            addPrim(ot, sprite);
            sprite_cursor++;
            count--;
        } while (count != 0);
    }

    if (flags & 0x80)
    {
        sprite_cursor = func_800AD658(ot, sprite_cursor, n);
    }

    tpage = (DR_TPAGE*)sprite_cursor;
    setDrawTPage(tpage, 0, 0, 0x1F);
    addPrim(ot, tpage);

    return tpage + 1;
}

/**
 * @brief Format a value as a narrow decimal string, then render it as text.
 * @param arg0 Ordering table for func_800A88A0.
 * @param arg1 Sprite scratch buffer for func_800A88A0.
 * @param arg2 Signed value to format into the glyph buffer.
 * @param arg3 Text color/style selector.
 * @param arg4 Pointer to a two-element x/y coordinate pair.
 * @param arg5 Alignment/post-processing flags.
 */
void func_800A8A78(void *arg0, void *arg1, s32 arg2, s32 arg3, s16 *arg4, s32 arg5)
{
    extern void func_800A8B90(void *out, s32 arg1, s32 arg2);
    u8 local[0x40];

    func_800A8B90(local, arg2, 0);
    func_800A88A0(arg1, arg0, local, arg3, arg4[0], arg4[1], arg5);
}

/**
 * @brief Format a value as a wide decimal string, then render it as text.
 * @param arg0 Ordering table for func_800A88A0.
 * @param arg1 Sprite scratch buffer for func_800A88A0.
 * @param arg2 Signed value to format into the glyph buffer.
 * @param arg3 Text color/style selector.
 * @param arg4 Pointer to a two-element x/y coordinate pair.
 * @param arg5 Alignment/post-processing flags.
 */
void func_800A8B04(void *arg0, void *arg1, s32 arg2, s32 arg3, s16 *arg4, s32 arg5)
{
    extern void func_800A8B90(void *out, s32 arg1, s32 arg2);
    u8 local[0x40];

    func_800A8B90(local, arg2, 1);
    func_800A88A0(arg1, arg0, local, arg3, arg4[0], arg4[1], arg5);
}

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
