#include "common.h"
/** @brief Packet word accessible as four bytes. */
typedef union Word
{
    u32 word;
    u8 bytes[4];
} Word;
/** @brief Forty-byte textured quad packet with four positions and UV coordinates. */
typedef struct Quad
{
    Word tag, color;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad2;
    s16 x3, y3;
    u8 u3, v3;
    u16 pad3;
} Quad;
extern void bcopy(void *, void *, s32);
/**
 * @brief Append an icon quad, its offset shadow, and a draw-mode packet to an ordering table.
 * @param arg0 Address of the available primitive buffer.
 * @param arg1 Ordering-table entry receiving the packets.
 * @param arg2 Icon selector controlling texture coordinates and palette.
 * @param arg3 Left screen coordinate.
 * @param arg4 Top screen coordinate.
 * @param arg5 Nonzero to mirror the icon horizontally.
 * @return Address immediately after the emitted packets.
 */
s32 func_800AEAC0(s32 arg0, u32 *arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5)
{
    s16 temp_v0;
    s16 temp_v1;
    s8 temp_v1_2;
    u8 temp_a0;
    u8 temp_a0_2;
    u8 temp_v0_2;
    u8 temp_v0_3;
    u8 var_v0;
    u8 var_v0_2;

    u8 *cursor = (u8 *)arg0;
    u8 *source;

    ((Quad *)arg0)->color.word = 0x808080;
    ((Quad *)arg0)->x0 = arg3;
    ((Quad *)arg0)->tag.bytes[3] = 9;
    ((Quad *)arg0)->color.bytes[3] = 0x2C;
    temp_v0 = arg3 + 0x2F;
    ((Quad *)arg0)->x3 = temp_v0;
    ((Quad *)arg0)->x1 = temp_v0;
    ((Quad *)arg0)->x2 = arg3;
    ((Quad *)arg0)->y0 = (s16)arg4;
    temp_v1 = arg4 + 0x2F;
    ((Quad *)arg0)->y1 = (s16)arg4;
    ((Quad *)arg0)->y3 = temp_v1;
    ((Quad *)arg0)->y2 = temp_v1;
    if (arg2 == 2)
    {
        var_v0 = 0xA0;
    }
    else
    {
        var_v0 = 0xD0;
    }
    ((Quad *)arg0)->u0 = var_v0;
    if (arg2 == 0)
    {
        var_v0_2 = 0x20;
    }
    else
    {
        var_v0_2 = 0x50;
    }
    ((Quad *)cursor)->v0 = var_v0_2;
    temp_a0 = ((Quad *)cursor)->v0;
    temp_v0_2 = ((Quad *)cursor)->u0;
    ((Quad *)cursor)->u2 = temp_v0_2;
    temp_v0_3 = temp_v0_2 + 0x2F;
    temp_v1_2 = temp_a0 + 0x2F;
    ((Quad *)cursor)->u3 = temp_v0_3;
    ((Quad *)cursor)->u1 = temp_v0_3;
    ((Quad *)cursor)->v1 = temp_a0;
    ((Quad *)cursor)->v3 = temp_v1_2;
    ((Quad *)cursor)->v2 = temp_v1_2;
    if (arg5 != 0)
    {
        ((Quad *)cursor)->u1 = (u8)((Quad *)cursor)->u0;
        temp_a0_2 = ((Quad *)cursor)->u2;
        ((Quad *)cursor)->u0 = temp_v0_3;
        ((Quad *)cursor)->u2 = temp_v0_3;
        ((Quad *)cursor)->u3 = temp_a0_2;
    }
    ((Quad *)cursor)->clut = (s16)(((arg2 + 0x1D8) << 6) | 0x11);
    ((Quad *)cursor)->tpage = 0x1F;
    ((Quad *)cursor)->tag.word =
        (s32)((((Quad *)cursor)->tag.word & 0xFF000000) | (*arg1 & 0xFFFFFF));
    source = cursor;
    *arg1 = (*arg1 & 0xFF000000) | ((s32)cursor & 0xFFFFFF);
    cursor += 0x28;
    bcopy(source, cursor, 0x28);
    ((Quad *)cursor)->tag.bytes[3] = 9;
    ((Quad *)cursor)->color.word = 0;
    ((Quad *)cursor)->color.bytes[3] = 0x2E;
    ((Quad *)cursor)->x1 = (u16)(((Quad *)cursor)->x1 + 2);
    ((Quad *)cursor)->x0 = (u16)(((Quad *)cursor)->x0 + 2);
    ((Quad *)cursor)->x3 = (u16)(((Quad *)cursor)->x3 + 2);
    ((Quad *)cursor)->x2 = (u16)(((Quad *)cursor)->x2 + 2);
    ((Quad *)cursor)->y1 = (u16)(((Quad *)cursor)->y1 + 2);
    ((Quad *)cursor)->y0 = (u16)(((Quad *)cursor)->y0 + 2);
    ((Quad *)cursor)->y3 = (u16)(((Quad *)cursor)->y3 + 2);
    ((Quad *)cursor)->y2 = (u16)(((Quad *)cursor)->y2 + 2);
    ((Quad *)cursor)->tag.word =
        (s32)((((Quad *)cursor)->tag.word & 0xFF000000) | (*arg1 & 0xFFFFFF));

    *arg1 = (*arg1 & 0xFF000000) | ((s32)cursor & 0xFFFFFF);
    cursor += 0x28;
    ((Quad *)cursor)->tag.bytes[3] = 1;
    ((Quad *)cursor)->color.word = 0xE100001F;
    ((Quad *)cursor)->tag.word =
        (s32)((((Quad *)cursor)->tag.word & 0xFF000000) | (*arg1 & 0xFFFFFF));
    *arg1 = (*arg1 & 0xFF000000) | ((s32)cursor & 0xFFFFFF);
    return (s32)(cursor + 8);
}
