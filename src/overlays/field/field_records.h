#ifndef FIELD_RECORDS_H
#define FIELD_RECORDS_H

/**
 * @file field_records.h
 * @brief FIELD-private layouts of the game-state records, the field runtime
 *        context and the field battle context.
 *
 * Three FIELD pointers reach these blocks:
 * - g_field_game_state points at g_saved_game (bound by func_800B0BDC) and is read
 *   through FieldGameState.
 * - g_field_runtime points at the field runtime context (D_80122C00):
 *   FieldRuntimeContext, with its actor and event records.
 * - g_field_battle points at the field battle context (D_80123B08):
 *   FieldBattleContext, with its FieldStatusRecord array.
 */

#include "common.h"
#include "field_state_ops.h"

#define FIELD_PARTY_SIZE 3
#define FIELD_LAND_COUNT 64
#define FIELD_ITEM_COUNT 100
#define FIELD_EQUIPMENT_SLOT_COUNT 4
/** @brief Equipment slot holding the character's weapon. */
#define FIELD_WEAPON_SLOT 0
#define FIELD_ITEM_SPECIAL_COUNT 4
#define FIELD_CHARACTER_STAT_COUNT 8
#define FIELD_REGION_COUNT 5

/** @brief Character info bits 0-6: character type. */
#define FIELD_CHARACTER_TYPE_MASK 0x7F

/** @brief Character type of a guest in party slot 1. */
#define FIELD_CHARACTER_GUEST 2

/** @brief Character type of a stored companion in party slot 2. */
#define FIELD_CHARACTER_COMPANION 3

/** @brief Character info bit 7: the character is AI-controlled. */
#define FIELD_CHARACTER_AI 0x80

#define FIELD_ACTOR_RECORD_COUNT 16
#define FIELD_EVENT_RECORD_COUNT 2
#define FIELD_ACTOR_SCRIPT_COUNT 16
#define FIELD_SCRIPT_FRAME_COUNT 8

#define FIELD_BATTLE_RECORD_COUNT 11

/** @brief Value written to an actor's pending-event byte when no event is queued. */
#define FIELD_NO_EVENT 0xFF

/** @brief Script id stored in an actor's event table for an unused event. */
#define FIELD_NO_SCRIPT 0xFFFF

/* ------------------------------------------------------------------------ */
/* Game state (g_field_game_state)                                                  */
/* ------------------------------------------------------------------------ */

/** @brief Item category: bits 8-9 of FieldItemRecord.info. */
#define FIELD_ITEM_CATEGORY(info) (((info) >> 8) & 3)

/** @brief Item type within its category: bits 10-15 of FieldItemRecord.info. */
#define FIELD_ITEM_TYPE(info) (((info) >> 10) & 0x3F)

/** @brief FIELD_ITEM_CATEGORY values. */
enum
{
    FIELD_ITEM_CATEGORY_WEAPON = 0,
    FIELD_ITEM_CATEGORY_ARMOR = 1,
    FIELD_ITEM_CATEGORY_ACCESSORY = 2
};

/** @brief Eight four-bit stat modifiers, one per character stat; indexes into D_800F0C38. */
typedef struct FieldItemModifiers
{
    u32 stat0 : 4;
    u32 stat1 : 4;
    u32 stat2 : 4;
    u32 stat3 : 4;
    u32 stat4 : 4;
    u32 stat5 : 4;
    u32 stat6 : 4;
    u32 stat7 : 4;
} FieldItemModifiers;

/**
 * @brief Item record: one inventory entry or one equipped item (0x40 bytes).
 * @note The same layout is used by the 100-entry inventory and by the four
 *       equipment slots of each party character.
 */
/** @brief Eight four-bit values packed into one word. */
typedef struct FieldNibbles
{
    unsigned n0 : 4;
    unsigned n1 : 4;
    unsigned n2 : 4;
    unsigned n3 : 4;
    unsigned n4 : 4;
    unsigned n5 : 4;
    unsigned n6 : 4;
    unsigned n7 : 4;
} FieldNibbles;

/** @brief Item identity word of a FieldItemRecord. */
typedef struct FieldItemInfo
{
    unsigned unk0 : 8;
    unsigned category : 2;
    unsigned item_type : 6;
    unsigned item_subtype : 6;
    unsigned unk22 : 10;
} FieldItemInfo;

