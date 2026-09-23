#include "checkps_internal.h"

#include "display.h"
#include "sdk/libapi.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

#define CHECKPS_GLYPH_CACHE_ENTRY_COUNT 256
#define CHECKPS_GLYPH_CACHE_USED_FLAG 0x10000
#define CHECKPS_GLYPH_CACHE_CODE_MASK 0xFFFF
#define CHECKPS_GLYPH_CLUT_Y (VRAM_HEIGHT - 1)
#define CHECKPS_GLYPH_WIDTH 16
#define CHECKPS_GLYPH_VRAM_WORD_WIDTH 4
#define CHECKPS_GLYPH_RASTER_SLOT_SIZE 0x80
#define CHECKPS_GLYPH_RASTER_BUFFER_SIZE (CHECKPS_GLYPH_CACHE_ENTRY_COUNT * CHECKPS_GLYPH_RASTER_SLOT_SIZE)
#define CHECKPS_GLYPH_V_COORD_MASK 0xF0
#define CHECKPS_GLYPH_SOURCE_MSB 0x80
#define CHECKPS_GLYPH_NEUTRAL_COLOR 0x80
#define CHECKPS_TEXT_WRAP_LIMIT (SCREEN_WIDTH * 2)
#define CHECKPS_TEXT_FIRST_PRINTABLE 0x20
#define CHECKPS_TEXT_SPACE 0x20
#define CHECKPS_SJIS_LEAD_BYTE_THRESHOLD 0x80
#define CHECKPS_SJIS_FULLWIDTH_ZERO 0x4F82
#define CHECKPS_SJIS_MINUS 0x5B81
#define CHECKPS_ASCII_TO_SJIS_OFFSET 0x851F
#define CHECKPS_HEX_RADIX 16
#define CHECKPS_DECIMAL_FIRST_DIGIT 1
#define CHECKPS_DECIMAL_DIGIT_COUNT 5
#define CHECKPS_DECIMAL_TERMINATOR_INDEX 6
#define CHECKPS_DECIMAL_GLYPH_BUFFER_SIZE 7
#define CHECKPS_DEFAULT_GLYPH_PALETTE 0
#define CHECKPS_INVALID_KROM_ADDRESS (-1)

/** @brief Horizontal positioning modes accepted by the text renderer. */
typedef enum
{
    CHECKPS_TEXT_ALIGN_LEFT = 0,
    CHECKPS_TEXT_ALIGN_RIGHT = 1,
    CHECKPS_TEXT_ALIGN_CENTER = 2
} CheckPSTextAlignment;

/**
 * @brief Character code and per-frame usage state for one glyph cache slot.
 */
typedef union
{
    u32 raw;
    struct
    {
        u16 character_code;
        u16 flags; /* Bit 0 marks the slot as used this frame. */
    } data;
} GlyphCacheEntry;

/**
 * @brief CPU packet-buffer slot used to draw one cached 16-by-16 glyph.
 *
 * The GPU consumes only the sprite. The trailing word preserves the original
 * 20-byte spacing between consecutive glyph packets in the CPU buffer.
 */
typedef struct
{
    SPRT_16 sprite;
    u32 padding;
} CheckPSGlyphPacket;

/**
 * @brief Shift-JIS full-width digit codes '0'-'9', zero-terminated.
 */
extern u16 g_decimal_glyph_table[12];

/**
 * @brief Shift-JIS full-width hex digit codes '0'-'9', 'A'-'F', zero-terminated.
 */
extern u16 g_hex_glyph_table[18];

void* draw_signed_decimal(void* primitive, u_long* ot_tag, s32 value, s32 x, s32 y, s32 palette, s32 alignment);
void draw_hex_byte(void* primitive, u_long* ot_tag, s32 value, s32 x, s32 y, s32 alignment);
void* draw_cached_text(void* primitive, u_long* ot_tag, const u8* text, s32 x, s32 y, s32 palette, s32 alignment);
void* render_cached_glyph(void* primitive, u_long* ot_tag, u16 character_code, s32 palette);
CheckPSGlyphPacket* emit_glyph_sprite(CheckPSGlyphPacket* packet, u_long* ot_tag, s32 cache_slot);

/**
 * @brief Initialized prefix of the 16-color glyph CLUT.
 *
 * LoadImage reads 16 entries starting here. The remaining entries come from
 * adjacent zero-initialized overlay storage, preserving the target layout.
 */
extern u_long g_glyph_clut_prefix[3];

