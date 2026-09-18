#include "common.h"
#include "sdk/strings.h"

/** @brief Controller sample with button bits and signed directional axes. */
typedef struct
{
    u8 status;
    u8 pad01;
    u16 buttons;
    u8 pad04[8];
    s16 axis_x;
    s16 axis_y;
    u8 pad10[0xAE - 0x10];
} ControllerState;

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
} UnkStruct80117ED0;

/* Shared extern data declared with a consistent type by every member. */
extern s32 D_80117E88[];
extern s32 D_80117EB8[];
extern u8 D_800EC2F4[];
extern u8 D_800EC2FC[];
extern u8 D_800EC304[];
extern u8 D_800EC334[];
extern u8 *g_pad_ctx;
extern UnkStruct80117ED0 D_80117ED0;
extern s32 D_80117EC0;
extern s32 D_80117EC4;
extern u8 D_80117EC8[];

/*
 * D_80117E90 is read as s16 by func_800A255C and u16 by func_800A2594; D_80117E98
 * is viewed as u8[][0x10] by func_800A2958/func_800A2990 and as flat u8[] by
 * func_800A2594/func_800A29F8. Those conflicting views stay block scope inside
 * each user with that function's original type.
 */

/* Forward prototypes for members defined later in the file. */
void func_800A2958(s32);
void func_800A2DD8(s32);

/**
 * @brief Reset a player's direction/button history counters and idle timers.
 * @note Clears the two-entry count, resets the idle counters to 0x10, and zeroes
 * the direction snapshots for both tracked players.
 */
void func_800A255C(void)
{
    extern s16 D_80117E90[];

    D_80117E88[1] = 0;
    D_80117E88[0] = 0;
    D_80117EB8[1] = 0x10;
    D_80117EB8[0] = 0x10;
    D_80117E90[1] = 0;
    D_80117E90[0] = 0;
}

/**
 * @brief Append direction and newly pressed button transitions to a player's input history.
 * @param player Controller index; indices at or above two are ignored.
 * @param age_sequence Nonzero to advance the inactivity counter and expire an idle sequence.
 * @note Button mappings come from the shared player context. Full histories are shifted by
 * func_800A2958; a sequence expires when its inactivity counter reaches fifteen.
 */