typedef struct FieldItemRecord
{
    u8 kind;
    u8 pad01[0x13];
    /** @brief Bits 8-9 category, 10-15 item type, 16-21 item subtype. */
    union
    {
        u32 word;
        FieldItemInfo bits;
    } info;
    /** @brief Eight four-bit bonus values. */
    union
    {
        u32 word;
        FieldNibbles bits;
    } bonus_nibbles;
    /** @brief Eight four-bit stat modifiers, indexes into D_800F0C38. */
    union
    {
        u32 word;
        FieldNibbles bits;
    } stat_nibbles;
    u8 special_ids[FIELD_ITEM_SPECIAL_COUNT];
    /** @brief Category-dependent derived values of an equipped item. */
    union
    {
        u8 bytes[8];
        u16 values[4];
        s16 signed_values[4];
        struct
        {
            u16 power;
            u8 stats[6];
        } weapon;
    } derived;
    u8 flags2C;
    u8 flags2D;
    u16 effect_index;
    u8 attributes[4];
    s32 handle;
    s32 unk38;
    s32 unk3C;
} FieldItemRecord;

/**
 * @brief Party character record in the game state (0x250 bytes).
 * @note A character slot is in use when the first byte of its name is nonzero.
 */
typedef struct FieldCharacterRecord
{
    u8 name[24];
    /** @brief Byte 0: bits 0-6 character type, bit 7 set while the slot is AI-controlled. */
    union
    {
        u32 word;
        u8 bytes[8];
        struct
        {
            u8 type : 7;
            /** @brief Same bit as FIELD_CHARACTER_AI; set members get pad control (mode 0) in battle. */
            u8 pad_controlled : 1;
        } bits;
        struct
        {
            u8 type;
            u8 unk19;
            /** @brief Commands of battle actions 0 and 1. */
            u8 commands[2];
            /** @brief Battle actions 4 to 7: a weapon-type skill, or 0x80 plus an item slot. */
            u8 skills[4];
        } actions;
    } info;
    /** @brief Low byte: level; bits 8-31: experience. */
    union
    {
        u32 word;
        u8 level;
        struct
        {
            unsigned level : 8;
            unsigned experience : 24;
        } bits;
    } progress;
    u16 hp;
    /** @brief Copy of unk24.values[0] of the weapon slot. */
    u16 unk26;
    /** @brief Sums of unk24.values[] over the equipped armor slots. */
    u16 equipment_totals[4];
    /** @brief Bits 0-8: base value in quarter units, bits 9-15: effective value with equipment. */
    u16 stats[FIELD_CHARACTER_STAT_COUNT];
    u8 unk40;
    u8 unk41;
    u8 unk42;
    u8 unk43;
    u8 pad44[4];
    /** @brief Identity permutation (0 to 7) rebuilt when a companion joins. */
    u8 unk48[8];
    FieldItemRecord equipment[FIELD_EQUIPMENT_SLOT_COUNT];
    /** @brief Four further item records; the hero's are scanned with the equipment as one run of eight. */
    FieldItemRecord unk150[4];
} FieldCharacterRecord;

/** @brief Per-land record (0xC bytes). */
typedef struct FieldLandRecord
{
    u8 flags;
    /** @brief Map grid position: low nibble X, high nibble Z. */
    u8 position;
    u8 unk2;
    u8 count;
    u8 levels[8];
} FieldLandRecord;

/** @brief One menu action slot (0x10 bytes). */
typedef struct FieldMenuSlot
{
    s32 handle;
    /** @brief Byte 0: entry index; bits 8-9: result type. */
    union
    {
        u32 word;
        u8 index;
    } entry;
    u8 pad8[8];
} FieldMenuSlot;

/** @brief Menu action slot group (0x8C bytes): a header, then eight slots. */
typedef struct FieldMenuSlotGroup
{
    /** @brief Bits 12-15: number of valid entries in @c selectors. */
    u32 flags;
    u8 pad4[4];
    /** @brief Effect-table rows (0x58 and up) combined into a slot effect; 0xFF when unused. */
    u8 selectors[4];
    FieldMenuSlot slots[8];
} FieldMenuSlotGroup;

