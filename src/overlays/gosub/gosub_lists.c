#include "gosub_internal.h"

/**
 * @brief List the color materials held, each with its color.
 */
void gosub_build_color_material_list(void)
{
    s32 i;
    s32 count;

    count = 0;
    for (i = GOSUB_COLOR_MATERIAL_FIRST; i < GOSUB_COLOR_MATERIAL_END; i++)
    {
        if (g_pad_ctx->item_counts[i] != 0)
        {
            GosubListRow* row = &g_gosub_rows[count];
            row->name = GOSUB_TEXT(GOSUB_TEXT_ITEM_NAMES, i);
            row->desc = GOSUB_TEXT(GOSUB_TEXT_COLOR_NAMES, g_gosub_item_colors[i]);
            row->value = g_pad_ctx->item_counts[i];
            row->text_color = GOSUB_TEXT_COLOR_NORMAL;
            row->index = i;
            count++;
        }
    }
    g_gosub_row_count = count;
    g_gosub_visible_row_count = GOSUB_ITEM_VISIBLE_ROWS;
    g_gosub_row_height = GOSUB_ITEM_ROW_HEIGHT;
    g_gosub_window_width = GOSUB_LIST_PANEL_WIDTH;
    g_gosub_window_height = GOSUB_ITEM_WINDOW_HEIGHT;
    g_gosub_title_text = GOSUB_MESSAGE(GOSUB_MSG_COLOR_MATERIAL);
}

/**
 * @brief List the produce held (fruit, vegetables and meat), with descriptions.
 */
void gosub_build_produce_list(void)
{
    s32 i;
    s32 count;

    count = 0;
    for (i = GOSUB_PRODUCE_FIRST; i < GOSUB_PRODUCE_END; i++)
    {
        if (g_pad_ctx->item_counts[i] != 0)
        {
            GosubListRow* row = &g_gosub_rows[count];
            row->name = GOSUB_TEXT(GOSUB_TEXT_ITEM_NAMES, i);
            row->desc = GOSUB_TEXT(GOSUB_TEXT_ITEM_DESCRIPTIONS, i);
            row->value = g_pad_ctx->item_counts[i];
            row->text_color = GOSUB_TEXT_COLOR_NORMAL;
            row->index = i;
            count++;
        }
    }
    g_gosub_row_count = count;
    g_gosub_visible_row_count = GOSUB_ITEM_VISIBLE_ROWS;
    g_gosub_row_height = GOSUB_ITEM_ROW_HEIGHT;
    g_gosub_window_width = GOSUB_LIST_PANEL_WIDTH;
    g_gosub_window_height = GOSUB_ITEM_WINDOW_HEIGHT;
    g_gosub_title_text = GOSUB_MESSAGE(GOSUB_MSG_CHOOSE_PRODUCE);
}

/**
 * @brief List the elemental coins held.
 */
void gosub_build_elemental_coin_list(void)
{
    s32 i;
    s32 count;

    count = 0;
    for (i = GOSUB_ELEMENTAL_COIN_FIRST; i < GOSUB_ELEMENTAL_COIN_END; i++)
    {
        if (g_pad_ctx->item_counts[i] != 0)
        {
            GosubListRow* row = &g_gosub_rows[count];
            row->name = GOSUB_TEXT(GOSUB_TEXT_ITEM_NAMES, i);
            row->value = g_pad_ctx->item_counts[i];
            row->text_color = GOSUB_TEXT_COLOR_NORMAL;
            row->index = i;
            count++;
        }
    }
    g_gosub_row_count = count;
    g_gosub_visible_row_count = GOSUB_ITEM_VISIBLE_ROWS;
    g_gosub_row_height = GOSUB_ITEM_ROW_HEIGHT;
    g_gosub_window_width = GOSUB_LIST_PANEL_WIDTH;
    g_gosub_window_height = GOSUB_ITEM_WINDOW_HEIGHT;
    g_gosub_title_text = GOSUB_MESSAGE(GOSUB_MSG_CHOOSE_ELEMENTAL_COIN);
}

/**
 * @brief List the secondary materials held: every item kind from GOSUB_SECONDARY_MATERIAL_FIRST up.
 */
void gosub_build_secondary_material_list(void)
{
    s32 i;
    s32 count;

    count = 0;
    for (i = GOSUB_SECONDARY_MATERIAL_FIRST; i < GOSUB_SECONDARY_MATERIAL_END; i++)
    {
        if (g_pad_ctx->item_counts[i] != 0)
        {
            GosubListRow* row = &g_gosub_rows[count];
            row->name = GOSUB_TEXT(GOSUB_TEXT_ITEM_NAMES, i);
            row->desc = GOSUB_TEXT(GOSUB_TEXT_ITEM_DESCRIPTIONS, i);
            row->value = g_pad_ctx->item_counts[i];
            row->text_color = GOSUB_TEXT_COLOR_NORMAL;
            row->index = i;
            count++;
        }
    }
    g_gosub_row_count = count;
    g_gosub_visible_row_count = GOSUB_ITEM_VISIBLE_ROWS;
    g_gosub_row_height = GOSUB_ITEM_ROW_HEIGHT;
    g_gosub_window_width = GOSUB_LIST_PANEL_WIDTH;
    g_gosub_window_height = GOSUB_ITEM_WINDOW_HEIGHT;
    g_gosub_title_text = GOSUB_MESSAGE(GOSUB_MSG_CHOOSE_SECONDARY_MATERIAL);
}

