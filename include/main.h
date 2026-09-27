#ifndef _MAIN_H
#define _MAIN_H

#include "common.h"
#include "game_state.h"
#include "saved_game.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "sdk/libapi.h"
#include "sdk/libetc.h"

/** @brief Value set by field pair opcode 0x49 and cleared at boot. */
extern s32 g_script_pair_value_49;

extern u32 g_field_scene_config;    /**< Packed field-entry configuration passed to field_set_scene_parameters. */
/** @brief Current scene/mode identifier (0, 0xD for default menu template). */
extern u16 g_scene_mode;
/** @brief Option/parameter word from SavedGameLayout; -1 = unset. */
extern s32 g_layout_option;
/** @brief Countdown timer for delayed music/SFX trigger on field entry. */
extern s32 g_field_audio_timer;
/** @brief Selected save slot index (7 = init, 0xFF = no save selected). */
extern s32 g_save_slot_index;
/** @brief Menu layout configuration flag byte (from SavedGameLayout.layout_flags). */
extern s32 g_layout_flag;
/** @brief Field-entry behavior flag (from SavedGameLayout.field_flags). Cleared in field-entry states. */
extern s32 g_field_entry_flag;
/** @brief Signed sub-mode byte from SavedGameLayout.sub_mode; -1 = unset. */
extern s32 g_layout_sub_mode;
/** @brief Index into g_music_track_table[] selecting the current music track. */
extern u16 g_music_track_index;

/*
 * Shared input / frame / script state.
 *
 * These globals live in the main executable's .bss (below the 0x80140000
 * overlay slot) and are read/written by multiple overlays (menu, gname,
 * gover, ...). They are declared here rather than per-overlay so the
 * overlays share a single definition.
 */

/** @brief Global frame counter, advanced once per rendered frame. */
extern s32 g_frame_counter;

/** @brief Base of the primitive-rect scratch buffer (stride 0x4A0 per record). */
extern u8 g_prim_rect_buf[];

/** @brief Number of armor stat values stored in a golem or pet record. */
#define COMPANION_STAT_COUNT 4
/** @brief Number of golem records in the pad context. */
#define GOLEM_RECORD_COUNT 3
/** @brief Number of pet records in the pad context. */
#define PET_RECORD_COUNT 5
/** @brief Mask of PadContext.companion_info selecting the companion's character type. */
#define COMPANION_KIND_MASK 0x7F
/** @brief Companion character type of a golem. */
#define COMPANION_KIND_GOLEM 4

/** @brief Mask of PadContext.golem_count selecting the number of golems. */
#define GOLEM_COUNT_MASK 0xF

/** @brief Capacity of the packed logic-block table in the pad context. */
#define LOGIC_BLOCK_CAPACITY 40

/** @brief Logic-type value marking a logic block that no logic type owns. */
#define LOGIC_BLOCK_UNASSIGNED 3

/** @brief Packed logic-block word stored in PadContext.logic_blocks. */
typedef union
{
    u32 word;
    struct
    {
        u32 logic_type : 2;    /**< Owning logic type; LOGIC_BLOCK_UNASSIGNED when free. */
        u32 id : 6;            /**< Logic-block type index. */
        u32 quantity : 4;      /**< Level shown after the name; zero hides it. */
        u32 shape : 4;         /**< Index of the block's composite-icon layout. */
        u32 placed : 1;        /**< Set while the block is placed on its golem group's grid. */
        u32 rotation : 2;      /**< Placed rotation, 0-3. */
        s32 grid_x : 5;        /**< Placed grid column, relative to the layout origin. */
        s32 grid_y : 5;        /**< Placed grid row, relative to the layout origin. */
        u32 unknown_bits : 3;
    } f;
} LogicBlock;

#define PLAYER_EQUIPMENT_SLOT_COUNT 4
#define INVENTORY_RECORD_COUNT 100
#define ITEM_TYPE_COUNT 256

/** @brief Item kind (bits 9:8) of a packed InventoryAttributes word: weapon, armor or instrument. */
#define INVENTORY_KIND(packed) (((packed) >> 8) & 0x3)
/** @brief Category (bits 15:10) of a packed InventoryAttributes word: the weapon, armor or instrument type. */
#define INVENTORY_CATEGORY(packed) (((packed) >> 10) & 0x3F)
/** @brief Mask of InventoryAttributes.halves.high selecting the material (an item name index). */
#define INVENTORY_MATERIAL_MASK 0x3F

/** @brief Packed item kind, category, and name index of an inventory record. */
typedef union
{
    u32 packed;
    struct
    {
        u16 low;  /**< Bits 9:8 select the item kind; bits 15:10 select its category. */
        u16 high; /**< Low six bits select an entry in the item-name table. */
    } halves;
} InventoryAttributes;

/** @brief Length of the encoded name at the start of an InventoryRecord. */
#define INVENTORY_NAME_LENGTH 0x14

