#include "common.h"
#include "sdk/libgpu.h"

s32 field_text_build_sprites(SPRT* prim, u8* text, s32 style);
SPRT* func_800AD658(s32* ordering_table, SPRT* sprite_cursor, s32 count);

/**
 * @brief Builds and links text sprite primitives into an ordering table.
 * @param sprite_cursor Primitive-buffer cursor used for generated sprites.
 * @param ordering_table Ordering-table entry that receives the generated primitives.
 * @param text Null-terminated text to render.
 * @param text_color Text style passed to the glyph builder.
 * @param x Horizontal origin used to position the rendered text.
 * @param y Vertical origin used to position the rendered text.
 * @param alignment Horizontal alignment mode for the generated glyphs.
 * @param color Sprite tint value, or 0x100 to use the neutral tint.
 * @return Primitive-buffer cursor immediately after the generated draw commands.
 */
void* func_800A66B4(SPRT* sprite_cursor, s32* ordering_table, u8* text, s32 text_color, s32 x, s32 y, s32 alignment, s32 color)
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

    if (alignment != 1)
    {
        if (alignment == 2)
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
            setSprt(sprite_cursor);
            if (color != 0x100)
            {
                setRGB0(sprite_cursor, color, color, color);
                setSemiTrans(sprite_cursor, 1);
            }
            else
            {
                setRGB0(sprite_cursor, 0x80, 0x80, 0x80);
            }
            sprite_cursor->x0 = x + acc;
            sprite_cursor->y0 = y;
            acc += sprite_cursor->w;

            addPrim(ordering_table, sprite_cursor);
            sprite_cursor++;
            count--;
        } while (count != 0);
    }

    sprite_cursor = func_800AD658(ordering_table, sprite_cursor, n);

    tpage = (DR_TPAGE*)sprite_cursor;
    setDrawTPage(tpage, 0, 0, 0x3F);
    addPrim(ordering_table, tpage);

    return tpage + 1;
}
