#include "menu.h"

/* ----- Macros ----- */

/*
 * CLUT ids for the menu's chrome glyphs/sprites. These are libgpu @c getClut
 * results; see @ref menu_upload_tim for where the two CLUT rows are placed in
 * VRAM. Each row is 256 entries wide and breaks into 16 sub-palettes of 16
 * colors (CLUT slots are 16-pixel aligned in x).
 *
 *   MENU_CLUT_GRID_BASE -> CLUT 0, slot 0  (VRAM x=0,   y=498)
 *   MENU_CLUT_GRID_ALT  -> CLUT 0, slot 1  (VRAM x=16,  y=498)
 *   MENU_CLUT_CORNER    -> CLUT 1, slot 10 (VRAM x=160, y=499)
 */
#define MENU_CLUT_GRID_BASE 0x7C80
#define MENU_CLUT_GRID_ALT 0x7C81
#define MENU_CLUT_CORNER 0x7CCA

/*
 * Packed texture-window / UV origin constants for the menu window chrome.
 * Format: bits 15..8 = VRAM v (y), bits 7..0 = VRAM u (x).
 * The chrome tiles live in a tilesheet at u=0xD0..0xD8, v=0x70..0x90
 * (8 px per tile). The interior fill uses a separate region at u=v=0xA0.
 *
 *   v=0x70: corner row (TL at u=0xD0, TR at u=0xD8)
 *   v=0x78: corner row (BL at u=0xD0, BR at u=0xD8)
 *   v=0x80: top h-edge  (16x8 tile, single column)
 *   v=0x88: bot h-edge  (16x8 tile, single column)
 *   v=0x90: v-edge row  (left at u=0xD0, right at u=0xD8, each 8x16)
 */
/*
 * Interior fill tint: r=0x80, g=0x80, b=0x00, code=0x80 as the initial
 * packed word. The code byte is immediately overwritten by setcode(0x64),
 * leaving a warm/yellow-tinted SPRT distinct from GPU_TINT_NEUTRAL (all
 * channels equal). Passed to SET_BGR0_PACKED so the store is one word write.
 */
#define MENU_TINT_FILL 0x80008080U

#define MENU_TW_CORNER_TL 0x70D0
#define MENU_TW_CORNER_TR 0x70D8
#define MENU_TW_CORNER_BL 0x78D0
#define MENU_TW_CORNER_BR 0x78D8
#define MENU_TW_EDGE_TOP 0x80D0
#define MENU_TW_EDGE_BOT 0x88D0
#define MENU_TW_EDGE_LEFT 0x90D0
#define MENU_TW_EDGE_RIGHT 0x90D8
#define MENU_TW_FILL 0xA0A0

/*
 * VRAM layout for the three menu window slots' primitive data.
 * Each slot has two regions:
 *   Strip: 16 halfwords wide x 1 scanline at x=272, y=472/473/474
 *          (cursor/highlight bar, just above the CLUT rows at y=498)
 *   Block: 12 halfwords wide x 48 scanlines at x=1012 (slots 0-1) or x=1000 (slot 2),
 *          y=288 (slot 0) or y=336 (slots 1-2) (main content texture block)
 * Source data stride between slots in g_prim_rect_buf: 0x4A0 bytes.
 */
#define PRIM_STRIP_VRAM_X 0x110    /* 272  - VRAM column                    */
#define PRIM_STRIP_VRAM_Y0 0x1D8   /* 472  - VRAM row for slot 0            */
#define PRIM_STRIP_W 0x10          /* 16 halfwords wide                     */
#define PRIM_STRIP_H 1             /* 1 scanline tall                       */
#define PRIM_BLOCK_VRAM_X 0x3F4    /* 1012 - VRAM column for slots 0 and 1  */
#define PRIM_BLOCK_VRAM_X2 0x3E8   /* 1000 - VRAM column for slot 2         */
#define PRIM_BLOCK_VRAM_Y0 0x120   /* 288  - VRAM row for slot 0            */
#define PRIM_BLOCK_VRAM_Y1 0x150   /* 336  - VRAM row for slots 1 and 2     */
#define PRIM_BLOCK_W 0xC           /* 12 halfwords wide                     */
#define PRIM_BLOCK_H 0x30          /* 48 scanlines tall                     */
#define PRIM_SLOT_STRIDE 0x4A0     /* 1184 bytes per slot                   */
#define PRIM_BLOCK_BUF_OFFSET 0x20 /* 32 bytes into each slot               */

/*
 * Node / scroll / layout constants
 */
/** @brief Total number of nodes in g_menu_nodes[]. */
#define MENU_NODE_COUNT 0x2C
/** @brief Bits [14:8] of idx_nav.nav_x_packed: the 7-bit column (nav_x) field. */
#define MENU_NAV_X_MASK 0x7F00
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
/** @brief Minimum Y for g_content_cursor_y within the content sub-window (12 px). */
#define MENU_CURSOR_Y_MIN 0x0C
/** @brief Maximum Y for g_content_cursor_y within the content sub-window (163 px). */
#define MENU_CURSOR_Y_MAX 0xA3
/** @brief Frames to suppress cursor highlight after opening a content view. */
#define MENU_CURSOR_REVEAL_DELAY 5
/** @brief g_menu_redraw_state: navigation key pressed, scroll position adjusted. */
#define MENU_REDRAW_NAVIGATE 6
/** @brief g_menu_redraw_state: layout pass completed (position change or first run). */
#define MENU_REDRAW_LAYOUT 8

/*
 * Sound effect IDs -- passed as first arg to func_8014F210 (menu_play_se).
 * Second arg is always MENU_SE_VOLUME.
 */
/** @brief Scroll navigation sound (D-up / D-down / Circle to scroll). */
#define MENU_SE_NAVIGATE 0x7D
/** @brief Open / select sound (Circle or D-right to enter a node). */
#define MENU_SE_SELECT 0x7E
/** @brief Close / cancel sound (Circle while at MENU_NODE_BROWSE_ALL). */
#define MENU_SE_CLOSE 0x7F
/** @brief Full volume level for all menu sound effects (128). */
#define MENU_SE_VOLUME 0x80

/* ----- Types ----- */

typedef struct MenuFrameCtx
{
    u8 pad0[0x34];
    u8 ot_base;       /* 0x0034 - start of the ordering-table buffer */
    u8 pad35[0x400B]; /* padding to 0x4040 */
    s32 prim_cursor;  /* 0x4040 - current primitive write cursor (pointer stored as s32) */
    u8 pad4044[8];    /* padding to 0x404C */
    s32 draw_buf_idx; /* 0x404C - display buffer page index (0 or 1, used for double-buffer flip) */
} MenuFrameCtx;

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

/**
 * @brief Pair of (u8) indices into a packed text-lookup table.
 *
 * @c entry is the character/entry index within the page; @c page is the page
 * index. Together they form a string pointer via
 * @c entry + ((page << 8) + base_ptr). See @ref menu_draw_label.
 */
typedef struct
{
    u8 entry; /**< Entry index within the page. */
    u8 page;  /**< Page index. */
} StringTableKey;

typedef struct
{
    u8 unk0;
    u8 state; /**< Node state: 0 = uninitialized, 4 = position assigned by menu_layout_node. */
    union
    {
        u16 unk2; /**< Full 16-bit word: low byte = flags, high byte = parent_idx. */
        struct
        {
            u8 flags;      /**< Bit 0: node active/enabled in layout. Bit 1: node expanded (children shown). */
            u8 parent_idx; /**< Index of parent node in g_menu_nodes, or MENU_NONE (0xFF) for root nodes. */
        } s;
    } u2;
    u8 unk4;
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

/**
 * @brief Four u32 pointers to item data for each comparison slot.
 * @note Used alongside g_item_slot_flags; each element maps to g_item_slot_flags's parallel flag.
 */
typedef struct
{
    u32 unk0; /**< Slot 0 data pointer. */
    u32 unk4; /**< Slot 1 data pointer. */
    u32 unk8; /**< Slot 2 data pointer. */
    u32 unkC; /**< Slot 3 data pointer. */
} ItemSlotData;

/**
 * @brief Four u8 flags indicating which item comparison slots are occupied (nonzero = active).
 * @note Parallel to g_item_slot_data; checked in func_80145608 before reading slot data.
 */
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
    u8 pad[5];
} MenuContentItem;

/* ----- Forward declarations ----- */

/* K&R-style declaration: original call site in menu_tick passes no explicit
 * argument and relies on register a0 (the caller's first parameter) being
 * live. Keep the empty parameter list to preserve that codegen exactly. */
void menu_build_grid();

/* ----- Extern globals ----- */

/** @brief Pending overlay element to emit at end of @ref menu_update_slots (0 = none). */
extern s32 g_menu_pending_overlay;
/** @brief When non-zero, cursor highlight is enabled for the active slot. */
extern s32 g_menu_cursor_enable;
/** @brief Set non-zero by a content callback to abort @ref menu_draw_window early. */
extern s32 g_menu_draw_early_out;
/** @brief Base address of the double-buffered DRAWENV array. */
extern s32 g_draw_buf_base;
/** @brief When non-zero, suppresses cursor highlight even on the active slot. */
extern s32 g_menu_suppress_cursor;
/** @brief Scene/language selector used in window title decoration layout switches. */
extern s32 g_menu_scene_type;

extern MenuNode g_menu_nodes[0x2C];
extern u8 g_menu_prev_node;
/** @brief Gate flag for func_80148A20: 0 = draw empty slot, nonzero = full item render. */
extern s32 g_menu_content_ready;

extern ItemSlotData g_item_slot_data;
extern ItemSlotFlags g_item_slot_flags;

/** @brief Pointer into g_pad_ctx item data for the current category; null = no items. */
extern s32 g_menu_item_ptr;
extern s32 D_80169410;
extern s32 D_80169404;
extern s32 D_80169408;
extern s32 D_8016911C;
extern s32 D_80169554;
extern s32 D_801694B0;
extern s32 D_801690B8[];
extern void* D_801693FC;
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
/** @brief Set to 0 before each content-load call; written back by func_8014E3C4. */
extern s8 D_801690F9;
/** @brief Node index for the companion character's stat page (0x2B = companion present, 0xFF = none). */
extern s8 g_menu_companion_node;

extern StringTableKey g_menu_label_key_a; /* 0x800EC3DA: offset +0x16 into shared text table at 0x800EC3C4 */
extern StringTableKey g_menu_label_key_b; /* 0x800EC3E4: offset +0x20 into shared text table at 0x800EC3C4 */

/** @brief Number of nodes in the linear navigation list. */
extern s32 g_menu_nav_count;
/** @brief Node ID at the start of the navigation list; used for wrap-around on down-navigation. */
extern s32 g_menu_nav_first;
/** @brief Y display coordinate for the content viewport origin. */
extern s32 g_content_view_y;
/** @brief Set to 1 to request an overlay/scene load at end of this input frame. */
extern s32 g_menu_load_request;
extern s32 D_80168C10;
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
 * decomp.me (100%) https://decomp.me/scratch/Dv8qB
 */
void menu_init(void)
{
    volatile u8 padding;
    menu_clear_vram();
    menu_state_init();
    menu_reset_slots();
    g_active_slot = -1;
    func_800AA02C();
    g_menu_unk_e8 = 0;
    menu_init_prim_rects();
    g_menu_frame = 0;
    g_script_cursor = 0;
    menu_node_tree_init();
}

/**
 * @brief Upload primitive-rectangle pixel data for the three menu window slots to VRAM.
 *
 * Iterates over three menu slots (0-2). For each slot, two @c LoadImage calls are made:
 *   1. A 16hword x 1-scanline cursor/highlight strip to a fixed VRAM row just above
 *      the CLUT region (y = @c PRIM_STRIP_VRAM_Y0 + slot).
 *   2. A 12hword x 48-scanline content block to a fixed VRAM page position that differs
 *      for the last slot (x = @c PRIM_BLOCK_VRAM_X2) vs the first two (x = @c PRIM_BLOCK_VRAM_X),
 *      and for the first slot (y = @c PRIM_BLOCK_VRAM_Y0) vs the later two (y = @c PRIM_BLOCK_VRAM_Y1).
 *
 * Source data is read from @c g_prim_rect_buf at slot-aligned offsets spaced
 * @c PRIM_SLOT_STRIDE bytes apart; the block data begins @c PRIM_BLOCK_BUF_OFFSET bytes
 * after the strip data within each slot.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/QnGCP
 */
void menu_init_prim_rects(void)
{
    s32 i = 0;

    u8* base = g_prim_rect_buf;

    s32 block_offset = PRIM_BLOCK_BUF_OFFSET;
    s32 strip_offset = 0;
    RECT rect;

    for (i = 0; i < 3; i++)
    {
        /* Upload the 1-scanline cursor strip for slot i. */
        rect.x = PRIM_STRIP_VRAM_X;
        rect.y = i + PRIM_STRIP_VRAM_Y0;
        rect.w = PRIM_STRIP_W;
        rect.h = PRIM_STRIP_H;
        LoadImage(&rect, (u8*)((u32)((strip_offset >> 2) << 2) + (u32)base));

        /* Upload the 12x48 content block for slot i. */
        rect.x = (i == 2) ? PRIM_BLOCK_VRAM_X2 : PRIM_BLOCK_VRAM_X;
        rect.y = (i == 0) ? PRIM_BLOCK_VRAM_Y0 : PRIM_BLOCK_VRAM_Y1;
        rect.w = PRIM_BLOCK_W;
        rect.h = PRIM_BLOCK_H;
        LoadImage(&rect, (u8*)((u32)((block_offset >> 2) << 2) + (u32)base));

        block_offset += PRIM_SLOT_STRIDE;
        strip_offset += PRIM_SLOT_STRIDE;
    };
}

/**
 * @brief Per-frame menu update: emit the grid, advance counters, and run
 *        the scripted-input player.
 *
 * @param gpu_work Per-frame render context; its @c prim_cursor is saved on
 *                 entry and restored before @ref menu_update_slots runs.
 * @see decomp.me (100%) https://decomp.me/scratch/kgN9O
 */
void menu_tick(RenderContext* gpu_work)
{
    s32 v0;
    s32 v1;
    s32 saved_prim_cursor;
    s32 var_s0;
    u16 temp_v1;
    s32 padding[2];

    menu_build_grid(gpu_work);
    v0 = g_menu_frame;
    v1 = g_frame_counter;
    /* RenderContext.prim_cursor - kept as a raw offset load to preserve codegen.
     * Tried gpu_work->prim_cursor (both with and without changing saved_prim_cursor
     * to void*); both broke the match by shifting v0/v1 allocation for the
     * surrounding g_menu_frame/g_frame_counter loads. */
    saved_prim_cursor = *((s32*)(((u8*)gpu_work) + 0x4040));
    g_menu_frame = v0 + 1;
    g_frame_counter = v1 + 1;
    func_800A9E78();

    if ((g_pad_ctx->inject_flags & 0x80) && (g_pad_ctx->inject_enable != 0))
    {
        g_pad_input |= g_pad_input_inject;
    }

    v0 = g_pad_input & MENU_PAD_CONFIRM_CANCEL;
    if (v0)
    {
        g_pad_input = v0;
    }
    v0 = g_pad_input & MENU_PAD_FACE_BUTTONS;
    if (v0)
    {
        g_pad_input = v0;
    }
    v0 = g_pad_input & MENU_PAD_SHOULDERS;
    if (v0)
    {
        g_pad_input = v0;
    }

    if (g_pad_input_latched != 0)
    {
        g_pad_input = 0;
    }
    g_pad_input_latched = g_pad_input;

    if (g_active_script != 0)
    {
        u32 base = (u32)g_script_table;
        u32 off = g_active_script * 48; // Or sizeof(MenuScript), forces 'sll, addu, sll' internally
        s32 idx;

        off += base;           // Accumulates to v1 matching Target 120
        idx = g_script_cursor; // Scheduled perfectly between pointer math

        g_pad_input = 0;

        // Ensure idx is the LHS of addition, emitting 'sll v0' then 'addu v0, v0, v1'
        temp_v1 = *(u16*)(idx * 2 + off);

        if (temp_v1 == (v0 = MENU_SCRIPT_END))
        {
            if (g_active_script < 4)
            {
                var_s0 = 0;
                if (g_script_repeat_count > 0)
                {
                    do
                    {
                        func_8014B69C(1);
                        var_s0++;
                    } while (var_s0 < g_script_repeat_count);
                }
                g_script_repeat_last = g_script_repeat_count;
            }
            g_active_script = 0;
        }
        else
        {
            g_pad_input = (s32)temp_v1;
            g_script_cursor = idx + 1;
        }
    }

    *((s32*)(((u8*)gpu_work) + 0x4040)) = saved_prim_cursor;
    menu_update_slots((MenuFrameCtx*)gpu_work);
}

/**
 * @brief Lay out a run of glyph sprites and link them into an OT chain.
 *
 * @param sprites Array of libgpu @c SPRT primitives (stride 0x14) - both the
 *                working buffer and the function's output.
 * @param ot      OT chain column the sprites are linked into via @c addPrim.
 * @param src     Source text/data copied into the local glyph buffer.
 * @param arg3    TODO: unknown - passed to @ref func_800644FC.
 * @param x       Starting X of the run; pre-shifted left by the total glyph
 *                width when @p mode is 1 or 2 (centering).
 * @param y       Y coordinate of the run.
 * @param len     Source length: element count for the buffer fill and the
 *                index at which the buffer is null-terminated.
 * @param mode    Glyph-width interpretation: 1 = signed halfword,
 *                2 = unsigned halfword (>> 1); other = no width adjustment.
 * @return Pointer just past the run (offset 0x8 of the trailing primitive).
 *
 * @note A @c SPRT (offset 0x4 @c rgbc, 0x8 packed @c (x0,y0), 0x10 signed
 *       @c w) is 0x14 bytes. Retyping @p sprites to @c SPRT* is desirable but
 *       must be verified against the asm - this scratch is not yet matched.
 * @see decomp.me (75.58%) https://decomp.me/scratch/AW5Sa
 */
s32* menu_build_text_run(s32* sprites, s32* ot, s32 src, s32 arg3, s32 x, s32 y, s32 len, s32 mode)
{
    u8 sp10[0x90]; /* buffer - size matches target frame */
    s32 tmp, count, i;
    s32 *ptr0, *ptr1;
    s32 acc; /* accumulator for halfwords */
    u8 *base, *col;

    /* first call: fill buffer */
    func_800171CC(sp10, src, len);
    sp10[len] = 0;

    /* second call: get number of elements */
    count = func_800644FC(sprites, sp10, arg3);

    /* subtract halfword values according to mode */
    if (mode == 1)
    {
        /* signed halfword (lh) */
        ptr0 = sprites;
        for (i = 0; i < count; i++)
        {
            x -= *(s16*)((char*)ptr0 + 0x10);
            ptr0 = (s32*)((char*)ptr0 + 0x14);
        }
    }
    else if (mode == 2)
    {
        /* unsigned halfword -> (val << 16) >> 17 */
        ptr0 = sprites;
        for (i = 0; i < count; i++)
        {
            u16 val = *(u16*)((char*)ptr0 + 0x10);
            x -= ((s16)val) >> 1; /* arithmetic right shift, matches sra */
            ptr0 = (s32*)((char*)ptr0 + 0x14);
        }
    }

    acc = 0;

    /* main loop - process each structure */
    if (count > 0)
    {
        base = (u8*)sprites;
        col = (u8*)ot;
        tmp = x + (y << 16); /* constant used inside loop */

        do
        {
            /* SPRT primitive: pos, white tint, len=4, code=0x64 */
            *(s32*)(base + 0x8) = tmp + acc;
            SET_BGR0_PACKED(base, GPU_TINT_NEUTRAL);
            setSprt(base);

            acc += *(s16*)(base + 0x10); /* accumulate halfword */

            /* link this SPRT into the OT chain headed at @c col */
            addPrim(col, base);

            /* advance to next structure (20 bytes) */
            base += 0x14;
            col += 0x14;
        } while (--count);
    }

    /* terminating DR_TPAGE primitive (tpage=0x1F) */
    setDrawTPage((DR_TPAGE*)base, 0, 0, 0x1F);
    addPrim(col, base);

    /* return pointer to offset 0x8 of the current structure */
    return (s32*)(base + 8);
}

/**
 * @brief Emit the menu grid: a texture-window delimiter, 0x1D glyph sprites,
 *        and a trailing texture-window + draw-tpage primitive.
 *
 * @param gpu_work Per-frame render context; primitives are appended to its
 *                 OT chain and @c prim_cursor is advanced past the emitted block.
 * @note The 0x14-byte records written in the loop are libgpu @c SPRT
 *       primitives (offset 0x4 @c rgbc, 0x8 @c (x0,y0), 0xC @c (u0,v0),
 *       0xE @c clut, 0x10 @c (w,h)). They are written via raw offsets to
 *       preserve the matched codegen.
 * @see decomp.me (94.19%) https://decomp.me/scratch/ZtHxG
 */
