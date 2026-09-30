#ifndef FIELD_RECORDS_H
#define FIELD_RECORDS_H

/**
 * @file field_records.h
 * @brief FIELD-private layouts of the game-state records, the field runtime
 *        context and the field battle context.
 *
 * Three FIELD pointers reach these blocks:
 * - g_field_game_state points at g_saved_game (bound by field_bind_runtime_pointers) and is read
 *   through SavedGameLayout.
 * - g_field_runtime points at the field runtime context (D_80122C00):
 *   FieldRuntimeContext, with its actor and event records.
 * - g_field_battle points at the field battle context (g_field_battle_context):
 *   FieldBattleContext, with its FieldStatusRecord array.
 */

#include "common.h"
#include "field_menu_vars.h"
#include "saved_game.h"
#include "field_state_ops.h"

/** @brief g_saved_game.layout, bound by field_bind_runtime_pointers. */
extern SavedGameLayoutPtr g_field_game_state;

#define FIELD_ACTOR_RECORD_COUNT 16
#define FIELD_EVENT_RECORD_COUNT 2
/** @brief Actor ids from this one up name the event records (FieldRuntimeContext::events). */
#define FIELD_EVENT_ACTOR_ID_BASE 0x80
#define FIELD_ACTOR_SCRIPT_COUNT 16
#define FIELD_SCRIPT_FRAME_COUNT 8

#define FIELD_BATTLE_RECORD_COUNT 11

/** @brief Value written to an actor's pending-event byte when no event is queued. */
#define FIELD_NO_EVENT 0xFF

/** @brief Script id stored in an actor's event table for an unused event. */
#define FIELD_NO_SCRIPT 0xFFFF

/** @brief Value of an empty pending-effect slot in PetRecord::status. */
#define FIELD_NO_EFFECT 0xFF

/**
 * @brief View of a pet record shifted by @p bytes bytes.
 * @note Element 0 of a byte array member of the view is element @p bytes of the real
 *       record.
 */
#define FIELD_PET_AT(record, bytes) ((PetRecord*)((u8*)(record) + (bytes)))

/**
 * @brief Experience a character or stored companion needs to advance from @p level.
 * @param level Current level.
 * @return 10 * level * (2 * level - 1); JP: 5 * level * (5 * level - 2).
 * @note JP uses a steeper curve.
 */
static inline s32 field_level_threshold(s32 level)
{
#if defined(VERSION_JP)
    return (level - 1) * (level * 24 + level) + level * 15;
#else
    return (level - 1) * ((level * 5) << 2) + ((level * 5) << 1);
#endif
}

/* ------------------------------------------------------------------------ */
/* Item record staging (g_field_item_staging) and its generation tables (g_field_item_tables) */
/* ------------------------------------------------------------------------ */

#define FIELD_STAGING_LEVEL_COUNT 8
#define FIELD_STAGING_STAT_COUNT 8
#define FIELD_STAGING_SLOT_COUNT 6
#define FIELD_STAGING_PROPERTY_COUNT 6
#define FIELD_STAGING_FACTOR_COUNT 4

/** @brief Value of an unused FieldItemStaging slot byte. */
#define FIELD_STAGING_SLOT_EMPTY 0xFF

/** @brief Largest four-bit level a staged level entry can reach. */
#define FIELD_STAGING_LEVEL_MAX 0xF

/**
 * @brief First item kind of the secondary materials (tempering items); the
 *        generation tables index them from this bias.
 */
#define FIELD_SECONDARY_ITEM_FIRST 0x40

/** @brief FieldItemStaging::secondary_item of an item made without one. */
#define FIELD_NO_SECONDARY_ITEM 0xFF

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
 * @note Cleared by field_create_item_from_gosub, filled from the generation tables, then
 *       written back into @c record by field_write_staged_item.
 */
