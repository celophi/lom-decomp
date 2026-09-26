/**
 * @file field_stat_counter_ops.c
 * @brief Game flag bits and the saturating item counts of the game state.
 */

#include "game_audio.h"
#include "common.h"
#include "field_calls.h"
#include "field_records.h"

/** @brief Largest quantity an item count can hold. */
#define FIELD_ITEM_COUNT_MAX 99

/** @brief Item kinds at or above this value are rejected with a diagnostic. */
#define FIELD_ITEM_KIND_LIMIT 0xFF

/** @brief Character limit of an item name placed in a text macro. */
#define FIELD_ITEM_NAME_LENGTH 21

/** @brief Diagnostic codes for an out-of-range item kind (passed as the first argument). */
#define DIAG_BAD_ITEM_KIND_GET 0x70
#define DIAG_BAD_ITEM_KIND_ADD 0x71
#define DIAG_BAD_ITEM_KIND_REMOVE 0x72

static void field_set_item_name_macro(s32 kind);

extern FieldGameState* g_field_game_state;
/** @brief Item name texts: a table of offsets from the table start, one per item kind. */
extern u16 D_800F0E98[];

/**
 * @brief Set one of the game flag bits.
 * @param bit_index Bit to set; bit n lives in word n / 32.
 * @return Always -1.
 */
s32 field_set_game_flag(s32 bit_index)
{
    s32 word;
    s32 bit;

    word = bit_index / 32;
    bit = bit_index % 32;
    g_field_game_state->flag_bits[word] |= 1 << bit;
    return -1;
}

/**
 * @brief Put an item's name in text macro 0 and return how many are held.
 * @param kind Item kind, or FIELD_ITEM_KIND_LIMIT and above to report an invalid kind.
 * @return The item count, or 0 for an invalid kind.
 */
u8 field_get_item_count(s32 kind)
{
    if (kind < FIELD_ITEM_KIND_LIMIT)
    {
        field_set_item_name_macro(kind);
        return g_field_game_state->item_counts[kind];
    }
    record_game_diagnostic(DIAG_ERROR, DIAG_BAD_ITEM_KIND_GET, kind, 0);
    return 0;
}

/**
 * @brief Add one item, saturating at FIELD_ITEM_COUNT_MAX, and put its name in text macro 0.
 * @param kind Item kind, or FIELD_ITEM_KIND_LIMIT and above to report an invalid kind.
 */
void field_receive_item(s32 kind)
{
    if (kind < FIELD_ITEM_KIND_LIMIT)
    {
        g_field_game_state->item_counts[kind]++;
        if (g_field_game_state->item_counts[kind] > FIELD_ITEM_COUNT_MAX)
        {
            g_field_game_state->item_counts[kind] = FIELD_ITEM_COUNT_MAX;
        }
        field_set_item_name_macro(kind);
    }
    else
    {
        record_game_diagnostic(DIAG_ERROR, DIAG_BAD_ITEM_KIND_ADD, kind, 0);
    }
}

/**
 * @brief Remove one item if any is held, and put its name in text macro 0.
 * @param kind Item kind, or FIELD_ITEM_KIND_LIMIT and above to report an invalid kind.
 */
void field_consume_item(s32 kind)
{
    u8 count;

    if (kind < FIELD_ITEM_KIND_LIMIT)
    {
        count = g_field_game_state->item_counts[kind];
        if (count != 0)
        {
            g_field_game_state->item_counts[kind] = count - 1;
        }
        field_set_item_name_macro(kind);
    }
    else
    {
        record_game_diagnostic(DIAG_ERROR, DIAG_BAD_ITEM_KIND_REMOVE, kind, 0);
    }
}

/**
 * @brief Put the name of an item kind in text macro 0.
 * @param kind Item kind; selects an offset in D_800F0E98.
 */
static void field_set_item_name_macro(s32 kind)
{
    field_set_text_macro(0, (u8*)D_800F0E98 + D_800F0E98[kind], FIELD_ITEM_NAME_LENGTH);
}
