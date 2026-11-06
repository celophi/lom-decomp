#ifndef FIELD_UI_TEXT_H
#define FIELD_UI_TEXT_H

#include "common.h"

/** @brief FIELD UI string table indexes. */
#define FIELD_UI_TEXT_MONEY 0
#define FIELD_UI_TEXT_EXPERIENCE 1
#define FIELD_UI_TEXT_BUY 2
#define FIELD_UI_TEXT_SELL 3
#define FIELD_UI_TEXT_ABILITY_MASTERED 4
#define FIELD_UI_TEXT_TECHNIQUE_MASTERED 5
#define FIELD_UI_TEXT_CANT_HOLD_MORE 6
#define FIELD_UI_TEXT_CHECK_DISC 7
#define FIELD_UI_TEXT_DISC_ERROR 8
#define FIELD_UI_TEXT_RESULTS_MONEY 9
#define FIELD_UI_TEXT_SPOILS 10
#define FIELD_UI_TEXT_PLUS 11
#define FIELD_UI_TEXT_TIMES 12
#define FIELD_UI_TEXT_EQUALS 13
#define FIELD_UI_TEXT_DASHES 14
#define FIELD_UI_TEXT_SPACE 15
#define FIELD_UI_TEXT_MINUS 16
#define FIELD_UI_TEXT_QUICK_ATTACKS 17
#define FIELD_UI_TEXT_POWER_ATTACKS 18
#define FIELD_UI_TEXT_CONTINUE 19
#define FIELD_UI_TEXT_TO_TITLE_SCREEN 20
#define FIELD_UI_TEXT_ATTACK_POWER 21
#define FIELD_UI_TEXT_TOTAL_DEFENSE 22
#define FIELD_UI_TEXT_POWER 23
#define FIELD_UI_TEXT_YOU_HAVE 24
#define FIELD_UI_TEXT_TIME_SEPARATOR 25
#define FIELD_UI_TEXT_CONFIRM 26
#define FIELD_UI_TEXT_YES 27
#define FIELD_UI_TEXT_NO 28
#define FIELD_UI_TEXT_CANT_BUY_MORE 29
#define FIELD_UI_TEXT_NO_ITEMS 30
#define FIELD_UI_TEXT_OPEN_PAREN 31
#define FIELD_UI_TEXT_CLOSE_PAREN 32
#define FIELD_UI_TEXT_VERSUS 33
#define FIELD_UI_TEXT_WINS 34
#define FIELD_UI_TEXT_LOSSES 35
#define FIELD_UI_TEXT_WINNER 36

/** @brief One offset entry of the FIELD UI string table: a little-endian offset from the table start. */
typedef struct
{
    u8 low;
    u8 high;
} FieldUiTextOffset;

/**
 * @brief Start of the FIELD UI string table, derived from the offset entry @p entry of string @p index.
 *
 * The table starts with one FieldUiTextOffset per string. Code reaches each
 * entry through its own symbol (g_field_ui_text_<name>) and derives the table
 * start back from it.
 */
#define FIELD_UI_TEXT_TABLE(entry, index) ((u8*)&(entry) - (index) * 2)

/** @brief Address of FIELD UI string @p index, given its offset entry @p entry. */
#define FIELD_UI_TEXT_AT(entry, index) (FIELD_UI_TEXT_TABLE(entry, index) + (entry).low + ((entry).high << 8))

/**
 * @brief Address of FIELD UI string @p index, given the start of the offset table in @p table.
 * @note Summed as integers, offset bytes first, which is how the original indexes the table.
 */
#define FIELD_UI_TEXT(table, index) ((u8*)((table)[(index) * 2] + (((table)[(index) * 2 + 1] << 8) + (uintptr_t)(table))))

/** @brief The FIELD UI string table: u16 offsets, one per string, then the strings. */
extern u8 g_field_ui_text_table[];

/** @brief Offset entries of the FIELD UI strings, each named after its FIELD_UI_TEXT_* index. */
extern FieldUiTextOffset g_field_ui_text_experience;
extern FieldUiTextOffset g_field_ui_text_ability_mastered;
extern FieldUiTextOffset g_field_ui_text_technique_mastered;
extern FieldUiTextOffset g_field_ui_text_cant_hold_more;
extern FieldUiTextOffset g_field_ui_text_check_disc;
extern FieldUiTextOffset g_field_ui_text_disc_error;
extern FieldUiTextOffset g_field_ui_text_results_money;
extern FieldUiTextOffset g_field_ui_text_spoils;
extern FieldUiTextOffset g_field_ui_text_plus;
extern FieldUiTextOffset g_field_ui_text_times;
extern FieldUiTextOffset g_field_ui_text_dashes;
extern FieldUiTextOffset g_field_ui_text_space;
extern FieldUiTextOffset g_field_ui_text_minus;
extern FieldUiTextOffset g_field_ui_text_quick_attacks;
extern FieldUiTextOffset g_field_ui_text_power_attacks;
extern FieldUiTextOffset g_field_ui_text_continue;
extern FieldUiTextOffset g_field_ui_text_attack_power;
extern FieldUiTextOffset g_field_ui_text_total_defense;
extern FieldUiTextOffset g_field_ui_text_power;
extern FieldUiTextOffset g_field_ui_text_you_have;
extern FieldUiTextOffset g_field_ui_text_time_separator;
extern FieldUiTextOffset g_field_ui_text_confirm;
extern FieldUiTextOffset g_field_ui_text_yes;
extern FieldUiTextOffset g_field_ui_text_cant_buy_more;
extern FieldUiTextOffset g_field_ui_text_no_items;
extern FieldUiTextOffset g_field_ui_text_versus;
extern FieldUiTextOffset g_field_ui_text_wins;
extern FieldUiTextOffset g_field_ui_text_losses;
extern FieldUiTextOffset g_field_ui_text_winner;

#endif
