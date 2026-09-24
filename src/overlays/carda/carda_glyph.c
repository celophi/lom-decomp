#include "carda_internal.h"

/* Glyph cache layout: 256 slots of 16x15 4-bit glyphs, 16 per VRAM row. */
#define CARDA_GLYPH_CACHE_SLOTS 0x100
#define CARDA_GLYPH_CACHE_COLUMNS 16
#define CARDA_GLYPH_CACHE_ROW_MASK 0xF0
#define CARDA_GLYPH_RASTER_BYTES 0x80
#define CARDA_GLYPH_RASTER_BUFFER_BYTES 0x8000

/* CardaGlyphCacheEntry.raw flag: the slot was drawn this frame. */
#define CARDA_GLYPH_CACHE_USED 0x10000

/* Shift-JIS glyph pairs as stored in the little-endian u16 digit tables. */
#define CARDA_GLYPH_PAIR_ZERO 0x4F82
#define CARDA_GLYPH_PAIR_MINUS 0x5B81

/* Added to an ASCII byte to get its Shift-JIS glyph code. */
#define CARDA_SJIS_ALNUM_OFFSET 0x821F
#define CARDA_SJIS_SYMBOL_OFFSET 0x851F

/* Glyph code tables: 33-byte rows of 16 two-byte glyphs plus a terminator. */
#define CARDA_CHAR_TABLE_ROW_BYTES 33
#define CARDA_CHAR_TABLE_PAGE_BYTES (16 * CARDA_CHAR_TABLE_ROW_BYTES)

/**
 * @brief Draw @p value as up to five full-width decimal digits through the
 *        glyph cache, suppressing leading zeros and prefixing a minus sign.
 * @param prim Primitive-buffer cursor.
 * @param ot Ordering-table entry the glyphs are linked into.
 * @param value Signed value to draw.
 * @param x Text x position, interpreted per @p alignment.
 * @param y Text top edge.
 * @param palette Glyph palette index.
 * @param alignment Alignment mode passed to carda_draw_cached_text.
 * @return Advanced primitive-buffer cursor.
 */
void* carda_draw_signed_decimal(void* prim, u_long* ot, s32 value, s32 x, s32 y, s32 palette, s32 alignment)
{
    u16 buf[7];
    s32 first_digit;
    s32 magnitude;
    s32 negative;

    magnitude = value;
    if (magnitude < 0)
    {
        magnitude = -magnitude;
        negative = 1;
    }
    else
    {
        negative = 0;
    }
    buf[1] = g_carda_decimal_glyphs[magnitude / 10000];
    buf[2] = g_carda_decimal_glyphs[(magnitude % 10000) / 1000];
    buf[3] = g_carda_decimal_glyphs[(magnitude % 1000) / 100];
    buf[4] = g_carda_decimal_glyphs[(magnitude % 100) / 10];
    buf[5] = g_carda_decimal_glyphs[magnitude % 10];

    first_digit = 1;
    buf[6] = 0;

    while (first_digit < 5 && buf[first_digit] == CARDA_GLYPH_PAIR_ZERO)
    {
        first_digit++;
    }

    if (negative != 0)
    {
        first_digit--;
        buf[first_digit] = CARDA_GLYPH_PAIR_MINUS;
    }
    prim = carda_draw_cached_text(prim, ot, (u8*)&buf[first_digit], x, y, palette, alignment);
    return prim;
}

/**
 * @brief Draw a byte as two full-width hex digits through the glyph cache, using palette 0.
 * @param prim Primitive-buffer cursor.
 * @param ot Ordering-table entry the glyphs are linked into.
 * @param value Byte value to draw.
 * @param x Text x position, interpreted per @p alignment.
 * @param y Text top edge.
 * @param alignment Alignment mode passed to carda_draw_cached_text.
 */
void carda_draw_hex_byte(void* prim, u_long* ot, s32 value, s32 x, s32 y, s32 alignment)
{
    u16 buf[3];
    u16* high_glyph;

    high_glyph = &g_carda_hex_glyphs[value / 16];
    buf[0] = *high_glyph;
    buf[1] = g_carda_hex_glyphs[value % 16];
    buf[2] = 0;
    carda_draw_cached_text(prim, ot, (u8*)buf, x, y, 0, alignment);
}

/**
 * @brief Draw a Shift-JIS / ASCII string through the glyph cache, then emit a
 *        texture-page packet restoring the default page.
 * @param prim Primitive-buffer cursor.
 * @param ot Ordering-table entry the glyphs are linked into.
 * @param text String ending at the first byte below 0x20 other than a space.
 * @param x Text x position, interpreted per @p alignment.
 * @param y Text top edge.
 * @param palette Glyph palette index.
 * @param alignment 0 left-aligned at @p x, 1 ends at @p x, 2 centered on @p x.
 * @return Advanced primitive-buffer cursor.
 */
