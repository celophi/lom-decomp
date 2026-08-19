#include "checkps.h"

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

    while (first_digit < 5 && glyph_buffer[first_digit] == 0x4F82)
    {
        first_digit++;
    }

    if (negative != 0)
    {
        first_digit--;
        glyph_buffer[first_digit] = 0x5B81;
    }
    primitive = draw_cached_text(primitive, ot_tag, (u8*)&glyph_buffer[first_digit], x, y, palette, alignment);
    return primitive;
}

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
    draw_cached_text(primitive, ot_tag, (u8*)&glyph_pair, x, y, 0, alignment);
}

void* draw_cached_text(void* primitive, u_long* ot_tag, const u8* text, s32 x, s32 y, s32 palette, s32 alignment)
{
    const u8* cursor = text;
    s32 glyph_count = 0;
    u32 old_tag;
    u16 character_code;
    const u8* scan;
    /* Count characters */
    if (*cursor >= 0x20)
    {
        scan = cursor;
        do
        {
            character_code = *scan;

            if (character_code >= 0x80)
            {
                scan++;
            }

            scan++;
            glyph_count++;

        } while (*scan >= 0x20);
    }

    /* Alignment adjustment */
    switch (alignment)
    {
    case 1:
        x -= 16 * glyph_count;
        break;

    case 2:
        x -= 8 * glyph_count;
        break;

    case 0:
    default:
        break;
    }
    g_text_line_start_x = x;
    g_glyph_cursor_x = x;
    g_glyph_cursor_y = y;

    /* Main loop */
    while (1)
    {
        unsigned long lead_byte = *cursor;

        if (lead_byte == 0x20)
        {
            cursor++;
            g_glyph_cursor_x += 0x10;
            continue;
        }

        if (lead_byte >= 0x80)
        {
            character_code = cursor[0];
            character_code = (character_code << 8) | cursor[1];
            cursor += 2;
        }
        else
        {
            if (lead_byte < 0x20)
            {
                break;
            }
            character_code = (u16)(*cursor - 0x7AE1);
            cursor++;
        }

        primitive = render_cached_glyph(primitive, ot_tag, character_code, palette);
    }

    /* Final write */
    ((u8*)primitive)[3] = 1;
    *((u32*)((u8*)primitive + 4)) = 0xE100000F;

    old_tag = *((u32*)primitive);
    *((u32*)primitive) = (old_tag & 0xFF000000) | ((*ot_tag) & 0xFFFFFF);

    *ot_tag = ((*ot_tag) & 0xFF000000) | (((u32)primitive) & 0xFFFFFF);

    return ((u8*)primitive) + 8;
}
void* render_cached_glyph(void* primitive, u_long* ot_tag, s32 character_code, s32 palette)
{
    GlyphCacheEntry* cache_entry;
    u8* font_data;
    unsigned int requested_code;
    int high_nibble_color;
    s32 slot;
    int low_bit_set;
    unsigned int mask_all_bits;
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
    requested_code = code & 0xFFFF;
    cache_entry = g_glyph_cache;

    while (slot < 0x100)
    {
        if (requested_code == (u16)cache_entry->raw)
        {
            return emit_glyph_sprite(primitive, ot_tag, slot);
        }
        slot++;
        cache_entry++;
    }

    font_data = (u8*)Krom2RawAdd(code & 0xFFFF);
    if (font_data == ((u8*)-1))
    {
        return primitive;
    }

    raster = g_glyph_raster_cursor;
    color_index = palette + 1;
    high_nibble_increment = color_index * 16;
    for (row = 0; row < 15; row++)
    {
        high_nibble_color = high_nibble_increment;

        for (source_byte = 0; source_byte < 2; source_byte++)
        {
            mask = 0x80;

            for (slot = 0; slot < 4; slot++)
            {
                *raster = ((*font_data) & mask) ? (color_index) : (0);

                mask >>= 1 & (mask_all_bits = 0xFFFFu);
                low_bit_set = (*font_data) & mask;

                raster_byte = (volatile u8*)raster;
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
    while ((slot < 0x100) && (g_glyph_cache[slot].raw != 0))
    {
        slot++;
    }

    if (slot == 0x100)
    {
        return primitive;
    }
    g_glyph_cache[slot].raw = code & (0xFFFF & 0xFFFFFFFFu);
    primitive = emit_glyph_sprite(primitive, ot_tag, slot);

    g_glyph_upload_x = (slot % 16) * 4;
    g_glyph_upload_y = slot & 0xF0;

    rect.w = 4;
    rect.h = 15;
    rect.x = g_glyph_upload_x + 0x3C0;
    rect.y = g_glyph_upload_y;

    LoadImage(&rect, (u_long*)g_glyph_raster_cursor);
    DrawSync(0);

    g_glyph_raster_cursor += 0x80;
    return primitive;
}
GlyphSpritePacket* emit_glyph_sprite(GlyphSpritePacket* packet, u_long* ot_tag, s32 cache_slot)
{
    u32 ot_tag_high_byte;
    s32 normalized_slot;
    GlyphSpritePacket* sprite = packet;
    u32 packet_address;
    s32 old_x;
    s32 new_x;
    s32 fits_line;

    // Mark this cache slot as used by the current frame
    g_glyph_cache[cache_slot].raw |= GLYPH_USED_FLAG;

    // Initialize the fixed SPRT packet fields
    sprite->tag.byte.word_count = 3;
    sprite->code = 0x7C;
    sprite->g = 0x80;
    sprite->b = 0x80;
    sprite->r = 0x80;
    normalized_slot = cache_slot;
    sprite->x = (s16)g_glyph_cursor_x;
    sprite->y = (s16)g_glyph_cursor_y;

    // Normalize cache_slot for atlas offset calculation
    if (cache_slot < 0)
    {
        normalized_slot = cache_slot + 15;
    }

    // Calculate relative UV offsets based on the cache slot
    sprite->u = (u8)((cache_slot - ((normalized_slot >> 4) * 0x10)) * 0x10);
    sprite->v = (u8)(cache_slot & 0xF0);
    sprite->clut = 0x7FC0;
    // Splice the packet into the ordering-table chain using its 24-bit GPU tag.
    // The upper byte is the packet word count and must be preserved.
    sprite->tag.raw = (sprite->tag.raw & 0xFF000000) | (*ot_tag & 0xFFFFFF);

    packet_address = ((s32)packet) & 0xFFFFFF;
    ot_tag_high_byte = *ot_tag & 0xFF000000;

    /* Preserve byte-pointer +0x14 arithmetic: typed packet arithmetic changes GCC 2.7.2 codegen. */
    packet = (GlyphSpritePacket*)(((char*)packet) + 0x14);
    // Update global X cursor and check for line wrap
    old_x = g_glyph_cursor_x;
    new_x = old_x + 0x10;
    fits_line = (old_x + 0x20) < 640;
    g_glyph_cursor_x = new_x;

    // Update the 24-bit ordering-table link for the following packet
    *ot_tag = ot_tag_high_byte | packet_address;

    if (!fits_line)
    {
        g_glyph_cursor_x = g_text_line_start_x;
        g_glyph_cursor_y += 16;
    }

    return packet;
}
void begin_glyph_cache_frame(void)
{
    s32 cache_slot;
    GlyphCacheEntry* cache_entry;

    g_glyph_raster_cursor = g_glyph_raster_buffer;

    cache_slot = 0;
    cache_entry = g_glyph_cache;

    // Clear the per-frame usage mark while preserving each cached character code
    while (cache_slot < MAX_GLYPH_ENTRIES)
    {
        cache_entry->raw &= 0x0000FFFF;
        cache_entry++;
        cache_slot++;
    }
}

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

void reset_glyph_renderer(void)
{
    s32 cache_slot;
    GlyphCacheEntry* cache_entry;
    RECT clut_rect;

    // Clear all cached character-code slots (descending loop)
    cache_slot = MAX_GLYPH_ENTRIES - 1;
    cache_entry = &g_glyph_cache[cache_slot];
    while (cache_slot >= 0)
    {
        cache_entry->raw = 0;
        cache_entry--;
        cache_slot--;
    }

    // Zero the 32 KiB 4bpp glyph raster staging buffer
    for (cache_slot = 0; cache_slot <= MAX_SHORT_VALUE; cache_slot++)
    {
        g_glyph_raster_buffer[cache_slot] = 0;
    }

    // Upload the 16-entry glyph CLUT to VRAM row y=511
    clut_rect.y = 511;
    clut_rect.w = 16;
    clut_rect.x = 0;
    clut_rect.h = 1;

    LoadImage(&clut_rect, g_glyph_clut_prefix);
}
