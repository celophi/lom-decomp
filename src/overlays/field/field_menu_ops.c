#include "game_audio.h"
#include "saved_game.h"
#include "common.h"
#include "field_calls.h"
#include "field_menu_vars.h"
#include "main.h"

/**
 * @brief The game-state workspace viewed as the pad context that main.h maps.
 * @note An object (not a pointer) view: indexed arrays in it are summed index
 *       first, like arrays of a global.
 */
#define FIELD_PAD (*(PadContext*)&g_saved_game)

/*
 * Golem bytes of the pad context that main.h does not map yet (inside
 * _pad26E0 and _pad29DB, and byte 1 of unkAA8). They are read as bytes and
 * as parts of words, so they are indexed through g_saved_game.bytes.
 */
/** @brief Offset of the packed golem counts word; its low nibble is the golem count. */
#define FIELD_GOLEM_COUNTS 0x29D4

/** @brief Offset of the golem creation counter, byte 1 of the golem counts word. */
#define FIELD_GOLEM_CREATED 0x29D5

/** @brief Offset of the packed golem display order, three 2-bit slot indices. */
#define FIELD_GOLEM_DISPLAY_ORDER 0x29DB

/** @brief Offset of the active golem's class, byte 1 of unkAA8. */
#define FIELD_ACTIVE_GOLEM_CLASS 0xAA9

/** @brief The object-menu variables, which start at D_80122C0C. */
#define FIELD_MENU_OBJECT ((FieldMenuObjectVars*)&D_80122C0C)

/** @brief The record display variables, which start at D_80122C10. */
#define FIELD_MENU_RECORD ((FieldMenuRecordVars*)&D_80122C10)

/** @brief The gosub result variables, which start at D_80122C12. */
#define FIELD_MENU_RESULT ((FieldMenuResultVars*)&D_80122C12)

/** @brief The named-selection variables, which start at D_80122C02. */
#define FIELD_MENU_NAMED ((FieldMenuNamedVars*)&D_80122C02)

/** @brief The golem swap variables, which start at D_80122C0A. */
#define FIELD_MENU_SWAP ((FieldMenuSwapVars*)&D_80122C0A)

/**
 * @brief Byte script local variable @p index (the locals start at D_80122C00).
 * @note Menu programs pass their arguments in these locals by index.
 */
#define FIELD_LOCAL_BYTE(index) (((u8*)&D_80122C00)[index])

/** @brief Halfword script local variable @p index. */
#define FIELD_LOCAL_HALF(index) (((s16*)&D_80122C00)[index])

/** @brief Word script local variable @p index. */
#define FIELD_LOCAL_WORD(index) (((s32*)&D_80122C00)[index])

/** @brief @p value clamped to @p low..@p high. */
#define FIELD_CLAMP(value, low, high) ((value) < (low) ? (low) : ((value) > (high) ? (high) : (value)))

/** @brief Number of menu operations in g_field_menu_ops. */
#define FIELD_MENU_OP_COUNT 0x60

/** @brief record_game_diagnostic status of a menu-op error; the code is the menu-op number. */
#define FIELD_DIAG_MENU_OP 0x8002

/* Menu-op numbers (g_field_menu_ops indices) that report errors. */
#define FIELD_MENU_OP_ITEM_NAME_ROW 0x22
#define FIELD_MENU_OP_DESCRIBE_SELECTED_HISTORY 0x27
#define FIELD_MENU_OP_RESTRICT_SELECTED_HISTORY 0x29
#define FIELD_MENU_OP_PUBLISH_HISTORY_SLOTS 0x2E
#define FIELD_MENU_OP_ADD_HISTORY_SLOT 0x30
#define FIELD_MENU_OP_UNRESTRICT_CURRENT_HISTORY 0x32
#define FIELD_MENU_OP_UNRESTRICT_CURSOR_HISTORY 0x4B
#define FIELD_MENU_OP_SET_HISTORY_ENTRY 0x4C

/* Script locals read by field_menu_apply_affinity (byte indices unless noted). */
#define FIELD_AFFINITY_ELEMENT 3           /**< Element of the applied source. */
#define FIELD_AFFINITY_AMOUNT 4            /**< Amount added per affinity point. */
#define FIELD_AFFINITY_ELEMENTS 5          /**< Elements of the two values, one per nibble. */
#define FIELD_AFFINITY_CATEGORIES 6        /**< Four 2-bit categories: first, first alt, second, second alt. */
#define FIELD_AFFINITY_SELECTED_CATEGORY 7 /**< Category of the applied source. */
#define FIELD_AFFINITY_LEVEL 8
#define FIELD_AFFINITY_NEUTRAL 10          /**< Nonzero: every element factor is 2. */
#define FIELD_AFFINITY_FIRST_VALUE 8       /**< Halfword index (D_80122C10). */
#define FIELD_AFFINITY_SECOND_VALUE 9      /**< Halfword index (D_80122C12). */

/* Script locals written by field_menu_read_ring_selection (halfword indices). */
#define FIELD_RING_ENTRY 0xA /**< Selected ring entry. */
#define FIELD_RING_BUSY 0xB  /**< 1 while the ring selection is still running. */

/* Script locals read by field_menu_add_logic_block (halfword indices). */
#define FIELD_NEW_BLOCK_SHAPE 0xC
#define FIELD_NEW_BLOCK_QUANTITY 0xD
#define FIELD_NEW_BLOCK_ID 0xE

/** @brief FIELD_NEW_BLOCK_ID value that clears the logic-block table. */
#define FIELD_NEW_BLOCK_CLEAR 0xFF

/* Script locals of the action item menus (byte indices unless noted). */
#define FIELD_ACTION_ITEM_COUNTS 0     /**< Eight bytes: owned count of each action item. */
#define FIELD_ACTION_USED_SLOTS 0xB    /**< Item slots in use in the active group. */
#define FIELD_ACTION_GROUP_FULL 0xC    /**< Set when fewer than three item slots stay free. */
#define FIELD_ACTION_SELECTED_ITEM 0xA /**< Halfword index: chosen action item, 0-7. */

/** @brief Size of a land record (field_records.h FieldLandRecord). */
#define FIELD_LAND_RECORD_SIZE 12

/** @brief Game-state offset of land 0's element levels (FieldGameState lands[0].levels). */
#define FIELD_LAND_LEVELS_OFFSET 0x2F4

/** @brief Weight of each element in field_menu_draw_land_element's uniform mode (3^4). */
#define FIELD_ELEMENT_UNIFORM_WEIGHT 81

/** @brief Total weight of the eight elements in uniform mode. */
#define FIELD_ELEMENT_UNIFORM_TOTAL (8 * FIELD_ELEMENT_UNIFORM_WEIGHT)

/** @brief Element value for no element. */
#define FIELD_NO_ELEMENT 0xFF

/** @brief Mystic Card id of an empty card slot. */
#define FIELD_NO_CARD 0xFF

/* Script locals of the pending and shared item record menus (byte indices unless noted). */
#define FIELD_RECORD_INDEX 2       /**< Selected record. */
#define FIELD_RECORD_STATUS 3      /**< 0 shown, 1 empty, 2 duplicate. */
#define FIELD_PENDING_COUNT 6      /**< Number of active pending records. */
#define FIELD_RECORD_RESULT 2      /**< Word index: pending result given to a set-aside record. */
#define FIELD_SET_ASIDE_ACTIVE 0x19 /**< Four bytes: saved active byte of each set-aside shared record. */

/* Script locals of the small history entry menus (byte indices). */
#define FIELD_HISTORY_ENTRY_ID 0x11
#define FIELD_HISTORY_INDEX 0x12
#define FIELD_UNRESTRICT_SLOT 0x19 /**< Cursor slot whose record field_menu_unrestrict_cursor_history frees. */

/** @brief Halfword local index of the three card ids left over by field_menu_remove_record_cards. */
#define FIELD_LEFTOVER_CARDS 3

/* Script locals written by field_menu_refresh_golem_order (halfword indices). */
#define FIELD_GOLEM_SLOT_STATUS 3      /**< Three halfwords: golem record per slot, 3 for none or active. */
#define FIELD_GOLEM_ACTIVE_POSITION 0xE /**< Display position of the selected slot. */
#define FIELD_GOLEM_ACTIVE_RECORD 0xF  /**< Active golem record index. */

/** @brief Byte local index of the four values field_menu_describe_selected_item stores. */
#define FIELD_ITEM_DESC_INDEX 1

/* Script locals written by field_menu_draw_land_element (byte indices). */
#define FIELD_DRAWN_ELEMENT 5   /**< Element drawn by weight (mode 0). */
#define FIELD_UNIFORM_ELEMENT 6 /**< Element drawn uniformly (other modes). */

/* Script locals of the small history cursor menus (field_menu_swap_cursor_history, field_menu_publish_cursor_history). */
#define FIELD_CURSOR_ENTRY_ID 0xA     /**< Halfword index: entry id of the swapped-in record. */
#define FIELD_CURSOR_SLOT 0x1C        /**< Selected cursor slot; also the text macro index. */
#define FIELD_CURSOR_SLOT_TABLE 0x1D  /**< Small history index shown in each cursor slot. */

/** @brief Text resource @p id. */
#define FIELD_TEXT_RESOURCE(id) ((FieldTextResource*)func_800C1E40(id))

/** @brief Offset of text @p index in text resource @p id; reads the resource twice. */
#define FIELD_TEXT_OFFSET(id, index) (FIELD_TEXT_RESOURCE(id)->texts[(index) * 2] + (FIELD_TEXT_RESOURCE(id)->texts[(index) * 2 + 1] << 8))

/** @brief Text resource of the menu objects' names. */
#define FIELD_OBJECT_TEXT_RESOURCE 0x101

/** @brief Text resource indexed by an item's stat bytes 0 and 1 (row * 14 + column). */
#define FIELD_ITEM_ENTRY_TEXT_RESOURCE 0x103

/** @brief Text resource indexed by an item's stat byte 1. */
#define FIELD_ITEM_COLUMN_TEXT_RESOURCE 0x104

/** @brief Resource of 0x40-byte item records read by field_menu_copy_resource_item. */
#define FIELD_ITEM_RECORD_RESOURCE 5

/** @brief Byte offset of 0x40-byte record @p index in a resource with a 4-byte header. */
#define FIELD_RESOURCE_RECORD_OFFSET(index) ((index) * sizeof(InventoryRecord) + 4)

/** @brief Largest value field_menu_apply_affinity stores. */
#define FIELD_AFFINITY_VALUE_MAX 0x7FFF

/** @brief The game-state workspace viewed as FieldMenuHistoryData. */
#define FIELD_MENU_HISTORY ((FieldMenuHistoryData*)g_saved_game.bytes)

/**
 * @brief Text @p id of the menu text table at D_800F0E98.
 * @note The table starts with little-endian 16-bit offsets relative to itself.
 */
#define FIELD_MENU_TEXT(id) (D_800F0E98[(id) * 2] + (D_800F0E98[(id) * 2 + 1] << 8) + D_800F0E98)

/** @brief Item id of the first of the eight menu action items. */
#define FIELD_ACTION_ITEM_BASE 0x58

/** @brief The game-state workspace viewed as FieldMenuActionData. */
#define FIELD_MENU_ACTIONS (*(FieldMenuActionData*)&g_saved_game)

/** @brief The action item choice variables, which start at D_80122C08. */
#define FIELD_MENU_CHOICES ((FieldMenuChoiceVars*)&D_80122C08)

/** @brief The record selection variables, which start at D_80122C02. */
#define FIELD_MENU_RECORD_SELECT ((FieldMenuRecordSelectVars*)&D_80122C02)

/** @brief The item description variables, which start at D_80122C04. */
#define FIELD_MENU_ITEM_INFO ((FieldMenuItemInfoVars*)&D_80122C04)

/** @brief The action slot variables, which start at D_80122C0D. */
#define FIELD_MENU_ACTION_SLOT ((FieldMenuActionSlotVars*)&D_80122C0D)

/** @brief The game-state workspace viewed as FieldMenuItemData. */
#define FIELD_MENU_ITEMS ((FieldMenuItemData*)g_saved_game.bytes)

/** @brief Script variables from D_80122C0C used by the menus that move a field object. */
typedef struct
{
    s16 object_id;
    s16 variant;
    s16 result;
    s16 height_offset;
    s16 target_x;
    s16 target_z;
    s16 target_height;
} FieldMenuObjectVars;

/** @brief Script variables from D_80122C10 that receive a record's display id. */
typedef struct
{
    s16 display_id;
    s16 mode;
} FieldMenuRecordVars;

/** @brief Script variables from D_80122C12 that receive a gosub selection. */
typedef struct
{
    s16 index;
    s16 unk02;
    u16 count;
} FieldMenuResultVars;

/** @brief Script variables from D_80122C02 used to name a gosub selection. */
typedef struct
{
    u8 macro_index;
    u8 unk01[0xD];
    s16 index;
    u8 unk10[4];
    u16 count;
} FieldMenuNamedVars;

/** @brief Table at D_80051CBC mapping selection ids from 0x60 to palette slots. */
typedef struct
{
    u8 slots[0x25];
} FieldPaletteSlotTable;

/** @brief Script variables from D_80122C0A that receive a golem slot swap. */
typedef struct
{
    s16 selected_position;
    u8 unk02[0x10];
    s16 active_slot;
} FieldMenuSwapVars;

/** @brief Per-logic-block class table (D_80051CE4), indexed by logic-block id. */
typedef struct
{
    s32 classes[0xE8 / 4];
} FieldLogicClassTable;

/** @brief Three-word gosub screen sequence descriptor passed to field_open_gosub_screen_sequence. */
typedef struct
{
    s32 words[3];
} FieldGosubSequence;

/**
 * @brief Small history record (main.h SmallHistoryRecord) with its 0x44 word split into bytes.
 * @note The low three bytes of the selection word hold extra slot ids (0xFE/0xFF are empty).
 */
typedef struct
{
    u8 name[0x15];
    u8 entry_id; /**< 0x15: id stored for the record's menu entry. */
    u8 unknown_0x16[0x44 - 0x16];
    u8 extra_slots[3]; /**< 0x44: extra slot ids; 0xFE and 0xFF mark an unused slot. */
    u8 selection_bits; /**< 0x47: bits 30-31 of the selection word. */
    u8 unknown_0x48[0x5A - 0x48];
    u16 unknown_0x5A;
    u8 unknown_0x5C[4];
} FieldSmallHistoryRecord;

