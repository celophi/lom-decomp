#include "common.h"

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

extern u8 D_800EC2F4[];
extern u8 D_800EC2FC[];
extern s32 D_80117E88[];
extern u16 D_80117E90[];
extern u8 D_80117E98[];
extern s32 D_80117EB8[];
extern u8 *g_pad_ctx;
void func_800A2958(s32);
void func_800A2DD8(s32);

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
