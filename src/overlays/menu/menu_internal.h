#ifndef LOM_MENU_INTERNAL_H
#define LOM_MENU_INTERNAL_H

#include "menu.h"
#include "display.h"
#include "controller.h"
#include "vector.h"

/* ----- Macros ----- */

/* Menu chrome CLUT ids produced by getClut for the rows uploaded in menu_upload_tim. */
#define MENU_CLUT_GRID_BASE 0x7C80
#define MENU_CLUT_GRID_ALT 0x7C81
#define MENU_CLUT_CORNER 0x7CCA
#define MENU_GRID_OT_INDEX 0x0F
#define MENU_OT_ENTRY_COUNT 16
#define MENU_GRID_ALT_CLUT_START 0x11
#define MENU_GRID_SPRITE_COUNT 0x1D
#define MENU_GRID_TEXTURE_WINDOW_SIZE 0xFF
#define MENU_GRID_TPAGE 5
#define MENU_SCROLL_ARROW_SIZE 16
#define MENU_SCROLL_ARROW_UV_UP 0x1080
#define MENU_SCROLL_ARROW_UV_DOWN 0x2080
#define MENU_SCROLL_ARROW_CLUT 0x7C86

#define MENU_SLOT_COUNT 4
#define MENU_SLOT_STATE_FREE 0
#define MENU_SLOT_STATE_OPENING 1
#define MENU_SLOT_STATE_OPEN 2
#define MENU_SLOT_STATE_CLOSING 3
#define MENU_SLOT_OT_INDEX_SHIFT 25
#define MENU_SLOT_OT_INDEX_CLEAR_MASK 0x01FFFFFF
#define MENU_WINDOW_TRANSITION_STEPS 12
#define MENU_WINDOW_MIN_WIDTH 0x20
#define MENU_WINDOW_MIN_HEIGHT 0x10

/** Size of the embedded TIM image block, including its block header. */
#define MENU_TIM_IMAGE_BLOCK_SIZE 0x800C

/* Packed menu-chrome UV origins: high byte V, low byte U. */
#define MENU_TW_CORNER_TL 0x70D0
#define MENU_TW_CORNER_TR 0x70D8
#define MENU_TW_CORNER_BL 0x78D0
#define MENU_TW_CORNER_BR 0x78D8
#define MENU_TW_EDGE_TOP 0x80D0
#define MENU_TW_EDGE_BOT 0x88D0
#define MENU_TW_EDGE_LEFT 0x90D0
#define MENU_TW_EDGE_RIGHT 0x90D8
#define MENU_TW_FILL 0xA0A0
#define MENU_WINDOW_CORNER_SIZE 8
#define MENU_WINDOW_FILL_TILE_SIZE 0x60
#define MENU_WINDOW_EDGE_TEXTURE_LONG_SIDE 16
#define MENU_WINDOW_EDGE_TEXTURE_SHORT_SIDE 8
#define MENU_LABEL_BUFFER_SIZE 16
/** Prefix range for encoded text units with one following byte. */
#define MENU_TEXT_PREFIX_FIRST 0x19U
#define MENU_TEXT_PREFIX_COUNT 7U

/* VRAM placement for each slot's cursor strip and content texture block. */
#define PRIM_CURSOR_STRIP_VRAM_X 0x110  /* 272  - VRAM column                */
#define PRIM_CURSOR_STRIP_VRAM_Y0 0x1D8 /* 472  - VRAM row for slot 0        */
#define PRIM_CURSOR_STRIP_W 0x10        /* 16 halfwords wide                 */
#define PRIM_CURSOR_STRIP_H 1           /* 1 scanline tall                   */
#define PRIM_CONTENT_VRAM_X 0x3F4       /* 1012 - VRAM column for slots 0, 1 */
#define PRIM_CONTENT_VRAM_X2 0x3E8      /* 1000 - VRAM column for slot 2     */
#define PRIM_CONTENT_VRAM_Y0 0x120      /* 288  - VRAM row for slot 0        */
#define PRIM_CONTENT_VRAM_Y1 0x150      /* 336  - VRAM row for slots 1, 2    */
#define PRIM_CONTENT_W 0xC              /* 12 halfwords wide                 */
#define PRIM_CONTENT_H 0x30             /* 48 scanlines tall                 */
#define PRIM_SLOT_COUNT 3
#define PRIM_CURSOR_STRIP_BYTE_SIZE (PRIM_CURSOR_STRIP_W * PRIM_CURSOR_STRIP_H * sizeof(u16))
#define PRIM_CONTENT_BYTE_SIZE (PRIM_CONTENT_W * PRIM_CONTENT_H * sizeof(u16))
#define PRIM_CONTENT_BUF_OFFSET PRIM_CURSOR_STRIP_BYTE_SIZE
#define PRIM_SLOT_BYTE_SIZE (PRIM_CURSOR_STRIP_BYTE_SIZE + PRIM_CONTENT_BYTE_SIZE)

/*
 * Node / scroll / layout constants
 */
