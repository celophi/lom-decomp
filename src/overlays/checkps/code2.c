#include "checkps.h"

/*
 * Nonzero prefix of the 16-color glyph CLUT.  LoadImage reads a 16x1 palette
 * from this address; the remaining entries are zero in the following linked
 * memory.
 */
u_long g_glyphClutPrefix[] = {
    0xFFFF0000,
    0x0000BDEF,
    0x00000000,
};

u8 g_glyphRasterBuffer[MAX_SHORT_VALUE + 1];

/**
 * Cached character-code slots for the 16x16 text renderer.
 * Bit 16 is a per-frame usage mark, not a persistent cache-validity bit.
 */
GlyphCacheEntry g_glyphCache[MAX_GLYPH_ENTRIES];

s32 g_glyphCursorX;

s32 g_glyphCursorY;
/** Next free 4bpp glyph block in the CPU-side staging buffer. */
u8* g_glyphRasterCursor;

s32 g_textLineStartX;

/**
 * VRAM X coordinate used for the most recently uploaded glyph slot.
 */
s32 g_glyphUploadX;

/**
 * VRAM Y coordinate used for the most recently uploaded glyph slot.
 */
s32 g_glyphUploadY;
void* DrawSignedDecimal(void* primitive, u_long* otTag, s32 value, s32 x, s32 y, s32 palette, s32 alignment)
{
    u16 glyphBuffer[7];
    s32 firstDigit;
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
    glyphBuffer[1] = g_decimalGlyphTable[magnitude / 10000];
    glyphBuffer[2] = g_decimalGlyphTable[(magnitude % 10000) / 1000];
    glyphBuffer[3] = g_decimalGlyphTable[(magnitude % 1000) / 100];
    glyphBuffer[4] = g_decimalGlyphTable[(magnitude % 100) / 10];
    glyphBuffer[5] = g_decimalGlyphTable[magnitude % 10];

    firstDigit = 1;

    glyphBuffer[6] = 0;

    while (firstDigit < 5 && glyphBuffer[firstDigit] == 0x4F82)
    {
        firstDigit++;
    }

    if (negative != 0)
    {
        firstDigit--;
        glyphBuffer[firstDigit] = 0x5B81;
    }
    primitive = DrawCachedText(primitive, otTag, (u8*)&glyphBuffer[firstDigit], x, y, palette, alignment);
    return primitive;
}

void DrawHexByte(void* primitive, u_long* otTag, s32 value, s32 x, s32 y, s32 alignment)
{
    s32 lowNibble;
    EncodedGlyphPair glyphPair;
    s32 highNibble;
    u16* highGlyph;
    highNibble = value / 16;
    highGlyph = &g_hexGlyphTable[highNibble];
    lowNibble = value % 16;
    glyphPair.firstGlyph = *highGlyph;
    glyphPair.secondGlyph = g_hexGlyphTable[lowNibble];
    glyphPair.terminator = 0;
    DrawCachedText(primitive, otTag, (u8*)&glyphPair, x, y, 0, alignment);
}

