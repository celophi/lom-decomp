#include "common.h"
#include "gpu_packet.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

s32 field_text_build_sprites(SPRT* prim, u8* text, s32 style);
SPRT* func_800AD658(s32* ot, SPRT* sprite_cursor, s32 count);

/**
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