typedef struct FieldItemStaging
{
    FieldItemRecordPtr record;
    /** @brief Item category (FieldItemRecord::info bits 8-9). */
    u8 category;
    /** @brief Item type (FieldItemRecord::info bits 10-15). */
    u8 item_type;
    /** @brief Material (FieldItemRecord::info bits 16-21). */
    u8 material;
    /** @brief Secondary material or tempering item kind, or FIELD_NO_SECONDARY_ITEM. */
    u8 secondary_item;
    /** @brief Pool that pays for level increases. */
    s32 pool;
    FieldStagingLevel levels[FIELD_STAGING_LEVEL_COUNT];
    u8 effect_index;
    u8 pad1D[3];
    /** @brief Per stat, low nibble: modifier index; high nibble: row of the g_field_stat_modifier_limits bounds. */
    union
    {
        u8 bytes[FIELD_STAGING_STAT_COUNT];
        FieldStagingStats words[2];
    } stats;
    /** @brief Slot values, FIELD_STAGING_SLOT_EMPTY when unused. */
    u8 slots[FIELD_STAGING_SLOT_COUNT];
    u8 properties[FIELD_STAGING_PROPERTY_COUNT];
    /** @brief Weapon power flags, rebuilt from power_flag_mask and the levels. */
    u8 power_flags;
    /** @brief Armor element resistances, rebuilt from element_flag_mask. */
    u8 element_flags;
    /** @brief Armor status immunities, kept from the item being tempered. */
    u8 immunity_flags;
    u8 pad37[3];
    /** @brief Subtype divisor applied to the summed levels (FieldItemMaterialEntry::divisor). */
    u16 divisor;
    u8 weights[FIELD_STAGING_FACTOR_COUNT];
    u8 multipliers[FIELD_STAGING_FACTOR_COUNT];
    /** @brief Level increases still to be applied, per level entry. */
    u8 pending_levels[FIELD_STAGING_LEVEL_COUNT];
    /** @brief Levels whose nonzero value sets the matching bit of power_flags. */
    u8 power_flag_mask;
    /** @brief Bits copied into element_flags. */
    u8 element_flag_mask;
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
typedef PS1_PTR(FieldItemStaging) FieldItemStagingPtr;

/**
 * @brief View of a staging block shifted by @p bytes bytes.
 * @note Element 0 of a byte array member of the view is element @p bytes of
 *       the real block.
 */
#define FIELD_STAGING_AT(staging, bytes) ((FieldItemStaging*)((u8*)(staging) + (bytes)))

/** @brief Per-weapon-type or per-armor-type entry of FieldItemTable (0xC bytes). */
typedef struct FieldItemTypeEntry
{
    u16 scripts[2];
    u8 weights[4];
    u8 factors[4];
} FieldItemTypeEntry;

/** @brief Per-material entry of FieldItemTable (0x14 bytes). */
typedef struct FieldItemMaterialEntry
{
    u16 script;
    u16 divisor;
    u8 weights[4];
    u8 multipliers[4];
    u8 costs[FIELD_STAGING_LEVEL_COUNT];
} FieldItemMaterialEntry;

/** @brief Per-secondary-material entry of FieldItemTable (4 bytes), indexed from FIELD_SECONDARY_ITEM_FIRST. */
typedef struct FieldItemSecondaryEntry
{
    u8 pool_bonus;
    u8 pad1;
    u16 script;
} FieldItemSecondaryEntry;

/** @brief Per-slot-value entry of FieldItemTable: one script per slot position. */
typedef struct FieldItemSlotEntry
{
    u16 scripts[4];
} FieldItemSlotEntry;

/**
 * @brief Item generation table (field_find_resource(4)) for weapons and armor.
 */
typedef struct FieldItemTable
{
    u32 header;
    FieldItemTypeEntry weapon_types[16];
    FieldItemTypeEntry armor_types[16];
    FieldItemMaterialEntry materials[64];
    FieldItemSecondaryEntry secondary_items[192];
    FieldItemSlotEntry slot_values[160];
} FieldItemTable;

/**
 * @brief Item generation table (field_find_resource(0xF)) for instruments.
 */
typedef struct FieldItemGridTable
{
    u32 header;
    /** @brief Spell index of each (row, column) cell. */
    u8 grid[8][8];
    /** @brief Per material and instrument type: packed column/row start and power. */
    u8 pairs[64][4][2];
    /** @brief Script of each secondary material, indexed from FIELD_SECONDARY_ITEM_FIRST. */
    u16 secondary_scripts[192];
} FieldItemGridTable;

/**
 * @brief Generation table currently loaded into g_field_item_tables.
 * @note Script offsets passed to field_run_item_script are relative to @c bytes.
 */
typedef union FieldItemTables
{
    u8 bytes[1];
    FieldItemTable item;
    FieldItemGridTable grid;
} FieldItemTables;
typedef PS1_PTR(FieldItemTables) FieldItemTablesPtr;

/** @brief Staging block of the item being created or tempered. */
extern FieldItemStagingPtr g_field_item_staging;

/** @brief Generation table currently loaded by field_find_resource. */
extern FieldItemTablesPtr g_field_item_tables;

/* ------------------------------------------------------------------------ */
/* Field runtime context (g_field_runtime)                                       */
/* ------------------------------------------------------------------------ */

/** @brief One nested script frame (0xC bytes). */
typedef struct FieldScriptFrame
{
    u8_ptr pc;
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
    /** @brief Pickup handed over by field_grant_actor_pickup: FIELD_PICKUP_COUNTER selects a counter, else a reward key. */
    u16 pickup;
    /** @brief Pending event, or FIELD_NO_EVENT. */
    u8 event;
    u8 event_argument;
    /** @brief Bit n set when scripts[n] may run. */
    u16 enabled_events;
    /** @brief Event scripts and actor parameters copied from FieldLayoutRecord::scripts. */
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

/** @brief FieldActorRecord::selector of a record without one. */
#define FIELD_NO_SELECTOR 0xFF

/** @brief field_resolve_talk_window operand values. */
#define FIELD_TALK_AUTO 0xFF
#define FIELD_TALK_EFFECT_SELECTOR 0xFE
#define FIELD_TALK_PLANE_MASK 3
#define FIELD_TALK_PLANE_NO_FACING 0x80
#define FIELD_TALK_PLANE_FACING 0x40
/** @brief Effect flag set when the speaker's facing angle is in the flipped range. */
#define FIELD_TALK_FACING_FLAG 0x40

/** @brief FieldActorRecord::pickup bit: the pickup is a counter, not a reward table key. */
#define FIELD_PICKUP_COUNTER 0x8000

/** @brief Region trigger: map bounds and the command started on entry (12 bytes). */
typedef struct FieldTriggerRegion
{
    u16 min_x;
    u16 min_z;
    u16 max_x;
    u16 max_z;
    /** @brief FIELD_TRIGGER_SCRIPT set: script id for field_start_interaction; otherwise a monster group for field_battle_start. */
    u16 command;
    u16 unk0A;
} FieldTriggerRegion;

/** @brief Command bit of a FieldTriggerRegion that starts a script instead of a battle. */
#define FIELD_TRIGGER_SCRIPT 0x8000

/** @brief Scene trigger table (resource 6): a count, then the regions. */
typedef struct FieldTriggerTable
{
    u16 unk0;
    u16 count;
    FieldTriggerRegion regions[1];
} FieldTriggerTable;
typedef PS1_PTR(FieldTriggerTable) FieldTriggerTablePtr;

/** @brief Runtime state flag: party actors take event scripts from their resource pages. */
#define FIELD_STATE_PARTY_PAGE_SCRIPTS 0x10000

/** @brief Both party mode bits of the runtime state (party mode 3). */
#define FIELD_STATE_PARTY_MODE_MASK 0x60000

/** @brief Party control modes of field_begin_party_script_control. */
#define FIELD_PARTY_MODE_ALL 1
#define FIELD_PARTY_MODE_PAD_CONTROLLED 2
#define FIELD_PARTY_MODE_EVENT 3

/** @brief Actor events delivered by the field runtime. */
#define FIELD_EVENT_ON_SCREEN 2
#define FIELD_EVENT_OFF_SCREEN 3
#define FIELD_EVENT_IDLE 8
#define FIELD_EVENT_INTERACTION 0xD
#define FIELD_EVENT_FRAME 0xE
#define FIELD_EVENT_START 15

/** @brief Arguments of FIELD_EVENT_INTERACTION. */
#define FIELD_INTERACTION_SCRIPT_TARGET 0x80
#define FIELD_INTERACTION_SCRIPT_OTHER 0x81
#define FIELD_INTERACTION_END 0x82
#define FIELD_INTERACTION_TALK_TARGET 0x83
#define FIELD_INTERACTION_TALK_OTHER 0x84
#define FIELD_INTERACTION_TALK_END 0x85

/**
 * @brief Field runtime context (D_80122C00, reached through g_field_runtime).
 */
typedef struct FieldRuntimeContext
{
    /** @brief The script locals at the start of the context; the menus read them too. */
    FieldMenuVars locals;
    u8 pad020[0x24 - 0x20];
    /** @brief Parameter block of the script commands (opcode 0x03): runtime script variable words 9 to 16. */
    s32 command_params[8];
    s32 actor_positions[FIELD_PARTY_SIZE];
    u8 pad050[4];
    s32 view_x;
    s32 view_z;
    u8 pad05C[0xB8 - 0x5C];
    s32 triggered_regions;
    s32 frame_count;
    u8 pad0C0[0x104 - 0xC0];
    /** @brief Staging block of the item commands (g_field_item_staging points here while they run). */
    FieldItemStaging item_staging;
    u8 pad164[0x400 - 0x164];
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
            /** @brief Field object of the new scene. */
            u8 object_id;
            /** @brief Bits 0-4: spawn point in the new scene (FIELD_SPAWN_ID_MASK). */
            u8 spawn;
        } fields;
        struct
        {
            u32 scene_id : 16;
            u32 object_id : 8;
            u32 spawn_id : 5;
            /** @brief Set once the transition fade has been started. */
            u32 fade_started : 1;
            /** @brief Set by a script to request the transition. */
            u32 requested : 1;
            /** @brief Set when the fade is done and the field is left. */
            u32 finished : 1;
        } bits;
    } transition;
    /** @brief Talk window state: the current window selector and plane. */
    union
    {
        s32 word;
        struct
        {
            u32 selector : 8;
            u32 plane : 2;
            u32 unk10 : 22;
        } bits;
    } talk_window;
    u8 pad420[4];
    /** @brief Scroll position saved by the camera script command (mode 0) and restored by mode 3. */
    s32 saved_scroll_x;
    s32 saved_scroll_z;
    u8 pad42C[4];
    FieldActorRecord actors[FIELD_ACTOR_RECORD_COUNT];
    FieldActorRecord events[FIELD_EVENT_RECORD_COUNT];
    FieldScriptState script;
    FieldTriggerTablePtr trigger_table;
} FieldRuntimeContext;
typedef PS1_PTR(FieldRuntimeContext) FieldRuntimeContextPtr;

