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
 * decomp.me link (100%) https://decomp.me/scratch/RGs7q
 */
void func_8001627C(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4)
{
    s32 orig = arg0;
    s32 flag = 1;
    s32 new_var;
    s32 quot;
    s32 prod;
    s32 digit;
    s32 sign = orig >> 31;
    digit = orig / 100;
    g_text_cursor_x = arg1;
    g_text_cursor_y = arg2;
    quot = digit;
    prod = quot * 100;
    digit = quot + 0x30;
    if (digit == 0x30)
    {
        g_text_cursor_x = arg1 + 8;
    }
    else
    {
        func_800165CC(digit, arg3, arg4);
        flag = 0;
    }

    orig -= prod;
    quot = orig >> 31;
    sign = quot;
    quot = (digit = orig / 10);
    prod = quot * 10;
    digit = quot + 0x30;

    if (flag == 0)
    {
        func_800165CC(digit, arg3, arg4);
    }
    else if (digit == 0x30)
    {
        g_text_cursor_x += 8;
    }
    else
    {
        func_800165CC(digit, arg3, arg4);
    }

    func_800165CC((orig - prod) + 0x30, arg3, arg4);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/S8Wds
 */
void func_800163B4(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4)
{
    u8 table[17];
    s32 var_s1;
    u32 index;
    u32 new_var;
    u32 high;
    var_s1 = arg0;
    memcpy(table, D_800102B0, 17);
    g_text_cursor_x = arg1;
    g_text_cursor_y = arg2;
    if (((u32)(var_s1 & 0xFFFF)) >= 0x100)
    {
        var_s1 = 0xFF;
    }
    index = (u32)(var_s1 & 0xFFFF);
    high = index >> 4;
    new_var = high << 4;
    func_800165CC(table[high], arg3, arg4);
    func_800165CC(table[(unsigned short)((u32)((var_s1 - ((s32)new_var)) & 0xFFFF))], arg3, arg4);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/4kW8K
 */
void func_800164B0(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4)
{
    /* local copy (stack area sp+0x10 to sp+0x20) */
    u8 table[17];
    u32 temp;

    /* Copy the unaligned data using memcpy (compiles to efficient byte loop) */
    memcpy(table, D_800102B0, 17);

    /* match assembly order */
    g_text_cursor_x = arg1;
    temp = (u32)(arg0 & 0xFFFF);
    g_text_cursor_y = arg2;

    /* Four calls using the four nibbles of the 16-bit value */
    func_800165CC(table[(temp >> 12) & 0xF], arg3, arg4);
    func_800165CC(table[(temp >> 8) & 0xF], arg3, arg4);
    func_800165CC(table[(temp >> 4) & 0xF], arg3, arg4);
    func_800165CC(table[temp & 0xF], arg3, arg4);
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
        D_800473EC->unk4 = 0x66808080;
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
        cb = D_800473EC; /* Bug 1 fix: cb was uninitialized */
        new_var4 = (u8*)cb;
        high = (((t - 0x20) & ((short)0xF0)) >> 1) + 0xE0;
        new_var = low | (high << 8);
        D_800473EC->unkC = new_var;
        D_800473EC->unk8 = (g_text_cursor_y << 16) | g_text_cursor_x;
        new_var8 = (new_var6 = 0xFF000000);
        new_var5 = (u8*)D_800473F4;
        D_800473EC->unk10 = 0x80008;
        new_var9 = 0x04000000;
        node = (NodeWithOffset16*)(new_var5 + new_var7);
        new_var2 = node->unk10;
        D_800473EC->unk0 = new_var9 | (node->unk10 & 0x00FFFFFF);
        cb_addr = (u32)cb;
        D_800473EC = (CommandBuffer*)(new_var4 + 0x14);
        high = new_var2 & new_var8;
        node->unk10 = high | (cb_addr & 0x00FFFFFF); /* Bug 2 fix: removed (cb_addr = 0x00FFFFFF) assignment */
    }
    g_text_cursor_x += 8;
}

/**
 * decomp.me link (63%) https://decomp.me/scratch/7XlDl
 * Note that this code could be a completely different compiler toolchain
 * It might have been compiled with a different optimization level like -O1
 */
void func_800166C8(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4)
{
    short new_var3;
    signed char* new_var5;
    s32 temp4;
    int new_var6;
    signed char* base = D_800102B0;
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
    if ((!D_800102B0) && (!D_800102B0))
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
    func_800165CC(*(new_var5 = &new_var[new_var8 & temp0]), new_var4, temp4);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/LO4aD
 * This function seems to be present in the JP release. Perhaps this is リングりんぐランド related.
 */
void func_80016764(void)
{
}