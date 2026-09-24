#ifndef FIELD_UI_TEXT_H
#define FIELD_UI_TEXT_H

#include "common.h"

/**
 * @brief Address of FIELD UI string @p index, given its two-byte offset entry @p entry.
 *
 * The UI string table at 0x800EC3C4 starts with little-endian u16 offsets,
 * one per string, relative to the table start. Each offset entry is its own
 * symbol, so the table start is derived back from the entry.
 */
#define FIELD_UI_TEXT_AT(entry, index) ((entry) - (index) * 2 + (entry)[0] + ((entry)[1] << 8))

/**
 * @brief Address of FIELD UI string @p index, given the start of the offset table in @p table.
 * @note Summed as integers, offset bytes first, which is how the original indexes the table.
 */
#define FIELD_UI_TEXT(table, index) ((u8*)((table)[(index) * 2] + (((table)[(index) * 2 + 1] << 8) + (s32)(table))))

#endif