/** @brief The context itself, which g_field_runtime points at. */
extern FieldRuntimeContext D_80122C00;

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
typedef PS1_PTR(FieldBattleAction) FieldBattleActionPtr;

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
typedef PS1_PTR(FieldActionDescriptor) FieldActionDescriptorPtr;

/**
 * @brief Action descriptor bank (g_field_default_action_bank, reached through g_field_action_bank).
 * @note Holds four descriptor tables, located by byte offsets from the bank start.
 */
typedef struct FieldActionBank
{
    s32 table_offsets[4];
} FieldActionBank;
typedef PS1_PTR(FieldActionBank) FieldActionBankPtr;

/** @brief Size of one party member's script page. */
#define FIELD_PARTY_SCRIPT_PAGE_SIZE 0x1000

/**
 * @brief Scripts and actions of one party member (g_field_party_script_pages), copied from its field data chunk.
 * @note The header holds the byte offsets of three tables from the page start: event script
 *       offsets, private script offsets and the action descriptors. The script offset tables
 *       hold halfword offsets relative to the table itself.
 */
typedef union FieldPartyScriptPage
{
    s32 table_offsets[3];
    u8 bytes[FIELD_PARTY_SCRIPT_PAGE_SIZE];
} FieldPartyScriptPage;

extern FieldPartyScriptPage g_field_party_script_pages[];