/** @brief Pad context view whose small history records use FieldSmallHistoryRecord. */
typedef struct
{
    u8 _pad0000[0x2EF4];
    FieldSmallHistoryRecord small_history_records[SMALL_HISTORY_RECORD_COUNT];
} FieldMenuHistoryData;

/** @brief One 0x10-byte action slot of a menu action group. */
typedef struct
{
    s32 handle;
    union
    {
        u32 word;
        u8 item_index; /**< Action item (index into item_counts); 0xFF when the slot is free. */
    } entry;
    u8 counters[8]; /**< Per-slot counters advanced by field_menu_advance_action_counters. */
} FieldMenuActionSlot;

/**
 * @brief One 0x8C-byte menu action group of the pad context, starting at 0x26E4.
 * @note flags bits 0-3 hold the slot capacity, bits 8-11 the item slot count
 *       and bits 12-15 the number of item slots in use.
 */
typedef struct
{
    u32 flags;
    u32 unknown_0x04;
    u8 item_slots[4]; /**< 0x08: item ids of the group; 0xFF marks an empty entry. */
    FieldMenuActionSlot slots[8];
} FieldMenuActionGroup;

/** @brief Pad context view exposing the four menu action groups. */
typedef struct
{
    u8 _pad0000[0x26E4];
    FieldMenuActionGroup groups[4];
} FieldMenuActionData;

/** @brief Script variables from D_80122C08: the action item choice mask and count. */
typedef struct
{
    u16 mask; /**< Bit set for each choice that is disabled. */
    u8 count;
} FieldMenuChoiceVars;

/** @brief Script variables from D_80122C0D that select one menu action slot. */
typedef struct
{
    u8 action_slot; /**< Action slot number plus 4. */
    u8 unk01[0xE];
    u8 item_index; /**< 0x0F (D_80122C1C): item of the slot, read by field_menu_load_action_slot. */
    u8 handle;     /**< 0x10 (D_80122C1D): low byte of the slot handle. */
    u8 unk11;
    u8 group; /**< 0x12 (D_80122C1F): action group index. */
} FieldMenuActionSlotVars;

/** @brief Script variables from D_80122C02 that select a record and report the result. */
typedef struct
{
    u8 index;  /**< Selected record index. */
    u8 status; /**< 0 when the record is shown, 1 when it is empty, 2 when it is a duplicate. */
} FieldMenuRecordSelectVars;

/** @brief Script variables from D_80122C04 that describe the selected item. */
typedef struct
{
    u8 kind;     /**< Item kind, bits 9:8 of the attributes. */
    u8 category; /**< Category with the kind's table offset added. */
    u16 value;   /**< Displayed stat value; bit 15 marks a special item. */
    u32 amount;  /**< Item value. */
} FieldMenuItemInfoVars;

/** @brief 0x40-byte item record (main.h InventoryRecord) with its identity words. */
typedef struct
{
    u8 active; /**< Zero marks an empty record. */
    u8 unknown_0x01[0x13];
    InventoryAttributes attributes; /**< 0x14: item kind, category and name index. */
    u32 nibbles[2];                 /**< 0x18: sixteen 4-bit values. */
    u8 saved_active;                /**< 0x20: active byte kept while the record is set aside. */
    u8 unknown_0x21[3];
    union
    {
        u16 values[4];
        u8 bytes[8];
    } stats;              /**< 0x24: stats, read as halfwords or bytes by kind. */
    u8 unknown_0x2C[8];
    s32 unknown_0x34;     /**< 0x34: item value; for a pending record, its pending result. */
    u32 identity[2];      /**< 0x38: words that identify the item; both zero for no item. */
} FieldMenuItemRecord;

/**
 * @brief Pad context view of the item records.
 * @note The equipment block at 0x640 is scanned as eight records, twice
 *       main.h's PLAYER_EQUIPMENT_SLOT_COUNT.
 */
typedef struct
{
    u8 _pad0000[0x640];
    FieldMenuItemRecord equipment[8];
    u8 _pad0840[0xCE0 - 0x840];
    FieldMenuItemRecord inventory[INVENTORY_RECORD_COUNT];
    u8 _pad25E0[0x3160 - 0x25E0];
    FieldMenuItemRecord pending[4];
} FieldMenuItemData;

/** @brief Actor position as field_get_actor_position returns it. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
} FieldPosition;

/** @brief Element table (D_80051ED8) indexed by the game-state byte at 0x2E6 (low 7 bits). */
typedef struct
{
    s32 elements[8];
} FieldFavoredElementTable;

/** @brief Game-state view read by field_menu_draw_land_element. */
typedef struct
{
    u8 unk000[0x2E6];
    u16 favored_index; /**< Low 7 bits index FieldFavoredElementTable. */
} FieldElementLevelView;

/** @brief One local that holds the game-state view, then the chosen element. */
typedef union
{
    FieldElementLevelView* view;
    s32 element;
} FieldViewOrElement;

/** @brief Element table indexed by element: entry e is the element paired with e (0-8). */
typedef struct
{
    s32 elements[9];
} FieldElementTable;

/**
 * @brief A text resource (func_800C1E40): a 4-byte header, then the texts.
 * @note The texts start with a table of little-endian 16-bit offsets, one per
 *       text, relative to the start of the texts.
 */
typedef struct
{
    u8 header[4];
    u8 texts[1];
} FieldTextResource;

u8* field_find_free_inventory_record();
void field_get_actor_position();
s32 field_set_actor_position();
extern u8 D_80122C01;
extern u8 D_80122C02;
extern u8 D_80122C03;
extern u8 D_80122C04;
extern u8 D_80122C05;
extern s16 D_80122C06;
extern s16 D_80122C08;
extern s16 D_80122C0A;
extern u8 D_80122C0C;
extern u8 D_80122C0D;
extern u16 D_80122C0E;
extern u8 D_80122C0F;
extern s16 D_80122C10;
extern u8 D_80122C11;
extern s8 D_80122C12;
extern s16 D_80122C14;
extern u16 D_80122C16;
extern u8 D_80122C19;
extern s16 D_80122C1C;
extern u8 D_80122C1E;
extern u8 D_80122C1F;
extern FieldMenuItemRecord g_field_shared_items[4];
extern FieldGosubSequence D_80051EB4;
extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values[];
extern void (*g_field_menu_ops[])(s32 arg0);
extern s32 D_801227F0;
extern FieldGosubSequence D_800F19AC;
extern FieldGosubSequence D_800F19B8;
extern FieldGosubSequence D_800F19C4;
extern FieldPaletteSlotTable D_80051CBC;
extern FieldGosubSequence D_800F19CC;
extern FieldLogicClassTable D_80051CE4;
extern FieldLogicClassTable D_80051DCC;
u8* func_800C1E40(s32 arg0);
extern FieldGosubSequence D_80051EC0;
extern FieldGosubSequence D_80051ECC;
extern u8 D_800F0E98[];
extern void field_menu_clear_item_slots(void);
extern s32 rand(void);
s32 field_get_actor_facing(s32 arg0);
void func_800C2A88(s32 arg0);
extern s32 D_8011F428;
extern void field_menu_compact_pending_items(void);
extern s32 func_800BD414(s32 arg0, s32 arg1);
extern void func_800BD520(s32 arg0, s32 arg1, s32 arg2);
extern FieldFavoredElementTable D_80051ED8;
extern u16 g_music_track_index;
extern FieldGosubSequence D_80051EF8;
extern FieldElementTable D_80051F04;
void field_menu_clear_gosub_request(void);

/**
 * @brief Run one menu operation from the menu-op handler table.
 * @param op Menu-op index; indices of 0x60 and above record a diagnostic instead.
 */
void field_run_menu_op(s32 op)
{
    if (op < FIELD_MENU_OP_COUNT)
    {
        g_field_menu_ops[op](op);
        return;
    }
    record_game_diagnostic(FIELD_DIAG_MENU_OP, op, 0, 0);
}

/**
 * @brief Classify the selected golem slot against the active golem.
 *
 * Stores 1 (empty slot, no active golem) or 2 (empty slot, a golem is active)
 * when the selected slot holds no golem, otherwise 3 when the slot holds the
 * active golem and 4 when it does not.
 */
void field_menu_classify_golem_slot(void)
{
    PadContext* ctx;

    ctx = &FIELD_PAD;
    if (ctx->large_history_order[D_80122C00.golem.slot] == 3)
    {
        if ((u8)ctx->large_history_index >= 3U)
        {
            D_80122C00.golem.state = 1;
            return;
        }
        D_80122C00.golem.state = 2;
        return;
    }
    if (ctx->large_history_order[D_80122C00.golem.slot] == ctx->large_history_index)
    {
        D_80122C00.golem.state = 3;
        return;
    }
    D_80122C00.golem.state = 4;
}

/**
 * @brief Clear the gosub-screen request word.
 */
void field_menu_clear_gosub_request(void)
{
    D_801227F0 = 0;
}

/**
 * @brief Open the gosub screen sequence described by D_800F19AC.
 */
void func_800C57E0(void)
{
    field_open_gosub_screen_sequence(&D_800F19AC);
}

/**
 * @brief Create a golem in the selected slot from the gosub-selected items.
 *
 * Picks a golem record that no slot uses and initializes it, bumps the golem
 * counters, moves the slot's previous golem to the first empty slot, then moves
 * the selected inventory records into the new golem's four item records and
 * clears the rest.
 */
void field_menu_create_golem(void)
{
    s32 packed_counts;
    s32 slot;
    s32 candidate;
    s32 i;
    s32 free_record;
    u8 count;
    u8 previous;

    free_record = 3;
    for (i = 0; i < 3; i++)
    {
        candidate = i;
        for (slot = 0; slot < 3; slot++)
        {
            if (FIELD_PAD.large_history_order[slot] == i)
            {
                candidate = 3;
            }
        }
        if (candidate != 3)
        {
            free_record = candidate;
        }
    }
    if (free_record != 3)
    {
        field_golem_build_group_record(free_record);
        count = g_saved_game.bytes[FIELD_GOLEM_CREATED] + 1;
        g_saved_game.bytes[FIELD_GOLEM_CREATED] = count;
        if (count >= 201)
        {
            g_saved_game.bytes[FIELD_GOLEM_CREATED] = 200;
        }
        previous = FIELD_PAD.large_history_order[D_80122C00.golem.slot];
        if (previous != 3)
        {
            if (FIELD_PAD.large_history_order[0] == 3)
            {
                FIELD_PAD.large_history_order[0] = previous;
            }
            else if (FIELD_PAD.large_history_order[1] == 3)
            {
                FIELD_PAD.large_history_order[1] = previous;
            }
            else if (FIELD_PAD.large_history_order[2] == 3)
            {
                FIELD_PAD.large_history_order[2] = previous;
            }
        }
        FIELD_PAD.large_history_order[D_80122C00.golem.slot] = free_record;
        packed_counts = (*(s32*)&g_saved_game.bytes[FIELD_GOLEM_COUNTS] & ~0xF) | (((g_saved_game.bytes[FIELD_GOLEM_COUNTS] & 0xF) + 1) & 0xF);
        *(s32*)&g_saved_game.bytes[FIELD_GOLEM_COUNTS] = packed_counts;
        if ((g_saved_game.bytes[FIELD_GOLEM_COUNTS] & 0xF) >= 4)
        {
            *(s32*)&g_saved_game.bytes[FIELD_GOLEM_COUNTS] = (packed_counts & ~0xF) | 3;
        }
        for (i = 0; i < g_gosub_result_count; i++)
        {
            field_copy_inventory_record(&FIELD_PAD.large_history_records[FIELD_PAD.large_history_order[D_80122C00.golem.slot]].unknown_0x4C[i << 6],
                                        (u8*)&FIELD_PAD.inventory[g_gosub_result_values[i]]);
            FIELD_PAD.inventory[g_gosub_result_values[i]].active = 0;
        }
        field_compact_inventory();
        for (i = g_gosub_result_count; i < 4; i++)
        {
            FIELD_PAD.large_history_records[FIELD_PAD.large_history_order[D_80122C00.golem.slot]].unknown_0x4C[i << 6] = 0;
        }
        func_800A54D0();
    }
}

/**
 * @brief Open the gosub screen sequence described by D_800F19B8.
 */
void func_800C5AA8(void)
{
    field_open_gosub_screen_sequence(&D_800F19B8);
}

/**
 * @brief Open the gosub screen sequence described by D_800F19C4.
 */
void func_800C5ACC(void)
{
    field_open_gosub_screen_sequence(&D_800F19C4);
}

/**
 * @brief Run field_run_golem as a menu operation.
 */
void func_800C5AF0(void)
{
    field_run_golem();
}

/**
 * @brief Store the selected slot's golem record index and its class nibble.
 */
void field_menu_load_golem_class(void)
{
    PadContext* ctx;
    s32 record_index;

    ctx = &FIELD_PAD;
    record_index = ctx->large_history_order[D_80122C00.golem.slot];
    D_80122C00.golem.slot_status[0] = record_index;
    D_80122C00.golem.detail = ctx->large_history_records[record_index].unknown_0x44 & 0xF;
}

/**
 * @brief Report whether the golem menu may create, or only view, a golem.
 *
 * Clamps an out-of-range active golem index to 3 (none), then stores 2 when the
 * low seven bits of the word at 0xAA8 equal 3, 0 when no golem is active and 1
 * otherwise.
 */
void field_menu_check_golem_creation(void)
{
    if ((u8)FIELD_PAD.large_history_index >= 4U)
    {
        FIELD_PAD.large_history_index = 3;
    }
    if ((FIELD_PAD.unkAA8 & 0x7F) == 3)
    {
        D_80122C00.golem.result = 2;
    }
    else if (FIELD_PAD.large_history_index == 3)
    {
        D_80122C00.golem.result = 0;
    }
    else
    {
        D_80122C00.golem.result = 1;
    }
}

/**
 * @brief Dismiss the selected slot's golem and return its four items.
 *
 * Unassigns every logic block that belongs to the golem, moves its item records
 * back to the inventory while there is room, clears the golem record, empties
 * the slot and decrements the golem count.
 */