void menu_build_grid(RenderContext* gpu_work)
{
    volatile u8 sp0;
    volatile u16 sp2;
    volatile u16 sp4;
    volatile u16 sp6;
    s32 var_t2;
    u8* var_a2;
    u8* var_t0;
    u8* var_t3;
    u8* temp_t1;
    RenderContext* t7 = gpu_work;
    RenderContext* t4 = t7;

    var_t3 = (u8*)g_menu_glyph_src;
    var_t2 = 0;
    temp_t1 = t7->prim_cursor;
    sp6 = 0xFF;
    sp4 = 0xFF;
    var_t0 = var_t3 + 8;
    sp2 = 0;
    *(u16*)&sp0 = 0;

    /* DR_AREA / texture-window primitive (GP0 0xE2) - leading delimiter */
    setTexWindow((DR_TWIN*)temp_t1, (RECT*)&sp0);
    addPrim(&t4->ot[0x0F], temp_t1);

    temp_t1 += 0xC;
    var_a2 = temp_t1;

    do
    {
        /* SPRT primitive: white tint, len=4, code=0x64 */
        SET_BGR0_PACKED(var_a2, GPU_TINT_NEUTRAL);
        *(u8*)(var_a2 + 3) = 4;
        *(u8*)(var_a2 + 7) = 0x64;
        *(u16*)(var_a2 + 0xC) = *(u16*)var_t3;
        *(u32*)(var_a2 + 8) = *(u32*)(var_t0 - 4);
        *(u32*)(var_a2 + 0x10) = *(u32*)var_t0;

        if (var_t2 >= 0x11)
        {
            *(u16*)(var_a2 + 0xE) = MENU_CLUT_GRID_ALT;
        }
        else
        {
            *(u16*)(var_a2 + 0xE) = MENU_CLUT_GRID_BASE;
        }

        var_t2++;
        var_t0 += 0xC;
        var_t3 += 0xC;

        addPrim(&t4->ot[0x0F], var_a2);
        var_a2 += 0x14;
    } while (var_t2 < 0x1D);

    temp_t1 = var_a2;

    sp4 = 0xFF;
    sp6 = 0xFF;
    *(u16*)&sp0 = 0;
    sp2 = 0;

    /* DR_AREA / texture-window primitive (GP0 0xE2) - trailing delimiter */
    setTexWindow((DR_TWIN*)temp_t1, (RECT*)&sp0);
    addPrim(&t4->ot[0x0F], temp_t1);

    temp_t1 += 0xC;
    /* DR_TPAGE primitive (tpage=5) */
    setDrawTPage((DR_TPAGE*)temp_t1, 0, 0, 5);
    addPrim(&t4->ot[0x0F], temp_t1);

    t7->prim_cursor = temp_t1 + 8;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/CKNIH
 */
void menu_clear_vram(void)
{
    RECT rect;

    rect.x = 0x140;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0x1F2;
    menu_upload_tim(&rect);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/A1YTp
 */
void menu_state_init(void)
{
    g_menu_state_ptr = &D_80151EBC;
}

/**
 * @brief Upload the packed menu texture asset (@c g_menu_tim) to VRAM.
 *
 * The asset is a TIM-style blob holding two 256-entry CLUTs and one texture
 * image. It is committed to VRAM as three transfers via @ref func_80019A34:
 *   1. CLUT 0 (256x1) to @c (rect->w, rect->h).
 *   2. The texture image to @c (rect->x, rect->y); its width/height are read
 *      from the image block, whose position is a self-relative offset stored
 *      inside the asset.
 *   3. CLUT 1 (256x1) to @c (rect->w, rect->h + 1).
 * Before each CLUT upload, the semi-transparency flag (STP, bit 0x8000) is
 * set on every non-zero palette entry.
 *
 * @param rect Destination coordinates: @c (w,h) position the CLUT bands,
 *             @c (x,y) position the texture image.
 * @note The image block is typed as a @ref TimBlock. The two CLUT regions are
 *       left as raw offsets because the matched code reaches them through two
 *       different base pointers (@c tim for the STP-bit loops, @c tim_body for
 *       the upload calls) - a deliberate register-allocation detail.
 * @see decomp.me (100%) https://decomp.me/scratch/tG03R
 */
void menu_upload_tim(Rect16* rect)
{
    u8* tim = g_menu_tim;
    u8* tim_body = tim + 0xC;
    s32 clut_block_len = *(s32*)(tim_body + 8); /* TIM CLUT block length (bnum) */
    Rect16 vram_rect;
    u16* clut_color;
    int i;

    g_menu_tim_dy = *(s32*)(tim_body + 0x14);

    /* Upload CLUT 0. */
    vram_rect.x = rect->w;
    vram_rect.y = rect->h;
    vram_rect.w = 0x100;
    vram_rect.h = 1;

    clut_color = (u16*)(tim + 0x20);
    for (i = 0; i < 0x100; i++)
    {
        if (*clut_color != 0)
        {
            *clut_color |= 0x8000;
        }

        clut_color++;
    }
    func_80019A34(&vram_rect, tim_body + 0x14);

    /* Upload the texture image. */
    vram_rect.x = rect->x;
    vram_rect.y = rect->y;
    {
        /* The CLUT block starts at tim_body+8, so the image block that
           follows it is at tim_body + 8 + clut_block_len. The parenthesization
           `(clut_block_len + 8)` is load-bearing: it must compile to an addiu
           (len + 8) then an addu (+ base). Do not fold it to `... + 8`. */
        TimBlock* image_block = (TimBlock*)(tim_body + (clut_block_len + 8));
        vram_rect.w = image_block->w;
        vram_rect.h = image_block->h;
        func_80019A34(&vram_rect, image_block + 1); /* payload follows header */
    }

    /* Upload CLUT 1. */
    vram_rect.x = rect->w;
    vram_rect.y = rect->h + 1;
    vram_rect.w = 0x100;
    vram_rect.h = 1;

    clut_color = (u16*)(tim + 0x822C);
    for (i = 0; i < 0x100; i++)
    {
        if (*clut_color != 0)
        {
            *clut_color |= 0x8000;
        }

        clut_color++;
    }
    func_80019A34(&vram_rect, tim + 0x822C);
}

/**
 * @brief Allocate a HUD/menu slot from the @c g_menu_slots pool.
 *
 * Scans for the first free slot (@c active == 0), initialises it, and stores
 * the slot's rectangle from @p rect.
 *
 * @param arg0 Value packed into @c MenuSlot.flags bits 31..25 (@c arg0 << 25).
 *             TODO: meaning unknown.
 * @param rect Pointer to four @c u16 values - the slot's x, y, w, h.
 * @return Pointer to the newly allocated @c MenuSlot.
 * @see decomp.me (100%) https://decomp.me/scratch/Xng7v
 */
void* menu_slot_alloc(s32 arg0, void* rect)
{
    s32 var_a2;
    MenuSlot* entry;
    u8* cur;
    u8* ptr;
    u32 temp;
    u32 mask;
    u16* src = (u16*)rect;
    var_a2 = 0;
    ptr = (u8*)&g_menu_slots[0];
    cur = (u8*)&g_menu_slots[0];
    while (var_a2 < 4)
    {
        if ((*cur) == 0)
        {
            break;
        }
        var_a2++;
        cur += 0x24;
    }

    if (var_a2 < 0)
    {
        return (void*)(-1);
    }
    entry = (MenuSlot*)((var_a2 * 0x24) + (u32)ptr);
    *((u16*)(((u8*)entry) + 4)) = 0;
    temp = entry->flags;
    entry->active = 1;
    entry->content_cb = 0;
    entry->index = (u8)var_a2;
    entry->tick_cb = 0;
    entry->anim_frame = 0;
    mask = 0x1FFFFFF;
    temp = temp & mask;
    temp = temp | (((u32)arg0) << 25);
    entry->flags = temp;
    entry->x = src[0];
    entry->y = src[1];
    entry->w = src[2];
    entry->h = src[3];
    entry->lerp_cur_a = 0;
    entry->lerp_cur_b = 0;
    entry->lerp_target_a = 0;
    entry->lerp_target_b = 0;
    entry->lerp_steps = 0;
    entry->has_title = 0;
    g_active_slot = var_a2;
    return (void*)entry;
}

/**
 * @brief Free all menu slots by clearing each slot's @c active byte.
 *
 * Walks @c g_menu_slots from index 3 down to 0 (stride 0x24) and zeroes the
 * leading @c active field, marking every slot as free. Called from
 * @ref menu_init.
 * @see decomp.me (100%) https://decomp.me/scratch/D9BI9
 */
void menu_reset_slots(void)
{
    s32 var_v1;
    s8* var_v0;

    var_v1 = 3;
    var_v0 = (s8*)g_menu_slots;
    var_v0 += 0x6C;
    while (var_v1 >= 0)
    {
        *var_v0 = 0;
        var_v1--;
        var_v0 -= 0x24;
    }
}

/**
 * @brief Per-frame update/draw pump for the four menu slots.
 *
 * Iterates @c g_menu_slots from index 3 down to 0 and dispatches on each
 * slot's @c active state: 1 = opening (advance the open animation via
 * @ref menu_draw_window_transition for 6 frames, then settle to state 2),
 * 2 = open/steady (draw via @ref menu_draw_window), 3 = closing (run the close
 * animation, free the slot when it finishes). The active slot's @c unk20
 * callback runs after its draw. Finally composites the frame and, when
 * @c g_menu_pending_overlay is set, emits an overlay element via @ref func_800A88A0.
 *
 * @param gpu_work Per-frame render context (layout matches @ref RenderContext).
 * @see decomp.me (77.68%) https://decomp.me/scratch/BlGK5
 */
void menu_update_slots(MenuFrameCtx* gpu_work)
{
    s16 sp_pair[2];
    void (*temp_v0_2)(MenuSlot*);
    s32 var_a3;
    s32 var_a0;
    s32 var_s1;
    u8 temp_a0;
    u8 temp_v0;
    u8 temp_v1;
    u8 temp_v1_2;
    u8 tmp_s5;
    MenuSlot* base = g_menu_slots;
    MenuSlot* var_s0;
    void* var_s2;

    g_menu_pending_overlay = 0;
    var_s1 = 3;
    tmp_s5 = 2;
    var_a0 = 0;

    /* 0x6C is exactly the start of the 4th slot (index 3) */
    var_s0 = base + 3;
    /* 0x74 is offset 0x8 into the 4th slot, which is the 'x' field */
    var_s2 = (void*)((u8*)base + 0x74);

    while (var_s1 >= 0)
    {
        temp_v1 = var_s0->active;
        if (temp_v1 != tmp_s5)
        {

            // tmpCmp this is done to force slti
            s32 tmpCmp = temp_v1;

            if (tmpCmp < 3)
            {
                if (temp_v1 != 1)
                {
                    /* merges */
                    var_s0--;
                    var_s1 -= 1;
                    var_s2 = (void*)((u32)var_s2 - sizeof(MenuSlot));
                    continue;
                }
            }
            else
            {
                if (temp_v1 != 3)
                {
                    /* merges */
                    var_s0--;
                    var_s1 -= 1;
                    var_s2 = (void*)((u32)var_s2 - sizeof(MenuSlot));
                    continue;
                }

                goto branch_11C;
            }

            menu_draw_window_transition(gpu_work, var_s0, g_menu_cursor_enable != 0);
            temp_a0 = var_s0->anim_frame;
            temp_v0 = temp_a0 + 1;
            var_s0->anim_frame = temp_v0;
            if ((temp_v0 & 0xff) == 6)
            {
                var_s0->anim_frame = temp_a0;
                var_s0->active = tmp_s5;
            }
        }
        else
        {

            sp_pair[1] = 0;
            sp_pair[0] = 0;
            menu_draw_window(var_s0, gpu_work, var_s2, sp_pair, g_menu_cursor_enable != 0);
        }

        var_a0 = 1;
        if (var_s1 == g_active_slot)
        {
            /* Casting u32 to function pointer */
            temp_v0_2 = (void (*)(MenuSlot*))var_s0->tick_cb;
            if (temp_v0_2 != 0)
            {
                temp_v0_2(var_s0);
                var_a0 = 1;
            }
        }
        var_s0--;
        var_s1 -= 1;

        /* Manually decrementing the void pointer by the size of the struct (0x24) */
        var_s2 = (void*)((u32)var_s2 - sizeof(MenuSlot));
        continue;

    branch_11C:
        menu_draw_window_transition(gpu_work, var_s0, g_menu_cursor_enable != 0);
        temp_v0 = var_s0->anim_frame - 1;
        var_s0->anim_frame = temp_v0;
        var_a0 = 1;
        if (!(temp_v0 & 0xFF))
        {
            func_8014DEB0(1);
            var_s0->active = 0;
            var_a0 = 1;
        }

        /* merges */
        var_s0--;
        var_s1 -= 1;
        var_s2 = (void*)((u32)var_s2 - sizeof(MenuSlot));
    }

    var_a3 = 0;
    if (var_a0 == 0)
    {
        g_active_slot = -1;
    }

    sp_pair[1] = 0;
    sp_pair[0] = 0;

    if ((g_active_slot == -1) || (g_menu_cursor_enable == 0))
    {
        var_a3 = 1;
    }

    gpu_work->prim_cursor = menu_draw_frame(gpu_work->prim_cursor, &gpu_work->ot_base, gpu_work->draw_buf_idx, var_a3);

    if (g_menu_pending_overlay != 0)
    {
        gpu_work->prim_cursor = func_800A88A0(gpu_work->prim_cursor, &gpu_work->ot_base, g_menu_pending_overlay, 1, 0xA0, 0xCA, 2);
    }
}

/**
 * @brief Draw a menu window mid-open/close animation at an interpolated inset.
 *
 * Computes a per-frame shrink amount from the slot's target size
 * (@c unkC / @c unkE divided by 12, scaled by the frame counter @c unk2) and,
 * while both axes are still positive, draws the window via @ref menu_draw_window
 * at the inset rectangle. Used for slot @c active states 1 (opening) and 3
 * (closing) by @ref menu_update_slots.
 *
 * @param gpu_work     Per-frame render context (passed through to @ref menu_draw_window).
 * @param slot         Slot whose animated rectangle is drawn.
 * @param cursor_enable Cursor-highlight enable (forwarded as the draw's @p cursor_enable).
 * @see decomp.me (100%) https://decomp.me/scratch/luaLZ
 */
void menu_draw_window_transition(s32 gpu_work, MenuSlotAnim* slot, s32 cursor_enable)
{
    s16 sp[8];
    s32 temp_a3;
    s32 temp_a1;
    s32 clampC;
    s32 clampE;

    /* First computation */
    temp_a3 = ((s32)((u16)slot->w << 0x10) >> 0x11) - ((s16)(slot->w / 12) * slot->anim_frame);
    sp[4] = (s16)temp_a3;

    /* Second computation */
    temp_a1 = ((s32)((u16)slot->h << 0x10) >> 0x11) - ((s16)(slot->h / 12) * slot->anim_frame);

    /* First branch logic block */

    sp[5] = (s16)temp_a1;

    /* Second branch logic block */
    if (temp_a3 > 0)
    {

        if (temp_a1 > 0)
        {

            clampC = slot->w - (temp_a3 * 2);
            if (clampC < 0x20)
            {
                clampC = 0x20;
            }

            clampE = slot->h - (temp_a1 * 2);
            if (clampE < 0x10)
            {
                clampE = 0x10;
            }

            sp[0] = slot->x + temp_a3;
            sp[1] = slot->y + temp_a1;
            sp[2] = clampC;
            sp[3] = clampE;

            menu_draw_window(slot, gpu_work, &sp[0], &sp[4], cursor_enable);
        }
    }
}

/**
 * @brief Build all GPU primitives for one menu window at a given rectangle.
 *
 * Sets up the window's draw environment, then emits the background fill
 * (@ref menu_fill_window_interior), the four edges (@ref menu_build_h_edge horizontal,
 * @ref menu_build_v_edge vertical), and the four corners (@ref menu_emit_corner),
 * splicing each into the slot's primitive chain. May also run the slot's
 * @c unk1C content callback and optional title/decoration passes.
 *
 * @param slot          Slot descriptor (geometry, flags, content callback).
 * @param gpu_work      Per-frame render context (layout matches @ref RenderContext).
 * @param rect          Window rectangle: x, y, w, h halfwords.
 * @param arg3          TODO: forwarded to the content callback and edge builders.
 * @param cursor_enable Cursor-highlight enable for the active slot.
 * @see decomp.me (91.20%) https://decomp.me/scratch/5k4SF
 */
void menu_draw_window(MenuSlotView* slot, MenuRenderCtx* gpu_work, MenuRect* rect, s32 arg3, s32 cursor_enable)
{
    MenuRectU16 sp18;
    DRAWENV sp20;
    u16 sp80[2];
    s16 temp_a0;
    s32 temp_v1;
    s32 var_a2_2;
    s32 var_a3;
    s32* temp_a1_2;
    s32* temp_s1;
    s32* temp_s1_2;
    s32* temp_s2;
    s32* prim_cur;
    s32* var_s1;
    u16 temp_a1;
    u16 temp_a2;
    MenuRect* temp_s3;
    u16 var_v0;
    void* temp_v0_2;

    temp_s3 = rect;
    var_s1 = gpu_work->prim_cursor;
    temp_s2 = (s32*)gpu_work + (((u32)slot->_u.flags >> 0x19));
    if (slot->lerp_steps != 0)
    {
        temp_a2 = slot->lerp_cur_a;
        temp_v1 = (s32)(slot->lerp_target_a - temp_a2) / (s32)slot->lerp_steps;
        temp_a1 = slot->lerp_cur_b;
        temp_a1 = temp_a1 + ((s32)(slot->lerp_target_b - temp_a1) / (s32) * (volatile u8*)&slot->lerp_steps);
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
        if ((temp_s3->w - 0x20) > 0)
        {
            if ((temp_s3->h - 0x10) > 0)
            {
                SetDrawEnv((DR_ENV*)var_s1, (DRAWENV*)(g_draw_buf_base + ((gpu_work->draw_buf_idx ^ 1) * 0x40C0) + 0x4064));
                var_a3 = 0;
                *var_s1 = (*var_s1 & 0xFF000000) | (*temp_s2 & 0xFFFFFF);
                g_menu_draw_early_out = 0;
                *temp_s2 = (*temp_s2 & 0xFF000000) | ((s32)var_s1 & 0xFFFFFF);
                var_s1 += 0x10;
                if ((slot->index == g_active_slot) && (cursor_enable != 0))
                {
                    if (g_menu_suppress_cursor == 0)
                    {
                        var_a3 = slot->active == 2;
                    }
                }
                temp_s1 = slot->content_cb(temp_s2, slot, var_s1, arg3, var_a3);
                if (g_menu_draw_early_out != 0)
                {
                    gpu_work->prim_cursor = temp_s1;
                    return;
                }
                temp_a0 = temp_s3->y;
                var_a2_2 = temp_a0 + 0x10;
                if (gpu_work->draw_buf_idx != 0)
                {
                    var_a2_2 = temp_a0 + 0xF8;
                }
                SetDefDrawEnv(&sp20, temp_s3->x + 8, var_a2_2, temp_s3->w - 0x10, temp_s3->h - 0x10);
                SetDrawEnv((DR_ENV*)temp_s1, &sp20);
                *temp_s1 = (*temp_s1 & 0xFF000000) | (*temp_s2 & 0xFFFFFF);
                *temp_s2 = (*temp_s2 & 0xFF000000) | ((s32)temp_s1 & 0xFFFFFF);
                var_s1 = temp_s1 + 0x10;
                if (slot->has_title != 0)
                {
                    switch (g_menu_scene_type)
                    {        /* switch 1 */
                    case 1:  /* switch 1 */
                    case 4:  /* switch 1 */
                    case 19: /* switch 1 */
                    case 22: /* switch 1 */
                    case 25: /* switch 1 */
                        var_v0 = ((u16)temp_s3->x + (u16)temp_s3->w) - 0x68;
                        break;
                    default: /* switch 1 */
                        var_v0 = ((u16)temp_s3->x + (u16)temp_s3->w) - 0x48;
                        break;
                    }
                    sp80[0] = var_v0;
                    sp80[1] = (u16)temp_s3->y;
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
                    var_s1 = (s32*)func_800AD208(temp_s2, temp_a1_2, slot->_u._s.unk6 & 0x1FF, 3, sp80, 0);
                    switch (g_menu_scene_type)
                    {        /* switch 2 */
                    case 1:  /* switch 2 */
                    case 4:  /* switch 2 */
                    case 19: /* switch 2 */
                    case 22: /* switch 2 */
                    case 25: /* switch 2 */
                        temp_s1_2 = func_800AD524((s32)var_s1, temp_s2, 0xB, sp80, 0);
                        sp80[0] += 8;
                        var_s1 = (s32*)func_800AD208(temp_s2, temp_s1_2, func_8014F23C(), 3, sp80, 0);
                        break;
                    }
                    var_s1 = func_80148578((s32)var_s1, temp_s2, slot);
                }
            }
        }
    }
    sp18.w = 0xFF;
    sp18.h = 0xFF;
    sp18.x = 0;
    sp18.y = 0;
    ((MenuPrimHead*)var_s1)->_u._s.unk3 = 2;
    {
        u8 t_unk2 = (u8)sp18.y;
        u8 t_unk0 = (u8)sp18.x;

        s16 t_unk4 = sp18.w;
        s16 t_unk6 = sp18.h;
        ((MenuPrimHead*)var_s1)->unk8 = 0;
        ((MenuPrimHead*)var_s1)->unk4 =
            (s32)(((t_unk2 >> 3) << 0xF) | (((t_unk0 >> 3) << 0xA) | 0xE2000000) | ((-t_unk6 << 2) & 0x3E0) | ((s32)(-t_unk4 & 0xFF) >> 3));
    }
    ((MenuPrimHead*)var_s1)->_u.unk0 = (((MenuPrimHead*)var_s1)->_u.unk0 & 0xFF000000) | (*temp_s2 & 0xFFFFFF);
    *temp_s2 = (*temp_s2 & 0xFF000000) | ((s32)var_s1 & 0xFFFFFF);
    prim_cur = var_s1 + 3;
    if (temp_s3->h >= 0x10)
    {
        sp18.x = (u16)temp_s3->x + 8;
        sp18.y = (u16)temp_s3->y;
        sp18.w = (u16)temp_s3->w - 0x10;
        sp18.h = 8;
        prim_cur = menu_build_h_edge(prim_cur, temp_s2, &sp18, MENU_TW_EDGE_TOP);
        if (temp_s3->h >= 0x10)
        {
            sp18.x = (u16)temp_s3->x + 8;
            sp18.y = ((u16)temp_s3->y + (u16)temp_s3->h) - 8;
            sp18.w = (u16)temp_s3->w - 0x10;
            sp18.h = 8;
            prim_cur = menu_build_h_edge(prim_cur, temp_s2, &sp18, MENU_TW_EDGE_BOT);
        }
    }
    if (temp_s3->w >= 0x20)
    {
        sp18.x = (u16)temp_s3->x;
        sp18.y = (u16)temp_s3->y + 8;
        sp18.w = 8;
        sp18.h = (u16)temp_s3->h - 0x10;
        prim_cur = menu_build_v_edge(prim_cur, temp_s2, &sp18, MENU_TW_EDGE_LEFT);
        if (temp_s3->w >= 0x20)
        {
            sp18.x = ((u16)temp_s3->x + (u16)temp_s3->w) - 8;
            sp18.y = (u16)temp_s3->y + 8;
            sp18.w = 8;
            sp18.h = (u16)temp_s3->h - 0x10;
            prim_cur = menu_build_v_edge(prim_cur, temp_s2, &sp18, MENU_TW_EDGE_RIGHT);
        }
    }
    sp18.x = (u16)temp_s3->x + 8;
    sp18.y = (u16)temp_s3->y + 8;
    sp18.w = (u16)temp_s3->w - 0x10;
    sp18.h = (u16)temp_s3->h - 0x10;
    temp_v0_2 = menu_emit_corner(menu_emit_corner(menu_emit_corner(menu_emit_corner(menu_fill_window_interior(prim_cur, temp_s2, &sp18, MENU_TW_FILL), temp_s2,
                                                                                    temp_s3->x, temp_s3->y, MENU_TW_CORNER_TL),
                                                                   temp_s2, temp_s3->x + temp_s3->w - 8, temp_s3->y, MENU_TW_CORNER_TR),
                                                  temp_s2, temp_s3->x, temp_s3->y + temp_s3->h - 8, MENU_TW_CORNER_BL),
                                 temp_s2, temp_s3->x + temp_s3->w - 8, temp_s3->y + temp_s3->h - 8, MENU_TW_CORNER_BR);
    ((MenuPrimHead*)temp_v0_2)->_u._s.unk3 = 1;
    ((MenuPrimHead*)temp_v0_2)->unk4 = 0xE1000005;
    ((MenuPrimHead*)temp_v0_2)->_u.unk0 = (s32)((((MenuPrimHead*)temp_v0_2)->_u.unk0 & 0xFF000000) | (*temp_s2 & 0xFFFFFF));
    *temp_s2 = (*temp_s2 & 0xFF000000) | ((s32)temp_v0_2 & 0xFFFFFF);
    gpu_work->prim_cursor = (s32*)((char*)temp_v0_2 + 8);
}

/**
 * @brief Emit one 8x8 textured corner sprite and splice it into a prim chain.
 *
 * Writes a libgpu @c SPRT (code 0x64, white tint 0x808080, fixed 8x8 size,
 * fixed CLUT @c MENU_CLUT_CORNER) at screen @p x / @p y with texture origin @p uv, links it
 * into the chain headed at @p ot, and returns the next primitive slot.
 * Called four times by @ref menu_draw_window, once per window corner.
 *
 * @param prim Primitive write cursor (the @c SPRT is built here).
 * @param ot   Ordering-table head the sprite is linked into.
 * @param x    Sprite screen X (@c x0).
 * @param y    Sprite screen Y (@c y0).
 * @param uv   Packed texture origin written to @c u0 / @c v0 (offset 0xC).
 * @return Pointer to the next primitive slot (@p prim + 0x14).
 * @see decomp.me (100%) https://decomp.me/scratch/GcWsA
 */
void* menu_emit_corner(SPRT* prim, s32* ot, s16 x, s16 y, s32 uv)
{
    SET_BGR0_PACKED(prim, GPU_TINT_NEUTRAL);

    setSprt(prim);

    SET_SPRT_WH_PACKED(prim, 8, 8);

    setXY0(prim, x, y);

    SET_SPRT_CLUT(prim, MENU_CLUT_CORNER);
    SET_SPRT_UV0_PACKED(prim, uv);

    addPrim(ot, prim);

    return prim + 1;
}

/**
 * @brief Tile the interior of a window with 0x60x0x60 textured sprites.
 *
 * Walks @p rect in 0x60-pixel steps on both axes, emitting one @c SPRT per
 * tile (code 0x64, tint @c MENU_TINT_FILL, CLUT @c MENU_CLUT_GRID_ALT)
 * clamped to the region edges, and links each into the chain headed at @p ot.
 *
 * @param prim  Primitive write cursor (advanced past every emitted tile).
 * @param ot    Ordering-table head the tiles are linked into.
 * @param rect  Region to fill: x, y (screen origin), w, h (pixel size).
 * @param uv    Packed texture origin written to each tile's @c u0 / @c v0.
 * @return Primitive write cursor just past the last tile emitted.
 * @see decomp.me (90.20%) https://decomp.me/scratch/R9mdk
 */
s32* menu_fill_window_interior(s32* prim, s32* ot, MenuRectU16* rect, s16 uv)
{
    u16 tile_x;
    short pad;
    u16 rect_y;
    s32 y = 0;
    if (rect->h > 0)
    {
        do
        {
            s32 x = 0;
            if (rect->w > 0)
            {
                s32 y_plus_60 = y + 0x60;
                u8* wp;
                do
                {
                    SET_BGR0_PACKED(prim, MENU_TINT_FILL);
                    setlen((SPRT*)prim, 4);
                    setcode((SPRT*)prim, 0x64);
                    SET_SPRT_UV0_PACKED(prim, uv);
                    if (rect->w < (x + 0x60))
                    {
                        ((SPRT*)prim)->w = (u16)((u16)rect->w - (u16)x);
                    }
                    else
                    {
                        ((SPRT*)prim)->w = 0x60;
                    }
                    if ((rect->h < y_plus_60) != 0)
                    {
                        ((SPRT*)prim)->h = (u16)((u16)rect->h - (u16)y);
                    }
                    else
                    {
                        ((SPRT*)prim)->h = 0x60;
                    }
                    tile_x = (u16)(rect->x + (u16)x);
                    ((SPRT*)prim)->x0 = tile_x;
                    rect_y = rect->y;
                    x += 0x60;
                    ((SPRT*)prim)->y0 = (u16)(rect_y + (u16)y);
                    wp += 0x14;
                    SET_SPRT_CLUT(prim, MENU_CLUT_GRID_ALT);
                    addPrim(ot, (SPRT*)prim);
                    prim = (s32*)((SPRT*)prim + 1);
                } while (x < rect->w);
            }
            y += 0x60;
        } while (y < rect->h);
    }
    return prim;
}

/**
 * @brief Build a horizontal border strip: one @c SPRT + one @c DR_TWIN primitive.
 *
 * Emits a single textured sprite covering the input rectangle, then a
 * texture-window primitive (GP0 0xE2) with a 16x8 region derived from
 * @p tw_uv. Used for the top and bottom edges of a menu window.
 *
 * @param ot      Primitive write cursor (SPRT is built here).
 * @param ot_ptr  Ordering-table head the primitives are linked into.
 * @param rect    Edge rectangle: x, y (screen position), w, h (pixel size).
 * @param tw_uv   Packed texture-window origin: bits 7..0 = u (x), bits 15..8 = v (y).
 * @return Pointer to the byte immediately after the emitted DR_TWIN.
 * @see decomp.me (100%) https://decomp.me/scratch/u17Fi
 */
u_long* menu_build_h_edge(u_long* ot, u_long* ot_ptr, MenuRectU16* rect, s32 tw_uv)
{
    RECT tw;
    SPRT* sprt;
    DR_TWIN* twin;

    if (rect->w <= 0)
    {
        return ot;
    }

    if (rect->h > 0)
    {
        sprt = (SPRT*)ot;
        SET_BGR0_PACKED(sprt, GPU_TINT_NEUTRAL);
        setSprt(sprt);
        SET_SPRT_UV0_PACKED(sprt, 0);
        sprt->w = rect->w;
        sprt->h = rect->h;
        sprt->x0 = rect->x;
        sprt->y0 = rect->y;
        sprt->clut = MENU_CLUT_CORNER;
        addPrim(ot_ptr, sprt);
        ot += sizeof(SPRT) / sizeof(u_long);

        twin = (DR_TWIN*)ot;
        tw.x = tw_uv & 0xFF;
        tw.y = tw_uv >> 8;
        tw.w = 16;
        tw.h = 8;
        setTexWindow(twin, &tw);
        addPrim(ot_ptr, twin);
        ot += sizeof(DR_TWIN) / sizeof(u_long);
    }

    return ot;
}

/**
 * @brief Build a vertical border strip: one @c SPRT + one @c DR_TWIN primitive.
 *
 * Mirror of @ref menu_build_h_edge for vertical (left/right) window edges.
 * The texture-window region is 8x16 instead of 16x8.
 *
 * @param ot      Primitive write cursor (SPRT is built here).
 * @param ot_ptr  Ordering-table head the primitives are linked into.
 * @param rect    Edge rectangle: x, y (screen position), w, h (pixel size).
 * @param tw_uv   Packed texture-window origin: bits 7..0 = u (x), bits 15..8 = v (y).
 * @return Pointer to the byte immediately after the emitted DR_TWIN.
 * @see decomp.me (100%) https://decomp.me/scratch/19jr7
 */
void* menu_build_v_edge(u_long* ot, u_long* ot_ptr, MenuRectU16* rect, s32 tw_uv)
{
    RECT tw;
    SPRT* sprt;
    DR_TWIN* twin;

    if (rect->w <= 0)
    {
        return ot;
    }

    if (rect->h > 0)
    {
        sprt = (SPRT*)ot;
        SET_BGR0_PACKED(sprt, GPU_TINT_NEUTRAL);
        setSprt(sprt);
        SET_SPRT_UV0_PACKED(sprt, 0);
        sprt->w = rect->w;
        sprt->h = rect->h;
        sprt->x0 = rect->x;
        sprt->y0 = rect->y;
        sprt->clut = MENU_CLUT_CORNER;
        addPrim(ot_ptr, sprt);
        ot += sizeof(SPRT) / sizeof(u_long);

        twin = (DR_TWIN*)ot;
        tw.x = tw_uv & 0xFF;
        tw.y = tw_uv >> 8;
        tw.w = 8;
        tw.h = 0x10;
        setTexWindow(twin, &tw);
        addPrim(ot_ptr, twin);
        ot += sizeof(DR_TWIN) / sizeof(u_long);
    }

    return ot;
}

/**
 * @brief Render a text label from a packed string table at the given screen position.
 *
 * Resolves one of two embedded @c StringTableKey entries in the shared string
 * table (base 0x800EC3C4) using @p label_id's sign, copies the packed glyph
 * string into a local buffer, then calls the glyph renderer.
 *
 * Both branches compute the same table base via pointer arithmetic from
 * whichever key they select: @c &g_menu_label_key_a-0x16 == @c &g_menu_label_key_b-0x20 ==
 * 0x800EC3C4.
 *
 * @param ot        Ordering-table head (@c u_long*; passed as arg1 to the glyph renderer).
 * @param prim      Primitive write cursor (@c u_long*; passed as arg0 to the glyph renderer).
 * @param pos       Screen position (x, y) to draw at.
 * @param label_id  >= 0: draw string keyed by g_menu_label_key_a; < 0: keyed by g_menu_label_key_b.
 * @see decomp.me (94.94%) https://decomp.me/scratch/ozwB7
 */
void menu_draw_label(u_long* ot, u_long* prim, ScreenPos* pos, s32 label_id)
{
    u8 sp20[16];
    u8* ptr = sp20;
    u8* str_ptr;

    if (label_id >= 0)
    {
        str_ptr = g_menu_label_key_a.entry + ((g_menu_label_key_a.page << 8) + ((u8*)&g_menu_label_key_a - 0x16));
    }
    else
    {
        str_ptr = g_menu_label_key_b.entry + ((g_menu_label_key_b.page << 8) + ((u8*)&g_menu_label_key_b - 0x20));
    }

    func_800A8E28(ptr, str_ptr);
    ptr += func_800A8DDC(str_ptr);

    *ptr = 0;

    func_800A88A0(prim, ot, sp20, 1, pos->x, pos->y, 0);
}

/**
 * @brief Initialize the full menu node tree and global menu state.
 * @note Builds all 44 g_menu_nodes entries with hardcoded parent-child links and flags,
 *       zeroes all layout counters, runs an initial position pass, then calls into the
 *       layout and render pipeline unless a script is still active.
 * @see decomp.me (90.94%) https://decomp.me/scratch/XJkmb
 * @warning NOT FUNCTIONALLY EQUIVALENT -- do not rely on this C for logic analysis.
 */
void menu_node_tree_init(void)
{
    MenuNode* var_a0;
    MenuNode* var_a2;
    s32 temp_v0_3;
    s32 temp_v0_5;
    unsigned int temp_v0_7;
    s32 temp_v0_9;
    s32 var_a3;
    s32 var_t0;
    u8* new_var5;
    MenuNode* new_var2;
    int new_var7;
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
    var_t0 = 0;
    var_a0 = g_menu_nodes;
    g_menu_prev_node = MENU_NONE;
    g_menu_content_ready = 0;
    g_item_slot_data.unk0 = 0;
    g_item_slot_data.unk4 = 0;
    g_item_slot_data.unk8 = 0;
    g_item_slot_data.unkC = 0;
    g_item_slot_flags.slot0 = 0;
    g_item_slot_flags.slot1 = 0;
    g_item_slot_flags.slot2 = 0;
    g_item_slot_flags.slot3 = 0;
    g_menu_item_ptr = 0;
    D_80169410 = 0;
    D_80169404 = 0;
    D_80169408 = 0;
    D_8016911C = 0;
    D_80169554 = 0;
    D_801694B0 = 0;
    g_menu_nav_prev[0] = 0;
    g_menu_content_height = 0;
    g_menu_scroll_pos = 0;
    g_menu_redraw_state = 0;
    g_menu_active_node = 0;
    g_menu_cursor_enable = 0;
    for (var_t0 = 0; var_t0 < MENU_NODE_COUNT; var_t0++)
    {
        g_menu_nodes[var_t0].state = MENU_NODE_STATE_UNINIT;
        g_menu_nodes[var_t0].unk4 = 0;
        g_menu_nodes[var_t0].content_id = var_a1;
        g_menu_nodes[var_t0].child3 = var_a1;
        g_menu_nodes[var_t0].child2 = var_a1;
        g_menu_nodes[var_t0].child1 = var_a1;
        g_menu_nodes[var_t0].uA.s.child0 = var_a1;
        g_menu_nodes[var_t0].u2.unk2 = (u16)((g_menu_nodes[var_t0].u2.unk2 & 0xFFFC) | 0x30);
        g_menu_nodes[var_t0].u2.s.parent_idx = var_a1;
    }

    g_menu_nodes[0].unk0 = 1;
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
        g_menu_nodes[0].unk4 = 2;
    }
    else
    {
        g_menu_nodes[0].unk4 = 1;
    }
    g_menu_nodes[0].uA.s.child0 = 1;
    g_menu_nodes[1].idx_nav.s.self_idx = 1;
    g_menu_nodes[1].unk4 = 5;
    g_menu_nodes[0].child1 = 2;
    g_menu_nodes[2].unk0 = 2;
    g_menu_nodes[2].idx_nav.s.self_idx = 2;
    g_menu_nodes[2].unk4 = 4;
    g_menu_nodes[3].unk0 = 4;
    g_menu_nodes[1].unk0 = 3;
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
    g_menu_nodes[4].idx_nav.s.self_idx = 4;
    if (D_800FDA80 & 2)
    {
        var_v0 = 0x6F;
    }
    else
    {
        g_menu_nodes[3].u2.unk2 = (u16)(temp_v0_3 & 0xFF3E);
        var_v0 = 0x6E;
    }
    g_menu_nodes[3].unk4 = var_v0;
    g_menu_nodes[4].u2.unk2 = (u16)((0xFF5F & g_menu_nodes[4].u2.unk2) | 0x50);
    g_menu_nodes[5].u2.unk2 = (u16)((g_menu_nodes[5].u2.unk2 & 0xFF5F) | 0x50);
    ;
    *((volatile u16*)(&g_menu_nodes[6].u2.unk2)) = (g_menu_nodes[6].u2.unk2 & 0xFFCD) | 0x10;
    temp_v0_5 = temp_v0_4 | 0x10;
    *((volatile u16*)(&g_menu_nodes[6].u2.unk2)) = (u16)(temp_v0_5 & 0xFF3F);
    *((volatile u16*)(&g_menu_nodes[6].u2.unk2)) = (u16)(temp_v0_5 & 0xFF3E);
    g_menu_nodes[3].uA.s.child0 = 4;
    g_menu_nodes[3].child1 = 5;
    g_menu_nodes[4].unk0 = 6;
    g_menu_nodes[4].unk4 = 5;
    g_menu_nodes[5].unk0 = 5;
    g_menu_nodes[5].idx_nav.s.self_idx = 5;
    g_menu_nodes[5].unk4 = 4;
    g_menu_nodes[6].unk0 = 7;
    g_menu_nodes[6].idx_nav.s.self_idx = 6;
    g_menu_nodes[6].unk4 = 3;
    g_menu_nodes[6].uA.s.child0 = 7;
    g_menu_nodes[6].child1 = 8;
    g_menu_nodes[7].unk0 = 9;
    g_menu_nodes[7].idx_nav.s.self_idx = 7;
    new_var7 = 0xFF3E;
    g_menu_nodes[7].unk4 = 5;
    g_menu_nodes[8].unk0 = 8;
    g_menu_nodes[4].u2.s.parent_idx = 3;
    g_menu_nodes[5].u2.s.parent_idx = 3;
    g_menu_nodes[6].u2.s.parent_idx = MENU_NONE;
    g_menu_nodes[7].u2.unk2 = (u16)((g_menu_nodes[7].u2.unk2 & 0xFF5F) | 0x50);
    g_menu_nodes[7].u2.s.parent_idx = 6;
    g_menu_nodes[8].u2.unk2 = (u16)((g_menu_nodes[8].u2.unk2 & 0xFF5F) | 0x50);
    g_menu_nodes[8].idx_nav.s.self_idx = 8;
    temp_v0_6 = (g_menu_nodes[9].u2.unk2 & 0xFFCD) | 0x20;
    g_menu_nodes[9].u2.unk2 = temp_v0_6;
    temp_v0_7 = temp_v0_6 | 0x20;
    g_menu_nodes[9].u2.unk2 = (u16)(temp_v0_7 & 0xFF3E);
    temp_v0_8 = (g_menu_nodes[0xC].u2.unk2 & 0xFFCD) | 0x20;
    new_var8 = 0x16;
    (*(&g_menu_nodes[9])).u2.unk2 = (u16)(temp_v0_7 & 0xFF3F);
    *((volatile u16*)g_menu_nodes[0xC].u2.unk2) = temp_v0_8;
    *((volatile u16*)g_menu_nodes[0xC].u2.unk2) = (u16)((temp_v0_8 | 0x20) & 0xFF3F);
    *((volatile u16*)g_menu_nodes[0xC].u2.unk2) = (u16)((temp_v0_8 | 0x20) & new_var7);
    g_menu_nodes[0xA].u2.unk2 = (u16)((g_menu_nodes[0xA].u2.unk2 & 0xFF6F) | 0x60);
    g_menu_nodes[8].u2.s.parent_idx = 6;
    g_menu_nodes[8].unk4 = 4;
    g_menu_nodes[9].unk0 = 0xA;
    g_menu_nodes[9].idx_nav.s.self_idx = 9;
    g_menu_nodes[9].unk4 = 6;
    g_menu_nodes[9].uA.s.child0 = 0xA;
    g_menu_nodes[0xA].unk0 = 0xB;
    g_menu_nodes[0x12].unk4 = 0xA;
    g_menu_nodes[0xA].idx_nav.s.self_idx = 0xA;
    g_menu_nodes[0xA].unk4 = 7;
    g_menu_nodes[0xC].unk0 = 0xA;
    g_menu_nodes[0xC].idx_nav.s.self_idx = 0xC;
    g_menu_nodes[0xC].unk4 = 6;
    g_menu_nodes[0xC].uA.s.child0 = 0xD;
    g_menu_nodes[0xD].unk0 = 0xB;
    g_menu_nodes[0xD].idx_nav.s.self_idx = 0xD;
    g_menu_nodes[0xD].unk4 = 7;
    g_menu_nodes[9].u2.s.parent_idx = MENU_NONE;
    temp_v0_7 = 0xF;
    g_menu_nodes[0xA].u2.s.parent_idx = 9;
    g_menu_nodes[0xC].u2.s.parent_idx = MENU_NONE;
    temp_v0_10 = (g_menu_nodes[temp_v0_7].u2.unk2 & 0xFFCD) | 0x20;
    g_menu_nodes[temp_v0_7].u2.unk2 = temp_v0_10;
    g_menu_nodes[0xD].u2.unk2 = (u16)(g_menu_nodes[0xD].u2.unk2 | 0x60);
    var_t0_2 = 0xF;
    g_menu_nodes[0xD].u2.s.parent_idx = 0xC;
    temp_v0_11 = ((temp_v0_10 & 0xFF6D) | 0x60) ^ 0;
    g_menu_nodes[0xF].u2.unk2 = temp_v0_11;
    g_menu_nodes[var_t0_2].u2.unk2 = (u16)(temp_v0_11 & 0xFFFE);
    g_menu_nodes[0xF].unk4 = 8;
    g_menu_nodes[0x10].unk4 = 7;
    g_menu_nodes[0xF].u2.s.parent_idx = MENU_NONE;
    g_menu_nodes[0xF].unk0 = 0xD;
    g_menu_nodes[0xF].idx_nav.s.self_idx = 0xF;
    g_menu_nodes[0xF].uA.s.child0 = 0x10;
    g_menu_nodes[0xF].child1 = 0x11;
    g_menu_nodes[0x10].unk0 = 0xC;
    g_menu_nodes[0x10].idx_nav.s.self_idx = 0x10;
    g_menu_nodes[0x11].unk0 = 0xE;
    g_menu_nodes[0x11].idx_nav.s.self_idx = 0x11;
    g_menu_nodes[0x11].unk4 = 9;
    g_menu_nodes[0x12].unk0 = 0x10;
    g_menu_nodes[0x12].idx_nav.s.self_idx = 0x12;
    g_menu_nodes[0x12].content_id = 4;
    g_menu_nodes[0x12].uA.s.child0 = 0x13;
    g_menu_nodes[0x12].child1 = 0x16;
    g_menu_nodes[0x12].child2 = 0x19;
    g_menu_nodes[0x12].child3 = 0x1C;
    g_menu_nodes[0x10].u2.unk2 = (u16)((g_menu_nodes[0x10].u2.unk2 & 0xFF6F) | 0x60);
    g_menu_nodes[0x10].u2.s.parent_idx = 0xF;
    g_menu_nodes[0x11].u2.unk2 = (u16)((g_menu_nodes[0x11].u2.unk2 & 0xFF6F) | 0x60);
    g_menu_nodes[0x11].u2.s.parent_idx = 0xF;
    temp_v0_12 = g_menu_nodes[0x12].u2.unk2 & 0xFF3D;
    g_menu_nodes[0x12].u2.unk2 = (u16)(g_menu_nodes[0x12].u2.unk2 & 0xFFFD);
    g_menu_nodes[0x12].u2.unk2 = temp_v0_12;
    g_menu_nodes[0x12].u2.unk2 = (u16)(temp_v0_12 | 1);
    g_menu_nodes[0x12].u2.s.parent_idx = MENU_NONE;
    g_menu_nodes[0x13].unk0 = 0x11;
    g_menu_nodes[0x16].content_id = 1;
    g_menu_nodes[0x14].unk4 = 0xF;
    g_menu_nodes[0x13].unk4 = 0xB;
    g_menu_nodes[0x13].idx_nav.s.self_idx = 0x13;
    g_menu_nodes[0x13].content_id = 0;
    g_menu_nodes[0x13].uA.s.child0 = 0x14;
    g_menu_nodes[0x13].child1 = 0x15;
    g_menu_nodes[0x14].unk0 = 0x12;
    g_menu_nodes[0x14].idx_nav.s.self_idx = 0x14;
    g_menu_nodes[0x15].unk0 = 0x13;
    g_menu_nodes[0x15].idx_nav.s.self_idx = 0x15;
    g_menu_nodes[0x15].unk4 = 0x12;
    g_menu_nodes[0x16].unk0 = 0x14;
    g_menu_nodes[0x16].idx_nav.s.self_idx = 0x16;
    g_menu_nodes[0x16].unk4 = 0xC;
    g_menu_nodes[0x16].uA.s.child0 = 0x17;
    g_menu_nodes[0x16].child1 = 0x18;
    g_menu_nodes[0x17].unk0 = 0x15;
    g_menu_nodes[0x17].idx_nav.s.self_idx = 0x17;
    g_menu_nodes[0x13].u2.unk2 = (u16)((g_menu_nodes[0x13].u2.unk2 & 0xFF3F) | 0x40);
    g_menu_nodes[0x14].u2.unk2 = (u16)((g_menu_nodes[0x14].u2.unk2 & 0xFF3F) | 0x80);
    g_menu_nodes[0x13].u2.s.parent_idx = 0x12;
    g_menu_nodes[0x14].u2.s.parent_idx = 0x13;
    g_menu_nodes[new_var8].u2.unk2 = (u16)((g_menu_nodes[0x16].u2.unk2 & 0xFF3F) | 0x40);
    g_menu_nodes[0x16].u2.s.parent_idx = 0x12;
    g_menu_nodes[0x15].u2.unk2 = (u16)((g_menu_nodes[0x15].u2.unk2 & 0xFF3F) | 0x80);
    g_menu_nodes[0x15].u2.s.parent_idx = 0x13;
    g_menu_nodes[0x17].u2.unk2 = (u16)((g_menu_nodes[0x17].u2.unk2 & 0xFF3F) | 0x80);
    g_menu_nodes[0x17].u2.s.parent_idx = 0x16;
    g_menu_nodes[0x19].content_id = 2;
    g_menu_nodes[0x17].unk4 = 0x10;
    g_menu_nodes[0x18].unk0 = 0x13;
    g_menu_nodes[0x18].idx_nav.s.self_idx = 0x18;
    g_menu_nodes[0x18].unk4 = 0x12;
    g_menu_nodes[0x19].unk0 = 0x16;
    g_menu_nodes[0x19].idx_nav.s.self_idx = 0x19;
    g_menu_nodes[0x19].unk4 = 0xD;
    g_menu_nodes[0x19].uA.s.child0 = 0x1A;
    g_menu_nodes[0x19].child1 = 0x1B;
    g_menu_nodes[0x1A].unk0 = 0x17;
    g_menu_nodes[0x1A].idx_nav.s.self_idx = 0x1A;
    g_menu_nodes[0x1A].unk4 = 0x11;
    g_menu_nodes[0x1B].unk0 = 0x13;
    g_menu_nodes[0x1B].idx_nav.s.self_idx = 0x1B;
    g_menu_nodes[0x1B].unk4 = 0x12;
    g_menu_nodes[0x1C].unk0 = 0x18;
    g_menu_nodes[0x1C].idx_nav.s.self_idx = 0x1C;
    g_menu_nodes[0x18].u2.unk2 = (u16)((g_menu_nodes[0x18].u2.unk2 & 0xFF3F) | 0x80);
    g_menu_nodes[0x19].u2.unk2 = (u16)((g_menu_nodes[0x19].u2.unk2 & 0xFF3F) | 0x40);
    g_menu_nodes[0x18].u2.s.parent_idx = 0x16;
    g_menu_nodes[0x19].u2.s.parent_idx = 0x12;
    g_menu_nodes[0x1B].u2.unk2 = (u16)((g_menu_nodes[0x1B].u2.unk2 & 0xFF3F) | 0x80);
    g_menu_nodes[0x1B].u2.s.parent_idx = 0x19;
    g_menu_nodes[0x1C].u2.unk2 = (u16)((g_menu_nodes[0x1C].u2.unk2 & 0xFF3F) | 0x40);
    g_menu_nodes[0x1C].u2.s.parent_idx = 0x12;
    var_a1 = 0x1E;
    g_menu_nodes[0x1A].u2.unk2 = (g_menu_nodes[0x1A].u2.unk2 & 0xFF3F) | 0x80;
    g_menu_nodes[0x1A].u2.s.parent_idx = 0x19;
    var_a2->uA.layout_child_packed += 0;
    g_menu_nodes[0x1C].unk4 = 0xE;
    g_menu_nodes[0x1D].idx_nav.s.self_idx = 0x1D;
    g_menu_nodes[0x1E].uA.s.child0 = 0x1F;
    g_menu_nodes[0x1F].idx_nav.s.self_idx = 0x1F;
    g_menu_nodes[0x1C].content_id = 5;
    g_menu_nodes[0x1D].unk0 = 0x1C;
    g_menu_nodes[0x1D].content_id = 3;
    g_menu_nodes[0x1D].unk4 = 0x18;
    g_menu_nodes[0x1E].unk0 = 0x19;
    g_menu_nodes[0x1E].idx_nav.s.self_idx = 0x1E;
    g_menu_nodes[0x1E].unk4 = 0x13;
    g_menu_nodes[0x1F].unk0 = 0x1A;
    g_menu_nodes[0x1F].unk4 = 0x14;
    g_menu_nodes[0x2B].unk0 = 0x1A;
    g_menu_nodes[0x2B].idx_nav.s.self_idx = 0x2B;
    temp_v0_13 = g_menu_nodes[0x1D].u2.unk2 & 0xFF3D;
    g_menu_nodes[0x1D].u2.unk2 = (u16)(g_menu_nodes[0x1D].u2.unk2 & 0xFFFD);
    (*(&g_menu_nodes[0x1D])).u2.unk2 = temp_v0_13;
    temp_v1 = g_menu_nodes[0x1E].u2.unk2;
    g_menu_nodes[0x1D].u2.unk2 = (u16)(temp_v0_13 | 1);
    g_menu_nodes[0x1D].u2.s.parent_idx = MENU_NONE;
    g_menu_nodes[0x1F].u2.unk2 = (u16)((g_menu_nodes[0x1F].u2.unk2 & 0xFF3F) | 0x40);
    g_menu_nodes[0x1F].u2.s.parent_idx = 0x1E;
    g_menu_nodes[0x1E].u2.unk2 = (u16)(temp_v1 & 0xFFFD);
    temp_v1_2 = temp_v1 & 0xFF3D;
    temp_v1 = (u16)((g_menu_nodes[0x2B].u2.unk2 & 0xFF3F) | 0x40);
    g_menu_nodes[0x1E].u2.unk2 = temp_v1_2;
    g_menu_nodes[0x2B].u2.unk2 = temp_v1;
    g_menu_nodes[0x1E].u2.unk2 = (u16)(temp_v1_2 | 1);
    g_menu_nodes[0x1E].u2.s.parent_idx = MENU_NONE;
    g_menu_nodes[0x2B].u2.s.parent_idx = 0x1E;
    g_menu_nodes[0x1F].u2.unk2 = (u16)(g_menu_nodes[0x1F].u2.unk2 & 0xFFCF);
    g_menu_nodes[0x2B].u2.unk2 = (u16)((g_menu_nodes[0x2B].u2.unk2 & 0xFFCF) | 0x10);
    g_menu_nodes[0x2B].unk4 = 0x15;
    g_menu_nodes[0x20].unk0 = 0x1B;
    g_menu_nodes[0x20].idx_nav.s.self_idx = 0x20;
    temp_v0_14 = 0xFF3D;
    temp_v0_14 = g_menu_nodes[0x20].u2.unk2 & temp_v0_14;
    new_var6 = D_800FD818.unk268 & 1;
    (*(g_menu_nodes + 0x20)).u2.unk2 = (u16)(g_menu_nodes[0x20].u2.unk2 & 0xFFFD);
    g_menu_nodes[0x20].u2.unk2 = temp_v0_14;
    g_menu_nodes[0x20].unk4 = 0x16;
    g_menu_nodes[0x20].u2.unk2 = (u16)(temp_v0_14 | 1);
    g_menu_nodes[0x20].u2.s.parent_idx = MENU_NONE;
    if (new_var6)
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
    }
    temp_v0_11 = var_a3 & 0xFFFF;
    var_t0_2 = 0;
    new_var3 = g_menu_nodes;
    var_a2 = new_var3;
    var_a3 = 0;
    new_var = &var_a2->u8_u;
    do
    {
        new_var5 = &new_var3->u2.s.parent_idx;
        if ((*new_var5) == 0xFF)
        {
            temp_a0 = temp_v0_11;
            if (((char)var_a2->u2.unk2) & 1)
            {
                temp_a1 = 0x1FF;
                temp_a1 = var_a3 & temp_a1;
                var_a3 = var_a3 + MENU_ROW_HEIGHT;
                var_a2->idx_nav.nav_x_packed = (u16)(var_a2->idx_nav.nav_x_packed & 0x80FF);
                var_a2->u8_u.nav_y_packed = (u16)((*new_var).nav_y_packed & 0x80FF);
                var_a2->uA.layout_child_packed = (u16)((var_a2->uA.layout_child_packed & 0xFF00) | ((temp_a0 >> 1) & 0xFF));
                var_a2->u8_u.nav_y_packed = (u16)(((*new_var).nav_y_packed & 0x7FFF) | (temp_a0 << 0xF));
                var_a2->idx_nav.nav_x_packed = (u16)((var_a2->idx_nav.nav_x_packed & 0x7FFF) | (((temp_a1 & 1) << 9) << 6));
                var_a1 = temp_a1 >> 1;
                var_a2->u8_u.nav_y_packed = (u16)((new_var3->u8_u.nav_y_packed & 0xFF00) | var_a1);
            }
        }
        var_t0_2 += 1;
        var_a2++;
    } while (var_t0_2 < MENU_NODE_COUNT);
    if (g_active_script != 0)
    {
        g_menu_scene_type = -1;
        return;
    }
    g_menu_scene_type = 0;
    new_var4 = g_menu_init_content_id;
    D_801690F9 = 0;
    func_8014E3C4(new_var4, var_a1, new_var3, var_a3);
    menu_set_active_node();
}