/**
 * @brief Stored companion record (0x60 bytes).
 * @note The active one is copied into characters[2] (character type 3) and
 *       written back when it leaves; region_index names it (5 when none).
 */
typedef struct FieldRegionRecord
{
    /** @brief Display name; the record is in use when the first byte is nonzero. */
    u8 name[0x14];
    u8 unk14;
    /** @brief Copied to the party record's info byte 1. */
    u8 unk15;
    u8 pad16[2];
    /** @brief Low byte: level; bits 8-31: experience. */
    union
    {
        u32 word;
        u8 level;
        struct
        {
            unsigned level : 8;
            unsigned experience : 24;
        } bits;
    } progress;
    u16 hp;
    u16 unk1E;
    u16 equipment_totals[4];
    /** @brief Bits 0-8: stat value (times four), bits 9-15: growth. */
    u16 stats[FIELD_CHARACTER_STAT_COUNT];
    u8 unk38[4];
    /** @brief Resource 0xD index of equipment[0]. */
    u8 weapon_id;
    /** @brief Resource 0xE indexes of equipment[1] to equipment[3]. */
    u8 armor_ids[3];
    u8 pad40[2];
    u16 unk42;
    /**
     * @brief Bytes 0-2: pending effect ids (0xFF when empty); bits 24-26: effects
     *        already applied; bit 30: gains experience; bit 31: newly joined.
     */
    union
    {
        u32 word;
        u8 effects[4];
    } status;
    /** @brief Low byte: flag bits set or cleared by effects. */
    union
    {
        s32 word;
        u8 flags;
    } unk48;
    /** @brief Per-stat growth: high nibble applied per level, low nibble base rate. */
    u8 stat_growth[FIELD_CHARACTER_STAT_COUNT];
    /** @brief Low nibble: rate; high nibble: accumulator whose bit 7 carries into equipment_totals. */
    u8 total_growth[4];
    /** @brief Byte 0 like total_growth for unk1E; byte 1 added to halfword 1 per level. */
    union
    {
        u32 word;
        u8 bytes[4];
        u16 halves[2];
    } extra_growth;
    /** @brief Random identifier, unique among the stored records. */
    s32 unique_id;
} FieldRegionRecord;

/**
 * @brief Layout of g_saved_game as FIELD reads it through g_field_game_state.
 */
typedef struct FieldGameState
{
    u8 pad000[0x2C];
    /** @brief Money, saturated at 10,000,000. */
    u32 money;
    u8 pad030[0xD4 - 0x30];
    u16 unkD4;
    u16 unkD6;
    u16 unkD8;
    u8 padDA[0xE4 - 0xDA];
    s32 words[0x80];
    union
    {
        u32 word;
        struct
        {
            u8 unk2E4;
            u8 hero_level;
            u16 unk2E6;
        } fields;
    } control;
    u32 flag_bits[2];
    FieldLandRecord lands[FIELD_LAND_COUNT];
    FieldCharacterRecord characters[FIELD_PARTY_SIZE];
    FieldItemRecord items[FIELD_ITEM_COUNT];
    u8 counters[0x26E4 - 0x25E0];
    FieldMenuSlotGroup menu_slots[2];
    u8 pad27FC[0x29D4 - 0x27FC];
    u8 default_group;
    u8 pad29D5[0x2A7C - 0x29D5];
    /** @brief Three-value script results chosen by func_800B32FC. */
    u8 result_rows[36][4];
    u8 pad2B0C[0x2EF0 - 0x2B0C];
    s32 region_index;
    FieldRegionRecord regions[FIELD_REGION_COUNT];
    u32 resource_bits[1];
} FieldGameState;

/* ------------------------------------------------------------------------ */
/* Item record staging (D_80123FC4) and its generation tables (D_80123FC0)  */
/* ------------------------------------------------------------------------ */

#define FIELD_STAGING_LEVEL_COUNT 8
#define FIELD_STAGING_STAT_COUNT 8
#define FIELD_STAGING_SLOT_COUNT 6
#define FIELD_STAGING_PROPERTY_COUNT 6

/** @brief Value of an unused FieldItemStaging slot byte. */
#define FIELD_STAGING_SLOT_EMPTY 0xFF

