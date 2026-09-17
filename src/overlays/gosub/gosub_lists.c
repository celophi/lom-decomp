#include "gosub_internal.h"

/**
 * @brief Build screen 15's rows from nonempty inventory slots 0x60-0x84.
 * @see decomp.me
 */
void gosub_build_screen_15_item_list(void)
{
    s32 i;
    s32 count;

    count = 0;
    for (i = 0x60; i < 0x85; i++)
    {
        if (*(g_pad_ctx + i + 0x25E0) != 0)
        {
            GosubListRow* row = &g_gosub_rows[count];
            row->name = ARCHIVE_ENTRY(g_gosub_text_archive_offsets_1[0], i);
            row->desc = ARCHIVE_ENTRY(g_gosub_text_archive_offsets_0[12], g_gosub_item_metadata[i]);
            row->value = *(g_pad_ctx + i + 0x25E0);
            row->text_color = 4;
            row->index = i;
            count++;
        }
    }
    g_gosub_row_count = count;
    g_gosub_visible_row_count = 8;
    g_gosub_row_height = 0x10;
    g_gosub_window_width = 0xE8;
    g_gosub_window_height = 0x84;
    g_gosub_title_text = GOSUB_MSG_PTR(0x3A);
}

/**
 * @brief Build screen 19's rows from nonempty inventory slots 0x60-0x8F.
 * @see decomp.me
 */
void gosub_build_screen_19_item_list(void)
{
    s32 i;
    s32 count;

    count = 0;
    for (i = 0x60; i < 0x90; i++)
    {
        if (*(g_pad_ctx + i + 0x25E0) != 0)
        {
            GosubListRow* row = &g_gosub_rows[count];
            row->name = ARCHIVE_ENTRY(g_gosub_text_archive_offsets_1[0], i);
            row->desc = ARCHIVE_ENTRY(g_gosub_text_archive_offsets_0[2], i);
            row->value = *(g_pad_ctx + i + 0x25E0);
            row->text_color = 4;
            row->index = i;
            count++;
        }
    }
    g_gosub_row_count = count;
    g_gosub_visible_row_count = 8;
    g_gosub_row_height = 0x10;
    g_gosub_window_width = 0xE8;
    g_gosub_window_height = 0x84;
    g_gosub_title_text = GOSUB_MSG_PTR(0x4A);
}

/**
 * @brief Build screen 16's rows from nonempty inventory slots 0x40-0x4F.
 * @see decomp.me
 */
void gosub_build_screen_16_item_list(void)
{
    s32 i;
    s32 count;

    count = 0;
    for (i = 0x40; i < 0x50; i++)
    {
        if (*(g_pad_ctx + i + 0x25E0) != 0)
        {
            GosubListRow* row = &g_gosub_rows[count];
            row->name = ARCHIVE_ENTRY(g_gosub_text_archive_offsets_1[0], i);
            row->value = *(g_pad_ctx + i + 0x25E0);
            row->text_color = 4;
            row->index = i;
            count++;
        }
    }
    g_gosub_row_count = count;
    g_gosub_visible_row_count = 8;
    g_gosub_row_height = 0x10;
    g_gosub_window_width = 0xE8;
    g_gosub_window_height = 0x84;
    g_gosub_title_text = GOSUB_MSG_PTR(0x38);
}

/**
 * @brief Build screen 1's rows from nonempty inventory slots 0x40-0xFE.
 * @see decomp.me
 */
void gosub_build_screen_1_item_list(void)
{
    s32 i;
    s32 count;

    count = 0;
    for (i = 0x40; i < 0xFF; i++)
    {
        if (*(g_pad_ctx + i + 0x25E0) != 0)
        {
            GosubListRow* row = &g_gosub_rows[count];
            row->name = ARCHIVE_ENTRY(g_gosub_text_archive_offsets_1[0], i);
            row->desc = ARCHIVE_ENTRY(g_gosub_text_archive_offsets_0[2], i);
            row->value = *(g_pad_ctx + i + 0x25E0);
            row->text_color = 4;
            row->index = i;
            count++;
        }
    }
    g_gosub_row_count = count;
    g_gosub_visible_row_count = 8;
    g_gosub_row_height = 0x10;
    g_gosub_window_width = 0xE8;
    g_gosub_window_height = 0x84;
    g_gosub_title_text = GOSUB_MSG_PTR(0x14);
}

/**
 * @brief Build screen 0's rows from nonempty inventory slots 0x00-0x3F.
 * @see decomp.me
 */
