#include "common.h"
#include <abs.h>

#include "../internal/field_calls.h"
#include "../internal/field_menu_vars.h"
#include "../internal/field_records.h"
#include "../internal/field_script.h"
#include "common/saved_game.h"
#include "main/audio/game_audio.h"
#include "main/main.h"
#include "overlays/field/field_actor_records.h"
#include "overlays/field/field_overlay_handoff.h"

void field_reset_actor_at(s32 key, s32 resource_entry_index, s32 group, s32 x, s32 y, s32 z);

/** @brief Value at which SavedGameLayout::golems_created saturates. */
#define FIELD_GOLEMS_CREATED_MAX 200

/** @brief golem_order and joined_golem value for no golem record. */
#define FIELD_GOLEM_NO_RECORD 3

/** @brief Offset of the joined golem's class, byte 1 of characters[2].info. */
#define FIELD_ACTIVE_GOLEM_CLASS 0xAA9

/** @brief FIELD_VARIABLE_GUEST_VARIANT value meaning a guest hero is loaded. */
#define FIELD_GUEST_PRESENT 0x80

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
#define FIELD_MENU_OP_DESCRIBE_SELECTED_PET 0x27
#define FIELD_MENU_OP_SEND_SELECTED_PET_GRAZING 0x29
#define FIELD_MENU_OP_PUBLISH_PET_PENDING_EFFECTS 0x2E
#define FIELD_MENU_OP_ADD_PET_PENDING_EFFECT 0x30
#define FIELD_MENU_OP_RECALL_JOINED_PET 0x32
#define FIELD_MENU_OP_RECALL_CURSOR_PET 0x4B
#define FIELD_MENU_OP_HATCH_PET 0x4C

/* Script locals read by field_menu_apply_affinity (byte indices unless noted). */
#define FIELD_AFFINITY_ELEMENT 3           /**< Element of the applied source. */
#define FIELD_AFFINITY_AMOUNT 4            /**< Amount added per affinity point. */
#define FIELD_AFFINITY_ELEMENTS 5          /**< Elements of the two values, one per nibble. */
#define FIELD_AFFINITY_CATEGORIES 6        /**< Four 2-bit categories: first, first alt, second, second alt. */
#define FIELD_AFFINITY_SELECTED_CATEGORY 7 /**< Category of the applied source. */
#define FIELD_AFFINITY_LEVEL 8
#define FIELD_AFFINITY_NEUTRAL 10     /**< Nonzero: every element factor is 2. */
#define FIELD_AFFINITY_FIRST_VALUE 8  /**< Halfword index (D_80122C10). */
#define FIELD_AFFINITY_SECOND_VALUE 9 /**< Halfword index (D_80122C12). */

/* Script locals written by field_menu_read_ring_selection (halfword indices). */
#define FIELD_RING_ENTRY 0xA /**< Selected ring entry. */
#define FIELD_RING_BUSY 0xB  /**< 1 while the ring selection is still running. */

/* Script locals read by field_menu_add_logic_block (halfword indices). */
#define FIELD_NEW_BLOCK_SHAPE 0xC
#define FIELD_NEW_BLOCK_LEVEL 0xD
#define FIELD_NEW_BLOCK_ID 0xE

/** @brief FIELD_NEW_BLOCK_ID value that clears the logic-block table. */
#define FIELD_NEW_BLOCK_CLEAR 0xFF

/* Script locals of the action item menus (byte indices unless noted). */
#define FIELD_ACTION_ITEM_COUNTS 0     /**< Eight bytes: owned count of each action item. */
#define FIELD_ACTION_USED_SLOTS 0xB    /**< Item slots in use in the active group. */
#define FIELD_ACTION_GROUP_FULL 0xC    /**< Set when fewer than three item slots stay free. */
#define FIELD_ACTION_SELECTED_ITEM 0xA /**< Halfword index: chosen action item, 0-7. */

/** @brief Size of a land record (field_records.h FieldLandRecord). */
#define FIELD_LAND_RECORD_SIZE sizeof(FieldLandRecord)

/** @brief Game-state offset of land 0's element levels (SavedGameLayout lands[0].levels). */
#define FIELD_LAND_LEVELS_OFFSET OFFSETOF(SavedGameLayout, lands[0].levels)

/** @brief Weight of each element in field_menu_draw_land_element's uniform mode (3^4). */
#define FIELD_ELEMENT_UNIFORM_WEIGHT 81

/** @brief Total weight of the eight elements in uniform mode. */
#define FIELD_ELEMENT_UNIFORM_TOTAL (FIELD_LAND_SPIRIT_COUNT * FIELD_ELEMENT_UNIFORM_WEIGHT)

/** @brief Scaling denominator for the 15-bit random element draw. */
#define FIELD_ELEMENT_RANDOM_RANGE 0x8000

/** @brief Element value for no element. */
#define FIELD_NO_ELEMENT 0xFF

/** @brief Mystic Card id of an empty card slot. */
#define FIELD_NO_CARD 0xFF

/* Script locals of the pending and shared item record menus (byte indices unless noted). */
#define FIELD_RECORD_INDEX 2             /**< Selected record. */
#define FIELD_RECORD_STATUS 3            /**< 0 shown, 1 empty, 2 duplicate. */
#define FIELD_PENDING_COUNT 6            /**< Number of active pending records. */
#define FIELD_RECORD_RESULT 2            /**< Word index: pending result given to a set-aside record. */
#define FIELD_SET_ASIDE_SPECIAL_IDS 0x19 /**< Four bytes: special_ids[0] of each set-aside shared record. */

/* Script locals of the pet ranch menus (byte indices). */
#define FIELD_HATCH_SPECIES 0x11   /**< Species field_menu_hatch_pet gives the egg. */
#define FIELD_HATCH_PET_INDEX 0x12 /**< Pet record field_menu_hatch_pet hatches. */
#define FIELD_RECALL_SLOT 0x19     /**< Cursor slot whose pet field_menu_recall_cursor_pet brings back. */

/** @brief Halfword local index of the three card ids left over by field_menu_remove_record_cards. */
#define FIELD_LEFTOVER_CARDS 3

/** @brief Golem slot status of a slot holding no golem, or the joined golem. */
#define FIELD_GOLEM_SLOT_EMPTY 3
/** @brief Golem slot status of an empty slot shown at display @p position. */
#define FIELD_GOLEM_SLOT_POSITION(position) (4 + (position))

/* Script locals written by field_menu_refresh_golem_order (halfword indices). */
#define FIELD_GOLEM_SLOT_STATUS 3       /**< Three halfwords: golem record per slot, or a FIELD_GOLEM_SLOT_* status. */
#define FIELD_GOLEM_ACTIVE_POSITION 0xE /**< Display position of the selected slot. */
#define FIELD_GOLEM_ACTIVE_RECORD 0xF   /**< Active golem record index. */

/** @brief Byte local index of the four values field_menu_describe_selected_item stores. */
#define FIELD_ITEM_DESC_INDEX 1

/* Script locals written by field_menu_draw_land_element (byte indices). */
#define FIELD_DRAWN_ELEMENT 5   /**< Element drawn by weight (mode 0). */
#define FIELD_UNIFORM_ELEMENT 6 /**< Element drawn uniformly (other modes). */

/* Script locals of the grazing pet cursor menus (field_menu_swap_joined_pet, field_menu_publish_cursor_pet). */
#define FIELD_CURSOR_SPECIES 0xA     /**< Halfword index: species of the pet sent grazing. */
#define FIELD_CURSOR_SLOT 0x1C       /**< Selected cursor slot; also the text macro index. */
#define FIELD_CURSOR_SLOT_TABLE 0x1D /**< Pet record index shown in each cursor slot. */

/** @brief Text resource @p id. */
#define FIELD_TEXT_RESOURCE(id) ((FieldTextResource*)field_find_resource(id))

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
#define FIELD_RESOURCE_RECORD_OFFSET(index) ((index) * sizeof(FieldItemRecord) + 4)

/** @brief Largest value field_menu_apply_affinity stores. */
#define FIELD_AFFINITY_VALUE_MAX 0x7FFF

/**
 * @brief Text @p id of the menu text table at g_field_item_name_table.
 * @note The table starts with little-endian 16-bit offsets relative to itself.
 */
#define FIELD_MENU_TEXT(id)                                                                                                                                    \
    (g_field_item_name_table.bytes[(id) * sizeof(u16)] + (g_field_item_name_table.bytes[(id) * sizeof(u16) + 1] << 8) + g_field_item_name_table.bytes)

/** @brief Item id of the first of the eight menu action items. */
#define FIELD_ACTION_ITEM_BASE 0x58

/** @brief The action item choice variables, which start at D_80122C08. */
#define FIELD_MENU_CHOICES ((FieldMenuChoiceVars*)&D_80122C08)

/** @brief The record selection variables, which start at D_80122C02. */
#define FIELD_MENU_RECORD_SELECT ((FieldMenuRecordSelectVars*)&D_80122C02)

/** @brief The item description variables, which start at D_80122C04. */
#define FIELD_MENU_ITEM_INFO ((FieldMenuItemInfoVars*)&D_80122C04)

/** @brief The action slot variables, which start at D_80122C0D. */
#define FIELD_MENU_ACTION_SLOT ((FieldMenuActionSlotVars*)&D_80122C0D)

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

/** @brief FieldMenuSlotGroup::flags fields: four-bit slot capacity (bits 0-3), item slot count (8-11) and item slots in use (12-15). */
#define FIELD_MENU_GROUP_FIELD_MASK 0xF
#define FIELD_MENU_GROUP_ITEM_SLOTS_SHIFT 8
#define FIELD_MENU_GROUP_USED_SHIFT 12
#define FIELD_MENU_GROUP_USED_MASK (FIELD_MENU_GROUP_FIELD_MASK << FIELD_MENU_GROUP_USED_SHIFT)

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
    u8 item_index; /**< item of the slot, read by field_menu_load_action_slot. */
    u8 handle;     /**< low byte of the slot handle. */
    u8 unk11;
    u8 group; /**< action group index. */
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
#if defined(VERSION_JP)
    u8 value;   /**< Displayed stat value. */
    u8 special; /**< Special-item flag or instrument spirit. */
#else
    u16 value; /**< Displayed stat value; bit 15 marks a special item. */
#endif
    u32 amount; /**< Item value. */
} FieldMenuItemInfoVars;

/**
 * @brief Name byte of a set-aside shared or pending item record, kept in special_ids[0].
 * @param record FieldItemRecord lvalue; its special id is parked in the FIELD_SET_ASIDE_SPECIAL_IDS locals meanwhile.
 */
#define FIELD_ITEM_SET_ASIDE_NAME(record) ((record).special_ids[0])

/** @brief Favored element for each saved-game weekday. */
typedef struct
{
    s32 elements[8];
} FieldFavoredElementTable;

/** @brief One local that holds the game-state view, then the chosen element. */
typedef union
{
    SavedGameLayout* view;
    s32 element;
} FieldViewOrElement;

/** @brief Element table indexed by element: entry e is the element paired with e (0-8). */
typedef struct
{
    s32 elements[9];
} FieldElementTable;