void field_menu_dismiss_golem(void)
{
    s32 i;
    u32 word;

    for (i = 0; i < FIELD_PAD.logic_block_count; i++)
    {
        word = FIELD_PAD.logic_blocks[i].word;
        if ((word & 3) == FIELD_PAD.large_history_order[D_80122C00.golem.slot])
        {
            FIELD_PAD.logic_blocks[i].word = (word | 3) & 0xFFFEFFFF;
        }
    }
    for (i = 0; i < 4; i++)
    {
        if (FIELD_PAD.large_history_records[FIELD_PAD.large_history_order[D_80122C00.golem.slot]].unknown_0x4C[i * 0x40] != 0)
        {
            if (field_find_free_inventory_record() != 0)
            {
                field_copy_inventory_record(field_find_free_inventory_record(),
                                            &FIELD_PAD.large_history_records[FIELD_PAD.large_history_order[D_80122C00.golem.slot]].unknown_0x4C[i * 0x40]);
            }
        }
    }
    FIELD_PAD.large_history_records[FIELD_PAD.large_history_order[D_80122C00.golem.slot]].name[0] = 0;
    FIELD_PAD.large_history_order[D_80122C00.golem.slot] = 3;
    word = *(u32*)&g_saved_game.bytes[FIELD_GOLEM_COUNTS];
    if ((word & 0xF) != 0)
    {
        *(u32*)&g_saved_game.bytes[FIELD_GOLEM_COUNTS] = (word & ~0xF) | (((g_saved_game.bytes[FIELD_GOLEM_COUNTS] & 0xF) - 1) & 0xF);
    }
}

/**
 * @brief Publish the selected slot's golem record to text macro 0.
 */
void field_menu_publish_golem_name(void)
{
    u8 record_index = FIELD_PAD.large_history_order[D_80122C00.golem.slot];

    field_set_text_macro(0, (u8*)&FIELD_PAD.large_history_records[record_index], 0xFF);
}

/**
 * @brief Clear the golem menu result when the gosub screen returned nothing.
 */
void field_menu_clear_golem_result(void)
{
    if (g_gosub_result_count == 0)
    {
        D_80122C00.golem.result = 0;
    }
}

/**
 * @brief Repair the group display order and publish the current selection state.
 */
void field_menu_refresh_golem_order(void)
{
    s32 order[3];
    s32 inverse[3];
    s32* inverse_entry;
    s32 packed_order;
    s32 clamped_order_0;
    s32 clamped_order_1;
    s32 clamped_order_2;
    s32 i;
    s32 active_index;
    s8 selected_index;
    s32 j;

    FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS) = 3;
    FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS + 1) = 3;
    FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS + 2) = 3;
    if (FIELD_PAD.large_history_order[0] != FIELD_PAD.large_history_index)
    {
        FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS) = FIELD_PAD.large_history_order[0];
    }
    if (FIELD_PAD.large_history_order[1] != FIELD_PAD.large_history_index)
    {
        FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS + 1) = FIELD_PAD.large_history_order[1];
    }
    if (FIELD_PAD.large_history_order[2] != FIELD_PAD.large_history_index)
    {
        FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS + 2) = FIELD_PAD.large_history_order[2];
    }
    packed_order = g_saved_game.bytes[FIELD_GOLEM_DISPLAY_ORDER];
    i = packed_order & 3;
    order[1] = (packed_order >> 2) & 3;
    order[0] = i;
    order[2] = (packed_order >> 4) & 3;
    if (i >= 0)
    {
        clamped_order_0 = 2;
        if (i < 3)
        {
            clamped_order_0 = i;
        }
    }
    else
    {
        clamped_order_0 = 0;
    }
    order[0] = clamped_order_0;
    if (order[1] >= 0)
    {
        clamped_order_1 = 2;
        if (order[1] < 3)
        {
            clamped_order_1 = order[1];
        }
    }
    else
    {
        clamped_order_1 = 0;
    }
    order[1] = clamped_order_1;
    if (order[2] >= 0)
    {
        clamped_order_2 = 2;
        if (order[2] < 3)
        {
            clamped_order_2 = order[2];
        }
    }
    else
    {
        clamped_order_2 = 0;
    }
    order[2] = clamped_order_2;
    inverse[0] = 3;
    inverse[1] = 3;
    inverse[2] = 3;
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            if (order[i] == j)
            {
                if (inverse[j] == 3)
                {
                    inverse[j] = i;
                }
                else
                {
                    order[i] = 3;
                }
            }
        }
    }
    for (i = 0; i < 3; i++)
    {
        if (order[i] == 3)
        {
            for (j = 0; j < 3; j++)
            {
                inverse_entry = &inverse[j];
                if (*inverse_entry == 3)
                {
                    order[i] = j;
                    j = 3;
                    *inverse_entry = i;
                }
            }
        }
    }
    packed_order = order[0] + (order[1] * 4) + (order[2] * 0x10);
    g_saved_game.bytes[FIELD_GOLEM_DISPLAY_ORDER] = packed_order;
    if (FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS) == 3)
    {
        if (order[0] == 0)
        {
            FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS) = 4;
        }
        if (order[1] == 0)
        {
            FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS) = 5;
        }
        if (order[2] == 0)
        {
            FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS) = 6;
        }
    }
    if (FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS + 1) == 3)
    {
        if (order[0] == 1)
        {
            FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS + 1) = 4;
        }
        if (order[1] == 1)
        {
            FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS + 1) = 5;
        }
        if (order[2] == 1)
        {
            FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS + 1) = 6;
        }
    }
    if (FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS + 2) == 3)
    {
        if (order[0] == 2)
        {
            FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS + 2) = 4;
        }
        if (order[1] == 2)
        {
            FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS + 2) = 5;
        }
        if (order[2] == 2)
        {
            FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS + 2) = 6;
        }
    }
    selected_index = FIELD_PAD.large_history_index;
    if (selected_index < 3)
    {
        if (FIELD_PAD.large_history_order[0] == selected_index)
        {
            FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS) = 3;
        }
        if (FIELD_PAD.large_history_order[1] == selected_index)
        {
            FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS + 1) = 3;
        }
        if (FIELD_PAD.large_history_order[2] == selected_index)
        {
            FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS + 2) = 3;
        }
    }
    active_index = D_80122C00.golem.slot;
    for (i = 0; i < 3; i++)
    {
        if (order[i] == active_index)
        {
            FIELD_LOCAL_HALF(FIELD_GOLEM_ACTIVE_POSITION) = i;
        }
    }
    FIELD_LOCAL_HALF(FIELD_GOLEM_ACTIVE_RECORD) = FIELD_PAD.large_history_index;
}

/**
 * @brief Report whether the logic-block table is full (40 blocks).
 */
void field_menu_check_logic_blocks_full(void)
{
    if (FIELD_PAD.logic_block_count >= LOGIC_BLOCK_CAPACITY)
    {
        D_80122C10 = 1;
    }
    else
    {
        D_80122C10 = 0;
    }
}

/**
 * @brief Run field_run_golem as a menu operation.
 */
void func_800C6208(void)
{
    field_run_golem();
}

/**
 * @brief Dispatch each populated row of the selected menu entry and count them.
 */
void field_menu_publish_golem_items(void)
{
    s32 count;
    s32 i;

    count = 0;
    for (i = 0; i < 4; i++)
    {
        if (FIELD_PAD.large_history_records[FIELD_PAD.large_history_order[D_80122C00.golem.slot]].unknown_0x4C[i << 6] != 0)
        {
            field_set_text_macro(count, &FIELD_PAD.large_history_records[FIELD_PAD.large_history_order[D_80122C00.golem.slot]].unknown_0x4C[i << 6], 0xFF);
            count += 1;
        }
    }
    D_80122C10 = count;
}

/**
 * @brief Subtract the number of free inventory records from the requested count, clamping at 0.
 */
void field_menu_count_missing_inventory_space(void)
{
    s32 i;
    s32 free_count;
    u16 remaining;
    s16* requested;

    free_count = 0;
    for (i = 0; i < INVENTORY_RECORD_COUNT; i++)
    {
        if (FIELD_PAD.inventory[i].active == 0)
        {
            free_count++;
        }
    }
    requested = &D_80122C10;
    if (free_count >= *requested)
    {
        remaining = 0;
    }
    else
    {
        remaining = (u16)*requested - free_count;
    }
    *requested = remaining;
}

/**
 * @brief Run field_golem_commit_group_edit with argument 0x92BC.
 */
void field_menu_commit_golem_group_edit(void)
{
    field_golem_commit_group_edit(0x92BC);
}

/**
 * @brief Pass the menu object's id and variant to field_reset_actor_at.
 */
void field_menu_reset_object(void)
{
    field_reset_actor_at(FIELD_MENU_OBJECT->object_id, FIELD_MENU_OBJECT->variant, FIELD_MENU_OBJECT->variant, 0, 0, 0);
}

/**
 * @brief Lower the menu object by its height offset, in whole position units.
 */
void field_menu_lower_object(void)
{
    s32 position[3];

    field_get_actor_position(FIELD_MENU_OBJECT->object_id, position);
    position[0] /= 256;
    position[1] /= 256;
    position[2] /= 256;
    position[1] -= FIELD_MENU_OBJECT->height_offset;
    field_set_actor_position(FIELD_MENU_OBJECT->object_id, position[0], position[1], position[2]);
}

/**
 * @brief Move the menu object one easing step toward its target position.
 *
 * Works in whole position units. While the object is at or above the target
 * height it closes two thirds of the horizontal distance per step (one unit
 * at a time when close); once it has arrived horizontally, or is below the
 * target height, the height is eased the same way.
 */
void field_menu_ease_object(void)
{
    s32 position[3];
    s32 target_y;
    s32 target_x;
    s32 target_z;
    s32 y;
    s32 delta;
    s32 distance;
    s32 x;

    field_get_actor_position(FIELD_MENU_OBJECT->object_id, position);
    x = position[0] / 256;
    position[0] = x;
    y = position[1] / 256;
    position[1] = y;
    position[2] /= 256;

    target_y = -FIELD_MENU_OBJECT->target_height;
    target_x = FIELD_MENU_OBJECT->target_x;
    target_z = FIELD_MENU_OBJECT->target_z;

    if (y == target_y || y < target_y)
    {
        delta = x - target_x;
        distance = delta;
        if (delta < 0)
        {
            /* Net-zero update: keeps the negation on the copy in distance. */
            delta++;
            delta--;
            distance = -distance;
        }
        if (distance * 2 >= 4)
        {
            position[0] = target_x + delta * 2 / 3;
        }
        else if (distance > 0)
        {
            position[0] = x - delta / distance;
        }
        else
        {
            position[0] = target_x;
        }
    }

    if (position[1] == target_y || position[1] < target_y)
    {
        delta = position[2] - target_z;
        distance = delta;
        if (delta < 0)
        {
            /* Net-zero update: keeps the negation on the copy in distance. */
            delta++;
            delta--;
            distance = -distance;
        }
        if (distance * 2 >= 4)
        {
            position[2] = target_z + delta * 2 / 3;
        }
        else if (distance > 0)
        {
            position[2] -= delta / distance;
        }
        else
        {
            position[2] = target_z;
        }
    }

    if ((position[0] == target_x && position[2] == target_z) || position[1] >= target_y)
    {
        delta = position[1] - target_y;
        distance = delta;
        if (delta < 0)
        {
            /* Net-zero update: keeps the negation on the copy in distance. */
            delta++;
            delta--;
            distance = -distance;
        }
        if (distance * 2 >= 4)
        {
            position[1] = target_y + delta * 2 / 3;
        }
        else if (distance > 0)
        {
            position[1] -= delta / distance;
        }
        else
        {
            position[1] = target_y;
        }
    }

    field_set_actor_position(FIELD_MENU_OBJECT->object_id, position[0], position[1], position[2]);
}

/**
 * @brief Store the gosub-selected palette entry's slot in the selected golem record.
 *
 * Looks the selection (an id from 0x60) up in the D_80051CBC table and stores
 * the result, clamped to 0..31, in the golem record's word at 0x48.
 */
void field_menu_set_golem_palette(void)
{
    FieldPaletteSlotTable table;
    s32 selection;
    u16 selection_low;
    s32 slot;
    s32 clamped;
    u8 record_index;

    table = D_80051CBC;
    selection = g_gosub_result_values[0];
    selection_low = *(u16*)g_gosub_result_values;
    slot = table.slots[selection - 0x60];
    D_80122C08 = selection_low;
    if (slot >= 32)
    {
        slot = 0;
    }
    if (slot >= 0)
    {
        clamped = 31;
        if (slot < 32)
        {
            clamped = slot;
        }
    }
    else
    {
        clamped = 0;
    }
    record_index = FIELD_PAD.large_history_order[D_80122C00.golem.slot];
    FIELD_PAD.large_history_records[record_index].unknown_0x48 = clamped;
    func_800A54D0();
}

/**
 * @brief Make the golem shown in slot status 0 the active golem.
 */
void field_menu_set_active_golem(void)
{
    s32 status = D_80122C06;

    g_game_diagnostic_status = status;
    FIELD_PAD.large_history_index = status;
}

/**
 * @brief Store the ring selection in local halfword 0xA, with a busy flag in 0xB.
 * @note While the ring is busy the flag is 1 and the value is the entry the
 *       ring cursor is on.
 */
void field_menu_read_ring_selection(void)
{
    s32 result = func_800A4744();

    if (result < 0)
    {
        FIELD_LOCAL_HALF(FIELD_RING_BUSY) = 1;
        /* Called as an int function (no prototype in the original), so the byte is not masked. */
        FIELD_LOCAL_HALF(FIELD_RING_ENTRY) = ((s32 (*)(void))func_800A4778)();
    }
    else
    {
        FIELD_LOCAL_HALF(FIELD_RING_BUSY) = 0;
        FIELD_LOCAL_HALF(FIELD_RING_ENTRY) = result;
    }
}

/**
 * @brief Open the gosub screen sequence described by D_800F19CC.
 */
void func_800C68A4(void)
{
    field_open_gosub_screen_sequence(&D_800F19CC);
}

/**
 * @brief Swap the selected golem slot with the active golem's slot in the display order.
 *
 * Stores the display position of the selected slot and the slot index of the
 * active golem for the menu script.
 */
