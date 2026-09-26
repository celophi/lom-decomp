/**
 * @file field_command_history.c
 * @brief Per-player history of pad directions and buttons, and the command
 *        recognizer that reads special moves out of it.
 *
 * Each frame field_command_history_record() appends direction changes and
 * newly pressed buttons (as button codes of the bound actions) to a 16-entry
 * byte history per player. field_command_history_match() scans that history
 * for a button or a direction run followed by the finishing button and
 * returns the command it stands for.
 */

#include "common.h"
#include "main.h"
#include "controller_internal.h"
#include "sdk/libetc.h"
#include "sdk/strings.h"
#include "field_actor.h"
#include "field_calls.h"
#include "field_records.h"

/** @brief Entries in one player's history row. */
#define FIELD_HISTORY_LENGTH 16

/** @brief History codes below this are direction nibbles (1 up, 2 right, 4 down, 8 left). */
#define FIELD_HISTORY_BUTTON_CODE_MIN 0x10

/** @brief Button code that ends a direction pattern. */
#define FIELD_HISTORY_PATTERN_FINISH 0x50

/** @brief Idle counter value at which the history is dropped. */
#define FIELD_HISTORY_IDLE_EXPIRE 15

/** @brief Largest value of the idle counter (it stops counting there). */
#define FIELD_HISTORY_IDLE_MAX 16

/** @brief Direction patterns in g_field_command_patterns. */
#define FIELD_COMMAND_PATTERN_COUNT 6

/** @brief Bytes per pattern string, including the terminator and padding. */
#define FIELD_COMMAND_PATTERN_SIZE 8

/** @brief Returned by field_command_history_match() when no command was found. */
#define FIELD_COMMAND_NONE 0xFF

/** @brief Pad direction bits (after the byte swap). */
#define FIELD_PAD_DIRECTIONS (PADLup | PADLright | PADLdown | PADLleft)

/** @brief Shift from a pad direction bit to its history nibble. */
#define FIELD_PAD_DIRECTION_SHIFT 12

/** @brief Number of face and shoulder buttons that can start a history entry. */
#define FIELD_PAD_ACTION_BUTTON_COUNT 8

/** @brief Entry count per player (s32[2]). */
extern s32 g_field_command_history_count[FIELD_PLAYER_COUNT];

/** @brief Buttons seen in the previous frame, per player (u16[4]). */
extern u16 g_field_command_prev_buttons[];

/** @brief History rows: direction nibbles and button codes, oldest first. */
extern u8 g_field_command_history[FIELD_PLAYER_COUNT][FIELD_HISTORY_LENGTH];

/** @brief Frames since the last new entry, per player (capped at 16). */
extern s32 g_field_command_idle_frames[FIELD_PLAYER_COUNT];

/** @brief Button code for each bound action slot (0x10 to 0x80). */
extern u8 g_field_action_command_codes[];

/** @brief Physical button index to action slot binding index. */
extern u8 g_field_hint_button_map[];

/** @brief Direction patterns (NUL-terminated nibble strings) that end with the finish button. */
extern char g_field_command_patterns[FIELD_COMMAND_PATTERN_COUNT][FIELD_COMMAND_PATTERN_SIZE];

/** @brief Command returned for each entry of g_field_command_patterns. */
extern u8 g_field_command_pattern_ids[FIELD_COMMAND_PATTERN_COUNT];

extern s32 g_field_pair_indicator_counters[];
extern s32 g_field_pair_indicator_count;
extern s32 g_field_pair_indicators_disabled;
extern u8 g_field_pair_indicator_list[];

static void field_command_history_shift(s32 player);
static void field_command_history_consume(s32 player, s32 count);

/**
 * @brief Reset both players' histories and idle counters.
 */