/** @brief Total number of nodes in g_menu_nodes[]. */
#define MENU_NODE_COUNT 0x2C
/** @brief MenuNode::u2 flag indicating that the node participates in layout. */
#define MENU_NODE_FLAG_ACTIVE 0x01
/** @brief MenuNode::u2 flag indicating that the node's children are visible. */
#define MENU_NODE_FLAG_EXPANDED 0x02
/** @brief Bits [14:8] of idx_nav.nav_x_packed: the 7-bit column (nav_x) field. */
#define MENU_NAV_X_MASK 0x7F00
/** @brief Clears bits [14:8] of idx_nav.nav_x_packed (inverse of MENU_NAV_X_MASK). */
#define MENU_NAV_X_CLEAR 0x80FF
/** @brief Bit 15 of idx_nav.nav_x_packed: bit 0 of the 9-bit nav cursor Y. */
#define MENU_NAV_Y0_BIT 0x8000
/** @brief Bit 15 of u8_u.nav_y_packed: bit 0 of the 9-bit layout Y position. */
#define MENU_LAYOUT_Y0_BIT 0x8000
/** @brief Maximum number of children per menu node. */
#define MENU_MAX_CHILDREN 4
/** @brief No layout interpolation remains; copy the target position directly. */
#define MENU_NODE_LAYOUT_IDLE 0
/** @brief Number of render updates used to interpolate a newly assigned row. */
#define MENU_NODE_LAYOUT_STEPS 4
/** @brief Index of the "browse all items" root node; Circle navigates here. */
#define MENU_NODE_BROWSE_ALL 0x20
/** @brief Sentinel value meaning "none" for parent_idx, content_id, and child indices. */
#define MENU_NONE 0xFF
/** @brief Vertical spacing per node in scroll-position units (19 px). */
#define MENU_ROW_HEIGHT 0x13
/** @brief Full visible scroll-viewport height: 9 rows * MENU_ROW_HEIGHT (171 px). */
#define MENU_VIEW_HEIGHT 0xAB
/** @brief Vertical spacing encoded in each packed item-navigation entry. */
#define MENU_ITEM_NAV_POSITION_STRIDE 0x10
/** @brief Bits [13:0] containing an item's vertical navigation position. */
#define MENU_ITEM_NAV_POSITION_MASK 0x3FFF
/** @brief Nine-bit mask for packed previous and next item indices. */
#define MENU_ITEM_NAV_INDEX_MASK 0x1FF
#define MENU_ITEM_NAV_PREVIOUS_SHIFT 14
#define MENU_ITEM_NAV_NEXT_SHIFT 23
#define MENU_LIST_COUNT_SHIFT 16
#define MENU_LIST_COUNT_MASK (MENU_ITEM_NAV_INDEX_MASK << MENU_LIST_COUNT_SHIFT)
/** @brief Number of equipment slots associated with one character. */
#define MENU_EQUIPMENT_SLOT_COUNT 4
/** @brief First menu subtype corresponding to an equipment slot. */
#define MENU_EQUIPMENT_SUBTYPE_BASE 7
/** @brief Number of 0x40-byte records in the inventory item table. */
#define MENU_ITEM_TABLE_COUNT 100
/** @brief Byte offset of the inventory item table within g_pad_ctx. */
#define MENU_ITEM_TABLE_OFFSET 0xCE0
/** @brief Size in bytes of one inventory/equipment item record. */
#define MENU_ITEM_RECORD_SIZE 0x40
/** @brief Size in bytes of one character's save/menu state block. */
#define MENU_CHARACTER_BLOCK_SIZE 0x250
/** @brief Offset of the character equipment-area header within its state block. */
#define MENU_CHARACTER_RECORD_OFFSET 0x5F0
/** Byte offset of the eight packed values displayed in the character panel. */
#define MENU_CHARACTER_PACKED_VALUES_OFFSET 0x620
/** @brief Offset of the first equipped-item record within the equipment area. */
#define MENU_CHARACTER_ITEMS_OFFSET 0x50
/** @brief Mask for the two-bit item-kind field in MenuItemEntry::attributes. */
#define MENU_ITEM_KIND_MASK 0x300
#define MENU_ITEM_KIND_SHIFT 8
/** @brief Shift and mask for the six-bit item-category field in MenuItemEntry::attributes. */
#define MENU_ITEM_CATEGORY_SHIFT 10
#define MENU_ITEM_CATEGORY_MASK 0x3F
/** @brief Clears the packed previous-index field while preserving all other bits. */
#define MENU_ITEM_NAV_PREVIOUS_CLEAR_MASK 0xFF803FFF
/** @brief Clears the packed next-index field while preserving all other bits. */
#define MENU_ITEM_NAV_NEXT_CLEAR_MASK 0x007FFFFF
/** @brief Offset of the 12-row spell-presence bitmap in g_pad_ctx. */
#define MENU_SPELL_GRID_OFFSET 0x60
#define MENU_SPELL_GRID_ROW_COUNT 12
#define MENU_SPELL_GRID_COLUMN_COUNT 8
/** @brief Offset of the packed 16-row equipment availability grid in g_pad_ctx. */
#define MENU_EQUIPMENT_GRID_OFFSET 0x104
#define MENU_EQUIPMENT_GRID_ROW_COUNT 16
#define MENU_EQUIPMENT_GRID_COLUMN_COUNT 8
#define MENU_EQUIPMENT_GRID_ENTRY_MASK 0xF
#define MENU_EQUIPMENT_GRID_FIRST_VALID 2
/** @brief Offset and size of the 256-byte key-item quantity table in g_pad_ctx. */
#define MENU_KEY_ITEM_TABLE_OFFSET 0x25E0
#define MENU_KEY_ITEM_TABLE_COUNT 256
#define MENU_KEY_ITEM_SENTINEL_INDEX (MENU_KEY_ITEM_TABLE_COUNT - 1)
/** @brief Offset of the four pending equipment-status bytes for a character. */
#define MENU_PENDING_STATUS_OFFSET 0x60C
/** @brief Value marking an unused pending equipment-status byte. */
#define MENU_PENDING_STATUS_NONE 0xFF
/** @brief Pending-status entries with this bit set are preserved. */
#define MENU_PENDING_STATUS_PRESERVE 0x80
/** @brief Minimum Y for g_content_cursor_y within the content sub-window (12 px). */
#define MENU_CURSOR_Y_MIN 0x0C
/** @brief Maximum Y for g_content_cursor_y within the content sub-window (163 px). */
#define MENU_CURSOR_Y_MAX 0xA3
/** @brief Horizontal inset from a node's navigation column to the content cursor. */
#define MENU_CONTENT_CURSOR_X_OFFSET 8
/** @brief Frames to suppress cursor highlight after opening a content view. */
#define MENU_CURSOR_REVEAL_DELAY 5
/** @brief Extracts the 9-bit screen X coordinate from MenuContentItem::packed_x. */
#define MENU_CONTENT_X_MASK 0x1FF
/** @brief Shift and mask for the three-bit text/style field in MenuContentItem::packed_x. */
#define MENU_CONTENT_STYLE_SHIFT 9
#define MENU_CONTENT_STYLE_MASK 0x7
/** @brief Shift of the high-nibble content type in MenuContentItem::packed_x. */
#define MENU_CONTENT_TYPE_SHIFT 12
/** @brief Converts a content item's Y coordinate to the viewport origin. */
#define MENU_CONTENT_VIEW_Y_OFFSET 8
/** @brief High-nibble mask selecting a content item's action class. */
#define MENU_CONTENT_ITEM_TYPE_MASK 0xF000
/** @brief Content item class that opens a nested submenu. */
#define MENU_CONTENT_ITEM_TYPE_SUBMENU 0x5000
/** @brief Content item class that dispatches a menu action code. */
#define MENU_CONTENT_ITEM_TYPE_ACTION 0xF000
/** @brief Packed bits marking a content item as the active selectable entry. */
#define MENU_CONTENT_ITEM_ACTIVE_MASK 0x0E00
/** @brief g_menu_redraw_state: navigation key pressed, scroll position adjusted. */
#define MENU_REDRAW_NAVIGATE 6
/** @brief g_menu_redraw_state: layout pass completed (position change or first run). */
#define MENU_REDRAW_LAYOUT 8
/** @brief g_pad_ctx->inject_flags bit enabling injected menu input. */
#define MENU_PAD_INJECT_ENABLED 0x80
/** @brief Enables vibration feedback in g_pad_ctx->menu_option_flags. */
#define MENU_OPTION_VIBRATION_ENABLED 0x01
/** @brief Enables menu audio in g_pad_ctx->menu_option_flags. */
#define MENU_OPTION_AUDIO_ENABLED 0x02
/** @brief Number of directional links stored by each content item. */
#define MENU_CONTENT_DIRECTION_COUNT 4
/** @brief Ordering-table entry used as the menu frame's list head. */
#define MENU_FRAME_OT_INDEX 13
/** @brief Vertical offset of the node-tree clipping region within a draw page. */
#define MENU_TREE_DRAW_Y_OFFSET 12
#define MENU_TREE_DRAW_X 15
#define MENU_TREE_DRAW_WIDTH 36
#define MENU_TREE_DRAW_HEIGHT 170

