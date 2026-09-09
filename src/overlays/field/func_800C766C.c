#include "common.h"

/** @brief Enabled-choice mask and count stored for the menu. */
typedef struct
{
    u16 mask;
    u8 count;
} Selection;
extern u8 D_80122C00[], D_80122C0F, D_80122C1F, g_menuLayoutBuffer[], D_800F0E98[];
extern Selection D_80122C08;
extern void func_800B2844(s32, u8 *, u8);
extern void func_800C7C88(void);
/** @brief Prepare menu choice availability and display the first enabled entry. */
void func_800C766C(void)
{
    s32 values[8];
    s32 count, i, mask, offset, limit, kind, record, record_offset, bit;
    s32 *cursor, *copy_cursor, *mask_cursor;
    u8 *layout, *output, *strings, *scan_base, *scan_ptr, *low, *high;
    u32 flags;
    record = D_80122C1F;
    count = 0;
    i = count;
    layout = g_menuLayoutBuffer;
    cursor = values;
    do
    {
        *cursor = layout[i + 0x2638];
        if (*cursor != 0)
        {
            count++;
        }
        i++;
        cursor++;
    } while (i < 8);
    i = 0;
    output = D_80122C00;
    copy_cursor = values;
    do
    {
        {
            s32 value = *copy_cursor;
            i++;
            output[i - 1] = value;
            copy_cursor++;
        }
    } while (i < 8);
    mask = 0x1FFF;
    i = 0;
    bit = 1;
    mask_cursor = values;
    do
    {
        if (*mask_cursor != 0)
        {
            mask &= ~(bit << i);
        }
        i++;
        mask_cursor++;
    } while (i < 8);
    limit = count + 1;
    count = 0;
    i = count;
    layout = g_menuLayoutBuffer;
    D_80122C08.mask = mask;
    D_80122C08.count = limit;
    record_offset = record * 0x8C;
    flags = *(u32 *)(layout + record_offset + 0x26E4);
    kind = (flags >> 8) & 0xF;
    limit = flags & 0xF;
    offset = record_offset;
    do
    {
        if ((u32)layout[record_offset + i * 0x10 + 0x26F4] < 0xFF)
        {
            count++;
        }
        i++;
        offset += 0x10;
    } while (i < 8);
    if (count >= limit)
    {
        D_80122C0F = 0xFF;
    }
    else
    {
        D_80122C0F = kind;
    }
    i = 0;
    scan_base = D_80122C00;
    scan_ptr = i + scan_base;
scan:
    do
    {
        offset = i + 0x58;
        if (*scan_ptr != 0)
        {
            goto found;
        }
        i++;
        scan_ptr = i + scan_base;
        if (i < 8)
        {
            goto scan;
        }
        offset = i + 0x58;
    } while (0);
found:
    strings = D_800F0E98;
    offset = offset * 2;
    low = offset + strings;
    offset++;
    high = offset + strings;
    func_800B2844(4, (*low + (*high << 8)) + strings, 0xFF);
    func_800C7C88();
}