void field_command_history_reset(void)
{
    g_field_command_history_count[1] = 0;
    g_field_command_history_count[0] = 0;
    g_field_command_idle_frames[1] = FIELD_HISTORY_IDLE_MAX;
    g_field_command_idle_frames[0] = FIELD_HISTORY_IDLE_MAX;
    g_field_command_prev_buttons[1] = 0;
    g_field_command_prev_buttons[0] = 0;
}

/**
 * @brief Append direction changes and newly pressed buttons to a player's history.
 * @param player Controller port; ports at or above two are ignored.
 * @param age_sequence Nonzero to advance the idle counter and drop an idle history.
 * @note A new button also records the direction held with it first.
 */
void field_command_history_record(s32 player, s32 age_sequence)
{
    s16 axis;
    /* Remapped buttons, later the idle counter: with a separate idle local, cse folds the copy into buttons. */
    s32 value;
    s32 buttons;
    u16 raw_buttons;
    u32 previous_direction;
    u32 button_index;
    s32 button_mask;
    u32 pressed_bit;
    u8 slot;
    s32 *count;
    ControllerPortState *ports = CONTROLLER_STATE->ports;
    /* Two port pointers: sharing one moves the reloaded port base out of v0. */
    ControllerPortState *stick_x_port;
    ControllerPortState *stick_y_port;

    if (player < FIELD_PLAYER_COUNT)
    {
        if (ports[player].published_sample.device_type >= CONTROLLER_DEVICE_CONFIGURING)
        {
            buttons = 0;
        }
        else
        {
            raw_buttons = ports[player].published_sample.held_buttons;
            buttons = (raw_buttons << 8) | (raw_buttons >> 8);
        }
        /* Swap the up/left and right/down face buttons. */
        value = ((u32)(buttons & PADRdown) >> 1) | ((buttons & PADRright) * 2) | ((u32)(buttons & PADRleft) >> 3) |
                ((buttons & PADRup) * 8) | (buttons & (u16)~(PADRup | PADRright | PADRdown | PADRleft));
        buttons = value;
        stick_x_port = ports + player;
        if (stick_x_port->published_sample.device_type != CONTROLLER_DEVICE_DIGITAL)
        {
            axis = stick_x_port->published_sample.left_stick_x;
            if (axis < 0)
            {
                buttons = value | PADLleft;
            }
            else if (axis > 0)
            {
                buttons = value | PADLright;
            }
            stick_y_port = ports + player;
            axis = stick_y_port->published_sample.left_stick_y;
            if (axis < 0)
            {
                buttons |= PADLup;
            }
            else if (axis > 0)
            {
                buttons |= PADLdown;
            }
        }
        previous_direction = g_field_command_prev_buttons[player] & FIELD_PAD_DIRECTIONS;
        if ((previous_direction != (buttons & FIELD_PAD_DIRECTIONS)) && (previous_direction != 0))
        {
            g_field_command_history[player][g_field_command_history_count[player]] = previous_direction >> FIELD_PAD_DIRECTION_SHIFT;
            if (g_field_command_history_count[player] < FIELD_HISTORY_LENGTH - 1)
            {
                g_field_command_history_count[player]++;
            }
            else
            {
                field_command_history_shift(player);
            }
            g_field_command_idle_frames[player] = 0;
        }
        for (button_index = 0, button_mask = 1; button_index < FIELD_PAD_ACTION_BUTTON_COUNT; button_index++, button_mask <<= 1)
        {
            pressed_bit = (u16)buttons & button_mask;
            if ((pressed_bit != 0) && ((g_field_command_prev_buttons[player] & button_mask) != pressed_bit))
            {
                if (buttons & FIELD_PAD_DIRECTIONS)
                {
                    g_field_command_history[player][g_field_command_history_count[player]] = (u16)buttons >> FIELD_PAD_DIRECTION_SHIFT;
                    if (g_field_command_history_count[player] < FIELD_HISTORY_LENGTH - 1)
                    {
                        g_field_command_history_count[player]++;
                    }
                    else
                    {
                        field_command_history_shift(player);
                    }
                }
                count = &g_field_command_history_count[player];
                /* g_pad_ctx points at the saved game, the block FieldGameState describes. */
                slot = ((FieldGameState *)g_pad_ctx)->characters[player].button_actions[g_field_hint_button_map[button_index]];
                g_field_command_history[player][*count] = g_field_action_command_codes[slot];
                if (*count < FIELD_HISTORY_LENGTH - 1)
                {
                    (*count)++;
                }
                else
                {
                    field_command_history_shift(player);
                }
                g_field_command_idle_frames[player] = 0;
            }
        }
        g_field_command_prev_buttons[player] = buttons;
        if (age_sequence != 0)
        {
            if (g_field_command_idle_frames[player] < FIELD_HISTORY_IDLE_MAX)
            {
                g_field_command_idle_frames[player]++;
            }
            value = g_field_command_idle_frames[player];
            if (value == FIELD_HISTORY_IDLE_EXPIRE)
            {
                field_command_history_clear(player);
            }
        }
    }
}

