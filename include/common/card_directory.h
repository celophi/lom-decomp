#ifndef CARD_DIRECTORY_H
#define CARD_DIRECTORY_H

#include "common.h"
#include <kernel.h>

/**
 * @file card_directory.h
 * @brief The memory-card directory listing kept by ADDHERO, CARDA, CLOAD and NIKI.
 *
 * Each of these overlays reads both cards' directories into g_card_entries and
 * parses the hex serial in each Legend of Mana file name with
 * parse_entry_fields, compiled in from src/common/card_directory/parse_entry_fields.inc.c.
 * The globals below map to each overlay's own data.
 */

/** @brief Memory-card slots, and directory entries read from each card. */
#define CARD_SLOT_COUNT 2
#define CARD_DIRECTORY_ENTRY_COUNT 20

/** @brief Bytes of one directory entry, and of one card's listing in g_card_entries. */
#define CARD_DIRECTORY_ENTRY_BYTES sizeof(struct DIRENTRY)
#define CARD_DIRECTORY_BYTES (CARD_DIRECTORY_ENTRY_COUNT * CARD_DIRECTORY_ENTRY_BYTES)

/** @brief Length of the Legend of Mana file name prefix before the hex serial. */
#define CARD_SAVE_FILENAME_PREFIX_LENGTH 12

/** @brief Hex digits parse_entry_fields reads from a file name's serial. */
#define CARD_SERIAL_DIGITS 5

/** @brief Bits contributed by each hex digit of a save serial. */
#define CARD_SERIAL_DIGIT_BITS 4

/** @brief Decimal digits before the first letter in a hex alphabet. */
#define CARD_SERIAL_DECIMAL_DIGITS 10

/** @brief Serial marker for an entry whose name does not have the save prefix. */
#define CARD_ENTRY_SERIAL_NONE (-1)

/**
 * @brief Locate the hex serial in a card directory entry's file name.
 * @param card Memory-card slot.
 * @param index Directory entry index.
 * @return Unsigned bytes following the save filename prefix.
 */
#define CARD_ENTRY_SERIAL_TEXT(card, index) \
    (((card) * CARD_DIRECTORY_BYTES + (index) * CARD_DIRECTORY_ENTRY_BYTES) + \
     (u8*)&g_card_entries[0][0].name[CARD_SAVE_FILENAME_PREFIX_LENGTH])

/** @brief Directory listing of both cards. */
extern struct DIRENTRY g_card_entries[CARD_SLOT_COUNT][CARD_DIRECTORY_ENTRY_COUNT];

/**
 * @brief Number of entries read from the current card, or an overlay-specific status above that range.
 * @note Each overlay defines its own status values.
 */
extern s32 g_card_entry_state;

/** @brief Hex serial of each Legend of Mana entry, or -1 for other files. */
extern s32 g_card_entry_fields[CARD_SLOT_COUNT][CARD_DIRECTORY_ENTRY_COUNT];

/** @brief Hex suffix after each save's serial and separator, or zero for other files. */
extern s32 g_card_entry_suffix_values[];

extern char g_lom_save_filename_prefix[];

s32 parse_entry_fields(void);

#endif
