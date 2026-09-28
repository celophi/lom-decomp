#include "field_text.h"
#include "addhero_internal.h"

/* Shift-JIS glyph pairs as stored in the little-endian u16 digit tables. */
#define ADDHERO_GLYPH_PAIR_ZERO 0x4F82
#define ADDHERO_GLYPH_PAIR_MINUS 0x5B81

/* Added to an ASCII byte to get its Shift-JIS glyph code. */
#define ADDHERO_SJIS_ALNUM_OFFSET 0x821F
#define ADDHERO_SJIS_SYMBOL_OFFSET 0x851F

/* Glyph code tables: 33-byte rows of 16 two-byte glyphs plus a terminator. */
#define ADDHERO_CHAR_TABLE_ROW_BYTES 33
#define ADDHERO_CHAR_TABLE_COLUMNS 16
#define ADDHERO_CHAR_TABLE_PAGE_BYTES (16 * ADDHERO_CHAR_TABLE_ROW_BYTES)

#define ADDHERO_GLYPH_CACHE_SLOTS 0x100
#define ADDHERO_GLYPH_CACHE_COLUMNS 16
#define ADDHERO_GLYPH_CACHE_ROW_MASK 0xF0
#define ADDHERO_GLYPH_RASTER_BYTES 0x80
#define ADDHERO_GLYPH_RASTER_BUFFER_BYTES 0x8000
#define ADDHERO_GLYPH_CACHE_USED 0x10000
#define ADDHERO_GLYPH_CACHE_CODE_MASK 0xFFFF

/** @brief Glyph cell: ADDHERO_GLYPH_SIZE square on screen, ADDHERO_GLYPH_ROWS rows of Kanji ROM bitmap. */
#define ADDHERO_GLYPH_SIZE 16
#define ADDHERO_GLYPH_ROWS 15

/** @brief VRAM x of the glyph cache (4-bit texels, so a glyph is ADDHERO_GLYPH_SIZE / 4 halfwords wide). */
#define ADDHERO_GLYPH_VRAM_X 320

/** @brief CLUT the cached glyphs are drawn with: FIELD's text CLUT. */
#define ADDHERO_GLYPH_CLUT_X 304
#define ADDHERO_GLYPH_CLUT_Y 511

/** @brief Pen x at which cached text wraps to the next line. */
#define ADDHERO_TEXT_WRAP_X 640

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
/**
 * @brief Base the two-byte character pages are reached from: page @c lead starts at
 *        this address + lead * ADDHERO_CHAR_TABLE_PAGE_BYTES, inside the character chart.
 * @note Not a table of its own; the address itself falls inside the icon set.
 */
extern u8 g_addhero_char_page_base[];
extern u8 g_addhero_glyph_raster_buffer[];
extern u16 g_addhero_decimal_glyphs[];
extern u16 g_addhero_hex_glyphs[];

void* addhero_draw_signed_decimal(void* prim, u_long* ot, s32 value, s32 x, s32 y, s32 palette, s32 alignment);
void addhero_draw_hex_byte(void* prim, u_long* ot, s32 value, s32 x, s32 y, s32 alignment);
void* addhero_render_cached_glyph(void* prim, u_long* ot, u16 code, s32 palette);
void* addhero_emit_glyph_sprite(AddheroGlyphSprite* sprite, u_long* ot, s32 cache_slot, s32 palette);
void addhero_expand_text_glyph_codes(u8* out, u8* in);

/**
 * @brief Render a signed decimal value as cached-glyph text, suppressing
 *        leading zeros and prefixing a minus glyph when negative.
 * @param prim      Current primitive cursor.
 * @param ot        Ordering table the glyphs are linked into.
 * @param value     Signed value to render.
 * @param x         X position (interpreted per @p alignment).
 * @param y         Y baseline.
 * @param palette   Glyph palette index.
 * @param alignment Text alignment mode passed to addhero_draw_cached_text.
 * @return The updated primitive cursor.
 */
void* addhero_draw_signed_decimal(void* prim, u_long* ot, s32 value, s32 x, s32 y, s32 palette, s32 alignment)
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

    while (first_digit < 5 && buf[first_digit] == ADDHERO_GLYPH_PAIR_ZERO)
    {
        first_digit++;
    }

    if (negative != 0)
    {
        first_digit--;
        buf[first_digit] = ADDHERO_GLYPH_PAIR_MINUS;
    }
    prim = addhero_draw_cached_text(prim, ot, (u8*)&buf[first_digit], x, y, palette, alignment);
    return prim;
}

