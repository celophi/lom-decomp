/**
 * @file field_resource_table_ops.c
 * @brief Party script page lookups, encyclopedia entry bits and inventory record creation and release.
 */

#include "game_audio.h"
#include "common.h"
#include "field_calls.h"
#include "field_records.h"

/** @brief Resource id of the item template table. */
#define FIELD_RESOURCE_ITEM_TEMPLATES 5

/** @brief Diagnostic for a missing or out-of-range item template (detail 0 or 1). */
#define DIAG_BAD_ITEM_TEMPLATE 0x6C

/** @brief Character budget of text macro 0 when it names a new template item. */
#define FIELD_TEMPLATE_ITEM_MACRO_LIMIT 21

/** @brief Action table of a party script page: a count, then the descriptors. */
typedef struct
{
    u32 count;
    FieldActionDescriptor actions[1];
} FieldPartyActionTable;

/** @brief Item template table (resource FIELD_RESOURCE_ITEM_TEMPLATES). */
typedef struct
{
    u16 unk0;
    u16 count;
    FieldItemRecord templates[1];
} FieldItemTemplateTable;

static s32 field_discard_all_items(void);

void* func_800C1E40(s32 resource_id);
FieldItemRecord* field_find_free_inventory_record(void);

extern FieldGameState* g_field_game_state;

/**
 * @brief Return an event script of a party member.
 * @param page Party member (script page index).
 * @param entry Event script index.
 * @return Start of the script.
 */
u8* field_get_party_event_script(s32 page, u16 entry)
{
    FieldPartyScriptPage* base = &g_field_party_script_pages[page];
    u8* table = base->bytes + base->table_offsets[0];

    return table + ((s16*)table)[entry];
}

/**
 * @brief Return a private script of a party member.
 * @param page Party member (script page index).
 * @param entry Private script index.
 * @return Start of the script.
 */
u8* field_get_party_private_script(s32 page, u16 entry)
{
    FieldPartyScriptPage* base;
    s16* table;

    base = &g_field_party_script_pages[page];
    table = (s16*)(base->bytes + base->table_offsets[1]);
    return (u8*)table + table[entry];
}

/**
 * @brief Return an action descriptor of a party member.
 * @param page Party member (script page index).
 * @param index Action index.
 * @return The descriptor, or NULL when @p index is out of range.
 */
FieldActionDescriptor* field_get_party_action(s32 page, u16 index)
{
    FieldPartyScriptPage* base;
    FieldPartyActionTable* table;

    base = &g_field_party_script_pages[page];
    table = (FieldPartyActionTable*)(base->bytes + base->table_offsets[2]);
    if (index < table->count)
    {
        return &table->actions[index];
    }
    return NULL;
}

/**
 * @brief Unlock an encyclopedia entry.
 * @param entry Entry bit index.
 */
void field_unlock_encyclopedia_entry(u32 entry)
{
    g_field_game_state->encyclopedia_bits[entry / 32] |= 1 << (entry % 32);
}

/**
 * @brief Create an inventory item from a template and name it in text macro 0.
 * @param index Template index.
 * @return 0 on success, -1 when the template is missing or the inventory is full.
 */
s32 field_add_template_item(s32 index)
{
    FieldItemTemplateTable* table;
    FieldItemRecord* item_template;
    FieldItemRecord* record;

    table = func_800C1E40(FIELD_RESOURCE_ITEM_TEMPLATES);
    if (table == NULL)
    {
        record_game_diagnostic(DIAG_ERROR, DIAG_BAD_ITEM_TEMPLATE, index, 0);
        return -1;
    }
    if (index >= table->count)
    {
        record_game_diagnostic(DIAG_ERROR, DIAG_BAD_ITEM_TEMPLATE, index, 1);
        return -1;
    }

    record = field_find_free_inventory_record();
    item_template = &table->templates[index];
    field_set_text_macro(0, (u8*)item_template, FIELD_TEMPLATE_ITEM_MACRO_LIMIT);
    if (record != NULL)
    {
        field_copy_inventory_record((u8*)record, (u8*)item_template);
        return 0;
    }
    return -1;
}

/**
 * @brief Discard one inventory item, or all of them for an index of FIELD_ITEM_COUNT or more.
 * @param index Inventory record index.
 */
void field_discard_item(s32 index)
{
    if (index < FIELD_ITEM_COUNT)
    {
        g_field_game_state->items[index].kind = 0;
        field_compact_inventory();
    }
    else
    {
        field_discard_all_items();
    }
}

/**
 * @brief Discard every inventory item.
 * @return Always -1.
 */
static s32 field_discard_all_items(void)
{
    s32 i;

    for (i = 0; i < FIELD_ITEM_COUNT; i++)
    {
        g_field_game_state->items[i].kind = 0;
    }
    field_compact_inventory();
    return -1;
}