/** @brief Base CLUT row used by menu icon sprites. */
#define MENU_ICON_CLUT_Y_BASE 0x1F2

/* Sound-effect ids passed to menu_play_se; volume is always MENU_SE_VOLUME. */
/** @brief Scroll navigation sound (D-up / D-down / Circle to scroll). */
#define MENU_SE_NAVIGATE 0x7D
/** @brief Open / select sound (Circle or D-right to enter a node). */
#define MENU_SE_SELECT 0x7E
/** @brief Close / cancel sound (Circle while at MENU_NODE_BROWSE_ALL). */
#define MENU_SE_CLOSE 0x7F
/** @brief Invalid-action sound. */
#define MENU_SE_ERROR 0x78
/** @brief Full volume level for all menu sound effects (128). */
#define MENU_SE_VOLUME 0x80

/** @brief Byte offset of the learned Special Technique bitsets in PadContext. */
#define MENU_SPECIAL_TECHNIQUE_FLAGS_OFFSET 0x34
/** @brief Number of Special Technique category bitsets. */
#define MENU_SPECIAL_TECHNIQUE_GROUP_COUNT 11
/** @brief Number of Special Technique bits and names in each category. */
#define MENU_SPECIAL_TECHNIQUES_PER_GROUP 24

/** @brief Byte offset of the ability-record table in PadContext. */
#define MENU_ABILITY_TABLE_OFFSET 0x2F0
/** @brief Number of fixed-size ability records. */
#define MENU_ABILITY_COUNT 64
/** @brief Ability is learned and appears in the list. */
#define MENU_ABILITY_FLAG_LEARNED 0x01
/** @brief Ability displays the auxiliary list icon. */
#define MENU_ABILITY_FLAG_SHOW_ICON 0x02

/* Type-view and fixed-address helpers. */
#define MENU_CONTROLLER_ACTUATORS ((MenuControllerActuatorState*)0x801ED600)

/* ----- Types ----- */

/* Rendering and window types. */

/** @brief VRAM destinations for the image and CLUT blocks in the menu TIM asset. */
typedef struct
{
    s16 texture_x;
    s16 texture_y;
    s16 clut_x;
    s16 clut_y;
} MenuTimVramLayout;

/**
 * @brief Container holding the menu TIM and its additional CLUT.
 *
 * The two offsets are stored in the asset header; the embedded TIM begins
 * immediately after the header, and its variable-sized image block follows
 * the first CLUT.
 */
typedef struct
{
    u32 entry_count;
    u32 tim_offset;
    u32 second_clut_offset;
    Tim tim;
    u8 image_block[MENU_TIM_IMAGE_BLOCK_SIZE];
    u16 second_clut[CLUT_ENTRY_COUNT];
} MenuTimAsset;


typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} MenuRect;

typedef struct
{
    u16 x;
    u16 y;
    s16 w;
    s16 h;
} MenuRectU16;

/** @brief 2-D screen coordinate (pixels). */
typedef struct
{
    s16 x; /**< Screen X. */
    s16 y; /**< Screen Y. */
} ScreenPos;

/**
 * @brief Two-byte offset into a paged string table.
 *
 * @c entry is the character/entry index within the page; @c page is the page
 * index. Together they form a string pointer via
 * @c entry + ((page << 8) + base_ptr). See @ref menu_draw_label.
 */
typedef struct
{
    u8 entry; /**< Entry index within the page. */
    u8 page;  /**< Page index. */
} StringTableOffset;