/** CPU-side staging storage for all unpacked 4bpp glyph rasters. */
u8 g_glyph_raster_buffer[CHECKPS_GLYPH_RASTER_BUFFER_SIZE];

/**
 * Cached character-code slots for the 16x16 text renderer.
 * Bit 16 is a per-frame usage mark, not a persistent cache-validity bit.
 */
GlyphCacheEntry g_glyph_cache[CHECKPS_GLYPH_CACHE_ENTRY_COUNT];

/** Screen-space position of the next glyph. */
s32 g_glyph_cursor_x;

/** Screen-space baseline of the current text line. */
s32 g_glyph_cursor_y;

/** Next free 4bpp glyph block in the CPU-side staging buffer. */
u8* g_glyph_raster_cursor;

/** Screen-space X coordinate restored when text wraps. */
s32 g_text_line_start_x;

/**
 * VRAM X coordinate used for the most recently uploaded glyph slot.
 */
s32 g_glyph_upload_x;

/**
 * VRAM Y coordinate used for the most recently uploaded glyph slot.
 */
s32 g_glyph_upload_y;

/**
 * @brief Format and draw a signed decimal value with cached glyphs.
 * @param primitive Primitive-buffer cursor.
 * @param ot_tag Ordering-table tag to append to.
 * @param value Signed value whose magnitude fits in five decimal digits.
 * @param x Screen-space x coordinate.
 * @param y Screen-space y coordinate.
 * @param palette Glyph palette index.
 * @param alignment One of the CheckPSTextAlignment values.
 * @return Updated primitive-buffer cursor.
 */
void* draw_signed_decimal(void* primitive, u_long* ot_tag, s32 value, s32 x, s32 y, s32 palette, s32 alignment)
{
    u16 glyph_buffer[CHECKPS_DECIMAL_GLYPH_BUFFER_SIZE];
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
    glyph_buffer[1] = g_decimal_glyph_table[magnitude / 10000];
    glyph_buffer[2] = g_decimal_glyph_table[(magnitude % 10000) / 1000];
    glyph_buffer[3] = g_decimal_glyph_table[(magnitude % 1000) / 100];
    glyph_buffer[4] = g_decimal_glyph_table[(magnitude % 100) / 10];
    glyph_buffer[5] = g_decimal_glyph_table[magnitude % 10];

    first_digit = CHECKPS_DECIMAL_FIRST_DIGIT;

    glyph_buffer[CHECKPS_DECIMAL_TERMINATOR_INDEX] = 0;

    while (first_digit < CHECKPS_DECIMAL_DIGIT_COUNT && glyph_buffer[first_digit] == CHECKPS_SJIS_FULLWIDTH_ZERO)
    {
        first_digit++;
    }

    if (negative != 0)
    {
        first_digit--;
        glyph_buffer[first_digit] = CHECKPS_SJIS_MINUS;
    }
    primitive = draw_cached_text(primitive, ot_tag, (const u8*)&glyph_buffer[first_digit], x, y, palette, alignment);
    return primitive;
}

/**
 * @brief Draw a two-digit hexadecimal value with cached glyphs.
 * @param primitive Primitive-buffer cursor.
 * @param ot_tag Ordering-table tag to append to.
 * @param value Value in the expected two-digit hexadecimal range.
 * @param x Screen-space x coordinate.
 * @param y Screen-space y coordinate.
 * @param alignment One of the CheckPSTextAlignment values.
 */
void draw_hex_byte(void* primitive, u_long* ot_tag, s32 value, s32 x, s32 y, s32 alignment)
{
    u16 glyph_buffer[3];
    u16* high_glyph;

    high_glyph = &g_hex_glyph_table[value / CHECKPS_HEX_RADIX];
    glyph_buffer[0] = *high_glyph;
    glyph_buffer[1] = g_hex_glyph_table[value % CHECKPS_HEX_RADIX];
    glyph_buffer[2] = 0;
    draw_cached_text(primitive, ot_tag, (const u8*)glyph_buffer, x, y, CHECKPS_DEFAULT_GLYPH_PALETTE, alignment);
}

/**
 * @brief Draw a cached Shift-JIS text string into the ordering table.
 * @param primitive Primitive-buffer cursor.
 * @param ot_tag Ordering-table tag to append to.
 * @param text Encoded text terminated by a byte below 0x20.
 * @param x Screen-space x coordinate.
 * @param y Screen-space y coordinate.
 * @param palette Glyph palette index.
 * @param alignment One of the CheckPSTextAlignment values.
 * @return Updated primitive-buffer cursor.
 */
