#ifndef SAVED_GAME_H
#define SAVED_GAME_H

/**
 * @file saved_game.h
 * @brief Layout of the saved game (g_saved_game): the game state that FIELD
 *        and the menus work on and that a memory-card save stores.
 */

#include "common.h"

#define SAVED_GAME_DATA_SIZE 0x3268
#define SAVED_GAME_BUFFER_SIZE 0x4000
#define SAVED_CHARACTER_SIZE 0x250

#define SAVED_OPTION_FLAG_2 0x04
#define SAVED_OPTION_FLAG_3 0x08

/** @brief SavedGameLayout::words index of the known-save flags: one bit per product code of a game save found on a memory card. */
#define SAVED_GAME_WORD_KNOWN_SAVES 0x48

/** @brief SavedGameLayout::words index of the mode flags; bit 0 marks a TITLE-prepared slot. */
#define SAVED_GAME_WORD_MODE_FLAGS 0x7F

/** @brief Spawn record a save file is written with; loading a game enters the field through it. */
#define FIELD_SPAWN_LOAD_GAME 6

/** @brief Save-browser party icon of an empty party slot. */
#define SAVE_NO_ICON 0x7F

/** @brief Save-browser party icons: the two hero portraits come first, then pets and golems. */
#define SAVE_ICON_HERO_COUNT 2
#define SAVE_ICON_PET_BASE 0x0E
#define SAVE_ICON_GOLEM_BASE 0x4F

/** @brief SavedGameLayout::play_time ticks in one minute (60 ticks per second). */
#define SAVED_PLAY_TIME_TICKS_PER_MINUTE 3600

/** @brief SavedGameLayout::play_time ticks in one hour. */
#define SAVED_PLAY_TIME_TICKS_PER_HOUR (SAVED_PLAY_TIME_TICKS_PER_MINUTE * 60)

#define FIELD_PARTY_SIZE 3
/** @brief Party slot of the hero. */
#define FIELD_PARTY_HERO 0
/** @brief Party slot of the guest character. */
#define FIELD_PARTY_GUEST 1
/** @brief Party slot of the companion (a stored companion or a golem). */
#define FIELD_PARTY_COMPANION 2
#define FIELD_LAND_COUNT 64
#define FIELD_ITEM_COUNT 100
/** @brief Number of consumable item kinds with a held quantity. */
#define FIELD_ITEM_KIND_COUNT 256
#define FIELD_EQUIPMENT_SLOT_COUNT 4
/** @brief Equipment slot holding the character's weapon. */
#define FIELD_WEAPON_SLOT 0
#define FIELD_ITEM_SPECIAL_COUNT 4
#define FIELD_CHARACTER_STAT_COUNT 8
/** @brief Number of pet records. */
#define PET_RECORD_COUNT 5
/** @brief Words of SavedGameLayout::encyclopedia_bits (1024 entries). */
#define FIELD_ENCYCLOPEDIA_BIT_WORDS 32
/** @brief Weapon categories (technique masks and weapon proficiency slots). */
#define FIELD_WEAPON_CATEGORY_COUNT 11
/** @brief Abilities with a proficiency counter. */
#define FIELD_ABILITY_COUNT 88
/** @brief Words of the learned-ability bit set. */
#define FIELD_ABILITY_WORD_COUNT 3

/** @brief Character info bits 0-6: character type. */
#define FIELD_CHARACTER_TYPE_MASK 0x7F

/** @brief Character type of a guest in party slot 1. */
#define FIELD_CHARACTER_GUEST 2

/** @brief Character type of a stored companion in party slot 2. */
#define FIELD_CHARACTER_COMPANION 3

/** @brief Character type of a golem companion in party slot 2. */
#define FIELD_CHARACTER_GOLEM 4

/** @brief Character info bit 7: a player's controller drives the character; clear for a computer-controlled companion. */
#define FIELD_CHARACTER_PAD_CONTROLLED 0x80
#define FIELD_CHARACTER_PAD_CONTROLLED_SHIFT 7

/**
 * @brief SavedGameLayout::compatibility_tag values.
 * @note A new game uses SAVE_TAG_ANY, loading a save adopts its tag, and saving writes the
 *       current tag; the load screens refuse a save whose tag differs unless one is SAVE_TAG_ANY.
 */