void* DrawCachedText(void* primitive, u_long* otTag, const u8* text, s32 x, s32 y, s32 palette, s32 alignment)
{
    const u8* cursor = text;
    s32 glyphCount = 0;
    u32 oldTag;
    u16 characterCode;
    const u8* scan;
    /* Count characters */
    if (*cursor >= 0x20)
    {
        scan = cursor;
        do
        {
            characterCode = *scan;

            if (characterCode >= 0x80)
            {
                scan++;
            }

            scan++;
            glyphCount++;

        } while (*scan >= 0x20);
    }

    /* Alignment adjustment */
    switch (alignment)
    {
    case 1:
        x -= 16 * glyphCount;
        break;

    case 2:
        x -= 8 * glyphCount;
        break;

    case 0:
    default:
        break;
    }
    g_textLineStartX = x;
    g_glyphCursorX = x;
    g_glyphCursorY = y;

    /* Main loop */
    while (1)
    {
        unsigned long leadByte = *cursor;

        if (leadByte == 0x20)
        {
            cursor++;
            g_glyphCursorX += 0x10;
            continue;
        }

        if (leadByte >= 0x80)
        {
            characterCode = cursor[0];
            characterCode = (characterCode << 8) | cursor[1];
            cursor += 2;
        }
        else
        {
            if (leadByte < 0x20)
            {
                break;
            }
            characterCode = (u16)(*cursor - 0x7AE1);
            cursor++;
        }

        primitive = RenderCachedGlyph(primitive, otTag, characterCode, palette);
    }

    /* Final write */
    ((u8*)primitive)[3] = 1;
    *((u32*)((u8*)primitive + 4)) = 0xE100000F;

    oldTag = *((u32*)primitive);
    *((u32*)primitive) = (oldTag & 0xFF000000) | ((*otTag) & 0xFFFFFF);

    *otTag = ((*otTag) & 0xFF000000) | (((u32)primitive) & 0xFFFFFF);

    return ((u8*)primitive) + 8;
}
void* RenderCachedGlyph(void* primitive, u_long* otTag, s32 characterCode, s32 palette)
{
    GlyphCacheEntry* cacheEntry;
    u8* fontData;
    unsigned int requestedCode;
    int highNibbleColor;
    s32 slot;
    int lowBitSet;
    unsigned int maskAllBits;
    s32 code;
    RECT rect;

    u8* raster;
    int colorIndex;
    int highNibbleIncrement;
    int row;
    int sourceByte;

    u16 mask;
    volatile u8* rasterByte;
    u8 packedPixels;
    code = characterCode;
    slot = 0;
    requestedCode = code & 0xFFFF;
    cacheEntry = g_glyphCache;

    while (slot < 0x100)
    {
        if (requestedCode == (u16)cacheEntry->raw)
        {
            return EmitGlyphSprite(primitive, otTag, slot);
        }
        slot++;
        cacheEntry++;
    }

    fontData = (u8*)Krom2RawAdd(code & 0xFFFF);
    if (fontData == ((u8*)-1))
    {
        return primitive;
    }

    raster = g_glyphRasterCursor;
    colorIndex = palette + 1;
    highNibbleIncrement = colorIndex * 16;
    for (row = 0; row < 15; row++)
    {
        highNibbleColor = highNibbleIncrement;

        for (sourceByte = 0; sourceByte < 2; sourceByte++)
        {
            mask = 0x80;

            for (slot = 0; slot < 4; slot++)
            {
                *raster = ((*fontData) & mask) ? (colorIndex) : (0);

                mask >>= 1 & (maskAllBits = 0xFFFFu);
                lowBitSet = (*fontData) & mask;

                rasterByte = (volatile u8*)raster;
                packedPixels = *rasterByte;
                if (lowBitSet)
                {
                    packedPixels += highNibbleColor;
                }

                *rasterByte = packedPixels;

                mask >>= 1;
                raster++;
            }

            fontData++;
        }
    }

    slot = 0;
    while ((slot < 0x100) && (g_glyphCache[slot].raw != 0))
    {
        slot++;
    }

    if (slot == 0x100)
    {
        return primitive;
    }
    g_glyphCache[slot].raw = code & (0xFFFF & 0xFFFFFFFFu);
    primitive = EmitGlyphSprite(primitive, otTag, slot);

    g_glyphUploadX = (slot % 16) * 4;
    g_glyphUploadY = slot & 0xF0;

    rect.w = 4;
    rect.h = 15;
    rect.x = g_glyphUploadX + 0x3C0;
    rect.y = g_glyphUploadY;

    LoadImage(&rect, (u_long*)g_glyphRasterCursor);
    DrawSync(0);

    g_glyphRasterCursor += 0x80;
    return primitive;
}
GlyphSpritePacket* EmitGlyphSprite(GlyphSpritePacket* packet, u_long* otTag, s32 cacheSlot)
{
    u32 otTagHighByte;
    s32 normalizedSlot;
    GlyphSpritePacket* sprite = packet;
    u32 packetAddress;
    s32 oldX;
    s32 newX;
    s32 fitsLine;

    // Mark this cache slot as used by the current frame
    g_glyphCache[cacheSlot].raw |= GLYPH_USED_FLAG;

    // Initialize the fixed SPRT packet fields
    sprite->tag.byte.wordCount = 3;
    sprite->code = 0x7C;
    sprite->g = 0x80;
    sprite->b = 0x80;
    sprite->r = 0x80;
    normalizedSlot = cacheSlot;
    sprite->x = (s16)g_glyphCursorX;
    sprite->y = (s16)g_glyphCursorY;

    // Normalize cacheSlot for atlas offset calculation
    if (cacheSlot < 0)
    {
        normalizedSlot = cacheSlot + 15;
    }

    // Calculate relative UV offsets based on the cache slot
    sprite->u = (u8)((cacheSlot - ((normalizedSlot >> 4) * 0x10)) * 0x10);
    sprite->v = (u8)(cacheSlot & 0xF0);
    sprite->clut = 0x7FC0;
    // Splice the packet into the ordering-table chain using its 24-bit GPU tag.
    // The upper byte is the packet word count and must be preserved.
    sprite->tag.raw = (sprite->tag.raw & 0xFF000000) | (*otTag & 0xFFFFFF);

    packetAddress = ((s32)packet) & 0xFFFFFF;
    otTagHighByte = *otTag & 0xFF000000;

    /* Preserve byte-pointer +0x14 arithmetic: typed packet arithmetic changes GCC 2.7.2 codegen. */
    packet = (GlyphSpritePacket*)(((char*)packet) + 0x14);
    // Update global X cursor and check for line wrap
    oldX = g_glyphCursorX;
    newX = oldX + 0x10;
    fitsLine = (oldX + 0x20) < 640;
    g_glyphCursorX = newX;

    // Update the 24-bit ordering-table link for the following packet
    *otTag = otTagHighByte | packetAddress;

    if (!fitsLine)
    {
        g_glyphCursorX = g_textLineStartX;
        g_glyphCursorY += 16;
    }

    return packet;
}
void BeginGlyphCacheFrame(void)
{
    s32 cacheSlot;
    GlyphCacheEntry* cacheEntry;

    g_glyphRasterCursor = g_glyphRasterBuffer;

    cacheSlot = 0;
    cacheEntry = g_glyphCache;

    // Clear the per-frame usage mark while preserving each cached character code
    while (cacheSlot < MAX_GLYPH_ENTRIES)
    {
        cacheEntry->raw &= 0x0000FFFF;
        cacheEntry++;
        cacheSlot++;
    }
}

