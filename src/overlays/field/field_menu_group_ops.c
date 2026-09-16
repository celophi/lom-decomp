#include "common.h"

extern s32 D_80122C00;
extern u8 g_menuLayoutBuffer[];
extern s32 g_gosub_result_values[];
extern s32 g_gosub_result_count;
extern s16 D_80122C06, D_80122C08, D_80122C0A, D_80122C1C, D_80122C1E;
extern u8 D_800459AF;
extern s8 D_800459B3;
extern void func_800C4364(s32);
extern void func_800A54D0(void);
extern void func_800A8FB4(void);
extern void func_800A8F8C(void *, void *);


extern void (*D_800F19D8[])(s32 arg0);

void func_800C5704(s32 arg0)
{
    if (arg0 < 0x60)
    {
        D_800F19D8[arg0](arg0);
        return;
    }
    akao_set_song_params(0x8002, arg0, 0, 0);
}


typedef struct
{
    s32 unk0;
    s16 unk4;
} GroupStateView;


extern u8 g_menuLayoutBuffer[];

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

    menu = g_menuLayoutBuffer;
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
            if (g_menuLayoutBuffer[scan_index + 0x29D8] == index)
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
        count = g_menuLayoutBuffer[0x29D5] + 1;
        g_menuLayoutBuffer[0x29D5] = count;
        if ((u32) (count & 0xFF) >= 0xC9U)
        {
            g_menuLayoutBuffer[0x29D5] = 0xC8U;
        }
        record_index = g_menuLayoutBuffer[D_80122C00 + 0x29D8];
        if (record_index != 3)
        {
            if (g_menuLayoutBuffer[0x29D8] == 3)
            {
                g_menuLayoutBuffer[0x29D8] = record_index;
            }
            else if (g_menuLayoutBuffer[0x29D9] == 3)
            {
                g_menuLayoutBuffer[0x29D9] = record_index;
            }
            else if (g_menuLayoutBuffer[0x29DA] == 3)
            {
                g_menuLayoutBuffer[0x29DA] = record_index;
            }
        }
        g_menuLayoutBuffer[D_80122C00 + 0x29D8] = unused_index;
        packed_flags = ((*(s32 *)&g_menuLayoutBuffer[0x29D4]) & ~0xF) | (((g_menuLayoutBuffer[0x29D4] & 0xF) + 1) & 0xF);
        (*(s32 *)&g_menuLayoutBuffer[0x29D4]) = packed_flags;
        if ((u32) (g_menuLayoutBuffer[0x29D4] & 0xF) >= 4U)
        {
            (*(s32 *)&g_menuLayoutBuffer[0x29D4]) = (s32) ((packed_flags & ~0xF) | 3);
        }
        index = 0;
        if (g_gosub_result_count > 0)
        {
            do
            {
                u8 *destination;

                destination = (g_menuLayoutBuffer[D_80122C00 + 0x29D8] * 0x14C) + (g_menuLayoutBuffer + 0x2B58);
                destination += index << 6;
                func_800A8F8C(destination, (g_gosub_result_values[index] << 6) + (g_menuLayoutBuffer + 0xCE0));
                g_menuLayoutBuffer[(g_gosub_result_values[index] << 6) + 0xCE0] = 0;
                index += 1;
            } while (index < g_gosub_result_count);
        }
        func_800A8FB4();
        index = g_gosub_result_count;
        if (index < 4)
        {
            do
            {
                entry_offset = index << 6;
                index += 1;
                g_menuLayoutBuffer[entry_offset + g_menuLayoutBuffer[D_80122C00 + 0x29D8] * 0x14C + 0x2B58] = 0;
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


extern u8 g_menuLayoutBuffer[];

void func_800C5B10(void)
{
    s32 temp_v1;
    Rec2B50 *p;
    u8 *menu;

    menu = g_menuLayoutBuffer;
    temp_v1 = ((Rec29D8 *)(((GroupClassView *)&D_80122C00)->unk0 + menu))->unk29D8;
    p = (Rec2B50 *)((temp_v1 * 0x14C) + menu);
    ((GroupClassView *)&D_80122C00)->unk6 = temp_v1;
    ((GroupClassView *)&D_80122C00)->unk1E = p->unk2B50 & 0xF;
}


extern u8 g_menuLayoutBuffer[];
extern s16 D_80122C10;

/**
 * @brief Selects the menu state associated with the current history slot.
 *
 * Clamps the signed history-slot byte at offset 0x29D7 to 3, then selects
 * state 2 when the low seven bits of the word at 0xAA8 equal 3. Otherwise it
 * selects state 0 for history slot 3 and state 1 for the remaining slots.
 */
void func_800C5B64(void)
{
    if (g_menuLayoutBuffer[0x29D7] >= 4U)
    {
        g_menuLayoutBuffer[0x29D7] = 3;
    }
    if ((*(u32 *)&g_menuLayoutBuffer[0xAA8] & 0x7F) == 3)
    {
        D_80122C10 = 2;
    }
    else if (*(s8 *)&g_menuLayoutBuffer[0x29D7] == 3)
    {
        D_80122C10 = 0;
    }
    else
    {
        D_80122C10 = 1;
    }
}


extern u8 g_menuLayoutBuffer[];
extern s32 D_80122C00;
extern void *func_800A9060(void);
extern void func_800A8F8C(void *, void *);
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
    first_layout = g_menuLayoutBuffer;
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
        group_offset = g_menuLayoutBuffer[D_80122C00 + 0x29D8] * 0x14C;
        layout = g_menuLayoutBuffer;
        entries = layout + 0x2B58;
        offset = i * 0x40;
        if (layout[offset + group_offset + 0x2B58] != 0)
        {
            if (func_800A9060() != 0)
            {
                item = func_800A9060();
                group_offset = layout[D_80122C00 + 0x29D8] * 0x14C;
                func_800A8F8C(item, group_offset + entries + offset);
            }
        }
        i++;
    } while (i < 4);
    final_layout = g_menuLayoutBuffer;
    final_layout[final_layout[D_80122C00 + 0x29D8] * 0x14C + 0x2B0C] = 0;
    final_layout[D_80122C00 + 0x29D8] = 3;
    flags = *(u32 *)(final_layout + 0x29D4);
    if ((flags & 0xF) != 0)
    {
        *(u32 *)(final_layout + 0x29D4) =
            (flags & ~0xF) | (((final_layout[0x29D4] & 0xF) - 1) & 0xF);
    }
}