/**
 * @brief Render a byte as two hex-digit glyphs via the cached-text renderer.
 * @param prim      Current primitive cursor.
 * @param ot        Ordering table the glyphs are linked into.
 * @param value     Byte value to render.
 * @param x         X position.
 * @param y         Y baseline.
 * @param alignment Text alignment mode.
 */
void addhero_draw_hex_byte(void* prim, u_long* ot, s32 value, s32 x, s32 y, s32 alignment)
{
    u16 buf[3];
    u16* high_glyph;

    high_glyph = &g_addhero_hex_glyphs[value / 16];
    buf[0] = *high_glyph;
    buf[1] = g_addhero_hex_glyphs[value % 16];
    buf[2] = 0;
    addhero_draw_cached_text(prim, ot, (u8*)buf, x, y, 0, alignment);
}

/**
 * @brief Render a multibyte string through the glyph cache: measure it, apply
 *        left/center/right alignment, then emit one cached glyph per character
 *        and terminate the primitive list.
 * @param prim      Current primitive cursor.
 * @param ot        Ordering table the glyphs are linked into.
 * @param text      Null-terminated multibyte string to render.
 * @param x         X position (interpreted per @p alignment).
 * @param y         Y baseline.
 * @param palette   Glyph palette index.
 * @param alignment FIELD_TEXT_ALIGN_LEFT, FIELD_TEXT_ALIGN_RIGHT or FIELD_TEXT_ALIGN_CENTER; every glyph is ADDHERO_GLYPH_SIZE wide.
 * @return The updated primitive cursor past the terminator.
 */
void* addhero_draw_cached_text(void* prim, u_long* ot, u8* text, s32 x, s32 y, s32 palette, s32 alignment)
{
    u8* cursor;
    s32 count;
    u16 code;
    u8* scan;
    DR_TPAGE* draw_mode;

    cursor = text;
    count = 0;
    if (*cursor >= ADDHERO_TEXT_FIRST_PRINTABLE)
    {
        for (scan = cursor; *scan >= ADDHERO_TEXT_FIRST_PRINTABLE; scan++)
        {
            if (*scan >= ADDHERO_SJIS_LEAD_MIN)
            {
                scan++;
            }
            count++;
        }
    }

    switch (alignment)
    {
    case FIELD_TEXT_ALIGN_RIGHT:
        x -= count * ADDHERO_GLYPH_SIZE;
        break;
    case FIELD_TEXT_ALIGN_CENTER:
        x -= count * (ADDHERO_GLYPH_SIZE / 2);
        break;
    case FIELD_TEXT_ALIGN_LEFT:
    default:
        break;
    }
    g_addhero_text_line_start_x = x;
    g_addhero_glyph_cursor_x = x;
    g_addhero_glyph_cursor_y = y;

    while (1)
    {
        if (*cursor == ' ')
        {
            cursor++;
            g_addhero_glyph_cursor_x += ADDHERO_GLYPH_SIZE;
            continue;
        }
        if (*cursor >= ADDHERO_SJIS_LEAD_MIN)
        {
            code = cursor[0];
            code = (code << 8) | cursor[1];
            cursor += 2;
        }
        else
        {
            if (*cursor < ADDHERO_TEXT_FIRST_PRINTABLE)
            {
                break;
            }
            if (*cursor >= '0' && *cursor < ADDHERO_SJIS_LEAD_MIN)
            {
                code = *cursor + ADDHERO_SJIS_ALNUM_OFFSET;
                cursor++;
            }
            else
            {
                code = *cursor + ADDHERO_SJIS_SYMBOL_OFFSET;
                cursor++;
            }
        }
        prim = addhero_render_cached_glyph(prim, ot, code, palette);
    }

    draw_mode = prim;
    setDrawTPage(draw_mode, 0, 0, getTPage(0, 0, ADDHERO_GLYPH_VRAM_X, 0));
    addPrim(ot, draw_mode);
    return draw_mode + 1;
}

