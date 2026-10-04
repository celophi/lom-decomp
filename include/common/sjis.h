#ifndef SJIS_H
#define SJIS_H

#include "common.h"

/**
 * @brief Shift-JIS bytes from this value up lead a two-byte character.
 * @note The game's text code tests only this bound, not the exact Shift-JIS lead ranges.
 */
#define SJIS_LEAD_MIN 0x80

/** @brief Shift-JIS full-width "0"; the other digits follow it. */
#define SJIS_DIGIT_ZERO 0x824F

/** @brief Three full-width characters and their terminator, written instead of a number that is too large. */
typedef struct
{
    s8 data[7];
} SjisDecimalOverflowText;

/**
 * @brief "MAX" in full-width characters, written by format_decimal for values of 1000000 and up.
 * @note Each overlay that includes format_decimal has its own copy.
 */
extern const SjisDecimalOverflowText g_decimal_overflow_text;

/**
 * @brief Format @p value as full-width Shift-JIS digits without leading zeros and null-terminate it.
 * @param out Destination buffer.
 * @param value Value to format.
 * @return Pointer to the terminator.
 * @note Compiled into ADDHERO, CARDA and NIKI from src/common/sjis/format_decimal.inc.c.
 */
s8* format_decimal(s8* out, s32 value);

#endif
