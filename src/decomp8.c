#include "decomp8.h"

/**
 * @brief Draw a string at a fixed screen position using the field text engine.
 * @param str      Null-terminated ASCII string to draw.
 * @param x        Starting X of the text cursor (g_text_cursor_x).
 * @param y        Starting Y of the text cursor (g_text_cursor_y).
 * @param ot_depth Ordering-table depth/priority each glyph is linked into.
 * @param clut     Font CLUT/color variant, added to g_text_atlas_base.
 * @see decomp.me (100%) https://decomp.me/scratch/mLcZm
 */
void field_draw_string(u8* str, s32 x, s32 y, s32 ot_depth, s32 clut)
{
    u8* p;
    u8* end;
    s32 len;
    s32 ot_depth_val;

    g_text_cursor_x = x;
    g_text_cursor_y = y;
    ot_depth_val = ot_depth;
    len = strlen((const char*)str);
    if (len > 0)
    {
        p = str;
        end = (u8*)(len + (s32)p);
        do
        {
            u8 ch = *p++;
            func_800165CC(ch, ot_depth_val, clut);
        } while ((s32)p < (s32)end);
    }
}

/**
 * @brief Draw a value as a fixed 2-digit unsigned number (no leading-zero
 *        suppression) using the field text engine.
 * @param value    Number to draw (0-99 expected).
 * @param x        Starting X of the text cursor (g_text_cursor_x).
 * @param y        Starting Y of the text cursor (g_text_cursor_y).
 * @param ot_depth Ordering-table depth/priority each glyph is linked into.
 * @param clut     Font CLUT/color variant, added to g_text_atlas_base.
 * @see decomp.me (100%) https://decomp.me/scratch/ENN60
 */
void field_draw_uint2(s32 value, s32 x, s32 y, s32 ot_depth, s32 clut)
{
    s32 digit;
    int tens_value;
    g_text_cursor_x = x;
    g_text_cursor_y = y;
    digit = value / 10;
    tens_value = digit * 10;
    func_800165CC(DIGIT_TO_ASCII(digit), ot_depth, clut);
    digit = value - tens_value;
    func_800165CC(DIGIT_TO_ASCII(digit), ot_depth, clut);
}

/**
 * @brief Draw a value as a right-aligned 3-digit unsigned number, blanking
 *        (advancing the cursor without drawing) any leading zero digits.
 *        The ones digit is always drawn.
 * @param value    Number to draw (0-999 expected).
 * @param x        Starting X of the text cursor (g_text_cursor_x).
 * @param y        Starting Y of the text cursor (g_text_cursor_y).
 * @param ot_depth Ordering-table depth/priority each glyph is linked into.
 * @param clut     Font CLUT/color variant, added to g_text_atlas_base.
 * @see decomp.me (100%) https://decomp.me/scratch/RGs7q
 */
void field_draw_uint3(s32 value, s32 x, s32 y, s32 ot_depth, s32 clut)
{
    s32 remaining = value;
    s32 still_blanking = 1;
    s32 new_var;
    s32 quot;
    s32 prod;
    s32 digit;
    s32 sign = remaining >> 31;
    digit = remaining / 100;
    g_text_cursor_x = x;
    g_text_cursor_y = y;
    quot = digit;
    prod = quot * 100;
    digit = DIGIT_TO_ASCII(quot);
    if (digit == DIGIT_TO_ASCII(0))
    {
        g_text_cursor_x = x + 8;
    }
    else
    {
        func_800165CC(digit, ot_depth, clut);
        still_blanking = 0;
    }

    remaining -= prod;
    quot = remaining >> 31;
    sign = quot;
    quot = (digit = remaining / 10);
    prod = quot * 10;
    digit = DIGIT_TO_ASCII(quot);

    if (still_blanking == 0)
    {
        func_800165CC(digit, ot_depth, clut);
    }
    else if (digit == DIGIT_TO_ASCII(0))
    {
        g_text_cursor_x += 8;
    }
    else
    {
        func_800165CC(digit, ot_depth, clut);
    }

    func_800165CC(DIGIT_TO_ASCII(remaining - prod), ot_depth, clut);
}

