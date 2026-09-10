#include "common.h"
#include "sdk/strings.h"
void func_800A2DD8(s32); /* extern */
extern u8 D_800EC304[];
extern u8 D_800EC334[];
extern s32 D_80117E88[];
extern u8 D_80117E98[];

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
