#include "game_audio.h"
#include "saved_game.h"
#include "common.h"



u8 *field_find_free_inventory_record();
void field_copy_inventory_record();
void field_open_gosub_screen_sequence();
void func_80087F44();
s32 func_80087D8C();
void func_800B2844();
extern s32 D_80122C00;
extern u8 D_80122C01;
extern u8 D_80122C02;
extern u8 D_80122C03;
extern u8 D_80122C04;
extern u8 D_80122C05;
extern s16 D_80122C06;
extern s16 D_80122C08;
extern s16 D_80122C0A;
extern u8 D_80122C0B[];
extern u8 D_80122C0C;
extern u8 D_80122C0D;
extern u16 D_80122C0E;
extern u8 D_80122C0F;
extern s16 D_80122C10;
extern u8 D_80122C11;
extern s8 D_80122C12;
extern s16 D_80122C14;
extern u16 D_80122C16;
extern u8 D_80122C19;
extern s16 D_80122C1C;
extern u8 D_80122C1E;
extern u8 D_80122C1F;
extern u8 D_80122A08[];
extern u8 D_80043CB8[];
extern u8 D_80051EB4;
extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values[];
extern u8 D_800459AF;
extern s8 D_800459B3;
extern void func_800C4364(s32);
extern void func_800A54D0(void);
extern void field_compact_inventory(void);


extern void (*D_800F19D8[])(s32 arg0);

void func_800C5704(s32 arg0)
{
    if (arg0 < 0x60)
    {
        D_800F19D8[arg0](arg0);
        return;
    }
    record_game_diagnostic(0x8002, arg0, 0, 0);
}


typedef struct
{
    s32 unk0;
    s16 unk4;
} GroupStateView;




/**
 * @brief Selects a menu state from the active record and history index.
 *
 * Stores state 1 or 2 when the active record has index 3. Otherwise, stores
 * state 3 when the record index matches the signed history index, or state 4
 * when it does not.
 */
void func_800C5760(void)
{
    u8 *menu;
    u8 *record;

    menu = g_saved_game.bytes;
    record = menu + ((GroupStateView *)&D_80122C00)->unk0;
    if (record[0x29D8] == 3)
    {
        if (menu[0x29D7] >= 3U)
        {
            ((GroupStateView *)&D_80122C00)->unk4 = 1;
            return;
        }
        ((GroupStateView *)&D_80122C00)->unk4 = 2;
        return;
    }
    if (record[0x29D8] == *(s8 *)&menu[0x29D7])
    {
        ((GroupStateView *)&D_80122C00)->unk4 = 3;
        return;
    }
    ((GroupStateView *)&D_80122C00)->unk4 = 4;
}


extern s32 D_801227F0;

void func_800C57D4(void)
{
    D_801227F0 = 0;
}


extern s32 D_800F19AC;

/**
 * @brief Thin stack-frame wrapper around field_open_gosub_screen_sequence passing &D_800F19AC.
 */
void func_800C57E0(void)
{
    field_open_gosub_screen_sequence(&D_800F19AC);
}

/**
 * @brief Creates a group from selected inventory records.
 */
void func_800C5804(void)
{
    s32 entry_offset;
    s32 packed_flags;
    s32 scan_index;
    s32 candidate_index;
    s32 index;
    s32 unused_index;
    u8 count;
    u8 record_index;

    unused_index = 3;
    index = 0;
    do
    {
        candidate_index = index;
        scan_index = 0;
        do
        {
            if (g_saved_game.bytes[scan_index + 0x29D8] == index)
            {
                candidate_index = 3;
            }
            scan_index += 1;
        } while (scan_index < 3);
        if (candidate_index != 3)
        {
            unused_index = candidate_index;
        }
        index += 1;
    } while (index < 3);
    if (unused_index != 3)
    {
        func_800C4364(unused_index);
        count = g_saved_game.bytes[0x29D5] + 1;
        g_saved_game.bytes[0x29D5] = count;
        if ((u32) (count & 0xFF) >= 0xC9U)
        {
            g_saved_game.bytes[0x29D5] = 0xC8U;
        }
        record_index = g_saved_game.bytes[D_80122C00 + 0x29D8];
        if (record_index != 3)
        {
            if (g_saved_game.bytes[0x29D8] == 3)
            {
                g_saved_game.bytes[0x29D8] = record_index;
            }
            else if (g_saved_game.bytes[0x29D9] == 3)
            {
                g_saved_game.bytes[0x29D9] = record_index;
            }
            else if (g_saved_game.bytes[0x29DA] == 3)
            {
                g_saved_game.bytes[0x29DA] = record_index;
            }
        }
        g_saved_game.bytes[D_80122C00 + 0x29D8] = unused_index;
        packed_flags = ((*(s32 *)&g_saved_game.bytes[0x29D4]) & ~0xF) | (((g_saved_game.bytes[0x29D4] & 0xF) + 1) & 0xF);
        (*(s32 *)&g_saved_game.bytes[0x29D4]) = packed_flags;
        if ((u32) (g_saved_game.bytes[0x29D4] & 0xF) >= 4U)
        {
            (*(s32 *)&g_saved_game.bytes[0x29D4]) = (s32) ((packed_flags & ~0xF) | 3);
        }
        index = 0;
        if (g_gosub_result_count > 0)
        {
            do
            {
                u8 *destination;

                destination = (g_saved_game.bytes[D_80122C00 + 0x29D8] * 0x14C) + (g_saved_game.bytes + 0x2B58);
                destination += index << 6;
                field_copy_inventory_record(destination, (g_gosub_result_values[index] << 6) + (g_saved_game.bytes + 0xCE0));
                g_saved_game.bytes[(g_gosub_result_values[index] << 6) + 0xCE0] = 0;
                index += 1;
            } while (index < g_gosub_result_count);
        }
        field_compact_inventory();
        index = g_gosub_result_count;
        if (index < 4)
        {
            do
            {
                entry_offset = index << 6;
                index += 1;
                g_saved_game.bytes[entry_offset + g_saved_game.bytes[D_80122C00 + 0x29D8] * 0x14C + 0x2B58] = 0;
            } while (index < 4);
        }
        func_800A54D0();
    }
}


extern s32 D_800F19B8;

/**
 * @brief Thin stack-frame wrapper around field_open_gosub_screen_sequence passing &D_800F19B8.
 */
void func_800C5AA8(void)
{
    field_open_gosub_screen_sequence(&D_800F19B8);
}

extern s32 D_800F19C4;

void func_800C5ACC(void)
{
    field_open_gosub_screen_sequence(&D_800F19C4);
}

void func_800C5AF0(void)
{
    func_800AD0C8();
}


typedef struct
{
    s32 unk0;
    u8 pad4[2];
    s16 unk6;
    u8 pad8[0x16];
    s16 unk1E;
} GroupClassView;

typedef struct
{
    u8 pad[0x29D8];
    u8 unk29D8;
} Rec29D8;

typedef struct
{
    u8 pad[0x2B50];
    u8 unk2B50;
} Rec2B50;




void func_800C5B10(void)
{
    s32 temp_v1;
    Rec2B50 *p;
    u8 *menu;

    menu = g_saved_game.bytes;
    temp_v1 = ((Rec29D8 *)(((GroupClassView *)&D_80122C00)->unk0 + menu))->unk29D8;
    p = (Rec2B50 *)((temp_v1 * 0x14C) + menu);
    ((GroupClassView *)&D_80122C00)->unk6 = temp_v1;
    ((GroupClassView *)&D_80122C00)->unk1E = p->unk2B50 & 0xF;
}




/**
 * @brief Selects the menu state associated with the current history slot.
 *
 * Clamps the signed history-slot byte at offset 0x29D7 to 3, then selects
 * state 2 when the low seven bits of the word at 0xAA8 equal 3. Otherwise it
 * selects state 0 for history slot 3 and state 1 for the remaining slots.
 */
void func_800C5B64(void)
{
    if (g_saved_game.bytes[0x29D7] >= 4U)
    {
        g_saved_game.bytes[0x29D7] = 3;
    }
    if ((*(u32 *)&g_saved_game.bytes[0xAA8] & 0x7F) == 3)
    {
        D_80122C10 = 2;
    }
    else if (*(s8 *)&g_saved_game.bytes[0x29D7] == 3)
    {
        D_80122C10 = 0;
    }
    else
    {
        D_80122C10 = 1;
    }
}



/**
 * @brief Clear the selected menu group and release its four active records.
 * @note Refresh the selection after each allocation call and preserve the packed count.
 */
void func_800C5BCC(void)
{
    s32 i, offset, group_offset;
    u8 *layout, *scan, *entries, *final_layout, *first_layout, *loop_base;
    u32 flags;
    void *item;
    first_layout = g_saved_game.bytes;
    i = 0;
    if (first_layout[0x29D6] != 0)
    {
        loop_base = first_layout;
        scan = loop_base;
        do
        {
            flags = *(u32 *)(loop_base + i * 4 + 0x29DC);
            if ((flags & 3) == loop_base[D_80122C00 + 0x29D8])
            {
                *(u32 *)(loop_base + i * 4 + 0x29DC) = (flags | 3) & 0xFFFEFFFF;
            }
            i++;
            scan += 4;
        } while (i < loop_base[0x29D6]);
    }
    i = 0;
    do
    {
        group_offset = g_saved_game.bytes[D_80122C00 + 0x29D8] * 0x14C;
        layout = g_saved_game.bytes;
        entries = layout + 0x2B58;
        offset = i * 0x40;
        if (layout[offset + group_offset + 0x2B58] != 0)
        {
            if (field_find_free_inventory_record() != 0)
            {
                item = field_find_free_inventory_record();
                group_offset = layout[D_80122C00 + 0x29D8] * 0x14C;
                field_copy_inventory_record(item, group_offset + entries + offset);
            }
        }
        i++;
    } while (i < 4);
    final_layout = g_saved_game.bytes;
    final_layout[final_layout[D_80122C00 + 0x29D8] * 0x14C + 0x2B0C] = 0;
    final_layout[D_80122C00 + 0x29D8] = 3;
    flags = *(u32 *)(final_layout + 0x29D4);
    if ((flags & 0xF) != 0)
    {
        *(u32 *)(final_layout + 0x29D4) =
            (flags & ~0xF) | (((final_layout[0x29D4] & 0xF) - 1) & 0xF);
    }
}





void func_800C5DA8(void)
{
    u8 idx = g_saved_game.bytes[D_80122C00 + 0x29D8];

    func_800B2844(0, &g_saved_game.bytes[idx * 332 + 0x2B0C], 0xFF);
}

/**
 * @brief Clear D_80122C10 when there are no gosub results.
 */
void func_800C5E08(void)
{
    if (g_gosub_result_count == 0)
    {
        D_80122C10 = 0;
    }
}

/**
 * @brief Repair the group display order and publish the current selection state.
 */