/**
 * @brief List the primary materials held (metals, woods, hides and so on).
 */
void gosub_build_primary_material_list(void)
{
    s32 i;
    s32 count;

    count = 0;
    for (i = GOSUB_PRIMARY_MATERIAL_FIRST; i < GOSUB_PRIMARY_MATERIAL_END; i++)
    {
        if (g_pad_ctx->item_counts[i] != 0)
        {
            GosubListRow* row = &g_gosub_rows[count];
            row->name = GOSUB_TEXT(GOSUB_TEXT_ITEM_NAMES, i);
            row->desc = GOSUB_TEXT(GOSUB_TEXT_ITEM_DESCRIPTIONS, i);
            row->value = g_pad_ctx->item_counts[i];
            row->text_color = GOSUB_TEXT_COLOR_NORMAL;
            row->index = i;
            count++;
        }
    }
    g_gosub_row_count = count;
    g_gosub_visible_row_count = GOSUB_ITEM_VISIBLE_ROWS;
    g_gosub_row_height = GOSUB_ITEM_ROW_HEIGHT;
    g_gosub_window_width = GOSUB_LIST_PANEL_WIDTH;
    g_gosub_window_height = GOSUB_ITEM_WINDOW_HEIGHT;
    g_gosub_title_text = GOSUB_MESSAGE(GOSUB_MSG_CHOOSE_PRIMARY_MATERIAL);
}

/**
 * @brief Build the logic-block list: name and level, description, and shape of every block.
 * @note A block counts as in use while it is placed on a golem's grid or owned by a golem.
 */
void gosub_build_logic_block_list(void)
{
    s32 i;
    u8* row_name;
    u8 number_text[32];

    for (i = 0; i < g_pad_ctx->logic_block_count; i++)
    {
        g_gosub_rows[i].detail_group = g_pad_ctx->logic_blocks[i].f.id;
        g_gosub_rows[i].detail_id = g_pad_ctx->logic_blocks[i].f.quantity;
        row_name = g_gosub_text_buffers[i];
        gosub_copy_encoded_string(row_name, GOSUB_TEXT(GOSUB_TEXT_LOGIC_BLOCK_NAMES, g_gosub_rows[i].detail_group));
        if (g_gosub_rows[i].detail_id != 0)
        {
            gosub_append_encoded_string(row_name, FIELD_UI_TEXT_AT(D_800EC3DA, FIELD_UI_TEXT_PLUS));
            field_format_number(number_text, g_gosub_rows[i].detail_id, 1);
            gosub_append_encoded_string(row_name, number_text);
        }
        g_gosub_rows[i].name = row_name;
        g_gosub_rows[i].desc = GOSUB_TEXT(GOSUB_TEXT_LOGIC_BLOCK_DESCRIPTIONS, g_gosub_rows[i].detail_group);
        g_gosub_rows[i].value = GOSUB_ROW_LOGIC_BLOCK;
        g_gosub_rows[i].detail_variant = g_pad_ctx->logic_blocks[i].f.shape;
        if (g_pad_ctx->logic_blocks[i].f.placed != 0 || g_pad_ctx->logic_blocks[i].f.logic_type != LOGIC_BLOCK_UNASSIGNED)
        {
            g_gosub_rows[i].flags.block.in_use = 1;
        }
        else
        {
            g_gosub_rows[i].flags.block.in_use = 0;
        }
        g_gosub_rows[i].index = i;
        g_gosub_rows[i].text_color = GOSUB_TEXT_COLOR_NORMAL;
    }
    g_gosub_row_count = g_pad_ctx->logic_block_count;
    g_gosub_visible_row_count = 4;
    g_gosub_row_height = 32;
    g_gosub_window_width = GOSUB_LOGIC_BLOCK_PANEL_WIDTH;
    g_gosub_window_height = 4 * 32 + GOSUB_LIST_PANEL_PADDING;
    g_gosub_title_text = GOSUB_MESSAGE(GOSUB_MSG_GOLEM_LOGIC_BLOCKS);
}

/**
 * @brief Build the golem and pet list: golems in their display order, then the pets.
 * @param mode GosubCompanionFilter; also selects the title message.
 * @note In the mixed list a golem row's index is its record index ANDed with
 *       0x80, which is always 0; the flag was probably meant to be ORed in.
 */
