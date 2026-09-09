#include "addhero_internal.h"

#define GLYPH_CHAR_TABLE_ROW_BYTES 33
#define GLYPH_CHAR_TABLE_PAGE_BYTES (16 * GLYPH_CHAR_TABLE_ROW_BYTES)

#define GLYPH_CACHE_SLOTS 0x100
#define GLYPH_CACHE_COLUMNS 16
#define GLYPH_CACHE_ROW_MASK 0xF0
#define GLYPH_RASTER_BYTES 0x80
#define GLYPH_RASTER_BUFFER_BYTES 0x8000
#define GLYPH_CACHE_USED 0x10000
#define GPU_ADDR_MASK 0xFFFFFF
#define GPU_TAG_HIGH_MASK 0xFF000000

/** @brief Cached character code and flags recording use in the current frame. */
typedef union
{
    u32 raw;
    struct
    {
        u16 code;
        u16 flags;
    } data;
} AddheroGlyphCacheEntry;

/** @brief Cached-glyph sprite packet and its trailing padding word. */
typedef struct
{
    SPRT_16 packet;
    u32 padding;
} AddheroGlyphSprite;

extern AddheroGlyphCacheEntry g_addhero_glyph_cache[];
extern u8* g_addhero_glyph_raster_cursor;
extern s32 g_addhero_glyph_cursor_x;
extern s32 g_addhero_glyph_cursor_y;
extern s32 g_addhero_text_line_start_x;
extern s32 g_addhero_glyph_upload_x;
extern s32 g_addhero_glyph_upload_y;
extern u8 g_addhero_single_byte_char_table[];
extern u8 g_addhero_double_byte_char_table[];
extern u8 g_addhero_glyph_raster_buffer[];
extern u16 g_addhero_decimal_glyphs[];
extern u16 g_addhero_hex_glyphs[];

s32 addhero_draw_signed_decimal(s32 prim, s32* ot, s32 value, s32 x, s32 y, s32 palette, s32 alignment);
void addhero_draw_hex_byte(s32 prim, s32* ot, s32 byte_value, s32 x, s32 y, s32 alignment);
s32 addhero_render_cached_glyph(s32 prim, s32* ot, s32 character_code, s32 palette);
s32 addhero_emit_glyph_sprite(AddheroGlyphSprite* sprite, s32* ot, s32 cache_slot, s32 palette);
void addhero_expand_text_glyph_codes(u8* out, u8* in);
s32 Krom2RawAdd(s32 a);

/**
 * @brief Render a signed decimal value as cached-glyph text, suppressing
 *        leading zeros and prefixing a minus glyph when negative.
 * @param prim      Current primitive pointer/index.
 * @param ot        Ordering table the glyphs are linked into.
 * @param value     Signed value to render.
 * @param x         X position (interpreted per @p alignment).
 * @param y         Y baseline.
 * @param palette   Glyph palette index.
 * @param alignment Text alignment mode passed to addhero_draw_cached_text.
 * @return The updated primitive pointer.
 * @see decomp.me (100%)
 */
s32 addhero_draw_signed_decimal(s32 prim, s32* ot, s32 value, s32 x, s32 y, s32 palette, s32 alignment)
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
    buf[1] = g_addhero_decimal_glyphs[magnitude / 10000];
    buf[2] = g_addhero_decimal_glyphs[(magnitude % 10000) / 1000];
    buf[3] = g_addhero_decimal_glyphs[(magnitude % 1000) / 100];
    buf[4] = g_addhero_decimal_glyphs[(magnitude % 100) / 10];
    buf[5] = g_addhero_decimal_glyphs[magnitude % 10];

    first_digit = 1;
    buf[6] = 0;

    while (first_digit < 5 && buf[first_digit] == 0x4F82)
    {
        first_digit++;
    }

    if (negative != 0)
    {
        first_digit--;
        buf[first_digit] = 0x5B81;
    }
    prim = addhero_draw_cached_text(prim, ot, (u8*)&buf[first_digit], x, y, palette, alignment);
    return prim;
}

/**
 * @brief Render a byte as two hex-digit glyphs via the cached-text renderer.
 * @param prim       Current primitive pointer/index.
 * @param ot         Ordering table the glyphs are linked into.
 * @param byte_value Byte value to render.
 * @param x          X position.
 * @param y          Y baseline.
 * @param alignment  Text alignment mode.
 * @see decomp.me (100%)
 */