void field_menu_swap_golem_order(void)
{
    s32 order[3];
    s32 selected_position;
    s32 active_slot;
    s32 packed_order;
    s32 selected_slot;
    s8 active_record;

    packed_order = g_saved_game.bytes[FIELD_GOLEM_DISPLAY_ORDER];
    selected_position = 3;
    active_slot = 3;
    order[0] = packed_order & 3;
    order[1] = (packed_order >> 2) & 3;
    order[2] = (packed_order >> 4) & 3;
    active_record = FIELD_PAD.large_history_index;
    selected_slot = D_80122C00.golem.slot;
    if (FIELD_PAD.large_history_order[selected_slot] != active_record)
    {
        s32 i;
        s32 swap_position;

        for (i = 0; i < 3; i++)
        {
            if (active_record == FIELD_PAD.large_history_order[i])
            {
                active_slot = i;
            }
        }
        for (i = 0; i < 3; i++)
        {
            if (order[i] == active_slot)
            {
                swap_position = i;
            }
        }
        for (i = 0; i < 3; i++)
        {
            if (order[i] == selected_slot)
            {
                selected_position = i;
            }
        }
        order[swap_position] = selected_slot;
        order[selected_position] = active_slot;
        packed_order = order[0] + (order[1] * 4) + (order[2] * 0x10);
        g_saved_game.bytes[FIELD_GOLEM_DISPLAY_ORDER] = packed_order;
    }
    FIELD_MENU_SWAP->selected_position = selected_position;
    FIELD_MENU_SWAP->active_slot = active_slot;
}

/**
 * @brief Publish the inventory record selected by the result variable to text macro 0.
 */
void field_menu_publish_inventory_item(void)
{
    field_set_text_macro(0, (u8*)&FIELD_PAD.inventory[D_80122C10], 0xFF);
}

/**
 * @brief Count the inventory records of the item kind in the result variable.
 */
void field_menu_count_inventory_kind(void)
{
    s32 i;
    s32 count;
    s32 kind;

    kind = D_80122C10;
    count = 0;
    for (i = 0; i < INVENTORY_RECORD_COUNT; i++)
    {
        if (FIELD_PAD.inventory[i].active != 0)
        {
            if (((FIELD_PAD.inventory[i].attributes.packed >> 8) & 3) == kind)
            {
                count++;
            }
        }
    }
    D_80122C10 = count;
}

/**
 * @brief Fill every free inventory record with a copy of the first record.
 *
 * Also advances the golem creation counter by 9.
 */
void field_menu_fill_free_inventory(void)
{
    while (field_find_free_inventory_record() != 0)
    {
        field_copy_inventory_record(field_find_free_inventory_record(), (u8*)FIELD_PAD.inventory);
    }
    g_saved_game.bytes[FIELD_GOLEM_CREATED] += 9;
}

/**
 * @brief Change the active golem's class and unassign its logic blocks that conflict.
 *
 * Stores the class in the variables at D_80122C1C into the active golem's
 * record, then unassigns every placed logic block of that golem whose class
 * entry in the D_80051CE4 table is set and differs from the new class.
 */
void field_menu_set_golem_class(void)
{
    FieldLogicClassTable class_table;
    FieldLogicClassTable unused_table;
    PadContext* ctx;
    s32 i;
    s32 active_record;
    s32 golem_class;
    u32 word;
    s32 block_class;
    PadContext* shifted;
    s32 placed;

    class_table = D_80051CE4;
    unused_table = D_80051DCC;

    ctx = &FIELD_PAD;
    active_record = ctx->large_history_index;
    golem_class = D_80122C1C;
    if (active_record < 3)
    {
        /* The pad context shifted by active_record records: record 0 is the active golem. */
        shifted = (PadContext*)((u8*)ctx + active_record * sizeof(LargeHistoryRecord));
        g_saved_game.bytes[FIELD_ACTIVE_GOLEM_CLASS] = (u8)D_80122C1C;
        *(s32*)&shifted->large_history_records[0].unknown_0x44 = (*(s32*)&shifted->large_history_records[0].unknown_0x44 & ~0xF) | ((u8)D_80122C1C & 0xF);
        for (i = 0; i < ctx->logic_block_count; i++)
        {
            word = ctx->logic_blocks[i].word;
            placed = (word >> 16) & 1;
            if (placed == 1 && (word & 3) == active_record)
            {
                block_class = class_table.classes[(word >> 2) & 0x3F];
                if (block_class != 0 && block_class != golem_class)
                {
                    ctx->logic_blocks[i].word = (word & 0xFFFEFFFF) | 3;
                }
            }
        }
        field_golem_rebuild_current_grid();
    }
}

/**
 * @brief Append a logic block built from script locals 0xC-0xE (halfwords).
 *
 * An id of 0xFF clears the logic-block table instead. The new block is
 * unassigned and not placed; nothing is added when the table is full.
 */
void field_menu_add_logic_block(void)
{
    s32 id = FIELD_LOCAL_HALF(FIELD_NEW_BLOCK_ID);
    s32 quantity = FIELD_LOCAL_HALF(FIELD_NEW_BLOCK_QUANTITY);
    s32 shape = FIELD_LOCAL_HALF(FIELD_NEW_BLOCK_SHAPE);

    if (id == FIELD_NEW_BLOCK_CLEAR)
    {
        FIELD_PAD.logic_block_count = 0;
        return;
    }

    if (FIELD_PAD.logic_block_count < LOGIC_BLOCK_CAPACITY)
    {
        FIELD_PAD.logic_blocks[FIELD_PAD.logic_block_count].f.id = id;
        FIELD_PAD.logic_blocks[FIELD_PAD.logic_block_count].f.quantity = quantity;
        FIELD_PAD.logic_blocks[FIELD_PAD.logic_block_count].f.shape = shape;
        FIELD_PAD.logic_blocks[FIELD_PAD.logic_block_count].f.logic_type = LOGIC_BLOCK_UNASSIGNED;
        FIELD_PAD.logic_blocks[FIELD_PAD.logic_block_count].f.placed = 0;
        FIELD_PAD.logic_block_count++;
    }
}

/**
 * @brief Pass the variable at D_80122C1C to func_800C9ED4.
 */
void func_800C6DA0(void)
{
    func_800C9ED4(D_80122C1C);
}

/**
 * @brief Open the gosub screen sequence described by D_80051EB4.
 */
void func_800C6DC8(void)
{
    FieldGosubSequence sequence;

    sequence = D_80051EB4;
    field_open_gosub_screen_sequence(&sequence);
}

/**
 * @brief Run field_open_carda with argument 0.
 */
void func_800C6E08(void)
{
    field_open_carda(0);
}

/**
 * @brief Replace the inventory record index in the result variable with its name-table row.
 *
 * The row is the record's category (bits 10-15 of its attribute word) offset by
 * its kind: +0 for kind 0, +0xB for kind 1 and +0x17 otherwise. A row of 0xFF
 * records a diagnostic and yields 0.
 */
void field_menu_item_name_row(void)
{
    s32 index;
    u32 attributes;
    s32 kind;
    s32 row;

    index = D_80122C10;
    attributes = FIELD_PAD.inventory[index].attributes.packed;
    kind = (attributes >> 8) & 3;
    if (kind == 0)
    {
        row = (attributes >> 10) & 0x3F;
    }
    else if (kind == 1)
    {
        row = ((attributes >> 10) & 0x3F) + 0xB;
    }
    else
    {
        row = ((attributes >> 10) & 0x3F) + 0x17;
    }
    if (row == 0xFF)
    {
        record_game_diagnostic(FIELD_DIAG_MENU_OP, FIELD_MENU_OP_ITEM_NAME_ROW, 0, 0);
        row = 0;
    }
    D_80122C10 = row;
}

/**
 * @brief Remove the inventory record's Mystic Cards from the menu's card list.
 *
 * Reads the three card ids of the inventory record selected by the result
 * variable. Each card-list entry that matches one of them is set to 0xFF and
 * that id is used up; the ids left over are stored at D_80122C06.
 */
void field_menu_remove_record_cards(void)
{
    s32 i;
    s32 index;
    u8 first;
    u8 second;
    u8 third;
    s16 card;

    index = D_80122C10;
    first = FIELD_PAD.inventory[index].unknown_0x18[8];
    second = FIELD_PAD.inventory[index].unknown_0x18[9];
    third = FIELD_PAD.inventory[index].unknown_0x18[10];
    for (i = 0; i < 3; i++)
    {
        card = D_80122C00.cards.card_ids[i];
        if (card != FIELD_NO_CARD)
        {
            if (card == first)
            {
                D_80122C00.cards.card_ids[i] = FIELD_NO_CARD;
                first = FIELD_NO_CARD;
            }
            else if (card == second)
            {
                D_80122C00.cards.card_ids[i] = FIELD_NO_CARD;
                second = FIELD_NO_CARD;
            }
            else if (card == third)
            {
                D_80122C00.cards.card_ids[i] = FIELD_NO_CARD;
                third = FIELD_NO_CARD;
            }
        }
    }
    FIELD_LOCAL_HALF(FIELD_LEFTOVER_CARDS) = first;
    FIELD_LOCAL_HALF(FIELD_LEFTOVER_CARDS + 1) = second;
    FIELD_LOCAL_HALF(FIELD_LEFTOVER_CARDS + 2) = third;
}

/**
 * @brief Load the Mystic Card ids of the gosub-selected inventory record into the card list.
 */
void field_menu_load_record_cards(void)
{
    s32 index;

    index = g_gosub_result_values[0];
    D_80122C00.cards.card_ids[0] = FIELD_PAD.inventory[index].unknown_0x18[8];
    D_80122C00.cards.card_ids[1] = FIELD_PAD.inventory[index].unknown_0x18[9];
    D_80122C00.cards.card_ids[2] = FIELD_PAD.inventory[index].unknown_0x18[10];
}

/**
 * @brief Look up the menu object's halfword in resource 0x102.
 *
 * Each object has two halfwords after the resource header; the variant selects
 * the second one. Object id 0xFF yields 0xFFFF.
 */
void field_menu_lookup_object_value(void)
{
    s16 object_id;
    u8* resource;
    s32 offset;
    u16 result;

    object_id = FIELD_MENU_OBJECT->object_id;
    if (object_id == 0xFF)
    {
        result = 0xFFFF;
    }
    else
    {
        if (FIELD_MENU_OBJECT->variant == 0)
        {
            resource = func_800C1E40(0x102);
            offset = object_id * 4;
        }
        else
        {
            resource = func_800C1E40(0x102);
            offset = object_id * 4;
            offset = offset | 2;
        }
        result = *(u16*)(resource + offset + 4);
    }
    D_80122C0E = result;
}

/**
 * @brief Publish the menu object's text from resource 0x101 to text macro 0.
 */
void field_menu_publish_object_text(void)
{
    s32 object_id;
    s32 offset;

    object_id = FIELD_MENU_OBJECT->object_id;
    offset = FIELD_TEXT_OFFSET(FIELD_OBJECT_TEXT_RESOURCE, object_id);
    field_set_text_macro(0, FIELD_TEXT_RESOURCE(FIELD_OBJECT_TEXT_RESOURCE)->texts + offset, 0xFF);
}

/**
 * @brief Decode the gosub-selected small history record into a display id and mode.
 *
 * A blocked record shows id 0x53 + its byte at 0x16 in mode 0; otherwise it
 * shows 0x12 + its byte at 0x15, in mode 1 when selection is restricted. The
 * record at the small-history index shows 0xFE. Also stores the result count.
 */
void field_menu_describe_selected_history(void)
{
    s32 index;
    SmallHistorySelection flags;
    s32 restricted;

    if (g_gosub_result_count != 0)
    {
        index = g_gosub_result_values[0];
        if (index < SMALL_HISTORY_RECORD_COUNT)
        {
            flags = FIELD_PAD.small_history_records[index].selection_flags;
            if (flags.selection_blocked)
            {
                FIELD_MENU_RECORD->display_id = FIELD_PAD.small_history_records[index].unknown_0x16 + 0x53;
                FIELD_MENU_RECORD->mode = 0;
            }
            else
            {
                restricted = flags.selection_restricted;
                FIELD_MENU_RECORD->display_id = FIELD_PAD.small_history_records[index].unknown_0x15 + 0x12;
                FIELD_MENU_RECORD->mode = restricted;
            }
            if (index == FIELD_PAD.small_history_index)
            {
                FIELD_MENU_RECORD->display_id = 0xFE;
            }
        }
        else
        {
            record_game_diagnostic(FIELD_DIAG_MENU_OP, FIELD_MENU_OP_DESCRIBE_SELECTED_HISTORY, index, 0);
        }
    }
    D_80122C16 = g_gosub_result_count;
}

/**
 * @brief Count the occupied small history records whose selection is restricted.
 */
void field_menu_count_restricted_history(void)
{
    s32 count;
    s32 i;

    if (g_gosub_result_count != 0)
    {
        count = 0;
        for (i = 0; i < SMALL_HISTORY_RECORD_COUNT; i++)
        {
            if (FIELD_PAD.small_history_records[i].name[0] != 0 && FIELD_PAD.small_history_records[i].selection_flags.selection_restricted == 1)
            {
                count++;
            }
        }
    }
    D_80122C16 = count;
}

/**
 * @brief Restrict selection of the gosub-selected small history record.
 */
void field_menu_restrict_selected_history(void)
{
    s32 index;

    index = g_gosub_result_values[0];
    if (index < SMALL_HISTORY_RECORD_COUNT)
    {
        FIELD_PAD.small_history_records[index].selection_flags.selection_restricted = 1;
    }
    else
    {
        record_game_diagnostic(FIELD_DIAG_MENU_OP, FIELD_MENU_OP_RESTRICT_SELECTED_HISTORY, index, 0);
    }
}

/**
 * @brief Open the gosub screen sequence described by D_80051EC0.
 */
void func_800C7238(void)
{
    FieldGosubSequence sequence;

    sequence = D_80051EC0;
    field_open_gosub_screen_sequence(&sequence);
}

/**
 * @brief Clear the gosub-screen request and store the selected record index and result count.
 */
void field_menu_store_gosub_result(void)
{
    s32 index;

    D_801227F0 = 0;
    index = g_gosub_result_values[0];
    FIELD_MENU_RESULT->index = index;
    FIELD_MENU_RESULT->count = g_gosub_result_count;
}

/**
 * @brief Open the gosub screen sequence described by D_80051ECC.
 */
void func_800C72A4(void)
{
    FieldGosubSequence sequence;

    sequence = D_80051ECC;
    field_open_gosub_screen_sequence(&sequence);
}

/**
 * @brief Store the selected small history record and publish its name to a text macro.
 *
 * The text macro index comes from the variable at D_80122C02.
 */
void field_menu_publish_selected_history_name(void)
{
    s32 index;

    FIELD_MENU_NAMED->index = index = g_gosub_result_values[0];
    FIELD_MENU_NAMED->count = g_gosub_result_count;
    field_set_text_macro(FIELD_MENU_NAMED->macro_index, FIELD_PAD.small_history_records[index].name, 0xFF);
}

