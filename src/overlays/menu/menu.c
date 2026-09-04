#include "menu.h"
#include "display.h"

/* ----- Macros ----- */

/* Menu chrome CLUT ids produced by getClut for the rows uploaded in menu_upload_tim. */
#define MENU_CLUT_GRID_BASE 0x7C80
#define MENU_CLUT_GRID_ALT 0x7C81
#define MENU_CLUT_CORNER 0x7CCA
#define MENU_GRID_OT_INDEX 0x0F
#define MENU_GRID_ALT_CLUT_START 0x11
#define MENU_GRID_SPRITE_COUNT 0x1D
#define MENU_GRID_TEXTURE_WINDOW_SIZE 0xFF
#define MENU_GRID_TPAGE 5

#define MENU_SLOT_COUNT 4
#define MENU_SLOT_OT_INDEX_SHIFT 25
#define MENU_SLOT_OT_INDEX_CLEAR_MASK 0x01FFFFFF
#define MENU_WINDOW_TRANSITION_STEPS 12
#define MENU_WINDOW_MIN_WIDTH 0x20
#define MENU_WINDOW_MIN_HEIGHT 0x10
/** Per-edge inset for the given opening or closing animation frame. */
#define MENU_WINDOW_EDGE_INSET(size, frame) \
    (((size) >> 1) - ((s16)((size) / MENU_WINDOW_TRANSITION_STEPS) * (frame)))

/** Size of the embedded TIM image block, including its block header. */
#define MENU_TIM_IMAGE_BLOCK_SIZE 0x800C
/** Read two adjacent CLUT colors as one packed 32-bit word. */
#define MENU_TIM_CLUT_WORD(tim, index) (((u32*)(tim)->clut_data)[index])

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

/* VRAM placement for each slot's cursor strip and content texture block. */
#define PRIM_STRIP_VRAM_X 0x110  /* 272  - VRAM column                    */
#define PRIM_STRIP_VRAM_Y0 0x1D8 /* 472  - VRAM row for slot 0            */
#define PRIM_STRIP_W 0x10        /* 16 halfwords wide                     */
#define PRIM_STRIP_H 1           /* 1 scanline tall                       */
#define PRIM_BLOCK_VRAM_X 0x3F4  /* 1012 - VRAM column for slots 0 and 1  */
#define PRIM_BLOCK_VRAM_X2 0x3E8 /* 1000 - VRAM column for slot 2         */
#define PRIM_BLOCK_VRAM_Y0 0x120 /* 288  - VRAM row for slot 0            */
#define PRIM_BLOCK_VRAM_Y1 0x150 /* 336  - VRAM row for slots 1 and 2     */
#define PRIM_BLOCK_W 0xC         /* 12 halfwords wide                     */
#define PRIM_BLOCK_H 0x30        /* 48 scanlines tall                     */
#define PRIM_SLOT_COUNT 3
#define PRIM_STRIP_BYTE_SIZE (PRIM_STRIP_W * PRIM_STRIP_H * sizeof(u16))
#define PRIM_BLOCK_BYTE_SIZE (PRIM_BLOCK_W * PRIM_BLOCK_H * sizeof(u16))
#define PRIM_BLOCK_BUF_OFFSET PRIM_STRIP_BYTE_SIZE
#define PRIM_SLOT_STRIDE (PRIM_STRIP_BYTE_SIZE + PRIM_BLOCK_BYTE_SIZE)
/** Normalize a byte offset to the word boundary required by LoadImage. */
#define PRIM_ALIGN_UPLOAD_OFFSET(offset) (((offset) >> 2) << 2)
#define PRIM_UPLOAD_PTR(base, offset) ((u_long*)(PRIM_ALIGN_UPLOAD_OFFSET(offset) + (u32)(base)))

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
/** @brief Extracts the 7-bit navigation X coordinate from its packed field. */
#define MENU_NAV_X(packed) (((u16)(packed) >> 8) & (MENU_NAV_X_MASK >> 8))
/** @brief Clears bits [14:8] of idx_nav.nav_x_packed (inverse of MENU_NAV_X_MASK). */
#define MENU_NAV_X_CLEAR 0x80FF
/** @brief Bit 15 of idx_nav.nav_x_packed: bit 0 of the 9-bit nav cursor Y. */
#define MENU_NAV_Y0_BIT 0x8000
/** @brief Bit 15 of u8_u.nav_y_packed: bit 0 of the 9-bit layout Y position. */
#define MENU_LAYOUT_Y0_BIT 0x8000
/** @brief Number of child-index slots per node (child0..child3). */
#define MENU_MAX_CHILDREN 4
/** @brief MenuNode::state value before menu_layout_node has run. */
#define MENU_NODE_STATE_UNINIT 0
/** @brief MenuNode::state value after menu_layout_node assigns a Y position. */
#define MENU_NODE_STATE_LAID_OUT 4
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
/** @brief Clears the packed previous-index field while preserving all other bits. */
#define MENU_ITEM_NAV_PREVIOUS_CLEAR_MASK 0xFF803FFF
/** @brief Clears the packed next-index field while preserving all other bits. */
#define MENU_ITEM_NAV_NEXT_CLEAR_MASK 0x007FFFFF
/** @brief Offset of the 12-row spell-presence bitmap in g_pad_ctx. */
#define MENU_SPELL_GRID_OFFSET 0x60
#define MENU_SPELL_GRID_ROW_COUNT 12
#define MENU_SPELL_GRID_COLUMN_COUNT 8
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
/** @brief Converts a content item's Y coordinate to the viewport origin. */
#define MENU_CONTENT_VIEW_Y_OFFSET 8
/** @brief g_menu_redraw_state: navigation key pressed, scroll position adjusted. */
#define MENU_REDRAW_NAVIGATE 6
/** @brief g_menu_redraw_state: layout pass completed (position change or first run). */
#define MENU_REDRAW_LAYOUT 8
/** @brief g_pad_ctx->inject_flags bit enabling injected menu input. */
#define MENU_PAD_INJECT_ENABLED 0x80
/** @brief Ordering-table entry used as the menu frame's list head. */
#define MENU_FRAME_OT_INDEX 13
/** @brief Vertical offset of the node-tree clipping region within a draw page. */
#define MENU_TREE_DRAW_Y_OFFSET 12
#define MENU_TREE_DRAW_X 15
#define MENU_TREE_DRAW_WIDTH 36
#define MENU_TREE_DRAW_HEIGHT 170

/* Sound-effect ids passed to menu_play_se; volume is always MENU_SE_VOLUME. */
/** @brief Scroll navigation sound (D-up / D-down / Circle to scroll). */
#define MENU_SE_NAVIGATE 0x7D
/** @brief Open / select sound (Circle or D-right to enter a node). */
#define MENU_SE_SELECT 0x7E
/** @brief Close / cancel sound (Circle while at MENU_NODE_BROWSE_ALL). */
#define MENU_SE_CLOSE 0x7F
/** @brief Full volume level for all menu sound effects (128). */
#define MENU_SE_VOLUME 0x80

/* ----- Types ----- */

/** VRAM destinations for the image and CLUT blocks in the menu TIM asset. */
typedef struct
{
    s16 texture_x;
    s16 texture_y;
    s16 clut_x;
    s16 clut_y;
} MenuTimVramLayout;

/** Halfword view of MenuSlot.flags for width-accurate field initialization. */
typedef union
{
    u32 value;
    struct
    {
        u16 low;
        u16 high;
    } half;
} MenuSlotFlagsView;

#define MENU_SLOT_FLAGS_VIEW(slot) (*(MenuSlotFlagsView*)&(slot)->flags)

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

void menu_upload_tim(const MenuTimVramLayout* layout);

/**
 * @brief Animation-facing view of a menu window slot.
 */
typedef struct
{
    u8 _pad0[2];   // offsets 0x00-0x01 (active, index)
    u8 anim_frame; // offset 0x02 - animation frame counter (counts up during open/close)
    u8 _pad1[5];   // offsets 0x03-0x07
    u16 x;         // offset 0x08 - window X origin
    u16 y;         // offset 0x0A - window Y origin
    s16 w;         // offset 0x0C - target window width  (clamped to >= 0x20)
    s16 h;         // offset 0x0E - target window height (clamped to >= 0x10)
} MenuSlotAnim;

typedef struct
{
    u8 active; /* 0x00 - mirrors MenuSlot.active (2 = open/steady) */
    u8 index;  /* 0x01 - mirrors MenuSlot.index */
    u8 pad2;
    u8 has_title; /* 0x03 - mirrors MenuSlot.has_title */
    union
    {
        s32 flags;
        struct
        {
            u16 _unk4lo;
            u16 unk6;
        } _s;
    } _u;
    u16 x;             /* 0x08 */
    u16 y;             /* 0x0A */
    u16 w;             /* 0x0C */
    u16 h;             /* 0x0E */
    u16 lerp_cur_a;    /* 0x10 */
    u16 lerp_cur_b;    /* 0x12 */
    u16 lerp_target_a; /* 0x14 */
    u16 lerp_target_b; /* 0x16 */
    u8 lerp_steps;     /* 0x18 */
    u8 pad19;
    u8 pad1A;
    u8 pad1B;
    s32* (*content_cb)(); /* 0x1C */
} MenuSlotView;

typedef struct
{
    u8 pad[0x4040];
    s32* prim_cursor; /* 0x4040 - primitive write cursor */
    u8 pad4044[8];
    s32 draw_buf_idx; /* 0x404C - display buffer page index (0 or 1) */
} MenuRenderCtx;

typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} MenuRect;

typedef struct
{
    union
    {
        s32 unk0;
        struct
        {
            u8 _pad0[3];
            u8 unk3;
        } _s;
    } _u;
    s32 unk4;
    s32 unk8;
} MenuPrimHead;

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

/** Stack layout used while drawing an opening or closing menu window. */
typedef struct
{
    MenuRect rect;
    ScreenPos view_origin;
    s16 padding[2];
} MenuWindowTransitionFrame;

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

/**
 * @brief Known prefix of the shared menu string-table layout.
 *
 * The two sign-dependent label offsets are embedded within the table data.
 */
typedef struct
{
    u8 prefix[0x16];
    StringTableOffset nonnegative_label;
    u8 between_labels[8];
    StringTableOffset negative_label;
} MenuStringTableLayout;

/** Recover the enclosing string-table base from one of its embedded offsets. */
#define MENU_STRING_TABLE_MEMBER_OFFSET(member) \
    ((u32)&((MenuStringTableLayout*)0)->member)
#define MENU_STRING_TABLE_BASE(key, member) \
    ((u8*)&(key) - MENU_STRING_TABLE_MEMBER_OFFSET(member))

/**
 * @brief One node in the hierarchical menu navigation tree.
 */
typedef struct
{
    u8 label_id; /**< Index into the menu label string table. */
    u8 state;    /**< Node state: 0 = uninitialized, 4 = position assigned by menu_layout_node. */
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
        u16 nav_y_packed; /**< Raw word; high byte = layout_y_lsb, low byte = nav_y_hi. */
        struct
        {
            u8 nav_y_hi;     /**< Bits 1-8 of the 9-bit nav cursor Y: reconstruct as (nav_y_hi<<1)|(nav_x>>7). */
            u8 layout_y_lsb; /**< Bit 7 = bit 0 of layout Y position; bits 0-6 always 0 after layout. */
        } s;
    } u8_u;
    union
    {
        u16 layout_child_packed; /**< Raw word; high byte = child0, low byte = layout_y_hi. */
        struct
        {
            u8 layout_y_hi; /**< Bits 1-8 of the 9-bit layout Y position: reconstruct as
                               (layout_y_hi<<1)|(layout_y_lsb>>7). */
            u8 child0;      /**< First child node index (0xFF = none). */
        } s;
    } uA;
    u8 child1; /**< Second child node index (0xFF = none). */
    u8 child2; /**< Third child node index (0xFF = none). */
    u8 child3; /**< Fourth child node index (0xFF = none). */
    u8 unkF;
} MenuNode;

/** @brief Four item-record pointers used by the comparison slots. */
typedef struct
{
    u32 slot0; /**< Slot 0 data pointer. */
    u32 slot1; /**< Slot 1 data pointer. */
    u32 slot2; /**< Slot 2 data pointer. */
    u32 slot3; /**< Slot 3 data pointer. */
} ItemSlotData;

/** @brief Occupancy flags parallel to g_item_slot_data. */
typedef struct
{
    u8 slot0; /**< Slot 0 occupied flag. */
    u8 slot1; /**< Slot 1 occupied flag. */
    u8 slot2; /**< Slot 2 occupied flag. */
    u8 slot3; /**< Slot 3 occupied flag. */
} ItemSlotFlags;

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
    u8 pad[5];    /**< pad[0] is the content/action id; remaining bytes are content-specific. */
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

#define MENU_CONTROLLER_ACTUATORS ((MenuControllerActuatorState*)0x801ED600)

typedef enum
{
    MENU_CURSOR_MODE_NODE_TREE = 0,
    MENU_CURSOR_MODE_CONTENT = 1,
    MENU_CURSOR_MODE_CONTENT_EXIT = 2,
} MenuCursorMode;

/* ----- Forward declarations ----- */

/* K&R form preserves the original menu_tick call-site use of live a0. */
void menu_build_grid();
void menu_update_slots(RenderContext* render_ctx);
u8* menu_draw_frame(u8* packet_cursor, u_long* ot_entry, s32 frame_parity, s32 allow_input);
u32 menu_step_item_selection(s32 step);

/* ----- Extern globals ----- */

/** @brief Optional help/description string drawn below the active menu content. */
extern s32 g_menu_help_text;
/** @brief Selects node-tree, content, or content-exit cursor handling. */
extern s32 g_menu_cursor_enable;
/** @brief Set non-zero by a content callback to abort @ref menu_draw_window early. */
extern s32 g_menu_draw_early_out;
/** @brief Base address of the menu double-buffered DRAWENV array. */
extern s32 g_menu_draw_buf_base;
/** @brief When non-zero, suppresses cursor highlight even on the active slot. */
extern s32 g_menu_suppress_cursor;
/** @brief Scene/language selector used in window title decoration layout switches. */
extern s32 g_menu_scene_type;

extern MenuNode g_menu_nodes[0x2C];
extern u8 g_menu_prev_node;
/** @brief Gate flag for menu_draw_content_cursor: 0 = draw empty slot, nonzero = full item render. */
extern s32 g_menu_content_ready;

extern ItemSlotData g_item_slot_data;
extern ItemSlotFlags g_item_slot_flags;

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
/** @brief Array mapping navigation-list position to the previous node ID (up navigation, D-pad Up). */
extern s32 g_menu_nav_prev[];
extern u8 g_menu_init_content_id;

extern Struct_D_800FD818 D_800FD818;
extern u16 D_800FDA80;
extern u16 D_800FDCE8;
/** @brief Ability-compatibility mask rebuilt before item content is loaded. */
extern s8 g_menu_ability_mask;
/** @brief Node index for the companion character's stat page (0x2B = companion present, 0xFF = none). */
extern s8 g_menu_companion_node;

/** @brief Nonnegative-label offset embedded in MenuStringTableLayout. */
extern StringTableOffset g_menu_label_key_a;
/** @brief Negative-label offset embedded in MenuStringTableLayout. */
extern StringTableOffset g_menu_label_key_b;

/** @brief Number of nodes in the linear navigation list. */
extern s32 g_menu_nav_count;
/** @brief Node ID at the start of the navigation list; used for wrap-around on down-navigation. */
extern s32 g_menu_nav_first;
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
/** @brief Array mapping navigation-list position to the next node ID (down navigation). */
extern s32 g_menu_nav_next[];
/** @brief Default X/Y origin for the content viewport when no item hit-test position is available. */
extern struct
{
    s16 x;
    s16 y;
} g_menu_default_view_pos;
/** @brief Per-node table of MenuContentItem arrays, indexed by node.idx_nav.s.self_idx; NULL = no cursor data. */
extern MenuContentItem* g_menu_content_table[];
extern s32 g_menu_layout_end;

/**
 * @brief Initialize menu graphics, runtime state, window slots, and the node tree.
 * @see decomp.me (100%) https://decomp.me/scratch/Dv8qB
 */
void menu_init(void)
{
    volatile u8 padding;
    menu_upload_graphics();
    menu_state_init();
    menu_reset_slots();
    g_active_slot = -1;
    func_800AA02C();
    g_menu_compare_window_active = 0;
    menu_init_prim_rects();
    g_menu_frame = 0;
    g_script_cursor = 0;
    menu_node_tree_init();
}

/**
 * @brief Upload each menu slot's cursor strip and content block to VRAM.
 * @see decomp.me (100%) https://decomp.me/scratch/QnGCP
 */
void menu_init_prim_rects(void)
{
    s32 slot = 0;
    u8* scratch = g_prim_rect_buf;
    s32 block_byte_offset = PRIM_BLOCK_BUF_OFFSET;
    s32 strip_byte_offset = 0;
    RECT rect;
    u_long* upload_src;

    for (; slot < PRIM_SLOT_COUNT; slot++)
    {
        /* Upload the slot's cursor-highlight strip. */
        rect.x = PRIM_STRIP_VRAM_X;
        rect.y = slot + PRIM_STRIP_VRAM_Y0;
        rect.w = PRIM_STRIP_W;
        rect.h = PRIM_STRIP_H;
        upload_src = PRIM_UPLOAD_PTR(scratch, strip_byte_offset);
        LoadImage(&rect, upload_src);

        /* Upload the slot's content texture block. */
        rect.x = (slot == PRIM_SLOT_COUNT - 1) ? PRIM_BLOCK_VRAM_X2 : PRIM_BLOCK_VRAM_X;
        rect.y = (slot == 0) ? PRIM_BLOCK_VRAM_Y0 : PRIM_BLOCK_VRAM_Y1;
        rect.w = PRIM_BLOCK_W;
        rect.h = PRIM_BLOCK_H;
        upload_src = PRIM_UPLOAD_PTR(scratch, block_byte_offset);
        LoadImage(&rect, upload_src);

        block_byte_offset += PRIM_SLOT_STRIDE;
        strip_byte_offset += PRIM_SLOT_STRIDE;
    }
}

/**
 * @brief Process input and render one menu frame.
 * @param render_ctx Render context receiving the menu primitives.
 * @see decomp.me (100%) https://decomp.me/scratch/kgN9O
 */
void menu_tick(RenderContext* render_ctx)
{
    s32 menu_frame;
    s32 frame_counter;
    s32 input_mask;
    void* saved_prim_cursor;
    s32 repeat_index;
    u16 script_input;
    s32 stack_padding[2];

    menu_build_grid(render_ctx);
    menu_frame = g_menu_frame;
    frame_counter = g_frame_counter;
    /* Preserve the packet cursor established by the grid pass. */
    saved_prim_cursor = render_ctx->prim_cursor;
    g_menu_frame = menu_frame + 1;
    g_frame_counter = frame_counter + 1;
    func_800A9E78();

    /* Merge externally injected input when enabled by the pad context. */
    if ((g_pad_ctx->inject_flags & MENU_PAD_INJECT_ENABLED) && g_pad_ctx->inject_enable)
    {
        g_pad_input |= g_pad_input_inject;
    }

    /* Keep only the highest-priority active button group. */
    input_mask = g_pad_input & MENU_PAD_CONFIRM_CANCEL;
    if (input_mask)
    {
        g_pad_input = input_mask;
    }
    input_mask = g_pad_input & MENU_PAD_FACE_BUTTONS;
    if (input_mask)
    {
        g_pad_input = input_mask;
    }
    input_mask = g_pad_input & MENU_PAD_SHOULDERS;
    if (input_mask)
    {
        g_pad_input = input_mask;
    }

    /* Prevent input from being accepted on consecutive frames. */
    if (g_pad_input_latched != 0)
    {
        g_pad_input = 0;
    }
    g_pad_input_latched = g_pad_input;

    /* Replace live input with the active scripted input sequence. */
    if (g_active_script != 0)
    {
        u8* script_table = (u8*)g_script_table;
        u32 script_row_addr = g_active_script * sizeof(MenuScript);
        MenuScript* script_row;
        s32 cursor;

        script_row_addr += (u32)script_table;
        script_row = (MenuScript*)script_row_addr;
        cursor = g_script_cursor;

        g_pad_input = 0;

        script_input = script_row->inputs[cursor];

        if (script_input == MENU_SCRIPT_END)
        {
            if (g_active_script < 4)
            {
                repeat_index = 0;
                if (g_script_repeat_count > 0)
                {
                    do
                    {
                        menu_step_item_selection(1);
                        repeat_index++;
                    } while (repeat_index < g_script_repeat_count);
                }
                g_script_repeat_last = g_script_repeat_count;
            }
            g_active_script = 0;
        }
        else
        {
            g_pad_input = script_input;
            g_script_cursor = cursor + 1;
        }
    }

    /* Render slots from the grid pass's packet cursor. */
    render_ctx->prim_cursor = saved_prim_cursor;
    menu_update_slots(render_ctx);
}

typedef enum
{
    MENU_TEXT_ALIGN_LEFT = 0,
    MENU_TEXT_ALIGN_RIGHT = 1,
    MENU_TEXT_ALIGN_CENTER = 2,
} MenuTextAlignment;

/**
 * @brief Build and queue a horizontally aligned run of glyph sprites.
 * @param sprite_cursor Start of the primitive-buffer region for the glyph sprites.
 * @param ot Ordering-table entry that receives the emitted packets.
 * @param src Address of the source text.
 * @param text_color Text-color index in the range 0–15.
 * @param x Horizontal anchor selected by @p alignment.
 * @param y Y coordinate applied to every glyph.
 * @param len Source byte length; must not exceed 0x7F.
 * @param alignment Horizontal alignment of the run relative to @p x.
 * @return Next free primitive-buffer address, immediately after the draw-mode packet.
 *
 * @see field_text_build_sprites
 * @see decomp.me (100%) https://decomp.me/scratch/AW5Sa
 */
void* menu_build_text_run(
    SPRT* sprite_cursor, s32* ot, s32 src, s32 text_color, s32 x, s32 y, s32 len, MenuTextAlignment alignment)
{
    char buf[0x80];
    s32 count, i, acc;
    SPRT* sprite;
    DR_TPAGE* tpage;

    /* Prepare a null-terminated slice for the glyph decoder. */
    strncpy(buf, (char*)src, len);
    buf[len] = 0;

    /* Populate one SPRT per decoded glyph. */
    count = field_text_build_sprites(sprite_cursor, buf, text_color);

    /* Convert the requested anchor into the run's left edge. */
    if (alignment != MENU_TEXT_ALIGN_RIGHT)
    {
        if (alignment == MENU_TEXT_ALIGN_CENTER)
        {
            sprite = sprite_cursor;
            for (i = 0; i < count; i++)
            {
                x -= sprite[i].w >> 1;
            }
        }
    }
    else
    {
        sprite = sprite_cursor;
        for (i = 0; i < count; i++)
        {
            x -= sprite[i].w;
        }
    }

    /* Finish and link the prebuilt sprites using a running x offset. */
    acc = 0;

    if (count != 0)
    {
        do
        {
            sprite = sprite_cursor;
            SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
            setSprt(sprite);

            SET_SPRT_XY0_WORD(sprite, PACK_U16_PAIR(x, y) + acc);
            acc += sprite->w;

            addPrim(ot, sprite);
            sprite_cursor++;
            count--;
        } while (count != 0);
    }

    /* addPrim prepends, so link the texture-page packet after the sprites. */
    tpage = (DR_TPAGE*)sprite_cursor;
    setDrawTPage(tpage, 0, 0, 0x1F);
    addPrim(ot, tpage);

    return tpage + 1;
}

/**
 * @brief Builds and queues the GPU packet sequence for the menu grid.
 * @param render_ctx Render context providing the packet buffer and ordering table;
 * @see decomp.me (100%) https://decomp.me/scratch/ZtHxG
 */
void menu_build_grid(RenderContext* render_ctx)
{
    RECT texture_window;
    s32 sprite_index;
    SPRT* sprite;
    const MenuGridSpriteDef* sprite_def;
    u_long* packet_cursor;
    RenderContext* first_ctx = render_ctx;
    RenderContext* ot_ctx = first_ctx;

    /* Disable texture-window masking for the grid sprite batch. */
    packet_cursor = first_ctx->prim_cursor;
    texture_window.h = MENU_GRID_TEXTURE_WINDOW_SIZE;
    texture_window.w = MENU_GRID_TEXTURE_WINDOW_SIZE;
    texture_window.y = 0;
    texture_window.x = 0;

    setTexWindow((DR_TWIN*)packet_cursor, &texture_window);
    addPrim(&first_ctx->ot[MENU_GRID_OT_INDEX], packet_cursor);

    /* Expand each packed grid definition into one SPRT packet. */
    sprite_def = (const MenuGridSpriteDef*)g_menu_glyph_src;
    packet_cursor += PRIM_WORDS(DR_TWIN);
    sprite = (SPRT*)packet_cursor;

    for (sprite_index = 0; sprite_index < MENU_GRID_SPRITE_COUNT; sprite_index++, sprite++, sprite_def++)
    {
        SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
        setSprt(sprite);
        SET_SPRT_UV0_PACKED(sprite, sprite_def->uv);

        /* Copy the packed coordinate and size pairs as words. */
        SET_SPRT_XY0_WORD(sprite, sprite_def->packed_xy);
        SET_SPRT_WH_WORD(sprite, sprite_def->packed_wh);

        /* The final sprite group uses the alternate grid palette. */
        if (sprite_index >= MENU_GRID_ALT_CLUT_START)
        {
            SET_SPRT_CLUT(sprite, MENU_CLUT_GRID_ALT);
        }
        else
        {
            SET_SPRT_CLUT(sprite, MENU_CLUT_GRID_BASE);
        }

        addPrim(&ot_ctx->ot[MENU_GRID_OT_INDEX], sprite);
    }

    packet_cursor = (u_long*)sprite;

    /* Close the batch with texture-window and texture-page state packets. */
    texture_window.w = MENU_GRID_TEXTURE_WINDOW_SIZE;
    texture_window.h = MENU_GRID_TEXTURE_WINDOW_SIZE;
    texture_window.x = 0;
    texture_window.y = 0;

    setTexWindow((DR_TWIN*)packet_cursor, &texture_window);
    addPrim(&ot_ctx->ot[MENU_GRID_OT_INDEX], packet_cursor);

    packet_cursor += PRIM_WORDS(DR_TWIN);
    setDrawTPage((DR_TPAGE*)packet_cursor, 0, 0, MENU_GRID_TPAGE);
    addPrim(&ot_ctx->ot[MENU_GRID_OT_INDEX], packet_cursor);

    /* Publish the first unused packet word for subsequent builders. */
    render_ctx->prim_cursor = packet_cursor + PRIM_WORDS(DR_TPAGE);
}

/**
 * @brief Upload the menu texture and CLUTs to their reserved VRAM regions.
 * @see decomp.me (100%) https://decomp.me/scratch/CKNIH
 */
void menu_upload_graphics(void)
{
    MenuTimVramLayout layout;

    layout.texture_x = SCREEN_WIDTH;
    layout.texture_y = 0;
    layout.clut_x = 0;
    layout.clut_y = VRAM_CLUT_Y;
    menu_upload_tim(&layout);
}

/**
 * @brief Initialize the base pointer for the menu's resource tables.
 * @see decomp.me (100%) https://decomp.me/scratch/A1YTp
 */
void menu_state_init(void)
{
    g_menu_state_ptr = &g_menu_state_data;
}

/**
 * @brief Upload the menu texture and its two CLUTs to VRAM.
 * @param layout VRAM destinations for the texture and first CLUT; the second CLUT is placed on the following row.
 * @see decomp.me (100%) https://decomp.me/scratch/tG03R
 */
void menu_upload_tim(const MenuTimVramLayout* layout)
{
    MenuTimAsset* asset = (MenuTimAsset*)g_menu_tim;
    Tim* tim = &asset->tim;
    s32 clut_block_len = tim->clut_block.bnum;
    RECT vram_rect;
    u16* clut_color;
    s32 i;

    /* Preserve the first color pair before modifying the palette in place. */
    g_menu_initial_clut_pair = MENU_TIM_CLUT_WORD(tim, 0);

    /* Enable STP on nonzero colors, then upload the first CLUT. */
    vram_rect.x = layout->clut_x;
    vram_rect.y = layout->clut_y;
    vram_rect.w = CLUT_ENTRY_COUNT;
    vram_rect.h = 1;

    clut_color = asset->tim.clut_data;
    for (i = 0; i < CLUT_ENTRY_COUNT; i++)
    {
        if (*clut_color != 0)
        {
            *clut_color |= GPU_STP_BIT;
        }

        clut_color++;
    }
    LoadImage(&vram_rect, tim->clut_data);

    /* Upload the image block following the variable-length first CLUT. */
    vram_rect.x = layout->texture_x;
    vram_rect.y = layout->texture_y;
    {
        TimBlock* image_block = TIM_PIXEL_BLOCK(tim, clut_block_len);
        vram_rect.w = image_block->dimensions.width;
        vram_rect.h = image_block->dimensions.height;
        LoadImage(&vram_rect, image_block + 1); /* Pixel data follows the block header. */
    }

    /* Apply the same STP treatment to the second CLUT. */
    vram_rect.x = layout->clut_x;
    vram_rect.y = layout->clut_y + 1;
    vram_rect.w = CLUT_ENTRY_COUNT;
    vram_rect.h = 1;

    clut_color = asset->second_clut;
    for (i = 0; i < CLUT_ENTRY_COUNT; i++)
    {
        if (*clut_color != 0)
        {
            *clut_color |= GPU_STP_BIT;
        }

        clut_color++;
    }
    LoadImage(&vram_rect, asset->second_clut);
}

/**
 * @brief Allocate and initialize the first available menu window slot.
 * @param ot_index Ordering-table entry used to link the slot's primitives.
 * @param rect Initial window position and dimensions.
 * @return Initialized menu slot.
 * @see decomp.me (100%) https://decomp.me/scratch/Xng7v
 */
MenuSlot* menu_slot_alloc(s32 ot_index, const MenuSlotRect* rect)
{
    s32 slot_index;
    MenuSlot* slot;
    MenuSlot* slot_cursor;
    MenuSlot* slot_pool;
    u32 slot_flags;
    u32 ot_index_clear_mask;

    /* Find the first slot whose active state is clear. */
    slot_index = 0;
    slot_pool = &g_menu_slots[0];
    slot_cursor = &g_menu_slots[0];
    while (slot_index < MENU_SLOT_COUNT)
    {
        if (slot_cursor->active == 0)
        {
            break;
        }
        slot_index++;
        slot_cursor++;
    }

    /* The original negative-index guard does not catch a full pool (index 4). */
    if (slot_index < 0)
    {
        return (MenuSlot*)(-1);
    }
    slot = (MenuSlot*)((slot_index * sizeof(MenuSlot)) + (u32)slot_pool);

    /* Clear low flags while retaining the slot's previous bits 24:16. */
    MENU_SLOT_FLAGS_VIEW(slot).half.low = 0;
    slot_flags = slot->flags;
    slot->active = 1;
    slot->content_cb = 0;
    slot->index = (u8)slot_index;
    slot->tick_cb = 0;
    slot->anim_frame = 0;
    ot_index_clear_mask = MENU_SLOT_OT_INDEX_CLEAR_MASK;
    slot_flags = slot_flags & ot_index_clear_mask;
    slot_flags = slot_flags | (((u32)ot_index) << MENU_SLOT_OT_INDEX_SHIFT);
    slot->flags = slot_flags;
    slot->x = rect->x;
    slot->y = rect->y;
    slot->w = rect->w;
    slot->h = rect->h;
    slot->lerp_cur_a = 0;
    slot->lerp_cur_b = 0;
    slot->lerp_target_a = 0;
    slot->lerp_target_b = 0;
    slot->lerp_steps = 0;
    slot->has_title = 0;
    g_active_slot = slot_index;
    return slot;
}

/**
 * @brief Mark every menu window slot as free.
 * @see decomp.me (100%) https://decomp.me/scratch/D9BI9
 */
void menu_reset_slots(void)
{
    s32 slot_index;
    MenuSlot* slot;

    slot_index = MENU_SLOT_COUNT - 1;
    slot = &g_menu_slots[slot_index];
    while (slot_index >= 0)
    {
        slot->active = 0;
        slot_index--;
        slot--;
    }
}

/**
 * @brief Per-frame update/draw pump for the four menu slots.
 * @param render_ctx Per-frame render context.
 * @see decomp.me (100%) https://decomp.me/scratch/BlGK5
 */
void menu_update_slots(RenderContext* render_ctx)
{
    s16 sp_pair[2];
    s32 unused_pad[2];
    void (*temp_v0_2)(MenuSlot*);
    s32 var_a3;
    s32 var_a0;
    s32 var_s1;
    s32 rect_i;
    u8 temp_a0;
    u8 temp_v0;
    u8 temp_v1;
    u8 temp_v1_2;
    u8 tmp_s5;
    MenuSlot* base = g_menu_slots;
    u_int* frame_ot;

    var_a0 = 0;
    g_menu_help_text = 0;
    var_s1 = 3;
    rect_i = 3;
    tmp_s5 = 2;

    while (var_s1 >= 0)
    {
        temp_v1 = base[rect_i].active;
        if (temp_v1 != tmp_s5)
        {
            s32 tmpCmp = temp_v1;
            if (tmpCmp < 3)
            {
                if (temp_v1 != 1)
                {
                    rect_i -= 1;
                    var_s1 -= 1;
                    continue;
                }
            }
            else
            {
                if (temp_v1 != 3)
                {
                    rect_i -= 1;
                    var_s1 -= 1;
                    continue;
                }
                goto branch_11C;
            }

            menu_draw_window_transition(render_ctx, &base[rect_i], g_menu_cursor_enable != 0);
            temp_a0 = base[rect_i].anim_frame;
            temp_v0 = temp_a0 + 1;
            base[rect_i].anim_frame = temp_v0;
            if ((temp_v0 & 0xff) == 6)
            {
                base[rect_i].anim_frame = temp_a0;
                base[rect_i].active = tmp_s5;
            }
        }
        else
        {
            sp_pair[1] = 0;
            sp_pair[0] = 0;
            menu_draw_window(&base[rect_i], render_ctx, (void*)(((u8*)g_menu_slots + 8) + (((rect_i * 2) - rect_i) * 36)), sp_pair, g_menu_cursor_enable != 0);
        }

        if (var_s1 == g_active_slot)
        {
            temp_v0_2 = (void (*)(MenuSlot*))base[rect_i].tick_cb;
            if (temp_v0_2 != 0)
            {
                temp_v0_2(&base[rect_i]);
            }
        }
        var_a0 = 1;
        rect_i -= 1;
        var_s1 -= 1;
        continue;

    branch_11C:
        menu_draw_window_transition(render_ctx, &base[rect_i], g_menu_cursor_enable != 0);
        temp_v0 = base[rect_i].anim_frame - 1;
        base[rect_i].anim_frame = temp_v0;
        if (!(temp_v0 & 0xFF))
        {
            base[rect_i].active = 0;
            menu_update_active_slot();
        }
        var_a0 = 1;
        rect_i -= 1;
        var_s1 -= 1;
    }

    var_a3 = 0;
    if (var_a0 == 0)
    {
        g_active_slot = -1;
    }

    frame_ot = &render_ctx->ot[MENU_FRAME_OT_INDEX];
    sp_pair[1] = 0;
    sp_pair[0] = 0;

    if ((g_active_slot == -1) || (g_menu_cursor_enable == 0))
    {
        var_a3 = 1;
    }

    render_ctx->prim_cursor = menu_draw_frame(render_ctx->prim_cursor, frame_ot, render_ctx->frame_parity, var_a3);
    if (g_menu_help_text != 0)
    {
        render_ctx->prim_cursor = (void*)func_800A88A0(render_ctx->prim_cursor, (u_int*)((u8*)render_ctx + 0x34 + g_menu_help_text - g_menu_help_text), g_menu_help_text, 1, 0xA0, 0xCA, 2);
    }
}

/**
 * @brief Draw one frame of a menu window's opening or closing transition.
 * @param render_ctx Per-frame menu rendering context.
 * @param slot Window slot being animated.
 * @param cursor_enable Nonzero to allow the active-slot cursor highlight.
 * @see decomp.me (100%) https://decomp.me/scratch/luaLZ
 */
void menu_draw_window_transition(MenuRenderCtx* render_ctx, MenuSlotAnim* slot, s32 cursor_enable)
{
    MenuWindowTransitionFrame frame;
    s32 inset_x;
    s32 inset_y;
    s32 width;
    s32 height;

    /* Expand from the center as anim_frame advances; closing runs it in reverse. */
    inset_x = MENU_WINDOW_EDGE_INSET(slot->w, slot->anim_frame);
    frame.view_origin.x = inset_x;

    inset_y = MENU_WINDOW_EDGE_INSET(slot->h, slot->anim_frame);
    frame.view_origin.y = inset_y;

    if (inset_x > 0)
    {
        if (inset_y > 0)
        {
            width = slot->w - (inset_x * 2);
            if (width < MENU_WINDOW_MIN_WIDTH)
            {
                width = MENU_WINDOW_MIN_WIDTH;
            }

            height = slot->h - (inset_y * 2);
            if (height < MENU_WINDOW_MIN_HEIGHT)
            {
                height = MENU_WINDOW_MIN_HEIGHT;
            }

            frame.rect.x = slot->x + inset_x;
            frame.rect.y = slot->y + inset_y;
            frame.rect.w = width;
            frame.rect.h = height;

            menu_draw_window(slot, render_ctx, &frame.rect, &frame.view_origin, cursor_enable);
        }
    }
}

/**
 * @brief Build all GPU primitives for one menu window at a given rectangle.
 * @param slot Slot descriptor (geometry, flags, content callback).
 * @param gpu_work Per-frame render context (layout matches @ref RenderContext).
 * @param rect Window rectangle: x, y, w, h halfwords.
 * @param view_origin View-origin offset forwarded to the content callback.
 * @param cursor_enable Cursor-highlight enable for the active slot.
 * @see decomp.me (99.96%) https://decomp.me/scratch/5k4SF
 */
void menu_draw_window(MenuSlotView* slot, MenuRenderCtx* gpu_work, MenuRect* rect, ScreenPos* view_origin, s32 cursor_enable)
{
    MenuRectU16 sp18;
    DRAWENV sp20;
    u16 sp80[2];
    s16 temp_a0;
    s32 temp_v1;
    s32 var_a2_2;
    s32 var_a3;
    s32* temp_a1_2;
    s32* temp_s1_2;
    s32* temp_s2;
    s32* prim_cur;
    s32* var_s1;
    u16 temp_a1;
    u16 temp_a2;
    u16 var_v0;
    void* temp_v0_2;
    s32 fill_uv;
    s32 draw_x;
    DRAWENV* env;
    s32 title_mask;

    var_s1 = gpu_work->prim_cursor;
    temp_s2 = (s32*)gpu_work + (((u32)slot->_u.flags >> 0x19));
    if (slot->lerp_steps != 0)
    {
        temp_v1 = (s32)(slot->lerp_target_a - slot->lerp_cur_a) / (s32)slot->lerp_steps;
        temp_a2 = slot->lerp_cur_a;
        temp_a1 = slot->lerp_cur_b + ((s32)(slot->lerp_target_b - slot->lerp_cur_b) / (s32) * (volatile u8*)&slot->lerp_steps);
        slot->lerp_steps = (u8)(*(volatile u8*)&slot->lerp_steps - 1);
        slot->lerp_cur_a = (u16)(temp_a2 + temp_v1);
        slot->lerp_cur_b = (u16)temp_a1;
    }
    else
    {
        slot->lerp_cur_a = (u16)slot->lerp_target_a;
        slot->lerp_cur_b = (u16)slot->lerp_target_b;
    }
    if (slot->content_cb != NULL)
    {
        if ((rect->w - 0x20) > 0)
        {
            if ((rect->h - 0x10) > 0)
            {
                SetDrawEnv((DR_ENV*)var_s1, (DRAWENV*)(g_menu_draw_buf_base + ((gpu_work->draw_buf_idx ^ 1) * DRAW_BUF_STRIDE) + DRAW_BUF_DRAWENV_OFF));
                addPrim(temp_s2, var_s1);
                var_a3 = 0;
                g_menu_draw_early_out = 0;
                var_s1 += PRIM_WORDS(DR_ENV);
                if ((slot->index == g_active_slot) && (0 != cursor_enable))
                {
                    if (g_menu_suppress_cursor == 0)
                    {
                        var_a3 = slot->active == 2;
                    }
                }
                var_s1 = slot->content_cb(temp_s2, slot, var_s1, view_origin, var_a3);
                if (g_menu_draw_early_out != 0)
                {
                    gpu_work->prim_cursor = var_s1;
                    return;
                }
                env = &sp20;
                temp_a0 = rect->y;
                draw_x = rect->x + 8;
                var_a2_2 = temp_a0 + 0x10;
                if (gpu_work->draw_buf_idx != 0)
                {
                    var_a2_2 = temp_a0 + 0xF8;
                }
                SetDefDrawEnv(env, draw_x, var_a2_2, rect->w - 0x10, rect->h - 0x10);
                SetDrawEnv((DR_ENV*)var_s1, env);
                addPrim(temp_s2, var_s1);
                var_s1 += PRIM_WORDS(DR_ENV);
                title_mask = 0x1FF;
                if (slot->has_title != 0)
                {
                    switch (g_menu_scene_type)
                    {        /* switch 1 */
                    case 1:  /* switch 1 */
                    case 4:  /* switch 1 */
                    case 19: /* switch 1 */
                    case 22: /* switch 1 */
                    case 25: /* switch 1 */
                        var_v0 = ((u16)rect->x + (u16)rect->w) - 0x68;
                        break;
                    default: /* switch 1 */
                        var_v0 = ((u16)rect->x + (u16)rect->w) - 0x48;
                        break;
                    }
                    sp80[0] = var_v0;
                    sp80[1] = (u16)rect->y;
                    if (slot->_u.flags & 0x01FF0000)
                    {
                        var_s1 = (s32*)func_800AD208(temp_s2, var_s1, (u16)slot->_u.flags + 1, 3, sp80, 0);
                    }
                    else
                    {
                        var_s1 = (s32*)func_800AD208(temp_s2, var_s1, 0, 3, sp80, 0);
                    }
                    temp_a1_2 = func_800AD524((s32)var_s1, temp_s2, 0xB, sp80, 0);
                    sp80[0] += 8;
                    var_s1 = (s32*)func_800AD208(temp_s2, temp_a1_2, slot->_u._s.unk6 & title_mask, 3, sp80, 0);
                    switch (g_menu_scene_type)
                    {        /* switch 2 */
                    case 1:  /* switch 2 */
                    case 4:  /* switch 2 */
                    case 19: /* switch 2 */
                    case 22: /* switch 2 */
                    case 25: /* switch 2 */
                        temp_s1_2 = func_800AD524((s32)var_s1, temp_s2, 0xB, sp80, 0);
                        sp80[0] += 8;
                        var_s1 = temp_s1_2;
                        var_s1 = (s32*)func_800AD208(temp_s2, var_s1, menu_count_inventory_items(), 3, sp80, 0);
                        break;
                    }
                    var_s1 = menu_emit_slot_scroll_arrows((s32)var_s1, temp_s2, slot);
                }
            }
        }
    }
    sp18.w = 0xFF;
    sp18.h = 0xFF;
    sp18.x = 0;
    sp18.y = 0;
    setTexWindow((DR_TWIN*)var_s1, &sp18);
    addPrim(temp_s2, var_s1);
    prim_cur = var_s1 + PRIM_WORDS(DR_TWIN);
    fill_uv = MENU_TW_FILL;
    if (rect->h >= 0x10)
    {
        sp18.x = (u16)rect->x + 8;
        sp18.y = (u16)rect->y;
        sp18.w = (u16)rect->w - 0x10;
        sp18.h = 8;
        prim_cur = menu_build_h_edge(prim_cur, temp_s2, &sp18, MENU_TW_EDGE_TOP);
        if (rect->h >= 0x10)
        {
            sp18.x = (u16)rect->x + 8;
            sp18.y = ((u16)rect->y + (u16)rect->h) - 8;
            sp18.w = (u16)rect->w - 0x10;
            sp18.h = 8;
            prim_cur = menu_build_h_edge(prim_cur, temp_s2, &sp18, MENU_TW_EDGE_BOT);
        }
    }
    if (rect->w >= 0x20)
    {
        sp18.x = (u16)rect->x;
        sp18.y = (u16)rect->y + 8;
        sp18.w = 8;
        sp18.h = (u16)rect->h - 0x10;
        prim_cur = menu_build_v_edge(prim_cur, temp_s2, &sp18, MENU_TW_EDGE_LEFT);
        if (rect->w >= 0x20)
        {
            sp18.x = ((u16)rect->x + (u16)rect->w) - 8;
            sp18.y = (u16)rect->y + 8;
            sp18.w = 8;
            sp18.h = (u16)rect->h - 0x10;
            prim_cur = menu_build_v_edge(prim_cur, temp_s2, &sp18, MENU_TW_EDGE_RIGHT);
        }
    }
    sp18.x = (u16)rect->x + 8;
    sp18.y = (u16)rect->y + 8;
    sp18.w = (u16)rect->w - 0x10;
    sp18.h = (u16)rect->h - 0x10;
    prim_cur = menu_fill_window_interior(prim_cur, temp_s2, &sp18, fill_uv);
    prim_cur = menu_emit_corner(prim_cur, temp_s2, rect->x, rect->y, MENU_TW_CORNER_TL);
    prim_cur = menu_emit_corner(prim_cur, temp_s2, rect->x + rect->w - 8, rect->y, MENU_TW_CORNER_TR);
    prim_cur = menu_emit_corner(prim_cur, temp_s2, rect->x, rect->y + rect->h - 8, MENU_TW_CORNER_BL);
    temp_v0_2 = menu_emit_corner(prim_cur, temp_s2, rect->x + rect->w - 8, rect->y + rect->h - 8, MENU_TW_CORNER_BR);
    prim_cur = temp_v0_2;
    setDrawTPage((DR_TPAGE*)prim_cur, 0, 0, 5);
    setaddr(prim_cur, getaddr(temp_s2));
    setaddr(temp_s2, temp_v0_2);
    gpu_work->prim_cursor = (s32*)((char*)prim_cur + 8);
}

/**
 * @brief Emit one textured window-corner sprite.
 * @param sprite Primitive buffer location for the sprite.
 * @param ot_entry Ordering-table entry to link the sprite into.
 * @param x Screen X coordinate.
 * @param y Screen Y coordinate.
 * @param uv Packed texture coordinates: U in bits 7:0, V in bits 15:8.
 * @return Primitive buffer location immediately after the sprite.
 * @see decomp.me (100%) https://decomp.me/scratch/GcWsA
 */
SPRT* menu_emit_corner(SPRT* sprite, u_long* ot_entry, s16 x, s16 y, u16 uv)
{
    SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);

    setSprt(sprite);

    SET_SPRT_WH_PACKED(sprite, MENU_WINDOW_CORNER_SIZE, MENU_WINDOW_CORNER_SIZE);

    setXY0(sprite, x, y);

    SET_SPRT_CLUT(sprite, MENU_CLUT_CORNER);
    SET_SPRT_UV0_PACKED(sprite, uv);

    addPrim(ot_entry, sprite);

    return sprite + 1;
}

/**
 * @brief Tile a window interior with textured sprites.
 * @param sprite Primitive buffer location for the first tile.
 * @param ot_entry Ordering-table entry to link the tiles into.
 * @param rect Screen-space region to fill.
 * @param uv Packed texture coordinates: U in bits 7:0, V in bits 15:8.
 * @return Primitive buffer location immediately after the emitted tiles.
 * @see decomp.me (100%) https://decomp.me/scratch/R9mdk
 */
SPRT* menu_fill_window_interior(SPRT* sprite, u_long* ot_entry, const MenuRectU16* rect, u16 uv)
{
    u16 screen_x;
    volatile s32 stack_pad;
    u16 origin_y;
    s32 tile_y = 0;

    if (rect->h > 0)
    {
        do
        {
            s32 tile_x = 0;

            if (rect->w > 0)
            {
                s32 tile_bottom = tile_y + MENU_WINDOW_FILL_TILE_SIZE;

                do
                {
                    SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
                    setSprt(sprite);
                    SET_SPRT_UV0_PACKED(sprite, uv);

                    /* Clamp the final tile in each row and column to the region. */
                    if (rect->w < (tile_x + MENU_WINDOW_FILL_TILE_SIZE))
                    {
                        sprite->w = rect->w - tile_x;
                    }
                    else
                    {
                        sprite->w = MENU_WINDOW_FILL_TILE_SIZE;
                    }

                    if (rect->h < tile_bottom)
                    {
                        sprite->h = rect->h - tile_y;
                    }
                    else
                    {
                        sprite->h = MENU_WINDOW_FILL_TILE_SIZE;
                    }

                    screen_x = rect->x + tile_x;
                    sprite->x0 = screen_x;
                    origin_y = rect->y;
                    tile_x += MENU_WINDOW_FILL_TILE_SIZE;
                    sprite->y0 = origin_y + tile_y;
                    SET_SPRT_CLUT(sprite, MENU_CLUT_GRID_ALT);
                    addPrim(ot_entry, sprite);
                    sprite++;
                } while (tile_x < rect->w);
            }

            tile_y += MENU_WINDOW_FILL_TILE_SIZE;
        } while (tile_y < rect->h);
    }

    return sprite;
}

/**
 * @brief Emit a textured top or bottom window edge.
 * @param packet_cursor Primitive buffer location for the edge.
 * @param ot_entry Ordering-table entry to link the primitives into.
 * @param rect Screen-space edge rectangle.
 * @param texture_origin Packed texture origin: U in bits 7:0, V in bits 15:8.
 * @return Primitive buffer location immediately after the emitted primitives.
 * @see decomp.me (100%) https://decomp.me/scratch/u17Fi
 */
u_long* menu_build_h_edge(
    u_long* packet_cursor,
    u_long* ot_entry,
    const MenuRectU16* rect,
    s32 texture_origin)
{
    RECT texture_window;
    SPRT* sprite;
    DR_TWIN* texture_window_primitive;

    if (rect->w <= 0)
    {
        return packet_cursor;
    }

    if (rect->h > 0)
    {
        sprite = (SPRT*)packet_cursor;
        SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
        setSprt(sprite);
        SET_SPRT_UV0_PACKED(sprite, 0);
        sprite->w = rect->w;
        sprite->h = rect->h;
        sprite->x0 = rect->x;
        sprite->y0 = rect->y;
        sprite->clut = MENU_CLUT_CORNER;
        addPrim(ot_entry, sprite);
        packet_cursor += PRIM_WORDS(SPRT);

        /* Repeat the 16x8 edge texture across the sprite. */
        texture_window_primitive = (DR_TWIN*)packet_cursor;
        texture_window.x = texture_origin & 0xFF;
        texture_window.y = texture_origin >> 8;
        texture_window.w = MENU_WINDOW_EDGE_TEXTURE_LONG_SIDE;
        texture_window.h = MENU_WINDOW_EDGE_TEXTURE_SHORT_SIDE;
        setTexWindow(texture_window_primitive, &texture_window);
        addPrim(ot_entry, texture_window_primitive);
        packet_cursor += PRIM_WORDS(DR_TWIN);
    }

    return packet_cursor;
}

/**
 * @brief Emit a textured left or right window edge.
 * @param packet_cursor Primitive buffer location for the edge.
 * @param ot_entry Ordering-table entry to link the primitives into.
 * @param rect Screen-space edge rectangle.
 * @param texture_origin Packed texture origin: U in bits 7:0, V in bits 15:8.
 * @return Primitive buffer location immediately after the emitted primitives.
 * @see decomp.me (100%) https://decomp.me/scratch/19jr7
 */
u_long* menu_build_v_edge(
    u_long* packet_cursor,
    u_long* ot_entry,
    const MenuRectU16* rect,
    s32 texture_origin)
{
    RECT texture_window;
    SPRT* sprite;
    DR_TWIN* texture_window_primitive;

    if (rect->w <= 0)
    {
        return packet_cursor;
    }

    if (rect->h > 0)
    {
        sprite = (SPRT*)packet_cursor;
        SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
        setSprt(sprite);
        SET_SPRT_UV0_PACKED(sprite, 0);
        sprite->w = rect->w;
        sprite->h = rect->h;
        sprite->x0 = rect->x;
        sprite->y0 = rect->y;
        sprite->clut = MENU_CLUT_CORNER;
        addPrim(ot_entry, sprite);
        packet_cursor += PRIM_WORDS(SPRT);

        /* Repeat the 8x16 edge texture across the sprite. */
        texture_window_primitive = (DR_TWIN*)packet_cursor;
        texture_window.x = texture_origin & 0xFF;
        texture_window.y = texture_origin >> 8;
        texture_window.w = MENU_WINDOW_EDGE_TEXTURE_SHORT_SIDE;
        texture_window.h = MENU_WINDOW_EDGE_TEXTURE_LONG_SIDE;
        setTexWindow(texture_window_primitive, &texture_window);
        addPrim(ot_entry, texture_window_primitive);
        packet_cursor += PRIM_WORDS(DR_TWIN);
    }

    return packet_cursor;
}

/**
 * @brief Draw a sign-dependent label at a screen position.
 * @param ot_entry Ordering-table entry for the label primitives.
 * @param packet_cursor Primitive buffer location for the label.
 * @param position Label screen position.
 * @param value Selects the nonnegative or negative label.
 * @return Updated primitive buffer location.
 * @see decomp.me (100%) https://decomp.me/scratch/ozwB7
 */
u_long* menu_draw_label(u_long* ot_entry, u_long* packet_cursor, const ScreenPos* position, s32 value)
{
    u8 label_buffer[MENU_LABEL_BUFFER_SIZE];
    u8* write_cursor = label_buffer;
    u8* source;

    /* Recover the shared table base from each key's table-relative address. */
    if (value >= 0)
    {
        s32 string_page_base =
            (g_menu_label_key_a.page << 8) +
            (s32)MENU_STRING_TABLE_BASE(g_menu_label_key_a, nonnegative_label);
        source = (u8*)(g_menu_label_key_a.entry + string_page_base);
        func_800A8E28(write_cursor, source);
    }
    else
    {
        s32 string_page_base =
            (g_menu_label_key_b.page << 8) +
            (s32)MENU_STRING_TABLE_BASE(g_menu_label_key_b, negative_label);
        source = (u8*)(g_menu_label_key_b.entry + string_page_base);
        func_800A8E28(write_cursor, source);
    }

    write_cursor += func_800A8DDC(source);

    *write_cursor = 0;

    packet_cursor =
        (u_long*)func_800A88A0(packet_cursor, ot_entry, label_buffer, 1, position->x, position->y, 0);

    return packet_cursor;
}

/**
 * @brief Initialize the full menu node tree and global menu state.
 * @see decomp.me (99.27%) https://decomp.me/scratch/XJkmb
 */
void menu_node_tree_init(void)
{
    MenuNode* var_a0;
    MenuNode* var_a2;
    u32 temp_v0_3;
    u32 temp_v0_5;
    unsigned int temp_v0_7;
    s32 temp_v0_9;
    s32 var_a3;
    s32 var_t0;
    u8* new_var5;
    MenuNode* new_var2;
    int new_var7;
    int new_var10;
    s32 var_t0_2;
    u8 new_var4;
    int new_var8;
    s8 var_v0;
    MenuNode* new_var3;
    u16 temp_v0;
    int new_var6;
    u16 temp_v0_10;
    u16 temp_v0_11;
    u16 temp_v0_12;
    unsigned short temp_v0_13;
    u16 temp_v0_14;
    u16 temp_v0_2;
    u16 temp_v0_4;
    u16 temp_v0_6;
    unsigned int temp_v0_8;
    union
    {
        u16 nav_y_packed;
        struct
        {
            u8 nav_y_hi;
            u8 layout_y_lsb;
        } s;
    }* new_var;
    u16 temp_v1;
    u16 temp_v1_2;
    u32 temp_a0;
    u32 temp_a1;
    u32 var_a1;
    u16 original_unk2;
    u16 temp1;
    u16 temp2;
    u16 loop_unk2;
    u32 new_var9;
    var_t0 = 0;
    var_a0 = g_menu_nodes;
    g_menu_prev_node = MENU_NONE;
    g_menu_content_ready = 0;
    g_item_slot_data.slot0 = 0;
    g_item_slot_data.slot1 = 0;
    g_item_slot_data.slot2 = 0;
    g_item_slot_data.slot3 = 0;
    g_item_slot_flags.slot0 = 0;
    g_item_slot_flags.slot1 = 0;
    g_item_slot_flags.slot2 = 0;
    g_item_slot_flags.slot3 = 0;
    g_menu_item_ptr = 0;
    g_menu_category0_item = 0;
    g_menu_category1_item = 0;
    g_menu_category2_item = 0;
    g_menu_active_equipped_item = 0;
    g_menu_saved_category0_item = 0;
    g_menu_saved_category1_item = 0;
    g_menu_nav_prev[0] = 0;
    g_menu_content_height = 0;
    g_menu_scroll_pos = 0;
    g_menu_redraw_state = 0;
    g_menu_active_node = 0;
    g_menu_cursor_enable = 0;
    do
    {
        loop_unk2 = var_a0[var_t0].u2.unk2;
        var_a0[var_t0].state = MENU_NODE_STATE_UNINIT;
        var_a0[var_t0].icon_id = 0;
        var_a0[var_t0].content_id = MENU_NONE;
        var_a0[var_t0].child3 = MENU_NONE;
        var_a0[var_t0].child2 = MENU_NONE;
        var_a0[var_t0].child1 = MENU_NONE;
        var_a0[var_t0].uA.s.child0 = MENU_NONE;
        var_a0[var_t0].u2.unk2 = (u16)((loop_unk2 & 0xFFFC) | 0x30);
        var_a0[var_t0].u2.s.parent_idx = MENU_NONE;
        var_t0 += 1;
    } while (var_t0 < MENU_NODE_COUNT);

    g_menu_nodes[0].label_id = 1;
    g_menu_nodes[0].idx_nav.s.self_idx = 0;
    original_unk2 = g_menu_nodes[0].u2.unk2;
    temp1 = original_unk2 & 0xFFCD;
    temp2 = original_unk2 & 0xFF0D;
    *((volatile u16*)(&g_menu_nodes[0].u2.unk2)) = temp1;
    *((volatile u16*)(&g_menu_nodes[0].u2.unk2)) = temp2;
    *((volatile u16*)(&g_menu_nodes[0].u2.unk2)) = temp2 | 1;
    g_menu_nodes[0].u2.s.parent_idx = MENU_NONE;
    if (D_800FD818.unk0 & 2)
    {
        g_menu_nodes[0].icon_id = 2;
    }
    else
    {
        g_menu_nodes[0].icon_id = 1;
    }
    g_menu_nodes[0].uA.s.child0 = 1;
    g_menu_nodes[1].idx_nav.s.self_idx = 1;
    g_menu_nodes[0].child1 = 2;
    g_menu_nodes[1].icon_id = 5;
    g_menu_nodes[2].label_id = 2;
    g_menu_nodes[2].idx_nav.s.self_idx = 2;
    g_menu_nodes[1].label_id = 3;
    g_menu_nodes[2].icon_id = 4;
    g_menu_nodes[3].label_id = 4;
    g_menu_nodes[3].idx_nav.s.self_idx = 3;
    g_menu_nodes[1].u2.unk2 = (u16)((g_menu_nodes[1].u2.unk2 & 0xFF0F) | 0x40);
    g_menu_nodes[1].u2.s.parent_idx = 0;
    g_menu_nodes[2].u2.unk2 = (u16)((g_menu_nodes[2].u2.unk2 & 0xFF0F) | 0x40);
    g_menu_nodes[2].u2.s.parent_idx = 0;
    temp_v0_2 = (g_menu_nodes[3].u2.unk2 & 0xFFCD) | 0x10;
    *((volatile u16*)(&g_menu_nodes[3].u2.unk2)) = temp_v0_2;
    temp_v0_3 = 0x10;
    temp_v0_3 = temp_v0_2 | temp_v0_3;
    *((volatile u16*)(&g_menu_nodes[3].u2.unk2)) = (u16)(temp_v0_3 & 0xFF3F);
    *((volatile u16*)(&g_menu_nodes[3].u2.unk2)) = (u16)(temp_v0_3 & 0xFF3E);
    g_menu_nodes[3].u2.s.parent_idx = MENU_NONE;
    if (D_800FDA80 & 2)
    {
        /* Empty conditional forces a basic-block boundary the compiler needs
           to reproduce the target's scheduling here; required to match. */
        if (1)
        {
        }
        g_menu_nodes[3].icon_id = 0x6F;
    }
    else
    {
        g_menu_nodes[3].icon_id = 0x6E;
    }
    g_menu_nodes[4].u2.unk2 = (u16)((0xFF5F & g_menu_nodes[4].u2.unk2) | 0x50);
    g_menu_nodes[5].u2.unk2 = (u16)((g_menu_nodes[5].u2.unk2 & 0xFF5F) | 0x50);
    temp_v0_4 = (g_menu_nodes[6].u2.unk2 & 0xFFCD) | 0x10;
    *((volatile u16*)(&g_menu_nodes[6].u2.unk2)) = temp_v0_4;
    temp_v0_5 = 0x10;
    temp_v0_5 = temp_v0_4 | temp_v0_5;
    *((volatile u16*)(&g_menu_nodes[6].u2.unk2)) = (u16)(temp_v0_5 & 0xFF3F);
    *((volatile u16*)(&g_menu_nodes[6].u2.unk2)) = (u16)(temp_v0_5 & 0xFF3E);
    g_menu_nodes[3].uA.s.child0 = 4;
    g_menu_nodes[3].child1 = 5;
    g_menu_nodes[4].label_id = 6;
    g_menu_nodes[4].idx_nav.s.self_idx = 4;
    g_menu_nodes[4].icon_id = 5;
    g_menu_nodes[5].label_id = 5;
    g_menu_nodes[5].idx_nav.s.self_idx = 5;
    g_menu_nodes[5].icon_id = 4;
    g_menu_nodes[6].label_id = 7;
    g_menu_nodes[6].idx_nav.s.self_idx = 6;
    g_menu_nodes[6].icon_id = 3;
    g_menu_nodes[6].uA.s.child0 = 7;
    g_menu_nodes[6].child1 = 8;
    g_menu_nodes[7].label_id = 9;
    g_menu_nodes[7].idx_nav.s.self_idx = 7;
    g_menu_nodes[7].icon_id = 5;
    new_var7 = 0xFF3E;
    g_menu_nodes[8].label_id = 8;
    g_menu_nodes[4].u2.s.parent_idx = 3;
    g_menu_nodes[5].u2.s.parent_idx = 3;
    g_menu_nodes[6].u2.s.parent_idx = MENU_NONE;
    g_menu_nodes[7].u2.unk2 = (u16)((g_menu_nodes[7].u2.unk2 & 0xFF5F) | 0x50);
    g_menu_nodes[7].u2.s.parent_idx = 6;
    g_menu_nodes[8].u2.unk2 = (u16)((g_menu_nodes[8].u2.unk2 & 0xFF5F) | 0x50);
    g_menu_nodes[8].idx_nav.s.self_idx = 8;
    temp_v0_6 = (g_menu_nodes[9].u2.unk2 & 0xFFCD) | 0x20;
    *((volatile u16*)(&g_menu_nodes[9].u2.unk2)) = temp_v0_6;
    temp_v0_7 = 0x20;
    temp_v0_7 = temp_v0_6 | temp_v0_7;
    *((volatile u16*)(&g_menu_nodes[9].u2.unk2)) = (u16)(temp_v0_7 & 0xFF3F);
    *((volatile u16*)(&g_menu_nodes[9].u2.unk2)) = (u16)(temp_v0_7 & 0xFF3E);
    g_menu_nodes[0xA].u2.unk2 = (u16)((g_menu_nodes[0xA].u2.unk2 & 0xFF6F) | 0x60);
    temp_v0_8 = (g_menu_nodes[0xC].u2.unk2 & 0xFFCD) | 0x20;
    *((volatile u16*)(&g_menu_nodes[0xC].u2.unk2)) = temp_v0_8;
    *((volatile u16*)(&g_menu_nodes[0xC].u2.unk2)) = (u16)((temp_v0_8 | 0x20) & 0xFF3F);
    *((volatile u16*)(&g_menu_nodes[0xC].u2.unk2)) = (u16)((temp_v0_8 | 0x20) & 0xFF3E);
    g_menu_nodes[8].u2.s.parent_idx = 6;
    g_menu_nodes[8].icon_id = 4;
    g_menu_nodes[9].label_id = 0xA;
    g_menu_nodes[9].idx_nav.s.self_idx = 9;
    (g_menu_nodes + 9)->icon_id = 6;
    g_menu_nodes[9].uA.s.child0 = 0xA;
    g_menu_nodes[0xA].label_id = 0xB;
    g_menu_nodes[0xA].idx_nav.s.self_idx = 0xA;
    g_menu_nodes[0xA].icon_id = 7;
    g_menu_nodes[0xC].label_id = 0xA;
    g_menu_nodes[0xC].idx_nav.s.self_idx = 0xC;
    g_menu_nodes[0xC].icon_id = 6;
    g_menu_nodes[0xC].uA.s.child0 = 0xD;
    g_menu_nodes[0xD].label_id = 0xB;
    g_menu_nodes[0xD].idx_nav.s.self_idx = 0xD;
    g_menu_nodes[0xD].icon_id = 7;
    g_menu_nodes[9].u2.s.parent_idx = MENU_NONE;
    g_menu_nodes[0xA].u2.s.parent_idx = 9;
    g_menu_nodes[0xC].u2.s.parent_idx = MENU_NONE;
    g_menu_nodes[0xD].u2.unk2 = (u16)((g_menu_nodes[0xD].u2.unk2 & 0xFF6F) | 0x60);
    g_menu_nodes[0xD].u2.s.parent_idx = 0xC;
    temp_v0_10 = (g_menu_nodes[0xF].u2.unk2 & 0xFFCD) | 0x20;
    *((volatile u16*)(&g_menu_nodes[0xF].u2.unk2)) = temp_v0_10;
    temp_v0_11 = (temp_v0_10 & 0xFF6D) | 0x60;
    *((volatile u16*)(&g_menu_nodes[0xF].u2.unk2)) = temp_v0_11;
    *((volatile u16*)(&g_menu_nodes[0xF].u2.unk2)) = (u16)(temp_v0_11 & 0xFFFE);
    g_menu_nodes[0xF].icon_id = 8;
    g_menu_nodes[0x10].icon_id = 7;
    g_menu_nodes[0xF].label_id = 0xD;
    g_menu_nodes[0xF].idx_nav.s.self_idx = 0xF;
    g_menu_nodes[0xF].u2.s.parent_idx = MENU_NONE;
    g_menu_nodes[0xF].uA.s.child0 = 0x10;
    g_menu_nodes[0xF].child1 = 0x11;
    g_menu_nodes[0x10].label_id = 0xC;
    g_menu_nodes[0x10].idx_nav.s.self_idx = 0x10;
    g_menu_nodes[0x11].label_id = 0xE;
    g_menu_nodes[0x11].idx_nav.s.self_idx = 0x11;
    g_menu_nodes[0x11].icon_id = 9;
    g_menu_nodes[0x12].label_id = 0x10;
    g_menu_nodes[0x12].idx_nav.s.self_idx = 0x12;
    g_menu_nodes[0x12].icon_id = 0xA;
    g_menu_nodes[0x12].content_id = 4;
    g_menu_nodes[0x12].uA.s.child0 = 0x13;
    g_menu_nodes[0x12].child1 = 0x16;
    g_menu_nodes[0x12].child2 = 0x19;
    g_menu_nodes[0x12].child3 = 0x1C;
    g_menu_nodes[0x10].u2.unk2 = (u16)((g_menu_nodes[0x10].u2.unk2 & 0xFF6F) | 0x60);
    g_menu_nodes[0x10].u2.s.parent_idx = 0xF;
    g_menu_nodes[0x11].u2.unk2 = (u16)((g_menu_nodes[0x11].u2.unk2 & 0xFF6F) | 0x60);
    new_var9 = g_menu_nodes[0x12].u2.unk2;
    temp_v0 = new_var9;
    g_menu_nodes[0x11].u2.s.parent_idx = 0xF;
    *((volatile u16*)(&g_menu_nodes[0x12].u2.unk2)) = (u16)(temp_v0 & 0xFFFD);
    temp_v0_12 = temp_v0 & 0xFF3D;
    *((volatile u16*)(&g_menu_nodes[0x12].u2.unk2)) = temp_v0_12;
    *((volatile u16*)(&g_menu_nodes[0x12].u2.unk2)) = (u16)(temp_v0_12 | 1);
    g_menu_nodes[0x12].u2.s.parent_idx = MENU_NONE;
    g_menu_nodes[0x13].label_id = 0x11;
    g_menu_nodes[0x16].content_id = 1;
    g_menu_nodes[0x14].icon_id = 0xF;
    g_menu_nodes[0x13].icon_id = 0xB;
    g_menu_nodes[0x13].idx_nav.s.self_idx = 0x13;
    g_menu_nodes[0x13].content_id = 0;
    g_menu_nodes[0x13].uA.s.child0 = 0x14;
    g_menu_nodes[0x13].child1 = 0x15;
    g_menu_nodes[0x14].label_id = 0x12;
    g_menu_nodes[0x14].idx_nav.s.self_idx = 0x14;
    g_menu_nodes[0x15].label_id = 0x13;
    g_menu_nodes[0x15].idx_nav.s.self_idx = 0x15;
    g_menu_nodes[0x15].icon_id = 0x12;
    g_menu_nodes[0x16].label_id = 0x14;
    g_menu_nodes[0x16].idx_nav.s.self_idx = 0x16;
    g_menu_nodes[0x16].icon_id = 0xC;
    g_menu_nodes[0x16].uA.s.child0 = 0x17;
    g_menu_nodes[0x16].child1 = 0x18;
    g_menu_nodes[0x17].label_id = 0x15;
    g_menu_nodes[0x17].idx_nav.s.self_idx = 0x17;
    g_menu_nodes[0x13].u2.unk2 = (u16)((g_menu_nodes[0x13].u2.unk2 & 0xFF3F) | 0x40);
    g_menu_nodes[0x14].u2.unk2 = (u16)((g_menu_nodes[0x14].u2.unk2 & 0xFF3F) | 0x80);
    g_menu_nodes[0x13].u2.s.parent_idx = 0x12;
    g_menu_nodes[0x14].u2.s.parent_idx = 0x13;
    g_menu_nodes[0x16].u2.unk2 = (u16)((g_menu_nodes[0x16].u2.unk2 & 0xFF3F) | 0x40);
    g_menu_nodes[0x16].u2.s.parent_idx = 0x12;
    g_menu_nodes[0x15].u2.unk2 = (u16)((g_menu_nodes[0x15].u2.unk2 & 0xFF3F) | 0x80);
    g_menu_nodes[0x15].u2.s.parent_idx = 0x13;
    g_menu_nodes[0x17].u2.unk2 = (u16)((g_menu_nodes[0x17].u2.unk2 & 0xFF3F) | 0x80);
    g_menu_nodes[0x17].u2.s.parent_idx = 0x16;
    g_menu_nodes[0x19].content_id = 2;
    g_menu_nodes[0x17].icon_id = 0x10;
    g_menu_nodes[0x18].label_id = 0x13;
    g_menu_nodes[0x18].idx_nav.s.self_idx = 0x18;
    g_menu_nodes[0x18].icon_id = 0x12;
    g_menu_nodes[0x19].label_id = 0x16;
    g_menu_nodes[0x19].idx_nav.s.self_idx = 0x19;
    g_menu_nodes[0x19].icon_id = 0xD;
    g_menu_nodes[0x19].uA.s.child0 = 0x1A;
    g_menu_nodes[0x19].child1 = 0x1B;
    g_menu_nodes[0x1A].label_id = 0x17;
    g_menu_nodes[0x1A].icon_id = 0x11;
    g_menu_nodes[0x1A].idx_nav.s.self_idx = 0x1A;
    g_menu_nodes[0x1B].label_id = 0x13;
    g_menu_nodes[0x1B].idx_nav.s.self_idx = 0x1B;
    g_menu_nodes[0x1B].icon_id = 0x12;
    g_menu_nodes[0x1C].label_id = 0x18;
    g_menu_nodes[0x1C].idx_nav.s.self_idx = 0x1C;
    g_menu_nodes[0x18].u2.unk2 = (u16)((g_menu_nodes[0x18].u2.unk2 & 0xFF3F) | 0x80);
    g_menu_nodes[0x19].u2.unk2 = (u16)((g_menu_nodes[0x19].u2.unk2 & 0xFF3F) | 0x40);
    g_menu_nodes[0x18].u2.s.parent_idx = 0x16;
    g_menu_nodes[0x19].u2.s.parent_idx = 0x12;
    g_menu_nodes[0x1B].u2.unk2 = (u16)((g_menu_nodes[0x1B].u2.unk2 & 0xFF3F) | 0x80);
    g_menu_nodes[0x1B].u2.s.parent_idx = 0x19;
    g_menu_nodes[0x1A].u2.unk2 = (u16)((g_menu_nodes[0x1A].u2.unk2 & 0xFF3F) | 0x80);
    g_menu_nodes[0x1C].u2.unk2 = (u16)((g_menu_nodes[0x1C].u2.unk2 & 0xFF3F) | 0x40);
    g_menu_nodes[0x1C].u2.s.parent_idx = 0x12;
    g_menu_nodes[0x1A].u2.s.parent_idx = 0x19;
    g_menu_nodes[0x1C].icon_id = 0xE;
    g_menu_nodes[0x1C].content_id = 5;
    g_menu_nodes[0x1D].label_id = 0x1C;
    g_menu_nodes[0x1D].content_id = 3;
    g_menu_nodes[0x1D].icon_id = 0x18;
    g_menu_nodes[0x1D].idx_nav.s.self_idx = 0x1D;
    g_menu_nodes[0x1E].label_id = 0x19;
    g_menu_nodes[0x1E].idx_nav.s.self_idx = 0x1E;
    g_menu_nodes[0x1E].uA.s.child0 = 0x1F;
    g_menu_nodes[0x1F].idx_nav.s.self_idx = 0x1F;
    (g_menu_nodes + 0x1E)->icon_id = 0x13;
    g_menu_nodes[0x1F].label_id = 0x1A;
    g_menu_nodes[0x1F].icon_id = 0x14;
    g_menu_nodes[0x2B].label_id = 0x1A;
    g_menu_nodes[0x2B].idx_nav.s.self_idx = 0x2B;
    temp_v0 = g_menu_nodes[0x1D].u2.unk2;
    *((volatile u16*)(&g_menu_nodes[0x1D].u2.unk2)) = (u16)(temp_v0 & 0xFFFD);
    temp_v0_13 = temp_v0 & 0xFF3D;
    *((volatile u16*)(&g_menu_nodes[0x1D].u2.unk2)) = temp_v0_13;
    temp_v1 = g_menu_nodes[0x1E].u2.unk2;
    *((volatile u16*)(&g_menu_nodes[0x1D].u2.unk2)) = (u16)(temp_v0_13 | 1);
    g_menu_nodes[0x1D].u2.s.parent_idx = MENU_NONE;
    g_menu_nodes[0x1F].u2.unk2 = (u16)((g_menu_nodes[0x1F].u2.unk2 & 0xFF3F) | 0x40);
    g_menu_nodes[0x1F].u2.s.parent_idx = 0x1E;
    *((volatile u16*)(&g_menu_nodes[0x1E].u2.unk2)) = (u16)(temp_v1 & 0xFFFD);
    temp_v1_2 = temp_v1 & 0xFF3D;
    *((volatile u16*)(&g_menu_nodes[0x1E].u2.unk2)) = temp_v1_2;
    g_menu_nodes[0x2B].u2.unk2 = (u16)((g_menu_nodes[0x2B].u2.unk2 & 0xFF3F) | 0x40);
    *((volatile u16*)(&g_menu_nodes[0x1E].u2.unk2)) = (u16)(temp_v1_2 | 1);
    g_menu_nodes[0x1E].u2.s.parent_idx = MENU_NONE;
    g_menu_nodes[0x2B].u2.s.parent_idx = 0x1E;
    g_menu_nodes[0x1F].u2.unk2 = (u16)(g_menu_nodes[0x1F].u2.unk2 & 0xFFCF);
    g_menu_nodes[0x2B].u2.unk2 = (u16)((g_menu_nodes[0x2B].u2.unk2 & 0xFFCF) | 0x10);
    g_menu_nodes[0x2B].icon_id = 0x15;
    g_menu_nodes[0x20].label_id = 0x1B;
    g_menu_nodes[0x20].idx_nav.s.self_idx = 0x20;
    {
        u16 block_v0;
        u16 block_v14;
        block_v0 = g_menu_nodes[0x20].u2.unk2;
        block_v14 = block_v0 & 0xFF3D;
        *((volatile u16*)(&g_menu_nodes[0x20].u2.unk2)) = (u16)(block_v0 & 0xFFFD);
        *((volatile u16*)(&g_menu_nodes[0x20].u2.unk2)) = (u16)block_v14;
        g_menu_nodes[0x20].icon_id = 0x16;
        *((volatile u16*)(&g_menu_nodes[0x20].u2.unk2)) = (u16)(block_v14 | 1);
    }
    g_menu_nodes[0x20].u2.s.parent_idx = MENU_NONE;
    if (D_800FD818.unk268 & 1)
    {
        if (D_800FD818.unk26B != 0)
        {
            g_menu_nodes[6].u2.unk2 = (u16)(g_menu_nodes[6].u2.unk2 | 1);
        }
        else
        {
            g_menu_nodes[3].u2.unk2 = (u16)((g_menu_nodes + 3)->u2.unk2 | 1);
        }
    }
    if (D_800FDCE8 & 1)
    {
        if ((g_pad_ctx->unkAA8 & 0x7F) == 4)
        {
            g_menu_nodes[0xF].u2.unk2 = (u16)(g_menu_nodes[0xF].u2.unk2 | 1);
        }
        else
        {
            g_menu_nodes[9].u2.unk2 = (u16)(g_menu_nodes[9].u2.unk2 | 1);
        }
    }
    var_a3 = 0;
    if ((g_pad_ctx->inject_flags & 0x80) && (g_pad_ctx->inject_enable != 0))
    {
        g_menu_companion_node = 0x2B;
        var_a3 = 0;
    }
    var_t0 = var_a3;
    var_t0_2 = var_a3;
    var_a2 = g_menu_nodes;
    do
    {
        if (var_a2->u2.s.parent_idx == MENU_NONE)
        {
            if (var_a2->u2.s.flags & 1)
            {
                temp_a0 = var_a3 & 0xFFFF;
                temp_a1 = var_a3 & 0x1FF;
                var_a3 += MENU_ROW_HEIGHT;
                new_var10 = (temp_a0 & 1) << 15;
                new_var6 = (temp_a0 >> 1) & 0xFF;
                var_a2->idx_nav.nav_x_packed = (u16)(var_a2->idx_nav.nav_x_packed & 0x80FF);
                var_a2->u8_u.nav_y_packed = (u16)(var_a2->u8_u.nav_y_packed & 0x80FF);
                var_a2->uA.layout_child_packed = (u16)((var_a2->uA.layout_child_packed & 0xFF00) | new_var6);
                new_var8 = (temp_a1 & 1) << 15;
                var_a2->u8_u.nav_y_packed = (u16)((var_a2->u8_u.nav_y_packed & 0x7FFF) | new_var10);
                var_a2->idx_nav.nav_x_packed = (u16)((var_a2->idx_nav.nav_x_packed & 0x7FFF) | new_var8);
                var_a1 = temp_a1 >> 1;
                var_a2->u8_u.nav_y_packed = (u16)((var_a2->u8_u.nav_y_packed & 0xFF00) | var_a1);
            }
        }
        var_t0 += 1;
        var_a2++;
    } while (var_t0 < MENU_NODE_COUNT);
    if (g_active_script != 0)
    {
        g_menu_scene_type = -1;
        return;
    }
    g_menu_scene_type = 0;
    new_var4 = g_menu_init_content_id;
    g_menu_ability_mask = 0;
    menu_open_content_page(new_var4);
    menu_set_active_node();
}

/**
 * @brief Collapse every menu node while preserving its other state flags.
 * @see decomp.me (100%) https://decomp.me/scratch/hyDM7
 */
void menu_collapse_all(void)
{
    s32 node_index;

    for (node_index = 0; node_index < MENU_NODE_COUNT; node_index++)
    {
        g_menu_nodes[node_index].u2.unk2 &= ~MENU_NODE_FLAG_EXPANDED;
    }
}

/**
 * @brief Rebuild the visible node layout and update its scroll state.
 * @see decomp.me (100%) https://decomp.me/scratch/YhGni
 */
void menu_update_layout(void)
{
    s32 has_visible_children = 0;
    s32 layout_end = has_visible_children;
    s32 node_index = layout_end;

    do
    {
        if (g_menu_nodes[node_index].u2.s.parent_idx == MENU_NONE)
        {
            s32 root_y = layout_end;
            layout_end++;
            layout_end--;

            if (g_menu_nodes[node_index].u2.s.flags & MENU_NODE_FLAG_ACTIVE)
            {
                layout_end = menu_layout_node(node_index, root_y);

                /* A root that contributes multiple rows has visible descendants. */
                if (root_y != (layout_end - MENU_ROW_HEIGHT))
                {
                    has_visible_children = 1;
                    if (layout_end > MENU_VIEW_HEIGHT)
                    {
                        g_menu_scroll_pos = layout_end - MENU_VIEW_HEIGHT;
                        g_menu_redraw_state = MENU_REDRAW_LAYOUT;
                    }
                }
            }
        }
        node_index += 1;
    } while (node_index < MENU_NODE_COUNT);

    g_menu_layout_end = layout_end;
    if (has_visible_children == 0)
    {
        g_menu_scroll_pos = 0;
        g_menu_redraw_state = MENU_REDRAW_LAYOUT;
    }
}

/**
 * @brief Assigns a position slot to a menu node and optionally recurses into its first child.
 * @param node_idx Index into g_menu_nodes of the node to lay out.
 * @param base_pos Running position counter; this node occupies [base_pos, base_pos + MENU_ROW_HEIGHT).
 * @return Updated position counter after processing this node and any expanded children.
 * @see decomp.me (100%) https://decomp.me/scratch/LDCeT
 */
s32 menu_layout_node(s32 node_idx, s32 base_pos)
{
    MenuNode* temp_a0;
    s32 cur_pos;
    int child_iter;
    MenuNode* node;
    int is_expanded;
    u32 layout_y; /* base_pos clamped to 16 bits; packed as 9-bit value into layout_y_lsb/layout_y_hi */
    /* Keep this alias distinct for target register allocation. */
    union
    {
        u16 nav_y_packed;
        struct
        {
            u8 nav_y_hi;
            u8 layout_y_lsb;
        } s;
    }* u8_alias;
    MenuNode* node2; /* Separate alias required for target register allocation. */
    cur_pos = base_pos;
    is_expanded = (((u16)(&g_menu_nodes[node_idx])->u2.unk2) >> 1) & 1;
    layout_y = cur_pos & 0xFFFF;
    cur_pos += MENU_ROW_HEIGHT;
    node = &g_menu_nodes[node_idx];

    (*(&g_menu_nodes[node_idx])).state = MENU_NODE_STATE_LAID_OUT;
    u8_alias = &node->u8_u;
    /* Pack 9-bit layout Y: bit 0 goes into MENU_LAYOUT_Y0_BIT of u8_u.nav_y_packed (layout_y_lsb bit 7);
     * bits 1-8 go into layout_y_hi. Reconstruct: (layout_y_hi << 1) | (layout_y_lsb >> 7). */
    node->u8_u.nav_y_packed = (*u8_alias).s.nav_y_hi | ((layout_y & 1) << 15);
    /* Preserve the target duplicate store. */
    (&g_menu_nodes[node_idx])->uA.layout_child_packed = ((&g_menu_nodes[node_idx])->uA.layout_child_packed & 0xFF00) | (0xFF & (layout_y >> 1));
    node->uA.layout_child_packed = (node->uA.layout_child_packed & 0xFF00) | ((layout_y >> 1) & 0xFF);
    if (is_expanded)
    {
        s32 child_idx;
        child_idx = 0;
        node2 = node;
        for (; child_idx < MENU_MAX_CHILDREN;)
        {
            if (*((u8*)node2 + child_idx + 0xB) == MENU_NONE) break;
            cur_pos = menu_layout_node(*((u8*)node2 + child_idx++ + 0xB), cur_pos);
        }
    }
    return cur_pos;
}

/**
 * @brief Draw the menu frame layers and optionally dispatch navigation input.
 * @param packet_cursor Primitive buffer location for the frame.
 * @param ot_entry Ordering-table entry for the frame primitives.
 * @param frame_parity Selects the active double-buffered VRAM page.
 * @param allow_input Nonzero to dispatch navigation input.
 * @return Primitive buffer location immediately after the frame.
 * @see decomp.me (100%) https://decomp.me/scratch/x8WyZ
 */
u8* menu_draw_frame(u8* packet_cursor, u_long* ot_entry, s32 frame_parity, s32 allow_input)
{
    DRAWENV draw_env;
    u8* draw_env_packet;
    DR_TPAGE* draw_mode;
    u8* frame_cursor;
    s32 draw_y;
    s32 node_tree_end;
    MenuControllerActuatorState* actuator_state;
    u8* frame_end;

    actuator_state = MENU_CONTROLLER_ACTUATORS;

    /* Emit the full-screen draw environment before the menu layers. */
    draw_env_packet = (u8*)menu_draw_scene_content(packet_cursor);
    draw_y = frame_parity ? SCREEN_HEIGHT : VRAM_BACK_DRAW_Y;
    SetDefDrawEnv(&draw_env, 0, draw_y, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    SetDrawEnv(draw_env_packet, &draw_env);
    addPrim(ot_entry, draw_env_packet);

    draw_env_packet += sizeof(DR_ENV);
    frame_cursor = draw_env_packet;

    /* Fade the large-motor command and mirror it to the second port. */
    if (actuator_state->ports[0].large_motor_command != 0)
    {
        u8 motor_command = actuator_state->ports[0].large_motor_command - 1;
        actuator_state->ports[1].large_motor_command = motor_command;
        actuator_state->ports[0].large_motor_command = motor_command;
    }

    /* Render and update the active node or content cursor mode. */
    switch (g_menu_cursor_enable)
    {
    case MENU_CURSOR_MODE_NODE_TREE:
        frame_cursor = (u8*)menu_draw_active_node_cursor(draw_env_packet, ot_entry - 1, allow_input);
        menu_handle_input(0);
        if (allow_input != 0)
        {
            menu_handle_node_input();
        }
        break;

    case MENU_CURSOR_MODE_CONTENT:
        frame_cursor = (u8*)menu_draw_content_cursor(draw_env_packet, ot_entry - 1, allow_input);
        if (g_menu_suppress_cursor == 0)
        {
            menu_handle_input(allow_input);
        }
        break;

    case MENU_CURSOR_MODE_CONTENT_EXIT:
        frame_cursor = (u8*)menu_draw_content_cursor(draw_env_packet, ot_entry - 1, allow_input);
        if (g_menu_suppress_cursor == 0)
        {
            g_menu_cursor_enable = MENU_CURSOR_MODE_NODE_TREE;
        }
        break;

    default:
        break;
    }

    /* Draw the node tree and its scroll indicators. */
    node_tree_end = menu_draw_node_tree(frame_cursor, ot_entry);
    frame_cursor = (u8*)menu_emit_tree_scroll_arrows(node_tree_end, ot_entry - 1);

    draw_mode = (DR_TPAGE*)frame_cursor;
    setDrawTPage(draw_mode, 0, 0, MENU_GRID_TPAGE);
    addPrim(ot_entry, draw_mode);
    draw_env_packet = (u8*)(draw_mode + 1);

    /* Restrict the final draw environment to the node-tree viewport. */
    draw_y = frame_parity ? SCREEN_HEIGHT + MENU_TREE_DRAW_Y_OFFSET
                          : VRAM_BACK_DRAW_Y + MENU_TREE_DRAW_Y_OFFSET;
    frame_end = draw_env_packet + sizeof(DR_ENV);
    SetDefDrawEnv(&draw_env, MENU_TREE_DRAW_X, draw_y, MENU_TREE_DRAW_WIDTH, MENU_TREE_DRAW_HEIGHT);
    SetDrawEnv(draw_env_packet, &draw_env);
    addPrim(ot_entry, draw_env_packet);

    return frame_end;
}

/**
 * @brief Process D-pad and face-button input to navigate and select menu nodes.
 * @return Undefined; callers ignore the nominal return value.
 * @see decomp.me (100%) https://decomp.me/scratch/YoOml
 */
unsigned int menu_handle_node_input(void)
{
    MenuNode* temp_a1;
    s32 temp_v0;
    s32 new_var15;
    MenuNode* new_var11;
    const u32 new_var14;
    int new_var13;
    int new_var4;
    s32 temp_v1;
    s32 temp_v0_3;
    s32 temp_a0_3;
    int new_var3;
    union
    {
        u16 unk6;
        struct
        {
            u8 self_idx;
            u8 unk7;
        } s;
    }* new_var8;
    s32 temp_a0;
    s16* new_var;
    s32 var_v1_2;
    s32 new_var10;
    unsigned char new_var5;
    int new_var7;
    char new_var9;
    u8* var_a0;
    u8* var_v0;
    unsigned int new_var12;
    MenuContentItem* temp_v1_2;
    u8* new_var6;
    int new_var2;
    const u32 browse_all_node = MENU_NODE_BROWSE_ALL; /* Local form preserves target codegen. */
    const u8 SENTINEL;
    temp_v0 = menu_find_nav_node_index(g_menu_active_node);
    if (temp_v0 == (-1))
    {
        return;
    }
    if (g_pad_input & PAD_BTN_UP)
    {
        if (temp_v0 != 0)
        {
            g_menu_active_node = g_menu_nav_prev[temp_v0];
        }
        else
        {
            g_menu_active_node = g_menu_nav_prev[g_menu_nav_count];
        }
    }
    if (g_pad_input & PAD_BTN_DOWN)
    {
        if (temp_v0 >= (g_menu_nav_count - 1))
        {
            g_menu_active_node = g_menu_nav_first;
        }
        else
        {
            g_menu_active_node = g_menu_nav_next[temp_v0];
        }
    }
    if (g_pad_input & PAD_BTN_CIRCLE)
    {
        if (g_menu_active_node == browse_all_node)
        {
            menu_play_se(MENU_SE_CLOSE, MENU_SE_VOLUME);
            g_menu_load_request = 1;
            return;
        }
        g_menu_active_node = g_menu_nav_first;
        g_menu_active_node = browse_all_node;
    }
    if ((PAD_BTN_UP | PAD_BTN_DOWN | PAD_BTN_CIRCLE) & (g_pad_input & 0xFFFFu))
    {
        menu_play_se(MENU_SE_NAVIGATE, MENU_SE_VOLUME);
        temp_a0 = menu_find_nav_node_index(g_menu_active_node);
        temp_a0 = temp_a0 * MENU_ROW_HEIGHT;
        var_v1_2 = temp_a0 - g_menu_scroll_pos;
        temp_v1 = var_v1_2;
        if (temp_v1 < 0)
        {
            g_menu_scroll_pos = temp_a0;
            g_menu_redraw_state = MENU_REDRAW_NAVIGATE;
        }
        else if (temp_v1 >= MENU_VIEW_HEIGHT)
        {
            g_menu_scroll_pos = temp_a0 - 0x98; /* 0xAB - 0x13: scroll so item is last row */
            g_menu_redraw_state = MENU_REDRAW_NAVIGATE;
        }
        return;
    }
    /* 0x0200 = undocumented bit, likely L3 (DualShock stick click) - never set on digital pad */
    if (g_pad_input & (PAD_BTN_RIGHT | PAD_BTN_CROSS | 0x0200))
    {
        menu_play_se(MENU_SE_SELECT, MENU_SE_VOLUME);
        new_var14 = browse_all_node;
        if (g_menu_active_node == new_var14)
        {
            if (!(g_pad_input & (PAD_BTN_CROSS | 0x0200)))
            {
                return;
            }
            SENTINEL = 0xFF;
            g_menu_load_request = 1;
        }
        if (g_menu_active_node == 0x11)
        {
            g_menu_load_request = 1;
            g_menu_transition_code = 0xA;
            return;
        }
        new_var11 = g_menu_nodes;
        temp_a1 = &g_menu_nodes[g_menu_active_node];
        temp_v0_3 = 0xFF;
        (&g_menu_nodes[g_menu_active_node])->u2.unk2 |= 0xC;
        new_var10 = temp_v0_3;
        if (temp_a1->u2.s.parent_idx == temp_v0_3) /* temp_v0_3 = MENU_NONE (0xFF) */
        {
            g_menu_category2_item = (g_menu_category1_item = (g_menu_category0_item = (g_menu_item_ptr = 0)));
        }
        g_menu_prev_node = temp_v0_3;
        new_var8 = &temp_a1->idx_nav;
        if (g_menu_scene_type != g_menu_active_node)
        {
            if (g_menu_content_table[temp_a1->idx_nav.s.self_idx] != NULL)
            {
                var_v1_2 = 3;
                temp_a0_3 = new_var14;
                new_var6 = &g_menu_slots;
                g_menu_scene_type = g_menu_active_node;
                var_v0 = new_var6 + 0x6C;
                for (var_v1_2 = 3; var_v1_2 >= 0; var_v1_2--)
                {
                    *var_v0 = 0;
                    var_v0 -= 0x24;
                }

                var_v1_2 = 3;
                if (g_menu_nodes[g_menu_scene_type].content_id != MENU_NONE)
                {
                    g_menu_ability_mask = 0;
                    menu_open_content_page(g_menu_nodes[g_menu_scene_type].content_id, temp_a1, new_var14, temp_v0_3);
                }
            }
            menu_set_active_node();
            return;
        }
        if (g_menu_scene_type == (-1))
        {
            return;
        }
        if (g_menu_scene_type == new_var14)
        {
            if (g_pad_input & (PAD_BTN_CROSS | 0x0200))
            {
                g_menu_load_request = 1;
            }
            return;
        }
        temp_a0_3 = (*new_var8).unk6 >> 0xF;
        new_var4 = temp_a1->u8_u.s.nav_y_hi;
        new_var12 = temp_a0_3;
        new_var15 = g_menu_content_height;
        g_content_cursor_y = MENU_CURSOR_Y_MIN;
        g_content_cursor_y = ((new_var4 * 2) | new_var12) - (new_var15 - g_content_cursor_y);
        if (g_content_cursor_y < MENU_CURSOR_Y_MIN)
        {
            g_content_cursor_y = MENU_CURSOR_Y_MIN;
        }
        if (g_content_cursor_y >= MENU_CURSOR_Y_MAX)
        {
            g_content_cursor_y = MENU_CURSOR_Y_MAX;
        }
        g_content_cursor_x = (((temp_a1->idx_nav.nav_x_packed >> 4) >> 4) & 0x7F) + 8;
        if (MENU_NONE != (&g_menu_nodes[g_menu_active_node])->content_id)
        {
            g_menu_cursor_enable = 1;
            var_v1_2 = 0;
            var_a0 = &g_menu_slots;
            while (1)
            {
                if ((*var_a0) != 0)
                {
                    break;
                }
                var_v1_2++;
                var_a0 += 0x24;
                if (var_v1_2 >= 4)
                {
                    do
                    {
                    } while (0);
                    return;
                }
            }

            g_menu_suppress_cursor = MENU_CURSOR_REVEAL_DELAY;
            g_content_view_x = g_menu_default_view_pos.x;
            new_var = &g_menu_default_view_pos.y;
            g_content_view_y = *new_var;
        }
        else
        {
            g_menu_hit_item_idx = menu_find_active_content_item();
            if (g_menu_hit_item_idx != (-1))
            {
                temp_v1_2 = g_menu_content_table[new_var11[g_menu_scene_type].idx_nav.s.self_idx] - (-g_menu_hit_item_idx);
                g_content_view_x = temp_v1_2->packed_x & 0x1FF;
                new_var3 = temp_v1_2->y - 8;
                g_menu_suppress_cursor = MENU_CURSOR_REVEAL_DELAY;
                g_menu_cursor_enable = 1;
                g_content_view_y = new_var3;
            }
        }
    }
}

/**
 * @brief Focus the active content item and snap the viewport to its position.
 * @return 1 if an active item was found; otherwise 0.
 * @see decomp.me (100%) https://decomp.me/scratch/q39Ou
 */
s32 menu_focus_active_content_item(void)
{
    MenuContentItem* content_items;
    MenuContentItem* active_item;
    s32 view_y;

    g_menu_hit_item_idx = menu_find_active_content_item();
    if (g_menu_hit_item_idx != (-1))
    {
        MenuNode* nodes = g_menu_nodes;
        u8 content_table_idx = (nodes + g_menu_scene_type)->idx_nav.s.self_idx;

        content_items = g_menu_content_table[content_table_idx];
        active_item = content_items - (-g_menu_hit_item_idx);
        g_content_view_x = active_item->packed_x & MENU_CONTENT_X_MASK;
        view_y = active_item->y - MENU_CONTENT_VIEW_Y_OFFSET;
        g_menu_suppress_cursor = MENU_CURSOR_REVEAL_DELAY;
        g_menu_cursor_enable = MENU_CURSOR_MODE_CONTENT;
        g_content_view_y = view_y;
        return 1;
    }
    return 0;
}

/** @brief Active character slot index: 0 = char slot 0 (node 0x1F), 1 = char slot 1 (node 0x2B). */
extern s32 g_menu_char_slot;

/**
 * @brief Mark the active node's ancestor chain as expanded, propagate its nav cursor position to its children, update g_menu_char_slot, and re-run the full layout.
 * @see decomp.me (100%) https://decomp.me/scratch/BF56X
 */
void menu_set_active_node()
{
    s32 node_idx;
    s32 walk_off;
    MenuNode* walk_base;
    MenuNode* curr_node;
    MenuNode* active_node;
    MenuNode* active_base;
    MenuNode* loop_active;
    u16 temp_v0;
    s32 active_idx;
    s32 char_slot_bits;
    s32 var_s3;     /* parent-walk: current ancestor index; layout: scroll-adjusted flag */
    s32 layout_pos; /* parent-walk: reused as temp for parent_idx check; layout: y accumulator */
    s32 node_i;
    MenuNode* node;
    s32 prev_layout_y;
    long child_slot;
    s32 child_idx;
    s32 child_idx2;
    u16 child_wide;
    MenuNode* child_node;
    s32 child_base_addr;
    u16 nav_col; /* bits [14:8] of nav_x_packed: 7-bit column, copied to children */
    s32 parent_packed;
    s32 parent_y_tmp;

    /* Clear the "expanded" bit (bit 1) on every node, then re-expand only the active path. */
    for (node_idx = 0; node_idx < MENU_NODE_COUNT; node_idx++)
    {
        g_menu_nodes[node_idx].u2.unk2 &= 0xFFFD;
    }

    /* Walk from g_menu_active_node up to the root, marking each ancestor expanded. */
    child_slot = g_menu_active_node;
    walk_base = g_menu_nodes;
    walk_off = child_slot << 4;
    if (((MenuNode*)((u8*)walk_base + walk_off))->u2.s.parent_idx != MENU_NONE)
    {
        do
        {
            child_slot = ((MenuNode*)(walk_off + (s32)walk_base))->u2.s.parent_idx;
            walk_off = child_slot << 4;
            ((MenuNode*)(walk_off + (s32)walk_base))->u2.unk2 |= 2;
        } while (((MenuNode*)(walk_off + (s32)walk_base))->u2.s.parent_idx != MENU_NONE);
    }

    /* Mark the active node itself expanded, then propagate its nav cursor to children. */
    active_base = g_menu_nodes;
    active_idx = g_menu_active_node;
    active_node = active_base + active_idx;
    temp_v0 = active_node->u2.unk2 | 2;
    active_node->u2.unk2 = temp_v0;
    if ((temp_v0 >> 1) & 1)
    {
        child_slot = 0;
        child_base_addr = (s32)active_base;
        loop_active = active_node;
        for (; child_slot < MENU_MAX_CHILDREN; child_slot++)
        {
            child_idx = *((u8*)loop_active + child_slot + 0xB);
            child_wide = child_idx;
            if (child_idx == (temp_v0 = MENU_NONE))
            {
                break;
            }
            child_node = (MenuNode*)(((u32)child_wide << 4) + child_base_addr);
            /* Propagate nav column X (bits [14:8]) from parent to child. */
            nav_col = loop_active->idx_nav.nav_x_packed & MENU_NAV_X_MASK;
            child_node->idx_nav.nav_x_packed = child_node->idx_nav.nav_x_packed & MENU_NAV_X_CLEAR;
            child_node->idx_nav.nav_x_packed = child_node->idx_nav.nav_x_packed | nav_col;
            child_node->u8_u.nav_y_packed = child_node->u8_u.nav_y_packed & MENU_NAV_X_CLEAR;
            child_node->u8_u.nav_y_packed = child_node->u8_u.nav_y_packed | nav_col;
            child_idx2 = (&loop_active->uA.s.child0)[child_slot];
            child_idx2 ^= child_slot;
            child_idx2 ^= child_slot;
            parent_packed = loop_active->idx_nav.nav_x_packed;
            parent_packed ^= child_slot;
            parent_packed ^= child_slot;
            parent_y_tmp = loop_active->u8_u.s.nav_y_hi;
            /* Propagate nav Y bit 0 (bit 15 of nav_x_packed) from parent to child. */
            ((MenuNode*)(((u32)child_idx2 << 4) + child_base_addr))->idx_nav.nav_x_packed =
                (((MenuNode*)(((u32)child_idx2 << 4) + child_base_addr))->idx_nav.nav_x_packed & ~MENU_NAV_Y0_BIT) | (parent_packed & MENU_NAV_Y0_BIT);
            /* Propagate nav_y_hi (nav Y bits 8:1) from parent to child's u8_u low byte. */
            ((MenuNode*)(((u32)child_idx2 << 4) + child_base_addr))->u8_u.nav_y_packed =
                (((MenuNode*)(((u32)child_idx2 << 4) + child_base_addr))->u8_u.nav_y_packed & 0xFF00) | parent_y_tmp;
        }
    }

    /* Update g_menu_char_slot from bits [5:4] of u2.unk2 (3 = preserve current value). */
    char_slot_bits = (((u16)g_menu_nodes[g_menu_active_node].u2.unk2) >> 4) & 3;
    if (char_slot_bits != 3)
    {
        g_menu_char_slot = char_slot_bits;
    }

    /* Re-run layout for all root nodes and adjust scroll if content overflows the viewport. */
    var_s3 = 0;
    layout_pos = 0;
    node = g_menu_nodes;
    for (node_i = 0; node_i < MENU_NODE_COUNT; node_i++, node++)
    {
        temp_v0 = MENU_VIEW_HEIGHT;
        if (g_menu_nodes[node_i].u2.s.parent_idx == MENU_NONE)
        {
            if (g_menu_nodes[node_i].u2.s.flags & 1)
            {
                prev_layout_y = layout_pos;
                layout_pos = menu_layout_node(node_i, layout_pos);
                /* If this node contributed more than one row, it has visible children. */
                if (prev_layout_y != (layout_pos - MENU_ROW_HEIGHT))
                {
                    var_s3 = 1;
                    if (layout_pos > MENU_VIEW_HEIGHT)
                    {
                        g_menu_scroll_pos = layout_pos - temp_v0;
                        g_menu_redraw_state = MENU_REDRAW_LAYOUT;
                    }
                }
            }
        }
    }

    g_menu_layout_end = layout_pos;
    if (var_s3 == 0)
    {
        g_menu_scroll_pos = 0;
        g_menu_redraw_state = MENU_REDRAW_LAYOUT;
    }
}

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

/* K&R declarations are intentional for callback-address use before the state types. */
s32 menu_spell_list_callback();
s32 menu_equipment_action_callback();
s32 menu_subtype_action_callback();

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

/** @brief Pad-context view exposing the party-sort order table. */
typedef struct
{
    u8 pad000[0x638]; /**< 0x000 - unmapped head of the pad context record. */
    u8 order[8];      /**< 0x638 - eight party slot indices, in display order. */
} MenuPartyOrder;

/** @brief Return the party-sort order record for the active reorder screen. */
static inline MenuPartyOrder* menu_sort_order_record(void)
{
    u8* base = (u8*)g_pad_ctx;
    s32 off = (g_menu_scene_type == 0x1F) ? 0 : 0x250;

    return (MenuPartyOrder*)(base + off);
}

#define MENU_SORT_SLOT(i) (menu_sort_order_record()->order[i])

/**
 * @brief Inline twin of @ref menu_focus_active_content_item.
 * @return 1 if an active content item was found; otherwise 0.
 */
static inline s32 menu_focus_active_item(void)
{
    MenuContentItem* content_items;
    MenuContentItem* active_item;
    s32 view_y;

    g_menu_hit_item_idx = menu_find_active_content_item();
    if (g_menu_hit_item_idx != (-1))
    {
        MenuNode* nodes = g_menu_nodes;
        u8 content_table_idx = (nodes + g_menu_scene_type)->idx_nav.s.self_idx;

        content_items = g_menu_content_table[content_table_idx];
        active_item = content_items - (-g_menu_hit_item_idx);
        g_content_view_x = active_item->packed_x & MENU_CONTENT_X_MASK;
        view_y = active_item->y - MENU_CONTENT_VIEW_Y_OFFSET;
        g_menu_suppress_cursor = MENU_CURSOR_REVEAL_DELAY;
        g_menu_cursor_enable = MENU_CURSOR_MODE_CONTENT;
        g_content_view_y = view_y;
        return 1;
    }
    return 0;
}

/**
 * @brief Per-frame menu navigation, confirm/cancel and cursor-move input handler.
 * @param process_actions Non-zero to also process confirm/cancel and the four cursor-move buttons; 0 runs only node switching.
 * @see decomp.me (99.02%) https://decomp.me/scratch/DRmEd
 */
void menu_handle_input(s32 process_actions)
{
    u8* var_s4 = (u8*)0x801ED600; /* Raw view preserves menu_handle_input codegen. */
    MenuContentItem* s5;
    u32 content_type;
    MenuSlot* slots;
    MenuSlot* var_a3;
    s32 self_idx;
    s32 idx_a, idx_b;
    u16 packed_x;
    u16 top_nibble;
    u8 table_idx;
    u8 val;
    u32 dir_mask;
    s32 dir_index;
    MenuNode* active_node;
    MenuNode* nodes;
    MenuNode* nav_nodes;
    s32 nav_y;
    s32 nav_hi;
    s32 new_type_left;
    s32 new_type_right;
    s32 new_type_up;
    s32 new_type_down;
    u8 flag;
    void* new_var7;
    u32 item14;
    s32 shift;
    u8* base;
    u8* base1;
    u8* base2;
    u8* base3;
    s32 var_s0;
    s8 companion;
    MenuSlotRect rect;
    s8 sp18[0x40];
    s8* buf;

    if ((u32)(g_menu_scene_type - 0x14) < 2 || g_menu_scene_type == 0x17 || g_menu_scene_type == 0x18 || g_menu_scene_type == 0x1A || g_menu_scene_type == 0x1B)
    {
        if (g_pad_input & 0xF)
        {
            menu_play_se(0x7D, 0x80);
            if (g_pad_input & 3)
            {
                if (g_menu_item_ptr != ((void*)0))
                {
                    if (g_pad_input & 1)
                    {
                        menu_step_item_selection(-1);
                        if (g_script_repeat_last == 0)
                        {
                            g_script_repeat_last = g_menu_page_count - 1;
                        }
                        else
                        {
                            g_script_repeat_last -= 1;
                        }
                    }
                    else if (g_pad_input & 2)
                    {
                        menu_step_item_selection(1);
                        if (g_script_repeat_last == (g_menu_page_count - 1))
                        {
                            g_script_repeat_last = 0;
                        }
                        else
                        {
                            g_script_repeat_last += 1;
                        }
                    }
                }
            }
            else
            {
                if (g_menu_scene_type == 0x14 || g_menu_scene_type == 0x17 || g_menu_scene_type == 0x1A)
                {
                    g_menu_scene_type += 1;
                    g_menu_active_node = g_menu_scene_type;
                }
                else if (g_menu_scene_type == 0x15 || g_menu_scene_type == 0x18 || g_menu_scene_type == 0x1B)
                {
                    g_menu_scene_type -= 1;
                    g_menu_active_node = g_menu_scene_type;
                }
                if (g_menu_cursor_enable == 0)
                {
                    menu_snap_view_to_cursor();
                }
                menu_set_active_node();
                menu_focus_active_item();
            }
        }
    }

    if (g_menu_scene_type < 0x11U && (g_pad_input & 0xF))
    {
        menu_play_se(0x7D, 0x80);
        switch (g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx - 0x14)
        {
        case 0:
        case 3:
        case 6:
            nodes = g_menu_nodes;
            nodes[g_menu_scene_type].idx_nav.s.self_idx += 1;
            menu_focus_active_item();
            goto after_do_while;

        case 1:
        case 4:
        case 7:
            nodes = g_menu_nodes;
            nodes[g_menu_scene_type].idx_nav.s.self_idx -= 1;
            menu_focus_active_item();
            goto after_do_while;

        case 2:
        case 5:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
            break;

        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
            goto after_do_while;
        }

        while (1)
        {
            {
                if (g_pad_input & 4)
                {
                    if (g_menu_scene_type == ((g_menu_scene_type / 3) * 3))
                    {
                        if (g_menu_scene_type >= 9)
                        {
                            new_type_left = g_menu_scene_type + 1;
                        }
                        else
                        {
                            new_type_left = g_menu_scene_type + 2;
                        }
                    }
                    else
                    {
                        new_type_left = g_menu_scene_type - 1;
                    }
                    g_menu_scene_type = new_type_left;
                    g_menu_active_node = new_type_left;
                }
                if (g_pad_input & 8)
                {
                    if (g_menu_scene_type < 9)
                    {
                        if ((g_menu_scene_type / 3) * 3 == g_menu_scene_type - 2)
                        {
                            new_type_right = g_menu_scene_type - 2;
                        }
                        else
                        {
                            new_type_right = g_menu_scene_type + 1;
                        }
                    }
                    else
                    {
                        if ((g_menu_scene_type / 3) * 3 == g_menu_scene_type - 1)
                        {
                            new_type_right = g_menu_scene_type - 1;
                        }
                        else
                        {
                            new_type_right = g_menu_scene_type + 1;
                        }
                    }
                    g_menu_scene_type = new_type_right;
                    g_menu_active_node = new_type_right;
                }
                if ((g_pad_input & 1) && (g_menu_scene_type < 9))
                {
                    if ((g_menu_scene_type / 3) == 0)
                    {
                        new_type_up = g_menu_scene_type + 6;
                    }
                    else
                    {
                        new_type_up = g_menu_scene_type - 3;
                    }
                    g_menu_scene_type = new_type_up;
                    g_menu_active_node = new_type_up;
                    menu_set_active_node();
                }
                if ((g_pad_input & 2) && (g_menu_scene_type < 9))
                {
                    if ((g_menu_scene_type / 3) == 2)
                    {
                        new_type_down = g_menu_scene_type - 6;
                    }
                    else
                    {
                        new_type_down = g_menu_scene_type + 3;
                    }
                    g_menu_scene_type = new_type_down;
                    g_menu_active_node = new_type_down;
                    menu_set_active_node();
                }

            }

            if (!(g_menu_nodes[(g_menu_scene_type / 3) * 3].u2.s.flags & 1))
            {
                continue;
            }
            if (g_menu_cursor_enable == 0)
            {
                menu_snap_view_to_cursor();
            }
            if (menu_focus_active_item())
            {
                break;
            }
        }

        for (var_s0 = 3; var_s0 >= 0; var_s0--)
        {
            g_menu_slots[var_s0].active = 0;
        }
    }

after_do_while:

    if (process_actions != 0)
    {
        s5 = g_menu_content_table[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];

        if ((g_menu_scene_type == 0x1F || g_menu_scene_type == 0x2B) && g_menu_hit_item_idx >= 0x11 && g_menu_hit_item_idx < 0x19)
        {
            if (g_pad_input & 0x40)
            {
                if (g_party_sort_marker.selected_idx != 0xFF)
                {
                    g_pad_input &= ~0x40;
                    g_party_sort_marker.selected_idx = 0xFF;
                }
                else
                {
                    g_pad_input &= ~0x40;
                    menu_play_se(0x7D, 0x80);
                    menu_reset_content_view();
                }
            }
            if (g_pad_input & 0x220)
            {
                if (g_party_sort_marker.selected_idx == 0xFF)
                {
                    menu_play_se(0x7D, 0x80);
                    g_party_sort_marker.x = (s8)(((MenuContentItemBits*)s5)[g_menu_hit_item_idx].x - 2);
                    g_party_sort_marker.y = (s8)(s5[g_menu_hit_item_idx].y - 8);
                    g_party_sort_marker.selected_idx = (u8)g_menu_hit_item_idx;
                }
                else if (g_party_sort_marker.selected_idx != g_menu_hit_item_idx)
                {
                    menu_play_se(0x7E, 0x80);
                    for (idx_a = 0; idx_a < 8; idx_a++)
                    {
                        if (MENU_SORT_SLOT(idx_a) == (g_menu_hit_item_idx - 0x11))
                        {
                            break;
                        }
                    }
                    for (idx_b = 0; idx_b < 8; idx_b++)
                    {
                        if (MENU_SORT_SLOT(idx_b) == (g_party_sort_marker.selected_idx - 0x11))
                        {
                            break;
                        }
                    }
                    dir_index = MENU_SORT_SLOT(idx_a);
                    {
                        u8* sort_base = (u8*)menu_sort_order_record();
                        u8* sort_dst = sort_base + idx_a + 0x638;
                        *sort_dst = MENU_SORT_SLOT(idx_b);
                    }
                    MENU_SORT_SLOT(idx_b) = dir_index;
                    g_party_sort_marker.selected_idx = 0xFF;
                }
                else
                {
                    menu_play_se(0x7F, 0x80);
                    g_party_sort_marker.selected_idx = 0xFF;
                }
            }
            else if (g_pad_input & 0x10)
            {
                menu_play_se(0x7E, 0x80);
                MENU_SORT_SLOT(0) = 0;
                MENU_SORT_SLOT(1) = 1;
                MENU_SORT_SLOT(2) = 2;
                MENU_SORT_SLOT(3) = 3;
                MENU_SORT_SLOT(4) = 4;
                MENU_SORT_SLOT(5) = 5;
                MENU_SORT_SLOT(6) = 6;
                MENU_SORT_SLOT(7) = 7;
            }
        }
        else if (g_pad_input & 0x220)
        {
            packed_x = s5[g_menu_hit_item_idx].packed_x;
            top_nibble = packed_x & 0xF000;
            if (top_nibble == 0x5000)
            {
                g_menu_active_subtype = s5[g_menu_hit_item_idx].pad[0];
                content_type = s5[g_menu_hit_item_idx].pad[0];
                switch (content_type)
                {
                case 1:
                case 2:
                    if (g_menu_char_slot == 0)
                    {
                        rect.x = 0x40;
                        rect.y = 0x60;
                        rect.w = 0xF0;
                        rect.h = 0x60;
                        var_a3 = menu_slot_alloc(3, &rect);
                        var_a3->content_cb = (s32 * (*)()) & menu_spell_list_callback;
                        var_a3->flags =
                            (var_a3->flags & 0xFE00FFFF) | ((menu_build_spell_nav_entries() & 0x1FF) << 16);
                        var_a3->has_title = 1;
                        menu_play_se(0x7D, 0x80);
                    }
                    break;

                case 3:
                case 4:
                case 5:
                case 6:
                    if (g_menu_char_slot == 0)
                    {
                        { u8* pad_base0 = (u8*)g_pad_ctx; flag = *(pad_base0 + content_type + 0x609); }
                        if ((flag != 0xFF) && (flag & 0x80))
                        {
                            rect.x = 0xB0;
                            rect.y = 0x60;
                            rect.w = 0x70;
                            rect.h = 0x50;
                        }
                        else
                        {
                            rect.x = 0xB0;
                            rect.y = 0x60;
                            rect.w = 0x70;
                            rect.h = 0x40;
                        }
                        var_a3 = menu_slot_alloc(3, &rect);
                        var_a3->content_cb = (s32 * (*)()) & menu_subtype_action_callback;
                        { u8* pad_base = (u8*)g_pad_ctx + (g_menu_char_slot * 0x250); flag = *(pad_base + content_type + 0x609); }
                        if ((flag != 0xFF) && (flag & 0x80))
                        {
                            var_a3->flags = (var_a3->flags & 0xFE00FFFF) | 0x40000;
                            menu_init_item_nav_entries(4);
                        }
                        else
                        {
                            var_a3->flags = (var_a3->flags & 0xFE00FFFF) | 0x30000;
                            menu_init_item_nav_entries(3);
                        }
                        scroll_list_update_target(var_a3, g_menu_scroll_nav_entries);
                        new_var7 = (void*)((u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0) + ((content_type << 6) + 0x90));
                        g_menu_item_ptr = new_var7;
                        g_menu_saved_category0_item = g_menu_item_ptr;
                        g_menu_category0_item = g_menu_item_ptr;
                        g_menu_saved_category1_item = g_menu_item_ptr;
                        g_menu_category1_item = g_menu_item_ptr;
                        g_menu_nav_prev[0] = g_menu_item_ptr;
                        g_menu_active_equipped_item = g_menu_item_ptr;
                        g_menu_category2_item = g_menu_item_ptr;
                        menu_play_se(0x7D, 0x80);
                    }
                    break;

                case 7:
                case 8:
                case 9:
                case 10:
                    if (g_menu_char_slot == 0)
                    {
                        rect.x = 0xB0;
                        rect.y = 0x30;
                        rect.w = 0x70;
                        rect.h = 0x60;
                        var_a3 = menu_slot_alloc(3, &rect);
                        var_a3->content_cb = (s32 * (*)()) & menu_equipment_action_callback;
                        var_a3->flags = (var_a3->flags & 0xFE00FFFF) | 0x50000;
                        menu_init_item_nav_entries(5);
                        {
                            void* ptr = (void*)((u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0) + ((content_type << 6) - 0x170));
                            g_menu_saved_category0_item = ptr;
                            g_menu_saved_category1_item = ptr;
                            g_menu_active_equipped_item = ptr;
                            g_menu_nav_prev[0] = ptr;
                            menu_play_se(0x7D, 0x80);
                        }
                    }
                    break;

                case 15:
                    if ((g_menu_scene_type >= 0x12) || (g_menu_scene_type == 1) || (g_menu_scene_type == 2))
                    {
                        if ((g_menu_item_ptr != ((void*)0)) && (*((u8*)g_menu_item_ptr) != 0))
                        {
                            D_8011F424 = (((u32)(*((u32*)((char*)g_menu_item_ptr + 0x14)))) >> 8) & 3;
                            func_800A8E28(&D_801226F0, g_menu_item_ptr);
                            sp18[0] = 0;
                            D_801226B8 = 0;
                            D_801227D4 = g_menu_item_ptr;
                            if (menu_item_is_nondefault(g_menu_item_ptr) != 0)
                                {
                                buf = sp18;
                                base1 = (u8*)g_menu_state_ptr + (*((s32*)((char*)g_menu_state_ptr + 0x88)));
                                func_800A8E28(buf, (s8*)(base1 + (*((u16*)(base1 + (((*((u16*)((char*)g_menu_item_ptr + 0x16))) & 0x3F) * 2) + 0x48)))));
                                item14 = *((u32*)((char*)g_menu_item_ptr + 0x14));
                                shift = (item14 >> 8) & 3;
                                switch (shift)
                                {
                                case 0:
                                    base = (u8*)g_menu_state_ptr + (*((s32*)((char*)g_menu_state_ptr + 0x88)));
                                    menu_concat_encoded_text(&D_801226B8, sp18, (s8*)(base + (*((u16*)(((item14 >> 9) & 0x7E) + (s32)base)))));
                                    break;

                                case 1:
                                    base2 = (u8*)g_menu_state_ptr + (*((s32*)((char*)g_menu_state_ptr + 0x88)));
                                    menu_concat_encoded_text(&D_801226B8, sp18, (s8*)(base2 + (*((u16*)(((item14 >> 9) & 0x7E) + (s32)base2 + 0x20)))));
                                    break;

                                default:
                                    base3 = (u8*)g_menu_state_ptr + (*((s32*)((char*)g_menu_state_ptr + 0x88)));
                                    menu_concat_encoded_text(
                                        &D_801226B8, sp18,
                                        (s8*)(base3 + (*((u16*)((((((u32)(*((u32*)((char*)g_menu_item_ptr + 0x14)))) >> 9) & 0x7E)) + (s32)base3 + 0x40)))));
                                    break;
                                }
                                g_menu_load_request = 1;
                                D_801229F4 = g_script_repeat_last;
                                if (g_menu_scene_type == 1)
                                {
                                    g_menu_transition_code = 0xB;
                                }
                                else if (g_menu_scene_type == 2)
                                {
                                    g_menu_transition_code = 0xC;
                                }
                                else
                                {
                                    g_menu_transition_code = 1;
                                }
                            }
                        }
                    }
                    break;
                }
            }
            else if (top_nibble == 0xF000)
            {
                table_idx = g_menu_content_group_ids[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];
                val = g_menu_content_action_codes[table_idx][(packed_x >> 9) & 7];
                if (val != 0)
                {
                    menu_play_se(0x7E, 0x80);
                    switch (val)
                    {
                    case 1:
                        {
                            u8* rec;
                            s32 idx;
                            s32 count;
                            u8* slot_base;

                            slot_base = (u8*)g_pad_ctx + (g_menu_char_slot * 0x250);
                            if (slot_base[0x5F0] != 0)
                            {
                                if ((slot_base[0x608] & 0x7F) == 4)
                                {
                                    D_801229F4 = slot_base[0x609] + 0x80;
                                }
                                else
                                {
                                    D_801229F4 = slot_base[0x609];
                                }
                                func_800A8E28(&D_801226F0, (u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0));
                                count = 0;
                                idx = 0;
                                rec = (u8*)&D_800FD818;
                                D_801227D4 = (void*)((u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0));
                                for (; idx < 3; idx++)
                                {
                                    if (idx == g_menu_char_slot)
                                    {
                                        break;
                                    }
                                    if (*rec & 1)
                                    {
                                        count += 1;
                                    }
                                    rec += 0x268;
                                }
                                D_8011F424 = count + 3;
                                g_menu_load_request = 1;
                                g_menu_transition_code = 3;
                            }
                        }
                        break;

                    case 6:
                        ((u32*)((u8*)g_pad_ctx + 0x28))[0] |= 2;
                        akao_set_paused(0);
                        break;

                    case 7:
                        ((u32*)((u8*)g_pad_ctx + 0x28))[0] &= ~2;
                        akao_set_paused(1);
                        break;

                    case 8:
                        ((u32*)((u8*)g_pad_ctx + 0x28))[0] |= 1;
                        var_s4[0x90] = 1;
                        var_s4[0x92] = 0x80;
                        if (g_pad_ctx->inject_flags & 0x80)
                        {
                            var_s4[0x13E] = 1;
                            var_s4[0x140] = 0x80;
                        }
                        break;

                    case 9:
                        ((u32*)((u8*)g_pad_ctx + 0x28))[0] &= ~1;
                        var_s4[0x90] = 0;
                        var_s4[0x13E] = 0;
                        break;

                    case 10:
                        if (var_s4[0xAE] == 0xFF)
                        {
                            s32 clear_i;
                            s32 state_off;
                            MenuSlot* clear_slot;

                            clear_i = 3;
                            clear_slot = (MenuSlot*)((u8*)g_menu_slots + 0x6C);
                            state_off = *((volatile s32*)((u8*)g_menu_state_ptr + 8));
                            g_menu_message_line1 = (u8*)g_menu_state_ptr + (*((s32*)((char*)g_menu_state_ptr + 8))) +
                                                   (*((u16*)((u8*)g_menu_state_ptr + (*((s32*)((char*)g_menu_state_ptr + 8))) + 0xCA)));
                            g_menu_message_line2 = (u8*)g_menu_state_ptr + state_off +
                                                   (*((u16*)((u8*)g_menu_state_ptr + state_off + 0xCC)));
                            do
                            {
                                clear_slot->active = 0;
                                clear_i--;
                                clear_slot--;
                            } while (clear_i >= 0);
                            menu_open_content_page(7);
                        }
                        else
                        {
                            g_pad_ctx->inject_flags |= 0x80;
                            companion = 0x2B;
                            goto set_companion;
                        }
                        break;

                    case 11:
                        g_pad_ctx->inject_flags &= ~0x80;
                        companion = 0xFF;
                    set_companion:
                        g_menu_companion_node = companion;
                        menu_set_active_node();
                        break;
                    }
                }
            }
        }
        else if (g_pad_input & 0x40)
        {
            menu_play_se(0x7F, 0x80);
            if ((u32)g_menu_scene_type >= 0x11U || (u8)(self_idx = g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx) < 0x14 || (u8)self_idx >= 0x1C)
            {
                menu_reset_content_view();
            }
            else
            {
                if ((u32)(self_idx - 0x14) < 5)
                {
                    g_menu_nodes[(g_menu_char_slot * 3) + 1].idx_nav.s.self_idx = 1;
                    g_menu_nodes[(g_menu_char_slot * 3) + 1].label_id = (g_menu_char_slot * 3) + 3;
                    menu_focus_active_item();
                }
                else if ((u32)(self_idx - 0x1A) < 2)
                {
                    g_menu_nodes[(g_menu_char_slot * 3) + 2].idx_nav.s.self_idx = 2;
                    g_menu_nodes[(g_menu_char_slot * 3) + 2].label_id = (g_menu_char_slot * 3) + 3;
                    menu_focus_active_item();
                }
            }
        }

        dir_index = 0;
        content_type = 0x1000;
        for (; dir_index < 4; dir_index++)
        {
            if (g_pad_input & content_type)
            {
                break;
            }
            content_type <<= 1;
        }

        if (dir_index != 4)
        {
            menu_play_se(0x7D, 0x80);
            if (s5[g_menu_hit_item_idx].pad[1 + dir_index] == 0xFF)
            {
                if ((u32)g_menu_scene_type >= 0x11U || g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx < 0x14 ||
                    g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx >= 0x1C)
                {
                    s32* view_y_ptr = &g_content_view_y;
                    g_menu_cursor_enable = 2;
                    nav_nodes = g_menu_nodes;
                    active_node = &nav_nodes[g_menu_active_node];
                    { s32 np = active_node->idx_nav.nav_x_packed; s32 ny = active_node->u8_u.s.nav_y_hi; nav_y = (np >> 15) | (ny << 1); }
                    nav_hi = g_menu_content_height - 12;
                    *view_y_ptr = nav_y - nav_hi;
                    if (*view_y_ptr < 12)
                    {
                        *view_y_ptr = 12;
                    }
                    if (*view_y_ptr >= 0xA3)
                    {
                        *view_y_ptr = 0xA3;
                    }
                    g_menu_suppress_cursor = 5;
                    g_content_view_x = ((active_node->idx_nav.nav_x_packed >> 8) & 0x7F) + 8;
                }
            }
            else if (s5[g_menu_hit_item_idx].pad[1 + dir_index] != 0)
            {
                if (g_menu_scene_type < 0x11)
                {
                    if ((s5[g_menu_hit_item_idx].packed_x & 0xF000) == 0x5000)
                    {
                        if (g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx < 0x11)
                        {
                            g_menu_item_ptr = (void*)0;
                            g_menu_category0_item = (void*)0;
                            g_menu_category2_item = (void*)0;
                            g_menu_category1_item = (void*)0;
                        }
                    }
                }
                g_menu_hit_item_idx = s5[g_menu_hit_item_idx].pad[1 + dir_index];
                g_content_view_x = s5[g_menu_hit_item_idx].packed_x & 0x1FF;
                g_content_view_y = s5[g_menu_hit_item_idx].y - 8;
                g_menu_suppress_cursor = 3;
            }
        }
    }
}

/**
 * @brief Clamp the content cursor and snap the viewport to its position.
 * @see decomp.me (100%) https://decomp.me/scratch/wBlQo
 */
void menu_snap_view_to_cursor(void)
{
    if (g_content_cursor_y < MENU_CURSOR_Y_MIN)
    {
        g_content_cursor_y = MENU_CURSOR_Y_MIN;
    }
    if (g_content_cursor_y >= MENU_CURSOR_Y_MAX)
    {
        g_content_cursor_y = MENU_CURSOR_Y_MAX;
    }
    g_content_view_y = g_content_cursor_y;

    /* Restore X from the active node's packed navigation column. */
    g_content_cursor_x =
        MENU_NAV_X(g_menu_nodes[g_menu_active_node].idx_nav.nav_x_packed) + MENU_CONTENT_CURSOR_X_OFFSET;
    g_content_view_x = g_content_cursor_x;
}

/** @brief Content item layout -- mirrors MenuContentItem; action_type is pad[0]. */
typedef struct
{
    u16 packed_x; /**< Bottom 9 bits = X screen pos; bits 15:12 = item type nibble. */
    u8 y;
    u8 action_type; /**< Sub-menu routing byte (types 1-15 for 0x5000 items). */
    u8 pad_4[4];
} MenuItem;

/**
 * @brief Return non-zero if the item currently under the cursor has a confirm action.
 * @return 1 if the item will trigger a sub-menu or named action on confirm, 0 otherwise.
 * @see decomp.me (100%) https://decomp.me/scratch/cV0x9
 */
int menu_item_has_action(void)
{
    int new_var3;
    int new_var;
    MenuItem* items;
    MenuItem* item;
    u16 type_nibble;
    short item_subtype;
    int action_code;

    items = g_menu_content_table[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];
    if ((g_menu_scene_type == 0x1F) || (g_menu_scene_type == 0x2B))
    {
        if (((int)g_menu_hit_item_idx) >= 0x11)
        {
            if (((int)g_menu_hit_item_idx) < 0x19)
            {
                return 1;
            }
        }
    }

    item = (MenuItem*)((g_menu_hit_item_idx * 8u) + ((u32)items));
    type_nibble = item->packed_x & 0xF000;

    if (type_nibble == 0x5000)
    {
        g_menu_active_subtype = item->action_type;
        item_subtype = item->action_type;
        if (item_subtype != 0)
        {
            if (item_subtype >= 11)
            {
                if (item_subtype == 15)
                {
                    goto success;
                }
                return 0;
            }
            else
            {
                return g_menu_char_slot == 0;
            }
        }
    }
    else if (type_nibble == 0xF000)
    {
        action_code = g_menu_content_action_codes[g_menu_content_group_ids[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx]][(item->packed_x >> 9) & 7];
        if (action_code == 0)
        {
        }
        else if (action_code == 1)
        {
            return 1;
        }
        else if ((action_code != 0) && (action_code < 12) && (action_code >= 6))
        {
success:
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Exit content focus and restore the node-tree viewport to the active node.
 * @see decomp.me (100%)
 */
void menu_reset_content_view(void)
{
    MenuNode* active_node;
    s32 node_y;
    s32 node_y_low_bit;
    s32 scroll_offset;
    s32* view_y_ptr;

    if (g_menu_nodes[g_menu_scene_type].content_id != MENU_NONE)
    {
        g_content_cursor_x = g_menu_default_view_pos.x;
        g_content_cursor_y = g_menu_default_view_pos.y;
    }

    g_menu_cursor_enable = MENU_CURSOR_MODE_CONTENT_EXIT;
    view_y_ptr = &g_content_view_y;
    active_node = &g_menu_nodes[g_menu_active_node];
    node_y_low_bit = active_node->idx_nav.nav_x_packed >> 15;
    node_y = (active_node->u8_u.s.nav_y_hi << 1) | node_y_low_bit;

    /* Convert the node's layout position into the scrolled viewport. */
    scroll_offset = g_menu_content_height - MENU_CURSOR_Y_MIN;
    *view_y_ptr = node_y - scroll_offset;
    if (*view_y_ptr < MENU_CURSOR_Y_MIN)
    {
        *view_y_ptr = MENU_CURSOR_Y_MIN;
    }
    if (*view_y_ptr >= MENU_CURSOR_Y_MAX)
    {
        *view_y_ptr = MENU_CURSOR_Y_MAX;
    }

    g_menu_suppress_cursor = MENU_CURSOR_REVEAL_DELAY;
    g_content_view_x = MENU_NAV_X(active_node->idx_nav.nav_x_packed) + MENU_CONTENT_CURSOR_X_OFFSET;
}

/**
 * @brief Initialize packed positions and circular links for item navigation.
 * @param count Number of entries to initialize (no-op if <= 0).
 * @see decomp.me (100%) https://decomp.me/scratch/x87Jm
 */
void menu_init_item_nav_entries(s32 count)
{
    s32 next_index;
    s32 has_next;
    s32 entry_with_position;
    s32 previous_index;
    s32 entry_index;
    s32 wrapped_next_index;
    s32* entry;
    s32 packed_entry;
    s32 position;
    s32 entry_with_previous;

    entry_index = 0;
    if (count > 0)
    {
        do
        {
            entry = entry_index + g_menu_item_nav_entries;
            packed_entry = *entry;
            previous_index = entry_index - 1;

            entry_with_position = packed_entry & ~MENU_ITEM_NAV_POSITION_MASK;
            position = entry_index * MENU_ITEM_NAV_POSITION_STRIDE;
            position = position & MENU_ITEM_NAV_POSITION_MASK;
            entry_with_position = entry_with_position | position;
            *entry = entry_with_position;

            if (previous_index < 0)
            {
                previous_index = count - 1;
            }

            entry_with_previous = entry_with_position & MENU_ITEM_NAV_PREVIOUS_CLEAR_MASK;
            entry_with_previous =
                entry_with_previous | ((previous_index & MENU_ITEM_NAV_INDEX_MASK) << MENU_ITEM_NAV_PREVIOUS_SHIFT);
            *entry = entry_with_previous;

            next_index = entry_index + 1;
            has_next = next_index < count;
            wrapped_next_index = 0;
            if (has_next != 0)
            {
                wrapped_next_index = next_index;
            }
            *entry = (entry_with_previous & MENU_ITEM_NAV_NEXT_CLEAR_MASK) |
                     (wrapped_next_index << MENU_ITEM_NAV_NEXT_SHIFT);
            entry_index = next_index;
        } while (has_next != 0);
    }
}

/**
 * @brief Build circular navigation entries for the spell grid.
 * @return Number of present grid cells.
 * @see decomp.me (100%) https://decomp.me/scratch/VjQt5
 */
s32 menu_build_spell_nav_entries(void)
{
    s32 row_bits;
    s32 has_next;
    u32 entry_with_position;
    s32 index;
    s32 previous_index;
    s32 row;
    s32 item_count;
    s32 working_value;
    s32 wrapped_next_index;
    u32* entry;
    u8* presence_rows;
    u32 packed_entry;
    s32 position;
    u32 entry_with_previous;

    item_count = 0;
    presence_rows = (u8*)g_pad_ctx + MENU_SPELL_GRID_OFFSET;

    for (row = MENU_SPELL_GRID_ROW_COUNT - 1; row >= 0; row--)
    {
        working_value = 1;
        row_bits = *presence_rows;
        for (index = MENU_SPELL_GRID_COLUMN_COUNT - 1; index >= 0; index--)
        {
            if (row_bits & working_value)
            {
                item_count++;
            }
            working_value *= 2;
        }

        presence_rows += 1;
    }

    g_menu_scroll_nav_entries[0] = 0;
    index = 0;
    if (item_count <= 0)
    {
        return item_count;
    }

    do
    {
        entry = g_menu_scroll_nav_entries + index;
        packed_entry = *entry;
        previous_index = index - 1;
        entry_with_position = packed_entry & ~MENU_ITEM_NAV_POSITION_MASK;
        position = index * MENU_ITEM_NAV_POSITION_STRIDE;
        position = position & MENU_ITEM_NAV_POSITION_MASK;
        entry_with_position = entry_with_position | position;
        *entry = entry_with_position;

        if (previous_index < 0)
        {
            previous_index = item_count - 1;
        }

        working_value = previous_index;
        entry_with_previous = entry_with_position & MENU_ITEM_NAV_PREVIOUS_CLEAR_MASK;
        entry_with_previous =
            entry_with_previous | ((working_value & MENU_ITEM_NAV_INDEX_MASK) << MENU_ITEM_NAV_PREVIOUS_SHIFT);
        *entry = entry_with_previous;

        index += 1;
        has_next = index < item_count;
        wrapped_next_index = 0;
        if (has_next != 0)
        {
            wrapped_next_index = index;
        }

        *entry = (entry_with_previous & MENU_ITEM_NAV_NEXT_CLEAR_MASK) |
                 (wrapped_next_index << MENU_ITEM_NAV_NEXT_SHIFT);
    } while (has_next != 0);

    return item_count;
}

/**
 * @brief Build navigation entries for the learned Special Technique list.
 * @return Low 16 bits: total set-bit count.
 * @see decomp.me (100%)
 */
s32 menu_build_special_technique_nav_entries(void)
{
    s32 i;
    s32 j;
    s32 prev;
    s32 next;
    s32 count;
    s32 more;
    s32 link;
    s32 word_prev;
    s32 found;
    s32 sentinel;
    s32 mask;
    s32 word;
    s32* p;

    count = 0;
    found = 0xFF;
    i = 0;
    sentinel = found;
    p = (s32*)((u8*)g_pad_ctx + 0x34);
    do
    {
        mask = 1;
        word = *p;
        j = 0x17;

        do
        {
            if ((word & mask) != 0)
            {
                if ((i == (s32)(((u32)(*(s32*)((u8*)g_menu_equipment_base + 0x14)) >> 0xA) & 0x3F)) && (found == sentinel))
                {
                    found = count;
                }
                count += 1;
            }
            j -= 1;
            mask = mask * 2;
        } while (j >= 0);
        i += 1;
        p += 1;
    } while (i < 0xB);

    if (found == 0xFF)
    {
        found = 0;
    }

    g_menu_scroll_nav_entries[0] = 0;

    j = 0;
    if (count > 0)
    {
        do
        {
            s32* slot = (s32*)g_menu_scroll_nav_entries + j;
            s32 cur = *slot;
            s32 word_self;

            prev = j - 1;
            link = cur & ~0x3FFF;
            link = link | ((j * 0x10) & 0x3FFF);
            word_self = link;
            *slot = word_self;
            if (prev < 0)
            {
                prev = count - 1;
            }
            word_prev = word_self & 0xFF803FFF;
            word_prev = word_prev | ((prev & 0x1FF) << 14);
            *slot = word_prev;
            next = j + 1;
            more = next < count;
            link = 0;
            if (more != 0)
            {
                link = next;
            }
            *slot = (word_prev & 0x7FFFFF) | (link << 23);
            j = next;
        } while (more != 0);
    }
    return count | (found << 16);
}

/**
 * @brief Count entries at g_pad_ctx + 0xCE0 (stride 0x40) whose 2-bit type field matches arg0, then initialize g_menu_scroll_nav_entries as a circular packed linked list of those entries.
 * @param arg0 2-bit type value to match against bits 9:8 of each entry's s32 field at offset 0x14.
 * @return Number of matching entries (and entries initialized in g_menu_scroll_nav_entries).
 * @see decomp.me (100%)
 */
s32 menu_build_inventory_nav_entries(s32 arg0)
{
    s32 i;
    s32 prev;
    s32 next;
    s32 count;
    s32 more;
    s32 link;
    s32 word_prev;
    u8* entry;

    i = 0;
    count = 0;
    entry = (u8*)g_pad_ctx + 0xCE0;
    do
    {
        u8 active = entry[0];

        if (active == 0)
        {
            break;
        }
        if ((((u32) * (s32*)(entry + 0x14) >> 8) & 3) == (u32)arg0)
        {
            count += 1;
        }
        i += 1;
        entry += 0x40;
    } while (i < 0x64);

    g_menu_scroll_nav_entries[0] = 0;

    i = 0;
    if (count > 0)
    {
        do
        {
            s32* slot = (s32*)g_menu_scroll_nav_entries + i;
            s32 cur = *slot;
            s32 word_self;

            prev = i - 1;
            link = cur & ~0x3FFF;
            link = link | ((i * 0x10) & 0x3FFF);
            word_self = link;
            *slot = word_self;
            if (prev < 0)
            {
                prev = count - 1;
            }
            word_prev = word_self & 0xFF803FFF;
            word_prev = word_prev | ((prev & 0x1FF) << 14);
            *slot = word_prev;
            next = i + 1;
            more = next < count;
            link = 0;
            if (more != 0)
            {
                link = next;
            }
            *slot = (word_prev & 0x7FFFFF) | (link << 23);
            i = next;
        } while (more != 0);
    }
    return count;
}
/* ----- M2C macros required by menu_draw_scene_content ----- */
typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8*)(expr) + (offset)))
#define M2C_BITWISE(type, expr) ((type)(expr))

typedef struct
{
    s16 x;
    s16 y;
} Vec2s;

extern s32 D_80042FB4;
extern u16 D_800F0C1C;
extern M2C_UNK D_80105AE0;
/** @brief Default scene content count encoded as count - 1 (value 3). */
extern u8 g_menu_default_content_count_minus_one;
extern u8 g_menu_content_item_counts[];
/** @brief Base of the four default MenuContentItem descriptors used by scene -1. */
extern u16 g_menu_default_content_items;
extern u8 D_80168659[];
extern u8 D_80168696[];
extern u8 D_801686B8[];
extern u8 D_80168C01[];
extern u8 D_80168C05[];
extern u8 D_80168C09[];
extern u8* D_80168C20;
extern u8* D_80168C24;
extern u8* D_80168C30;
extern u32 D_801694CC[];
extern u32 D_801694DC[];

/* ----- Access helpers used by menu_draw_scene_content ----- */
static inline s32 menu_probe_slot_off(s32 slot)
{
    return slot * 0x250;
}

static inline s32 menu_probe_node_off(s32 idx)
{
    return idx * 0x14C;
}

static inline u8 menu_reload_plain_u8(const u8* p)
{
    return *p;
}

static inline s32 menu_mask_test_pref(s32 value, s32 shift, u32 mask)
{
    (void)value;
    return (mask >> shift) & 1;
}

/**
 * @brief Draw the active scene's content entries.
 * @param var_s1 GPU packet cursor advanced as primitives are emitted.
 * @param arg1 Ordering-table entry passed to render helpers.
 * @return Updated GPU packet cursor after drawing.
 * @note WIP - not yet byte-matching. Currently 95.06%.
 * @see decomp.me (89.27%) https://decomp.me/scratch/D6Nba
 */
void* menu_draw_scene_content(void* var_s1, s32* arg1)
{
    u8 sp28[0x40];
    u8 sp68[0x40];
    Vec2s pos;
    u8 spB0[16];
    s32 spC0;
    Vec2s* pos_p;
    s32 var_a0_3, var_a0_4, var_a0_7, var_a0_8;
    s32 var_a1, var_a2_3, var_a2_4, var_a2_5, var_a2_7, var_a2_8, var_a2_10;
    s32 var_a2_11, var_a2_12, var_a2_13, var_a2_14, var_a2_15;
    s32 var_a2_16, var_a2_17, var_a2_18, var_a2_19;
    s32 var_a2_21, var_a2_22, var_a2_24, var_a2_25, var_a2_29;
    s32 var_a2_20, var_a2_23;
    u16 var_a2_9, var_a2_7u, var_a2_15u;
    u16 var_a2_20u, var_a2_23u;
    u32 var_a2_26;
    s32 temp_v0_5, temp_v0_6;
    u16* var_s3;
    u8* var_s4;
    u8 **var_t0_2, **var_t0_3, **var_t0_4, **var_t0_5;
    u8 *var_a2_2, *var_a2_6, *var_a2_27, *var_a2_28;
    u8 *var_v1_2, *var_v1_7, *var_v1_9, *var_v1_12, *var_v1_13, *var_v1_14, *var_v1_15, *var_v1_16, *var_v1_18, *var_v1_20;
    void* var_a0;
    void *base_a2_0, *base_a2_1, *base_a2_2, *base_a2_3, *base_a2_4, *base_a2_5, *base_a2_6, *base_a2_7, *base_a2_8, *base_a2_9, *base_a2_10, *base_a2_11, *base_a2_12;
    s32 temp_a1_2, temp_a1_3, temp_a1_4, temp_a1_5, temp_a1_6;
    u32 temp_a1_2u, temp_hi;
    s8 temp_v1_13, temp_v1_14, temp_v1_15, temp_v1_16;
    u8 temp_v1_19, temp_v1_21;
    u16 temp_s0_17;
    u32 temp_s0_16, temp_s2;
    s32 temp_t3;
    s32 i, k;
    u32 tmpa0;
    u32 shared_s0;
    s32 two_outer = 2;

    if (g_menu_scene_type == -1)
    {
        var_s3 = &g_menu_default_content_items;
        spC0 = g_menu_default_content_count_minus_one + 1;
    }
    else
    {
        u8 idx = g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx;
        var_s3 = (u16*)g_menu_content_table[idx];
        spC0 = g_menu_content_item_counts[g_menu_content_group_ids[idx]];
    }

    if (var_s3 != NULL)
    {
        if (var_s3 == (u16*)1)
        {
            g_menu_load_request = (s32)var_s3;
        }
        else
        {
            u8* temp_a2;
            u8* temp_a1;
            g_menu_equipment_base = (u32)(temp_a1 = (temp_a2 = (u8*)g_pad_ctx + (g_menu_char_slot * 0x250 + 0x5F0)) + 0x50);
            D_80168C30 = temp_a1;
            D_80168C20 = temp_a1;
            D_80168C24 = temp_a1;
            if (g_menu_scene_type == 0x10)
            {
                u8* temp_v1_2;
                g_menu_category0_item = (u32)temp_a1;
                g_menu_category1_item = (u32)(temp_v1_2 = temp_a2 + 0x90);
                g_menu_category2_item = (u32)temp_v1_2;
            }

            if (spC0 != 0)
            {
                var_s4 = (u8*)var_s3 + 2;
            do
            {
                pos_p = &pos;
                pos.x = (s16)(*var_s3 & 0x1FF);
                do { do { do { do { do { do { do { do { do { pos.y = (s16)(*var_s4 - 8); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0);
                {
                    s32 t0 = 0xFF;
                    u16 item_word = *var_s3;
                    s32 item_type = item_word >> 12;
                switch (item_type)
                {
                case 1:
                    var_a0 = var_s1;
                    base_a2_0 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 8));
                    {
                        s32 a3 = 1;
                        void* a2_2 = (void*)((u8*)base_a2_0 + *(u16*)((u8*)base_a2_0 + (*(volatile u8*)(var_s4 + 1) * 2)));
                        var_s1 = func_800A88A0(var_a0, arg1, a2_2, a3, *var_s3 & 0x1FF, *var_s4 - 8, (*var_s3 >> 9) & 7);
                    }
                    break;

                case 2:
                {
                    s32 subtype;
                    if (*(volatile u8*)(var_s4 + 1) < 2)
                    {
                        t0 = 0xFF;
                        { s32 y = two_outer; if (y == 2)
                            t0 = 0x1E; }
                    }
                    subtype = *(volatile u8*)(var_s4 + 1);
                    if ((u32)(subtype - 2) < 8)
                    {
                        s32 a2_3 = 0;
                        if (*(u8*)g_menu_category1_item != 0 && g_menu_category1_item != 0)
                        {
                            a2_3 = *(u8*)(g_menu_category1_item + 0x2C);
                        }
                        if (((a2_3 >> (*(volatile u8*)(var_s4 + 1) - 2)) & 1) != 0)
                        {
                            t0 = *(volatile u8*)(var_s4 + 1) + 0x3B;
                        }
                        else
                        {
                            t0 = 0x21;
                        }
                    }
                    else
                    {
                        s32 a2_4;
                        s32 a2_4_shift;
                        if ((u32)(subtype - 0xA) < 8)
                        {
                            a2_4 = 0;
                            if (*(u8*)g_menu_category1_item != 0 && g_menu_category1_item != 0)
                            {
                                a2_4 = *(u8*)(g_menu_category1_item + 0x2D);
                            }
                            t0 = 0x21;
                            a2_4_shift = *(volatile u8*)(var_s4 + 1) - 0xA;
                            goto case2_a2_4_test;
                        }
                        if ((u32)(subtype - 0x12) < 8)
                        {
                            a2_4 = 0;
                            if (*(u8*)(D_80168C20 + 0x40) != 0)
                            {
                                a2_4 = *(u8*)(D_80168C20 + 0x6C);
                            }
                            if (*(u8*)(D_80168C20 + 0x80) != 0)
                            {
                                a2_4 |= *(u8*)(D_80168C20 + 0xAC);
                            }
                            if (*(u8*)(D_80168C20 + 0xC0) != 0)
                            {
                                a2_4 |= *(u8*)(D_80168C20 + 0xEC);
                            }
                            t0 = 0x21;
                            a2_4_shift = *(volatile u8*)(var_s4 + 1) - 0x12;
                        case2_a2_4_test:
                            if (((a2_4 >> a2_4_shift) & 1) != 0)
                            {
                                t0 = *(volatile u8*)(var_s4 + 1) + 0x2B;
                            }
                        }
                    else if ((u32)(subtype - 0x1A) < 8)
                    {
                        s32 a2_5 = 0;
                        if (*(u8*)(D_80168C20 + 0x40) != 0)
                        {
                            a2_5 = *(u8*)(D_80168C20 + 0x6D);
                        }
                        if (*(u8*)(D_80168C20 + 0x80) != 0)
                        {
                            a2_5 |= *(u8*)(D_80168C20 + 0xAD);
                        }
                        if (*(u8*)(D_80168C20 + 0xC0) != 0)
                        {
                            a2_5 |= *(u8*)(D_80168C20 + 0xED);
                        }
                        {
                            s32 item = *(volatile u8*)(var_s4 + 1);
                            if (((a2_5 >> (item - 0x1A)) & 1) != 0)
                            {
                                t0 = D_80168696[item];
                            }
                            else
                            {
                                t0 = 0x21;
                            }
                        }
                    }
                    else if ((u32)(subtype - 0x22) < 8)
                    {
                        t0 = 0x21;
                        if (g_menu_category0_item != 0)
                        {
                            s32 item = *(volatile u8*)(var_s4 + 1);
                            if (((*(u8*)(g_menu_category0_item + 0x2C) >> (item - 0x22)) & 1) != 0)
                            {
                                t0 = item + 0x13;
                            }
                        }
                    }
                    else if ((u32)(subtype - 0x2A) < 8)
                    {
                        s32 idx = 0;
                        void* base2 = (void*)g_pad_ctx;
                        u8* ptr = (u8*)base2 + g_menu_char_slot * 0x250;
                        s32 needle = *(volatile u8*)(var_s4 + 1) - 0x2A;
                        while (idx < 8)
                        {
                            if (*(u8*)(ptr + idx + 0x638) == needle)
                            {
                                break;
                            }
                            idx++;
                        }
                        t0 = D_801686B8[idx];
                    }
                    else if ((u32)(subtype - 0x32) < 4)
                    {
                        if (g_menu_char_slot != 2)
                        {
                            u8* tmp2 = (u8*)g_menu_equipment_base + (*(volatile u8*)(var_s4 + 1) << 6) - 0xC80;
                            t0 = 0x21;
                            if (*tmp2 != 0)
                            {
                                u32 val = *(u32*)(tmp2 + 0x14);
                                u32 sel = (val >> 8) & 3;
                                switch (sel)
                                {
                                case 1:
                                    t0 = ((val >> 10) & 0x3F) + 0x50;
                                    break;
                                case 2:
                                    t0 = ((val >> 10) & 0x3F) + 0x5C;
                                    break;
                                case 0:
                                    t0 = ((val >> 10) & 0x3F) + 0x45;
                                    break;
                                }
                            }
                        }
                    }
                    else if ((u8)subtype == 0x36)
                    {
                        t0 = 0xFF;
                        if (g_menu_item_ptr != 0)
                        {
                            if (*(u8*)g_menu_item_ptr != 0)
                            {
                                u32 val = *(u32*)(g_menu_item_ptr + 0x14);
                                u32 sel = (val >> 8) & 3;
                                switch (sel)
                                {
                                case 0:
                                    t0 = ((val >> 10) & 0x3F) + 0x45;
                                    break;
                                case 1:
                                    t0 = ((val >> 10) & 0x3F) + 0x50;
                                    break;
                                case 2:
                                    t0 = ((val >> 10) & 0x3F) + 0x5C;
                                    break;
                                }
                            }
                        }
                    }
                    else if ((u32)((u8)subtype - 0x37) < 0x1E)
                    {
                        t0 = D_80168659[*(volatile u8*)(var_s4 + 1)];
                    }
                    else if (subtype == 0x55)
                    {
                        t0 = 0xFF;
                        if (((*(u32*)((void*)g_pad_ctx + 0x28) >> 1) & 1) != 0)
                        {
                            t0 = 0x6A;
                        }
                    }
                    else if (subtype == 0x56)
                    {
                        t0 = 0x6A;
                        if (((*(u32*)((void*)g_pad_ctx + 0x28) >> 1) & 1) != 0)
                        {
                            t0 = 0xFF;
                        }
                    }
                    else if (subtype == 0x57)
                    {
                        t0 = 0xFF;
                        if ((*(u32*)((void*)g_pad_ctx + 0x28) & 1) != 0)
                        {
                            t0 = 0x6A;
                        }
                    }
                    else if (subtype == 0x58)
                    {
                        t0 = 0x6A;
                        if ((*(u32*)((void*)g_pad_ctx + 0x28) & 1) != 0)
                        {
                            t0 = 0xFF;
                        }
                    }
                    else if (subtype == 0x59)
                    {
                        t0 = 0xFF;
                        if ((*(s32*)((void*)g_pad_ctx + 0x858) & 0x80) != 0)
                        {
                            t0 = 0x6A;
                        }
                    }
                    else if (subtype == 0x5A)
                    {
                        t0 = 0x6A;
                        if ((*(s32*)((void*)g_pad_ctx + 0x858) & 0x80) != 0)
                        {
                            t0 = 0xFF;
                        }
                    }
                    else
                    {
                        t0 = 0xFF;
                    }
                    }
                    {
                        if (t0 != 0xFF)
                        {
                            u8 icon_arg = t0;
                            var_s1 = menu_emit_draw_mode_primitive(menu_emit_icon_sprite(var_s1, arg1, icon_arg, *var_s3 & 0x1FF, *var_s4 - 8, 0, 0, 0, 0), arg1);
                        }
                    }
                }
                break;

                case 3:
                {
                    s32 tmp = *(volatile u8*)(var_s4 + 1);
                    switch (tmp)
                    {
                    case 0x1:
                        if (g_menu_item_ptr != 0)
                        {
                            var_a0 = var_s1;
                            var_a2_2 = (void*)g_menu_item_ptr;
                            var_s1 = func_800A88A0(var_a0, arg1, var_a2_2, 1, *var_s3 & 0x1FF, *var_s4 - 8, (*var_s3 >> 9) & 7);
                        }
                        break;
                    case 0x2:
                        if (g_menu_item_ptr != 0)
                        {
                            u32 v;
                            u32 v1;

                            if (menu_item_is_nondefault((u8*)g_menu_item_ptr) != 0)
                            {
                                void* a3 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x30));
                                void* a2 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 8));
                                menu_concat_encoded_text(&sp68, (void*)((u8*)a3 + *(u16*)((u8*)a3 + (*(u16*)(g_menu_item_ptr + 0x16) & 0x3F) * 2)),
                                                         (void*)((u8*)a2 + *(u16*)((u8*)a2 + 0xB4)), a3);
                            }
                            else
                            {
                                sp68[0] = 0;
                            }
                            v = *(u32*)(g_menu_item_ptr + 0x14);
                            v1 = (v >> 8) & 3;
                            switch (v1)
                            {
                            case 0:
                            {
                                void* base0 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x68));
                                menu_concat_encoded_text(&sp28, &sp68, (void*)((u8*)base0 + *(u16*)((s32)((v >> 9) & 0x7E) + (s32)base0)));
                                break;
                            }
                            case 1:
                            {
                                void* base1 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x68));
                                menu_concat_encoded_text(&sp28, &sp68, (void*)((u8*)base1 + *(u16*)((s32)((v >> 9) & 0x7E) + (s32)base1 + 0x16)));
                                break;
                            }
                            default:
                            {
                                void* base2 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x68));
                                menu_concat_encoded_text(&sp28, &sp68, (void*)((u8*)base2 + *(u16*)((s32)((*(u32*)(g_menu_item_ptr + 0x14) >> 9) & 0x7E) + (s32)base2 + 0x2E)));
                                break;
                            }
                            }
                            var_s1 = func_800A88A0(var_s1, arg1, &sp28, 1, *var_s3 & 0x1FF, *var_s4 - 8, (*var_s3 >> 9) & 7);
                        }
                        break;
                    case 0x3:
                        if (g_menu_item_ptr != 0)
                        {
                            var_s1 = menu_draw_clamped_number(arg1, var_s1, *(u32*)(g_menu_item_ptr + 0x18) & 0xF, 1, pos_p, ((*var_s3 >> 9) & 7));
                        }
                        break;
                    case 0x4:
                        if (g_menu_item_ptr != 0)
                        {
                            var_s1 = menu_draw_clamped_number(arg1, var_s1, (*(u32*)(g_menu_item_ptr + 0x18) >> 4) & 0xF, 1, pos_p, ((*var_s3 >> 9) & 7));
                        }
                        break;
                    case 0x5:
                        if (g_menu_item_ptr != 0)
                        {
                            var_s1 = menu_draw_clamped_number(arg1, var_s1, (*(u32*)(g_menu_item_ptr + 0x18) >> 8) & 0xF, 1, pos_p, ((*var_s3 >> 9) & 7));
                        }
                        break;
                    case 0x6:
                        if (g_menu_item_ptr != 0)
                        {
                            var_s1 = menu_draw_clamped_number(arg1, var_s1, (*(u32*)(g_menu_item_ptr + 0x18) >> 12) & 0xF, 1, pos_p, ((*var_s3 >> 9) & 7));
                        }
                        break;
                    case 0x7:
                        if (g_menu_item_ptr != 0)
                        {
                            var_s1 = menu_draw_clamped_number(arg1, var_s1, *(u16*)(g_menu_item_ptr + 0x1A) & 0xF, 1, pos_p, ((*var_s3 >> 9) & 7));
                        }
                        break;
                    case 0x8:
                        if (g_menu_item_ptr != 0)
                        {
                            var_s1 = menu_draw_clamped_number(arg1, var_s1, (*(u32*)(g_menu_item_ptr + 0x18) >> 20) & 0xF, 1, pos_p, ((*var_s3 >> 9) & 7));
                        }
                        break;
                    case 0x9:
                        if (g_menu_item_ptr != 0)
                        {
                            var_s1 = menu_draw_clamped_number(arg1, var_s1, *(u8*)(g_menu_item_ptr + 0x1B) & 0xF, 1, pos_p, ((*var_s3 >> 9) & 7));
                        }
                        break;
                    case 0xA:
                        if (g_menu_item_ptr != 0)
                        {
                            var_s1 = menu_draw_clamped_number(arg1, var_s1, (*(u32*)(g_menu_item_ptr + 0x18) >> 28) & 0xF, 1, pos_p, ((*var_s3 >> 9) & 7));
                        }
                        break;
                    case 0xB:
                    case 0xC:
                    case 0xD:
                        if (g_menu_item_ptr != 0)
                        {
                            var_a0 = var_s1;
                            base_a2_1 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x5C));
                            {
                                s32 a3 = 1;
                                void* a2_2 = (void*)((u8*)base_a2_1 + *(u16*)((u8*)base_a2_1 + (*(u8*)(g_menu_item_ptr + (tmp - 0xB + 0x15)) * 2)));
                                var_s1 = func_800A88A0(var_a0, arg1, a2_2, a3, *var_s3 & 0x1FF, *var_s4 - 8, (*var_s3 >> 9) & 7);
                            }
                        }
                        break;
                    case 0xE:
                    case 0xF:
                    case 0x10:
                        if (g_menu_category0_item != 0)
                        {
                            var_a0 = var_s1;
                            base_a2_2 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x64));
                            {
                                s32 a3 = 1;
                                void* a2_2 = (void*)((u8*)base_a2_2 + *(u16*)((u8*)base_a2_2 + (*(u8*)(g_menu_category0_item + (tmp - 0xE + 0x1A)) * 2)));
                                var_s1 = func_800A88A0(var_a0, arg1, a2_2, a3, *var_s3 & 0x1FF, *var_s4 - 8, (*var_s3 >> 9) & 7);
                            }
                        }
                        break;
                    case 0x11:
                        if (g_menu_category0_item != 0)
                        {
                            var_s1 = func_800A8A78(arg1, var_s1, *(u16*)(g_menu_category0_item + 0x24), 1, pos_p, ((*var_s3 >> 9) & 7));
                        }
                        break;
                    case 0x12:
                        if (g_menu_category1_item != 0)
                        {
                            var_s1 = menu_draw_clamped_number(arg1, var_s1, *(u16*)(g_menu_category1_item + 0x24), 1, pos_p, ((*var_s3 >> 9) & 7));
                        }
                        break;
                    case 0x13:
                        if (g_menu_item_ptr != 0)
                        {
                            void* a2;
                            u8* a3ptr;
                            s32 idx;
                            void* a2_2;
                            var_a0 = var_s1;
                            a2 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x44));
                            a3ptr = (u8*)g_menu_category2_item;
                            idx = (a3ptr[0x24] * 0xE + a3ptr[0x25]) * 2;
                            a2_2 = (void*)((u8*)a2 + *(u16*)((u8*)a2 + idx));
                            var_s1 = func_800A88A0(var_a0, arg1, a2_2, 1, *var_s3 & 0x1FF, *var_s4 - 8, (*var_s3 >> 9) & 7);
                        }
                        break;
                    case 0x14:
                        if (g_menu_item_ptr != 0)
                        {
                            var_a0 = var_s1;
                            base_a2_3 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x28));
                            {
                                void* a2_2 = (void*)((u8*)base_a2_3 + *(u16*)((u8*)base_a2_3 + (*(u8*)(g_menu_category2_item + 0x24) * 2)));
                                var_s1 = func_800A88A0(var_a0, arg1, a2_2, 1, *var_s3 & 0x1FF, *var_s4 - 8, (*var_s3 >> 9) & 7);
                            }
                        }
                        break;
                    case 0x15:
                        if (g_menu_item_ptr != 0)
                        {
                            var_a0 = var_s1;
                            base_a2_4 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x38));
                            {
                                void* a2_2 = (void*)((u8*)base_a2_4 + *(u16*)((u8*)base_a2_4 + (*(u8*)(g_menu_category2_item + 0x25) * 2)));
                                var_s1 = func_800A88A0(var_a0, arg1, a2_2, 1, *var_s3 & 0x1FF, *var_s4 - 8, (*var_s3 >> 9) & 7);
                            }
                        }
                        break;
                    case 0x16:
                        if (g_menu_item_ptr != 0)
                        {
                            var_s1 = func_800A8A78(arg1, var_s1, *(u8*)(g_menu_category2_item + 0x26), 1, pos_p, ((*var_s3 >> 9) & 7));
                        }
                        break;
                    case 0x17:
                    case 0x18:
                    case 0x19:
                    case 0x1A:
                    {
                        s32 delta = 0;
                        if (D_80168C05[tmp] != 0)
                        {
                            u8* base2 = (u8*)((tmp << 6) + (s32)D_80168C30);
                            void* v1;
                            if (*(u8*)(base2 - 0x5C0) != 0)
                            {
                                base2 -= 0x5C0;
                                delta = *(u16*)(base2 + 0x24);
                            }
                            v1 = (void*)D_801694DC[tmp];
                            if (v1 != 0 && *(u8*)v1 != 0)
                                delta -= *(u16*)((u8*)v1 + 0x24);
                            else
                                delta -= D_800F0C1C;
                            if (delta < 0)
                                delta = -delta;
                            var_s1 = func_800A8A78(arg1, var_s1, (u16)delta, 1, pos_p, ((*var_s3 >> 9) & 7));
                        }
                    }
                    /* fallthrough */
                    case 0x1B:
                    case 0x1C:
                    case 0x1D:
                    case 0x1E:
                    {
                        s32 delta2 = 0;
                        if (D_80168C01[tmp] != 0)
                        {
                            u8* base2 = (u8*)((tmp << 6) + (s32)D_80168C30);
                            void* v1;
                            u8* source;
                            u8* label_buf;
                            if (*(u8*)(base2 - 0x6C0) != 0)
                            {
                                base2 -= 0x6C0;
                                delta2 = *(u16*)(base2 + 0x24);
                            }
                            v1 = (void*)D_801694CC[tmp];
                            if (v1 != 0 && *(u8*)v1 != 0)
                                delta2 -= *(u16*)((u8*)v1 + 0x24);
                            else
                                delta2 -= D_800F0C1C;
                            label_buf = spB0;
                            if (delta2 >= 0)
                            {
                                s32 string_page_base =
                                    (g_menu_label_key_a.page << 8) +
                                    (s32)MENU_STRING_TABLE_BASE(g_menu_label_key_a, nonnegative_label);
                                source = (u8*)(g_menu_label_key_a.entry + string_page_base);
                                func_800A8E28(label_buf, source, delta2);
                            }
                            else
                            {
                                s32 string_page_base =
                                    (g_menu_label_key_b.page << 8) +
                                    (s32)MENU_STRING_TABLE_BASE(g_menu_label_key_b, negative_label);
                                source = (u8*)(g_menu_label_key_b.entry + string_page_base);
                                func_800A8E28(label_buf, source, delta2);
                            }
                            label_buf += func_800A8DDC(source);
                            *label_buf = 0;
                            var_s1 = func_800A88A0(var_s1, arg1, &spB0, 1, pos.x, pos.y, 0);
                        }
                    }
                    break;
                    case 0x1F:
                    case 0x20:
                    case 0x21:
                    case 0x22:
                    case 0x23:
                    case 0x24:
                    case 0x25:
                    case 0x26:
                        if (g_menu_item_ptr != 0)
                        {
                            s32 res = menu_lookup_item_nibble(g_menu_item_ptr, tmp - 0x1F);
                            u8* source;
                            u8* label_buf;
                            label_buf = spB0;
                            if (res >= 0)
                            {
                                s32 string_page_base =
                                    (g_menu_label_key_a.page << 8) +
                                    (s32)MENU_STRING_TABLE_BASE(g_menu_label_key_a, nonnegative_label);
                                source = (u8*)(g_menu_label_key_a.entry + string_page_base);
                                func_800A8E28(label_buf, source);
                            }
                            else
                            {
                                s32 string_page_base =
                                    (g_menu_label_key_b.page << 8) +
                                    (s32)MENU_STRING_TABLE_BASE(g_menu_label_key_b, negative_label);
                                source = (u8*)(g_menu_label_key_b.entry + string_page_base);
                                func_800A8E28(label_buf, source);
                            }
                            label_buf += func_800A8DDC(source);
                            *label_buf = 0;
                            var_s1 = func_800A88A0(var_s1, arg1, &spB0, 1, pos.x, pos.y, 0);
                        }
                        break;
                    case 0x27:
                    case 0x28:
                    case 0x29:
                    case 0x2A:
                    case 0x2B:
                    case 0x2C:
                    case 0x2D:
                    case 0x2E:
                        if (g_menu_item_ptr != 0)
                        {
                            s32 diff = menu_lookup_item_nibble(g_menu_item_ptr, tmp - 0x27);
                            var_s1 = menu_draw_clamped_number(arg1, var_s1, (u32)(diff < 0 ? -diff : diff), 1, pos_p, ((*var_s3 >> 9) & 7));
                        }
                        break;
                    case 0x2F:
                    case 0x30:
                    case 0x31:
                    case 0x32:
                    case 0x33:
                    case 0x34:
                    case 0x35:
                    case 0x36:
                    {
                        s32 idx2 = tmp - 0x2F;
                        if (g_menu_item_ptr != 0)
                        {
                            s32 v1 = menu_lookup_item_nibble(g_menu_item_ptr, idx2);
                            s32 v2 = menu_lookup_item_nibble((void*)g_menu_active_equipped_item, idx2);
                            s32 diff = v1 - v2;
                            u8* source;
                            u8* label_buf;
                            label_buf = spB0;
                            if (diff >= 0)
                            {
                                s32 string_page_base =
                                    (g_menu_label_key_a.page << 8) +
                                    (s32)MENU_STRING_TABLE_BASE(g_menu_label_key_a, nonnegative_label);
                                source = (u8*)(g_menu_label_key_a.entry + string_page_base);
                                func_800A8E28(label_buf, source);
                            }
                            else
                            {
                                s32 string_page_base =
                                    (g_menu_label_key_b.page << 8) +
                                    (s32)MENU_STRING_TABLE_BASE(g_menu_label_key_b, negative_label);
                                source = (u8*)(g_menu_label_key_b.entry + string_page_base);
                                func_800A8E28(label_buf, source);
                            }
                            label_buf += func_800A8DDC(source);
                            *label_buf = 0;
                            var_s1 = func_800A88A0(var_s1, arg1, &spB0, 1, pos.x, pos.y, 0);
                        }
                    }
                    break;
                    case 0x37:
                    case 0x38:
                    case 0x39:
                    case 0x3A:
                    case 0x3B:
                    case 0x3C:
                    case 0x3D:
                    case 0x3E:
                    {
                        s32 idx2 = tmp - 0x2F;
                        if (g_menu_item_ptr != 0)
                        {
                            s32 v1 = menu_lookup_item_nibble(g_menu_item_ptr, idx2);
                            s32 v2 = menu_lookup_item_nibble((void*)g_menu_active_equipped_item, idx2);
                            s32 diff = v1 - v2;
                            if (diff < 0)
                                diff = -diff;
                            var_s1 = func_800A8A78(arg1, var_s1, (u16)diff, 1, pos_p, ((*var_s3 >> 9) & 7));
                        }
                    }
                    break;
                    case 0x3F:
                    case 0x40:
                    case 0x41:
                    case 0x42:
                        if (g_menu_category1_item != 0)
                        {
                            var_s1 = menu_draw_clamped_number(arg1, var_s1, *(u16*)(g_menu_category1_item + (tmp * 2) - 0x5A), 1, pos_p, ((*var_s3 >> 9) & 7));
                        }
                        break;
                    case 0x43:
                    case 0x44:
                    case 0x45:
                    case 0x46:
                    {
                        s32 has = 0;
                        s32 total = has;
                        u8* source;
                        u8* label_buf;
                        s32 item_off = tmp * 2;
                        for (k = 1; k < 4; k++)
                        {
                            if (((u8*)&g_item_slot_flags)[k] != 0)
                            {
                                u8* slot = D_80168C20 + (k * 0x40);
                                if (*slot != 0)
                                {
                                    total += *(u16*)(slot + item_off - 0x62);
                                }
                                has = ((u32*)&g_item_slot_data)[k];
                                if (has != 0 && *(u8*)has != 0)
                                {
                                    total -= *(u16*)(has + item_off - 0x62);
                                }
                                has = 1;
                            }
                        }
                        if (has)
                        {
                            label_buf = spB0;
                            if (total >= 0)
                            {
                                s32 string_page_base =
                                    (g_menu_label_key_a.page << 8) +
                                    (s32)MENU_STRING_TABLE_BASE(g_menu_label_key_a, nonnegative_label);
                                source = (u8*)(g_menu_label_key_a.entry + string_page_base);
                                func_800A8E28(label_buf, source, total, k);
                            }
                            else
                            {
                                s32 string_page_base =
                                    (g_menu_label_key_b.page << 8) +
                                    (s32)MENU_STRING_TABLE_BASE(g_menu_label_key_b, negative_label);
                                source = (u8*)(g_menu_label_key_b.entry + string_page_base);
                                func_800A8E28(label_buf, source, total, k);
                            }
                            label_buf += func_800A8DDC(source);
                            *label_buf = 0;
                            var_s1 = func_800A88A0(var_s1, arg1, &spB0, 1, pos.x, pos.y, 0);
                        }
                    }
                    break;
                    case 0x47:
                    case 0x48:
                    case 0x49:
                    case 0x4A:
                    {
                        s32 has = 0;
                        s32 total = has;
                        s32 item_off = tmp * 2;
                        for (k = 1; k < 4; k++)
                        {
                            if (((u8*)&g_item_slot_flags)[k] != 0)
                            {
                                u8* slot = D_80168C20 + (k * 0x40);
                                if (*slot != 0)
                                {
                                    total += *(u16*)(slot + item_off - 0x6A);
                                }
                                has = ((u32*)&g_item_slot_data)[k];
                                if (has != 0 && *(u8*)has != 0)
                                {
                                    total -= *(u16*)(has + item_off - 0x6A);
                                }
                                has = 1;
                            }
                        }
                        if (has)
                        {
                            if (total < 0)
                                total = -total;
                            var_s1 = func_800A8A78(arg1, var_s1, total, 1, pos_p, ((*var_s3 >> 9) & 7));
                        }
                    }
                    break;
                    default:
                        break;
                    }
                }
                break;

                case 4:
                {
                    shared_s0 = *(volatile u8*)(var_s4 + 1);
                    switch (shared_s0)
                    {
                    case 0x1:
                    {
                        u8 u_val;
                        u8 v_val;
                        SET_BGR0_PACKED(var_s1, GPU_TINT_NEUTRAL);
                        setSprt(var_s1);
                        ((SPRT*)var_s1)->x0 = *var_s3 & 0x1FF;
                        ((SPRT*)var_s1)->y0 = *var_s4 - 8;
                        u_val = 0xD0;
                        if (g_menu_char_slot == 2)
                        {
                            u_val = 0xA0;
                        }
                        ((SPRT*)var_s1)->u0 = u_val;
                        v_val = 0x50;
                        if (g_menu_char_slot == 0)
                        {
                            v_val = 0x20;
                        }
                        ((SPRT*)var_s1)->v0 = v_val;
                        SET_SPRT_WH_PACKED(var_s1, 0x30, 0x30);
                        SET_SPRT_CLUT(var_s1, (((u16)g_menu_char_slot + 0x1D8) << 6) | 0x11);
                        addPrim(arg1, var_s1);
                        var_s1 = (SPRT*)var_s1 + 1;
                        SET_BGR0_PACKED(var_s1, 0);
                        setlen(var_s1, 4);
                        setcode(var_s1, 0x66);
                        ((SPRT*)var_s1)->x0 = (*var_s3 & 0x1FF) + 2;
                        ((SPRT*)var_s1)->y0 = *var_s4 - 6;
                        u_val = 0xD0;
                        if (g_menu_char_slot == 2)
                        {
                            u_val = 0xA0;
                        }
                        ((SPRT*)var_s1)->u0 = u_val;
                        v_val = 0x50;
                        if (g_menu_char_slot == 0)
                        {
                            v_val = 0x20;
                        }
                        ((SPRT*)var_s1)->v0 = v_val;
                        SET_SPRT_WH_PACKED(var_s1, 0x30, 0x30);
                        SET_SPRT_CLUT(var_s1, (((u16)g_menu_char_slot + 0x1D8) << 6) | 0x11);
                        addPrim(arg1, var_s1);
                        var_s1 = (SPRT*)var_s1 + 1;
                        setDrawTPage(var_s1, 0, 0, 0x1F);
                        addPrim(arg1, var_s1);
                        var_s1 = (DR_TPAGE*)var_s1 + 1;
                    }
                    break;
                    case 0x2:
                    {
                        u8* call_base = (u8*)g_pad_ctx;
                        s32 off = g_menu_char_slot * 0x250 + 0x5F0;
                        var_s1 = func_800A88A0(var_s1, arg1, call_base + off, 1, *var_s3 & 0x1FF, *var_s4 - 8, (*var_s3 >> 9) & 7);
                    }
                    break;
                    case 0x3:
                    {
                        void* base = (void*)g_pad_ctx;
                        u8 v = *(u8*)((u8*)base + menu_probe_slot_off(g_menu_char_slot) + 0x610);
                        var_s1 = menu_draw_clamped_number(arg1, var_s1, v, 1, pos_p, ((*var_s3 >> 9) & 7));
                    }
                    break;
                    case 0x4:
                    {
                        u8* rec_base = (u8*)&D_80105AE0;
                        s32 rec_off = g_menu_char_slot * 0x23C;
                        var_s1 = func_800A8A78(arg1, var_s1, *(s32*)(rec_base + rec_off + 4), 1, pos_p, ((*var_s3 >> 9) & 7));
                    }
                        break;
                    case 0x5:
                    {
                        void* base = (void*)g_pad_ctx;
                        u16 v = *(u16*)((u8*)base + menu_probe_slot_off(g_menu_char_slot) + 0x614);
                        var_s1 = func_800A8A78(arg1, var_s1, v, 1, pos_p, ((*var_s3 >> 9) & 7));
                    }
                    break;
                    case 0x6:
                    {
                        void* base = (void*)g_pad_ctx;
                        u32 v = *(u32*)((u8*)base + menu_probe_slot_off(g_menu_char_slot) + 0x610);
                        var_s1 = func_800A8A78(arg1, var_s1, (v >> 8), 1, pos_p, ((*var_s3 >> 9) & 7));
                    }
                    break;
                    case 0x7:
                    case 0x8:
                    case 0x9:
                    case 0xA:
                    case 0xB:
                    case 0xC:
                    case 0xD:
                    case 0xE:
                    {
                        void* base = (void*)g_pad_ctx;
                        s32 idx = shared_s0 - 7;
                        var_s1 = menu_draw_clamped_number(arg1, var_s1, *(u16*)((u8*)base + (idx << 1) + g_menu_char_slot * 0x250 + 0x620) >> 9, 1, pos_p, ((*var_s3 >> 9) & 7));
                    }
                    break;
                    case 0xF:
                    {
                        void* a2_2 = (void*)g_menu_equipment_base;
                        if (*(u8*)a2_2 != 0)
                        {
                            s32 a3 = 1;
                            if (g_item_slot_flags.slot0 != 0)
                                a3 = 2;
                            var_s1 = func_800A88A0(var_s1, arg1, a2_2, a3, *var_s3 & 0x1FF, *var_s4 - 8, (*var_s3 >> 9) & 7);
                        }
                    }
                    break;
                    case 0x10:
                    case 0x11:
                    case 0x12:
                        if (*(u8*)D_80168C30 != 0)
                        {
                            var_a0 = var_s1;
                            base_a2_5 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x64));
                            {
                                void* a2_2 = (void*)((u8*)base_a2_5 + *(u16*)((u8*)base_a2_5 + (*(u8*)(D_80168C30 + (shared_s0 - 0x10 + 0x18)) * 2)));
                                var_s1 = func_800A88A0(var_a0, arg1, a2_2, 1, *var_s3 & 0x1FF, *var_s4 - 8, (*var_s3 >> 9) & 7);
                            }
                        }
                        break;
                    case 0x13:
                    {
                        u16 a2_15 = 0;
                        if (g_item_slot_flags.slot0 != 0)
                        {
                            void* v1_12 = (void*)g_item_slot_data.slot0;
                            if (v1_12 == 0 || *(u8*)v1_12 == 0)
                                a2_15 = D_800F0C1C;
                            else
                                a2_15 = *(u16*)((u8*)v1_12 + 0x24);
                        }
                        else
                        {
                            if (*(u8*)D_80168C30 != 0)
                                a2_15 = *(u16*)(D_80168C30 + 0x24);
                        }
                        var_s1 = func_800A8A78(arg1, var_s1, a2_15, 1, pos_p, ((*var_s3 >> 9) & 7));
                    }
                    break;
                    case 0x14:
                    case 0x15:
                    case 0x16:
                    {
                        u8* base = (u8*)g_menu_equipment_base + (shared_s0 << 6);
                        if (*(u8*)(base - 0x4C0) != 0)
                        {
                            s32 a3 = 1;
                            void* a2_2;
                            if (D_80168C09[shared_s0] != 0)
                                a3 = 2;
                            a2_2 = (void*)(g_menu_equipment_base + (shared_s0 << 6) - 0x4C0);
                            var_s1 = func_800A88A0(var_s1, arg1, a2_2, a3, *var_s3 & 0x1FF, *var_s4 - 8, (*var_s3 >> 9) & 7);
                        }
                    }
                    break;
                    case 0x17:
                    case 0x18:
                    {
                        void* base;
                        u8 idx;
                        var_a0 = var_s1;
                        base_a2_6 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x10));
                        base = (void*)g_pad_ctx;
                        {
                            u8* idxp = (u8*)base + g_menu_char_slot * 0x250 + shared_s0 + 0x5F3;
                            idx = *idxp;
                        }
                        {
                            void* a2_2 = (void*)((u8*)base_a2_6 + *(u16*)((u8*)base_a2_6 + (idx * 2)));
                            var_s1 = func_800A88A0(var_a0, arg1, a2_2, 1, *var_s3 & 0x1FF, *var_s4 - 8, (*var_s3 >> 9) & 7);
                        }
                    }
                    break;
                    case 0x19:
                    {
                        s32 v = func_800B607C(g_menu_char_slot);
                        void* base = (void*)g_pad_ctx;
                        u32 shift = *(u32*)((u8*)base + menu_probe_slot_off(g_menu_char_slot) + 0x610) >> 8;
                        var_s1 = func_800A8A78(arg1, var_s1, v - shift, 1, pos_p, ((*var_s3 >> 9) & 7));
                    }
                    break;
                    case 0x1B:
                    case 0x1C:
                    case 0x1D:
                    case 0x1E:
                    {
                        void* base = (void*)g_pad_ctx;
                        u8* ptr;
                        u8 val;
                        shared_s0 = (u32)((u8*)base + g_menu_char_slot * 0x250 + shared_s0);
                        ptr = (u8*)shared_s0;
                        val = *(ptr + 0x5F1);
                        if (val != 0xFF)
                        {
                            var_a0 = var_s1;
                            if (val & 0x80)
                            {
                                u16* lvar_v1_2 = (u16*)g_pad_ctx;
                                void* lvar_a2 = (void*)((u8*)lvar_v1_2 + (g_menu_char_slot * 0x250 + 0x5F0));
                                void* a2_2 = (void*)((u8*)lvar_a2 + (((*(volatile u8*)(ptr + 0x5F1) & 0x7F) << 6) + 0x150));
                                var_s1 = func_800A88A0(var_a0, arg1, a2_2, 1, *var_s3 & 0x1FF, *var_s4 - 8, (*var_s3 >> 9) & 7);
                            }
                            else
                            {
                                u32 tmpv;
                                u8 idx;
                                base_a2_7 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x20));
                                tmpv = ((*(u32*)(g_menu_equipment_base + 0x14) >> 10) & 0x3F);
                                tmpv *= 3;
                                tmpv <<= 4;
                                idx = menu_reload_plain_u8(ptr + 0x5F1);
                                idx &= 0x7F;
                                {
                                    void* a2_2 = (void*)((u8*)base_a2_7 + *(u16*)((u8*)base_a2_7 + (idx * 2) + tmpv));
                                    var_s1 = func_800A88A0(var_a0, arg1, a2_2, 1, *var_s3 & 0x1FF, *var_s4 - 8, (*var_s3 >> 9) & 7);
                                }
                            }
                        }
                    }
                    break;
                    case 0x1F:
                    {
                        void* base;
                        u8 idx;
                        var_a0 = var_s1;
                        base_a2_8 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x4C));
                        base = (void*)g_pad_ctx;
                        {
                            u8* idxp = (u8*)base + g_menu_char_slot * 0x250 + 0x609;
                            idx = *idxp;
                        }
                        {
                            void* a2_2 = (void*)((u8*)base_a2_8 + *(u16*)((u8*)base_a2_8 + (idx * 2)));
                            var_s1 = func_800A88A0(var_a0, arg1, a2_2, 1, *var_s3 & 0x1FF, *var_s4 - 8, (*var_s3 >> 9) & 7);
                        }
                    }
                    break;
                    case 0x20:
                    {
                        u32 total = 0;
                        for (k = 1; k < 4; k++)
                        {
                            if (((u8*)&g_item_slot_flags)[k] != 0)
                            {
                                u32 ptr = ((u32*)&g_item_slot_data)[k];
                                if (ptr != 0 && *(u8*)ptr != 0)
                                    total += *(u16*)(ptr + 0x24);
                            }
                            else
                            {
                                u8* v = D_80168C20 + 0x40 + (k - 1) * 0x40;
                                if (*v != 0)
                                    total += *(u16*)(v + 0x24);
                            }
                        }
                        var_s1 = menu_draw_clamped_number(arg1, var_s1, total, 1, pos_p, ((*var_s3 >> 9) & 7));
                    }
                    break;
                    case 0x21:
                    {
                        u32 total = 0;
                        for (k = 1; k < 4; k++)
                        {
                            if (((u8*)&g_item_slot_flags)[k] != 0)
                            {
                                u32 ptr = ((u32*)&g_item_slot_data)[k];
                                if (ptr != 0 && *(u8*)ptr != 0)
                                    total += *(u16*)(ptr + 0x26);
                            }
                            else
                            {
                                u8* v = D_80168C20 + 0x40 + (k - 1) * 0x40;
                                if (*v != 0)
                                    total += *(u16*)(v + 0x26);
                            }
                        }
                        var_s1 = menu_draw_clamped_number(arg1, var_s1, total, 1, pos_p, ((*var_s3 >> 9) & 7));
                    }
                    break;
                    case 0x22:
                    {
                        u32 total = 0;
                        for (k = 1; k < 4; k++)
                        {
                            if (((u8*)&g_item_slot_flags)[k] != 0)
                            {
                                u32 ptr = ((u32*)&g_item_slot_data)[k];
                                if (ptr != 0 && *(u8*)ptr != 0)
                                    total += *(u16*)(ptr + 0x28);
                            }
                            else
                            {
                                u8* v = D_80168C20 + 0x40 + (k - 1) * 0x40;
                                if (*v != 0)
                                    total += *(u16*)(v + 0x28);
                            }
                        }
                        var_s1 = menu_draw_clamped_number(arg1, var_s1, total, 1, pos_p, ((*var_s3 >> 9) & 7));
                    }
                    break;
                    case 0x23:
                    {
                        u32 total = 0;
                        for (k = 1; k < 4; k++)
                        {
                            if (((u8*)&g_item_slot_flags)[k] != 0)
                            {
                                u32 ptr = ((u32*)&g_item_slot_data)[k];
                                if (ptr != 0 && *(u8*)ptr != 0)
                                    total += *(u16*)(ptr + 0x2A);
                            }
                            else
                            {
                                u8* v = D_80168C20 + 0x40 + (k - 1) * 0x40;
                                if (*v != 0)
                                    total += *(u16*)(v + 0x2A);
                            }
                        }
                        var_s1 = menu_draw_clamped_number(arg1, var_s1, total, 1, pos_p, ((*var_s3 >> 9) & 7));
                    }
                    break;
                    case 0x3F:
                    {
                        if (g_item_slot_flags.slot0 != 0)
                        {
                            u16 a2_20 = 0;
                            s32 diff;
                            u8* source;
                            u8* label_buf;
                            if (*(u8*)D_80168C20 != 0)
                                a2_20 = *(u16*)(D_80168C30 + 0x24);
                            if (g_item_slot_data.slot0 != 0 && *(u8*)g_item_slot_data.slot0 != 0)
                                diff = a2_20 - *(u16*)(g_item_slot_data.slot0 + 0x24);
                            else
                                diff = a2_20 - D_800F0C1C;
                            label_buf = spB0;
                            if (diff >= 0)
                            {
                                s32 string_page_base =
                                    (g_menu_label_key_a.page << 8) +
                                    (s32)MENU_STRING_TABLE_BASE(g_menu_label_key_a, nonnegative_label);
                                source = (u8*)(g_menu_label_key_a.entry + string_page_base);
                                func_800A8E28(label_buf, source, diff);
                            }
                            else
                            {
                                s32 string_page_base =
                                    (g_menu_label_key_b.page << 8) +
                                    (s32)MENU_STRING_TABLE_BASE(g_menu_label_key_b, negative_label);
                                source = (u8*)(g_menu_label_key_b.entry + string_page_base);
                                func_800A8E28(label_buf, source, diff);
                            }
                            label_buf += func_800A8DDC(source);
                            *label_buf = 0;
                            var_s1 = func_800A88A0(var_s1, arg1, &spB0, 1, pos.x, pos.y, 0);
                        }
                    }
                    break;
                    case 0x40:
                    case 0x41:
                    case 0x42:
                    case 0x43:
                    {
                        s32 has = 0;
                        s32 total = 0;
                        u32* lvar_t0_4;
                        u8* lvar_v1_18;
                        u8* source;
                        u8* label_buf;
                        s32 item_off;
                        k = 1;
                        lvar_t0_4 = (u32*)&g_item_slot_data;
                        lvar_t0_4 += 1;
                        item_off = shared_s0 * 2;
                        lvar_v1_18 = D_80168C20;
                        lvar_v1_18 += 0x40;
                        do
                        {
                            if (((u8*)&g_item_slot_flags)[k] != 0)
                            {
                                if (*lvar_v1_18 != 0)
                                    total += *(u16*)(lvar_v1_18 + item_off - 0x5C);
                                tmpa0 = *lvar_t0_4;
                                if (tmpa0 != 0 && *(u8*)tmpa0 != 0)
                                    total -= *(u16*)(tmpa0 + item_off - 0x5C);
                                has = 1;
                            }
                            lvar_t0_4 += 1;
                            k++;
                            lvar_v1_18 += 0x40;
                        } while (k < 4);
                        if (has)
                        {
                            label_buf = spB0;
                            if (total >= 0)
                            {
                                s32 string_page_base =
                                    (g_menu_label_key_a.page << 8) +
                                    (s32)MENU_STRING_TABLE_BASE(g_menu_label_key_a, nonnegative_label);
                                source = (u8*)(g_menu_label_key_a.entry + string_page_base);
                                func_800A8E28(label_buf, source, total, k);
                            }
                            else
                            {
                                s32 string_page_base =
                                    (g_menu_label_key_b.page << 8) +
                                    (s32)MENU_STRING_TABLE_BASE(g_menu_label_key_b, negative_label);
                                source = (u8*)(g_menu_label_key_b.entry + string_page_base);
                                func_800A8E28(label_buf, source, total, k);
                            }
                            label_buf += func_800A8DDC(source);
                            *label_buf = 0;
                            var_s1 = func_800A88A0(var_s1, arg1, &spB0, 1, pos.x, pos.y, 0);
                        }
                    }
                    break;
                    case 0x44:
                    {
                        if (g_item_slot_flags.slot0 != 0)
                        {
                            u16 a2_23 = 0;
                            s32 diff;
                            if (*(u8*)D_80168C20 != 0)
                                a2_23 = *(u16*)(D_80168C30 + 0x24);
                            if (g_item_slot_data.slot0 != 0 && *(u8*)g_item_slot_data.slot0 != 0)
                                diff = a2_23 - *(u16*)(g_item_slot_data.slot0 + 0x24);
                            else
                                diff = a2_23 - D_800F0C1C;
                            if (diff < 0)
                                diff = -diff;
                            var_s1 = func_800A8A78(arg1, var_s1, diff, 1, pos_p, ((*var_s3 >> 9) & 7));
                        }
                    }
                    break;
                    case 0x45:
                    case 0x46:
                    case 0x47:
                    case 0x48:
                    {
                        s32 has = 0;
                        s32 total = 0;
                        u32* lvar_t0_5;
                        u8* lvar_v1_20;
                        s32 item_off;
                        k = 1;
                        lvar_v1_20 = D_80168C20;
                        lvar_v1_20 += 0x40;
                        item_off = shared_s0 * 2;
                        lvar_t0_5 = &g_item_slot_data.slot1;
                        do
                        {
                            if (((u8*)&g_item_slot_flags)[k] != 0)
                            {
                                if (*lvar_v1_20 != 0)
                                    total += *(u16*)(lvar_v1_20 + item_off - 0x66);
                                tmpa0 = *lvar_t0_5;
                                if (tmpa0 != 0 && *(u8*)tmpa0 != 0)
                                    total -= *(u16*)(tmpa0 + item_off - 0x66);
                                has = 1;
                            }
                            lvar_t0_5 += 1;
                            k++;
                            lvar_v1_20 += 0x40;
                        } while (k < 4);
                        if (has)
                        {
                            if (total < 0)
                                total = -total;
                            var_s1 = menu_draw_clamped_number(arg1, var_s1, (u32)total, 1, pos_p, ((*var_s3 >> 9) & 7));
                        }
                    }
                    break;
                    case 0x49:
                    {
                        void* base;
                        u8 idx;
                        var_a0 = var_s1;
                        base_a2_9 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x74));
                        base = (void*)g_pad_ctx;
                        {
                            u8* idxp = (u8*)base + g_menu_char_slot * 0x250 + 0x633;
                            idx = *idxp;
                        }
                        {
                            void* a2_2 = (void*)((u8*)base_a2_9 + *(u16*)((u8*)base_a2_9 + (idx * 2)));
                            var_s1 = func_800A88A0(var_a0, arg1, a2_2, 1, *var_s3 & 0x1FF, *var_s4 - 8, (*var_s3 >> 9) & 7);
                        }
                    }
                    break;
                    case 0x4A:
                    {
                        s8 v1 = *(s8*)((void*)g_pad_ctx + 0x29D7);
                        if (v1 >= 0)
                        {
                            void* base = (void*)g_pad_ctx;
                            u8 v = *(u8*)((u8*)base + menu_probe_node_off(v1) + 0x2B50) & 0xF;
                            void* a2 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x80));
                            var_s1 =
                                func_800A88A0(var_s1, arg1, (void*)((u8*)a2 + *(u16*)((u8*)a2 + (v * 2))), 1, *var_s3 & 0x1FF, *var_s4 - 8, (*var_s3 >> 9) & 7);
                        }
                    }
                    break;
                    case 0x4B:
                    {
                        s8 v1 = *(s8*)((void*)g_pad_ctx + 0x29D7);
                        if (v1 >= 0)
                        {
                            void* base = (void*)g_pad_ctx;
                            u8 v = *(u8*)((u8*)base + menu_probe_node_off(v1) + 0x2B50) >> 4;
                            void* a2 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x7C));
                            var_s1 =
                                func_800A88A0(var_s1, arg1, (void*)((u8*)a2 + *(u16*)((u8*)a2 + (v * 2))), 1, *var_s3 & 0x1FF, *var_s4 - 8, (*var_s3 >> 9) & 7);
                        }
                    }
                    break;
                    case 0x4C:
                    {
                        s8 v1 = *(s8*)((void*)g_pad_ctx + 0x29D7);
                        if (v1 >= 0)
                        {
                            void* base = (void*)g_pad_ctx;
                            u8 v = *(u8*)((u8*)base + menu_probe_node_off(v1) + 0x2B52);
                            var_s1 = func_800A8A78(arg1, var_s1, v, 1, pos_p, ((*var_s3 >> 9) & 7));
                        }
                    }
                    break;
                    case 0x4D:
                    {
                        s8 v1 = *(s8*)((void*)g_pad_ctx + 0x29D7);
                        if (v1 >= 0)
                        {
                            void* base;
                            s32 idx;
                            var_a0 = var_s1;
                            base_a2_10 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x70));
                            base = (void*)g_pad_ctx;
                            idx = *(s32*)((u8*)base + menu_probe_node_off(v1) + 0x2B54);
                            {
                                void* a2_2 = (void*)((u8*)base_a2_10 + *(u16*)((u8*)base_a2_10 + (idx * 2)));
                                var_s1 = func_800A88A0(var_a0, arg1, a2_2, 1, *var_s3 & 0x1FF, *var_s4 - 8, (*var_s3 >> 9) & 7);
                            }
                        }
                    }
                    break;
                    case 0x4E:
                    case 0x4F:
                    case 0x50:
                    case 0x51:
                    case 0x52:
                    case 0x53:
                    case 0x54:
                    case 0x55:
                    {
                        u32 a2_26 = 0;
                        void** data_pp = (void**)&g_pad_ctx;
                        switch (shared_s0)
                        {
                        case 0x4E:
                            a2_26 = *(u32*)((u8*)(void*)g_pad_ctx + (g_menu_char_slot * 0x250) + 0x658) & 0xF;
                            break;
                        case 0x4F:
                            a2_26 = (*(u8*)((u8*)(void*)g_pad_ctx + (g_menu_char_slot * 0x250) + 0x658) >> 4);
                            break;
                        case 0x50:
                            a2_26 = (*(u32*)((u8*)(void*)g_pad_ctx + menu_probe_slot_off(g_menu_char_slot) + 0x658) >> 8) & 0xF;
                            break;
                        case 0x51:
                            a2_26 = (*(u32*)((u8*)(void*)g_pad_ctx + menu_probe_slot_off(g_menu_char_slot) + 0x658) >> 12) & 0xF;
                            break;
                        case 0x52:
                            a2_26 = *(u16*)((u8*)*data_pp + (g_menu_char_slot * 0x250) + 0x65A) & 0xF;
                            break;
                        case 0x53:
                            a2_26 = (*(u32*)((u8*)*data_pp + menu_probe_slot_off(g_menu_char_slot) + 0x658) >> 20) & 0xF;
                            break;
                        case 0x54:
                            a2_26 = *(u8*)((u8*)*data_pp + (g_menu_char_slot * 0x250) + 0x65B) & 0xF;
                            break;
                        case 0x55:
                            a2_26 = *(u32*)((u8*)*data_pp + menu_probe_slot_off(g_menu_char_slot) + 0x658) >> 28;
                            break;
                        }
                        var_s1 = menu_draw_clamped_number(arg1, var_s1, a2_26, 1, pos_p, ((*var_s3 >> 9) & 7));
                    }
                    break;
                    default:
                        break;
                    }
                }
                break;

                case 6:
                {
                    shared_s0 = *(volatile u8*)(var_s4 + 1);
                    switch (shared_s0)
                    {
                    case 1:
                        if (*(u32*)((void*)g_pad_ctx + 0x2C) > 0x989680U)
                        {
                            var_s1 = func_800A8A78(arg1, var_s1, 0x989680U, 1, pos_p, ((*var_s3 >> 9) & 7));
                        }
                        else
                        {
                            var_s1 = func_800A8A78(arg1, var_s1, *(u32*)((void*)g_pad_ctx + 0x2C), 1, pos_p, ((*var_s3 >> 9) & 7));
                        }
                        break;
                    case 2:
                    {
                        u32 ltemp_v0_6;
                        s32 v1;
                        s32 s2;
                        s32 split_tmp;
                        s32 one;
                        ltemp_v0_6 = (*var_s3 >> 9) & 7;
                        switch (ltemp_v0_6)
                        {
                        case 1:
                            pos.x -= 0x32;
                            break;
                        case 2:
                            pos.x -= 0x19;
                            break;
                        }
                        v1 = *(s32*)((void*)g_pad_ctx + 0x30) + VSync(-1);
                        shared_s0 = v1 - D_80042FB4;
                        pos.x += 0x14;
                        s2 = shared_s0 / 216000;
                        one = 1;
                        var_s1 = (void*)func_800A8A78(arg1, var_s1, s2, one, pos_p, one);
                        if ((g_frame_counter / 15) & 1)
                        {
                            var_s1 = func_800A88A0(var_s1, arg1, ":", one, pos.x, pos.y, 0);
                        }
                        pos.x += 7;
                        split_tmp = s2 * 0x3C;
                        shared_s0 = (shared_s0 / 3600) - split_tmp;
                        if (shared_s0 < 0xA)
                            var_s1 = (void*)func_800A8A78(arg1, var_s1, 0U, 1, pos_p, 0);
                        pos.x += 0x10;
                        var_s1 = (void*)func_800A8A78(arg1, var_s1, shared_s0, 1, pos_p, one);
                    }
                    break;
                    }
                }
                break;

                case 7:
                {
                    s32 two = 2;
                    shared_s0 = *(volatile u8*)(var_s4 + 1);
                    switch (shared_s0)
                    {
                    case 1:
                    {
                        void* base = (void*)g_pad_ctx + g_menu_char_slot * 0x250;
                        if ((*(u8*)((u8*)base + 0x608) & 0x7F) != two || ((*(u8*)((u8*)base + 0x609) != 5) && (*(u8*)((u8*)base + 0x609) != 8)))
                        {
                            var_a0 = var_s1;
                            base_a2_11 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 8));
                            {
                                void* a2_2 = (void*)((u8*)base_a2_11 + *(u16*)((u8*)base_a2_11 + 0x74));
                                var_s1 = func_800A88A0(var_a0, arg1, a2_2, 1, *var_s3 & 0x1FF, *var_s4 - 8, (*var_s3 >> 9) & 7);
                            }
                        }
                    }
                    break;
                    case 2:
                    {
                        void* base = (void*)g_pad_ctx + g_menu_char_slot * 0x250;
                        if ((*(u8*)((u8*)base + 0x608) & 0x7F) != 2 || ((*(u8*)((u8*)base + 0x609) != 5) && (*(u8*)((u8*)base + 0x609) != 8)))
                        {
                            var_a0 = var_s1;
                            base_a2_12 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 8));
                            {
                                void* a2_2 = (void*)((u8*)base_a2_12 + *(u16*)((u8*)base_a2_12 + 0x76));
                                var_s1 = func_800A88A0(var_a0, arg1, a2_2, 1, *var_s3 & 0x1FF, *var_s4 - 8, (*var_s3 >> 9) & 7);
                            }
                        }
                    }
                    break;
                    }
                }
                break;

                default:
                    break;
                }
                }

                var_s4 += 8;
                var_s3 += 4;
                spC0--;
            } while (spC0 != 0);
            }

            {
                MenuNode* node = g_menu_nodes;
                node += g_menu_scene_type;
                if (node->label_id == 0x13)
                {
                    u8* a2_27 = (u8*)g_menu_item_ptr;
                    if (*a2_27 != 0)
                    {
                        var_s1 = func_800A88A0(var_s1, arg1, a2_27, 1, 0xAC, 0xC, 2);
                    }
                }
                else if (g_menu_scene_type == 0x1D)
                {
                    void* a2_28 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 8));
                    u16 v1 = *(u16*)((u8*)a2_28 + 0x78);
                    void* a2_27 = (void*)((u8*)a2_28 + v1);
                    var_s1 = func_800A88A0(var_s1, arg1, a2_27, 1, 0xAC, 0xC, 2);
                }
                else if (g_menu_scene_type != -1)
                {
                    void* a2_28 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 8));
                    u16 v1 = *(u16*)((u8*)a2_28 + node->label_id * 2);
                    void* a2_27 = (void*)((u8*)a2_28 + v1);
                    var_s1 = func_800A88A0(var_s1, arg1, a2_27, 1, 0xAC, 0xC, 2);
                }
            }

            switch (g_menu_scene_type)
            {
            case 20:
            case 21:
            case 23:
            case 24:
            case 26:
            case 27:
            {
                void* ltemp;
                pos.x = 0x88;
                pos.y = 0x28;
                if (g_menu_page_count != 0)
                {
                    ltemp = func_800AD208(arg1, var_s1, g_script_repeat_last + 1, 3, &pos, 0);
                }
                else
                {
                    ltemp = func_800AD208(arg1, var_s1, 0, 3, &pos, 0);
                }
                ltemp = func_800AD524(ltemp, arg1, 0xB, &pos, 0);
                pos.x += 8;
                ltemp = func_800AD208(arg1, ltemp, g_menu_page_count, 3, &pos, 0);
                ltemp = func_800AD524(ltemp, arg1, 0xB, &pos, 0);
                pos.x += 8;
                var_s1 = func_800AD208(arg1, ltemp, menu_count_inventory_items(), 3, &pos, 0);
            }
            break;
            default:
                break;
            }
        }
    }

    return var_s1;
}


extern u8 D_800F0BE0[];
extern u8 D_800F0BEC[];

/**
 * @brief Scan up to arg0 entries in g_menu_equipment_base and OR together lookup bytes keyed by bits 10-15 of unk14.
 * @param arg0 Maximum number of entries to inspect (loop exits early when the index equals this value).
 * @return Bitwise OR of the looked-up bytes from each active entry, or 0 if none are active.
 * @see decomp.me (100%)
 */
s32 menu_get_equipment_ability_mask(s32 arg0)
{
    s32 i;
    s32 result;
    u8* entry;
    u32 unk14;

    result = 0;
    entry = (u8*)g_menu_equipment_base;
    i = 0;
    do
    {
        if ((i != arg0) && (entry[0] != 0))
        {
            unk14 = *(u32*)(entry + 0x14);
            if (unk14 & 0x300)
            {
                result |= D_800F0BEC[(unk14 >> 10) & 0x3F];
            }
            else
            {
                result |= D_800F0BE0[(unk14 >> 10) & 0x3F];
            }
        }
        i += 1;
        entry += 0x40;
    } while (i < 4);
    return result;
}

/**
 * @brief Initialize a one-word Draw Mode Setting primitive (GP0 0xE1000005) and link it into an OT slot.
 * @param prim Pointer to an uninitialized MenuPrimHead to fill in (must have at least 8 bytes of space).
 * @param ot Pointer to the ordering-table entry that the new primitive should be prepended to.
 * @return Pointer to the byte immediately following the 8-byte primitive (next free prim slot).
 * @see decomp.me (100%) https://decomp.me/scratch/3Wup8
 */
void* menu_emit_draw_mode_primitive(MenuPrimHead* prim, s32* ot)
{
    DR_TPAGE* draw_mode;

    draw_mode = (DR_TPAGE*)prim;
    setDrawTPage(draw_mode, 0, 0, 5);
    addPrim(ot, draw_mode);
    draw_mode++;
    return draw_mode;
}

/**
 * @brief Concatenate two encoded text command streams into a destination buffer.
 * @param dst Destination byte buffer; receives all bytes from src1, then src2, then a null terminator.
 * @param src1 First source stream; processed until its null terminator.
 * @param src2 Second source stream; appended after src1, processed until its null terminator.
 * @see decomp.me (100%)
 */
void menu_concat_encoded_text(u8* dst, u8* src1, u8* src2)
{
    for (;;)
    {
        u32 ch = *src1;

        if (ch == 0)
        {
            break;
        }
        if ((u32)(ch - 0x19) < 7)
        {
            *dst++ = ch;
            src1++;
            *dst++ = *src1++;
        }
        else
        {
            *dst++ = ch;
            src1++;
        }
    }

    for (;;)
    {
        u32 ch = *src2;

        if (ch == 0)
        {
            break;
        }
        if ((u32)(ch - 0x19) < 7)
        {
            *dst++ = ch;
            src2++;
            *dst++ = *src2++;
        }
        else
        {
            *dst++ = ch;
            src2++;
        }
    }

    *dst = 0;
}

extern s8 D_800F0C38[];

/**
 * @brief Extract a 4-bit nibble from a packed u32 field in the item struct and look it up in a byte table.
 * @param item Pointer to the item data record; nibbles are packed into the u32 at byte offset 0x1C.
 * @param index Nibble selector (0-7), selecting four bits at a time.
 * @param fallback Default table index used when @p index is out of range.
 * @return Signed byte from D_800F0C38 at the selected nibble index.
 * @see decomp.me (99.78%)
 */
s8 menu_lookup_item_nibble(void* item, u32 index, u32 fallback)
{
    u32 nibble;

    nibble = fallback;
    if (index < 8U)
    {
        switch (index)
        {
        case 0:
            nibble = *(u32*)((u8*)item + 0x1C) & 0xF;
            break;
        case 1:
            nibble = *(u8*)((u8*)item + 0x1C);
            nibble = nibble >> 4;
            break;
        case 2:
            nibble = *(u32*)((u8*)item + 0x1C) >> 8;
            nibble = nibble & 0xF;
            break;
        case 3:
            nibble = *(u32*)((u8*)item + 0x1C) >> 12;
            nibble = nibble & 0xF;
            break;
        case 4:
            nibble = *(u16*)((u8*)item + 0x1E);
            nibble = nibble & 0xF;
            break;
        case 5:
            nibble = *(u32*)((u8*)item + 0x1C) >> 20;
            nibble = nibble & 0xF;
            break;
        case 6:
            nibble = *(u8*)((u8*)item + 0x1F);
            nibble = nibble & 0xF;
            break;
        case 7:
            nibble = *(u32*)((u8*)item + 0x1C) >> 28;
            break;
        }
    }
    return D_800F0C38[nibble];
}


/**
 * @brief Search the current scene's content table for the first item flagged as the active hit item.
 * @return Index into the content table array of the first matching MenuContentItem, or -1 if none found.
 * @see decomp.me (100%)
 */
s32 menu_find_active_content_item(void)
{
    u8 self_idx;
    s32 count;
    MenuContentItem* items;
    s32 i;
    u16 packed_x;
    u16 upper;

    if (g_menu_scene_type == -1)
    {
        return -1;
    }

    self_idx = g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx;
    items = g_menu_content_table[self_idx];
    count = g_menu_content_item_counts[g_menu_content_group_ids[self_idx]];

    for (i = 0; i < count; i++, items++)
    {
        packed_x = items->packed_x;
        upper = packed_x & 0xF000;
        if ((upper == 0xF000 || upper == 0x5000) && (packed_x & 0xE00) == 0xE00)
        {
            return i;
        }
    }

    return -1;
}

/**
 * @brief Find the navigation-list index of a given node ID.
 * @param node_id Node ID to search for (typically g_menu_active_node at call sites).
 * @return Zero-based index of @p node_id within the navigation list starting at g_menu_nav_first, or -1 if not found or the list is empty.
 * @see decomp.me (100%)
 */
s32 menu_find_nav_node_index(s32 node_id)
{
    s32* nav;
    s32 i;
    s32 count;

    i = 0;
    if (g_menu_nav_count > 0)
    {
        count = g_menu_nav_count;
        nav = &g_menu_nav_first;
        do
        {
            if (*nav == node_id)
            {
                return i;
            }
            i++;
            nav++;
        } while (i < count);
    }

    return -1;
}

/**
 * @brief Emit up to two scroll-arrow SPRT primitives and a trailing draw-mode reset primitive.
 * @param buf Destination primitive buffer; each arrow occupies 0x14 bytes, the DR_TPAGE tail 8 bytes.
 * @param ot Pointer to the ordering-table entry to prepend each emitted primitive to.
 * @param state Slot view whose scroll fields drive the arrows: x/w place the arrow column, y is the top edge, h the window height, _u._s.unk6 (low 9 bits) the row.
 * @return Pointer to the next free byte in @p buf after all emitted primitives.
 * @see decomp.me (100%)
 */
void* menu_emit_slot_scroll_arrows(SPRT* buf, s32* ot, MenuSlotView* state)
{
    s32 emitted = 0;
    s32 max;
    u8* end;

    if (state->lerp_cur_b != 0)
    {
        SET_BGR0_PACKED(buf, GPU_TINT_NEUTRAL);
        setSprt(buf);
        buf->x0 = state->x + state->w - 0x10;
        buf->y0 = state->y;
        SET_SPRT_UV0_PACKED(buf, 0x1080);
        SET_SPRT_CLUT(buf, 0x7C86);
        SET_SPRT_WH_PACKED(buf, 0x10, 0x10);
        addPrim(ot, buf);
        emitted = 1;
        buf++;
    }

    max = ((state->_u._s.unk6 & 0x1FF) << 4) - state->lerp_cur_b;
    if (((s16)state->h - 0x10) < max)
    {
        SET_BGR0_PACKED(buf, GPU_TINT_NEUTRAL);
        setSprt(buf);
        buf->x0 = state->x + state->w - 0x10;
        buf->y0 = state->y + state->h - 8;
        SET_SPRT_UV0_PACKED(buf, 0x2080);
        SET_SPRT_WH_PACKED(buf, 0x10, 0x10);
        SET_SPRT_CLUT(buf, 0x7C86);
        addPrim(ot, buf);
        emitted += 1;
        buf++;
    }

    end = (u8*)buf;
    if (emitted != 0)
    {
        DR_TPAGE* mode = (DR_TPAGE*)end;
        setDrawTPage(mode, 0, 0, 5);
        addPrim(ot, mode);
        end = (u8*)(mode + 1);
    }

    return end;
}

/**
 * @brief Emit up to two fixed-position scroll-arrow SPRT primitives driven by global scroll state.
 * @param buf Destination primitive buffer; each arrow occupies 0x14 bytes, the Draw Mode tail 8 bytes.
 * @param ot Pointer to the ordering-table entry to prepend each emitted primitive to.
 * @return Pointer to the next free byte in @p buf after all emitted primitives.
 * @see decomp.me (100%)
 */
void* menu_emit_tree_scroll_arrows(SPRT* buf, s32* ot)
{
    u8* end;

    if (g_menu_content_height != 0)
    {
        SET_BGR0_PACKED(buf, GPU_TINT_NEUTRAL);
        setSprt(buf);
        setXY0(buf, 0x20, 3);
        SET_SPRT_UV0_PACKED(buf, 0x1080);
        SET_SPRT_CLUT(buf, 0x7C86);
        SET_SPRT_WH_PACKED(buf, 0x10, 0x10);
        addPrim(ot, buf);
        buf++;
    }

    if ((g_menu_layout_end - g_menu_content_height) >= 0xAC)
    {
        SET_BGR0_PACKED(buf, GPU_TINT_NEUTRAL);
        setSprt(buf);
        setXY0(buf, 0x20, 0xBA);
        SET_SPRT_UV0_PACKED(buf, 0x2080);
        SET_SPRT_CLUT(buf, 0x7C86);
        SET_SPRT_WH_PACKED(buf, 0x10, 0x10);
        addPrim(ot, buf);
        buf++;
    }

    end = (u8*)buf;
    if ((g_menu_content_height != 0) || (g_menu_layout_end >= 0xAC))
    {
        DR_TPAGE* mode = (DR_TPAGE*)end;
        setDrawTPage(mode, 0, 0, 5);
        addPrim(ot, mode);
        end = (u8*)(mode + 1);
    }

    return end;
}

s32 menu_emit_cursor(s32, s32*, s32, s32, s32);

/**
 * @brief Draw the navigation cursor for the active menu node, and optionally its label.
 * @param buf Primitive buffer pointer passed through to the rendering helpers.
 * @param ot Pointer to the ordering-table entry used by the rendering helpers.
 * @param label When non-zero, also draws the node's text label from the g_menu_state_ptr string table.
 * @return Updated primitive buffer pointer returned from the last rendering call.
 * @see decomp.me (100%)
 */
s32 menu_draw_active_node_cursor(s32 buf, s32* ot, s32 label)
{
    u16 nav_x_packed;
    u8 y_hi;
    s32 hi_bit;
    s32 y_base;
    s32 cursor_y;
    s32 scroll;
    u8* base;

    nav_x_packed = g_menu_nodes[g_menu_active_node].idx_nav.nav_x_packed;
    y_hi = g_menu_nodes[g_menu_active_node].u8_u.s.nav_y_hi;

    hi_bit = nav_x_packed >> 15;
    y_base = (y_hi << 1) | hi_bit;
    scroll = g_menu_content_height - 0xC;
    cursor_y = y_base - scroll;
    if (cursor_y < 0xC)
    {
        cursor_y = 0xC;
    }
    if (cursor_y >= 0xA3)
    {
        cursor_y = 0xA3;
    }

    buf = menu_emit_cursor(buf, ot, ((nav_x_packed >> 8) & 0x7F) + 8, cursor_y, 1);

    if (label != 0)
    {
        base = (u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 4);
        buf = func_800A88A0(buf, ot, base + *(u16*)(base + g_menu_nodes[g_menu_active_node].label_id * 2), 1, 0xA0, 0xCA, 2);
    }

    return buf;
}

extern s32 D_80168C6C;
void* menu_emit_sort_marker(void*, s32*, s16, s16);
s32 menu_item_is_nondefault(s32);

/** @brief OR two values without exposing the expression directly to the optimizer. */
inline int menu_or_bits(int arg0, int arg1)
{
    return arg0 | arg1;
}

/** @brief Read a byte through a helper to preserve address materialization. */
inline u8 menu_read_u8(u8* arg0)
{
    return arg0[0];
}

/** @brief Zero-extend a byte through an inline call. */
inline u32 menu_zext_u8(u8 arg0)
{
    return (u32)arg0;
}

/** @brief Add two signed values while preserving operand order. */
inline s32 menu_add_s32(s32 arg0, s32 arg1)
{
    return arg0 + arg1;
}

/** @brief Copy one encoded menu string, including two-byte glyph codes. */
#define MENU_TEXT_COPY(d, s)                                                                                                                                   \
    while (menu_or_bits(ch = *(s), 0))                                                                                                                            \
    {                                                                                                                                                          \
        if (menu_zext_u8(ch - 0x19U) < 7U)                                                                                                                      \
        {                                                                                                                                                      \
            *(d)++ = ch;                                                                                                                                       \
            (s)++;                                                                                                                                             \
            *(d)++ = *(s)++;                                                                                                                                   \
        }                                                                                                                                                      \
        else                                                                                                                                                   \
        {                                                                                                                                                      \
            *(d)++ = ch;                                                                                                                                       \
            (s)++;                                                                                                                                             \
        }                                                                                                                                                      \
    }

/** @brief Address the active character equipment slot used by content subtypes 7-10. */
#define MENU_SLOT_BASE ((u8*)g_pad_ctx + menu_add_s32(((item_sub - 7) << 6), g_menu_char_slot * 0x250))

/** @brief Emit one content label at the fixed cursor-label position. */
#define MENU_EMIT_EXPR(e) arg0 = func_800A88A0(arg0, arg1, (e), 1, 0xA0, 0xCA, 2)

/** @brief Resolve a state-table base from its stored offset. */
#define MENU_STATE_BASE(off) ((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + (off)))

/** @brief Resolve a string-table entry from a base and offset. */
#define MENU_TAIL(base, idx) ((u8*)(base) + *(u16*)((u8*)(base) + (idx)))

/** @brief Emit one indexed state-table label. */
#define MENU_EMIT_STATE(off, idx)                                                                                                                              \
    MENU_EMIT_EXPR(({                                                                                                                                          \
        s32 i = (idx);                                                                                                                                         \
        u8* b = (u8*)g_menu_state_ptr;                                                                                                                         \
        b = b + *(s32*)((u8*)g_menu_state_ptr + (off));                                                                                                        \
        (u8*)b + *(u16*)((u8*)b + i);                                                                                                                          \
    }))

/** @brief Load a state pointer through a pointer-to-pointer. */
inline u8* menu_load_ptr(u8** p)
{
    return *p;
}

/**
 * @brief Render the content cursor, update its lerped position, and optionally render the active hit-item label.
 * @param arg0 Current primitive buffer pointer; reused as the running cursor (the original keeps it in $s0 and copies it from $a0 in the prologue).
 * @param arg1 Pointer to the current ordering-table entry.
 * @param arg2 Non-zero to also render the label string for the active hit item.
 * @return Updated primitive buffer pointer after all emitted primitives.
 * @see decomp.me (100%)
 */
void* menu_draw_content_cursor(void* arg0, s32* arg1, s32 arg2)
{
    u8 sp20[0x40];
    u8 sp60[0x40];
    s32* var_a1;
    s32 var_s1;
    s32 var_t0;
    s32 var_v0_2;
    s16 var_v0;
    MenuContentItem* content_base;
    u16 upper;
    s32 item_sub;
    MenuPrimHead* ot_head;

    if (g_menu_content_ready == 0)
    {
        var_s1 = 0;
        if (menu_item_has_action() != 0)
        {
            var_s1 = (arg2 != 0);
        }
        var_a1 = arg1;
        if (g_menu_suppress_cursor != 0)
        {
            var_a1 = (s32*)((u8*)arg1 - 0x28);
        }
        arg0 = menu_emit_cursor(arg0, var_a1, g_content_cursor_x, g_content_cursor_y, var_s1);
    }

    if (g_menu_suppress_cursor != 0)
    {
        /* Address-taken cursor globals preserve the target load/register order. */
        s32* cxp = &g_content_cursor_x;
        s32 dx = (g_content_view_x - *cxp) / g_menu_suppress_cursor;
        s32* cyp = &g_content_cursor_y;
        s32 dy = (g_content_view_y - *cyp) / g_menu_suppress_cursor;
        g_menu_suppress_cursor -= 1;
        *cxp = dx + *cxp;
        *cyp = dy + *cyp;
    }
    else
    {
        g_content_cursor_x = g_content_view_x;
        g_content_cursor_y = g_content_view_y;
    }

    if (arg2 != 0)
    {
        content_base = g_menu_content_table[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];
        upper = content_base[g_menu_hit_item_idx].packed_x & 0xF000;

        if (upper == 0xF000)
        {
            /* Volatile preserves the target's three independent pad-byte loads. */
            if (*(volatile u8*)content_base[g_menu_hit_item_idx].pad < 0xF0U)
            {
                MENU_EMIT_STATE(0x4, (s32) * (volatile u8*)content_base[g_menu_hit_item_idx].pad * 2);
            }
            else
            {
                item_sub = menu_read_u8(content_base[g_menu_hit_item_idx].pad);
                switch (item_sub)
                {
                case 0xF0:
                    MENU_EMIT_STATE(0x48, (s32)menu_read_u8(content_base[g_menu_hit_item_idx].pad) * 2);
                    break;
                case 0xF1:
                    MENU_EMIT_STATE(0x14, (s32)menu_read_u8(content_base[g_menu_hit_item_idx].pad) * 2);
                    break;
                case 0xF2:
                    MENU_EMIT_STATE(0x34, (s32)menu_read_u8(content_base[g_menu_hit_item_idx].pad) * 2);
                    break;
                case 0xF3:
                    MENU_EMIT_STATE(0x24, (s32)menu_read_u8(content_base[g_menu_hit_item_idx].pad) * 2);
                    break;
                case 0xF4:
                    MENU_EMIT_STATE(0x40, (s32)menu_read_u8(content_base[g_menu_hit_item_idx].pad) * 2);
                    break;
                case 0xF5:
                    MENU_EMIT_STATE(0x58, (s32)menu_read_u8(content_base[g_menu_hit_item_idx].pad) * 2);
                    break;
                case 0xF6:
                    MENU_EMIT_STATE(0x50, (s32)menu_read_u8(content_base[g_menu_hit_item_idx].pad) * 2);
                    break;
                case 0xF7:
                    MENU_EMIT_STATE(0x60, (s32)menu_read_u8(content_base[g_menu_hit_item_idx].pad) * 2);
                    break;
                case 0xF8:
                    if (D_80168C6C != 0xFF)
                    {
                        if (D_80168C6C & 0x80)
                        {
                            MENU_EMIT_STATE(0x40, (s32)menu_read_u8(content_base[g_menu_hit_item_idx].pad) * 2);
                        }
                        else
                        {
                            MENU_EMIT_STATE(0x1C, (s32)menu_read_u8(content_base[g_menu_hit_item_idx].pad) * 2);
                        }
                    }
                    break;
                case 0xF9:
                    MENU_EMIT_STATE(0x1C, (s32)menu_read_u8(content_base[g_menu_hit_item_idx].pad) * 2);
                    break;
                case 0xFA:
                    MENU_EMIT_STATE(0x3C, (s32)menu_read_u8(content_base[g_menu_hit_item_idx].pad) * 2);
                    break;
                case 0xFB:
                case 0xFC:
                case 0xFD:
                case 0xFE:
                    MENU_EMIT_STATE(0x2C, (s32)menu_read_u8(content_base[g_menu_hit_item_idx].pad) * 2);

                }
            }
        }
        else if (upper == 0x5000)
        {
            item_sub = menu_read_u8(content_base[g_menu_hit_item_idx].pad);
            switch (item_sub)
            {
            case 1:
            case 2:
            {
                u8** pp = (u8**)&g_menu_state_ptr;
                u8* pb = (u8*)g_pad_ctx + (g_menu_char_slot * 0x250);
                u8 idx = *(pb + item_sub + 0x609);
                u8* b = menu_load_ptr(pp);
                b += *(s32*)(b + 0x3C);
                MENU_EMIT_EXPR(b + *(u16*)(b + (s32)idx * 2));
                break;
            }
            case 3:
            case 4:
            case 5:
            case 6:
            {
                u8* pad_base = (u8*)g_pad_ctx + (g_menu_char_slot * 0x250);
                u8* flag_addr = pad_base + item_sub;
                u8 flag = *(flag_addr + 0x609);
                if (flag != 0xFF)
                {
                    if (flag & 0x80)
                    {
                        u8* item_ptr = pad_base + (menu_zext_u8(menu_read_u8(flag_addr + 0x609) & 0x7F) << 6) + 0x740;
                        u8 cat = *(item_ptr + 0x24);
                        u8 entry = *(item_ptr + 0x25);
                        u8* state44 = (u8*)g_menu_state_ptr;
                        state44 += *(s32*)(state44 + 0x44);
                        MENU_EMIT_EXPR(state44 + *(u16*)(state44 + menu_zext_u8(cat) * 0x1C + menu_zext_u8(entry) * 2));
                    }
                    else
                    {
                        u32 slot654 = *(u32*)(pad_base + 0x654);
                        u8* state1c = (u8*)g_menu_state_ptr;
                        state1c += *(s32*)(state1c + 0x1C);
                        MENU_EMIT_EXPR(state1c + *(u16*)(state1c + menu_zext_u8(menu_read_u8(flag_addr + 0x609) & 0x7F) * 2 +
                                                        ((slot654 >> 0xA) & 0x3F) * 0x30));
                    }
                }
                break;
            }
            case 7:
            case 8:
            case 9:
            case 10:
                if (g_menu_char_slot < 2)
                {
                    u8** checkpp = (u8**)&g_pad_ctx;
                    u8* checkpad = *checkpp;
                    if (*(checkpad + menu_add_s32(((item_sub - 7) << 6), g_menu_char_slot * 0x250) + 0x640) != 0)
                    {
                        if (menu_item_is_nondefault((s32)(checkpad + ((g_menu_char_slot * 0x250) + 0x5F0) + (((s32)item_sub << 6) - 0x170))) != 0)
                        {
                            s32 idx656 = *(u16*)(MENU_SLOT_BASE + 0x656) & 0x3F;
                            u16 name_off = *(u16*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x30) + idx656 * 2);
                            u8* state30;
                            u8* state8 = (u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x8);
                            u16 surname_off = *(u16*)((u8*)state8 + 0xB4);
                            u8* name = (u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x30) + name_off;
                            u8* surname = (u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x8) + surname_off;
                            u8* out = sp60;
                            { u8 ch; MENU_TEXT_COPY(out, name); }
                            { u8 ch; MENU_TEXT_COPY(out, surname); }
                            *out = 0;
                        }
                        else
                        {
                            sp60[0] = 0;
                        }
                        {
                            u8** padpp = (u8**)&g_pad_ctx;
                            u32 unk654 = *(u32*)((u8*)*padpp + menu_add_s32(((item_sub - 7) << 6), g_menu_char_slot * 0x250) + 0x654);
                            u32 kind = (unk654 >> 8) & 3;
                            switch (kind)
                            {
                            case 0:
                            {
                                u32 idx = (unk654 >> 9) & 0x7E;
                                u8* state68 = (u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x68);
                                u8* str2 = state68 + *(u16*)((u8*)((s32)idx + (s32)state68) + 0);
                                u8 ch;
                                u8* out = sp20;
                                u8* first = sp60;
                                MENU_TEXT_COPY(out, first);
                                MENU_TEXT_COPY(out, str2);
                                *out = 0;
                                break;
                            }
                            case 1:
                            {
                                u32 idx = (unk654 >> 9) & 0x7E;
                                u8* state68 = (u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x68);
                                u8* str2 = state68 + *(u16*)((u8*)((s32)idx + (s32)state68) + 0x16);
                                u8 ch;
                                u8* out = sp20;
                                u8* first = sp60;
                                MENU_TEXT_COPY(out, first);
                                MENU_TEXT_COPY(out, str2);
                                *out = 0;
                                break;
                            }
                            default:
                            {
                                /* Address-taking preserves the target state-table load order. */
                                s32* state68_off = (s32*)((u8*)g_menu_state_ptr + 0x68);
                                /* Keep the item term first to preserve address-expression codegen. */
                                u8** padpp2 = (u8**)&g_pad_ctx;
                                u32 unk654_2 = *(u32*)((u8*)*padpp2 + menu_add_s32(((item_sub - 7) << 6), g_menu_char_slot * 0x250) + 0x654);
                                u32 idx = (unk654_2 >> 9) & 0x7E;
                                u8* state68 = (u8*)g_menu_state_ptr + *state68_off;
                                u8* str2 = state68 + *(u16*)((u8*)((s32)idx + (s32)state68) + 0x2E);
                                u8 ch;
                                u8* out = sp20;
                                u8* first = sp60;
                                MENU_TEXT_COPY(out, first);
                                MENU_TEXT_COPY(out, str2);
                                *out = 0;
                                break;
                            }
                            }
                        }
                        MENU_EMIT_EXPR(sp20);
                    }
                }
                break;
            case 11:
            case 12:
            case 13:
            {
                u8* char_base = (u8*)g_pad_ctx + (g_menu_char_slot * 0x250);
                if (*(char_base + 0x640) != 0)
                {
                    var_v0 = (s32) * (char_base + item_sub + 0x65D) * 2;
                    MENU_EMIT_STATE(0x60, var_v0);
                }
                break;
            }
            case 14:
            {
                u8** pp = (u8**)&g_menu_state_ptr;
                u8* pb = (u8*)g_pad_ctx + (g_menu_char_slot * 0x250);
                u8 idx = *(pb + 0x609);
                u8* b = menu_load_ptr(pp);
                b += *(s32*)(b + 0x48);
                MENU_EMIT_EXPR(b + *(u16*)(b + (s32)idx * 2));
                break;
            }
            case 15:
                if ((u8*)g_menu_item_ptr != NULL)
                {
                    MENU_EMIT_EXPR((u8*)g_menu_item_ptr);
                }
                break;
            case 16:
            case 17:
            case 18:
                if (g_menu_item_ptr != 0)
                {
                    var_v0 = (s32) * ((u8*)g_menu_item_ptr + item_sub + 0x10) * 2;
                    MENU_EMIT_STATE(0x58, var_v0);
                }
                break;
            case 19:
            case 20:
            case 21:
                if (g_menu_item_ptr != 0)
                {
                    var_v0 = (s32) * ((u8*)g_menu_item_ptr + item_sub + 0x15) * 2;
                    MENU_EMIT_STATE(0x60, var_v0);
                }
                break;
            case 22:
                if (g_menu_item_ptr != 0)
                {
                    u8 cat = *((u8*)g_menu_category2_item + 0x24);
                    u8 entry = *((u8*)g_menu_category2_item + 0x25);
                    MENU_EMIT_EXPR((u8*)MENU_STATE_BASE(0x40) + *(u16*)((u8*)MENU_STATE_BASE(0x40) + menu_zext_u8(cat) * 0x1C + menu_zext_u8(entry) * 2));
                }
                break;
            case 23:
                if (g_menu_item_ptr != 0)
                {
                    u8** pp = (u8**)&g_menu_state_ptr;
                    s32* dp = &g_menu_category2_item;
                    u8* b = menu_load_ptr(pp);
                    u8* q = (u8*)*dp;
                    u8 idx = *(q + 0x24);
                    b += *(s32*)(b + 0x24);
                    MENU_EMIT_EXPR(b + *(u16*)(b + (s32)idx * 2));
                }
                break;
            case 24:
                if (g_menu_item_ptr != 0)
                {
                    u8** pp = (u8**)&g_menu_state_ptr;
                    s32* dp = &g_menu_category2_item;
                    u8* b = menu_load_ptr(pp);
                    u8* q = (u8*)*dp;
                    u8 idx = *(q + 0x25);
                    b += *(s32*)(b + 0x34);
                    MENU_EMIT_EXPR(b + *(u16*)(b + (s32)idx * 2));
                }
                break;
            case 25:
            {
                u8** pp = (u8**)&g_menu_state_ptr;
                u8* pb = (u8*)g_pad_ctx + (g_menu_char_slot * 0x250);
                u8 idx = *(pb + 0x633);
                u8* b = menu_load_ptr(pp);
                b += *(s32*)(b + 0x78);
                MENU_EMIT_EXPR(b + *(u16*)(b + (s32)idx * 2));
                break;
            }

            }
        }
    }

    if (g_party_sort_marker.selected_idx != 0xFF)
    {
        arg0 = menu_emit_sort_marker(arg0, arg1, g_party_sort_marker.x, g_party_sort_marker.y);
        ot_head = (MenuPrimHead*)arg0;
        ot_head->_u._s.unk3 = 1;
        ot_head->unk4 = 0xE1000005;
        ot_head->_u.unk0 = (s32)((ot_head->_u.unk0 & 0xFF000000) | (*arg1 & 0xFFFFFF));
        *arg1 = (*arg1 & 0xFF000000) | ((s32)arg0 & 0xFFFFFF);
        arg0 = (u8*)arg0 + 8;
    }

    return arg0;
}

s32 menu_draw_node_recursive(s32, s32, s32*);

/**
 * @brief Render all root menu nodes, then lerp g_menu_content_height toward g_menu_scroll_pos.
 * @param arg0 Current primitive buffer pointer.
 * @param arg1 Pointer to the ordering-table entry used by node-rendering helpers.
 * @return Updated primitive buffer pointer after rendering all active root nodes.
 * @see decomp.me (100%) https://decomp.me/scratch/AIXmd
 */
s32 menu_draw_node_tree(s32 arg0, s32* arg1)
{
    s32 i;
    s32 temp_v0;

    g_menu_nav_count = 0;

    for (i = 0; i < MENU_NODE_COUNT; i++)
    {
        if ((g_menu_nodes[i].u2.s.parent_idx == MENU_NONE) && (g_menu_nodes[i].u2.s.flags & 1))
        {
            arg0 = menu_draw_node_recursive(i, arg0, arg1);
        }
    }

    if ((g_menu_redraw_state == 0) || (g_menu_scroll_pos == g_menu_content_height))
    {
        g_menu_redraw_state = 0;
        g_menu_content_height = g_menu_scroll_pos;
    }
    else
    {
        temp_v0 = (g_menu_scroll_pos - g_menu_content_height) / g_menu_redraw_state;
        g_menu_redraw_state -= 1;
        g_menu_content_height += temp_v0;
    }

    return arg0;
}

void* menu_emit_icon_sprite(void*, s32*, s32, s32, s32, s32, s32, s32, s32);

/**
 * @brief Render one menu node's panel and update its animated Y position; recurse into children.
 * @param arg0 Node index within g_menu_nodes.
 * @param arg1 Current primitive buffer pointer.
 * @param arg2 Pointer to the ordering-table entry.
 * @return Updated primitive buffer pointer after rendering this node and all expanded children.
 * @see decomp.me (100.00%) https://decomp.me/scratch/TNThR
 */
s32 menu_draw_node_recursive(s32 arg0, s32 arg1, s32* arg2)
{
    MenuNode* node;
    MenuNode* new_var7;
    int new_var5;
    int new_var6;
    MenuNode* new_var4;
    s32 buf;
    int new_var2;
    int new_var3;
    int new_var;

    new_var3 = 3;
    *((&g_menu_nav_first) + g_menu_nav_count) = arg0;
    new_var7 = g_menu_nodes;
    node = new_var7 + arg0;
    g_menu_nav_count += 1;
    buf = menu_emit_icon_sprite(arg1, arg2, node->icon_id, ((node->idx_nav.nav_x_packed >> 8) & (new_var5 = 0x7F)) - (-1),
                                ((node->u8_u.s.nav_y_hi << 1) | (new_var6 = node->idx_nav.nav_x_packed >> 15)) - g_menu_content_height, 1,
                                ((node->u2.unk2 >> 2) & 3) != 0, g_menu_scene_type == arg0, (node->u2.unk2 >> 6) & new_var3);

    {
        u16 unk2 = (&node->u2)->unk2;
        u32 anim_cnt = (unk2 >> 2) & 3;
        if (anim_cnt != 0)
        {
            s32 split_tmp;
            new_var6 = anim_cnt - 1;
            new_var2 = new_var6 & 3;
            new_var5 = new_var2 << 2;
            split_tmp = unk2 & 0xFFF3;
            node->u2.unk2 = split_tmp | new_var5;
        }
    }

    if (node->state == 0)
    {
        arg1 = node->u8_u.nav_y_packed & 0x8000;
        ((volatile MenuNode*)node)->idx_nav.nav_x_packed = (node->idx_nav.nav_x_packed & 0x7FFF) | arg1;
        {
            u16 t = node->u8_u.nav_y_packed;
            t &= 0xFF00;
            t |= node->uA.s.layout_y_hi;
            node->u8_u.nav_y_packed = t;
        }
    }
    else
    {
        u16 nav_x_packed = node->idx_nav.nav_x_packed;
        u16 nav_y_packed = node->u8_u.nav_y_packed;
        s32 xpart = nav_x_packed >> 15;
        s32 current_y = ((nav_y_packed & 0xFF) << 1) | xpart;
        u16 target_y = (nav_y_packed >> 15) | (node->uA.s.layout_y_hi << 1);

        if (((u16)current_y) == target_y)
        {
            node->state = 0;
        }
        else
        {
            s32 step = ((s32)target_y - ((u16)current_y)) / ((s32)node->state);
            u32 new_y = current_y + step;
            new_y &= 0xFFFF;
            new_var3 = 15;
            new_var3 = (new_y & 1) << new_var3;
            ((volatile MenuNode*)node)->state -= 1;
            ((volatile MenuNode*)node)->idx_nav.nav_x_packed = (nav_x_packed & 0x7FFF) | new_var3;
            {
                u16 t2 = ((volatile MenuNode*)node)->u8_u.nav_y_packed;
                t2 &= 0xFF00;
                t2 |= (new_y >> 1) & 0xFF;
                node->u8_u.nav_y_packed = t2;
            }
        }
    }

    {
        MenuNode* node2;
        MenuNode* base2 = g_menu_nodes;
        node2 = base2 + arg0;
        if ((node2->u2.unk2 >> 1) & 1)
        {
            s32 j = 0;
            s32 sentinel;
            new_var4 = node2;
            for (; j < 4; j++)
            {
                sentinel = 0xFF;
                if (*((u8*)new_var4 + j + 0xB) == (u8)sentinel)
                {
                    break;
                }
                buf = menu_draw_node_recursive(*((u8*)new_var4 + j + 0xB), buf, arg2);
                sentinel = 0;
            }
        }
    }

    return buf;
}

typedef struct
{
    u8 u_coord; /**< Texture U coordinate. */
    u8 v_coord; /**< Texture V coordinate. */
    u8 w;       /**< Sprite width in pixels. */
    u8 h;       /**< Sprite height in pixels. */
} NodeSpriteInfo;

extern NodeSpriteInfo g_menu_icon_sprite_defs[];
extern u8 g_menu_icon_clut_codes[];

/**
 * @brief Emit the sprite primitives for one menu icon.
 * @param arg0 Primitive-buffer cursor.
 * @param arg1 Ordering-table entry to update.
 * @param arg2 Icon definition index.
 * @param arg3 Base X coordinate.
 * @param arg4 Base Y coordinate.
 * @param arg5 Nonzero to emit the secondary sprite.
 * @param arg6 Pixel offset applied to the sprite position.
 * @param arg7 Nonzero to use the active secondary-sprite mode.
 * @param arg8 Packed node style bits; currently unused.
 * @return Next free primitive-buffer address.
 * @see decomp.me (100%) https://decomp.me/scratch/IXG0l
 */
void* menu_emit_icon_sprite(void* arg0, s32* arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, s32 arg7, s32 arg8)
{
    int new_var;
    u8* p1 = (u8*)arg0;
    (void)arg8;
    *((u32*)(p1 + 0x4)) = 0x808080;
    p1[3] = 4;
    new_var = arg3 - arg5;
    p1[7] = 0x64;
    *((s16*)(p1 + 0x8)) = (s16)(new_var + arg6);
    *((s16*)(p1 + 0xA)) = (s16)((arg4 - arg5) + arg6);
    p1[0xC] = g_menu_icon_sprite_defs[arg2].u_coord;
    p1[0xD] = g_menu_icon_sprite_defs[arg2].v_coord;
    *((s16*)(p1 + 0x10)) = (s16)g_menu_icon_sprite_defs[arg2].w;
    *((s16*)(p1 + 0x12)) = (s16)g_menu_icon_sprite_defs[arg2].h;
    *((s16*)(p1 + 0xE)) = (s16)menu_or_bits(((g_menu_icon_clut_codes[arg2] >> 4) + 0x1F2) << 6, g_menu_icon_clut_codes[arg2] & 0xF);
    *((s32*)p1) = ((*((s32*)p1)) & 0xFF000000) | ((*arg1) & 0xFFFFFF);
    *arg1 = menu_or_bits((*arg1) & 0xFF000000, ((s32)p1) & 0xFFFFFF);
    p1 += 0x14;
    if (arg5 != 0)
    {
        new_var = 0xA00000;
        if (arg7 != 0)
        {
            *((u32*)(p1 + 0x4)) = new_var;
        }
        else
        {
            *((u32*)(p1 + 0x4)) = 0;
        }
        p1[3] = 4;
        p1[7] = 0x64;
        if (arg7 == 0)
        {
            p1[7] = 0x66;
        }
        *((s16*)(p1 + 0x8)) = (s16)(arg3 + (arg5 - arg6) * 2);
        *((s16*)(p1 + 0xA)) = (s16)(arg4 + (arg5 - arg6) * 2);
        p1[0xC] = g_menu_icon_sprite_defs[arg2].u_coord;
        p1[0xD] = g_menu_icon_sprite_defs[arg2].v_coord;
        *((s16*)(p1 + 0x10)) = (s16)g_menu_icon_sprite_defs[arg2].w;
        *((s16*)(p1 + 0x12)) = (s16)g_menu_icon_sprite_defs[arg2].h;
        *((s16*)(p1 + 0xE)) = (s16)menu_or_bits(((g_menu_icon_clut_codes[arg2] >> 4) + 0x1F2) << 6, g_menu_icon_clut_codes[arg2] & 0xF);
        *((s32*)p1) = ((*((s32*)p1)) & 0xFF000000) | ((*arg1) & 0xFFFFFF);
        *arg1 = menu_or_bits((*arg1) & 0xFF000000, ((s32)p1) & 0xFFFFFF);
        p1 += 0x14;
    }
    return p1;
}

/**
 * @brief Emit a single 16x16 gray SPRT primitive and OT-link it.
 * @param prim_buf Current primitive buffer pointer; must have at least 0x14 bytes of space.
 * @param ot Pointer to the ordering-table entry; updated to prepend this SPRT.
 * @param x X screen position of the sprite.
 * @param y Y screen position of the sprite.
 * @return Pointer to the next free byte after the emitted 0x14-byte SPRT.
 * @see decomp.me (100%) https://decomp.me/scratch/16UQc
 */
void* menu_emit_sort_marker(void* prim_buf, s32* ot, s16 x, s16 y)
{
    u8* p = (u8*)prim_buf;

    *(u32*)(p + 0x4) = 0x505050;
    p[3] = 4;
    *(u32*)(p + 0x10) = 0x100010;
    p[7] = 0x64;
    *(s16*)(p + 0xC) = 0x80;
    *(s16*)(p + 0x8) = x;
    *(s16*)(p + 0xA) = y;
    *(s16*)(p + 0xE) = 0x7C86;
    *(s32*)p = (*(s32*)p & (s32)0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & (s32)0xFF000000) | ((s32)p & 0xFFFFFF);
    return p + 0x14;
}

/** @brief State block for a scrollable circular list widget. */
typedef struct
{
    u8 unk0; /* 0x00 - set to 3 to request a state change */
    u8 pad01;
    u8 unk2; /* 0x02 - cleared when the page opens a sub-window */
    u8 pad03;
    u16 sel_idx;    /* 0x04 - currently selected item index (the page code's "unk4") */
    u16 item_count; /* 0x06 - total items; lower 9 bits active (& 0x1FF) */
    u16 base_x;     /* 0x08 - widget screen base x */
    u16 base_y;     /* 0x0A - widget screen base y */
    s16 viewport_w; /* 0x0C - visible list width */
    s16 viewport_h; /* 0x0E - visible list height; (viewport_h - 16) >> 4 = fast-scroll step */
    u16 scroll_x;   /* 0x10 - current applied x scroll offset */
    u16 scroll_y;   /* 0x12 - current applied y scroll offset */
    s16 target_x;   /* 0x14 - x scroll lerp target (set by scroll_list_update_target) */
    s16 target_y;   /* 0x16 - y scroll lerp target (set by scroll_list_update_target) */
    u8 lerp_steps;  /* 0x18 - remaining lerp steps; always reset to 4 */
} ScrollListState;

void scroll_list_update_target(ScrollListState*, u32*);

/**
 * @brief Process shoulder/D-pad scroll input for a list widget and draw its animated cursor.
 * @param prim_buf Primitive buffer write cursor; forwarded to menu_emit_cursor.
 * @param ot Ordering-table pointer; forwarded to menu_emit_cursor.
 * @param state Scroll-list state block.
 * @param entries Packed circular linked-list entry array (g_menu_scroll_nav_entries).
 * @param view_origin Viewport anchor in list-local coordinates.
 * @param active Non-zero to process input this frame; zero draws cursor only.
 * @return The advanced primitive write cursor (the value menu_emit_cursor returns).
 * @see decomp.me (100%) https://decomp.me/scratch/tfyt3
 */
s32 scroll_list_draw(s32 prim_buf, s32* ot, ScrollListState* state, u32* entries, Vec2s* view_origin, int active)
{
    int count;
    s32 cursor_x;
    s32 cursor_y;
    if (active)
    {
        if (g_pad_input & PADR1)
        {
            g_pad_input = PADLdown;
            count = (state->viewport_h - 16) >> 4;
        }
        else if (g_pad_input & PADL1)
        {
            g_pad_input = PADLup;
            count = (state->viewport_h - 16) >> 4;
        }
        else
        {
            count = 1;
        }
        while (count != 0)
        {
            if (g_pad_input & MENU_PAD_CONFIRM_CANCEL)
            {
                if (g_pad_input & PADLup)
                {
                    state->sel_idx = (entries[state->sel_idx] >> 14) & 0x1FF;
                }
                else
                {
                    state->sel_idx = entries[state->sel_idx] >> 23;
                }
                scroll_list_update_target(state, entries);
                if (state->sel_idx == ((state->item_count & 0x1FF) - 1))
                {
                    count = 1;
                }
                if (state->sel_idx == 0)
                {
                    count = 1;
                }
            }
            count--;
        }

        if (g_pad_input & MENU_PAD_CONFIRM_CANCEL)
        {
            menu_play_se(MENU_SE_NAVIGATE, MENU_SE_VOLUME);
        }
        if (g_pad_input & PADLleft)
        {
            g_pad_input |= PAD_BTN_CIRCLE;
        }
    }
    prim_buf =
        menu_emit_cursor(prim_buf, ot, (4 - view_origin->x) - state->scroll_x, ((entries[state->sel_idx] & 0x3FFF) - view_origin->y) - state->scroll_y, active);
    g_menu_default_view_pos.x = (state->base_x + ((4 - (view_origin->x & 0xFFFFFFFF)) - state->scroll_x)) + 8;
    g_menu_default_view_pos.y = (state->base_y + (((entries[state->sel_idx] & 0x3FFF) - view_origin->y) - state->scroll_y)) + 8;
    return prim_buf;
}

/**
 * @brief Recompute the scroll lerp targets so the selected list item is inside the viewport.
 * @param state Scroll-list state block to update.
 * @param entries Packed circular linked-list entry array; bits [13:0] hold the item y position.
 * @see decomp.me (100%)
 */
void scroll_list_update_target(ScrollListState* state, u32* entries)
{
    s32 item_y;

    if (4 - state->scroll_x > state->viewport_w - 0x20)
    {
        state->target_x = -0x1C - state->viewport_w;
    }
    if (4 - state->scroll_x < 0)
    {
        state->target_x = 4;
    }

    if ((s32)((entries[state->sel_idx] & 0x3FFF) - state->scroll_y) > state->viewport_h - 0x20)
    {
        state->target_y = (entries[state->sel_idx] & 0x3FFF) - state->viewport_h + 0x20;
    }

    item_y = entries[state->sel_idx] & 0x3FFF;
    if (item_y - state->scroll_y < 0)
    {
        state->target_y = item_y;
    }
    state->lerp_steps = 4;
}

/** @brief Three-frame cursor icon-id sequence (0x6B, 0x6C, 0x6D). */
extern u8 g_menu_cursor_icon_ids[];

/**
 * @brief Emit the animated menu cursor: one or two SPRTs plus a texpage prim, OT-linked.
 * @param prim Primitive write cursor; the SPRTs are built here.
 * @param ot Ordering-table entry; updated after each emitted primitive.
 * @param x Cursor screen X before the bob offset is applied.
 * @param y Cursor screen Y before the bob offset is applied.
 * @param active Non-zero to animate and to emit the second, semi-transparent (0x66) SPRT.
 * @return Pointer to the next free primitive slot (past the 8-byte texpage prim).
 * @see decomp.me (100%)
 */
s32 menu_emit_cursor(s32 prim, s32* ot, s32 x, s32 y, s32 active)
{
    SPRT* p = (SPRT*)prim;
    DR_TPAGE* tp;
    u8* id;
    NodeSpriteInfo* spr;
    u8* clut;
    s32 phase;

    if (active == 0 || g_menu_frame < 9)
    {
        phase = 0;
    }
    else
    {
        if (g_menu_frame < 0x10)
        {
            phase = 1;
        }
        else if (g_menu_frame < 0x17)
        {
            phase = 2;
        }
        else if (g_menu_frame < 0x1E)
        {
            phase = 1;
        }
        else
        {
            phase = 0;
            g_menu_frame = 0;
        }
    }

    setlen(p, 4);
    spr = g_menu_icon_sprite_defs;
    id = &g_menu_cursor_icon_ids[phase];
    clut = g_menu_icon_clut_codes;

    SET_BGR0_PACKED(p, GPU_TINT_NEUTRAL);
    setcode(p, 0x64);
    setXY0(p, x - phase, y + phase);
    setUV0(p, spr[*id].u_coord, spr[*id].v_coord);
    setWH(p, spr[*id].w, spr[*id].h);
    SET_SPRT_CLUT(p, menu_or_bits(((clut[*id] >> 4) + 0x1F2) << 6, clut[*id] & 0xF));
    addPrim(ot, p);
    p++;

    if (active != 0)
    {
        SET_BGR0_PACKED(p, 0);
        setcode(p, 0x66);
        setlen(p, 4);
        setXY0(p, (x - phase) + 2, (y + phase) + 2);
        setUV0(p, spr[*id].u_coord, spr[*id].v_coord);
        setWH(p, spr[*id].w, spr[*id].h);
        SET_SPRT_CLUT(p, menu_or_bits(((clut[*id] >> 4) + 0x1F2) << 6, clut[*id] & 0xF));
        addPrim(ot, p);
        p++;
    }

    tp = (DR_TPAGE*)p;
    setDrawTPage(tp, 0, 0, 5);
    addPrim(ot, tp);
    return (s32)(tp + 1);
}

/* Packed word at ScrollListState+0x04: low 16 = sel_idx, bits 16-24 = item_count. */
#define LIST_WORD(st) (*(u32*)((u8*)(st) + 4))
/* Packed fields of a 0x40-byte pad-context item record. */
#define PAD_ITEM_W14(p) (*(u32*)((u8*)(p) + 0x14))
#define PAD_ITEM_W16(p) (*(u16*)((u8*)(p) + 0x16))

extern u8 D_8016869F[];
extern u8 D_801686A0[];
/** @brief Slot-occupied flags indexed by equipment subtype; g_item_slot_flags is the subtype-7 view. */
extern u8 g_item_slot_flags_by_subtype[];
extern u8 g_menu_item_description_buffer[];
extern s32 g_menu_inventory_index;
extern s32 g_menu_active_item_category;
/** @brief Slot-data pointers indexed by equipment subtype; g_item_slot_data is the subtype-7 view. */
extern s32 g_item_slot_data_by_subtype[];
/** @brief Selected equipment row awaiting a swap, or MENU_NONE. */
extern s32 g_menu_pending_item_row;

s32 menu_item_followup_callback(s32* ot, ScrollListState* state, s32 prim_buf, Vec2s* view_origin, int active);
s32 menu_equipment_compare_callback(s32* ot, ScrollListState* state, s32 prim_buf, Vec2s* view_origin, int active);
/* K&R declarations preserve the original call-site register behavior. */
void func_800A8F8C();
void func_800A8FB4();
s32 func_800A9060();
s32 menu_special_technique_list_callback(s32* ot, ScrollListState* state, s32 prim_buf, Vec2s* view_origin, s32 active);
void menu_swap_item_records(s32, s32);

/**
 * @brief Draw the equip/ability list for the active character and handle its input.
 * @param ot Ordering table the primitives are linked into.
 * @param st Scroll-list state for this window (selection, scroll, viewport).
 * @param prim_buf Primitive write cursor.
 * @param view_origin Viewport anchor in list-local coordinates.
 * @param active Non-zero when this window owns input this frame.
 * @return The advanced primitive write cursor.
 * @see decomp.me (100%)
 */
#define MENU_CLEAR_SLOTS() \
{ \
    s32 _i; \
    for (_i = 3; _i >= 0; _i--) \
    { \
        g_menu_slots[_i].active = 0; \
    } \
}

#define MENU_RELINK() \
{ \
    s32 _j; \
    s32 _prev; \
    s32 _next; \
    s32 _more; \
    s32 _link; \
    s32 _word_prev; \
    _j = 0; \
    do \
    { \
        s32 _cur = g_menu_item_nav_entries[_j]; \
        s32 _word_self; \
        _prev = 1; \
        _link = _cur & ~0x3FFF; \
        _link = _link | ((_j * 0x10) & 0x3FFF); \
        _word_self = _link; \
        g_menu_item_nav_entries[_j] = _word_self; \
        if ((_j - 1) >= 0) \
        { \
            _prev = _j - 1; \
        } \
        _word_prev = _word_self & 0xFF803FFF; \
        _word_prev = _word_prev | ((_prev & 0x1FF) << 14); \
        g_menu_item_nav_entries[_j] = _word_prev; \
        _next = _j + 1; \
        _more = _next < 2; \
        _link = 0; \
        if (_more != 0) \
        { \
            _link = _next; \
        } \
        g_menu_item_nav_entries[_j] = (_word_prev & 0x7FFFFF) | (_link << 23); \
        _j = _next; \
    } while (_more != 0); \
}

void* menu_inventory_list_callback(s32* ot, ScrollListState* st, s32 prim_buf, Vec2s* view_origin, s32 active)
{
    ScrollListState* list;
    void* buf;
    s32 scroll_y;
    void* hit_slot;
    s32 idx;
    s32 y;
    u8* item;
    u8 ch;
    MenuSlotRect rect;
    u8 sp30[0x40];

    list = st;
    buf = (void*)prim_buf;

    if ((g_pad_input & 0x10) && (active != 0))
    {
        menu_play_se(0x7D, 0x80);
        switch (g_menu_active_item_category)
        {
        case 0:
        {
            s32 v = g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx;
            switch (v)
            {
            case 0x21:
            case 0x22:
            case 0x23:
                g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx = v + 1;
                break;
            case 0x13:
            case 0x24:
                g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx = 0x21;
                break;
            }
            g_menu_nodes[g_menu_scene_type].label_id = D_8016869F[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];
            break;
        }
        case 1:
        {
            s32 v = g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx;
            switch (v)
            {
            case 0x25:
            case 0x26:
                g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx = v + 1;
                break;
            case 0x16:
            case 0x27:
                g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx = 0x25;
                break;
            }
            g_menu_nodes[g_menu_scene_type].label_id = D_8016869F[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];
            break;
        }
        case 2:
        {
            u8 v = g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx;
            switch (v)
            {
            case 0x28:
                g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx = 0x29;
                break;
            case 0x29:
            case 0x19:
                g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx = 0x28;
                break;
            }
            g_menu_nodes[g_menu_scene_type].label_id = D_801686A0[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];
            break;
        }
        }
    }

    buf = scroll_list_draw((s32)buf, ot, list, g_menu_scroll_nav_entries, view_origin, active);

    if (g_menu_scene_type < 0x13 && (g_pad_input & 0x8000))
    {
        g_pad_input &= ~0x40;
    }

    if ((g_pad_input & 0x40) && (active != 0))
    {
        menu_play_se(0x7F, 0x80);
        if (g_menu_scene_type < 0x13)
        {
            s32 cat;
            g_menu_content_ready = 0;
            cat = g_menu_active_item_category;
            if (cat == 2)
            {
                g_menu_nodes[(g_menu_char_slot * 3) + 2].idx_nav.s.self_idx = (s8)cat;
                g_menu_nodes[(g_menu_char_slot * 3) + 2].label_id = (u8)((g_menu_char_slot * 3) + 2);
                MENU_CLEAR_SLOTS();
                return buf;
            }
            g_menu_nodes[(g_menu_char_slot * 3) + 1].idx_nav.s.self_idx = 1;
            g_menu_nodes[(g_menu_char_slot * 3) + 1].label_id = (u8)((g_menu_char_slot * 3) + 3);
            MENU_CLEAR_SLOTS();
            return buf;
        }
        if (g_menu_pending_item_row != 0xFF)
        {
            g_menu_pending_item_row = 0xFF;
        }
        else
        {
            g_menu_nodes[0x13].label_id = 0x11;
            g_menu_nodes[0x16].label_id = 0x14;
            g_menu_nodes[0x19].label_id = 0x16;
            g_menu_content_ready = 0;
            menu_play_se(0x7F, 0x80);
            menu_reset_content_view();
            g_pad_input = 0;
        }
    }

    idx = 0;
    y = idx;
    scroll_y = list->scroll_y;
    g_menu_item_ptr = 0;
    g_menu_category0_item = 0;
    g_menu_category1_item = 0;
    g_menu_category2_item = 0;
    item = (u8*)g_pad_ctx + 0xCE0;

    do
    {
        u32 word;
        s32 cat;
        s32 icon;

        if (*item == 0)
        {
            break;
        }
        word = PAD_ITEM_W14(item);
        cat = (word >> 8) & 3;
        icon = 0x45;

        if (cat == g_menu_active_item_category)
        {
            switch (cat)
            {
            case 0:
                icon = ((word >> 0xA) & 0x3F) + 0x45;
                break;
            case 1:
                icon = ((word >> 0xA) & 0x3F) + 0x50;
                break;
            case 2:
                icon = ((word >> 0xA) & 0x3F) + 0x5C;
                break;
            }

            if ((y - scroll_y) >= -0xF && (y - scroll_y) < (list->viewport_h - 0x10))
            {
                u32 w2;
                u8* tbl;
                s32 pal;

                buf = (void*)menu_emit_icon_sprite(buf, ot, icon, 0x10 - view_origin->x, (y - scroll_y) - view_origin->y, 0, 0, 0, 0);
                setlen((DR_TPAGE*)buf, 1);
                ((DR_TPAGE*)buf)->code[0] = 0xE1000005;
                addPrim(ot, (DR_TPAGE*)buf);
                buf = (DR_TPAGE*)buf + 1;

                w2 = PAD_ITEM_W14(item);
                if (w2 & 0x300)
                {
                    tbl = &D_800F0BEC[(w2 >> 0xA) & 0x3F];
                }
                else
                {
                    tbl = &D_800F0BE0[(w2 >> 0xA) & 0x3F];
                }
                pal = 1;
                { u32 entry=*(volatile u8*)tbl; u32 mask; do { mask=(u8)g_menu_ability_mask; } while(0); mask=entry & mask; if(mask){pal=3;} }
                buf = (void*)func_800A88A0(buf, ot, item, pal, 0x22 - view_origin->x, (y - scroll_y) - view_origin->y, 0);

                if (g_menu_pending_item_row != 0xFF)
                {
                    if ((y >> 4) == g_menu_pending_item_row)
                    {
                        s32 view_x = view_origin->x;
                        s32 view_y = view_origin->y;
                        SET_BGR0_PACKED((SPRT*)buf, 0x505050);
                        setlen((SPRT*)buf, 4);
                        setcode((SPRT*)buf, 0x64);
                        SET_SPRT_UV0_PACKED((SPRT*)buf, 0x80);
                        SET_SPRT_CLUT((SPRT*)buf, 0x7C86);
                        SET_SPRT_WH_PACKED((SPRT*)buf, 0x10, 0x10);
                        SET_YX0((SPRT*)buf, (y - scroll_y) - view_y, -2 - view_x);
                        addPrim(ot, (SPRT*)buf);
                        buf = (SPRT*)buf + 1;
                        setlen((DR_TPAGE*)buf, 1);
                        ((DR_TPAGE*)buf)->code[0] = 0xE1000005;
                        addPrim(ot, (DR_TPAGE*)buf);
                        buf = (DR_TPAGE*)buf + 1;
                    }
                }
            }

            if (g_menu_pending_item_row != 0xFF && (y >> 4) == g_menu_pending_item_row)
            {
                hit_slot = (u8*)g_pad_ctx + ((idx << 6) + 0xCE0);
            }

            if ((y >> 4) == list->sel_idx)
            {
                { u8* shadow_pad = (u8*)g_pad_ctx; }
                g_menu_inventory_index = idx;
                g_menu_item_ptr = (s32)((u8*)g_pad_ctx + ((idx << 6) + 0xCE0));
                switch (g_menu_active_item_category)
                {
                case 0:
                    g_menu_category0_item = g_menu_item_ptr;
                    break;
                case 1:
                    g_menu_category1_item = g_menu_item_ptr;
                    break;
                case 2:
                    g_menu_category2_item = g_menu_item_ptr;
                    break;
                }
            }
            y += 0x10;
        }

        idx += 1;
        item += 0x40;
    } while (idx < 0x64);

    if ((y - scroll_y) <= 0 && list->lerp_steps == 0)
    {
        u16 cur = list->scroll_y;
        if (cur != 0)
        {
            list->target_y = cur - 0x10;
            list->lerp_steps = 4;
        }
    }

    LIST_WORD(list) = (LIST_WORD(list) & 0xFE00FFFF) | ((menu_build_inventory_nav_entries(g_menu_active_item_category) & 0x1FF) << 0x10);
    g_menu_page_count = (LIST_WORD(list) >> 0x10) & 0x1FF;
    g_script_repeat_last = list->sel_idx;

    if (LIST_WORD(list) & 0x01FF0000)
    {
        if ((u32)((LIST_WORD(list) >> 0x10) & 0x1FF) <= (u32)list->sel_idx)
        {
            menu_step_item_selection(-1);
            list->sel_idx = (u16)((list->item_count & 0x1FF) - 1);
            if (((list->sel_idx * 0x10) - scroll_y) < (list->viewport_h - 0x10))
            {
                u16 cur = list->scroll_y;
                if (cur != 0)
                {
                    list->target_y = cur - 0x10;
                    list->lerp_steps = 4;
                }
            }
        }
        g_menu_page_count = list->item_count & 0x1FF;
        g_script_repeat_last = list->sel_idx;

        if (g_menu_item_ptr != 0)
        {
            if (active != 0)
            {
                if (menu_item_is_nondefault(g_menu_item_ptr) != 0)
                {
                    u8* state = g_menu_state_ptr;
                    u16 item16 = PAD_ITEM_W16(g_menu_item_ptr);
                    s32 name_idx = (item16 & 0x3F) * 2;
                    s32 o30 = *(s32*)(state + 0x30);
                    s32 o04 = *(s32*)(state + 0x04);
                    u8* state30 = state + o30;
                    u8* name = state30 + *(u16*)(u8*)((s32)name_idx + (s32)state30);
                    u8* state04 = state + o04;
                    u8* suffix = state04 + *(u16*)(state04 + 0xDA);
                    u8* out = sp30;
                    { u8 ch; MENU_TEXT_COPY(out, name); }
                    { u8 ch; MENU_TEXT_COPY(out, suffix); }
                    *out = 0;
                } else { sp30[0] = 0; }

                {
                    u32 item_word = PAD_ITEM_W14(g_menu_item_ptr);
                    u32 kind = (item_word >> 8) & 3;
                    switch (kind)
                    {
                    case 0:
                    {
                        u32 str_idx = (item_word >> 9) & 0x7E;
                        u8* state68 = g_menu_state_ptr + *(s32*)(g_menu_state_ptr + 0x68);
                        u8* str2 = state68 + *(u16*)((u8*)((s32)str_idx + (s32)state68) + 0);
                        u8 ch;
                        u8* out = g_menu_item_description_buffer;
                        u8* first = sp30;
                        MENU_TEXT_COPY(out, first);
                        MENU_TEXT_COPY(out, str2);
                        *out = 0;
                        break;
                    }
                    case 1:
                    {
                        u32 str_idx = (item_word >> 9) & 0x7E;
                        u8* state68 = g_menu_state_ptr + *(s32*)(g_menu_state_ptr + 0x68);
                        u8* str2 = state68 + *(u16*)((u8*)((s32)str_idx + (s32)state68) + 0x16);
                        u8 ch;
                        u8* out = g_menu_item_description_buffer;
                        u8* first = sp30;
                        MENU_TEXT_COPY(out, first);
                        MENU_TEXT_COPY(out, str2);
                        *out = 0;
                        break;
                    }
                    default:
                    {
                        s32* state68_off = (s32*)(g_menu_state_ptr + 0x68);
                        u32 item_word2 = PAD_ITEM_W14(g_menu_item_ptr);
                        u32 str_idx = (item_word2 >> 9) & 0x7E;
                        u8* state68 = g_menu_state_ptr + *state68_off;
                        u8* str2 = state68 + *(u16*)((u8*)((s32)str_idx + (s32)state68) + 0x2E);
                        u8 ch;
                        u8* out = g_menu_item_description_buffer;
                        u8* first = sp30;
                        MENU_TEXT_COPY(out, first);
                        MENU_TEXT_COPY(out, str2);
                        *out = 0;
                        break;
                    }
                    }
                    g_menu_help_text = (s32)g_menu_item_description_buffer;
                }
            }
        }

        if (g_pad_input & 0x220)
        {
            if (active != 0)
            {
                if (g_menu_scene_type < 0x13)
                {
                    s32 sub = g_menu_active_item_category;

                    if (sub == 2)
                    {
                        menu_swap_item_records(g_menu_item_ptr, g_menu_active_equipped_item);
                        func_800A8FB4();
                        {
                            s32 char_slot = g_menu_char_slot;
                            u8* ctx = (u8*)g_pad_ctx + (char_slot * 0x250);
                            ctx += g_menu_active_subtype;
                            ctx[0x609] = (s8)((u8)g_menu_active_subtype + 0x7D);
                        }
                        menu_play_se(0x7E, 0x80);
                        g_menu_nodes[(g_menu_char_slot * 3) + 2].idx_nav.s.self_idx = (s8)sub;
                        g_menu_nodes[(g_menu_char_slot * 3) + 2].label_id = (u8)((g_menu_char_slot * 3) + 2);
                        MENU_CLEAR_SLOTS();
                        g_menu_content_ready = 0;
                    }
                    else
                    {
                        u32 w = PAD_ITEM_W14(g_menu_item_ptr);
                        u8* tbl;
                        if (w & 0x300)
                        {
                            w >>= 0xA;
                            w &= 0x3F;
                            tbl = D_800F0BEC;
                        }
                        else
                        {
                            w >>= 0xA;
                            w &= 0x3F;
                            tbl = D_800F0BE0;
                        }
                        w += (u32)tbl;
                        {
                            u8 entry;
                            u32 mask;
                            do { entry = *(volatile u8*)w; } while (0);
                            mask = (u8)g_menu_ability_mask;
                            mask = entry & mask;
                            if (mask) { menu_play_se(0x78, 0x80); return buf; }
                        }
                        menu_play_se(0x7E, 0x80);
                        MENU_CLEAR_SLOTS();
                        if (menu_item_is_nondefault(g_menu_active_equipped_item) != 0)
                        {
                            menu_swap_item_records(g_menu_item_ptr, g_menu_active_equipped_item);
                            g_item_slot_data_by_subtype[g_menu_active_subtype] = g_menu_item_ptr;
                            g_item_slot_flags_by_subtype[g_menu_active_subtype] = 1;
                        }
                        else
                        {
                            func_800A8F8C(g_menu_active_equipped_item, g_menu_item_ptr);
                            *(u8*)g_menu_item_ptr = 0;
                            g_item_slot_data_by_subtype[g_menu_active_subtype] = 0;
                            g_item_slot_flags_by_subtype[g_menu_active_subtype] = 1;
                        }
                        MENU_CLEAR_SLOTS();
                        rect.x = 0xB0;
                        rect.y = 0x40;
                        rect.w = 0x70;
                        rect.h = 0x30;
                        list = (ScrollListState*)menu_slot_alloc(0, &rect);
                        ((MenuSlot*)list)->content_cb = (s32 * (*)()) & menu_equipment_compare_callback;
                        g_menu_compare_window_active = 1;
                        ((MenuSlot*)list)->flags = (((MenuSlot*)list)->flags & 0xFE00FFFF) | 0x20000;
                        MENU_RELINK();
                        g_menu_content_ready = 0;
                        g_menu_nodes[(g_menu_char_slot * 3) + 1].idx_nav.s.self_idx = 1;
                        g_menu_nodes[(g_menu_char_slot * 3) + 1].label_id = (u8)((g_menu_char_slot * 3) + 3);
                        return buf;
                    }
                }
                else
                {
                    u16 sel;

                    g_menu_nodes[0x13].label_id = 0x11;
                    g_menu_nodes[0x16].label_id = 0x14;
                    g_menu_nodes[0x19].label_id = 0x16;
                    menu_play_se(0x7D, 0x80);
                    sel = list->sel_idx;
                    if (sel != g_menu_pending_item_row)
                    {
                        if (g_menu_pending_item_row != 0xFF)
                        {
                            menu_swap_item_records((s32)hit_slot, g_menu_item_ptr);
                            g_menu_pending_item_row = 0xFF;
                        }
                        else
                        {
                            g_menu_pending_item_row = sel;
                        }
                    }
                    else
                    {
                        rect.x = 0xB0;
                        rect.y = 0x60;
                        rect.w = 0x70;
                        rect.h = 0x30;
                        list = (ScrollListState*)menu_slot_alloc(0, &rect);
                        ((MenuSlot*)list)->content_cb = (s32 * (*)()) & menu_item_followup_callback;
                        ((MenuSlot*)list)->flags = (((MenuSlot*)list)->flags & 0xFE00FFFF) | 0x20000;
                        MENU_RELINK();
                    }
                }
            }
        }
    }
    return buf;
}

/**
 * @brief Clear the four "pending" character-status bytes for the active character slot.
 * @return 1 if at least one byte was reset, 0 if none were.
 * @see decomp.me (100%)
 */
s32 menu_clear_pending_status(void)
{
    s32 changed;
    s32 i;

    changed = 0;
    for (i = 0; i < 4; i++)
    {
        u8* p = (u8*)g_pad_ctx + (g_menu_char_slot * 0x250) + i;

        if (p[0x60C] != 0xFF)
        {
            if ((p[0x60C] & 0x80) == 0)
            {
                p[0x60C] = 0xFF;
                changed = 1;
            }
        }
    }
    return changed;
}

/** @brief One 0x40-byte menu item entry in the item table at g_pad_ctx + 0xCE0. */
typedef struct
{
    u8 flag; /* 0x00: 0 = empty slot */
    u8 pad01[0x13];
    u32 attr; /* 0x14: bits [9:8] select the item kind */
    u8 pad18[0x28];
} MenuItemEntry;

/**
 * @brief Scan the item table for the next entry whose kind matches g_menu_active_item_category.
 * @param step Direction/stride to walk the table (1 = forward, -1 = backward).
 * @return Selected item-table index.
 * @see decomp.me (100%)
 */
u32 menu_step_item_selection(s32 step)
{
    u32 start;
    u32 index;
    s32 kind;
    u8* item;
    u32 result;
    u8* base;

    g_menu_item_ptr = 0;
    base = (u8*)g_pad_ctx;
    start = g_menu_inventory_index + step;
    item = base + ((start << 6) + 0xCE0);
    index = start;
    while (index < 0x64)
    {
        if (item[0] != 0)
        {
            kind = ((*(u32*)(item + 0x14)) >> 8) & 3;
            if (kind == g_menu_active_item_category)
            {
                g_menu_inventory_index = index;
                g_menu_item_ptr = (s32)((u8*)g_pad_ctx + ((index << 6) + 0xCE0));
                switch (kind)
                {
                case 0:
                    g_menu_category0_item = g_menu_item_ptr;
                    break;
                case 1:
                    g_menu_category1_item = g_menu_item_ptr;
                    break;
                case 2:
                    g_menu_category2_item = g_menu_item_ptr;
                    break;
                }
                return index;
            }
        }
        index += step;
        item += step << 6;
    }

    result = g_menu_item_ptr;
    if (result == 0)
    {
        if (step == 1)
        {
            g_menu_inventory_index = -1;
        }
        else
        {
            g_menu_inventory_index = 0x64;
        }
        result = menu_step_item_selection(step);
    }
    return result;
}

s32 func_800A88A0(s32 prim, s32* ot, void* glyph, s32 a3, s32 x, s32 y, s32 mode);
void menu_play_se(s32 sound_id, s32 volume);

/**
 * @brief Draw the character's spell/ability grid and handle its selection input.
 * @param ot Ordering-table pointer, forwarded to the glyph renderer.
 * @param state Scroll-list state for this grid.
 * @param prim_buf Primitive buffer write cursor.
 * @param view_origin Viewport anchor; the glyph origin is (0x10 - x, rel_y - y).
 * @param active Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor.
 * @see decomp.me (100%)
 */
s32 menu_spell_list_callback(s32* ot, ScrollListState* state, s32 prim_buf, Vec2s* view_origin, int active)
{
    s32 sel;
    s32 row;
    s32 col;
    s32 bit;
    s32 y;
    s32 rel_y;
    u8* mask;
    u32 scroll_y;
    void* base;
    void* sel_base;
    ScrollListState* list;

    list = state;

    if ((g_pad_input & 0x40) && (active != 0))
    {
        menu_play_se(0x7F, 0x80);
        list->unk0 = 3;
        g_pad_input = 0;
    }

    prim_buf = scroll_list_draw(prim_buf, ot, list, g_menu_scroll_nav_entries, view_origin, active);

    y = 0;
    sel = -1;
    row = 0;
    mask = (u8*)g_pad_ctx + 0x60;
    scroll_y = list->scroll_y;

    do
    {
        col = 0;
        bit = 1;
        do
        {
            if (*mask & bit)
            {
                rel_y = y - scroll_y;
                if ((rel_y >= -0xF) && (rel_y < (list->viewport_h - 0x10)))
                {
                    base = (void*)(g_menu_state_ptr + *(s32*)(g_menu_state_ptr + 0x10));
                    prim_buf = func_800A88A0(prim_buf, ot, (void*)((u8*)base + *(u16*)((u8*)base + (col * 2) + (row * 0x10))), 1, 0x10 - view_origin->x,
                                             rel_y - view_origin->y, 0);
                }
                if (list->sel_idx == (y >> 4))
                {
                    sel = col + (row * 8);
                }
                y += 0x10;
            }
            col += 1;
            bit *= 2;
        } while (col < 8);
        row += 1;
        mask += 1;
    } while (row < 0xC);

    if ((g_pad_input & 0x220) && (active != 0))
    {
        *((u8*)g_pad_ctx + (g_menu_char_slot * 0x250) + g_menu_active_subtype + (row = 0x609)) = sel;
        menu_play_se(0x7E, 0x80);
        list->unk0 = 3;
    }

    if (sel != -1)
    {
        sel_base = (void*)(g_menu_state_ptr + *(s32*)(g_menu_state_ptr + 0x3C));
        g_menu_help_text = (s32)((u8*)sel_base + *(u16*)((u8*)sel_base + (sel * 2)));
    }

    return prim_buf;
}

/**
 * @brief Draw a character's equipment grid and handle its selection input.
 * @param ot Ordering-table pointer, forwarded to the glyph renderer.
 * @param state Scroll-list state for this grid.
 * @param prim_buf Primitive buffer write cursor.
 * @param view_origin Viewport anchor; the glyph origin is (0x20 - x, rel_y - y).
 * @param active Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor.
 * @see decomp.me (100%)
 */
s32 menu_equipment_grid_callback(s32* ot, ScrollListState* state, s32 prim_buf, Vec2s* view_origin, int active)
{
    ScrollListState* list;
    u32 scroll_y;
    s32 sel;
    s32 row;
    s32 col;
    s32 y;
    s32 rel_y;
    s32 kind;
    u32 word;
    void* base;
    void* sel_base;
    DR_TPAGE* tp;

    list = state;

    prim_buf = scroll_list_draw(prim_buf, ot, list, g_menu_scroll_nav_entries, view_origin, active);

    if ((g_pad_input & 0x40) && (active != 0))
    {
        menu_play_se(0x7F, 0x80);
        menu_reset_content_view();
        g_pad_input = 0;
    }

    y = 0;
    scroll_y = list->scroll_y;
    sel = -1;
    row = 0;

    do
    {
        col = 0;
        word = *(u32*)((u8*)g_pad_ctx + (row << 2) + 0x104);
        do
        {
            kind = word & 0xF;
            if (kind >= 2)
            {
                rel_y = y - scroll_y;
                if ((rel_y >= -0xF) && (rel_y < (list->viewport_h - 0x10)))
                {
                    base = (void*)(g_menu_state_ptr + *(s32*)(g_menu_state_ptr + 0x6C));
                    prim_buf = func_800A88A0(prim_buf, ot, (void*)((u8*)base + *(u16*)((u8*)base + (col * 2) + (row * 0x10))), 1, 0x20 - view_origin->x,
                                             rel_y - view_origin->y, 0);
                    if (kind >= 8)
                    {
                        if (kind >= 0xF)
                        {
                            prim_buf = (s32)menu_emit_icon_sprite((void*)prim_buf, ot, 0x70, 0x10 - view_origin->x, rel_y - view_origin->y, 0, 0, 0, 0);
                        }
                        else
                        {
                            prim_buf = (s32)menu_emit_icon_sprite((void*)prim_buf, ot, 0x69, 0x10 - view_origin->x, rel_y - view_origin->y, 0, 0, 0, 0);
                        }

                        tp = (DR_TPAGE*)prim_buf;
                        setlen(tp, 1);
                        tp->code[0] = 0xE1000005;
                        addPrim(ot, tp);
                        prim_buf = (s32)(tp + 1);
                    }
                }
                if (list->sel_idx == (y >> 4))
                {
                    sel = col + (row * 8);
                }
                y += 0x10;
            }
            col += 1;
            word >>= 4;
        } while (col < 8);
        row += 1;
    } while (row < 0x10);

    if ((active != 0) && (sel != -1))
    {
        sel_base = (void*)(g_menu_state_ptr + *(s32*)(g_menu_state_ptr + 0x84));
        g_menu_help_text = (s32)((u8*)sel_base + *(u16*)((u8*)sel_base + (sel * 2)));
    }

    return prim_buf;
}

/**
 * @brief Draw a character's key-item list and handle its selection input.
 * @param ot Ordering-table pointer, forwarded to the glyph renderer.
 * @param state Scroll-list state for this list.
 * @param prim_buf Primitive buffer write cursor.
 * @param view_origin Viewport anchor; the glyph origin is (0x10 - x, rel_y - y) and the quantity anchor is (0xC0 - x, rel_y - y).
 * @param active Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor.
 * @see decomp.me (100%)
 */
s32 menu_key_item_list_callback(s32* ot, ScrollListState* state, s32 prim_buf, Vec2s* view_origin, int active)
{
    Vec2s pos;
    s32 idx;
    s32 y;
    s32 rel_y;
    u32 scroll_y;
    u8* item;
    s32 sel;
    void* base;
    void* sel_base;
    ScrollListState* list;

    list = state;

    prim_buf = scroll_list_draw(prim_buf, ot, list, g_menu_scroll_nav_entries, view_origin, active);

    if ((g_pad_input & 0x40) && (active != 0))
    {
        menu_play_se(0x7F, 0x80);
        menu_reset_content_view();
        g_pad_input = 0;
    }

    y = 0;
    sel = -1;
    idx = 0;
    item = (u8*)g_pad_ctx + 0x25E0;
    scroll_y = list->scroll_y;

    do
    {
        if (*item != 0)
        {
            rel_y = y - scroll_y;
            if ((rel_y >= -0xF) && (rel_y < (list->viewport_h - 0x10)))
            {
                pos.x = 0xC0 - (u16)view_origin->x;
                pos.y = rel_y - (u16)view_origin->y;

                base = (void*)(g_menu_state_ptr + *(s32*)(g_menu_state_ptr + 0x30));
                prim_buf =
                    func_800A88A0(prim_buf, ot, (void*)((u8*)base + *(u16*)((u8*)base + (idx * 2))), 1, 0x10 - view_origin->x, rel_y - view_origin->y, 0);
                prim_buf = menu_draw_clamped_number(ot, prim_buf, *item, 1, &pos, 1);
            }
            if (list->sel_idx == (y >> 4))
            {
                sel = idx;
            }
            y += 0x10;
        }
        idx += 1;
        item += 1;
    } while (idx < 0x100);

    if (sel != -1)
    {
        if (active != 0)
        {
            sel_base = (void*)(g_menu_state_ptr + *(s32*)(g_menu_state_ptr + 0x2C));
            g_menu_help_text = (s32)((u8*)sel_base + *(u16*)((u8*)sel_base + (sel * 2)));
        }
    }

    return prim_buf;
}

/**
 * @brief Draw a character's ability list and handle its selection input.
 * @param ot Ordering-table pointer, forwarded to the glyph renderer.
 * @param state Scroll-list state for this list.
 * @param prim_buf Primitive buffer write cursor.
 * @param view_origin Viewport anchor; the glyph origin is (0x20 - x, rel_y - y) and the marker origin is (0x10 - x, rel_y - y).
 * @param active Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor.
 * @see decomp.me (100%)
 */
s32 menu_ability_list_callback(s32* ot, ScrollListState* state, s32 prim_buf, Vec2s* view_origin, int active)
{
    u32 scroll_y;
    s32 sel;
    s32 idx;
    s32 y;
    s32 rel_y;
    u8* item;
    void* base;
    void* sel_base;
    DR_TPAGE* tp;
    ScrollListState* list;

    list = state;

    prim_buf = scroll_list_draw(prim_buf, ot, list, g_menu_scroll_nav_entries, view_origin, active);

    if ((g_pad_input & 0x40) && (active != 0))
    {
        menu_play_se(0x7F, 0x80);
        menu_reset_content_view();
        g_pad_input = 0;
    }

    y = 0;
    sel = -1;
    idx = 0;
    item = (u8*)g_pad_ctx + 0x2F0;
    scroll_y = list->scroll_y;

    do
    {
        if (*item & 1)
        {
            rel_y = y - scroll_y;
            if ((rel_y >= -0xF) && (rel_y < (list->viewport_h - 0x10)))
            {
                base = (void*)(g_menu_state_ptr + *(s32*)(g_menu_state_ptr + 0x18));
                prim_buf =
                    func_800A88A0(prim_buf, ot, (void*)((u8*)base + *(u16*)((u8*)base + (idx * 2))), 1, 0x20 - view_origin->x, rel_y - view_origin->y, 0);
                if (*item & 2)
                {
                    prim_buf = (s32)menu_emit_icon_sprite((void*)prim_buf, ot, 0x2C, 0x10 - view_origin->x, rel_y - view_origin->y, 0, 0, 0, 0);

                    tp = (DR_TPAGE*)prim_buf;
                    setlen(tp, 1);
                    tp->code[0] = 0xE1000005;
                    addPrim(ot, tp);
                    prim_buf = (s32)(tp + 1);
                }
            }
            if (list->sel_idx == (y >> 4))
            {
                sel = idx;
            }
            y += 0x10;
        }
        idx += 1;
        item += 0xC;
    } while (idx < 0x40);

    if (sel != -1)
    {
        if (active != 0)
        {
            sel_base = (void*)(g_menu_state_ptr + *(s32*)(g_menu_state_ptr + 0x14));
            g_menu_help_text = (s32)((u8*)sel_base + *(u16*)((u8*)sel_base + (sel * 2)));
        }
    }

    return prim_buf;
}

/**
 * @brief Handle input for the equip/status page and draw its four state-table labels.
 * @param ot Ordering-table pointer, forwarded to the glyph renderer.
 * @param state Scroll-list state for this page.
 * @param prim_buf Primitive buffer write cursor.
 * @param view_origin Viewport anchor; label origins are (0x30 - x, N - y).
 * @param active Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor (unchanged on the early-return paths).
 * @see decomp.me (100%)
 */
s32 menu_subtype_action_callback(s32* ot, ScrollListState* state, s32 prim_buf, Vec2s* view_origin, int active)
{
    MenuSlotRect rect;
    s32 packed;
    s32 hi;
    s32 handle;
    s32 off;
    s8* p;
    s8* pbase;
    s32 i;
    s8* p1;
    s32 i1;
    s8* p3;
    s32 i3;
    u8 flag;
    MenuContentItem* item;
    MenuContentItem* items;
    ScrollListState* list;
    s32 buf;

    list = state;
    buf = prim_buf;

    if ((g_pad_input & 0x40) && (active != 0))
    {
        menu_play_se(0x7F, 0x80);
        list->unk0 = 3;
    }
    else if ((g_pad_input & 0x220) && (active != 0))
    {
        menu_play_se(0x7D, 0x80);
        switch (list->sel_idx)
        {
        case 0:
            list->unk2 = 0;
            list->unk0 = 0;
            rect.x = 0x40;
            rect.y = 0x60;
            rect.w = 0xF0;
            rect.h = 0x60;
            list = (ScrollListState*)menu_slot_alloc(3, &rect);
            ((MenuSlot*)list)->content_cb = (s32 * (*)()) & menu_special_technique_list_callback;
            packed = menu_build_special_technique_nav_entries();
            hi = packed >> 0x10;
            ((MenuSlot*)list)->lerp_target_b = hi * 0x10;
            ((MenuSlot*)list)->lerp_cur_b = hi * 0x10;
            ((MenuSlot*)list)->anim_frame = 5;
            ((MenuSlot*)list)->active = 2;
            ((MenuSlot*)list)->has_title = 1;
            g_menu_draw_early_out = 1;
            ((MenuSlot*)list)->flags = (((MenuSlot*)list)->flags & 0xFE00FFFF) | ((packed & 0x1FF) << 0x10);
            *(u16*)&((MenuSlot*)list)->flags = (u16)hi;
            return buf;

        case 1:
            i1 = 3;
            p1 = (s8*)g_menu_slots;
            p1 += 0x6C;
            while (i1 >= 0)
            {
                *p1 = 0;
                i1--;
                p1 -= 0x24;
            }
            g_menu_nodes[(g_menu_char_slot * 3) + 2].idx_nav.s.self_idx = 0x29;
            g_menu_nodes[(g_menu_char_slot * 3) + 2].label_id = D_801686A0[g_menu_nodes[(g_menu_char_slot * 3) + 2].idx_nav.s.self_idx];
            g_menu_ability_mask = 0;
            g_menu_content_ready = 1;
            menu_open_content_page(2);
            g_menu_draw_early_out = 1;
            g_menu_item_ptr = 0;
            g_menu_category0_item = 0;
            g_menu_category1_item = 0;
            g_menu_category2_item = 0;
            return buf;

        case 2:
        {
            u8* flag_ptr = (u8*)g_pad_ctx + (g_menu_char_slot * 0x250);
            flag_ptr += g_menu_active_subtype;
            flag = *(flag_ptr + 0x609);
            if (flag == 0xFF)
            {
                break;
            }
            if (flag & 0x80)
            {
                handle = func_800A9060(g_menu_active_subtype);
                if (handle != 0)
                {
                    func_800A8F8C(handle, (s32)(((u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0)) + ((g_menu_active_subtype << 6) + 0x90)));
                    off = ((g_menu_active_subtype + 1) << 6) + (g_menu_char_slot * 0x250);
                    flag_ptr = (u8*)g_pad_ctx + off;
                    flag_ptr[0x640] = 0;
                    func_800A8FB4(off);
                }
                else
                {
                    g_menu_message_line1 = (void*)MENU_TAIL(MENU_STATE_BASE(8), 0xAC);
                    i = 3;
                    pbase = (s8*)g_menu_slots;
                    p = pbase + 0x6C;
                    while (i >= 0)
                    {
                        *p = 0;
                        i--;
                        p -= 0x24;
                    }
                    menu_open_content_page(6);
                    return buf;
                }
            }
            flag_ptr = (u8*)g_pad_ctx + (g_menu_char_slot * 0x250) + g_menu_active_subtype;
            *(flag_ptr + 0x609) = 0xFF;
            list->unk0 = 3;
            break;
        }

        case 3:
            if (*(u8*)g_menu_active_equipped_item == 0)
            {
                break;
            }
            i3 = 3;
            p3 = (s8*)g_menu_slots;
            p3 += 0x6C;
            while (i3 >= 0)
            {
                *p3 = 0;
                i3--;
                p3 -= 0x24;
            }
            g_menu_nodes[(g_menu_char_slot * 3) + 2].idx_nav.s.self_idx = 0x1A;
            g_menu_nodes[(g_menu_char_slot * 3) + 2].label_id = 0x17;
            g_menu_hit_item_idx = menu_find_active_content_item();
            if (g_menu_hit_item_idx != -1)
            {
                items = g_menu_content_table[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];
                item = items - (-g_menu_hit_item_idx);
                g_content_view_x = item->packed_x & 0x1FF;
                g_content_view_y = item->y - 8;
                g_menu_suppress_cursor = 5;
                g_menu_cursor_enable = 1;
            }
            g_menu_draw_early_out = 1;
            return buf;
        }
    }

    buf = scroll_list_draw(buf, ot, list, g_menu_item_nav_entries, view_origin, active);

    buf = func_800A88A0(buf, ot, MENU_TAIL(MENU_STATE_BASE(8), 0x7A), 1, 0x30 - view_origin->x, -view_origin->y, 2);
    buf = func_800A88A0(buf, ot, MENU_TAIL(MENU_STATE_BASE(8), 0x7C), 1, 0x30 - view_origin->x, 0x10 - view_origin->y, 2);
    buf = func_800A88A0(buf, ot, MENU_TAIL(MENU_STATE_BASE(8), 0x86), 1, 0x30 - view_origin->x, 0x20 - view_origin->y, 2);
    buf = func_800A88A0(buf, ot, MENU_TAIL(MENU_STATE_BASE(8), 0x80), 1, 0x30 - view_origin->x, 0x30 - view_origin->y, 2);
    return buf;
}

/**
 * @brief Draws the confirmation page and dismisses it on any cancel/confirm press.
 * @param ot Ordering-table pointer, forwarded to the glyph renderer.
 * @param state Scroll-list state for this page.
 * @param prim_buf Primitive buffer write cursor.
 * @param view_origin Viewport anchor; the glyph origin is (0x88 - x, -y).
 * @param active Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor.
 * @see decomp.me (100%)
 */
s32 menu_message_callback(s32* ot, ScrollListState* state, s32 prim_buf, Vec2s* view_origin, int active)
{
    Vec2s pos;
    ScrollListState* list;
    s32 buf;

    list = state;
    buf = prim_buf;

    if ((g_pad_input & 0x260) && (active != 0))
    {
        list->unk2 = 0;
        list->unk0 = 0;
        menu_play_se(MENU_SE_NAVIGATE, MENU_SE_VOLUME);
    }

    buf = func_800A88A0(buf, ot, g_menu_message_line1, 1, 0x88 - view_origin->x, -view_origin->y, 2);
    return buf;
}

/**
 * @brief Draws the two-line confirmation page and dismisses it on cancel/confirm.
 * @param ot Ordering-table pointer, forwarded to the glyph renderer.
 * @param state Scroll-list state for this page.
 * @param prim_buf Primitive buffer write cursor.
 * @param view_origin Viewport anchor; the glyph origins are (0x88 - x, N - y).
 * @param active Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor.
 * @see decomp.me (100%)
 */
s32 menu_two_line_message_callback(s32* ot, ScrollListState* state, s32 prim_buf, Vec2s* view_origin, int active)
{
    Vec2s pos;
    ScrollListState* list;
    s32 buf;

    list = state;
    buf = prim_buf;

    if ((g_pad_input & 0x260) && (active != 0))
    {
        list->unk2 = 0;
        list->unk0 = 0;
        menu_play_se(MENU_SE_NAVIGATE, MENU_SE_VOLUME);
    }

    buf = func_800A88A0(buf, ot, g_menu_message_line1, 1, 0x88 - view_origin->x, -view_origin->y, 2);
    buf = func_800A88A0(buf, ot, g_menu_message_line2, 1, 0x88 - view_origin->x, 0x10 - view_origin->y, 2);
    return buf;
}

/**
 * @brief Draws a two-glyph scroll-list page and handles its cancel/confirm input.
 * @param ot Ordering-table pointer, forwarded to the glyph renderer.
 * @param state Scroll-list state for this page.
 * @param prim_buf Primitive buffer write cursor.
 * @param view_origin Viewport anchor; the glyph origins are (0x30 - x, N - y).
 * @param active Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor.
 * @see decomp.me (100%)
 */
s32 menu_item_followup_callback(s32* ot, ScrollListState* state, s32 prim_buf, Vec2s* view_origin, int active)
{
    MenuContentItem* item;
    MenuContentItem* tbl;
    ScrollListState* list;
    Vec2s pos;
    s32 i;
    s32 buf;
    s32 idx;

    list = state;
    buf = prim_buf;

    if ((g_pad_input & 0x40) && (active != 0))
    {
        menu_play_se(MENU_SE_CLOSE, MENU_SE_VOLUME);
        list->unk0 = 3;
    }
    else if ((g_pad_input & 0x220) && (active != 0))
    {
        menu_play_se(MENU_SE_NAVIGATE, MENU_SE_VOLUME);
        if (list->sel_idx != 0)
        {
            list->unk0 = 3;
            *(u8*)g_menu_item_ptr = 0;
            func_800A8FB4();
            g_menu_pending_item_row = 0xFF;
        }
        else
        {
            g_menu_scene_type += 1;
            g_menu_active_node = g_menu_scene_type;
            for (i = 3; i >= 0; i--)
            {
                g_menu_slots[i].active = 0;
            }
            idx = menu_find_active_content_item();
            g_menu_hit_item_idx = idx;
            if (idx != -1)
            {
                tbl = g_menu_content_table[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];
                item = (MenuContentItem*)((idx * 8) + (s32)tbl);
                g_content_view_x = item->packed_x & 0x1FF;
                g_content_view_y = item->y - 8;
                g_menu_suppress_cursor = 5;
                g_menu_cursor_enable = 1;
            }
        }
        return buf;
    }

    buf = scroll_list_draw(buf, ot, list, g_menu_item_nav_entries, view_origin, active);

    buf = func_800A88A0(buf, ot, MENU_TAIL(MENU_STATE_BASE(8), 0x88), 1, 0x30 - view_origin->x, -view_origin->y, 2);
    buf = func_800A88A0(buf, ot, MENU_TAIL(MENU_STATE_BASE(8), 0x8A), 1, 0x30 - view_origin->x, 0x10 - view_origin->y, 2);
    return buf;
}

/* Globals and helpers used only by menu_equipment_action_callback. */
extern u8 D_8016869B[];
extern u8 D_800F0BF8[];
/** @brief The u32 at D_800F0BF8 + 0x14; the item-kind word of the default compare entry. */
extern u32 D_800F0C0C;
/** @brief Confirm-action dispatch jump table (rodata), indexed by list->sel_idx. */
extern void* jtbl_80140544[];
s32 menu_stage_best_equipment_for_slot0();
s32 menu_stage_best_equipment_for_active_slot();

/**
 * @brief Draw the character's equip/ability page and dispatch its confirm action.
 * @param ot Ordering-table pointer, forwarded to the glyph renderer.
 * @param state Scroll-list state for this page.
 * @param prim_buf Primitive buffer write cursor.
 * @param view_origin Viewport anchor; label origins are (0x30 - x, N - y).
 * @param active Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor.
 * @see decomp.me (100.00%)
 * @note The confirm-action switch on list->sel_idx is emitted as a computed goto
 *       through the rodata jump table jtbl_80140544; the static outer_keep[] array
 *       keeps the case labels address-taken so gcc reproduces that dispatch exactly.
 */
s32 menu_equipment_action_callback(s32* ot, ScrollListState* state, s32 prim_buf, Vec2s* view_origin, int active)
{
    u16 rect[4];
    ScrollListState* list;
    MenuContentItem* item;
    MenuContentItem* tbl;
    u8* entry;
    u8* pad_item;
    u8* cmp_tbl;
    u8* ctx;
    u8* ctx2;
    s32 slot_off;
    u32 unk14;
    u32 lhs_shift;
    u32 cmp_shift;
    s32 mask;
    s32 arg;
    s32 idx;
    s32 hit;
    s32 buf;
    s32 handle;
    s32 handle2;
    s32 off;
    s32 off2;
    s32 j;
    s32 prev;
    s32 next;
    s32 more;
    s32 link;
    s32 word_prev;
    s32 i9;
    s32 j1;
    s32 prev1;
    s32 next1;
    s32 more1;
    s32 link1;
    s32 word_prev1;
    s32 j2;
    s32 prev2;
    s32 next2;
    s32 more2;
    s32 link2;
    s32 word_prev2;
    s32 i;
    s32 i1;
    s32 i2;
    s32 i3;
    s32 i4;
    s32 i5;
    s32 i6;
    s32 i7;
    s32 i8;
    u16 dispatch;

    static void* const outer_keep[] = {
        &&outer_case_0, &&outer_case_1, &&outer_case_2, &&outer_case_3, &&outer_case_4
    };

    list = state;
    buf = prim_buf;

    if ((g_pad_input & 0x40) && (active != 0))
    {
        menu_play_se(0x7F, 0x80);
        list->unk0 = 3;
    }
    else if ((g_pad_input & 0x220) && (active != 0))
    {
        menu_play_se(0x7D, 0x80);
        switch (0)
        {
        case 0:
            dispatch = list->sel_idx;
            if ((u32)dispatch >= 5)
            {
                break;
            }
            goto *jtbl_80140544[dispatch];

        outer_case_0:
            switch (g_menu_active_subtype)
            {
            case 7:
            {
                s8* ability_mask_ptr;

                i = 3;
                while (i >= 0)
                {
                    g_menu_slots[i].active = 0;
                    i--;
                }
                g_menu_nodes[(g_menu_char_slot * 3) + 1].idx_nav.s.self_idx = 0x24;
                g_menu_nodes[(g_menu_char_slot * 3) + 1].label_id = D_8016869F[g_menu_nodes[(g_menu_char_slot * 3) + 1].idx_nav.s.self_idx];

                ability_mask_ptr = &g_menu_ability_mask;
                mask = 0;
                entry = (u8*)g_menu_equipment_base;
                idx = 0;
                do
                {
                    if ((idx != (g_menu_active_subtype - 7)) && (entry[0] != 0))
                    {
                        unk14 = *(u32*)(entry + 0x14);
                        if (unk14 & 0x300)
                        {
                            mask |= D_800F0BEC[(unk14 >> 10) & 0x3F];
                        }
                        else
                        {
                            mask |= D_800F0BE0[(unk14 >> 10) & 0x3F];
                        }
                    }
                    idx += 1;
                    entry += 0x40;
                } while (idx < 4);

                *ability_mask_ptr = mask;
                g_menu_content_ready = 1;
                menu_open_content_page(0);
                g_menu_draw_early_out = 1;
                return buf;
            }

            case 8:
            case 9:
            case 0xA:
            {
                s8* ability_mask_ptr;

                i1 = 3;
                while (i1 >= 0)
                {
                    g_menu_slots[i1].active = 0;
                    i1--;
                }
                g_menu_nodes[(g_menu_char_slot * 3) + 1].idx_nav.s.self_idx = 0x27;
                g_menu_nodes[(g_menu_char_slot * 3) + 1].label_id = D_8016869B[g_menu_nodes[(g_menu_char_slot * 3) + 1].idx_nav.s.self_idx];

                ability_mask_ptr = &g_menu_ability_mask;
                mask = 0;
                idx = 0;
                entry = (u8*)g_menu_equipment_base;

                do
                {
                    if ((idx != (g_menu_active_subtype - 7)) && (entry[0] != 0))
                    {
                        unk14 = *(u32*)(entry + 0x14);
                        if (unk14 & 0x300)
                        {
                            mask |= D_800F0BEC[(unk14 >> 10) & 0x3F];
                        }
                        else
                        {
                            mask |= D_800F0BE0[(unk14 >> 10) & 0x3F];
                        }
                    }
                    idx += 1;
                    entry += 0x40;
                } while (idx < 4);

                *ability_mask_ptr = mask;
                g_menu_content_ready = 1;
                menu_open_content_page(1);
                g_menu_draw_early_out = 1;
                return buf;
            }

            default:
                break;
            }
            break;

        outer_case_1:
            if (g_menu_active_subtype == 7)
            {
                if (menu_stage_best_equipment_for_slot0() == 0)
                {
                    break;
                }
                i2 = 3;
                while (i2 >= 0)
                {
                    g_menu_slots[i2].active = 0;
                    i2--;
                }
                rect[0] = 0xB0;
                rect[1] = 0x40;
                rect[2] = 0x70;
                rect[3] = 0x30;
                list = (ScrollListState*)menu_slot_alloc(0, rect);
                ((MenuSlot*)list)->content_cb = (s32 * (*)()) & menu_equipment_compare_callback;
                g_menu_compare_window_active = 1;
                ((MenuSlot*)list)->flags = (((MenuSlot*)list)->flags & 0xFE00FFFF) | 0x20000;

                j = 0;
                do
                {
                    s32 cur = g_menu_item_nav_entries[j];
                    s32 word_self;

                    prev = 1;
                    link = cur & ~0x3FFF;
                    link = link | ((j * 0x10) & 0x3FFF);
                    word_self = link;
                    g_menu_item_nav_entries[j] = word_self;
                    if ((j - 1) >= 0)
                    {
                        prev = j - 1;
                    }
                    word_prev = word_self & 0xFF803FFF;
                    word_prev = word_prev | ((prev & 0x1FF) << 14);
                    g_menu_item_nav_entries[j] = word_prev;
                    next = j + 1;
                    more = next < 2;
                    link = 0;
                    if (more != 0)
                    {
                        link = next;
                    }
                    g_menu_item_nav_entries[j] = (word_prev & 0x7FFFFF) | (link << 23);
                    j = next;
                } while (more != 0);
                break;
            }
            g_item_slot_data.slot3 = 0;
            g_item_slot_data.slot2 = 0;
            g_item_slot_data.slot1 = 0;
            if (menu_stage_best_equipment_for_active_slot() == 0)
            {
                break;
            }
            i3 = 3;
            while (i3 >= 0)
            {
                g_menu_slots[i3].active = 0;
                i3--;
            }
            rect[0] = 0xB0;
            rect[1] = 0x40;
            rect[2] = 0x70;
            rect[3] = 0x30;
            list = (ScrollListState*)menu_slot_alloc(0, rect);
            ((MenuSlot*)list)->content_cb = (s32 * (*)()) & menu_equipment_compare_callback;
            g_menu_compare_window_active = 1;
            ((MenuSlot*)list)->flags = (((MenuSlot*)list)->flags & 0xFE00FFFF) | 0x20000;

            j1 = 0;
            do
            {
                s32 cur = g_menu_item_nav_entries[j1];
                s32 word_self;

                prev1 = 1;
                link1 = cur & ~0x3FFF;
                link1 = link1 | ((j1 * 0x10) & 0x3FFF);
                word_self = link1;
                g_menu_item_nav_entries[j1] = word_self;
                if ((j1 - 1) >= 0)
                {
                    prev1 = j1 - 1;
                }
                word_prev1 = word_self & 0xFF803FFF;
                word_prev1 = word_prev1 | ((prev1 & 0x1FF) << 14);
                g_menu_item_nav_entries[j1] = word_prev1;
                next1 = j1 + 1;
                more1 = next1 < 2;
                link1 = 0;
                if (more1 != 0)
                {
                    link1 = next1;
                }
                g_menu_item_nav_entries[j1] = (word_prev1 & 0x7FFFFF) | (link1 << 23);
                j1 = next1;
            } while (more1 != 0);
            break;

        outer_case_2:
            g_item_slot_data.slot3 = 0;
            g_item_slot_data.slot2 = 0;
            g_item_slot_data.slot1 = 0;
            menu_stage_best_equipment_for_slot0();
            g_menu_active_subtype = 8;
            menu_stage_best_equipment_for_active_slot();
            g_menu_active_subtype = 9;
            menu_stage_best_equipment_for_active_slot();
            g_menu_active_subtype = 0xA;
            menu_stage_best_equipment_for_active_slot();

            i4 = 3;
            while (i4 >= 0)
            {
                g_menu_slots[i4].active = 0;
                i4--;
            }
            rect[0] = 0xB0;
            rect[1] = 0x40;
            rect[2] = 0x70;
            rect[3] = 0x30;
            list = (ScrollListState*)menu_slot_alloc(0, rect);
            ((MenuSlot*)list)->content_cb = (s32 * (*)()) & menu_equipment_compare_callback;
            g_menu_compare_window_active = 1;
            ((MenuSlot*)list)->flags = (((MenuSlot*)list)->flags & 0xFE00FFFF) | 0x20000;

            j2 = 0;
            do
            {
                s32 cur = g_menu_item_nav_entries[j2];
                s32 word_self;

                prev2 = 1;
                link2 = cur & ~0x3FFF;
                link2 = link2 | ((j2 * 0x10) & 0x3FFF);
                word_self = link2;
                g_menu_item_nav_entries[j2] = word_self;
                if ((j2 - 1) >= 0)
                {
                    prev2 = j2 - 1;
                }
                word_prev2 = word_self & 0xFF803FFF;
                word_prev2 = word_prev2 | ((prev2 & 0x1FF) << 14);
                g_menu_item_nav_entries[j2] = word_prev2;
                next2 = j2 + 1;
                more2 = next2 < 2;
                link2 = 0;
                if (more2 != 0)
                {
                    link2 = next2;
                }
                g_menu_item_nav_entries[j2] = (word_prev2 & 0x7FFFFF) | (link2 << 23);
                j2 = next2;
            } while (more2 != 0);
            break;

        outer_case_3:
            if (*(u8*)g_menu_active_equipped_item == 0)
            {
                break;
            }
            i5 = 3;
            while (i5 >= 0)
            {
                g_menu_slots[i5].active = 0;
                i5--;
            }
            if (g_menu_active_subtype == 7)
            {
                g_menu_item_ptr = g_menu_active_equipped_item;
                g_menu_category0_item = g_menu_saved_category0_item;
                g_menu_nodes[(g_menu_char_slot * 3) + 1].idx_nav.s.self_idx = 0x14;
                g_menu_nodes[(g_menu_char_slot * 3) + 1].label_id = 0x12;
            }
            else if (g_menu_active_subtype >= 7)
            {
                if (g_menu_active_subtype < 0xB)
                {
                    g_menu_item_ptr = g_menu_active_equipped_item;
                    g_menu_category1_item = g_menu_saved_category1_item;
                    g_menu_nodes[(g_menu_char_slot * 3) + 1].idx_nav.s.self_idx = 0x17;
                    g_menu_nodes[(g_menu_char_slot * 3) + 1].label_id = 0x15;
                }
            }
            hit = menu_find_active_content_item();
            g_menu_hit_item_idx = hit;
            if (hit != -1)
            {
                tbl = g_menu_content_table[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];
                item = (MenuContentItem*)((hit * 8) + (s32)tbl);
                g_content_view_x = item->packed_x & 0x1FF;
                g_content_view_y = item->y - 8;
                g_menu_suppress_cursor = 5;
                g_menu_cursor_enable = 1;
            }
            g_menu_draw_early_out = 1;
            return buf;

        outer_case_4:
            if (*(u8*)g_menu_active_equipped_item == 0)
            {
                break;
            }
            handle = func_800A9060();
            if (handle != 0)
            {
                if (menu_item_is_nondefault((s32)(((u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0)) + ((g_menu_active_subtype << 6) - 0x170))) != 0)
                {
                    func_800A8F8C(handle, (s32)(((u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0)) + ((g_menu_active_subtype << 6) - 0x170)));
                    switch (g_menu_active_subtype)
                    {
                    case 7:
                        g_menu_active_equipped_item = 0;
                        g_menu_saved_category0_item = 0;
                        slot_off = g_menu_char_slot * 0x250;
                        ctx = (u8*)g_pad_ctx;
                        pad_item = ctx - (-slot_off);
                        lhs_shift = *(u32*)(pad_item + 0x654) >> 10;
                        cmp_shift = *(u32*)((cmp_tbl = D_800F0BF8) + 0x14) >> 10;
                        if ((lhs_shift & 0x3F) == (cmp_shift & 0x3F))
                        {
                            func_800A8F8C((slot_off + (s32)ctx) + 0x640, cmp_tbl);
                            break;
                        }
                        func_800A8F8C((slot_off + (s32)ctx) + 0x640, cmp_tbl);

                        i9 = 1;
                        do
                        {
                            off = i9 << 6;
                            pad_item = (u8*)g_pad_ctx + (off + (g_menu_char_slot * 0x250));
                            if (pad_item[0x640] != 0)
                            {
                                if ((((*(u32*)(pad_item + 0x654)) >> 10) & 0x3F) == 0)
                                {
                                    handle2 = func_800A9060();
                                    if (handle2 != 0)
                                    {
                                        func_800A8F8C(handle2, (s32)(((u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0)) + (off + 0x50)));
                                        *((u8*)g_pad_ctx + (off - (-(g_menu_char_slot * 0x250))) + 0x640) = 0;
                                        func_800A8FB4();
                                        break;
                                    }
                                    func_800A8F8C(((g_menu_char_slot * 0x250) + (s32)g_pad_ctx) + 0x640, handle);
                                    *(u8*)handle = 0;
                                    g_menu_message_line1 = (void*)MENU_TAIL(MENU_STATE_BASE(8), 0xAC);
                                    i6 = 3;
                                    while (i6 >= 0)
                                    {
                                        g_menu_slots[i6].active = 0;
                                        i6--;
                                    }
                                    menu_open_content_page(6);
                                    return buf;
                                }
                            }
                            i9 += 1;
                        } while (i9 < 4);

                        if (menu_clear_pending_status() == 0)
                        {
                            break;
                        }
                        g_menu_message_line1 = (void*)MENU_TAIL(MENU_STATE_BASE(8), 0x3A);
                        i7 = 3;
                        while (i7 >= 0)
                        {
                            g_menu_slots[i7].active = 0;
                            i7--;
                        }
                        menu_open_content_page(6);
                        return buf;

                    case 8:
                    case 9:
                    case 0xA:
                        g_menu_active_equipped_item = 0;
                        g_menu_saved_category1_item = 0;
                        do
                        {
                            s32 subtype2;
                            subtype2 = g_menu_active_subtype;
                            ctx2 = (u8*)g_pad_ctx;
                            off2 = ((subtype2 - 7) << 6) + (g_menu_char_slot * 0x250);
                            *(ctx2 + off2 + 0x640) = 0;
                        } while (0);
                        func_800A8FB4(off2);
                        break;
                    }
                }
                list->unk0 = 3;
                break;
            }
            g_menu_message_line1 = (void*)MENU_TAIL(MENU_STATE_BASE(8), 0xAC);
            i8 = 3;
            while (i8 >= 0)
            {
                g_menu_slots[i8].active = 0;
                i8--;
            }
            menu_open_content_page(6);
            return buf;
        }
    }

    buf = scroll_list_draw(buf, ot, list, g_menu_item_nav_entries, view_origin, active);

    buf = func_800A88A0(buf, ot, MENU_TAIL(MENU_STATE_BASE(8), 0x7E), 1, 0x30 - view_origin->x, -view_origin->y, 2);
    buf = func_800A88A0(buf, ot, MENU_TAIL(MENU_STATE_BASE(8), 0x82), 1, 0x30 - view_origin->x, 0x10 - view_origin->y, 2);
    buf = func_800A88A0(buf, ot, MENU_TAIL(MENU_STATE_BASE(8), 0x84), 1, 0x30 - view_origin->x, 0x20 - view_origin->y, 2);
    buf = func_800A88A0(buf, ot, MENU_TAIL(MENU_STATE_BASE(8), 0x80), 1, 0x30 - view_origin->x, 0x30 - view_origin->y, 2);
    buf = func_800A88A0(buf, ot, MENU_TAIL(MENU_STATE_BASE(8), 0x86), 1, 0x30 - view_origin->x, 0x40 - view_origin->y, 2);
    return buf;
}

/**
 * @brief Content callback for the item-compare window opened by @ref menu_equipment_action_callback.
 * @param ot Ordering-table pointer.
 * @param state Scroll-list state for this window.
 * @param prim_buf Primitive buffer write cursor.
 * @param view_origin Viewport anchor in list-local coordinates.
 * @param active Non-zero when this window owns input.
 * @return Updated primitive write cursor; the unchanged @p prim_buf on the close path.
 * @see decomp.me (100%) https://decomp.me/scratch/JSzAG
 */
s32 menu_equipment_compare_callback(s32* ot, ScrollListState* state, s32 prim_buf, Vec2s* view_origin, int active)
{
    u16 rect[4];
    ScrollListState* list;
    s32 i;
    s32 item;
    s32 slot_off;
    s32 buf;

    list = state;
    buf = prim_buf;
    if ((g_pad_input & 0x260) && (active != 0))
    {
        g_menu_compare_window_active = 0;

        if ((list->sel_idx != 0) || (g_pad_input & 0x40))
        {
            menu_play_se(MENU_SE_CLOSE, MENU_SE_VOLUME);
            list->unk0 = 3;

            i = 0;
            do
            {
                if (((u8*)&g_item_slot_flags)[i] != 0)
                {
                    item = (s32)((u32*)&g_item_slot_data)[i];
                    if (item != 0)
                    {
                        menu_swap_item_records(item, (s32)(((u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0)) + ((i << 6) + 0x50)));
                    }
                    else if (i == 0)
                    {
                        func_800A8F8C(func_800A9060(), (s32)((g_menu_char_slot * 0x250) + (s32)g_pad_ctx + 0x640));
                        func_800A8F8C((s32)((g_menu_char_slot * 0x250) + (s32)g_pad_ctx + 0x640), D_800F0BF8);
                    }
                    else
                    {
                        func_800A8F8C(func_800A9060(), (s32)(((u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0)) + ((i << 6) + 0x50)));
                        slot_off = g_menu_char_slot * 0x250;
                        *((u8*)g_pad_ctx + ((i << 6) + slot_off) + 0x640) = 0;
                    }
                }
                i += 1;
            } while (i < 4);
        }
        else
        {
            menu_play_se(MENU_SE_SELECT, MENU_SE_VOLUME);
            list->unk0 = 3;

            if (g_item_slot_flags.slot0 != 0)
            {
                if (((g_item_slot_data.slot0 != 0) &&
                     ((PAD_ITEM_W14(g_item_slot_data.slot0) & 0xFC00) != (PAD_ITEM_W14(g_menu_equipment_base) & 0xFC00))) ||
                    ((g_item_slot_data.slot0 == 0) &&
                     ((D_800F0C0C & 0xFC00) != (PAD_ITEM_W14(g_menu_equipment_base) & 0xFC00))))
                {
                    if (menu_clear_pending_status() != 0)
                    {
                        s32 n = 3;
                        u8* base;
                        u8* sp;

                        g_menu_message_line1 = (void*)MENU_TAIL(MENU_STATE_BASE(8), 0x3A);
                        base = (u8*)g_menu_slots;
                        sp = base + 0x6C;
                        do
                        {
                            *sp = 0;
                            n -= 1;
                            sp -= 0x24;
                        } while (n >= 0);
                        menu_open_content_page(MENU_REDRAW_NAVIGATE);
                    }
                }
            }
        }

        func_800A8FB4();
        g_item_slot_flags.slot0 = 0;
        g_item_slot_flags.slot1 = 0;
        g_item_slot_flags.slot2 = 0;
        g_item_slot_flags.slot3 = 0;
        return buf;
    }

    buf = scroll_list_draw(buf, ot, list, g_menu_item_nav_entries, view_origin, active);
    buf = func_800A88A0(buf, ot, MENU_TAIL(MENU_STATE_BASE(8), 0x90), 1, 0x30 - view_origin->x, -view_origin->y, 2);
    buf = func_800A88A0(buf, ot, MENU_TAIL(MENU_STATE_BASE(8), 0x92), 1, 0x30 - view_origin->x, 0x10 - view_origin->y, 2);
    g_pad_input = 0;
    return buf;
}

/**
 * @brief Test whether an item record differs from the default/empty compare entry.
 * @param item_addr Address of the 0x40-byte item record to test.
 * @return 1 as soon as a byte differs; 0 if all 0x40 bytes are equal.
 * @see decomp.me (100%) https://decomp.me/scratch/Jm6yb
 */
s32 menu_item_is_nondefault(s32 item_addr)
{
    u8* item;
    u8* cmp;
    u32 i;

    item = (u8*)item_addr;
    cmp = D_800F0BF8;

    for (i = 0; i < 0x40; i++, cmp++, item++)
    {
        if (*cmp != *item)
        {
            return 1;
        }
    }

    return 0;
}

/**
 * @brief Swap two 0x40-byte item records through a stack buffer.
 * @param first_addr Address of the first 0x40-byte item record.
 * @param second_addr Address of the second 0x40-byte item record.
 * @see decomp.me (100%)
 */
void menu_swap_item_records(s32 first_addr, s32 second_addr)
{
    u8 tmp[0x40];

    func_800A8F8C(tmp, first_addr);
    func_800A8F8C(first_addr, second_addr);
    func_800A8F8C(second_addr, tmp);
}

/**
 * @brief Point @ref g_active_slot at the highest-numbered menu slot still in use.
 * @see decomp.me (100%) https://decomp.me/scratch/7whwm
 */
void menu_update_active_slot(void)
{
    s32 i;

    for (i = 0; i < 4; i++)
    {
        if (g_menu_slots[i].active != 0)
        {
            g_active_slot = i;
        }
    }
}


s32 scroll_list_draw(s32 prim_buf, s32* ot, ScrollListState* state, u32* entries, Vec2s* view_origin, int active);
void func_800A8F8C();
void func_800A8FB4();
s32 func_800A9060();
s32 func_800A88A0(s32 prim, s32* ot, void* glyph, s32 a3, s32 x, s32 y, s32 mode);
void menu_play_se(s32 sound_id, s32 volume);
void* menu_find_best_equipment_for_active_slot(void);
void menu_open_content_page(u32 content_id);

/** @brief Active item-list category selected by content pages 0-5. */

s32 menu_build_inventory_nav_entries(s32 arg0);
s32 menu_build_equipment_nav_entries(void);
s32 menu_build_key_item_nav_entries(void);
s32 menu_build_ability_nav_entries(void);
/**
 * @brief Draw the learned Special Technique list and handle technique assignment.
 * @param ot Ordering-table pointer, forwarded to the glyph renderer.
 * @param arg1 Scroll-list state for this page (aliased into @c state).
 * @param arg2 Primitive buffer write cursor (aliased into @c prim).
 * @param view_origin Viewport anchor; glyph origins are (0x10 - x, rel - y).
 * @param active Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor.
 * @see decomp.me (100%)
 */
s32 menu_special_technique_list_callback(s32* ot, ScrollListState* arg1, s32 arg2, Vec2s* view_origin, s32 active)
{
    ScrollListState* state = arg1;
    s32 prim = arg2;
    s32 found;
    s32 count;
    s32 i;
    s32 j;
    s32 mask;
    s32 word;
    s32* p;
    s32 rel;
    s32 scroll_y;
    u8 flag;
    s32 v0;

    if ((g_pad_input & 0x40) && (active != 0))
    {
        menu_play_se(0x7F, 0x80);
        state->unk0 = 3;
        return prim;
    }

    found = -1;
    count = 0;
    i = count;
    p = (s32*)((u8*)g_pad_ctx + 0x34);
    do
    {
        j = 0;
        mask = 1;
        word = *p;
        do
        {
            if (word & mask)
            {
                if (state->sel_idx == (count >> 4))
                {
                    found = j + (i * 0x18);
                }
                count += 0x10;
            }
            j += 1;
            mask = mask << 1;
        } while (j < 0x18);
        i += 1;
        p += 1;
    } while (i < 0xB);

    if ((g_pad_input & 0x220) && (active != 0))
    {
        if ((found / 24) == (s32)(((u32)(*(s32*)((u8*)g_menu_equipment_base + 0x14)) >> 0xA) & 0x3F))
        {
            u8* flag_ptr = (u8*)g_pad_ctx + (g_menu_char_slot * 0x250);
            u8* ctx;

            flag_ptr += g_menu_active_subtype;
            flag = *(flag_ptr + 0x609);
            if (flag != 0xFF)
            {
                if (flag & 0x80)
                {
                    v0 = func_800A9060();
                    if (v0 != 0)
                    {
                        s32 off;

                        do
                        {
                            func_800A8F8C(v0, (u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0) + ((g_menu_active_subtype << 6) + 0x90));
                            *((u8*)g_pad_ctx + (off = ((g_menu_active_subtype + 1) << 6) + (g_menu_char_slot * 0x250)) + 0x640) = 0;
                        } while (0);
                        func_800A8FB4(off);
                    }
                    else
                    {
                        s32 n = 3;
                        MenuSlot* pool;
                        s8* slot;
                        u8* b;

                        b = g_menu_state_ptr + *(s32*)(g_menu_state_ptr + 8);
                        pool = g_menu_slots;
                        slot = (s8*)pool + 0x6C;
                        g_menu_message_line1 = b + *(u16*)(b + 0xAC);
                        do
                        {
                            *slot = 0;
                            n -= 1;
                            slot -= 0x24;
                        } while (n >= 0);
                        menu_open_content_page(6);
                        return prim;
                    }
                }
            }
            ctx = (u8*)g_pad_ctx + (g_menu_char_slot * 0x250);
            ctx += g_menu_active_subtype;
            *(ctx + 0x609) = found % 24;
            menu_play_se(0x7E, 0x80);
            state->unk0 = 3;
        }
        else
        {
            menu_play_se(0x78, 0x80);
        }
    }

    prim = scroll_list_draw(prim, ot, state, (u32*)&g_menu_scroll_nav_entries, view_origin, active);

    found = -1;
    count = 0;
    i = count;
    p = (s32*)((u8*)g_pad_ctx + 0x34);
    scroll_y = state->scroll_y;
    do
    {
        j = 0;
        mask = 1;
        do
        {
            if (*p & mask)
            {
                rel = count - scroll_y;
                if (rel >= -0xF)
                {
                    if (rel < state->viewport_h - 0x10)
                    {
                        s32 color;
                        u8* base;
                        s32 offs;
                        u8* glyph;

                        color = 3;
                        base = g_menu_state_ptr;
                        base += *(s32*)(base + 0x20);
                        offs = *(u16*)((u8*)base + (j * 2) + (i * 0x30));
                        glyph = base + offs;
                        if (i == (s32)(((u32)(*(s32*)((u8*)g_menu_equipment_base + 0x14)) >> 0xA) & 0x3F))
                        {
                            color = 1;
                        }
                        prim = func_800A88A0(prim, ot, glyph, color, 0x10 - view_origin->x, rel - view_origin->y, 0);
                    }
                }
                if (state->sel_idx == (count >> 4))
                {
                    found = j + (i * 0x18);
                }
                count += 0x10;
            }
            j += 1;
            mask = mask << 1;
        } while (j < 0x18);
        i += 1;
        p += 1;
    } while (i < 0xB);

    if (found != -1)
    {
        u8* base = g_menu_state_ptr + *(s32*)(g_menu_state_ptr + 0x1C);

        g_menu_help_text = (s32)(base + *(u16*)(base + (found * 2)));
    }
    return prim;
}

/**
 * @brief Open the menu content window for the given content page id.
 * @param content_id Content page id (0-7); out of range is a no-op beyond the g_menu_pending_item_row reset.
 * @see decomp.me (99.97%)
 */
void menu_open_content_page(u32 content_id)
{
    MenuSlotRect rect;
    MenuSlot* slot;
    s32 v0;
    s32 j;
    s32 prev;
    s32 next;
    s32 more;
    s32 link;
    s32 word_self;
    s32 word_prev;

    g_menu_pending_item_row = 0xFF;
    switch (content_id)
    {
    case 0:
        rect.x = 0x40;
        rect.y = 0x60;
        rect.w = 0xF0;
        rect.h = 0x60;
        slot = menu_slot_alloc(3, &rect);
        slot->content_cb = (s32 * (*)()) & menu_inventory_list_callback;
        slot->has_title = 1;
        slot->anim_frame = 5;
        slot->active = 2;
        v0 = menu_build_inventory_nav_entries(0);
        g_menu_active_item_category = 0;
        slot->flags = (slot->flags & 0xFE00FFFF) | ((v0 & 0x1FF) << 16);
        break;

    case 1:
        rect.x = 0x40;
        rect.y = 0x60;
        rect.w = 0xF0;
        rect.h = 0x60;
        slot = menu_slot_alloc(3, &rect);
        slot->content_cb = (s32 * (*)()) & menu_inventory_list_callback;
        slot->has_title = 1;
        slot->anim_frame = 5;
        slot->active = 2;
        v0 = menu_build_inventory_nav_entries(1);
        g_menu_active_item_category = 1;
        slot->flags = (slot->flags & 0xFE00FFFF) | ((v0 & 0x1FF) << 16);
        break;

    case 2:
        rect.x = 0x40;
        rect.y = 0x60;
        rect.w = 0xF0;
        rect.h = 0x60;
        slot = menu_slot_alloc(3, &rect);
        slot->content_cb = (s32 * (*)()) & menu_inventory_list_callback;
        slot->has_title = 1;
        slot->anim_frame = 5;
        slot->active = 2;
        v0 = menu_build_inventory_nav_entries(2);
        g_menu_active_item_category = 2;
        slot->flags = (slot->flags & 0xFE00FFFF) | ((v0 & 0x1FF) << 16);
        break;

    case 3:
        rect.x = 0x40;
        rect.y = 0x2C;
        rect.w = 0xE8;
        rect.h = 0x90;
        slot = menu_slot_alloc(3, &rect);
        slot->content_cb = (s32 * (*)()) & menu_equipment_grid_callback;
        slot->has_title = 1;
        slot->anim_frame = 5;
        slot->active = 2;
        v0 = menu_build_equipment_nav_entries();
        g_menu_active_item_category = 3;
        slot->flags = (slot->flags & 0xFE00FFFF) | ((v0 & 0x1FF) << 16);
        break;

    case 4:
        rect.x = 0x40;
        rect.y = 0x2C;
        rect.w = 0xE8;
        rect.h = 0x80;
        slot = menu_slot_alloc(3, &rect);
        slot->content_cb = (s32 * (*)()) & menu_key_item_list_callback;
        slot->has_title = 1;
        slot->anim_frame = 5;
        slot->active = 2;
        v0 = menu_build_key_item_nav_entries();
        g_menu_active_item_category = 3;
        slot->flags = (slot->flags & 0xFE00FFFF) | ((v0 & 0x1FF) << 16);
        break;

    case 5:
        rect.x = 0x40;
        rect.y = 0x2C;
        rect.w = 0xE8;
        rect.h = 0x90;
        slot = menu_slot_alloc(3, &rect);
        slot->content_cb = (s32 * (*)()) & menu_ability_list_callback;
        slot->has_title = 1;
        slot->anim_frame = 5;
        slot->active = 2;
        v0 = menu_build_ability_nav_entries();
        g_menu_active_item_category = 3;
        slot->flags = (slot->flags & 0xFE00FFFF) | ((v0 & 0x1FF) << 16);
        break;

    case 6:
        rect.x = 0x10;
        rect.y = 0x60;
        rect.w = 0x120;
        rect.h = 0x20;
        slot = menu_slot_alloc(3, &rect);
        slot->content_cb = (s32 * (*)()) & menu_message_callback;
        slot->anim_frame = 5;
        slot->active = 2;
        slot->flags = (slot->flags & 0xFE00FFFF) | 0x10000;

        j = 0;
        do
        {
            s32 cur = g_menu_item_nav_entries[j];

            prev = 0;
            link = cur & ~0x3FFF;
            link = link | ((j * 0x10) & 0x3FFF);
            word_self = link;
            g_menu_item_nav_entries[j] = word_self;
            if ((j - 1) >= 0)
            {
                prev = j - 1;
            }
            word_prev = word_self & 0xFF803FFF;
            word_prev = word_prev | ((prev & 0x1FF) << 14);
            g_menu_item_nav_entries[j] = word_prev;
            next = j + 1;
            more = next < 1;
            link = 0;
            if (more != 0)
            {
                link = next;
            }
            g_menu_item_nav_entries[j] = (word_prev & 0x7FFFFF) | (link << 23);
            j = next;
        } while (more != 0);
        g_menu_draw_early_out = 1;
        break;

    case 7:
        rect.x = 0x10;
        rect.y = 0x60;
        rect.w = 0x120;
        rect.h = 0x30;
        slot = menu_slot_alloc(3, &rect);
        slot->content_cb = (s32 * (*)()) & menu_two_line_message_callback;
        slot->anim_frame = 5;
        slot->active = 2;
        slot->flags = (slot->flags & 0xFE00FFFF) | 0x10000;

        j = 0;
        do
        {
            s32 cur = g_menu_item_nav_entries[j];

            prev = 0;
            link = cur & ~0x3FFF;
            link = link | ((j * 0x10) & 0x3FFF);
            word_self = link;
            g_menu_item_nav_entries[j] = word_self;
            if ((j - 1) >= 0)
            {
                prev = j - 1;
            }
            word_prev = word_self & 0xFF803FFF;
            word_prev = word_prev | ((prev & 0x1FF) << 14);
            g_menu_item_nav_entries[j] = word_prev;
            next = j + 1;
            more = next < 1;
            link = 0;
            if (more != 0)
            {
                link = next;
            }
            g_menu_item_nav_entries[j] = (word_prev & 0x7FFFFF) | (link << 23);
            j = next;
        } while (more != 0);
        g_menu_draw_early_out = 1;
        break;
    }
}

/**
 * @brief Count usable 4-bit entries in the pad-ctx table at +0x104 and rebuild the g_menu_scroll_nav_entries circular nav list to that size.
 * @return Number of entries counted (also the nav-list length).
 * @see decomp.me (100%)
 */
s32 menu_build_equipment_nav_entries(void)
{
    s32 temp_a1;
    s32 temp_a3;
    s32 temp_v1;
    s32 var_a2;
    s32 var_v1;
    s32* temp_t0;

    s32 tmp;
    s32 tmp2;
    s32 tmp3;

    s32 count;
    s32 i;
    s32 j;
    u32 word;
    s32 mask;

    count = 0;
    i = count;
    mask = 0xF;
    do
    {
        word = *(u32*)((u8*)g_pad_ctx + (i * 4) + 0x104);
        j = 7;
        do
        {
            if ((word & mask) >= 2)
            {
                count += 1;
            }
            j -= 1;
            word = word >> 4;
        } while (j >= 0);
        i += 1;
    } while (i < 0x10);

    g_menu_scroll_nav_entries[0] = 0;
    j = 0;
    if (count > 0)
    {
        do
        {
            temp_t0 = (j) + (s32*)&g_menu_scroll_nav_entries;

            tmp = *temp_t0;
            var_a2 = j - 1;

            temp_v1 = (tmp & ~0x3FFF);

            tmp2 = (j * 0x10);
            tmp2 = tmp2 & 0x3FFF;

            temp_v1 = temp_v1 | tmp2;
            *temp_t0 = temp_v1;

            if (var_a2 < 0)
            {
                var_a2 = count - 1;
            }

            word = var_a2;
            tmp3 = (temp_v1 & 0xFF803FFF);

            tmp3 = tmp3 | ((word & 0x1FF) << 0xE);

            *temp_t0 = tmp3;
            j += 1;
            temp_a3 = j < count;
            var_v1 = 0;
            if (temp_a3 != 0)
            {
                var_v1 = j;
            }
            *temp_t0 = (tmp3 & 0x7FFFFF) | (var_v1 << 0x17);
        } while (temp_a3 != 0);
    }
    return count;
}

/**
 * @brief Count active byte entries in the pad-ctx table at +0x25E0 and rebuild the g_menu_scroll_nav_entries circular nav list to that size.
 * @return Number of entries counted (also the nav-list length).
 * @see decomp.me (100%)
 */
s32 menu_build_key_item_nav_entries(void)
{
    s32 temp_a1;
    s32 temp_a3;
    s32 temp_v1;
    s32 var_a2;
    s32 var_v1;
    s32* temp_t0;

    s32 tmp;
    s32 tmp2;
    s32 tmp3;

    s32 count;
    s32 i;
    s32 j;
    u8* p;

    count = 0;
    ((u8*)g_pad_ctx)[0x26DF] = 0;
    p = (u8*)g_pad_ctx + 0x25E0;
    i = 0xFF;
    do
    {
        if (*p != 0)
        {
            count += 1;
        }
        i -= 1;
        p += 1;
    } while (i >= 0);

    g_menu_scroll_nav_entries[0] = 0;
    j = 0;
    if (count > 0)
    {
        do
        {
        do
        {
            temp_t0 = (j) + (s32*)&g_menu_scroll_nav_entries;

            tmp = *temp_t0;
            var_a2 = j - 1;

            temp_v1 = (tmp & ~0x3FFF);

            tmp2 = (j * 0x10);
            tmp2 = tmp2 & 0x3FFF;

            temp_v1 = temp_v1 | tmp2;
            *temp_t0 = temp_v1;

            if (var_a2 < 0)
            {
                var_a2 = count - 1;
            }

            tmp3 = (temp_v1 & 0xFF803FFF);

            tmp3 = tmp3 | ((var_a2 & 0x1FF) << 0xE);

            *temp_t0 = tmp3;
            temp_a1 = j + 1;
            temp_a3 = temp_a1 < count;
            var_v1 = 0;
            if (temp_a3 != 0)
            {
                var_v1 = temp_a1;
            }
            *temp_t0 = (tmp3 & 0x7FFFFF) | (var_v1 << 0x17);
            j = temp_a1;
        } while (temp_a3 != 0);
        } while (0);
    }
    return count;
}

/**
 * @brief Count entries with bit 0 set in the pad-ctx table at +0x2F0 and rebuild the g_menu_scroll_nav_entries circular nav list to that size.
 * @return Number of entries counted (also the nav-list length).
 * @see decomp.me (100%)
 */
s32 menu_build_ability_nav_entries(void)
{
    s32 temp_a1;
    s32 temp_a3;
    s32 temp_v1;
    s32 var_a2;
    s32 var_v1;
    s32* temp_t0;

    s32 tmp;
    s32 tmp2;
    s32 tmp3;

    s32 count;
    s32 i;
    s32 j;
    u8* p;

    count = 0;
    p = (u8*)g_pad_ctx + 0x2F0;
    i = 0x3F;
    do
    {
        if (*p & 1)
        {
            count += 1;
        }
        i -= 1;
        p += 0xC;
    } while (i >= 0);

    g_menu_scroll_nav_entries[0] = 0;
    j = 0;
    if (count > 0)
    {
        do
        {
        do
        {
            temp_t0 = (j) + (s32*)&g_menu_scroll_nav_entries;

            tmp = *temp_t0;
            var_a2 = j - 1;

            temp_v1 = (tmp & ~0x3FFF);

            tmp2 = (j * 0x10);
            tmp2 = tmp2 & 0x3FFF;

            temp_v1 = temp_v1 | tmp2;
            *temp_t0 = temp_v1;

            if (var_a2 < 0)
            {
                var_a2 = count - 1;
            }

            tmp3 = (temp_v1 & 0xFF803FFF);

            tmp3 = tmp3 | ((var_a2 & 0x1FF) << 0xE);

            *temp_t0 = tmp3;
            temp_a1 = j + 1;
            temp_a3 = temp_a1 < count;
            var_v1 = 0;
            if (temp_a3 != 0)
            {
                var_v1 = temp_a1;
            }
            *temp_t0 = (tmp3 & 0x7FFFFF) | (var_v1 << 0x17);
            j = temp_a1;
        } while (temp_a3 != 0);
        } while (0);
    }
    return count;
}



/** @brief Return non-zero when two 0x40-byte item records differ. */
static inline s32 buffers_differ(u8* t, u8* p)
{
    u32 i = 0;
    do
    {
        i += 1;
        if (*t != *p)
            return 1;
        t += 1;
        p += 1;
    } while (i < 0x40);
    return 0;
}

/* Matching helper: preserves the original six-argument outgoing stack area. */
extern s32 menu_stage_stack_shape(s32, s32, s32, s32, s32, s32) __attribute__((const));

/**
 * @brief Stage the best slot-0 equipment candidate for comparison.
 * @return 1 when a candidate was staged, otherwise 0.
 * @see decomp.me (100%) https://decomp.me/scratch/cUQBA
 */
s32 menu_stage_best_equipment_for_slot0(void)
{
    u8 buf[0x40];
    u8* entry;
    u8* p;
    u8* slot_buf;
    s32 diff;
    PadContext* ctx;

    menu_stage_stack_shape(0, 0, 0, 0, 0, 0);

    entry = (u8*)menu_find_best_equipment_for_slot0();
    if (entry != 0)
    {
        p = (u8*)((g_menu_char_slot * 0x250) + (s32)g_pad_ctx) + 0x640;
        diff = buffers_differ(D_800F0BF8, p);
        if (diff == 0)
        {
            ctx = g_pad_ctx;
            func_800A8F8C((u8*)((g_menu_char_slot * 0x250) + (s32)ctx) + 0x640, entry);
            *entry = 0;
            g_item_slot_data.slot0 = 0;
        }
        else
        {
            slot_buf = (u8*)((g_menu_char_slot * 0x250) + (s32)g_pad_ctx) + 0x640;
            func_800A8F8C(buf, slot_buf);
            func_800A8F8C(slot_buf, entry);
            func_800A8F8C(entry, buf);
            g_item_slot_data.slot0 = (u32)entry;
        }
        g_item_slot_flags.slot0 = 1;
        return 1;
    }
    return 0;
}

/**
 * @brief Select the highest-valued eligible item record from the pad-context item table.
 * @return Pointer to the selected 0x40-byte record, or NULL if no eligible record exists.
 * @see decomp.me (100%) https://decomp.me/scratch/hQoB8
 */
void* menu_find_best_equipment_for_slot0(void)
{
    s32 var_a0;
    s32 var_a0_2;
    s32 var_a1;
    s32 temp_v1_3;
    s32 var_t1;
    u32 temp_v1;
    u32 temp_v1_2;
    u8* var_v0;
    u8* temp_v0;
    u8* var_a2;
    u8* var_a3;
    u8* var_t0;

    var_t1 = 0;
    temp_v0 = (u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0);
    var_t0 = temp_v0 + 0x50;
    if (*(u8*)(temp_v0 + 0x50) != 0)
    {
        var_t1 = *(u16*)(var_t0 + 0x24);
    }
    var_a3 = (u8*)g_pad_ctx + 0xCE0;
    var_t0 = 0;
    var_a1 = 0;
    var_a2 = (u8*)g_menu_equipment_base;
    var_a0 = 0;
    do
    {
        if ((var_a0 != 0) && (*var_a2 != 0))
        {
            temp_v1_2 = *(u32*)(var_a2 + 0x14);
            temp_v1 = temp_v1_2;
            if (temp_v1 & 0x300)
            {
                var_v0 = ((temp_v1 >> 0xA) & 0x3F) + D_800F0BEC;
            }
            else
            {
                var_v0 = ((temp_v1 >> 0xA) & 0x3F) + D_800F0BE0;
            }
            var_a1 ^= temp_v1_2;
            var_a1 ^= temp_v1_2;
            var_a1 |= *var_v0;
        }
        var_a0 ^= var_a1;
        var_a0 ^= var_a1;
        var_a0 += 1;
        var_a2 += 0x40;
    } while (var_a0 < 4);
    var_a0_2 = 0;
    do
    {
        if (*var_a3 != 0)
        {
            temp_v1_2 = *(u32*)(var_a3 + 0x14);
            if (!(temp_v1_2 & 0x300) && !(var_a1 & D_800F0BE0[(temp_v1_2 >> 0xA) & 0x3F]))
            {
                temp_v1_3 = *(u16*)(var_a3 + 0x24);
                if (var_t1 < temp_v1_3)
                {
                    var_t0 = var_a3;
                    var_t1 = temp_v1_3;
                }
            }
        }
        var_a0_2 += 1;
        var_a3 += 0x40;
    } while (var_a0_2 < 0x64);
    return var_t0;
}


/**
 * @brief Commit the pending item into the active character's slot buffer for the CURRENT menu subtype (@ref g_menu_active_subtype).
 * @return 1 if a record was committed or exchanged, 0 if menu_find_best_equipment_for_active_slot failed.
 * @see decomp.me (100%)
 */
s32 menu_stage_best_equipment_for_active_slot(void)
{
    u8 buf[0x40];
    u8* entry;
    u8* slot_buf;
    u32* slots;
    u8* pad;
    s32 ret;
    s32 off;
    s32 i;

    if (0)
    {
        func_800A8F8C(0, 0, 0, 0, 0, 0);
    }
    entry = (u8*)menu_find_best_equipment_for_active_slot();
    if (entry != 0)
    {
        i = 1;
        off = g_menu_active_subtype - 7;
        pad = (u8*)g_pad_ctx;
        if (*(pad - (-((off << 6) + (g_menu_char_slot * 0x250))) + 0x640) == 0)
        {
            slots = &g_item_slot_data.slot0;
            do
            {
                if ((u32)entry == slots[i])
                {
                    goto found_a;
                }
                i += 1;
            } while (i < 4);
done_a:
            func_800A8F8C((u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0) + ((g_menu_active_subtype << 6) - 0x170), entry);
            *entry = 0;
            g_item_slot_data_by_subtype[g_menu_active_subtype] = 0;
            goto tail;
found_a:
            slots[i] = (u32)((u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0) + ((g_menu_active_subtype << 6) - 0x170));
            goto done_a;
        }
        else
        {
            goto scan_b;
found_b:
            slots[i] = (u32)((u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0) + ((g_menu_active_subtype << 6) - 0x170));
            goto done_b;
scan_b:
            slots = &g_item_slot_data.slot0;
            do
            {
                if ((u32)entry == slots[i])
                {
                    goto found_b;
                }
                i += 1;
            } while (i < 4);
done_b:
            slot_buf = (u8*)((g_menu_char_slot * 0x250) + (s32)g_pad_ctx) + (g_menu_active_subtype << 6) + 0x480;
            func_800A8F8C(buf, slot_buf);
            func_800A8F8C(slot_buf, entry);
            func_800A8F8C(entry, buf);
            g_item_slot_data_by_subtype[g_menu_active_subtype] = (u32)entry;
        }
        ret = 1;
tail:
        g_item_slot_flags_by_subtype[g_menu_active_subtype] = ret;
        return 1;
    }
    return 0;
}

/**
 * @brief Pick the highest-valued eligible item record for the CURRENT menu subtype.
 * @return Pointer to the winning 0x40-byte record, or NULL if none qualifies.
 * @see decomp.me (100%)
 */
void* menu_find_best_equipment_for_active_slot(void)
{
    s32 slot_idx;
    s32 rec_idx;
    s32 mask;
    s32 total;
    s32 best_total;
    u32 slot_flags;
    u32 rec_flags;
    u8* category;
    u8* char_base;
    u8* slot;
    u8* rec_flag;
    u8* rec;
    u8* best;

    char_base = (u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0);
    best = char_base + ((g_menu_active_subtype << 6) - 0x170);
    if (*best == 0)
    {
        best_total = 0;
    }
    else
    {
        best_total = *(u16*)(best + 0x24) + *(u16*)(best + 0x26) + *(u16*)(best + 0x28) + *(u16*)(best + 0x2A);
    }
    best = 0;
    mask = 0;
    slot_idx = 0;
    slot = (u8*)g_menu_equipment_base;
    rec_flag = (u8*)g_pad_ctx + 0xCE0;
    do
    {
        if ((slot_idx != (g_menu_active_subtype - 7)) && (*slot != 0))
        {
            slot_flags = *(u32*)(slot + 0x14);
            if (slot_flags & 0x300)
            {
                category = ((slot_flags >> 0xA) & 0x3F) + D_800F0BEC;
            }
            else
            {
                category = ((slot_flags >> 0xA) & 0x3F) + D_800F0BE0;
            }
            mask |= *category;
        }
        slot_idx += 1;
        slot += 0x40;
    } while (slot_idx < 4);
    slot_idx = mask;
    rec_idx = 0;
    rec = rec_flag;
    do
    {
        if (*rec_flag != 0)
        {
            rec_flags = *(u32*)(rec + 0x14);
            if (((rec_flags & 0x300) == 0x100) && !(slot_idx & D_800F0BEC[(rec_flags >> 0xA) & 0x3F]))
            {
                total = *(u16*)(rec + 0x24) + *(u16*)(rec + 0x26) + *(u16*)(rec + 0x28) + *(u16*)(rec + 0x2A);
                if (best_total < total)
                {
                    best = rec;
                    mask = total;
                    best_total = mask;
                }
            }
        }
        rec_idx += 1;
        rec += 0x40;
        rec_flag += 0x40;
    } while (rec_idx < 0x64);
    return best;
}

/**
 * @brief Play a menu sound effect, unless a menu script is currently driving input.
 * @param sound_id Sound effect ID (see the MENU_SE_ constants in menu.c).
 * @param volume Playback volume (menu callers always pass 0x80).
 * @see decomp.me (100%)
 */
void menu_play_se(s32 sound_id, s32 volume)
{
    if (g_active_script == 0)
    {
        func_800A3938(sound_id, volume);
    }
}

/**
 * @brief Count the in-use entries in the 100-slot record table at g_pad_ctx+0xCE0.
 * @return Index of the first empty (zero first byte) record, i.e.
 * @see decomp.me (100%)
 */
s32 menu_count_inventory_items(void)
{
    s32 count;
    u8* rec;

    rec = (u8*)g_pad_ctx + 0xCE0;
    for (count = 0; count < 0x64; count++)
    {
        if (*rec == 0)
        {
            break;
        }
        rec += 0x40;
    }
    return count;
}

/**
 * @brief Draw a number through func_800A8A78, clamped to a maximum of 99.
 * @param prim Primitive buffer write cursor.
 * @param cursor Glyph write cursor, forwarded unchanged.
 * @param value Number to draw; anything >= 100 is drawn as 99.
 * @param arg3 Forwarded unchanged.
 * @param origin Viewport anchor, forwarded unchanged.
 * @param color Palette index, forwarded unchanged.
 * @see decomp.me (100%)
 */
void menu_draw_clamped_number(s32 prim, s32 cursor, s32 value, s32 arg3, Vec2s* origin, s32 color)
{
    if (value >= 0x64)
    {
        value = 0x63;
    }
    func_800A8A78(prim, cursor, value, arg3, origin, color);
}

/** @brief SwCARD completion-event descriptor. */
extern s32 g_card_sw_io_event;
/** @brief SwCARD error-event descriptor. */
extern s32 g_card_sw_error_event;
/** @brief SwCARD timeout-event descriptor. */
extern s32 g_card_sw_timeout_event;
/** @brief SwCARD new-card event descriptor. */
extern s32 g_card_sw_new_event;
/** @brief HwCARD completion-event descriptor. */
extern s32 g_card_hw_io_event;
/** @brief HwCARD error-event descriptor. */
extern s32 g_card_hw_error_event;
/** @brief HwCARD timeout-event descriptor. */
extern s32 g_card_hw_timeout_event;
/** @brief HwCARD new-card event descriptor. */
extern s32 g_card_hw_new_event;

/**
 * @brief Open and enable the eight memory-card events used by the save/load menu.
 * @see decomp.me (100%)
 */
void memory_card_open_events(void)
{
    reset_controller_vsync_state();
    EnterCriticalSection();
    g_card_sw_io_event = OpenEvent(SwCARD, EvSpIOE, EvMdNOINTR, NULL);
    g_card_sw_error_event = OpenEvent(SwCARD, EvSpERROR, EvMdNOINTR, NULL);
    g_card_sw_timeout_event = OpenEvent(SwCARD, EvSpTIMOUT, EvMdNOINTR, NULL);
    g_card_sw_new_event = OpenEvent(SwCARD, EvSpNEW, EvMdNOINTR, NULL);
    g_card_hw_io_event = OpenEvent(HwCARD, EvSpIOE, EvMdNOINTR, NULL);
    g_card_hw_error_event = OpenEvent(HwCARD, EvSpERROR, EvMdNOINTR, NULL);
    g_card_hw_timeout_event = OpenEvent(HwCARD, EvSpTIMOUT, EvMdNOINTR, NULL);
    g_card_hw_new_event = OpenEvent(HwCARD, EvSpNEW, EvMdNOINTR, NULL);
    EnableEvent(g_card_sw_io_event);
    EnableEvent(g_card_sw_error_event);
    EnableEvent(g_card_sw_timeout_event);
    EnableEvent(g_card_sw_new_event);
    EnableEvent(g_card_hw_io_event);
    EnableEvent(g_card_hw_error_event);
    EnableEvent(g_card_hw_timeout_event);
    EnableEvent(g_card_hw_new_event);
    ExitCriticalSection();
}

/**
 * @brief Close the eight memory-card events opened by memory_card_open_events.
 * @see decomp.me (100%)
 */
void memory_card_close_events(void)
{
    reset_controller_vsync_state();
    EnterCriticalSection();
    CloseEvent(g_card_sw_io_event);
    CloseEvent(g_card_sw_error_event);
    CloseEvent(g_card_sw_timeout_event);
    CloseEvent(g_card_sw_new_event);
    CloseEvent(g_card_hw_io_event);
    CloseEvent(g_card_hw_error_event);
    CloseEvent(g_card_hw_timeout_event);
    CloseEvent(g_card_hw_new_event);
    ExitCriticalSection();
}

/** @brief Memory-card slot 1 device path, "bu00:". */
extern char g_card_slot1_path[];
/** @brief Directory entries filled while scanning memory-card slot 1. */
extern struct DIRENTRY g_card_dir_entries[];
/** @brief Number of directory entries found in memory-card slot 1. */
extern s32 g_card_file_count;

/**
 * @brief Scan memory card slot 1 and record how many save files it holds.
 * @see decomp.me (100%)
 */
void memory_card_scan_slot1_files(void)
{
    g_card_file_count = 0;
    g_card_file_count = memory_card_scan_files(g_card_slot1_path, g_card_dir_entries);
}

/**
 * @brief Bring memory card slot 1 up to a usable state, formatting it if needed.
 * @return 1 if the card is ready for use, 0 if it was rejected up front or the format attempt failed.
 * @see decomp.me (100%)
 */
s32 memory_card_prepare_slot1(void)
{
    s32 status;

    _card_info(0);
    status = memory_card_wait_software_event();
    if ((status == 1) || (status == 2))
    {
        return 0;
    }
    if (status == 3)
    {
        memory_card_clear_hardware_events();
        _card_clear(0);
        memory_card_wait_hardware_event();
    }
    memory_card_clear_software_events();
    _card_load(0);
    if (memory_card_wait_software_event() == 3)
    {
        if (_card_format(0) == 0)
        {
            return 0;
        }
    }
    return 1;
}

/** @brief Test-save path, "bu00:HAND". */
extern char g_card_save_path[];
/** @brief Shared buffer used to assemble and write a memory-card save block. */
extern u8 g_card_work_buffer[];

/**
 * @brief Populate the card work buffer and write it as "bu00:HAND".
 * @see decomp.me (100%)
 */
void memory_card_write_test_save(void)
{
    memory_card_fill_test_data(g_card_work_buffer);
    memory_card_create_save_file(g_card_save_path, g_card_work_buffer);
}

/**
 * @brief Block until one of the four SwCARD events fires and report which.
 * @return 0 for EvSpIOE (operation completed), 1 for EvSpERROR, 2 for EvSpTIMOUT, 3 for EvSpNEW (card newly inserted / unformatted).
 * @see decomp.me (100%)
 */
s32 memory_card_wait_software_event(void)
{
    for (;;)
    {
        if (TestEvent(g_card_sw_io_event) == 1)
        {
            return 0;
        }
        if (TestEvent(g_card_sw_error_event) == 1)
        {
            return 1;
        }
        if (TestEvent(g_card_sw_timeout_event) == 1)
        {
            return 2;
        }
        if (TestEvent(g_card_sw_new_event) == 1)
        {
            return 3;
        }
    }
}

/**
 * @brief Drain the four SwCARD events by testing each one once.
 * @see decomp.me (100%)
 */
void memory_card_clear_software_events(void)
{
    TestEvent(g_card_sw_io_event);
    TestEvent(g_card_sw_error_event);
    TestEvent(g_card_sw_timeout_event);
    TestEvent(g_card_sw_new_event);
}

/**
 * @brief Block until one of the four HwCARD events fires and report which.
 * @return 0 for EvSpIOE (operation completed), 1 for EvSpERROR, 2 for EvSpTIMOUT, 3 for EvSpNEW (card newly inserted / unformatted).
 * @see decomp.me (100%)
 */
s32 memory_card_wait_hardware_event(void)
{
    for (;;)
    {
        if (TestEvent(g_card_hw_io_event) == 1)
        {
            return 0;
        }
        if (TestEvent(g_card_hw_error_event) == 1)
        {
            return 1;
        }
        if (TestEvent(g_card_hw_timeout_event) == 1)
        {
            return 2;
        }
        if (TestEvent(g_card_hw_new_event) == 1)
        {
            return 3;
        }
    }
}

/**
 * @brief Drain the four HwCARD events by testing each one once.
 * @see decomp.me (100%)
 */
void memory_card_clear_hardware_events(void)
{
    TestEvent(g_card_hw_io_event);
    TestEvent(g_card_hw_error_event);
    TestEvent(g_card_hw_timeout_event);
    TestEvent(g_card_hw_new_event);
}

extern char g_card_wildcard[];

/**
 * @brief Count the files on a memory card matching a path prefix.
 * @param path Memory-card path prefix; the wildcard suffix is appended internally.
 * @param entry Start of the caller's directory-entry table; one struct DIRENTRY is filled per file found, so it must have room for every match.
 * @return Number of files found; 0 if the card holds no match at all.
 * @see decomp.me (100%)
 */
s32 memory_card_scan_files(char* path, struct DIRENTRY* entry)
{
    char pattern[0x80];
    s32 count;

    strcpy(pattern, path);
    strcat(pattern, g_card_wildcard);
    count = 0;
    if (firstfile(pattern, entry) == entry)
    {
        do
        {
            count += 1;
            entry += 1;
        } while (nextfile(entry) == entry);
    }
    return count;
}

/**
 * @brief Read block 0 of a memory card and report whether it is formatted.
 * @param chan Card channel / slot to probe, passed straight to _card_read.
 * @return 1 if the card is formatted, 0 if the "MC" magic is absent, -1 if the event poll reported anything other than completion.
 * @see decomp.me (100%) https://decomp.me/scratch/lhdFU
 */
s32 memory_card_check_formatted(s32 chan)
{
    u8 header[0x80];
    s32 status;
    s32 one;
    s32 *ioPtr;
    s32 *errPtr;

    bzero(header, 0x80);
    TestEvent(g_card_hw_io_event);
    TestEvent(g_card_hw_error_event);
    TestEvent(g_card_hw_timeout_event);
    TestEvent(g_card_hw_new_event);
    _new_card();
    _card_read(chan, 0, header);

    while (1)
    {
        ioPtr = &g_card_hw_io_event;
        one = 1;
        errPtr = &g_card_hw_error_event;
        status = 3;
        if (TestEvent(*ioPtr) == one)
        {
            status = 0;
            break;
        }
        if (TestEvent(*errPtr) == one)
        {
            status = 1;
            break;
        }
        if (TestEvent(g_card_hw_timeout_event) == one)
        {
            status = 2;
            break;
        }
        if (TestEvent(g_card_hw_new_event) == one)
        {
            break;
        }
    }
    if (status != 0)
    {
        return -1;
    }

    if ((header[0] == 'M') && (header[1] == 'C'))
    {
        return 1;
    }

    return 0;
}

extern u8 g_card_save_title_sjis[];
extern u8 g_card_header[];
extern u8 g_card_header_block_count;

#define MEMORY_CARD_HEADER_TYPE_THREE_ICONS 0x13
#define MEMORY_CARD_HEADER_TITLE_OFFSET 0x04
#define MEMORY_CARD_SAVE_TITLE_SIZE 0x11
#define MEMORY_CARD_HEADER_PADDING_OFFSET 0x44
#define MEMORY_CARD_HEADER_PADDING_SIZE 0x1C
#define MEMORY_CARD_FILE_HEADER_SIZE 0x200
#define MEMORY_CARD_OPEN_BLOCK_COUNT_SHIFT 16
#define MEMORY_CARD_OPEN_CREATE_FLAG 0x200
#define MEMORY_CARD_OPEN_WRITE_FLAG 0x02

/**
 * @brief Create a one-block memory-card save file and write its data.
 * @param name Memory-card file path to create.
 * @param buf 8 KiB save buffer; the generated header replaces its first 512 bytes.
 * @return 1 if the complete file is written; otherwise 0.
 * @see decomp.me (100%) https://decomp.me/scratch/ljqxf
 */
s32 memory_card_create_save_file(char* name, void* buf)
{
    s32 blocks;
    s32 fd;
    s32 size;

    /* Build the 512-byte memory-card file header. */
    g_card_header[0] = 'S';
    g_card_header[1] = 'C';
    g_card_header[2] = MEMORY_CARD_HEADER_TYPE_THREE_ICONS;
    blocks = 1;
    g_card_header[3] = blocks;
    memcpy(&g_card_header[MEMORY_CARD_HEADER_TITLE_OFFSET], g_card_save_title_sjis, MEMORY_CARD_SAVE_TITLE_SIZE);
    bzero(&g_card_header[MEMORY_CARD_HEADER_PADDING_OFFSET], MEMORY_CARD_HEADER_PADDING_SIZE);
    memcpy(buf, g_card_header, MEMORY_CARD_FILE_HEADER_SIZE);

    /* Allocate one card block, then reopen the file for writing. */
    fd = open(
        name,
        (g_card_header_block_count << MEMORY_CARD_OPEN_BLOCK_COUNT_SHIFT) | MEMORY_CARD_OPEN_CREATE_FLAG);
    if (fd == -1)
    {
        return 0;
    }
    close(fd);
    fd = open(name, MEMORY_CARD_OPEN_WRITE_FLAG);
    if (fd == -1)
    {
        return 0;
    }

    /* Each memory-card block contains 8 KiB. */
    size = blocks << 13;
    if (write(fd, buf, size) != size)
    {
        close(fd);
        return 0;
    }
    close(fd);
    return 1;
}

extern u8 g_card_test_payload[];

/**
 * @brief Copy the five-byte test payload into a save buffer.
 * @param buf Destination buffer with space for at least five bytes.
 * @see decomp.me (100%)
 */
void memory_card_fill_test_data(void* buf)
{
    memcpy(buf, g_card_test_payload, 5);
}
