/**
 * @file field_resource_table_ops.c
 * @brief Resource-page table lookups, resource bits and inventory record release.
 */

#include "game_audio.h"
#include "common.h"
#include "field_records.h"

/** @brief Size of one resource page in D_801148B0. */
#define FIELD_RESOURCE_PAGE_SIZE 0x1000

/** @brief Resource id of the item template table. */
#define FIELD_RESOURCE_ITEM_TEMPLATES 5

/**
 * @brief Header of a resource page: byte offsets of its three tables.
 * @note The first two tables are halfword offset tables relative to the table start.
 */
typedef struct
{
    s32 table_offsets[3];
} FieldResourcePage;

/** @brief Third table of a resource page: a count, then 8-byte records. */
typedef struct
{
    u32 count;
    u8 records[1][8];
} FieldResourceRecordTable;

/** @brief Item item_template table (resource 5). */
typedef struct
{
    u16 unk0;
    u16 count;
    FieldItemRecord templates[1];
} FieldItemTemplateTable;

void* func_800C1E40(s32 resource_id);
FieldItemRecord* field_find_free_inventory_record(void);
void func_800B2844(s32 arg0, void* script, s32 arg2);
void field_copy_inventory_record(FieldItemRecord* destination, FieldItemRecord* source);
void field_compact_inventory(void);
s32 func_800C2AD0(void);

extern u8 D_801148B0[];
extern FieldGameState* D_80122B74;

/**
 * @brief Resolve an entry of the first offset table in a resource page.
 * @param page Page index into D_801148B0.
 * @param entry Entry index within the table.
 * @return Address of the entry.
 */
s32 func_800C28F8(s32 page, u16 entry)
{
    u8* base = D_801148B0 + page * FIELD_RESOURCE_PAGE_SIZE;
    u8* table = base + ((FieldResourcePage*)base)->table_offsets[0];

    return (s32)table + ((s16*)table)[entry];
}

/**
 * @brief Resolve an entry of the second offset table in a resource page.
 * @param page Page index into D_801148B0.
 * @param entry Entry index within the table (low 16 bits used).
 * @return Address of the entry.
 */
void* func_800C2928(s32 page, s32 entry)
{
    u8* base;
    s16* table;

    base = D_801148B0 + page * FIELD_RESOURCE_PAGE_SIZE;
    table = (s16*)(base + ((FieldResourcePage*)base)->table_offsets[1]);
    return (u8*)table + table[entry & 0xFFFF];
}

/**
 * @brief Return an 8-byte record of the third table in a resource page.
 * @param page Page index into D_801148B0.
 * @param index Record index.
 * @return Record address, or NULL when @p index is out of range.
 */
void* func_800C2958(s32 page, u16 index)
{
    u8* base;
    FieldResourceRecordTable* table;

    base = &D_801148B0[page * FIELD_RESOURCE_PAGE_SIZE];
    table = (FieldResourceRecordTable*)(base + ((FieldResourcePage*)base)->table_offsets[2]);
    if (index < table->count)
    {
        return table->records[index];
    }
    return NULL;
}

/**
 * @brief Set one bit of the game-state resource bits.
 * @param bit_index Bit to set.
 */
void func_800C299C(s32 bit_index)
{
    D_80122B74->resource_bits[(u32)bit_index / 32] |= 1 << (bit_index & 0x1F);
}

/**
 * @brief Run an item template's script and copy it into a free inventory record.
 * @param index Template index; validated against the table's count.
 * @return 0 on success, -1 on any failure.
 * @see decomp.me (100%)
 */
s32 func_800C29CC(s32 index)
{
    FieldItemTemplateTable* table;
    FieldItemRecord* item_template;
    FieldItemRecord* record;

    table = func_800C1E40(FIELD_RESOURCE_ITEM_TEMPLATES);
    if (table == NULL)
    {
        record_game_diagnostic(0x8001, 0x6C, index, 0);
        return -1;
    }
    if (index >= table->count)
    {
        record_game_diagnostic(0x8001, 0x6C, index, 1);
        return -1;
    }

    record = field_find_free_inventory_record();
    item_template = &table->templates[index];
    func_800B2844(0, item_template, 0x15);
    if (record != NULL)
    {
        field_copy_inventory_record(record, item_template);
        return 0;
    }
    return -1;
}

/**
 * @brief Release one inventory record, or all of them for an index >= FIELD_ITEM_COUNT.
 * @param index Inventory record index.
 */
void func_800C2A88(s32 index)
{
    if (index < FIELD_ITEM_COUNT)
    {
        D_80122B74->items[index].kind = 0;
        field_compact_inventory();
    }
    else
    {
        func_800C2AD0();
    }
}

/**
 * @brief Release every inventory record and compact the inventory.
 * @return Always -1.
 */
s32 func_800C2AD0(void)
{
    s32 i;

    for (i = 0; i < FIELD_ITEM_COUNT; i++)
    {
        D_80122B74->items[i].kind = 0;
    }
    field_compact_inventory();
    return -1;
}
