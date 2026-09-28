#ifndef ENCODED_TEXT_H
#define ENCODED_TEXT_H

#include "common.h"

/**
 * @file encoded_text.h
 * @brief The game's own text encoding and the string helpers built into many overlays.
 *
 * Names and messages are null-terminated byte strings. Codes 0x19 to 0x1F
 * start a two-byte code whose second byte may be zero, so these strings are
 * measured and copied a code at a time rather than with the C library.
 *
 * The helpers were compiled into every overlay that edits names or builds
 * messages. Each overlay includes the ones it has from
 * src/overlays/common/encoded_text_*.inc.c at the point where they sit in its
 * binary, so every overlay still links its own copy.
 *
 * FIELD's resident copies (field_name_byte_length, field_copy_name) keep their
 * own names and source: MENU calls them by name through the shared symbol
 * file, which GNAME, GOLEM, GOSUB and SHOP also read alongside their own copies.
 */

/**
 * @brief First code these helpers treat as the start of a two-byte code.
 * @note In the US renderer only 0x19 (a two-byte character) and 0x1F (an extended dictionary entry)
 *       take a second byte, and 0x1A-0x1E are one-byte dictionary entries; the Japanese text uses
 *       the whole range as lead bytes. The helpers skip two bytes for the whole range in both versions.
 */
#define ENCODED_TEXT_DOUBLE_BYTE_LEAD_FIRST 0x19

/** @brief Last code these helpers treat as the start of a two-byte code. */
#define ENCODED_TEXT_DOUBLE_BYTE_LEAD_LAST 0x1F

/** @brief True when @p code starts a two-byte code. */
#define ENCODED_TEXT_IS_DOUBLE_BYTE_LEAD(code) ((code) >= ENCODED_TEXT_DOUBLE_BYTE_LEAD_FIRST && (code) <= ENCODED_TEXT_DOUBLE_BYTE_LEAD_LAST)

void encoded_text_append(u8* dst, u8* src);
s32 encoded_text_byte_length(u8* text);
void encoded_text_copy(u8* dst, u8* src);

#endif
