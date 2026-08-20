#include "checkps_internal.h"

/*
 * Nonzero prefix of the 16-color glyph CLUT.  LoadImage reads a 16x1 palette
 * from this address; the remaining entries are zero in the following linked
 * memory.
 */
u_long g_glyph_clut_prefix[] = {
    0xFFFF0000,
    0x0000BDEF,
    0x00000000,
};

u8 g_glyph_raster_buffer[MAX_SHORT_VALUE + 1];

/**
 * Cached character-code slots for the 16x16 text renderer.
 * Bit 16 is a per-frame usage mark, not a persistent cache-validity bit.
 */
GlyphCacheEntry g_glyph_cache[MAX_GLYPH_ENTRIES];

s32 g_glyph_cursor_x;

s32 g_glyph_cursor_y;
/** Next free 4bpp glyph block in the CPU-side staging buffer. */
u8* g_glyph_raster_cursor;

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
 * @param value Signed value to draw.
 * @param x Screen-space x coordinate.
 * @param y Screen-space y coordinate.
 * @param palette Glyph palette index.
 * @param alignment Text alignment mode.
 * @return Updated primitive-buffer cursor.
 */
void* draw_signed_decimal(void* primitive, u_long* ot_tag, s32 value, s32 x, s32 y, s32 palette, s32 alignment)
{
    u16 glyph_buffer[7];
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

    first_digit = 1;

    glyph_buffer[6] = 0;

    while (first_digit < 5 && glyph_buffer[first_digit] == CHECKPS_SJIS_FULLWIDTH_ZERO)
    {
        first_digit++;
    }

    if (negative != 0)
    {
        first_digit--;
        glyph_buffer[first_digit] = CHECKPS_SJIS_MINUS;
    }
    primitive = draw_cached_text(primitive, ot_tag, &glyph_buffer[first_digit], x, y, palette, alignment);
    return primitive;
}

/**
 * @brief Draw a two-digit hexadecimal value with cached glyphs.
 * @param primitive Primitive-buffer cursor.
 * @param ot_tag Ordering-table tag to append to.
 * @param value Value whose low byte is displayed.
 * @param x Screen-space x coordinate.
 * @param y Screen-space y coordinate.
 * @param alignment Text alignment mode.
 */
void draw_hex_byte(void* primitive, u_long* ot_tag, s32 value, s32 x, s32 y, s32 alignment)
{
    s32 low_nibble;
    EncodedGlyphPair glyph_pair;
    s32 high_nibble;
    u16* high_glyph;
    high_nibble = value / 16;
    high_glyph = &g_hex_glyph_table[high_nibble];
    low_nibble = value % 16;
    glyph_pair.first_glyph = *high_glyph;
    glyph_pair.second_glyph = g_hex_glyph_table[low_nibble];
    glyph_pair.terminator = 0;
    draw_cached_text(primitive, ot_tag, &glyph_pair, x, y, 0, alignment);
}

/**
 * @brief Draw a cached Shift-JIS text string into the ordering table.
 * @param primitive Primitive-buffer cursor.
 * @param ot_tag Ordering-table tag to append to.
 * @param text Encoded text terminated by a control byte.
 * @param x Screen-space x coordinate.
 * @param y Screen-space y coordinate.
 * @param palette Glyph palette index.
 * @param alignment Text alignment mode.
 * @return Updated primitive-buffer cursor.
 */