void func_800C5E28(void)
{
    s32 order[3];
    s32 inverse[3];
    s16* slot_status;
    s32* inverse_entry;
    u8* layout;
    s32 packed_order;
    s32 clamped_order_0;
    s32 clamped_order_1;
    s32 clamped_order_2;
    s32 i;
    s32 active_index;
    s8 selected_index;
    s32 j;

    slot_status = &D_80122C06;
    slot_status[0] = 3;
    slot_status[1] = 3;
    slot_status[2] = 3;
    if (g_saved_game.bytes[0x29D8] != *(s8*)&g_saved_game.bytes[0x29D7])
    {
        slot_status[0] = (s16)g_saved_game.bytes[0x29D8];
    }
    if (g_saved_game.bytes[0x29D9] != *(s8*)&g_saved_game.bytes[0x29D7])
    {
        slot_status[1] = (s16)g_saved_game.bytes[0x29D9];
    }
    if (g_saved_game.bytes[0x29DA] != *(s8*)&g_saved_game.bytes[0x29D7])
    {
        slot_status[2] = (s16)g_saved_game.bytes[0x29DA];
    }
    packed_order = g_saved_game.bytes[0x29DB];
    i = packed_order & 3;
    order[1] = (packed_order >> 2) & 3;
    order[0] = i;
    order[2] = (packed_order >> 4) & 3;
    if (i >= 0)
    {
        clamped_order_0 = 2;
        if (i < 3)
        {
            clamped_order_0 = i;
        }
    }
    else
    {
        clamped_order_0 = 0;
    }
    order[0] = clamped_order_0;
    if (order[1] >= 0)
    {
        clamped_order_1 = 2;
        if (order[1] < 3)
        {
            clamped_order_1 = order[1];
        }
    }
    else
    {
        clamped_order_1 = 0;
    }
    order[1] = clamped_order_1;
    if (order[2] >= 0)
    {
        clamped_order_2 = 2;
        if (order[2] < 3)
        {
            clamped_order_2 = order[2];
        }
    }
    else
    {
        clamped_order_2 = 0;
    }
    order[2] = clamped_order_2;
    inverse[0] = 3;
    inverse[1] = 3;
    inverse[2] = 3;
    i = 0;
    do
    {
        j = 0;
        do
        {
            if (order[i] == j)
            {
                if (inverse[j] == 3)
                {
                    inverse[j] = i;
                }
                else
                {
                    order[i] = 3;
                }
            }
            j += 1;
        } while (j < 3);
        i += 1;
    } while (i < 3);
    i = 0;
    do
    {
        if (order[i] == 3)
        {
            j = 0;
            do
            {
                inverse_entry = &inverse[j];
                if (*inverse_entry == 3)
                {
                    order[i] = j;
                    j = 3;
                    *inverse_entry = i;
                }
                j += 1;
            } while (j < 3);
        }
        i += 1;
    } while (i < 3);
    packed_order = order[0] + (order[1] * 4) + (order[2] * 0x10);
    D_800459B3 = packed_order;
    if (D_80122C06 == 3)
    {
        if (order[0] == 0)
        {
            D_80122C06 = 4;
        }
        if (order[1] == 0)
        {
            D_80122C06 = 5;
        }
        if (order[2] == 0)
        {
            D_80122C06 = 6;
        }
    }
    if (D_80122C08 == 3)
    {
        if (order[0] == 1)
        {
            D_80122C08 = 4;
        }
        if (order[1] == 1)
        {
            D_80122C08 = 5;
        }
        if (order[2] == 1)
        {
            D_80122C08 = 6;
        }
    }
    if (D_80122C0A == 3)
    {
        if (order[0] == 2)
        {
            D_80122C0A = 4;
        }
        if (order[1] == 2)
        {
            D_80122C0A = 5;
        }
        if (order[2] == 2)
        {
            D_80122C0A = 6;
        }
    }
    layout = g_saved_game.bytes;
    selected_index = *(s8*)&layout[0x29D7];
    if (selected_index < 3)
    {
        if (layout[0x29D8] == selected_index)
        {
            D_80122C06 = 3;
        }
        if (layout[0x29D9] == selected_index)
        {
            D_80122C08 = 3;
        }
        if (layout[0x29DA] == selected_index)
        {
            D_80122C0A = 3;
        }
    }
    active_index = D_80122C00;
    i = 0;
    do
    {
        if (order[i] == active_index)
        {
            D_80122C1C = i;
        }
        i += 1;
    } while (i < 3);
    (*(s16 *)&D_80122C1E) = (s16)(s8)D_800459AF;
}

#include "game_audio.h"
#include "saved_game.h"
#include "common.h"

/**
 * @brief Object record at D_80122C0C shared by the wrappers below.
 */
typedef struct
{
    s16 unk0; /* 0x00 */
    s16 unk2; /* 0x02 */
    u8 pad4[2];
    s16 unk6; /* 0x06 */
    s16 unk8; /* 0x08 */
    s16 unkA; /* 0x0A */
    s16 unkC; /* 0x0C */
} UnkStruct80122C0C;


extern u8 D_800459AE;


/**
 * @brief Set D_80122C10 to 1 when D_800459AE is at least 0x28, else 0.
 */
void func_800C61D8(void)
{
    if (D_800459AE >= 0x28)
    {
        D_80122C10 = 1;
    }
    else
    {
        D_80122C10 = 0;
    }
}

/**
 * @brief Thin wrapper around func_800AD0C8.
 */
void func_800C6208(void)
{
    func_800AD0C8();
}

/**
 * @brief Dispatch each populated row of the selected menu entry and count them.
 */
void func_800C6228(void)
{
    s32 count;
    s32 i;
    s32 entry_offset;
    s32 row_offset;
    u8 *menu;
    u8 *base;

    count = 0;
    i = 0;
    menu = g_saved_game.bytes;
    base = menu + 0x2B58;
loop:
    row_offset = i << 6;
    entry_offset = menu[D_80122C00 + 0x29D8] * 0x14C;
    if (menu[row_offset + entry_offset + 0x2B58] != 0)
    {
        entry_offset += (s32)base;
        func_800B2844(count, (u8 *)(entry_offset + row_offset), 0xFF);
        count += 1;
    }
    i += 1;
    if (i < 4)
    {
        goto loop;
    }
    D_80122C10 = count;
}

/**
 * @brief Count the empty 0x40-byte slots and subtract them from D_80122C10, clamping at 0.
 */
void func_800C62E8(void)
{
    s32 i;
    s32 count;
    u8 *p;
    u16 v0;
    s16 *ptr;

    count = 0;
    for (i = 0; i < 0x64; i++)
    {
        p = &g_saved_game.bytes[i * 0x40];
        if (p[0xCE0] == 0)
        {
            count++;
        }
    }
    ptr = &D_80122C10;
    if (count >= *ptr)
    {
        v0 = 0;
    }
    else
    {
        v0 = (u16) *ptr - count;
    }
    *ptr = v0;
}

/**
 * @brief Thin stack-frame wrapper around func_800C3A00 with a fixed arg.
 */
void func_800C6344(void)
{
    func_800C3A00(0x92BC);
}

/**
 * @brief Forward the D_80122C0C object id and position pair to func_80087680.
 */
void func_800C6364(void)
{
    func_80087680((*(UnkStruct80122C0C *)&D_80122C0C).unk0, (*(UnkStruct80122C0C *)&D_80122C0C).unk2, (*(UnkStruct80122C0C *)&D_80122C0C).unk2, 0, 0, 0);
}

/**
 * @brief Fetch a 3D position for the current object, scale it down and re-emit it.
 *
 * Queries func_80087F44 for the object named by @c D_80122C0C.unk0 into a local
 * vector, divides each component by 256 (rounding toward zero via the +0xFF
 * negative fix-up), subtracts @c D_80122C0C.unk6 from the Y component, and passes
 * the result to func_80087D8C.
 */
void func_800C63A0(void)
{
    s32 vec[3];
    s32 x;
    s32 y;
    s32 z;
    s32 yshift;
    s32 y2;

    func_80087F44((*(UnkStruct80122C0C *)&D_80122C0C).unk0, vec);
    x = vec[0];
    if (x < 0)
    {
        x += 0xFF;
    }
    y = vec[1];
    x >>= 8;
    vec[0] = x;
    if (y < 0)
    {
        y += 0xFF;
    }
    z = vec[2];
    yshift = y >> 8;
    vec[1] = yshift;
    if (z < 0)
    {
        z += 0xFF;
    }
    z >>= 8;
    vec[2] = z;
    y2 = yshift - (*(UnkStruct80122C0C *)&D_80122C0C).unk6;
    vec[1] = y2;
    func_80087D8C((*(UnkStruct80122C0C *)&D_80122C0C).unk0, x, y2, z);
}

/**
 * @brief Move the selected object toward its target position with decreasing steps.
 *
 * Fetches the object's position, converts it from fixed-point units, moves the
 * horizontal coordinates toward their targets while below the target height,
 * then moves the vertical coordinate toward its target.
 */
void func_800C642C(void)
{
    s32 pos[3];
    s32 target_y;
    s32 target_x;
    s32 target_z;
    s32 scaled_y;
    s32 x_delta;
    s32 z_delta;
    s32 y_delta;
    s32 scaled_x;
    s32 raw_x;
    s32 raw_z;
    s32 raw_y;
    s32 x_distance;
    s32 z_distance;
    s32 y_distance;

    func_80087F44((*(UnkStruct80122C0C *)&D_80122C0C).unk0, pos);
    raw_x = pos[0];
    if (raw_x < 0)
    {
        raw_x += 0xFF;
    }
    raw_y = pos[1];
    scaled_x = raw_x >> 8;
    pos[0] = scaled_x;
    if (raw_y < 0)
    {
        raw_y += 0xFF;
    }
    raw_z = pos[2];
    scaled_y = raw_y >> 8;
    pos[1] = scaled_y;
    if (raw_z < 0)
    {
        raw_z += 0xFF;
    }
    pos[2] = raw_z >> 8;

    target_y = -(*(UnkStruct80122C0C *)&D_80122C0C).unkC;
    target_x = (*(UnkStruct80122C0C *)&D_80122C0C).unk8;
    target_z = (*(UnkStruct80122C0C *)&D_80122C0C).unkA;

    if ((scaled_y == target_y) || (scaled_y < target_y))
    {
        x_delta = scaled_x - target_x;
        x_distance = x_delta;
        if (x_delta < 0)
        {
            x_delta++;
            x_delta--;
            x_distance = -x_distance;
        }
        if ((x_distance * 2) >= 4)
        {
            pos[0] = target_x + ((x_delta * 2) / 3);
        }
        else if (x_distance > 0)
        {
            pos[0] = scaled_x - (x_delta / x_distance);
        }
        else
        {
            pos[0] = target_x;
        }
    }

    if ((pos[1] == target_y) || (pos[1] < target_y))
    {
        z_delta = pos[2] - target_z;
        z_distance = z_delta;
        if (z_delta < 0)
        {
            z_delta++;
            z_delta--;
            z_distance = -z_distance;
        }
        if ((z_distance * 2) >= 4)
        {
            pos[2] = target_z + ((z_delta * 2) / 3);
        }
        else if (z_distance > 0)
        {
            pos[2] -= z_delta / z_distance;
        }
        else
        {
            pos[2] = target_z;
        }
    }

    if (((pos[0] == target_x) && (pos[2] == target_z)) || (pos[1] >= target_y))
    {
        y_delta = pos[1] - target_y;
        y_distance = y_delta;
        if (y_delta < 0)
        {
            y_delta++;
            y_delta--;
            y_distance = -y_distance;
        }
        if ((y_distance * 2) >= 4)
        {
            pos[1] = target_y + ((y_delta * 2) / 3);
        }
        else if (y_distance > 0)
        {
            pos[1] -= y_delta / y_distance;
        }
        else
        {
            pos[1] = target_y;
        }
    }

    func_80087D8C((*(UnkStruct80122C0C *)&D_80122C0C).unk0, pos[0], pos[1], pos[2]);
}


typedef struct
{
    u8 raw[0x25];
} CopyBuf;

extern CopyBuf D_80051CBC;


void func_800A54D0(void);

/**
 * @brief Translate the selected resource id to a layout palette index and upload it.
 */
void func_800C66DC(void)
{
    CopyBuf tmp;
    s32 offset;
    u16 half_val;
    s32 raw_val;
    s32 clamped;
    u8 *base;
    u8 idx;

    tmp = D_80051CBC;

    offset = g_gosub_result_values[0];
    half_val = *(u16 *)g_gosub_result_values;
    raw_val = tmp.raw[offset - 0x60];
    *(u16 *)&(*(s32 *)&D_80122C08) = half_val;

    if (raw_val >= 0x20)
    {
        raw_val = 0;
    }

    if (raw_val >= 0)
    {
        clamped = 0x1F;
        if (raw_val < 0x20)
        {
            clamped = raw_val;
        }
    }
    else
    {
        clamped = 0;
    }

    base = g_saved_game.bytes;
    idx = base[D_80122C00 + 0x29D8];
    *(s32 *)(base + idx * 332 + 0x2B54) = clamped;
    func_800A54D0();
}


extern u8 D_800459AF;

/**
 * @brief Copy the selected status to the diagnostic and layout state.
 * @see decomp.me (100%) N/A -- trivial 7-instruction leaf function, no scratch needed.
 */
void func_800C6834(void)
{
    s32 temp = D_80122C06;
    g_game_diagnostic_status = temp;
    D_800459AF = temp;
}


extern s32 func_800A4744(void);
extern s32 func_800A4778(void);
extern s32 D_800F19CC;

/** @brief Store the selected result, using the alternate result on failure. */
void func_800C6850(void)
{
    s32 result = func_800A4744();

    if (result < 0)
    {
        D_80122C16 = 1;
        *(&D_80122C16 - 1) = func_800A4778();
    }
    else
    {
        D_80122C16 = 0;
        *(&D_80122C16 - 1) = result;
    }
}

/**
 * @brief Thin stack-frame wrapper around field_open_gosub_screen_sequence passing &D_800F19CC.
 */
void func_800C68A4(void)
{
    field_open_gosub_screen_sequence(&D_800F19CC);
}

#include "game_audio.h"
#include "saved_game.h"
#include "common.h"



extern s8 D_800459B3;

typedef struct
{
    s16 unk0;
    u8 pad2[0x12 - 2];
    s16 unk12;
} StructD80122C0A;


/**
 * @brief Reconcile the selected layout entry with the packed three-slot ordering.
 */