/** @brief Known table indices in the menu text-resource directory. */
typedef enum
{
    MENU_TEXT_GENERAL = 0,
    MENU_TEXT_MESSAGES = 1,
    MENU_TEXT_SPELL_NAMES = 3,
    MENU_TEXT_ABILITY_HELP = 4,
    MENU_TEXT_ABILITY_NAMES = 5,
    MENU_TEXT_TECHNIQUE_HELP = 6,
    MENU_TEXT_TECHNIQUE_NAMES = 7,
    MENU_TEXT_KEY_ITEM_HELP = 10,
    MENU_TEXT_KEY_ITEM_NAMES = 11,
    MENU_TEXT_SPELL_HELP = 14,
    MENU_TEXT_CATEGORY_ENTRY_HELP = 15,
    MENU_TEXT_ITEM_CATEGORY_HELP = 25,
    MENU_TEXT_EQUIPMENT_NAMES = 26,
    MENU_TEXT_EQUIPMENT_HELP = 32,
    MENU_TEXT_TABLE_COUNT = 34
} MenuTextTable;

/** @brief Text-resource directory with byte offsets relative to the resource base. */
typedef struct
{
    u32 table_count;
    s32 table_offsets[MENU_TEXT_TABLE_COUNT];
} MenuTextResources;

/* Navigation and content types. */

/**
 * @brief One node in the hierarchical menu navigation tree.
 */
typedef struct
{
    u8 label_id; /**< Index into the menu label string table. */
    u8 layout_frames_remaining; /**< Updates left to reach the target Y; zero snaps to the target. */
    union
    {
        u16 unk2; /**< Full 16-bit word: low byte = flags, high byte = parent_idx. */
        struct
        {
            u8 flags;      /**< Bit 0: node active/enabled in layout. Bit 1: node expanded (children shown). */
            u8 parent_idx; /**< Index of parent node in g_menu_nodes, or MENU_NONE (0xFF) for root nodes. */
        } s;
    } u2;
    u8 icon_id;    /**< Sprite/icon definition passed to menu_emit_icon_sprite. */
    u8 content_id; /**< Passed to the content-open function; 0xFF = no content. */
    union
    {
        u16 nav_x_packed; /**< Raw word; high byte = nav_x, low byte = self_idx. */
        struct
        {
            u8 self_idx; /**< This node's own index in g_menu_nodes (used as content-table key). */
            u8 nav_x;    /**< Bits 0-6: nav cursor X = (nav_x & 0x7F) + 8. Bit 7: bit 0 of nav cursor Y. */
        } s;
    } idx_nav;
    union
    {
        u16 nav_y_packed; /**< Raw word; high byte = layout_x_y0, low byte = nav_y_hi. */
        struct
        {
            u8 nav_y_hi;     /**< Bits 1-8 of the 9-bit nav cursor Y: reconstruct as (nav_y_hi<<1)|(nav_x>>7). */
            u8 layout_x_y0; /**< Bits 6:0 = layout X; bit 7 = bit 0 of layout Y. */
        } s;
    } u8_u;
    union
    {
        u16 layout_child_packed; /**< Low byte: layout Y bits 8:1; high byte: first child index. */
        struct
        {
            u8 layout_y_hi;                 /**< Layout Y bits 8:1. */
            u8 children[MENU_MAX_CHILDREN]; /**< Child indices, terminated by MENU_NONE. */
            u8 unknown_0xf;
        } s;
    } layout;
} MenuNode;

/**
 * @brief Packed field view of a menu node's current and target positions.
 * The two coordinate pairs follow the content-table index without byte padding.
 */
typedef struct
{
    u8 label_id;
    u8 layout_frames_remaining;
    u16 flags_and_parent;
    u8 icon_id;
    u8 content_id;
    u32 self_idx:8;
    u32 nav_x:7;
    u32 nav_y:9;
    u32 layout_x:7;
    u32 layout_y:9;
    u8 children[MENU_MAX_CHILDREN];
    u8 unknown_0xf;
} __attribute__((packed, aligned(2))) MenuNodeCoordinateView;

typedef struct
{
    u16 unk0;
    u8 pad2[0x266];
    u16 unk268;
    u8 unk26A;
    u8 unk26B;
} Struct_D_800FD818;

typedef struct
{
    u16 packed_x; /**< Bottom 9 bits = X screen position; upper bits unknown. */
    u8 y;         /**< Y position; caller subtracts 8 when using as display offset. */
    u8 action_type; /**< Content/action selector interpreted by the active scene. */
    u8 params[4]; /**< Remaining content-specific parameter bytes. */
} MenuContentItem;

typedef struct
{
    u16 x : 9;
    u16 upper : 7;
    u8 y;
    u8 pad[5];
} MenuContentItemBits;

/**
 * @brief Packed source fields for one sprite emitted by @ref menu_build_grid.
 */
typedef struct
{
    u16 uv; /**< Packed texture coordinates: low byte U, high byte V. */
    u16 _pad2;
    u32 packed_xy; /**< Packed signed screen coordinates: low half X, high half Y. */
    u32 packed_wh; /**< Packed signed dimensions: low half width, high half height. */
} MenuGridSpriteDef;

/** Partial view of one controller port through its large-motor command. */
typedef struct
{
    u8 padding0[0x90];
    u8 small_motor_command;
    u8 padding91;
    u8 large_motor_command;
    u8 padding93[0x1B];
} MenuControllerActuatorPort;

/** Fixed two-port controller state at 0x801ED600. */
typedef struct
{
    MenuControllerActuatorPort ports[2];
} MenuControllerActuatorState;


typedef enum
{
    MENU_CURSOR_MODE_NODE_TREE = 0,
    MENU_CURSOR_MODE_CONTENT = 1,
    MENU_CURSOR_MODE_CONTENT_EXIT = 2,
} MenuCursorMode;

/* Text, list, and equipment types. */

typedef enum
{
    MENU_TEXT_ALIGN_LEFT = 0,
    MENU_TEXT_ALIGN_RIGHT = 1,
    MENU_TEXT_ALIGN_CENTER = 2,
} MenuTextAlignment;

/**
 * @brief Marker shown while selecting two party slots to swap.
 */