void EvictUnusedGlyphs(void)
{
    s32 usedFlag;
    s32 cacheSlot;
    GlyphCacheEntry* cacheEntry;
    cacheSlot = 0;
    usedFlag = GLYPH_USED_FLAG;
    cacheEntry = g_glyphCache;

    while (cacheSlot < MAX_GLYPH_ENTRIES)
    {
        if (!(cacheEntry->raw & usedFlag))
        {
            cacheEntry->raw = 0;
        }

        cacheSlot++;
        cacheEntry++;
    }
}

void ResetGlyphRenderer(void)
{
    s32 cacheSlot;
    GlyphCacheEntry* cacheEntry;
    RECT clutRect;

    // Clear all cached character-code slots (descending loop)
    cacheSlot = MAX_GLYPH_ENTRIES - 1;
    cacheEntry = &g_glyphCache[cacheSlot];
    while (cacheSlot >= 0)
    {
        cacheEntry->raw = 0;
        cacheEntry--;
        cacheSlot--;
    }

    // Zero the 32 KiB 4bpp glyph raster staging buffer
    for (cacheSlot = 0; cacheSlot <= MAX_SHORT_VALUE; cacheSlot++)
    {
        g_glyphRasterBuffer[cacheSlot] = 0;
    }

    // Upload the 16-entry glyph CLUT to VRAM row y=511
    clutRect.y = 511;
    clutRect.w = 16;
    clutRect.x = 0;
    clutRect.h = 1;

    LoadImage(&clutRect, g_glyphClutPrefix);
}