void func_800C68C8(void)
{
    s32 order[3];
    s32 active_slot;
    s32 matching_index;
    s32 packed_order;
    s32 active_index;
    s8 selected_index;

    packed_order = g_saved_game.bytes[0x29DB];
    active_slot = 3;
    matching_index = 3;
    order[0] = packed_order & 3;
    order[1] = (packed_order >> 2) & 3;
    order[2] = (packed_order >> 4) & 3;
    selected_index = (s8)g_saved_game.bytes[0x29D7];
    active_index = D_80122C00;
    if (g_saved_game.bytes[active_index + 0x29D8] != selected_index)
    {
        s32 i;
        s32 matching_slot;

        i = 0;
        do
        {
            if (selected_index == g_saved_game.bytes[i + 0x29D8])
            {
                matching_index = i;
            }
            i += 1;
        } while (i < 3);
        i = 0;
        do
        {
            if (order[i] == matching_index)
            {
                matching_slot = i;
            }
            i += 1;
        } while (i < 3);
        i = 0;
        do
        {
            if (order[i] == active_index)
            {
                active_slot = i;
            }
            i += 1;
        } while (i < 3);
        order[matching_slot] = active_index;
        order[active_slot] = matching_index;
        packed_order = order[0] + (order[1] * 4) + (order[2] * 0x10);
        D_800459B3 = packed_order;
    }
    (*(StructD80122C0A *)&D_80122C0A).unk0 = active_slot;
    (*(StructD80122C0A *)&D_80122C0A).unk12 = (s16)matching_index;
}



/** @brief Dispatch the selected shared equipment record. */
void func_800C69F4(void)
{
    func_800B2844(0, D_80043CB8 + (D_80122C10 << 6), 0xFF);
}




/**
 * @brief Counts active menu-layout slots whose packed field matches D_80122C10.
 *
 * Scans all 100 slots of @c g_saved_game.bytes (stride 0x40). A slot counts
 * when its active byte at +0xCE0 is nonzero and bits 8-9 of the packed word at
 * +0xCF4 equal the value in @c D_80122C10 (read once up front). The total is
 * written back to @c D_80122C10.
 */
void func_800C6A30(void)
{
    s32 i;
    s32 count;
    u8 *p;
    u32 field;
    s32 target_val;

    target_val = D_80122C10;
    count = 0;
    for (i = 0; i < 0x64; i++)
    {
        p = &g_saved_game.bytes[i * 0x40];
        if (p[0xCE0] != 0)
        {
            field = *(u32 *) &p[0xCF4];
            field = (field >> 8) & 3;
            if (field == target_val)
            {
                count++;
            }
        }
    }
    D_80122C10 = count;
}





/** @brief Fill available record destinations and advance the layout counter. */
void func_800C6A90(void)
{
    while (field_find_free_inventory_record() != 0)
    {
        field_copy_inventory_record(field_find_free_inventory_record(), D_80043CB8);
    }

    g_saved_game.bytes[0x29D5] += 9;
}


typedef struct
{
    s32 words[0xE8 / 4];
} Struct0xE8;

extern Struct0xE8 D_80051CE4;
extern Struct0xE8 D_80051DCC;


void func_800C3BB0(void);

/**
 * @brief Reconcile active logic blocks with the selected layout class.
 */
void func_800C6AF0(void)
{
    Struct0xE8 primary_table;
    Struct0xE8 secondary_table;
    u8 *layout_buffer;
    s32 index;
    s8 layout_class;
    s32 selected_value;
    u32 packed_state;
    s32 table_value;
    u8 *layout_record;
    s32 active_flag;
    u8 *primary_table_bytes;
    u32 clear_active_mask;

    primary_table = D_80051CE4;
    secondary_table = D_80051DCC;

    layout_buffer = g_saved_game.bytes;
    layout_class = ((s8 *)layout_buffer)[0x29D7];
    selected_value = D_80122C1C;
    if (layout_class < 3)
    {
        layout_record = layout_buffer + layout_class * 0x14C;
        g_saved_game.bytes[0xAA9] = (u8)D_80122C1C;
        *(s32 *)(layout_record + 0x2B50) = (*(s32 *)(layout_record + 0x2B50) & ~0xF) | ((u8)D_80122C1C & 0xF);
        index = 0;
        if (layout_buffer[0x29D6] != 0)
        {
            active_flag = 1;
            primary_table_bytes = (u8 *)&primary_table;
            clear_active_mask = 0xFFFEFFFF;
            do
            {
                packed_state = *(u32 *)(layout_buffer + index * 4 + 0x29DC);
                if (((packed_state >> 0x10) & 1) == active_flag && (packed_state & 3) == layout_class)
                {
                    table_value = *(s32 *)(primary_table_bytes + (packed_state & 0xFC));
                    if (table_value != 0 && table_value != selected_value)
                    {
                        *(u32 *)(layout_buffer + index * 4 + 0x29DC) = (packed_state & clear_active_mask) | 3;
                    }
                }
                index = index + 1;
            } while (index < layout_buffer[0x29D6]);
        }
        func_800C3BB0();
    }
}


/**
 * @brief View of g_saved_game.bytes used by func_800C6C80: a packed-word
 *        cursor at 0x29D6 followed by the word table at 0x29DC.
 */
typedef struct
{
    u8 pad0[0x29D6];
    u8 index;
    u8 pad29D7[5];
    u32 words[1];
} SavedGameLayoutBuffer;

/**
 * @brief Three-word parameter block forwarded to field_open_gosub_screen_sequence.
 */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
} UnkStruct80051EB4;

typedef struct
{
    u8 pad[0xCF4];
    s32 unkCF4;
} MenuRec;

typedef struct
{
    s16 mystic_card_ids[3];
} FieldMysticCardSlots;

typedef struct
{
    s16 unk0;
    s16 unk2;
} FieldObjState;

/**
 * @brief Placeholder layout for the record returned by func_800C1E40; only the
 *        byte at offset 4 is read here.
 */
typedef struct
{
    u8 unk0;
    u8 unk1;
    u8 unk2;
    u8 unk3;
    u8 unk4;
} Rec;

#define MENU_LAYOUT ((SavedGameLayoutBuffer *)g_saved_game.bytes)

void func_800C9ED4();
void func_800AD030(s32 arg0);

u8 *func_800C1E40(s32 arg0);


extern u8 D_800459AE;


/**
 * @brief Append a packed word built from D_80122C18..D_80122C1C to the menu word table.
 */
void func_800C6C80(void)
{
    s16 *p = &D_80122C1C;
    s32 arg0 = p[0];
    s32 arg1 = p[-1];
    s32 arg2 = p[-2];

    if (arg0 == 0xFF)
    {
        D_800459AE = 0;
        return;
    }

    if (MENU_LAYOUT->index < 0x28)
    {
        MENU_LAYOUT->words[MENU_LAYOUT->index] =
            (MENU_LAYOUT->words[MENU_LAYOUT->index] & ~0xFC) | ((arg0 & 0x3F) << 2);
        MENU_LAYOUT->words[MENU_LAYOUT->index] =
            (MENU_LAYOUT->words[MENU_LAYOUT->index] & ~0xF00) | ((arg1 & 0xF) << 8);
        MENU_LAYOUT->words[MENU_LAYOUT->index] =
            (MENU_LAYOUT->words[MENU_LAYOUT->index] & ~0xF000) | ((arg2 & 0xF) << 12);
        MENU_LAYOUT->words[MENU_LAYOUT->index] |= 3;
        MENU_LAYOUT->words[MENU_LAYOUT->index] &= ~0x10000;
        MENU_LAYOUT->index++;
    }
}

/**
 * @brief Forward D_80122C1C to func_800C9ED4.
 */
void func_800C6DA0(void)
{
    func_800C9ED4(D_80122C1C);
}

/**
 * @brief Copy the D_80051EB4 constant onto the stack and forward it to field_open_gosub_screen_sequence.
 */
void func_800C6DC8(void)
{
    UnkStruct80051EB4 local;

    local = (*(UnkStruct80051EB4 *)&D_80051EB4);
    field_open_gosub_screen_sequence(&local);
}

/**
 * @brief Call func_800AD030 with argument 0.
 */
void func_800C6E08(void)
{
    func_800AD030(0);
}

/**
 * @brief Resolve the current menu record's slot and latch it.
 *
 * Reads the packed field @c unkCF4 of the record selected by @c D_80122C10,
 * decoding a base index (@c bits 10-15) offset by mode (@c bits 8-9): +0 for
 * mode 0, +0xB for mode 1, +0x17 otherwise. When the result is the 0xFF
 * sentinel it triggers record_game_diagnostic and stores 0; otherwise the resolved
 * slot is written back to @c D_80122C10.
 *
 * @see decomp.me (100%) TODO
 */
void func_800C6E28(void)
{
    u32 rec;
    s32 mode;
    s32 v;

    rec = ((MenuRec *)(g_saved_game.bytes + D_80122C10 * 0x40))->unkCF4;
    mode = (rec >> 8) & 3;
    if (mode == 0)
    {
        v = (rec >> 10) & 0x3F;
    }
    else if (mode == 1)
    {
        v = ((rec >> 10) & 0x3F) + 0xB;
    }
    else
    {
        v = ((rec >> 10) & 0x3F) + 0x17;
    }
    if (v == 0xFF)
    {
        record_game_diagnostic(0x8002, 0x22, 0, 0);
        v = 0;
    }
    D_80122C10 = v;
}

/**
 * @brief Invalidates matching menu slots and preserves the remaining IDs.
 *
 * Loads three menu IDs from the active 0x40-byte layout record, then scans
 * the three halfword slots at D_80122C00. Matching slots are replaced with
 * 0xFF and the corresponding menu ID is invalidated before the final IDs are
 * written to D_80122C06.
 */
void func_800C6EBC(void)
{
    s32 i;
    s16 *slot;
    u8 a, b, c;
    u8 *p;
    s16 value;
    s32 invalid;

    i = 0;
    invalid = 0xFF;
    slot = ((FieldMysticCardSlots *)&D_80122C00)->mystic_card_ids;
    {
        s16 *idxp;
        u8 *base;

        idxp = &D_80122C10;
        base = g_saved_game.bytes;
        p = base + (*idxp << 6);
    }
    a = p[0xD00];
    b = p[0xD01];
    c = p[0xD02];
    do
    {
        value = *slot;
        if (value != invalid)
        {
            if (value == a)
            {
                *slot = invalid;
                a = 0xFF;
            }
            else if (value == b)
            {
                *slot = invalid;
                b = 0xFF;
            }
            else if (value == c)
            {
                *slot = invalid;
                c = 0xFF;
            }
        }
        i++;
        slot++;
    } while (i < 3);
    ((s16 *)&D_80122C06)[0] = a;
    ((s16 *)&D_80122C06)[1] = b;
    ((s16 *)&D_80122C06)[2] = c;
}

/**
 * @brief Load the gosub-selected equipment's visible Mystic Card IDs.
 */
void func_800C6F60(void)
{
    s32 *selection_results;
    u8 *layout_buffer;
    u8 *equipment_record;

    selection_results = g_gosub_result_values;
    layout_buffer = g_saved_game.bytes;
    equipment_record = (u8 *)((selection_results[0] * 64) + (s32)layout_buffer);
    ((FieldMysticCardSlots *)&D_80122C00)->mystic_card_ids[0] = equipment_record[0xD00];
    ((FieldMysticCardSlots *)&D_80122C00)->mystic_card_ids[1] = equipment_record[0xD01];
    ((FieldMysticCardSlots *)&D_80122C00)->mystic_card_ids[2] = equipment_record[0xD02];
}

/**
 * @brief Look up the current object's halfword in resource 0x102 and store it in D_80122C0E.
 * @see decomp.me (100%)
 */
void func_800C6F9C(void)
{
    s16 temp_s0;
    u8 *p;
    s32 offset;
    u16 result;

    temp_s0 = (*(FieldObjState *)&D_80122C0C).unk0;
    if (temp_s0 == 0xFF)
    {
        result = 0xFFFF;
    }
    else
    {
        if ((*(FieldObjState *)&D_80122C0C).unk2 == 0)
        {
            p = func_800C1E40(0x102);
            offset = temp_s0 * 4;
        }
        else
        {
            p = func_800C1E40(0x102);
            offset = temp_s0 * 4;
            offset = offset | 2;
        }
        result = *(u16 *)(p + offset + 4);
    }
    D_80122C0E = result;
}

/**
 * @brief Decode a little-endian offset from resource 0x101 and dispatch the referenced entry.
 * @see decomp.me (100%) TODO
 */
void func_800C7014(void)
{
    s32 idx;
    s32 k;
    u8 *p1;
    s32 value;

    idx = (*(FieldObjState *)&D_80122C0C).unk0;
    p1 = func_800C1E40(0x101);
    k = idx * 2;
    value = ((Rec *)(p1 + k))->unk4 +
            (((Rec *)(func_800C1E40(0x101) + (k += 1)))->unk4 << 8);
    func_800B2844(0, func_800C1E40(0x101) + (value + 4), 0xFF);
}

#include "game_audio.h"
#include "saved_game.h"
#include "main.h"