/**
 * @brief A text resource (field_find_resource): a 4-byte header, then the texts.
 * @note The texts start with a table of little-endian 16-bit offsets, one per
 *       text, relative to the start of the texts.
 */
typedef struct
{
    u8 header[4];
    u8 texts[1];
} FieldTextResource;

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
extern FieldGosubSequence D_80051EB4;
extern void (*g_field_menu_ops[])(s32 arg0);
extern FieldGosubSequence D_800F19AC;
extern FieldGosubSequence D_800F19B8;
extern FieldGosubSequence D_800F19C4;
extern FieldPaletteSlotTable D_80051CBC;
extern FieldGosubSequence D_800F19CC;
extern FieldLogicClassTable D_80051CE4;
extern FieldLogicClassTable D_80051DCC;
extern FieldGosubSequence D_80051EC0;
extern FieldGosubSequence D_80051ECC;
extern void field_menu_clear_item_slots(void);
extern s32 rand(void);
s32 field_get_actor_facing(s32 arg0);
extern void field_menu_compact_pending_items(void);
extern FieldFavoredElementTable g_field_weekday_favored_elements;
extern FieldGosubSequence D_80051EF8;
extern FieldElementTable D_80051F04;
void field_menu_clear_gosub_request(void);

/**
 * @brief Increment each of the eight counters of action slot @p slot once.
 * @param slot An action slot lvalue (FieldMenuSlot).
 */
#define FIELD_STEP_ACTION_COUNTERS(slot)                                                                                                                       \
    ((slot).counters[0]++, (slot).counters[1]++, (slot).counters[2]++, (slot).counters[3]++, (slot).counters[4]++, (slot).counters[5]++, (slot).counters[6]++, \
     (slot).counters[7]++)

/** @brief First item-name rows of the armor and instrument types (the weapon types use rows 0 to FIELD_WEAPON_TYPE_COUNT - 1). */
#define FIELD_ITEM_ROW_ARMOR FIELD_WEAPON_TYPE_COUNT
#define FIELD_ITEM_ROW_INSTRUMENT 0x17

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
    SavedGameLayout* ctx;

    ctx = &g_saved_game.layout;

    if (ctx->golem_order[D_80122C00.locals.golem.slot] == FIELD_GOLEM_NO_RECORD)
    {
        if ((u8)ctx->joined_golem >= GOLEM_RECORD_COUNT)
        {
            D_80122C00.locals.golem.state = 1;
            return;
        }

        D_80122C00.locals.golem.state = 2;
        return;
    }

    if (ctx->golem_order[D_80122C00.locals.golem.slot] == ctx->joined_golem)
    {
        D_80122C00.locals.golem.state = 3;
        return;
    }

    D_80122C00.locals.golem.state = 4;
}

/**
 * @brief Clear the gosub-screen request word.
 */
void field_menu_clear_gosub_request(void)
{
    g_field_gosub_state = 0;
}

/**
 * @brief Open the GOSUB golem parts screen, then its confirmation dialog.
 * @note field_menu_create_golem reads the chosen items from the GOSUB results.
 */
void field_menu_choose_golem_parts(void)
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

    free_record = FIELD_GOLEM_NO_RECORD;

    for (i = 0; i < GOLEM_RECORD_COUNT; i++)
    {
        candidate = i;

        for (slot = 0; slot < GOLEM_RECORD_COUNT; slot++)
        {
            if (g_saved_game.layout.golem_order[slot] == i)
            {
                candidate = FIELD_GOLEM_NO_RECORD;
            }
        }

        if (candidate != FIELD_GOLEM_NO_RECORD)
        {
            free_record = candidate;
        }
    }

    if (free_record != FIELD_GOLEM_NO_RECORD)
    {
        field_golem_build_group_record(free_record);
        count = g_saved_game.layout.golems_created + 1;
        g_saved_game.layout.golems_created = count;

        if (count > FIELD_GOLEMS_CREATED_MAX)
        {
            g_saved_game.layout.golems_created = FIELD_GOLEMS_CREATED_MAX;
        }

        /* A golem already in the slot moves to the first empty slot. */
        previous = g_saved_game.layout.golem_order[D_80122C00.locals.golem.slot];

        if (previous != FIELD_GOLEM_NO_RECORD)
        {
            if (g_saved_game.layout.golem_order[0] == FIELD_GOLEM_NO_RECORD)
            {
                g_saved_game.layout.golem_order[0] = previous;
            }
            else if (g_saved_game.layout.golem_order[1] == FIELD_GOLEM_NO_RECORD)
            {
                g_saved_game.layout.golem_order[1] = previous;
            }
            else if (g_saved_game.layout.golem_order[2] == FIELD_GOLEM_NO_RECORD)
            {
                g_saved_game.layout.golem_order[2] = previous;
            }
        }

        g_saved_game.layout.golem_order[D_80122C00.locals.golem.slot] = free_record;
        /* The count nibble is updated with word read-modify-writes. */
        packed_counts = (SAVED_GAME_GOLEM_HEADER_WORD(g_saved_game.layout) & ~GOLEM_COUNT_MASK) |
                        (((g_saved_game.layout.golem_count & GOLEM_COUNT_MASK) + 1) & GOLEM_COUNT_MASK);
        SAVED_GAME_GOLEM_HEADER_WORD(g_saved_game.layout) = packed_counts;

        if ((g_saved_game.layout.golem_count & GOLEM_COUNT_MASK) > GOLEM_RECORD_COUNT)
        {
            SAVED_GAME_GOLEM_HEADER_WORD(g_saved_game.layout) = (packed_counts & ~GOLEM_COUNT_MASK) | GOLEM_RECORD_COUNT;
        }

        for (i = 0; i < g_gosub_result_count; i++)
        {
            field_copy_inventory_record(&g_saved_game.layout.golem_records[g_saved_game.layout.golem_order[D_80122C00.locals.golem.slot]].source_items[i],
                                        &g_saved_game.layout.items[g_gosub_result_values[i]]);
            g_saved_game.layout.items[g_gosub_result_values[i]].name[0] = 0;
        }

        field_compact_inventory();

        for (i = g_gosub_result_count; i < GOLEM_SOURCE_ITEM_COUNT; i++)
        {
            g_saved_game.layout.golem_records[g_saved_game.layout.golem_order[D_80122C00.locals.golem.slot]].source_items[i].name[0] = 0;
        }

        field_upload_golem_palettes();
    }
}

/**
 * @brief Open the GOSUB block components screen, then its confirmation dialog.
 */
void field_menu_choose_block_components(void)
{
    field_open_gosub_screen_sequence(&D_800F19B8);
}

/**
 * @brief Open the GOSUB logic blocks screen.
 */
void field_menu_choose_logic_blocks(void)
{
    field_open_gosub_screen_sequence(&D_800F19C4);
}

/**
 * @brief Open the GOLEM logic-block editor at the end of golem creation.
 * @note Identical to field_menu_edit_golem_logic; this op follows the creation ops 0x02-0x05.
 */
void field_menu_edit_new_golem_logic(void)
{
    field_run_golem();
}

/**
 * @brief Store the selected slot's golem record index and its class nibble.
 */
void field_menu_load_golem_class(void)
{
    SavedGameLayout* ctx;
    s32 record_index;

    ctx = &g_saved_game.layout;
    record_index = ctx->golem_order[D_80122C00.locals.golem.slot];
    D_80122C00.locals.golem.slot_status[0] = record_index;
    D_80122C00.locals.golem.detail = ctx->golem_records[record_index].logic_layout.byte & GOLEM_LOGIC_CLASS_MASK;
}

/**
 * @brief Report whether the golem menu may create, or only view, a golem.
 *
 * Clamps an out-of-range active golem index to 3 (none), then stores 2 when the
 * companion's character type is 3, 0 when no golem is active and 1
 * otherwise.
 */