extern u8 g_menuLayoutBuffer[];
extern s32 D_80122C00;
extern s16 D_80122C10;
extern s32 g_gosub_result_count;

extern void func_800B2844(s32 arg0, u8 *arg1, u8 arg2);

void func_800C5DA8(void)
{
    u8 idx = g_menuLayoutBuffer[D_80122C00 + 0x29D8];

    func_800B2844(0, &g_menuLayoutBuffer[idx * 332 + 0x2B0C], 0xFF);
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
    if (g_menuLayoutBuffer[0x29D8] != *(s8*)&g_menuLayoutBuffer[0x29D7])
    {
        slot_status[0] = (s16)g_menuLayoutBuffer[0x29D8];
    }
    if (g_menuLayoutBuffer[0x29D9] != *(s8*)&g_menuLayoutBuffer[0x29D7])
    {
        slot_status[1] = (s16)g_menuLayoutBuffer[0x29D9];
    }
    if (g_menuLayoutBuffer[0x29DA] != *(s8*)&g_menuLayoutBuffer[0x29D7])
    {
        slot_status[2] = (s16)g_menuLayoutBuffer[0x29DA];
    }
    packed_order = g_menuLayoutBuffer[0x29DB];
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
    layout = g_menuLayoutBuffer;
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
    D_80122C1E = (s16)(s8)D_800459AF;
}