typedef struct
{
    u8 x; /**< Screen X byte of the held-party-slot marker. */
    u8 y; /**< Screen Y byte of the held-party-slot marker. */
    u8 pad2;
    u8 selected_idx; /**< Content item index being held, or MENU_NONE. */
} PartySortMarker;

/** @brief Pad-context view exposing the party-sort order table. */
typedef struct
{
    u8 pad000[0x638]; /**< Unmapped head of the pad-context record. */
    u8 order[8]; /**< Eight party slot indices in display order. */
} MenuPartyOrder;

typedef struct
{
    u8 u_coord; /**< Texture U coordinate. */
    u8 v_coord; /**< Texture V coordinate. */
    u8 w;       /**< Sprite width in pixels. */
    u8 h;       /**< Sprite height in pixels. */
} MenuIconSpriteInfo;

/** @brief Partial menu-slot view used by scrollable list callbacks. */
typedef struct
{
    u8 active; /**< MENU_SLOT_STATE_* lifecycle state. */
    u8 index;
    u8 anim_frame;
    u8 has_title;
    MenuListNavigation navigation;
    u16 base_x;     /**< Widget screen base X. */
    u16 base_y;     /**< Widget screen base Y. */
    s16 viewport_w; /**< Visible list width. */
    s16 viewport_h; /**< Visible list height; also determines the fast-scroll step. */
    u16 scroll_x;   /**< Current X scroll offset. */
    u16 scroll_y;   /**< Current Y scroll offset. */
    s16 target_x;   /**< X scroll interpolation target. */
    s16 target_y;   /**< Y scroll interpolation target. */
    u8 lerp_steps;  /**< Remaining interpolation steps. */
} ScrollListState;

/** @brief Partial view of one 12-byte ability record. */
typedef struct
{
    u8 flags; /**< Learned/display state bits used by the menu. */
    u8 pad01[0xB];
} MenuAbilityEntry;

/** @brief Packed eight-nibble property field stored in an item record. */
typedef union
{
    u32 packed;       /**< All eight four-bit properties packed into one word. */
    u8 bytes[4];      /**< Byte view used by selectors that only need one pair of nibbles. */
    u16 halfwords[2]; /**< Halfword view used by the middle property pair. */
} MenuItemNibbleField;

/** @brief Packed item classification, name index, and flags. */
typedef union
{
    u32 packed;
    struct
    {
        u16 low;  /**< Bits 9:8 select the item kind; bits 15:10 select its category. */
        u16 high; /**< Low six bits select an entry in the item-name table. */
    } halves;
} MenuItemAttributes;

/** @brief One 0x40-byte inventory/equipment item record. */
typedef struct
{
    u8 active; /**< Zero marks an empty slot. */
    u8 pad01[0x13];
    MenuItemAttributes attributes;
    MenuItemNibbleField display_nibbles; /**< Eight packed four-bit values displayed by the item-detail scene. */
    MenuItemNibbleField nibbles;         /**< Eight packed four-bit item properties used by comparison helpers. */
    u8 pad20[4];
    u16 stat_values[4];    /**< Equipment values used when ranking candidates. */
    u8 effect_flags[0x14]; /**< Per-item effect and ability flag bytes. */
} MenuItemEntry;

/** @brief Level byte and 24-bit experience counter stored in one character word. */
typedef union
{
    u32 packed;
    struct
    {
        u8 level;
        u8 experience_bytes[3];
    } fields;
} MenuCharacterProgression;

/** @brief Partial character record containing progression data and equipped items. */
typedef struct
{
    u8 unknown_0x00[0x20];
    MenuCharacterProgression progression;
    u16 unknown_0x24;
    u8 unknown_0x26[0x48 - 0x26];
    u8 unknown_0x48[8];
    MenuItemEntry items[MENU_EQUIPMENT_SLOT_COUNT];
} MenuCharacterRecord;

/**
 * @brief One-byte selector for a stored character name or a resource-table name.
 * @note A packed value of MENU_NONE denotes an empty selection.
 */
typedef union
{
    u8 packed;
    struct
    {
        u32 index : 7;
        u32 stored_name : 1;
    } __attribute__((packed)) fields;
} MenuNameSelection;

/* ----- Forward Declarations ----- */

void menu_init(void);
void menu_tick(RenderContext* render_ctx);
void menu_build_grid(RenderContext* render_ctx);
void menu_set_active_node(void);
void menu_snap_view_to_cursor(void);
void menu_reset_content_view(void);
void menu_init_item_nav_entries(s32 count);
void menu_update_active_slot(void);
void menu_update_slots(RenderContext* render_ctx);
void menu_draw_window_transition(RenderContext* render_ctx, MenuSlot* slot, s32 cursor_enable);
void menu_draw_window(MenuSlot* slot, RenderContext* render_ctx, MenuRect* rect, ScreenPos* view_origin, s32 cursor_enable);
SPRT* menu_emit_corner(SPRT* sprite, u_long* ot_entry, s32 x, s32 y, u32 uv);
SPRT* menu_fill_window_interior(SPRT* sprite, u_long* ot_entry, const MenuRectU16* rect, u32 uv);
u_long* menu_build_h_edge(u_long* packet_cursor, u_long* ot_entry, const MenuRectU16* rect, s32 texture_origin);
u_long* menu_build_v_edge(u_long* packet_cursor, u_long* ot_entry, const MenuRectU16* rect, s32 texture_origin);
u8* menu_draw_frame(u8* packet_cursor, u_long* ot_entry, s32 frame_parity, s32 allow_input);
void* menu_draw_scene_content(void* packet_cursor, s32* ot_entry);
void* menu_draw_content_cursor(void* prim_buf, s32* ot, s32 draw_label);
s32 menu_handle_node_input(void);
u32 menu_step_item_selection(s32 step);

void menu_upload_tim(const MenuTimVramLayout* layout);
s32 menu_draw_clamped_number(s32* ot_entry, s32 packet_cursor, s32 value, s32 format, Vec2s* origin, s32 style);