void func_800A2594(s32 player, s32 age_sequence)
{
    extern u16 D_80117E90[];
    extern u8 D_80117E98[];
    s16 axis_x;
    s16 axis_y;
    s32 work_value;
    s32 *direction_count;
    s32 *button_count;
    s32 *idle_counter;
    s32 idle_frames;
    s32 player_word_offset;
    s32 address_or_index;
    s32 pressed_bit;
    s32 button_length;
    s32 buttons;
    s32 button_mask;
    u16 raw_buttons;
    u32 previous_direction;
    u32 current_buttons;
    u32 button_index;
    u32 direction_nibble;
    u8 *mapping;
    u8 *history;
    u16 *previous_buttons;
    ControllerState *pads = (ControllerState *)0x801ED600;
    ControllerState *horizontal_pad;
    ControllerState *vertical_pad;

    if (player < 2)
    {
        address_or_index = player;
        if (pads[address_or_index].status >= 0xFEU)
        {
            buttons = 0;
        }
        else
        {
            raw_buttons = pads[address_or_index].buttons;
            buttons = (raw_buttons << 8) | (raw_buttons >> 8);
        }
        work_value = ((u32) (buttons & 0x40) >> 1) | ((buttons & 0x20) * 2) | ((u32) (buttons & 0x80) >> 3) | ((buttons & 0x10) * 8) | (buttons & 0xFF0F);
        buttons = work_value;
        horizontal_pad = pads + player;
        if (horizontal_pad->status != 0)
        {
            axis_x = horizontal_pad->axis_x;
            if (axis_x < 0)
            {
                buttons = work_value | 0x8000;
            }
            else if (axis_x > 0)
            {
                buttons = work_value | 0x2000;
            }
            vertical_pad = pads + player;
            axis_y = vertical_pad->axis_y;
            if (axis_y < 0)
            {
                buttons |= 0x1000;
            }
            else if (axis_y > 0)
            {
                buttons |= 0x4000;
            }
        }
        previous_direction = D_80117E90[player] & 0xF000;
        button_index = 0;
        if ((previous_direction != (buttons & 0xF000)) && (previous_direction != 0))
        {
            u8 *history_row;
            s32 history_row_offset;
            s32 count_table_address;
            s32 count_entry_offset;
            s32 *history_count;
            s32 history_length;
            s32 history_table_address;
            history_table_address = (s32)D_80117E98;
            history_row = (u8 *)history_table_address;
            history_row_offset = player * 0x10;
            count_table_address = (s32)D_80117E88;
            count_entry_offset = player * 4;
            history_count = (s32 *)(count_entry_offset + count_table_address);
            history_row += history_row_offset;
            history_row[*history_count] = (s8)(previous_direction >> 0xC);
            history_length = *history_count;
            if (history_length < 0xF)
            {
                *history_count = history_length + 1;
            }
            else
            {
                func_800A2958(player);
            }
            D_80117EB8[player] = 0;
            button_index = 0;
        }
        button_mask = 1;
        address_or_index = (s32)D_80117E90;
        work_value = player * 2;
        previous_buttons = (u16 *)(work_value + address_or_index);
        address_or_index = (s32)D_80117E98;
        work_value = player * 0x10;
        history = (u8 *)(work_value + address_or_index);
        player_word_offset = player * 4;
        current_buttons = buttons & 0xFFFF;
        direction_nibble = current_buttons >> 12;
button_loop:
        {
            pressed_bit = current_buttons & button_mask;
            if ((pressed_bit != 0) && ((*previous_buttons & button_mask) != pressed_bit))
            {
                if (buttons & 0xF000)
                {
                    direction_count = (s32 *)((s32)D_80117E88 + player_word_offset);
                    history[*direction_count] = (s8)direction_nibble;
                    work_value = *direction_count;
                    if (work_value < 0xF)
                    {
                        *direction_count = work_value + 1;
                    }
                    else
                    {
                        func_800A2958(player);
                    }
                }
                do
                {
                    button_count = (s32 *)((s32)D_80117E88 + player_word_offset);
                    mapping = (u8 *)((s32)g_pad_ctx + player * 0x250 + D_800EC2FC[button_index]);
                    history[*button_count] = D_800EC2F4[mapping[0x638]];
                    button_length = *button_count;
                    if (button_length < 0xF)
                    {
                        *button_count = button_length + 1;
                    }
                    else
                    {
                        func_800A2958(player);
                    }
                    *(s32 *)((s32)D_80117EB8 + player_word_offset) = 0;
                } while (0);
            }
            do
            {
                button_index += 1;
            } while (0);
            do
            {
                do { button_mask *= 2; } while (0);
            } while (0);
        }
        if (button_index < 8U)
        {
            goto button_loop;
        }
        D_80117E90[player] = buttons;
        if (age_sequence != 0)
        {
            address_or_index = (s32)D_80117EB8;
            work_value = player * 4;
            idle_counter = (s32 *)(work_value + address_or_index);
            idle_frames = *idle_counter;
            if (idle_frames < 0x10)
            {
                *idle_counter = idle_frames + 1;
            }
            if (*idle_counter == 0xF)
            {
                func_800A2DD8(player);
            }
        }
    }
}

/**
 * @brief Shift a player's row of the byte history table down by one entry.
 * @param arg0 Row index into D_80117E98.
 */
void func_800A2958(s32 arg0)
{
    extern u8 D_80117E98[][0x10];
    s32 i;

    for (i = 0; i < 0xF; i++)
    {
        D_80117E98[arg0][i] = D_80117E98[arg0][i + 1];
    }
}

/**
 * @brief Compacts a row's byte table, dropping the first @p start entries.
 *
 * When @p start is below 0x10, shifts @c D_80117E98[row] entries [start, 0x10)
 * down to the front of the row, then decrements the row's count in
 * @c D_80117E88 by @p start.
 *
 * @param row Row index into @c D_80117E98 / @c D_80117E88.
 * @param start Number of leading entries to drop (also subtracted from the
 *              count).
 */