void gosub_build_companion_list(s32 mode)
{
    s32 order_index;
    s32 stat_index;
    s32 row_count;
    s32 record_index;
    s32 slot;
    u8 unused[32]; /* never used, but the original stack frame has room for it */

    row_count = 0;
    if (mode != GOSUB_COMPANIONS_PETS)
    {
        for (order_index = 0; order_index < GOLEM_RECORD_COUNT; order_index++)
        {
            record_index = g_pad_ctx->golem_order[order_index];
            if (record_index < GOLEM_RECORD_COUNT)
            {
                if (mode == GOSUB_COMPANIONS_ALL)
                {
                    g_gosub_rows[row_count].index = record_index & 0x80;
                }
                else
                {
                    g_gosub_rows[row_count].index = record_index;
                }
                g_gosub_rows[row_count].value = GOSUB_ROW_COMPANION;
                g_gosub_rows[row_count].flags.companion.pet = 0;
                if (g_pad_ctx->joined_golem == record_index)
                {
                    g_gosub_rows[row_count].detail_group = 1;
                }
                else
                {
                    g_gosub_rows[row_count].detail_group = 0;
                }
                g_gosub_rows[row_count].flags.companion.egg = 0;
                g_gosub_rows[row_count].flags.companion.grazing = 0;
                g_gosub_rows[row_count].text_color = GOSUB_TEXT_COLOR_NORMAL;
                g_gosub_rows[row_count].name = g_pad_ctx->golem_records[record_index].name;
                /* The golem's type and palette. */
                g_gosub_rows[row_count].detail_id = g_pad_ctx->golem_records[record_index].logic_layout & GOLEM_LOGIC_CLASS_MASK;
                g_gosub_rows[row_count].detail_variant = g_pad_ctx->golem_records[record_index].palette;
                g_gosub_rows[row_count].primary_value = g_pad_ctx->golem_records[record_index].primary_value;
                for (stat_index = 0; stat_index < COMPANION_STAT_COUNT; stat_index++)
                {
                    g_gosub_rows[row_count].stats[stat_index] = g_pad_ctx->golem_records[record_index].stats[stat_index];
                }
                g_gosub_rows[row_count].secondary_value = g_pad_ctx->golem_records[record_index].secondary_value;
                row_count++;
            }
        }
    }
    if (mode != GOSUB_COMPANIONS_GOLEMS)
    {
        for (slot = 0; slot < PET_RECORD_COUNT; slot++)
        {
            if (g_pad_ctx->pet_records[slot].name[0] != 0)
            {
                g_gosub_rows[row_count].index = slot;
                g_gosub_rows[row_count].value = GOSUB_ROW_COMPANION;
                g_gosub_rows[row_count].flags.companion.pet = 1;
                if (g_pad_ctx->joined_pet == slot)
                {
                    g_gosub_rows[row_count].detail_group = 1;
                }
                else
                {
                    g_gosub_rows[row_count].detail_group = 0;
                }
                g_gosub_rows[row_count].text_color = GOSUB_TEXT_COLOR_NORMAL;
                g_gosub_rows[row_count].name = g_pad_ctx->pet_records[slot].name;
                /* The pet's species and level. */
                g_gosub_rows[row_count].detail_id = g_pad_ctx->pet_records[slot].species;
                g_gosub_rows[row_count].primary_value = g_pad_ctx->pet_records[slot].primary_value;
                g_gosub_rows[row_count].flags.companion.egg = g_pad_ctx->pet_records[slot].status.egg;
                g_gosub_rows[row_count].flags.companion.grazing = g_pad_ctx->pet_records[slot].status.grazing;
                g_gosub_rows[row_count].detail_variant = g_pad_ctx->pet_records[slot].level;
                if (g_gosub_rows[row_count].flags.half & 1)
                {
                    /* An egg shows its egg portrait and how long it still needs to hatch. */
                    g_gosub_rows[row_count].detail_id = g_pad_ctx->pet_records[slot].egg_species + GOSUB_EGG_PORTRAIT_FIRST;
                    if (g_pad_ctx->pet_records[slot].hatch_counter < GOSUB_EGG_ANY_TIME_BELOW)
                    {
                        g_gosub_rows[row_count].detail_variant = GOSUB_EGG_HATCH_ANY_TIME;
                    }
                    else if (g_pad_ctx->pet_records[slot].hatch_counter < GOSUB_EGG_ALMOST_READY_BELOW)
                    {
                        g_gosub_rows[row_count].detail_variant = GOSUB_EGG_HATCH_ALMOST_READY;
                    }
                    else
                    {
                        g_gosub_rows[row_count].detail_variant = GOSUB_EGG_HATCH_NEEDS_TIME;
                    }
                }
                for (stat_index = 0; stat_index < COMPANION_STAT_COUNT; stat_index++)
                {
                    g_gosub_rows[row_count].stats[stat_index] = g_pad_ctx->pet_records[slot].stats[stat_index];
                }
                g_gosub_rows[row_count].secondary_value = g_pad_ctx->pet_records[slot].secondary_value;
                row_count++;
            }
        }
    }
    g_gosub_row_count = row_count;
    g_gosub_visible_row_count = 3;
    g_gosub_row_height = 48;
    g_gosub_window_width = GOSUB_LOGIC_BLOCK_PANEL_WIDTH;
    g_gosub_window_height = 3 * 48 + GOSUB_LIST_PANEL_PADDING;
    g_gosub_title_text = GOSUB_MESSAGE(GOSUB_MSG_CHOOSE_PET_OR_GOLEM + mode);
}