void addhero_draw_hex_byte(s32 prim, s32* ot, s32 byte_value, s32 x, s32 y, s32 alignment)
{
    u16 pair[3];
    s32 high_digit;
    s32 glyph_offset;
    u16* glyphs;

    high_digit = byte_value / 16;
    glyph_offset = high_digit * 2;
    glyphs = g_addhero_hex_glyphs;
    pair[0] = *(u16*)((u8*)glyphs + glyph_offset);
    glyph_offset = (byte_value - high_digit * 16) * 2;
    pair[1] = *(u16*)((u8*)glyphs + glyph_offset);
    pair[2] = 0;
    addhero_draw_cached_text(prim, ot, (u8*)pair, x, y, 0, alignment);
}

/**
 * @brief Render a multibyte string through the glyph cache: measure it, apply
 *        left/center/right alignment, then emit one cached glyph per character
 *        and terminate the primitive list.
 * @param prim      Current primitive pointer/index.
 * @param ot        Ordering table the glyphs are linked into.
 * @param text      Null-terminated multibyte string to render.
 * @param x         X position (interpreted per @p alignment).
 * @param y         Y baseline.
 * @param palette   Glyph palette index.
 * @param alignment 0 left, 1 right (16px/char), 2 right (8px/char).
 * @return The updated primitive pointer past the terminator.
 * @see decomp.me (100%)
 */