/**
 * @brief Field battle context (g_field_battle_context, reached through g_field_battle).
 * @note field_battle_reset_context clears 0x4A4 bytes and rebuilds the header.
 */
typedef struct FieldBattleContext
{
    /** @brief Bit 31 set while the battle is finished or suspended; the low byte holds the monster level. */
    union
    {
        s32 flags;
        u8 level;
    } state;
    FieldActorTemplateTablePtr templates;
    u8_ptr resources;
    u8 element_levels[8];
    /** @brief Flags of the action being resolved; cleared by field_battle_bind_action. */
    union
    {
        u32 word;
        struct
        {
            /** @brief Set when the target guards; the damage then has no minimum of 1. */
            u32 guarded : 1;
            /** @brief field_is_actor_in_front result for the attacker and the target. */
            u32 side : 1;
            /** @brief Set when field_battle_check_target_stance marks a follow-up action. */
            u32 follow_up : 1;
            u32 unk3 : 29;
        } bits;
    } action_flags;
    FieldBattleActionPtr action;
    FieldActionDescriptorPtr descriptor;
    FieldStatusRecordPtr attacker;
    FieldStatusRecordPtr target;
    FieldStatusRecord records[FIELD_BATTLE_RECORD_COUNT];
    u16 power;
    u8 power_flags;
    u8 pad4A3;
} FieldBattleContext;
typedef PS1_PTR(FieldBattleContext) FieldBattleContextPtr;

