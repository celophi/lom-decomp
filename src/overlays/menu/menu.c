#include "menu.h"

typedef struct UnkArg0
{
    u8 pad0[0x34];
    u8 unk34;         /* Start of buffer at offset 0x34 */
    u8 pad35[0x400B]; /* Padding to 0x4040 */
    s32 unk4040;      /* Offset 0x4040 */
    u8 pad4044[8];    /* Padding to 0x404C */
    s32 unk404C;      /* Offset 0x404C */
} UnkArg0;

typedef struct
{
    u8 _pad0[2]; // offsets 0x00-0x01
    u8 unk2;     // offset 0x02
    u8 _pad1[5]; // offsets 0x03-0x07
    u16 unk8;    // offset 0x08
    u16 unkA;    // offset 0x0A
    s16 unkC;    // offset 0x0C
    s16 unkE;    // offset 0x0E
} UnknownStruct;

extern s32 D_80169124;
extern s32 D_80169130;

/* K&R-style declaration: original call site in menu_tick passes no explicit
 * argument and relies on register a0 (the caller's first parameter) being
 * live. Keep the empty parameter list to preserve that codegen exactly. */
void menu_build_grid();

void menu_init(void)
{
    volatile u8 padding;
    menu_clear_vram();
    menu_state_init();
    func_80141324();
    g_active_slot = -1;
    func_800AA02C();
    g_menu_unk_e8 = 0;
    menu_init_prim_rects();
    g_menu_frame = 0;
    g_script_cursor = 0;
    func_801423D8();
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
 *                 entry and restored before @ref func_8014134C runs.
 * @see decomp.me (97.74%) https://decomp.me/scratch/vmp4D
 */
void menu_tick(RenderContext* gpu_work)
{
    s32 v0;
    s32 v1;
    s32 saved_prim_cursor;
    s32 var_s0;
    u16 temp_v1;
    s32 padding[2];
    menu_build_grid();
    v0 = g_menu_frame;
    v1 = g_frame_counter;
    /* RenderContext.prim_cursor - kept as a raw offset load to preserve codegen */
    saved_prim_cursor = *((s32*)(((u8*)gpu_work) + 0x4040));
    g_menu_frame = v0 + 1;
    g_frame_counter = v1 + 1;
    func_800A9E78(&g_menu_frame, &g_frame_counter);
    if (((*((u32*)(((u8*)g_pad_ctx) + 0x858))) & 0x80) && ((*((u8*)(((u8*)g_pad_ctx) + 0x840))) != 0))
    {
        g_pad_input |= g_pad_input_inject;
    }
    v0 = g_pad_input & 0x5000;
    if (v0)
    {
        g_pad_input = v0;
    }
    v0 = g_pad_input & 0xF000;
    if (v0)
    {
        g_pad_input = v0;
    }
    v0 = g_pad_input & 0xF;
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
        s32 idx;
        u8* base = (u8*)g_script_table;
        s32 off = g_active_script;
        off = (off << 1) + off;
        off <<= 4;
        base += off;
        idx = g_script_cursor;
        base += idx * 2;
        g_pad_input = 0;
        temp_v1 = *((u16*)base);
        if (temp_v1 == (v0 = 0xFFFF))
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
    func_8014134C(gpu_work);
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
            *(u32*)(base + 0x4) = 0x808080U;
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
        *(u32*)(var_a2 + 4) = 0x808080;
        *(u8*)(var_a2 + 3) = 4;
        *(u8*)(var_a2 + 7) = 0x64;
        *(u16*)(var_a2 + 0xC) = *(u16*)var_t3;
        *(u32*)(var_a2 + 8) = *(u32*)(var_t0 - 4);
        *(u32*)(var_a2 + 0x10) = *(u32*)var_t0;

        if (var_t2 >= 0x11)
        {
            *(u16*)(var_a2 + 0xE) = 0x7C81;
        }
        else
        {
            *(u16*)(var_a2 + 0xE) = 0x7C80;
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
 * @see decomp.me (99.82%) https://decomp.me/scratch/Xng7v
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
    entry = (MenuSlot*)(ptr + (var_a2 * 0x24));
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
 * decomp.me (100%) https://decomp.me/scratch/D9BI9
 */
void func_80141324(void)
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
 * decomp.me (77.68%) https://decomp.me/scratch/BlGK5
 */
void func_8014134C(UnkArg0* arg0)
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

    D_80169124 = 0;
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

            func_80141570(arg0, var_s0, D_80169130 != 0);
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
            func_8014169C(var_s0, arg0, var_s2, sp_pair, D_80169130 != 0);
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
        func_80141570(arg0, var_s0, D_80169130 != 0);
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

    if ((g_active_slot == -1) || (D_80169130 == 0))
    {
        var_a3 = 1;
    }

    arg0->unk4040 = func_80142F10(arg0->unk4040, &arg0->unk34, arg0->unk404C, var_a3);

    if (D_80169124 != 0)
    {
        arg0->unk4040 = func_800A88A0(arg0->unk4040, &arg0->unk34, D_80169124, 1, 0xA0, 0xCA, 2);
    }
}

/**
 * decomp.me (100%) https://decomp.me/scratch/luaLZ
 */
void func_80141570(s32 arg0, UnknownStruct* arg1, s32 arg2)
{
    s16 sp[8];
    s32 temp_a3;
    s32 temp_a1;
    s32 clampC;
    s32 clampE;

    /* First computation */
    temp_a3 = ((s32)((u16)arg1->unkC << 0x10) >> 0x11) - ((s16)(arg1->unkC / 12) * arg1->unk2);
    sp[4] = (s16)temp_a3;

    /* Second computation */
    temp_a1 = ((s32)((u16)arg1->unkE << 0x10) >> 0x11) - ((s16)(arg1->unkE / 12) * arg1->unk2);

    /* First branch logic block */

    sp[5] = (s16)temp_a1;

    /* Second branch logic block */
    if (temp_a3 > 0)
    {

        if (temp_a1 > 0)
        {

            clampC = arg1->unkC - (temp_a3 * 2);
            if (clampC < 0x20)
            {
                clampC = 0x20;
            }

            clampE = arg1->unkE - (temp_a1 * 2);
            if (clampE < 0x10)
            {
                clampE = 0x10;
            }

            sp[0] = arg1->unk8 + temp_a3;
            sp[1] = arg1->unkA + temp_a1;
            sp[2] = clampC;
            sp[3] = clampE;

            func_8014169C(arg1, arg0, &sp[0], &sp[4], arg2);
        }
    }
}