/**
 * @brief Emit one glyph sprite, rasterizing and uploading the glyph to the VRAM
 *        cache first when it is not already cached.
 * @param prim    Current primitive cursor.
 * @param ot      Ordering table the sprite is linked into.
 * @param code    Shift-JIS glyph code to render.
 * @param palette Glyph palette index; sets the 4-bit pixel value (palette + 1) * 2.
 * @return The updated primitive cursor, unchanged when the glyph is missing or
 *         the cache is full.
 */
void* addhero_render_cached_glyph(void* prim, u_long* ot, u16 code, s32 palette)
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

    for (i = 0; i < ADDHERO_GLYPH_CACHE_SLOTS; i++)
    {
        if (code == g_addhero_glyph_cache[i].data.code)
        {
            return addhero_emit_glyph_sprite(prim, ot, i, palette);
        }
    }

    font_data = (u8*)Krom2RawAdd(code);
    if (font_data == (u8*)-1)
    {
        return prim;
    }

    raster = g_addhero_glyph_raster_cursor;
    row = 0;
    color_index = (palette + 1) * 2;
    high_nibble_color = color_index * 16;
    for (; row < ADDHERO_GLYPH_ROWS; row++)
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

    for (i = 0; i < ADDHERO_GLYPH_CACHE_SLOTS; i++)
    {
        if (g_addhero_glyph_cache[i].raw == 0)
        {
            break;
        }
    }

    if (i == ADDHERO_GLYPH_CACHE_SLOTS)
    {
        return prim;
    }
    g_addhero_glyph_cache[i].raw = code;
    prim = addhero_emit_glyph_sprite(prim, ot, i, palette);

    g_addhero_glyph_upload_x = (i % ADDHERO_GLYPH_CACHE_COLUMNS) * (ADDHERO_GLYPH_SIZE / 4);
    g_addhero_glyph_upload_y = i & ADDHERO_GLYPH_CACHE_ROW_MASK;

    setWH(&rect, ADDHERO_GLYPH_SIZE / 4, ADDHERO_GLYPH_ROWS);
    rect.x = g_addhero_glyph_upload_x + ADDHERO_GLYPH_VRAM_X;
    rect.y = g_addhero_glyph_upload_y;

    LoadImage(&rect, (u_long*)g_addhero_glyph_raster_cursor);
    DrawSync(0);

    g_addhero_glyph_raster_cursor += ADDHERO_GLYPH_RASTER_BYTES;
    return prim;
}

/**
 * @brief Write a 16x16 sprite for a cached glyph at the current text cursor,
 *        mark the slot used, and advance the cursor (wrapping to the next line).
 * @param sprite     Destination sprite primitive.
 * @param ot         Ordering table the sprite is linked into.
 * @param cache_slot Glyph cache slot whose VRAM tile to sample.
 * @param palette    Unused here; the CLUT is fixed.
 * @return The primitive cursor advanced past the emitted sprite.
 */
void* addhero_emit_glyph_sprite(AddheroGlyphSprite* sprite, u_long* ot, s32 cache_slot, s32 palette)
{
    g_addhero_glyph_cache[cache_slot].raw |= ADDHERO_GLYPH_CACHE_USED;

    setSprt16(&sprite->packet);
    sprite->packet.g0 = 0x80;
    sprite->packet.b0 = 0x80;
    sprite->packet.r0 = 0x80;
    setXY0(&sprite->packet, g_addhero_glyph_cursor_x, g_addhero_glyph_cursor_y);
    setUV0(&sprite->packet, (cache_slot % ADDHERO_GLYPH_CACHE_COLUMNS) * ADDHERO_GLYPH_SIZE, cache_slot & ADDHERO_GLYPH_CACHE_ROW_MASK);
    sprite->packet.clut = getClut(ADDHERO_GLYPH_CLUT_X, ADDHERO_GLYPH_CLUT_Y);
    addPrim(ot, &sprite->packet);
    sprite++;

    g_addhero_glyph_cursor_x += ADDHERO_GLYPH_SIZE;
    if (g_addhero_glyph_cursor_x + ADDHERO_GLYPH_SIZE >= ADDHERO_TEXT_WRAP_X)
    {
        g_addhero_glyph_cursor_x = g_addhero_text_line_start_x;
        g_addhero_glyph_cursor_y += ADDHERO_GLYPH_SIZE;
    }

    return sprite;
}

