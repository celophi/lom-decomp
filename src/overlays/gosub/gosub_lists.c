#include "gosub_internal.h"

/**
 * @brief Build screen 15's rows from nonempty inventory slots 0x60-0x84.
 */
void gosub_build_screen_15_item_list(void)
{
    s32 i;
    s32 count;

    count = 0;
    for (i = 0x60; i < 0x85; i++)
    {
        if (g_pad_ctx->item_counts[i] != 0)
        {
            GosubListRow* row = &g_gosub_rows[count];
            row->name = ARCHIVE_ENTRY(g_gosub_text_archive_offsets_1[0], i);
            row->desc = ARCHIVE_ENTRY(g_gosub_text_archive_offsets_0[12], g_gosub_item_metadata[i]);
            row->value = g_pad_ctx->item_counts[i];
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
 */
void gosub_build_screen_19_item_list(void)
{
    s32 i;
    s32 count;

    count = 0;
    for (i = 0x60; i < 0x90; i++)
    {
        if (g_pad_ctx->item_counts[i] != 0)
        {
            GosubListRow* row = &g_gosub_rows[count];
            row->name = ARCHIVE_ENTRY(g_gosub_text_archive_offsets_1[0], i);
            row->desc = ARCHIVE_ENTRY(g_gosub_text_archive_offsets_0[2], i);
            row->value = g_pad_ctx->item_counts[i];
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
 */
void gosub_build_screen_16_item_list(void)
{
    s32 i;
    s32 count;

    count = 0;
    for (i = 0x40; i < 0x50; i++)
    {
        if (g_pad_ctx->item_counts[i] != 0)
        {
            GosubListRow* row = &g_gosub_rows[count];
            row->name = ARCHIVE_ENTRY(g_gosub_text_archive_offsets_1[0], i);
            row->value = g_pad_ctx->item_counts[i];
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
 */
void gosub_build_screen_1_item_list(void)
{
    s32 i;
    s32 count;

    count = 0;
    for (i = 0x40; i < 0xFF; i++)
    {
        if (g_pad_ctx->item_counts[i] != 0)
        {
            GosubListRow* row = &g_gosub_rows[count];
            row->name = ARCHIVE_ENTRY(g_gosub_text_archive_offsets_1[0], i);
            row->desc = ARCHIVE_ENTRY(g_gosub_text_archive_offsets_0[2], i);
            row->value = g_pad_ctx->item_counts[i];
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
 */
void gosub_build_screen_0_item_list(void)
{
    s32 i;
    s32 count;

    count = 0;
    for (i = 0; i < 0x40; i++)
    {
        if (g_pad_ctx->item_counts[i] != 0)
        {
            GosubListRow* row = &g_gosub_rows[count];
            row->name = ARCHIVE_ENTRY(g_gosub_text_archive_offsets_1[0], i);
            row->desc = ARCHIVE_ENTRY(g_gosub_text_archive_offsets_0[2], i);
            row->value = g_pad_ctx->item_counts[i];
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
 * @brief Build the logic-block list for the gosub screen entered by arm 11.
 * @note Bit 2 of the row flag word is cleared only for blocks that have
 *       unknown_bit16 clear and are fully ready; every other block sets it.
 */
void gosub_build_packed_record_list(void)
{
    s32 i;
    u8* row_name;
    u8 number_text[32];

    for (i = 0; i < g_pad_ctx->logic_block_count; i++)
    {
        g_gosub_rows[i].detail_group = g_pad_ctx->logic_blocks[i].f.id;
        g_gosub_rows[i].detail_id = g_pad_ctx->logic_blocks[i].f.quantity;
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
        g_gosub_rows[i].detail_variant = g_pad_ctx->logic_blocks[i].f.shape;
        if (g_pad_ctx->logic_blocks[i].f.unknown_bit16 != 0 ||
            g_pad_ctx->logic_blocks[i].f.logic_type != LOGIC_BLOCK_UNASSIGNED)
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
    g_gosub_row_count = g_pad_ctx->logic_block_count;
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
 */
void gosub_build_roster_list(s32 mode)
{
    s32 large_ref_index;
    s32 stat_index;
    s32 row_count;
    s32 record_index;
    s32 slot;
    u8 unused_name_buf[32]; /* Unused; sets the original 0x20-byte frame. */

    row_count = 0;
    if (mode != 1)
    {
        for (large_ref_index = 0; large_ref_index < LARGE_HISTORY_RECORD_COUNT; large_ref_index++)
        {
            record_index = g_pad_ctx->large_history_order[large_ref_index];
            if (record_index < LARGE_HISTORY_RECORD_COUNT)
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
                if (g_pad_ctx->large_history_index == record_index)
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
                g_gosub_rows[row_count].name = g_pad_ctx->large_history_records[record_index].name;
                g_gosub_rows[row_count].detail_id = g_pad_ctx->large_history_records[record_index].unknown_0x44 & 0xF;
                g_gosub_rows[row_count].detail_variant = g_pad_ctx->large_history_records[record_index].unknown_0x48;
                g_gosub_rows[row_count].primary_value = g_pad_ctx->large_history_records[record_index].primary_value;
                for (stat_index = 0; stat_index < HISTORY_RECORD_STAT_COUNT; stat_index++)
                {
                    g_gosub_rows[row_count].stats[stat_index] = g_pad_ctx->large_history_records[record_index].stats[stat_index];
                }
                g_gosub_rows[row_count].secondary_value = g_pad_ctx->large_history_records[record_index].secondary_value;
                row_count++;
            }
        }
    }
    if (mode != 2)
    {
        for (slot = 0; slot < SMALL_HISTORY_RECORD_COUNT; slot++)
        {
            if (g_pad_ctx->small_history_records[slot].name[0] != 0)
            {
                g_gosub_rows[row_count].index = slot;
                g_gosub_rows[row_count].value = -3;
                g_gosub_rows[row_count].flags.f.alternate_format = 1;
                if (g_pad_ctx->small_history_index == slot)
                {
                    g_gosub_rows[row_count].detail_group = 1;
                }
                else
                {
                    g_gosub_rows[row_count].detail_group = 0;
                }
                g_gosub_rows[row_count].text_color = 4;
                g_gosub_rows[row_count].name = g_pad_ctx->small_history_records[slot].name;
                g_gosub_rows[row_count].detail_id = g_pad_ctx->small_history_records[slot].unknown_0x15;
                g_gosub_rows[row_count].primary_value = g_pad_ctx->small_history_records[slot].primary_value;
                g_gosub_rows[row_count].flags.f.selection_blocked = g_pad_ctx->small_history_records[slot].selection_flags.selection_blocked;
                g_gosub_rows[row_count].flags.f.selection_restricted = g_pad_ctx->small_history_records[slot].selection_flags.selection_restricted;
                g_gosub_rows[row_count].detail_variant = g_pad_ctx->small_history_records[slot].unknown_0x18;
                if (g_gosub_rows[row_count].flags.half & 1)
                {
                    g_gosub_rows[row_count].detail_id = g_pad_ctx->small_history_records[slot].unknown_0x16 + 0x48;
                    if (g_pad_ctx->small_history_records[slot].unknown_0x42 < 6)
                    {
                        g_gosub_rows[row_count].detail_variant = 0;
                    }
                    else if (g_pad_ctx->small_history_records[slot].unknown_0x42 < 0x1F)
                    {
                        g_gosub_rows[row_count].detail_variant = 1;
                    }
                    else
                    {
                        g_gosub_rows[row_count].detail_variant = 2;
                    }
                }
                for (stat_index = 0; stat_index < HISTORY_RECORD_STAT_COUNT; stat_index++)
                {
                    g_gosub_rows[row_count].stats[stat_index] = g_pad_ctx->small_history_records[slot].stats[stat_index];
                }
                g_gosub_rows[row_count].secondary_value = g_pad_ctx->small_history_records[slot].secondary_value;
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