/**
 * @brief Clears the expand flag (bit 1 of u2.s.flags) for all menu nodes.
 * @note Called before menu_update_layout to ensure no node recurses into children.
 * @see decomp.me (100%) https://decomp.me/scratch/hyDM7
 */
void menu_collapse_all(void)
{
    s32 i;
    for (i = 0; i < MENU_NODE_COUNT; i++)
    {
        g_menu_nodes[i].u2.unk2 &= 0xFFFD;
    }
}

/**
 * @brief Assigns layout positions to all active root menu nodes and stores the final position count.
 * @note Sets g_menu_scroll_pos and g_menu_redraw_state to signal scroll state after layout.
 * @see decomp.me (100%) https://decomp.me/scratch/YhGni
 */
void menu_update_layout(void)
{
    s32 changed = 0;
    s32 pos = changed;
    s32 i = pos;

    do
    {
        if (g_menu_nodes[i].u2.s.parent_idx == MENU_NONE)
        {
            s32 prev_pos = pos;
            pos++;
            pos--;

            if (g_menu_nodes[i].u2.s.flags & 1)
            {
                pos = menu_layout_node(i, prev_pos);
                if (prev_pos != (pos - MENU_ROW_HEIGHT))
                {
                    changed = 1;
                    if (pos >= (MENU_VIEW_HEIGHT + 1))
                    {
                        g_menu_scroll_pos = pos - MENU_VIEW_HEIGHT;
                        g_menu_redraw_state = MENU_REDRAW_LAYOUT;
                    }
                }
            }
        }
        i += 1;
    } while (i < MENU_NODE_COUNT);

    g_menu_layout_end = pos;
    if (changed == 0)
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
 * @note Bit 1 of u2.s.flags controls child recursion; menu_collapse_all clears it before layout.
 * @see decomp.me (99.04%) https://decomp.me/scratch/LDCeT
 */
s32 menu_layout_node(s32 node_idx, s32 base_pos)
{
    MenuNode* temp_a0;
    s32 cur_pos;
    int child_iter;
    MenuNode* node;
    int is_expanded;
    u32 layout_y; /* base_pos clamped to 16 bits; packed as 9-bit value into layout_y_lsb/layout_y_hi */
    /* Codegen artifact: separate union pointer so the compiler reads nav_y_hi via a distinct register. */
    union
    {
        u16 nav_y_packed;
        struct
        {
            u8 nav_y_hi;
            u8 layout_y_lsb;
        } s;
    }* u8_alias;
    MenuNode* node2; /* second pointer alias -- register allocation artifact, do not merge with node */
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
    /* Duplicate write (codegen artifact): both lines store (layout_y >> 1) into layout_y_hi. */
    (&g_menu_nodes[node_idx])->uA.layout_child_packed = ((&g_menu_nodes[node_idx])->uA.layout_child_packed & 0xFF00) | (0xFF & (layout_y >> 1));
    node->uA.layout_child_packed = (node->uA.layout_child_packed & 0xFF00) | ((layout_y >> 1) & 0xFF);
    child_iter = 0;
    if (is_expanded)
    {
        s32 child_idx;
        s32 child;
        /* child0..child3 are consecutive bytes; treat as a 4-element array via pointer arithmetic. */
        for (child_idx = child_iter; (child_idx < MENU_MAX_CHILDREN) != 0;)
        {
            node2 = g_menu_nodes + node_idx;
            child_idx = (&(&(*node2).uA.s)->child0)[child_idx];
            child = child_idx;
            if (child == MENU_NONE)
            {
                break;
            }
            child_idx++;
            cur_pos = menu_layout_node(child, cur_pos);
        }
    }
    return cur_pos;
}

/**
 * @brief Draw the menu frame layers and optionally dispatch navigation input.
 * @param prim_cursor_id Prim-buffer ID passed to the initial buffer setup call.
 * @param ot Pointer into the ordering table used for addPrim calls.
 * @param draw_page 0 = first display page (Y near 0); nonzero = second page (Y+240).
 * @param handle_input When nonzero and cursor is disabled, calls menu_handle_node_input.
 * @return Updated prim-buffer write cursor after all primitives are emitted.
 * @see decomp.me (100%) https://decomp.me/scratch/x8WyZ
 */
u_char* menu_draw_frame(int prim_cursor_id, u_int* ot, int draw_page, int handle_input)
{
    DRAWENV stack_drawenv;
    u_char* s1;
    u_char* s0;
    int var_a2;
    int v0_190;
    u8* scd_base;
    u_char* prim_end;

    scd_base = (u8*)0x801ed600;

    /* 30: jal func_80145608 */
    s1 = (u_char*)func_80145608(prim_cursor_id);

    /* 3c: ternary generating the conditional branch at 48 */
    var_a2 = draw_page ? 0xf0 : 8;

    /* 58: SetDefDrawEnv(&stack_drawenv, 0, var_a2, 0x140, 0xe0) */
    SetDefDrawEnv(&stack_drawenv, 0, var_a2, 0x140, 0xe0);

    /* 64: SetDrawEnv(s1) passing &stack_drawenv as side argument */
    SetDrawEnv(s1, &stack_drawenv);

    /* 78 - 94: First OT link operation */
    addPrim(ot, s1);

    /* 98: Advance the structural pointer by 0x40 bytes */
    s1 += 0x40;
    s0 = s1;

    /* a8 - c0: Decrement tracker logic */
    if (scd_base[0x92] != 0)
    {
        u8 val = scd_base[0x92] - 1;
        scd_base[0x140] = val;
        scd_base[0x92] = val;
    }

    /* c4: Explicit switch structure to enforce the exact MIPS jump table/branch sequence */
    switch (g_menu_cursor_enable)
    {
    case 0:
        s0 = (u_char*)func_80148900(s1, ot - 1, handle_input);
        func_80143964(0);
        if (handle_input != 0)
        {
            menu_handle_node_input();
        }
        break;

    case 1:
        s0 = (u_char*)func_80148A20(s1, ot - 1, handle_input);
        if (g_menu_suppress_cursor == 0)
        {
            func_80143964(handle_input);
        }
        break;

    case 2:
        s0 = (u_char*)func_80148A20(s1, ot - 1, handle_input);
        if (g_menu_suppress_cursor == 0)
        {
            g_menu_cursor_enable = 0;
        }
        break;

    default:
        break;
    }

    /* 190: jal func_80149828 */
    v0_190 = func_80149828(s0, ot);

    /* 19c: jal func_8014874C */
    s0 = (u_char*)func_8014874C(v0_190, ot - 1);

    /* 1a8 - 1cc: Inline primitive data initialization */
    *(u8*)(s0 + 3) = 1;
    *(u_int*)(s0 + 4) = 0xe1000005;

    /* 1d0 - 1fc: Second OT link operation */
    addPrim(ot, s0);
    s1 = s0 + 8;

    /* 1f8: Second conditional ternary logic blocks */
    var_a2 = draw_page ? 0xfc : 0x14;

    /* 214: SetDefDrawEnv */
    prim_end = s0 + 0x48;
    SetDefDrawEnv(&stack_drawenv, 0xf, var_a2, 0x24, 0xaa);

    /* 220: SetDrawEnv with s1 advanced to s0 + 8 */
    SetDrawEnv(s1, &stack_drawenv);

    /* 22c - 254: Third OT link operation */
    addPrim(ot, s1);

    /* 248: Evaluates to `addiu v0, s0, 0x48` as the function's return statement */
    return prim_end;
}

/**
 * @brief Process D-pad and face-button input to navigate and select menu nodes.
 * @see decomp.me (99.42%) https://decomp.me/scratch/YoOml
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
    const u32 browse_all_node = MENU_NODE_BROWSE_ALL; /* = 0x20, kept as local for register allocation */
    const u8 SENTINEL;
    temp_v0 = func_8014852C(g_menu_active_node);
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
            do
            {
            } while (0);
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
            func_8014F210(MENU_SE_CLOSE, MENU_SE_VOLUME);
            g_menu_load_request = 1;
            return;
        }
        g_menu_active_node = g_menu_nav_first;
        g_menu_active_node = browse_all_node;
    }
    if ((PAD_BTN_UP | PAD_BTN_DOWN | PAD_BTN_CIRCLE) & (g_pad_input & 0xFFFFu))
    {
        func_8014F210(MENU_SE_NAVIGATE, MENU_SE_VOLUME);
        temp_a0 = func_8014852C(g_menu_active_node);
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
        func_8014F210(MENU_SE_SELECT, MENU_SE_VOLUME);
        new_var13 = 0xFF;
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
            D_80168C10 = 0xA;
            return;
        }
        new_var11 = g_menu_nodes;
        temp_a1 = &g_menu_nodes[g_menu_active_node];
        temp_v0_3 = new_var13;
        (&g_menu_nodes[g_menu_active_node])->u2.unk2 |= 0xC;
        new_var10 = temp_v0_3;
        if (temp_a1->u2.s.parent_idx == temp_v0_3) /* temp_v0_3 = MENU_NONE (0xFF) */
        {
            D_80169408 = (D_80169404 = (D_80169410 = (g_menu_item_ptr = 0)));
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
                    D_801690F9 = 0;
                    func_8014E3C4(g_menu_nodes[g_menu_scene_type].content_id, temp_a1, new_var14, temp_v0_3);
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
        temp_v0_3 = temp_a1->u8_u.s.nav_y_hi;
        new_var12 = temp_a0_3;
        new_var15 = g_menu_content_height;
        g_content_cursor_y = MENU_CURSOR_Y_MIN;
        g_content_cursor_y = ((temp_v0_3 * 2) | new_var12) - (new_var15 - g_content_cursor_y);
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
            g_menu_hit_item_idx = func_8014847C(new_var12, temp_a1, &g_content_cursor_y, temp_v0_3);
            if (g_menu_hit_item_idx != (-1))
            {
                temp_v1_2 = &g_menu_content_table[new_var11[g_menu_scene_type].idx_nav.s.self_idx][g_menu_hit_item_idx];
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
 * decomp.me (100%) https://decomp.me/scratch/q39Ou
 */
s32 func_80143640(void)
{
    MenuContentItem* base;
    MenuContentItem* temp_a0;
    int new_var;
    g_menu_hit_item_idx = func_8014847C();
    if (g_menu_hit_item_idx != (-1))
    {
        MenuNode* nodes = g_menu_nodes;
        u8 self_idx = (nodes + g_menu_scene_type)->idx_nav.s.self_idx;
        base = g_menu_content_table[self_idx];
        temp_a0 = base - (-g_menu_hit_item_idx);
        g_content_view_x = temp_a0->packed_x & 0x1FF;
        new_var = temp_a0->y - 8;
        g_menu_suppress_cursor = 5;
        g_menu_cursor_enable = 1;
        g_content_view_y = new_var;
        return 1;
    }
    return 0;
}

/** @brief Active character slot index: 0 = char slot 0 (node 0x1F), 1 = char slot 1 (node 0x2B). */
extern s32 g_menu_char_slot;

/**
 * @brief Mark the active node's ancestor chain as expanded, propagate its nav cursor
 *        position to its children, update g_menu_char_slot, and re-run the full layout.
 * @note var_s3 and layout_pos are each reused for two unrelated purposes to match the
 *       compiler's register allocation exactly.
 * @see decomp.me (93.73%) https://decomp.me/scratch/UqSRu
 */
void menu_set_active_node(void)
{
    s32 node_idx;
    MenuNode* curr_node;
    MenuNode* active_node;
    u16 temp_v0;
    s32 char_slot_bits;
    s32 var_s3;     /* parent-walk: current ancestor index; layout: scroll-adjusted flag */
    s32 layout_pos; /* parent-walk: reused as temp for parent_idx check; layout: y accumulator */
    s32 node_i;
    MenuNode* node;
    s32 prev_layout_y;
    long child_slot;
    u8 child_idx;
    MenuNode* child_node;
    u16 nav_col; /* bits [14:8] of nav_x_packed: 7-bit column, copied to children */
    u16 parent_packed;

    /* Clear the "expanded" bit (bit 1) on every node, then re-expand only the active path. */
    for (node_idx = 0; node_idx < MENU_NODE_COUNT; node_idx++)
    {
        g_menu_nodes[node_idx].u2.unk2 &= 0xFFFD;
    }

    /* Walk from g_menu_active_node up to the root, marking each ancestor expanded. */
    var_s3 = g_menu_active_node;
    layout_pos = g_menu_nodes[g_menu_active_node].u2.s.parent_idx;
    if (layout_pos != MENU_NONE)
    {
        do
        {
            var_s3 = g_menu_nodes[var_s3].u2.s.parent_idx;
            curr_node = &g_menu_nodes[var_s3];
            curr_node->u2.unk2 |= 2; /* set expanded */
        } while (curr_node->u2.s.parent_idx != MENU_NONE);
    }

    /* Mark the active node itself expanded, then propagate its nav cursor to children. */
    active_node = &g_menu_nodes[g_menu_active_node];
    temp_v0 = active_node->u2.unk2 | 2;
    active_node->u2.unk2 = temp_v0;
    if ((temp_v0 >> 1) & 1)
    {
        for (child_slot = 0; child_slot < MENU_MAX_CHILDREN; child_slot++)
        {
            child_idx = (&active_node->uA.s.child0)[child_slot];
            if (child_idx == (temp_v0 = MENU_NONE))
            {
                break;
            }
            child_node = &g_menu_nodes[child_idx];
            /* Propagate nav column X (bits [14:8]) from parent to child. */
            nav_col = active_node->idx_nav.nav_x_packed & MENU_NAV_X_MASK;
            child_node->idx_nav.nav_x_packed = child_node->idx_nav.nav_x_packed & MENU_NAV_X_CLEAR;
            child_node->idx_nav.nav_x_packed = child_node->idx_nav.nav_x_packed | nav_col;
            child_node->u8_u.nav_y_packed = child_node->u8_u.nav_y_packed & MENU_NAV_X_CLEAR;
            child_node->u8_u.nav_y_packed = child_node->u8_u.nav_y_packed | nav_col;
            child_idx = (&active_node->uA.s.child0)[child_slot];
            parent_packed = active_node->idx_nav.nav_x_packed;
            ;
            /* Propagate nav_y_hi (nav Y bits 8:1) from parent to child's u8_u low byte. */
            (&g_menu_nodes[child_idx])->u8_u.nav_y_packed =
                (((&g_menu_nodes[child_idx])->u8_u.nav_y_packed & 0xFF00) & 0xFFFFFFFFFFFFFFFFu) | (&g_menu_nodes[g_menu_active_node])->u8_u.s.nav_y_hi;
            /* Propagate nav Y bit 0 (bit 15 of nav_x_packed) from parent to child. */
            (&g_menu_nodes[child_idx])->idx_nav.nav_x_packed =
                ((&g_menu_nodes[child_idx])->idx_nav.nav_x_packed & ~MENU_NAV_Y0_BIT) | (parent_packed & MENU_NAV_Y0_BIT);
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
        if (node->u2.s.parent_idx == MENU_NONE)
        {
            if (node->u2.s.flags & 1)
            {
                prev_layout_y = layout_pos;
                layout_pos = menu_layout_node(node_i, layout_pos);
                /* If this node contributed more than one row, it has visible children. */
                if (prev_layout_y != (layout_pos - MENU_ROW_HEIGHT))
                {
                    if (layout_pos > MENU_VIEW_HEIGHT)
                    {
                        var_s3 = 1;
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

extern u8 D_801ED600[];
/** @brief Total page count for the current sub-menu view; g_script_repeat_last cycles in [0, g_menu_page_count-1]. */
extern s32 g_menu_page_count;
/** @brief Action sub-type of the most recently confirmed 0x5000 menu item; routes downstream handlers. */
extern s32 g_menu_active_subtype;
extern s8 D_801226F0;
extern void* D_80168C70;
extern s8 D_801226B8;
extern s32 D_801229F4;
extern s32 D_8011F424;

extern void func_8014B7DC(void);
extern void func_8014CC08(void);
extern void func_8014C200(void);

typedef struct
{
    u8 unk0;
    u8 unk1;
    u8 unk2;
    u8 unk3;
} D_801690B0_type;

extern D_801690B0_type D_801690B0;
extern u8 D_801686CC[];
extern u8 D_8014FE54[][8];
extern void* D_801690A8;
extern void* D_801690E0;
extern void* D_801227D4;
extern u8 D_801686F8[];

/**
 * decomp.me (68.48%) https://decomp.me/scratch/DRmEd
 * Warning. Highly unlikely to be functionally equivalent. Don't make assumptions based on this function.
 */
void func_80143964(s32 arg0)
{
    u8* var_s4 = D_801ED600;
    MenuContentItem* s5;
    u8 content_type;
    MenuSlot* slots;
    u8 self_idx;
    s32 idxA, idxB;
    u16 packed_x;
    u16 top_nibble;
    u8 table_idx;
    u8 val;
    u32 dir_mask;
    s32 dir_index;
    MenuNode* active_node;
    s32 nav_y;
    s32 new_type;
    u8 flag;
    void* new_var7;
    u32 item14;
    s32 shift;
    u8* base;
    s32 var_s0;
    u8 sp18;

    if ((u32)(g_menu_scene_type - 0x14) < 2 || g_menu_scene_type == 0x17 || g_menu_scene_type == 0x18 || g_menu_scene_type == 0x1A || g_menu_scene_type == 0x1B)
    {
        if (g_pad_input & 0xF)
        {
            func_8014F210(0x7D, 0x80);
            if (g_pad_input & 3)
            {
                if (g_menu_item_ptr != ((void*)0))
                {
                    if (g_pad_input & 1)
                    {
                        func_8014B69C(-1);
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
                        func_8014B69C(1);
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
                menu_set_active_node(g_menu_scene_type);
                g_menu_hit_item_idx = func_8014847C((s32*)0);
                if (g_menu_hit_item_idx != (-1))
                {
                    MenuContentItem* temp_s5 = g_menu_content_table[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];
                    g_content_view_x = temp_s5[g_menu_hit_item_idx].packed_x & 0x1FF;
                    g_content_view_y = temp_s5[g_menu_hit_item_idx].y - 8;
                    g_menu_suppress_cursor = 5;
                    g_menu_cursor_enable = 1;
                }
            }
        }
    }

    if (g_menu_scene_type < 0x11U && (g_pad_input & 0xF))
    {
        func_8014F210(0x7D, 0x80);
        self_idx = g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx;
        if (self_idx - 0x14 < 0x16U)
        {
            switch (self_idx - 0x14)
            {
            case 0:
            case 3:
            case 6:
                g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx += 1;
                goto incdec_common;

            case 1:
            case 4:
            case 7:
                g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx -= 1;
                goto incdec_common;

            case 2:
            case 5:
            case 8:
            incdec_common:
                g_menu_hit_item_idx = func_8014847C((s32*)0);
                if (g_menu_hit_item_idx != (-1))
                {
                    MenuContentItem* temp_s5 = g_menu_content_table[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];
                    g_content_view_x = temp_s5[g_menu_hit_item_idx].packed_x & 0x1FF;
                    g_content_view_y = temp_s5[g_menu_hit_item_idx].y - 8;
                    g_menu_suppress_cursor = 5;
                    g_menu_cursor_enable = 1;
                }
                goto after_do_while;

            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
            case 16:
            case 17:
            case 18:
            case 19:
            case 20:
            case 21:
                break;
            }
        }

        do
        {
            do
            {
                if (g_pad_input & 4)
                {
                    new_type = g_menu_scene_type - 1;
                    if (g_menu_scene_type == ((g_menu_scene_type / 3) * 3))
                    {
                        new_type = g_menu_scene_type + 2;
                        if (g_menu_scene_type >= 9)
                        {
                            new_type = g_menu_scene_type + 1;
                        }
                    }
                    g_menu_scene_type = new_type;
                    g_menu_active_node = new_type;
                }
                if (g_pad_input & 8)
                {
                    if (g_menu_scene_type < 9)
                    {
                        if ((g_menu_scene_type / 3) * 3 == g_menu_scene_type - 2)
                        {
                            new_type = g_menu_scene_type - 2;
                        }
                        else
                        {
                            new_type = g_menu_scene_type + 1;
                        }
                    }
                    else
                    {
                        if ((g_menu_scene_type / 3) * 3 == g_menu_scene_type - 1)
                        {
                            new_type = g_menu_scene_type - 1;
                        }
                        else
                        {
                            new_type = g_menu_scene_type + 1;
                        }
                    }
                    g_menu_scene_type = new_type;
                    g_menu_active_node = new_type;
                }
                if ((g_pad_input & 1) && (g_menu_scene_type < 9))
                {
                    new_type = g_menu_scene_type - 3;
                    if ((g_menu_scene_type / 3) == 0)
                    {
                        new_type = g_menu_scene_type + 6;
                    }
                    g_menu_scene_type = new_type;
                    g_menu_active_node = new_type;
                    menu_set_active_node(g_menu_scene_type);
                }
                if ((g_pad_input & 2) && (g_menu_scene_type < 9))
                {
                    new_type = g_menu_scene_type + 3;
                    if ((g_menu_scene_type / 3) == 2)
                    {
                        new_type = g_menu_scene_type - 6;
                    }
                    g_menu_scene_type = new_type;
                    g_menu_active_node = new_type;
                    menu_set_active_node(g_menu_scene_type);
                }
            } while (!(g_menu_nodes[(g_menu_scene_type / 3) * 3].u2.s.flags & 1));

            if (g_menu_cursor_enable == 0)
            {
                menu_snap_view_to_cursor();
            }
            g_menu_hit_item_idx = func_8014847C((s32*)0);
            if (g_menu_hit_item_idx != (-1))
            {
                MenuContentItem* temp_s5 = g_menu_content_table[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];
                g_content_view_x = temp_s5[g_menu_hit_item_idx].packed_x & 0x1FF;
                g_content_view_y = temp_s5[g_menu_hit_item_idx].y - 8;
                g_menu_suppress_cursor = 5;
                g_menu_cursor_enable = 1;
            }
        } while (g_menu_hit_item_idx == (-1));

        slots = g_menu_slots + 3;
        for (var_s0 = 3; var_s0 >= 0; var_s0--)
        {
            slots->active = 0;
            slots--;
        }
    }

after_do_while:

    if (arg0 != 0)
    {
        s5 = g_menu_content_table[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];

        if ((g_menu_scene_type == 0x1F || g_menu_scene_type == 0x2B) && g_menu_hit_item_idx >= 0x11 && g_menu_hit_item_idx < 0x19)
        {
            if (g_pad_input & 0x40)
            {
                if (D_801690B0.unk3 != 0xFF)
                {
                    g_pad_input &= ~0x40;
                    D_801690B0.unk3 = 0xFF;
                }
                else
                {
                    g_pad_input &= ~0x40;
                    func_8014F210(0x7D, 0x80);
                    func_8014519C();
                }
            }
            if (g_pad_input & 0x220)
            {
                if (D_801690B0.unk3 == 0xFF)
                {
                    func_8014F210(0x7D, 0x80);
                    D_801690B0.unk0 = (s8)((s5[g_menu_hit_item_idx].packed_x & 0x1FF) - 2);
                    D_801690B0.unk3 = (u8)g_menu_hit_item_idx;
                    D_801690B0.unk1 = (s8)(s5[g_menu_hit_item_idx].y - 8);
                }
                else if (D_801690B0.unk3 != g_menu_hit_item_idx)
                {
                    u8* pad_arr;
                    u8 tmp;

                    func_8014F210(0x7E, 0x80);
                    pad_arr = (u8*)g_pad_ctx + ((g_menu_scene_type == 0x1F) ? 0 : 0x250) + 0x638;
                    for (idxA = 0; idxA < 8; idxA++)
                    {
                        if (pad_arr[idxA] == (g_menu_hit_item_idx - 0x11))
                            break;
                    }
                    for (idxB = 0; idxB < 8; idxB++)
                    {
                        if (pad_arr[idxB] == (D_801690B0.unk3 - 0x11))
                            break;
                    }
                    tmp = pad_arr[idxA];
                    pad_arr[idxA] = pad_arr[idxB];
                    pad_arr[idxB] = tmp;
                    D_801690B0.unk3 = 0xFF;
                }
                else
                {
                    func_8014F210(0x7F, 0x80);
                    D_801690B0.unk3 = 0xFF;
                }
            }
            else if (g_pad_input & 0x10)
            {
                u8* base_arr;

                func_8014F210(0x7E, 0x80);
                base_arr = (u8*)g_pad_ctx + ((g_menu_scene_type == 0x1F) ? 0 : 0x250) + 0x638;
                base_arr[0] = 0;
                base_arr[1] = 1;
                base_arr[2] = 2;
                base_arr[3] = 3;
                base_arr[4] = 4;
                base_arr[5] = 5;
                base_arr[6] = 6;
                base_arr[7] = 7;
            }
        }
        else if (g_pad_input & 0x220)
        {
            packed_x = s5[g_menu_hit_item_idx].packed_x;
            top_nibble = packed_x & 0xF000;
            if (top_nibble == 0x5000)
            {
                content_type = s5[g_menu_hit_item_idx].pad[0];
                g_menu_active_subtype = content_type;
                switch (content_type)
                {
                case 1:
                case 2:
                    if (g_menu_char_slot == 0)
                    {
                        u16 sp10 = 0x40;
                        u16 sp12 = 0x60;
                        u16 sp14 = 0xF0;
                        u16 sp16 = 0x60;
                        MenuSlot* var_a3 = (MenuSlot*)menu_slot_alloc(3, &sp10);
                        var_a3->content_cb = (s32 * (*)()) & func_8014B7DC;
                        var_a3->flags = (var_a3->flags & 0xFE00FFFF) | ((func_80145310() & 0x1FF) << 16);
                        var_a3->has_title = 1;
                        func_8014F210(0x7D, 0x80);
                    }
                    break;

                case 3:
                case 4:
                case 5:
                case 6:
                    if (g_menu_char_slot == 0)
                    {
                        flag = ((u8*)g_pad_ctx)[(g_menu_char_slot * 0x250) + 0x609 + content_type];
                        {
                            u16 sp10 = 0xB0;
                            u16 sp12, sp14, sp16;
                            if ((flag != 0xFF) && (flag & 0x80))
                            {
                                sp14 = 0x70;
                                sp12 = 0x60;
                                sp16 = 0x50;
                            }
                            else
                            {
                                sp12 = 0x60;
                                sp14 = 0x70;
                                sp16 = 0x40;
                            }
                            {
                                MenuSlot* var_a3 = (MenuSlot*)menu_slot_alloc(3, &sp10);
                                var_a3->content_cb = (s32 * (*)()) & func_8014C200;
                                new_var7 = (void*)((u8*)g_pad_ctx + (g_menu_char_slot * 0x250) + 0x5F0 + (content_type << 6) + 0x90);
                                if ((flag != 0xFF) && (flag & 0x80))
                                {
                                    var_a3->flags = (var_a3->flags & 0xFE00FFFF) | 0x40000;
                                    func_80145278(4);
                                }
                                else
                                {
                                    var_a3->flags = (var_a3->flags & 0xFE00FFFF) | 0x30000;
                                    func_80145278(3);
                                }
                                func_8014A044(var_a3, &D_80168C70);
                                g_menu_item_ptr = new_var7;
                                D_80169554 = g_menu_item_ptr;
                                D_80169410 = g_menu_item_ptr;
                                D_801694B0 = g_menu_item_ptr;
                                D_80169404 = g_menu_item_ptr;
                                g_menu_nav_prev[0] = g_menu_item_ptr;
                                D_8016911C = g_menu_item_ptr;
                                D_80169408 = g_menu_item_ptr;
                                func_8014F210(0x7D, 0x80, &g_pad_ctx, &D_8016911C);
                            }
                        }
                    }
                    break;

                case 7:
                case 8:
                case 9:
                case 10:
                    if (g_menu_char_slot == 0)
                    {
                        u16 sp10 = 0xB0;
                        u16 sp12 = 0x30;
                        u16 sp14 = 0x70;
                        u16 sp16 = 0x60;
                        MenuSlot* var_a3 = (MenuSlot*)menu_slot_alloc(3, &sp10);
                        var_a3->content_cb = (s32 * (*)()) & func_8014CC08;
                        var_a3->flags = (var_a3->flags & 0xFE00FFFF) | 0x50000;
                        func_80145278(5);
                        {
                            void* ptr = (void*)((u8*)g_pad_ctx + (g_menu_char_slot * 0x250) + 0x5F0 + (content_type << 6) - 0x170);
                            D_80169554 = ptr;
                            D_801694B0 = ptr;
                            D_8016911C = ptr;
                            g_menu_nav_prev[0] = ptr;
                            func_8014F210(0x7D, 0x80, &g_pad_ctx, &D_8016911C);
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
                            {
                                sp18 = 0;
                                D_801226B8 = 0;
                                D_801227D4 = g_menu_item_ptr;
                                if (func_8014DE1C(g_menu_item_ptr) != 0)
                                {
                                    base = (u8*)g_menu_state_ptr + (*((s32*)((char*)g_menu_state_ptr + 0x88)));
                                    func_800A8E28(&sp18, (s8*)(base + (((*((u16*)((char*)g_menu_item_ptr + 0x16))) & 0x3F) * 2 + (*((u16*)(base + 0x48))))));
                                    item14 = *((u32*)((char*)g_menu_item_ptr + 0x14));
                                    shift = (item14 >> 8) & 3;
                                    if (shift == 0)
                                    {
                                        func_80148324(&D_801226B8, &sp18, (s8*)(base + (*((s32*)(base + ((item14 >> 9) & 0x7E))))));
                                    }
                                    else if (shift == 1)
                                    {
                                        func_80148324(&D_801226B8, &sp18, (s8*)(base + (*((s32*)(base + ((item14 >> 9) & 0x7E) + 0x20)))));
                                    }
                                    else
                                    {
                                        func_80148324(&D_801226B8, &sp18, (s8*)(base + (*((s32*)(base + ((item14 >> 9) & 0x7E) + 0x40)))));
                                    }
                                    g_menu_load_request = 1;
                                    if (g_menu_scene_type == 1)
                                    {
                                        D_801229F4 = g_script_repeat_last;
                                        D_80168C10 = 0xB;
                                    }
                                    else if (g_menu_scene_type == 2)
                                    {
                                        D_80168C10 = 0xC;
                                    }
                                    else
                                    {
                                        D_801229F4 = g_script_repeat_last;
                                        D_80168C10 = 1;
                                    }
                                }
                            }
                        }
                    }
                    break;
                }
            }
            else if (top_nibble == 0xF000)
            {
                table_idx = D_801686CC[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];
                val = D_8014FE54[(table_idx * 8) + ((packed_x >> 9) & 7)];
                if (val != 0)
                {
                    func_8014F210(0x7E, 0x80);
                    switch (val)
                    {
                    case 1:
                        D_80169554 = (void*)((u8*)g_pad_ctx + (g_menu_char_slot * 0x250) + 0x5F0 + (content_type << 6) - 0x170);
                        break;

                    case 6:
                        ((u32*)((u8*)g_pad_ctx + 0x28))[0] |= 2;
                        akao_set_paused(0);
                        break;

                    case 7:
                        ((u32*)((u8*)g_pad_ctx + 0x28))[0] &= ~3;
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
                        ((u32*)((u8*)g_pad_ctx + 0x28))[0] &= ~2;
                        var_s4[0x90] = 0;
                        var_s4[0x13E] = 0;
                        break;

                    case 10:
                        if (var_s4[0xAE] != 0xFF)
                        {
                            g_pad_ctx->inject_flags |= 0x80;
                            g_menu_companion_node = 0x2B;
                            menu_set_active_node(0x2B);
                            var_s0 = 0;
                            goto direction_loop;
                        }
                        else
                        {
                            base = (u8*)g_menu_state_ptr + (*((s32*)((char*)g_menu_state_ptr + 8)));
                            D_801690A8 = base + (*((u16*)(base + 0xCA)));
                            D_801690E0 = base + (*((u16*)(base + 0xCC)));
                            slots = g_menu_slots + 3;
                            for (idxA = 3; idxA >= 0; idxA--)
                            {
                                g_menu_companion_node = 0x2B;
                                slots->active = 0;
                                slots--;
                            }
                            func_8014E3C4(7, 0, &D_801690A8, 0);
                            var_s0 = 0;
                            goto direction_loop;
                        }
                        break;

                    case 11:
                        g_pad_ctx->inject_flags &= ~0x81;
                        g_menu_companion_node = 0xFF;
                        menu_set_active_node(0xFF);
                        var_s0 = 0;
                        goto direction_loop;
                    }
                }
            }
        }

        if (g_pad_input & 0x40)
        {
            func_8014F210(0x7F, 0x80);
            self_idx = g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx;
            if (g_menu_scene_type >= 0x11 || self_idx < 0x14 || self_idx >= 0x1C)
            {
                func_8014519C();
            }
            else
            {
                if (self_idx - 0x14 < 5)
                {
                    g_menu_nodes[(g_menu_char_slot * 3) + 1].idx_nav.s.self_idx = 1;
                    g_menu_nodes[(g_menu_char_slot * 3) + 1].unk0 = (g_menu_char_slot * 3) + 3;
                }
                else if (self_idx - 0x1A < 2)
                {
                    g_menu_nodes[(g_menu_char_slot * 3) + 2].idx_nav.s.self_idx = 2;
                    g_menu_nodes[(g_menu_char_slot * 3) + 2].unk0 = (g_menu_char_slot * 3) + 3;
                }
                g_menu_hit_item_idx = func_8014847C(&g_menu_char_slot);
                if (g_menu_hit_item_idx != (-1))
                {
                    MenuContentItem* temp_s5 = g_menu_content_table[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];
                    g_content_view_x = temp_s5[g_menu_hit_item_idx].packed_x & 0x1FF;
                    g_content_view_y = temp_s5[g_menu_hit_item_idx].y - 8;
                    g_menu_suppress_cursor = 5;
                    g_menu_cursor_enable = 1;
                }
            }
        }

    direction_loop:
        dir_mask = 0x1000;
        for (dir_index = 0; dir_index < 4; dir_index++)
        {
            if (g_pad_input & dir_mask)
                break;
            dir_mask <<= 1;
        }

        if (dir_index != 4)
        {
            func_8014F210(0x7D, 0x80);
            if (s5[g_menu_hit_item_idx].pad[1 + dir_index] == 0xFF)
            {
                if (g_menu_scene_type >= 0x11 || g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx < 0x14 ||
                    g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx >= 0x1C)
                {
                    g_menu_cursor_enable = 2;
                    active_node = &g_menu_nodes[g_menu_active_node];
                    nav_y = (active_node->u8_u.s.nav_y_hi << 1) | ((active_node->idx_nav.nav_x_packed >> 15) & 1);
                    g_content_view_y = nav_y - (g_menu_content_height - 12);
                    if (g_content_view_y < 12)
                        g_content_view_y = 12;
                    if (g_content_view_y >= 0xA3)
                        g_content_view_y = 0xA3;
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
                            D_80169410 = (void*)0;
                            D_80169408 = (void*)0;
                            D_80169404 = (void*)0;
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
 * @brief Clamp the content cursor Y to the valid viewport range and snap both view
 *        axes to the current cursor position.
 * @note Called when the cursor is disabled (g_menu_cursor_enable == 0) after a node
 *       switch, so the viewport re-homes to wherever the cursor was left.  The X
 *       coordinate is derived from the active node's nav_x field (idx_nav.nav_x_packed bits 14:8).
 * @see decomp.me (100%) https://decomp.me/scratch/wBlQo
 */
void menu_snap_view_to_cursor(void)
{
    if (g_content_cursor_y < 12)
    {
        g_content_cursor_y = 12;
    }
    if (g_content_cursor_y >= 163)
    {
        g_content_cursor_y = 163;
    }
    g_content_view_y = g_content_cursor_y;
    g_content_cursor_x = (((u16)g_menu_nodes[g_menu_active_node].idx_nav.nav_x_packed >> 8) & 0x7F) + 8;
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
 * @note For character-page nodes (0x1F = char slot 0, 0x2B = char slot 1), items
 *       17-24 are the 8 equipment/ability swap slots and always return 1.
 *       0x5000 sub-menu items with action_type 1-10 only return 1 when char slot 0
 *       is active (g_menu_char_slot == 0); type 15 (equip) always returns 1.
 *       0xF000 action items look up an action code in D_8014FE54; codes 1 and 6-11
 *       (navigation pointer setup and audio/companion controls) return 1.
 * @see decomp.me (91%) https://decomp.me/scratch/cV0x9
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
                    return 1;
                }
            }
            else
            {
                return g_menu_char_slot == 0;
            }
        }
    }
    else if (type_nibble == 0xF000)
    {
        action_code = D_8014FE54[D_801686CC[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx]][(item->packed_x >> 9) & 7];
        if (action_code == 0)
        {
        }
        else if (action_code == 1)
        {
            return 1;
        }
        else if ((action_code != 0) && (action_code < 12))
        {
            if (action_code >= 6)
            {
                return 1;
            }
        }
    }
    return 0;
}

/**
 * @brief Reset content viewport and cursor to the active node's navigation position.
 * @note  Copies g_menu_default_view_pos into g_content_cursor_x/y when the scene node
 *        has content (content_id != MENU_NONE). Then reconstructs the 9-bit nav cursor Y
 *        from the active node's packed nav fields, writes it to g_content_view_y (clamped
 *        to [12, 0xA3]), and sets g_content_view_x from the 7-bit nav_x column.
 * @see decomp.me TODO
 */
void func_8014519C(void)
{
    MenuNode* scene_node;
    MenuNode* active_node;
    s32 nav_y;

    scene_node = &g_menu_nodes[g_menu_scene_type];
    if (scene_node->content_id != MENU_NONE)
    {
        g_content_cursor_x = g_menu_default_view_pos.x;
        g_content_cursor_y = g_menu_default_view_pos.y;
    }
    g_menu_cursor_enable = 2;
    active_node = &g_menu_nodes[g_menu_active_node];
    nav_y = (active_node->u8_u.s.nav_y_hi << 1) | ((active_node->idx_nav.nav_x_packed >> 15) & 1);
    g_content_view_y = nav_y - (g_menu_content_height - 12);
    if (g_content_view_y < 12)
    {
        g_content_view_y = 12;
    }
    if (g_content_view_y >= 0xA3)
    {
        g_content_view_y = 0xA3;
    }
    g_menu_suppress_cursor = 5;
    g_content_view_x = (((u16)active_node->idx_nav.nav_x_packed >> 8) & 0x7F) + 8;
}

/**
 * @brief Initialize circular prev/next link indices packed into D_801690B8[] entries.
 * @param arg0 Number of entries to initialize (no-op if <= 0).
 * @note  Each s32 element of D_801690B8 holds three packed bit-fields:
 *        bits 13:0  -- (i * 0x10) & 0x3FFF (slot identity / stride field),
 *        bits 22:14 -- previous circular index (wraps: entry 0's prev = arg0 - 1),
 *        bits 30:23 -- next circular index (wraps: last entry's next = 0).
 * @see decomp.me TODO
 */
void func_80145278(s32 arg0)
{
    s32* temp_t0;
    s32 temp_a1;
    s32 temp_a2;
    s32 temp_a3;
    s32 var_a2;
    s32 var_t1;
    s32 var_v1;

    var_t1 = 0;
    if (arg0 > 0)
    {
        do
        {
            temp_t0 = (s32*)((var_t1 * 4) + (u32)D_801690B8);
            var_a2 = var_t1 - 1;
            *temp_t0 = (*temp_t0 & ~0x3FFF) | ((var_t1 * 0x10) & 0x3FFF);
            if (var_a2 < 0)
            {
                var_a2 = arg0 - 1;
            }
            temp_a1 = (*temp_t0 & 0xFF803FFF) | ((var_a2 & 0x1FF) << 0xE);
            *temp_t0 = temp_a1;
            temp_a2 = var_t1 + 1;
            temp_a3 = temp_a2 < arg0;
            var_v1 = 0;
            if (temp_a3 != 0)
            {
                var_v1 = temp_a2;
            }
            *temp_t0 = (temp_a1 & 0x7FFFFF) | (var_v1 << 0x17);
            var_t1 = temp_a2;
        } while (temp_a3 != 0);
    }
}

/**
 * @brief Count set bits across 12 bytes of g_pad_ctx at offset 0x60, then
 *        initialize D_80168C70 as a circular packed linked list of that many entries.
 * @return Number of set bits found (i.e. number of list entries initialized).
 * @note  The bit scan covers bytes [0x60, 0x6B] of g_pad_ctx (12 bytes, 96 bits).
 *        Each s32 word of D_80168C70[] gets the same three packed fields as
 *        func_80145278: bits 13:0 = slot field, bits 22:14 = prev index, bits 30:23 = next.
 * @see decomp.me TODO
 */
s32 func_80145310(void)
{
    s32* temp_a3;
    s32 temp_a0;
    s32 temp_a2;
    s32 temp_v1;
    s32 var_a1;
    s32 var_a1_2;
    s32 var_a2_2;
    s32 var_a3;
    s32 var_t0;
    s32 var_v1;
    s32 var_v1_2;
    u8* var_a2;

    var_t0 = 0;
    var_a3 = 0xB;
    var_a2 = (u8*)g_pad_ctx + 0x60;
    do
    {
        var_v1 = 1;
        var_a1 = 7;
        do
        {
            if (*var_a2 & var_v1)
            {
                var_t0 += 1;
            }
            var_a1 -= 1;
            var_v1 *= 2;
        } while (var_a1 >= 0);
        var_a3 -= 1;
        var_a2 += 1;
    } while (var_a3 >= 0);
    D_80168C70 = (void*)0;
    var_a1_2 = 0;
    if (var_t0 > 0)
    {
        do
        {
            temp_a3 = (s32*)&D_80168C70 + var_a1_2;
            var_a2_2 = var_a1_2 - 1;
            temp_v1 = (*temp_a3 & ~0x3FFF) | ((var_a1_2 * 0x10) & 0x3FFF);
            *temp_a3 = temp_v1;
            if (var_a2_2 < 0)
            {
                var_a2_2 = var_t0 - 1;
            }
            temp_a0 = (temp_v1 & 0xFF803FFF) | ((var_a2_2 & 0x1FF) << 0xE);
            *temp_a3 = temp_a0;
            var_a1_2 += 1;
            temp_a2 = var_a1_2 < var_t0;
            var_v1_2 = 0;
            if (temp_a2 != 0)
            {
                var_v1_2 = var_a1_2;
            }
            *temp_a3 = (temp_a0 & 0x7FFFFF) | (var_v1_2 << 0x17);
        } while (temp_a2 != 0);
    }
    return var_t0;
}

/**
 * @brief Scan g_pad_ctx->unk034 for set bits, find the first match against
 *        a 6-bit field in D_801693FC, then initialize D_80168C70 as a circular
 *        packed linked list of the set-bit entries.
 * @return Low 16 bits: total set-bit count. High 16 bits: index of the first
 *         entry whose bit position matches bits 15:10 of D_801693FC->unk14,
 *         or 0 if no match was found.
 * @note  Scans 11 s32 slots at g_pad_ctx + 0x34, 24 bits each (bits 0-23).
 *        The first match index defaults to 0 when the 0xFF sentinel is never
 *        cleared (no slot matched).
 * @see decomp.me TODO
 */
s32 func_801453F0(void)
{
    s32* temp_t0;
    s32 temp_a0;
    s32 temp_a1;
    s32 temp_a3;
    s32 temp_v1;
    s32 var_a0;
    s32 var_a1;
    s32 var_a2;
    s32 var_a2_2;
    s32 var_t1;
    s32 var_t2;
    s32 var_v1;
    s32 var_v1_2;
    s32* var_a3;

    var_t1 = 0;
    var_t2 = 0xFF;
    var_a0 = 0;
    var_a3 = (s32*)((u8*)g_pad_ctx + 0x34);
    do
    {
        var_v1 = 1;
        var_a2 = 0x17;
        do
        {
            if (*var_a3 & var_v1)
            {
                if ((var_a0 == (((u32)(*(s32*)((u8*)D_801693FC + 0x14)) >> 0xA) & 0x3F)) && (var_t2 == 0xFF))
                {
                    var_t2 = var_t1;
                }
                var_t1 += 1;
            }
            var_a2 -= 1;
            var_v1 *= 2;
        } while (var_a2 >= 0);
        var_a0 += 1;
        var_a3 += 1;
    } while (var_a0 < 0xB);
    if (var_t2 == 0xFF)
    {
        var_t2 = 0;
    }
    D_80168C70 = (void*)0;
    var_a2_2 = 0;
    if (var_t1 > 0)
    {
        do
        {
            temp_t0 = (s32*)&D_80168C70 + var_a2_2;
            var_a1 = var_a2_2 - 1;
            temp_v1 = (*temp_t0 & ~0x3FFF) | ((var_a2_2 * 0x10) & 0x3FFF);
            *temp_t0 = temp_v1;
            if (var_a1 < 0)
            {
                var_a1 = var_t1 - 1;
            }
            temp_a0 = (temp_v1 & 0xFF803FFF) | ((var_a1 & 0x1FF) << 0xE);
            *temp_t0 = temp_a0;
            temp_a1 = var_a2_2 + 1;
            temp_a3 = temp_a1 < var_t1;
            var_v1_2 = 0;
            if (temp_a3 != 0)
            {
                var_v1_2 = temp_a1;
            }
            *temp_t0 = (temp_a0 & 0x7FFFFF) | (var_v1_2 << 0x17);
            var_a2_2 = temp_a1;
        } while (temp_a3 != 0);
    }
    return var_t1 | (var_t2 << 0x10);
}

/**
 * @brief Count entries at g_pad_ctx + 0xCE0 (stride 0x40) whose 2-bit type
 *        field matches arg0, then initialize D_80168C70 as a circular packed
 *        linked list of those entries.
 * @param arg0 2-bit type value to match against bits 9:8 of each entry's
 *             s32 field at offset 0x14.
 * @return Number of matching entries (and entries initialized in D_80168C70).
 * @note  Scans up to 100 entries; stops early on the first entry whose byte
 *        at offset 0 is 0 (sentinel / end-of-list marker).
 * @see decomp.me TODO
 */
s32 func_8014551C(s32 arg0)
{
    s32* temp_t0;
    s32 temp_a0;
    s32 temp_a1;
    s32 temp_a3;
    s32 temp_v1;
    s32 var_a1;
    s32 var_a2;
    s32 var_a2_2;
    s32 var_t1;
    s32 var_v1_2;
    u8* var_v1;

    var_a2 = 0;
    var_t1 = 0;
    var_v1 = (u8*)g_pad_ctx + 0xCE0;
    do
    {
        if (*var_v1 == 0)
        {
            break;
        }
        if ((((u32) * (s32*)(var_v1 + 0x14) >> 8) & 3) == (u32)arg0)
        {
            var_t1 += 1;
        }
        var_a2 += 1;
        var_v1 += 0x40;
    } while (var_a2 < 0x64);
    D_80168C70 = (void*)0;
    var_a2_2 = 0;
    if (var_t1 > 0)
    {
        do
        {
            temp_t0 = (s32*)&D_80168C70 + var_a2_2;
            var_a1 = var_a2_2 - 1;
            temp_v1 = (*temp_t0 & ~0x3FFF) | ((var_a2_2 * 0x10) & 0x3FFF);
            *temp_t0 = temp_v1;
            if (var_a1 < 0)
            {
                var_a1 = var_t1 - 1;
            }
            temp_a0 = (temp_v1 & 0xFF803FFF) | ((var_a1 & 0x1FF) << 0xE);
            *temp_t0 = temp_a0;
            temp_a1 = var_a2_2 + 1;
            temp_a3 = temp_a1 < var_t1;
            var_v1_2 = 0;
            if (temp_a3 != 0)
            {
                var_v1_2 = temp_a1;
            }
            *temp_t0 = (temp_a0 & 0x7FFFFF) | (var_v1_2 << 0x17);
            var_a2_2 = temp_a1;
        } while (temp_a3 != 0);
    }
    return var_t1;
}
/* ----- M2C macros required by func_80145608 ----- */
typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8*)(expr) + (offset)))
#define M2C_BITWISE(type, expr) ((type)(expr))

/* New globals introduced by func_80145608 */
extern s32 D_80042FB4;
extern u16 D_800F0C1C;
extern M2C_UNK D_80105AE0;
extern u8 D_8014FE4E;
extern u16 D_80151A34;
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
extern u8 D_80168C1C[];

/**
 * @brief Main menu item-action dispatch for the current scene.
 * @param arg0 GPU work buffer pointer passed through to render helpers.
 * @param arg1 OT/prim cursor pointer passed through to render helpers.
 * @return Updated GPU work buffer pointer after drawing.
 * @note  Reads the active scene content table and dispatches on item type
 *        packed in each MenuContentItem. Handles navigation, equip, buy/sell,
 *        status, companion, and script-driven actions. Rebuilds D_801690B8
 *        and D_80168C70 circular lists via the four preceding helpers as needed.
 * @see decomp.me (67.06%) https://decomp.me/scratch/D6Nba
 */
void* func_80145608(void* arg0, s32* arg1)
{
    s32 spC0;
    u8 spB0[16];
    s16 spAA;
    s16 spA8;
    u8 sp68[4];
    u8 sp28[4];
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
    u16* var_s4;
    u8 **var_t0_2, **var_t0_3, **var_t0_4, **var_t0_5;
    u8 *var_a2_2, *var_a2_6, *var_a2_27, *var_a2_28;
    u8 *var_v1_2, *var_v1_7, *var_v1_9, *var_v1_12, *var_v1_13, *var_v1_14, *var_v1_15, *var_v1_16, *var_v1_18, *var_v1_20;
    void* var_s1;
    void* var_a0;
    void* var_a2;
    s32 temp_a1_2, temp_a1_3, temp_a1_4, temp_a1_5, temp_a1_6;
    u32 temp_a1_2u, temp_hi;
    s8 temp_v1_13, temp_v1_14, temp_v1_15, temp_v1_16;
    u8 temp_v1_19, temp_v1_21;
    s16 var_v0_12;
    u16 temp_s0_17;
    u32 temp_s0_16, temp_s2;
    s32 temp_t3;
    void *temp_s1, *temp_s1_2, *temp_s1_3;
    void* var_s1_2;
    s32 i, k;
    u32 tmpa0;

    var_s1 = arg0;

    if (g_menu_scene_type == -1)
    {
        var_s3 = &D_80151A34;
        spC0 = D_8014FE4E + 1;
    }
    else
    {
        u8 idx = g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx;
        var_s3 = (u16*)g_menu_content_table[idx];
        spC0 = D_8014FE2C[idx];
    }

    if (var_s3 != NULL)
    {
        if (var_s3 == (u16*)1)
        {
            g_menu_load_request = (s32)var_s3;
        }
        else
        {
            s32 off = g_menu_char_slot * 0x250;
            u8* temp_a2 = (u8*)g_pad_ctx + off + 0x5F0;
            u8* temp_a1 = temp_a2 + 0x50;
            D_801693FC = (u32)temp_a1;
            D_80168C30 = temp_a1;
            D_80168C24 = temp_a1;
            D_80168C20 = temp_a1;
            if (g_menu_scene_type == 0x10)
            {
                u8* temp_v1_2 = temp_a2 + 0x90;
                D_80169410 = (u32)temp_a1;
                D_80169404 = (u32)temp_v1_2;
                D_80169408 = (u32)temp_v1_2;
            }

            if (spC0 != 0)
            {
                var_s4 = var_s3 + 2;
                do
                {
                    spA8 = (s16)(*var_s3 & 0x1FF);
                    spAA = (s16)(*(u8*)var_s4 - 8);
                    switch ((*var_s3) >> 12)
                    {
                    case 1:
                        var_a0 = var_s1;
                        var_a2 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 8));
                        var_v1_2 = (u8*)((u8*)var_a2 + (*(u8*)((u8*)var_s4 + 1) * 2));
                        {
                            s32 a3 = 1;
                            void* a2_2 = (void*)((u8*)var_a2 + (s32)var_v1_2);
                            var_s1 = func_800A88A0(var_a0, arg1, a2_2, a3);
                        }
                        break;

                    case 2:
                    {
                        u8 t0 = 0xFF;
                        u8 b1 = *(u8*)((u8*)var_s4 + 1);
                        if (b1 < 2)
                        {
                            t0 = 0x1E;
                        }
                        else if (b1 - 2 < 8)
                        {
                            s32 a2_3 = 0;
                            if (D_80169404 != 0 && *(u8*)D_80169404 != 0)
                                a2_3 = *(u8*)(D_80169404 + 0x2C);
                            if (((a2_3 >> (b1 - 2)) & 1) != 0)
                                t0 = b1 + 0x3B;
                            else
                                t0 = 0x21;
                        }
                        else if (b1 - 0xA < 8)
                        {
                            s32 a2_4 = 0;
                            if (D_80169404 != 0 && *(u8*)D_80169404 != 0)
                                a2_4 = *(u8*)(D_80169404 + 0x2D);
                            if (((a2_4 >> (b1 - 0xA)) & 1) != 0)
                                t0 = b1 + 0x2B;
                            else
                                t0 = 0x21;
                        }
                        else if (b1 - 0x12 < 8)
                        {
                            s32 a2_4 = 0;
                            if (*(u8*)(D_80168C20 + 0x40) != 0)
                                a2_4 = *(u8*)(D_80168C20 + 0x6C);
                            if (*(u8*)(D_80168C20 + 0x80) != 0)
                                a2_4 |= *(u8*)(D_80168C20 + 0xAC);
                            if (*(u8*)(D_80168C20 + 0xC0) != 0)
                                a2_4 |= *(u8*)(D_80168C20 + 0xEC);
                            if (((a2_4 >> (b1 - 0x12)) & 1) != 0)
                                t0 = b1 + 0x2B;
                            else
                                t0 = 0x21;
                        }
                        else if (b1 - 0x1A < 8)
                        {
                            s32 a2_5 = 0;
                            if (*(u8*)(D_80168C20 + 0x40) != 0)
                                a2_5 = *(u8*)(D_80168C20 + 0x6D);
                            if (*(u8*)(D_80168C20 + 0x80) != 0)
                                a2_5 |= *(u8*)(D_80168C20 + 0xAD);
                            if (*(u8*)(D_80168C20 + 0xC0) != 0)
                                a2_5 |= *(u8*)(D_80168C20 + 0xED);
                            if (((a2_5 >> (b1 - 0x1A)) & 1) != 0)
                                t0 = D_80168696[b1];
                            else
                                t0 = 0x21;
                        }
                        else if (b1 - 0x22 < 8)
                        {
                            t0 = 0x21;
                            if (D_80169410 != 0)
                            {
                                if (((*(u8*)(D_80169410 + 0x2C) >> (b1 - 0x22)) & 1) != 0)
                                    t0 = b1 + 0x13;
                            }
                        }
                        else if (b1 - 0x2A < 8)
                        {
                            s32 idx = 0;
                            void* base2 = *(void**)((u8*)&g_pad_ctx + 0x271C);
                            u8* ptr = (u8*)base2 + g_menu_char_slot * 0x250;
                            while (idx < 8)
                            {
                                if (*(u8*)(ptr + idx + 0x638) == b1 - 0x2A)
                                    break;
                                idx++;
                            }
                            t0 = D_801686B8[idx];
                        }
                        else if (b1 - 0x32 < 4)
                        {
                            if (g_menu_char_slot != 2)
                            {
                                u8* tmp2 = (u8*)D_801693FC + (b1 << 6);
                                t0 = 0x21;
                                if (*(u8*)(tmp2 - 0xC80) != 0)
                                {
                                    u32 val = *(u32*)(tmp2 - 0xC80 + 0x14);
                                    u32 v1 = (val >> 8) & 3;
                                    if (v1 == 1)
                                    {
                                        t0 = ((val >> 10) & 0x3F) + 0x50;
                                    }
                                    else if (v1 == 2)
                                    {
                                        t0 = ((val >> 10) & 0x3F) + 0x5C;
                                    }
                                    else if (v1 == 0)
                                    {
                                        t0 = ((val >> 10) & 0x3F) + 0x45;
                                    }
                                }
                            }
                        }
                        else if (b1 == 0x36)
                        {
                            t0 = 0xFF;
                            if (g_menu_item_ptr != 0)
                            {
                                if (*(u8*)g_menu_item_ptr != 0)
                                {
                                    u32 val = *(u32*)(g_menu_item_ptr + 0x14);
                                    u32 v1 = (val >> 8) & 3;
                                    if (v1 == 1)
                                        t0 = ((val >> 10) & 0x3F) + 0x50;
                                    else if (v1 == 2)
                                        t0 = ((val >> 10) & 0x3F) + 0x5C;
                                }
                            }
                        }
                        else if (b1 - 0x37 < 0x1E)
                        {
                            t0 = D_80168659[b1];
                        }
                        else if (b1 == 0x55)
                        {
                            u32 v0 = (*(u32*)(*(void**)((u8*)&g_pad_ctx + 0x271C) + 0x28) >> 1) & 1;
                            t0 = (v0 != 0) ? 0x6A : 0xFF;
                        }
                        else if (b1 == 0x56)
                        {
                            u32 v0 = (*(u32*)(*(void**)((u8*)&g_pad_ctx + 0x271C) + 0x28) >> 1) & 1;
                            t0 = (v0 != 0) ? 0xFF : 0x6A;
                        }
                        else if (b1 == 0x57)
                        {
                            u32 v0 = *(u32*)(*(void**)((u8*)&g_pad_ctx + 0x271C) + 0x28) & 1;
                            t0 = (v0 != 0) ? 0x6A : 0xFF;
                        }
                        else if (b1 == 0x58)
                        {
                            u32 v0 = *(u32*)(*(void**)((u8*)&g_pad_ctx + 0x271C) + 0x28) & 1;
                            t0 = (v0 != 0) ? 0xFF : 0x6A;
                        }
                        else if (b1 == 0x59)
                        {
                            u32 v0 = *(s32*)(*(void**)((u8*)&g_pad_ctx + 0x271C) + 0x858) & 0x80;
                            t0 = (v0 != 0) ? 0x6A : 0xFF;
                        }
                        else if (b1 == 0x5A)
                        {
                            u32 v0 = *(s32*)(*(void**)((u8*)&g_pad_ctx + 0x271C) + 0x858) & 0x80;
                            t0 = (v0 != 0) ? 0xFF : 0x6A;
                        }
                        else
                        {
                            t0 = 0xFF;
                        }
                        if (t0 != 0xFF)
                        {
                            var_s1 = func_801482D0(func_80149BB4(var_s1, arg1, t0, *var_s3 & 0x1FF, *(u8*)var_s4 - 8, 0, 0, 0, 0), arg1);
                        }
                    }
                    break;

                    case 3:
                    {
                        u8 tmp = *(u8*)((u8*)var_s4 + 1);
                        switch (tmp)
                        {
                        case 0x1:
                            if (g_menu_item_ptr != 0)
                            {
                                var_a0 = var_s1;
                                var_a2_2 = (void*)g_menu_item_ptr;
                                var_s1 = func_800A88A0(var_a0, arg1, var_a2_2, 1);
                            }
                            break;
                        case 0x2:
                            if (g_menu_item_ptr != 0)
                            {
                                u32 v;
                                u32 v1;
                                void* base2;
                                void* a2_6;
                                if (func_8014DE1C((u8*)g_menu_item_ptr) != 0)
                                {
                                    void* a3 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x30));
                                    void* a2 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 8));
                                    func_80148324(&sp68, (void*)((u8*)a3 + (*(u16*)(g_menu_item_ptr + 0x16) & 0x3F) * 2),
                                                  (void*)((u8*)a2 + *(u16*)((u8*)a2 + 0xB4)), a3);
                                }
                                else
                                {
                                    sp68[0] = 0;
                                }
                                v = *(u32*)(g_menu_item_ptr + 0x14);
                                v1 = (v >> 8) & 3;
                                base2 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x68));
                                if (v1 == 0)
                                {
                                    a2_6 = (void*)((u8*)base2 + *(u16*)((u8*)base2 + ((v >> 9) & 0x7E)));
                                }
                                else if (v1 == 1)
                                {
                                    a2_6 = (void*)((u8*)base2 + *(u16*)((u8*)base2 + ((v >> 9) & 0x7E) + 0x16));
                                }
                                else
                                {
                                    a2_6 = (void*)((u8*)base2 + *(u16*)((u8*)base2 + ((v >> 9) & 0x7E) + 0x2E));
                                }
                                func_80148324(&sp28, &sp68, a2_6);
                                var_s1 = func_800A88A0(var_s1, arg1, &sp28, 1);
                            }
                            break;
                        case 0x3:
                            if (g_menu_item_ptr != 0)
                            {
                                var_s1 = func_8014F274(arg1, var_s1, *(u32*)(g_menu_item_ptr + 0x18) & 0xF, 1, &spA8, ((*var_s3 >> 9) & 7));
                            }
                            break;
                        case 0x4:
                            if (g_menu_item_ptr != 0)
                            {
                                var_s1 = func_8014F274(arg1, var_s1, (*(u32*)(g_menu_item_ptr + 0x18) >> 4) & 0xF, 1, &spA8, ((*var_s3 >> 9) & 7));
                            }
                            break;
                        case 0x5:
                            if (g_menu_item_ptr != 0)
                            {
                                var_s1 = func_8014F274(arg1, var_s1, (*(u32*)(g_menu_item_ptr + 0x18) >> 8) & 0xF, 1, &spA8, ((*var_s3 >> 9) & 7));
                            }
                            break;
                        case 0x6:
                            if (g_menu_item_ptr != 0)
                            {
                                var_s1 = func_8014F274(arg1, var_s1, (*(u32*)(g_menu_item_ptr + 0x18) >> 12) & 0xF, 1, &spA8, ((*var_s3 >> 9) & 7));
                            }
                            break;
                        case 0x7:
                            if (g_menu_item_ptr != 0)
                            {
                                var_s1 = func_8014F274(arg1, var_s1, *(u16*)(g_menu_item_ptr + 0x1A) & 0xF, 1, &spA8, ((*var_s3 >> 9) & 7));
                            }
                            break;
                        case 0x8:
                            if (g_menu_item_ptr != 0)
                            {
                                var_s1 = func_8014F274(arg1, var_s1, (*(u32*)(g_menu_item_ptr + 0x18) >> 20) & 0xF, 1, &spA8, ((*var_s3 >> 9) & 7));
                            }
                            break;
                        case 0x9:
                            if (g_menu_item_ptr != 0)
                            {
                                var_s1 = func_8014F274(arg1, var_s1, *(u8*)(g_menu_item_ptr + 0x1B) & 0xF, 1, &spA8, ((*var_s3 >> 9) & 7));
                            }
                            break;
                        case 0xA:
                            if (g_menu_item_ptr != 0)
                            {
                                var_s1 = func_8014F274(arg1, var_s1, (*(u32*)(g_menu_item_ptr + 0x18) >> 28) & 0xF, 1, &spA8, ((*var_s3 >> 9) & 7));
                            }
                            break;
                        case 0xB:
                        case 0xC:
                        case 0xD:
                            if (g_menu_item_ptr != 0)
                            {
                                var_a0 = var_s1;
                                var_a2 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x5C));
                                var_v1_2 = (u8*)((u8*)var_a2 + (*(u8*)(g_menu_item_ptr + (tmp - 0xB + 0x15)) * 2));
                                {
                                    s32 a3 = 1;
                                    void* a2_2 = (void*)((u8*)var_a2 + (s32)var_v1_2);
                                    var_s1 = func_800A88A0(var_a0, arg1, a2_2, a3);
                                }
                            }
                            break;
                        case 0xE:
                        case 0xF:
                        case 0x10:
                            if (D_80169410 != 0)
                            {
                                var_a0 = var_s1;
                                var_a2 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x64));
                                var_v1_2 = (u8*)((u8*)var_a2 + (*(u8*)(D_80169410 + (tmp - 0xE + 0x1A)) * 2));
                                {
                                    s32 a3 = 1;
                                    void* a2_2 = (void*)((u8*)var_a2 + (s32)var_v1_2);
                                    var_s1 = func_800A88A0(var_a0, arg1, a2_2, a3);
                                }
                            }
                            break;
                        case 0x11:
                            if (D_80169410 != 0)
                            {
                                var_s1 = func_800A8A78(arg1, var_s1, *(u16*)(D_80169410 + 0x24), 1, &spA8, ((*var_s3 >> 9) & 7));
                            }
                            break;
                        case 0x12:
                            if (D_80169404 != 0)
                            {
                                var_s1 = func_8014F274(arg1, var_s1, *(u16*)(D_80169404 + 0x24), 1, &spA8, ((*var_s3 >> 9) & 7));
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
                                a3ptr = (u8*)D_80169408;
                                idx = (a3ptr[0x24] * 0xE + a3ptr[0x25]) * 2;
                                a2_2 = (void*)((u8*)a2 + *(u16*)((u8*)a2 + idx));
                                var_s1 = func_800A88A0(var_a0, arg1, a2_2, 1);
                            }
                            break;
                        case 0x14:
                            if (g_menu_item_ptr != 0)
                            {
                                var_a0 = var_s1;
                                var_a2 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x28));
                                var_v1_2 = (u8*)((u8*)var_a2 + (*(u8*)(D_80169408 + 0x24) * 2));
                                {
                                    void* a2_2 = (void*)((u8*)var_a2 + (s32)var_v1_2);
                                    var_s1 = func_800A88A0(var_a0, arg1, a2_2, 1);
                                }
                            }
                            break;
                        case 0x15:
                            if (g_menu_item_ptr != 0)
                            {
                                var_a0 = var_s1;
                                var_a2 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x38));
                                var_v1_2 = (u8*)((u8*)var_a2 + (*(u8*)(D_80169408 + 0x25) * 2));
                                {
                                    void* a2_2 = (void*)((u8*)var_a2 + (s32)var_v1_2);
                                    var_s1 = func_800A88A0(var_a0, arg1, a2_2, 1);
                                }
                            }
                            break;
                        case 0x16:
                            if (g_menu_item_ptr != 0)
                            {
                                var_s1 = func_800A8A78(arg1, var_s1, *(u8*)(D_80169408 + 0x26), 1, &spA8, ((*var_s3 >> 9) & 7));
                            }
                            break;
                        case 0x17:
                        case 0x18:
                        case 0x19:
                        case 0x1A:
                        case 0x1B:
                        case 0x1C:
                        case 0x1D:
                        case 0x1E:
                        {
                            s32 delta = 0;
                            if (D_80168C05[tmp] != 0)
                            {
                                u8* base2 = (u8*)D_80168C30 + (tmp << 6);
                                void* v1;
                                if (*(u8*)(base2 - 0x5C0) != 0)
                                    delta = *(u16*)(base2 - 0x5C0 + 0x24);
                                v1 = (void*)D_801694DC[tmp];
                                if (v1 != 0 && *(u8*)v1 != 0)
                                    delta -= *(u16*)((u8*)v1 + 0x24);
                                else
                                    delta -= D_800F0C1C;
                                if (delta < 0)
                                    delta = -delta;
                                var_s1 = func_800A8A78(arg1, var_s1, (u16)delta, 1, &spA8, ((*var_s3 >> 9) & 7));
                            }
                            if (tmp >= 0x1B && tmp <= 0x1E)
                            {
                                s32 delta2 = 0;
                                if (D_80168C01[tmp] != 0)
                                {
                                    u8* base2 = (u8*)D_80168C30 + (tmp << 6);
                                    void* v1;
                                    if (*(u8*)(base2 - 0x6C0) != 0)
                                        delta2 = *(u16*)(base2 - 0x6C0 + 0x24);
                                    v1 = (void*)D_801694CC[tmp];
                                    if (v1 != 0 && *(u8*)v1 != 0)
                                        delta2 -= *(u16*)((u8*)v1 + 0x24);
                                    else
                                        delta2 -= D_800F0C1C;
                                    if (delta2 >= 0)
                                    {
                                        u8 v0_6 = g_menu_label_key_a.page;
                                        u8 a1_2 = g_menu_label_key_a.entry;
                                        u8* v1_4 = (u8*)((u8*)&g_menu_label_key_a - 0x16);
                                        void* tmp2 = (void*)(a1_2 + ((v0_6 << 8) + (s32)v1_4));
                                        func_800A8E28(&spB0, tmp2, delta2);
                                        ((u8*)&spB0)[func_800A8DDC(tmp2)] = 0;
                                        var_s1 = func_800A88A0(var_s1, arg1, &spB0, 1);
                                    }
                                    else
                                    {
                                        u8 v0_6 = g_menu_label_key_b.page;
                                        u8 a1_2 = g_menu_label_key_b.entry;
                                        u8* v1_4 = (u8*)((u8*)&g_menu_label_key_b - 0x20);
                                        void* tmp2 = (void*)(a1_2 + ((v0_6 << 8) + (s32)v1_4));
                                        func_800A8E28(&spB0, tmp2, delta2);
                                        ((u8*)&spB0)[func_800A8DDC(tmp2)] = 0;
                                        var_s1 = func_800A88A0(var_s1, arg1, &spB0, 1);
                                    }
                                }
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
                                s32 res = func_801483C4(g_menu_item_ptr, tmp - 0x1F, 0);
                                u8 v0_7, a1_3;
                                u8* v1_5;
                                void* tmp2;
                                if (res >= 0)
                                {
                                    v0_7 = g_menu_label_key_a.page;
                                    a1_3 = g_menu_label_key_a.entry;
                                    v1_5 = (u8*)((u8*)&g_menu_label_key_a - 0x16);
                                }
                                else
                                {
                                    v0_7 = g_menu_label_key_b.page;
                                    a1_3 = g_menu_label_key_b.entry;
                                    v1_5 = (u8*)((u8*)&g_menu_label_key_b - 0x20);
                                }
                                tmp2 = (void*)(a1_3 + ((v0_7 << 8) + (s32)v1_5));
                                func_800A8E28(&spB0, tmp2);
                                ((u8*)&spB0)[func_800A8DDC(tmp2)] = 0;
                                var_s1 = func_800A88A0(var_s1, arg1, &spB0, 1);
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
                                s32 diff = func_801483C4(g_menu_item_ptr, tmp - 0x27, 0);
                                if (diff < 0)
                                    diff = -diff;
                                var_s1 = func_8014F274(arg1, var_s1, (u32)diff, 1, &spA8, ((*var_s3 >> 9) & 7));
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
                                s32 v1 = func_801483C4(g_menu_item_ptr, idx2, 0);
                                s32 v2 = func_801483C4((void*)D_8016911C, idx2, 0);
                                s32 diff = v1 - v2;
                                u8 v0_8, a1_4;
                                u8* v1_6;
                                void* tmp2;
                                if (diff >= 0)
                                {
                                    v0_8 = g_menu_label_key_a.page;
                                    a1_4 = g_menu_label_key_a.entry;
                                    v1_6 = (u8*)((u8*)&g_menu_label_key_a - 0x16);
                                }
                                else
                                {
                                    v0_8 = g_menu_label_key_b.page;
                                    a1_4 = g_menu_label_key_b.entry;
                                    v1_6 = (u8*)((u8*)&g_menu_label_key_b - 0x20);
                                }
                                tmp2 = (void*)(a1_4 + ((v0_8 << 8) + (s32)v1_6));
                                func_800A8E28(&spB0, tmp2);
                                ((u8*)&spB0)[func_800A8DDC(tmp2)] = 0;
                                var_s1 = func_800A88A0(var_s1, arg1, &spB0, 1);
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
                                s32 v1 = func_801483C4(g_menu_item_ptr, idx2, 0);
                                s32 v2 = func_801483C4((void*)D_8016911C, idx2, 0);
                                s32 diff = v1 - v2;
                                if (diff < 0)
                                    diff = -diff;
                                var_s1 = func_800A8A78(arg1, var_s1, (u16)diff, 1, &spA8, ((*var_s3 >> 9) & 7));
                            }
                        }
                        break;
                        case 0x3F:
                        case 0x40:
                        case 0x41:
                        case 0x42:
                            if (D_80169404 != 0)
                            {
                                var_s1 = func_8014F274(arg1, var_s1, *(u16*)(D_80169404 + (tmp * 2) - 0x5A), 1, &spA8, ((*var_s3 >> 9) & 7));
                            }
                            break;
                        case 0x43:
                        case 0x44:
                        case 0x45:
                        case 0x46:
                        {
                            s32 has = 0;
                            s32 total = 0;
                            u32* lvar_t0_2;
                            u8* lvar_v1_7;
                            void* tmp2;
                            u8 v0_9, a1_5;
                            u8* v1_8;
                            k = 1;
                            lvar_t0_2 = &g_item_slot_data.unk4;
                            lvar_v1_7 = D_80168C20 + 0x40;
                            do
                            {
                                if (((u8*)&g_item_slot_flags)[k] != 0)
                                {
                                    if (*lvar_v1_7 != 0)
                                    {
                                        total += *(u16*)(lvar_v1_7 + (tmp * 2) - 0x62);
                                    }
                                    tmpa0 = *lvar_t0_2;
                                    if (tmpa0 != 0 && *(u8*)tmpa0 != 0)
                                    {
                                        total -= *(u16*)(tmpa0 + (tmp * 2) - 0x62);
                                    }
                                    has = 1;
                                }
                                lvar_t0_2 += 1;
                                k++;
                                lvar_v1_7 += 0x40;
                            } while (k < 4);
                            if (has)
                            {
                                if (total >= 0)
                                {
                                    v0_9 = g_menu_label_key_a.page;
                                    a1_5 = g_menu_label_key_a.entry;
                                    v1_8 = (u8*)((u8*)&g_menu_label_key_a - 0x16);
                                }
                                else
                                {
                                    v0_9 = g_menu_label_key_b.page;
                                    a1_5 = g_menu_label_key_b.entry;
                                    v1_8 = (u8*)((u8*)&g_menu_label_key_b - 0x20);
                                }
                                tmp2 = (void*)(a1_5 + ((v0_9 << 8) + (s32)v1_8));
                                func_800A8E28(&spB0, tmp2, total, k);
                                ((u8*)&spB0)[func_800A8DDC(tmp2)] = 0;
                                var_s1 = func_800A88A0(var_s1, arg1, &spB0, 1);
                            }
                        }
                        break;
                        case 0x47:
                        case 0x48:
                        case 0x49:
                        case 0x4A:
                        {
                            s32 has = 0;
                            s32 total = 0;
                            u32* lvar_t0_3;
                            u8* lvar_v1_9;
                            k = 1;
                            lvar_t0_3 = &g_item_slot_data.unk4;
                            lvar_v1_9 = D_80168C20 + 0x40;
                            do
                            {
                                if (((u8*)&g_item_slot_flags)[k] != 0)
                                {
                                    if (*lvar_v1_9 != 0)
                                    {
                                        total += *(u16*)(lvar_v1_9 + (tmp * 2) - 0x6A);
                                    }
                                    tmpa0 = *lvar_t0_3;
                                    if (tmpa0 != 0 && *(u8*)tmpa0 != 0)
                                    {
                                        total -= *(u16*)(tmpa0 + (tmp * 2) - 0x6A);
                                    }
                                    has = 1;
                                }
                                lvar_t0_3 += 1;
                                k++;
                                lvar_v1_9 += 0x40;
                            } while (k < 4);
                            if (has)
                            {
                                if (total < 0)
                                    total = -total;
                                var_s1 = func_800A8A78(arg1, var_s1, (u16)total, 1, &spA8, ((*var_s3 >> 9) & 7));
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
                        u8 tmp = *(u8*)((u8*)var_s4 + 1);
                        switch (tmp)
                        {
                        case 0x1:
                        {
                            s32* ptr = (s32*)var_s1;
                            u8 a0_5;
                            u8 v1_10;
                            u8 a0_6;
                            u8 v1_11;
                            ptr[1] = 0x808080;
                            *(s8*)((u8*)var_s1 + 3) = 4;
                            *(s8*)((u8*)var_s1 + 7) = 0x64;
                            *(s16*)((u8*)var_s1 + 8) = (s16)(*var_s3 & 0x1FF);
                            *(s16*)((u8*)var_s1 + 10) = (s16)(*(u8*)var_s4 - 8);
                            a0_5 = (g_menu_char_slot == 2) ? 0xA0 : 0xD0;
                            *(s8*)((u8*)var_s1 + 12) = a0_5;
                            v1_10 = (g_menu_char_slot == 0) ? 0x20 : 0x50;
                            *(s8*)((u8*)var_s1 + 13) = v1_10;
                            *(s32*)((u8*)var_s1 + 0x10) = 0x300030;
                            *(s16*)((u8*)var_s1 + 14) = (s16)((((u16)g_menu_char_slot + 0x1D8) << 6) | 0x11);
                            *(s32*)var_s1 = (*(s32*)var_s1 & 0xFF000000) | (*arg1 & 0xFFFFFF);
                            temp_s1 = (void*)((u8*)var_s1 + 0x14);
                            *arg1 = (*arg1 & 0xFF000000) | ((s32)temp_s1 & 0xFFFFFF);
                            *(s32*)((u8*)temp_s1 + 4) = 0;
                            *(s8*)((u8*)temp_s1 + 3) = 4;
                            *(s8*)((u8*)temp_s1 + 7) = 0x66;
                            *(s16*)((u8*)temp_s1 + 8) = (s16)((*var_s3 & 0x1FF) + 2);
                            *(s16*)((u8*)temp_s1 + 10) = (s16)(*(u8*)var_s4 - 6);
                            a0_6 = (g_menu_char_slot == 2) ? 0xA0 : 0xD0;
                            *(s8*)((u8*)temp_s1 + 12) = a0_6;
                            v1_11 = (g_menu_char_slot == 0) ? 0x20 : 0x50;
                            *(s8*)((u8*)temp_s1 + 13) = v1_11;
                            *(s32*)((u8*)temp_s1 + 0x10) = 0x300030;
                            *(s16*)((u8*)temp_s1 + 14) = (s16)((((u16)g_menu_char_slot + 0x1D8) << 6) | 0x11);
                            *(s32*)((u8*)var_s1 + 0x14) = (*(s32*)((u8*)var_s1 + 0x14) & 0xFF000000) | (*arg1 & 0xFFFFFF);
                            temp_s1_2 = (void*)((u8*)temp_s1 + 0x14);
                            *arg1 = (*arg1 & 0xFF000000) | ((s32)temp_s1 & 0xFFFFFF);
                            *(s8*)((u8*)temp_s1_2 + 3) = 1;
                            *(s32*)((u8*)temp_s1_2 + 4) = 0xE100001F;
                            *(s32*)((u8*)temp_s1 + 0x14) = (*(s32*)((u8*)temp_s1 + 0x14) & 0xFF000000) | (*arg1 & 0xFFFFFF);
                            var_s1 = (void*)((u8*)temp_s1_2 + 8);
                            *arg1 = (*arg1 & 0xFF000000) | ((s32)temp_s1_2 & 0xFFFFFF);
                        }
                        break;
                        case 0x2:
                        {
                            s32 a3;
                            u16* lvar_v1_2;
                            void* lvar_a2;
                            void* a2_2;
                            var_a0 = var_s1;
                            a3 = 1;
                            lvar_v1_2 = *(u16**)((u8*)&g_pad_ctx + 0x271C);
                            lvar_a2 = (void*)((u8*)lvar_v1_2 + g_menu_char_slot * 0x250 + 0x5F0);
                            a2_2 = (void*)((u8*)lvar_a2 + (s32)lvar_v1_2);
                            var_s1 = func_800A88A0(var_a0, arg1, a2_2, a3);
                        }
                        break;
                        case 0x3:
                        {
                            void* base = *(void**)((u8*)&g_pad_ctx + 0x271C);
                            u8 v = *(u8*)((u8*)base + g_menu_char_slot * 0x250 + 0x610);
                            var_s1 = func_8014F274(arg1, var_s1, v, 1, &spA8, ((*var_s3 >> 9) & 7));
                        }
                        break;
                        case 0x4:
                        {
                            s32 v = *(s32*)((u8*)&D_80105AE0 + g_menu_char_slot * 0x23C + 4);
                            var_s1 = func_800A8A78(arg1, var_s1, (u16)v, 1, &spA8, ((*var_s3 >> 9) & 7));
                        }
                        break;
                        case 0x5:
                        {
                            void* base = *(void**)((u8*)&g_pad_ctx + 0x271C);
                            u16 v = *(u16*)((u8*)base + g_menu_char_slot * 0x250 + 0x614);
                            var_s1 = func_800A8A78(arg1, var_s1, v, 1, &spA8, ((*var_s3 >> 9) & 7));
                        }
                        break;
                        case 0x6:
                        {
                            void* base = *(void**)((u8*)&g_pad_ctx + 0x271C);
                            u32 v = *(u32*)((u8*)base + g_menu_char_slot * 0x250 + 0x610);
                            var_s1 = func_800A8A78(arg1, var_s1, (u16)(v >> 8), 1, &spA8, ((*var_s3 >> 9) & 7));
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
                            void* base = *(void**)((u8*)&g_pad_ctx + 0x271C);
                            u16 v = *(u16*)((u8*)base + ((tmp - 7) * 2) + g_menu_char_slot * 0x250 + 0x620);
                            var_s1 = func_8014F274(arg1, var_s1, v >> 9, 1, &spA8, ((*var_s3 >> 9) & 7));
                        }
                        break;
                        case 0xF:
                        {
                            void* a2_2 = (void*)D_801693FC;
                            if (*(u8*)a2_2 != 0)
                            {
                                s32 a3 = 1;
                                if (g_item_slot_flags.slot0 != 0)
                                    a3 = 2;
                                var_s1 = func_800A88A0(var_s1, arg1, a2_2, a3);
                            }
                        }
                        break;
                        case 0x10:
                        case 0x11:
                        case 0x12:
                            if (*(u8*)D_80168C30 != 0)
                            {
                                var_a0 = var_s1;
                                var_a2 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x64));
                                var_v1_2 = (u8*)((u8*)var_a2 + (*(u8*)(D_80168C30 + (tmp - 0x10 + 0x18)) * 2));
                                {
                                    void* a2_2 = (void*)((u8*)var_a2 + (s32)var_v1_2);
                                    var_s1 = func_800A88A0(var_a0, arg1, a2_2, 1);
                                }
                            }
                            break;
                        case 0x13:
                        {
                            u16 a2_15 = 0;
                            if (g_item_slot_flags.slot0 != 0)
                            {
                                void* v1_12 = (void*)g_item_slot_data.unk0;
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
                            var_s1 = func_800A8A78(arg1, var_s1, a2_15, 1, &spA8, ((*var_s3 >> 9) & 7));
                        }
                        break;
                        case 0x14:
                        case 0x15:
                        case 0x16:
                        {
                            u8* base = (u8*)D_801693FC + (tmp << 6);
                            if (*(u8*)(base - 0x4C0) != 0)
                            {
                                s32 a3 = 1;
                                void* a2_2;
                                if (D_80168C09[tmp] != 0)
                                    a3 = 2;
                                a2_2 = (void*)(D_801693FC + (tmp << 6) - 0x4C0);
                                var_s1 = func_800A88A0(var_s1, arg1, a2_2, a3);
                            }
                        }
                        break;
                        case 0x17:
                        case 0x18:
                        {
                            void* base;
                            u8 idx;
                            var_a0 = var_s1;
                            var_a2 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x10));
                            base = *(void**)((u8*)&g_pad_ctx + 0x271C);
                            idx = *(u8*)((u8*)base + g_menu_char_slot * 0x250 + (tmp - 0x17) + 0x5F3);
                            var_v1_2 = (u8*)((u8*)var_a2 + (idx * 2));
                            {
                                void* a2_2 = (void*)((u8*)var_a2 + (s32)var_v1_2);
                                var_s1 = func_800A88A0(var_a0, arg1, a2_2, 1);
                            }
                        }
                        break;
                        case 0x19:
                        {
                            s32 v = func_800B607C(g_menu_char_slot);
                            void* base = *(void**)((u8*)&g_pad_ctx + 0x271C);
                            u32 shift = *(u32*)((u8*)base + g_menu_char_slot * 0x250 + 0x610) >> 8;
                            var_s1 = func_800A8A78(arg1, var_s1, (u16)(v - shift), 1, &spA8, ((*var_s3 >> 9) & 7));
                        }
                        break;
                        case 0x1B:
                        case 0x1C:
                        case 0x1D:
                        case 0x1E:
                        {
                            void* base = *(void**)((u8*)&g_pad_ctx + 0x271C);
                            u8* ptr = (u8*)base + g_menu_char_slot * 0x250 + (tmp - 0x1B) + 0x5F1;
                            if (*ptr != 0xFF)
                            {
                                if (*ptr & 0x80)
                                {
                                    u16* lvar_v1_2 = *(u16**)((u8*)&g_pad_ctx + 0x271C);
                                    void* lvar_a2 = (void*)((u8*)lvar_v1_2 + g_menu_char_slot * 0x250 + 0x5F0);
                                    void* a2_2 = (void*)((u8*)lvar_a2 + ((*ptr & 0x7F) << 6) + 0x150);
                                    var_s1 = func_800A88A0(var_s1, arg1, a2_2, 1);
                                }
                                else
                                {
                                    u32 tmpv;
                                    u8 idx;
                                    var_a0 = var_s1;
                                    var_a2 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x20));
                                    tmpv = ((*(u32*)(D_801693FC + 0x14) >> 10) & 0x3F) * 0x30;
                                    idx = *ptr & 0x7F;
                                    var_v1_2 = (u8*)((u8*)var_a2 + (idx * 2) + tmpv);
                                    {
                                        void* a2_2 = (void*)((u8*)var_a2 + (s32)var_v1_2);
                                        var_s1 = func_800A88A0(var_a0, arg1, a2_2, 1);
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
                            var_a2 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x4C));
                            base = *(void**)((u8*)&g_pad_ctx + 0x271C);
                            idx = *(u8*)((u8*)base + g_menu_char_slot * 0x250 + 0x609);
                            var_v1_2 = (u8*)((u8*)var_a2 + (idx * 2));
                            {
                                void* a2_2 = (void*)((u8*)var_a2 + (s32)var_v1_2);
                                var_s1 = func_800A88A0(var_a0, arg1, a2_2, 1);
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
                            var_s1 = func_8014F274(arg1, var_s1, total, 1, &spA8, ((*var_s3 >> 9) & 7));
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
                            var_s1 = func_8014F274(arg1, var_s1, total, 1, &spA8, ((*var_s3 >> 9) & 7));
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
                            var_s1 = func_8014F274(arg1, var_s1, total, 1, &spA8, ((*var_s3 >> 9) & 7));
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
                            var_s1 = func_8014F274(arg1, var_s1, total, 1, &spA8, ((*var_s3 >> 9) & 7));
                        }
                        break;
                        case 0x3F:
                        {
                            if (g_item_slot_flags.slot0 != 0)
                            {
                                u16 a2_20 = 0;
                                s32 diff;
                                u8 v0_10, a1_10;
                                u8* v1_17;
                                void* tmp2;
                                if (*(u8*)D_80168C20 != 0)
                                    a2_20 = *(u16*)(D_80168C30 + 0x24);
                                if (g_item_slot_data.unk0 != 0 && *(u8*)g_item_slot_data.unk0 != 0)
                                    diff = a2_20 - *(u16*)(g_item_slot_data.unk0 + 0x24);
                                else
                                    diff = a2_20 - D_800F0C1C;
                                if (diff >= 0)
                                {
                                    v0_10 = g_menu_label_key_a.page;
                                    a1_10 = g_menu_label_key_a.entry;
                                    v1_17 = (u8*)((u8*)&g_menu_label_key_a - 0x16);
                                }
                                else
                                {
                                    v0_10 = g_menu_label_key_b.page;
                                    a1_10 = g_menu_label_key_b.entry;
                                    v1_17 = (u8*)((u8*)&g_menu_label_key_b - 0x20);
                                }
                                tmp2 = (void*)(a1_10 + ((v0_10 << 8) + (s32)v1_17));
                                func_800A8E28(&spB0, tmp2, diff);
                                ((u8*)&spB0)[func_800A8DDC(tmp2)] = 0;
                                var_s1 = func_800A88A0(var_s1, arg1, &spB0, 1);
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
                            void* tmp2;
                            u8 v0_11, a1_11;
                            u8* v1_19;
                            k = 1;
                            lvar_t0_4 = &g_item_slot_data.unk4;
                            lvar_v1_18 = D_80168C20 + 0x40;
                            do
                            {
                                if (((u8*)&g_item_slot_flags)[k] != 0)
                                {
                                    if (*lvar_v1_18 != 0)
                                        total += *(u16*)(lvar_v1_18 + (tmp * 2) - 0x5C);
                                    tmpa0 = *lvar_t0_4;
                                    if (tmpa0 != 0 && *(u8*)tmpa0 != 0)
                                        total -= *(u16*)(tmpa0 + (tmp * 2) - 0x5C);
                                    has = 1;
                                }
                                lvar_t0_4 += 1;
                                k++;
                                lvar_v1_18 += 0x40;
                            } while (k < 4);
                            if (has)
                            {
                                if (total >= 0)
                                {
                                    v0_11 = g_menu_label_key_a.page;
                                    a1_11 = g_menu_label_key_a.entry;
                                    v1_19 = (u8*)((u8*)&g_menu_label_key_a - 0x16);
                                }
                                else
                                {
                                    v0_11 = g_menu_label_key_b.page;
                                    a1_11 = g_menu_label_key_b.entry;
                                    v1_19 = (u8*)((u8*)&g_menu_label_key_b - 0x20);
                                }
                                tmp2 = (void*)(a1_11 + ((v0_11 << 8) + (s32)v1_19));
                                func_800A8E28(&spB0, tmp2, total, k);
                                ((u8*)&spB0)[func_800A8DDC(tmp2)] = 0;
                                var_s1 = func_800A88A0(var_s1, arg1, &spB0, 1);
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
                                if (g_item_slot_data.unk0 != 0 && *(u8*)g_item_slot_data.unk0 != 0)
                                    diff = a2_23 - *(u16*)(g_item_slot_data.unk0 + 0x24);
                                else
                                    diff = a2_23 - D_800F0C1C;
                                if (diff < 0)
                                    diff = -diff;
                                var_s1 = func_800A8A78(arg1, var_s1, (u16)diff, 1, &spA8, ((*var_s3 >> 9) & 7));
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
                            k = 1;
                            lvar_t0_5 = &g_item_slot_data.unk4;
                            lvar_v1_20 = D_80168C20 + 0x40;
                            do
                            {
                                if (((u8*)&g_item_slot_flags)[k] != 0)
                                {
                                    if (*lvar_v1_20 != 0)
                                        total += *(u16*)(lvar_v1_20 + (tmp * 2) - 0x66);
                                    tmpa0 = *lvar_t0_5;
                                    if (tmpa0 != 0 && *(u8*)tmpa0 != 0)
                                        total -= *(u16*)(tmpa0 + (tmp * 2) - 0x66);
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
                                var_s1 = func_8014F274(arg1, var_s1, (u32)total, 1, &spA8, ((*var_s3 >> 9) & 7));
                            }
                        }
                        break;
                        case 0x49:
                        {
                            void* base;
                            u8 idx;
                            var_a0 = var_s1;
                            var_a2 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x74));
                            base = *(void**)((u8*)&g_pad_ctx + 0x271C);
                            idx = *(u8*)((u8*)base + g_menu_char_slot * 0x250 + 0x633);
                            var_v1_2 = (u8*)((u8*)var_a2 + (idx * 2));
                            {
                                void* a2_2 = (void*)((u8*)var_a2 + (s32)var_v1_2);
                                var_s1 = func_800A88A0(var_a0, arg1, a2_2, 1);
                            }
                        }
                        break;
                        case 0x4A:
                        {
                            s8 v1 = *(s8*)(*(void**)((u8*)&g_pad_ctx + 0x271C) + 0x29D7);
                            if (v1 >= 0)
                            {
                                void* base = *(void**)((u8*)&g_pad_ctx + 0x271C);
                                u8 v = *(u8*)((u8*)base + v1 * 0x14C + 0x2B50) & 0xF;
                                void* a2 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x80));
                                void* a2_2 = (void*)((u8*)a2 + (v * 2));
                                a2_2 = (void*)((u8*)a2 + *(u16*)a2_2);
                                var_s1 = func_800A88A0(var_s1, arg1, a2_2, 1);
                            }
                        }
                        break;
                        case 0x4B:
                        {
                            s8 v1 = *(s8*)(*(void**)((u8*)&g_pad_ctx + 0x271C) + 0x29D7);
                            if (v1 >= 0)
                            {
                                void* base = *(void**)((u8*)&g_pad_ctx + 0x271C);
                                u8 v = *(u8*)((u8*)base + v1 * 0x14C + 0x2B50) >> 4;
                                void* a2 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x7C));
                                void* a2_2 = (void*)((u8*)a2 + (v * 2));
                                a2_2 = (void*)((u8*)a2 + *(u16*)a2_2);
                                var_s1 = func_800A88A0(var_s1, arg1, a2_2, 1);
                            }
                        }
                        break;
                        case 0x4C:
                        {
                            s8 v1 = *(s8*)(*(void**)((u8*)&g_pad_ctx + 0x271C) + 0x29D7);
                            if (v1 >= 0)
                            {
                                void* base = *(void**)((u8*)&g_pad_ctx + 0x271C);
                                u8 v = *(u8*)((u8*)base + v1 * 0x14C + 0x2B52);
                                var_s1 = func_800A8A78(arg1, var_s1, v, 1, &spA8, ((*var_s3 >> 9) & 7));
                            }
                        }
                        break;
                        case 0x4D:
                        {
                            s8 v1 = *(s8*)(*(void**)((u8*)&g_pad_ctx + 0x271C) + 0x29D7);
                            if (v1 >= 0)
                            {
                                void* base;
                                s32 idx;
                                var_a0 = var_s1;
                                var_a2 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x70));
                                base = *(void**)((u8*)&g_pad_ctx + 0x271C);
                                idx = *(s32*)((u8*)base + v1 * 0x14C + 0x2B54);
                                var_v1_2 = (u8*)((u8*)var_a2 + (idx * 2));
                                {
                                    void* a2_2 = (void*)((u8*)var_a2 + (s32)var_v1_2);
                                    var_s1 = func_800A88A0(var_a0, arg1, a2_2, 1);
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
                            void* base = *(void**)((u8*)&g_pad_ctx + 0x271C);
                            u8* ptr = (u8*)base + g_menu_char_slot * 0x250;
                            switch (tmp)
                            {
                            case 0x4E:
                                a2_26 = *(u32*)(ptr + 0x658) & 0xF;
                                break;
                            case 0x4F:
                                a2_26 = (*(u8*)(ptr + 0x658) >> 4);
                                break;
                            case 0x50:
                                a2_26 = (*(u32*)(ptr + 0x658) >> 8) & 0xF;
                                break;
                            case 0x51:
                                a2_26 = (*(u32*)(ptr + 0x658) >> 12) & 0xF;
                                break;
                            case 0x52:
                                a2_26 = *(u16*)(ptr + 0x65A) & 0xF;
                                break;
                            case 0x53:
                                a2_26 = (*(u32*)(ptr + 0x658) >> 20) & 0xF;
                                break;
                            case 0x54:
                                a2_26 = *(u8*)(ptr + 0x65B) & 0xF;
                                break;
                            case 0x55:
                                a2_26 = *(u32*)(ptr + 0x658) >> 28;
                                break;
                            }
                            var_s1 = func_8014F274(arg1, var_s1, a2_26, 1, &spA8, ((*var_s3 >> 9) & 7));
                        }
                        break;
                        default:
                            break;
                        }
                    }
                    break;

                    case 6:
                    {
                        u8 tmp = *(u8*)((u8*)var_s4 + 1);
                        if (tmp == 1)
                        {
                            u32 val = *(u32*)(*(void**)((u8*)&g_pad_ctx + 0x271C) + 0x2C);
                            if (val > 0x989680U)
                                var_s1 = func_800A8A78(arg1, var_s1, 0x989680U, 1, &spA8, ((*var_s3 >> 9) & 7));
                            else
                                var_s1 = func_800A8A78(arg1, var_s1, (u16)val, 1, &spA8, ((*var_s3 >> 9) & 7));
                        }
                        else if (tmp == 2)
                        {
                            u32 ltemp_v0_6;
                            s32 v1;
                            s32 diff;
                            s32 quotient;
                            s32 s2;
                            u16 remainder;
                            ltemp_v0_6 = (*var_s3 >> 9) & 7;
                            if (ltemp_v0_6 == 1)
                                spA8 -= 0x32;
                            else if (ltemp_v0_6 == tmp)
                                spA8 -= 0x19;
                            v1 = *(s32*)(*(void**)((u8*)&g_pad_ctx + 0x271C) + 0x30) + VSync(-1);
                            diff = v1 - D_80042FB4;
                            quotient = diff / 6750;
                            spA8 += 0x14;
                            s2 = quotient >> 5;
                            var_s1_2 = (void*)func_800A8A78(arg1, var_s1, (u16)s2, 1, &spA8, 1);
                            if ((g_frame_counter / 15) & 1)
                            {
                                var_s1_2 = func_800A88A0(var_s1_2, arg1, ":", 1);
                            }
                            spA8 += 7;
                            remainder = (diff / 3600) - (s2 * 0x3C);
                            if (remainder < 0xA)
                                var_s1_2 = (void*)func_800A8A78(arg1, var_s1_2, 0U, 1, &spA8, 0);
                            spA8 += 0x10;
                            var_s1 = (void*)func_800A8A78(arg1, var_s1_2, remainder, 1, &spA8, 1);
                        }
                    }
                    break;

                    case 7:
                    {
                        u8 tmp = *(u8*)((u8*)var_s4 + 1);
                        if (tmp == 1)
                        {
                            void* base = *(void**)((u8*)&g_pad_ctx + 0x271C) + g_menu_char_slot * 0x250;
                            if ((*(u8*)((u8*)base + 0x608) & 0x7F) != 2 || ((*(u8*)((u8*)base + 0x609) != 5) && (*(u8*)((u8*)base + 0x609) != 8)))
                            {
                                var_a0 = var_s1;
                                var_a2 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 8));
                                var_v1_2 = (u8*)((u8*)var_a2 + *(u16*)((u8*)var_a2 + 0x74));
                                {
                                    void* a2_2 = (void*)((u8*)var_a2 + (s32)var_v1_2);
                                    var_s1 = func_800A88A0(var_a0, arg1, a2_2, 1);
                                }
                            }
                        }
                        else if (tmp == 2)
                        {
                            void* base = *(void**)((u8*)&g_pad_ctx + 0x271C) + g_menu_char_slot * 0x250;
                            if ((*(u8*)((u8*)base + 0x608) & 0x7F) != 2 || ((*(u8*)((u8*)base + 0x609) != 5) && (*(u8*)((u8*)base + 0x609) != 8)))
                            {
                                var_a0 = var_s1;
                                var_a2 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 8));
                                var_v1_2 = (u8*)((u8*)var_a2 + *(u16*)((u8*)var_a2 + 0x76));
                                {
                                    void* a2_2 = (void*)((u8*)var_a2 + (s32)var_v1_2);
                                    var_s1 = func_800A88A0(var_a0, arg1, a2_2, 1);
                                }
                            }
                        }
                    }
                    break;

                    default:
                        break;
                    }

                    var_s4 += 8;
                    spC0--;
                } while (spC0 != 0);
            }

            {
                MenuNode* node = &g_menu_nodes[g_menu_scene_type];
                if (node->unk0 == 0x13)
                {
                    u8* a2_27 = (u8*)g_menu_item_ptr;
                    if (*a2_27 != 0)
                    {
                        var_s1 = func_800A88A0(var_s1, arg1, a2_27, 1);
                    }
                }
                else if (g_menu_scene_type == 0x1D)
                {
                    void* a2_28 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 8));
                    u16 v1 = *(u16*)((u8*)a2_28 + 0x78);
                    void* a2_27 = (void*)((u8*)a2_28 + v1);
                    var_s1 = func_800A88A0(var_s1, arg1, a2_27, 1);
                }
                else if (g_menu_scene_type != -1)
                {
                    void* a2_28 = (void*)((u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 8));
                    u16 v1 = *(u16*)((u8*)a2_28 + node->unk0 * 2);
                    void* a2_27 = (void*)((u8*)a2_28 + v1);
                    var_s1 = func_800A88A0(var_s1, arg1, a2_27, 1);
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
                s32 la2_29;
                void* ltemp;
                spA8 = 0x88;
                spAA = 0x28;
                la2_29 = (g_menu_page_count != 0) ? (g_script_repeat_last + 1) : 0;
                ltemp = func_800AD208(arg1, var_s1, la2_29, 3, &spA8, 0);
                ltemp = func_800AD524(ltemp, arg1, 0xB, &spA8, 0);
                spA8 += 8;
                ltemp = func_800AD208(arg1, ltemp, g_menu_page_count, 3, &spA8, 0);
                ltemp = func_800AD524(ltemp, arg1, 0xB, &spA8, 0);
                spA8 += 8;
                var_s1 = func_800AD208(arg1, ltemp, func_8014F23C(), 3, &spA8, 0);
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
 * @brief Scan up to arg0 entries in D_801693FC and OR together lookup bytes keyed by bits 10-15 of unk14.
 * @param arg0 Maximum number of entries to inspect (loop exits early when the index equals this value).
 * @return Bitwise OR of the looked-up bytes from each active entry, or 0 if none are active.
 * @note Iterates at most 4 entries (indices 0-3). An entry is skipped when its byte at offset 0 is zero.
 *       Bits 8-9 of unk14 select the table: set uses D_800F0BEC, clear uses D_800F0BE0. Bits 10-15 form
 *       the 6-bit index into whichever table is selected.
 * @see decomp.me TODO
 */
s32 func_8014824C(s32 arg0)
{
    s32 i;
    s32 result;
    u8* entry;
    u32 unk14;
    u32 idx;

    result = 0;
    entry = (u8*)D_801693FC;
    i = 0;
    do
    {
        if ((i != arg0) && (entry[0] != 0))
        {
            unk14 = *(u32*)(entry + 0x14);
            idx = (unk14 >> 10) & 0x3F;
            if (unk14 & 0x300)
            {
                result |= D_800F0BEC[idx];
            }
            else
            {
                result |= D_800F0BE0[idx];
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
 * @param ot   Pointer to the ordering-table entry that the new primitive should be prepended to.
 * @return Pointer to the byte immediately following the 8-byte primitive (next free prim slot).
 * @note Mirrors the inline OT-link pattern used throughout menu.c (e.g. around line 1361).
 *       Sets word count to 1 and hard-codes the mode word to 0xE1000005.
 * @see decomp.me (100%) https://decomp.me/scratch/3Wup8
 */
void* func_801482D0(MenuPrimHead* prim, s32* ot)
{
    prim->_u._s.unk3 = 1;
    prim->unk4 = 0xE1000005;
    prim->_u.unk0 = (s32)((prim->_u.unk0 & 0xFF000000) | (*ot & 0xFFFFFF));
    *ot = (*ot & 0xFF000000) | ((s32)prim & 0xFFFFFF);
    return (void*)((u8*)prim + 8);
}

/**
 * @brief Concatenate two encoded text command streams into a destination buffer.
 * @param dst  Destination byte buffer; receives all bytes from src1, then src2, then a null terminator.
 * @param src1 First source stream; processed until its null terminator.
 * @param src2 Second source stream; appended after src1, processed until its null terminator.
 * @note Both streams use a variable-width encoding: bytes in the range 0x19-0x1F are two-byte codes
 *       (the control byte followed by one parameter byte); all other non-zero bytes are single-byte codes.
 *       The function tail-calls itself in the original asm to loop, so the effective max depth is bounded
 *       by the stream lengths, not the call stack.
 * @see decomp.me TODO
 */
void func_80148324(u8* dst, u8* src1, u8* src2)
{
    u8 ch;

    ch = *src1;
    while (ch != 0)
    {
        *dst++ = ch;
        src1++;
        if ((u32)(ch - 0x19U) < 7U)
        {
            *dst++ = *src1++;
        }
        ch = *src1;
    }

    ch = *src2;
    while (ch != 0)
    {
        *dst++ = ch;
        src2++;
        if ((u32)(ch - 0x19U) < 7U)
        {
            *dst++ = *src2++;
        }
        ch = *src2;
    }

    *dst = 0;
}

extern s8 D_800F0C38[];

/**
 * @brief Extract a 4-bit nibble from a packed u32 field in the item struct and look it up in a byte table.
 * @param item  Pointer to the item data record; nibbles are packed into the u32 at byte offset 0x1C.
 * @param index Nibble selector (0-7); selects bits [index*4 .. index*4+3] of the packed word.
 *              If >= 8 the switch is skipped and @p fallback is used as the table index instead.
 * @param fallback Default table index used when @p index is out of range.
 * @return Signed byte from D_800F0C38 at the selected nibble index.
 * @note Cases 1, 4, and 6 load the nibble via byte/halfword access rather than the full word;
 *       this reflects the original compiler output and must be preserved for match work.
 * @see decomp.me TODO
 */
s8 func_801483C4(void* item, u32 index, u32 fallback)
{
    u32 nibble;
    u32 word;

    nibble = fallback;
    if (index < 8U)
    {
        word = *(u32*)((u8*)item + 0x1C);
        switch (index)
        {
        case 0:
            nibble = word & 0xF;
            break;
        case 1:
            nibble = *(u8*)((u8*)item + 0x1C) >> 4;
            break;
        case 2:
            nibble = (word >> 8) & 0xF;
            break;
        case 3:
            nibble = (word >> 12) & 0xF;
            break;
        case 4:
            nibble = *(u16*)((u8*)item + 0x1E) & 0xF;
            break;
        case 5:
            nibble = (word >> 20) & 0xF;
            break;
        case 6:
            nibble = *(u8*)((u8*)item + 0x1F) & 0xF;
            break;
        case 7:
            nibble = word >> 28;
            break;
        }
    }
    return D_800F0C38[nibble];
}

extern u8 D_8014FE2C[];

/**
 * @brief Search the current scene's content table for the first item flagged as the active hit item.
 * @return Index into the content table array of the first matching MenuContentItem, or -1 if none found.
 * @note Looks up the current node's self_idx from g_menu_nodes[g_menu_scene_type], then resolves
 *       the item count via D_8014FE2C[D_801686CC[self_idx]] and the item array via
 *       g_menu_content_table[self_idx].  An item matches when bits 12-15 of packed_x are 0xF or 0x5
 *       AND bits 9-11 are all set (== 0xE00).
 * @see decomp.me TODO
 */
s32 func_8014847C(void)
{
    u8 self_idx;
    u8 count;
    MenuContentItem* items;
    s32 i;
    u16 packed_x;
    u16 upper;

    if (g_menu_scene_type == -1)
    {
        return -1;
    }

    self_idx = g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx;
    count = D_8014FE2C[D_801686CC[self_idx]];
    items = g_menu_content_table[self_idx];

    if (count == 0)
    {
        return -1;
    }

    for (i = 0; i < (s32)count; i++, items++)
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
 * @return Zero-based index of @p node_id within the navigation list starting at g_menu_nav_first,
 *         or -1 if not found or the list is empty.
 * @note The navigation list is a flat s32 array beginning at g_menu_nav_first with g_menu_nav_count
 *       entries; each element is one node ID.
 * @see decomp.me TODO
 */
s32 func_8014852C(s32 node_id)
{
    s32* nav;
    s32 i;

    if (g_menu_nav_count <= 0)
    {
        return -1;
    }

    nav = &g_menu_nav_first;
    for (i = 0; i < g_menu_nav_count; i++, nav++)
    {
        if (*nav == node_id)
        {
            return i;
        }
    }

    return -1;
}

/**
 * @brief Emit up to two scroll-arrow SPRT primitives and a trailing Draw Mode Setting primitive.
 * @param buf   Destination primitive buffer; each arrow occupies 0x14 bytes, the Draw Mode tail 8 bytes.
 * @param ot    Pointer to the ordering-table entry to prepend each emitted primitive to.
 * @param state Opaque scroll-state record; the following byte-offset fields are read:
 *              - 0x06 (u16): total-item count, bottom 9 bits used (stride 0x10 each).
 *              - 0x08 (u16): Y-base coordinate.
 *              - 0x0A (u16): texture V origin for the up-arrow sprite.
 *              - 0x0C (u16): Y offset added to Y-base.
 *              - 0x0E (s16): current scroll position in pixels; also used as UV delta for down arrow.
 *              - 0x12 (u16): number of items scrolled above the viewport (0 = at top).
 * @return Pointer to the next free byte in @p buf after all emitted primitives.
 * @note Up arrow emitted when unk12 != 0 (content hidden above viewport).
 *       Down arrow emitted when (unk0E-16) < ((unk06 & 0x1FF)*16 - unk12) (content hidden below).
 *       If either arrow was emitted, appends a one-word DR_MODE (0xE1000005) to restore draw mode.
 *       All arrow sprites use GPU code 0x64 (semi-transparent texture-mapped variable sprite).
 * @see decomp.me TODO
 */
void* func_80148578(void* buf, s32* ot, void* state)
{
    u8* prim = (u8*)buf;
    u8* st = (u8*)state;
    s32 emitted = 0;
    s32 addr;
    u16 scroll_top;
    s16 y;

    scroll_top = *(u16*)(st + 0x12);

    if (scroll_top != 0)
    {
        y = (s16)(*(u16*)(st + 0x8) + *(u16*)(st + 0xC) - 0x10);
        emitted = 1;
        *(u32*)(prim + 0x4) = 0x808080;
        prim[3] = 4;
        prim[7] = 0x64;
        *(s16*)(prim + 0x8) = y;
        *(u16*)(prim + 0xA) = *(u16*)(st + 0xA);
        *(u16*)(prim + 0xC) = 0x1080;
        *(u16*)(prim + 0xE) = 0x7C86;
        *(u32*)(prim + 0x10) = 0x100010;
        addr = (s32)prim & 0xFFFFFF;
        *(s32*)prim = (*(s32*)prim & 0xFF000000) | (*ot & 0xFFFFFF);
        *ot = (*ot & 0xFF000000) | addr;
        prim += 0x14;
    }

    {
        s32 max = (s32)((*(u16*)(st + 0x6) & 0x1FF) * 0x10) - (s32)scroll_top;
        if ((s32)(*(s16*)(st + 0xE) - 0x10) < max)
        {
            y = (s16)(*(u16*)(st + 0x8) + *(u16*)(st + 0xC) - 0x10);
            emitted += 1;
            *(u32*)(prim + 0x4) = 0x808080;
            prim[3] = 4;
            prim[7] = 0x64;
            *(s16*)(prim + 0x8) = y;
            *(u16*)(prim + 0xA) = (s16)(*(u16*)(st + 0xA) + *(u16*)(st + 0xE) - 8);
            *(u16*)(prim + 0xC) = 0x2080;
            *(u16*)(prim + 0xE) = 0x7C86;
            *(u32*)(prim + 0x10) = 0x100010;
            addr = (s32)prim & 0xFFFFFF;
            *(s32*)prim = (*(s32*)prim & 0xFF000000) | (*ot & 0xFFFFFF);
            *ot = (*ot & 0xFF000000) | addr;
            prim += 0x14;
        }
    }

    if (emitted != 0)
    {
        addr = (s32)prim & 0xFFFFFF;
        prim[3] = 1;
        *(u32*)(prim + 0x4) = 0xE1000005;
        *(s32*)prim = (*(s32*)prim & 0xFF000000) | (*ot & 0xFFFFFF);
        *ot = (*ot & 0xFF000000) | addr;
        prim += 8;
    }

    return prim;
}

/**
 * @brief Emit up to two fixed-position scroll-arrow SPRT primitives driven by global scroll state.
 * @param buf Destination primitive buffer; each arrow occupies 0x14 bytes, the Draw Mode tail 8 bytes.
 * @param ot  Pointer to the ordering-table entry to prepend each emitted primitive to.
 * @return Pointer to the next free byte in @p buf after all emitted primitives.
 * @note Up arrow (unkA=3)    emitted when g_menu_content_height != 0 (content scrolled up).
 *       Down arrow (unkA=0xBA) emitted when (g_menu_layout_end - g_menu_content_height) >= 0xAC
 *       (content extends below viewport). Both use fixed X=0x20, code=0x64, color=0x808080.
 *       If either arrow was emitted, appends a one-word DR_MODE (0xE1000005). Mirrors the
 *       pattern of func_80148578 but reads from globals rather than a state struct.
 * @see decomp.me TODO
 */
void* func_8014874C(void* buf, s32* ot)
{
    u8* prim = (u8*)buf;
    s32 addr;

    if (g_menu_content_height != 0)
    {
        *(u32*)(prim + 0x4) = 0x808080;
        prim[3] = 4;
        prim[7] = 0x64;
        *(u16*)(prim + 0x8) = 0x20;
        *(u16*)(prim + 0xA) = 3;
        *(u16*)(prim + 0xC) = 0x1080;
        *(u16*)(prim + 0xE) = 0x7C86;
        *(u32*)(prim + 0x10) = 0x100010;
        addr = (s32)prim & 0xFFFFFF;
        *(s32*)prim = (*(s32*)prim & 0xFF000000) | (*ot & 0xFFFFFF);
        *ot = (*ot & 0xFF000000) | addr;
        prim += 0x14;
    }

    if ((g_menu_layout_end - g_menu_content_height) >= 0xAC)
    {
        *(u32*)(prim + 0x4) = 0x808080;
        prim[3] = 4;
        prim[7] = 0x64;
        *(u16*)(prim + 0x8) = 0x20;
        *(u16*)(prim + 0xA) = 0xBA;
        *(u16*)(prim + 0xC) = 0x2080;
        *(u16*)(prim + 0xE) = 0x7C86;
        *(u32*)(prim + 0x10) = 0x100010;
        addr = (s32)prim & 0xFFFFFF;
        *(s32*)prim = (*(s32*)prim & 0xFF000000) | (*ot & 0xFFFFFF);
        *ot = (*ot & 0xFF000000) | addr;
        prim += 0x14;
    }

    if ((g_menu_content_height != 0) || (g_menu_layout_end >= 0xAC))
    {
        addr = (s32)prim & 0xFFFFFF;
        prim[3] = 1;
        *(u32*)(prim + 0x4) = 0xE1000005;
        *(s32*)prim = (*(s32*)prim & 0xFF000000) | (*ot & 0xFFFFFF);
        *ot = (*ot & 0xFF000000) | addr;
        prim += 8;
    }

    return prim;
}

s32 func_8014A10C(s32, s32*, s32, s32, s32);

/**
 * @brief Draw the navigation cursor for the active menu node, and optionally its label.
 * @param buf   Primitive buffer pointer passed through to the rendering helpers.
 * @param ot    Pointer to the ordering-table entry used by the rendering helpers.
 * @param label When non-zero, also draws the node's text label from the g_menu_state_ptr string table.
 * @return Updated primitive buffer pointer returned from the last rendering call.
 * @note Cursor X = ((nav_x & 0x7F) + 8); cursor Y = ((nav_y_hi << 1) | (nav_x >> 7)) -
 *       (g_menu_content_height - 0xC), clamped to [0xC, 0xA2].
 *       Label pointer: base = (u8*)g_menu_state_ptr + *(s32*)(state + 4);
 *                      text = base + *(u16*)(base + node->unk0 * 2).
 * @see decomp.me TODO
 */
s32 func_80148900(s32 buf, s32* ot, s32 label)
{
    MenuNode* node;
    u16 nav_x_packed;
    s32 cursor_x;
    s32 cursor_y;
    s32 result;
    u8* base;

    node = &g_menu_nodes[g_menu_active_node];
    nav_x_packed = node->idx_nav.nav_x_packed;

    cursor_y = (s32)((node->u8_u.s.nav_y_hi << 1) | (nav_x_packed >> 15)) - (g_menu_content_height - 0xC);
    if (cursor_y < 0xC)
    {
        cursor_y = 0xC;
    }
    if (cursor_y >= 0xA3)
    {
        cursor_y = 0xA3;
    }

    cursor_x = ((nav_x_packed >> 8) & 0x7F) + 8;
    result = func_8014A10C(buf, ot, cursor_x, cursor_y, 1);

    if (label != 0)
    {
        base = (u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 4);
        result = func_800A88A0(result, ot, base + *(u16*)(base + node->unk0 * 2), 1, 0xA0, 0xCA, 2);
    }

    return result;
}

extern s32 D_80168C6C;
void* func_80149D90(void*, s32*, s16, s16);
s32 func_8014DE1C(s32);

/**
 * @brief Render the content cursor, update its lerped position, and optionally render the active hit-item label.
 * @param arg0 Current primitive buffer pointer.
 * @param arg1 Pointer to the current ordering-table entry.
 * @param arg2 Non-zero to also render the label string for the active hit item.
 * @return Updated primitive buffer pointer after all emitted primitives.
 * @note The cursor position (g_content_cursor_x, g_content_cursor_y) is lerped toward
 *       (g_content_view_x, g_content_view_y) using g_menu_suppress_cursor as step count,
 *       then snapped when the counter reaches zero.  The label path dispatches via two
 *       switch statements on MenuContentItem::packed_x bits [15:12] and pad[0]:
 *       the 0xF000 family selects a string table offset from g_menu_state_ptr;
 *       the 0x5000 family additionally considers g_pad_ctx slot data and may copy
 *       encoded text into stack scratch buffers before rendering.
 *       A final sprite from D_801690B0 is appended and OT-linked when unk3 != 0xFF.
 * @see decomp.me TODO
 */
void* func_80148A20(void* arg0, s32* arg1, s32 arg2)
{
    u8 sp60[0x40];
    u8 sp20[0x40];
    void* var_s0;
    s32* var_a1;
    s32 var_s1;
    s32 var_t0;
    u8 var_v0_2;
    s32 var_v0;
    u8* var_a2;
    u16* var_v0_3;
    u16 var_v1;
    u8* var_a2_2;
    MenuContentItem* content_base;
    MenuContentItem* hit_item;
    u16 upper;
    u8 item_sub;
    u8 ch;
    u8* dst;
    u8* src;
    MenuPrimHead* ot_head;

    var_s0 = arg0;

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
        var_s0 = func_8014A10C(var_s0, var_a1, g_content_cursor_x, g_content_cursor_y, var_s1);
    }

    if (g_menu_suppress_cursor != 0)
    {
        s32 dx = (g_content_view_x - g_content_cursor_x) / g_menu_suppress_cursor;
        s32 dy = (g_content_view_y - g_content_cursor_y) / g_menu_suppress_cursor;
        g_menu_suppress_cursor -= 1;
        g_content_cursor_x += dx;
        g_content_cursor_y += dy;
    }
    else
    {
        g_content_cursor_x = g_content_view_x;
        g_content_cursor_y = g_content_view_y;
    }

    if (arg2 != 0)
    {
        content_base = g_menu_content_table[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];
        hit_item = &content_base[g_menu_hit_item_idx];
        upper = hit_item->packed_x & 0xF000;

        if (upper == 0xF000)
        {
            item_sub = hit_item->pad[0];
            if ((u8)item_sub < 0xF0U)
            {
                var_v0 = (s32)item_sub * 2;
                var_a2 = (u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x4);
                goto block_103;
            }
            switch (item_sub)
            {
            case 0xF0:
                var_v0_2 = item_sub;
                var_a2 = (u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x48);
                goto block_102;
            case 0xF1:
                var_v0_2 = item_sub;
                var_a2 = (u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x14);
                goto block_102;
            case 0xF2:
                var_v0_2 = item_sub;
                var_a2 = (u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x34);
                goto block_102;
            case 0xF3:
                var_v0_2 = item_sub;
                var_a2 = (u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x24);
                goto block_102;
            case 0xF4:
                var_v0_2 = item_sub;
                var_a2 = (u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x40);
                goto block_102;
            case 0xF5:
                var_v0_2 = item_sub;
                var_a2 = (u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x58);
                goto block_102;
            case 0xF6:
                var_v0_2 = item_sub;
                var_a2 = (u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x50);
                goto block_102;
            case 0xF7:
                var_v0_2 = item_sub;
                var_a2 = (u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x60);
                goto block_102;
            case 0xF8:
                if (D_80168C6C != 0xFF)
                {
                    if (D_80168C6C & 0x80)
                    {
                        var_v0_2 = item_sub;
                        var_a2 = (u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x40);
                    }
                    else
                    {
                        var_v0_2 = item_sub;
                        var_a2 = (u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x1C);
                    }
                    goto block_102;
                }
                break;
            case 0xF9:
                var_v0_2 = item_sub;
                var_a2 = (u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x1C);
                goto block_102;
            case 0xFA:
                var_v0_2 = item_sub;
                var_a2 = (u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x3C);
                goto block_102;
            case 0xFB:
            case 0xFC:
            case 0xFD:
            case 0xFE:
                var_v0_2 = item_sub;
                var_a2 = (u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x2C);
                goto block_102;
            }
        }
        else if (upper == 0x5000)
        {
            item_sub = hit_item->pad[0];
            switch (item_sub)
            {
            case 1:
            case 2:
                var_t0 = *(s32*)((u8*)g_menu_state_ptr + 0x3C);
                var_v0_2 = ((u8*)g_pad_ctx)[(g_menu_char_slot * 0x250) + item_sub + 0x609];
            block_101:
                var_a2 = (u8*)g_menu_state_ptr + var_t0;
            block_102:
                var_v0 = (s32)var_v0_2 * 2;
            block_103:
                var_v0_3 = (u16*)((u8*)var_a2 + var_v0);
            block_104:
                var_v1 = *var_v0_3;
            block_105:
                var_a2_2 = (u8*)var_a2 + var_v1;
            block_106:
                var_s0 = func_800A88A0(var_s0, arg1, var_a2_2, 1);
                break;
            case 3:
            case 4:
            case 5:
            case 6:
            {
                u8* pad_base = (u8*)g_pad_ctx + (g_menu_char_slot * 0x250);
                u8 flag = *(pad_base + item_sub + 0x609);
                if (flag != 0xFF)
                {
                    if (flag & 0x80)
                    {
                        u8* item_ptr = pad_base + ((u32)(flag & 0x7F) << 6) + 0x740;
                        u8 cat = *(item_ptr + 0x24);
                        u8 entry = *(item_ptr + 0x25);
                        var_a2 = (u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x44);
                        var_v1 = *(u16*)(var_a2 + (u32)cat * 0x1C + (u32)entry * 2);
                        goto block_105;
                    }
                    else
                    {
                        u32 slot654 = *(u32*)(pad_base + 0x654);
                        var_a2 = (u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x1C);
                        var_v0_3 = (u16*)(var_a2 + ((slot654 >> 0xA) & 0x3F) * 0x30 + (u32)(flag & 0x7F) * 2);
                        goto block_104;
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
                    s32 s2_off = (item_sub - 7) << 6;
                    s32 char_base = g_menu_char_slot * 0x250;
                    u8* slot_base = (u8*)g_pad_ctx + s2_off + char_base;
                    if (*(slot_base + 0x640) != 0)
                    {
                        if (func_8014DE1C((s32)((u8*)g_pad_ctx + char_base + 0x5F0 + ((s32)item_sub << 6) - 0x170)) != 0)
                        {
                            u8* state30 = (u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x30);
                            u8* state8 = (u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x8);
                            u16 idx656 = *(u16*)(slot_base + 0x656) & 0x3F;
                            u8* name = state30 + *(u16*)(state30 + idx656 * 2);
                            u8* surname = state8 + *(u16*)((u8*)state8 + 0xB4);
                            dst = sp60;
                            src = name;
                            while ((ch = *src) != 0)
                            {
                                *dst++ = ch;
                                src++;
                                if ((u32)(ch - 0x19U) < 7U)
                                {
                                    *dst++ = *src++;
                                }
                            }
                            src = surname;
                            while ((ch = *src) != 0)
                            {
                                *dst++ = ch;
                                src++;
                                if ((u32)(ch - 0x19U) < 7U)
                                {
                                    *dst++ = *src++;
                                }
                            }
                            *dst = 0;
                        }
                        else
                        {
                            sp60[0] = 0;
                        }
                        {
                            u32 unk654 = *(u32*)(slot_base + 0x654);
                            u32 kind = (unk654 >> 8) & 3;
                            u32 idx = (unk654 >> 9) & 0x7E;
                            u8* state68 = (u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x68);
                            u32 str_off = (kind != 0) ? (kind != 1 ? 0x2E : 0x16) : 0;
                            u8* str2 = state68 + *(u16*)(state68 + idx + str_off);
                            dst = sp20;
                            src = sp60;
                            while ((ch = *src) != 0)
                            {
                                *dst++ = ch;
                                src++;
                                if ((u32)(ch - 0x19U) < 7U)
                                {
                                    *dst++ = *src++;
                                }
                            }
                            src = str2;
                            while ((ch = *src) != 0)
                            {
                                *dst++ = ch;
                                src++;
                                if ((u32)(ch - 0x19U) < 7U)
                                {
                                    *dst++ = *src++;
                                }
                            }
                            *dst = 0;
                        }
                        var_a2_2 = sp20;
                        goto block_106;
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
                    var_a2 = (u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x60);
                    goto block_103;
                }
                break;
            }
            case 14:
                var_t0 = *(s32*)((u8*)g_menu_state_ptr + 0x48);
                var_v0_2 = ((u8*)g_pad_ctx)[(g_menu_char_slot * 0x250) + 0x609];
                goto block_101;
            case 15:
                var_a2_2 = (u8*)g_menu_item_ptr;
                if (var_a2_2 != NULL)
                {
                    goto block_106;
                }
                break;
            case 16:
            case 17:
            case 18:
                if (g_menu_item_ptr != 0)
                {
                    var_v0 = (s32) * ((u8*)g_menu_item_ptr + item_sub + 0x10) * 2;
                    var_a2 = (u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x58);
                    goto block_103;
                }
                break;
            case 19:
            case 20:
            case 21:
                if (g_menu_item_ptr != 0)
                {
                    var_v0 = (s32) * ((u8*)g_menu_item_ptr + item_sub + 0x15) * 2;
                    var_a2 = (u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x60);
                    goto block_103;
                }
                break;
            case 22:
                if (g_menu_item_ptr != 0)
                {
                    u8 cat = *((u8*)D_80169408 + 0x24);
                    u8 entry = *((u8*)D_80169408 + 0x25);
                    var_a2 = (u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x40);
                    var_v1 = *(u16*)(var_a2 + (u32)cat * 0x1C + (u32)entry * 2);
                    goto block_105;
                }
                break;
            case 23:
                if (g_menu_item_ptr != 0)
                {
                    var_v0_2 = *((u8*)D_80169408 + 0x24);
                    var_a2 = (u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x24);
                    goto block_102;
                }
                break;
            case 24:
                if (g_menu_item_ptr != 0)
                {
                    var_v0_2 = *((u8*)D_80169408 + 0x25);
                    var_a2 = (u8*)g_menu_state_ptr + *(s32*)((u8*)g_menu_state_ptr + 0x34);
                    goto block_102;
                }
                break;
            case 25:
                var_t0 = *(s32*)((u8*)g_menu_state_ptr + 0x78);
                var_v0_2 = ((u8*)g_pad_ctx)[(g_menu_char_slot * 0x250) + 0x633];
                goto block_101;
            }
        }
    }

    if (D_801690B0.unk3 != 0xFF)
    {
        ot_head = (MenuPrimHead*)func_80149D90(var_s0, arg1, D_801690B0.unk0, D_801690B0.unk1);
        ot_head->_u._s.unk3 = 1;
        ot_head->unk4 = 0xE1000005;
        ot_head->_u.unk0 = (s32)((ot_head->_u.unk0 & 0xFF000000) | (*arg1 & 0xFFFFFF));
        var_s0 = (u8*)ot_head + 8;
        *arg1 = (*arg1 & 0xFF000000) | ((s32)ot_head & 0xFFFFFF);
    }

    return var_s0;
}

s32 func_80149948(s32, s32, s32*);

/**
 * @brief Render all root menu nodes, then lerp g_menu_content_height toward g_menu_scroll_pos.
 * @param arg0 Current primitive buffer pointer.
 * @param arg1 Pointer to the ordering-table entry used by node-rendering helpers.
 * @return Updated primitive buffer pointer after rendering all active root nodes.
 * @note Resets g_menu_nav_count to 0, then iterates all MENU_NODE_COUNT nodes.
 *       Calls func_80149948 for each node with parent_idx == MENU_NONE and flags bit 0 set.
 *       After the loop, g_menu_content_height is lerped toward g_menu_scroll_pos using
 *       g_menu_redraw_state as the step count; snaps immediately when the count reaches
 *       zero or the values are already equal.
 * @see decomp.me (100%) https://decomp.me/scratch/AIXmd
 */
s32 func_80149828(s32 arg0, s32* arg1)
{
    s32 i;
    s32 temp_v0;

    g_menu_nav_count = 0;

    for (i = 0; i < MENU_NODE_COUNT; i++)
    {
        if ((g_menu_nodes[i].u2.s.parent_idx == MENU_NONE) && (g_menu_nodes[i].u2.s.flags & 1))
        {
            arg0 = func_80149948(i, arg0, arg1);
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

void* func_80149BB4(void*, s32*, s32, s32, s32, s32, s32, s32, s32);

/**
 * @brief Render one menu node's panel and update its animated Y position; recurse into children.
 * @param arg0 Node index within g_menu_nodes.
 * @param arg1 Current primitive buffer pointer.
 * @param arg2 Pointer to the ordering-table entry.
 * @return Updated primitive buffer pointer after rendering this node and all expanded children.
 * @note Appends arg0 to the g_menu_nav_first list and increments g_menu_nav_count.
 *       Calls func_80149BB4 to render the node's window panel, passing position, animation
 *       counter state, and whether this node is the active scene.
 *       Decrements the 2-bit animation counter in u2.unk2 bits [3:2] when nonzero.
 *       Lerps the node's nav cursor Y toward its layout Y using node->state as step count;
 *       snaps immediately when state is zero.
 *       If the node's expanded flag (u2.s.flags bit 1) is set, recursively renders
 *       child0, child1, child2, child3 in order, stopping at the first MENU_NONE child.
 * @see decomp.me (90.48%) https://decomp.me/scratch/TNThR
 */
s32 func_80149948(s32 arg0, s32 arg1, s32* arg2)
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
    node = &g_menu_nodes[arg0];
    g_menu_nav_count += 1;
    buf = func_80149BB4(arg1, arg2, node->unk4, ((node->idx_nav.nav_x_packed >> 8) & (new_var5 = 0x7F)) - (-1),
                        ((new_var6 = node->idx_nav.nav_x_packed >> 15) | (node->u8_u.s.nav_y_hi << 1)) - g_menu_content_height, 1,
                        ((node->u2.unk2 >> 2) & 3) != 0, g_menu_scene_type == arg0, (node->u2.unk2 >> 6) & new_var3);

    {
        u16 unk2 = (&node->u2)->unk2;
        u32 anim_cnt = (unk2 >> 2) & 3;
        if (anim_cnt != 0)
        {
            new_var6 = anim_cnt - 1;
            new_var2 = new_var6 & 3;
            new_var5 = new_var2 << 2;
            new_var2 = (unk2 & 0xFFF3) | new_var5;
            node->u2.unk2 = new_var2;
        }
    }

    if (node->state == 0)
    {
        node->idx_nav.nav_x_packed = (new_var2 = node->idx_nav.nav_x_packed & 0x7FFF) | (node->u8_u.nav_y_packed & 0x8000);
        node->u8_u.nav_y_packed = (node->u8_u.nav_y_packed & 0xFF00) | node->uA.s.layout_y_hi;
    }
    else
    {
        u16 nav_x_packed = node->idx_nav.nav_x_packed;
        u16 nav_y_packed = node->u8_u.nav_y_packed;
        s32 current_y = (nav_x_packed >> 15) | ((nav_y_packed & 0xFF) << 1);
        s32 target_y = (nav_y_packed >> 15) | (node->uA.s.layout_y_hi << 1);
        s32 delta_y;
        if (((u16)current_y) == target_y)
        {
            node->state = 0;
        }
        else
        {
            s32 step = (target_y - ((u16)current_y)) / ((s32)node->state);
            u32 new_y = (current_y + step) & 0xFFFF;
            new_var3 = 15;
            new_var3 = (new_y & 1) << new_var3;
            node->state -= 1;
            node->idx_nav.nav_x_packed = (nav_x_packed & 0x7FFF) | new_var3;
            new_var = node->u8_u.nav_y_packed & 0xFF00;
            new_var7 = node;
            node->u8_u.nav_y_packed = (new_y >> 1) & 0xFF;
            new_var7->u8_u.nav_y_packed = new_var | node->u8_u.nav_y_packed;
        }
    }

    {
        MenuNode* node2 = &g_menu_nodes[arg0];
        node->state += 0;
        if ((node2->u2.unk2 >> 1) & 1)
        {
            s32 j;
            new_var4 = node2;
            for (j = 0; j < 4; j++)
            {
                u8 child = *((&new_var4->uA.s.child0) + j);
                if (child == 0xFF)
                {
                    break;
                }
                buf = func_80149948(child & 0xFFu, buf, arg2);
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

extern NodeSpriteInfo D_8014FBF4[];
extern u8 D_8014FDB8[];

inline int inline_fn(int arg0, int arg1)
{
    return arg0 | arg1;
}

/**
 * @brief Emit one or two SPRT primitives for a single menu node's panel and OT-link them.
 * @param arg0 Current primitive buffer pointer; the function writes SPRTs starting here.
 * @param arg1 Pointer to the ordering-table entry; updated after each emitted primitive.
 * @param arg2 Content-ID index used to look up UV, size, and CLUT data in D_8014FBF4/D_8014FDB8.
 * @param arg3 Node column position (used as base X for both sprites).
 * @param arg4 Node row position (used as base Y for both sprites).
 * @param arg5 When non-zero, a second (highlight/shadow) SPRT is also emitted after the first.
 * @param arg6 Animation-in-progress flag; adjusts the pixel offset applied to both sprite positions.
 * @param arg7 Non-zero when this node is the active scene; selects opaque (0x64) vs.
 *             semi-transparent (0x66) code and blue vs. black tint for the second SPRT.
 * @param arg8 TODO: type-bits field from u2.unk2 bits [7:6]; not read inside this function.
 * @return Pointer to the next free byte in the primitive buffer after the last emitted SPRT.
 * @note First SPRT: neutral tint (0x808080), code 0x64, position (arg3-arg5+arg6, arg4-arg5+arg6).
 *       Second SPRT (when arg5 != 0): blue/black tint, position offset by (arg5-arg6)*2 from base;
 *       code 0x64 when arg7 != 0 (active scene), 0x66 otherwise (semi-transparent).
 *       CLUT is computed from D_8014FDB8[arg2]: high nibble = VRAM Y delta from row 0x1F2,
 *       low nibble = VRAM X/16.
 * @see decomp.me (100%) https://decomp.me/scratch/IXG0l
 */
void* func_80149BB4(void* arg0, s32* arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, s32 arg7, s32 arg8)
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
    p1[0xC] = D_8014FBF4[arg2].u_coord;
    p1[0xD] = D_8014FBF4[arg2].v_coord;
    *((s16*)(p1 + 0x10)) = (s16)D_8014FBF4[arg2].w;
    *((s16*)(p1 + 0x12)) = (s16)D_8014FBF4[arg2].h;
    *((s16*)(p1 + 0xE)) = (s16)inline_fn(((D_8014FDB8[arg2] >> 4) + 0x1F2) << 6, D_8014FDB8[arg2] & 0xF);
    *((s32*)p1) = ((*((s32*)p1)) & 0xFF000000) | ((*arg1) & 0xFFFFFF);
    *arg1 = inline_fn((*arg1) & 0xFF000000, ((s32)p1) & 0xFFFFFF);
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
        p1[0xC] = D_8014FBF4[arg2].u_coord;
        p1[0xD] = D_8014FBF4[arg2].v_coord;
        *((s16*)(p1 + 0x10)) = (s16)D_8014FBF4[arg2].w;
        *((s16*)(p1 + 0x12)) = (s16)D_8014FBF4[arg2].h;
        *((s16*)(p1 + 0xE)) = (s16)inline_fn(((D_8014FDB8[arg2] >> 4) + 0x1F2) << 6, D_8014FDB8[arg2] & 0xF);
        *((s32*)p1) = ((*((s32*)p1)) & 0xFF000000) | ((*arg1) & 0xFFFFFF);
        *arg1 = inline_fn((*arg1) & 0xFF000000, ((s32)p1) & 0xFFFFFF);
        p1 += 0x14;
    }
    return p1;
}

/**
 * @brief Emit a single 16x16 gray SPRT primitive and OT-link it.
 * @param arg0 Current primitive buffer pointer; must have at least 0x14 bytes of space.
 * @param arg1 Pointer to the ordering-table entry; updated to prepend this SPRT.
 * @param arg2 X screen position of the sprite.
 * @param arg3 Y screen position of the sprite.
 * @return Pointer to the next free byte after the emitted 0x14-byte SPRT.
 * @note Emits a SPRT with medium-gray tint (0x505050), code 0x64, 16x16 size,
 *       U=0x80, V=0x00, CLUT=0x7C86.
 * @see decomp.me (100%) https://decomp.me/scratch/16UQc
 */
void* func_80149D90(void* arg0, s32* arg1, s16 arg2, s16 arg3)
{
    u8* p = (u8*)arg0;

    *(u32*)(p + 0x4) = 0x505050;
    p[3] = 4;
    *(u32*)(p + 0x10) = 0x100010;
    p[7] = 0x64;
    *(s16*)(p + 0xC) = 0x80;
    *(s16*)(p + 0x8) = arg2;
    *(s16*)(p + 0xA) = arg3;
    *(s16*)(p + 0xE) = 0x7C86;
    *(s32*)p = (*(s32*)p & (s32)0xFF000000) | (*arg1 & 0xFFFFFF);
    *arg1 = (*arg1 & (s32)0xFF000000) | ((s32)p & 0xFFFFFF);
    return p + 0x14;
}

typedef struct
{
    s16 x;
    s16 y;
} Vec2s;

/**
 * @brief State block for a scrollable circular linked-list widget.
 * @note Fields 0x14/0x16/0x18 (target_x, target_y, lerp_steps) are written
 *       by func_8014A044 to drive smooth-scroll interpolation.
 */
typedef struct
{
    u32 pad;         /* 0x00 */
    u16 sel_idx;     /* 0x04 - currently selected item index */
    u16 item_count;  /* 0x06 - total items; lower 9 bits active (& 0x1FF) */
    u16 base_x;      /* 0x08 - widget screen base x */
    u16 base_y;      /* 0x0A - widget screen base y */
    s16 unk_0C;      /* 0x0C - TODO: used in func_8014A044 x-lerp target calc */
    s16 viewport_h;  /* 0x0E - visible list height; (viewport_h - 16) >> 4 = fast-scroll step */
    u16 scroll_x;    /* 0x10 - current applied x scroll offset */
    u16 scroll_y;    /* 0x12 - current applied y scroll offset */
    s16 target_x;    /* 0x14 - x scroll lerp target (set by func_8014A044) */
    s16 target_y;    /* 0x16 - y scroll lerp target (set by func_8014A044) */
    u8  lerp_steps;  /* 0x18 - remaining lerp steps; always reset to 4 */
} ScrollListState;

void func_8014A044(ScrollListState*, u32*);

/**
 * @brief Process shoulder/D-pad scroll input for a list widget and draw its animated cursor.
 * @param prim_buf    Primitive buffer write cursor; forwarded to func_8014A10C.
 * @param ot          Ordering-table pointer; forwarded to func_8014A10C.
 * @param state       Scroll-list state block.
 * @param entries     Packed circular linked-list entry array (D_80168C70).
 *                    Each u32: bits [13:0] = item y position, [22:14] = prev index (9 bits),
 *                    [31:23] = next index (9 bits).
 * @param view_origin Viewport anchor in list-local coordinates.
 * @param active      Non-zero to process input this frame; zero draws cursor only.
 * @note R1 (PADR1) injects PADLdown for fast-scroll down; L1 (PADL1) injects PADLup for fast-scroll up.
 *       MENU_PAD_CONFIRM_CANCEL advances the linked-list index; PADLleft is remapped to PAD_BTN_CIRCLE.
 *       Writes g_menu_default_view_pos with the selected item's screen coordinates.
 * @see decomp.me (96.52%) https://decomp.me/scratch/tfyt3
 */
void scroll_list_draw(s32 prim_buf, s32* ot, ScrollListState* state, u32* entries, Vec2s* view_origin, int active)
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
                func_8014A044(state, entries);
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
            func_8014F210(MENU_SE_NAVIGATE, MENU_SE_VOLUME);
        }
        if (g_pad_input & PADLleft)
        {
            g_pad_input |= PAD_BTN_CIRCLE;
        }
    }
    func_8014A10C(prim_buf, ot, (4 - view_origin->x) - state->scroll_x, ((entries[state->sel_idx] & 0x3FFF) - view_origin->y) - state->scroll_y, active);
    g_menu_default_view_pos.x = (state->base_x + ((4 - (view_origin->x & 0xFFFFFFFF)) - state->scroll_x)) + 8;
    g_menu_default_view_pos.y = (state->base_y + (((entries[state->sel_idx] & 0x3FFF) - view_origin->y) - state->scroll_y)) + 8;
}