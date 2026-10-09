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

/** @brief Bytes in one memory-card block. */
#define CARD_BLOCK_BYTES 8192

/** @brief open() mode bits that allocate @p count blocks when the file is created (with FCREAT). */
#define CARD_FILE_BLOCKS(count) ((count) << 16)

/** @brief Bytes of the device prefix "bu00:" and of the search pattern "bu00:*", with their terminators. */
#define CARD_DEVICE_BYTES sizeof("bu00:")
#define CARD_SEARCH_PATTERN_BYTES sizeof("bu00:*")

/** @brief Memory-card device prefix, such as "bu00"; adding a card slot to its slot digit selects that card. */
typedef union
{
    u32 word;
    struct
    {
        u8 name[2];
        u8 slot;
        u8 port;
    } characters;
} CardDevice;

/** @brief Eight-byte path template in the overlay data: the device prefix "bu00:" or the search pattern "bu00:*". */
typedef union
{
    char text[8];
    CardDevice device;
} CardPathTemplate;

/** @brief Directory search pattern buffer, built from the "bu00:*" template. */
typedef union
{
    char text[16];
    CardDevice device;
} CardSearchPattern;

/** @brief Path of one file on the card: the device prefix and a directory entry name. */
typedef union
{
    char text[32];
    CardDevice device;
} CardFilePath;

/** @brief Path workspace of the card load/save sequence. */
typedef union
{
    char text[104];
    CardDevice device;
} CardSequencePath;

/** @brief Path of the selected directory entry, built to read its save header. */
typedef union
{
    char text[256];
    CardDevice device;
} CardEntryPath;

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

/** @brief File name prefix of the PocketStation mini-game (Ring Ring Land) save. */
extern char g_lom_pocketstation_filename_prefix[];

/** @brief Name prefix of the new-save placeholder entry (CARD_MENU_NEW_SAVE_NAME_LENGTH characters). */
extern char g_new_save_entry_prefix[];

/** @brief Temporary file names a save, and a PocketStation save, are written under before the rename. */
extern char g_lom_save_dummy_filename[];
extern char g_lom_pocketstation_dummy_filename[];

s32 parse_entry_fields(void);

/**
 * @brief Set the SavedGameLayout.known_save_flags bit of every product code that @p file_name starts with (FIELD).
 * @param file_name Memory-card file name of one directory entry.
 */
void field_flag_known_save(char* file_name);

#endif
