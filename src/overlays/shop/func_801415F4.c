#include "common.h"
#include "vector.h"
#include "sdk/libgpu.h"

typedef struct
{
    u16 id;
    u16 count;
    s32 value;
} ShopEntry;

typedef struct
{
    u8 pad[0x2C];
    u32 money;
} ShopState;

extern u8 D_800EC3C4[];
extern u8 D_800EC3DC[];
extern u8 D_800EC3E0[];
extern ShopState *D_8012271C;
extern u8 D_80142D04[];
extern s32 D_80142D0C;
extern s32 D_801451C4;
extern s32 D_80145240;
extern s32 D_80145244;
extern s32 D_80145248;
extern ShopEntry *D_80145250;
extern s32 D_80145CD8;
extern s32 D_80145CDC;

extern s32 func_800A88A0(s32 prim, s32 *ot, void *text, s32 color, s32 x, s32 y, s32 mode);
extern s32 func_800A8A78(s32 *ot, s32 prim, s32 value, s32 color, Vec2s *position, s32 mode);

/**
 * @brief Draw the visible shop list rows and append the list marker primitive.
 * @param ot Ordering-table entry used for emitted primitives.
 * @param prim Current primitive-buffer cursor.
 * @param xoff Horizontal list offset.
 * @param yoff Vertical list offset.
 * @return Updated primitive-buffer cursor.
 */
s32 func_801415F4(s32 *ot, s32 prim, s32 xoff, s32 yoff)
{
    Vec2s pos;
    Vec2s *position;
    s32 i;
    s32 y;
    ShopEntry *entry;
    u16 id;
    s32 value;
    s32 row_value;
    s32 row_base;
    s32 x;
    u8 *fallback;

    i = 0;
    x = xoff;
    if (D_80145CD8 > 0)
    {
        s32 fixed_x;
        u8 *text_base;
        fallback = D_800EC3E0;
        do
        {
            ShopEntry **entry_list_ptr = &D_80145250;
            fixed_x = 0x50 - x;
            text_base = D_80142D04;
            position = &pos;
            ot++;
            ot--;
            row_value = i * 0x10;
            row_base = yoff - 2;
            row_base += i;
            row_base -= i;
            row_value -= row_base;
            y = row_value - D_80145248;
            if ((u32)(y + 0xF) < 0x83U)
            {
                do
                {
                    entry = (ShopEntry *)((i * 8) + (s32)*entry_list_ptr);
                } while (0);
                id = entry->id;
                if (id == 0xFFFF)
                {
                    s32 low;
                    s32 high;
                    s32 base_addr;
                    s32 based;
                    low = D_800EC3E0[0];
                    high = fallback[1];
                    base_addr = (s32)D_800EC3C4;
                    based = (high << 8) + base_addr;
                    prim = func_800A88A0(prim, ot, (void *)(low + based), 4, fixed_x, y, 2);
                }
                else
                {
                    if (id & 0x8000)
                    {
                        prim = func_800A88A0(prim, ot,
                            (void *)(D_80145244 + ((id & 0x7FFF) << 6)),
                        4, fixed_x, y, 2);
                    }
                    else
                    {
                        do
                        {
                            s32 table_offset = D_80142D0C;
                            s32 entry_id = *(u16 *)entry;
                            u16 glyph_offset;
                            void *text;
                            entry_id *= 2;
                            glyph_offset = *(u16 *)((entry_id + table_offset) + (s32)text_base);
                            text = (void *)(table_offset + (glyph_offset + (s32)text_base));
                            prim = func_800A88A0(prim, ot, text, 4, fixed_x, y, 2);
                        } while (0);
                    }
                    pos.x = 0xD0 - x;
                    pos.y = y;
                    if (i == D_80145CDC)
                    {
                        value = ((ShopEntry *)((i * 8) + (s32)D_80145250))->value * D_80145240;
                        if ((D_8012271C->money >= (u32)value) || (D_801451C4 == 0))
                        {
                            prim = func_800A8A78(ot, prim, value, 4, position, 0);
                        }
                        else
                        {
                            prim = func_800A8A78(ot, prim, value, 5, position, 0);
                        }
                        prim = func_800A88A0(prim, ot,
                            (void *)(D_800EC3C4 + D_800EC3DC[0] + (D_800EC3DC[1] << 8)),
                        4, 0xAC - x, y, 0);
                        value = D_80145240;
                        pos.x = 0xB8 - x;
                        prim = func_800A8A78(ot, prim, value, 4, position, 0);
                    }
                    else
                    {
                        ShopEntry *value_entry;
                        value_entry = (ShopEntry *)((i * 8) + (s32)D_80145250);
                        if (((u32)value_entry->value <= D_8012271C->money) || (D_801451C4 == 0))
                        {
                            prim = func_800A8A78(ot, prim, value_entry->value, 4, position, 0);
                        }
                        else
                        {
                            prim = func_800A8A78(ot, prim, value_entry->value, 5, position, 0);
                        }
                    }
                }
            }
        } while (++i < D_80145CD8);
    }

    {
        s32 count = D_80145CDC;
        s32 scroll = D_80145248;
        s32 scaled_count = count * 0x10;
        s32 base_y = yoff - 2;
        y = (scaled_count - base_y) - scroll;
    }
    {
        TILE *tile = (TILE *)prim;
        *(u32 *)&tile->r0 = 0xF080F0;
        setlen(tile, 3);
        tile->code = 0x62;
        tile->x0 = 0;
        tile->y0 = y - 1;
        tile->w = 0x126;
        tile->h = 0xE;
        {
            s32 tag = (*(s32 *)prim & 0xFF000000) | (*ot & 0xFFFFFF);
            s32 next = prim + 0x10;
            *(s32 *)prim = tag;
            *ot = (*ot & 0xFF000000) | (prim & 0xFFFFFF);
            return next;
        }
    }
}