void* carda_draw_cached_text(void* prim, u_long* ot, u8* text, s32 x, s32 y, s32 palette, s32 alignment)
{
    u8* cursor;
    s32 count;
    u16 code;
    u8* scan;
    DR_TPAGE* draw_mode;

    cursor = text;
    count = 0;
    if (*cursor >= 0x20)
    {
        scan = cursor;
        do
        {
            if (*scan >= 0x80)
            {
                scan++;
            }
            scan++;
            count++;
        } while (*scan >= 0x20);
    }

    switch (alignment)
    {
    case 1:
        x -= count * 16;
        break;
    case 2:
        x -= count * 8;
        break;
    case 0:
    default:
        break;
    }
    g_carda_text_line_start_x = x;
    g_carda_glyph_cursor_x = x;
    g_carda_glyph_cursor_y = y;

    while (1)
    {
        if (*cursor == ' ')
        {
            cursor++;
            g_carda_glyph_cursor_x += 16;
            continue;
        }
        if (*cursor >= 0x80)
        {
            code = cursor[0];
            code = (code << 8) | cursor[1];
            cursor += 2;
        }
        else
        {
            if (*cursor < 0x20)
            {
                break;
            }
            if (*cursor >= '0' && *cursor < 0x80)
            {
                code = *cursor + CARDA_SJIS_ALNUM_OFFSET;
                cursor++;
            }
            else
            {
                code = *cursor + CARDA_SJIS_SYMBOL_OFFSET;
                cursor++;
            }
        }
        prim = carda_render_cached_glyph(prim, ot, code, palette);
    }

    draw_mode = prim;
    setDrawTPage(draw_mode, 0, 0, getTPage(0, 0, 320, 0));
    addPrim(ot, draw_mode);
    return draw_mode + 1;
}

/**
 * @brief Draw one glyph from the cache; on a miss, rasterize it from the kanji
 *        ROM into a free cache slot and upload it to VRAM first.
 * @param prim Primitive-buffer cursor.
 * @param ot Ordering-table entry the sprite is linked into.
 * @param code Shift-JIS glyph code.
 * @param palette Glyph palette index; sets the 4-bit pixel value (palette + 1) * 2.
 * @return Advanced primitive-buffer cursor, or @p prim when the glyph is missing
 *         from the ROM or the cache is full.
 */
void* carda_render_cached_glyph(void* prim, u_long* ot, u16 code, s32 palette)
{
    u8* font_data;
    u8* raster;
    s32 i;
    s32 row;
    s32 source_byte;
    s32 color_index;
    s32 high_nibble_color;
    u16 mask;
    RECT rect;

    for (i = 0; i < CARDA_GLYPH_CACHE_SLOTS; i++)
    {
        if (code == g_carda_glyph_cache[i].data.code)
        {
            return carda_emit_glyph_sprite(prim, ot, i, palette);
        }
    }

    font_data = Krom2RawAdd(code);
    if (font_data == (u8*)-1)
    {
        return prim;
    }

    raster = g_carda_glyph_raster_cursor;
    row = 0;
    color_index = (palette + 1) * 2;
    high_nibble_color = color_index * 16;
    for (; row < 15; row++)
    {
        for (source_byte = 0; source_byte < 2; source_byte++)
        {
            mask = 0x80;
            for (i = 0; i < 4; i++)
            {
                *raster = (*font_data & mask) ? color_index : 0;
                mask >>= 1;
                *raster += (*font_data & mask) ? high_nibble_color : 0;
                mask >>= 1;
                raster++;
            }
            font_data++;
        }
    }

    for (i = 0; i < CARDA_GLYPH_CACHE_SLOTS; i++)
    {
        if (g_carda_glyph_cache[i].raw == 0)
        {
            break;
        }
    }

    if (i == CARDA_GLYPH_CACHE_SLOTS)
    {
        return prim;
    }
    g_carda_glyph_cache[i].raw = code;
    prim = carda_emit_glyph_sprite(prim, ot, i, palette);

    g_carda_glyph_upload_x = (i % CARDA_GLYPH_CACHE_COLUMNS) * 4;
    g_carda_glyph_upload_y = i & CARDA_GLYPH_CACHE_ROW_MASK;

    setWH(&rect, 4, 15);
    rect.x = g_carda_glyph_upload_x + 320;
    rect.y = g_carda_glyph_upload_y;

    LoadImage(&rect, g_carda_glyph_raster_cursor);
    DrawSync(0);

    g_carda_glyph_raster_cursor += CARDA_GLYPH_RASTER_BYTES;
    return prim;
}

/**
 * @brief Emit one cached glyph as a 16x16 sprite at the text cursor, mark the
 *        slot used this frame and advance the cursor, wrapping at x = 640.
 * @param sprite Sprite packet to fill.
 * @param ot Ordering-table entry the sprite is linked into.
 * @param cache_slot Glyph-cache slot whose VRAM tile is sampled.
 * @param palette Unused; the CLUT is fixed.
 * @return Primitive-buffer cursor after the sprite.
 */
