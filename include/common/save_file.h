#ifndef SAVE_FILE_H
#define SAVE_FILE_H

#include "common.h"
#include "common/saved_game.h"
#include "common/sjis.h"

/**
 * @file save_file.h
 * @brief Save-file checksum, card-title and file-name functions compiled into ADDHERO, CARDA, CLOAD and NIKI.
 *
 * They check a save file's checksum, zero-fill its card title after the text,
 * and read and write the hex serial in its file name. Each overlay includes them from
 * src/common/save_file/<function>.inc.c at the point where they sit in its
 * binary, so every overlay still links its own copy.
 */

/** @brief Hex digit layout used to format a 32-bit value. */
#define SAVE_HEX_MAX_DIGITS 8
#define SAVE_HEX_DIGIT_BITS 4
#define SAVE_HEX_DIGIT_MASK 0xF

/** @brief Hex radix and number of numeric digits before 'A'. */
#define SAVE_HEX_RADIX (SAVE_HEX_DIGIT_MASK + 1)
#define SAVE_HEX_DECIMAL_DIGITS 10

/** @brief Placeholder written for hex digit values at or above SAVE_HEX_RADIX. */
#define SAVE_HEX_INVALID_DIGIT '_'

/** @brief Maximum hex digits read from the suffix after a save file's serial. */
#define SAVE_HEX_SUFFIX_DIGITS 2

s32 validate_save_file(SaveFile* file);
s32 compute_save_checksum(const void* save_data);
void terminate_multibyte_text(void* title_text);
u8* skip_hex_digits(u8* cursor);
void format_hex(s8* destination, s32 value, s32 max_digits);
void hex_nibble_to_ascii(s8* destination, s32 nibble);
u32 parse_hex(const u8* cursor, s32 digits_left);
s32 parse_hex_suffix_byte(const char* field_text);

#endif