s32 menu_spell_list_callback(s32* ot, ScrollListState* state, s32 prim_buf, Vec2s* view_origin, int active);
s32 menu_equipment_grid_callback(s32* ot, ScrollListState* state, s32 prim_buf, Vec2s* view_origin, int active);
s32 menu_key_item_list_callback(s32* ot, ScrollListState* state, s32 prim_buf, Vec2s* view_origin, int active);
s32 menu_ability_list_callback(s32* ot, ScrollListState* state, s32 prim_buf, Vec2s* view_origin, int active);
s32 menu_message_callback(s32* ot, ScrollListState* state, s32 prim_buf, Vec2s* view_origin, int active);
s32 menu_two_line_message_callback(s32* ot, ScrollListState* state, s32 prim_buf, Vec2s* view_origin, int active);
void* menu_inventory_list_callback(s32* ot, ScrollListState* state, s32 prim_buf, Vec2s* view_origin, s32 active);
s32 menu_equipment_action_callback(s32* ot, ScrollListState* state, s32 prim_buf, Vec2s* view_origin, int active);
s32 menu_subtype_action_callback(s32* ot, ScrollListState* state, s32 prim_buf, Vec2s* view_origin, int active);

s32 menu_emit_cursor(s32, s32*, s32, s32, s32);
void* menu_emit_draw_mode_primitive(DR_TPAGE* draw_mode, s32* ot);
void* menu_emit_slot_scroll_arrows(SPRT* sprite, u_long* ot_entry, MenuSlot* slot);
void* menu_emit_tree_scroll_arrows(SPRT* sprite, s32* ot_entry);

void* menu_emit_sort_marker(void*, s32*, s16, s16);
s32 menu_item_is_nondefault(const MenuItemEntry*);

void* menu_draw_node_tree(void* prim_buf, s32* ot);
void* menu_draw_node_recursive(s32 node_index, void* prim_buf, s32* ot);

void* menu_emit_icon_sprite(void*, s32*, s32, s32, s32, s32, s32, s32, s32);

void scroll_list_update_target(ScrollListState*, u32*);

s32 menu_item_followup_callback(s32* ot, ScrollListState* state, s32 prim_buf, Vec2s* view_origin, int active);
s32 menu_equipment_compare_callback(s32* ot, ScrollListState* state, s32 prim_buf, Vec2s* view_origin, int active);
void func_800A8F8C();
void func_800A8FB4();
s32 func_800A9060();
s32 menu_special_technique_list_callback(s32* ot, ScrollListState* state, s32 prim_buf, Vec2s* view_origin, s32 active);
void menu_swap_item_records(MenuItemEntry*, MenuItemEntry*);

s32 func_800A88A0(s32 prim, s32* ot, void* glyph, s32 a3, s32 x, s32 y, s32 mode);
void menu_play_se(s32 sound_id, s32 volume);

s32 menu_stage_best_equipment_for_slot0(void);
s32 menu_stage_best_equipment_for_active_slot(void);

s32 scroll_list_draw(s32 prim_buf, s32* ot, ScrollListState* state, u32* entries, Vec2s* view_origin, int active);

MenuItemEntry* menu_find_best_equipment_for_slot0(void);
MenuItemEntry* menu_find_best_equipment_for_active_slot(void);
void menu_open_content_page(u32 content_id);

s32 menu_build_inventory_nav_entries(s32 item_kind);
s32 menu_build_equipment_nav_entries(void);
s32 menu_build_key_item_nav_entries(void);
s32 menu_build_ability_nav_entries(void);
extern s32 menu_stage_stack_shape(s32, s32, s32, s32, s32, s32) __attribute__((const));

/* ----- External Data ----- */

/* Core menu state and shared assets. */

extern s32 D_80042FB4;
extern u16 D_800F0C1C;
extern s32 D_80105AE0;
/** @brief Default scene content count encoded as count - 1 (value 3). */
extern u8 g_menu_default_content_count_minus_one;
extern u8 g_menu_content_item_counts[];
/** @brief Base of the four default MenuContentItem descriptors used by scene -1. */
extern MenuContentItem g_menu_default_content_items;
extern u8 D_80168659[];
extern u8 D_80168696[];
extern u8 D_801686B8[];
extern u8* D_80168C20;
extern u8* D_80168C24;
extern u8* D_80168C30;

/** @brief Optional help/description string drawn below the active menu content. */
extern s32 g_menu_help_text;
/** @brief Selects node-tree, content, or content-exit cursor handling. */
extern s32 g_menu_cursor_enable;
/** @brief Set non-zero by a content callback to abort @ref menu_draw_window early. */
extern s32 g_menu_draw_early_out;
/** @brief Base address of the menu double-buffered DRAWENV array. */
extern RenderContext* g_menu_draw_buf_base;
extern s32 D_80168C08;
/** @brief When non-zero, suppresses cursor highlight even on the active slot. */
extern s32 g_menu_suppress_cursor;
/** @brief Scene/language selector used in window title decoration layout switches. */
extern s32 g_menu_scene_type;

extern MenuNode g_menu_nodes[0x2C];
extern u8 g_menu_prev_node;
/** @brief Gate flag for menu_draw_content_cursor: 0 = draw empty slot, nonzero = full item render. */
extern s32 g_menu_content_ready;

/** @brief Comparison item addresses, or zero for an empty replacement. */
extern u32 g_item_slot_data[MENU_EQUIPMENT_SLOT_COUNT];
/** @brief Nonzero for slots with a pending comparison change. */
extern u8 g_item_slot_flags[MENU_EQUIPMENT_SLOT_COUNT];

/** @brief Pointer into g_pad_ctx item data for the current category; null = no items. */
extern s32 g_menu_item_ptr;
extern s32 g_menu_category0_item;
extern s32 g_menu_category1_item;
extern s32 g_menu_category2_item;
extern s32 g_menu_active_equipped_item;
extern s32 g_menu_saved_category0_item;
extern s32 g_menu_saved_category1_item;
/** @brief Packed circular navigation entries for item sub-pages. */
extern s32 g_menu_item_nav_entries[];
extern void* g_menu_equipment_base;
/** @brief Current interpolated vertical scroll position of the node tree. */
extern s32 g_menu_content_height;
extern s32 g_menu_scroll_pos;
extern s32 g_menu_redraw_state;
extern s32 g_menu_active_node;
/** @brief Equipped-item address saved when opening an item-action submenu. */
extern s32 g_menu_saved_equipment_item;
extern u8 g_menu_init_content_id;