/** @brief One 0x40-byte equipment/inventory record. */
typedef struct
{
    u8 name[INVENTORY_NAME_LENGTH]; /**< Encoded item name; an empty name marks a free slot. */
    InventoryAttributes attributes;
    u8 unknown_0x18[0xC];
    union
    {
        u16 values[4]; /**< Weapon power (values[0]) or the four armor stats. */
        u8 bytes[8];   /**< Instrument: spell group, spell index within the group, power. */
    } stats;
    u8 unknown_0x2c[8];
    s32 price;
    u8 unknown_0x38[8];
} InventoryRecord;

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
    s32 palette;     /**< Portrait and sprite palette, 0-31; MENU also names it as the golem's color. */
    InventoryRecord source_items[GOLEM_SOURCE_ITEM_COUNT]; /**< Items the golem was built from; an empty name ends the list. */
} GolemRecord;

/** @brief Status flags word of a PetRecord. */
typedef struct
{
    u32 unknown_bits : 30;
    u32 grazing : 1; /**< The pet is left grazing at the ranch and cannot join the party. */
    u32 egg : 1;     /**< The pet has not hatched yet. */
} PetStatusFlags;

/** @brief Pet record: a monster raised at the ranch, or an egg waiting to hatch. */
typedef struct
{
    u8 name[0x15];
    u8 species;     /**< Monster species; also picks the pet's portrait. */
    u8 egg_species; /**< Species of the egg's portrait while the pet is still an egg. */
    u8 unknown_0x17;
    u8 level;
    u8 unknown_0x19[0x1C - 0x19];
    u16 secondary_value; /**< Value shown in the second stat column of the GOSUB roster. */
    u16 primary_value;   /**< Value shown in the first stat column of the GOSUB roster. */
    u16 stats[COMPANION_STAT_COUNT];
    u8 unknown_0x28[0x42 - 0x28];
    u16 hatch_counter; /**< Egg hatching countdown; low values mean the egg is nearly ready. */
    PetStatusFlags status;
    u8 unknown_0x48[0x60 - 0x48];
} PetRecord;

/**
 * @brief Controller/pad context object (partial layout).
 *
 * Only fields used by currently decompiled menu/controller paths are mapped;
 * the rest of the structure remains opaque.
 */
typedef struct
{
    u8  _pad000[0x28];          /**< 0x000: not yet mapped. */
    u32 menu_option_flags;      /**< 0x028: menu audio/vibration option bits. */
    u32 money;
    u8  _pad030[0x204 - 0x30];
    u32 known_save_flags;       /**< One bit per product code of a known game save found on a memory card. */
    u8  _pad208[0x640 - 0x208];
    InventoryRecord player_equipment[PLAYER_EQUIPMENT_SLOT_COUNT];
    u8  _pad740[0x840 - 0x740];
    u8  inject_enable;          /**< 0x840: non-zero allows input injection. */
    u8  _pad841[0x858 - 0x841]; /**< 0x841: not yet mapped. */
    u32 inject_flags;       /**< 0x858: bit 0x80 enables input injection. */
    u8  _pad85C[0x234];          /**< 0x85C: not yet mapped. */
    u8  companion_name[0x18];    /**< 0xA90: name of the party companion (character slot 2); GNAME edits it. */
    u32 companion_info;          /**< 0xAA8: bits 0-6 the companion's character type; byte 1 a golem's class. */
    u8  _padAAC[0xCE0 - 0xAAC];
    InventoryRecord inventory[INVENTORY_RECORD_COUNT];
    u8  item_counts[ITEM_TYPE_COUNT];
    u8  _pad26E0[0x29D4 - 0x26E0];
    u8  golem_count;              /**< 0x29D4: low nibble the number of golems; high nibble the golem that last left the party. */
    u8  golems_created;           /**< 0x29D5: golems created so far, saturating at 200. */
    u8  logic_block_count;        /**< Number of used entries in @c logic_blocks. */
    s8  joined_golem;             /**< 0x29D7: golem record in the party, or 3 for none. */
    u8  golem_order[GOLEM_RECORD_COUNT]; /**< Display order of @c golem_records; values >= 3 are empty. */
    u8  golem_display_order;      /**< 0x29DB: golem slot shown at each display position, three 2-bit fields. */
    LogicBlock logic_blocks[LOGIC_BLOCK_CAPACITY];
    u8  _pad2A7C[0x2B0C - 0x2A7C];
    GolemRecord golem_records[GOLEM_RECORD_COUNT];
    u32 joined_pet;               /**< 0x2EF0: pet record in the party. */
    PetRecord pet_records[PET_RECORD_COUNT];
} PadContext;

/** @brief Pointer to the controller/pad context object. */
extern PadContext* g_pad_ctx;

/** @brief Forward selection steps applied when menu scripts 1-3 terminate. */
extern s32 g_script_repeat_count;

/** @brief Active script id; selects a row of @c g_script_table (0 = none). */
extern s32 g_active_script;

/** @brief Current frame's debounced pad button bitmask. */
extern s32 g_pad_input;

/** @brief Extra button bits OR'd into @c g_pad_input when the pad context requests it. */
extern s32 g_pad_input_inject;

/** @brief Initialize the game and dispatch overlays forever. */
void main_game_loop(void);

void field_scene_reset(u32);
void field_draw_frame(s32, s32, s32, s32);
void field_clear_node_accumulators(s32, s32);
void field_restore_entry_music(void);
#endif