void* draw_cached_text(void* primitive, u_long* ot_tag, const u8* text, s32 x, s32 y, s32 palette, s32 alignment)
{
    const u8* cursor;
    s32 glyph_count;
    u16 character_code;
    const u8* scan;
    DR_TPAGE* draw_mode_packet;

    cursor = text;
    glyph_count = 0;
    if (*cursor >= CHECKPS_TEXT_FIRST_PRINTABLE)
    {
        scan = cursor;
        do
        {
            if (*scan >= CHECKPS_SJIS_LEAD_BYTE_THRESHOLD)
            {
                scan++;
            }
            scan++;
            glyph_count++;
        } while (*scan >= CHECKPS_TEXT_FIRST_PRINTABLE);
    }

    switch (alignment)
    {
    case CHECKPS_TEXT_ALIGN_RIGHT:
        x -= glyph_count * CHECKPS_GLYPH_WIDTH;
        break;
    case CHECKPS_TEXT_ALIGN_CENTER:
        x -= glyph_count * (CHECKPS_GLYPH_WIDTH / 2);
        break;
    case CHECKPS_TEXT_ALIGN_LEFT:
    default:
        break;
    }
    g_text_line_start_x = x;
    g_glyph_cursor_x = x;
    g_glyph_cursor_y = y;

    while (1)
    {
        if (*cursor == CHECKPS_TEXT_SPACE)
        {
            cursor++;
            g_glyph_cursor_x += CHECKPS_GLYPH_WIDTH;
            continue;
        }
        if (*cursor >= CHECKPS_SJIS_LEAD_BYTE_THRESHOLD)
        {
            character_code = cursor[0];
            character_code = (character_code << 8) | cursor[1];
            cursor += 2;
        }
        else
        {
            if (*cursor < CHECKPS_TEXT_FIRST_PRINTABLE)
            {
                break;
            }
            character_code = *cursor + CHECKPS_ASCII_TO_SJIS_OFFSET;
            cursor++;
        }
        primitive = render_cached_glyph(primitive, ot_tag, character_code, palette);
    }

    draw_mode_packet = primitive;
    setDrawTPage(draw_mode_packet, 0, 0, getTPage(0, 0, CHECKPS_GLYPH_VRAM_X, 0));
    addPrim(ot_tag, draw_mode_packet);
    return draw_mode_packet + 1;
}

/**
 * @brief Resolve one glyph in the cache, uploading it to VRAM when necessary.
 * @param primitive Primitive-buffer cursor.
 * @param ot_tag Ordering-table tag to append to.
 * @param character_code Shift-JIS character code.
 * @param palette Glyph palette index.
 * @return Updated primitive-buffer cursor.
 */