/**
 * @brief Publish the selected small history record's extra slot texts and its name.
 * @note Unused slots (0xFE/0xFF) are skipped; the number published goes to D_80122C16.
 * @note Selections of five or greater record a diagnostic instead.
 */
void field_menu_publish_history_slots(void)
{
    s32 selection;
    s32 slot;
    s32 count;
    u8 entry;

    selection = D_80122C10;
    if (selection < SMALL_HISTORY_RECORD_COUNT)
    {
        count = 0;
        for (slot = 0; slot < 3; slot++)
        {
            entry = FIELD_MENU_HISTORY->small_history_records[selection].extra_slots[slot];
            /* Two separate tests; && folds them into one range check. */
            if (entry != 0xFF)
            {
                if (entry != 0xFE)
                {
                    field_set_text_macro(count, FIELD_MENU_TEXT(entry), 0xFF);
                    count++;
                }
            }
        }
        field_set_text_macro(3, FIELD_PAD.small_history_records[selection].name, 0xFF);
        D_80122C16 = count;
        return;
    }
    record_game_diagnostic(FIELD_DIAG_MENU_OP, FIELD_MENU_OP_PUBLISH_HISTORY_SLOTS, selection, 0);
}

/** @brief Clear pad-context byte 0xC06 and the active large history record's 0x46 byte. */
void func_800C745C(void)
{
    PadContext* ctx;
    s32 index;

    ctx = &FIELD_PAD;
    index = ctx->large_history_index;
    g_saved_game.bytes[0xC06] = 0;
    ctx->large_history_records[index].unknown_0x46 = 0;
}

/**
 * @brief Store D_80122C12 in the first unused extra slot of small history record D_80122C10.
 * @note Selections of five or greater record a diagnostic instead.
 */
void field_menu_add_history_slot(void)
{
    s32 selection;
    s32 value;
    s32 slot;
    u8 entry;

    selection = FIELD_MENU_RECORD->display_id;
    value = FIELD_MENU_RECORD->mode;
    if (selection < SMALL_HISTORY_RECORD_COUNT)
    {
        for (slot = 0; slot < 3; slot++)
        {
            entry = FIELD_MENU_HISTORY->small_history_records[selection].extra_slots[slot];
            if (entry == 0xFE || entry == 0xFF)
            {
                FIELD_MENU_HISTORY->small_history_records[selection].extra_slots[slot] = value;
                return;
            }
        }
        return;
    }
    record_game_diagnostic(FIELD_DIAG_MENU_OP, FIELD_MENU_OP_ADD_HISTORY_SLOT, selection, 0);
}

/** @brief Replace the small history index in D_80122C10 with that record's entry id. */
void field_menu_history_entry_id(void)
{
    D_80122C10 = FIELD_PAD.small_history_records[D_80122C10].unknown_0x15;
}

/**
 * @brief Lift the selection restriction of the current small history record.
 * @note Indices of five or greater record a diagnostic instead.
 */
void field_menu_unrestrict_current_history(void)
{
    s32 index;

    index = FIELD_PAD.small_history_index;
    if (index < SMALL_HISTORY_RECORD_COUNT)
    {
        FIELD_PAD.small_history_records[index].selection_flags.selection_restricted = 0;
    }
    else
    {
        record_game_diagnostic(FIELD_DIAG_MENU_OP, FIELD_MENU_OP_UNRESTRICT_CURRENT_HISTORY, index, 0);
    }
}

/**
 * @brief Lift the selection restriction of the gosub-selected small history record.
 * @note Indices of five or greater record a diagnostic instead, under menu
 *       op 0x32 (field_menu_unrestrict_current_history's number), not 0x33.
 */
void field_menu_unrestrict_selected_history(void)
{
    s32 index;

    index = g_gosub_result_values[0];
    if (index < SMALL_HISTORY_RECORD_COUNT)
    {
        FIELD_PAD.small_history_records[index].selection_flags.selection_restricted = 0;
    }
    else
    {
        record_game_diagnostic(FIELD_DIAG_MENU_OP, FIELD_MENU_OP_UNRESTRICT_CURRENT_HISTORY, index, 0);
    }
}

/**
 * @brief Load the gosub-selected small history record's 0x5A halfword into D_80122C00.
 * @note Does nothing when the gosub returned no selection.
 */
void field_menu_load_history_value(void)
{
    s32 index;

    if (g_gosub_result_count != 0)
    {
        index = g_gosub_result_values[0];
        D_80122C00.words[0] = FIELD_MENU_HISTORY->small_history_records[index].unknown_0x5A;
    }
}

/**
 * @brief Prepare the action item choices for group D_80122C1F and show the first owned item.
 *
 * Copies the eight action item counts to D_80122C00, stores the mask of items
 * not owned and the owned count plus one at D_80122C08, sets D_80122C0F to the
 * group's item slot count (0xFF when the group has no free action slot) and
 * publishes the first owned item's text to macro 4.
 */
void field_menu_prepare_action_items(void)
{
    s32 counts[8];
    s32 group;
    s32 count; /* owned items, then used action slots */
    s32 i;
    s32 mask;
    s32 capacity;
    s32 item_slot_count;
    s32 item;
    u32 flags;

    group = D_80122C1F;
    count = 0;
    for (i = 0; i < 8; i++)
    {
        counts[i] = FIELD_PAD.item_counts[FIELD_ACTION_ITEM_BASE + i];
        if (counts[i] != 0)
        {
            count++;
        }
    }

    for (i = 0; i < 8; i++)
    {
        D_80122C00.bytes[i] = counts[i];
    }

    mask = 0x1FFF;
    for (i = 0; i < 8; i++)
    {
        if (counts[i] != 0)
        {
            mask &= ~(1 << i);
        }
    }

    capacity = count + 1;
    count = 0;
    FIELD_MENU_CHOICES->mask = mask;
    FIELD_MENU_CHOICES->count = capacity;
    flags = FIELD_MENU_ACTIONS.groups[group].flags;
    item_slot_count = flags >> 8;
    item_slot_count &= 0xF;
    capacity = flags & 0xF;
    for (i = 0; i < 8; i++)
    {
        if (FIELD_MENU_ACTIONS.groups[group].slots[i].entry.item_index < 0xFF)
        {
            count++;
        }
    }

    if (count >= capacity)
    {
        D_80122C0F = 0xFF;
    }
    else
    {
        D_80122C0F = item_slot_count;
    }

    for (i = 0; i < 8; i++)
    {
        if (D_80122C00.bytes[i] != 0)
        {
            break;
        }
    }
    item = FIELD_ACTION_ITEM_BASE + i;
    field_set_text_macro(4, FIELD_MENU_TEXT(item), 0xFF);
    field_menu_clear_item_slots();
}

/**
 * @brief Put the chosen action item into the first free item slot of group D_80122C1F.
 *
 * Records the new used-slot count in the group flags and at D_80122C0B, sets
 * D_80122C0C when fewer than three slots stay free, takes one of the item
 * (D_80122C14) from the counts copied to D_80122C00, publishes its text to
 * macro 0 and refreshes the item mask and owned count at D_80122C08.
 */
void field_menu_add_action_item(void)
{
    s32 group;
    s32 count;
    s32 owned;
    s32 slot;
    s32 capacity;
    s32 item_slot_count;
    s32 used;
    s32 item;
    s32 mask;
    u32 flags;
    s32 selected;

    count = 0;
    group = D_80122C1F;
    flags = FIELD_MENU_ACTIONS.groups[group].flags;
    item_slot_count = flags >> 8;
    item_slot_count &= 0xF;
    capacity = flags & 0xF;
    for (slot = 0; slot < 8; slot++)
    {
        if (FIELD_MENU_ACTIONS.groups[group].slots[slot].entry.item_index != 0xFF)
        {
            count++;
        }
    }
    capacity -= count;

    for (slot = 0; slot < item_slot_count; slot++)
    {
        if (FIELD_MENU_ACTIONS.groups[group].item_slots[slot] == 0xFF)
        {
            break;
        }
    }

    used = slot + 1;
    FIELD_LOCAL_BYTE(FIELD_ACTION_USED_SLOTS) = used;
    FIELD_MENU_ACTIONS.groups[group].flags = (FIELD_MENU_ACTIONS.groups[group].flags & 0xFFFF0FFF) | ((used & 0xF) << 12);
    if (capacity < slot + 3)
    {
        FIELD_LOCAL_BYTE(FIELD_ACTION_GROUP_FULL) = 1;
    }
    selected = FIELD_LOCAL_HALF(FIELD_ACTION_SELECTED_ITEM);
    FIELD_LOCAL_BYTE(FIELD_ACTION_ITEM_COUNTS + selected)--;
    item = selected + FIELD_ACTION_ITEM_BASE;
    FIELD_MENU_ACTIONS.groups[group].item_slots[slot] = item;
    field_set_text_macro(0, FIELD_MENU_TEXT(item), 0xFF);

    mask = 0x1FFF;
    owned = 0;
    for (slot = 0; slot < 8; slot++)
    {
        if (FIELD_LOCAL_BYTE(FIELD_ACTION_ITEM_COUNTS + slot) != 0)
        {
            mask &= ~(1 << slot);
            owned++;
        }
    }
    FIELD_MENU_CHOICES->mask = mask;
    FIELD_MENU_CHOICES->count = owned;
}

/**
 * @brief Fill free action slots of group D_80122C1F and take back its item slots' items.
 *
 * Fills up to (flags bits 12-15) + 2 free action slots, clamped to the free
 * slot count, through field_roll_menu_slot_effect. Each fill picks a random slot (at most
 * 1000 tries) and falls back to the first free slot. Then gives back one of
 * each item in the group's item slots and clears them through field_menu_clear_item_slots.
 */
void field_menu_fill_action_slots(void)
{
    s32 group;
    s32 i;
    s32 j; /* used-slot count, then pick attempts and the free-slot scan */
    s32 free_slots;
    s32 item_slot_count;
    s32 fill;
    s32 limit;
    s32 pick;
    s32 item;
    u32 flags;

    i = j = 0;
    group = D_80122C1F;
    flags = FIELD_MENU_ACTIONS.groups[group].flags;
    free_slots = flags & 0xF;
    item_slot_count = flags >> 8;
    item_slot_count &= 0xF;
    fill = flags >> 12;
    fill &= 0xF;
    fill += 2;
    for (; i < 8; i++)
    {
        if (FIELD_MENU_ACTIONS.groups[group].slots[i].entry.item_index != 0xFF)
        {
            j++;
        }
    }
    free_slots -= j;
    if (fill >= 0)
    {
        limit = free_slots;
        if (limit >= fill)
        {
            limit = fill;
        }
    }
    else
    {
        limit = 0;
    }
    fill = limit;

    for (i = 0; i < fill; i++)
    {
        for (j = 0; j < 1000; j++)
        {
            pick = rand() / 4096;
            if (FIELD_MENU_ACTIONS.groups[group].slots[pick].entry.item_index == 0xFF)
            {
                break;
            }
        }
        if (FIELD_MENU_ACTIONS.groups[group].slots[pick].entry.item_index == 0xFF)
        {
            field_roll_menu_slot_effect(group, pick);
        }
        else
        {
            for (j = 0; j < 8; j++)
            {
                if (FIELD_MENU_ACTIONS.groups[group].slots[j].entry.item_index == 0xFF)
                {
                    field_roll_menu_slot_effect(group, j);
                    break;
                }
            }
        }
    }

    for (i = 0; i < item_slot_count; i++)
    {
        item = FIELD_MENU_ACTIONS.groups[group].item_slots[i];
        if (item < 0xFF)
        {
            FIELD_PAD.item_counts[item]--;
        }
    }
    field_menu_clear_item_slots();
}

/** @brief Clear the used item-slot count of group D_80122C1F and empty its item slots. */
void field_menu_clear_item_slots(void)
{
    s32 group;
    s32 i;

    group = D_80122C1F;
    FIELD_MENU_ACTIONS.groups[group].flags &= 0xFFFF0FFF;
    for (i = 0; i < 4; i++)
    {
        FIELD_MENU_ACTIONS.groups[group].item_slots[i] = 0xFF;
    }
}

/** @brief Free the eight action slots of group 0 and clear its used item-slot count. */
void field_menu_reset_group0_slots(void)
{
    FieldMenuActionData* data;
    u32 flags;

    data = &FIELD_MENU_ACTIONS;
    flags = data->groups[0].flags;
    data->groups[0].slots[0].handle = 0;
    data->groups[0].slots[0].entry.item_index = 0xFF;
    data->groups[0].slots[1].handle = 0;
    data->groups[0].slots[1].entry.item_index = 0xFF;
    data->groups[0].slots[2].handle = 0;
    data->groups[0].slots[2].entry.item_index = 0xFF;
    data->groups[0].slots[3].handle = 0;
    data->groups[0].slots[3].entry.item_index = 0xFF;
    data->groups[0].slots[4].handle = 0;
    data->groups[0].slots[4].entry.item_index = 0xFF;
    data->groups[0].slots[5].handle = 0;
    data->groups[0].slots[5].entry.item_index = 0xFF;
    data->groups[0].slots[6].handle = 0;
    data->groups[0].slots[6].entry.item_index = 0xFF;
    data->groups[0].slots[7].handle = 0;
    data->groups[0].slots[7].entry.item_index = 0xFF;
    data->groups[0].flags = flags & 0xFFFF0FFF;
}

/** @brief Publish the text of the chosen action item (D_80122C14) to macro 4. */
void field_menu_publish_chosen_action_item(void)
{
    s32 item;

    item = D_80122C14 + FIELD_ACTION_ITEM_BASE;
    field_set_text_macro(4, FIELD_MENU_TEXT(item), 0xFF);
}

/**
 * @brief Place actor D_80122C0D next to actor 0, on the side it faces.
 *
 * Takes actor 0's position (8.8 fixed point) and heading (0x00-0xFF), steps
 * 0x14 along x or 0xC along z for the four main headings and 0xA along both
 * axes in between, and moves the actor there, 0xC higher.
 */