/** @brief Largest four-bit level a staged level entry can reach. */
#define FIELD_STAGING_LEVEL_MAX 0xF

/** @brief Bias subtracted from FieldItemStaging::command_index before indexing a table. */
#define FIELD_STAGING_COMMAND_BASE 0x40

/** @brief One staged level: its base cost and current four-bit level. */
typedef struct FieldStagingLevel
{
    u8 cost;
    u8 level;
} FieldStagingLevel;

/** @brief Four staged stats: a modifier index and a bounds row each. */
typedef struct FieldStagingStats
{
    unsigned modifier0 : 4;
    unsigned bounds0 : 4;
    unsigned modifier1 : 4;
    unsigned bounds1 : 4;
    unsigned modifier2 : 4;
    unsigned bounds2 : 4;
    unsigned modifier3 : 4;
    unsigned bounds3 : 4;
} FieldStagingStats;

/**
 * @brief Staging block (0x60 bytes) used while an item record is generated.
 * @note Cleared by func_800BE710, filled from the generation tables, then
 *       written back into @c record by func_800BFA34.
 */
typedef struct FieldItemStaging
{
    FieldItemRecord* record;
    /** @brief Item category (FieldItemRecord::info bits 8-9). */
    u8 category;
    /** @brief Item type (FieldItemRecord::info bits 10-15). */
    u8 item_type;
    /** @brief Item subtype (FieldItemRecord::info bits 16-21). */
    u8 item_subtype;
    /** @brief Command selector, biased by FIELD_STAGING_COMMAND_BASE. */
    u8 command_index;
    /** @brief Pool that pays for level increases. */
    s32 pool;
    FieldStagingLevel levels[FIELD_STAGING_LEVEL_COUNT];
    u8 effect_index;
    u8 pad1D[3];
    /** @brief Per stat, low nibble: modifier index; high nibble: row of the D_800F0E88 bounds. */
    union
    {
        u8 bytes[FIELD_STAGING_STAT_COUNT];
        FieldStagingStats words[2];
    } stats;
    /** @brief Slot values, FIELD_STAGING_SLOT_EMPTY when unused. */
    u8 slots[FIELD_STAGING_SLOT_COUNT];
    u8 properties[FIELD_STAGING_PROPERTY_COUNT];
    u8 flags2C;
    u8 flags2D;
    u8 alternate_flags2C;
    u8 pad37[3];
    u16 unk3A;
    u8 weights[4];
    u8 multipliers[4];
    /** @brief Level increases still to be applied, per level entry. */
    u8 pending_levels[FIELD_STAGING_LEVEL_COUNT];
    /** @brief Levels whose nonzero value sets the matching bit of flags2C. */
    u8 flags2C_mask;
    /** @brief Bits copied into flags2D. */
    u8 flags2D_mask;
    u8 pad4E[2];
    /** @brief Stat modifier index competing with the low nibble of each stats byte. */
    u8 base_stats[FIELD_STAGING_STAT_COUNT];
    /** @brief Staging flags; bits 30 and 31 decide which slot values stay in slot 4. */
    union
    {
        s32 word;
        struct
        {
            /** @brief Last slot value below 0x10, or 0xF when there is none. */
            unsigned slot_class : 4;
            unsigned reserved : 26;
            /** @brief Set to keep a slot value in 0x3E-0x4B. */
            unsigned keep_low_slot : 1;
            /** @brief Clear to keep a slot value in 0x51-0x57. */
            unsigned release_high_slot : 1;
        } bits;
    } flags;
    u8 pad5C[4];
} FieldItemStaging;

/**
 * @brief View of a staging block shifted by @p bytes bytes.
 * @note Element 0 of a byte array member of the view is element @p bytes of
 *       the real block. The original code reaches several parallel arrays
 *       from one shifted base like this; indexing each array separately does
 *       not produce the same code.
 */
#define FIELD_STAGING_AT(staging, bytes) ((FieldItemStaging*)((u8*)(staging) + (bytes)))

/** @brief Per-type entry of FieldItemTable (0xC bytes). */
typedef struct FieldItemTypeEntry
{
    u16 scripts[2];
    u8 weights[4];
    u8 factors[4];
} FieldItemTypeEntry;