s32 addhero_draw_cached_text(s32 prim, s32* ot, u8* text, s32 x, s32 y, s32 palette, s32 alignment)
{
    u8* cursor;
    s32 count;
    u16 code;
    u8* scan;

    cursor = text;
    count = 0;
    if (*cursor >= 0x20)
    {
        scan = cursor;
        do
        {
            code = *scan;
            if (code >= 0x80)
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
        x -= count * 0x10;
        break;
    case 2:
        x -= count * 8;
        break;
    case 0:
    default:
        break;
    }
    g_addhero_text_line_start_x = x;
    g_addhero_glyph_cursor_x = x;
    g_addhero_glyph_cursor_y = y;

    while (1)
    {
        u32 lead = *cursor;

        if ((u8)lead == 0x20)
        {
            cursor++;
            g_addhero_glyph_cursor_x += 0x10;
            continue;
        }
        if ((u8)lead >= 0x80)
        {
            code = cursor[0];
            code = (code << 8) | cursor[1];
            cursor += 2;
        }
        else
        {
            if ((u8)lead < 0x20)
            {
                break;
            }
            if ((u32)(lead - 0x30) < 0x50)
            {
                code = *cursor - 0x7DE1;
                cursor++;
            }
            else
            {
                code = *cursor - 0x7AE1;
                cursor++;
            }
        }
        prim = addhero_render_cached_glyph(prim, ot, code, palette);
    }

    setDrawTPage(prim, 0, 0, 5);
    addPrim(ot, prim);
    return prim + 8;
}

/**
 * @brief Emit one glyph sprite, rasterizing and uploading the glyph to the VRAM
 *        cache first when it is not already cached.
 * @param prim           Current primitive pointer/index.
 * @param ot             Ordering table the sprite is linked into.
 * @param character_code Glyph code to render.
 * @param palette        Glyph palette index used when rasterizing.
 * @return The updated primitive pointer, unchanged when the glyph is missing or
 *         the cache is full.
 * @see decomp.me (100%)
 */
s32 addhero_render_cached_glyph(s32 prim, s32* ot, s32 character_code, s32 palette)
{
    AddheroGlyphCacheEntry* entry;
    u8* font_data;
    s32 font_address;
    u32 requested_code;
    s32 slot;
    s32 high_pixel_set;
    s32 code;
    RECT rect;

    u8* raster;
    s32 color_index;
    s32 high_nibble_color;
    s32 row;
    s32 source_byte;

    u16 mask;
    volatile u8* raster_byte;
    u8 packed_pixels;

    code = character_code;
    slot = 0;
    requested_code = code & 0xFFFF;
    entry = g_addhero_glyph_cache;

    while (slot < GLYPH_CACHE_SLOTS)
    {
        if (requested_code == entry->data.code)
        {
            return addhero_emit_glyph_sprite((AddheroGlyphSprite*)prim, ot, slot, palette);
        }
        slot++;
        entry++;
    }

    font_address = Krom2RawAdd(code & 0xFFFF);
    font_data = (u8*)font_address;
    if (font_address == -1)
    {
        return prim;
    }

    raster = g_addhero_glyph_raster_cursor;
    row = 0;
    color_index = (palette + 1) * 2;
    high_nibble_color = color_index * 16;
    for (; row < 15; row++)
    {
        for (source_byte = 0; source_byte < 2; source_byte++)
        {
            mask = 0x80;

            for (slot = 0; slot < 4; slot++)
            {
                *raster = ((*font_data) & mask) ? color_index : 0;

                mask >>= 1;
                high_pixel_set = (*font_data) & mask;

                raster_byte = raster;
                packed_pixels = *raster_byte;
                if (high_pixel_set)
                {
                    packed_pixels += high_nibble_color;
                }

                *raster_byte = packed_pixels;

                mask >>= 1;
                raster++;
            }

            font_data++;
        }
    }

    slot = 0;
    while ((slot < GLYPH_CACHE_SLOTS) && (g_addhero_glyph_cache[slot].raw != 0))
    {
        slot++;
    }

    if (slot == GLYPH_CACHE_SLOTS)
    {
        return prim;
    }
    g_addhero_glyph_cache[slot].raw = code & 0xFFFF;
    prim = addhero_emit_glyph_sprite((AddheroGlyphSprite*)prim, ot, slot, palette);

    g_addhero_glyph_upload_x = (slot % GLYPH_CACHE_COLUMNS) * 4;
    g_addhero_glyph_upload_y = slot & GLYPH_CACHE_ROW_MASK;

    rect.w = 4;
    rect.h = 15;
    rect.x = g_addhero_glyph_upload_x + 0x140;
    rect.y = g_addhero_glyph_upload_y;

    LoadImage(&rect, (u_long*)g_addhero_glyph_raster_cursor);
    DrawSync(0);

    g_addhero_glyph_raster_cursor += GLYPH_RASTER_BYTES;
    return prim;
}

/**
 * @brief Write a 16x16 sprite for a cached glyph at the current text cursor,
 *        mark the slot used, and advance the cursor (wrapping to the next line).
 * @param sprite     Destination sprite primitive.
 * @param ot         Ordering table the sprite is linked into.
 * @param cache_slot Glyph cache slot whose VRAM tile to sample.
 * @param palette    Unused here; the CLUT is fixed.
 * @return The primitive pointer advanced past the emitted sprite.
 * @see decomp.me (100%)
 */
s32 addhero_emit_glyph_sprite(AddheroGlyphSprite* sprite, s32* ot, s32 cache_slot, s32 palette)
{
    u32 ot_tag_high_byte;
    s32 normalized_slot;
    u32 packet_address;
    s32 old_x;
    s32 new_x;
    s32 fits_line;

    g_addhero_glyph_cache[cache_slot].raw |= GLYPH_CACHE_USED;

    setSprt16(sprite);
    sprite->packet.g0 = 0x80;
    sprite->packet.b0 = 0x80;
    sprite->packet.r0 = 0x80;
    normalized_slot = cache_slot;
    setXY0(&sprite->packet, g_addhero_glyph_cursor_x, g_addhero_glyph_cursor_y);

    if (cache_slot < 0)
    {
        normalized_slot = cache_slot + 15;
    }

    setUV0(&sprite->packet, (cache_slot - ((normalized_slot >> 4) * 16)) * 16, cache_slot & GLYPH_CACHE_ROW_MASK);
    sprite->packet.clut = 0x7FD3;
    sprite->packet.tag = (sprite->packet.tag & GPU_TAG_HIGH_MASK) | (*ot & GPU_ADDR_MASK);

    packet_address = ((u32)sprite) & GPU_ADDR_MASK;
    ot_tag_high_byte = *ot & GPU_TAG_HIGH_MASK;

    sprite++;
    old_x = g_addhero_glyph_cursor_x;
    new_x = old_x + 16;
    fits_line = (old_x + 32) < 0x280;
    g_addhero_glyph_cursor_x = new_x;

    *ot = ot_tag_high_byte | packet_address;

    if (!fits_line)
    {
        g_addhero_glyph_cursor_x = g_addhero_text_line_start_x;
        g_addhero_glyph_cursor_y += 16;
    }

    return (s32)sprite;
}

/**
 * @brief Start a new glyph cache frame: rewind the raster cursor and clear each
 *        cache entry's per-frame "used" flag (the high half-word).
 * @see decomp.me (100.00%)
 */
void addhero_begin_glyph_cache_frame(void)
{
    s32 slot;
    AddheroGlyphCacheEntry* entry;

    g_addhero_glyph_raster_cursor = g_addhero_glyph_raster_buffer;
    for (slot = 0, entry = g_addhero_glyph_cache; slot < GLYPH_CACHE_SLOTS; slot++, entry++)
    {
        entry->raw = (u16)entry->raw;
    }
}

/**
 * @brief Evict cache entries not touched this frame by zeroing any slot whose
 *        "used" flag (bit 0x10000) is clear.
 * @see decomp.me (100.00%)
 */
void addhero_evict_unused_glyphs(void)
{
    s32 slot;
    AddheroGlyphCacheEntry* entry;
    s32 used_flag;

    slot = 0;
    used_flag = GLYPH_CACHE_USED;
    entry = g_addhero_glyph_cache;
    for (; slot < GLYPH_CACHE_SLOTS; slot++, entry++)
    {
        if (!(entry->raw & used_flag))
        {
            entry->raw = 0;
        }
    }
}

/**
 * @brief Fully reset the glyph cache: zero all 0x100 cache entries and clear the
 *        entire 0x8000-byte glyph raster buffer.
 * @see decomp.me (100.00%)
 */
void addhero_reset_glyph_cache(void)
{
    s32 slot;
    AddheroGlyphCacheEntry* entry;
    u8* raster;

    slot = GLYPH_CACHE_SLOTS - 1;
    entry = g_addhero_glyph_cache;
    entry += GLYPH_CACHE_SLOTS - 1;
    for (; slot >= 0; slot--, entry--)
    {
        entry->raw = 0;
    }

    slot = 0;
    raster = g_addhero_glyph_raster_buffer;
    for (; slot < GLYPH_RASTER_BUFFER_BYTES; slot++)
    {
        *(u8*)(slot + (s32)raster) = 0;
    }
}

/**
 * @brief Translate a source string into internal glyph codes via the single-
 *        and double-byte character tables, writing two output bytes per input
 *        character and null-terminating the result.
 * @param out Destination glyph-code buffer.
 * @param in  Null-terminated source string.
 * @see decomp.me (100%)
 */
void addhero_expand_text_glyph_codes(u8* out, u8* in)
{
    u32 character;
    s32 character_index;
    s16 character_code;

    for (;;)
    {
        character = *in;
        if ((u8)character == 0)
        {
            goto done;
        }
        if ((u32)(character - 0x19) < 7)
        {
            u32 trail_byte;
            s32 row_index;
            u8* first_byte;
            u8* second_byte;

            trail_byte = in[1];
            row_index = trail_byte >> 4;
            trail_byte &= 0xF;
            first_byte = g_addhero_double_byte_char_table + trail_byte * 2;
            first_byte += row_index * GLYPH_CHAR_TABLE_ROW_BYTES;
            character_code = *in;
            first_byte += character_code * GLYPH_CHAR_TABLE_PAGE_BYTES;
            *out = *first_byte;
            out++;
            trail_byte = in[1];
            row_index = trail_byte >> 4;
            trail_byte &= 0xF;
            second_byte = g_addhero_double_byte_char_table + 1 + trail_byte * 2;
            second_byte += row_index * GLYPH_CHAR_TABLE_ROW_BYTES;
            character_code = *in;
            second_byte += character_code * GLYPH_CHAR_TABLE_PAGE_BYTES;
            *out = *second_byte;
            out++;
            in += 2;
        }
        else if ((u8)character >= 0x21)
        {
            character_code = *in;
            character_index = character_code - 0x20;
            *out = g_addhero_single_byte_char_table[(character_index / 16) * GLYPH_CHAR_TABLE_ROW_BYTES + (character_index & 0xF) * 2];
            out++;
            character_code = *in;
            character_index = character_code - 0x20;
            *out = g_addhero_single_byte_char_table[(character_index / 16) * GLYPH_CHAR_TABLE_ROW_BYTES + (character_index & 0xF) * 2 + 1];
            out++;
            in += 1;
        }
        else
        {
            *out = g_addhero_single_byte_char_table[0];
            out++;
            *out = g_addhero_single_byte_char_table[1];
            out++;
            in += 1;
        }
    }
done:
    *out = 0;
}