void field_place_actor_beside_actor0(void)
{
    s32 pos[3];
    s32 heading;
    s32 actor;

    actor = D_80122C0D;
    field_get_actor_position(0, pos);
    pos[0] /= 256;
    pos[1] /= 256;
    pos[2] /= 256;

    heading = field_get_actor_facing(0);
    if (heading == 0)
    {
        pos[0] += 0x14;
    }
    else if (heading >= 0x01 && heading < 0x40)
    {
        pos[0] += 0xA;
        pos[2] -= 0xA;
    }
    else if (heading == 0x40)
    {
        pos[2] -= 0xC;
    }
    else if (heading >= 0x41 && heading < 0x80)
    {
        pos[0] -= 0xA;
        pos[2] -= 0xA;
    }
    else if (heading == 0x80)
    {
        pos[0] -= 0x14;
    }
    else if (heading >= 0x81 && heading < 0xC0)
    {
        pos[0] -= 0xA;
        pos[2] += 0xA;
    }
    else if (heading == 0xC0)
    {
        pos[2] += 0xC;
    }
    else if (heading >= 0xC1 && heading < 0x100)
    {
        pos[0] += 0xA;
        pos[2] += 0xA;
    }

    field_set_actor_position(actor, pos[0], pos[1] - 0xC, pos[2]);
}

/**
 * @brief Free the action slot selected by D_80122C0D and give its item back.
 * @note The item count is clamped to 0-99.
 */
void field_menu_release_action_slot(void)
{
    s32 slot;
    s32 group;
    s32 count;
    s32 clamped;
    u8 item;

    slot = FIELD_MENU_ACTION_SLOT->action_slot;
    slot -= 4;
    group = FIELD_MENU_ACTION_SLOT->group;
    item = FIELD_MENU_ACTIONS.groups[group].slots[slot].entry.item_index;
    count = FIELD_PAD.item_counts[item];
    count += 1;
    if (count >= 0)
    {
        clamped = 99;
        if (count < 100)
        {
            clamped = count;
        }
    }
    else
    {
        clamped = 0;
    }

    FIELD_PAD.item_counts[item] = clamped;
    FIELD_MENU_ACTIONS.groups[group].slots[slot].handle = 0;
    FIELD_MENU_ACTIONS.groups[group].slots[slot].entry.item_index = 0xFF;
    FIELD_MENU_ACTIONS.groups[group].slots[slot].counters[0] = 0;
    FIELD_MENU_ACTIONS.groups[group].slots[slot].counters[1] = 0;
    FIELD_MENU_ACTIONS.groups[group].slots[slot].counters[2] = 0;
    FIELD_MENU_ACTIONS.groups[group].slots[slot].counters[3] = 0;
    FIELD_MENU_ACTIONS.groups[group].slots[slot].counters[4] = 0;
    FIELD_MENU_ACTIONS.groups[group].slots[slot].counters[5] = 0;
    FIELD_MENU_ACTIONS.groups[group].slots[slot].counters[6] = 0;
    FIELD_MENU_ACTIONS.groups[group].slots[slot].counters[7] = 0;
}

/**
 * @brief Load the action slot selected by D_80122C0D into its script variables.
 * @note Publishes the slot item's text to macro 0 and stores the item in D_80122C1C.
 */
void field_menu_load_action_slot(void)
{
    s32 slot;
    s32 group;
    s32 handle;
    u8 item;

    slot = FIELD_MENU_ACTION_SLOT->action_slot - 4;
    group = FIELD_MENU_ACTION_SLOT->group;
    handle = FIELD_MENU_ACTIONS.groups[group].slots[slot].handle;
    FIELD_MENU_ACTION_SLOT->handle = handle;
    item = FIELD_MENU_ACTIONS.groups[group].slots[slot].entry.item_index;
    field_set_text_macro(0, FIELD_MENU_TEXT(item), 0xFF);
    FIELD_MENU_ACTION_SLOT->item_index = item;
}

/**
 * @brief Copy the gosub-selected inventory record into the first free pending record.
 *
 * Sets D_80122C1F to 0 when the gosub returned nothing, 2 when the record
 * holds no item, 3 when a pending record already holds the same item and 1
 * when the record was copied (its 0x34 word is then at least 1).
 */
void field_menu_add_pending_record(void)
{
    s32 selected;
    s32 i;
    s32 value;

    D_80122C1F = 0;
    if (g_gosub_result_count == 0)
    {
        return;
    }

    selected = g_gosub_result_values[0];
    if (FIELD_MENU_ITEMS->inventory[selected].identity[0] == 0 && FIELD_MENU_ITEMS->inventory[selected].identity[1] == 0)
    {
        D_80122C1F = 2;
        return;
    }

    for (i = 0; i < 4; i++)
    {
        if (FIELD_MENU_ITEMS->pending[i].active != 0 && FIELD_MENU_ITEMS->inventory[selected].identity[0] == FIELD_MENU_ITEMS->pending[i].identity[0] &&
            FIELD_MENU_ITEMS->inventory[selected].identity[1] == FIELD_MENU_ITEMS->pending[i].identity[1])
        {
            D_80122C1F = 3;
            return;
        }
    }

    for (i = 0; i < 4; i++)
    {
        if (FIELD_MENU_ITEMS->pending[i].active == 0)
        {
            field_copy_inventory_record((u8*)&FIELD_MENU_ITEMS->pending[i], (u8*)&FIELD_MENU_ITEMS->inventory[selected]);
            func_800C2A88(selected);
            value = FIELD_MENU_ITEMS->pending[i].unknown_0x34;
            if (value == 0)
            {
                value = 1;
            }
            FIELD_MENU_ITEMS->pending[i].unknown_0x34 = value;
            D_80122C1F = 1;
            return;
        }
    }
}

/** @brief Open the pending-record selection screen sequence. */
void func_800C8220(void)
{
    FieldGosubSequence sequence;

    sequence = D_80051EB4;
    field_open_gosub_screen_sequence(&sequence);
}

/**
 * @brief Place actor 0xC next to actor 0, on the side it faces.
 *
 * Like field_place_actor_beside_actor0, with steps of 0xA along one axis for the four main
 * headings and 8 along both axes in between, at the same height.
 */
void field_place_actor_0c_beside_actor0(void)
{
    FieldPosition pos;
    s32 heading;

    field_get_actor_position(0, (s32*)&pos);
    pos.x /= 256;
    pos.y /= 256;
    pos.z /= 256;

    heading = field_get_actor_facing(0);
    if (heading == 0)
    {
        pos.x += 0xA;
    }
    else if (heading >= 0x01 && heading < 0x40)
    {
        pos.x += 8;
        pos.z -= 8;
    }
    else if (heading == 0x40)
    {
        pos.z -= 0xA;
    }
    else if (heading >= 0x41 && heading < 0x80)
    {
        pos.x -= 8;
        pos.z -= 8;
    }
    else if (heading == 0x80)
    {
        pos.x -= 0xA;
    }
    else if (heading >= 0x81 && heading < 0xC0)
    {
        pos.x -= 8;
        pos.z += 8;
    }
    else if (heading == 0xC0)
    {
        pos.z += 0xA;
    }
    else if (heading >= 0xC1 && heading < 0x100)
    {
        pos.x += 8;
        pos.z += 8;
    }

    field_set_actor_position(0xC, pos.x, pos.y, pos.z);
}

/**
 * @brief Increment each of the eight counters of action slot @p slot once.
 * @param slot An action slot lvalue (FieldMenuActionSlot).
 */
#define FIELD_STEP_ACTION_COUNTERS(slot)                                                                                                                       \
    ((slot).counters[0]++, (slot).counters[1]++, (slot).counters[2]++, (slot).counters[3]++, (slot).counters[4]++, (slot).counters[5]++, (slot).counters[6]++, \
     (slot).counters[7]++)

/**
 * @brief Advance the counters of every used action slot of the four groups.
 * @note A first counter of zero only becomes 1; below 0xF0 all eight counters
 *       advance by 9; from 0xF0 on they stay.
 */
void field_menu_advance_action_counters(void)
{
    s32 group;
    s32 slot;
    u8 counter;

    for (group = 0; group < 4; group++)
    {
        for (slot = 0; slot < 8; slot++)
        {
            if (FIELD_MENU_ACTIONS.groups[group].slots[slot].entry.item_index != 0xFF)
            {
                counter = FIELD_MENU_ACTIONS.groups[group].slots[slot].counters[0];
                if (counter == 0)
                {
                    FIELD_MENU_ACTIONS.groups[group].slots[slot].counters[0] = counter + 1;
                }
                else if (counter < 0xF0)
                {
                    FIELD_STEP_ACTION_COUNTERS(FIELD_MENU_ACTIONS.groups[group].slots[slot]);
                    FIELD_STEP_ACTION_COUNTERS(FIELD_MENU_ACTIONS.groups[group].slots[slot]);
                    FIELD_STEP_ACTION_COUNTERS(FIELD_MENU_ACTIONS.groups[group].slots[slot]);
                    FIELD_STEP_ACTION_COUNTERS(FIELD_MENU_ACTIONS.groups[group].slots[slot]);
                    FIELD_STEP_ACTION_COUNTERS(FIELD_MENU_ACTIONS.groups[group].slots[slot]);
                    FIELD_STEP_ACTION_COUNTERS(FIELD_MENU_ACTIONS.groups[group].slots[slot]);
                    FIELD_STEP_ACTION_COUNTERS(FIELD_MENU_ACTIONS.groups[group].slots[slot]);
                    FIELD_STEP_ACTION_COUNTERS(FIELD_MENU_ACTIONS.groups[group].slots[slot]);
                    FIELD_STEP_ACTION_COUNTERS(FIELD_MENU_ACTIONS.groups[group].slots[slot]);
                }
            }
        }
    }
}

/** @brief Free the action slot selected by D_80122C0D without giving its item back. */
void field_reset_menu_action_slot(void)
{
    s32 slot;
    s32 group;

    slot = FIELD_MENU_ACTION_SLOT->action_slot - 4;
    group = FIELD_MENU_ACTION_SLOT->group;
    FIELD_MENU_ACTIONS.groups[group].slots[slot].entry.item_index = 0xFF;
    FIELD_MENU_ACTIONS.groups[group].slots[slot].handle = 0;
    FIELD_MENU_ACTIONS.groups[group].slots[slot].counters[0] = 0;
    FIELD_MENU_ACTIONS.groups[group].slots[slot].counters[1] = 0;
    FIELD_MENU_ACTIONS.groups[group].slots[slot].counters[2] = 0;
    FIELD_MENU_ACTIONS.groups[group].slots[slot].counters[3] = 0;
    FIELD_MENU_ACTIONS.groups[group].slots[slot].counters[4] = 0;
    FIELD_MENU_ACTIONS.groups[group].slots[slot].counters[5] = 0;
    FIELD_MENU_ACTIONS.groups[group].slots[slot].counters[6] = 0;
    FIELD_MENU_ACTIONS.groups[group].slots[slot].counters[7] = 0;
    /* Bytes 5-7 of the slot are set to 0xFF as part of their word. */
    FIELD_MENU_ACTIONS.groups[group].slots[slot].entry.word |= ~0xFF;
}

/** @brief Reset all lands and place the default set in order. */
void field_reset_default_lands(void)
{
    field_reset_lands();
    field_place_land(0);
    field_place_land(1);
    field_place_land(2);
    field_place_land(3);
    field_place_land(4);
    field_place_land(5);
    field_place_land(7);
    field_place_land(8);
    field_place_land(9);
    field_place_land(0xA);
    field_place_land(0xB);
    field_place_land(0xC);
    field_place_land(0xD);
    field_place_land(0xF);
    field_place_land(0x10);
    field_place_land(0x11);
    field_place_land(0x12);
    field_place_land(0x13);
    field_place_land(0x15);
    field_place_land(0x16);
    field_place_land(0x18);
    field_place_land(0x19);
    field_place_land(0x1A);
    field_place_land(0x1B);
    field_place_land(0x1E);
    field_place_land(0x1F);
    field_place_land(0x20);
    field_place_land(0x20);
    field_place_land(0x17);
}

/** @brief Restore the saved field mode through the mode dispatcher. */
void field_menu_restore_field_mode(void)
{
    D_8011F428 = D_80122C1E;
    field_open_niki(D_80122C1E);
}

/**
 * @brief List the occupied small history records whose selection is restricted.
 * @note Stores each listed record's entry id from D_80122C00 and its index from
 *       D_80122C1D, and publishes its name to macro 0, 1, ... in order.
 */
void field_menu_list_restricted_history(void)
{
    s32 count;
    s32 i;

    count = 0;
    for (i = 0; i < SMALL_HISTORY_RECORD_COUNT; i++)
    {
        if (FIELD_PAD.small_history_records[i].name[0] != 0 && FIELD_PAD.small_history_records[i].selection_flags.selection_restricted == 1)
        {
            FIELD_LOCAL_BYTE(count) = FIELD_PAD.small_history_records[i].unknown_0x15;
            FIELD_LOCAL_BYTE(FIELD_CURSOR_SLOT_TABLE + count) = i;
            field_set_text_macro(count, FIELD_PAD.small_history_records[i].name, 0xFF);
            count++;
        }
    }
}

/**
 * @brief Check the selected shared item record and publish its description.
 *
 * An empty record sets status 1. A record whose identity words match an
 * inventory, equipment or pending record sets status 2. Otherwise the name,
 * name text, kind, category, stat value and item value go to the script
 * variables, with the value's digit count at D_80122C0C.
 */