void field_menu_check_golem_creation(void)
{
    if ((u8)g_saved_game.layout.joined_golem > FIELD_GOLEM_NO_RECORD)
    {
        g_saved_game.layout.joined_golem = FIELD_GOLEM_NO_RECORD;
    }

    if ((g_saved_game.layout.characters[FIELD_PARTY_COMPANION].info.word & FIELD_CHARACTER_TYPE_MASK) == 3)
    {
        D_80122C00.locals.golem.result = 2;
    }
    else if (g_saved_game.layout.joined_golem == FIELD_GOLEM_NO_RECORD)
    {
        D_80122C00.locals.golem.result = 0;
    }
    else
    {
        D_80122C00.locals.golem.result = 1;
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

    for (i = 0; i < g_saved_game.layout.logic_block_count; i++)
    {
        if (g_saved_game.layout.logic_blocks[i].f.logic_type == g_saved_game.layout.golem_order[D_80122C00.locals.golem.slot])
        {
            g_saved_game.layout.logic_blocks[i].f.logic_type = LOGIC_BLOCK_UNASSIGNED;
            g_saved_game.layout.logic_blocks[i].f.placed = 0;
        }
    }

    for (i = 0; i < GOLEM_SOURCE_ITEM_COUNT; i++)
    {
        if (g_saved_game.layout.golem_records[g_saved_game.layout.golem_order[D_80122C00.locals.golem.slot]].source_items[i].name[0] != 0)
        {
            if (field_find_free_inventory_record() != 0)
            {
                field_copy_inventory_record(field_find_free_inventory_record(),
                                            &g_saved_game.layout.golem_records[g_saved_game.layout.golem_order[D_80122C00.locals.golem.slot]].source_items[i]);
            }
        }
    }

    g_saved_game.layout.golem_records[g_saved_game.layout.golem_order[D_80122C00.locals.golem.slot]].name[0] = 0;
    g_saved_game.layout.golem_order[D_80122C00.locals.golem.slot] = FIELD_GOLEM_NO_RECORD;
    word = SAVED_GAME_GOLEM_HEADER_WORD(g_saved_game.layout);

    if ((word & GOLEM_COUNT_MASK) != 0)
    {
        SAVED_GAME_GOLEM_HEADER_WORD(g_saved_game.layout) =
            (word & ~GOLEM_COUNT_MASK) | (((g_saved_game.layout.golem_count & GOLEM_COUNT_MASK) - 1) & GOLEM_COUNT_MASK);
    }
}

/**
 * @brief Publish the selected slot's golem record to text macro 0.
 */
void field_menu_publish_golem_name(void)
{
    u8 record_index = g_saved_game.layout.golem_order[D_80122C00.locals.golem.slot];

    field_set_text_macro(0, g_saved_game.layout.golem_records[record_index].name, FIELD_TEXT_MACRO_UNLIMITED);
}

/**
 * @brief Clear the golem menu result when the gosub screen returned nothing.
 */
void field_menu_clear_golem_result(void)
{
    if (g_gosub_result_count == 0)
    {
        D_80122C00.locals.golem.result = 0;
    }
}

/**
 * @brief Repair the group display order and publish the current selection state.
 */
void field_menu_refresh_golem_order(void)
{
    s32 order[GOLEM_RECORD_COUNT];
    s32 inverse[GOLEM_RECORD_COUNT];
    s32 packed_order;
    s32 i;
    s32 active_index;
    s8 selected_index;
    s32 j;

    FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS) = FIELD_GOLEM_SLOT_EMPTY;
    FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS + 1) = FIELD_GOLEM_SLOT_EMPTY;
    FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS + 2) = FIELD_GOLEM_SLOT_EMPTY;

    if (g_saved_game.layout.golem_order[0] != g_saved_game.layout.joined_golem)
    {
        FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS) = g_saved_game.layout.golem_order[0];
    }

    if (g_saved_game.layout.golem_order[1] != g_saved_game.layout.joined_golem)
    {
        FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS + 1) = g_saved_game.layout.golem_order[1];
    }

    if (g_saved_game.layout.golem_order[2] != g_saved_game.layout.joined_golem)
    {
        FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS + 2) = g_saved_game.layout.golem_order[2];
    }

    packed_order = g_saved_game.layout.golem_display_order;
    order[0] = packed_order & 3;
    order[1] = (packed_order >> 2) & 3;
    order[2] = (packed_order >> 4) & 3;
    order[0] = FIELD_CLAMP(order[0], 0, GOLEM_RECORD_COUNT - 1);
    order[1] = FIELD_CLAMP(order[1], 0, GOLEM_RECORD_COUNT - 1);
    order[2] = FIELD_CLAMP(order[2], 0, GOLEM_RECORD_COUNT - 1);
    /* Drop duplicate positions, then give each dropped slot the first unused position. */
    inverse[0] = FIELD_GOLEM_NO_RECORD;
    inverse[1] = FIELD_GOLEM_NO_RECORD;
    inverse[2] = FIELD_GOLEM_NO_RECORD;

    for (i = 0; i < GOLEM_RECORD_COUNT; i++)
    {
        for (j = 0; j < GOLEM_RECORD_COUNT; j++)
        {
            if (order[i] == j)
            {
                if (inverse[j] == FIELD_GOLEM_NO_RECORD)
                {
                    inverse[j] = i;
                }
                else
                {
                    order[i] = FIELD_GOLEM_NO_RECORD;
                }
            }
        }
    }

    for (i = 0; i < GOLEM_RECORD_COUNT; i++)
    {
        if (order[i] == FIELD_GOLEM_NO_RECORD)
        {
            for (j = 0; j < GOLEM_RECORD_COUNT; j++)
            {
                if (inverse[j] == FIELD_GOLEM_NO_RECORD)
                {
                    order[i] = j;
                    inverse[j] = i;
                    j = GOLEM_RECORD_COUNT; /* stop searching */
                }
            }
        }
    }

    packed_order = order[0] + (order[1] * 4) + (order[2] * 0x10);
    g_saved_game.layout.golem_display_order = packed_order;

    if (FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS) == FIELD_GOLEM_SLOT_EMPTY)
    {
        if (order[0] == 0)
        {
            FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS) = FIELD_GOLEM_SLOT_POSITION(0);
        }

        if (order[1] == 0)
        {
            FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS) = FIELD_GOLEM_SLOT_POSITION(1);
        }

        if (order[2] == 0)
        {
            FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS) = FIELD_GOLEM_SLOT_POSITION(2);
        }
    }

    if (FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS + 1) == FIELD_GOLEM_SLOT_EMPTY)
    {
        if (order[0] == 1)
        {
            FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS + 1) = FIELD_GOLEM_SLOT_POSITION(0);
        }

        if (order[1] == 1)
        {
            FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS + 1) = FIELD_GOLEM_SLOT_POSITION(1);
        }

        if (order[2] == 1)
        {
            FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS + 1) = FIELD_GOLEM_SLOT_POSITION(2);
        }
    }

    if (FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS + 2) == FIELD_GOLEM_SLOT_EMPTY)
    {
        if (order[0] == 2)
        {
            FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS + 2) = FIELD_GOLEM_SLOT_POSITION(0);
        }

        if (order[1] == 2)
        {
            FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS + 2) = FIELD_GOLEM_SLOT_POSITION(1);
        }

        if (order[2] == 2)
        {
            FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS + 2) = FIELD_GOLEM_SLOT_POSITION(2);
        }
    }

    selected_index = g_saved_game.layout.joined_golem;

    if (selected_index < GOLEM_RECORD_COUNT)
    {
        if (g_saved_game.layout.golem_order[0] == selected_index)
        {
            FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS) = FIELD_GOLEM_SLOT_EMPTY;
        }

        if (g_saved_game.layout.golem_order[1] == selected_index)
        {
            FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS + 1) = FIELD_GOLEM_SLOT_EMPTY;
        }

        if (g_saved_game.layout.golem_order[2] == selected_index)
        {
            FIELD_LOCAL_HALF(FIELD_GOLEM_SLOT_STATUS + 2) = FIELD_GOLEM_SLOT_EMPTY;
        }
    }

    active_index = D_80122C00.locals.golem.slot;

    for (i = 0; i < GOLEM_RECORD_COUNT; i++)
    {
        if (order[i] == active_index)
        {
            FIELD_LOCAL_HALF(FIELD_GOLEM_ACTIVE_POSITION) = i;
        }
    }

    FIELD_LOCAL_HALF(FIELD_GOLEM_ACTIVE_RECORD) = g_saved_game.layout.joined_golem;
}

/**
 * @brief Report whether the logic-block table is full (40 blocks).
 */
void field_menu_check_logic_blocks_full(void)
{
    if (g_saved_game.layout.logic_block_count >= LOGIC_BLOCK_CAPACITY)
    {
        D_80122C10 = 1;
    }
    else
    {
        D_80122C10 = 0;
    }
}

/**
 * @brief Open the GOLEM logic-block editor for an existing golem.
 * @note Identical to field_menu_edit_new_golem_logic.
 */
void field_menu_edit_golem_logic(void)
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
        if (g_saved_game.layout.golem_records[g_saved_game.layout.golem_order[D_80122C00.locals.golem.slot]].source_items[i].name[0] != 0)
        {
            field_set_text_macro(count, g_saved_game.layout.golem_records[g_saved_game.layout.golem_order[D_80122C00.locals.golem.slot]].source_items[i].name,
                                 FIELD_TEXT_MACRO_UNLIMITED);
            count++;
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

    for (i = 0; i < FIELD_ITEM_COUNT; i++)
    {
        if (g_saved_game.layout.items[i].name[0] == 0)
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
        remaining = *requested - free_count;
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
    Vec3i position;

    field_get_actor_position(FIELD_MENU_OBJECT->object_id, &position);
    position.x /= 256;
    position.y /= 256;
    position.z /= 256;
    position.y -= FIELD_MENU_OBJECT->height_offset;
    field_set_actor_position(FIELD_MENU_OBJECT->object_id, position.x, position.y, position.z);
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
    Vec3i position;
    s32 target_y;
    s32 target_x;
    s32 target_z;
    s32 y;
    s32 delta;
    s32 distance;
    s32 x;

    field_get_actor_position(FIELD_MENU_OBJECT->object_id, &position);
    x = position.x / 256;
    position.x = x;
    y = position.y / 256;
    position.y = y;
    position.z /= 256;

    target_y = -FIELD_MENU_OBJECT->target_height;
    target_x = FIELD_MENU_OBJECT->target_x;
    target_z = FIELD_MENU_OBJECT->target_z;

    if (y == target_y || y < target_y)
    {
        delta = x - target_x;
        distance = abs(delta);

        if (distance * 2 >= 4)
        {
            position.x = target_x + delta * 2 / 3;
        }
        else if (distance > 0)
        {
            position.x = x - delta / distance;
        }
        else
        {
            position.x = target_x;
        }
    }

    if (position.y == target_y || position.y < target_y)
    {
        delta = position.z - target_z;
        distance = abs(delta);

        if (distance * 2 >= 4)
        {
            position.z = target_z + delta * 2 / 3;
        }
        else if (distance > 0)
        {
            position.z -= delta / distance;
        }
        else
        {
            position.z = target_z;
        }
    }

    if ((position.x == target_x && position.z == target_z) || position.y >= target_y)
    {
        delta = position.y - target_y;
        distance = abs(delta);

        if (distance * 2 >= 4)
        {
            position.y = target_y + delta * 2 / 3;
        }
        else if (distance > 0)
        {
            position.y -= delta / distance;
        }
        else
        {
            position.y = target_y;
        }
    }

    field_set_actor_position(FIELD_MENU_OBJECT->object_id, position.x, position.y, position.z);
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
    selection_low = *g_gosub_result_values;
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

    record_index = g_saved_game.layout.golem_order[D_80122C00.locals.golem.slot];
    g_saved_game.layout.golem_records[record_index].palette = clamped;
    field_upload_golem_palettes();
}

/**
 * @brief Make the golem shown in slot status 0 the active golem.
 */
void field_menu_set_active_golem(void)
{
    s32 status = D_80122C06;

    g_game_diagnostic_status = status;
    g_saved_game.layout.joined_golem = status;
}

/**
 * @brief Store the ring selection in local halfword 0xA, with a busy flag in 0xB.
 * @note While the ring is busy the flag is 1 and the value is the entry the
 *       ring cursor is on.
 */
void field_menu_read_ring_selection(void)
{
    s32 result = field_get_ring_result();

    if (result < 0)
    {
        FIELD_LOCAL_HALF(FIELD_RING_BUSY) = 1;
        FIELD_LOCAL_HALF(FIELD_RING_ENTRY) = field_get_ring_cursor_entry();
    }
    else
    {
        FIELD_LOCAL_HALF(FIELD_RING_BUSY) = 0;
        FIELD_LOCAL_HALF(FIELD_RING_ENTRY) = result;
    }
}

/**
 * @brief Open the GOSUB color material screen, then its confirmation dialog.
 */
void field_menu_choose_color_material(void)
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
    s32 order[GOLEM_RECORD_COUNT];
    s32 selected_position;
    s32 active_slot;
    s32 packed_order;
    s32 selected_slot;
    s8 active_record;

    packed_order = g_saved_game.layout.golem_display_order;
    selected_position = FIELD_GOLEM_NO_RECORD;
    active_slot = FIELD_GOLEM_NO_RECORD;
    order[0] = packed_order & 3;
    order[1] = (packed_order >> 2) & 3;
    order[2] = (packed_order >> 4) & 3;
    active_record = g_saved_game.layout.joined_golem;
    selected_slot = D_80122C00.locals.golem.slot;

    if (g_saved_game.layout.golem_order[selected_slot] != active_record)
    {
        s32 i;
        s32 swap_position;

        for (i = 0; i < GOLEM_RECORD_COUNT; i++)
        {
            if (active_record == g_saved_game.layout.golem_order[i])
            {
                active_slot = i;
            }
        }

        for (i = 0; i < GOLEM_RECORD_COUNT; i++)
        {
            if (order[i] == active_slot)
            {
                swap_position = i;
            }
        }

        for (i = 0; i < GOLEM_RECORD_COUNT; i++)
        {
            if (order[i] == selected_slot)
            {
                selected_position = i;
            }
        }

        order[swap_position] = selected_slot;
        order[selected_position] = active_slot;
        packed_order = order[0] + (order[1] * 4) + (order[2] * 0x10);
        g_saved_game.layout.golem_display_order = packed_order;
    }

    FIELD_MENU_SWAP->selected_position = selected_position;
    FIELD_MENU_SWAP->active_slot = active_slot;
}

