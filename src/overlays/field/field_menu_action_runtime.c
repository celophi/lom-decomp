#include "common.h"

void func_800B2844();


/** @brief Enabled-choice mask and count stored for the menu. */
typedef struct
{
    u16 mask;
    u8 count;
} Selection;

/** @brief FieldActionGroupView view exposing group flags and menu entry bytes. */
typedef struct
{
    u8 pad0[0x26E4];
    u32 unk26E4;
    u8 pad26E8[4];
    u8 unk26EC;
    u8 pad26ED[7];
    u8 unk26F4;
} FieldActionGroupView;
extern u8 D_80122C00[], D_80122C0F, D_80122C1F, g_menuLayoutBuffer[], D_800F0E98[];
extern u8 D_80122C08[];
extern void func_800C7C88(void);
/**
 * @brief Prepare menu choice availability and display the first enabled entry.
 */
void func_800C766C(void)
{
    s32 values[8];
    s32 count, i, mask, limit, kind, record, record_offset, bit;
    s32 text_id;
    s32 *cursor, *mask_cursor;
    u8 *layout, *record_base, *output, *strings, *scan_base, *scan_ptr, *low, *high;
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
    do
    {
        *(u8 *)(i + (s32)output) = *((u8 *)values + i * 4);
        i++;
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
    ((Selection *)D_80122C08)->mask = mask;
    ((Selection *)D_80122C08)->count = limit;
    record_offset = record * 0x8C;
    record_base = g_menuLayoutBuffer;
    flags = ((FieldActionGroupView *)(record_base + record_offset))->unk26E4;
    kind = flags >> 8;
    kind &= 0xF;
    limit = flags & 0xF;
    do
    {
        if ((u32)((FieldActionGroupView *)(g_menuLayoutBuffer + record_offset + i * 0x10))->unk26F4 < 0xFF)
        {
            count++;
        }
        i++;
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
scan:
    scan_ptr = (u8 *)(i + (s32)scan_base);
    if (*scan_ptr == 0)
    {
        i++;
        if (i < 8)
        {
            goto scan;
        }
    }

    text_id = i + 0x58;
    strings = D_800F0E98;
    text_id *= 2;
    low = text_id + strings;
    {
        s32 offset2 = text_id + 1;
        high = offset2 + strings;
    }
    func_800B2844(4, (*low + (*high << 8)) + strings, 0xFF, kind);
    func_800C7C88();
}


/** @brief Remaining menu item mask and nonzero item count. */
typedef struct
{
    s16 mask;
    s8 count;
} Availability;
extern u8 g_menuLayoutBuffer[], D_80122C0B[], D_800F0E98[];
extern u8 D_80122C08[];
extern u8 D_80122C1F;
/**
 * @brief Add the selected menu item to its group and refresh availability.
 */
void func_800C7840(void)
{
    s32 available_mask;
    s32 slot_limit;
    s32 group;
    s32 capacity;
    u8 *layout_base;
    u8 *second_base;
    s32 second_offset;
    u8 *text_base;
    s32 group_offset;
    s32 initial_offset;
    s32 text_offset;
    s32 var_a0;
    s32 occupied_count;
    s32 slot_index;
    s32 item_index;
    s32 var_v0;
    s32 new_count;
    s32 item_id;
    s32 available_count;
    u32 group_flags;
    u8 *selected_count;
    u8 *item_counts;
    u8 *var_v0_2;
    FieldActionGroupView *group_view;

    occupied_count = 0;
    slot_index = occupied_count;
    layout_base = g_menuLayoutBuffer;
    group = D_80122C1F;
    initial_offset = group * 0x8C;
    group_flags = ((FieldActionGroupView *)(layout_base + initial_offset))->unk26E4;
    var_a0 = initial_offset;
    slot_limit = (group_flags >> 8) & 0xF;
    capacity = group_flags & 0xF;
    do
    {
        if (((FieldActionGroupView *)(initial_offset + slot_index * 0x10 + layout_base))->unk26F4 != 0xFF)
        {
            occupied_count += 1;
        }
        slot_index += 1;

    } while (slot_index < 8);
    capacity -= occupied_count;
    slot_index = 0;
    if (slot_limit != 0)
    {
        second_base = g_menuLayoutBuffer;
        second_offset = group * 0x8C;
        var_v0 = slot_index + second_offset;
    loop_6:
        if (((FieldActionGroupView *)(var_v0 + second_base))->unk26EC != 0xFF)
        {
            slot_index += 1;
            var_v0 = slot_index + second_offset;
            if (slot_index < slot_limit)
            {
                goto loop_6;
            }
        }
    }
    new_count = slot_index + 1;
    layout_base = g_menuLayoutBuffer;
    group_offset = group * 0x8C;
    group_view = (FieldActionGroupView *)(layout_base + group_offset);
    D_80122C0B[0] = new_count;
    group_view->unk26E4 = (s32)((group_view->unk26E4 & 0xFFFF0FFF) | ((new_count & 0xF) << 0xC));
    if (capacity < (slot_index + 3))
    {
        D_80122C0B[1] = 1;
    }
    item_counts = D_80122C0B - 0xB;
    text_base = D_800F0E98;
    selected_count = (*(s16 *)(D_80122C0B + 9)) + item_counts;
    item_id = (*(s16 *)(D_80122C0B + 9)) + 0x58;
    *selected_count -= 1;
    ((FieldActionGroupView *)(layout_base + slot_index + group_offset))->unk26EC = item_id;
    text_offset = item_id * 2;
    func_800B2844(0, text_base[text_offset] + (text_base[text_offset + 1] << 8) + text_base, 0xFF,
                  slot_index);
    available_mask = 0x1FFF;
    available_count = 0;
    item_index = available_count;

    do
    {
        if (item_counts[item_index] != 0)
        {
            available_mask &= ~(1 << item_index);
            available_count += 1;
        }
        item_index += 1;

    } while (item_index < 8);
    ((Availability *)D_80122C08)->mask = available_mask;
    ((Availability *)D_80122C08)->count = available_count;
}

extern void func_800C0260(s32, s32);
extern void func_800C7C88(void);
extern s32 rand(void);
extern u8 D_80122C1F;
/** @brief Layout buffer view exposing record metadata, entry slots, and shared counters. */
typedef struct Layout
{
    u8 pad[0x25E0];
    u8 counters[0x104];
    u32 packed;
    u8 gap[4];
    u8 slots[8];
    u8 entry;
} Layout;
extern u8 g_menuLayoutBuffer[];
/**
 * @brief Fill free record entries up to the clamped capacity and update linked counters.
 * @note Random slot selection retries at most 1000 times before scanning for a free slot.
 */
void func_800C7A3C(void)
{
    s32 temp_a0;
    s32 temp_s4;
    s32 temp_s6;
    s32 temp_s7;
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v1_2;
    s32 var_a0;
    s32 var_a1;
    s32 var_s0;
    s32 var_s1;
    s32 var_v0;
    s32 var_v1;
    s32 var_v1_2;
    u32 temp_v1;
    s32 temp_s2;

    u8 *temp_v1_4;

    var_s0 = 0;
    var_s1 = var_s0;
    temp_s2 = D_80122C1F;
    temp_v0 = temp_s2 * 0x8C;
    temp_v1 = ((Layout *)((u8 *)g_menuLayoutBuffer + temp_v0))->packed;
    var_a1 = temp_v1 & 0xF;
    var_a0 = var_s1 * 16 + temp_v0;
    temp_s7 = temp_v1 >> 8;
    temp_s7 &= 0xF;
    temp_s4 = temp_v1 >> 12;
    temp_s4 &= 0xF;
    temp_s4 += 2;
    do
   
   {
        if (((Layout *)((u8 *)g_menuLayoutBuffer + (var_a0)))->entry != 0xFF)
       
       {
            var_s0 += 1;
        }
        var_s1 += 1;
        var_a0 = var_s1 * 16 + temp_v0;
    } while (var_s1 < 8);
    var_a1 -= var_s0;
    if (temp_s4 >= 0)
   
   {
        var_v1 = var_a1;
        if (var_v1 >= temp_s4)
       
       {
            var_v1 = temp_s4;
        }
    }
    else
   
   {
        var_v1 = 0;
    }
    temp_s4 = var_v1;
    var_v1 = 0;
    var_s1 = var_v1;
    if (temp_s4 > 0)
   
   {
        temp_s6 = temp_s2 * 0x24;
        do
       
       {
            for (var_s0 = 0; var_s0 < 1000; var_s0++)
           
           {
                var_a1 = rand() / 4096;
                if (((Layout *)((u8 *)g_menuLayoutBuffer + var_a1 * 16 + temp_s2 * 0x8C))->entry ==
                    0xFF)
               
               {
                    break;
                }
            }
            if (((Layout *)((u8 *)g_menuLayoutBuffer + var_a1 * 16 + temp_s2 * 0x8C))->entry ==
                0xFF)
           
           {
                func_800C0260(temp_s2, var_a1);
            }
            else
           
           {
                for (var_s0 = 0; var_s0 < 8; var_s0++)
               
               {
                    if (((Layout *)((u8 *)g_menuLayoutBuffer + var_s0 * 16 + temp_s2 * 0x8C))
                            ->entry == 0xFF)
                   
                   {
                        func_800C0260(temp_s2, var_s0);
                        break;
                    }
                }
            }
            var_s1++;
        } while (var_s1 < temp_s4);
    }
    var_s1 = 0;
    if (temp_s7 != 0)
   
   {
        temp_v1_4 = (u8 *)g_menuLayoutBuffer;
        var_a1 = temp_s2 * 0x8C;
        do
       
       {
            var_v1 = ((Layout *)((var_s1 + var_a1) + (u32)temp_v1_4))->slots[0];
            if ((s32)var_v1 < 0xFF)
           
           {
                temp_v1_4[var_v1 + 0x25E0]--;
            }
            var_s1 += 1;

        } while (var_s1 < temp_s7);
    }
    func_800C7C88();
}


extern u8 D_800F0E98[];
extern s16 D_80122C14;
extern u8 D_80122C1F;
extern u8 g_menuLayoutBuffer[];

/**
 * @brief Clears the active menu entry's high flag nibble and status bytes.
 *
 * For the entry selected by D_80122C1F (stride 0x8C in g_menuLayoutBuffer),
 * masks off bits 12-15 of its 0x26E4 word and writes 0xFF to the four status
 * bytes at 0x26EC.
 */
void func_800C7C88(void)
{
    u8 *base = g_menuLayoutBuffer;
    u8 *base2;
    s32 offset = D_80122C1F * 0x8C;
    s32 offset2;
    s32 i;

    *(u32 *)(base + offset + 0x26E4) &= 0xFFFF0FFF;
    i = 0;
    base2 = base;
    offset2 = offset;
    for (; i < 4; i++)
   
   {
        base2[i + offset2 + 0x26EC] = 0xFF;
    }
}

/**
 * @brief Resets the eight menu action slots and clears a nibble of the flags.
 *
 * Reads the flags word at @c g_menuLayoutBuffer + 0x26E4, zeroes the eight
 * slots at stride 0x10 from +0x26F0 (each slot's word set to 0 and its status
 * byte at +4 set to 0xFF), then clears bits 12-15 of the flags word.
 */
void func_800C7CF8(void)
{
    u8 *base = g_menuLayoutBuffer;
    s32 flags = *(s32 *)(base + 0x26E4);

    *(s32 *)(base + 0x26F0) = 0;
    *(base + 0x26F4) = 0xFF;
    *(s32 *)(base + 0x2700) = 0;
    *(base + 0x2704) = 0xFF;
    *(s32 *)(base + 0x2710) = 0;
    *(base + 0x2714) = 0xFF;
    *(s32 *)(base + 0x2720) = 0;
    *(base + 0x2724) = 0xFF;
    *(s32 *)(base + 0x2730) = 0;
    *(base + 0x2734) = 0xFF;
    *(s32 *)(base + 0x2740) = 0;
    *(base + 0x2744) = 0xFF;
    *(s32 *)(base + 0x2750) = 0;
    *(base + 0x2754) = 0xFF;
    *(s32 *)(base + 0x2760) = 0;
    *(base + 0x2764) = 0xFF;

    *(s32 *)(base + 0x26E4) = flags & 0xFFFF0FFF;
}

/**
 * @brief Decodes a little-endian offset from the field table and dispatches
 *        the referenced entry.
 */
void func_800C7D5C(void)
{
    s32 offset = (D_80122C14 + 0x58) * 2;

    func_800B2844(4, D_800F0E98[offset] +
                         (D_800F0E98[offset + 1] << 8) + D_800F0E98, 0xFF);
}

extern u8 D_80122C0D;

void func_80087F44(s32 arg0, s32 *out);
s32 func_8008B288(s32 arg0);
s32 func_80087D8C(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/**
 * @brief Move the active field actor one step in its current heading.
 */
void func_800C7DB8(void)
{
    s32 pos[3];
    s32 x;
    s32 y;
    s32 z;
    s32 heading;
    s32 owner;

    owner = D_80122C0D;
    func_80087F44(0, pos);

    x = pos[0];
    if (x < 0)
   
   {
        x += 0xFF;
    }
    pos[0] = x >> 8;

    y = pos[1];
    if (y < 0)
   
   {
        y += 0xFF;
    }
    pos[1] = y >> 8;

    z = pos[2];
    if (z < 0)
   
   {
        z += 0xFF;
    }
    pos[2] = z >> 8;

    heading = func_8008B288(0);
    if (heading == 0)
   
   {
        pos[0] += 0x14;
    }
    else if ((u32)(heading - 1) < 0x3F)
   
   {
        pos[0] = pos[0] + 0xA;
        pos[2] = pos[2] - 0xA;
    }
    else if (heading == 0x40)
   
   {
        pos[2] -= 0xC;
    }
    else if ((u32)(heading - 0x41) < 0x3F)
   
   {
        pos[0] = pos[0] - 0xA;
        pos[2] = pos[2] - 0xA;
    }
    else if (heading == 0x80)
   
   {
        pos[0] -= 0x14;
    }
    else if ((u32)(heading - 0x81) < 0x3F)
   
   {
        pos[0] = pos[0] - 0xA;
        pos[2] = pos[2] + 0xA;
    }
    else if (heading == 0xC0)
   
   {
        pos[2] += 0xC;
    }
    else if ((u32)(heading - 0xC1) < 0x3F)
   
   {
        pos[0] = pos[0] + 0xA;
        pos[2] = pos[2] + 0xA;
    }

    func_80087D8C(owner, pos[0], pos[1] - 0xC, pos[2]);
}

typedef struct { u8 unk0; u8 pad[0x11]; u8 unk12; } FieldActionViewC7F44;
extern u8 D_80122C0D;
extern u8 g_menuLayoutBuffer[];
/** @brief Release the selected action slot and replenish its entry count. */
void func_800C7F44(void)
{
    s32 va;
    s32 vb;
    s32 value;
    s32 out;
    u8 idx;

    va = ((FieldActionViewC7F44 *)&D_80122C0D)->unk0;
    va -= 4;
    vb = ((FieldActionViewC7F44 *)&D_80122C0D)->unk12;
   
   {
        u8 *buf = g_menuLayoutBuffer;
        u8 *base = buf + (va * 0x10 + vb * 0x8C);
        idx = base[0x26F4];
    }
    value = g_menuLayoutBuffer[idx + 0x25E0];
    value += 1;
    if (value >= 0)
    {
        out = 0x63;
        if (value < 0x64)
        {
            out = value;
        }
    } else
    {
        out = 0;
    }
   
   {
        u8 *buf = g_menuLayoutBuffer;
        u8 *base;
        buf[idx + 0x25E0] = out;
        base = buf + (va * 0x10 + vb * 0x8C);
        *(s32 *)(base + 0x26F0) = 0;
        base[0x26F4] = 0xFF;
        base[0x26F8] = 0;
        base[0x26F9] = 0;
        base[0x26FA] = 0;
        base[0x26FB] = 0;
        base[0x26FC] = 0;
        base[0x26FD] = 0;
        base[0x26FE] = 0;
        base[0x26FF] = 0;
    }
}

typedef struct
{
    u8 unk0;
    u8 pad1[0xE];
    u8 unkF;
    u8 unk10;
    u8 pad11;
    u8 unk12;
} FieldActionViewC8014;
extern u8 D_80122C0D;
extern u8 g_menuLayoutBuffer[];
extern u8 D_800F0E98[];

/** @brief Read the selected action slot and dispatch its entry description. */
void func_800C8014(void)
{
    u8 *base;
    u8 *menu;
    u8 *table;
    u8 *lo;
    u8 *hi;
    s32 va;
    s32 offset;
    s32 offset2;
    s32 value;
    u8 idx;

    va = ((FieldActionViewC8014 *)&D_80122C0D)->unk0 - 4;
    menu = g_menuLayoutBuffer;
    table = D_800F0E98;
    base = menu + (va * 0x10 + ((FieldActionViewC8014 *)&D_80122C0D)->unk12 * 0x8C);
    value = *(volatile s32 *)(base + 0x26F0);
    ((FieldActionViewC8014 *)&D_80122C0D)->unk10 = value;
    idx = base[0x26F4];
    offset = idx * 2;
    lo = table + offset;
    offset2 = offset + 1;
    hi = table + offset2;
    func_800B2844(0, *lo + (*hi << 8) + table, 0xFF);
    ((FieldActionViewC8014 *)&D_80122C0D)->unkF = idx;
}

extern u8 g_menuLayoutBuffer[];
extern u8 D_80122C1F;
extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values[];

void func_800A8F8C(void *arg0, u8 *arg1);
void func_800C2A88(s32 arg0);

/**
 * @brief Copy the gosub-selected menu record into the first free slot.
 */
void func_800C80BC(void)
{
    s32 selected_index;
    u8 *selected_record;
    u8 *initial_base;
    u8 *scan_base;
    s32 record_index;
    s32 count;

    D_80122C1F = 0;
    if (g_gosub_result_count == 0)
    {
        return;
    }

    selected_index = g_gosub_result_values[0];
    initial_base = g_menuLayoutBuffer;
    {
        u8 *check_record = initial_base + selected_index * 0x40;
        if (*(s32 *)(check_record + 0xD18) == 0)
        {
            if (*(s32 *)(check_record + 0xD1C) == 0)
            {
                D_80122C1F = 2;
                return;
            }
        }
    }

    record_index = 0;
    scan_base = g_menuLayoutBuffer;
    selected_record = scan_base + selected_index * 0x40;
    while (record_index < 4)
    {
        u8 *record = (u8 *)((s32)g_menuLayoutBuffer + record_index * 0x40);
        if (record[0x3160] != 0)
        {
            if (*(s32 *)(selected_record + 0xD18) == *(s32 *)(record + 0x3198))
            {
                if (*(s32 *)(selected_record + 0xD1C) == *(s32 *)(record + 0x319C))
                {
                    D_80122C1F = 3;
                    return;
                }
            }
        }
        record_index++;
    }

    for (record_index = 0; record_index < 4; record_index++)
    {
        u8 *record = (u8 *)((s32)g_menuLayoutBuffer + record_index * 0x40);
        if (record[0x3160] == 0)
        {
            func_800A8F8C(g_menuLayoutBuffer + record_index * 0x40 + 0x3160, g_menuLayoutBuffer + 0xCE0 + selected_index * 0x40);
            func_800C2A88(selected_index);
            count = *(s32 *)(record + 0x3194);
            if (count == 0)
            {
                count = 1;
            }
            *(s32 *)(record + 0x3194) = count;
            D_80122C1F = 1;
            return;
        }
    }
}

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
} UnkStruct80051EB4;

extern UnkStruct80051EB4 D_80051EB4;
extern void field_open_gosub_screen_sequence(UnkStruct80051EB4 *arg0);

/** @brief Open the pending-record selection screen sequence. */
void func_800C8220(void)
{
    UnkStruct80051EB4 local;

    local = D_80051EB4;
    field_open_gosub_screen_sequence(&local);
}

typedef struct
{
    s32 x;
    s32 y;
    s32 z;
} FieldPosition;


s32 func_8008B288(s32 arg0);
s32 func_80087D8C(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/**
 * @brief Offset the current field position according to its heading and submit the result.
 */
void func_800C8260(void)
{
    FieldPosition pos;
    s32 dir;
    s32 x;
    s32 y;
    s32 z;

    func_80087F44(0, (s32 *)&pos);

    x = pos.x;
    if (x < 0)
   
   {
        x += 0xFF;
    }
    pos.x = x >> 8;

    y = pos.y;
    if (y < 0)
   
   {
        y += 0xFF;
    }
    pos.y = y >> 8;

    z = pos.z;
    if (z < 0)
   
   {
        z += 0xFF;
    }
    pos.z = z >> 8;

    dir = func_8008B288(0);
    if (dir == 0)
   
   {
        pos.x += 0xA;
    }
    else
   
   {
        if ((u32)(dir - 1) < 0x3F)
       
       {
            pos.x += 8;
            pos.z -= 8;
        }
        else if (dir == 0x40)
       
       {
            pos.z -= 0xA;
        }
        else if ((u32)(dir - 0x41) < 0x3F)
       
       {
            pos.x -= 8;
            pos.z -= 8;
        }
        else if (dir == 0x80)
       
       {
            pos.x -= 0xA;
        }
        else if ((u32)(dir - 0x81) < 0x3F)
       
       {
            pos.x -= 8;
            pos.z += 8;
        }
        else if (dir == 0xC0)
       
       {
            pos.z += 0xA;
        }
        else if ((u32)(dir - 0xC1) < 0x3F)
       
       {
            pos.x += 8;
            pos.z += 8;
        }
    }

    func_80087D8C(0xC, pos.x, pos.y, pos.z);
}

/**
 * @brief Advance the eight byte counters in active menu action slots.
 * @note Nonmatching m2c translation. Preserve the repeated byte increments
 * and their ordering; the target updates four records of eight slots.
 */
void func_800C83DC(void)
{
    s32 var_a2;
    s32 var_a3;
    s32 var_t0;
    s32 var_v0;
    u32 temp_v0;
    u8 temp_a1;
    u8 *temp_a0;

    var_t0 = 0;
    var_a3 = 0;
    do
    {
        var_a2 = 0;
        var_v0 = 0 * 0x10;
loop_2:
        temp_a0 = var_v0 + var_a3 + g_menuLayoutBuffer;
        if (temp_a0[0x26F4] != 0xFF)
        {
            temp_a1 = temp_a0[0x26F8];
            temp_v0 = temp_a1 & 0xFF;
            if (temp_v0 == 0)
            {
                temp_a0[0x26F8] = (u8) (temp_a1 + 1);
            } else if (temp_v0 < 0xF0U)
            {
                temp_a0[0x26F8] = (u8) (temp_a1 + 1);
                temp_a0[0x26F9] = (u8) (temp_a0[0x26F9] + 1);
                temp_a0[0x26FA] = (u8) (temp_a0[0x26FA] + 1);
                temp_a0[0x26FB] = (u8) (temp_a0[0x26FB] + 1);
                temp_a0[0x26FC] = (u8) (temp_a0[0x26FC] + 1);
                temp_a0[0x26FD] = (u8) (temp_a0[0x26FD] + 1);
                temp_a0[0x26FE] = (u8) (temp_a0[0x26FE] + 1);
                temp_a0[0x26FF] = (u8) (temp_a0[0x26FF] + 1);
                temp_a0[0x26F8] = (u8) (temp_a0[0x26F8] + 1);
                temp_a0[0x26F9] = (u8) (temp_a0[0x26F9] + 1);
                temp_a0[0x26FA] = (u8) (temp_a0[0x26FA] + 1);
                temp_a0[0x26FB] = (u8) (temp_a0[0x26FB] + 1);
                temp_a0[0x26FC] = (u8) (temp_a0[0x26FC] + 1);
                temp_a0[0x26FD] = (u8) (temp_a0[0x26FD] + 1);
                temp_a0[0x26FE] = (u8) (temp_a0[0x26FE] + 1);
                temp_a0[0x26FF] = (u8) (temp_a0[0x26FF] + 1);
                temp_a0[0x26F8] = (u8) (temp_a0[0x26F8] + 1);
                temp_a0[0x26F9] = (u8) (temp_a0[0x26F9] + 1);
                temp_a0[0x26FA] = (u8) (temp_a0[0x26FA] + 1);
                temp_a0[0x26FC] = (u8) (temp_a0[0x26FC] + 1);
                temp_a0[0x26FB] = (u8) (temp_a0[0x26FB] + 1);
                temp_a0[0x26FE] = (u8) (temp_a0[0x26FE] + 1);
                temp_a0[0x26FD] = (u8) (temp_a0[0x26FD] + 1);
                temp_a0[0x26F8] = (u8) (temp_a0[0x26F8] + 1);
                temp_a0[0x26FF] = (u8) (temp_a0[0x26FF] + 1);
                temp_a0[0x26FA] = (u8) (temp_a0[0x26FA] + 1);
                temp_a0[0x26F9] = (u8) (temp_a0[0x26F9] + 1);
                temp_a0[0x26FC] = (u8) (temp_a0[0x26FC] + 1);
                temp_a0[0x26FB] = (u8) (temp_a0[0x26FB] + 1);
                temp_a0[0x26FE] = (u8) (temp_a0[0x26FE] + 1);
                temp_a0[0x26FD] = (u8) (temp_a0[0x26FD] + 1);
                temp_a0[0x26F8] = (u8) (temp_a0[0x26F8] + 1);
                temp_a0[0x26FF] = (u8) (temp_a0[0x26FF] + 1);
                temp_a0[0x26F9] = (u8) (temp_a0[0x26F9] + 1);
                temp_a0[0x26FA] = (u8) (temp_a0[0x26FA] + 1);
                temp_a0[0x26FB] = (u8) (temp_a0[0x26FB] + 1);
                temp_a0[0x26FD] = (u8) (temp_a0[0x26FD] + 1);
                temp_a0[0x26FC] = (u8) (temp_a0[0x26FC] + 1);
                temp_a0[0x26FF] = (u8) (temp_a0[0x26FF] + 1);
                temp_a0[0x26FE] = (u8) (temp_a0[0x26FE] + 1);
                temp_a0[0x26F9] = (u8) (temp_a0[0x26F9] + 1);
                temp_a0[0x26F8] = (u8) (temp_a0[0x26F8] + 1);
                temp_a0[0x26FB] = (u8) (temp_a0[0x26FB] + 1);
                temp_a0[0x26FA] = (u8) (temp_a0[0x26FA] + 1);
                temp_a0[0x26FD] = (u8) (temp_a0[0x26FD] + 1);
                temp_a0[0x26FC] = (u8) (temp_a0[0x26FC] + 1);
                temp_a0[0x26FF] = (u8) (temp_a0[0x26FF] + 1);
                temp_a0[0x26FE] = (u8) (temp_a0[0x26FE] + 1);
                temp_a0[0x26F9] = (u8) (temp_a0[0x26F9] + 1);
                temp_a0[0x26F8] = (u8) (temp_a0[0x26F8] + 1);
                temp_a0[0x26FA] = (u8) (temp_a0[0x26FA] + 1);
                temp_a0[0x26FB] = (u8) (temp_a0[0x26FB] + 1);
                temp_a0[0x26FC] = (u8) (temp_a0[0x26FC] + 1);
                temp_a0[0x26FE] = (u8) (temp_a0[0x26FE] + 1);
                temp_a0[0x26FD] = (u8) (temp_a0[0x26FD] + 1);
                temp_a0[0x26F8] = (u8) (temp_a0[0x26F8] + 1);
                temp_a0[0x26FF] = (u8) (temp_a0[0x26FF] + 1);
                temp_a0[0x26FA] = (u8) (temp_a0[0x26FA] + 1);
                temp_a0[0x26F9] = (u8) (temp_a0[0x26F9] + 1);
                temp_a0[0x26FC] = (u8) (temp_a0[0x26FC] + 1);
                temp_a0[0x26FB] = (u8) (temp_a0[0x26FB] + 1);
                temp_a0[0x26FE] = (u8) (temp_a0[0x26FE] + 1);
                temp_a0[0x26FD] = (u8) (temp_a0[0x26FD] + 1);
                temp_a0[0x26F8] = (u8) (temp_a0[0x26F8] + 1);
                temp_a0[0x26FF] = (u8) (temp_a0[0x26FF] + 1);
                temp_a0[0x26FA] = (u8) (temp_a0[0x26FA] + 1);
                temp_a0[0x26F9] = (u8) (temp_a0[0x26F9] + 1);
                temp_a0[0x26FB] = (u8) (temp_a0[0x26FB] + 1);
                temp_a0[0x26FC] = (u8) (temp_a0[0x26FC] + 1);
                temp_a0[0x26FD] = (u8) (temp_a0[0x26FD] + 1);
                temp_a0[0x26FF] = (u8) (temp_a0[0x26FF] + 1);
                temp_a0[0x26FE] = (u8) (temp_a0[0x26FE] + 1);
            }
        }
        var_a2 += 1;
        var_v0 = var_a2 * 0x10;
        if (var_a2 < 8)
        {
            goto loop_2;
        }
        var_t0 += 1;
        var_a3 += 0x8C;
    } while (var_t0 < 4);
}

typedef struct
{
    u8 action_slot_id;
    u8 pad1[0x11];
    u8 record_index;
} FieldMenuCommandState;

typedef struct
{
    u8 pad0[0x26F0];
    s32 handle;
    u8 entry_index;
    u8 entry_state[3];
    u8 counters[8];
} FieldMenuActionSlotView;

extern u8 D_80122C0D;
extern u8 g_menuLayoutBuffer[];

/**
 * @brief Reset the selected menu action slot to its inactive state.
 * @note Matched under GCC 2.7.2 CDK. Folding the -4 adjustment into
 *       slot_index before constructing the record address makes GCC load
 *       action_slot_id before materializing the global base, matching the
 *       target schedule exactly.
 */
void field_reset_menu_action_slot(void)
{
    FieldMenuActionSlotView *slot;
    u8 *menu;
    s32 slot_index;

    slot_index = ((FieldMenuCommandState *)&D_80122C0D)->action_slot_id - 4;
    menu = g_menuLayoutBuffer;
    slot = (FieldMenuActionSlotView *)(menu + (slot_index * 0x10 + ((FieldMenuCommandState *)&D_80122C0D)->record_index * 0x8C));
    slot->entry_index = 0xFF;
    slot->handle = 0;
    slot->counters[0] = 0;
    slot->counters[1] = 0;
    slot->counters[2] = 0;
    slot->counters[3] = 0;
    slot->counters[4] = 0;
    slot->counters[5] = 0;
    slot->counters[6] = 0;
    slot->counters[7] = 0;
    *(s32 *)&slot->entry_index |= ~0xFF;
}