/**
 * @brief Start a new glyph cache frame: rewind the raster cursor and clear each
 *        cache entry's per-frame "used" flag (the high half-word).
 */
void addhero_begin_glyph_cache_frame(void)
{
    s32 i;

    g_addhero_glyph_raster_cursor = g_addhero_glyph_raster_buffer;
    for (i = 0; i < ADDHERO_GLYPH_CACHE_SLOTS; i++)
    {
        g_addhero_glyph_cache[i].raw &= ADDHERO_GLYPH_CACHE_CODE_MASK;
    }
}

/**
 * @brief Evict cache entries not touched this frame by zeroing any slot whose
 *        "used" flag (ADDHERO_GLYPH_CACHE_USED) is clear.
 */
void addhero_evict_unused_glyphs(void)
{
    s32 i;

    for (i = 0; i < ADDHERO_GLYPH_CACHE_SLOTS; i++)
    {
        if (!(g_addhero_glyph_cache[i].raw & ADDHERO_GLYPH_CACHE_USED))
        {
            g_addhero_glyph_cache[i].raw = 0;
        }
    }
}

/**
 * @brief Fully reset the glyph cache: zero all cache entries and clear the
 *        entire glyph raster buffer.
 */
void addhero_reset_glyph_cache(void)
{
    s32 i;

    for (i = ADDHERO_GLYPH_CACHE_SLOTS - 1; i >= 0; i--)
    {
        g_addhero_glyph_cache[i].raw = 0;
    }

    for (i = 0; i < ADDHERO_GLYPH_RASTER_BUFFER_BYTES; i++)
    {
        g_addhero_glyph_raster_buffer[i] = 0;
    }
}

/**
 * @brief Translate a source string into internal glyph codes via the single-
 *        and double-byte character tables, writing two output bytes per input
 *        character and null-terminating the result.
 * @param out Destination glyph-code buffer.
 * @param in  Null-terminated source string.
 * @note Lead bytes 0x19-0x1F start a two-byte code whose second byte's nibbles
 *       pick the row and column of one 16-row page of the double-byte table;
 *       bytes above ADDHERO_TEXT_FIRST_PRINTABLE index the single-byte table by their offset from it; any other
 *       byte becomes the table's first (blank) glyph.
 */
void addhero_expand_text_glyph_codes(u8* out, u8* in)
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
        if (ENCODED_TEXT_IS_DOUBLE_BYTE_LEAD(c))
        {
            u32 column;
            s32 row;
            u8* first_byte;
            u8* second_byte;

            column = in[1];
            row = column >> 4;
            column &= 0xF;
            first_byte = g_addhero_char_page_base + column * 2;
            first_byte += row * ADDHERO_CHAR_TABLE_ROW_BYTES;
            lead = *in;
            first_byte += lead * ADDHERO_CHAR_TABLE_PAGE_BYTES;
            *out = *first_byte;
            out++;
            column = in[1];
            row = column >> 4;
            column &= 0xF;
            second_byte = g_addhero_char_page_base + 1 + column * 2;
            second_byte += row * ADDHERO_CHAR_TABLE_ROW_BYTES;
            lead = *in;
            second_byte += lead * ADDHERO_CHAR_TABLE_PAGE_BYTES;
            *out = *second_byte;
            out++;
            in += 2;
        }
        else if (c > ADDHERO_TEXT_FIRST_PRINTABLE)
        {
            lead = *in;
            index = lead - ADDHERO_TEXT_FIRST_PRINTABLE;
            *out = g_addhero_single_byte_char_table[(index / ADDHERO_CHAR_TABLE_COLUMNS) * ADDHERO_CHAR_TABLE_ROW_BYTES +
                                                    (index & (ADDHERO_CHAR_TABLE_COLUMNS - 1)) * 2];
            out++;
            lead = *in;
            index = lead - ADDHERO_TEXT_FIRST_PRINTABLE;
            *out = g_addhero_single_byte_char_table[(index / ADDHERO_CHAR_TABLE_COLUMNS) * ADDHERO_CHAR_TABLE_ROW_BYTES +
                                                    (index & (ADDHERO_CHAR_TABLE_COLUMNS - 1)) * 2 + 1];
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
    *out = 0;
}