void* render_cached_glyph(void* primitive, u_long* ot_tag, u16 character_code, s32 palette)
{
    u8* font_data;
    u8* raster;
    s32 slot;
    s32 row;
    s32 source_byte;
    s32 color_index;
    s32 high_nibble_color;
    u16 mask;
    RECT rect;

    for (slot = 0; slot < CHECKPS_GLYPH_CACHE_ENTRY_COUNT; slot++)
    {
        if (character_code == g_glyph_cache[slot].data.character_code)
        {
            return emit_glyph_sprite(primitive, ot_tag, slot);
        }
    }

    /* Psy-Q exposes the KROM pointer as a signed integer address. */
    font_data = (u8*)Krom2RawAdd(character_code);
    if (font_data == (u8*)CHECKPS_INVALID_KROM_ADDRESS)
    {
        return primitive;
    }

    raster = g_glyph_raster_cursor;
    row = 0;
    color_index = palette + 1;
    high_nibble_color = color_index * CHECKPS_GLYPH_WIDTH;
    for (; row < CHECKPS_GLYPH_BITMAP_ROWS; row++)
    {
        for (source_byte = 0; source_byte < 2; source_byte++)
        {
            mask = CHECKPS_GLYPH_SOURCE_MSB;
            for (slot = 0; slot < 4; slot++)
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

    for (slot = 0; slot < CHECKPS_GLYPH_CACHE_ENTRY_COUNT; slot++)
    {
        if (g_glyph_cache[slot].raw == 0)
        {
            break;
        }
    }

    if (slot == CHECKPS_GLYPH_CACHE_ENTRY_COUNT)
    {
        return primitive;
    }
    g_glyph_cache[slot].raw = character_code;
    primitive = emit_glyph_sprite(primitive, ot_tag, slot);

    g_glyph_upload_x = (slot % CHECKPS_GLYPH_WIDTH) * CHECKPS_GLYPH_VRAM_WORD_WIDTH;
    g_glyph_upload_y = slot & CHECKPS_GLYPH_V_COORD_MASK;

    setWH(&rect, CHECKPS_GLYPH_VRAM_WORD_WIDTH, CHECKPS_GLYPH_BITMAP_ROWS);
    rect.x = g_glyph_upload_x + CHECKPS_GLYPH_VRAM_X;
    rect.y = g_glyph_upload_y;

    LoadImage(&rect, (u_long*)g_glyph_raster_cursor);
    DrawSync(0);

    g_glyph_raster_cursor += CHECKPS_GLYPH_RASTER_SLOT_SIZE;
    return primitive;
}

/**
 * @brief Emit a 16x16 sprite for one cached glyph slot.
 * @param packet Sprite packet to initialize.
 * @param ot_tag Ordering-table tag to append to.
 * @param cache_slot Glyph-cache slot index.
 * @return Pointer to the next primitive-buffer packet.
 */
CheckPSGlyphPacket* emit_glyph_sprite(CheckPSGlyphPacket* packet, u_long* ot_tag, s32 cache_slot)
{
    g_glyph_cache[cache_slot].raw |= CHECKPS_GLYPH_CACHE_USED_FLAG;

    setSprt16(&packet->sprite);
    packet->sprite.g0 = CHECKPS_GLYPH_NEUTRAL_COLOR;
    packet->sprite.b0 = CHECKPS_GLYPH_NEUTRAL_COLOR;
    packet->sprite.r0 = CHECKPS_GLYPH_NEUTRAL_COLOR;
    setXY0(&packet->sprite, g_glyph_cursor_x, g_glyph_cursor_y);
    setUV0(&packet->sprite, (cache_slot % CHECKPS_GLYPH_WIDTH) * CHECKPS_GLYPH_WIDTH, cache_slot & CHECKPS_GLYPH_V_COORD_MASK);
    packet->sprite.clut = getClut(0, CHECKPS_GLYPH_CLUT_Y);
    addPrim(ot_tag, &packet->sprite);
    packet++;

    g_glyph_cursor_x += CHECKPS_GLYPH_WIDTH;
    if (g_glyph_cursor_x + CHECKPS_GLYPH_WIDTH >= CHECKPS_TEXT_WRAP_LIMIT)
    {
        g_glyph_cursor_x = g_text_line_start_x;
        g_glyph_cursor_y += CHECKPS_GLYPH_WIDTH;
    }

    return packet;
}

/**
 * @brief Clear per-frame usage marks and reset the glyph raster cursor.
 */
void begin_glyph_cache_frame(void)
{
    s32 cache_slot;

    g_glyph_raster_cursor = g_glyph_raster_buffer;
    for (cache_slot = 0; cache_slot < CHECKPS_GLYPH_CACHE_ENTRY_COUNT; cache_slot++)
    {
        g_glyph_cache[cache_slot].raw &= CHECKPS_GLYPH_CACHE_CODE_MASK;
    }
}

/**
 * @brief Release glyph-cache entries that were not used this frame.
 */
void evict_unused_glyphs(void)
{
    s32 cache_slot;

    for (cache_slot = 0; cache_slot < CHECKPS_GLYPH_CACHE_ENTRY_COUNT; cache_slot++)
    {
        if (!(g_glyph_cache[cache_slot].raw & CHECKPS_GLYPH_CACHE_USED_FLAG))
        {
            g_glyph_cache[cache_slot].raw = 0;
        }
    }
}

/**
 * @brief Clear glyph-cache state and upload the renderer CLUT.
 */
void reset_glyph_renderer(void)
{
    s32 i;
    RECT clut_rect;

    for (i = CHECKPS_GLYPH_CACHE_ENTRY_COUNT - 1; i >= 0; i--)
    {
        g_glyph_cache[i].raw = 0;
    }

    for (i = 0; i < CHECKPS_GLYPH_RASTER_BUFFER_SIZE; i++)
    {
        g_glyph_raster_buffer[i] = 0;
    }

    clut_rect.y = CHECKPS_GLYPH_CLUT_Y;
    clut_rect.w = CHECKPS_GLYPH_WIDTH;
    clut_rect.x = 0;
    clut_rect.h = 1;

    LoadImage(&clut_rect, g_glyph_clut_prefix);
}