/** @brief Per-subtype entry of FieldItemTable (0x14 bytes). */
typedef struct FieldItemSubtypeEntry
{
    u16 script;
    u16 divisor;
    u8 weights[4];
    u8 multipliers[4];
    u8 costs[FIELD_STAGING_LEVEL_COUNT];
} FieldItemSubtypeEntry;

/** @brief Per-command entry of FieldItemTable (4 bytes). */
typedef struct FieldItemCommandEntry
{
    u8 pool_bonus;
    u8 pad1;
    u16 script;
} FieldItemCommandEntry;

/** @brief Per-slot-value entry of FieldItemTable: one script per slot position. */
typedef struct FieldItemSlotEntry
{
    u16 scripts[4];
} FieldItemSlotEntry;

/**
 * @brief Item generation table (func_800C1E40(4)) for categories 0 and 1.
 */
typedef struct FieldItemTable
{
    u32 header;
    FieldItemTypeEntry types[16];
    FieldItemTypeEntry alternate_types[16];
    FieldItemSubtypeEntry subtypes[64];
    FieldItemCommandEntry commands[192];
    FieldItemSlotEntry slot_values[160];
} FieldItemTable;

/**
 * @brief Item generation table (func_800C1E40(0xF)) for category 2.
 */
typedef struct FieldItemGridTable
{
    u32 header;
    u8 grid[8][8];
    u8 pairs[64][4][2];
    u16 commands[192];
} FieldItemGridTable;

/**
 * @brief Generation table currently loaded into D_80123FC0.
 * @note Script offsets passed to func_800BF2F0 are relative to @c bytes.
 */
typedef union FieldItemTables
{
    u8 bytes[1];
    FieldItemTable item;
    FieldItemGridTable grid;
} FieldItemTables;

/* ------------------------------------------------------------------------ */
/* Field runtime context (g_field_runtime)                                       */
/* ------------------------------------------------------------------------ */

/** @brief One nested script frame (0xC bytes). */
typedef struct FieldScriptFrame
{
    u8* pc;
    /** @brief Bit 0 holds the result of the last comparison. */
    u32 flags;
    /** @brief Remaining wait frames; bit 0 lets the frame resume after a return. */
    union
    {
        u32 word;
        struct
        {
            u32 resume : 1;
            u32 frames : 31;
        } bits;
    } wait;
} FieldScriptFrame;

/**
 * @brief Script execution state: owner, nesting depth and eight frames (0x68 bytes).
 * @note field_script_run takes a pointer to this block.
 */
typedef struct FieldScriptState
{
    union
    {
        u32 word;
        u8 owner_id;
        struct
        {
            u32 owner_id : 8;
            u32 unk8 : 1;
            u32 local_base : 7;
            u32 unk16 : 15;
            u32 running : 1;
        } bits;
    } status;
    s32 depth;
    FieldScriptFrame frames[FIELD_SCRIPT_FRAME_COUNT];
} FieldScriptState;

/**
 * @brief Actor or event record in the field runtime context (0x94 bytes).
 */
typedef struct FieldActorRecord
{
    u8 id;
    u8 selector;
    /** @brief Pickup handed over by func_800C1A18: bit 15 selects a counter, else a reward key. */
    u16 pickup;
    /** @brief Pending event, or FIELD_NO_EVENT. */
    u8 event;
    u8 event_argument;
    /** @brief Bit n set when scripts[n] may run. */
    u16 enabled_events;
    u16 scripts[FIELD_ACTOR_SCRIPT_COUNT];
    FieldScriptState script;
    union
    {
        s32 word;
        struct
        {
            unsigned trigger_group : 4;
            unsigned source : 10;
            unsigned reserved : 15;
            unsigned spawned : 1;
            unsigned script_only : 1;
            unsigned active : 1;
        } bits;
    } flags;
} FieldActorRecord;

/**
 * @brief Field runtime context (D_80122C00, reached through g_field_runtime).
 */