extern Struct_D_800FD818 D_800FD818;
extern u16 D_800FDA80;
extern u16 D_800FDCE8;
/** @brief Ability-compatibility mask rebuilt before item content is loaded. */
extern s8 g_menu_ability_mask;
/** @brief Node index for the companion character's stat page (0x2B = companion present, 0xFF = none). */
extern u8 g_menu_companion_node;

/** @brief Shared string-directory entry 11: nonnegative label. */
extern StringTableOffset g_menu_label_key_a;
/** @brief Shared string-directory entry 16: negative label. */
extern StringTableOffset g_menu_label_key_b;

/** @brief Number of nodes in the linear navigation list. */
extern s32 g_menu_nav_count;
/** @brief Visible node IDs in drawing and navigation order. */
extern s32 g_menu_nav_nodes[];
/** @brief Y display coordinate for the content viewport origin. */
extern s32 g_content_view_y;
/** @brief Set to 1 to request an overlay/scene load at end of this input frame. */
extern s32 g_menu_load_request;
/** @brief Transition/result code paired with g_menu_load_request. */
extern s32 g_menu_transition_code;
/** @brief X pixel position of the content cursor within the content window. */
extern s32 g_content_cursor_x;
/** @brief Index of the item found by hit-test, or -1 if none. */
extern s32 g_menu_hit_item_idx;
/** @brief X display coordinate for the content viewport origin. */
extern s32 g_content_view_x;
/** @brief Y pixel position of the content cursor within the content window; clamped to [0xC, 0xA3]. */
extern s32 g_content_cursor_y;
/** @brief Default X/Y origin for the content viewport when no item hit-test position is available. */
extern struct
{
    s16 x;
    s16 y;
} g_menu_default_view_pos;
/** @brief Per-node table of MenuContentItem arrays, indexed by node.idx_nav.s.self_idx; NULL = no cursor data. */
extern MenuContentItem* g_menu_content_table[];
extern s32 g_menu_layout_end;

/* Content and action state. */

/** @brief Active character slot index: 0 = char slot 0 (node 0x1F), 1 = char slot 1 (node 0x2B). */
extern s32 g_menu_char_slot;

/** @brief Total page count for the current sub-menu view; g_script_repeat_last cycles in [0, g_menu_page_count-1]. */
extern s32 g_menu_page_count;
/** @brief Action sub-type of the most recently confirmed 0x5000 menu item; routes downstream handlers. */
extern s32 g_menu_active_subtype;
extern s8 D_801226F0;
/** @brief Storage for packed circular navigation entries used by scroll-list pages. */
extern u32 g_menu_scroll_nav_entries[];
extern s8 D_801226B8;
extern s32 D_801229F4;
extern s32 D_8011F424;

/** @brief Party-sort selection marker; selected_idx is MENU_NONE when inactive. */
extern PartySortMarker g_party_sort_marker;
extern u8 g_menu_content_group_ids[];
/** @brief Action code for each content group and one of its eight encoded item slots. */
extern u8 g_menu_content_action_codes[][8];
/** @brief First glyph/string pointer used by confirmation and status messages. */
extern void* g_menu_message_line1;
/** @brief Optional second glyph/string pointer used by two-line messages. */
extern void* g_menu_message_line2;
extern void* D_801227D4;

extern u8 D_800F0BE0[];
extern u8 D_800F0BEC[];

extern s8 D_800F0C38[];

extern s32 D_80168C6C;

/* Rendering assets. */
extern MenuIconSpriteInfo g_menu_icon_sprite_defs[];
extern u8 g_menu_icon_clut_codes[];

/** @brief Three-frame cursor icon-id sequence (0x6B, 0x6C, 0x6D). */
extern u8 g_menu_cursor_icon_ids[];

/* Inventory and equipment state. */
extern u8 D_8016869F[];
extern u8 D_801686A0[];
extern u8 g_menu_item_description_buffer[];
extern s32 g_menu_inventory_index;
extern s32 g_menu_active_item_category;
/** @brief Selected equipment row awaiting a swap, or MENU_NONE. */
extern s32 g_menu_pending_item_row;

/* Equipment-action assets. */
extern u8 D_8016869B[];
extern u8 D_800F0BF8[];
/** @brief The u32 at D_800F0BF8 + 0x14; the item-kind word of the default compare entry. */
extern u32 D_800F0C0C;

/** @brief Mark every menu window slot as free. */
static inline void menu_clear_slots(void)
{
    s32 slot_index;
    for (slot_index = MENU_SLOT_COUNT - 1; slot_index >= 0; slot_index--)
    {
        g_menu_slots[slot_index].active = 0;
    }
}

/** @brief Build circular navigation links for a two-entry item list. */
static inline void menu_relink_pair(void)
{
    s32 index;
    s32 previous_index;
    s32 next_index;
    s32 has_next;
    s32 link;
    s32 entry_with_previous;
    index = 0;
    while (index < 2)
    {
        s32 entry = g_menu_item_nav_entries[index];
        s32 entry_with_position;
        previous_index = 1;
        link = entry & ~MENU_ITEM_NAV_POSITION_MASK;
        link = link | ((index * MENU_ITEM_NAV_POSITION_STRIDE) & MENU_ITEM_NAV_POSITION_MASK);
        entry_with_position = link;
        g_menu_item_nav_entries[index] = entry_with_position;
        if ((index - 1) >= 0)
        {
            previous_index = index - 1;
        }
        entry_with_previous = entry_with_position & MENU_ITEM_NAV_PREVIOUS_CLEAR_MASK;
        entry_with_previous = entry_with_previous | ((previous_index & MENU_ITEM_NAV_INDEX_MASK) << MENU_ITEM_NAV_PREVIOUS_SHIFT);
        g_menu_item_nav_entries[index] = entry_with_previous;
        next_index = index + 1;
        has_next = next_index < 2;
        link = 0;
        if (has_next != 0)
        {
            link = next_index;
        }
        g_menu_item_nav_entries[index] = (entry_with_previous & MENU_ITEM_NAV_NEXT_CLEAR_MASK) | (link << MENU_ITEM_NAV_NEXT_SHIFT);
        index = next_index;
    }
}