void gosub_build_screen_0_item_list(void)
{
    s32 i;
    s32 count;

    count = 0;
    for (i = 0; i < 0x40; i++)
    {
        if (*(g_pad_ctx + i + 0x25E0) != 0)
        {
            GosubListRow* row = &g_gosub_rows[count];
            row->name = ARCHIVE_ENTRY(g_gosub_text_archive_offsets_1[0], i);
            row->desc = ARCHIVE_ENTRY(g_gosub_text_archive_offsets_0[2], i);
            row->value = *(g_pad_ctx + i + 0x25E0);
            row->text_color = 4;
            row->index = i;
            count++;
        }
    }
    g_gosub_row_count = count;
    g_gosub_visible_row_count = 8;
    g_gosub_row_height = 0x10;
    g_gosub_window_width = 0xE8;
    g_gosub_window_height = 0x84;
    g_gosub_title_text = GOSUB_MSG_PTR(0x12);
}

/**
 * @brief Build the packed-record list for the gosub screen entered by arm 11.
 * @note Bit 2 of the flag word is cleared only for records that are both unflagged at
 *       bit 16 and have both low bits set; every other record sets it.
 * @see decomp.me
 */
void gosub_build_packed_record_list(void)
{
    s32 i;
    u8* row_name;
    u8 number_text[32];

    for (i = 0; i < *(g_pad_ctx + 0x29D6); i++)
    {
        g_gosub_rows[i].detail_group = *(g_pad_ctx + (i << 2) + 0x29DC) >> 2;
        g_gosub_rows[i].detail_id = (*(u32*)(g_pad_ctx + (i << 2) + 0x29DC) >> 8) & 0xF;
        row_name = g_gosub_text_buffers + i * 0x50;
        gosub_copy_encoded_string(row_name, ARCHIVE_ENTRY(g_gosub_text_archive_offsets_3[0], g_gosub_rows[i].detail_group));
        if (g_gosub_rows[i].detail_id != 0)
        {
            gosub_append_encoded_string(row_name, D_800EC3DA - 0x16 + D_800EC3DA[0] + (D_800EC3DA[1] << 8));
            func_800A8B90(number_text, g_gosub_rows[i].detail_id, 1);
            gosub_append_encoded_string(row_name, number_text);
        }
        g_gosub_rows[i].name = row_name;
        g_gosub_rows[i].desc = ARCHIVE_ENTRY(g_gosub_text_archive_offsets_0[7], g_gosub_rows[i].detail_group);
        g_gosub_rows[i].value = -2;
        g_gosub_rows[i].detail_variant = (*(u32*)(g_pad_ctx + (i << 2) + 0x29DC) >> 12) & 0xF;
        if (((*(u32*)(g_pad_ctx + (i << 2) + 0x29DC) >> 16) & 1) != 0 || (*(u32*)(g_pad_ctx + (i << 2) + 0x29DC) & 3) != 3)
        {
            g_gosub_rows[i].flags.word |= 4;
        }
        else
        {
            g_gosub_rows[i].flags.word &= ~4;
        }
        g_gosub_rows[i].index = i;
        g_gosub_rows[i].text_color = 4;
    }
    g_gosub_row_count = *(g_pad_ctx + 0x29D6);
    g_gosub_visible_row_count = 4;
    g_gosub_row_height = 0x20;
    g_gosub_window_width = 0x120;
    g_gosub_window_height = 0x84;
    g_gosub_title_text = GOSUB_MSG_PTR(0x18);
}

/**
 * @brief Build a roster list for gosub screens 12-14 and 17-18.
 * @param mode Which blocks to emit: 1 = second only, 2 = first only, otherwise
 *             both. Also picks the screen's title message.
 * @see decomp.me
 */