#define SAVE_TAG_ANY 0xFF  /**< Matches every save; what new games use. */
#define SAVE_TAG_STARTUP 7 /**< Set at startup, before a new game or a load replaces it. */

/**
 * @brief Identity of a saved game.
 * @note Two saves with the same game_id come from the same playthrough.
 */
typedef union
{
    u32 word;
    struct
    {
        /** @brief Random id chosen when a new game starts (TITLE). */
        u16 game_id;
        /** @brief Random id chosen again each time the game is saved (CARDA). */
        u16 save_id;
    } ids;
} SaveIdentity;

/* ------------------------------------------------------------------------ */
/* Saved game records                                                       */
/* ------------------------------------------------------------------------ */

/** @brief Length of the encoded name at the start of a FieldItemRecord. */
#define FIELD_ITEM_NAME_LENGTH 0x14

/** @brief Item category: bits 8-9 of FieldItemRecord.info. */
#define FIELD_ITEM_CATEGORY(info) (((info) >> 8) & 3)

/** @brief Mask of FieldItemRecord.info.halves[1] selecting the material (an item name index). */
#define FIELD_ITEM_MATERIAL_MASK 0x3F

/** @brief Item type within its category: bits 10-15 of FieldItemRecord.info. */
#define FIELD_ITEM_TYPE(info) (((info) >> 10) & 0x3F)

/** @brief FIELD_ITEM_CATEGORY values. */
enum
{
    FIELD_ITEM_CATEGORY_WEAPON = 0,
    FIELD_ITEM_CATEGORY_ARMOR = 1,
    FIELD_ITEM_CATEGORY_INSTRUMENT = 2
};

/** @brief Shift of FieldStat's effective value (bits 9-15). */
#define FIELD_STAT_EFFECTIVE_SHIFT 9

/** @brief FieldStat base value bits (0-8) of a stat halfword. */
#define FIELD_STAT_BASE_MASK 0x1FF

/** @brief FieldStat effective value bits (9-15) of a stat halfword. */
#define FIELD_STAT_EFFECTIVE_MASK 0xFE00

/** @brief One character stat: base value (bits 0-8, quarter units) and effective value with equipment (bits 9-15). */
typedef union FieldStat
{
    u16 value;
    struct
    {
        u16 base : 9;
        u16 effective : 7;
    } bits;
} FieldStat;

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
    /** @brief Material the item was made from (an item kind below FIELD_SECONDARY_ITEM_FIRST). */
    unsigned material : 6;
    unsigned unk22 : 10;
} FieldItemInfo;

/** @brief Pair of words that identifies one item record. */
typedef struct FieldItemKey
{
    s32 first;
    s32 second;
} FieldItemKey;

typedef struct FieldItemRecord
{
    /** @brief Encoded item name; an empty name marks a free record. */
    u8 name[FIELD_ITEM_NAME_LENGTH];
    /** @brief Bits 8-9 category, 10-15 item type, 16-21 item subtype. */
    union
    {
        u32 word;
        FieldItemInfo bits;
        /** @brief Halfword view; halves[1] holds the item subtype in bits 0-5. */
        u16 halves[2];
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
        /** @brief Instrument: summoned spirit, spell (a cell of the instrument grid) and power. */
        struct
        {
            u8 spirit;
            u8 spell;
            u8 power;
        } instrument;
        /** @brief Golem companion (party slot 2, unk150[0]): GolemGroupRecord bytes 0x44-0x4B. */
        struct
        {
            u32 logic_class : 4;
            u32 grid_bound : 4;
            u32 unknown_bits : 24;
            s32 palette;
        } golem;
    } derived;
    /** @brief Weapon: power flags used in battle. Armor: status immunities. */
    u8 status_flags;
    /** @brief Armor: element resistances. */
    u8 element_flags;
    u16 effect_index;
    u8 attributes[4];
    /** @brief Cached item value (field_get_item_value); 0 until computed. */
    s32 value;
    /** @brief Unique key, generated by field_generate_item_key when the item is created. */
    FieldItemKey key;
} FieldItemRecord;

/** @brief Four-byte pointer to FieldItemRecord in PS1 storage. */
typedef FieldItemRecord* PS1_PTR32 FieldItemRecordPtr;

