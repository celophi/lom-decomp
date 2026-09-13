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
 * @note WIP: 97.966805% gcc272_cdk with allocation and scheduling differences.
 */
void func_800A2594(s32 player, s32 age_sequence)
{
    extern u16 D_80117E90[];
    extern u8 D_80117E98[];
    s16 axis_x;
    s16 axis_y;
    s32 remapped_buttons;
    s32 *direction_count;
    s32 *button_count;
    s32 *history_count;
    s32 *idle_counter;
    s32 old_count;
    s32 idle_frames;
    s32 player_word_offset;
    s32 address_or_index;
    s32 scaled_player;
    s32 pressed_bit;
    s32 direction_length;
    s32 button_length;
    s32 buttons;
    s32 button_mask;
    s32 axis_index;
    u16 raw_buttons;
    u32 previous_direction;
    u32 current_buttons;
    u32 button_index;
    u32 direction_nibble;
    u8 *history_base;
    u8 *mapping;
    u8 *history;
    u16 *previous_buttons;
    ControllerState *pads = (ControllerState *)0x801ED600;

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
        remapped_buttons = ((u32)(buttons & 0x40) >> 1) | ((buttons & 0x20) * 2) |
                           ((u32)(buttons & 0x80) >> 3) | ((buttons & 0x10) * 8) |
                           (buttons & 0xFF0F);
        scaled_player = player;
        buttons = (u16)remapped_buttons;
        if (pads[scaled_player].status != 0)
        {
            axis_x = pads[scaled_player].axis_x;
            if (axis_x < 0)
            {
                buttons = remapped_buttons | 0x8000;
                goto check_vertical_axis;
            }
            axis_index = player * 2;
            if (axis_x > 0)
            {
                buttons = remapped_buttons | 0x2000;
            check_vertical_axis:
                axis_index = player * 2;
            }
            axis_y = pads[player].axis_y;
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
            history_base = D_80117E98 + player * 0x10;
            history_count = &D_80117E88[player];
            history_base[*history_count] = (s8)(previous_direction >> 0xC);
            old_count = *history_count;
            if (old_count < 0xF)
            {
                *history_count = old_count + 1;
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
        scaled_player = player * 2;
        previous_buttons = (u16 *)(scaled_player + address_or_index);
        address_or_index = (s32)D_80117E98;
        scaled_player = player * 0x10;
        history = (u8 *)(scaled_player + address_or_index);
        player_word_offset = player * 4;
        current_buttons = buttons & 0xFFFF;
        direction_nibble = current_buttons >> 12;
    /* Keep the scan explicit to avoid hoisting extra table-address carriers. */
    button_loop:
    {
        pressed_bit = current_buttons & button_mask;
        if ((pressed_bit != 0) && ((*previous_buttons & button_mask) != pressed_bit))
        {
            if (buttons & 0xF000)
            {
                direction_count = (s32 *)((s32)D_80117E88 + player_word_offset);
                *(history + *direction_count) = (s8)direction_nibble;
                direction_length = *direction_count;
                if (direction_length < 0xF)
                {
                    *direction_count = direction_length + 1;
                }
                else
                {
                    func_800A2958(player);
                }
            }
            /* Preserve the separate append-and-reset block used by the input scan. */
            do
            {
                button_count = (s32 *)((s32)D_80117E88 + player_word_offset);
                mapping = (u8 *)((s32)g_pad_ctx + player * 0x250 + D_800EC2FC[button_index]);
                *(history + *button_count) = D_800EC2F4[mapping[0x638]];
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
        button_index += 1;
        button_mask *= 2;
    }
        if (button_index < 8U)
        {
            goto button_loop;
        }
        D_80117E90[player] = buttons;
        if (age_sequence != 0)
        {
            address_or_index = (s32)D_80117EB8;
            scaled_player = player * 4;
            idle_counter = (s32 *)(scaled_player + address_or_index);
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
 * @note Direction patterns use six eight-byte strings and an external result table.
 */
s32 func_800A29F8(s32 player, s32 unused, s32 peek)
{
    extern u8 D_80117E98[];
    s32 buffer_offset;
    s32 *counts;
    s32 *temp_v1_2;
    s32 *temp_v1_3;
    s32 *temp_v1_4;
    s32 temp_a2;
    s32 temp_a2_2;
    s32 temp_a2_3;
    s32 temp_t3;
    s32 temp_v0_2;
    s32 temp_v0_3;
    s32 temp_v1;
    s32 var_a0_2;
    s32 var_a0_3;
    s32 var_a0_4;
    s32 var_a1;
    s32 var_a1_2;
    s32 var_a1_3;
    s32 peek_only;
    s32 match_start;
    s32 direction_end;
    s32 var_s1_2;
    s32 pattern_offset;
    s32 scan_index;
    s32 pattern_index;
    s32 player_index;
    s32 count_offset;
    u8 *buffer;
    u8 *temp_v0_4;
    u8 *temp_v0_5;
    u8 *temp_v0_6;
    u8 *scan_pointer;
    u8 temp_v0;
    u8 result;
    void *pattern_pointer;
    u8 *patterns;

    player_index = player;
    peek_only = peek;
    scan_index = 0;
    if ((player_index < 2) &&
        (temp_v1 = player_index * 4, temp_t3 = player_index * 0x10, (*(s32 *)(temp_v1 + (u8 *)D_80117E88) != 0)))
    {
        buffer_offset = temp_t3;
        buffer = temp_t3 + D_80117E98;
        counts = D_80117E88;
        count_offset = temp_v1;
        scan_pointer = buffer + scan_index;
        do
        {
            temp_v0 = *scan_pointer;
            switch (temp_v0)
            {
            case 0x1:
            case 0x2:
            case 0x3:
            case 0x4:
            case 0x6:
            case 0x8:
            case 0x9:
            case 0xC:
                direction_end = scan_index;
                if ((u8) * (buffer + scan_index) < 0x10U)
                {
                    direction_end++;
                    do
                    {
                        if (direction_end == *(s32 *)(count_offset + (u8 *)counts))
                        {
                            goto block_48;
                        }
                    } while (D_80117E98[player_index * 0x10 + direction_end++] < 0x10);
                    direction_end--;
                }
            block_10:
                pattern_index = 0;
                if (*(buffer + direction_end) == 0x50)
                {
                    patterns = D_800EC304;
                    pattern_offset = pattern_index;
                    do
                    {
                        temp_v0_2 = strlen(pattern_offset + patterns);
                        if ((direction_end - scan_index) >= temp_v0_2)
                        {
                            match_start = direction_end - temp_v0_2;
                            pattern_pointer = pattern_offset + patterns;
                            if (match_start >= scan_index)
                            {
                                do
                                {
                                    temp_v0_3 =
                                        strncmp(pattern_pointer, buffer_offset + (match_start + D_80117E98), temp_v0_2);
                                    if (temp_v0_3 == 0)
                                    {
                                        if (peek_only == 0)
                                        {
                                            temp_a2 = direction_end + 1;
                                            var_a1 = 0;
                                            if (temp_a2 < 0x10)
                                            {
                                                var_a0_2 = temp_a2;
                                                do
                                                {
                                                    temp_v0_4 = buffer + var_a0_2;
                                                    var_a0_2 += 1;
                                                    *(buffer + var_a1) = *temp_v0_4;
                                                    var_a1 += 1;
                                                } while (var_a0_2 < 0x10);
                                            }
                                            temp_v1_2 = (s32 *)(count_offset + (u8 *)counts);
                                            *temp_v1_2 -= temp_a2;
                                        }
                                        return *(pattern_index + D_800EC334);
                                    }
                                    match_start -= 1;
                                    pattern_pointer = pattern_offset + patterns;
                                } while (match_start >= scan_index);
                            }
                        }
                    block_23:
                        pattern_index += 1;
                        pattern_offset += 8;
                    } while (pattern_index < 6);
                }
            block_24:
                scan_index = direction_end - 1;
                break;
            case 0x50:
                result = 3;
                if (peek_only == 0)
                {
                    temp_a2_2 = scan_index + 1;
                    var_a1_2 = 0;
                    if (temp_a2_2 < 0x10)
                    {
                        var_a0_3 = temp_a2_2;
                        do
                        {
                            temp_v0_5 = buffer + var_a0_3;
                            var_a0_3 += 1;
                            *(buffer + var_a1_2) = *temp_v0_5;
                            var_a1_2 += 1;
                        } while (var_a0_3 < 0x10);
                    }
                    temp_v1_3 = (s32 *)(count_offset + (u8 *)counts);
                    *temp_v1_3 -= temp_a2_2;
                    return 3U;
                }
                return result;
            case 0x60:
                result = 2;
                if (peek_only == 0)
                {
                    temp_a2_3 = scan_index + 1;
                    var_a1_3 = 0;
                    if (temp_a2_3 < 0x10)
                    {
                        var_a0_4 = temp_a2_3;
                        do
                        {
                            temp_v0_6 = buffer + var_a0_4;
                            var_a0_4 += 1;
                            *(buffer + var_a1_3) = *temp_v0_6;
                            var_a1_3 += 1;
                        } while (var_a0_4 < 0x10);
                    }
                    temp_v1_4 = (s32 *)(count_offset + (u8 *)counts);
                    *temp_v1_4 -= temp_a2_3;
                    return 2U;
                }
                return result;
            case 0x70:
                result = 1;
                if (peek_only == 0)
                {
                    func_800A2DD8(player_index);
                    return 1U;
                }
                return result;
            case 0x80:
                result = 0;
                if (peek_only == 0)
                {
                    func_800A2DD8(player_index);
                    return 0U;
                }
                return result;
            case 0x40:
                result = 4;
                if (peek_only == 0)
                {
                    func_800A2DD8(player_index);
                    return 4U;
                }
                return result;
            case 0x20:
                result = 6;
                if (peek_only == 0)
                {
                    func_800A2DD8(player_index);
                    return 6U;
                }
                return result;
            case 0x30:
                result = 5;
                if (peek_only == 0)
                {
                    func_800A2DD8(player_index);
                    return 5U;
                }
                return result;
            case 0x10:
                result = 7;
                if (peek_only == 0)
                {
                    func_800A2DD8(player_index);
                    return 7U;
                }
                return result;
            default:
                break;
            }
            scan_index++;
            scan_pointer = buffer + scan_index;
        } while (scan_index != *(s32 *)(count_offset + (u8 *)counts));
        goto block_48;
    }
    else
    {
    block_48:
        result = 0xFF;
        return result;
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
