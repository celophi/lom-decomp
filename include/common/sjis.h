#ifndef SJIS_H
#define SJIS_H

#include "common.h"

/**
 * @brief Shift-JIS bytes from this value up lead a two-byte character.
 * @note The game's text code tests only this bound, not the exact Shift-JIS lead ranges.
 */
#define SJIS_LEAD_MIN 0x80

/** @brief Byte width of a multibyte Shift-JIS character. */
#define SJIS_MULTIBYTE_CHAR_BYTES 2

/** @brief Shift-JIS full-width "0"; the other digits follow it. */
#define SJIS_DIGIT_ZERO 0x824F

/** @brief Shift and mask for the leading and trailing bytes of a Shift-JIS code. */
#define SJIS_CODE_LEAD_SHIFT 8
#define SJIS_CODE_TRAIL_MASK 0xFF

/** @brief Decimal radix and starting divisor for formatting up to six digits. */
#define SJIS_DECIMAL_RADIX 10
#define SJIS_DECIMAL_FIRST_DIVISOR 100000

/** @brief Full-width "MAX" in Shift-JIS, written by format_decimal for values of 1000000 and up. */
#define SJIS_DECIMAL_OVERFLOW_TEXT "\x82\x6c\x82\x60\x82\x77"

/**
 * @brief Format @p value as full-width Shift-JIS digits without leading zeros and null-terminate it.
 * @param destination Buffer with room for up to six two-byte digits and a null byte.
 * @param value Nonnegative value to format; 1000000 and up becomes full-width "MAX".
 * @return Pointer to the terminator.
 * @note Compiled into ADDHERO, CARDA and NIKI from src/common/sjis/format_decimal.inc.c.
 */
s8* format_decimal(s8* destination, s32 value);

#endif
