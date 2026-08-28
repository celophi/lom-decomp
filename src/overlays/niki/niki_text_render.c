#include "common.h"
#include "gpu_packet.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

#define GLYPH_CACHE_SLOTS 0x100
#define GLYPH_CACHE_COLUMNS 16
#define GLYPH_CACHE_ROW_MASK 0xF0
#define GLYPH_RASTER_BYTES 0x80
#define GPU_ADDR_MASK 0xFFFFFF
#define GPU_TAG_HIGH_MASK 0xFF000000

typedef struct
{
    s32 tag;
    s32 word4;
    s16 x0;
    s16 y0;
    s16 unkC;
    u16 unkE;
} GenericGpuPacket;

typedef union
{
    u32 raw;
    struct
    {
        u16 code;
        u16 flags;
    } data;
} GlyphCacheEntry;

typedef struct
{
    SPRT_16 packet;
    u32 padding;
} GlyphSprite;

extern s32 D_8016DA78;
extern s32 D_80165668;
extern s32 D_8016DA7C;
extern GlyphCacheEntry D_8016D678[];
extern u8 *D_80165674;
extern s32 D_8016566C;
extern s32 D_80165670;

extern s32 func_8001687C(s32);
extern void func_80019A34(RECT *, void *);
extern void func_80019788(s32);

s32 func_801469C0(s32 prim, s32 *ot, u8 *text, s32 x, s32 y, s32 palette, s32 alignment);
s32 func_80146B90(s32 prim, s32 *ot, s32 character_code, s32 palette);
s32 func_80146DB0(GlyphSprite *sprite, s32 *ot, s32 cache_slot, s32 palette);

s32 func_801469C0(s32 prim, s32 *ot, u8 *text, s32 x, s32 y, s32 palette, s32 alignment)
{
    u8 *cursor;
    s32 count;
    u16 code;
    u8 *scan;

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
    D_80165668 = x;
    D_8016DA78 = x;
    D_8016DA7C = y;

    while (1)
    {
        u32 lead = *cursor;

        if ((u8)lead == 0x20)
        {
            cursor++;
            D_8016DA78 += 0x10;
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
        prim = func_80146B90(prim, ot, code, palette);
    }

    setlen(prim, 1);
    ((GenericGpuPacket *)prim)->word4 = 0xE1000005;
    addPrim(ot, prim);
    return prim + 8;
}

s32 func_80146B90(s32 prim, s32 *ot, s32 character_code, s32 palette)
{
    GlyphCacheEntry *entry;
    u8 *font_data;
    s32 font_address;
    u32 requested_code;
    s32 slot;
    s32 high_pixel_set;
    s32 code;
    RECT rect;

    u8 *raster;
    s32 color_index;
    s32 high_nibble_color;
    s32 row;
    s32 source_byte;

    u16 mask;
    volatile u8 *raster_byte;
    u8 packed_pixels;

    code = character_code;
    slot = 0;
    requested_code = code & 0xFFFF;
    entry = D_8016D678;

    while (slot < GLYPH_CACHE_SLOTS)
    {
        if (requested_code == entry->data.code)
        {
            return func_80146DB0((GlyphSprite *)prim, ot, slot, palette);
        }
        slot++;
        entry++;
    }

    font_address = func_8001687C(code & 0xFFFF);
    font_data = (u8 *)font_address;
    if (font_address == -1)
    {
        return prim;
    }

    raster = D_80165674;
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
    while ((slot < GLYPH_CACHE_SLOTS) && (D_8016D678[slot].raw != 0))
    {
        slot++;
    }

    if (slot == GLYPH_CACHE_SLOTS)
    {
        return prim;
    }
    D_8016D678[slot].raw = code & 0xFFFF;
    prim = func_80146DB0((GlyphSprite *)prim, ot, slot, palette);

    D_8016566C = (slot % GLYPH_CACHE_COLUMNS) * 4;
    D_80165670 = slot & GLYPH_CACHE_ROW_MASK;

    rect.w = 4;
    rect.h = 15;
    rect.x = D_8016566C + 0x140;
    rect.y = D_80165670;

    func_80019A34(&rect, D_80165674);
    func_80019788(0);

    D_80165674 += GLYPH_RASTER_BYTES;
    return prim;
}

s32 func_80146DB0(GlyphSprite *sprite, s32 *ot, s32 cache_slot, s32 palette)
{
    u32 ot_tag_high_byte;
    s32 normalized_slot;
    u32 packet_address;
    s32 old_x;
    s32 new_x;
    s32 fits_line;

    D_8016D678[cache_slot].raw |= 0x10000;

    setlen(sprite, 3);
    setcode(sprite, 0x7C);
    sprite->packet.g0 = 0x80;
    sprite->packet.b0 = 0x80;
    sprite->packet.r0 = 0x80;
    normalized_slot = cache_slot;
    sprite->packet.x0 = D_8016DA78;
    sprite->packet.y0 = D_8016DA7C;

    if (cache_slot < 0)
    {
        normalized_slot = cache_slot + 15;
    }

    sprite->packet.u0 = (cache_slot - ((normalized_slot >> 4) * 16)) * 16;
    sprite->packet.v0 = cache_slot & GLYPH_CACHE_ROW_MASK;
    sprite->packet.clut = 0x7FD3;
    sprite->packet.tag = (sprite->packet.tag & GPU_TAG_HIGH_MASK) | (*ot & GPU_ADDR_MASK);

    packet_address = ((u32)sprite) & GPU_ADDR_MASK;
    ot_tag_high_byte = *ot & GPU_TAG_HIGH_MASK;

    sprite++;
    old_x = D_8016DA78;
    new_x = old_x + 16;
    fits_line = (old_x + 32) < 0x280;
    D_8016DA78 = new_x;

    *ot = ot_tag_high_byte | packet_address;

    if (!fits_line)
    {
        D_8016DA78 = D_80165668;
        D_8016DA7C += 16;
    }

    return (s32)sprite;
}