/**
 * @brief Draw a value clamped to a byte as 2 hex digits.
 * @param value    Number to draw (clamped to 0-0xFF).
 * @param x        Starting X of the text cursor (g_text_cursor_x).
 * @param y        Starting Y of the text cursor (g_text_cursor_y).
 * @param ot_depth Ordering-table depth/priority each glyph is linked into.
 * @param clut     Font CLUT/color variant, added to g_text_atlas_base.
 * @see decomp.me (100%) https://decomp.me/scratch/S8Wds
 */
void field_draw_hex_byte(s32 value, s32 x, s32 y, s32 ot_depth, s32 clut)
{
    u8 table[17];
    s32 var_s1;
    u32 index;
    u32 new_var;
    u32 high;
    var_s1 = value;
    memcpy(table, g_hex_digit_table, 17);
    g_text_cursor_x = x;
    g_text_cursor_y = y;
    if (((u32)(var_s1 & 0xFFFF)) >= 0x100)
    {
        var_s1 = 0xFF;
    }
    index = (u32)(var_s1 & 0xFFFF);
    high = index >> 4;
    new_var = high << 4;
    func_800165CC(table[high], ot_depth, clut);
    func_800165CC(table[(unsigned short)((u32)((var_s1 - ((s32)new_var)) & 0xFFFF))], ot_depth, clut);
}

/**
 * @brief Draw a value masked to 16 bits as 4 hex digits, most significant first.
 * @param value    Number to draw (masked to 0-0xFFFF).
 * @param x        Starting X of the text cursor (g_text_cursor_x).
 * @param y        Starting Y of the text cursor (g_text_cursor_y).
 * @param ot_depth Ordering-table depth/priority each glyph is linked into.
 * @param clut     Font CLUT/color variant, added to g_text_atlas_base.
 * @see decomp.me (100%) https://decomp.me/scratch/4kW8K
 */
void field_draw_hex_word(s32 value, s32 x, s32 y, s32 ot_depth, s32 clut)
{
    /* local copy (stack area sp+0x10 to sp+0x20) */
    u8 table[17];
    u32 temp;

    /* Copy the unaligned data using memcpy (compiles to efficient byte loop) */
    memcpy(table, g_hex_digit_table, 17);

    /* match assembly order */
    g_text_cursor_x = x;
    temp = (u32)(value & 0xFFFF);
    g_text_cursor_y = y;

    /* Four calls using the four nibbles of the 16-bit value */
    func_800165CC(table[(temp >> 12) & 0xF], ot_depth, clut);
    func_800165CC(table[(temp >> 8) & 0xF], ot_depth, clut);
    func_800165CC(table[(temp >> 4) & 0xF], ot_depth, clut);
    func_800165CC(table[temp & 0xF], ot_depth, clut);
}

/**
 * decomp.me link (70%) https://decomp.me/scratch/1IyXY
 * Note that this code could be a completely different compiler toolchain
 * It might have been compiled with a different optimization level like -O1
 */