void field_menu_describe_shared_item(void)
{
    s32 i;
    s32 duplicate_found;
    s32 selected;
    u32 attributes;
    s32 kind;
    s32 category;
    s32 name_index;
    s32 value;
    u32 low_word;
    u32 high_word;
    s32 nibble_sum;
    s32 special;
    u32 amount;
    s32 digits;
    FieldMenuItemInfoVars* info;
    FieldMenuRecordSelectVars* selection;

    selection = FIELD_MENU_RECORD_SELECT;
    selection->status = 0;
    selected = selection->index;
    if (g_field_shared_items[selected].active == 0)
    {
        selection->status = 1;
        return;
    }

    duplicate_found = 0;
    for (i = 0; i < INVENTORY_RECORD_COUNT; i++)
    {
        if (FIELD_MENU_ITEMS->inventory[i].active != 0 && FIELD_MENU_ITEMS->inventory[i].identity[0] == g_field_shared_items[selected].identity[0] &&
            FIELD_MENU_ITEMS->inventory[i].identity[1] == g_field_shared_items[selected].identity[1])
        {
            duplicate_found = 1;
            break;
        }
    }

    for (i = 0; i < 8; i++)
    {
        if (FIELD_MENU_ITEMS->equipment[i].active != 0 && FIELD_MENU_ITEMS->equipment[i].identity[0] == g_field_shared_items[selected].identity[0] &&
            FIELD_MENU_ITEMS->equipment[i].identity[1] == g_field_shared_items[selected].identity[1])
        {
            duplicate_found = 1;
            break;
        }
    }

    for (i = 0; i < 4; i++)
    {
        if (FIELD_MENU_ITEMS->pending[i].active != 0 && FIELD_MENU_ITEMS->pending[i].identity[0] == g_field_shared_items[selected].identity[0] &&
            FIELD_MENU_ITEMS->pending[i].identity[1] == g_field_shared_items[selected].identity[1])
        {
            duplicate_found = 1;
            break;
        }
    }

    if (duplicate_found == 0)
    {
        attributes = g_field_shared_items[selected].attributes.packed;
        kind = (attributes >> 8) & 3;
        category = (attributes >> 10) & 0x3F;
        if (kind == 1)
        {
            category += 0xB;
        }
        else if (kind == 2)
        {
            category += 0x17;
        }

        name_index = g_field_shared_items[selected].attributes.halves.high & 0x3F;
        if (kind == 0)
        {
            value = g_field_shared_items[selected].stats.values[0];
        }
        else if (kind == 1)
        {
            value = g_field_shared_items[selected].stats.values[0];
            value += g_field_shared_items[selected].stats.values[1];
            value += g_field_shared_items[selected].stats.values[2];
            value += g_field_shared_items[selected].stats.values[3];
        }
        else
        {
            value = g_field_shared_items[selected].stats.bytes[2];
        }

        low_word = g_field_shared_items[selected].nibbles[0];
        high_word = g_field_shared_items[selected].nibbles[1];
        nibble_sum = (low_word & 0xF) + ((low_word >> 4) & 0xF) + ((low_word >> 8) & 0xF) + ((low_word >> 12) & 0xF) + ((low_word >> 16) & 0xF) +
                     ((low_word >> 20) & 0xF) + ((low_word >> 24) & 0xF) + (low_word >> 28) + (high_word & 0xF) + ((high_word >> 4) & 0xF) +
                     ((high_word >> 8) & 0xF) + ((high_word >> 12) & 0xF) + ((high_word >> 16) & 0xF) + ((high_word >> 20) & 0xF) + ((high_word >> 24) & 0xF) +
                     (high_word >> 28);
        special = nibble_sum >= 0x29;
        if (kind == 2)
        {
            special = g_field_shared_items[selected].stats.bytes[0];
        }

        amount = g_field_shared_items[selected].unknown_0x34;
        field_set_text_macro(0, &g_field_shared_items[selected].active, 0xFF);
        info = FIELD_MENU_ITEM_INFO;
        info->kind = kind;
        info->category = category;
        field_set_text_macro(1, FIELD_MENU_TEXT(name_index), 0xFF);
        info->value = value;
        if (special != 0)
        {
            info->value = value - 0x8000;
        }
        info->amount = amount;
        digits = 0;
        do
        {
            amount /= 10;
            digits++;
        } while (amount != 0);
        D_80122C0C = digits;
        return;
    }

    D_80122C03 = 2;
    field_set_text_macro(0, &g_field_shared_items[selected].active, 0xFF);
}

/**
 * @brief Drop spent pending item records and compact the four-entry pending table.
 *
 * An active pending record with no pending result is cleared. Each free slot
 * without a result then takes the next active record after it.
 */
void field_menu_compact_pending_items(void)
{
    s32 i;
    s32 j;

    for (i = 0; i < 4; i++)
    {
        if (FIELD_MENU_ITEMS->pending[i].active != 0 && FIELD_MENU_ITEMS->pending[i].unknown_0x34 == 0)
        {
            FIELD_MENU_ITEMS->pending[i].active = 0;
        }
    }

    for (i = 0; i < 4; i++)
    {
        if (FIELD_MENU_ITEMS->pending[i].active == 0 && FIELD_MENU_ITEMS->pending[i].unknown_0x34 == 0)
        {
            for (j = i + 1; j < 4; j++)
            {
                if (FIELD_MENU_ITEMS->pending[j].active != 0)
                {
                    field_copy_inventory_record((u8*)&FIELD_MENU_ITEMS->pending[i], (u8*)&FIELD_MENU_ITEMS->pending[j]);
                    FIELD_MENU_ITEMS->pending[j].active = 0;
                    FIELD_MENU_ITEMS->pending[j].unknown_0x34 = 0;
                    break;
                }
            }
        }
    }
}

/**
 * @brief Move the selected pending item record into a free inventory record and compact the table.
 */
void field_menu_take_pending_item(void)
{
    s32 selected;
    u8* free_record;

    selected = D_80122C02;
    free_record = field_find_free_inventory_record();
    field_copy_inventory_record(free_record, (u8*)&FIELD_MENU_ITEMS->pending[selected]);
    FIELD_MENU_ITEMS->pending[selected].active = 0;
    FIELD_MENU_ITEMS->pending[selected].unknown_0x34 = 0;
    field_menu_compact_pending_items();
}

/**
 * @brief Restore the first set-aside pending item record and publish its pending result.
 *
 * The record's index goes to D_80122C02 (0xFF when there is none) and its
 * result to the word at D_80122C08. Without a result the table is compacted.
 */
void field_menu_restore_pending_item(void)
{
    s32 i;
    s32 result;

    result = 0;
    D_80122C02 = 0xFF;
    for (i = 0; i < 4; i++)
    {
        if (FIELD_MENU_ITEMS->pending[i].active == 0 && FIELD_MENU_ITEMS->pending[i].unknown_0x34 != 0)
        {
            FIELD_MENU_ITEMS->pending[i].active = FIELD_MENU_ITEMS->pending[i].saved_active;
            field_set_text_macro(0, &FIELD_MENU_ITEMS->pending[i].active, 0xFF);
            result = FIELD_MENU_ITEMS->pending[i].unknown_0x34;
            FIELD_MENU_ITEMS->pending[i].unknown_0x34 = 0;
            D_80122C02 = i;
            break;
        }
    }

    D_80122C00.words[2] = result;
    if (result == 0)
    {
        field_menu_compact_pending_items();
    }
}

/**
 * @brief Count the pending item records and publish the selected one's description.
 *
 * The number of active pending records goes to D_80122C06. An empty selected
 * record sets status 1, one whose identity words match an inventory or
 * equipment record sets status 2; otherwise its name, name text, kind and
 * category go to the script variables.
 */
void field_menu_describe_pending_item(void)
{
    s32 i;
    s32 count;
    s32 selected;
    s32 duplicate_found;
    u32 attributes;
    s32 kind;
    s32 category;
    s32 name_index;
    FieldMenuItemInfoVars* info;

    selected = FIELD_LOCAL_BYTE(FIELD_RECORD_INDEX);
    count = 0;
    for (i = 0; i < 4; i++)
    {
        if (FIELD_MENU_ITEMS->pending[i].active != 0)
        {
            count++;
        }
    }

    FIELD_LOCAL_BYTE(FIELD_PENDING_COUNT) = count;
    FIELD_LOCAL_BYTE(FIELD_RECORD_STATUS) = 0;
    if (FIELD_MENU_ITEMS->pending[selected].active == 0)
    {
        FIELD_LOCAL_BYTE(FIELD_RECORD_STATUS) = 1;
        return;
    }

    duplicate_found = 0;
    for (i = 0; i < INVENTORY_RECORD_COUNT; i++)
    {
        if (FIELD_MENU_ITEMS->inventory[i].active != 0 && FIELD_MENU_ITEMS->inventory[i].identity[0] == FIELD_MENU_ITEMS->pending[selected].identity[0] &&
            FIELD_MENU_ITEMS->inventory[i].identity[1] == FIELD_MENU_ITEMS->pending[selected].identity[1])
        {
            duplicate_found = 1;
            break;
        }
    }

    for (i = 0; i < 8; i++)
    {
        if (FIELD_MENU_ITEMS->equipment[i].active != 0 && FIELD_MENU_ITEMS->equipment[i].identity[0] == FIELD_MENU_ITEMS->pending[selected].identity[0] &&
            FIELD_MENU_ITEMS->equipment[i].identity[1] == FIELD_MENU_ITEMS->pending[selected].identity[1])
        {
            duplicate_found = 1;
            break;
        }
    }

    if (duplicate_found == 0)
    {
        attributes = FIELD_MENU_ITEMS->pending[selected].attributes.packed;
        kind = (attributes >> 8) & 3;
        category = (attributes >> 10) & 0x3F;
        if (kind == 1)
        {
            category += 0xB;
        }
        else if (kind == 2)
        {
            category += 0x17;
        }

        name_index = FIELD_MENU_ITEMS->pending[selected].attributes.halves.high & 0x3F;
        field_set_text_macro(0, &FIELD_MENU_ITEMS->pending[selected].active, 0xFF);
        info = FIELD_MENU_ITEM_INFO;
        info->kind = kind;
        info->category = category;
        field_set_text_macro(1, FIELD_MENU_TEXT(name_index), 0xFF);
        return;
    }

    field_set_text_macro(0, &FIELD_MENU_ITEMS->pending[selected].active, 0xFF);
    FIELD_LOCAL_BYTE(FIELD_RECORD_STATUS) = 2;
}

/**
 * @brief Reset the current music track index to zero.
 */
void field_reset_music_track_index(void)
{
    g_music_track_index = 0;
}

/**
 * @brief Clear the restriction flag of the small history record picked by the cursor slot.
 *
 * The record index comes from the cursor slot table at D_80122C19; an index
 * of five or more is reported as a diagnostic.
 */
void field_menu_unrestrict_cursor_history(void)
{
    s32 index;

    index = FIELD_LOCAL_BYTE(FIELD_CURSOR_SLOT_TABLE + FIELD_LOCAL_BYTE(FIELD_UNRESTRICT_SLOT));
    if (index < SMALL_HISTORY_RECORD_COUNT)
    {
        FIELD_PAD.small_history_records[index].selection_flags.selection_restricted = 0;
    }
    else
    {
        record_game_diagnostic(FIELD_DIAG_MENU_OP, FIELD_MENU_OP_UNRESTRICT_CURSOR_HISTORY, index, 0);
    }
}

/**
 * @brief Store an entry id in the selected small history record and lift its restriction.
 *
 * An unnamed record gets the name 'A' (0x41). An index of five or more is
 * reported as a diagnostic.
 */
void field_menu_set_history_entry(void)
{
    s32 index;
    u8 entry_id;

    entry_id = FIELD_LOCAL_BYTE(FIELD_HISTORY_ENTRY_ID);
    index = FIELD_LOCAL_BYTE(FIELD_HISTORY_INDEX);
    if (index < SMALL_HISTORY_RECORD_COUNT)
    {
        FIELD_MENU_HISTORY->small_history_records[index].entry_id = entry_id;
        FIELD_PAD.small_history_records[index].selection_flags.selection_blocked = 0;
        if (FIELD_PAD.small_history_records[index].name[0] == 0)
        {
            FIELD_PAD.small_history_records[index].name[0] = 0x41;
        }
    }
    else
    {
        record_game_diagnostic(FIELD_DIAG_MENU_OP, FIELD_MENU_OP_SET_HISTORY_ENTRY, index, 0);
    }
}

/**
 * @brief Checks field state 0x2F08 and performs the corresponding update.
 */
void func_800C93B4(void)
{
    if (func_800BD414(0, 0x2F08) == 0x80)
    {
        field_open_addhero(1);
    }
    else if (func_800BD414(0, 0x2F08) == 0xFF)
    {
        field_open_addhero(0);
    }
}

/** @brief Dispatch the nonempty menu record at buffer offset 0x840. */
void func_800C9404(void)
{
    D_80122C00.bytes[0] = g_saved_game.bytes[0x840];
    if (D_80122C00.bytes[0] != 0)
    {
        field_set_text_macro(0, &g_saved_game.bytes[0x840], 0xFF);
    }
}

/** @brief Count the free pending item records into D_80122C1F. */
void field_menu_count_free_pending_items(void)
{
    s32 i;
    s32 count;

    count = 0;
    for (i = 0; i < 4; i++)
    {
        if (FIELD_MENU_ITEMS->pending[i].active == 0)
        {
            count++;
        }
    }
    D_80122C1F = count;
}

/** @brief Count the free inventory records into D_80122C1F. */
void field_menu_count_free_inventory(void)
{
    s32 i;
    s32 count;

    count = 0;
    for (i = 0; i < INVENTORY_RECORD_COUNT; i++)
    {
        if (FIELD_MENU_ITEMS->inventory[i].active == 0)
        {
            count++;
        }
    }
    D_80122C1F = count;
}

/** @brief Clear the four shared item records and their values. */
void field_menu_clear_shared_items(void)
{
    g_field_shared_items[0].active = 0;
    g_field_shared_items[1].active = 0;
    g_field_shared_items[2].active = 0;
    g_field_shared_items[3].active = 0;
    g_field_shared_items[0].unknown_0x34 = 0;
    g_field_shared_items[1].unknown_0x34 = 0;
    g_field_shared_items[2].unknown_0x34 = 0;
    g_field_shared_items[3].unknown_0x34 = 0;
}

/**
 * @brief Apply the pending field mode transition to the four shared records.
 */
void func_800C94F4(void)
{
    s32 mode;
    s32 i;
    u8* entry;
    s32 result;

    mode = D_80122C1E;
    if (mode == 1 && D_8011F428 == 0)
    {
        for (i = 0; i < 4; i++)
        {
            if (g_field_shared_items[i].active == 0)
            {
                entry = &FIELD_LOCAL_BYTE(FIELD_SET_ASIDE_ACTIVE + i);
                if (g_field_shared_items[i].unknown_0x34 != 0 && *entry != 0xFA)
                {
                    g_field_shared_items[i].active = g_field_shared_items[i].saved_active;
                    g_field_shared_items[i].saved_active = *entry;
                    if (field_find_free_inventory_record() != 0)
                    {
                        field_copy_inventory_record(field_find_free_inventory_record(), (u8*)&g_field_shared_items[i]);
                    }
                }
            }
        }
    }
    if (mode == 0 && D_8011F428 == 1)
    {
        for (i = 0; i < 4; i++)
        {
            if (g_field_shared_items[i].active != 0)
            {
                result = g_field_shared_items[i].unknown_0x34;
                if (result == 0)
                {
                    result = 1;
                }
                g_field_shared_items[i].unknown_0x34 = result;
            }
        }
    }
    D_80122C1E = D_8011F428;
}

