#include "common.h"
#include "field_calls.h"
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
extern u8 g_field_hint_button_map[];
extern u8 D_800EC304[];
extern u8 D_800EC334[];
extern u8 *g_pad_ctx;
extern UnkStruct80117ED0 D_80117ED0;
extern s32 D_80117EC0;
extern s32 D_80117EC4;
extern u8 D_80117EC8[];

/*
 * D_80117E90 is read as s16 by func_800A255C and u16 by func_800A2594, so it
 * stays block scope inside each user. D_80117E98 is a u8[][0x10] byte history
 * (one 16-entry row per player); its users declare it block scope as well.
 */

/* Forward prototypes for members defined later in the file. */
void func_800A2958(s32);
void field_command_history_clear(s32);

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
    extern u8 D_80117E98[][0x10];
    s16 axis;
    /* Holds the remapped buttons, then the idle counter; the shared life keeps the copy into buttons. */
    s32 value;
    s32 buttons;
    u16 raw_buttons;
    u32 previous_direction;
    u32 button_index;
    s32 button_mask;
    u32 pressed_bit;
    u8 *binding;
    s32 *count;
    ControllerState *pads = (ControllerState *)0x801ED600;
    ControllerState *pad_x;
    ControllerState *pad_y;

    if (player < 2)
    {
        if (pads[player].status >= 0xFEU)
        {
            buttons = 0;
        }
        else
        {
            raw_buttons = pads[player].buttons;
            buttons = (raw_buttons << 8) | (raw_buttons >> 8);
        }
        value = ((u32) (buttons & 0x40) >> 1) | ((buttons & 0x20) * 2) | ((u32) (buttons & 0x80) >> 3) | ((buttons & 0x10) * 8) | (buttons & 0xFF0F);
        buttons = value;
        pad_x = pads + player;
        if (pad_x->status != 0)
        {
            axis = pad_x->axis_x;
            if (axis < 0)
            {
                buttons = value | 0x8000;
            }
            else if (axis > 0)
            {
                buttons = value | 0x2000;
            }
            pad_y = pads + player;
            axis = pad_y->axis_y;
            if (axis < 0)
            {
                buttons |= 0x1000;
            }
            else if (axis > 0)
            {
                buttons |= 0x4000;
            }
        }
        previous_direction = D_80117E90[player] & 0xF000;
        if ((previous_direction != (buttons & 0xF000)) && (previous_direction != 0))
        {
            D_80117E98[player][D_80117E88[player]] = previous_direction >> 12;
            if (D_80117E88[player] < 0xF)
            {
                D_80117E88[player]++;
            }
            else
            {
                func_800A2958(player);
            }
            D_80117EB8[player] = 0;
        }
        for (button_index = 0, button_mask = 1; button_index < 8; button_index++, button_mask <<= 1)
        {
            pressed_bit = (u16)buttons & button_mask;
            if ((pressed_bit != 0) && ((D_80117E90[player] & button_mask) != pressed_bit))
            {
                if (buttons & 0xF000)
                {
                    D_80117E98[player][D_80117E88[player]] = (u16)buttons >> 12;
                    if (D_80117E88[player] < 0xF)
                    {
                        D_80117E88[player]++;
                    }
                    else
                    {
                        func_800A2958(player);
                    }
                }
                count = &D_80117E88[player];
                binding = g_pad_ctx + player * 0x250 + g_field_hint_button_map[button_index];
                D_80117E98[player][*count] = D_800EC2F4[binding[0x638]];
                if (*count < 0xF)
                {
                    (*count)++;
                }
                else
                {
                    func_800A2958(player);
                }
                D_80117EB8[player] = 0;
            }
        }
        D_80117E90[player] = buttons;
        if (age_sequence != 0)
        {
            if (D_80117EB8[player] < 0x10)
            {
                D_80117EB8[player]++;
            }
            value = D_80117EB8[player];
            if (value == 0xF)
            {
                field_command_history_clear(player);
            }
        }
    }
}

/**
 * @brief Shift a player's row of the byte history table down by one entry.
 * @param player Row index into D_80117E98.
 */
void func_800A2958(s32 player)
{
    extern u8 D_80117E98[][0x10];
    s32 i;

    for (i = 0; i < 0xF; i++)
    {
        D_80117E98[player][i] = D_80117E98[player][i + 1];
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
                            return 0xFF;
                        }
                    } while (direction_row[direction_end++] < 0x10);
                    direction_end--;
                }
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
                                do { pattern_index += 1; } while (0);
                        pattern_offset += 8;
                    } while (pattern_index < 6);
                }
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
                /* 0x70/0x80/0x40/0x20 keep a do/while(0); it sets the player/peek registers (99.46% without). */
                command_id = 1;
                do
                {
                    if (peek == 0)
                    {
                        field_command_history_clear(player);
                        return 1U;
                    }
                } while (0);
                return command_id;
            case 0x80:
                command_id = 0;
                do
                {
                    if (peek == 0)
                    {
                        field_command_history_clear(player);
                        return 0U;
                    }
                } while (0);
                return command_id;
            case 0x40:
                command_id = 4;
                do
                {
                    if (peek == 0)
                    {
                        field_command_history_clear(player);
                        return 4U;
                    }
                } while (0);
                return command_id;
            case 0x20:
                command_id = 6;
                do
                {
                    if (peek == 0)
                    {
                        field_command_history_clear(player);
                        return 6U;
                    }
                } while (0);
                return command_id;
            case 0x30:
                command_id = 5;
                if (peek == 0)
                {
                    field_command_history_clear(player);
                    return 5U;
                }
                return command_id;
            case 0x10:
                command_id = 7;
                if (peek == 0)
                {
                    field_command_history_clear(player);
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
    }
    return 0xFF;
}

/**
 * @brief Clear a player's history count if the index is in range.
 * @param player Player index; only indices below two are cleared.
 */
void field_command_history_clear(s32 player)
{
    if (player < 2)
    {
        D_80117E88[player] = 0;
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
