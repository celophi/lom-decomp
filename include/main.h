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

/** @brief Number of stat values stored in each saved-history record. */
#define HISTORY_RECORD_STAT_COUNT 4
/** @brief Number of large saved-history records in the pad context. */
#define LARGE_HISTORY_RECORD_COUNT 3
/** @brief Number of small saved-history records in the pad context. */
#define SMALL_HISTORY_RECORD_COUNT 5
/** @brief Capacity of the packed logic-block table in the pad context. */
#define LOGIC_BLOCK_CAPACITY 40

/** @brief Packed logic-block word stored in PadContext.logic_blocks. */
typedef union
{
    u32 word;
    struct
    {
        u32 ready : 2;         /**< Both bits are set when a combination creates the block. */
        u32 id : 6;            /**< Logic-block type index. */
        u32 quantity : 4;      /**< Level shown after the name; zero hides it. */
        u32 variant : 4;
        u32 unknown_bit16 : 1;
        u32 unknown_bits : 15;
    } f;
} LogicBlock;

/** @brief Large saved-history record with a leading encoded name. */
typedef struct
{
    u8 name[0x15];
    u8 unknown_0x15;
    u16 secondary_value; /**< Value shown in the second stat column of the GOSUB roster. */
    u16 primary_value;   /**< Value shown in the first stat column of the GOSUB roster. */
    u16 stats[HISTORY_RECORD_STAT_COUNT];
    u8 unknown_0x22[0x44 - 0x22];
    u8 unknown_0x44; /**< Packed indices into two menu text tables. */
    u8 unknown_0x45;
    u8 unknown_0x46;
    u8 unknown_0x47;
    s32 unknown_0x48; /**< Index into a menu text table. */
    u8 unknown_0x4C[0x14C - 0x4C];
} LargeHistoryRecord;

/** @brief Compact saved-history record with a leading encoded name. */
typedef struct
{
    u8 name[0x15];
    u8 unknown_0x15;
    u8 unknown_0x16;
    u8 unknown_0x17;
    u8 unknown_0x18;
    u8 unknown_0x19[0x1C - 0x19];
    u16 secondary_value; /**< Value shown in the second stat column of the GOSUB roster. */
    u16 primary_value;   /**< Value shown in the first stat column of the GOSUB roster. */
    u16 stats[HISTORY_RECORD_STAT_COUNT];
    u8 unknown_0x28[0x42 - 0x28];
    u16 unknown_0x42;
    struct
    {
        u32 unknown_bits : 30;
        u32 selection_restricted : 1; /**< Restricts GOSUB selection of this record. */
        u32 selection_blocked : 1;    /**< Blocks GOSUB selection of this record. */
    } selection_flags;
    u8 unknown_0x48[0x60 - 0x48];
} SmallHistoryRecord;

#define PLAYER_EQUIPMENT_SLOT_COUNT 4
#define INVENTORY_RECORD_COUNT 100
#define ITEM_TYPE_COUNT 256

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

/** @brief One 0x40-byte equipment/inventory record. */
typedef struct
{
    u8 active; /**< Zero marks an empty slot. */
    u8 unknown_0x01[0x13];
    InventoryAttributes attributes;
    u8 unknown_0x18[0xC];
    union
    {
        u16 values[4]; /**< Equipment stats. */
        u8 bytes[8];   /**< Byte view used by kinds that store small fields here. */
    } stats;
    u8 unknown_0x2c[8];
    s32 price;
    u8 unknown_0x38[8];
} InventoryRecord;

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
    u8  _pad030[0x640 - 0x30];
    InventoryRecord player_equipment[PLAYER_EQUIPMENT_SLOT_COUNT];
    u8  _pad740[0x840 - 0x740];
    u8  inject_enable;          /**< 0x840: non-zero allows input injection. */
    u8  _pad841[0x858 - 0x841]; /**< 0x841: not yet mapped. */
    u32 inject_flags;       /**< 0x858: bit 0x80 enables input injection. */
    u8  _pad85C[0x234];          /**< 0x85C: not yet mapped. */
    u8  gname_name[0x18];        /**< 0xA90: name buffer edited by the GNAME overlay. */
    u32 unkAA8;
    u8  _padAAC[0xCE0 - 0xAAC];
    InventoryRecord inventory[INVENTORY_RECORD_COUNT];
    u8  item_counts[ITEM_TYPE_COUNT];
    u8  _pad26E0[0x29D6 - 0x26E0];
    u8  logic_block_count;        /**< Number of used entries in @c logic_blocks. */
    s8  large_history_index;      /**< 0x29D7: slot index into @c large_history_records. */
    u8  large_history_order[LARGE_HISTORY_RECORD_COUNT]; /**< Display order of @c large_history_records; values >= 3 are empty. */
    u8  _pad29DB;
    LogicBlock logic_blocks[LOGIC_BLOCK_CAPACITY];
    u8  _pad2A7C[0x2B0C - 0x2A7C];
    LargeHistoryRecord large_history_records[LARGE_HISTORY_RECORD_COUNT];
    u32 small_history_index;      /**< 0x2EF0: slot index into @c small_history_records. */
    SmallHistoryRecord small_history_records[SMALL_HISTORY_RECORD_COUNT];
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