typedef struct FieldRuntimeContext
{
    s32 local_variable_base;
    u8 pad004[0x44 - 0x4];
    s32 actor_positions[FIELD_PARTY_SIZE];
    u8 pad050[4];
    s32 view_x;
    s32 view_z;
    u8 pad05C[0xB8 - 0x5C];
    s32 triggered_regions;
    s32 frame_count;
    u8 pad0C0[0x400 - 0xC0];
    union
    {
        u32 flags;
        u16 actor_count;
        struct
        {
            u8 low[3];
            u8 trigger_group;
        } bytes;
        struct
        {
            u32 actor_count : 16;
            u32 group_active : 1;
            u32 party_mode : 2;
            u32 talking : 1;
            u32 unk20 : 4;
            u32 trigger_group : 8;
        } bits;
    } state;
    s32 scene_entry;
    s32 scene_argument1;
    s32 scene_argument2;
    union
    {
        u32 word;
        struct
        {
            u32 red : 10;
            u32 green : 10;
            u32 blue : 10;
            u32 unk30 : 2;
        } bits;
    } fade_color;
    s32 fade_timer;
    union
    {
        u32 flags;
        struct
        {
            u16 scene_id;
            u8 unk41A;
            u8 unk41B;
        } fields;
    } transition;
    s32 unk41C;
    u8 pad420[0x430 - 0x420];
    FieldActorRecord actors[FIELD_ACTOR_RECORD_COUNT];
    FieldActorRecord events[FIELD_EVENT_RECORD_COUNT];
    FieldScriptState script;
    u8* trigger_table;
} FieldRuntimeContext;

/* ------------------------------------------------------------------------ */
/* Field battle context (g_field_battle)                                        */
/* ------------------------------------------------------------------------ */

/**
 * @brief Action request resolved by the field battle code.
 * @note Callers pass larger request blocks; only these words are read here.
 */
typedef struct FieldBattleAction
{
    s32 attacker_id;
    s32 action_id;
    /** @brief Nonzero when the target's counter also runs down. */
    s32 param;
    s32 target_id;
    s32 unk10;
    s32 unk14;
    /** @brief Multiplier applied to the damage by field_apply_damage. */
    s32 damage_scale;
} FieldBattleAction;

/**
 * @brief Handler parameters of an action descriptor, one view per handler.
 * @note Every view starts with the attacker stat (for field_compute_attack) and the
 *       target stat (for field_compute_defense).
 */
typedef union FieldActionParams
{
    u32 word;
    /** @brief Handlers 0 and 4: damage plus a status effect. */
    struct
    {
        u32 attack_stat : 4;
        u32 defense_stat : 4;
        u32 element_mask : 8;
        u32 chance : 4;
        u32 effect : 4;
        u32 duration : 8;
    } status;
    /** @brief Handlers 1, 3 and 6: a base plus a random spread. */
    struct
    {
        u32 attack_stat : 4;
        u32 defense_stat : 4;
        u32 element_mask : 8;
        u32 base : 8;
        u32 spread : 8;
    } roll;
    /** @brief Handler 2: damage plus stat changes on both sides. */
    struct
    {
        u32 attack_stat : 4;
        u32 defense_stat : 4;
        u32 element_mask : 8;
        u32 attacker_scale : 4;
        u32 attacker_stat : 4;
        u32 target_scale : 4;
        u32 target_stat : 4;
    } stat_change;
    /** @brief Handler 4: the status part only applies to a matching target. */
    struct
    {
        u32 attack_stat : 4;
        u32 defense_stat : 4;
        u32 match : 4;
        u32 doubled : 4;
        u32 chance : 4;
        u32 effect : 4;
        u32 duration : 8;
    } conditional;
    /** @brief Handler 5: damage plus a target record modification. */
    struct
    {
        u32 attack_stat : 4;
        u32 defense_stat : 4;
        u32 operation : 8;
        u32 value : 16;
    } modify;
} FieldActionParams;

/**
 * @brief Action descriptor selected by field_select_action_descriptor.
 */
typedef struct FieldActionDescriptor
{
    /**
     * @brief Bits 0-3: kind; bits 4-5: side rule; bits 6-7: defense slot;
     *        bits 8-10: status intensity shift; byte 2: power; byte 3: handler.
     */
    union
    {
        u32 word;
        struct
        {
            u8 flags;
            u8 unk1;
            u8 power;
            u8 handler;
        } bytes;
    } info;
    FieldActionParams params;
} FieldActionDescriptor;