void gosub_build_roster_list(s32 mode)
{
    s32 large_ref_index;
    s32 stat_index;
    s32 row_count;
    s32 record_index;
    s32 record_offset;
    s32 slot;
    s32 record_offset_reload;
    u8 unused_name_buf[32];

    row_count = 0;
    if (mode != 1)
    {
        for (large_ref_index = 0; large_ref_index < 3; large_ref_index++)
        {
            record_index = *(g_pad_ctx + large_ref_index + 0x29D8);
            if (record_index < 3)
            {
                if (mode == 0)
                {
                    g_gosub_rows[row_count].index = record_index & 0x80;
                }
                else
                {
                    g_gosub_rows[row_count].index = record_index;
                }
                g_gosub_rows[row_count].value = -3;
                g_gosub_rows[row_count].flags.f.alternate_format = 0;
                if (*(s8*)(g_pad_ctx + 0x29D7) == record_index)
                {
                    g_gosub_rows[row_count].detail_group = 1;
                }
                else
                {
                    g_gosub_rows[row_count].detail_group = 0;
                }
                g_gosub_rows[row_count].flags.f.selection_blocked = 0;
                g_gosub_rows[row_count].flags.f.selection_restricted = 0;
                g_gosub_rows[row_count].text_color = 4;
                record_offset = record_index * 332;
                g_gosub_rows[row_count].name = g_pad_ctx + 0x2B0C + record_offset;
                g_gosub_rows[row_count].detail_id = *(g_pad_ctx + record_offset + 0x2B50) & 0xF;
                g_gosub_rows[row_count].detail_variant = *(g_pad_ctx + record_offset + 0x2B54);
                g_gosub_rows[row_count].primary_value = *(u16*)(g_pad_ctx + record_offset + 0x2B24);
                for (stat_index = 0; stat_index < 4; stat_index++)
                {
                    g_gosub_rows[row_count].stats[stat_index] = *(u16*)(g_pad_ctx + record_offset + 0x2B26 + stat_index * 2);
                }
                record_offset_reload = record_index * 332;
                g_gosub_rows[row_count].secondary_value = *(u16*)(g_pad_ctx + record_offset_reload + 0x2B22);
                row_count++;
            }
        }
    }
    if (mode != 2)
    {
        for (slot = 0; slot < 5; slot++)
        {
            record_offset = slot * 0x60;
            if (*(g_pad_ctx + record_offset + 0x2EF4) != 0)
            {
                g_gosub_rows[row_count].index = slot;
                g_gosub_rows[row_count].value = -3;
                g_gosub_rows[row_count].flags.f.alternate_format = 1;
                if (*(s32*)(g_pad_ctx + 0x2EF0) == slot)
                {
                    g_gosub_rows[row_count].detail_group = 1;
                }
                else
                {
                    g_gosub_rows[row_count].detail_group = 0;
                }
                g_gosub_rows[row_count].text_color = 4;
                g_gosub_rows[row_count].name = g_pad_ctx + 0x2EF4 + slot * 0x60;
                g_gosub_rows[row_count].detail_id = *(g_pad_ctx + record_offset + 0x2F09);
                g_gosub_rows[row_count].primary_value = *(u16*)(g_pad_ctx + record_offset + 0x2F12);
                g_gosub_rows[row_count].flags.f.selection_blocked = *(u32*)(g_pad_ctx + record_offset + 0x2F38) >> 31;
                g_gosub_rows[row_count].flags.f.selection_restricted = (*(u32*)(g_pad_ctx + record_offset + 0x2F38) >> 30) & 1;
                g_gosub_rows[row_count].detail_variant = *(g_pad_ctx + record_offset + 0x2F0C);
                if (g_gosub_rows[row_count].flags.half & 1)
                {
                    g_gosub_rows[row_count].detail_id = *(g_pad_ctx + record_offset + 0x2F0A) + 0x48;
                    if (*(u16*)(g_pad_ctx + record_offset + 0x2F36) < 6)
                    {
                        g_gosub_rows[row_count].detail_variant = 0;
                    }
                    else if (*(u16*)(g_pad_ctx + record_offset + 0x2F36) < 0x1F)
                    {
                        g_gosub_rows[row_count].detail_variant = 1;
                    }
                    else
                    {
                        g_gosub_rows[row_count].detail_variant = 2;
                    }
                }
                for (stat_index = 0; stat_index < 4; stat_index++)
                {
                    g_gosub_rows[row_count].stats[stat_index] = *(u16*)(g_pad_ctx + record_offset + 0x2F14 + stat_index * 2);
                }
                record_offset_reload = slot * 0x60;
                g_gosub_rows[row_count].secondary_value = *(u16*)(g_pad_ctx + record_offset_reload + 0x2F10);
                row_count++;
            }
        }
    }
    g_gosub_row_count = row_count;
    g_gosub_visible_row_count = 3;
    g_gosub_row_height = 0x30;
    g_gosub_window_width = 0x120;
    g_gosub_window_height = 0x94;
    g_gosub_title_text = GOSUB_MSG_PTR(mode * 2 + 0x2C);
}