typedef struct
{
    s16 unk0;
    s16 unk2;
} FieldC7090State;


extern s32 D_80045EC8;


/**
 * @brief Decode the gosub-selected record flags into its display id and mode.
 * @see decomp.me (100%)
 */
void func_800C7090(void)
{
    s32 idx;
    u8 *base;
    u8 *rec;
    s32 flags;

    if (*(s32 *)&(*(u16 *)&g_gosub_result_count) != 0)
    {
        idx = g_gosub_result_values[0];
        if (idx < 5)
        {
            base = g_saved_game.bytes;
            rec = base + idx * 0x60;
            flags = *(s32 *)(rec + 0x2F38);
            if (flags < 0)
            {
                ((FieldC7090State *)&D_80122C10)->unk0 = rec[0x2F0A] + 0x53;
                ((FieldC7090State *)&D_80122C10)->unk2 = 0;
            }
            else
            {
                s16 masked = (s16)(((u32) flags >> 30) & 1);
                ((FieldC7090State *)&D_80122C10)->unk0 = rec[0x2F09] + 0x12;
                ((FieldC7090State *)&D_80122C10)->unk2 = masked;
            }
            if (idx == D_80045EC8)
            {
                ((FieldC7090State *)&D_80122C10)->unk0 = 0xFE;
            }
        }
        else
        {
            record_game_diagnostic(0x8002, 0x27, idx, 0);
        }
    }
    D_80122C16 = (u16) *(s32 *)&(*(u16 *)&g_gosub_result_count);
}

/**
 * @brief Three-word parameter block forwarded to field_open_gosub_screen_sequence.
 */
typedef struct
{
    s32 unk0; /* 0x00 */
    s32 unk4; /* 0x04 */
    s32 unk8; /* 0x08 */
} UnkStruct80051EC0;

typedef struct
{
    u8 unk0;
    u8 pad1[0xD];
    s16 unkE;
    u8 pad10[4];
    u16 unk14;
} UnkStruct80122C02;

typedef struct
{
    s16 unk0;
    s16 unk2;
    u16 unk4;
} UnkStruct80122C12;


extern UnkStruct80051EC0 D_80051EC0;
extern UnkStruct80051EC0 D_80051ECC;
extern u8 D_80045ECC[];
extern s32 D_801227F0;


/**
 * @brief Counts active gosub-result entries whose bit 30 flag is set.
 *
 * When there are gosub results, walks the five 0x60-byte entries starting at
 * g_saved_game.bytes[0x2EF4]; for each entry whose leading byte is nonzero and
 * whose word at +0x44 has bit 30 set, increments the tally stored to
 * D_80122C16.
 *
 * @note Reads g_gosub_result_count as a full word here, while the other
 *       functions in this file read it as a halfword.
 */
void func_800C7168(void)
{
    s32 count;
    s32 i;
    s32 one = 1;
    u8 *p;

    if (*(s32 *)&(*(u16 *)&g_gosub_result_count) != 0)
    {
        count = 0;
        for (i = 0; i < 5; i++)
        {
            p = &g_saved_game.bytes[i * 0x60];
            if (p[0x2EF4] != 0 &&
                (((*(u32 *)(p + 0x2F38) >> 30) & 1) == one))
            {
                count++;
            }
        }
    }
    D_80122C16 = count;
}

/**
 * @brief Flag the current gosub result's menu layout entry, or trigger a diagnostic for an invalid index.
 */
void func_800C71D4(void)
{
    s32 idx;
    u8 *base;
    u8 *rec;

    idx = g_gosub_result_values[0];
    if (idx < 5)
    {
        base = g_saved_game.bytes;
        rec = &base[idx * 0x60];
        *(u32 *)(rec + 0x2F38) |= 0x40000000;
    }
    else
    {
        record_game_diagnostic(0x8002, 0x29, idx, 0);
    }
}

/**
 * @brief Copy the D_80051EC0 constant onto the stack and forward it to field_open_gosub_screen_sequence.
 */
void func_800C7238(void)
{
    UnkStruct80051EC0 local;

    local = D_80051EC0;
    field_open_gosub_screen_sequence(&local);
}

/**
 * @brief Reset D_801227F0 and latch the current gosub result index and count.
 */
void func_800C7278(void)
{
    s32 temp;

    D_801227F0 = 0;
    temp = g_gosub_result_values[0];
    (*(UnkStruct80122C12 *)&D_80122C12).unk0 = temp;
    (*(UnkStruct80122C12 *)&D_80122C12).unk4 = (*(u16 *)&g_gosub_result_count);
}

/**
 * @brief Copy the D_80051ECC constant onto the stack and forward it to field_open_gosub_screen_sequence.
 */
void func_800C72A4(void)
{
    UnkStruct80051EC0 sp10;

    sp10 = D_80051ECC;
    field_open_gosub_screen_sequence(&sp10);
}

/**
 * @brief Record the current gosub result index and count, then emit its portrait icon.
 */
void func_800C72E4(void)
{
    s32 temp;

    (*(UnkStruct80122C02 *)&D_80122C02).unkE = temp = g_gosub_result_values[0];
    (*(UnkStruct80122C02 *)&D_80122C02).unk14 = (*(u16 *)&g_gosub_result_count);
    func_800B2844((*(UnkStruct80122C02 *)&D_80122C02).unk0, (temp * 0x60) + D_80045ECC, 0xFF);
}



extern u8 D_800F0E98[];
extern u8 D_80045ECC[];


/**
 * @brief Dispatch the selected menu record's extra slots and fixed trailing slot.
 * @note Skip 0xFE/0xFF entries and record the count of dispatched extra slots.
 * @note Selections of five or greater record a diagnostic instead.
 * @note WIP: instruction ordering and temporary-register differences remain.
 */
void func_800C7340(void)
{
    s32 selection;
    s32 slot_index;
    s32 dispatch_count;
    s32 record_offset;
    u8 *menu_base;
    u8 slot_flag;
    s32 table_offset;
    u8 *table_base;

    selection = D_80122C10;
    if (selection < 5)
    {
        dispatch_count = 0;
        slot_index = 0;
        menu_base = g_saved_game.bytes;
        record_offset = selection * 0x60;
        do
        {
            table_base = D_800F0E98;
            slot_flag = menu_base[slot_index + record_offset + 0x2F38];
            if (slot_flag != 0xFF)
            {
                if (slot_flag != 0xFE)
                {
                    table_offset = slot_flag * 2;
                    func_800B2844(dispatch_count, D_800F0E98[table_offset] + (D_800F0E98[table_offset + 1] << 8) + table_base, 0xFF);
                    dispatch_count++;
                }
            }
            slot_index++;
        } while (slot_index < 3);
        func_800B2844(3, (selection * 0x60) + D_80045ECC, 0xFF);
        D_80122C16 = (u16)dispatch_count;
        return;
    }
    record_game_diagnostic(0x8002, 0x2E, selection, 0);
}

/** @brief Clear the selected large-history record status. */
void func_800C745C(void)
{
    PadContext* ctx = (PadContext*)g_saved_game.bytes;
    s32 idx = ctx->large_history_index;
    ((u8*)ctx)[0xC06] = 0;
    ctx->large_history_records[idx].unknown_0x46 = 0;
}

typedef struct
{
    s16 unk0;
    s16 unk2;
} FieldC7494State;



/** @brief Write an available extra history slot or report an invalid history index. */
void func_800C7494(void)
{
    s32 idx;
    s32 val;
    s32 count;
    s32 row;
    u8 *base;
    u8 *slot;

    idx = ((FieldC7494State *)&D_80122C10)->unk0;
    val = ((FieldC7494State *)&D_80122C10)->unk2;
    if (idx < 5)
    {
        count = 0;
        base = g_saved_game.bytes;
        row = idx * 0x60;
        do
        {
            slot = (u8 *)((count + row) + (s32)base);
            count++;
            if ((u32)((slot[0x2F38] + 2) & 0xFF) < 2)
            {
                slot[0x2F38] = val;
                return;
            }
        } while (count < 3);
        return;
    }
    record_game_diagnostic(0x8002, 0x30, idx, 0);
}


/** @brief Replace the selected history index with its entry id. */
void func_800C752C(void)
{
    D_80122C10 = g_saved_game.bytes[(D_80122C10 * 0x60) + 0x2F09];
}



/**
 * @brief Clears the active flag for the selected small history slot.
 *
 * Uses the small-history index at offset 0x2EF0 to select a 0x60-byte record
 * and clears bit 30 of its word at offset 0x2F38. Out-of-range indices record a diagnostic.
 *
 */
void func_800C7558(void)
{
    s32 index;
    u8 *base;
    u8 *record;

    base = g_saved_game.bytes;
    index = *(s32 *)(base + 0x2EF0);
    if (index < 5)
    {
        record = base + index * 0x60;
        *(u32 *)(record + 0x2F38) &= 0xBFFFFFFF;
    }
    else
    {
        record_game_diagnostic(0x8002, 0x32, index, 0);
    }
}

/**
 * @brief Clears the active flag for the gosub-selected small history slot.
 *
 * Uses the first gosub result to select a 0x60-byte record and clears bit 30
 * of its word at offset 0x2F38. Out-of-range indices record a diagnostic.
 *
 */
void func_800C75C0(void)
{
    s32 index;
    u8 *base;
    u8 *record;

    index = g_gosub_result_values[0];
    if (index < 5)
    {
        base = g_saved_game.bytes;
        record = base + index * 0x60;
        *(u32 *)(record + 0x2F38) &= 0xBFFFFFFF;
    }
    else
    {
        record_game_diagnostic(0x8002, 0x32, index, 0);
    }
}

typedef struct
{
    u8 _pad00[0x5A];
    u16 unk5A;
    u8 _pad5C[4];
} FieldSmallHistoryRecord;

typedef struct
{
    u8 _pad0000[0x2EF4];
    FieldSmallHistoryRecord small_history_records[5];
} FieldMenuHistoryData;



/**
 * @brief Load a value from the gosub-selected compact history record.
 *
 * If the gosub returned a selection, copies the selected record's unk5A
 * halfword into D_80122C00.
 */
void func_800C7628(void)
{
    FieldMenuHistoryData* history_data;
    s32 history_index;

    if (*(s32 *)&(*(u16 *)&g_gosub_result_count) != 0)
    {
        history_index = g_gosub_result_values[0];
        history_data = (FieldMenuHistoryData*)g_saved_game.bytes;
        (*(u32 *)&D_80122C00) = history_data->small_history_records[history_index].unk5A;
    }
}

#include "saved_game.h"
#include "common.h"



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
extern u8 D_800F0E98[];
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
    layout = g_saved_game.bytes;
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
    output = ((u8 *)&D_80122C00);
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
    ((Selection *)((u8 *)&D_80122C08))->mask = mask;
    ((Selection *)((u8 *)&D_80122C08))->count = limit;
    record_offset = record * 0x8C;
    record_base = g_saved_game.bytes;
    flags = ((FieldActionGroupView *)(record_base + record_offset))->unk26E4;
    kind = flags >> 8;
    kind &= 0xF;
    limit = flags & 0xF;
    do
    {
        if ((u32)((FieldActionGroupView *)(g_saved_game.bytes + record_offset + i * 0x10))->unk26F4 < 0xFF)
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
    scan_base = ((u8 *)&D_80122C00);
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
extern u8 D_800F0E98[];
/**
 * @brief Add the selected menu item to its group and refresh availability.
 */
void func_800C7840(void)
{
    s32 available_mask;
    s32 slot_limit;
    s32 group;
    s32 capacity;
    s32 layout_base;
    s32 post_layout_base;
    s32 second_base;
    s32 second_offset;
    u8 *text_base;
    s32 flag_mask;
    s32 initial_offset;
    s32 text_offset;
    s32 group_offset;
    s32 occupied_count;
    s32 slot_index;
    s32 empty_slot;
    s32 new_count;
    s32 item_id;
    s32 available_count;
    u32 group_flags;
    u8 *selected_count;
    u8 *text_low;
    u8 *text_high;
    s32 item_counts_base;
    FieldActionGroupView *group_view;

    occupied_count = 0;
    slot_index = occupied_count;
    group = D_80122C1F;
    initial_offset = group * 0x8C;
    group_flags = ((FieldActionGroupView *)(g_saved_game.bytes + initial_offset))->unk26E4;
    layout_base = (s32)g_saved_game.bytes;
    group_offset = initial_offset;
    slot_limit = group_flags >> 8;
    slot_limit &= 0xF;
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
        second_base = (s32)g_saved_game.bytes;
        second_offset = group * 0x8C;
        empty_slot = 0xFF;
loop_6:
        if (((FieldActionGroupView *)(slot_index + second_offset + second_base))->unk26EC != empty_slot)
        {
            slot_index += 1;
            if (slot_index < slot_limit)
            {
                goto loop_6;
            }
        }
    }
    flag_mask = 0xFFFF0FFF;
    new_count = slot_index + 1;
    post_layout_base = (s32)g_saved_game.bytes;
    group_offset = group * 0x8C;
    group_view = (FieldActionGroupView *)(post_layout_base + group_offset);
    D_80122C0B[0] = new_count;
    group_view->unk26E4 = (s32)((group_view->unk26E4 & flag_mask) | ((new_count & 0xF) << 0xC));
    if (capacity < (slot_index + 3))
    {
        D_80122C0B[1] = 1;
    }
    item_counts_base = (s32)(D_80122C0B - 0xB);
    text_base = D_800F0E98;
    selected_count = (u8 *)((*(s16 *)(D_80122C0B + 9)) + item_counts_base);
    item_id = (*(s16 *)(D_80122C0B + 9)) + 0x58;
    *selected_count -= 1;
    ((FieldActionGroupView *)(slot_index + group_offset + post_layout_base))->unk26EC = item_id;
    text_offset = item_id * 2;
    text_low = text_offset + text_base;
    {
        s32 text_offset2 = text_offset + 1;
        text_high = text_offset2 + text_base;
    }
    func_800B2844(0, *text_low + (*text_high << 8) + text_base, 0xFF, slot_index);
    available_mask = 0x1FFF;
    available_count = 0;
    slot_index = available_count;

    do
    {
        if (*(u8 *)(slot_index + item_counts_base) != 0) {
            available_mask &= ~(1 << slot_index);
            available_count += 1;
        }
        slot_index += 1;
    } while (slot_index < 8);
    ((Availability *)((u8 *)&D_80122C08))->mask = available_mask;
    ((Availability *)((u8 *)&D_80122C08))->count = available_count;
}