/**
 * @brief Publish the inventory record selected by the result variable to text macro 0.
 */
void field_menu_publish_inventory_item(void)
{
    field_set_text_macro(0, g_saved_game.layout.items[D_80122C10].name, FIELD_TEXT_MACRO_UNLIMITED);
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

    for (i = 0; i < FIELD_ITEM_COUNT; i++)
    {
        if (g_saved_game.layout.items[i].name[0] != 0)
        {
            if (FIELD_ITEM_CATEGORY(g_saved_game.layout.items[i].info.word) == kind)
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
        field_copy_inventory_record(field_find_free_inventory_record(), g_saved_game.layout.items);
    }

    g_saved_game.layout.golems_created += 9;
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
    SavedGameLayout* ctx;
    s32 i;
    s32 active_record;
    s32 golem_class;
    s32 block_class;

    class_table = D_80051CE4;
    unused_table = D_80051DCC;

    ctx = &g_saved_game.layout;
    active_record = ctx->joined_golem;
    golem_class = D_80122C1C;

    if (active_record < GOLEM_RECORD_COUNT)
    {
        g_saved_game.bytes[FIELD_ACTIVE_GOLEM_CLASS] = (u8)D_80122C1C;
        /* The class nibble is updated with a word read-modify-write of bytes 0x44-0x47. */
        *(s32*)&ctx->golem_records[active_record].logic_layout =
            (*(s32*)&ctx->golem_records[active_record].logic_layout & ~GOLEM_LOGIC_CLASS_MASK) | ((u8)D_80122C1C & GOLEM_LOGIC_CLASS_MASK);
        for (i = 0; i < ctx->logic_block_count; i++)
        {
            if (ctx->logic_blocks[i].f.placed == 1 && ctx->logic_blocks[i].f.logic_type == active_record)
            {
                block_class = class_table.classes[ctx->logic_blocks[i].f.id];

                if (block_class != 0 && block_class != golem_class)
                {
                    ctx->logic_blocks[i].f.placed = 0;
                    ctx->logic_blocks[i].f.logic_type = LOGIC_BLOCK_UNASSIGNED;
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
    s32 level = FIELD_LOCAL_HALF(FIELD_NEW_BLOCK_LEVEL);
    s32 shape = FIELD_LOCAL_HALF(FIELD_NEW_BLOCK_SHAPE);

    if (id == FIELD_NEW_BLOCK_CLEAR)
    {
        g_saved_game.layout.logic_block_count = 0;
        return;
    }

    if (g_saved_game.layout.logic_block_count < LOGIC_BLOCK_CAPACITY)
    {
        g_saved_game.layout.logic_blocks[g_saved_game.layout.logic_block_count].f.id = id;
        g_saved_game.layout.logic_blocks[g_saved_game.layout.logic_block_count].f.level = level;
        g_saved_game.layout.logic_blocks[g_saved_game.layout.logic_block_count].f.shape = shape;
        g_saved_game.layout.logic_blocks[g_saved_game.layout.logic_block_count].f.logic_type = LOGIC_BLOCK_UNASSIGNED;
        g_saved_game.layout.logic_blocks[g_saved_game.layout.logic_block_count].f.placed = 0;
        g_saved_game.layout.logic_block_count++;
    }
}

/**
 * @brief Run field_select_distance_bucket for the actor in D_80122C1C.
 * @note The selected bucket is discarded, so the only effect is the random draw.
 */
void field_menu_select_distance_bucket(void)
{
    field_select_distance_bucket(D_80122C1C);
}

/**
 * @brief Open the GOSUB equipment screen, then its confirmation dialog.
 */
void field_menu_choose_equipment(void)
{
    FieldGosubSequence sequence;

    sequence = D_80051EB4;
    field_open_gosub_screen_sequence(&sequence);
}

/**
 * @brief Open CARDA to save the game.
 */
void field_menu_open_save_screen(void)
{
    field_open_carda(0);
}

/**
 * @brief Replace the inventory record index in the result variable with its name-table row.
 *
 * The row is the record's item type offset by its category: weapon types come
 * first, then armor types from FIELD_ITEM_ROW_ARMOR, then instrument types from
 * FIELD_ITEM_ROW_INSTRUMENT. A row of 0xFF records a diagnostic and yields 0.
 */
void field_menu_item_name_row(void)
{
    s32 index;
    u32 attributes;
    s32 kind;
    s32 row;

    index = D_80122C10;
    attributes = g_saved_game.layout.items[index].info.word;
    kind = FIELD_ITEM_CATEGORY(attributes);

    if (kind == FIELD_ITEM_CATEGORY_WEAPON)
    {
        row = FIELD_ITEM_TYPE(attributes);
    }
    else if (kind == FIELD_ITEM_CATEGORY_ARMOR)
    {
        row = FIELD_ITEM_TYPE(attributes) + FIELD_ITEM_ROW_ARMOR;
    }
    else
    {
        row = FIELD_ITEM_TYPE(attributes) + FIELD_ITEM_ROW_INSTRUMENT;
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
    first = g_saved_game.layout.items[index].special_ids[0];
    second = g_saved_game.layout.items[index].special_ids[1];
    third = g_saved_game.layout.items[index].special_ids[2];

    for (i = 0; i < 3; i++)
    {
        card = D_80122C00.locals.cards.card_ids[i];

        if (card != FIELD_NO_CARD)
        {
            if (card == first)
            {
                D_80122C00.locals.cards.card_ids[i] = FIELD_NO_CARD;
                first = FIELD_NO_CARD;
            }
            else if (card == second)
            {
                D_80122C00.locals.cards.card_ids[i] = FIELD_NO_CARD;
                second = FIELD_NO_CARD;
            }
            else if (card == third)
            {
                D_80122C00.locals.cards.card_ids[i] = FIELD_NO_CARD;
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
    D_80122C00.locals.cards.card_ids[0] = g_saved_game.layout.items[index].special_ids[0];
    D_80122C00.locals.cards.card_ids[1] = g_saved_game.layout.items[index].special_ids[1];
    D_80122C00.locals.cards.card_ids[2] = g_saved_game.layout.items[index].special_ids[2];
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
            resource = field_find_resource(0x102);
            offset = object_id * 4;
        }
        else
        {
            resource = field_find_resource(0x102);
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
    field_set_text_macro(0, FIELD_TEXT_RESOURCE(FIELD_OBJECT_TEXT_RESOURCE)->texts + offset, FIELD_TEXT_MACRO_UNLIMITED);
}

/**
 * @brief Decode the gosub-selected pet into a display id and mode.
 *
 * An egg shows id 0x53 + its egg species in mode 0; a hatched pet shows
 * 0x12 + its species, in mode 1 while it is grazing. The pet in the party
 * shows 0xFE. Also stores the result count.
 */
void field_menu_describe_selected_pet(void)
{
    s32 index;
    PetStatusFlags flags;
    s32 grazing;

    if (g_gosub_result_count != 0)
    {
        index = g_gosub_result_values[0];

        if (index < PET_RECORD_COUNT)
        {
            flags = g_saved_game.layout.pets[index].status.bits;

            if (flags.egg)
            {
                FIELD_MENU_RECORD->display_id = g_saved_game.layout.pets[index].egg_species + 0x53;
                FIELD_MENU_RECORD->mode = 0;
            }
            else
            {
                grazing = flags.grazing;
                FIELD_MENU_RECORD->display_id = g_saved_game.layout.pets[index].species + 0x12;
                FIELD_MENU_RECORD->mode = grazing;
            }

            if (index == g_saved_game.layout.joined_pet)
            {
                FIELD_MENU_RECORD->display_id = 0xFE;
            }
        }
        else
        {
            record_game_diagnostic(FIELD_DIAG_MENU_OP, FIELD_MENU_OP_DESCRIBE_SELECTED_PET, index, 0);
        }
    }

    D_80122C16 = g_gosub_result_count;
}

/**
 * @brief Count the grazing pets into D_80122C16.
 */
void field_menu_count_grazing_pets(void)
{
    s32 count;
    s32 i;

    if (g_gosub_result_count != 0)
    {
        count = 0;

        for (i = 0; i < PET_RECORD_COUNT; i++)
        {
            if (g_saved_game.layout.pets[i].name[0] != 0 && g_saved_game.layout.pets[i].status.bits.grazing == 1)
            {
                count++;
            }
        }
    }

    D_80122C16 = count;
}

/**
 * @brief Leave the gosub-selected pet grazing at the ranch.
 */
void field_menu_send_selected_pet_grazing(void)
{
    s32 index;

    index = g_gosub_result_values[0];

    if (index < PET_RECORD_COUNT)
    {
        g_saved_game.layout.pets[index].status.bits.grazing = 1;
    }
    else
    {
        record_game_diagnostic(FIELD_DIAG_MENU_OP, FIELD_MENU_OP_SEND_SELECTED_PET_GRAZING, index, 0);
    }
}

/**
 * @brief Open the GOSUB produce screen, then its confirmation dialog.
 */
void field_menu_choose_produce(void)
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

    g_field_gosub_state = 0;
    index = g_gosub_result_values[0];
    FIELD_MENU_RESULT->index = index;
    FIELD_MENU_RESULT->count = g_gosub_result_count;
}

/**
 * @brief Open the GOSUB pet screen, then its confirmation dialog.
 */
void field_menu_choose_pet(void)
{
    FieldGosubSequence sequence;

    sequence = D_80051ECC;
    field_open_gosub_screen_sequence(&sequence);
}

/**
 * @brief Store the gosub-selected pet and publish its name to a text macro.
 *
 * The text macro index comes from the variable at D_80122C02.
 */
void field_menu_publish_selected_pet_name(void)
{
    s32 index;

    FIELD_MENU_NAMED->index = index = g_gosub_result_values[0];
    FIELD_MENU_NAMED->count = g_gosub_result_count;
    field_set_text_macro(FIELD_MENU_NAMED->macro_index, g_saved_game.layout.pets[index].name, FIELD_TEXT_MACRO_UNLIMITED);
}

/**
 * @brief Publish the texts of pet D_80122C10's pending effects and its name.
 * @note Empty and cleared effects are skipped; the number published goes to D_80122C16.
 * @note Selections of five or greater record a diagnostic instead.
 */
void field_menu_publish_pet_pending_effects(void)
{
    s32 selection;
    s32 slot;
    s32 count;
    u8 entry;

    selection = D_80122C10;

    if (selection < PET_RECORD_COUNT)
    {
        count = 0;

        for (slot = 0; slot < FIELD_REGION_PENDING_EFFECT_COUNT; slot++)
        {
            entry = g_saved_game.layout.pets[selection].status.effects[slot];

            if (entry != FIELD_NO_EFFECT)
            {
                if (entry != FIELD_EFFECT_CLEARED)
                {
                    field_set_text_macro(count, FIELD_MENU_TEXT(entry), FIELD_TEXT_MACRO_UNLIMITED);
                    count++;
                }
            }
        }

        field_set_text_macro(3, g_saved_game.layout.pets[selection].name, FIELD_TEXT_MACRO_UNLIMITED);
        D_80122C16 = count;
        return;
    }

    record_game_diagnostic(FIELD_DIAG_MENU_OP, FIELD_MENU_OP_PUBLISH_PET_PENDING_EFFECTS, selection, 0);
}

/**
 * @brief Set the joined golem's random-action chance to zero.
 * @note Clears both the golem record and the companion's copy (byte 2 of its golem data).
 */
void field_menu_clear_golem_random_chance(void)
{
    SavedGameLayout* ctx;
    s32 index;

    ctx = &g_saved_game.layout;
    index = ctx->joined_golem;
    g_saved_game.layout.characters[FIELD_PARTY_COMPANION].unk150[0].derived.bytes[2] = 0;
    ctx->golem_records[index].unknown_0x46 = 0;
}

/**
 * @brief Store effect D_80122C12 in the first empty or cleared pending effect of pet D_80122C10.
 * @note Selections of five or greater record a diagnostic instead.
 */
void field_menu_add_pet_pending_effect(void)
{
    s32 selection;
    s32 value;
    s32 slot;
    u8 entry;

    selection = FIELD_MENU_RECORD->display_id;
    value = FIELD_MENU_RECORD->mode;

    if (selection < PET_RECORD_COUNT)
    {
        for (slot = 0; slot < FIELD_REGION_PENDING_EFFECT_COUNT; slot++)
        {
            entry = g_saved_game.layout.pets[selection].status.effects[slot];

            if (entry == FIELD_EFFECT_CLEARED || entry == FIELD_NO_EFFECT)
            {
                g_saved_game.layout.pets[selection].status.effects[slot] = value;
                return;
            }
        }

        return;
    }

    record_game_diagnostic(FIELD_DIAG_MENU_OP, FIELD_MENU_OP_ADD_PET_PENDING_EFFECT, selection, 0);
}

/** @brief Replace the pet record index in D_80122C10 with that pet's species. */
void field_menu_load_pet_species(void)
{
    D_80122C10 = g_saved_game.layout.pets[D_80122C10].species;
}

/**
 * @brief Bring the pet in the party back from grazing.
 * @note Indices of five or greater record a diagnostic instead.
 */
void field_menu_recall_joined_pet(void)
{
    s32 index;

    index = g_saved_game.layout.joined_pet;

    if (index < PET_RECORD_COUNT)
    {
        g_saved_game.layout.pets[index].status.bits.grazing = 0;
    }
    else
    {
        record_game_diagnostic(FIELD_DIAG_MENU_OP, FIELD_MENU_OP_RECALL_JOINED_PET, index, 0);
    }
}

/**
 * @brief Bring the gosub-selected pet back from grazing.
 * @note Indices of five or greater record a diagnostic instead, under menu
 *       op 0x32 (field_menu_recall_joined_pet's number), not 0x33.
 */
void field_menu_recall_selected_pet(void)
{
    s32 index;

    index = g_gosub_result_values[0];

    if (index < PET_RECORD_COUNT)
    {
        g_saved_game.layout.pets[index].status.bits.grazing = 0;
    }
    else
    {
        record_game_diagnostic(FIELD_DIAG_MENU_OP, FIELD_MENU_OP_RECALL_JOINED_PET, index, 0);
    }
}

/**
 * @brief Load the gosub-selected pet's extra_growth.halves[1] into D_80122C00.locals.
 * @note Does nothing when the gosub returned no selection.
 */
void field_menu_load_pet_value(void)
{
    s32 index;

    if (g_gosub_result_count != 0)
    {
        index = g_gosub_result_values[0];
        D_80122C00.locals.words[0] = g_saved_game.layout.pets[index].extra_growth.halves[1];
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
        counts[i] = g_saved_game.layout.item_counts[FIELD_ACTION_ITEM_BASE + i];

        if (counts[i] != 0)
        {
            count++;
        }
    }

    for (i = 0; i < 8; i++)
    {
        D_80122C00.locals.bytes[i] = counts[i];
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
    flags = g_saved_game.layout.menu_slots[group].flags.word;
    item_slot_count = flags >> FIELD_MENU_GROUP_ITEM_SLOTS_SHIFT;
    item_slot_count &= FIELD_MENU_GROUP_FIELD_MASK;
    capacity = flags & FIELD_MENU_GROUP_FIELD_MASK;

    for (i = 0; i < 8; i++)
    {
        if (g_saved_game.layout.menu_slots[group].slots[i].entry.index < FIELD_MENU_ENTRY_EMPTY)
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
        if (D_80122C00.locals.bytes[i] != 0)
        {
            break;
        }
    }

    item = FIELD_ACTION_ITEM_BASE + i;
    field_set_text_macro(4, FIELD_MENU_TEXT(item), FIELD_TEXT_MACRO_UNLIMITED);
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
    flags = g_saved_game.layout.menu_slots[group].flags.word;
    item_slot_count = flags >> FIELD_MENU_GROUP_ITEM_SLOTS_SHIFT;
    item_slot_count &= FIELD_MENU_GROUP_FIELD_MASK;
    capacity = flags & FIELD_MENU_GROUP_FIELD_MASK;

    for (slot = 0; slot < 8; slot++)
    {
        if (g_saved_game.layout.menu_slots[group].slots[slot].entry.index != FIELD_MENU_ENTRY_EMPTY)
        {
            count++;
        }
    }

    capacity -= count;

    for (slot = 0; slot < item_slot_count; slot++)
    {
        if (g_saved_game.layout.menu_slots[group].items[slot] == FIELD_MENU_ENTRY_EMPTY)
        {
            break;
        }
    }

    used = slot + 1;
    FIELD_LOCAL_BYTE(FIELD_ACTION_USED_SLOTS) = used;
    g_saved_game.layout.menu_slots[group].flags.word = (g_saved_game.layout.menu_slots[group].flags.word & ~FIELD_MENU_GROUP_USED_MASK) |
                                                       ((used & FIELD_MENU_GROUP_FIELD_MASK) << FIELD_MENU_GROUP_USED_SHIFT);

    if (capacity < slot + 3)
    {
        FIELD_LOCAL_BYTE(FIELD_ACTION_GROUP_FULL) = 1;
    }

    selected = FIELD_LOCAL_HALF(FIELD_ACTION_SELECTED_ITEM);
    FIELD_LOCAL_BYTE(FIELD_ACTION_ITEM_COUNTS + selected)--;
    item = selected + FIELD_ACTION_ITEM_BASE;
    g_saved_game.layout.menu_slots[group].items[slot] = item;
    field_set_text_macro(0, FIELD_MENU_TEXT(item), FIELD_TEXT_MACRO_UNLIMITED);

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
    flags = g_saved_game.layout.menu_slots[group].flags.word;
    free_slots = flags & FIELD_MENU_GROUP_FIELD_MASK;
    item_slot_count = flags >> FIELD_MENU_GROUP_ITEM_SLOTS_SHIFT;
    item_slot_count &= FIELD_MENU_GROUP_FIELD_MASK;
    fill = flags >> FIELD_MENU_GROUP_USED_SHIFT;
    fill &= FIELD_MENU_GROUP_FIELD_MASK;
    fill += 2;

    for (; i < 8; i++)
    {
        if (g_saved_game.layout.menu_slots[group].slots[i].entry.index != FIELD_MENU_ENTRY_EMPTY)
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

            if (g_saved_game.layout.menu_slots[group].slots[pick].entry.index == FIELD_MENU_ENTRY_EMPTY)
            {
                break;
            }
        }

        if (g_saved_game.layout.menu_slots[group].slots[pick].entry.index == FIELD_MENU_ENTRY_EMPTY)
        {
            field_roll_menu_slot_effect(group, pick);
        }
        else
        {
            for (j = 0; j < 8; j++)
            {
                if (g_saved_game.layout.menu_slots[group].slots[j].entry.index == FIELD_MENU_ENTRY_EMPTY)
                {
                    field_roll_menu_slot_effect(group, j);
                    break;
                }
            }
        }
    }

    for (i = 0; i < item_slot_count; i++)
    {
        item = g_saved_game.layout.menu_slots[group].items[i];

        if (item < FIELD_MENU_ENTRY_EMPTY)
        {
            g_saved_game.layout.item_counts[item]--;
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
    g_saved_game.layout.menu_slots[group].flags.word &= ~FIELD_MENU_GROUP_USED_MASK;

    for (i = 0; i < 4; i++)
    {
        g_saved_game.layout.menu_slots[group].items[i] = FIELD_MENU_ENTRY_EMPTY;
    }
}

/** @brief Free the eight action slots of group 0 and clear its used item-slot count. */
void field_menu_reset_group0_slots(void)
{
    SavedGameLayout* data;
    u32 flags;

    data = &g_saved_game.layout;
    flags = data->menu_slots[0].flags.word;
    data->menu_slots[0].slots[0].handle = 0;
    data->menu_slots[0].slots[0].entry.index = FIELD_MENU_ENTRY_EMPTY;
    data->menu_slots[0].slots[1].handle = 0;
    data->menu_slots[0].slots[1].entry.index = FIELD_MENU_ENTRY_EMPTY;
    data->menu_slots[0].slots[2].handle = 0;
    data->menu_slots[0].slots[2].entry.index = FIELD_MENU_ENTRY_EMPTY;
    data->menu_slots[0].slots[3].handle = 0;
    data->menu_slots[0].slots[3].entry.index = FIELD_MENU_ENTRY_EMPTY;
    data->menu_slots[0].slots[4].handle = 0;
    data->menu_slots[0].slots[4].entry.index = FIELD_MENU_ENTRY_EMPTY;
    data->menu_slots[0].slots[5].handle = 0;
    data->menu_slots[0].slots[5].entry.index = FIELD_MENU_ENTRY_EMPTY;
    data->menu_slots[0].slots[6].handle = 0;
    data->menu_slots[0].slots[6].entry.index = FIELD_MENU_ENTRY_EMPTY;
    data->menu_slots[0].slots[7].handle = 0;
    data->menu_slots[0].slots[7].entry.index = FIELD_MENU_ENTRY_EMPTY;
    data->menu_slots[0].flags.word = flags & ~FIELD_MENU_GROUP_USED_MASK;
}

/** @brief Publish the text of the chosen action item (D_80122C14) to macro 4. */
void field_menu_publish_chosen_action_item(void)
{
    s32 item;

    item = D_80122C14 + FIELD_ACTION_ITEM_BASE;
    field_set_text_macro(4, FIELD_MENU_TEXT(item), FIELD_TEXT_MACRO_UNLIMITED);
}

/**
 * @brief Place actor D_80122C0D next to actor 0, on the side it faces.
 *
 * Takes actor 0's position (8.8 fixed point) and heading, steps 20 along x or
 * 12 along z for the four main headings and 10 along both axes in between, and
 * moves the actor there, 12 higher.
 */
void field_place_actor_beside_actor0(void)
{
    Vec3i pos;
    s32 heading;
    s32 actor;

    actor = D_80122C0D;
    field_get_actor_position(0, &pos);
    pos.x /= 256;
    pos.y /= 256;
    pos.z /= 256;

    heading = field_get_actor_facing(0);

    if (heading == 0)
    {
        pos.x += 20;
    }
    else if (heading > 0 && heading < FIELD_HEADING_QUARTER_TURN)
    {
        pos.x += 10;
        pos.z -= 10;
    }
    else if (heading == FIELD_HEADING_QUARTER_TURN)
    {
        pos.z -= 12;
    }
    else if (heading > FIELD_HEADING_QUARTER_TURN && heading < FIELD_HEADING_HALF_TURN)
    {
        pos.x -= 10;
        pos.z -= 10;
    }
    else if (heading == FIELD_HEADING_HALF_TURN)
    {
        pos.x -= 20;
    }
    else if (heading > FIELD_HEADING_HALF_TURN && heading < FIELD_HEADING_THREE_QUARTER_TURN)
    {
        pos.x -= 10;
        pos.z += 10;
    }
    else if (heading == FIELD_HEADING_THREE_QUARTER_TURN)
    {
        pos.z += 12;
    }
    else if (heading > FIELD_HEADING_THREE_QUARTER_TURN && heading < FIELD_HEADING_STEPS)
    {
        pos.x += 10;
        pos.z += 10;
    }

    field_set_actor_position(actor, pos.x, pos.y - 12, pos.z);
}

/**
 * @brief Free the action slot selected by D_80122C0D and give its item back.
 * @note The item count is clamped to 0-ITEM_COUNT_MAX.
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
    item = g_saved_game.layout.menu_slots[group].slots[slot].entry.index;
    count = g_saved_game.layout.item_counts[item];
    count++;

    if (count >= 0)
    {
        clamped = ITEM_COUNT_MAX;

        if (count <= ITEM_COUNT_MAX)
        {
            clamped = count;
        }
    }
    else
    {
        clamped = 0;
    }

    g_saved_game.layout.item_counts[item] = clamped;
    g_saved_game.layout.menu_slots[group].slots[slot].handle = 0;
    g_saved_game.layout.menu_slots[group].slots[slot].entry.index = FIELD_MENU_ENTRY_EMPTY;
    g_saved_game.layout.menu_slots[group].slots[slot].counters[0] = 0;
    g_saved_game.layout.menu_slots[group].slots[slot].counters[1] = 0;
    g_saved_game.layout.menu_slots[group].slots[slot].counters[2] = 0;
    g_saved_game.layout.menu_slots[group].slots[slot].counters[3] = 0;
    g_saved_game.layout.menu_slots[group].slots[slot].counters[4] = 0;
    g_saved_game.layout.menu_slots[group].slots[slot].counters[5] = 0;
    g_saved_game.layout.menu_slots[group].slots[slot].counters[6] = 0;
    g_saved_game.layout.menu_slots[group].slots[slot].counters[7] = 0;
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
    handle = g_saved_game.layout.menu_slots[group].slots[slot].handle;
    FIELD_MENU_ACTION_SLOT->handle = handle;
    item = g_saved_game.layout.menu_slots[group].slots[slot].entry.index;
    field_set_text_macro(0, FIELD_MENU_TEXT(item), FIELD_TEXT_MACRO_UNLIMITED);
    FIELD_MENU_ACTION_SLOT->item_index = item;
}

/**
 * @brief Copy the gosub-selected inventory record into the first free pending record.
 *
 * Sets D_80122C1F to 0 when the gosub returned nothing, 2 when the record
 * holds no item, 3 when a pending record already holds the same item and 1
 * when the record was copied (its value is then at least 1).
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

    if (g_saved_game.layout.items[selected].key.first == 0 && g_saved_game.layout.items[selected].key.second == 0)
    {
        D_80122C1F = 2;
        return;
    }

    for (i = 0; i < 4; i++)
    {
        if (g_saved_game.layout.pending_items[i].name[0] != 0 &&
            g_saved_game.layout.items[selected].key.first == g_saved_game.layout.pending_items[i].key.first &&
            g_saved_game.layout.items[selected].key.second == g_saved_game.layout.pending_items[i].key.second)
        {
            D_80122C1F = 3;
            return;
        }
    }

    for (i = 0; i < 4; i++)
    {
        if (g_saved_game.layout.pending_items[i].name[0] == 0)
        {
            field_copy_inventory_record(&g_saved_game.layout.pending_items[i], &g_saved_game.layout.items[selected]);
            field_discard_item(selected);
            value = g_saved_game.layout.pending_items[i].value;

            if (value == 0)
            {
                value = 1;
            }

            g_saved_game.layout.pending_items[i].value = value;
            D_80122C1F = 1;
            return;
        }
    }
}

/**
 * @brief Open the GOSUB equipment screen, then its confirmation dialog.
 * @note Identical to field_menu_choose_equipment; this op sits with the pending-record ops.
 */
void field_menu_choose_pending_equipment(void)
{
    FieldGosubSequence sequence;

    sequence = D_80051EB4;
    field_open_gosub_screen_sequence(&sequence);
}

/**
 * @brief Place actor 0xC next to actor 0, on the side it faces.
 *
 * Like field_place_actor_beside_actor0, with steps of 10 along one axis for the four main
 * headings and 8 along both axes in between, at the same height.
 */
void field_place_actor_0c_beside_actor0(void)
{
    Vec3i pos;
    s32 heading;

    field_get_actor_position(0, &pos);
    pos.x /= 256;
    pos.y /= 256;
    pos.z /= 256;

    heading = field_get_actor_facing(0);

    if (heading == 0)
    {
        pos.x += 10;
    }
    else if (heading > 0 && heading < FIELD_HEADING_QUARTER_TURN)
    {
        pos.x += 8;
        pos.z -= 8;
    }
    else if (heading == FIELD_HEADING_QUARTER_TURN)
    {
        pos.z -= 10;
    }
    else if (heading > FIELD_HEADING_QUARTER_TURN && heading < FIELD_HEADING_HALF_TURN)
    {
        pos.x -= 8;
        pos.z -= 8;
    }
    else if (heading == FIELD_HEADING_HALF_TURN)
    {
        pos.x -= 10;
    }
    else if (heading > FIELD_HEADING_HALF_TURN && heading < FIELD_HEADING_THREE_QUARTER_TURN)
    {
        pos.x -= 8;
        pos.z += 8;
    }
    else if (heading == FIELD_HEADING_THREE_QUARTER_TURN)
    {
        pos.z += 10;
    }
    else if (heading > FIELD_HEADING_THREE_QUARTER_TURN && heading < FIELD_HEADING_STEPS)
    {
        pos.x += 8;
        pos.z += 8;
    }

    field_set_actor_position(0xC, pos.x, pos.y, pos.z);
}

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
            if (g_saved_game.layout.menu_slots[group].slots[slot].entry.index != FIELD_MENU_ENTRY_EMPTY)
            {
                counter = g_saved_game.layout.menu_slots[group].slots[slot].counters[0];

                if (counter == 0)
                {
                    g_saved_game.layout.menu_slots[group].slots[slot].counters[0] = counter + 1;
                }
                else if (counter < 0xF0)
                {
                    FIELD_STEP_ACTION_COUNTERS(g_saved_game.layout.menu_slots[group].slots[slot]);
                    FIELD_STEP_ACTION_COUNTERS(g_saved_game.layout.menu_slots[group].slots[slot]);
                    FIELD_STEP_ACTION_COUNTERS(g_saved_game.layout.menu_slots[group].slots[slot]);
                    FIELD_STEP_ACTION_COUNTERS(g_saved_game.layout.menu_slots[group].slots[slot]);
                    FIELD_STEP_ACTION_COUNTERS(g_saved_game.layout.menu_slots[group].slots[slot]);
                    FIELD_STEP_ACTION_COUNTERS(g_saved_game.layout.menu_slots[group].slots[slot]);
                    FIELD_STEP_ACTION_COUNTERS(g_saved_game.layout.menu_slots[group].slots[slot]);
                    FIELD_STEP_ACTION_COUNTERS(g_saved_game.layout.menu_slots[group].slots[slot]);
                    FIELD_STEP_ACTION_COUNTERS(g_saved_game.layout.menu_slots[group].slots[slot]);
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
    g_saved_game.layout.menu_slots[group].slots[slot].entry.index = FIELD_MENU_ENTRY_EMPTY;
    g_saved_game.layout.menu_slots[group].slots[slot].handle = 0;
    g_saved_game.layout.menu_slots[group].slots[slot].counters[0] = 0;
    g_saved_game.layout.menu_slots[group].slots[slot].counters[1] = 0;
    g_saved_game.layout.menu_slots[group].slots[slot].counters[2] = 0;
    g_saved_game.layout.menu_slots[group].slots[slot].counters[3] = 0;
    g_saved_game.layout.menu_slots[group].slots[slot].counters[4] = 0;
    g_saved_game.layout.menu_slots[group].slots[slot].counters[5] = 0;
    g_saved_game.layout.menu_slots[group].slots[slot].counters[6] = 0;
    g_saved_game.layout.menu_slots[group].slots[slot].counters[7] = 0;
    /* Bytes 5-7 of the slot are set to 0xFF as part of their word. */
    g_saved_game.layout.menu_slots[group].slots[slot].entry.word |= ~0xFF;
}

/** @brief Reset all lands and award the default set of land artifacts in order. */
void field_reset_default_lands(void)
{
    field_reset_lands();
    field_award_land_artifact(0);
    field_award_land_artifact(1);
    field_award_land_artifact(2);
    field_award_land_artifact(3);
    field_award_land_artifact(4);
    field_award_land_artifact(5);
    field_award_land_artifact(7);
    field_award_land_artifact(8);
    field_award_land_artifact(9);
    field_award_land_artifact(0xA);
    field_award_land_artifact(0xB);
    field_award_land_artifact(0xC);
    field_award_land_artifact(0xD);
    field_award_land_artifact(0xF);
    field_award_land_artifact(0x10);
    field_award_land_artifact(0x11);
    field_award_land_artifact(0x12);
    field_award_land_artifact(0x13);
    field_award_land_artifact(0x15);
    field_award_land_artifact(0x16);
    field_award_land_artifact(0x18);
    field_award_land_artifact(0x19);
    field_award_land_artifact(0x1A);
    field_award_land_artifact(0x1B);
    field_award_land_artifact(0x1E);
    field_award_land_artifact(0x1F);
    field_award_land_artifact(0x20);
    field_award_land_artifact(0x20);
    field_award_land_artifact(0x17);
}

/** @brief Restore the saved field mode through the mode dispatcher. */
void field_menu_restore_field_mode(void)
{
    g_field_niki_state = D_80122C1E;
    field_open_niki(D_80122C1E);
}

/**
 * @brief List the grazing pets.
 * @note Stores each listed pet's species from D_80122C00 and its index from
 *       D_80122C1D, and publishes its name to macro 0, 1, ... in order.
 */
void field_menu_list_grazing_pets(void)
{
    s32 count;
    s32 i;

    count = 0;

    for (i = 0; i < PET_RECORD_COUNT; i++)
    {
        if (g_saved_game.layout.pets[i].name[0] != 0 && g_saved_game.layout.pets[i].status.bits.grazing == 1)
        {
            FIELD_LOCAL_BYTE(count) = g_saved_game.layout.pets[i].species;
            FIELD_LOCAL_BYTE(FIELD_CURSOR_SLOT_TABLE + count) = i;
            field_set_text_macro(count, g_saved_game.layout.pets[i].name, FIELD_TEXT_MACRO_UNLIMITED);
            count++;
        }
    }
}

/**
 * @brief Check the selected shared item record and publish its description.
 *
 * An empty record sets status 1. A record whose key matches an
 * inventory, equipment or pending record sets status 2. Otherwise the name,
 * name text, kind, category, stat value and item value go to the script
 * variables, with the value's digit count at D_80122C0C.
 * @note JP returns a byte stat value and a separate special-item byte.
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

    if (g_field_shared_items[selected].name[0] == 0)
    {
        selection->status = 1;
        return;
    }

    duplicate_found = 0;

    for (i = 0; i < FIELD_ITEM_COUNT; i++)
    {
        if (g_saved_game.layout.items[i].name[0] != 0 && g_saved_game.layout.items[i].key.first == g_field_shared_items[selected].key.first &&
            g_saved_game.layout.items[i].key.second == g_field_shared_items[selected].key.second)
        {
            duplicate_found = 1;
            break;
        }
    }

    for (i = 0; i < 8; i++)
    {
        if (g_saved_game.hero_items.records[i].name[0] != 0 && g_saved_game.hero_items.records[i].key.first == g_field_shared_items[selected].key.first &&
            g_saved_game.hero_items.records[i].key.second == g_field_shared_items[selected].key.second)
        {
            duplicate_found = 1;
            break;
        }
    }

    for (i = 0; i < 4; i++)
    {
        if (g_saved_game.layout.pending_items[i].name[0] != 0 && g_saved_game.layout.pending_items[i].key.first == g_field_shared_items[selected].key.first &&
            g_saved_game.layout.pending_items[i].key.second == g_field_shared_items[selected].key.second)
        {
            duplicate_found = 1;
            break;
        }
    }

    if (duplicate_found == 0)
    {
        attributes = g_field_shared_items[selected].info.word;
        kind = (attributes >> 8) & 3;
        category = FIELD_ITEM_TYPE(attributes);

        if (kind == 1)
        {
            category += 0xB;
        }
        else if (kind == 2)
        {
            category += 0x17;
        }

        name_index = g_field_shared_items[selected].info.halves[1] & FIELD_ITEM_MATERIAL_MASK;

        if (kind == 0)
        {
            value = g_field_shared_items[selected].derived.values[0];
        }
        else if (kind == 1)
        {
            value = g_field_shared_items[selected].derived.values[0];
            value += g_field_shared_items[selected].derived.values[1];
            value += g_field_shared_items[selected].derived.values[2];
            value += g_field_shared_items[selected].derived.values[3];
        }
        else
        {
            value = g_field_shared_items[selected].derived.bytes[2];
        }

        low_word = g_field_shared_items[selected].bonus_nibbles.word;
        high_word = g_field_shared_items[selected].stat_nibbles.word;
        nibble_sum = (low_word & 0xF) + ((low_word >> 4) & 0xF) + ((low_word >> 8) & 0xF) + ((low_word >> 12) & 0xF) + ((low_word >> 16) & 0xF) +
                     ((low_word >> 20) & 0xF) + ((low_word >> 24) & 0xF) + (low_word >> 28) + (high_word & 0xF) + ((high_word >> 4) & 0xF) +
                     ((high_word >> 8) & 0xF) + ((high_word >> 12) & 0xF) + ((high_word >> 16) & 0xF) + ((high_word >> 20) & 0xF) + ((high_word >> 24) & 0xF) +
                     (high_word >> 28);
        special = nibble_sum >= 0x29;

        if (kind == 2)
        {
            special = g_field_shared_items[selected].derived.bytes[0];
        }

        amount = g_field_shared_items[selected].value;
        field_set_text_macro(0, g_field_shared_items[selected].name, FIELD_TEXT_MACRO_UNLIMITED);
        info = FIELD_MENU_ITEM_INFO;
        info->kind = kind;
        info->category = category;
        field_set_text_macro(1, FIELD_MENU_TEXT(name_index), FIELD_TEXT_MACRO_UNLIMITED);
        info->value = value;
#if defined(VERSION_JP)
        info->special = special;
#else
        if (special != 0)
        {
            info->value = value - 0x8000;
        }
#endif
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
    field_set_text_macro(0, g_field_shared_items[selected].name, FIELD_TEXT_MACRO_UNLIMITED);
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
        if (g_saved_game.layout.pending_items[i].name[0] != 0 && g_saved_game.layout.pending_items[i].value == 0)
        {
            g_saved_game.layout.pending_items[i].name[0] = 0;
        }
    }

    for (i = 0; i < 4; i++)
    {
        if (g_saved_game.layout.pending_items[i].name[0] == 0 && g_saved_game.layout.pending_items[i].value == 0)
        {
            for (j = i + 1; j < 4; j++)
            {
                if (g_saved_game.layout.pending_items[j].name[0] != 0)
                {
                    field_copy_inventory_record(&g_saved_game.layout.pending_items[i], &g_saved_game.layout.pending_items[j]);
                    g_saved_game.layout.pending_items[j].name[0] = 0;
                    g_saved_game.layout.pending_items[j].value = 0;
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
    FieldItemRecord* free_record;

    selected = D_80122C02;
    free_record = field_find_free_inventory_record();
    field_copy_inventory_record(free_record, &g_saved_game.layout.pending_items[selected]);
    g_saved_game.layout.pending_items[selected].name[0] = 0;
    g_saved_game.layout.pending_items[selected].value = 0;
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
        if (g_saved_game.layout.pending_items[i].name[0] == 0 && g_saved_game.layout.pending_items[i].value != 0)
        {
            g_saved_game.layout.pending_items[i].name[0] = FIELD_ITEM_SET_ASIDE_NAME(g_saved_game.layout.pending_items[i]);
            field_set_text_macro(0, g_saved_game.layout.pending_items[i].name, FIELD_TEXT_MACRO_UNLIMITED);
            result = g_saved_game.layout.pending_items[i].value;
            g_saved_game.layout.pending_items[i].value = 0;
            D_80122C02 = i;
            break;
        }
    }

    D_80122C00.locals.words[2] = result;

    if (result == 0)
    {
        field_menu_compact_pending_items();
    }
}

/**
 * @brief Count the pending item records and publish the selected one's description.
 *
 * The number of active pending records goes to D_80122C06. An empty selected
 * record sets status 1, one whose key matches an inventory or
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
        if (g_saved_game.layout.pending_items[i].name[0] != 0)
        {
            count++;
        }
    }

    FIELD_LOCAL_BYTE(FIELD_PENDING_COUNT) = count;
    FIELD_LOCAL_BYTE(FIELD_RECORD_STATUS) = 0;

    if (g_saved_game.layout.pending_items[selected].name[0] == 0)
    {
        FIELD_LOCAL_BYTE(FIELD_RECORD_STATUS) = 1;
        return;
    }

    duplicate_found = 0;

    for (i = 0; i < FIELD_ITEM_COUNT; i++)
    {
        if (g_saved_game.layout.items[i].name[0] != 0 && g_saved_game.layout.items[i].key.first == g_saved_game.layout.pending_items[selected].key.first &&
            g_saved_game.layout.items[i].key.second == g_saved_game.layout.pending_items[selected].key.second)
        {
            duplicate_found = 1;
            break;
        }
    }

    for (i = 0; i < 8; i++)
    {
        if (g_saved_game.hero_items.records[i].name[0] != 0 &&
            g_saved_game.hero_items.records[i].key.first == g_saved_game.layout.pending_items[selected].key.first &&
            g_saved_game.hero_items.records[i].key.second == g_saved_game.layout.pending_items[selected].key.second)
        {
            duplicate_found = 1;
            break;
        }
    }

    if (duplicate_found == 0)
    {
        attributes = g_saved_game.layout.pending_items[selected].info.word;
        kind = (attributes >> 8) & 3;
        category = FIELD_ITEM_TYPE(attributes);

        if (kind == 1)
        {
            category += 0xB;
        }
        else if (kind == 2)
        {
            category += 0x17;
        }

        name_index = g_saved_game.layout.pending_items[selected].info.halves[1] & FIELD_ITEM_MATERIAL_MASK;
        field_set_text_macro(0, g_saved_game.layout.pending_items[selected].name, FIELD_TEXT_MACRO_UNLIMITED);
        info = FIELD_MENU_ITEM_INFO;
        info->kind = kind;
        info->category = category;
        field_set_text_macro(1, FIELD_MENU_TEXT(name_index), FIELD_TEXT_MACRO_UNLIMITED);
        return;
    }

    field_set_text_macro(0, g_saved_game.layout.pending_items[selected].name, FIELD_TEXT_MACRO_UNLIMITED);
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
 * @brief Bring the grazing pet picked by the cursor slot back from grazing.
 *
 * The record index comes from the cursor slot table at D_80122C19; an index
 * of five or more is reported as a diagnostic.
 */
void field_menu_recall_cursor_pet(void)
{
    s32 index;

    index = FIELD_LOCAL_BYTE(FIELD_CURSOR_SLOT_TABLE + FIELD_LOCAL_BYTE(FIELD_RECALL_SLOT));

    if (index < PET_RECORD_COUNT)
    {
        g_saved_game.layout.pets[index].status.bits.grazing = 0;
    }
    else
    {
        record_game_diagnostic(FIELD_DIAG_MENU_OP, FIELD_MENU_OP_RECALL_CURSOR_PET, index, 0);
    }
}

/**
 * @brief Hatch an egg: store the hatched species in the selected pet and clear its egg flag.
 *
 * An unnamed pet gets the name 'A' (0x41). An index of five or more is
 * reported as a diagnostic.
 */
void field_menu_hatch_pet(void)
{
    s32 index;
    u8 species;

    species = FIELD_LOCAL_BYTE(FIELD_HATCH_SPECIES);
    index = FIELD_LOCAL_BYTE(FIELD_HATCH_PET_INDEX);

    if (index < PET_RECORD_COUNT)
    {
        g_saved_game.layout.pets[index].species = species;
        g_saved_game.layout.pets[index].status.bits.egg = 0;

        if (g_saved_game.layout.pets[index].name[0] == 0)
        {
            g_saved_game.layout.pets[index].name[0] = 0x41;
        }
    }
    else
    {
        record_game_diagnostic(FIELD_DIAG_MENU_OP, FIELD_MENU_OP_HATCH_PET, index, 0);
    }
}

/**
 * @brief Open ADDHERO for the guest state set by field_menu_publish_guest_presence.
 * @note A guest already present opens mode 1, no guest opens mode 0 (load a hero).
 */
void field_menu_open_addhero(void)
{
    if (field_get_script_var(0, FIELD_VARIABLE_GUEST_VARIANT) == FIELD_GUEST_PRESENT)
    {
        field_open_addhero(1);
    }
    else if (field_get_script_var(0, FIELD_VARIABLE_GUEST_VARIANT) == FIELD_NO_VARIANT)
    {
        field_open_addhero(0);
    }
}

/**
 * @brief Store the guest hero's first name byte in local 0 and publish the name.
 * @note The name goes to text macro 0 only when a guest is present.
 */
void field_menu_publish_guest_name(void)
{
    D_80122C00.locals.bytes[0] = g_saved_game.layout.characters[FIELD_PARTY_GUEST].name[0];

    if (D_80122C00.locals.bytes[0] != 0)
    {
        field_set_text_macro(0, g_saved_game.layout.characters[FIELD_PARTY_GUEST].name, FIELD_TEXT_MACRO_UNLIMITED);
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
        if (g_saved_game.layout.pending_items[i].name[0] == 0)
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

    for (i = 0; i < FIELD_ITEM_COUNT; i++)
    {
        if (g_saved_game.layout.items[i].name[0] == 0)
        {
            count++;
        }
    }

    D_80122C1F = count;
}

/** @brief Clear the four shared item records and their values. */
void field_menu_clear_shared_items(void)
{
    g_field_shared_items[0].name[0] = 0;
    g_field_shared_items[1].name[0] = 0;
    g_field_shared_items[2].name[0] = 0;
    g_field_shared_items[3].name[0] = 0;
    g_field_shared_items[0].value = 0;
    g_field_shared_items[1].value = 0;
    g_field_shared_items[2].value = 0;
    g_field_shared_items[3].value = 0;
}

/**
 * @brief Update the four shared item records for the mode NIKI finished in.
 *
 * D_80122C1E holds the mode the script requested and g_field_niki_state the mode NIKI
 * left. Going from 1 to 0 moves every set-aside record with a value back into
 * the inventory; going from 0 to 1 gives every active record a value of at
 * least 1. D_80122C1E then takes NIKI's mode.
 */
void field_menu_sync_shared_items(void)
{
    s32 mode;
    s32 i;
    u8* entry;
    s32 result;

    mode = D_80122C1E;

    if (mode == 1 && g_field_niki_state == 0)
    {
        for (i = 0; i < 4; i++)
        {
            if (g_field_shared_items[i].name[0] == 0)
            {
                entry = &FIELD_LOCAL_BYTE(FIELD_SET_ASIDE_SPECIAL_IDS + i);

                if (g_field_shared_items[i].value != 0 && *entry != 0xFA)
                {
                    g_field_shared_items[i].name[0] = FIELD_ITEM_SET_ASIDE_NAME(g_field_shared_items[i]);
                    FIELD_ITEM_SET_ASIDE_NAME(g_field_shared_items[i]) = *entry;

                    if (field_find_free_inventory_record() != 0)
                    {
                        field_copy_inventory_record(field_find_free_inventory_record(), &g_field_shared_items[i]);
                    }
                }
            }
        }
    }

    if (mode == 0 && g_field_niki_state == 1)
    {
        for (i = 0; i < 4; i++)
        {
            if (g_field_shared_items[i].name[0] != 0)
            {
                result = g_field_shared_items[i].value;

                if (result == 0)
                {
                    result = 1;
                }

                g_field_shared_items[i].value = result;
            }
        }
    }

    D_80122C1E = g_field_niki_state;
}

/** @brief Set FIELD_VARIABLE_GUEST_VARIANT to FIELD_GUEST_PRESENT when a guest hero is loaded, else FIELD_NO_VARIANT. */
void field_menu_publish_guest_presence(void)
{
    if (g_saved_game.layout.characters[FIELD_PARTY_GUEST].name[0] != 0)
    {
        field_set_script_var(0, FIELD_VARIABLE_GUEST_VARIANT, FIELD_GUEST_PRESENT);
    }
    else
    {
        field_set_script_var(0, FIELD_VARIABLE_GUEST_VARIANT, FIELD_NO_VARIANT);
    }
}

/**
 * @brief Set the selected shared item record aside and give it the pending result.
 *
 * Its special_ids[0] goes to the FIELD_SET_ASIDE_SPECIAL_IDS locals, the
 * name byte is kept in FIELD_ITEM_SET_ASIDE_NAME, and the word at D_80122C08 becomes
 * the record's value.
 */
void field_menu_set_aside_shared_item(void)
{
    s32 index;
    FieldItemRecord* record;
    u8 saved;
    u8 active;

    index = FIELD_LOCAL_BYTE(FIELD_RECORD_INDEX);
    saved = FIELD_ITEM_SET_ASIDE_NAME(g_field_shared_items[index]);
    record = &g_field_shared_items[index];
    FIELD_LOCAL_BYTE(FIELD_SET_ASIDE_SPECIAL_IDS + index) = saved;
    active = record->name[0];
    record->name[0] = 0;
    FIELD_ITEM_SET_ASIDE_NAME(*record) = active;
    record->value = FIELD_LOCAL_WORD(FIELD_RECORD_RESULT);
}

/**
 * @brief Pick one of the eight elements, weighted by the current land's element levels.
 *
 * Each element weighs (level - 3)^4 with the level clamped to 0..3 after the
 * offset, one level more for the favored element. Mode 0 (local 0) draws by
 * weight and stores the element in local 5; any other mode draws uniformly
 * and stores it in local 6. 0xFF means no element was drawn.
 */
void field_menu_draw_land_element(void)
{
    s32 weights[FIELD_LAND_SPIRIT_COUNT];
    FieldFavoredElementTable favored;
    s32 total, i, land_offset, level, weight, draw;
    FieldViewOrElement work;
    s32* cursor;
    s32* base;
    s32* weight_slot;
    s32 land, product;
    s32 mode;

    total = 0;
    i = total;
    work.view = &g_saved_game.layout;
    mode = D_80122C00.locals.bytes[0];
    land = g_music_track_index;
    favored = g_field_weekday_favored_elements;
    land_offset = land * FIELD_LAND_RECORD_SIZE;
next_weight:
    level = ((u8*)work.view)[i + land_offset + FIELD_LAND_LEVELS_OFFSET];
    weight = level - 3;

    if (i == favored.elements[work.view->control.fields.weekday & FIELD_WEEKDAY_MASK])
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
    weight_slot = &weights[i];
    i++;
    base = weights;
    *weight_slot = weight;
    total += weight;

    if (i < FIELD_LAND_SPIRIT_COUNT)
    {
        goto next_weight;
    }

    if (mode == 1)
    {
        total = FIELD_ELEMENT_UNIFORM_TOTAL;
    }

    product = rand() * total;
    draw = product / FIELD_ELEMENT_RANDOM_RANGE;

    total = 0;
    work.element = FIELD_NO_ELEMENT;

    for (i = 0, cursor = base; i < FIELD_LAND_SPIRIT_COUNT; i++, cursor++)
    {
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

/** @brief Open the GOSUB instrument screen, then its confirmation dialog. */
void field_menu_choose_instrument(void)
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
    SavedGameLayout* items;
    FieldItemRecord* inventory;
    FieldItemRecord* record;

    index = 0xFF;
    category = 0xFF;
    stat_byte0 = 0xFF;
    stat_byte2 = 0;

    if (g_gosub_result_count != 0)
    {
        items = &g_saved_game.layout;
        index = g_gosub_result_values[0];
        category = FIELD_ITEM_TYPE(items->items[index].info.word);
        inventory = items->items;
        record = &inventory[index];
        stat_byte0 = record->derived.bytes[0];
        stat_byte2 = record->derived.bytes[2];
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
    FieldItemRecord* free_record;
    s32 offset;

    index = D_80122C00.locals.bytes[0];

    if (field_find_free_inventory_record() != 0)
    {
        free_record = field_find_free_inventory_record();
        field_copy_inventory_record(free_record,
                                    (FieldItemRecord*)(field_find_resource(FIELD_ITEM_RECORD_RESOURCE) + (offset = FIELD_RESOURCE_RECORD_OFFSET(index))));
        field_set_text_macro(0, field_find_resource(FIELD_ITEM_RECORD_RESOURCE) + offset, FIELD_TEXT_MACRO_UNLIMITED);
    }
}

/**
 * @brief Swap the pet in the party with the grazing pet in the cursor slot.
 *
 * Returns the pet the cursor slot (local 0x1C) held as the gosub
 * result, puts the joined pet's index in that slot, stores its species
 * in local halfword 0xA, leaves it grazing and
 * publishes its name to the text macro named by local 0x1C.
 */
void field_menu_swap_joined_pet(void)
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
    index = g_saved_game.layout.joined_pet;
    g_gosub_result_values[0] = previous;

    if (index < PET_RECORD_COUNT)
    {
        *entry = index;
        FIELD_LOCAL_HALF(FIELD_CURSOR_SPECIES) = g_saved_game.layout.pets[index].species;
        g_saved_game.layout.pets[index].status.bits.grazing = 1;
        field_set_text_macro(slot, g_saved_game.layout.pets[index].name, FIELD_TEXT_MACRO_UNLIMITED);
    }
}

/**
 * @brief Publish the name of the pet in the cursor slot to text macro 3.
 * @note Local 0x1C is replaced by the record's unk48 flag byte.
 */
void field_menu_publish_cursor_pet(void)
{
    s32 index;

    index = FIELD_LOCAL_BYTE(FIELD_CURSOR_SLOT_TABLE + FIELD_LOCAL_BYTE(FIELD_CURSOR_SLOT));
    FIELD_LOCAL_BYTE(FIELD_CURSOR_SLOT) = g_saved_game.layout.pets[index].unk48.flags;
    field_set_text_macro(3, g_saved_game.layout.pets[index].name, FIELD_TEXT_MACRO_UNLIMITED);
}

/** @brief Count the pets (occupied pet records) into D_80122C16. */
void field_menu_count_pets(void)
{
    s32 i;
    s32 count;

    count = 0;

    for (i = 0; i < PET_RECORD_COUNT; i++)
    {
        if (g_saved_game.layout.pets[i].name[0] != 0)
        {
            count++;
        }
    }

    D_80122C16 = count;
}

/**
 * @brief Clear the GOSUB request and open CARDA in the mode held in D_80122C01.
 */
void field_menu_open_carda(void)
{
    field_menu_clear_gosub_request();
    field_open_carda(D_80122C01);
}

/**
 * @brief Store the golem that last left the party (high nibble of golem_count) into D_80122C12.
 */
void field_menu_load_golem_saved_group(void)
{
    D_80122C12 = g_saved_game.layout.golem_count >> 4;
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
    FieldItemRecord* records;
    FieldItemRecord* record;
    s32 index;
    s32 row;
    s32 column;
    s32 entry;
    s32 offset;

    index = D_80122C10;
    records = g_saved_game.layout.items;
    record = records + index;
    row = record->derived.bytes[0];
    column = record->derived.bytes[1];
    entry = row * 14 + column;
    field_set_text_macro(0, record->name, FIELD_TEXT_MACRO_UNLIMITED);
    offset = FIELD_TEXT_OFFSET(FIELD_ITEM_ENTRY_TEXT_RESOURCE, entry);
    field_set_text_macro(1, FIELD_TEXT_RESOURCE(FIELD_ITEM_ENTRY_TEXT_RESOURCE)->texts + offset, FIELD_TEXT_MACRO_UNLIMITED);
    offset = FIELD_TEXT_OFFSET(FIELD_ITEM_COLUMN_TEXT_RESOURCE, column);
    field_set_text_macro(2, FIELD_TEXT_RESOURCE(FIELD_ITEM_COLUMN_TEXT_RESOURCE)->texts + offset, FIELD_TEXT_MACRO_UNLIMITED);
}
