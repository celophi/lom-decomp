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
    u8 unk0;
    u8 unk1;
    u8 pad2;
    u8 unk3;
    union
    {
        s32 unk4;
        struct
        {
            u16 _unk4lo;
            u16 unk6;
        } _s;
    } _u;
    u16 unk8;
    u16 unkA;
    u16 unkC;
    u16 unkE;
    u16 unk10;
    u16 unk12;
    u16 unk14;
    u16 unk16;
    u8 unk18;
    u8 pad19;
    u8 pad1A;
    u8 pad1B;
    s32* (*unk1C)();
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
        u16 unk6;
        struct
        {
            u8 self_idx; /**< This node's own index in g_menu_nodes (used as content-table key). */
            u8 nav_x;    /**< Bits 0-6: nav cursor X = (nav_x & 0x7F) + 8. Bit 7: bit 0 of nav cursor Y. */
        } s;
    } u6;
    union
    {
        u16 unk8;
        struct
        {
            u8 nav_y_hi;     /**< Bits 1-8 of the 9-bit nav cursor Y: reconstruct as (nav_y_hi<<1)|(nav_x>>7). */
            u8 layout_y_lsb; /**< Bit 7 = bit 0 of layout Y position; bits 0-6 always 0 after layout. */
        } s;
    } u8_u;
    union
    {
        u16 unkA;
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
extern s8 D_801690F9;
extern s8 D_80169324;

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
/** @brief Per-node table of MenuContentItem arrays, indexed by node.u6.s.self_idx; NULL = no cursor data. */
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
    entry->unk1C = 0;
    entry->index = (u8)var_a2;
    entry->unk20 = 0;
    entry->unk2 = 0;
    mask = 0x1FFFFFF;
    temp = temp & mask;
    temp = temp | (((u32)arg0) << 25);
    entry->flags = temp;
    entry->x = src[0];
    entry->y = src[1];
    entry->w = src[2];
    entry->h = src[3];
    entry->unk10 = 0;
    entry->unk12 = 0;
    entry->unk14 = 0;
    entry->unk16 = 0;
    entry->unk18 = 0;
    entry->unk3 = 0;
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
            temp_a0 = var_s0->unk2;
            temp_v0 = temp_a0 + 1;
            var_s0->unk2 = temp_v0;
            if ((temp_v0 & 0xff) == 6)
            {
                var_s0->unk2 = temp_a0;
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
            temp_v0_2 = (void (*)(MenuSlot*))var_s0->unk20;
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
        temp_v0 = var_s0->unk2 - 1;
        var_s0->unk2 = temp_v0;
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
        gpu_work->prim_cursor =
            func_800A88A0(gpu_work->prim_cursor, &gpu_work->ot_base, g_menu_pending_overlay, 1, 0xA0, 0xCA, 2);
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
    temp_s2 = (s32*)gpu_work + (((u32)slot->_u.unk4 >> 0x19));
    if (slot->unk18 != 0)
    {
        temp_a2 = slot->unk10;
        temp_v1 = (s32)(slot->unk14 - temp_a2) / (s32)slot->unk18;
        temp_a1 = slot->unk12;
        temp_a1 = temp_a1 + ((s32)(slot->unk16 - temp_a1) / (s32) * (volatile u8*)&slot->unk18);
        slot->unk18 = (u8)(*(volatile u8*)&slot->unk18 - 1);
        slot->unk10 = (u16)(temp_a2 + temp_v1);
        slot->unk12 = (u16)temp_a1;
    }
    else
    {
        slot->unk10 = (u16)slot->unk14;
        slot->unk12 = (u16)slot->unk16;
    }
    if (slot->unk1C != NULL)
    {
        if ((temp_s3->w - 0x20) > 0)
        {
            if ((temp_s3->h - 0x10) > 0)
            {
                SetDrawEnv((DR_ENV*)var_s1,
                           (DRAWENV*)(g_draw_buf_base + ((gpu_work->draw_buf_idx ^ 1) * 0x40C0) + 0x4064));
                var_a3 = 0;
                *var_s1 = (*var_s1 & 0xFF000000) | (*temp_s2 & 0xFFFFFF);
                g_menu_draw_early_out = 0;
                *temp_s2 = (*temp_s2 & 0xFF000000) | ((s32)var_s1 & 0xFFFFFF);
                var_s1 += 0x10;
                if ((slot->unk1 == g_active_slot) && (cursor_enable != 0))
                {
                    if (g_menu_suppress_cursor == 0)
                    {
                        var_a3 = slot->unk0 == 2;
                    }
                }
                temp_s1 = slot->unk1C(temp_s2, slot, var_s1, arg3, var_a3);
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
                if (slot->unk3 != 0)
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
                    if (slot->_u.unk4 & 0x01FF0000)
                    {
                        var_s1 = (s32*)func_800AD208(temp_s2, var_s1, (u16)slot->_u.unk4 + 1, 3, sp80, 0);
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
        ((MenuPrimHead*)var_s1)->unk4 = (s32)(((t_unk2 >> 3) << 0xF) | (((t_unk0 >> 3) << 0xA) | 0xE2000000) |
                                              ((-t_unk6 << 2) & 0x3E0) | ((s32)(-t_unk4 & 0xFF) >> 3));
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
    temp_v0_2 = menu_emit_corner(
        menu_emit_corner(
            menu_emit_corner(menu_emit_corner(menu_fill_window_interior(prim_cur, temp_s2, &sp18, MENU_TW_FILL),
                                              temp_s2, temp_s3->x, temp_s3->y, MENU_TW_CORNER_TL),
                             temp_s2, temp_s3->x + temp_s3->w - 8, temp_s3->y, MENU_TW_CORNER_TR),
            temp_s2, temp_s3->x, temp_s3->y + temp_s3->h - 8, MENU_TW_CORNER_BL),
        temp_s2, temp_s3->x + temp_s3->w - 8, temp_s3->y + temp_s3->h - 8, MENU_TW_CORNER_BR);
    ((MenuPrimHead*)temp_v0_2)->_u._s.unk3 = 1;
    ((MenuPrimHead*)temp_v0_2)->unk4 = 0xE1000005;
    ((MenuPrimHead*)temp_v0_2)->_u.unk0 =
        (s32)((((MenuPrimHead*)temp_v0_2)->_u.unk0 & 0xFF000000) | (*temp_s2 & 0xFFFFFF));
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
        u16 unk8;
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
        g_menu_nodes[var_t0].state = 0;
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
    g_menu_nodes[0].u6.s.self_idx = 0;
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
    g_menu_nodes[1].u6.s.self_idx = 1;
    g_menu_nodes[1].unk4 = 5;
    g_menu_nodes[0].child1 = 2;
    g_menu_nodes[2].unk0 = 2;
    g_menu_nodes[2].u6.s.self_idx = 2;
    g_menu_nodes[2].unk4 = 4;
    g_menu_nodes[3].unk0 = 4;
    g_menu_nodes[1].unk0 = 3;
    g_menu_nodes[3].u6.s.self_idx = 3;
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
    g_menu_nodes[4].u6.s.self_idx = 4;
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
    g_menu_nodes[5].u6.s.self_idx = 5;
    g_menu_nodes[5].unk4 = 4;
    g_menu_nodes[6].unk0 = 7;
    g_menu_nodes[6].u6.s.self_idx = 6;
    g_menu_nodes[6].unk4 = 3;
    g_menu_nodes[6].uA.s.child0 = 7;
    g_menu_nodes[6].child1 = 8;
    g_menu_nodes[7].unk0 = 9;
    g_menu_nodes[7].u6.s.self_idx = 7;
    new_var7 = 0xFF3E;
    g_menu_nodes[7].unk4 = 5;
    g_menu_nodes[8].unk0 = 8;
    g_menu_nodes[4].u2.s.parent_idx = 3;
    g_menu_nodes[5].u2.s.parent_idx = 3;
    g_menu_nodes[6].u2.s.parent_idx = MENU_NONE;
    g_menu_nodes[7].u2.unk2 = (u16)((g_menu_nodes[7].u2.unk2 & 0xFF5F) | 0x50);
    g_menu_nodes[7].u2.s.parent_idx = 6;
    g_menu_nodes[8].u2.unk2 = (u16)((g_menu_nodes[8].u2.unk2 & 0xFF5F) | 0x50);
    g_menu_nodes[8].u6.s.self_idx = 8;
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
    g_menu_nodes[9].u6.s.self_idx = 9;
    g_menu_nodes[9].unk4 = 6;
    g_menu_nodes[9].uA.s.child0 = 0xA;
    g_menu_nodes[0xA].unk0 = 0xB;
    g_menu_nodes[0x12].unk4 = 0xA;
    g_menu_nodes[0xA].u6.s.self_idx = 0xA;
    g_menu_nodes[0xA].unk4 = 7;
    g_menu_nodes[0xC].unk0 = 0xA;
    g_menu_nodes[0xC].u6.s.self_idx = 0xC;
    g_menu_nodes[0xC].unk4 = 6;
    g_menu_nodes[0xC].uA.s.child0 = 0xD;
    g_menu_nodes[0xD].unk0 = 0xB;
    g_menu_nodes[0xD].u6.s.self_idx = 0xD;
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
    g_menu_nodes[0xF].u6.s.self_idx = 0xF;
    g_menu_nodes[0xF].uA.s.child0 = 0x10;
    g_menu_nodes[0xF].child1 = 0x11;
    g_menu_nodes[0x10].unk0 = 0xC;
    g_menu_nodes[0x10].u6.s.self_idx = 0x10;
    g_menu_nodes[0x11].unk0 = 0xE;
    g_menu_nodes[0x11].u6.s.self_idx = 0x11;
    g_menu_nodes[0x11].unk4 = 9;
    g_menu_nodes[0x12].unk0 = 0x10;
    g_menu_nodes[0x12].u6.s.self_idx = 0x12;
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
    g_menu_nodes[0x13].u6.s.self_idx = 0x13;
    g_menu_nodes[0x13].content_id = 0;
    g_menu_nodes[0x13].uA.s.child0 = 0x14;
    g_menu_nodes[0x13].child1 = 0x15;
    g_menu_nodes[0x14].unk0 = 0x12;
    g_menu_nodes[0x14].u6.s.self_idx = 0x14;
    g_menu_nodes[0x15].unk0 = 0x13;
    g_menu_nodes[0x15].u6.s.self_idx = 0x15;
    g_menu_nodes[0x15].unk4 = 0x12;
    g_menu_nodes[0x16].unk0 = 0x14;
    g_menu_nodes[0x16].u6.s.self_idx = 0x16;
    g_menu_nodes[0x16].unk4 = 0xC;
    g_menu_nodes[0x16].uA.s.child0 = 0x17;
    g_menu_nodes[0x16].child1 = 0x18;
    g_menu_nodes[0x17].unk0 = 0x15;
    g_menu_nodes[0x17].u6.s.self_idx = 0x17;
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
    g_menu_nodes[0x18].u6.s.self_idx = 0x18;
    g_menu_nodes[0x18].unk4 = 0x12;
    g_menu_nodes[0x19].unk0 = 0x16;
    g_menu_nodes[0x19].u6.s.self_idx = 0x19;
    g_menu_nodes[0x19].unk4 = 0xD;
    g_menu_nodes[0x19].uA.s.child0 = 0x1A;
    g_menu_nodes[0x19].child1 = 0x1B;
    g_menu_nodes[0x1A].unk0 = 0x17;
    g_menu_nodes[0x1A].u6.s.self_idx = 0x1A;
    g_menu_nodes[0x1A].unk4 = 0x11;
    g_menu_nodes[0x1B].unk0 = 0x13;
    g_menu_nodes[0x1B].u6.s.self_idx = 0x1B;
    g_menu_nodes[0x1B].unk4 = 0x12;
    g_menu_nodes[0x1C].unk0 = 0x18;
    g_menu_nodes[0x1C].u6.s.self_idx = 0x1C;
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
    var_a2->uA.unkA += 0;
    g_menu_nodes[0x1C].unk4 = 0xE;
    g_menu_nodes[0x1D].u6.s.self_idx = 0x1D;
    g_menu_nodes[0x1E].uA.s.child0 = 0x1F;
    g_menu_nodes[0x1F].u6.s.self_idx = 0x1F;
    g_menu_nodes[0x1C].content_id = 5;
    g_menu_nodes[0x1D].unk0 = 0x1C;
    g_menu_nodes[0x1D].content_id = 3;
    g_menu_nodes[0x1D].unk4 = 0x18;
    g_menu_nodes[0x1E].unk0 = 0x19;
    g_menu_nodes[0x1E].u6.s.self_idx = 0x1E;
    g_menu_nodes[0x1E].unk4 = 0x13;
    g_menu_nodes[0x1F].unk0 = 0x1A;
    g_menu_nodes[0x1F].unk4 = 0x14;
    g_menu_nodes[0x2B].unk0 = 0x1A;
    g_menu_nodes[0x2B].u6.s.self_idx = 0x2B;
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
    g_menu_nodes[0x20].u6.s.self_idx = 0x20;
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
        D_80169324 = 0x2B;
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
                var_a2->u6.unk6 = (u16)(var_a2->u6.unk6 & 0x80FF);
                var_a2->u8_u.unk8 = (u16)((*new_var).unk8 & 0x80FF);
                var_a2->uA.unkA = (u16)((var_a2->uA.unkA & 0xFF00) | ((temp_a0 >> 1) & 0xFF));
                var_a2->u8_u.unk8 = (u16)(((*new_var).unk8 & 0x7FFF) | (temp_a0 << 0xF));
                var_a2->u6.unk6 = (u16)((var_a2->u6.unk6 & 0x7FFF) | (((temp_a1 & 1) << 9) << 6));
                var_a1 = temp_a1 >> 1;
                var_a2->u8_u.unk8 = (u16)((new_var3->u8_u.unk8 & 0xFF00) | var_a1);
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
    MenuNode* new_var4;
    int has_children;
    u32 temp_a1;
    union
    {
        u16 unk8;
        struct
        {
            u8 nav_y_hi;
            u8 layout_y_lsb;
        } s;
    }* new_var2;
    MenuNode* new_var3;
    cur_pos = base_pos;
    has_children = (((u16)(&g_menu_nodes[node_idx])->u2.unk2) >> 1) & 1;
    temp_a1 = cur_pos & 0xFFFF;
    cur_pos += MENU_ROW_HEIGHT;
    new_var4 = &g_menu_nodes[node_idx];

    (*(&g_menu_nodes[node_idx])).state = 4;
    new_var2 = &new_var4->u8_u;
    new_var4->u8_u.unk8 = (*new_var2).s.nav_y_hi | ((temp_a1 & 1) << 0xF);
    (&g_menu_nodes[node_idx])->uA.unkA = ((&g_menu_nodes[node_idx])->uA.unkA & 0xFF00) | (0xFF & (temp_a1 >> 1));
    new_var4->uA.unkA = (new_var4->uA.unkA & 0xFF00) | ((temp_a1 >> 1) & 0xFF);
    child_iter = 0;
    if (has_children)
    {
        s32 child_idx;
        s32 child;
        for (child_idx = child_iter; (child_idx < 4) != 0;)
        {
            new_var3 = g_menu_nodes + node_idx;
            child_idx = (&(&(*new_var3).uA.s)->child0)[child_idx];
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
        new_var8 = &temp_a1->u6;
        if (g_menu_scene_type != g_menu_active_node)
        {
            if (g_menu_content_table[temp_a1->u6.s.self_idx] != NULL)
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
        g_content_cursor_x = (((temp_a1->u6.unk6 >> 4) >> 4) & 0x7F) + 8;
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
                temp_v1_2 = &g_menu_content_table[new_var11[g_menu_scene_type].u6.s.self_idx][g_menu_hit_item_idx];
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
        u8 self_idx = (nodes + g_menu_scene_type)->u6.s.self_idx;
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

extern s32 D_80169550;

/**
 * decomp.me (93.73%) https://decomp.me/scratch/UqSRu
 */
void menu_set_active_node(void)
{
    s32 var_a0;
    MenuNode* curr_node;
    MenuNode* temp_a0;
    u16 temp_v0;
    s32 temp_a0_4;
    s32 var_s3;
    s32 layout_pos;
    s32 var_s2;
    MenuNode* var_s1;
    s32 temp_s0;
    long child_i;
    u8 child_idx;
    MenuNode* child_node;
    u16 nav_x;
    u16 new_var;
    for (var_a0 = 0; var_a0 < 0x2C; var_a0++)
    {
        g_menu_nodes[var_a0].u2.unk2 &= 0xFFFD;
    }

    var_s3 = g_menu_active_node;
    layout_pos = g_menu_nodes[g_menu_active_node].u2.s.parent_idx;
    if (layout_pos != 0xFF)
    {
        do
        {

            var_s3 = g_menu_nodes[var_s3].u2.s.parent_idx;
            curr_node = &g_menu_nodes[var_s3];

            curr_node->u2.unk2 |= 2;
        } while (curr_node->u2.s.parent_idx != 0xFF);
    }
    temp_a0 = &g_menu_nodes[g_menu_active_node];
    temp_v0 = temp_a0->u2.unk2 | 2;
    temp_a0->u2.unk2 = temp_v0;
    if ((temp_v0 >> 1) & 1)
    {
        for (child_i = 0; child_i < 4; child_i++)
        {
            child_idx = (&temp_a0->uA.s.child0)[child_i];
            if (child_idx == (temp_v0 = 0xFF))
            {
                break;
            }
            child_node = &g_menu_nodes[child_idx];
            nav_x = temp_a0->u6.unk6 & 0x7F00;
            child_node->u6.unk6 = child_node->u6.unk6 & 0x80FF;
            child_node->u6.unk6 = child_node->u6.unk6 | nav_x;
            child_node->u8_u.unk8 = child_node->u8_u.unk8 & 0x80FF;
            child_node->u8_u.unk8 = child_node->u8_u.unk8 | nav_x;
            child_idx = (&temp_a0->uA.s.child0)[child_i];
            new_var = temp_a0->u6.unk6;
            ;
            (&g_menu_nodes[child_idx])->u8_u.unk8 =
                (((&g_menu_nodes[child_idx])->u8_u.unk8 & 0xFF00) & 0xFFFFFFFFFFFFFFFFu) |
                (&g_menu_nodes[g_menu_active_node])->u8_u.s.nav_y_hi;
            (&g_menu_nodes[child_idx])->u6.unk6 = ((&g_menu_nodes[child_idx])->u6.unk6 & 0x7FFF) | (new_var & 0x8000);
        }
    }
    temp_a0_4 = (((u16)g_menu_nodes[g_menu_active_node].u2.unk2) >> 4) & 3;
    if (temp_a0_4 != 3)
    {
        D_80169550 = temp_a0_4;
    }
    var_s3 = 0;
    layout_pos = 0;
    var_s1 = g_menu_nodes;
    for (var_s2 = 0; var_s2 < 0x2C; var_s2++, var_s1++)
    {
        temp_v0 = 0xAB;
        if (var_s1->u2.s.parent_idx == 0xFF)
        {
            if (var_s1->u2.s.flags & 1)
            {
                temp_s0 = layout_pos;
                layout_pos = menu_layout_node(var_s2, layout_pos);
                if (temp_s0 != (layout_pos - 0x13))
                {
                    if (layout_pos > (0xAC - 1))
                    {
                        var_s3 = 1;
                        g_menu_scroll_pos = layout_pos - temp_v0;
                        g_menu_redraw_state = 8;
                    }
                }
            }
        }
    }

    g_menu_layout_end = layout_pos;
    if (var_s3 == 0)
    {
        g_menu_scroll_pos = 0;
        g_menu_redraw_state = 8;
    }
}

extern u8 D_801ED600[];
extern s32 D_801690FC;
extern s32 D_80169118;
extern s8 D_801226F0;
extern void *D_80168C70;
extern s8 D_801226B8;
extern s32 D_801229F4;
extern s32 D_8011F424;

extern void func_8014B7DC(void);
extern void func_8014CC08(void);
extern void func_8014C200(void);

typedef struct {
    u8 unk0;
    u8 unk1;
    u8 unk2;
    u8 unk3;
} D_801690B0_type;

extern D_801690B0_type D_801690B0;
extern u8 D_801686CC[];
extern u8 D_8014FE54[];
extern void *D_801690A8;
extern void *D_801690E0;
extern void *D_801227D4;

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

    if ((u32)(g_menu_scene_type - 0x14) < 2 || g_menu_scene_type == 0x17 || g_menu_scene_type == 0x18 ||
        g_menu_scene_type == 0x1A || g_menu_scene_type == 0x1B)
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
                            g_script_repeat_last = D_801690FC - 1;
                        }
                        else
                        {
                            g_script_repeat_last -= 1;
                        }
                    }
                    else if (g_pad_input & 2)
                    {
                        func_8014B69C(1);
                        if (g_script_repeat_last == (D_801690FC - 1))
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
                    func_80144FB8();
                }
                menu_set_active_node(g_menu_scene_type);
                g_menu_hit_item_idx = func_8014847C((s32*)0);
                if (g_menu_hit_item_idx != (-1))
                {
                    MenuContentItem* temp_s5 = g_menu_content_table[g_menu_nodes[g_menu_scene_type].u6.s.self_idx];
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
        self_idx = g_menu_nodes[g_menu_scene_type].u6.s.self_idx;
        if (self_idx - 0x14 < 0x16U)
        {
            switch (self_idx - 0x14)
            {
            case 0:
            case 3:
            case 6:
                g_menu_nodes[g_menu_scene_type].u6.s.self_idx += 1;
                goto incdec_common;

            case 1:
            case 4:
            case 7:
                g_menu_nodes[g_menu_scene_type].u6.s.self_idx -= 1;
                goto incdec_common;

            case 2:
            case 5:
            case 8:
            incdec_common:
                g_menu_hit_item_idx = func_8014847C((s32*)0);
                if (g_menu_hit_item_idx != (-1))
                {
                    MenuContentItem* temp_s5 = g_menu_content_table[g_menu_nodes[g_menu_scene_type].u6.s.self_idx];
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
                func_80144FB8();
            }
            g_menu_hit_item_idx = func_8014847C((s32*)0);
            if (g_menu_hit_item_idx != (-1))
            {
                MenuContentItem* temp_s5 = g_menu_content_table[g_menu_nodes[g_menu_scene_type].u6.s.self_idx];
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
        s5 = g_menu_content_table[g_menu_nodes[g_menu_scene_type].u6.s.self_idx];

        if ((g_menu_scene_type == 0x1F || g_menu_scene_type == 0x2B) && g_menu_hit_item_idx >= 0x11 &&
            g_menu_hit_item_idx < 0x19)
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
                D_80169118 = content_type;
                switch (content_type)
                {
                case 1:
                case 2:
                    if (D_80169550 == 0)
                    {
                        u16 sp10 = 0x40;
                        u16 sp12 = 0x60;
                        u16 sp14 = 0xF0;
                        u16 sp16 = 0x60;
                        MenuSlot* var_a3 = (MenuSlot*)menu_slot_alloc(3, &sp10);
                        var_a3->unk1C = (s32 * (*)()) & func_8014B7DC;
                        var_a3->flags = (var_a3->flags & 0xFE00FFFF) | ((func_80145310() & 0x1FF) << 16);
                        var_a3->unk3 = 1;
                        func_8014F210(0x7D, 0x80);
                    }
                    break;

                case 3:
                case 4:
                case 5:
                case 6:
                    if (D_80169550 == 0)
                    {
                        flag = ((u8*)g_pad_ctx)[(D_80169550 * 0x250) + 0x609 + content_type];
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
                                var_a3->unk1C = (s32 * (*)()) & func_8014C200;
                                new_var7 =
                                    (void*)((u8*)g_pad_ctx + (D_80169550 * 0x250) + 0x5F0 + (content_type << 6) + 0x90);
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
                    if (D_80169550 == 0)
                    {
                        u16 sp10 = 0xB0;
                        u16 sp12 = 0x30;
                        u16 sp14 = 0x70;
                        u16 sp16 = 0x60;
                        MenuSlot* var_a3 = (MenuSlot*)menu_slot_alloc(3, &sp10);
                        var_a3->unk1C = (s32 * (*)()) & func_8014CC08;
                        var_a3->flags = (var_a3->flags & 0xFE00FFFF) | 0x50000;
                        func_80145278(5);
                        {
                            void* ptr =
                                (void*)((u8*)g_pad_ctx + (D_80169550 * 0x250) + 0x5F0 + (content_type << 6) - 0x170);
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
                                    func_800A8E28(
                                        &sp18, (s8*)(base + (((*((u16*)((char*)g_menu_item_ptr + 0x16))) & 0x3F) * 2 +
                                                             (*((u16*)(base + 0x48))))));
                                    item14 = *((u32*)((char*)g_menu_item_ptr + 0x14));
                                    shift = (item14 >> 8) & 3;
                                    if (shift == 0)
                                    {
                                        func_80148324(&D_801226B8, &sp18,
                                                      (s8*)(base + (*((s32*)(base + ((item14 >> 9) & 0x7E))))));
                                    }
                                    else if (shift == 1)
                                    {
                                        func_80148324(&D_801226B8, &sp18,
                                                      (s8*)(base + (*((s32*)(base + ((item14 >> 9) & 0x7E) + 0x20)))));
                                    }
                                    else
                                    {
                                        func_80148324(&D_801226B8, &sp18,
                                                      (s8*)(base + (*((s32*)(base + ((item14 >> 9) & 0x7E) + 0x40)))));
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
                table_idx = D_801686CC[g_menu_nodes[g_menu_scene_type].u6.s.self_idx];
                val = D_8014FE54[(table_idx * 8) + ((packed_x >> 9) & 7)];
                if (val != 0)
                {
                    func_8014F210(0x7E, 0x80);
                    switch (val)
                    {
                    case 1:
                        D_80169554 =
                            (void*)((u8*)g_pad_ctx + (D_80169550 * 0x250) + 0x5F0 + (content_type << 6) - 0x170);
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
                            D_80169324 = 0x2B;
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
                                D_80169324 = 0x2B;
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
                        D_80169324 = 0xFF;
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
            self_idx = g_menu_nodes[g_menu_scene_type].u6.s.self_idx;
            if (g_menu_scene_type >= 0x11 || self_idx < 0x14 || self_idx >= 0x1C)
            {
                func_8014519C();
            }
            else
            {
                if (self_idx - 0x14 < 5)
                {
                    g_menu_nodes[(D_80169550 * 3) + 1].u6.s.self_idx = 1;
                    g_menu_nodes[(D_80169550 * 3) + 1].unk0 = (D_80169550 * 3) + 3;
                }
                else if (self_idx - 0x1A < 2)
                {
                    g_menu_nodes[(D_80169550 * 3) + 2].u6.s.self_idx = 2;
                    g_menu_nodes[(D_80169550 * 3) + 2].unk0 = (D_80169550 * 3) + 3;
                }
                g_menu_hit_item_idx = func_8014847C(&D_80169550);
                if (g_menu_hit_item_idx != (-1))
                {
                    MenuContentItem* temp_s5 = g_menu_content_table[g_menu_nodes[g_menu_scene_type].u6.s.self_idx];
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
                if (g_menu_scene_type >= 0x11 || g_menu_nodes[g_menu_scene_type].u6.s.self_idx < 0x14 ||
                    g_menu_nodes[g_menu_scene_type].u6.s.self_idx >= 0x1C)
                {
                    g_menu_cursor_enable = 2;
                    active_node = &g_menu_nodes[g_menu_active_node];
                    nav_y = (active_node->u8_u.s.nav_y_hi << 1) | ((active_node->u6.unk6 >> 15) & 1);
                    g_content_view_y = nav_y - (g_menu_content_height - 12);
                    if (g_content_view_y < 12)
                        g_content_view_y = 12;
                    if (g_content_view_y >= 0xA3)
                        g_content_view_y = 0xA3;
                    g_menu_suppress_cursor = 5;
                    g_content_view_x = ((active_node->u6.unk6 >> 8) & 0x7F) + 8;
                }
            }
            else if (s5[g_menu_hit_item_idx].pad[1 + dir_index] != 0)
            {
                if (g_menu_scene_type < 0x11)
                {
                    if ((s5[g_menu_hit_item_idx].packed_x & 0xF000) == 0x5000)
                    {
                        if (g_menu_nodes[g_menu_scene_type].u6.s.self_idx < 0x11)
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