#include "common.h"

extern u8 g_menuLayoutBuffer[];

typedef struct
{
    unsigned int nibble0 : 4;
    unsigned int nibble1 : 4;
    unsigned int nibble2 : 4;
    unsigned int nibble3 : 4;
    unsigned int nibble4 : 4;
    unsigned int nibble5 : 4;
    unsigned int nibble6 : 4;
    unsigned int nibble7 : 4;
} PackedNibbles8;

typedef struct
{
    unsigned int low : 8;
    unsigned int high : 24;
} ActivePacked32;

typedef struct
{
    unsigned short low : 9;
    unsigned short high : 7;
} ActivePacked16;

#define U8(p, o) (*(u8 *)((u8 *)(p) + (o)))
#define U16(p, o) (*(u16 *)((u8 *)(p) + (o)))
#define U32(p, o) (*(u32 *)((u8 *)(p) + (o)))

/**
 * @brief Copy the selected group's derived stats into an active record.
 * @param group_index Index of the source group in the menu-layout stat table.
 * @param destination Active record to populate.
 */
void func_800C3F18(s32 group_index, void *destination)
{
    u8 *record = destination;
    s32 packed_metadata;
    s32 group_offset;
    s32 stat_record_offset;
    s32 stat_cursor_offset;
    s32 index;
    s32 resistance_offset;
    u16 primary_stat;
    u16 stat_value;
    u8 *name_destination;
    u8 name_value;
    u8 flags0;
    u8 flags1;
    u8 flags2_or_enabled;
    u8 *stat_record;
    u8 *resistance_source;
    u8 *source_record;
    u8 *stat_source;
    u8 *resistance_cursor;
    u8 *slot_cursor;
    u8 *secondary_clear_cursor;
    u8 *clear_cursor;
    u8 *record_stat_cursor;
    u8 *name_table;
    u8 *group_table;
    s32 stat_table_address;
    u8 *final_table;

    index = 0;
    name_table = g_menuLayoutBuffer;
    group_offset = group_index * 0x14C;
    U8(record, 0x50) = 1;
    U8(record, 0x90) = 1;
    U8(record, 0xD0) = 0;
    U8(record, 0x110) = 0;
    do
    {
        name_destination = record + index;
        name_value = U8(name_table, index + group_offset + 0x2B0C);
        index += 1;
        *name_destination = name_value;
    } while (index < 0x15);
    index = 1;
    U32(record, 0x18) = (s32) (((U32(record, 0x18) & ~0x7F) | 4) & ~0x80);
    clear_cursor = record + 1;
    group_table = g_menuLayoutBuffer;
    U8(record, 0x19) = (s8) (U8(group_table, group_index * 0x14C + 0x2B50) & 0xF);
    do
    {
        U8(clear_cursor, 0x1A) = 0;
        index -= 1;
        clear_cursor -= 1;
    } while (index >= 0);
    index = 3;
    secondary_clear_cursor = record + 3;
    do
    {
        U8(secondary_clear_cursor, 0x1C) = 0;
        index -= 1;
        secondary_clear_cursor -= 1;
    } while (index >= 0);
    index = 0;
    stat_record_offset = group_index * 0x14C;
    stat_record = (u8 *)(stat_record_offset + (s32) g_menuLayoutBuffer);
    stat_table_address = (s32) g_menuLayoutBuffer;
    ((ActivePacked32 *)(record + 0x20))->low = 0x63;
    ((ActivePacked32 *)(record + 0x20))->high = 0;
    slot_cursor = record + 0x90;
    U16(record, 0x24) = (u16) U16(stat_record, 0x2B22);
    stat_cursor_offset = stat_record_offset;
    primary_stat = U16(stat_record, 0x2B24);
    record_stat_cursor = record;
    U16(record, 0x26) = primary_stat;
    U16(record, 0x74) = primary_stat;
    do
    {
        stat_source = (u8 *)(stat_cursor_offset + stat_table_address);
        stat_cursor_offset += 2;
        stat_value = U16(stat_source, 0x2B26);
        index += 1;
        U16(record_stat_cursor, 0x28) = stat_value;
        U16(slot_cursor, 0x24) = stat_value;
        slot_cursor += 2;
        record_stat_cursor += 2;
    } while (index < 4);
    index = 0;
    stat_table_address = (s32) g_menuLayoutBuffer;
    resistance_offset = group_index * 0x14C;
    resistance_cursor = record;
    do
    {
        resistance_cursor++;
        resistance_cursor--;
        resistance_source = (u8 *)(resistance_offset + stat_table_address);
        resistance_offset += 2;
        index += 1;
        ((ActivePacked16 *)(resistance_cursor + 0x30))->low = ((ActivePacked16 *)(resistance_source + 0x2B38))->low;
        ((ActivePacked16 *)(resistance_cursor + 0x30))->high = ((ActivePacked16 *)(resistance_source + 0x2B38))->high;
        resistance_cursor += 2;
    } while (index < 8);
    final_table = g_menuLayoutBuffer;
    source_record = (group_index * 0x14C) + final_table;
    flags0 = U8(source_record, 0x2B48);
    U8(record, 0x40) = flags0;
    U8(record, 0xBC) = flags0;
    flags1 = U8(source_record, 0x2B49);
    U8(record, 0x41) = flags1;
    U8(record, 0x7C) = flags1;
    flags2_or_enabled = U8(source_record, 0x2B4A);
    U8(record, 0x42) = flags2_or_enabled;
    U8(record, 0xBD) = flags2_or_enabled;
    flags2_or_enabled = U8(source_record, 0x2B4B);
    U8(record, 0x48) = 0;
    U8(record, 0x49) = 0;
    U8(record, 0x4A) = 0;
    U8(record, 0x4B) = 0;
    U8(record, 0x4C) = 0;
    U8(record, 0x4D) = 0;
    U8(record, 0x4E) = 0;
    U8(record, 0x4F) = 0;
    U8(record, 0x43) = flags2_or_enabled;
    ((PackedNibbles8 *)(record + 0x68))->nibble0 = ((PackedNibbles8 *)(source_record + 0x2B30))->nibble0;
    ((PackedNibbles8 *)(record + 0x68))->nibble1 = ((PackedNibbles8 *)(source_record + 0x2B30))->nibble1;
    ((PackedNibbles8 *)(record + 0x68))->nibble2 = ((PackedNibbles8 *)(source_record + 0x2B30))->nibble2;
    ((PackedNibbles8 *)(record + 0x68))->nibble3 = ((PackedNibbles8 *)(source_record + 0x2B30))->nibble3;
    ((PackedNibbles8 *)(record + 0x68))->nibble4 = ((PackedNibbles8 *)(source_record + 0x2B30))->nibble4;
    ((PackedNibbles8 *)(record + 0x68))->nibble5 = ((PackedNibbles8 *)(source_record + 0x2B30))->nibble5;
    ((PackedNibbles8 *)(record + 0x68))->nibble6 = ((PackedNibbles8 *)(source_record + 0x2B30))->nibble6;
    ((PackedNibbles8 *)(record + 0x68))->nibble7 = ((PackedNibbles8 *)(source_record + 0x2B30))->nibble7;
    ((PackedNibbles8 *)(record + 0xA8))->nibble0 = ((PackedNibbles8 *)(source_record + 0x2B34))->nibble0;
    ((PackedNibbles8 *)(record + 0xA8))->nibble1 = ((PackedNibbles8 *)(source_record + 0x2B34))->nibble1;
    ((PackedNibbles8 *)(record + 0xA8))->nibble2 = ((PackedNibbles8 *)(source_record + 0x2B34))->nibble2;
    ((PackedNibbles8 *)(record + 0xA8))->nibble3 = ((PackedNibbles8 *)(source_record + 0x2B34))->nibble3;
    ((PackedNibbles8 *)(record + 0xA8))->nibble4 = ((PackedNibbles8 *)(source_record + 0x2B34))->nibble4;
    ((PackedNibbles8 *)(record + 0xA8))->nibble5 = ((PackedNibbles8 *)(source_record + 0x2B34))->nibble5;
    ((PackedNibbles8 *)(record + 0xA8))->nibble6 = ((PackedNibbles8 *)(source_record + 0x2B34))->nibble6;
    ((PackedNibbles8 *)(record + 0xA8))->nibble7 = ((PackedNibbles8 *)(source_record + 0x2B34))->nibble7;
    packed_metadata = (U32(record, 0x174) & ~0xF) | (U8(source_record, 0x2B50) & 0xF);
    U32(record, 0x174) = packed_metadata;
    U32(record, 0x174) = (s32) ((packed_metadata & ~0xF0) | (U8(source_record, 0x2B50) & 0xF0));
    U8(record, 0x175) = (u8) U8(source_record, 0x2B51);
    U8(record, 0x176) = (u8) U8(source_record, 0x2B52);
    U8(record, 0x177) = (u8) U8(source_record, 0x2B53);
    U32(record, 0x178) = (s32) U32(source_record, 0x2B54);
}
