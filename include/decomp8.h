#ifndef _DECOMP8_H
#define _DECOMP8_H

#include "common.h"
#include "psyq/strings.h"
#include "psyq/memory.h"

/** @brief Convert a 0-9 digit value to its ASCII character code. */
#define DIGIT_TO_ASCII(d) ((d) + 0x30)

typedef struct
{
    u32 unk0;
    u32 unk4;
    u32 unk8;
    u32 unkC;
    u32 unk10;
} CommandBuffer;
typedef struct
{
    u8 pad[16];
    u32 unk10;
} NodeWithOffset16;

extern s32 g_text_cursor_x;
extern s32 g_text_cursor_y;
extern u8 g_hex_digit_table[17];
extern CommandBuffer *g_field_primitive_cursor;
extern NodeWithOffset16 *g_field_current_render_half;
extern s32 g_text_atlas_base;


extern void func_800165CC(int, int, s32);

#endif