void func_800A2990(s32 row, s32 start)
{
    extern u8 D_80117E98[][0x10];
    u8 (*table)[0x10];
    u8 *base;
    s32 i;
    s32 j;

    j = 0;
    if (start < 0x10)
    {
        table = D_80117E98;
        base = table[row];
        i = start;
        do
        {
            base[j++] = base[i++];
        } while (i < 0x10);
    }

    D_80117E88[row] -= start;
}

/**
 * @brief Find a command in a player's buffered direction and button history.
 * @param player Input history index; indices at or above two yield no command.
 * @param unused Unused caller argument.
 * @param peek Nonzero to inspect the history without consuming it.
 * @return Command identifier, or 0xFF when no complete command was found.
 */
s32 func_800A29F8(s32 player, s32 unused, s32 peek)
{
    extern u8 D_80117E98[][0x10];
    s32 saved_history_offset;
    s32 counts_address;
    s32 consume_count;
    s32 consume_count_50;
    s32 consume_count_60;
    s32 history_offset;
    s32 pattern_length;
    s32 match_value;
    s32 source_index;
    s32 source_index_50;
    s32 source_index_60;
    s32 dest_index;
    s32 dest_index_50;
    s32 dest_index_60;
    s32 match_start;
    s32 direction_end;
    s32 pattern_offset;
    s32 pattern_retry_offset;
    s32 scan_index;
    s32 pattern_index;
    s32 count_offset;
    u8 *history_row;
    u8 *history_table_base;
    u8 *source_ptr;
    u8 *source_ptr_50;
    u8 *source_ptr_60;
    u8 command_byte;
    u8 command_id;
    u8 *pattern_table;

    scan_index = 0;
    if ((player < 2) && (D_80117E88[player] != 0))
    {
        history_offset = player * 0x10;
        saved_history_offset = history_offset;
        history_table_base = (u8 *)D_80117E98;
        history_row = history_offset + history_table_base;
        counts_address = (s32)D_80117E88;
        count_offset = player * 4;
    scan_loop:
        command_byte = *(history_row + scan_index);
        switch (command_byte)
        {
            case 0x1:
            case 0x2:
            case 0x3:
            case 0x4:
            case 0x6:
            case 0x8:
            case 0x9:
            case 0xC:
                direction_end = (scan_index + scan_index) - scan_index;
                if ((u8) * (history_row + scan_index) < 0x10U)
                {
                    s32 direction_count = *(s32 *)(count_offset + counts_address);
                    u8 *direction_base = (u8 *)D_80117E98;
                    u8 *direction_row = direction_base + player * 0x10;

                    direction_end++;
                    do
                    {
                        if (direction_end == direction_count)
                        {
                            goto block_48;
                        }
                    } while (direction_row[direction_end++] < 0x10);
                    direction_end--;
                }
            block_10:
                pattern_index = 0;
                if (*(history_row + direction_end) == 0x50)
                {
                    do { do { do { do { pattern_table = D_800EC304; } while (0); } while (0); } while (0); } while (0);
                    pattern_offset = pattern_index;
                    do
                    {
                        pattern_length = strlen((char *)(pattern_offset + (s32)pattern_table));
                        if ((direction_end - scan_index) >= pattern_length)
                        {
                            match_start = direction_end - pattern_length;
                            if (match_start >= scan_index)
                            {
                                u8 *match_ptr;

                                pattern_retry_offset = pattern_offset;
                            retry_loop:
                                match_ptr = (u8 *)match_start;
                                match_ptr += (s32)history_table_base;
                                {
                                    s32 pattern_address = pattern_retry_offset + (s32)pattern_table;
                                    s32 match_address = saved_history_offset + (s32)match_ptr;
                                    match_value = strncmp((char *)pattern_address, (char *)match_address, pattern_length);
                                }
                                if (match_value == 0)
                                {
                                        if (peek == 0)
                                        {
                                            consume_count = direction_end + 1;
                                            dest_index = 0;
                                            if (consume_count < 0x10)
                                            {
                                                source_index = consume_count;
                                                do
                                                {
                                                    source_ptr = history_row + source_index;
                                                    source_index += 1;
                                                    *(history_row + dest_index) = *source_ptr;
                                                    dest_index += 1;
                                                } while (source_index < 0x10);
                                            }
                                            *(s32 *)((count_offset + pattern_offset) + counts_address - pattern_offset) -= consume_count;
                                        }
                                        match_value = (s32)D_800EC334;
                                        return *((u8 *)match_value + pattern_index);
                                    }
                                match_start -= 1;
                                if (match_start >= scan_index)
                                {
                                    goto retry_loop;
                                }
                            }
                        }
                    block_23:
                        do { pattern_index += 1; } while (0);
                        pattern_offset += 8;
                    } while (pattern_index < 6);
                }
            block_24:
                scan_index = direction_end - 1;
                break;
            case 0x50:
                command_id = 3;
                if (peek == 0)
                {
                    consume_count_50 = scan_index + 1;
                    dest_index_50 = 0;
                    if (consume_count_50 < 0x10)
                    {
                        source_index_50 = consume_count_50;
                        do
                        {
                            source_ptr_50 = history_row + source_index_50;
                            source_index_50 += 1;
                            *(history_row + dest_index_50) = *source_ptr_50;
                            dest_index_50 += 1;
                        } while (source_index_50 < 0x10);
                    }
                    do
                    {
                        *(s32 *)(count_offset + counts_address) -= consume_count_50;
                    } while (0);
                    return 3U;
                }
                return command_id;
            case 0x60:
                command_id = 2;
                if (peek == 0)
                {
                    consume_count_60 = scan_index + 1;
                    dest_index_60 = 0;
                    if (consume_count_60 < 0x10)
                    {
                        source_index_60 = consume_count_60;
                        do
                        {
                            source_ptr_60 = history_row + source_index_60;
                            source_index_60 += 1;
                            *(history_row + dest_index_60) = *source_ptr_60;
                            dest_index_60 += 1;
                        } while (source_index_60 < 0x10);
                    }
                    do
                    {
                        *(s32 *)(count_offset + counts_address) -= consume_count_60;
                    } while (0);
                    return 2U;
                }
                return command_id;
            case 0x70:
                command_id = 1;
                do
                {
                    if (peek != 0)
                    {
                        goto case70_done;
                    }
                } while (0);
                do { func_800A2DD8(player); } while (0);
                return 1U;
            case70_done:
                return command_id;
            case 0x80:
                command_id = 0;
                do
                {
                    if (peek != 0)
                    {
                        goto case80_done;
                    }
                } while (0);
                do { func_800A2DD8(player); } while (0);
                return 0U;
            case80_done:
                return command_id;
            case 0x40:
                command_id = 4;
                do
                {
                    if (peek != 0)
                    {
                        goto case40_done;
                    }
                } while (0);
                do { func_800A2DD8(player); } while (0);
                return 4U;
            case40_done:
                return command_id;
            case 0x20:
                command_id = 6;
                do
                {
                    if (peek != 0)
                    {
                        goto case20_done;
                    }
                } while (0);
                do { func_800A2DD8(player); } while (0);
                return 6U;
            case20_done:
                return command_id;
            case 0x30:
                command_id = 5;
                if (peek == 0)
                {
                    func_800A2DD8(player);
                    return 5U;
                }
                return command_id;
            case 0x10:
                command_id = 7;
                if (peek == 0)
                {
                    func_800A2DD8(player);
                    return 7U;
                }
                return command_id;
            default:
                break;
            }
            scan_index++;
            if (scan_index != *(s32 *)(count_offset + counts_address))
            {
                goto scan_loop;
            }
        goto block_48;
    }
    else
    {
    block_48:
        command_id = 0xFF;
        return command_id;
    }
}

/**
 * @brief Clear a player's history count if the index is in range.
 * @param arg0 Player index; only indices below two are cleared.
 */
void func_800A2DD8(s32 arg0)
{
    if (arg0 < 2)
    {
        D_80117E88[arg0] = 0;
    }
}

/**
 * @brief Reset the command-recognizer scratch state to its idle defaults.
 */
void func_800A2DFC(void)
{
    D_80117ED0.unk8 = -2;
    D_80117ED0.unk4 = -2;
    D_80117ED0.unk0 = -2;
    D_80117EC0 = 0;
    D_80117EC8[0] = 0xFF;
    D_80117EC4 = 0;
}

/**
 * @brief Return a pointer to the D_80117EC8 byte table.
 * @return Address of D_80117EC8.
 */
u8 *func_800A2E34(void)
{
    return D_80117EC8;
}