void func_800165CC(s32 arg0, s32 arg1, s32 arg2)
{
    int new_var;
    u32 t;
    CommandBuffer* cb;
    u32 new_var2;
    u8* new_var5;
    int new_var7;
    u8* new_var4;
    u32 base;
    NodeWithOffset16* node;
    u32 low;
    int new_var8;
    int new_var6;
    int new_var9;
    u32 high;
    int new_var3;
    u32 cb_addr;
    new_var3 = arg0 & 0xFF;
    t = new_var3;
    new_var7 = arg1 * 4;
    if (t != 0x20)
    {

        high = t;
        g_field_primitive_cursor->unk4 = 0x66808080;
        low = (u16)g_text_atlas_base;
        if (!g_text_cursor_x)
        {
        }
        base = (low + arg2) << 16;
        low = ((high & 0xF) * 8) + 0x80;
        if (!g_text_cursor_y)
        {
        }
        low = base | low;
        cb = g_field_primitive_cursor; /* Bug 1 fix: cb was uninitialized */
        new_var4 = (u8*)cb;
        high = (((t - 0x20) & ((short)0xF0)) >> 1) + 0xE0;
        new_var = low | (high << 8);
        g_field_primitive_cursor->unkC = new_var;
        g_field_primitive_cursor->unk8 = (g_text_cursor_y << 16) | g_text_cursor_x;
        new_var8 = (new_var6 = 0xFF000000);
        new_var5 = (u8*)g_field_current_render_half;
        g_field_primitive_cursor->unk10 = 0x80008;
        new_var9 = 0x04000000;
        node = (NodeWithOffset16*)(new_var5 + new_var7);
        new_var2 = node->unk10;
        g_field_primitive_cursor->unk0 = new_var9 | (node->unk10 & 0x00FFFFFF);
        cb_addr = (u32)cb;
        g_field_primitive_cursor = (CommandBuffer*)(new_var4 + 0x14);
        high = new_var2 & new_var8;
        node->unk10 = high | (cb_addr & 0x00FFFFFF); /* Bug 2 fix: removed (cb_addr = 0x00FFFFFF) assignment */
    }
    g_text_cursor_x += 8;
}

/**
 * decomp.me link (63%) https://decomp.me/scratch/7XlDl
 * Note that this code could be a completely different compiler toolchain
 * It might have been compiled with a different optimization level like -O1
 *
 * Local objdiff results (working/func_800166C8/): 40.8% at -O2, 67.2% at -O1
 * with this source. The remaining diffs look like non-gcc codegen: trapping
 * add (not addu) for the table indexing, ori+and for plain immediate masks,
 * the 5th arg loaded at 0x10(sp) BEFORE the sp decrement, and s0..s3,ra save
 * order. Same signatures in func_800165CC (addi a1,a1,0x80). Suspected
 * LSI-style toolchain, as in the PSX BIOS/kernel.
 *
 * The target asm passes arg3 (ot_depth) to BOTH func_800165CC calls
 * (addu s1,a1 / addu a1,s1 around the calls), so the second call below
 * passes arg3; an earlier revision wrongly passed new_var4 (0xF0).
 */
void func_800166C8(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4)
{
    u8 table[17]; /* unused; required to match the -0x40 frame size */
    short new_var3;
    signed char* new_var5;
    s32 temp4;
    int new_var6;
    signed char* base = g_hex_digit_table;
    signed char* new_var;
    short new_var7;
    int new_var4;
    unsigned short new_var8;
    int temp3;
    int new_var2;
    s32 temp0;
    s32 extra0;
    unsigned int extra1;
    s32 extra2;
    s32 extra3;
    temp4 = arg4;
    g_text_cursor_x = arg1;
    temp3 = arg3;
    new_var4 = arg3;
    temp0 = !temp3;
    temp0 = temp0 && (!temp3);
    new_var8 = 0xf;
    new_var2 = temp0 != 0;
    temp3 = 0xF0;
    new_var6 = new_var4;
    new_var3 = (!arg4) && (!new_var6);
    extra0 = (0, arg4);
    extra1 = new_var4;
    extra2 = arg4;
    extra3 = arg3;
    if (new_var2)
    {
    }
    if ((!g_hex_digit_table) && (!g_hex_digit_table))
    {
    }
    g_text_cursor_y = arg2;
    new_var7 = new_var3;
    new_var6 = (new_var4 = temp3);
    if (new_var7)
    {
    }
    temp0 = arg0;
    extra1 = (temp3 & ((int)temp0)) >> 3;
    func_800165CC(base[extra1 >> 1], arg3, temp4);
    new_var = base;
    func_800165CC(*(new_var5 = &new_var[new_var8 & temp0]), arg3, temp4);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/LO4aD
 * This function seems to be present in the JP release. Perhaps this is リングりんぐランド related.
 */
void func_80016764(void)
{
}
