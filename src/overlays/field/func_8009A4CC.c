#include "common.h"
#include "sdk/memory.h"

/* The signed byte count preserves the original memcpy calls with GCC 2.7.2. */
void *memcpy(void *, const void *, s32);

/** @brief Byte-aligned 18-byte section copied from the packed resource stream. */
typedef struct
{
    s8 bytes[18];
} Copy18;

/** @brief Byte-aligned 10-byte section copied from the packed resource stream. */
typedef struct
{
    s8 bytes[10];
} Copy10;

/** @brief Byte-aligned 28-byte section copied from the packed resource stream. */
typedef struct
{
    s8 bytes[28];
} Copy28;

/** @brief Byte-aligned 6-byte section copied from the packed resource stream. */
typedef struct
{
    s8 bytes[6];
} Copy6;

/** @brief Actor resource pointers and selectors at the original 0x244-byte stride. */
typedef struct
{
    u8 *entries;
    u8 *records;
    u8 *indices;
    u8 *header;
    u8 *current;
    u8 *payload;
    u8 *groups;
    u8 *optional[2];
    u8 reserved24;
    u8 count;
    u8 reserved26[3];
    u8 state29;
    u8 state2a;
    u8 reserved2b[0x222 - 0x2B];
    u16 header_id;
    u8 reserved224[0x240 - 0x224];
    u8 *extra;
} Actor;

#define U8_AT(p, o) (*(u8 *)((u8 *)(p) + (o)))
#define U16_AT(p, o) (*(u16 *)((u8 *)(p) + (o)))
#define S32_AT(p, o) (*(s32 *)((u8 *)(p) + (o)))
#define PTR_AT(p, o) (*(u8 **)((u8 *)(p) + (o)))
extern u8 D_80105798[];
extern u8 *D_8010D034;
extern u8 *D_8010D038;
extern u8 D_8011BF00[];
void *func_8009CA54(u8 *, s32, s32);
void func_8009AE38(u8 *, s32);

/**
 * @brief Unpack a loaded actor resource into allocated runtime sections.
 * @param group Allocation group; values at least three use allocation tag two.
 * @param actor Actor slot receiving resource pointers, counts, and header state.
 * @note Resource offsets, alignment, optional sections, and record remapping follow the packed format.
 */
