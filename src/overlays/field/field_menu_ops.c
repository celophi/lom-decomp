#include "game_audio.h"
#include "saved_game.h"
#include "common.h"
#include "main.h"

/** @brief The game-state workspace viewed as the pad context that main.h maps. */
#define FIELD_PAD_CTX ((PadContext*)g_saved_game.bytes)

/**
 * @brief Small history or inventory record @p index of the pad context.
 * @note Adds the scaled index to the workspace base before the field offset,
 *       which is the address order the original code uses at these sites.
 */
#define FIELD_SMALL_HISTORY_RECORD(index) (((PadContext*)(g_saved_game.bytes + (index) * sizeof(SmallHistoryRecord)))->small_history_records[0])
#define FIELD_INVENTORY_RECORD(index) (((PadContext*)(g_saved_game.bytes + (index) * sizeof(InventoryRecord)))->inventory[0])

/*
 * Pad-context bytes that main.h does not map yet (inside _pad26E0 and
 * _pad29DB, and byte 1 of unkAA8). They are indexed through
 * g_saved_game.bytes because the original code mixes byte and word accesses.
 */
/** @brief Offset of the packed golem counts word; its low nibble is the golem count. */
#define FIELD_GOLEM_COUNTS 0x29D4
/** @brief Offset of the golem creation counter, byte 1 of the golem counts word. */
#define FIELD_GOLEM_CREATED 0x29D5
/** @brief Offset of the packed golem display order, three 2-bit slot indices. */
#define FIELD_GOLEM_DISPLAY_ORDER 0x29DB
/** @brief Offset of the active golem's class, byte 1 of unkAA8. */
#define FIELD_ACTIVE_GOLEM_CLASS 0xAA9
/**
 * @brief The script variables at D_80122C00, addressed from D_80122C0B.
 * @note The action item menus reach these variables through the D_80122C0B
 *       address, which the original code keeps as the base register.
 */
#define FIELD_MENU_ACTION_VARS ((FieldMenuVars*)(D_80122C0B - 0xB))
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

/** @brief The game-state workspace viewed as FieldMenuHistoryData. */
#define FIELD_MENU_HISTORY ((FieldMenuHistoryData*)g_saved_game.bytes)

/**
 * @brief Text @p id of the menu text table at D_800F0E98.
 * @note The table starts with little-endian 16-bit offsets relative to itself.
 */
#define FIELD_MENU_TEXT(id) (D_800F0E98[(id) * 2] + (D_800F0E98[(id) * 2 + 1] << 8) + D_800F0E98)

/**
 * @brief Byte @p index of the byte table at @p table.
 * @note Written as an integer sum so the index stays the first addu operand.
 */
#define FIELD_TABLE_BYTE(table, index) (*(u8*)((index) + (s32)(table)))

/** @brief Item id of the first of the eight menu action items. */
#define FIELD_ACTION_ITEM_BASE 0x58