void* draw_cached_text(void* primitive, u_long* ot_tag, const void* text, s32 x, s32 y, s32 palette, s32 alignment)
{
    const u8* cursor = text;
    s32 glyph_count = 0;
    u32 old_tag;
    u16 character_code;
    const u8* scan;
    if (*cursor >= CHECKPS_TEXT_FIRST_PRINTABLE)
    {
        scan = cursor;
        do
        {
            character_code = *scan;

            if (character_code >= CHECKPS_SJIS_LEAD_BYTE_THRESHOLD)
            {
                scan++;
            }

            scan++;
            glyph_count++;

        } while (*scan >= CHECKPS_TEXT_FIRST_PRINTABLE);
    }

    switch (alignment)
    {
    case 1:
        x -= CHECKPS_GLYPH_WIDTH * glyph_count;
        break;

    case 2:
        x -= (CHECKPS_GLYPH_WIDTH / 2) * glyph_count;
        break;

    case 0:
    default:
        break;
    }
    g_text_line_start_x = x;
    g_glyph_cursor_x = x;
    g_glyph_cursor_y = y;

    while (1)
    {
        unsigned long lead_byte = *cursor;

        if (lead_byte == CHECKPS_TEXT_SPACE)
        {
            cursor++;
            g_glyph_cursor_x += CHECKPS_GLYPH_WIDTH;
            continue;
        }

        if (lead_byte >= CHECKPS_SJIS_LEAD_BYTE_THRESHOLD)
        {
            character_code = cursor[0];
            character_code = (character_code << 8) | cursor[1];
            cursor += 2;
        }
        else
        {
            if (lead_byte < CHECKPS_TEXT_FIRST_PRINTABLE)
            {
                break;
            }
            character_code = (u16)(*cursor - CHECKPS_ASCII_TO_SJIS_BIAS);
            cursor++;
        }

        primitive = render_cached_glyph(primitive, ot_tag, character_code, palette);
    }

    ((u8*)primitive)[3] = 1;
    *((u32*)((u8*)primitive + 4)) = CHECKPS_TEXT_DRAW_MODE_COMMAND;

    old_tag = *((u32*)primitive);
    *((u32*)primitive) = (old_tag & CHECKPS_GPU_TAG_LENGTH_MASK) | ((*ot_tag) & CHECKPS_GPU_TAG_ADDRESS_MASK);

    *ot_tag = ((*ot_tag) & CHECKPS_GPU_TAG_LENGTH_MASK) | (((u32)primitive) & CHECKPS_GPU_TAG_ADDRESS_MASK);

    return ((u8*)primitive) + sizeof(DR_TPAGE);
}

/**
 * @brief Resolve one glyph in the cache, uploading it to VRAM when necessary.
 * @param primitive Primitive-buffer cursor.
 * @param ot_tag Ordering-table tag to append to.
 * @param character_code Shift-JIS character code.
 * @param palette Glyph palette index.
 * @return Updated primitive-buffer cursor.
 */
