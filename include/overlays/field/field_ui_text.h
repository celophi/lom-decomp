#ifndef FIELD_UI_TEXT_H
#define FIELD_UI_TEXT_H

#include "common.h"

/** @brief FIELD UI string table indexes (entry N is at 0x800EC3C4 + N * 2). */
#define FIELD_UI_TEXT_CANT_HOLD_MORE 6
#define FIELD_UI_TEXT_PLUS 11
#define FIELD_UI_TEXT_DASHES 14
#define FIELD_UI_TEXT_SPACE 15
#define FIELD_UI_TEXT_ATTACK_POWER 21
#define FIELD_UI_TEXT_TOTAL_DEFENSE 22
#define FIELD_UI_TEXT_POWER 23
#define FIELD_UI_TEXT_TIME_SEPARATOR 25
#define FIELD_UI_TEXT_YES 27
#define FIELD_UI_TEXT_NO 28

/**
 * @brief Address of FIELD UI string @p index, given its two-byte offset entry @p entry.
 *
 * The UI string table at 0x800EC3C4 starts with little-endian u16 offsets,
 * one per string, relative to the table start. Each offset entry is its own
 * symbol, so the table start is derived back from the entry.
 */
#define FIELD_UI_TEXT_AT(entry, index) (FIELD_UI_TEXT_TABLE(entry, index) + (entry)[0] + ((entry)[1] << 8))

/** @brief Start of the FIELD UI string table, derived from the offset entry @p entry of string @p index. */
#define FIELD_UI_TEXT_TABLE(entry, index) ((entry) - (index) * 2)

/**
 * @brief Address of FIELD UI string @p index, given the start of the offset table in @p table.
 * @note Summed as integers, offset bytes first, which is how the original indexes the table.
 */
#define FIELD_UI_TEXT(table, index) ((u8*)((table)[(index) * 2] + (((table)[(index) * 2 + 1] << 8) + (uintptr_t)(table))))

/** @brief Offset entries of the FIELD UI strings FIELD_UI_TEXT_YES and FIELD_UI_TEXT_TIME_SEPARATOR. */
extern u8 g_text_choice_glyph_offsets[];
extern u8 g_text_time_separator_offset_bytes[2];

#endif