/** @brief One 0x10-byte action slot of a menu action group. */
typedef struct
{
    s32 handle;
    u8 item_index; /**< 0x04: action item (index into item_counts); 0xFF when the slot is free. */
    u8 unknown_0x05[3];
    u8 counters[8]; /**< 0x08: per-slot counters advanced by func_800C83DC. */
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

/** @brief The game-state workspace viewed as FieldMenuActionData. */
#define FIELD_MENU_ACTIONS ((FieldMenuActionData*)g_saved_game.bytes)

/**
 * @brief The action data at @p base shifted by @p slot slots and @p group groups.
 * @note groups[0].slots[0] of the result is slot @p slot of group @p group.
 *       The slot and group offsets are summed as integers and the base is
 *       added last, which is the address order the original code uses.
 */
#define FIELD_MENU_ACTIONS_SHIFTED(base, group, slot) ((FieldMenuActionData*)((slot) * 0x10 + (group) * 0x8C + (s32)(base)))

/** @brief Script variables from D_80122C08: the action item choice mask and count. */
typedef struct
{
    u16 mask; /**< Bit set for each choice that is disabled. */
    u8 count;
} FieldMenuChoiceVars;

/** @brief The action item choice variables, which start at D_80122C08. */
#define FIELD_MENU_CHOICES ((FieldMenuChoiceVars*)&D_80122C08)

/** @brief Script variables from D_80122C0D that select one menu action slot. */
typedef struct
{
    u8 action_slot; /**< Action slot number plus 4. */
    u8 unk01[0xE];
    u8 item_index; /**< 0x0F (D_80122C1C): item of the slot, read by func_800C8014. */
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

/** @brief The record selection variables, which start at D_80122C02. */
#define FIELD_MENU_RECORD_SELECT ((FieldMenuRecordSelectVars*)&D_80122C02)

/** @brief Script variables from D_80122C04 that describe the selected item. */
typedef struct
{
    u8 kind;     /**< Item kind, bits 9:8 of the attributes. */
    u8 category; /**< Category with the kind's table offset added. */
    u16 value;   /**< Displayed stat value; bit 15 marks a special item. */
    u32 amount;  /**< Item value. */
} FieldMenuItemInfoVars;

/** @brief The item description variables, which start at D_80122C04. */
#define FIELD_MENU_ITEM_INFO ((FieldMenuItemInfoVars*)&D_80122C04)

/** @brief The action slot variables, which start at D_80122C0D. */
#define FIELD_MENU_ACTION_SLOT ((FieldMenuActionSlotVars*)&D_80122C0D)

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

/** @brief The game-state workspace viewed as FieldMenuItemData. */
#define FIELD_MENU_ITEMS ((FieldMenuItemData*)g_saved_game.bytes)

/** @brief Actor position as func_80087F44 returns it. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
} FieldPosition;

/** @brief Eight selection-index adjustments copied to the stack. */
typedef struct
{
    s32 entries[8];
} Choices;

/** @brief Menu-layout field used to select the preferred choice. */
typedef struct
{
    u8 pad[0x2E6];
    u16 selected;
} AttributeLayout;

/** @brief Scratch storage reused for the active layout and selected choice. */
typedef union
{
    AttributeLayout* layout;
    s32 selected;
} LayoutSelection;

typedef struct RecC98D4
{
    u8 pad0[0x24];
    u8 unk24;
    u8 pad25;
    u8 unk26;
} RecC98D4;

typedef struct OutC98D4
{
    u8 unk0;
    u8 unk1;
    u8 unk2;
    u8 unk3;
} OutC98D4;

/** @brief Nine-entry relationship lookup copied into the calculation workspace. */
typedef struct Lookup
{
    s32 values[9];
} Lookup;

/** @brief Byte-oriented view of a resource table entry's payload. */
typedef struct
{
    u8 pad0[4];
    u8 value;
} FieldResourceOffsetByte;

/** @brief Active menu record selected from D_80043CB8. */
typedef struct
{
    u8 pad0[0x24];
    u8 unk24;
    u8 unk25;
    u8 pad26[0x1A];
} FieldMenuRecordC9DCC;

/**
 * @brief Script temporary-variable words at D_80122C00.golem.slot, as the menu operations use them.
 *
 * Field scripts address this area as bitfield variables; each menu program
 * gives the bytes its own meaning, so each program has its own view.
 */
typedef union
{
    u8 bytes[0x20];
    s32 words[8];
    /** @brief Golem slot menus. */
    struct
    {
        s32 slot;           /**< Selected golem slot, an index into large_history_order. */
        s16 state;
        s16 slot_status[3];
        u8 unk0C[4];
        s16 result;
        u8 unk12[6];
        s16 unk18;
        s16 unk1A;
        s16 order_position;
        s16 detail;
    } golem;
    /** @brief Mystic Card slot menus. */
    struct
    {
        s16 card_ids[3];
    } cards;
    /** @brief Menu action item menus. */
    struct
    {
        u8 item_counts[8]; /**< Owned count of each action item. */
        u16 item_mask;     /**< Bit set for each action item that is not owned. */
        u8 owned_count;
        u8 used_slots; /**< Item slots in use in the active group. */
        u8 group_full; /**< Set when fewer than three item slots stay free. */
        u8 unk0D[7];
        s16 selected_item; /**< Chosen action item, 0-7. */
    } actions;
} FieldMenuVars;

u8* field_find_free_inventory_record();
void field_copy_inventory_record();
void field_open_gosub_screen_sequence();
void func_80087F44();
s32 func_80087D8C();
void func_800B2844(s32 macro_index, u8* text, u8 limit);
extern FieldMenuVars D_80122C00;
extern u8 D_80122C01;
extern u8 D_80122C02;
extern u8 D_80122C03;
extern u8 D_80122C04;
extern u8 D_80122C05;
extern s16 D_80122C06;
extern s16 D_80122C08;
extern s16 D_80122C0A;
extern u8 D_80122C0B[];
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
extern FieldMenuItemRecord D_80122A08[4];
/**
 * @brief Point @p record at shared item record @p index of D_80122A08.
 * @note Each use loads the table base into a local of its own.
 */
#define FIELD_SET_SHARED_RECORD(record, index)       \
    {                                                \
        FieldMenuItemRecord* shared_records;         \
                                                     \
        shared_records = D_80122A08;                 \
        (record) = &shared_records[index];           \
    }
extern u8 D_80043CB8[];
extern FieldGosubSequence D_80051EB4;
extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values[];
extern u8 D_800459AF;
extern s8 D_800459B3;
extern void func_800C4364(s32);
extern void func_800A54D0(void);
extern void field_compact_inventory(void);
extern void (*D_800F19D8[])(s32 arg0);
extern s32 D_801227F0;
extern FieldGosubSequence D_800F19AC;
extern FieldGosubSequence D_800F19B8;
extern FieldGosubSequence D_800F19C4;
extern u8 D_800459AE;
extern FieldPaletteSlotTable D_80051CBC;
extern s32 func_800A4744(void);
extern s32 func_800A4778(void);
extern FieldGosubSequence D_800F19CC;
extern FieldLogicClassTable D_80051CE4;
extern FieldLogicClassTable D_80051DCC;
void func_800C3BB0(void);
void func_800C9ED4();
void func_800AD030(s32 arg0);
u8* func_800C1E40(s32 arg0);
extern s32 D_80045EC8;
extern FieldGosubSequence D_80051EC0;
extern FieldGosubSequence D_80051ECC;
extern u8 D_80045ECC[];
extern u8 D_800F0E98[];
extern void func_800C7C88(void);
extern void func_800C0260(s32, s32);
extern s32 rand(void);
s32 func_8008B288(s32 arg0);
void func_800C2A88(s32 arg0);
void func_800CA1E0(void);
void func_800CA1A0(s32 arg0);
extern s32 D_8011F428;
extern u8 D_80046138[];
extern void func_800C8E2C(void);
extern s32 func_800BD414(s32 arg0, s32 arg1);
extern void func_800AD194(s32 arg0);
extern u8 D_80043818;
extern void func_800BD520(s32 arg0, s32 arg1, s32 arg2);
extern Choices D_80051ED8;
extern u16 g_music_track_index;
extern FieldGosubSequence D_80051EF8;
extern Lookup D_80051F04;
void func_800C57D4(void);
extern u8 D_800459AC;

/**
 * @brief Run one menu operation from the menu-op handler table.
 * @param op Menu-op index; indices of 0x60 and above record a diagnostic instead.
 */
void func_800C5704(s32 op)
{
    if (op < 0x60)
    {
        D_800F19D8[op](op);
        return;
    }
    record_game_diagnostic(0x8002, op, 0, 0);
}

/**
 * @brief Classify the selected golem slot against the active golem.
 *
 * Stores 1 (empty slot, no active golem) or 2 (empty slot, a golem is active)
 * when the selected slot holds no golem, otherwise 3 when the slot holds the
 * active golem and 4 when it does not.
 */
void func_800C5760(void)
{
    PadContext* ctx;

    ctx = FIELD_PAD_CTX;
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
void func_800C57D4(void)
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
void func_800C5804(void)
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
            if (FIELD_PAD_CTX->large_history_order[slot] == i)
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
        func_800C4364(free_record);
        count = g_saved_game.bytes[FIELD_GOLEM_CREATED] + 1;
        g_saved_game.bytes[FIELD_GOLEM_CREATED] = count;
        if (count >= 201)
        {
            g_saved_game.bytes[FIELD_GOLEM_CREATED] = 200;
        }
        previous = FIELD_PAD_CTX->large_history_order[D_80122C00.golem.slot];
        if (previous != 3)
        {
            if (FIELD_PAD_CTX->large_history_order[0] == 3)
            {
                FIELD_PAD_CTX->large_history_order[0] = previous;
            }
            else if (FIELD_PAD_CTX->large_history_order[1] == 3)
            {
                FIELD_PAD_CTX->large_history_order[1] = previous;
            }
            else if (FIELD_PAD_CTX->large_history_order[2] == 3)
            {
                FIELD_PAD_CTX->large_history_order[2] = previous;
            }
        }
        FIELD_PAD_CTX->large_history_order[D_80122C00.golem.slot] = free_record;
        packed_counts = (*(s32*)&g_saved_game.bytes[FIELD_GOLEM_COUNTS] & ~0xF) | (((g_saved_game.bytes[FIELD_GOLEM_COUNTS] & 0xF) + 1) & 0xF);
        *(s32*)&g_saved_game.bytes[FIELD_GOLEM_COUNTS] = packed_counts;
        if ((g_saved_game.bytes[FIELD_GOLEM_COUNTS] & 0xF) >= 4)
        {
            *(s32*)&g_saved_game.bytes[FIELD_GOLEM_COUNTS] = (packed_counts & ~0xF) | 3;
        }
        for (i = 0; i < g_gosub_result_count; i++)
        {
            field_copy_inventory_record(&FIELD_PAD_CTX->large_history_records[FIELD_PAD_CTX->large_history_order[D_80122C00.golem.slot]].unknown_0x4C[i << 6],
                                        &FIELD_PAD_CTX->inventory[g_gosub_result_values[i]]);
            FIELD_PAD_CTX->inventory[g_gosub_result_values[i]].active = 0;
        }
        field_compact_inventory();
        for (i = g_gosub_result_count; i < 4; i++)
        {
            FIELD_PAD_CTX->large_history_records[FIELD_PAD_CTX->large_history_order[D_80122C00.golem.slot]].unknown_0x4C[i << 6] = 0;
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
 * @brief Run func_800AD0C8 as a menu operation.
 */
void func_800C5AF0(void)
{
    func_800AD0C8();
}

/**
 * @brief Store the selected slot's golem record index and its class nibble.
 */
void func_800C5B10(void)
{
    PadContext* ctx;
    s32 record_index;

    ctx = FIELD_PAD_CTX;
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
void func_800C5B64(void)
{
    if ((u8)FIELD_PAD_CTX->large_history_index >= 4U)
    {
        FIELD_PAD_CTX->large_history_index = 3;
    }
    if ((FIELD_PAD_CTX->unkAA8 & 0x7F) == 3)
    {
        D_80122C00.golem.result = 2;
    }
    else if (FIELD_PAD_CTX->large_history_index == 3)
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
void func_800C5BCC(void)
{
    s32 i;
    u32 word;

    for (i = 0; i < FIELD_PAD_CTX->logic_block_count; i++)
    {
        word = FIELD_PAD_CTX->logic_blocks[i].word;
        if ((word & 3) == FIELD_PAD_CTX->large_history_order[D_80122C00.golem.slot])
        {
            FIELD_PAD_CTX->logic_blocks[i].word = (word | 3) & 0xFFFEFFFF;
        }
    }
    for (i = 0; i < 4; i++)
    {
        if (FIELD_PAD_CTX->large_history_records[FIELD_PAD_CTX->large_history_order[D_80122C00.golem.slot]].unknown_0x4C[i * 0x40] != 0)
        {
            if (field_find_free_inventory_record() != 0)
            {
                field_copy_inventory_record(field_find_free_inventory_record(),
                                            &FIELD_PAD_CTX->large_history_records[FIELD_PAD_CTX->large_history_order[D_80122C00.golem.slot]].unknown_0x4C[i * 0x40]);
            }
        }
    }
    FIELD_PAD_CTX->large_history_records[FIELD_PAD_CTX->large_history_order[D_80122C00.golem.slot]].name[0] = 0;
    FIELD_PAD_CTX->large_history_order[D_80122C00.golem.slot] = 3;
    word = *(u32*)&g_saved_game.bytes[FIELD_GOLEM_COUNTS];
    if ((word & 0xF) != 0)
    {
        *(u32*)&g_saved_game.bytes[FIELD_GOLEM_COUNTS] = (word & ~0xF) | (((g_saved_game.bytes[FIELD_GOLEM_COUNTS] & 0xF) - 1) & 0xF);
    }
}

/**
 * @brief Publish the selected slot's golem record to text macro 0.
 */
void func_800C5DA8(void)
{
    u8 record_index = FIELD_PAD_CTX->large_history_order[D_80122C00.golem.slot];

    func_800B2844(0, (u8*)&FIELD_PAD_CTX->large_history_records[record_index], 0xFF);
}

/**
 * @brief Clear the golem menu result when the gosub screen returned nothing.
 */
void func_800C5E08(void)
{
    if (g_gosub_result_count == 0)
    {
        D_80122C00.golem.result = 0;
    }
}

/**
 * @brief Repair the group display order and publish the current selection state.
 */
void func_800C5E28(void)
{
    s32 order[3];
    s16* slot_status;
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

    slot_status = &D_80122C06;
    slot_status[0] = 3;
    slot_status[1] = 3;
    slot_status[2] = 3;
    if (FIELD_PAD_CTX->large_history_order[0] != FIELD_PAD_CTX->large_history_index)
    {
        slot_status[0] = FIELD_PAD_CTX->large_history_order[0];
    }
    if (FIELD_PAD_CTX->large_history_order[1] != FIELD_PAD_CTX->large_history_index)
    {
        slot_status[1] = FIELD_PAD_CTX->large_history_order[1];
    }
    if (FIELD_PAD_CTX->large_history_order[2] != FIELD_PAD_CTX->large_history_index)
    {
        slot_status[2] = FIELD_PAD_CTX->large_history_order[2];
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
    if (D_80122C06 == 3)
    {
        if (order[0] == 0)
        {
            D_80122C06 = 4;
        }
        if (order[1] == 0)
        {
            D_80122C06 = 5;
        }
        if (order[2] == 0)
        {
            D_80122C06 = 6;
        }
    }
    if (D_80122C08 == 3)
    {
        if (order[0] == 1)
        {
            D_80122C08 = 4;
        }
        if (order[1] == 1)
        {
            D_80122C08 = 5;
        }
        if (order[2] == 1)
        {
            D_80122C08 = 6;
        }
    }
    if (D_80122C0A == 3)
    {
        if (order[0] == 2)
        {
            D_80122C0A = 4;
        }
        if (order[1] == 2)
        {
            D_80122C0A = 5;
        }
        if (order[2] == 2)
        {
            D_80122C0A = 6;
        }
    }
    selected_index = FIELD_PAD_CTX->large_history_index;
    if (selected_index < 3)
    {
        if (FIELD_PAD_CTX->large_history_order[0] == selected_index)
        {
            D_80122C06 = 3;
        }
        if (FIELD_PAD_CTX->large_history_order[1] == selected_index)
        {
            D_80122C08 = 3;
        }
        if (FIELD_PAD_CTX->large_history_order[2] == selected_index)
        {
            D_80122C0A = 3;
        }
    }
    active_index = D_80122C00.golem.slot;
    for (i = 0; i < 3; i++)
    {
        if (order[i] == active_index)
        {
            D_80122C1C = i;
        }
    }
    *(s16*)&D_80122C1E = FIELD_PAD_CTX->large_history_index;
}

/**
 * @brief Report whether the logic-block table is full (40 blocks).
 */
void func_800C61D8(void)
{
    if (FIELD_PAD_CTX->logic_block_count >= LOGIC_BLOCK_CAPACITY)
    {
        D_80122C10 = 1;
    }
    else
    {
        D_80122C10 = 0;
    }
}

/**
 * @brief Run func_800AD0C8 as a menu operation.
 */
void func_800C6208(void)
{
    func_800AD0C8();
}

/**
 * @brief Dispatch each populated row of the selected menu entry and count them.
 */
void func_800C6228(void)
{
    s32 count;
    s32 i;

    count = 0;
    for (i = 0; i < 4; i++)
    {
        if (FIELD_PAD_CTX->large_history_records[FIELD_PAD_CTX->large_history_order[D_80122C00.golem.slot]].unknown_0x4C[i << 6] != 0)
        {
            func_800B2844(count, &FIELD_PAD_CTX->large_history_records[FIELD_PAD_CTX->large_history_order[D_80122C00.golem.slot]].unknown_0x4C[i << 6], 0xFF);
            count += 1;
        }
    }
    D_80122C10 = count;
}

/**
 * @brief Subtract the number of free inventory records from the requested count, clamping at 0.
 */
void func_800C62E8(void)
{
    s32 i;
    s32 free_count;
    u16 remaining;
    s16* requested;

    free_count = 0;
    for (i = 0; i < INVENTORY_RECORD_COUNT; i++)
    {
        if (FIELD_PAD_CTX->inventory[i].active == 0)
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
 * @brief Run func_800C3A00 with argument 0x92BC.
 */
void func_800C6344(void)
{
    func_800C3A00(0x92BC);
}

/**
 * @brief Pass the menu object's id and variant to func_80087680.
 */
void func_800C6364(void)
{
    func_80087680(FIELD_MENU_OBJECT->object_id, FIELD_MENU_OBJECT->variant, FIELD_MENU_OBJECT->variant, 0, 0, 0);
}

/**
 * @brief Lower the menu object by its height offset, in whole position units.
 */
void func_800C63A0(void)
{
    s32 position[3];

    func_80087F44(FIELD_MENU_OBJECT->object_id, position);
    position[0] /= 256;
    position[1] /= 256;
    position[2] /= 256;
    position[1] -= FIELD_MENU_OBJECT->height_offset;
    func_80087D8C(FIELD_MENU_OBJECT->object_id, position[0], position[1], position[2]);
}

/**
 * @brief Move the menu object one easing step toward its target position.
 *
 * Works in whole position units. While the object is at or above the target
 * height it closes two thirds of the horizontal distance per step (one unit
 * at a time when close); once it has arrived horizontally, or is below the
 * target height, the height is eased the same way.
 */
void func_800C642C(void)
{
    s32 position[3];
    s32 target_y;
    s32 target_x;
    s32 target_z;
    s32 y;
    s32 delta;
    s32 distance;
    s32 x;

    func_80087F44(FIELD_MENU_OBJECT->object_id, position);
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

    func_80087D8C(FIELD_MENU_OBJECT->object_id, position[0], position[1], position[2]);
}

/**
 * @brief Store the gosub-selected palette entry's slot in the selected golem record.
 *
 * Looks the selection (an id from 0x60) up in the D_80051CBC table and stores
 * the result, clamped to 0..31, in the golem record's word at 0x48.
 */
void func_800C66DC(void)
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
    record_index = FIELD_PAD_CTX->large_history_order[D_80122C00.golem.slot];
    FIELD_PAD_CTX->large_history_records[record_index].unknown_0x48 = clamped;
    func_800A54D0();
}

/**
 * @brief Make the golem shown in slot status 0 the active golem.
 */
void func_800C6834(void)
{
    s32 status = D_80122C06;

    g_game_diagnostic_status = status;
    D_800459AF = status;
}

/**
 * @brief Store func_800A4744's result, or func_800A4778's with a failure flag when it fails.
 */
void func_800C6850(void)
{
    s32 result = func_800A4744();

    if (result < 0)
    {
        D_80122C16 = 1;
        /* The value variable sits just below the failure flag. */
        *(&D_80122C16 - 1) = func_800A4778();
    }
    else
    {
        D_80122C16 = 0;
        *(&D_80122C16 - 1) = result;
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
void func_800C68C8(void)
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
    active_record = FIELD_PAD_CTX->large_history_index;
    selected_slot = D_80122C00.golem.slot;
    if (FIELD_PAD_CTX->large_history_order[selected_slot] != active_record)
    {
        s32 i;
        s32 swap_position;

        for (i = 0; i < 3; i++)
        {
            if (active_record == FIELD_PAD_CTX->large_history_order[i])
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
        D_800459B3 = packed_order;
    }
    FIELD_MENU_SWAP->selected_position = selected_position;
    FIELD_MENU_SWAP->active_slot = active_slot;
}

/**
 * @brief Publish the inventory record selected by the result variable to text macro 0.
 */
void func_800C69F4(void)
{
    func_800B2844(0, (u8*)&FIELD_PAD_CTX->inventory[D_80122C10], 0xFF);
}

/**
 * @brief Count the inventory records of the item kind in the result variable.
 */
void func_800C6A30(void)
{
    s32 i;
    s32 count;
    s32 kind;

    kind = D_80122C10;
    count = 0;
    for (i = 0; i < INVENTORY_RECORD_COUNT; i++)
    {
        if (FIELD_PAD_CTX->inventory[i].active != 0)
        {
            if (((FIELD_PAD_CTX->inventory[i].attributes.packed >> 8) & 3) == kind)
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
void func_800C6A90(void)
{
    while (field_find_free_inventory_record() != 0)
    {
        field_copy_inventory_record(field_find_free_inventory_record(), FIELD_PAD_CTX->inventory);
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
void func_800C6AF0(void)
{
    FieldLogicClassTable class_table;
    FieldLogicClassTable unused_table;
    PadContext* ctx;
    s32 i;
    s8 active_record;
    s32 golem_class;
    u32 word;
    s32 block_class;
    PadContext* shifted;
    s32 placed;
    u8* classes;
    u32 unplaced_mask;

    class_table = D_80051CE4;
    unused_table = D_80051DCC;

    ctx = FIELD_PAD_CTX;
    active_record = ctx->large_history_index;
    golem_class = D_80122C1C;
    if (active_record < 3)
    {
        /* The pad context shifted by active_record records: record 0 is the active golem. */
        shifted = (PadContext*)((u8*)ctx + active_record * sizeof(LargeHistoryRecord));
        g_saved_game.bytes[FIELD_ACTIVE_GOLEM_CLASS] = (u8)D_80122C1C;
        *(s32*)&shifted->large_history_records[0].unknown_0x44 = (*(s32*)&shifted->large_history_records[0].unknown_0x44 & ~0xF) | ((u8)D_80122C1C & 0xF);
        i = 0;
        if (ctx->logic_block_count != 0)
        {
            placed = 1;
            classes = (u8*)&class_table;
            unplaced_mask = 0xFFFEFFFF;
            do
            {
                word = ctx->logic_blocks[i].word;
                if (((word >> 16) & 1) == placed && (word & 3) == active_record)
                {
                    block_class = *(s32*)(classes + (word & 0xFC));
                    if (block_class != 0 && block_class != golem_class)
                    {
                        ctx->logic_blocks[i].word = (word & unplaced_mask) | 3;
                    }
                }
                i++;
            } while (i < ctx->logic_block_count);
        }
        func_800C3BB0();
    }
}

/**
 * @brief Append a logic block built from the variables at D_80122C18..D_80122C1C.
 *
 * An id of 0xFF clears the logic-block count instead. The new block is
 * unassigned and not placed.
 */
void func_800C6C80(void)
{
    s16* params = &D_80122C1C;
    s32 id = params[0];
    s32 quantity = params[-1];
    s32 shape = params[-2];

    if (id == 0xFF)
    {
        D_800459AE = 0;
        return;
    }

    if (FIELD_PAD_CTX->logic_block_count < LOGIC_BLOCK_CAPACITY)
    {
        FIELD_PAD_CTX->logic_blocks[FIELD_PAD_CTX->logic_block_count].word =
            (FIELD_PAD_CTX->logic_blocks[FIELD_PAD_CTX->logic_block_count].word & ~0xFC) | ((id & 0x3F) << 2);
        FIELD_PAD_CTX->logic_blocks[FIELD_PAD_CTX->logic_block_count].word =
            (FIELD_PAD_CTX->logic_blocks[FIELD_PAD_CTX->logic_block_count].word & ~0xF00) | ((quantity & 0xF) << 8);
        FIELD_PAD_CTX->logic_blocks[FIELD_PAD_CTX->logic_block_count].word =
            (FIELD_PAD_CTX->logic_blocks[FIELD_PAD_CTX->logic_block_count].word & ~0xF000) | ((shape & 0xF) << 12);
        FIELD_PAD_CTX->logic_blocks[FIELD_PAD_CTX->logic_block_count].word |= 3;
        FIELD_PAD_CTX->logic_blocks[FIELD_PAD_CTX->logic_block_count].word &= ~0x10000;
        FIELD_PAD_CTX->logic_block_count++;
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
 * @brief Run func_800AD030 with argument 0.
 */
void func_800C6E08(void)
{
    func_800AD030(0);
}

/**
 * @brief Replace the inventory record index in the result variable with its name-table row.
 *
 * The row is the record's category (bits 10-15 of its attribute word) offset by
 * its kind: +0 for kind 0, +0xB for kind 1 and +0x17 otherwise. A row of 0xFF
 * records a diagnostic and yields 0.
 */
void func_800C6E28(void)
{
    u32 attributes;
    s32 kind;
    s32 row;

    attributes = FIELD_INVENTORY_RECORD(D_80122C10).attributes.packed;
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
        record_game_diagnostic(0x8002, 0x22, 0, 0);
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
void func_800C6EBC(void)
{
    s32 i;
    s16* card;
    u8 first;
    u8 second;
    u8 third;
    s16 value;
    s32 empty;

    i = 0;
    empty = 0xFF;
    card = D_80122C00.cards.card_ids;
    first = FIELD_INVENTORY_RECORD(D_80122C10).unknown_0x18[8];
    second = FIELD_INVENTORY_RECORD(D_80122C10).unknown_0x18[9];
    third = FIELD_INVENTORY_RECORD(D_80122C10).unknown_0x18[10];
    for (; i < 3; i++, card++)
    {
        value = *card;
        if (value != empty)
        {
            if (value == first)
            {
                *card = empty;
                first = 0xFF;
            }
            else if (value == second)
            {
                *card = empty;
                second = 0xFF;
            }
            else if (value == third)
            {
                *card = empty;
                third = 0xFF;
            }
        }
    }
    (&D_80122C06)[0] = first;
    (&D_80122C06)[1] = second;
    (&D_80122C06)[2] = third;
}

/**
 * @brief Load the Mystic Card ids of the gosub-selected inventory record into the card list.
 */
void func_800C6F60(void)
{
    D_80122C00.cards.card_ids[0] = FIELD_INVENTORY_RECORD(g_gosub_result_values[0]).unknown_0x18[8];
    D_80122C00.cards.card_ids[1] = FIELD_INVENTORY_RECORD(g_gosub_result_values[0]).unknown_0x18[9];
    D_80122C00.cards.card_ids[2] = FIELD_INVENTORY_RECORD(g_gosub_result_values[0]).unknown_0x18[10];
}

/**
 * @brief Look up the menu object's halfword in resource 0x102.
 *
 * Each object has two halfwords after the resource header; the variant selects
 * the second one. Object id 0xFF yields 0xFFFF.
 */
void func_800C6F9C(void)
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
 *
 * The resource starts with a four-byte header followed by a table of
 * little-endian text offsets, one per object.
 */
void func_800C7014(void)
{
    s32 object_id;
    s32 index;
    u8* resource;
    s32 offset;

    object_id = FIELD_MENU_OBJECT->object_id;
    resource = func_800C1E40(0x101);
    index = object_id * 2;
    offset = (resource + index)[4] + ((func_800C1E40(0x101) + (index += 1))[4] << 8);
    func_800B2844(0, func_800C1E40(0x101) + (offset + 4), 0xFF);
}

/**
 * @brief Decode the gosub-selected small history record into a display id and mode.
 *
 * A blocked record shows id 0x53 + its byte at 0x16 in mode 0; otherwise it
 * shows 0x12 + its byte at 0x15, in mode 1 when selection is restricted. The
 * record at the small-history index shows 0xFE. Also stores the result count.
 */
void func_800C7090(void)
{
    s32 index;
    u8* base;
    PadContext* shifted;
    s32 flags;

    if (g_gosub_result_count != 0)
    {
        index = g_gosub_result_values[0];
        if (index < SMALL_HISTORY_RECORD_COUNT)
        {
            base = g_saved_game.bytes;
            /* The pad context shifted by index records: record 0 is record index. */
            shifted = (PadContext*)(base + index * sizeof(SmallHistoryRecord));
            flags = *(s32*)&shifted->small_history_records[0].selection_flags;
            if (flags < 0)
            {
                FIELD_MENU_RECORD->display_id = shifted->small_history_records[0].unknown_0x16 + 0x53;
                FIELD_MENU_RECORD->mode = 0;
            }
            else
            {
                s16 restricted = (s16)(((u32)flags >> 30) & 1);
                FIELD_MENU_RECORD->display_id = shifted->small_history_records[0].unknown_0x15 + 0x12;
                FIELD_MENU_RECORD->mode = restricted;
            }
            if (index == D_80045EC8)
            {
                FIELD_MENU_RECORD->display_id = 0xFE;
            }
        }
        else
        {
            record_game_diagnostic(0x8002, 0x27, index, 0);
        }
    }
    D_80122C16 = g_gosub_result_count;
}

/**
 * @brief Count the occupied small history records whose selection is restricted.
 */
void func_800C7168(void)
{
    s32 count;
    s32 i;
    s32 one = 1; /* compared in a register, as the original does */
    PadContext* shifted;

    if (g_gosub_result_count != 0)
    {
        count = 0;
        for (i = 0; i < SMALL_HISTORY_RECORD_COUNT; i++)
        {
            /* The pad context shifted by i records: record 0 is record i. */
            shifted = (PadContext*)&g_saved_game.bytes[i * sizeof(SmallHistoryRecord)];
            if (shifted->small_history_records[0].name[0] != 0 && shifted->small_history_records[0].selection_flags.selection_restricted == one)
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
void func_800C71D4(void)
{
    s32 index;

    index = g_gosub_result_values[0];
    if (index < SMALL_HISTORY_RECORD_COUNT)
    {
        FIELD_PAD_CTX->small_history_records[index].selection_flags.selection_restricted = 1;
    }
    else
    {
        record_game_diagnostic(0x8002, 0x29, index, 0);
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
void func_800C7278(void)
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
void func_800C72E4(void)
{
    s32 index;

    FIELD_MENU_NAMED->index = index = g_gosub_result_values[0];
    FIELD_MENU_NAMED->count = g_gosub_result_count;
    func_800B2844(FIELD_MENU_NAMED->macro_index, FIELD_PAD_CTX->small_history_records[index].name, 0xFF);
}

/**
 * @brief Publish the selected small history record's extra slot texts and its name.
 * @note Unused slots (0xFE/0xFF) are skipped; the number published goes to D_80122C16.
 * @note Selections of five or greater record a diagnostic instead.
 */
void func_800C7340(void)
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
                    func_800B2844(count, FIELD_MENU_TEXT(entry), 0xFF);
                    count++;
                }
            }
        }
        func_800B2844(3, D_80045ECC + selection * 0x60, 0xFF);
        D_80122C16 = count;
        return;
    }
    record_game_diagnostic(0x8002, 0x2E, selection, 0);
}

/** @brief Clear pad-context byte 0xC06 and the active large history record's 0x46 byte. */
void func_800C745C(void)
{
    PadContext* ctx;
    s32 index;

    ctx = FIELD_PAD_CTX;
    index = ctx->large_history_index;
    g_saved_game.bytes[0xC06] = 0;
    ctx->large_history_records[index].unknown_0x46 = 0;
}

/**
 * @brief Store D_80122C12 in the first unused extra slot of small history record D_80122C10.
 * @note Selections of five or greater record a diagnostic instead.
 */
void func_800C7494(void)
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
    record_game_diagnostic(0x8002, 0x30, selection, 0);
}

/** @brief Replace the small history index in D_80122C10 with that record's entry id. */
void func_800C752C(void)
{
    D_80122C10 = FIELD_PAD_CTX->small_history_records[D_80122C10].unknown_0x15;
}

/**
 * @brief Lift the selection restriction of the current small history record.
 * @note Indices of five or greater record a diagnostic instead.
 */
void func_800C7558(void)
{
    s32 index;

    index = FIELD_PAD_CTX->small_history_index;
    if (index < SMALL_HISTORY_RECORD_COUNT)
    {
        FIELD_PAD_CTX->small_history_records[index].selection_flags.selection_restricted = 0;
    }
    else
    {
        record_game_diagnostic(0x8002, 0x32, index, 0);
    }
}

/**
 * @brief Lift the selection restriction of the gosub-selected small history record.
 * @note Indices of five or greater record a diagnostic instead.
 */
void func_800C75C0(void)
{
    s32 index;

    index = g_gosub_result_values[0];
    if (index < SMALL_HISTORY_RECORD_COUNT)
    {
        FIELD_PAD_CTX->small_history_records[index].selection_flags.selection_restricted = 0;
    }
    else
    {
        record_game_diagnostic(0x8002, 0x32, index, 0);
    }
}

/**
 * @brief Load the gosub-selected small history record's 0x5A halfword into D_80122C00.
 * @note Does nothing when the gosub returned no selection.
 */
void func_800C7628(void)
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
void func_800C766C(void)
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
        counts[i] = FIELD_PAD_CTX->item_counts[FIELD_ACTION_ITEM_BASE + i];
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
    flags = FIELD_MENU_ACTIONS->groups[group].flags;
    item_slot_count = flags >> 8;
    item_slot_count &= 0xF;
    capacity = flags & 0xF;
    for (i = 0; i < 8; i++)
    {
        if (FIELD_MENU_ACTIONS->groups[group].slots[i].item_index < 0xFF)
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
    func_800B2844(4, FIELD_MENU_TEXT(item), 0xFF);
    func_800C7C88();
}

/**
 * @brief Put the chosen action item into the first free item slot of group D_80122C1F.
 *
 * Records the new used-slot count in the group flags and at D_80122C0B, sets
 * D_80122C0C when fewer than three slots stay free, takes one of the item
 * (D_80122C14) from the counts copied to D_80122C00, publishes its text to
 * macro 0 and refreshes the item mask and owned count at D_80122C08.
 */
void func_800C7840(void)
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
    flags = FIELD_MENU_ACTIONS->groups[group].flags;
    item_slot_count = flags >> 8;
    item_slot_count &= 0xF;
    capacity = flags & 0xF;
    for (slot = 0; slot < 8; slot++)
    {
        if (FIELD_MENU_ACTIONS->groups[group].slots[slot].item_index != 0xFF)
        {
            count++;
        }
    }
    capacity -= count;

    for (slot = 0; slot < item_slot_count; slot++)
    {
        if (FIELD_MENU_ACTIONS->groups[group].item_slots[slot] == 0xFF)
        {
            break;
        }
    }

    used = slot + 1;
    /* D_80122C0B, D_80122C0C and the selected item are reached from D_80122C0B. */
    D_80122C0B[0] = used;
    FIELD_MENU_ACTIONS->groups[group].flags = (FIELD_MENU_ACTIONS->groups[group].flags & 0xFFFF0FFF) | ((used & 0xF) << 12);
    if (capacity < slot + 3)
    {
        D_80122C0B[1] = 1;
    }
    selected = *(s16*)&D_80122C0B[9];
    FIELD_MENU_ACTION_VARS->actions.item_counts[selected]--;
    item = selected + FIELD_ACTION_ITEM_BASE;
    FIELD_MENU_ACTIONS->groups[group].item_slots[slot] = item;
    func_800B2844(0, FIELD_MENU_TEXT(item), 0xFF);

    mask = 0x1FFF;
    owned = 0;
    for (slot = 0; slot < 8; slot++)
    {
        if (FIELD_MENU_ACTION_VARS->actions.item_counts[slot] != 0)
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
 * slot count, through func_800C0260. Each fill picks a random slot (at most
 * 1000 tries) and falls back to the first free slot. Then gives back one of
 * each item in the group's item slots and clears them through func_800C7C88.
 */
void func_800C7A3C(void)
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
    flags = FIELD_MENU_ACTIONS->groups[group].flags;
    free_slots = flags & 0xF;
    item_slot_count = flags >> 8;
    item_slot_count &= 0xF;
    fill = flags >> 12;
    fill &= 0xF;
    fill += 2;
    for (; i < 8; i++)
    {
        if (FIELD_MENU_ACTIONS->groups[group].slots[i].item_index != 0xFF)
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
            if (FIELD_MENU_ACTIONS->groups[group].slots[pick].item_index == 0xFF)
            {
                break;
            }
        }
        if (FIELD_MENU_ACTIONS->groups[group].slots[pick].item_index == 0xFF)
        {
            func_800C0260(group, pick);
        }
        else
        {
            for (j = 0; j < 8; j++)
            {
                if (FIELD_MENU_ACTIONS->groups[group].slots[j].item_index == 0xFF)
                {
                    func_800C0260(group, j);
                    break;
                }
            }
        }
    }

    for (i = 0; i < item_slot_count; i++)
    {
        item = FIELD_MENU_ACTIONS->groups[group].item_slots[i];
        if (item < 0xFF)
        {
            FIELD_PAD_CTX->item_counts[item]--;
        }
    }
    func_800C7C88();
}

/** @brief Clear the used item-slot count of group D_80122C1F and empty its item slots. */
void func_800C7C88(void)
{
    s32 group;
    s32 i;

    group = D_80122C1F;
    FIELD_MENU_ACTIONS->groups[group].flags &= 0xFFFF0FFF;
    for (i = 0; i < 4; i++)
    {
        FIELD_MENU_ACTIONS->groups[group].item_slots[i] = 0xFF;
    }
}

/** @brief Free the eight action slots of group 0 and clear its used item-slot count. */
void func_800C7CF8(void)
{
    FieldMenuActionData* data;
    u32 flags;

    data = FIELD_MENU_ACTIONS;
    flags = data->groups[0].flags;
    data->groups[0].slots[0].handle = 0;
    data->groups[0].slots[0].item_index = 0xFF;
    data->groups[0].slots[1].handle = 0;
    data->groups[0].slots[1].item_index = 0xFF;
    data->groups[0].slots[2].handle = 0;
    data->groups[0].slots[2].item_index = 0xFF;
    data->groups[0].slots[3].handle = 0;
    data->groups[0].slots[3].item_index = 0xFF;
    data->groups[0].slots[4].handle = 0;
    data->groups[0].slots[4].item_index = 0xFF;
    data->groups[0].slots[5].handle = 0;
    data->groups[0].slots[5].item_index = 0xFF;
    data->groups[0].slots[6].handle = 0;
    data->groups[0].slots[6].item_index = 0xFF;
    data->groups[0].slots[7].handle = 0;
    data->groups[0].slots[7].item_index = 0xFF;
    data->groups[0].flags = flags & 0xFFFF0FFF;
}

/** @brief Publish the text of the chosen action item (D_80122C14) to macro 4. */
void func_800C7D5C(void)
{
    s32 item;

    item = D_80122C14 + FIELD_ACTION_ITEM_BASE;
    func_800B2844(4, FIELD_MENU_TEXT(item), 0xFF);
}

/**
 * @brief Place actor D_80122C0D next to actor 0, on the side it faces.
 *
 * Takes actor 0's position (8.8 fixed point) and heading (0x00-0xFF), steps
 * 0x14 along x or 0xC along z for the four main headings and 0xA along both
 * axes in between, and moves the actor there, 0xC higher.
 */
void func_800C7DB8(void)
{
    s32 pos[3];
    s32 heading;
    s32 actor;

    actor = D_80122C0D;
    func_80087F44(0, pos);
    pos[0] /= 256;
    pos[1] /= 256;
    pos[2] /= 256;

    heading = func_8008B288(0);
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

    func_80087D8C(actor, pos[0], pos[1] - 0xC, pos[2]);
}

/**
 * @brief Free the action slot selected by D_80122C0D and give its item back.
 * @note The item count is clamped to 0-99.
 */
void func_800C7F44(void)
{
    s32 slot;
    s32 group;
    s32 count;
    s32 clamped;
    u8 item;

    slot = FIELD_MENU_ACTION_SLOT->action_slot;
    slot -= 4;
    group = FIELD_MENU_ACTION_SLOT->group;
    item = FIELD_MENU_ACTIONS->groups[group].slots[slot].item_index;
    count = FIELD_PAD_CTX->item_counts[item];
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

    FIELD_PAD_CTX->item_counts[item] = clamped;
    FIELD_MENU_ACTIONS->groups[group].slots[slot].handle = 0;
    FIELD_MENU_ACTIONS->groups[group].slots[slot].item_index = 0xFF;
    FIELD_MENU_ACTIONS->groups[group].slots[slot].counters[0] = 0;
    FIELD_MENU_ACTIONS->groups[group].slots[slot].counters[1] = 0;
    FIELD_MENU_ACTIONS->groups[group].slots[slot].counters[2] = 0;
    FIELD_MENU_ACTIONS->groups[group].slots[slot].counters[3] = 0;
    FIELD_MENU_ACTIONS->groups[group].slots[slot].counters[4] = 0;
    FIELD_MENU_ACTIONS->groups[group].slots[slot].counters[5] = 0;
    FIELD_MENU_ACTIONS->groups[group].slots[slot].counters[6] = 0;
    FIELD_MENU_ACTIONS->groups[group].slots[slot].counters[7] = 0;
}

/**
 * @brief Load the action slot selected by D_80122C0D into its script variables.
 * @note Publishes the slot item's text to macro 0 and stores the item in D_80122C1C.
 */
void func_800C8014(void)
{
    FieldMenuActionData* shifted;
    u8* base;
    s32 slot;
    s32 handle;
    u8 item;

    slot = FIELD_MENU_ACTION_SLOT->action_slot - 4;
    base = g_saved_game.bytes;
    shifted = FIELD_MENU_ACTIONS_SHIFTED(base, FIELD_MENU_ACTION_SLOT->group, slot);
    handle = shifted->groups[0].slots[0].handle;
    FIELD_MENU_ACTION_SLOT->handle = handle;
    item = shifted->groups[0].slots[0].item_index;
    func_800B2844(0, FIELD_MENU_TEXT(item), 0xFF);
    FIELD_MENU_ACTION_SLOT->item_index = item;
}

/**
 * @brief Copy the gosub-selected inventory record into the first free pending record.
 *
 * Sets D_80122C1F to 0 when the gosub returned nothing, 2 when the record
 * holds no item, 3 when a pending record already holds the same item and 1
 * when the record was copied (its 0x34 word is then at least 1).
 */
void func_800C80BC(void)
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
            field_copy_inventory_record(&FIELD_MENU_ITEMS->pending[i], &FIELD_MENU_ITEMS->inventory[selected]);
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
 * Like func_800C7DB8, with steps of 0xA along one axis for the four main
 * headings and 8 along both axes in between, at the same height.
 */
void func_800C8260(void)
{
    FieldPosition pos;
    s32 heading;

    func_80087F44(0, (s32*)&pos);
    pos.x /= 256;
    pos.y /= 256;
    pos.z /= 256;

    heading = func_8008B288(0);
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

    func_80087D8C(0xC, pos.x, pos.y, pos.z);
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
void func_800C83DC(void)
{
    FieldMenuActionData* shifted;
    u8* base;
    s32 group;
    s32 slot;
    u8 counter;

    group = 0;
    base = g_saved_game.bytes;
    for (; group < 4; group++)
    {
        for (slot = 0; slot < 8; slot++)
        {
            shifted = FIELD_MENU_ACTIONS_SHIFTED(base, group, slot);
            if (shifted->groups[0].slots[0].item_index != 0xFF)
            {
                counter = shifted->groups[0].slots[0].counters[0];
                if (counter == 0)
                {
                    shifted->groups[0].slots[0].counters[0] = counter + 1;
                }
                else if (counter < 0xF0)
                {
                    FIELD_STEP_ACTION_COUNTERS(shifted->groups[0].slots[0]);
                    FIELD_STEP_ACTION_COUNTERS(shifted->groups[0].slots[0]);
                    FIELD_STEP_ACTION_COUNTERS(shifted->groups[0].slots[0]);
                    FIELD_STEP_ACTION_COUNTERS(shifted->groups[0].slots[0]);
                    FIELD_STEP_ACTION_COUNTERS(shifted->groups[0].slots[0]);
                    FIELD_STEP_ACTION_COUNTERS(shifted->groups[0].slots[0]);
                    FIELD_STEP_ACTION_COUNTERS(shifted->groups[0].slots[0]);
                    FIELD_STEP_ACTION_COUNTERS(shifted->groups[0].slots[0]);
                    FIELD_STEP_ACTION_COUNTERS(shifted->groups[0].slots[0]);
                }
            }
        }
    }
}

/** @brief Free the action slot selected by D_80122C0D without giving its item back. */
void field_reset_menu_action_slot(void)
{
    FieldMenuActionData* shifted;
    u8* base;
    s32 slot;

    slot = FIELD_MENU_ACTION_SLOT->action_slot - 4;
    base = g_saved_game.bytes;
    shifted = FIELD_MENU_ACTIONS_SHIFTED(base, FIELD_MENU_ACTION_SLOT->group, slot);
    shifted->groups[0].slots[0].item_index = 0xFF;
    shifted->groups[0].slots[0].handle = 0;
    shifted->groups[0].slots[0].counters[0] = 0;
    shifted->groups[0].slots[0].counters[1] = 0;
    shifted->groups[0].slots[0].counters[2] = 0;
    shifted->groups[0].slots[0].counters[3] = 0;
    shifted->groups[0].slots[0].counters[4] = 0;
    shifted->groups[0].slots[0].counters[5] = 0;
    shifted->groups[0].slots[0].counters[6] = 0;
    shifted->groups[0].slots[0].counters[7] = 0;
    /* Bytes 5-7 of the slot are set to 0xFF as part of their word. */
    *(u32*)&shifted->groups[0].slots[0].item_index |= ~0xFF;
}

/** @brief Reset layout slots and activate the default set in order. */
void func_800C8830(void)
{
    func_800CA1E0();
    func_800CA1A0(0);
    func_800CA1A0(1);
    func_800CA1A0(2);
    func_800CA1A0(3);
    func_800CA1A0(4);
    func_800CA1A0(5);
    func_800CA1A0(7);
    func_800CA1A0(8);
    func_800CA1A0(9);
    func_800CA1A0(0xA);
    func_800CA1A0(0xB);
    func_800CA1A0(0xC);
    func_800CA1A0(0xD);
    func_800CA1A0(0xF);
    func_800CA1A0(0x10);
    func_800CA1A0(0x11);
    func_800CA1A0(0x12);
    func_800CA1A0(0x13);
    func_800CA1A0(0x15);
    func_800CA1A0(0x16);
    func_800CA1A0(0x18);
    func_800CA1A0(0x19);
    func_800CA1A0(0x1A);
    func_800CA1A0(0x1B);
    func_800CA1A0(0x1E);
    func_800CA1A0(0x1F);
    func_800CA1A0(0x20);
    func_800CA1A0(0x20);
    func_800CA1A0(0x17);
}

/** @brief Restore the saved field mode through the mode dispatcher. */
void func_800C8938(void)
{
    D_8011F428 = (s32)D_80122C1E;
    func_800AD120(D_80122C1E);
}

/**
 * @brief List the occupied small history records whose selection is restricted.
 * @note Stores each listed record's entry id from D_80122C00 and its index from
 *       D_80122C1D, and publishes its name to macro 0, 1, ... in order.
 */
void func_800C8964(void)
{
    s32 count;
    s32 i;
    u8* ids;
    u8* indices;

    i = count = 0;
    ids = D_80122C00.bytes;
    indices = ids + 0x1D;
    for (; i < SMALL_HISTORY_RECORD_COUNT; i++)
    {
        if (FIELD_PAD_CTX->small_history_records[i].name[0] != 0 && FIELD_PAD_CTX->small_history_records[i].selection_flags.selection_restricted == 1)
        {
            FIELD_TABLE_BYTE(ids, count) = FIELD_PAD_CTX->small_history_records[i].unknown_0x15;
            FIELD_TABLE_BYTE(indices, count) = i;
            func_800B2844(count, FIELD_PAD_CTX->small_history_records[i].name, 0xFF);
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
void func_800C8A2C(void)
{
    s32 i;
    s32 duplicate_found;
    s32 selected;
    FieldMenuItemRecord* record;
    FieldMenuItemRecord* search;
    FieldMenuItemRecord* detail;
    FieldMenuItemRecord* shown;
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
    FIELD_SET_SHARED_RECORD(record, selected);
    if (record->active == 0)
    {
        selection->status = 1;
        return;
    }

    duplicate_found = 0;
    i = 0;
    search = record;
    for (; i < INVENTORY_RECORD_COUNT; i++)
    {
        if (FIELD_MENU_ITEMS->inventory[i].active != 0 && FIELD_MENU_ITEMS->inventory[i].identity[0] == search->identity[0] &&
            FIELD_MENU_ITEMS->inventory[i].identity[1] == search->identity[1])
        {
            duplicate_found = 1;
            break;
        }
    }

    i = 0;
    FIELD_SET_SHARED_RECORD(search, selected);
    for (; i < 8; i++)
    {
        if (FIELD_MENU_ITEMS->equipment[i].active != 0 && FIELD_MENU_ITEMS->equipment[i].identity[0] == search->identity[0] &&
            FIELD_MENU_ITEMS->equipment[i].identity[1] == search->identity[1])
        {
            duplicate_found = 1;
            break;
        }
    }

    i = 0;
    FIELD_SET_SHARED_RECORD(search, selected);
    for (; i < 4; i++)
    {
        if (FIELD_MENU_ITEMS->pending[i].active != 0 && FIELD_MENU_ITEMS->pending[i].identity[0] == search->identity[0] &&
            FIELD_MENU_ITEMS->pending[i].identity[1] == search->identity[1])
        {
            duplicate_found = 1;
            break;
        }
    }

    if (duplicate_found == 0)
    {
        attributes = D_80122A08[selected].attributes.packed;
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

        FIELD_SET_SHARED_RECORD(detail, selected);
        name_index = detail->attributes.halves.high & 0x3F;
        if (kind == 0)
        {
            value = detail->stats.values[0];
        }
        else if (kind == 1)
        {
            value = detail->stats.values[0];
            value += detail->stats.values[1];
            value += detail->stats.values[2];
            value += detail->stats.values[3];
        }
        else
        {
            value = detail->stats.bytes[2];
        }

        FIELD_SET_SHARED_RECORD(shown, selected);
        low_word = shown->nibbles[0];
        high_word = shown->nibbles[1];
        nibble_sum = (low_word & 0xF) + ((low_word >> 4) & 0xF) + ((low_word >> 8) & 0xF) + ((low_word >> 12) & 0xF) + ((low_word >> 16) & 0xF) +
                     ((low_word >> 20) & 0xF) + ((low_word >> 24) & 0xF) + (low_word >> 28) + (high_word & 0xF) + ((high_word >> 4) & 0xF) +
                     ((high_word >> 8) & 0xF) + ((high_word >> 12) & 0xF) + ((high_word >> 16) & 0xF) + ((high_word >> 20) & 0xF) + ((high_word >> 24) & 0xF) +
                     (high_word >> 28);
        special = nibble_sum >= 0x29;
        if (kind == 2)
        {
            special = shown->stats.bytes[0];
        }

        amount = shown->unknown_0x34;
        func_800B2844(0, &shown->active, 0xFF);
        info = FIELD_MENU_ITEM_INFO;
        info->kind = kind;
        info->category = category;
        func_800B2844(1, FIELD_MENU_TEXT(name_index), 0xFF);
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
    func_800B2844(0, &D_80122A08[selected].active, 0xFF);
}

/**
 * @brief Drop spent pending item records and compact the four-entry pending table.
 *
 * An active pending record with no pending result is cleared. Each free slot
 * without a result then takes the next active record after it.
 */
void func_800C8E2C(void)
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
                    field_copy_inventory_record(&FIELD_MENU_ITEMS->pending[i], &FIELD_MENU_ITEMS->pending[j]);
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
void func_800C8F4C(void)
{
    s32 selected;
    u8* free_record;

    selected = D_80122C02;
    free_record = field_find_free_inventory_record();
    field_copy_inventory_record(free_record, &FIELD_MENU_ITEMS->pending[selected]);
    FIELD_MENU_ITEMS->pending[selected].active = 0;
    FIELD_MENU_ITEMS->pending[selected].unknown_0x34 = 0;
    func_800C8E2C();
}

/**
 * @brief Restore the first set-aside pending item record and publish its pending result.
 *
 * The record's index goes to D_80122C02 (0xFF when there is none) and its
 * result to the word at D_80122C08. Without a result the table is compacted.
 */
void func_800C8FA8(void)
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
            func_800B2844(0, &FIELD_MENU_ITEMS->pending[i].active, 0xFF);
            result = FIELD_MENU_ITEMS->pending[i].unknown_0x34;
            FIELD_MENU_ITEMS->pending[i].unknown_0x34 = 0;
            D_80122C02 = i;
            break;
        }
    }

    D_80122C00.words[2] = result;
    if (result == 0)
    {
        func_800C8E2C();
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
void func_800C905C(void)
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
    u8* count_var;

    selected = D_80122C02;
    count = 0;
    for (i = 0; i < 4; i++)
    {
        if (FIELD_MENU_ITEMS->pending[i].active != 0)
        {
            count++;
        }
    }

    /* The status byte D_80122C03 is reached from D_80122C06, as the original code does. */
    count_var = (u8*)&D_80122C06;
    count_var[0] = count;
    count_var[-3] = 0;
    if (FIELD_MENU_ITEMS->pending[selected].active == 0)
    {
        count_var[-3] = 1;
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
        func_800B2844(0, &FIELD_MENU_ITEMS->pending[selected].active, 0xFF);
        info = FIELD_MENU_ITEM_INFO;
        info->kind = kind;
        info->category = category;
        func_800B2844(1, FIELD_MENU_TEXT(name_index), 0xFF);
        return;
    }

    func_800B2844(0, (selected << 6) + D_80046138, 0xFF);
    D_80122C03 = 2;
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
void func_800C92B8(void)
{
    s32 index;

    index = (&D_80122C19)[D_80122C19 + 4];
    if (index < SMALL_HISTORY_RECORD_COUNT)
    {
        FIELD_PAD_CTX->small_history_records[index].selection_flags.selection_restricted = 0;
    }
    else
    {
        record_game_diagnostic(0x8002, 0x4B, index, 0);
    }
}

/**
 * @brief Store an entry id in the selected small history record and lift its restriction.
 *
 * An unnamed record gets the name 'A' (0x41). An index of five or more is
 * reported as a diagnostic.
 */
void func_800C9330(void)
{
    s32 index;
    u8 entry_id;

    entry_id = D_80122C11;
    index = (&D_80122C11)[1];
    if (index < SMALL_HISTORY_RECORD_COUNT)
    {
        FIELD_MENU_HISTORY->small_history_records[index].entry_id = entry_id;
        FIELD_PAD_CTX->small_history_records[index].selection_flags.selection_blocked = 0;
        if (FIELD_PAD_CTX->small_history_records[index].name[0] == 0)
        {
            FIELD_PAD_CTX->small_history_records[index].name[0] = 0x41;
        }
    }
    else
    {
        record_game_diagnostic(0x8002, 0x4C, index, 0);
    }
}

/**
 * @brief Checks field state 0x2F08 and performs the corresponding update.
 */
void func_800C93B4(void)
{
    if (func_800BD414(0, 0x2F08) == 0x80)
    {
        func_800AD194(1);
    }
    else if (func_800BD414(0, 0x2F08) == 0xFF)
    {
        func_800AD194(0);
    }
}

/** @brief Dispatch the nonempty menu record at buffer offset 0x840. */
void func_800C9404(void)
{
    D_80122C00.bytes[0] = g_saved_game.bytes[0x840];
    if (D_80122C00.bytes[0] != 0)
    {
        func_800B2844(0, &g_saved_game.bytes[0x840], 0xFF);
    }
}

/** @brief Count the free pending item records into D_80122C1F. */
void func_800C9448(void)
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
void func_800C9488(void)
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
void func_800C94C8(void)
{
    D_80122A08[0].active = 0;
    D_80122A08[1].active = 0;
    D_80122A08[2].active = 0;
    D_80122A08[3].active = 0;
    D_80122A08[0].unknown_0x34 = 0;
    D_80122A08[1].unknown_0x34 = 0;
    D_80122A08[2].unknown_0x34 = 0;
    D_80122A08[3].unknown_0x34 = 0;
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
            if (D_80122A08[i].active == 0)
            {
                entry = &(&D_80122C19)[i];
                if (D_80122A08[i].unknown_0x34 != 0 && *entry != 0xFA)
                {
                    D_80122A08[i].active = D_80122A08[i].saved_active;
                    D_80122A08[i].saved_active = *entry;
                    if (field_find_free_inventory_record() != 0)
                    {
                        field_copy_inventory_record(field_find_free_inventory_record(), &D_80122A08[i]);
                    }
                }
            }
        }
    }
    if (mode == 0 && D_8011F428 == 1)
    {
        for (i = 0; i < 4; i++)
        {
            if (D_80122A08[i].active != 0)
            {
                result = D_80122A08[i].unknown_0x34;
                if (result == 0)
                {
                    result = 1;
                }
                D_80122A08[i].unknown_0x34 = result;
            }
        }
    }
    D_80122C1E = (u8)D_8011F428;
}

/** @brief Set field state 0x2F08 from the pending mode flag. */
void func_800C963C(void)
{
    if (D_80043818 != 0)
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
void func_800C9684(void)
{
    s32 index;
    FieldMenuItemRecord* record;
    u8 saved;
    u8 active;

    index = D_80122C02;
    saved = D_80122A08[index].saved_active;
    record = &D_80122A08[index];
    /* D_80122C19[index] and the D_80122C08 word, both reached from D_80122C02 as the original does. */
    (&D_80122C02)[index + 0x17] = saved;
    active = record->active;
    record->active = 0;
    record->saved_active = active;
    record->unknown_0x34 = *(u32*)((u8*)&D_80122C02 + 6);
}

/** @brief Choose one of eight entries using fourth-power weights and the current mode. */
void func_800C96C4(void)
{
    s32 weights[8];
    Choices choices;
    s32 total, i, offset, value, weight, draw;
    LayoutSelection selection;
    s32 *cursor, *base;
    s32 track, byte_offset, random_product;
    s32 mode;
    total = 0;
    i = total;
    selection.layout = (AttributeLayout*)g_saved_game.bytes;
    mode = D_80122C00.bytes[0];
    track = g_music_track_index;
    choices = D_80051ED8;
    offset = track * 12;
loop_weights:
    value = ((u8*)selection.layout)[i + offset + 0x2F4];
    weight = value - 3;
    if (i == choices.entries[selection.layout->selected & 0x7F])
    {
        weight = value - 2;
    }
    if (weight >= 0)
    {
        value = 3;
        if (weight < 4)
        {
            value = weight;
        }
    }
    else
    {
        value = 0;
    }
    weight = value * value;
    weight = weight * weight;
    byte_offset = i * 4;
    i++;
    base = weights;
    *(s32*)((u8*)base + byte_offset) = weight;
    total += weight;
    if (i < 8)
    {
        goto loop_weights;
    }
    if (mode == 1)
    {
        total = 0x288;
    }
    random_product = rand() * total;
    draw = random_product >> 15;
    if (random_product < 0)
    {
        draw = (random_product + 0x7FFF) >> 15;
    }
    total = 0;
    selection.selected = 0xFF;
    i = total;
    cursor = base;
loop:
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
        total += 0x51;
    }
    i++;
    cursor++;
    if (i < 8)
    {
        goto loop;
    }
finish:
    if (mode != 0)
    {
        goto store_mode_one;
    }
    D_80122C05 = selection.selected;
    goto done;
found:
    selection.selected = i;
    goto finish;
store_mode_one:
    (*(u8*)&D_80122C06) = selection.selected;
done:
    return;
}

/** @brief Open the attribute selection screen sequence. */
void func_800C9894(void)
{
    FieldGosubSequence local;

    local = D_80051EF8;
    field_open_gosub_screen_sequence(&local);
}

/**
 * @brief Pack the current gosub result's color/attribute bytes into D_80122C01.
 *
 * If there are gosub results, reads the selected result's 0x40-byte layout record
 * (at @c g_saved_game.bytes + 0xCE0) for its 0x24 and 0x26 fields and a 6-bit
 * attribute from the 0xCF4 word; otherwise defaults to 0xFF. The 0x26 field is
 * clamped to 0..0x63 and all four bytes are written to @c D_80122C01.
 *
 * @note gcc280_g0, 100% match.
 */
void func_800C98D4(void)
{
    s32 result_value;
    s32 attr;
    s32 flags;
    s32 clamp_src;
    s32 out;
    RecC98D4* rec;

    result_value = 0xFF;
    attr = 0xFF;
    flags = 0xFF;
    clamp_src = 0;
    if (g_gosub_result_count != 0)
    {
        u8* buf = g_saved_game.bytes;
        u8* base;
        u8* recbase;
        result_value = (*(s32*)&g_gosub_result_values);
        base = buf + result_value * 0x40;
        recbase = buf + 0xCE0;
        rec = (RecC98D4*)(recbase + result_value * 0x40);
        flags = rec->unk24;
        attr = (*(u32*)(base + 0xCF4) >> 10) & 0x3F;
        clamp_src = rec->unk26;
    }
    if (clamp_src >= 0)
    {
        out = 0x63;
        if (clamp_src < 0x64)
        {
            out = clamp_src;
        }
    }
    else
    {
        out = 0;
    }
    ((OutC98D4*)&D_80122C01)->unk0 = result_value;
    ((OutC98D4*)&D_80122C01)->unk1 = attr;
    ((OutC98D4*)&D_80122C01)->unk2 = flags;
    ((OutC98D4*)&D_80122C01)->unk3 = out;
}

/**
 * @brief Apply packed category and element multipliers to the two pending values.
 *
 * Both results remain signed 32-bit values until clamped to 0..32767.
 */
void func_800C9960(void)
{
    Lookup lookup;
    u8* base;
    s32 packed;
    s32 first;
    s32 second;
    s32 element;
    s32 amount;
    s32 level;
    s32 primary;
    s32 secondary;
    s32 third;
    u32 fourth;
    s32 pair;
    s32 low;
    u32 high;
    s32 selected;
    s32 mode;
    s32 factor1;
    s32 factor2;
    s32 element1;
    s32 element2;
    s32 product1, product2, scaled1, scaled2;
    s32 clamp;
    s32 result1;
    s32 result2;
    lookup = D_80051F04;
    factor1 = 10;
    do
    {
        factor1 = factor1;
    } while (0);
    base = &(*(u8*)&D_80122C06);
    packed = base[0];
    first = *(s16*)(base + 10);
    second = *(s16*)(base + 12);
    element = base[-3];
    amount = base[-2];
    level = base[2];
    second++;
    second--;
    primary = packed & 3;
    secondary = (packed >> 2) & 3;
    third = (packed >> 4) & 3;
    fourth = (u8)packed >> 6;
    packed = base[-1];
    pair = packed;
    low = pair & 15;
    high = (u8)pair >> 4;
    packed = base[1];
    selected = packed;
    mode = base[4];
    if (selected != primary)
    {
        factor1 = 2;
        if (selected == secondary)
        {
            factor1 = 1;
        }
    }
    factor2 = 10;
    if (selected != third)
    {
        factor2 = 2;
        if (selected == fourth)
        {
            factor2 = 1;
        }
    }
    if (mode == 0)
    {
        if (element == low)
        {
            element1 = 3;
        }
        else if (element == lookup.values[low])
        {
            element1 = 1;
        }
        else
        {
            element1 = 2;
        }
        if (element == high)
        {
            element2 = 3;
        }
        else if (element == lookup.values[high])
        {
            element2 = 1;
        }
        else
        {
            element2 = 2;
        }
    }
    else
    {
        element1 = 2;
        element2 = element1;
    }
    first /= level + 6;
    second /= level + 6;
    product1 = factor1 * element1;
    product2 = factor2 * element2;
    scaled1 = product1 * amount;
    scaled2 = product2 * amount;
    first += scaled1;
    second += scaled2;
    first *= level + 7;
    second *= level + 7;
    if (first >= 0)
    {
        result1 = 0x7FFF;
        clamp = result1;
        clamp = clamp < first;
        if (!clamp)
        {
            result1 = first;
        }
    }
    else
    {
        result1 = 0;
    }
    first = result1;
    if (second >= 0)
    {
        result2 = 0x7FFF;
        if (factor1)
        {
            clamp = result2;
        }
        else
        {
            clamp = (result2 | 0x10000) & 0x7FFF;
        }
        clamp = clamp < second;
        if (!clamp)
        {
            result2 = second;
        }
    }
    else
    {
        result2 = 0;
    }
    ((s16*)&D_80122C10)[0] = first;
    ((s16*)&D_80122C10)[1] = result2;
}

/**
 * @brief Dispatch the current menu record for the active cursor slot.
 */
void func_800C9BC4(void)
{
    s32 temp_s1;
    s32 temp_s0;
    s32 off;

    temp_s1 = D_80122C00.bytes[0];
    if (field_find_free_inventory_record() != 0)
    {
        temp_s0 = field_find_free_inventory_record();
        field_copy_inventory_record(temp_s0, func_800C1E40(5) + (off = (temp_s1 << 6) + 4));
        func_800B2844(0, func_800C1E40(5) + off, 0xFF);
    }
}

/**
 * @brief Latch the current menu selection as the pending gosub result.
 *
 * Reads the active menu record's selection index from @c g_saved_game.bytes; if
 * it is in range (< 5), records it into the @c D_80122C1C cursor slot and the
 * pending-result globals, sets the record's 0x40000000 flag, and dispatches
 * func_800B2844 for it.
 *
 */
void func_800C9C3C(void)
{
    u8* dbase;
    u8 d0;
    u8* mlb;
    u8* ptr;
    u8* dp1;
    u8* argp;
    s32 val;
    s32 sel;
    s32 off;
    u8* rec;

    func_800C57D4();
    dbase = &(*(u8*)&D_80122C1C);
    d0 = (*(u8*)&D_80122C1C);
    dp1 = dbase + 1;
    ptr = d0 + dp1;
    val = *ptr;
    g_gosub_result_count = 1;
    mlb = g_saved_game.bytes;
    sel = *(s32*)(mlb + 0x2EF0);
    (*(s32*)&g_gosub_result_values) = val;
    if (sel < 5)
    {
        *ptr = (u8)sel;
        off = sel * 0x60;
        rec = off + mlb;
        *(s16*)(dbase - 8) = *(u8*)(rec + 0x2F09);
        *(s32*)(rec + 0x2F38) = *(s32*)(rec + 0x2F38) | 0x40000000;
        argp = mlb + 0x2EF4;
        func_800B2844(d0, off + argp, 0xFF);
    }
}

/**
 * @brief Re-select the cursor slot's menu record and dispatch it.
 */
void func_800C9CE4(void)
{
    u8* base;
    u8* rec;
    s32 idx;

    idx = ((u8*)&(*(u8*)&D_80122C1C))[(*(u8*)&D_80122C1C) + 1] * 0x60;
    base = g_saved_game.bytes;
    rec = idx + base;
    (*(u8*)&D_80122C1C) = rec[0x2F3C];
    base = base + 0x2EF4;
    func_800B2844(3, idx + base, 0xFF);
}

/**
 * @brief Count the active menu records and store the total in D_80122C16.
 */
void func_800C9D44(void)
{
    s32 i;
    s32 count;
    u8* p;

    count = 0;
    for (i = 0; i < 5; i++)
    {
        p = &g_saved_game.bytes[i * 0x60];
        if (p[0x2EF4] != 0)
        {
            count++;
        }
    }
    D_80122C16 = (u16)count;
}

/**
 * @brief Forward D_80122C01 to func_800AD030 after the common menu prologue.
 */
void func_800C9D84(void)
{
    func_800C57D4();
    func_800AD030(D_80122C01);
}

/**
 * @brief Store the high nibble of D_800459AC into D_80122C12.
 */
void func_800C9DB4(void)
{
    D_80122C12 = (s8)((u8)D_800459AC >> 4);
}

/**
 * @brief Dispatch the active menu record and entries referenced by resource tables 0x103 and 0x104.
 */
void func_800C9DCC(void)
{
    FieldMenuRecordC9DCC* record;
    FieldMenuRecordC9DCC* record_base;
    s32 record_index;
    s32 unk24_value;
    s32 unk25_value;
    s32 lookup_index;
    s32 resource_index_103;
    s32 resource_index_104;
    u8* resource_103;
    u8* resource_104;
    s32 resource_offset;

    record_index = D_80122C10;
    record_base = ((FieldMenuRecordC9DCC*)&D_80043CB8);
    record = record_base + record_index;
    unk24_value = record->unk24;
    unk25_value = record->unk25;
    lookup_index = (unk24_value * 0xE) + unk25_value;
    func_800B2844(0, (u8*)record, 0xFF);

    resource_103 = func_800C1E40(0x103);
    resource_index_103 = lookup_index * 2;
    resource_offset = ((FieldResourceOffsetByte*)(resource_103 + resource_index_103))->value +
                      (((FieldResourceOffsetByte*)(func_800C1E40(0x103) + (resource_index_103 += 1)))->value << 8);
    func_800B2844(1, func_800C1E40(0x103) + (resource_offset + 4), 0xFF);

    resource_104 = func_800C1E40(0x104);
    resource_index_104 = unk25_value * 2;
    resource_offset = ((FieldResourceOffsetByte*)(resource_104 + resource_index_104))->value +
                      (((FieldResourceOffsetByte*)(func_800C1E40(0x104) + (resource_index_104 += 1)))->value << 8);
    func_800B2844(2, func_800C1E40(0x104) + (resource_offset + 4), 0xFF);
}