/** @brief Set field state 0x2F08 from the pending mode flag. */
void func_800C963C(void)
{
    if (FIELD_PAD.inject_enable != 0)
    {
        func_800BD520(0, 0x2F08, 0x80);
    }
    else
    {
        func_800BD520(0, 0x2F08, 0xFF);
    }
}

/**
 * @brief Set the selected shared item record aside and give it the pending result.
 *
 * Its saved active byte goes to the cursor slot table at D_80122C19, the
 * active byte is kept in saved_active, and the word at D_80122C08 becomes
 * the record's value.
 */
void field_menu_set_aside_shared_item(void)
{
    s32 index;
    FieldMenuItemRecord* record;
    u8 saved;
    u8 active;

    index = FIELD_LOCAL_BYTE(FIELD_RECORD_INDEX);
    saved = g_field_shared_items[index].saved_active;
    record = &g_field_shared_items[index];
    FIELD_LOCAL_BYTE(FIELD_SET_ASIDE_ACTIVE + index) = saved;
    active = record->active;
    record->active = 0;
    record->saved_active = active;
    record->unknown_0x34 = FIELD_LOCAL_WORD(FIELD_RECORD_RESULT);
}

/**
 * @brief Pick one of the eight elements, weighted by the current land's element levels.
 *
 * Each element weighs (level - 3)^4 with the level clamped to 0..3 after the
 * offset, one level more for the favored element. Mode 0 (local 0) draws by
 * weight and stores the element in local 5; any other mode draws uniformly
 * and stores it in local 6. 0xFF means no element was drawn.
 * @note Written with gotos: both loops must stay out of loop.c's reach, as
 *       the original's did (a structured loop hoists the table addresses).
 */
void field_menu_draw_land_element(void)
{
    s32 weights[8];
    FieldFavoredElementTable favored;
    s32 total, i, row, level, weight, draw;
    FieldViewOrElement work;
    s32 *cursor, *base;
    s32 land, byte_offset, product;
    s32 mode;

    total = 0;
    i = total;
    work.view = (FieldElementLevelView*)g_saved_game.bytes;
    mode = D_80122C00.bytes[0];
    land = g_music_track_index;
    favored = D_80051ED8;
    row = land * FIELD_LAND_RECORD_SIZE;
next_weight:
    level = ((u8*)work.view)[i + row + FIELD_LAND_LEVELS_OFFSET];
    weight = level - 3;
    if (i == favored.elements[work.view->favored_index & 0x7F])
    {
        weight = level - 2;
    }
    if (weight >= 0)
    {
        level = 3;
        if (weight < 4)
        {
            level = weight;
        }
    }
    else
    {
        level = 0;
    }
    weight = level * level;
    weight = weight * weight;
    byte_offset = i * 4;
    i++;
    base = weights;
    *(s32*)((u8*)base + byte_offset) = weight;
    total += weight;
    if (i < 8)
    {
        goto next_weight;
    }
    if (mode == 1)
    {
        total = FIELD_ELEMENT_UNIFORM_TOTAL;
    }
    product = rand() * total;
    draw = product >> 15;
    if (product < 0)
    {
        draw = (product + 0x7FFF) >> 15;
    }
    total = 0;
    work.element = FIELD_NO_ELEMENT;
    i = total;
    cursor = base;
next_draw:
    if (draw >= total && draw < total + *cursor)
    {
        goto found;
    }
    if (mode == 0)
    {
        total += *cursor;
    }
    else
    {
        total += FIELD_ELEMENT_UNIFORM_WEIGHT;
    }
    i++;
    cursor++;
    if (i < 8)
    {
        goto next_draw;
    }
store:
    if (mode != 0)
    {
        goto store_uniform;
    }
    FIELD_LOCAL_BYTE(FIELD_DRAWN_ELEMENT) = work.element;
    return;
found:
    work.element = i;
    goto store;
store_uniform:
    FIELD_LOCAL_BYTE(FIELD_UNIFORM_ELEMENT) = work.element;
}

/** @brief Open the attribute selection screen sequence. */
void func_800C9894(void)
{
    FieldGosubSequence local;

    local = D_80051EF8;
    field_open_gosub_screen_sequence(&local);
}

/**
 * @brief Describe the gosub-selected inventory record in script locals 1-4.
 *
 * Local 1 gets the record index, local 2 its category (attribute bits
 * 10-15), local 3 its first stat byte and local 4 its third stat byte
 * clamped to 0..99. Without a selection they are 0xFF, 0xFF, 0xFF and 0.
 */
void field_menu_describe_selected_item(void)
{
    s32 index;
    s32 category;
    s32 stat_byte0;
    s32 stat_byte2;
    FieldMenuItemData* items;
    FieldMenuItemRecord* inventory;
    FieldMenuItemRecord* record;

    index = 0xFF;
    category = 0xFF;
    stat_byte0 = 0xFF;
    stat_byte2 = 0;
    if (g_gosub_result_count != 0)
    {
        items = FIELD_MENU_ITEMS;
        index = g_gosub_result_values[0];
        category = (items->inventory[index].attributes.packed >> 10) & 0x3F;
        inventory = items->inventory;
        record = &inventory[index];
        stat_byte0 = record->stats.bytes[0];
        stat_byte2 = record->stats.bytes[2];
    }
    stat_byte2 = FIELD_CLAMP(stat_byte2, 0, 99);
    FIELD_LOCAL_BYTE(FIELD_ITEM_DESC_INDEX) = index;
    FIELD_LOCAL_BYTE(FIELD_ITEM_DESC_INDEX + 1) = category;
    FIELD_LOCAL_BYTE(FIELD_ITEM_DESC_INDEX + 2) = stat_byte0;
    FIELD_LOCAL_BYTE(FIELD_ITEM_DESC_INDEX + 3) = stat_byte2;
}

/**
 * @brief Scale the two values in the script locals by level, category and element affinity.
 *
 * Each value is divided by level + 6, raised by amount * category factor *
 * element factor and multiplied by level + 7, then clamped to 0..0x7FFF.
 * The category factor is 10 when the selected category is the value's first
 * category, 1 when it is its second and 2 otherwise. The element factor is 3
 * for the same element, 1 for the element's pair in D_80051F04 and 2
 * otherwise, or always 2 in neutral mode.
 */
void field_menu_apply_affinity(void)
{
    FieldElementTable pairs;
    s32 packed; /* the packed categories, then the packed elements */
    s32 first;
    s32 second;
    s32 element;
    s32 amount;
    s32 level;
    s32 first_category;
    s32 first_alt_category;
    s32 second_category;
    s32 second_alt_category;
    s32 first_element;
    s32 second_element;
    s32 selected;
    s32 neutral;
    s32 first_category_factor;
    s32 second_category_factor;
    s32 first_element_factor;
    s32 second_element_factor;

    pairs = D_80051F04;
    first_category_factor = 10;
    packed = FIELD_LOCAL_BYTE(FIELD_AFFINITY_CATEGORIES);
    first = FIELD_LOCAL_HALF(FIELD_AFFINITY_FIRST_VALUE);
    second = FIELD_LOCAL_HALF(FIELD_AFFINITY_SECOND_VALUE);
    element = FIELD_LOCAL_BYTE(FIELD_AFFINITY_ELEMENT);
    amount = FIELD_LOCAL_BYTE(FIELD_AFFINITY_AMOUNT);
    level = FIELD_LOCAL_BYTE(FIELD_AFFINITY_LEVEL);
    first_category = packed & 3;
    first_alt_category = (packed >> 2) & 3;
    second_category = (packed >> 4) & 3;
    second_alt_category = (packed >> 6) & 3;
    packed = FIELD_LOCAL_BYTE(FIELD_AFFINITY_ELEMENTS);
    first_element = packed & 0xF;
    second_element = (packed >> 4) & 0xF;
    selected = FIELD_LOCAL_BYTE(FIELD_AFFINITY_SELECTED_CATEGORY);
    neutral = FIELD_LOCAL_BYTE(FIELD_AFFINITY_NEUTRAL);
    if (selected != first_category)
    {
        first_category_factor = 2;
        if (selected == first_alt_category)
        {
            first_category_factor = 1;
        }
    }
    second_category_factor = 10;
    if (selected != second_category)
    {
        second_category_factor = 2;
        if (selected == second_alt_category)
        {
            second_category_factor = 1;
        }
    }
    if (neutral == 0)
    {
        if (element == first_element)
        {
            first_element_factor = 3;
        }
        else if (element == pairs.elements[first_element])
        {
            first_element_factor = 1;
        }
        else
        {
            first_element_factor = 2;
        }
        if (element == second_element)
        {
            second_element_factor = 3;
        }
        else if (element == pairs.elements[second_element])
        {
            second_element_factor = 1;
        }
        else
        {
            second_element_factor = 2;
        }
    }
    else
    {
        first_element_factor = 2;
        second_element_factor = 2;
    }
    first /= level + 6;
    second /= level + 6;
    first += first_category_factor * first_element_factor * amount;
    second += second_category_factor * second_element_factor * amount;
    first *= level + 7;
    second *= level + 7;
    first = FIELD_CLAMP(first, 0, FIELD_AFFINITY_VALUE_MAX);
    second = FIELD_CLAMP(second, 0, FIELD_AFFINITY_VALUE_MAX);
    FIELD_LOCAL_HALF(FIELD_AFFINITY_FIRST_VALUE) = first;
    FIELD_LOCAL_HALF(FIELD_AFFINITY_SECOND_VALUE) = second;
}

/**
 * @brief Copy item record D_80122C00 of resource 5 into a free inventory record.
 * @note Also publishes the copied record's name to text macro 0.
 */
void field_menu_copy_resource_item(void)
{
    s32 index;
    u8* free_record;
    s32 offset;

    index = D_80122C00.bytes[0];
    if (field_find_free_inventory_record() != 0)
    {
        free_record = field_find_free_inventory_record();
        field_copy_inventory_record(free_record, func_800C1E40(FIELD_ITEM_RECORD_RESOURCE) + (offset = FIELD_RESOURCE_RECORD_OFFSET(index)));
        field_set_text_macro(0, func_800C1E40(FIELD_ITEM_RECORD_RESOURCE) + offset, 0xFF);
    }
}

/**
 * @brief Swap the current small history record into the cursor slot and restrict it.
 *
 * Returns the record the cursor slot (local 0x1C) held as the gosub
 * result, puts the current small history index in that slot, stores the
 * record's entry id in local halfword 0xA, restricts its selection and
 * publishes its name to the text macro named by local 0x1C.
 */
void field_menu_swap_cursor_history(void)
{
    s32 slot;
    u8* entry;
    s32 previous;
    s32 index;

    field_menu_clear_gosub_request();
    slot = FIELD_LOCAL_BYTE(FIELD_CURSOR_SLOT);
    entry = &FIELD_LOCAL_BYTE(FIELD_CURSOR_SLOT_TABLE + slot);
    previous = *entry;
    g_gosub_result_count = 1;
    index = FIELD_PAD.small_history_index;
    g_gosub_result_values[0] = previous;
    if (index < SMALL_HISTORY_RECORD_COUNT)
    {
        *entry = index;
        FIELD_LOCAL_HALF(FIELD_CURSOR_ENTRY_ID) = FIELD_MENU_HISTORY->small_history_records[index].entry_id;
        FIELD_PAD.small_history_records[index].selection_flags.selection_restricted = 1;
        field_set_text_macro(slot, FIELD_PAD.small_history_records[index].name, 0xFF);
    }
}

/**
 * @brief Publish the name of the small history record in the cursor slot to text macro 3.
 * @note Local 0x1C is replaced by the record's extra slot id at 0x48.
 */
void field_menu_publish_cursor_history(void)
{
    s32 index;

    index = FIELD_LOCAL_BYTE(FIELD_CURSOR_SLOT_TABLE + FIELD_LOCAL_BYTE(FIELD_CURSOR_SLOT));
    FIELD_LOCAL_BYTE(FIELD_CURSOR_SLOT) = FIELD_MENU_HISTORY->small_history_records[index].unknown_0x48[0];
    field_set_text_macro(3, FIELD_PAD.small_history_records[index].name, 0xFF);
}

/** @brief Count the occupied small history records into D_80122C16. */
void field_menu_count_history_records(void)
{
    s32 i;
    s32 count;

    count = 0;
    for (i = 0; i < SMALL_HISTORY_RECORD_COUNT; i++)
    {
        if (FIELD_PAD.small_history_records[i].name[0] != 0)
        {
            count++;
        }
    }
    D_80122C16 = count;
}

/**
 * @brief Forward D_80122C01 to field_open_carda after the common menu prologue.
 */
void func_800C9D84(void)
{
    field_menu_clear_gosub_request();
    field_open_carda(D_80122C01);
}

/**
 * @brief Store the high nibble of D_800459AC into D_80122C12.
 */
void field_menu_load_golem_saved_group(void)
{
    D_80122C12 = g_saved_game.bytes[FIELD_GOLEM_COUNTS] >> 4;
}

/**
 * @brief Publish inventory record D_80122C10 and its two description texts.
 *
 * The record's name goes to text macro 0. Its first two stat bytes select
 * text row * 14 + column of resource 0x103 (macro 1) and text column of
 * resource 0x104 (macro 2).
 */
void field_menu_publish_item_texts(void)
{
    FieldMenuItemRecord* records;
    FieldMenuItemRecord* record;
    s32 index;
    s32 row;
    s32 column;
    s32 entry;
    s32 offset;

    index = D_80122C10;
    records = FIELD_MENU_ITEMS->inventory;
    record = records + index;
    row = record->stats.bytes[0];
    column = record->stats.bytes[1];
    entry = row * 14 + column;
    field_set_text_macro(0, &record->active, 0xFF);
    offset = FIELD_TEXT_OFFSET(FIELD_ITEM_ENTRY_TEXT_RESOURCE, entry);
    field_set_text_macro(1, FIELD_TEXT_RESOURCE(FIELD_ITEM_ENTRY_TEXT_RESOURCE)->texts + offset, 0xFF);
    offset = FIELD_TEXT_OFFSET(FIELD_ITEM_COLUMN_TEXT_RESOURCE, column);
    field_set_text_macro(2, FIELD_TEXT_RESOURCE(FIELD_ITEM_COLUMN_TEXT_RESOURCE)->texts + offset, 0xFF);
}
