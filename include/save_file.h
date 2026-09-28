#ifndef SAVE_FILE_H
#define SAVE_FILE_H

#include "common.h"
#include "saved_game.h"
#include "sjis.h"

/**
 * @file save_file.h
 * @brief Save-file checksum, card-title and file-name functions compiled into ADDHERO, CARDA, CLOAD and NIKI.
 *
 * They check a save file's checksum, zero-fill its card title after the text,
 * and read and write the hex serial in its file name. Each overlay includes them from
 * src/overlays/common/<function>.inc.c at the point where they sit in its
 * binary, so every overlay still links its own copy.
 */

s32 validate_save_file(SaveFile* file);
s32 compute_save_checksum(void* data);
void terminate_multibyte_text(void* text);
u8* skip_hex_digits(u8* text);
void format_hex(s8* out, s32 value, s32 max_chars);
void hex_nibble_to_ascii(s8* out, s32 value);
u32 parse_hex(u8* text, s32 digits_left);
s32 parse_hex_suffix_byte(u8* text);

#endif