/**
 * @brief Locate an equipped item belonging to the active character.
 * @param context Shared save-state context.
 * @param equipment_slot Index within the character's equipment array.
 * @return Address of the equipped item record.
 */
static inline MenuItemEntry* menu_equipped_item(PadContext* context, s32 equipment_slot)
{
    s32 offset = equipment_slot * MENU_ITEM_RECORD_SIZE;
    offset += g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE;
    return (MenuItemEntry*)((u8*)context + offset + MENU_CHARACTER_RECORD_OFFSET + MENU_CHARACTER_ITEMS_OFFSET);
}

/**
 * @brief Locate one character record in the shared context.
 * @param context Shared save-state context.
 * @param character_slot Character record index.
 * @return Character record, including its equipped items.
 */
static inline MenuCharacterRecord* menu_character_record(PadContext* context, s32 character_slot)
{
    return (MenuCharacterRecord*)((u8*)context + (character_slot * MENU_CHARACTER_BLOCK_SIZE + MENU_CHARACTER_RECORD_OFFSET));
}

/**
 * @brief Draw a content label at the menu's fixed help-text position.
 * @param packet_cursor First free primitive byte.
 * @param ot Ordering-table entry receiving the text primitives.
 * @param text Encoded text to draw.
 * @return First free primitive byte after the label.
 */
static inline void* menu_emit_content_label(void* packet_cursor, s32* ot, void* text)
{
    return (void*)func_800A88A0((s32)packet_cursor, ot, text, 1, 0xA0, 0xCA, 2);
}

/**
 * @brief Concatenate two encoded menu strings and write their terminator.
 * @param destination Buffer large enough for both strings and the terminator.
 * @param first First null-terminated encoded string.
 * @param second Second null-terminated encoded string.
 * @note Bytes 0x19-0x1F introduce a second byte, which may be zero.
 */
static inline void menu_copy_encoded_pair(u8* destination, const u8* first, const u8* second)
{
    for (;;)
    {
        u8 character = *first;
        if (character == 0)
        {
            break;
        }
        if ((u8)(character - MENU_TEXT_PREFIX_FIRST) < MENU_TEXT_PREFIX_COUNT)
        {
            *destination++ = character;
            first++;
            *destination++ = *first++;
        }
        else
        {
            *destination++ = character;
            first++;
        }
    }
    for (;;)
    {
        u8 character = *second;
        if (character == 0)
        {
            break;
        }
        if ((u8)(character - MENU_TEXT_PREFIX_FIRST) < MENU_TEXT_PREFIX_COUNT)
        {
            *destination++ = character;
            second++;
            *destination++ = *second++;
        }
        else
        {
            *destination++ = character;
            second++;
        }
    }
    *destination = 0;
}

/**
 * @brief Return one text table from the active resource directory.
 * @param table Resource-directory index.
 * @return Table base used by its relative string offsets.
 */
static inline u8* menu_text_table_base(s32 table)
{
    u8* resources = (u8*)g_menu_state_ptr;
    return resources + ((MenuTextResources*)resources)->table_offsets[table];
}

/**
 * @brief Resolve one table-relative string offset.
 * @param base Text table beginning with halfword offsets.
 * @param index String-offset index.
 * @return Encoded string at that offset.
 */
static inline u8* menu_text_entry(void* base, s32 index)
{
    return (u8*)base + ((u16*)base)[index];
}

/**
 * @brief Locate image data at a word-aligned offset in the upload buffer.
 * @param buffer Word-aligned base of the image upload buffer.
 * @param byte_offset Byte offset, rounded down to a four-byte boundary.
 * @return Source words for LoadImage.
 */
static inline u_long* menu_image_upload_source(void* buffer, s32 byte_offset)
{
    return (u_long*)(((byte_offset >> 2) << 2) + (u32)buffer);
}

/** @brief Convert a packed menu icon palette code to a Psy-Q CLUT id. */
static inline u16 menu_icon_clut(u8 code)
{
    return getClut((code & 0xF) << 4, (code >> 4) + MENU_ICON_CLUT_Y_BASE);
}

/**
 * @brief Read the seven-bit navigation column from a packed position word.
 * @param packed Position word containing the column in bits 14:8.
 * @return Navigation column.
 */
static inline s32 menu_nav_x(u16 packed)
{
    return (packed >> 8) & (MENU_NAV_X_MASK >> 8);
}

/**
 * @brief Resolve a string offset using its entry's position in the shared directory.
 * @param entry Address of the two-byte offset entry.
 * @param index Entry index in the shared string directory.
 * @return Encoded string referenced by the entry.
 */
static inline u8* menu_shared_text_entry(const StringTableOffset* entry, s32 index)
{
    s32 string_page_base = (entry->page << 8) + (s32)((u8*)entry - index * sizeof(StringTableOffset));
    return (u8*)(entry->entry + string_page_base);
}

/** @brief Copy the label for a value's sign and append its terminator. */
static inline void menu_copy_sign_label(u8* buffer, s32 value)
{
    u8* write_cursor = buffer;
    u8* source;

    if (value >= 0)
    {
        source = menu_shared_text_entry(&g_menu_label_key_a, 11);
        func_800A8E28(write_cursor, source);
    }
    else
    {
        source = menu_shared_text_entry(&g_menu_label_key_b, 16);
        func_800A8E28(write_cursor, source);
    }
    write_cursor += func_800A8DDC(source);
    *write_cursor = 0;
}


#endif