/**
 * @brief Action descriptor bank (D_800EF8C0, reached through g_field_action_bank).
 * @note Holds four descriptor tables, located by byte offsets from the bank start.
 */
typedef struct FieldActionBank
{
    s32 table_offsets[4];
} FieldActionBank;

/**
 * @brief Field battle context (D_80123B08, reached through g_field_battle).
 * @note func_800B3580 clears 0x4A4 bytes and rebuilds the header.
 */
typedef struct FieldBattleContext
{
    /** @brief Bit 31 set while the battle is finished or suspended; the low byte holds the monster level. */
    union
    {
        s32 flags;
        u8 level;
    } state;
    FieldActorTemplateTable* templates;
    u8* resources;
    u8 element_levels[8];
    /** @brief Flags of the action being resolved; cleared by field_battle_bind_action. */
    union
    {
        u32 word;
        struct
        {
            /** @brief Set when the target guards; the damage then has no minimum of 1. */
            u32 guarded : 1;
            /** @brief func_800B302C result for the attacker and the target. */
            u32 side : 1;
            /** @brief Set when field_battle_check_target_stance marks a follow-up action. */
            u32 follow_up : 1;
            u32 unk3 : 29;
        } bits;
    } action_flags;
    FieldBattleAction* action;
    FieldActionDescriptor* descriptor;
    FieldStatusRecord* attacker;
    FieldStatusRecord* target;
    FieldStatusRecord records[FIELD_BATTLE_RECORD_COUNT];
    u16 power;
    u8 power_flags;
    u8 pad4A3;
} FieldBattleContext;

/** @brief FIELD diagnostic codes passed to record_game_diagnostic. */
#define DIAG_BAD_MONSTER_OBJECT 0x64 /**< A monster group member has no object state. */
#define DIAG_BAD_COMMAND 0x66        /**< Unknown command in a character's command slot. */
#define DIAG_BAD_MONSTER_ACTION 0x69 /**< Monster action id past the end of its template's list. */
#define DIAG_BAD_GUEST 0x6D          /**< No guest template for the requested guest. */
#define DIAG_BAD_COMPANION 0x6E      /**< The requested stored companion does not exist. */

/** @brief Script variables read or written by the battle code (see func_800BD414). */
/** @brief Script variable: number of party records still standing. */
#define FIELD_VAR_ALLY_COUNT 0x4280

/** @brief Script variable: number of monster records still standing. */
#define FIELD_VAR_ENEMY_COUNT 0x4284

/** @brief Script variable: battle result reported by field_battle_finish. */
#define FIELD_VAR_BATTLE_RESULT 0x4288

/** @brief Script variable: record id watched by field_battle_handle_defeat (-1 when none). */
#define FIELD_VAR_WATCHED_RECORD 0x428C

/** @brief Per-record script variable receiving the element mask of an action. */
#define FIELD_VAR_RECORD_ELEMENTS 0xD008

/** @brief Companion script variable; four times its value adds to the power of type 4 attackers. */
#define FIELD_VAR_COMPANION_POWER_BONUS 0xD038

/** @brief Debug flags: log damage, and spare the party or the monsters. */
#define FIELD_VAR_DEBUG_LOG_DAMAGE 0xFFC
#define FIELD_VAR_DEBUG_SPARE_PARTY 0xFFA
#define FIELD_VAR_DEBUG_SPARE_ENEMIES 0xFFB
/** @brief Debug flag: monsters start with 1 HP. */
#define FIELD_VAR_DEBUG_ONE_HP_MONSTERS 0xFFE
/** @brief Script variable: when non-zero, the level every monster uses. */
#define FIELD_VAR_MONSTER_LEVEL 0x2F78
/** @brief Script variable: difficulty; 1 adds 20 monster levels and doubles monster HP, 2 uses the top level row and triples it. */
#define FIELD_VAR_DIFFICULTY 0x2938
/** @brief Script variable: bit 7 bases every monster level on the hero's level. */
#define FIELD_VAR_LEVEL_FLAGS 0x52F0
#define FIELD_LEVEL_FLAG_HERO 0x80
/** @brief Script variables: lowest and highest monster level. */
#define FIELD_VAR_MONSTER_LEVEL_MIN 0x52E0
#define FIELD_VAR_MONSTER_LEVEL_MAX 0x52E8

#endif
