#include "menu.h"

typedef struct MenuFrameCtx
{
    u8 pad0[0x34];
    u8 unk34;         /* Start of buffer at offset 0x34 */
    u8 pad35[0x400B]; /* Padding to 0x4040 */
    s32 unk4040;      /* Offset 0x4040 */
    u8 pad4044[8];    /* Padding to 0x404C */
    s32 unk404C;      /* Offset 0x404C */
} MenuFrameCtx;

typedef struct
{
    u8 _pad0[2]; // offsets 0x00-0x01
    u8 unk2;     // offset 0x02
    u8 _pad1[5]; // offsets 0x03-0x07
    u16 unk8;    // offset 0x08
    u16 unkA;    // offset 0x0A
    s16 unkC;    // offset 0x0C
    s16 unkE;    // offset 0x0E
} MenuSlotAnim;

typedef struct
{
    u16 x, y;
    s16 w, h;
} InputStruct;

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
    s32* unk4040;
    u8 pad4044[8];
    s32 unk404C;
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

/* K&R-style declaration: original call site in menu_tick passes no explicit
 * argument and relies on register a0 (the caller's first parameter) being
 * live. Keep the empty parameter list to preserve that codegen exactly. */
void menu_build_grid();

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
 * decomp.me (100%) https://decomp.me/scratch/AGd9K
 */
