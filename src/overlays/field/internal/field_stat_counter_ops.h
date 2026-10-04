#ifndef FIELD_STAT_COUNTER_OPS_H
#define FIELD_STAT_COUNTER_OPS_H

#include "common.h"

#define FIELD_ITEM_NAME_COUNT 256

/** @brief Offset table and strings, with offsets relative to the table base. */
typedef union
{
    u16 offsets[FIELD_ITEM_NAME_COUNT];
    u8 bytes[1];
} FieldItemNameTable;

extern FieldItemNameTable g_field_item_name_table;

s32 field_set_game_flag(s32 bit_index);
s32 field_get_item_count(s32 kind);
void field_receive_item(s32 kind);
void field_consume_item(s32 kind);

#endif
