#include "common.h"

/** @brief Layout view exposing group flags and menu entry bytes. */
typedef struct
{
    u8 pad0[0x26E4];
    u32 unk26E4;
    u8 pad26E8[4];
    u8 unk26EC;
    u8 pad26ED[7];
    u8 unk26F4;
} Layout;
/** @brief Remaining menu item mask and nonzero item count. */
typedef struct
{
    s16 mask;
    s8 count;
} Availability;
extern u8 g_menuLayoutBuffer[], D_80122C0B[], D_800F0E98[];
extern Availability D_80122C08;
extern u8 D_80122C1F;
extern void func_800B2844(s32, void *, s32, s32);
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
    Layout *group_view;

    occupied_count = 0;
    slot_index = occupied_count;
    layout_base = g_menuLayoutBuffer;
    group = D_80122C1F;
    initial_offset = group * 0x8C;
    group_flags = ((Layout *)(layout_base + initial_offset))->unk26E4;
    var_a0 = initial_offset;
    slot_limit = (group_flags >> 8) & 0xF;
    capacity = group_flags & 0xF;
    do
    {
        if (((Layout *)(initial_offset + slot_index * 0x10 + layout_base))->unk26F4 != 0xFF)
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
        if (((Layout *)(var_v0 + second_base))->unk26EC != 0xFF)
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
    group_view = (Layout *)(layout_base + group_offset);
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
    ((Layout *)(layout_base + slot_index + group_offset))->unk26EC = item_id;
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
    D_80122C08.mask = available_mask;
    D_80122C08.count = available_count;
}