/**
 * @brief Drop the oldest entry of a player's full history row.
 * @param player Player index.
 */
static void field_command_history_shift(s32 player)
{
    s32 i;

    for (i = 0; i < FIELD_HISTORY_LENGTH - 1; i++)
    {
        g_field_command_history[player][i] = g_field_command_history[player][i + 1];
    }
}

/**
 * @brief Drop the first entries of a player's history.
 * @param player Player index.
 * @param count Number of leading entries to drop; also subtracted from the entry count.
 */
static void field_command_history_consume(s32 player, s32 count)
{
    u8 (*history)[FIELD_HISTORY_LENGTH];
    u8 *row;
    s32 src;
    s32 dst;

    dst = 0;
    if (count < FIELD_HISTORY_LENGTH)
    {
        history = g_field_command_history;
        row = history[player];
        src = count;
        do
        {
            row[dst++] = row[src++];
        } while (src < FIELD_HISTORY_LENGTH);
    }

    g_field_command_history_count[player] -= count;
}

/**
 * @brief Find a command at the start of a player's history.
 * @param player Player index; indices at or above two yield no command.
 * @param unused Not read (callers pass the facing direction).
 * @param peek Nonzero to look at the history without consuming the command.
 * @return Command for a button code, g_field_command_pattern_ids entry for a direction
 *         pattern ended by the finish button, or FIELD_COMMAND_NONE.
 * @note Entries scanned past without a match stay in the history.
 */
