#include "checkps.h"

/**
 * decomp.me link (100%) https://decomp.me/scratch/jqJzK
 */
void func_80051DD4(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5) {
    s32 new_var;
    Sp20Data sp;
    s32 temp;
    u16 *new_var3;
    s32 new_var4;

    new_var4 = arg2;
    if (arg2 >= 0) {
        temp = arg2;
    } else {
        temp = new_var4 + 15;
    }
    temp >>= 4;
    new_var3 = &D_8005D030[temp];
    do {
    } while (0);
    new_var = temp << 4;
    sp.sp20 = *new_var3;

    new_var = arg2 - new_var;
    sp.sp22 = D_8005D030[new_var];
    sp.sp24 = 0;
    func_80051E58((void *)arg0, (s32 *)arg1, (u8 *)(&sp), arg3, arg4, 0, arg5);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/gVtK1
 */
void *func_80051E58(void *arg0, s32 *arg1, u8 *arg2, s32 arg3, s32 arg4,
                    s32 arg5, s32 arg6) {
    u8 *s = arg2;
    s32 count = 0;
    u32 old;
    u16 val;
    u8 *p;

    /* Count characters */
    if (*s >= 0x20) {
        p = s;
        do {
            val = *p;

            if (val >= 0x80) {
                p++;
            }

            p++;
            count++;

        } while (*p >= 0x20);
    }

    /* Alignment adjustment */
    switch (arg6) {
    case 1:
        arg3 -= 16 * count;
        break;

    case 2:
        arg3 -= 8 * count;
        break;

    case 0:
    default:
        break;
    }

    g_textOriginX = arg3;
    g_textCursorX = arg3;
    g_textCursorY = arg4;

    /* Main loop */
    while (1) {
        unsigned long c = *s;

        if (c == 0x20) {
            s++;
            g_textCursorX += 0x10;
            continue;
        }

        if (c >= 0x80) {
            val = s[0];
            val = (val << 8) | s[1];
            s += 2;
        } else {
            if (c < 0x20) {
                break;
            }

            val = (u16)(*s - 0x7AE1);
            s++;
        }

        arg0 = RenderGlyph(arg0, arg1, val, arg5);
    }

    /* Final write */
    ((u8 *)arg0)[3] = 1;
    *((u32 *)((u8 *)arg0 + 4)) = 0xE100000F;

    old = *((u32 *)arg0);
    *((u32 *)arg0) = (old & 0xFF000000) | ((*arg1) & 0xFFFFFF);

    *arg1 = ((*arg1) & 0xFF000000) | (((u32)arg0) & 0xFFFFFF);

    return ((u8 *)arg0) + 8;
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/6ygLn
 */
s32 RenderGlyph(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    u32 *ptr;
    u8 *font_data;
    unsigned int new_var3;
    int new_var4;
    s32 slot;
    int new_var;
    unsigned int new_var2;
    s32 new_var5;
    RECT rect;

    u8 *dest;
    int inc;
    int inc16;
    int outer;
    int middle;

    u16 mask;
    u8 font_byte;
    volatile u8 *vptr;
    u8 temp;

    new_var5 = arg2;
    slot = 0;
    new_var3 = new_var5 & 0xFFFF;
    ptr = g_characterCache;

    while (slot < 0x100) {
        if (new_var3 == ((u16)(*ptr))) {
            return CreateGlyphInstance(arg0, arg1, slot);
        }
        slot++;
        ptr++;
    }

    font_data = func_8001687C(new_var5 & 0xFFFF);
    if (font_data == ((u8 *)-1)) {
        return arg0;
    }

    dest = g_glyphBufferCursor;
    inc = arg3 + 1;
    inc16 = inc * 16;

    for (outer = 0; outer < 15; outer++) {
        new_var4 = inc16;

        for (middle = 0; middle < 2; middle++) {
            mask = 0x80;

            for (slot = 0; slot < 4; slot++) {
                *dest = ((*font_data) & mask) ? (inc) : (0);

                mask >>= 1 & (new_var2 = 0xFFFFu);
                new_var = (*font_data) & mask;

                vptr = (volatile u8 *)dest;
                temp = *vptr;

                if (new_var) {
                    temp += new_var4;
                }

                *vptr = temp;

                mask >>= 1;
                dest++;
            }

            font_data++;
        }
    }

    slot = 0;
    while ((slot < 0x100) && (g_characterCache[slot].raw != 0)) {
        slot++;
    }

    if (slot == 0x100) {
        return arg0;
    }

    g_characterCache[slot].raw = new_var5 & (0xFFFF & 0xFFFFFFFFu);
    arg0 = CreateGlyphInstance(arg0, arg1, slot);

    g_glyphAtlasX = (slot % 16) * 4;
    g_glyphAtlasY = slot & 0xF0;

    rect.w = 4;
    rect.h = 15;
    rect.x = g_glyphAtlasX + 0x3C0;
    rect.y = g_glyphAtlasY;

    LoadImage(&rect, (u_long *)g_glyphBufferCursor);
    DrawSync(0);

    g_glyphBufferCursor += 0x80;
    return arg0;
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/FyrJc
 */
void* CreateGlyphInstance(void *instance, s32 *next, s32 index) {
    int new_var;
    s32 var_a0;
    GlyphInstance *s = (GlyphInstance *)instance;
    s32 arg0_masked;
    s32 old_c0;
    s32 new_c0;
    s32 cond;

    g_characterCache[index].raw |= GLYPH_CACHED_FLAG;
    s->u.byte.unk3 = 3;
    s->unk7 = 0x7C;
    s->unk5 = 0x80;
    s->unk6 = 0x80;
    s->unk4 = 0x80;
    var_a0 = index;
    s->positionX = (u16)g_textCursorX;
    s->positionY = (u16)g_textCursorY;

    if (index < 0) {
        var_a0 = index + 0xF;
    }

    s->unkC = (s8)((index - ((var_a0 >> 4) * 0x10)) * 0x10);
    s->unkD = (s8)(index & 0xF0);
    s->unkE = 0x7FC0;
    s->u.unk0 = (s->u.unk0 & 0xFF000000) | ((*next) & 0xFFFFFF);
    arg0_masked = ((s32)instance) & 0xFFFFFF;
    new_var = (*next) & 0xFF000000;
    instance = ((char *)instance) + 0x14;

    old_c0 = g_textCursorX;
    new_c0 = old_c0 + 0x10;
    cond = (old_c0 + 0x20) < 640;
    g_textCursorX = new_c0;
    *next = new_var | arg0_masked;

    if (!cond) {
        g_textCursorX = g_textOriginX;
        g_textCursorY += 16;
    }

    return instance;
}

void InvalidateGlyphCache(void) {
    s32 index;
    GlyphCacheEntry* entry;

    g_glyphBufferCursor = (s32)&g_TextBuffer;
    
    index = 0;
    entry = g_characterCache;

    // Strip all flags (including isCached) and keep only the charId
    while (index < MAX_GLYPH_ENTRIES) {
        entry->raw &= 0x0000FFFF;
        entry++;
        index++;
    }
}

void ClearInvalidGlyphs(void) {
    int flag;
    s32 index;
    GlyphCacheEntry *entry;

    index = 0;
    flag = GLYPH_CACHED_FLAG;
    entry = g_characterCache;

    while (index < MAX_GLYPH_ENTRIES) {
        if (!(entry->raw & flag)) {
            entry->raw = 0;
        }

        index++;
        entry++;
    }
}

void ResetTextRenderer(void) {
    s32 index;
    GlyphCacheEntry *entry;
    RECT clearRect;

    // Clear the character cache entries (descending loop)
    index = MAX_GLYPH_ENTRIES - 1;
    entry = &g_characterCache[index];

    while (index >= 0) {
        entry->raw = 0;
        entry--;
        index--;
    }

    // Zero out the global text bitmap buffer (32KB)
    for (index = 0; index <= MAX_SHORT_VALUE; index++) {
        g_TextBuffer[index] = 0;
    }

    // Set up a small rectangle to reset the GPU texture state
    clearRect.y = 511;
    clearRect.w = 16;
    clearRect.x = 0;
    clearRect.h = 1;

    LoadImage(&clearRect, (u_long *)&g_textBufferAddr);
}