void* carda_emit_glyph_sprite(CardaGlyphSprite* sprite, u_long* ot, s32 cache_slot, s32 palette)
{
    g_carda_glyph_cache[cache_slot].raw |= CARDA_GLYPH_CACHE_USED;

    setSprt16(&sprite->packet);
    sprite->packet.g0 = 0x80;
    sprite->packet.b0 = 0x80;
    sprite->packet.r0 = 0x80;
    setXY0(&sprite->packet, g_carda_glyph_cursor_x, g_carda_glyph_cursor_y);
    setUV0(&sprite->packet, (cache_slot % CARDA_GLYPH_CACHE_COLUMNS) * 16, cache_slot & CARDA_GLYPH_CACHE_ROW_MASK);
    sprite->packet.clut = getClut(304, 511);
    addPrim(ot, &sprite->packet);
    sprite++;

    g_carda_glyph_cursor_x += 16;
    if (g_carda_glyph_cursor_x + 16 >= 640)
    {
        g_carda_glyph_cursor_x = g_carda_text_line_start_x;
        g_carda_glyph_cursor_y += 16;
    }

    return sprite;
}

/**
 * @brief Clear per-frame glyph-use flags and reset raster allocation.
 */
void carda_begin_glyph_cache_frame(void)
{
    s32 i;

    g_carda_glyph_raster_cursor = g_carda_glyph_raster_buffer;
    for (i = 0; i < CARDA_GLYPH_CACHE_SLOTS; i++)
    {
        g_carda_glyph_cache[i].raw &= 0xFFFF;
    }
}

/**
 * @brief Evict glyph-cache entries not used this frame.
 */
void carda_evict_unused_glyphs(void)
{
    s32 i;

    for (i = 0; i < CARDA_GLYPH_CACHE_SLOTS; i++)
    {
        if (!(g_carda_glyph_cache[i].raw & CARDA_GLYPH_CACHE_USED))
        {
            g_carda_glyph_cache[i].raw = 0;
        }
    }
}

/**
 * @brief Clear the glyph cache and raster scratch buffer.
 */
void carda_reset_glyph_cache(void)
{
    s32 i;

    for (i = CARDA_GLYPH_CACHE_SLOTS - 1; i >= 0; i--)
    {
        g_carda_glyph_cache[i].raw = 0;
    }

    for (i = 0; i < CARDA_GLYPH_RASTER_BUFFER_BYTES; i++)
    {
        g_carda_glyph_raster_buffer[i] = 0;
    }
}

/**
 * @brief Expand a game-encoded string into Shift-JIS, two bytes per source
 *        character, and NUL-terminate it.
 * @param out Destination buffer.
 * @param in Source string, terminated by a 0 byte.
 * @note Lead bytes 0x19..0x1F start a two-byte code whose second byte's nibbles
 *       pick the row and column of one 16-row page of the double-byte table;
 *       bytes from 0x21 index the single-byte table by (c - 0x20); any other
 *       byte becomes the table's first (blank) glyph.
 * @note D_801629D0 is the double-byte table indexed by the raw lead byte: the
 *       table start minus 0x19 * CARDA_CHAR_TABLE_PAGE_BYTES.
 */
void carda_expand_text_glyph_codes(u8* out, u8* in)
{
    s32 index;
    s16 lead;

    while (1)
    {
        u8 c = *in;

        if (c == 0)
        {
            break;
        }
        if (c >= 0x19 && c <= 0x1F)
        {
            u32 column;
            s32 row;
            u8* first_byte;
            u8* second_byte;

            column = in[1];
            row = column >> 4;
            column &= 0xF;
            first_byte = D_801629D0 + column * 2;
            first_byte += row * CARDA_CHAR_TABLE_ROW_BYTES;
            lead = *in;
            first_byte += lead * CARDA_CHAR_TABLE_PAGE_BYTES;
            *out = *first_byte;
            out++;
            column = in[1];
            row = column >> 4;
            column &= 0xF;
            second_byte = D_801629D0 + 1 + column * 2;
            second_byte += row * CARDA_CHAR_TABLE_ROW_BYTES;
            lead = *in;
            second_byte += lead * CARDA_CHAR_TABLE_PAGE_BYTES;
            *out = *second_byte;
            out++;
            in += 2;
        }
        else if (c >= 0x21)
        {
            lead = *in;
            index = lead - 0x20;
            *out = g_carda_single_byte_char_table[(index / 16) * CARDA_CHAR_TABLE_ROW_BYTES + (index & 0xF) * 2];
            out++;
            lead = *in;
            index = lead - 0x20;
            *out = g_carda_single_byte_char_table[(index / 16) * CARDA_CHAR_TABLE_ROW_BYTES + (index & 0xF) * 2 + 1];
            out++;
            in += 1;
        }
        else
        {
            *out = g_carda_single_byte_char_table[0];
            out++;
            *out = g_carda_single_byte_char_table[1];
            out++;
            in += 1;
        }
    }
    *out = 0;
}