s32 field_command_history_match(s32 player, s32 unused, s32 peek)
{
    s32 command;
    s32 scan_index;
    s32 direction_end;
    s32 pattern_index;
    s32 pattern_length;
    s32 match_start;
    s32 consume_count;
    s32 src;
    s32 dst;
    u8 *history;

    command = FIELD_COMMAND_NONE;
    scan_index = 0;
    if ((player < FIELD_PLAYER_COUNT) && (g_field_command_history_count[player] != 0))
    {
        do
        {
            history = g_field_command_history[player];
            switch (history[scan_index])
            {
                /* Direction nibbles: 1 up, 2 right, 4 down, 8 left, and the diagonals. */
                case 0x1:
                case 0x2:
                case 0x3:
                case 0x4:
                case 0x6:
                case 0x8:
                case 0x9:
                case 0xC:
                    direction_end = scan_index;
                    while (g_field_command_history[player][direction_end] < FIELD_HISTORY_BUTTON_CODE_MIN)
                    {
                        direction_end++;
                        if (direction_end == g_field_command_history_count[player])
                        {
                            return command;
                        }
                    }
                    if (history[direction_end] == FIELD_HISTORY_PATTERN_FINISH)
                    {
                        for (pattern_index = 0; pattern_index < FIELD_COMMAND_PATTERN_COUNT; pattern_index++)
                        {
                            pattern_length = strlen(g_field_command_patterns[pattern_index]);
                            if ((direction_end - scan_index) >= pattern_length)
                            {
                                /* Try the pattern against every tail of the run, longest first. */
                                for (match_start = direction_end - pattern_length; match_start >= scan_index; match_start--)
                                {
                                    if (strncmp(g_field_command_patterns[pattern_index], (char *)&g_field_command_history[player][match_start], pattern_length) == 0)
                                    {
                                        if (peek == 0)
                                        {
                                            /* Step past the finish button first: with the increment
                                             * below the count, direction_end and pattern_length
                                             * swap registers. */
                                            direction_end++;
                                            dst = 0;
                                            consume_count = direction_end;
                                            if (consume_count < FIELD_HISTORY_LENGTH)
                                            {
                                                src = consume_count;
                                                do
                                                {
                                                    history[dst++] = history[src++];
                                                } while (src < FIELD_HISTORY_LENGTH);
                                            }
                                            g_field_command_history_count[player] -= consume_count;
                                        }
                                        return g_field_command_pattern_ids[pattern_index];
                                    }
                                }
                            }
                        }
                    }
                    scan_index = direction_end - 1;
                    break;
                /* Button codes, see g_field_action_command_codes. */
                case 0x50:
                    if (peek == 0)
                    {
                        consume_count = scan_index + 1;
                        dst = 0;
                        if (consume_count < FIELD_HISTORY_LENGTH)
                        {
                            src = consume_count;
                            do
                            {
                                history[dst++] = history[src++];
                            } while (src < FIELD_HISTORY_LENGTH);
                        }
                        g_field_command_history_count[player] -= consume_count;
                    }
                    return 3;
                case 0x60:
                    if (peek == 0)
                    {
                        consume_count = scan_index + 1;
                        dst = 0;
                        if (consume_count < FIELD_HISTORY_LENGTH)
                        {
                            src = consume_count;
                            do
                            {
                                history[dst++] = history[src++];
                            } while (src < FIELD_HISTORY_LENGTH);
                        }
                        g_field_command_history_count[player] -= consume_count;
                    }
                    return 2;
                case 0x70:
                    if (peek == 0)
                    {
                        field_command_history_clear(player);
                    }
                    return 1;
                case 0x80:
                    if (peek == 0)
                    {
                        field_command_history_clear(player);
                    }
                    return 0;
                case 0x40:
                    if (peek == 0)
                    {
                        field_command_history_clear(player);
                    }
                    return 4;
                case 0x20:
                    if (peek == 0)
                    {
                        field_command_history_clear(player);
                    }
                    return 6;
                case 0x30:
                    if (peek == 0)
                    {
                        field_command_history_clear(player);
                    }
                    return 5;
                case 0x10:
                    if (peek == 0)
                    {
                        field_command_history_clear(player);
                    }
                    return 7;
                default:
                    break;
            }
            scan_index++;
        } while (scan_index != g_field_command_history_count[player]);
    }
    return command;
}

/**
 * @brief Clear a player's history.
 * @param player Player index; only indices below two are cleared.
 */
void field_command_history_clear(s32 player)
{
    if (player < FIELD_PLAYER_COUNT)
    {
        g_field_command_history_count[player] = 0;
    }
}

/**
 * @brief Reset the pair indicator state to idle.
 */
void field_pair_indicators_reset(void)
{
    g_field_pair_indicator_counters[2] = -2;
    g_field_pair_indicator_counters[1] = -2;
    g_field_pair_indicator_counters[0] = -2;
    g_field_pair_indicator_count = 0;
    g_field_pair_indicator_list[0] = 0xFF;
    g_field_pair_indicators_disabled = 0;
}

/**
 * @brief Return the pair indicator list.
 * @return Address of g_field_pair_indicator_list.
 */
u8 *field_pair_indicators_get_list(void)
{
    return g_field_pair_indicator_list;
}