void func_8009A4CC(s32 group, Actor *actor)
{
    s32 part_count;
    u8 *resource_base;
    s32 texture_total;
    s32 direct_group;
    /** @brief Six-byte remapping record with word-sized bitfield writes. */
    union
    {
        u32 word;
        /** @brief Low-byte remapping coordinate and flag. */
        struct
        {
            u32 x : 7;
            u32 flag : 1;
            u32 unused : 24;
        } bits;
        /** @brief Byte and halfword view of the remapped record. */
        struct
        {
            u8 b0;
            u8 b1;
            u16 h2;
            u16 h4;
        } h;
    } packed;
    s32 allocation_size;
    s32 has_payload;
    s32 payload_size;
    s32 record_bytes;
    s32 index_bytes;
    s32 entry_bytes;
    s32 optional_bytes;
    s32 payload_tag;
    s32 header_tag;
    s32 optional_tag;
    s32 part_tag;
    s32 vector_tag;
    s32 byte_tag;
    s32 texture_tag;
    s32 record_tag;
    s32 index_tag;
    s32 entry_tag;
    s32 row;
    s32 texture_height;
    s32 item_index;
    s32 record_offset;
    s32 optional_count;
    s32 has_optional;
    u8 *cursor;
    s32 part_index;
    s32 table_group;
    s32 table_group_offset;
    s32 column;
    u8 *indices;
    u8 *entries;
    u8 *header;
    u8 *payload;
    u8 *records;
    u8 *vector_output;
    u8 *pixel_input;
    u8 *row_input;
    u8 *pixel_output;
    u8 *header_cursor;
    u32 optional_offset;
    u8 *part;
    u8 *part_fields;
    u16 payload_offset;
    u16 component;
    u16 pixel;
    s32 byte_index;
    u8 *texture_record;
    s32 record_count;
    u8 byte_value;
    u8 texture_count;
    u8 *payload_source;
    u8 *texture_fields;
    u8 **header_pool;
    u8 **optional_pool;
    u8 *shared_base;

    header_cursor = D_8010D038;
    cursor = header_cursor;
    resource_base = cursor;
    if (U16_AT(cursor, 0) != 0)
    {
        func_8009AE38(cursor + U16_AT(cursor, 0), group);
    }
    cursor = header_cursor + 2;
    has_payload = U16_AT(cursor, 0);
    cursor = (u8 *)((u32)(cursor + 5) & ~3);
    if (has_payload != 0)
    {
        payload_offset = U16_AT(cursor, 0);
        payload_source = header_cursor + payload_offset;
        payload_size = U16_AT(cursor, 2) - payload_offset;
        payload_tag = group;
        if (payload_tag >= 3)
        {
            payload_tag = 2;
        }
        header_cursor = func_8009CA54(D_8010D034, payload_size, payload_tag);
        cursor += 2;
        memcpy(header_cursor, payload_source, payload_size);
        actor->payload = header_cursor;
    }
    cursor += 2;
    part_count = U16_AT(cursor, 0);
    cursor += 2;
    if (part_count != 0)
    {
        part_tag = group;
        if (part_tag >= 3)
        {
            part_tag = 2;
        }
        part = func_8009CA54(D_8010D034, 0x48, part_tag);
        actor->groups = part;
    }
    part_index = 0;
    texture_total = 0;
    if (part_count != 0)
    {
        part_fields = part;
        direct_group = group < 3;
        do
        {
            header_cursor = resource_base + U16_AT(cursor, 0);
            U16_AT(part, 0) = U16_AT(header_cursor, 0);
            header_cursor += 4;
            U8_AT(part_fields, 2) = *header_cursor;
            header_cursor += 4;
            allocation_size = U16_AT(part, 0) * 0x28;
            vector_tag = group;
            if (direct_group == 0)
            {
                vector_tag = 2;
            }
            item_index = 0;
            vector_output = func_8009CA54(D_8010D034, allocation_size, vector_tag);
            PTR_AT(part_fields, 8) = vector_output;
            if ((U16_AT(part, 0) * 3) != 0)
            {
                do
                {
                    component = U16_AT(header_cursor, 0);
                    header_cursor += 2;
                    item_index++;
                    U16_AT(vector_output, 0) = component;
                    vector_output += 2;
                    U16_AT(vector_output, 0) = U16_AT(header_cursor, 0);
                    header_cursor += 2;
                    vector_output += 2;
                    U16_AT(vector_output, 0) = U16_AT(header_cursor, 0);
                    header_cursor += 2;
                    vector_output += 4;
                } while (item_index < (U16_AT(part, 0) * 3));
            }
            PTR_AT(part_fields, 12) = vector_output;
            item_index = 0;
            if (U16_AT(part, 0) != 0)
            {
                do
                {
                    component = U16_AT(header_cursor, 0);
                    header_cursor += 2;
                    item_index++;
                    U16_AT(vector_output, 0) = component;
                    vector_output += 2;
                    U16_AT(vector_output, 0) = U16_AT(header_cursor, 0);
                    header_cursor += 2;
                    vector_output += 2;
                    U16_AT(vector_output, 0) = U16_AT(header_cursor, 0);
                    header_cursor += 2;
                    vector_output += 4;
                } while (item_index < (s32)U16_AT(part, 0));
            }
            PTR_AT(part_fields, 16) = vector_output;
            item_index = 0;
            if (U16_AT(part, 0) != 0)
            {
                do
                {
                    component = U16_AT(header_cursor, 0);
                    header_cursor += 2;
                    item_index++;
                    U16_AT(vector_output, 0) = component;
                    vector_output += 2;
                    U16_AT(vector_output, 0) = U16_AT(header_cursor, 0);
                    header_cursor += 2;
                    vector_output += 2;
                    U16_AT(vector_output, 0) = U16_AT(header_cursor, 0);
                    header_cursor += 2;
                    vector_output += 4;
                } while (item_index < (s32)U16_AT(part, 0));
            }
            allocation_size = U16_AT(part, 0) * 0x10;
            byte_tag = group;
            if (direct_group == 0)
            {
                byte_tag = 2;
            }
            item_index = 0;
            vector_output = func_8009CA54(D_8010D034, allocation_size, byte_tag);
            PTR_AT(part_fields, 20) = vector_output;
            if (U16_AT(part, 0) != 0)
            {
                do
                {
                    byte_index = 0;
                    do
                    {
                        byte_index += 1;
                        byte_value = *header_cursor;
                        header_cursor += 1;
                        *vector_output = byte_value;
                        vector_output += 1;
                    } while ((u32)byte_index < 0x10U);
                    item_index += 1;
                } while (item_index < (s32)U16_AT(part, 0));
            }
            table_group = group;
            if (direct_group == 0)
            {
                table_group = 2;
            }
            table_group_offset = table_group * 8;
            texture_record = PTR_AT(part_fields, 4) = (u8 *)((((table_group_offset + table_group) * 8) + (u32)D_80105798) + (texture_total * 8));
            byte_index = 0;
            if (U8_AT(part_fields, 2) != 0)
            {
                texture_fields = texture_record;
                do
                {
                    *texture_record = *header_cursor++;
                    U8_AT(texture_fields, 1) = *header_cursor++;
                    U8_AT(texture_fields, 2) = *header_cursor++;
                    U8_AT(texture_fields, 3) = *header_cursor++;
                    allocation_size = U8_AT(texture_fields, 2) * U8_AT(texture_fields, 3) * 2;
                    texture_tag = group;
                    if (direct_group == 0)
                    {
                        texture_tag = 2;
                    }
                    pixel_output = func_8009CA54(D_8010D034, allocation_size, texture_tag);
                    row = 0;
                    PTR_AT(texture_fields, 4) = pixel_output;
                    texture_height = U8_AT(texture_fields, 3);
                    row_input = resource_base + U16_AT(resource_base, 0) + 0x220;
                    row_input = row_input + *texture_record * 2 + (U8_AT(texture_fields, 1) << 7);
                    do {
                    if (texture_height != 0)
                    {
                        do
                        {
                            pixel_input = row_input;
                            column = 0;
                            if (U8_AT(texture_fields, 2) != 0)
                            {
                                do
                                {
                                    column += 1;
                                    pixel = U16_AT(pixel_input, 0);
                                    pixel_input += 2;
                                    U16_AT(pixel_output, 0) = pixel;
                                    pixel_output += 2;
                                } while (column < (s32)U8_AT(texture_fields, 2));
                            }
                            row += 1;
                            row_input += 0x80;
                        } while (row < (s32)U8_AT(texture_fields, 3));
                    }
                    byte_index += 1;
                    texture_fields += 8;
                    texture_record += 8;
                    } while (0);
                } while (byte_index < (s32)U8_AT(part_fields, 2));
            }
            part_index += 1;
            cursor += 2;
            texture_count = U8_AT(part_fields, 2);
            texture_total += texture_count;
            part_fields += 0x18;
            part += 0x18;
        } while (part_index < part_count);
    }
    header_pool = &D_8010D034;
    part_index = U16_AT(cursor, 0);
    cursor += 2;
    record_count = *cursor++;
    record_tag = group;
    record_bytes = record_count * 6;
    if (record_tag >= 3)
    {
        record_tag = 2;
    }
    records = func_8009CA54(*header_pool, record_bytes, record_tag);
    actor->records = records;
    memcpy(records, cursor, record_bytes);
    cursor += record_bytes;
    byte_index = *cursor++;
    index_bytes = byte_index * 2;
    index_tag = group;
    if (group >= 3)
    {
        index_tag = 2;
    }
    indices = func_8009CA54(*header_pool, index_bytes, index_tag);
    actor->indices = indices;
    memcpy(indices, cursor, index_bytes);
    cursor += index_bytes;
    byte_index = *cursor;
    optional_count = byte_index >> 6;
    if (optional_count >= 3)
    {
        optional_count = 0;
    }
    byte_index = byte_index & 0x3F;
    cursor++;
    actor->count = byte_index;
    entry_bytes = byte_index * 0x48;
    entry_tag = group;
    if (group >= 3)
    {
        entry_tag = 2;
    }
    entries = func_8009CA54(*header_pool, entry_bytes, entry_tag);
    actor->entries = entries;
    memcpy(entries, cursor, entry_bytes);
    cursor += entry_bytes;
    actor->state29 = 0;
    header_tag = group;
    if (group >= 3)
    {
        header_tag = 2;
    }
    header = func_8009CA54(*header_pool, 0x5A, header_tag);
    actor->header = header;
    U16_AT(header, 0x12) = part_index;
    header_cursor = actor->header;
    actor->current = header_cursor;
    *(Copy18 *)header_cursor = *(Copy18 *)cursor;
    header_cursor += 0x12;
    cursor += 0x12;
    if (U16_AT(actor->header, 0xC) & 0x8000)
    {
        *(Copy10 *)header_cursor = *(Copy10 *)cursor;
        header_cursor += 0xA;
        cursor += 0xA;
    }
    if ((u32)((u8)U8_AT(actor->entries, 0x14) >> 4) < 2U)
    {
        part_index = 0;
        if (record_count != 0)
        {
            record_offset = 0;
            do
            {
                packed.bits.x = *(u8 *)(record_offset + (u32)actor->records) & 0x7F;
                packed.bits.flag = *(u16 *)(record_offset + (u32)actor->records) >> 15;
                packed.h.b1 = ((u16)*(u16 *)(record_offset + (u32)actor->records) >> 8) & 0x7F;
                packed.h.h4 = U16_AT(actor->records, record_offset + 4);
                packed.h.h2 = U16_AT(actor->records, record_offset + 2);
                bcopy((u8 *)&packed, actor->records + record_offset, 6);
                part_index += 1;
                record_offset += 6;
            } while (part_index < (s32)record_count);
        }
    }
    actor->header_id = (u16)U16_AT(actor->header, 0x12);
    if (U16_AT(actor->current, 0xC) & 0x800)
    {
        *(Copy28 *)header_cursor = *(Copy28 *)cursor;
        header_cursor += 0x1C;
        cursor += 0x1C;
        *(Copy28 *)header_cursor = *(Copy28 *)cursor;
        cursor += 0x1C;
        header_cursor = (u8 *)((u32)(header_cursor + 0x1D) & ~1);
        actor->extra = header_cursor;
        actor->state29 = 0;
        actor->state2a = 0;
        *(Copy6 *)header_cursor = *(Copy6 *)cursor;
    }
    if (optional_count != 0)
    {
        part_index = 0;
        has_optional = optional_count != 0;
        if (has_optional)
        {
            optional_offset = 0;
            do
            {
                cursor = (u8 *)((u32)(cursor + 3) & ~3);
                optional_pool = &D_8010D034;
                shared_base = D_8011BF00;
                optional_bytes = S32_AT(cursor, 0);
                if (optional_bytes < 0x1000)
                {
                    optional_tag = group;
                    if (optional_tag >= 3)
                    {
                        optional_tag = 2;
                    }
                    PTR_AT((u8 *)actor + optional_offset, 0x1C) = func_8009CA54(*optional_pool, optional_bytes, optional_tag);
                }
                else
                {
                    PTR_AT((u8 *)actor + optional_offset, 0x1C) = (u8 *)(S32_AT(shared_base, 4) + (u32)shared_base);
                }
                cursor += 4;
                if (optional_bytes < 0x1000)
                {
                    memcpy(PTR_AT((u8 *)actor + optional_offset, 0x1C), cursor, optional_bytes);
                }
                cursor += optional_bytes;
                part_index += 1;
                optional_offset += 4;
            } while (part_index < optional_count);
        }
    }
}