extern void func_800C0260(s32, s32);
extern void func_800C7C88(void);
extern s32 rand(void);
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
    temp_v1 = ((Layout *)((u8 *)g_saved_game.bytes + temp_v0))->packed;
    var_a1 = temp_v1 & 0xF;
    var_a0 = var_s1 * 16 + temp_v0;
    temp_s7 = temp_v1 >> 8;
    temp_s7 &= 0xF;
    temp_s4 = temp_v1 >> 12;
    temp_s4 &= 0xF;
    temp_s4 += 2;
    do
   
   {
        if (((Layout *)((u8 *)g_saved_game.bytes + (var_a0)))->entry != 0xFF)
       
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
                if (((Layout *)((u8 *)g_saved_game.bytes + var_a1 * 16 + temp_s2 * 0x8C))->entry ==
                    0xFF)
               
               {
                    break;
                }
            }
            if (((Layout *)((u8 *)g_saved_game.bytes + var_a1 * 16 + temp_s2 * 0x8C))->entry ==
                0xFF)
           
           {
                func_800C0260(temp_s2, var_a1);
            }
            else
           
           {
                for (var_s0 = 0; var_s0 < 8; var_s0++)
               
               {
                    if (((Layout *)((u8 *)g_saved_game.bytes + var_s0 * 16 + temp_s2 * 0x8C))
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
        temp_v1_4 = (u8 *)g_saved_game.bytes;
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


/**
 * @brief Clears the active menu entry's high flag nibble and status bytes.
 *
 * For the entry selected by D_80122C1F (stride 0x8C in g_saved_game.bytes),
 * masks off bits 12-15 of its 0x26E4 word and writes 0xFF to the four status
 * bytes at 0x26EC.
 */
void func_800C7C88(void)
{
    u8 *base = g_saved_game.bytes;
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
 * Reads the flags word at @c g_saved_game.bytes + 0x26E4, zeroes the eight
 * slots at stride 0x10 from +0x26F0 (each slot's word set to 0 and its status
 * byte at +4 set to 0xFF), then clears bits 12-15 of the flags word.
 */
void func_800C7CF8(void)
{
    u8 *base = g_saved_game.bytes;
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


s32 func_8008B288(s32 arg0);

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
        u8 *buf = g_saved_game.bytes;
        u8 *base = buf + (va * 0x10 + vb * 0x8C);
        idx = base[0x26F4];
    }
    value = g_saved_game.bytes[idx + 0x25E0];
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
        u8 *buf = g_saved_game.bytes;
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
    menu = g_saved_game.bytes;
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
    initial_base = g_saved_game.bytes;
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
    scan_base = g_saved_game.bytes;
    selected_record = scan_base + selected_index * 0x40;
    while (record_index < 4)
    {
        u8 *record = (u8 *)((s32)g_saved_game.bytes + record_index * 0x40);
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
        u8 *record = (u8 *)((s32)g_saved_game.bytes + record_index * 0x40);
        if (record[0x3160] == 0)
        {
            field_copy_inventory_record(g_saved_game.bytes + record_index * 0x40 + 0x3160, g_saved_game.bytes + 0xCE0 + selected_index * 0x40);
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
} RuntimeUnkStruct80051EB4;


/** @brief Open the pending-record selection screen sequence. */
void func_800C8220(void)
{
    RuntimeUnkStruct80051EB4 local;

    local = (*(RuntimeUnkStruct80051EB4 *)&D_80051EB4);
    field_open_gosub_screen_sequence(&local);
}

typedef struct
{
    s32 x;
    s32 y;
    s32 z;
} FieldPosition;


s32 func_8008B288(s32 arg0);

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
 */
void func_800C83DC(void)
{
    s32 slot_index;
    s32 record_offset;
    s32 record_index;
    s32 slot_offset;
    u32 counter_value;
    u8 first_counter;
    u8 *slot;
    u8 *menu_base;

    record_index = 0;
    menu_base = g_saved_game.bytes;
    record_offset = record_index;
    do
    {
        slot_index = 0;
        do
        {
            slot_offset = slot_index * 0x10;
            slot = (u8 *)(slot_offset + record_offset + (s32)menu_base);
            if (slot[0x26F4] != 0xFF)
            {
                first_counter = slot[0x26F8];
                counter_value = first_counter & 0xFF;
                if (counter_value == 0)
                {
                    slot[0x26F8] = (u8) (first_counter + 1);
                } else if (counter_value < 0xF0U)
                {
                    slot[0x26F8] = (u8) (first_counter + 1);
                    slot[0x26F9] = (u8) (slot[0x26F9] + 1);
                    slot[0x26FA] = (u8) (slot[0x26FA] + 1);
                    slot[0x26FB] = (u8) (slot[0x26FB] + 1);
                    slot[0x26FC] = (u8) (slot[0x26FC] + 1);
                    slot[0x26FD] = (u8) (slot[0x26FD] + 1);
                    slot[0x26FE] = (u8) (slot[0x26FE] + 1);
                    slot[0x26FF] = (u8) (slot[0x26FF] + 1);
                    slot[0x26F8] = (u8) (slot[0x26F8] + 1);
                    slot[0x26F9] = (u8) (slot[0x26F9] + 1);
                    slot[0x26FA] = (u8) (slot[0x26FA] + 1);
                    slot[0x26FB] = (u8) (slot[0x26FB] + 1);
                    slot[0x26FC] = (u8) (slot[0x26FC] + 1);
                    slot[0x26FD] = (u8) (slot[0x26FD] + 1);
                    slot[0x26FE] = (u8) (slot[0x26FE] + 1);
                    slot[0x26FF] = (u8) (slot[0x26FF] + 1);
                    slot[0x26F8] = (u8) (slot[0x26F8] + 1);
                    slot[0x26F9] = (u8) (slot[0x26F9] + 1);
                    slot[0x26FA] = (u8) (slot[0x26FA] + 1);
                    slot[0x26FB] = (u8) (slot[0x26FB] + 1);
                    slot[0x26FC] = (u8) (slot[0x26FC] + 1);
                    slot[0x26FD] = (u8) (slot[0x26FD] + 1);
                    slot[0x26FE] = (u8) (slot[0x26FE] + 1);
                    slot[0x26FF] = (u8) (slot[0x26FF] + 1);
                    slot[0x26F8] = (u8) (slot[0x26F8] + 1);
                    slot[0x26F9] = (u8) (slot[0x26F9] + 1);
                    slot[0x26FA] = (u8) (slot[0x26FA] + 1);
                    slot[0x26FB] = (u8) (slot[0x26FB] + 1);
                    slot[0x26FC] = (u8) (slot[0x26FC] + 1);
                    slot[0x26FD] = (u8) (slot[0x26FD] + 1);
                    slot[0x26FE] = (u8) (slot[0x26FE] + 1);
                    slot[0x26FF] = (u8) (slot[0x26FF] + 1);
                    slot[0x26F8] = (u8) (slot[0x26F8] + 1);
                    slot[0x26F9] = (u8) (slot[0x26F9] + 1);
                    slot[0x26FA] = (u8) (slot[0x26FA] + 1);
                    slot[0x26FB] = (u8) (slot[0x26FB] + 1);
                    slot[0x26FC] = (u8) (slot[0x26FC] + 1);
                    slot[0x26FD] = (u8) (slot[0x26FD] + 1);
                    slot[0x26FE] = (u8) (slot[0x26FE] + 1);
                    slot[0x26FF] = (u8) (slot[0x26FF] + 1);
                    slot[0x26F8] = (u8) (slot[0x26F8] + 1);
                    slot[0x26F9] = (u8) (slot[0x26F9] + 1);
                    slot[0x26FA] = (u8) (slot[0x26FA] + 1);
                    slot[0x26FB] = (u8) (slot[0x26FB] + 1);
                    slot[0x26FC] = (u8) (slot[0x26FC] + 1);
                    slot[0x26FD] = (u8) (slot[0x26FD] + 1);
                    slot[0x26FE] = (u8) (slot[0x26FE] + 1);
                    slot[0x26FF] = (u8) (slot[0x26FF] + 1);
                    slot[0x26F8] = (u8) (slot[0x26F8] + 1);
                    slot[0x26F9] = (u8) (slot[0x26F9] + 1);
                    slot[0x26FA] = (u8) (slot[0x26FA] + 1);
                    slot[0x26FB] = (u8) (slot[0x26FB] + 1);
                    slot[0x26FC] = (u8) (slot[0x26FC] + 1);
                    slot[0x26FD] = (u8) (slot[0x26FD] + 1);
                    slot[0x26FE] = (u8) (slot[0x26FE] + 1);
                    slot[0x26FF] = (u8) (slot[0x26FF] + 1);
                    slot[0x26F8] = (u8) (slot[0x26F8] + 1);
                    slot[0x26F9] = (u8) (slot[0x26F9] + 1);
                    slot[0x26FA] = (u8) (slot[0x26FA] + 1);
                    slot[0x26FB] = (u8) (slot[0x26FB] + 1);
                    slot[0x26FC] = (u8) (slot[0x26FC] + 1);
                    slot[0x26FD] = (u8) (slot[0x26FD] + 1);
                    slot[0x26FE] = (u8) (slot[0x26FE] + 1);
                    slot[0x26FF] = (u8) (slot[0x26FF] + 1);
                    slot[0x26F8] = (u8) (slot[0x26F8] + 1);
                    slot[0x26F9] = (u8) (slot[0x26F9] + 1);
                    slot[0x26FA] = (u8) (slot[0x26FA] + 1);
                    slot[0x26FB] = (u8) (slot[0x26FB] + 1);
                    slot[0x26FC] = (u8) (slot[0x26FC] + 1);
                    slot[0x26FD] = (u8) (slot[0x26FD] + 1);
                    slot[0x26FE] = (u8) (slot[0x26FE] + 1);
                    slot[0x26FF] = (u8) (slot[0x26FF] + 1);
                }
            }
            slot_index += 1;
        } while (slot_index < 8);
        record_index += 1;
        record_offset += 0x8C;
    } while (record_index < 4);
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
    menu = g_saved_game.bytes;
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

#include "saved_game.h"
#include "common.h"

void func_800CA1E0(void);
void func_800CA1A0(s32 arg0);

/** @brief Reset layout slots and activate the default set in order. */
void func_800C8830(void)
{
    func_800CA1E0();
    func_800CA1A0(0);
    func_800CA1A0(1);
    func_800CA1A0(2);
    func_800CA1A0(3);
    func_800CA1A0(4);
    func_800CA1A0(5);
    func_800CA1A0(7);
    func_800CA1A0(8);
    func_800CA1A0(9);
    func_800CA1A0(0xA);
    func_800CA1A0(0xB);
    func_800CA1A0(0xC);
    func_800CA1A0(0xD);
    func_800CA1A0(0xF);
    func_800CA1A0(0x10);
    func_800CA1A0(0x11);
    func_800CA1A0(0x12);
    func_800CA1A0(0x13);
    func_800CA1A0(0x15);
    func_800CA1A0(0x16);
    func_800CA1A0(0x18);
    func_800CA1A0(0x19);
    func_800CA1A0(0x1A);
    func_800CA1A0(0x1B);
    func_800CA1A0(0x1E);
    func_800CA1A0(0x1F);
    func_800CA1A0(0x20);
    func_800CA1A0(0x20);
    func_800CA1A0(0x17);
}

extern s32 D_8011F428;

/** @brief Restore the saved field mode through the mode dispatcher. */
void func_800C8938(void)
{
    D_8011F428 = (s32) D_80122C1E;
    func_800AD120(D_80122C1E);
}



/**
 * @brief Scan the five menu layout records and build a compacted index of the
 *        active entries whose flag bit 30 is set.
 * @note For each active record (offset 0x2EF4 non-zero) with bit 30 of the word
 *       at 0x2F38 set, appends its 0x2F09 id and slot index to the D_80122C00
 *       tables and clears the record's 0x2EF4 marker via func_800B2844.
 * @see decomp.me (100.00%)
 */
void func_800C8964(void)
{
    s32 count;
    s32 i;
    u8 *ids;
    u8 *slots;
    u8 *record;
    u8 *arg;
    u32 bit;

    count = 0;
    i = count;
    ids = ((u8 *)&D_80122C00);
    slots = ids + 0x1D;
    record = g_saved_game.bytes;
    arg = record + 0x2EF4;
    do
    {
        record = &g_saved_game.bytes[i * 0x60];
        if (record[0x2EF4] != 0)
        {
            bit = (*(u32 *)(record + 0x2F38) >> 30) & 1;
            if (bit == 1)
            {
                *(u8 *)((u32)count + (u32)ids) = record[0x2F09];
                *(u8 *)((u32)count + (u32)slots) = i;
                func_800B2844(count, arg, 0xFF);
                count++;
            }
        }
        arg += 0x60;
        i++;
    } while (i < 5);
}

#include "saved_game.h"
#include "common.h"

/** @brief Layout buffer view for active records, identity words, and packed metadata. */
typedef struct FieldMenuRecordLayout
{
    u8 pad0[0x640];
    u8 unk640;
    u8 pad641[0x37];
    u32 unk678, unk67C;
    u8 pad680[0x660];
    u8 unkCE0;
    u8 padCE1[0x37];
    u32 unkD18, unkD1C;
    u8 padD20[0x2440];
    u8 unk3160;
    u8 pad3161[0x13];
    union
    {
        u32 word;
        u16 halves[2];
    } packed;
    u8 pad3178[0x20];
    u32 unk3198, unk319C;
} FieldMenuRecordLayout;

/** @brief Shared 64-byte record used by the menu record transfer path. */
typedef struct FieldSharedRecord
{
    u8 unk0;
    u8 pad1[0x13];
    union
    {
        u32 word;
        u16 halves[2];
    } packed_metadata;
    u32 unk18;
    u32 unk1C;
    u8 pad20[4];
    u16 unk24;
    u16 unk26;
    u16 unk28;
    u16 unk2A;
    u8 pad2C[8];
    u32 unk34;
    u32 unk38;
    u32 unk3C;
} FieldSharedRecord;

extern u8 D_800F0E98[];

/**
 * @brief Validate the selected shared record and prepare its menu display state.
 */
void func_800C8A2C(void)
{
    s32 work_index;
    s32 duplicate_found;
    s32 selected_index;
    u8 *initial_record;
    u8 *selected_record;
    u8 *detail_record;
    u8 *search_record;
    u8 *scan;
    u32 packed_metadata;
    s32 record_type;
    s32 lookup_row;
    s32 metadata_index;
    s32 display_value;
    u32 low_word;
    u32 high_word;
    s32 nibble_sum;
    s32 special_flag;
    u32 amount;
    s32 digits;
    u8 *lookup_base;
    s32 lookup_offset;
    u8 *display;
    u8 *status;
    u8 *initial_base;

    status = &D_80122C02;
    status[1] = 0;
    selected_index = D_80122C02;
    initial_base = D_80122A08;
    initial_record = (selected_index << 6) + initial_base;
    if (initial_record[0] == 0)
    {
        status[1] = 1;
        return;
    }

    duplicate_found = 0;

    goto search_first;

found_first:
    duplicate_found = 1;
    goto search_second;

found_second:
    duplicate_found = 1;
    goto search_third;

found_third:
    duplicate_found = 1;
    goto searches_done;

search_first:
    work_index = duplicate_found;
    search_record = initial_record;
    scan = g_saved_game.bytes;
first_loop:
    if (((FieldMenuRecordLayout *)scan)->unkCE0 != 0 &&
        ((FieldMenuRecordLayout *)scan)->unkD18 == *(u32 *)(search_record + 0x38) &&
        ((FieldMenuRecordLayout *)scan)->unkD1C == *(u32 *)(search_record + 0x3C))
    {
        goto found_first;
    }
    work_index++;
    scan += 0x40;
    if (work_index < 100)
    {
        goto first_loop;
    }

search_second:
    work_index = 0;
    {
        u8 *record_base;
        record_base = D_80122A08;
        search_record = record_base + (selected_index << 6);
    }
    scan = g_saved_game.bytes;
second_loop:
    if (((FieldMenuRecordLayout *)scan)->unk640 != 0 &&
        ((FieldMenuRecordLayout *)scan)->unk678 == *(u32 *)(search_record + 0x38) &&
        ((FieldMenuRecordLayout *)scan)->unk67C == *(u32 *)(search_record + 0x3C))
    {
        goto found_second;
    }
    work_index++;
    scan += 0x40;
    if (work_index < 8)
    {
        goto second_loop;
    }

search_third:
    work_index = 0;
    {
        u8 *record_base;
        record_base = D_80122A08;
        search_record = record_base + (selected_index << 6);
    }
    scan = g_saved_game.bytes;
third_loop:
    if (((FieldMenuRecordLayout *)scan)->unk3160 != 0 &&
        ((FieldMenuRecordLayout *)scan)->unk3198 == *(u32 *)(search_record + 0x38) &&
        ((FieldMenuRecordLayout *)scan)->unk319C == *(u32 *)(search_record + 0x3C))
    {
        goto found_third;
    }
    work_index++;
    scan += 0x40;
    if (work_index < 4)
    {
        goto third_loop;
    }

searches_done:
    if (duplicate_found == 0)
    {
        packed_metadata = ((FieldSharedRecord *)D_80122A08)[selected_index].packed_metadata.word;
        record_type = (packed_metadata >> 8) & 3;
        lookup_row = (packed_metadata >> 10) & 0x3F;
        if (record_type == 1)
        {
            lookup_row += 0xB;
        }
        else if (record_type == 2)
        {
            lookup_row += 0x17;
        }

        {
            u8 *detail_base;
            detail_base = D_80122A08;
            detail_record = (selected_index << 6) + detail_base;
        }
        metadata_index = *(u16 *)(detail_record + 0x16) & 0x3F;
        if (record_type == 0)
        {
            do
            {
                display_value = *(u16 *)(detail_record + 0x24);
            } while (0);
        }
        else if (record_type == 1)
        {
            do
            {
                display_value = *(u16 *)(detail_record + 0x24);
                display_value += *(u16 *)(detail_record + 0x26);
                display_value += *(u16 *)(detail_record + 0x28);
                display_value += *(u16 *)(detail_record + 0x2A);
            } while (0);
        }
        else
        {
            do
            {
                display_value = detail_record[0x26];
            } while (0);
        }

        {
            u8 *final_base;
            final_base = D_80122A08;
            selected_record = (selected_index << 6) + final_base;
        }
        low_word = *(u32 *)(selected_record + 0x18);
        high_word = *(u32 *)(selected_record + 0x1C);
        nibble_sum = (low_word & 0xF) + ((low_word >> 4) & 0xF) + ((low_word >> 8) & 0xF) + ((low_word >> 12) & 0xF) +
                     ((low_word >> 16) & 0xF) + ((low_word >> 20) & 0xF) + ((low_word >> 24) & 0xF) + (low_word >> 28) +
                     (high_word & 0xF) + ((high_word >> 4) & 0xF) + ((high_word >> 8) & 0xF) + ((high_word >> 12) & 0xF) +
                     ((high_word >> 16) & 0xF) + ((high_word >> 20) & 0xF) + ((high_word >> 24) & 0xF) + (high_word >> 28);
        special_flag = nibble_sum >= 0x29;
        if (record_type == 2)
        {
            special_flag = selected_record[0x24];
        }

        amount = *(u32 *)(selected_record + 0x34);
        func_800B2844(0, selected_record, 0xFF);
        D_80122C04 = record_type;
        display = &D_80122C04;
        lookup_base = D_800F0E98;
        lookup_offset = metadata_index * 2;
        display[1] = lookup_row;
        func_800B2844(1, lookup_base[lookup_offset] + (D_800F0E98[lookup_offset + 1] << 8) + D_800F0E98, 0xFF);
        *(u16 *)(display + 2) = display_value;
        if (special_flag != 0)
        {
            *(u16 *)(display + 2) = display_value - 0x8000;
        }
        *(u32 *)(display + 4) = amount;
        digits = 0;
        do
        {
            do
            {
                amount /= 10;
            } while (0);
            digits++;
        } while (amount != 0);
        D_80122C0C = digits;
        return;
    }

    D_80122C03 = 2;
    func_800B2844(0, (selected_index << 6) + D_80122A08, 0xFF);
}



/** @brief Remove spent records and compact the four-entry pending table. */
void func_800C8E2C(void)
{
    u8 *rec;
    s32 outer_i;
    s32 src_off;
    u8 *scan;
    u8 *base;
    u8 *dest_base;
    s32 inner_i;
    s32 off;
    u8 *a1;
    u8 *s0;
    s32 dest_off;

    outer_i = 0;
    rec = g_saved_game.bytes;
clear_records:
    {
        if (rec[0x3160] != 0 && *(s32 *)(rec + 0x3194) == 0)
        {
            rec[0x3160] = 0;
        }
        outer_i += 1;
        rec += 0x40;
    }
    if (outer_i < 4)
    {
        goto clear_records;
    }

    outer_i = 0;
    base = g_saved_game.bytes;
    dest_base = base + 0x3160;
    src_off = outer_i;
    scan = base;
compact_records:
    {
        if (scan[0x3160] == 0 && *(s32 *)(scan + 0x3194) == 0)
        {
            inner_i = outer_i + 1;
            if (inner_i < 4)
            {
                off = inner_i << 6;
                dest_off = src_off;
                a1 = (u8 *)((u32)off + (u32)dest_base);
                s0 = (u8 *)((u32)off + (u32)base);
            loop_10:
                inner_i += 1;
                if (s0[0x3160] != 0)
                {
                    do
                    {
                        field_copy_inventory_record((void *)((u32)dest_off + (u32)dest_base), a1);
                    } while (0);
                    s0[0x3160] = 0;
                    *(s32 *)(s0 + 0x3194) = 0;
                }
                else
                {
                    a1 += 0x40;
                    s0 += 0x40;
                    if (inner_i < 4)
                    {
                        goto loop_10;
                    }
                }
            }
        }
        src_off += 0x40;
        outer_i += 1;
        scan += 0x40;
    }
    if (outer_i < 4)
    {
        goto compact_records;
    }
}


typedef struct
{
    u8 pad0[0x3160];
    u8 unk3160;   /* 0x3160 */
    u8 pad3161[0x3194 - 0x3161];
    s32 unk3194;  /* 0x3194 */
} BigStruct;

extern u8 D_80046138[];


extern /** @brief Remove spent records and compact the four-entry pending table. */
void func_800C8E2C(void);

/** @brief Copy the selected pending record out and compact the table. */
void func_800C8F4C(void)
{
    s32 idx;
    s32 offset;
    u8 *handle;
    BigStruct *rec;

    idx = D_80122C02;
    handle = field_find_free_inventory_record();
    offset = idx << 6;
    field_copy_inventory_record(handle, &D_80046138[offset]);
    rec = (BigStruct *) (D_80046138 - 0x3160 + offset);
    rec->unk3160 = 0;
    rec->unk3194 = 0;
    func_800C8E2C();
}




extern /** @brief Remove spent records and compact the four-entry pending table. */
void func_800C8E2C(void);

/** @brief Restore the first pending result record and expose its result. */
void func_800C8FA8(void)
{
    s32 i;
    s32 result;
    u8 *base;
    u8 *arg;
    u8 *p;

    result = 0;
    D_80122C02 = 0xFF;
    i = 0;
    p = g_saved_game.bytes;
    arg = p + 0x3160;
    base = p;

loop:
    if (base[0x3160] == 0 && *(s32 *)&base[0x3194] != 0)
    {
        base[0x3160] = base[0x3180];
        func_800B2844(0, arg, 0xFF);
        result = *(s32 *)&base[0x3194];
        *(s32 *)&base[0x3194] = 0;
        D_80122C02 = i;
    }
    else
    {
        arg += 0x40;
        i++;
        base += 0x40;
        if (i < 4)
        {
            goto loop;
        }
    }

    (*(s32 *)&D_80122C08) = result;
    if (result == 0)
    {
        func_800C8E2C();
    }
}


extern u8 D_80046138[], D_800F0E98[];
/**
 * @brief Count available records, detect duplicate identities, and prepare the selected record display.
 */
void func_800C905C(void)
{
    s32 layout_base;
    s32 second_search_base;
    s32 display_base;
    s32 metadata_index;
    s32 lookup_offset;
    s32 selected_offset;
    s32 work_index;
    s32 duplicate_found;
    s32 record_type;
    s32 work_value;
    s32 lookup_row;
    u32 packed_metadata;
    u8 *search_record;
    u8 *selected_record;
    u8 *lookup_base;
    u8 *record_base;
    s32 selected_index;

    selected_index = D_80122C02;
    work_value = 0;
    work_index = work_value;
    layout_base = (s32)g_saved_game.bytes + work_index * 0x40;
    do
    {
        if (((FieldMenuRecordLayout *)layout_base)->unk3160 != 0)
        {
            work_value += 1;
        }
        work_index += 1;
        layout_base = (s32)g_saved_game.bytes + work_index * 0x40;
    } while (work_index < 4);

    work_index = (s32)&(*(u8 *)&D_80122C06);
    *(u8 *)work_index = work_value;
    work_value = (s32)g_saved_game.bytes;
    selected_record = (selected_index << 6) + g_saved_game.bytes;
    ((u8 *)work_index)[-3] = 0;
    if (((FieldMenuRecordLayout *)selected_record)->unk3160 == 0)
    {
        ((u8 *)work_index)[-3] = 1;
        return;
    }

    duplicate_found = 0;
    goto search_first;

found_first:
    duplicate_found = 1;
    goto search_second;

found_second:
    duplicate_found = 1;
    goto searches_done;

search_first:
    work_index = duplicate_found;
    search_record = selected_record;
first_loop:
    if (((FieldMenuRecordLayout *)work_value)->unkCE0 != 0 &&
        ((FieldMenuRecordLayout *)work_value)->unkD18 == ((FieldMenuRecordLayout *)search_record)->unk3198 &&
        ((FieldMenuRecordLayout *)work_value)->unkD1C == ((FieldMenuRecordLayout *)search_record)->unk319C)
    {
        goto found_first;
    }
    work_index++;
    work_value += 0x40;
    if (work_index < 100)
    {
        goto first_loop;
    }

search_second:
    work_index = 0;
    second_search_base = (s32)g_saved_game.bytes;
    search_record = (u8 *)((selected_index << 6) + second_search_base);
    work_value = second_search_base;
second_loop:
    if (((FieldMenuRecordLayout *)work_value)->unk640 != 0 &&
        ((FieldMenuRecordLayout *)work_value)->unk678 == ((FieldMenuRecordLayout *)search_record)->unk3198 &&
        ((FieldMenuRecordLayout *)work_value)->unk67C == ((FieldMenuRecordLayout *)search_record)->unk319C)
    {
        goto found_second;
    }
    work_index++;
    work_value += 0x40;
    if (work_index < 8)
    {
        goto second_loop;
    }

searches_done:
    if (duplicate_found == 0)
    {
        s32 row_offset = selected_index << 6;

        layout_base = (s32)g_saved_game.bytes;
        packed_metadata = ((FieldMenuRecordLayout *)(row_offset + layout_base))->packed.word;
        record_type = (packed_metadata >> 8) & 3;
        lookup_row = (packed_metadata >> 0xA) & 0x3F;
        if (record_type == 1)
        {
            lookup_row += 0xB;
            work_value = 0;
        }
        else if (record_type == 2)
        {
            lookup_row += 0x17;
            work_value = 0;
        }
        else
        {
            work_value = 0;
        }

        display_base = (s32)g_saved_game.bytes;
        selected_offset = selected_index << 6;
        metadata_index = ((FieldMenuRecordLayout *)(selected_offset + display_base))->packed.halves[1] & 0x3F;
        record_base = (u8 *)display_base + 0x3160;
        func_800B2844(work_value, selected_offset + record_base, 0xFF);
        D_80122C04 = record_type;
        lookup_base = D_800F0E98;
        lookup_offset = metadata_index * 2;
        (&D_80122C04)[1] = lookup_row;
        func_800B2844(1, lookup_base[lookup_offset] + (D_800F0E98[lookup_offset + 1] << 8) + D_800F0E98, 0xFF);
        return;
    }

    func_800B2844(0, (selected_index << 6) + D_80046138, 0xFF);
    D_80122C03 = 2;
}

#include "game_audio.h"
#include "saved_game.h"
#include "main.h"

/**
 * @brief Reset the current music track index to zero.
 */
void field_reset_music_track_index(void)
{
    g_music_track_index = 0;
}



extern s32 func_800BD414(s32 arg0, s32 arg1);
extern void func_800AD194(s32 arg0);

/**
 * @brief Clear the selected record flag or report an invalid index.
 * @see decomp.me (100%)
 */
void func_800C92B8(void)
{
    s32 index;
    u8 *base;
    u8 *record;

    index = (&D_80122C19)[D_80122C19 + 4];
    if (index < 5)
    {
        base = g_saved_game.bytes;
        record = base + index * 0x60;
        *(u32 *)(record + 0x2F38) &= 0xBFFFFFFF;
    }
    else
    {
        record_game_diagnostic(0x8002, 0x4B, index, 0);
    }
}

/**
 * @brief Update the selected record value or report an invalid index.
 * @see decomp.me (100%)
 */
void func_800C9330(void)
{
    s32 index;
    u8 val;
    u8 *base;
    u8 *record;

    val = D_80122C11;
    index = (&D_80122C11)[1];
    if (index < 5)
    {
        base = g_saved_game.bytes;
        record = base + index * 0x60;
        record[0x2F09] = val;
        *(u32 *)(record + 0x2F38) &= 0x7FFFFFFF;
        if (record[0x2EF4] == 0)
        {
            record[0x2EF4] = 0x41;
        }
    }
    else
    {
        record_game_diagnostic(0x8002, 0x4C, index, 0);
    }
}

/**
 * @brief Checks field state 0x2F08 and performs the corresponding update.
 */
void func_800C93B4(void)
{
    if (func_800BD414(0, 0x2F08) == 0x80)
    {
        func_800AD194(1);
    }
    else if (func_800BD414(0, 0x2F08) == 0xFF)
    {
        func_800AD194(0);
    }
}

#include "saved_game.h"
#include "common.h"

typedef struct
{
    u8 unk0;
    u8 pad1[0x1F];
    u8 unk20;
    u8 pad21[0x13];
    u32 unk34;
    u8 pad38[8];
} Rec800C9684;




/** @brief Dispatch the nonempty menu record at buffer offset 0x840. */
void func_800C9404(void)
{
    (*(u8 *)&D_80122C00) = g_saved_game.bytes[0x840];
    if ((*(u8 *)&D_80122C00) != 0)
    {
        func_800B2844(0, &g_saved_game.bytes[0x840], 0xFF);
    }
}


/** @brief Count empty entries in the four-record menu table. */
void func_800C9448(void)
{
    s32 i;
    s32 count;
    u8 *p;

    count = 0;
    for (i = 0; i < 4; i++)
    {
        p = &g_saved_game.bytes[i * 0x40];
        if (p[0x3160] == 0)
        {
            count++;
        }
    }
    D_80122C1F = (u8) count;
}


/** @brief Count empty entries in the hundred-record equipment table. */
void func_800C9488(void)
{
    s32 i;
    s32 count;
    u8 *p;

    count = 0;
    for (i = 0; i < 0x64; i++)
    {
        p = &g_saved_game.bytes[i * 0x40];
        if (p[0xCE0] == 0)
        {
            count++;
        }
    }
    D_80122C1F = (u8) count;
}



/** @brief Clear the four shared records and their saved result words. */
void func_800C94C8(void)
{
    ((u8 *)((Rec800C9684 *)&D_80122A08))[0] = 0;
    ((u8 *)((Rec800C9684 *)&D_80122A08))[0x40] = 0;
    ((u8 *)((Rec800C9684 *)&D_80122A08))[0x80] = 0;
    ((u8 *)((Rec800C9684 *)&D_80122A08))[0xC0] = 0;
    *(s32 *)&((u8 *)((Rec800C9684 *)&D_80122A08))[0x34] = 0;
    *(s32 *)&((u8 *)((Rec800C9684 *)&D_80122A08))[0x74] = 0;
    *(s32 *)&((u8 *)((Rec800C9684 *)&D_80122A08))[0xB4] = 0;
    *(s32 *)&((u8 *)((Rec800C9684 *)&D_80122A08))[0xF4] = 0;
}

extern s32 D_8011F428;


/**
 * @brief Apply the pending field mode transition to the four shared records.
 */
void func_800C94F4(void)
{
    s32 mode;
    s32 i;
    u8 *entry;
    s32 result;

    mode = D_80122C1E;
    if (mode == 1 && D_8011F428 == 0)
    {
        for (i = 0; i < 4; i++)
        {
            if (((Rec800C9684 *)&D_80122A08)[i].unk0 == 0)
            {
                entry = &((u8 *)&D_80122C19)[i];
                if (((Rec800C9684 *)&D_80122A08)[i].unk34 != 0 && *entry != 0xFA)
                {
                    ((Rec800C9684 *)&D_80122A08)[i].unk0 = ((Rec800C9684 *)&D_80122A08)[i].unk20;
                    ((Rec800C9684 *)&D_80122A08)[i].unk20 = *entry;
                    if (field_find_free_inventory_record(entry) != 0)
                    {
                        field_copy_inventory_record(field_find_free_inventory_record(), &((Rec800C9684 *)&D_80122A08)[i]);
                    }
                }
            }
        }
    }
    if (mode == 0 && D_8011F428 == 1)
    {
        for (i = 0; i < 4; i++)
        {
            if (((Rec800C9684 *)&D_80122A08)[i].unk0 != 0)
            {
                result = ((Rec800C9684 *)&D_80122A08)[i].unk34;
                if (result == 0)
                {
                    result = 1;
                }
                ((Rec800C9684 *)&D_80122A08)[i].unk34 = result;
            }
        }
    }
    D_80122C1E = (u8) D_8011F428;
}

extern u8 D_80043818;
extern void func_800BD520(s32 arg0, s32 arg1, s32 arg2);

/** @brief Set field state 0x2F08 from the pending mode flag. */
void func_800C963C(void)
{
    if (D_80043818 != 0)
    {
        func_800BD520(0, 0x2F08, 0x80);
    }
    else
    {
        func_800BD520(0, 0x2F08, 0xFF);
    }
}


/** @brief Save the selected shared record and install its pending result. */
void func_800C9684(void)
{
    s32 index;
    Rec800C9684 *rec;
    u8 tmp;
    u8 tmp2;

    index = D_80122C02;
    tmp = ((Rec800C9684 *)&D_80122A08)[index].unk20;
    rec = &((Rec800C9684 *)&D_80122A08)[index];
    (&D_80122C02)[index + 0x17] = tmp;
    tmp2 = rec->unk0;
    rec->unk0 = 0;
    rec->unk20 = tmp2;
    rec->unk34 = *(u32 *) ((u8 *) &D_80122C02 + 6);
}

#include "saved_game.h"
#include "common.h"

/** @brief Eight selection-index adjustments copied to the stack. */
typedef struct
{
    s32 entries[8];
} Choices;
/** @brief Menu-layout field used to select the preferred choice. */
typedef struct
{
    u8 pad[0x2E6];
    u16 selected;
} AttributeLayout;

/** @brief Scratch storage reused for the active layout and selected choice. */
typedef union
{
    AttributeLayout *layout;
    s32 selected;
} LayoutSelection;
extern Choices D_80051ED8;

extern u16 g_music_track_index;
extern s32 rand(void);
/** @brief Choose one of eight entries using fourth-power weights and the current mode. */
void func_800C96C4(void)
{
    s32 weights[8];
    Choices choices;
    s32 total, i, offset, value, weight, draw;
    LayoutSelection selection;
    s32 *cursor, *base;
    s32 track, byte_offset, random_product;
    s32 mode;
    total = 0;
    i = total;
    selection.layout = (AttributeLayout *)g_saved_game.bytes;
    mode = (*(u8 *)&D_80122C00);
    track = g_music_track_index;
    choices = D_80051ED8;
    offset = track * 12;
loop_weights:
    value = ((u8 *)selection.layout)[i + offset + 0x2F4];
    weight = value - 3;
    if (i == choices.entries[selection.layout->selected & 0x7F])
    {
        weight = value - 2;
    }
    if (weight >= 0)
    {
        value = 3;
        if (weight < 4)
        {
            value = weight;
        }
    }
    else
    {
        value = 0;
    }
    weight = value * value;
    weight = weight * weight;
    byte_offset = i * 4;
    i++;
    base = weights;
    *(s32 *)((u8 *)base + byte_offset) = weight;
    total += weight;
    if (i < 8)
    {
        goto loop_weights;
    }
    if (mode == 1)
    {
        total = 0x288;
    }
    random_product = rand() * total;
    draw = random_product >> 15;
    if (random_product < 0)
    {
        draw = (random_product + 0x7FFF) >> 15;
    }
    total = 0;
    selection.selected = 0xFF;
    i = total;
    cursor = base;
loop:
    if (draw >= total && draw < total + *cursor)
    {
        goto found;
    }
    if (mode == 0)
    {
        total += *cursor;
    }
    else
    {
        total += 0x51;
    }
    i++;
    cursor++;
    if (i < 8)
    {
        goto loop;
    }
finish:
    if (mode != 0)
    {
        goto store_mode_one;
    }
    D_80122C05 = selection.selected;
    goto done;
found:
    selection.selected = i;
    goto finish;
store_mode_one:
    (*(u8 *)&D_80122C06) = selection.selected;
done:
    return;
}

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
} UnkStruct80051EF8;

extern UnkStruct80051EF8 D_80051EF8;

/** @brief Open the attribute selection screen sequence. */
void func_800C9894(void)
{
    UnkStruct80051EF8 local;

    local = D_80051EF8;
    field_open_gosub_screen_sequence(&local);
}

typedef struct RecC98D4
{
    u8 pad0[0x24];
    u8 unk24;
    u8 pad25;
    u8 unk26;
} RecC98D4;

typedef struct OutC98D4
{
    u8 unk0;
    u8 unk1;
    u8 unk2;
    u8 unk3;
} OutC98D4;


/**
 * @brief Pack the current gosub result's color/attribute bytes into D_80122C01.
 *
 * If there are gosub results, reads the selected result's 0x40-byte layout record
 * (at @c g_saved_game.bytes + 0xCE0) for its 0x24 and 0x26 fields and a 6-bit
 * attribute from the 0xCF4 word; otherwise defaults to 0xFF. The 0x26 field is
 * clamped to 0..0x63 and all four bytes are written to @c D_80122C01.
 *
 * @note gcc280_g0, 100% match.
 */
void func_800C98D4(void)
{
    s32 result_value;
    s32 attr;
    s32 flags;
    s32 clamp_src;
    s32 out;
    RecC98D4 *rec;

    result_value = 0xFF;
    attr = 0xFF;
    flags = 0xFF;
    clamp_src = 0;
    if (g_gosub_result_count != 0)
    {
        u8 *buf = g_saved_game.bytes;
        u8 *base;
        u8 *recbase;
        result_value = (*(s32 *)&g_gosub_result_values);
        base = buf + result_value * 0x40;
        recbase = buf + 0xCE0;
        rec = (RecC98D4 *)(recbase + result_value * 0x40);
        flags = rec->unk24;
        attr = (*(u32 *)(base + 0xCF4) >> 10) & 0x3F;
        clamp_src = rec->unk26;
    }
    if (clamp_src >= 0)
    {
        out = 0x63;
        if (clamp_src < 0x64)
        {
            out = clamp_src;
        }
    }
    else
    {
        out = 0;
    }
    ((OutC98D4 *)&D_80122C01)->unk0 = result_value;
    ((OutC98D4 *)&D_80122C01)->unk1 = attr;
    ((OutC98D4 *)&D_80122C01)->unk2 = flags;
    ((OutC98D4 *)&D_80122C01)->unk3 = out;
}

/** @brief Nine-entry relationship lookup copied into the calculation workspace. */
typedef struct Lookup
{
    s32 values[9];
} Lookup;
extern Lookup D_80051F04;
/**
 * @brief Apply packed category and element multipliers to the two pending values.
 *
 * Both results remain signed 32-bit values until clamped to 0..32767.
 */
void func_800C9960(void)
{
    Lookup lookup;
    u8 *base;
    s32 packed;
    s32 first;
    s32 second;
    s32 element;
    s32 amount;
    s32 level;
    s32 primary;
    s32 secondary;
    s32 third;
    u32 fourth;
    s32 pair;
    s32 low;
    u32 high;
    s32 selected;
    s32 mode;
    s32 factor1;
    s32 factor2;
    s32 element1;
    s32 element2;
    s32 product1, product2, scaled1, scaled2;
    s32 clamp;
    s32 result1;
    s32 result2;
    lookup = D_80051F04;
    factor1 = 10;
    do
    {
        factor1 = factor1;
    } while (0);
    base = &(*(u8 *)&D_80122C06);
    packed = base[0];
    first = *(s16 *)(base + 10);
    second = *(s16 *)(base + 12);
    element = base[-3];
    amount = base[-2];
    level = base[2];
    second++;
    second--;
    primary = packed & 3;
    secondary = (packed >> 2) & 3;
    third = (packed >> 4) & 3;
    fourth = (u8)packed >> 6;
    packed = base[-1];
    pair = packed;
    low = pair & 15;
    high = (u8)pair >> 4;
    packed = base[1];
    selected = packed;
    mode = base[4];
    if (selected != primary)
    {
        factor1 = 2;
        if (selected == secondary)
        {
            factor1 = 1;
        }
    }
    factor2 = 10;
    if (selected != third)
    {
        factor2 = 2;
        if (selected == fourth)
        {
            factor2 = 1;
        }
    }
    if (mode == 0)
    {
        if (element == low)
        {
            element1 = 3;
        }
        else if (element == lookup.values[low])
        {
            element1 = 1;
        }
        else
        {
            element1 = 2;
        }
        if (element == high)
        {
            element2 = 3;
        }
        else if (element == lookup.values[high])
        {
            element2 = 1;
        }
        else
        {
            element2 = 2;
        }
    }
    else
    {
        element1 = 2;
        element2 = element1;
    }
    first /= level + 6;
    second /= level + 6;
    product1 = factor1 * element1;
    product2 = factor2 * element2;
    scaled1 = product1 * amount;
    scaled2 = product2 * amount;
    first += scaled1;
    second += scaled2;
    first *= level + 7;
    second *= level + 7;
    if (first >= 0)
    {
        result1 = 0x7FFF;
        clamp = result1;
        clamp = clamp < first;
        if (!clamp)
        {
            result1 = first;
        }
    }
    else
    {
        result1 = 0;
    }
    first = result1;
    if (second >= 0)
    {
        result2 = 0x7FFF;
        if (factor1)
        {
            clamp = result2;
        }
        else
        {
            clamp = (result2 | 0x10000) & 0x7FFF;
        }
        clamp = clamp < second;
        if (!clamp)
        {
            result2 = second;
        }
    }
    else
    {
        result2 = 0;
    }
    ((s16 *)&D_80122C10)[0] = first;
    ((s16 *)&D_80122C10)[1] = result2;
}

#include "saved_game.h"
#include "common.h"

u8 *func_800C1E40(s32 arg0);
void func_800C57D4(void);
void func_800AD030(s32 arg0);

/*
 * Deliberately declared without a prototype: func_800C9C3C passes a fourth
 * argument to keep its cursor byte live in a3, while the other callers pass
 * three. A fixed-arity prototype would change codegen for one side or the
 * other.
 */

extern u8 D_800459AC;


/**
 * @brief Dispatch the current menu record for the active cursor slot.
 * @see decomp.me (100%)
 */
void func_800C9BC4(void)
{
    s32 temp_s1;
    s32 temp_s0;
    s32 off;

    temp_s1 = (*(u8 *)&D_80122C00);
    if (field_find_free_inventory_record() != 0)
    {
        temp_s0 = field_find_free_inventory_record();
        field_copy_inventory_record(temp_s0, func_800C1E40(5) + (off = (temp_s1 << 6) + 4));
        func_800B2844(0, func_800C1E40(5) + off, 0xFF);
    }
}

/**
 * @brief Latch the current menu selection as the pending gosub result.
 *
 * Reads the active menu record's selection index from @c g_saved_game.bytes; if
 * it is in range (< 5), records it into the @c D_80122C1C cursor slot and the
 * pending-result globals, sets the record's 0x40000000 flag, and dispatches
 * func_800B2844 for it.
 *
 * @see decomp.me (100%) TODO
 */
void func_800C9C3C(void)
{
    u8 *dbase;
    u8 d0;
    u8 *mlb;
    u8 *ptr;
    u8 *dp1;
    u8 *argp;
    s32 val;
    s32 sel;
    s32 off;
    u8 *rec;

    func_800C57D4();
    dbase = &(*(u8 *)&D_80122C1C);
    d0 = (*(u8 *)&D_80122C1C);
    dp1 = dbase + 1;
    ptr = d0 + dp1;
    val = *ptr;
    g_gosub_result_count = 1;
    mlb = g_saved_game.bytes;
    sel = *(s32 *)(mlb + 0x2EF0);
    (*(s32 *)&g_gosub_result_values) = val;
    if (sel < 5)
    {
        *ptr = (u8)sel;
        off = sel * 0x60;
        rec = off + mlb;
        *(s16 *)(dbase - 8) = *(u8 *)(rec + 0x2F09);
        *(s32 *)(rec + 0x2F38) = *(s32 *)(rec + 0x2F38) | 0x40000000;
        argp = mlb + 0x2EF4;
        func_800B2844(d0, off + argp, 0xFF, d0);
    }
}

/**
 * @brief Re-select the cursor slot's menu record and dispatch it.
 */
void func_800C9CE4(void)
{
    u8 *base;
    u8 *rec;
    s32 idx;

    idx = ((u8 *)&(*(u8 *)&D_80122C1C))[(*(u8 *)&D_80122C1C) + 1] * 0x60;
    base = g_saved_game.bytes;
    rec = idx + base;
    (*(u8 *)&D_80122C1C) = rec[0x2F3C];
    base = base + 0x2EF4;
    func_800B2844(3, idx + base, 0xFF);
}

/**
 * @brief Count the active menu records and store the total in D_80122C16.
 */
void func_800C9D44(void)
{
    s32 i;
    s32 count;
    u8 *p;

    count = 0;
    for (i = 0; i < 5; i++)
    {
        p = &g_saved_game.bytes[i * 0x60];
        if (p[0x2EF4] != 0)
        {
            count++;
        }
    }
    D_80122C16 = (u16) count;
}

/**
 * @brief Forward D_80122C01 to func_800AD030 after the common menu prologue.
 */
void func_800C9D84(void)
{
    func_800C57D4();
    func_800AD030(D_80122C01);
}

/**
 * @brief Store the high nibble of D_800459AC into D_80122C12.
 */
void func_800C9DB4(void)
{
    D_80122C12 = (s8) ((u8) D_800459AC >> 4);
}


/** @brief Byte-oriented view of a resource table entry's payload. */
typedef struct
{
    u8 pad0[4];
    u8 value;
} FieldResourceOffsetByte;

/** @brief Active menu record selected from D_80043CB8. */
typedef struct
{
    u8 pad0[0x24];
    u8 unk24;
    u8 unk25;
    u8 pad26[0x1A];
} FieldMenuRecordC9DCC;



u8 *func_800C1E40(s32 arg0);

/**
 * @brief Dispatch the active menu record and entries referenced by resource tables 0x103 and 0x104.
 */
void func_800C9DCC(void)
{
    FieldMenuRecordC9DCC *record;
    FieldMenuRecordC9DCC *record_base;
    s32 record_index;
    s32 unk24_value;
    s32 unk25_value;
    s32 lookup_index;
    s32 resource_index_103;
    s32 resource_index_104;
    u8 *resource_103;
    u8 *resource_104;
    s32 resource_offset;

    record_index = D_80122C10;
    record_base = ((FieldMenuRecordC9DCC *)&D_80043CB8);
    record = record_base + record_index;
    unk24_value = record->unk24;
    unk25_value = record->unk25;
    lookup_index = (unk24_value * 0xE) + unk25_value;
    func_800B2844(0, (u8 *)record, 0xFF);

    resource_103 = func_800C1E40(0x103);
    resource_index_103 = lookup_index * 2;
    resource_offset = ((FieldResourceOffsetByte *)(resource_103 + resource_index_103))->value +
                      (((FieldResourceOffsetByte *)(func_800C1E40(0x103) + (resource_index_103 += 1)))->value << 8);
    func_800B2844(1, func_800C1E40(0x103) + (resource_offset + 4), 0xFF);

    resource_104 = func_800C1E40(0x104);
    resource_index_104 = unk25_value * 2;
    resource_offset = ((FieldResourceOffsetByte *)(resource_104 + resource_index_104))->value +
                      (((FieldResourceOffsetByte *)(func_800C1E40(0x104) + (resource_index_104 += 1)))->value << 8);
    func_800B2844(2, func_800C1E40(0x104) + (resource_offset + 4), 0xFF);
}