/**
 * @brief Party character record in the game state (0x250 bytes).
 * @note A character slot is in use when the first byte of its name is nonzero.
 */
typedef struct FieldCharacterRecord
{
    u8 name[24];
    /** @brief Byte 0: bits 0-6 character type, bit 7 FIELD_CHARACTER_PAD_CONTROLLED. */
    union
    {
        u32 word;
        u8 bytes[8];
        struct
        {
            u8 type : 7;
            /** @brief FIELD_CHARACTER_PAD_CONTROLLED: the member is driven by a controller, not scripted. */
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
    /** @brief Duels won and lost against the other player. */
    u16 duel_wins;
    u16 duel_losses;
    /** @brief Action slot bound to each pad button (see g_field_hint_button_map); reset to identity when a companion joins. */
    u8 button_actions[8];
    FieldItemRecord equipment[FIELD_EQUIPMENT_SLOT_COUNT];
    /** @brief Four further item records; the hero's are scanned with the equipment as one run of eight. */
    FieldItemRecord unk150[4];
} FieldCharacterRecord;

/** @brief FieldLandRecord::flags bits. */
#define FIELD_LAND_PLACED 0x01    /**< The land has been placed on the map. */
#define FIELD_LAND_FLAG_02 0x02   /**< TODO meaning unknown; excludes a placed land from the active count. */
#define FIELD_LAND_FLAG_04 0x04   /**< TODO meaning unknown. */
#define FIELD_LAND_AVAILABLE 0x08 /**< The land is available for placement. */
/** @brief FieldLandRecord::x and ::z of a land that is not on the map. */
#define FIELD_LAND_CELL_NONE 15

/** @brief Per-land record (0xC bytes). */
typedef struct FieldLandRecord
{
    u8 flags;
    /** @brief Map grid column, or FIELD_LAND_CELL_NONE. */
    u8 x : 4;
    /** @brief Map grid row, or FIELD_LAND_CELL_NONE. */
    u8 z : 4;
    u8 unk2;
    /** @brief Placement order: SavedGameLayout placed_land_count when the land was placed. */
    u8 count;
    u8 levels[8];
} FieldLandRecord;

/** @brief A FieldLandRecord whose first four bytes are also read as one word. */
typedef union FieldLandWords
{
    FieldLandRecord record;
    u32 word;
} FieldLandWords;

/** @brief Menu slot entry index of an empty slot. */
#define FIELD_MENU_ENTRY_EMPTY 0xFF

/** @brief First entry index of an effect slot (effect id; row 0 of the effect tables). */
#define FIELD_EFFECT_ID_BASE 0x60

/** @brief Effect ids FIELD_EFFECT_ID_BASE and up that have effect table rows. */
#define FIELD_EFFECT_COUNT 40

/** @brief Menu slot handle kinds; field_classify_menu_slots grades a slot into one of them. */
#define FIELD_MENU_SLOT_UNUSED 0
#define FIELD_MENU_SLOT_PLAIN 1
#define FIELD_MENU_SLOT_ITEM 2
#define FIELD_MENU_SLOT_INDEXED 3

/** @brief Counters of a menu slot, graded against its effect thresholds. */
#define FIELD_MENU_SLOT_COUNTER_COUNT 8

/** @brief One menu action slot (0x10 bytes). */
typedef struct FieldMenuSlot
{
    /** @brief FIELD_MENU_SLOT_* kind. */
    s32 handle;
    /** @brief Byte 0: entry index; bits 8-9: result type. */
    union
    {
        u32 word;
        u8 index;
        struct
        {
            u32 index : 8;
            u32 result_type : 2;
            u32 unk10 : 22;
        } bits;
    } entry;
    /** @brief Per-slot counters advanced by field_menu_advance_action_counters. */
    u8 counters[FIELD_MENU_SLOT_COUNTER_COUNT];
} FieldMenuSlot;

/** @brief Items a menu slot group holds. */
#define FIELD_MENU_GROUP_ITEM_COUNT 4

/** @brief Action slots of a menu slot group. */
#define FIELD_MENU_GROUP_SLOT_COUNT 8

/** @brief Menu action slot group (0x8C bytes): a header, then eight slots. */
typedef struct FieldMenuSlotGroup
{
    union
    {
        u32 word;
        struct
        {
            u32 capacity : 4;
            u32 unk4 : 4;
            u32 item_slot_count : 4;
            /** @brief Number of valid entries in @c items. */
            u32 item_count : 4;
            u32 unk16 : 16;
        } bits;
    } flags;
    u8 pad4[4];
    /** @brief Items placed in the group (effect-table rows from 0x58 up); 0xFF when unused. */
    u8 items[FIELD_MENU_GROUP_ITEM_COUNT];
    FieldMenuSlot slots[FIELD_MENU_GROUP_SLOT_COUNT];
} FieldMenuSlotGroup;

/**
 * @brief Growth byte of a stored companion record: low nibble the per-level rate, high
 *        nibble an accumulator whose bit 3 (bit 7 of the byte) carries into a total.
 */
typedef union FieldGrowth
{
    u8 byte;
    struct
    {
        u8 rate : 4;
        u8 accumulator : 4;
    } bits;
} FieldGrowth;

/** @brief Length of a stored companion name as passed to field_set_text_macro. */
#define FIELD_COMPANION_NAME_LENGTH 21

/** @brief Status flags of a PetRecord (the high bits of PetRecord::status). */
typedef struct
{
    u32 unknown_bits : 30;
    /** @brief Left grazing at the ranch: the pet cannot join the party and gains experience. */
    u32 grazing : 1;
    /** @brief The pet has not hatched yet; it hatches when hatch_counter reaches 0. */
    u32 egg : 1;
} PetStatusFlags;

/** @brief PetRecord::status bit of PetStatusFlags::egg. */
#define PET_STATUS_EGG 0x80000000

/** @brief Length of a pet name as passed to field_set_text_macro. */
#define PET_NAME_LENGTH 21

/**
 * @brief Pet record (0x60 bytes): a monster raised at the ranch, or an egg waiting to hatch.
 * @note The pet in the party (SavedGameLayout::joined_pet) is copied into
 *       characters[2] (character type 3) and written back when it leaves.
 */
typedef struct PetRecord
{
    /** @brief Display name; the record is in use when the first byte is nonzero. */
    u8 name[PET_NAME_LENGTH];
    /** @brief Monster species; picks the portrait and becomes the party record's info byte 1. */
    u8 species;
    /** @brief Species of the egg's portrait while the pet is still an egg. */
    u8 egg_species;
    u8 unk17;
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
    /** @brief Attack power; shown in the first stat column of the GOSUB roster. */
    u16 power;
    u16 equipment_totals[4];
    /** @brief Bits 0-8: stat value (times four), bits 9-15: growth. */
    u16 stats[FIELD_CHARACTER_STAT_COUNT];
    u8 unk38[4];
    /** @brief Resource 0xD index of equipment[0]. */
    u8 weapon_id;
    /** @brief Resource 0xE indexes of equipment[1] to equipment[3]. */
    u8 armor_ids[3];
    u8 pad40[2];
    /** @brief Egg hatching countdown; low values mean the egg is nearly ready. */
    u16 hatch_counter;
    /**
     * @brief Bytes 0-2: pending effect ids (0xFF when empty); bits 24-26: effects
     *        already applied; bits 30-31: PetStatusFlags.
     */
    union
    {
        u32 word;
        u8 effects[4];
        PetStatusFlags bits;
    } status;
    /** @brief Low byte: flag bits set or cleared by effects. */
    union
    {
        s32 word;
        u8 flags;
    } unk48;
    /** @brief Per-stat growth: the accumulator is added to the stat per level, then reset to the rate. */
    FieldGrowth stat_growth[FIELD_CHARACTER_STAT_COUNT];
    /** @brief Growth of equipment_totals: the accumulator carry (bit 7) is added per level. */
    FieldGrowth total_growth[4];
    /** @brief Byte 0 like total_growth for power; byte 1 added to halfword 1 per level. */
    union
    {
        u32 word;
        u8 bytes[4];
        u16 halves[2];
        /** @brief Byte 0 as a growth byte inside the word: the rate is read as a byte, the accumulator written with the word. */
        struct
        {
            u8 rate : 4;
            u32 accumulator : 4;
            u32 unk8 : 24;
        } growth;
    } extra_growth;
    /** @brief Random identifier, unique among the pet records. */
    s32 unique_id;
} PetRecord;

/** @brief Number of cells in the six-by-six golem logic-block placement grid. */
#define GOLEM_GRID_CELL_COUNT 36
/** @brief Number of columns in the placement grid. */
#define GOLEM_GRID_WIDTH 6
/** @brief Grid cell owner value for an empty cell. */
#define GOLEM_GRID_EMPTY 99

/** @brief One cell of the six-by-six golem logic-block placement grid. */
typedef struct
{
    u8 block_id; /**< Id of the block covering the cell. */
    u8 detail;   /**< Detail value of that block. */
    u8 edge;     /**< Index of the cell below when it belongs to another block, else 99. */
    u8 owner;    /**< Logic-block index covering the cell, or 99 when empty. */
} GolemGridCell;

/** @brief Number of golem records. */
#define GOLEM_RECORD_COUNT 3
/** @brief Number of armor stat values stored in a golem record. */
#define COMPANION_STAT_COUNT 4
/** @brief Mask of SavedGameLayout::golem_count selecting the number of golems. */
#define GOLEM_COUNT_MASK 0xF
/** @brief Capacity of the packed logic-block table. */
#define LOGIC_BLOCK_CAPACITY 40
/** @brief Logic-type value marking a logic block that no logic type owns. */
#define LOGIC_BLOCK_UNASSIGNED 3

/** @brief Packed logic-block word stored in SavedGameLayout::logic_blocks. */
typedef union
{
    u32 word;
    struct
    {
        u32 logic_type : 2; /**< Owning logic type; LOGIC_BLOCK_UNASSIGNED when free. */
        u32 id : 6;         /**< Logic-block type index. */
        u32 quantity : 4;   /**< Level shown after the name; zero hides it. */
        u32 shape : 4;      /**< Index of the block's composite-icon layout. */
        u32 placed : 1;     /**< Set while the block is placed on its golem group's grid. */
        u32 rotation : 2;   /**< Placed rotation, 0-3. */
        s32 grid_x : 5;     /**< Placed grid column, relative to the layout origin. */
        s32 grid_y : 5;     /**< Placed grid row, relative to the layout origin. */
        u32 unknown_bits : 3;
    } f;
} LogicBlock;

/** @brief Mask of GolemRecord.logic_layout selecting the golem's logic class (its type). */
#define GOLEM_LOGIC_CLASS_MASK 0xF
/** @brief Shift of the grid bound (usable placement grid size) in GolemRecord.logic_layout. */
#define GOLEM_GRID_BOUND_SHIFT 4

/** @brief Number of source items (weapons and armor) a golem is built from. */
#define GOLEM_SOURCE_ITEM_COUNT 4

/**
 * @brief Golem record: a golem built at the workshop from weapons and armor.
 * @note FIELD reads the same bytes through GolemGroupRecord (field_golem_layout.h).
 */
typedef struct
{
    u8 name[0x15];
    u8 unknown_0x15;
    u16 secondary_value; /**< Hit points; the second stat column of the GOSUB roster. */
    u16 primary_value;   /**< Weapon power; the first stat column of the GOSUB roster. */
    u16 stats[COMPANION_STAT_COUNT];
    u8 unknown_0x22[0x44 - 0x22];
    u8 logic_layout; /**< Low nibble: logic class; high nibble: grid bound. */
    u8 unknown_0x45;
    u8 unknown_0x46; /**< 75 - 10 * grid bound, clamped to 0-50; shown as a number by MENU. */
    u8 unknown_0x47;
    s32 palette;                                           /**< Portrait and sprite palette, 0-31; MENU also names it as the golem's color. */
    FieldItemRecord source_items[GOLEM_SOURCE_ITEM_COUNT]; /**< Items the golem was built from; an empty name ends the list. */
} GolemRecord;

/**
 * @brief Layout of the saved game (g_saved_game).
 * @note The leading SAVED_GAME_DATA_SIZE bytes are what a save file stores.
 */
typedef struct SavedGameLayout
{
    /** @brief Hero name shown by the save browsers; copied from characters[0] when saving. */
    u8 summary_name[21];
    u8 unk15;
    u8 unk16;
    /** @brief Number of in-use save slot records, stored for the save browsers. */
    u8 summary_slot_count;
    /** @brief Word 0x18: spawn record of the current field scene, and the first party icon. */
    union
    {
        s32 word;
        struct
        {
            /** @brief Spawn record of the current field scene. */
            u32 id : 25;
            /** @brief Save-browser icon of party slot 0; SAVE_NO_ICON when empty. */
            u32 party_icon_0 : 7;
        } bits;
    } spawn;
    s16 sound_bank_id;
    s8 secondary_music_id;
    /** @brief Palette of the save-browser party icons. */
    u8 icon_palette;
    /**
     * @brief Word 0x20: music track of the current field scene, and the second and
     *        third party icons. The save browsers also use the track to pick the
     *        location name.
     */
    union
    {
        u32 word;
        struct
        {
            u32 music_track : 18;
            /** @brief Save-browser icon of party slot 1; SAVE_NO_ICON when empty. */
            u32 party_icon_1 : 7;
            /** @brief Save-browser icon of party slot 2; SAVE_NO_ICON when empty. */
            u32 party_icon_2 : 7;
        } bits;
    } track;
    u16 scene_id;
    u8 object_id;
    u8 music_id;
    /** @brief Config menu options: controller vibration, mono sound, SAVED_OPTION_FLAG_*. */
    union
    {
        u32 word;
        struct
        {
            u32 vibration : 1;
            u32 mono_sound : 1;
            /** @brief SAVED_OPTION_FLAG_2. */
            u32 flag_2 : 1;
            /** @brief SAVED_OPTION_FLAG_3. */
            u32 flag_3 : 1;
            u32 unk4 : 28;
        } bits;
    } options;
    /** @brief Money, saturated at 10,000,000. */
    u32 money;
    /** @brief Play time in 1/60 s ticks (SAVED_PLAY_TIME_TICKS_PER_MINUTE). */
    s32 play_time;
    /** @brief Learned techniques: one bit mask per weapon category (bit = technique index). */
    u32 technique_bits[FIELD_WEAPON_CATEGORY_COUNT];
    /** @brief Learned abilities, one bit per ability. */
    u32 ability_bits[FIELD_ABILITY_WORD_COUNT];
    /** @brief Training level of each ability, 0 to 100. */
    u8 ability_proficiency[FIELD_ABILITY_COUNT];
    /** @brief Training level of each weapon category, 0 to 100. */
    u8 weapon_proficiency[FIELD_WEAPON_CATEGORY_COUNT];
    /** @brief Save compatibility tag (SAVE_TAG_ANY in ordinary saves); not a slot number. */
    u8 compatibility_tag;
    u8 padD0[0xD4 - 0xD0];
    SaveIdentity identity;
    /** @brief Identity of the save the guest hero in characters[1] was loaded from (ADDHERO). */
    SaveIdentity guest_origin;
    u8 padDC[2];
    /** @brief Set to 1 by ADDHERO when it loads a guest hero into characters[1]. */
    u16 guest_loaded;
    /** @brief World map grid cell (column + row * 19) chosen in WSEL. */
    s32 world_map_cell;
    s32 words[0x80];
    union
    {
        u32 word;
        struct
        {
            /** @brief Lands placed so far; the next placed land gets this count as its order. */
            u8 placed_land_count;
            u8 hero_level;
            /** @brief Bits 0-6: day of the week (index into g_field_weekday_names). */
            u16 weekday;
        } fields;
        struct
        {
            u32 placed_land_count : 8;
            u32 hero_level : 8;
            /** @brief Day of the week (index into g_field_weekday_names). */
            u32 weekday : 7;
            u32 unk23 : 9;
        } bits;
    } control;
    u32 flag_bits[2];
    FieldLandRecord lands[FIELD_LAND_COUNT];
    FieldCharacterRecord characters[FIELD_PARTY_SIZE];
    FieldItemRecord items[FIELD_ITEM_COUNT];
    /** @brief Quantity held of each consumable item. */
    u8 item_counts[FIELD_ITEM_KIND_COUNT];
    u8 pad26E0[0x26E4 - 0x26E0];
    FieldMenuSlotGroup menu_slots[2];
    u8 pad27FC[0x29D4 - 0x27FC];
    /** @brief Low nibble: the number of golems; high nibble: the golem that last left the party. */
    u8 golem_count;
    /** @brief Golems created so far, saturating at 200. */
    u8 golems_created;
    /** @brief Number of used entries in @c logic_blocks. */
    u8 logic_block_count;
    /** @brief Golem record in the party, or 3 for none. */
    s8 joined_golem;
    /** @brief Display order of @c golem_records; values >= 3 are empty. */
    u8 golem_order[GOLEM_RECORD_COUNT];
    /** @brief Golem slot shown at each display position, three 2-bit fields. */
    u8 golem_display_order;
    LogicBlock logic_blocks[LOGIC_BLOCK_CAPACITY];
    /** @brief Logic-block grid of the golem companion (GolemLayoutView::grid). */
    GolemGridCell golem_grid[GOLEM_GRID_CELL_COUNT];
    GolemRecord golem_records[GOLEM_RECORD_COUNT];
    /** @brief Pet record in the party (characters[2]), or PET_RECORD_COUNT for none. */
    s32 joined_pet;
    PetRecord pets[PET_RECORD_COUNT];
    /** @brief Unlocked encyclopedia (ZUKAN) entries, one bit each. */
    u32 encyclopedia_bits[FIELD_ENCYCLOPEDIA_BIT_WORDS];
    u8 pad3154[0x315C - 0x3154];
    /** @brief Times the battle was retried from g_field_retry_snapshot; -1 stops counting. */
    s32 retry_count;
    /** @brief Records whose first byte is nonzero while in use; the save summary counts them. */
    struct
    {
        u8 in_use;
        u8 unk01[0x3F];
    } summary_records[4];
} SavedGameLayout;

/** @brief Four-byte pointer to SavedGameLayout in PS1 storage. */
typedef SavedGameLayout* PS1_PTR32 SavedGameLayoutPtr;

/** @brief Size of a Legend of Mana save file: two 8 KiB memory-card blocks. */
#define SAVE_FILE_BYTES 0x4000

/** @brief Bytes of a save file covered by its checksum: everything before SaveFile::checksum. */
#define SAVE_FILE_CHECKSUM_BYTES 0x33E0

/** @brief SaveFile::magic of a valid save ("ANA"). */
#define SAVE_FILE_MAGIC 0x00414E41

/** @brief Added to twice the byte sum of a save file to form SaveFile::checksum. */
#define SAVE_FILE_CHECKSUM_BIAS 0x0414E410

/** @brief Length of one of the two lines of SaveFileHeader::title. */
#define SAVE_FILE_TITLE_LINE_BYTES 0x20

/** @brief Standard PSX memory-card file header (0x180 bytes): title and icon. */
typedef struct
{
    char magic[2];
    u8 icon_flags;
    u8 block_count;
    /** @brief Shift-JIS title in two lines. */
    u8 title[2][SAVE_FILE_TITLE_LINE_BYTES];
    u8 reserved[0x1C];
    /** @brief 16-color icon CLUT (16-bit entries), stored as bytes. */
    u8 clut[0x20];
    u8 icon_frames[2][0x80];
} SaveFileHeader;

/**
 * @brief A Legend of Mana save file.
 * @note The saved game is copied in as SAVED_GAME_DATA_SIZE bytes, so the copy
 *       runs 8 bytes past saved_game; checksum and magic are written over them.
 */
typedef struct
{
    SaveFileHeader header;
    SavedGameLayout saved_game;
    /** @brief Checksum of the bytes before it (see SAVE_FILE_CHECKSUM_BIAS). */
    s32 checksum;
    s32 magic;
    u8 unused[SAVE_FILE_BYTES - sizeof(SaveFileHeader) - sizeof(SavedGameLayout) - 2 * sizeof(s32)];
} SaveFile;

/** @brief Game-state workspace; the leading SAVED_GAME_DATA_SIZE bytes are saved to the memory card. */
typedef union
{
    SavedGameLayout layout;
    u8 bytes[SAVED_GAME_BUFFER_SIZE];
    s32 words[SAVED_GAME_BUFFER_SIZE / sizeof(s32)];
} SavedGame;

extern SavedGame g_saved_game;

/** @brief Pointer to g_saved_game.layout, bound by field_bind_saved_game_context. */
extern SavedGameLayout* g_saved_game_ctx;

#endif