/** @brief FIELD diagnostic codes passed to record_game_diagnostic. */
#define DIAG_SCRIPT_FRAME_OVERFLOW 2 /**< A script pushed past its last frame. */
#define DIAG_BAD_MONSTER_OBJECT 0x64 /**< A monster group member has no object state. */
#define DIAG_BAD_COMMAND 0x66        /**< Unknown command in a character's command slot. */
#define DIAG_BAD_MONSTER_ACTION 0x69 /**< Monster action id past the end of its template's list. */
#define DIAG_BAD_GUEST 0x6D          /**< No guest template for the requested guest. */
#define DIAG_BAD_COMPANION 0x6E      /**< The requested stored companion does not exist. */
#define DIAG_BAD_LAND 0x73           /**< Land index past the land table. */
#define DIAG_MISSING_EFFECT_THRESHOLDS 0x3E7 /**< The effect threshold resource (0x10) is not loaded. */

/** @brief Script variables holding the resource variant of the guest and companion actors. */
#define FIELD_VARIABLE_GUEST_VARIANT 0x2F08
#define FIELD_VARIABLE_COMPANION_VARIANT 0x2F00

/** @brief Resource variant written when a slot is emptied, or returned when a join fails. */
#define FIELD_NO_VARIANT 0xFF

/** @brief Script variable that receives the result of many script commands. */
#define FIELD_VAR_RESULT 0x7100

/** @brief Script variables read or written by the battle code (see field_get_script_var). */
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
/** @brief FIELD_VAR_DIFFICULTY values. */
#define FIELD_DIFFICULTY_NORMAL 0
#define FIELD_DIFFICULTY_HARD 1
#define FIELD_DIFFICULTY_HARDEST 2
/** @brief Script variable: bit 7 bases every monster level on the hero's level. */
#define FIELD_VAR_LEVEL_FLAGS 0x52F0
#define FIELD_LEVEL_FLAG_HERO 0x80
/** @brief Script variables: lowest and highest monster level. */
#define FIELD_VAR_MONSTER_LEVEL_MIN 0x52E0
#define FIELD_VAR_MONSTER_LEVEL_MAX 0x52E8

/** @brief Owner-relative script variable: event argument of the script owner, set when its script runs. */
#define FIELD_VAR_EVENT_ARGUMENT 0xD000

#endif