void menu_init_prim_rects(void)
{
    s32 thing;
    s32 s0 = 0;

    /* Force base address into s3 early */
    u8* base = g_prim_rect_buf;

    s32 s2 = 0x20;
    s32 s1 = 0;
    s16 params[4];

    while (s0 < 3)
    {
        /* First call */
        params[0] = 0x110;
        params[1] = s0 + 0x1D8;
        params[2] = 0x10;
        params[3] = 1;
        func_80019A34(params, (u8*)((u32)((s1 >> 2) << 2) + (u32)base));

        /* Second call */
        params[0] = (s0 == 2) ? 0x3E8 : 0x3F4;
        params[1] = (s0 == 0) ? 0x120 : 0x150;
        params[2] = 0xC;
        params[3] = 0x30;
        func_80019A34(params, (u8*)((u32)((s2 >> 2) << 2) + (u32)base));

        s2 += 0x4A0;
        s1 += 0x4A0;
        s0++;
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

    gpu_work->unk4040 = func_80142F10(gpu_work->unk4040, &gpu_work->unk34, gpu_work->unk404C, var_a3);

    if (g_menu_pending_overlay != 0)
    {
        gpu_work->unk4040 =
            func_800A88A0(gpu_work->unk4040, &gpu_work->unk34, g_menu_pending_overlay, 1, 0xA0, 0xCA, 2);
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
    temp_a3 = ((s32)((u16)slot->unkC << 0x10) >> 0x11) - ((s16)(slot->unkC / 12) * slot->unk2);
    sp[4] = (s16)temp_a3;

    /* Second computation */
    temp_a1 = ((s32)((u16)slot->unkE << 0x10) >> 0x11) - ((s16)(slot->unkE / 12) * slot->unk2);

    /* First branch logic block */

    sp[5] = (s16)temp_a1;

    /* Second branch logic block */
    if (temp_a3 > 0)
    {

        if (temp_a1 > 0)
        {

            clampC = slot->unkC - (temp_a3 * 2);
            if (clampC < 0x20)
            {
                clampC = 0x20;
            }

            clampE = slot->unkE - (temp_a1 * 2);
            if (clampE < 0x10)
            {
                clampE = 0x10;
            }

            sp[0] = slot->unk8 + temp_a3;
            sp[1] = slot->unkA + temp_a1;
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
    s32* var_a2_4;
    s32* var_s1;
    u16 temp_a1;
    u16 temp_a2;
    MenuRect* temp_s3;
    u16 var_v0;
    void* temp_v0_2;

    temp_s3 = rect;
    var_s1 = gpu_work->unk4040;
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
                SetDrawEnv((DR_ENV*)var_s1, (DRAWENV*)(g_draw_buf_base + ((gpu_work->unk404C ^ 1) * 0x40C0) + 0x4064));
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
                    gpu_work->unk4040 = temp_s1;
                    return;
                }
                temp_a0 = temp_s3->y;
                var_a2_2 = temp_a0 + 0x10;
                if (gpu_work->unk404C != 0)
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
    var_a2_4 = var_s1 + 3;
    if (temp_s3->h >= 0x10)
    {
        sp18.x = (u16)temp_s3->x + 8;
        sp18.y = (u16)temp_s3->y;
        sp18.w = (u16)temp_s3->w - 0x10;
        sp18.h = 8;
        var_a2_4 = menu_build_h_edge(var_a2_4, temp_s2, &sp18.x, 0x80D0);
        if (temp_s3->h >= 0x10)
        {
            sp18.x = (u16)temp_s3->x + 8;
            sp18.y = ((u16)temp_s3->y + (u16)temp_s3->h) - 8;
            sp18.w = (u16)temp_s3->w - 0x10;
            sp18.h = 8;
            var_a2_4 = menu_build_h_edge(var_a2_4, temp_s2, &sp18.x, 0x88D0);
        }
    }
    if (temp_s3->w >= 0x20)
    {
        sp18.x = (u16)temp_s3->x;
        sp18.y = (u16)temp_s3->y + 8;
        sp18.w = 8;
        sp18.h = (u16)temp_s3->h - 0x10;
        var_a2_4 = menu_build_v_edge(var_a2_4, temp_s2, &sp18.x, 0x90D0);
        if (temp_s3->w >= 0x20)
        {
            sp18.x = ((u16)temp_s3->x + (u16)temp_s3->w) - 8;
            sp18.y = (u16)temp_s3->y + 8;
            sp18.w = 8;
            sp18.h = (u16)temp_s3->h - 0x10;
            var_a2_4 = menu_build_v_edge(var_a2_4, temp_s2, &sp18.x, 0x90D8);
        }
    }
    sp18.x = (u16)temp_s3->x + 8;
    sp18.y = (u16)temp_s3->y + 8;
    sp18.w = (u16)temp_s3->w - 0x10;
    sp18.h = (u16)temp_s3->h - 0x10;
    temp_v0_2 = menu_emit_corner(
        menu_emit_corner(
            menu_emit_corner(menu_emit_corner(menu_fill_window_interior(var_a2_4, temp_s2, &sp18.x, 0xA0A0), temp_s2,
                                              temp_s3->x, temp_s3->y, 0x70D0),
                             temp_s2, temp_s3->x + temp_s3->w - 8, temp_s3->y, 0x70D8),
            temp_s2, temp_s3->x, temp_s3->y + temp_s3->h - 8, 0x78D0),
        temp_s2, temp_s3->x + temp_s3->w - 8, temp_s3->y + temp_s3->h - 8, 0x78D8);
    ((MenuPrimHead*)temp_v0_2)->_u._s.unk3 = 1;
    ((MenuPrimHead*)temp_v0_2)->unk4 = 0xE1000005;
    ((MenuPrimHead*)temp_v0_2)->_u.unk0 =
        (s32)((((MenuPrimHead*)temp_v0_2)->_u.unk0 & 0xFF000000) | (*temp_s2 & 0xFFFFFF));
    *temp_s2 = (*temp_s2 & 0xFF000000) | ((s32)temp_v0_2 & 0xFFFFFF);
    gpu_work->unk4040 = (s32*)((char*)temp_v0_2 + 8);
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
 * Walks the region described by @p rect (width at +4, height at +6, origin at
 * +0/+2) in 0x60-pixel steps on both axes, emitting one @c SPRT per tile
 * (code 0x64, tint 0x80008080, CLUT @c MENU_CLUT_GRID_ALT) clamped to the region edges,
 * and links each into the chain headed at @p ot.
 *
 * @param prim  Primitive write cursor (advanced past every emitted tile).
 * @param ot    Ordering-table head the tiles are linked into.
 * @param rect  Region descriptor: x (+0), y (+2), width (+4), height (+6).
 * @param uv    Packed texture origin written to each tile's @c u0 / @c v0.
 * @return Primitive write cursor just past the last tile emitted.
 * @see decomp.me (90.20%) https://decomp.me/scratch/R9mdk
 */
s32* menu_fill_window_interior(s32* prim, s32* ot, u8* rect, s16 uv)
{
    u16 new_var3;
    int new_var2;
    u8* ap = rect;
    short pad;
    u16 new_var;
    s32 y = 0;
    if ((*((s16*)(ap + 6))) > 0)
    {
        do
        {
            s32 x = 0;
            if ((*((s16*)(ap + 4))) > 0)
            {
                s32 y_plus_60 = y + 0x60;
                u8* wp;
                do
                {
                    *((u32*)((((u8*)prim) + 0x0E) - 0x0A)) = 0x80008080U;
                    (((u8*)prim) + 0x0E)[-0x0B] = 4;
                    (((u8*)prim) + 0x0E)[-7] = 0x64;
                    *((u16*)((((u8*)prim) + 0x0E) - 2)) = uv;
                    if ((*((s16*)(ap + 4))) < (x + 0x60))
                    {
                        *((u16*)((((u8*)prim) + 0x0E) + 2)) = (u16)((*((u16*)(ap + 4))) - ((u16)x));
                    }
                    else
                    {
                        *((u16*)((((u8*)prim) + 0x0E) + 2)) = 0x60;
                    }
                    if (((*((s16*)(ap + 6))) < y_plus_60) != 0)
                    {
                        *((u16*)((((u8*)prim) + 0x0E) + 4)) = (u16)((*((u16*)(ap + 6))) - ((u16)y));
                    }
                    else
                    {
                        *((u16*)((((u8*)prim) + 0x0E) + 4)) = 0x60;
                    }
                    new_var3 = (u16)((*((u16*)(ap + 0))) + ((u16)x));
                    *((u16*)((((u8*)prim) + 0x0E) - 6)) = new_var3;
                    new_var = *((u16*)(ap + 2));
                    x += 0x60;
                    new_var2 = 0x00FFFFFFU & ((u32)prim);
                    *((u16*)((((u8*)prim) + 0x0E) - 4)) = (u16)(new_var + ((u16)y));
                    wp += 0x14;
                    *((u16*)((((u8*)prim) + 0x0E) + 0)) = MENU_CLUT_GRID_ALT;
                    *((u32*)prim) = ((*((u32*)prim)) & 0xFF000000U) | ((*((u32*)ot)) & 0x00FFFFFFU);
                    *ot = ((*((u32*)ot)) & 0xFF000000U) | new_var2;
                    prim = (s32*)(((u8*)prim) + 0x14);
                } while (x < (*((s16*)(ap + 4))));
            }
            y += 0x60;
        } while (y < (*((s16*)(ap + 6))));
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
 * @param input   Edge rectangle: x, y (screen position), w, h (pixel size).
 * @param tw_uv   Packed texture-window origin: bits 7..0 = u (x), bits 15..8 = v (y).
 * @return Pointer to the byte immediately after the emitted DR_TWIN.
 * @see decomp.me (78.62%) https://decomp.me/scratch/u17Fi
 */
void* menu_build_h_edge(void* ot, u_long* ot_ptr, InputStruct* input, s32 tw_uv)
{
    u8* pkt;      /* maps to t1 */
    u_long* otp;  /* maps to t2 */
    s32 texParam; /* maps to a0 */
    SPRT* sprt;
    DR_TWIN* twin;
    RECT tw;

    pkt = (u8*)ot;
    otp = ot_ptr;

    if (input->w <= 0)
        return ot;

    texParam = tw_uv; /* placed in delay slot of first blez */

    if (input->h <= 0)
        return ot; /* lui v0,0x80 in delay slot */

    sprt = (SPRT*)pkt;

    /* Single 32‑bit store to r0,g0,b0,code (offsets 4..7) */
    *(u32*)(pkt + 4) = 0x00808080;

    setlen(sprt, 4);     /* sb v0,3(t1) */
    setcode(sprt, 0x64); /* sb v0,7(t1) */

    /* Combined 16‑bit store for u0,v0 (offset 0xC) */
    *(u16*)(pkt + 0xc) = 0; /* sh zero,0xc(t1) */

    sprt->w = input->w;  /* lhu w,  sh w  */
    sprt->h = input->h;  /* lhu h,  sh h  */
    sprt->x0 = input->x; /* lhu x,  sh x0 */
    sprt->clut = 0x7CCA; /* li clut,sh clut */
    sprt->y0 = input->y; /* lhu y,  sh y0 */

    addPrim(otp, sprt);

    pkt += 0x14; /* addiu t1,t1,0x14  -> pkt = &twin */
    twin = (DR_TWIN*)pkt;

    tw.x = texParam & 0xFF;
    tw.y = (texParam >> 8) & 0xFF;
    tw.w = 16;
    tw.h = 8;

    setTexWindow(twin, &tw); /* macro expands to len=2, code, zero */
    addPrim(otp, twin);

    pkt += 0xC; /* addiu t1,t1,0xC  -> after twin */
    return pkt; /* move v0,t1 */
}

/**
 * @brief Build a vertical border strip: one @c SPRT + one @c DR_TWIN primitive.
 *
 * Mirror of @ref menu_build_h_edge for vertical (left/right) window edges.
 * The texture-window region is 8x16 instead of 16x8.
 *
 * @param pkt     Primitive write cursor (SPRT is built here).
 * @param otp     Ordering-table head the primitives are linked into.
 * @param input   Edge rectangle: x, y (screen position), w, h (pixel size).
 * @param tw_uv   Packed texture-window origin: bits 7..0 = u (x), bits 15..8 = v (y).
 * @return Pointer to the byte immediately after the emitted DR_TWIN.
 * @see decomp.me (100%) https://decomp.me/scratch/19jr7
 */
void* menu_build_v_edge(u8* pkt, u_long* otp, InputStruct* input, s32 tw_uv)
{
    s32 texParam;
    RECT tw;

    if (input->w > 0)
    {
        texParam = tw_uv;
        if (input->h > 0)
        {
            *(u32*)(pkt + 4) = 0x00808080;
            setlen((SPRT*)pkt, 4);
            setcode((SPRT*)pkt, 0x64);
            *(u16*)(pkt + 0xc) = 0;
            ((SPRT*)pkt)->w = input->w;
            ((SPRT*)pkt)->h = input->h;
            ((SPRT*)pkt)->x0 = input->x;
            ((SPRT*)pkt)->y0 = input->y;
            ((SPRT*)pkt)->clut = 0x7CCA;

            addPrim(otp, (SPRT*)pkt);

            pkt += 0x14;
            tw.x = texParam & 0xFF;
            tw.y = texParam >> 8;
            tw.w = 8;
            tw.h = 0x10;

            setTexWindow((DR_TWIN*)pkt, &tw);
            addPrim(otp, (DR_TWIN*)pkt);

            pkt += 0xC;
        }
    }
    return pkt;
}

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

/** @brief 2-D screen coordinate (pixels). */
typedef struct
{
    s16 x; /**< Screen X. */
    s16 y; /**< Screen Y. */
} ScreenPos;

extern StringTableKey D_800EC3DA;
extern StringTableKey D_800EC3E4;

/**
 * @brief Render a text label from a packed string table at the given screen position.
 * @param arg0 OT (ordering table) pointer.
 * @param arg1 Primitive buffer context.
 * @param arg2 Screen position to draw at.
 * @param arg3 String table selector: >= 0 uses D_800EC3DA table, < 0 uses D_800EC3E4 table.
 * @see decomp.me (83.81%) https://decomp.me/scratch/ozwB7
 */
void menu_draw_label(s32 arg0, register s32 arg1, ScreenPos* arg2, s32 arg3)
{
    u8 sp20[16];
    u8* ptr = sp20;
    u8* addr;
    u32 unk1;
    u32 unk0;
    u8* temp_s0;

    if (arg3 >= 0)
    {
        unk1 = D_800EC3DA.page;
        unk0 = D_800EC3DA.entry;
        addr = (u8*)&D_800EC3DA - 0x16;
    }
    else
    {
        unk1 = D_800EC3E4.page;
        addr = (u8*)&D_800EC3E4 - 0x20;
        unk0 = D_800EC3E4.entry;
    }

    temp_s0 = unk0 + ((unk1 << 8) + addr);

    func_800A8E28(ptr, temp_s0);
    ptr += func_800A8DDC(temp_s0);

    *ptr = 0;

    func_800A88A0(arg1, arg0, sp20, 1, arg2->x, arg2->y, 0);
}

typedef struct
{
    u8 unk0;
    u8 unk1;
    union
    {
        u16 unk2;
        struct
        {
            u8 unk2_hi;
            u8 parent_idx;
        } s;
    } u2;
    u8 unk4;
    u8 unk5;
    union
    {
        u16 unk6;
        struct
        {
            u8 unk6_hi;
            u8 unk7;
        } s;
    } u6;
    union
    {
        u16 unk8;
        struct
        {
            u8 unk8_hi;
            u8 unk9;
        } s;
    } u8_u;
    union
    {
        u16 unkA;
        struct
        {
            u8 unkA_hi;
            u8 unkB;
        } s;
    } uA;
    u8 unkC;
    u8 unkD;
    u8 unkE;
    u8 unkF;
} MenuNode;
extern MenuNode g_menu_nodes[0x2C];
extern u8 D_801690B3;
extern s32 D_80168C2C;

typedef struct
{
    u32 unk0;
    u32 unk4;
    u32 unk8;
    u32 unkC;
} Struct_D_80169538;
extern Struct_D_80169538 D_80169538;

typedef struct
{
    u8 unk0;
    u8 unk1;
    u8 unk2;
    u8 unk3;
} Struct_D_80168C1C;
extern Struct_D_80168C1C D_80168C1C;

extern s32 D_801690EC;
extern s32 D_80169410;
extern s32 D_80169404;
extern s32 D_80169408;
extern s32 D_8016911C;
extern s32 D_80169554;
extern s32 D_801694B0;
extern s32 D_801694B4;
extern s32 D_80169418;
extern s32 D_8016954C;
extern s32 D_80169400;
extern s32 D_801690E4;
extern s32 D_80169130;
extern u8 D_8016913D;

typedef struct
{
    u16 unk0;
    u8 pad2[0x266];
    u16 unk268;
    u8 unk26A;
    u8 unk26B;
} Struct_D_800FD818;

extern Struct_D_800FD818 D_800FD818;
extern u16 D_800FDA80;
extern u16 D_800FDCE8;
extern s8 D_801690F9;
extern s8 D_80169324;

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
            u8 unk8_hi;
            u8 unk9;
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
    D_801690B3 = 0xFF;
    D_80168C2C = 0;
    D_80169538.unk0 = 0;
    D_80169538.unk4 = 0;
    D_80169538.unk8 = 0;
    D_80169538.unkC = 0;
    D_80168C1C.unk0 = 0;
    D_80168C1C.unk1 = 0;
    D_80168C1C.unk2 = 0;
    D_80168C1C.unk3 = 0;
    D_801690EC = 0;
    D_80169410 = 0;
    D_80169404 = 0;
    D_80169408 = 0;
    D_8016911C = 0;
    D_80169554 = 0;
    D_801694B0 = 0;
    D_801694B4 = 0;
    D_80169418 = 0;
    D_8016954C = 0;
    D_80169400 = 0;
    D_801690E4 = 0;
    D_80169130 = 0;
    for (var_t0 = 0; var_t0 < 0x2C; var_t0++)
    {
        g_menu_nodes[var_t0].unk1 = 0;
        g_menu_nodes[var_t0].unk4 = 0;
        g_menu_nodes[var_t0].unk5 = var_a1;
        g_menu_nodes[var_t0].unkE = var_a1;
        g_menu_nodes[var_t0].unkD = var_a1;
        g_menu_nodes[var_t0].unkC = var_a1;
        g_menu_nodes[var_t0].uA.s.unkB = var_a1;
        g_menu_nodes[var_t0].u2.unk2 = (u16)((g_menu_nodes[var_t0].u2.unk2 & 0xFFFC) | 0x30);
        g_menu_nodes[var_t0].u2.s.parent_idx = var_a1;
    }

    g_menu_nodes[0].unk0 = 1;
    g_menu_nodes[0].u6.s.unk6_hi = 0;
    original_unk2 = g_menu_nodes[0].u2.unk2;
    temp1 = original_unk2 & 0xFFCD;
    temp2 = original_unk2 & 0xFF0D;
    *((volatile u16*)(&g_menu_nodes[0].u2.unk2)) = temp1;
    *((volatile u16*)(&g_menu_nodes[0].u2.unk2)) = temp2;
    *((volatile u16*)(&g_menu_nodes[0].u2.unk2)) = temp2 | 1;
    g_menu_nodes[0].u2.s.parent_idx = 0xFF;
    if (D_800FD818.unk0 & 2)
    {
        g_menu_nodes[0].unk4 = 2;
    }
    else
    {
        g_menu_nodes[0].unk4 = 1;
    }
    g_menu_nodes[0].uA.s.unkB = 1;
    g_menu_nodes[1].u6.s.unk6_hi = 1;
    g_menu_nodes[1].unk4 = 5;
    g_menu_nodes[0].unkC = 2;
    g_menu_nodes[2].unk0 = 2;
    g_menu_nodes[2].u6.s.unk6_hi = 2;
    g_menu_nodes[2].unk4 = 4;
    g_menu_nodes[3].unk0 = 4;
    g_menu_nodes[1].unk0 = 3;
    g_menu_nodes[3].u6.s.unk6_hi = 3;
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
    g_menu_nodes[3].u2.s.parent_idx = 0xFF;
    g_menu_nodes[4].u6.s.unk6_hi = 4;
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
    g_menu_nodes[3].uA.s.unkB = 4;
    g_menu_nodes[3].unkC = 5;
    g_menu_nodes[4].unk0 = 6;
    g_menu_nodes[4].unk4 = 5;
    g_menu_nodes[5].unk0 = 5;
    g_menu_nodes[5].u6.s.unk6_hi = 5;
    g_menu_nodes[5].unk4 = 4;
    g_menu_nodes[6].unk0 = 7;
    g_menu_nodes[6].u6.s.unk6_hi = 6;
    g_menu_nodes[6].unk4 = 3;
    g_menu_nodes[6].uA.s.unkB = 7;
    g_menu_nodes[6].unkC = 8;
    g_menu_nodes[7].unk0 = 9;
    g_menu_nodes[7].u6.s.unk6_hi = 7;
    new_var7 = 0xFF3E;
    g_menu_nodes[7].unk4 = 5;
    g_menu_nodes[8].unk0 = 8;
    g_menu_nodes[4].u2.s.parent_idx = 3;
    g_menu_nodes[5].u2.s.parent_idx = 3;
    g_menu_nodes[6].u2.s.parent_idx = 0xFF;
    g_menu_nodes[7].u2.unk2 = (u16)((g_menu_nodes[7].u2.unk2 & 0xFF5F) | 0x50);
    g_menu_nodes[7].u2.s.parent_idx = 6;
    g_menu_nodes[8].u2.unk2 = (u16)((g_menu_nodes[8].u2.unk2 & 0xFF5F) | 0x50);
    g_menu_nodes[8].u6.s.unk6_hi = 8;
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
    g_menu_nodes[9].u6.s.unk6_hi = 9;
    g_menu_nodes[9].unk4 = 6;
    g_menu_nodes[9].uA.s.unkB = 0xA;
    g_menu_nodes[0xA].unk0 = 0xB;
    g_menu_nodes[0x12].unk4 = 0xA;
    g_menu_nodes[0xA].u6.s.unk6_hi = 0xA;
    g_menu_nodes[0xA].unk4 = 7;
    g_menu_nodes[0xC].unk0 = 0xA;
    g_menu_nodes[0xC].u6.s.unk6_hi = 0xC;
    g_menu_nodes[0xC].unk4 = 6;
    g_menu_nodes[0xC].uA.s.unkB = 0xD;
    g_menu_nodes[0xD].unk0 = 0xB;
    g_menu_nodes[0xD].u6.s.unk6_hi = 0xD;
    g_menu_nodes[0xD].unk4 = 7;
    g_menu_nodes[9].u2.s.parent_idx = 0xFF;
    temp_v0_7 = 0xF;
    g_menu_nodes[0xA].u2.s.parent_idx = 9;
    g_menu_nodes[0xC].u2.s.parent_idx = 0xFF;
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
    g_menu_nodes[0xF].u2.s.parent_idx = 0xFF;
    g_menu_nodes[0xF].unk0 = 0xD;
    g_menu_nodes[0xF].u6.s.unk6_hi = 0xF;
    g_menu_nodes[0xF].uA.s.unkB = 0x10;
    g_menu_nodes[0xF].unkC = 0x11;
    g_menu_nodes[0x10].unk0 = 0xC;
    g_menu_nodes[0x10].u6.s.unk6_hi = 0x10;
    g_menu_nodes[0x11].unk0 = 0xE;
    g_menu_nodes[0x11].u6.s.unk6_hi = 0x11;
    g_menu_nodes[0x11].unk4 = 9;
    g_menu_nodes[0x12].unk0 = 0x10;
    g_menu_nodes[0x12].u6.s.unk6_hi = 0x12;
    g_menu_nodes[0x12].unk5 = 4;
    g_menu_nodes[0x12].uA.s.unkB = 0x13;
    g_menu_nodes[0x12].unkC = 0x16;
    g_menu_nodes[0x12].unkD = 0x19;
    g_menu_nodes[0x12].unkE = 0x1C;
    g_menu_nodes[0x10].u2.unk2 = (u16)((g_menu_nodes[0x10].u2.unk2 & 0xFF6F) | 0x60);
    g_menu_nodes[0x10].u2.s.parent_idx = 0xF;
    g_menu_nodes[0x11].u2.unk2 = (u16)((g_menu_nodes[0x11].u2.unk2 & 0xFF6F) | 0x60);
    g_menu_nodes[0x11].u2.s.parent_idx = 0xF;
    temp_v0_12 = g_menu_nodes[0x12].u2.unk2 & 0xFF3D;
    g_menu_nodes[0x12].u2.unk2 = (u16)(g_menu_nodes[0x12].u2.unk2 & 0xFFFD);
    g_menu_nodes[0x12].u2.unk2 = temp_v0_12;
    g_menu_nodes[0x12].u2.unk2 = (u16)(temp_v0_12 | 1);
    g_menu_nodes[0x12].u2.s.parent_idx = 0xFF;
    g_menu_nodes[0x13].unk0 = 0x11;
    g_menu_nodes[0x16].unk5 = 1;
    g_menu_nodes[0x14].unk4 = 0xF;
    g_menu_nodes[0x13].unk4 = 0xB;
    g_menu_nodes[0x13].u6.s.unk6_hi = 0x13;
    g_menu_nodes[0x13].unk5 = 0;
    g_menu_nodes[0x13].uA.s.unkB = 0x14;
    g_menu_nodes[0x13].unkC = 0x15;
    g_menu_nodes[0x14].unk0 = 0x12;
    g_menu_nodes[0x14].u6.s.unk6_hi = 0x14;
    g_menu_nodes[0x15].unk0 = 0x13;
    g_menu_nodes[0x15].u6.s.unk6_hi = 0x15;
    g_menu_nodes[0x15].unk4 = 0x12;
    g_menu_nodes[0x16].unk0 = 0x14;
    g_menu_nodes[0x16].u6.s.unk6_hi = 0x16;
    g_menu_nodes[0x16].unk4 = 0xC;
    g_menu_nodes[0x16].uA.s.unkB = 0x17;
    g_menu_nodes[0x16].unkC = 0x18;
    g_menu_nodes[0x17].unk0 = 0x15;
    g_menu_nodes[0x17].u6.s.unk6_hi = 0x17;
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
    g_menu_nodes[0x19].unk5 = 2;
    g_menu_nodes[0x17].unk4 = 0x10;
    g_menu_nodes[0x18].unk0 = 0x13;
    g_menu_nodes[0x18].u6.s.unk6_hi = 0x18;
    g_menu_nodes[0x18].unk4 = 0x12;
    g_menu_nodes[0x19].unk0 = 0x16;
    g_menu_nodes[0x19].u6.s.unk6_hi = 0x19;
    g_menu_nodes[0x19].unk4 = 0xD;
    g_menu_nodes[0x19].uA.s.unkB = 0x1A;
    g_menu_nodes[0x19].unkC = 0x1B;
    g_menu_nodes[0x1A].unk0 = 0x17;
    g_menu_nodes[0x1A].u6.s.unk6_hi = 0x1A;
    g_menu_nodes[0x1A].unk4 = 0x11;
    g_menu_nodes[0x1B].unk0 = 0x13;
    g_menu_nodes[0x1B].u6.s.unk6_hi = 0x1B;
    g_menu_nodes[0x1B].unk4 = 0x12;
    g_menu_nodes[0x1C].unk0 = 0x18;
    g_menu_nodes[0x1C].u6.s.unk6_hi = 0x1C;
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
    g_menu_nodes[0x1D].u6.s.unk6_hi = 0x1D;
    g_menu_nodes[0x1E].uA.s.unkB = 0x1F;
    g_menu_nodes[0x1F].u6.s.unk6_hi = 0x1F;
    g_menu_nodes[0x1C].unk5 = 5;
    g_menu_nodes[0x1D].unk0 = 0x1C;
    g_menu_nodes[0x1D].unk5 = 3;
    g_menu_nodes[0x1D].unk4 = 0x18;
    g_menu_nodes[0x1E].unk0 = 0x19;
    g_menu_nodes[0x1E].u6.s.unk6_hi = 0x1E;
    g_menu_nodes[0x1E].unk4 = 0x13;
    g_menu_nodes[0x1F].unk0 = 0x1A;
    g_menu_nodes[0x1F].unk4 = 0x14;
    g_menu_nodes[0x2B].unk0 = 0x1A;
    g_menu_nodes[0x2B].u6.s.unk6_hi = 0x2B;
    temp_v0_13 = g_menu_nodes[0x1D].u2.unk2 & 0xFF3D;
    g_menu_nodes[0x1D].u2.unk2 = (u16)(g_menu_nodes[0x1D].u2.unk2 & 0xFFFD);
    (*(&g_menu_nodes[0x1D])).u2.unk2 = temp_v0_13;
    temp_v1 = g_menu_nodes[0x1E].u2.unk2;
    g_menu_nodes[0x1D].u2.unk2 = (u16)(temp_v0_13 | 1);
    g_menu_nodes[0x1D].u2.s.parent_idx = 0xFF;
    g_menu_nodes[0x1F].u2.unk2 = (u16)((g_menu_nodes[0x1F].u2.unk2 & 0xFF3F) | 0x40);
    g_menu_nodes[0x1F].u2.s.parent_idx = 0x1E;
    g_menu_nodes[0x1E].u2.unk2 = (u16)(temp_v1 & 0xFFFD);
    temp_v1_2 = temp_v1 & 0xFF3D;
    temp_v1 = (u16)((g_menu_nodes[0x2B].u2.unk2 & 0xFF3F) | 0x40);
    g_menu_nodes[0x1E].u2.unk2 = temp_v1_2;
    g_menu_nodes[0x2B].u2.unk2 = temp_v1;
    g_menu_nodes[0x1E].u2.unk2 = (u16)(temp_v1_2 | 1);
    g_menu_nodes[0x1E].u2.s.parent_idx = 0xFF;
    g_menu_nodes[0x2B].u2.s.parent_idx = 0x1E;
    g_menu_nodes[0x1F].u2.unk2 = (u16)(g_menu_nodes[0x1F].u2.unk2 & 0xFFCF);
    g_menu_nodes[0x2B].u2.unk2 = (u16)((g_menu_nodes[0x2B].u2.unk2 & 0xFFCF) | 0x10);
    g_menu_nodes[0x2B].unk4 = 0x15;
    g_menu_nodes[0x20].unk0 = 0x1B;
    g_menu_nodes[0x20].u6.s.unk6_hi = 0x20;
    temp_v0_14 = 0xFF3D;
    temp_v0_14 = g_menu_nodes[0x20].u2.unk2 & temp_v0_14;
    new_var6 = D_800FD818.unk268 & 1;
    (*(g_menu_nodes + 0x20)).u2.unk2 = (u16)(g_menu_nodes[0x20].u2.unk2 & 0xFFFD);
    g_menu_nodes[0x20].u2.unk2 = temp_v0_14;
    g_menu_nodes[0x20].unk4 = 0x16;
    g_menu_nodes[0x20].u2.unk2 = (u16)(temp_v0_14 | 1);
    g_menu_nodes[0x20].u2.s.parent_idx = 0xFF;
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
                var_a3 = var_a3 + 0x13;
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
    } while (var_t0_2 < 0x2C);
    if (g_active_script != 0)
    {
        g_menu_scene_type = -1;
        return;
    }
    g_menu_scene_type = 0;
    new_var4 = D_8016913D;
    D_801690F9 = 0;
    func_8014E3C4(new_var4, var_a1, new_var3, var_a3);
    func_801436F0();
}

/**
 * @brief Clears the expand flag (bit 1 of u2.unk2) for all menu nodes.
 * @note Called before menu_update_layout to ensure no node recurses into children.
 * @see decomp.me (100%) https://decomp.me/scratch/hyDM7
 */
void menu_collapse_all(void)
{
    s32 i;
    for (i = 0; i < 0x2C; i++)
    {
        g_menu_nodes[i].u2.unk2 &= 0xFFFD;
    }
}

extern s32 g_menu_layout_end;

/**
 * @brief Assigns layout positions to all active root menu nodes and stores the final position count.
 * @note Sets D_8016954C and D_80169400 to signal scroll state after layout.
 * @see decomp.me (100%) https://decomp.me/scratch/YhGni
 */
void menu_update_layout(void)
{
    s32 changed = 0;
    s32 pos = changed;
    s32 i = pos;

    do
    {
        if (g_menu_nodes[i].u2.s.parent_idx == 0xFF)
        {
            s32 prev_pos = pos;
            pos++;
            pos--;

            if (g_menu_nodes[i].u2.s.unk2_hi & 1)
            {
                pos = menu_layout_node(i, prev_pos);
                if (prev_pos != (pos - 0x13))
                {
                    changed = 1;
                    if (pos >= 0xAC)
                    {
                        D_8016954C = pos - 0xAB;
                        D_80169400 = 8;
                    }
                }
            }
        }
        i += 1;
    } while (i < 0x2C);

    g_menu_layout_end = pos;
    if (changed == 0)
    {
        D_8016954C = 0;
        D_80169400 = 8;
    }
}

/**
 * @brief Assigns a position slot to a menu node and optionally recurses into its first child.
 * @param node_idx Index into g_menu_nodes of the node to lay out.
 * @param base_pos Running position counter; this node occupies [base_pos, base_pos+0x13).
 * @return Updated position counter after processing this node and any expanded children.
 * @note Bit 1 of u2.unk2 controls child recursion; menu_collapse_all clears it before layout.
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
            u8 unk8_hi;
            u8 unk9;
        } s;
    }* new_var2;
    MenuNode* new_var3;
    cur_pos = base_pos;
    has_children = (((u16)(&g_menu_nodes[node_idx])->u2.unk2) >> 1) & 1;
    temp_a1 = cur_pos & 0xFFFF;
    cur_pos += 0x13;
    new_var4 = &g_menu_nodes[node_idx];

    (*(&g_menu_nodes[node_idx])).unk1 = 4;
    new_var2 = &new_var4->u8_u;
    new_var4->u8_u.unk8 = (*new_var2).s.unk8_hi | ((temp_a1 & 1) << 0xF);
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
            child_idx = (&(&(*new_var3).uA.s)->unkB)[child_idx];
            child = child_idx;
            if (child == 0xFF)
            {
                break;
            }
            child_idx++;
            cur_pos = menu_layout_node(child, cur_pos);
        }
    }
    return cur_pos;
}