void* render_cached_glyph(void* primitive, u_long* ot_tag, s32 character_code, s32 palette)
{
    GlyphCacheEntry* cache_entry;
    u8* font_data;
    unsigned int requested_code;
    int high_nibble_color;
    s32 slot;
    int low_bit_set;
    s32 code;
    RECT rect;

    u8* raster;
    int color_index;
    int high_nibble_increment;
    int row;
    int source_byte;

    u16 mask;
    volatile u8* raster_byte;
    u8 packed_pixels;
    code = character_code;
    slot = 0;
    requested_code = code & CHECKPS_GLYPH_CODE_MASK;
    cache_entry = g_glyph_cache;

    while (slot < MAX_GLYPH_ENTRIES)
    {
        if (requested_code == (u16)cache_entry->raw)
        {
            return emit_glyph_sprite(primitive, ot_tag, slot);
        }
        slot++;
        cache_entry++;
    }

    font_data = (u8*)Krom2RawAdd(code & CHECKPS_GLYPH_CODE_MASK);
    if (font_data == ((u8*)-1))
    {
        return primitive;
    }

    raster = g_glyph_raster_cursor;
    color_index = palette + 1;
    high_nibble_increment = color_index * CHECKPS_GLYPH_WIDTH;
    for (row = 0; row < CHECKPS_GLYPH_BITMAP_ROWS; row++)
    {
        high_nibble_color = high_nibble_increment;

        for (source_byte = 0; source_byte < 2; source_byte++)
        {
            mask = CHECKPS_GLYPH_SOURCE_MSB;

            for (slot = 0; slot < 4; slot++)
            {
                *raster = ((*font_data) & mask) ? (color_index) : (0);

                mask >>= 1;
                low_bit_set = (*font_data) & mask;

                raster_byte = raster;
                packed_pixels = *raster_byte;
                if (low_bit_set)
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
    while ((slot < MAX_GLYPH_ENTRIES) && (g_glyph_cache[slot].raw != 0))
    {
        slot++;
    }

    if (slot == MAX_GLYPH_ENTRIES)
    {
        return primitive;
    }
    g_glyph_cache[slot].raw = code & CHECKPS_GLYPH_CODE_MASK;
    primitive = emit_glyph_sprite(primitive, ot_tag, slot);

    g_glyph_upload_x = (slot % CHECKPS_GLYPH_WIDTH) * CHECKPS_GLYPH_VRAM_WORD_WIDTH;
    g_glyph_upload_y = slot & CHECKPS_GLYPH_V_COORD_MASK;

    rect.w = CHECKPS_GLYPH_VRAM_WORD_WIDTH;
    rect.h = CHECKPS_GLYPH_BITMAP_ROWS;
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
GlyphSpritePacket* emit_glyph_sprite(GlyphSpritePacket* packet, u_long* ot_tag, s32 cache_slot)
{
    u32 ot_tag_high_byte;
    s32 normalized_slot;
    GlyphSpritePacket* sprite = packet;
    u32 packet_address;
    s32 old_x;
    s32 new_x;
    s32 fits_line;

    g_glyph_cache[cache_slot].raw |= GLYPH_USED_FLAG;

    setSprt16(sprite);
    sprite->g0 = 0x80;
    sprite->b0 = 0x80;
    sprite->r0 = 0x80;
    normalized_slot = cache_slot;
    sprite->x0 = g_glyph_cursor_x;
    sprite->y0 = g_glyph_cursor_y;

    if (cache_slot < 0)
    {
        normalized_slot = cache_slot + (CHECKPS_GLYPH_WIDTH - 1);
    }

    sprite->u0 = (cache_slot - ((normalized_slot >> 4) * CHECKPS_GLYPH_WIDTH)) * CHECKPS_GLYPH_WIDTH;
    sprite->v0 = cache_slot & CHECKPS_GLYPH_V_COORD_MASK;
    sprite->clut = getClut(0, CHECKPS_GLYPH_CLUT_Y);
    /* Preserve the packet length while linking its 24-bit address into the OT. */
    sprite->tag = (sprite->tag & CHECKPS_GPU_TAG_LENGTH_MASK) | (*ot_tag & CHECKPS_GPU_TAG_ADDRESS_MASK);

    packet_address = ((s32)packet) & CHECKPS_GPU_TAG_ADDRESS_MASK;
    ot_tag_high_byte = *ot_tag & CHECKPS_GPU_TAG_LENGTH_MASK;

    /* The linked glyph packet is 20 bytes; typed SPRT_16 arithmetic would advance only 16. */
    packet = (GlyphSpritePacket*)(((char*)packet) + CHECKPS_GLYPH_PACKET_STRIDE);
    old_x = g_glyph_cursor_x;
    new_x = old_x + CHECKPS_GLYPH_WIDTH;
    fits_line = (old_x + (CHECKPS_GLYPH_WIDTH * 2)) < CHECKPS_TEXT_WRAP_LIMIT;
    g_glyph_cursor_x = new_x;

    *ot_tag = ot_tag_high_byte | packet_address;

    if (!fits_line)
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
    GlyphCacheEntry* cache_entry;

    g_glyph_raster_cursor = g_glyph_raster_buffer;

    cache_slot = 0;
    cache_entry = g_glyph_cache;

    while (cache_slot < MAX_GLYPH_ENTRIES)
    {
        cache_entry->raw &= CHECKPS_GLYPH_CODE_MASK;
        cache_entry++;
        cache_slot++;
    }
}

/**
 * @brief Release glyph-cache entries that were not used this frame.
 */
void evict_unused_glyphs(void)
{
    s32 used_flag;
    s32 cache_slot;
    GlyphCacheEntry* cache_entry;
    cache_slot = 0;
    used_flag = GLYPH_USED_FLAG;
    cache_entry = g_glyph_cache;

    while (cache_slot < MAX_GLYPH_ENTRIES)
    {
        if (!(cache_entry->raw & used_flag))
        {
            cache_entry->raw = 0;
        }

        cache_slot++;
        cache_entry++;
    }
}

/**
 * @brief Clear glyph-cache state and upload the renderer CLUT.
 */
void reset_glyph_renderer(void)
{
    s32 cache_slot;
    GlyphCacheEntry* cache_entry;
    RECT clut_rect;

    cache_slot = MAX_GLYPH_ENTRIES - 1;
    cache_entry = &g_glyph_cache[cache_slot];
    while (cache_slot >= 0)
    {
        cache_entry->raw = 0;
        cache_entry--;
        cache_slot--;
    }

    for (cache_slot = 0; cache_slot <= MAX_SHORT_VALUE; cache_slot++)
    {
        g_glyph_raster_buffer[cache_slot] = 0;
    }

    clut_rect.y = CHECKPS_GLYPH_CLUT_Y;
    clut_rect.w = CHECKPS_GLYPH_WIDTH;
    clut_rect.x = 0;
    clut_rect.h = 1;

    LoadImage(&clut_rect, g_glyph_clut_prefix);
}
