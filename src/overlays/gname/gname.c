#include "gname.h"

/**
 * @brief Reset the RGB fade state.
 *
 * Zeros the current color (@c g_fade_current) and the target color +
 * step count (@c g_fade_target). After this call the next
 * @ref render_fade_overlay tick will write a black tint (0,0,0) to the
 * primitive at @c arg0->unk4040.
 *
 * @see https://decomp.me/scratch/ld2aW (100%)
 */
void reset_fade_state(void)
{
    g_fade_current.r = 0;
    g_fade_current.g = 0;
    g_fade_current.b = 0;
    g_fade_target.r = 0;
    g_fade_target.g = 0;
    g_fade_target.b = 0;
    g_fade_target.steps = 0;
}

/**
 * @brief Per-frame RGB fade tick: lerp current toward target and emit a
 *        full-screen tint quad + draw-mode pair into the OT.
 *
 * If `g_fade_target.steps` is non-zero, advances `g_fade_current` by one step of
 * `(target - current) / steps` per channel and decrements `steps`.
 * Otherwise snaps current to target (RGB only — `steps` is left alone).
 *
 * If the current color is anything other than (0x100, 0x100, 0x100) — i.e.
 * not the no-tint identity — emits two primitives at @c ctx->unk4040:
 *   1. A 16-byte flat-shaded full-screen quad (tag 0x62) covering the
 *      320x240 viewport with the tinted RGB. When any channel is >0x100,
 *      colors are written as `value - 1` (additive bias); when <0x100,
 *      colors are written as `~value` (subtractive bias).
 *   2. An 8-byte Draw-Mode (GP0 0xE1) packet selecting abr=2 (additive,
 *      command bits 0x25) for >0x100 brightening or abr=1 (subtractive,
 *      0x45) for <=0x100 darkening.
 *
 * Both packets are spliced into the 24-bit OT at @c ctx->unk0 and
 * `ctx->unk4040` is advanced past them. When the color is identity, no
 * primitives are emitted and the heap cursor is unchanged.
 *
 * @param ctx Render context whose `unk0` is the OT head and `unk4040` is
 *            the primitive heap cursor.
 *
 * @note Equivalent to TITLE.BIN's RenderFadeOverlay.
 * @see https://decomp.me/scratch/hVLdu (100%)
 */
void render_fade_overlay(ArgStruct* ctx)
{
    u32* prim = (u32*)ctx->unk4040; /* t4 — current primitive write pos */
    ArgStruct* arg = ctx;           /* t6 = t7 — preserves load order */
    s32 step_r, step_g, step_b;
    s32 abr_cmd; /* low byte of GP0 0xE1 (abr select) */

    /* Lerp current toward target, or snap if no steps remain. */
    if (g_fade_target.steps != 0)
    {
        step_r = (g_fade_target.r - g_fade_current.r) / g_fade_target.steps;
        step_g = (g_fade_target.g - g_fade_current.g) / g_fade_target.steps;
        step_b = (g_fade_target.b - g_fade_current.b) / g_fade_target.steps;
        g_fade_target.steps--;
        g_fade_current.r += step_r;
        g_fade_current.g += step_g;
        g_fade_current.b += step_b;
    }
    else
    {
        g_fade_current.r = g_fade_target.r;
        g_fade_current.g = g_fade_target.g;
        g_fade_current.b = g_fade_target.b;
    }

    /* Skip emit when fully transparent / identity tint. */
    if (!((g_fade_current.r == 0x100) && (g_fade_current.g == 0x100) && (g_fade_current.b == 0x100)))
    {
        /* Flat-quad RGB bytes at prim[4..6]. */
        if (g_fade_current.r >= 0x101)
        {
            /* Additive bias: subtract 1 so 0x101 → 0x00..0xFF. */
            ((u8*)prim)[4] = (u8)g_fade_current.r - 1;
            ((u8*)prim)[5] = (u8)g_fade_current.g - 1;
            ((u8*)prim)[6] = (u8)g_fade_current.b - 1;
        }
        else
        {
            /* Subtractive bias: bitwise NOT so 0xFF → 0x00, 0x00 → 0xFF. */
            if (g_fade_current.r == 0x100)
                ((u8*)prim)[4] = 0;
            else
                ((u8*)prim)[4] = ~(u8)g_fade_current.r;

            if (g_fade_current.g == 0x100)
                ((u8*)prim)[5] = 0;
            else
                ((u8*)prim)[5] = ~(u8)g_fade_current.g;

            if (g_fade_current.b == 0x100)
                ((u8*)prim)[6] = 0;
            else
                ((u8*)prim)[6] = ~(u8)g_fade_current.b;
        }

        /* Flat-shaded full-screen quad header: 3-word tag 0x62. */
        ((u8*)prim)[3] = 3;
        ((u8*)prim)[7] = 0x62;

        *((u16*)((u8*)prim + 10)) = 0; /* y = 0   */
        *((u16*)((u8*)prim + 8)) = 0;  /* x = 0   */

        *((u16*)((u8*)prim + 12)) = 0x140; /* w = 320 */
        *((u16*)((u8*)prim + 14)) = 0xF0;  /* h = 240 */

        /* Splice quad into OT. */
        *prim = (*prim & 0xFF000000) | (arg->unk0 & 0xFFFFFF);
        arg->unk0 = (arg->unk0 & 0xFF000000) | ((u32)prim & 0xFFFFFF);

        /* Choose blend mode by direction of tint. */
        abr_cmd = 0x25;
        prim = (u32*)((u8*)prim + 0x10);
        if (g_fade_current.r < 0x101)
            abr_cmd = 0x45;

        /* Draw-Mode packet (GP0 0xE1 | abr_cmd). */
        ((u8*)prim)[3] = 1;
        *((u32*)((u8*)prim + 4)) = abr_cmd | 0xE1000000;

        /* Splice draw-mode packet into OT. */
        *prim = (*prim & 0xFF000000) | (arg->unk0 & 0xFFFFFF);
        arg->unk0 = (arg->unk0 & 0xFF000000) | ((u32)prim & 0xFFFFFF);
        prim = (u32*)((u8*)prim + 8);
    }

    ctx->unk4040 = prim;
}

/**
 * @brief Set the RGB fade target and step count.
 *
 * Writes the four-field target struct in one call. The next
 * `steps` ticks of @ref render_fade_overlay will lerp the current color toward
 * `(r, g, b)` and then snap on the final tick.
 *
 * @param r     Target red   (0..0x100 normal, >0x100 = additive).
 * @param g     Target green (0..0x100 normal, >0x100 = additive).
 * @param b     Target blue  (0..0x100 normal, >0x100 = additive).
 * @param steps Frames over which to interpolate. 0 means "snap immediately".
 *
 * @see https://decomp.me/scratch/jq3uD (100%)
 */
void set_fade_target(s32 r, s32 g, s32 b, s32 steps)
{
    g_fade_target.r = r;
    g_fade_target.g = g;
    g_fade_target.b = b;
    g_fade_target.steps = steps;
}

/**
 * @brief Overlay boot/reset entry: prep VRAM, load CLUT, init run state.
 *
 * Calls (in order):
 *  - @ref func_8014075C  — RECT build + handoff (likely VRAM clear or CLUT load).
 *  - @ref func_800AA02C  — engine helper (audio/SFX init).
 *  - sets @c g_startup_delay = 0x28 (40 — likely a startup delay countdown).
 *  - @ref func_8006441C  — engine helper.
 *  - @ref reset_run_state  — zero/seed all of the overlay's run-state globals.
 *  - @ref func_80063194  — engine helper.
 *
 * @see https://decomp.me/scratch/pnzC1 (100%)
 */
void gname_init(void)
{
    volatile int dummy[2]; /* forces 0x20 stack frame, ra at 0x18(sp) */
    func_8014075C();
    func_800AA02C();
    g_startup_delay = 0x28;
    func_8006441C();
    reset_run_state();
    func_80063194();
}

/**
 * @brief Build a hard-coded VRAM RECT and hand it to @ref func_80140794.
 *
 * Constructs a `RECT { x=0x140 (320), y=0, w=0, h=0x1F2 (498) }` on the
 * stack — note the field order matches the data passed to the consumer,
 * which reads `arr[0..3]` directly (not the standard PSY-Q `RECT` order).
 *
 * @see https://decomp.me/scratch/EWwJI (100%)
 */
void func_8014075C(void)
{
    s16 rect[4];
    rect[0] = 0x140; /* 320 */
    rect[1] = 0;
    rect[2] = 0;
    rect[3] = 0x1F2; /* 498 */
    func_80140794(rect);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/P3W9C
 */
void func_80140794(void* arg0)
{
    void* s0 = arg0;
    u16 new_var;
    u16* s1 = (u16*)D_80147494;
    s32 s2 = *((s32*)(((u8*)s1) + 8));
    u16 new_var2;
    u16* ptr = (u16*)(((u8*)s1) + 0x14);
    u16 arr[4];
    int counter;
    u16 tmp0 = *((u16*)(((u8*)s0) + 4));
    counter = 0;
    arr[0] = tmp0;
    new_var = *((u16*)(((u8*)s0) + 6));
    arr[2] = 0x100;
    arr[3] = 1;
    arr[1] = new_var;
    do
    {
        if ((*ptr) != 0)
        {
            *ptr |= 0x8000;
        }
        ptr++;
        counter++;
    } while (counter < 0x100);
    func_80019A34((u16*)arr, ((u8*)s1) + 0x14);
    arr[0] = *((u16*)(((u8*)s0) + 0));
    arr[1] = *((u16*)(((u8*)s0) + 2));
    {
        u16* p = (u16*)(((u8*)s1) + (s2 + 8));
        arr[2] = p[4];
        arr[3] = p[5];
        func_80019A34((u16*)arr, p + 6);
    }
    arr[0] = *((u16*)(((u8*)s0) + 4));
    new_var2 = *((u16*)(((u8*)s0) + 6));
    arr[2] = 0x100;
    arr[3] = 1;
    arr[1] = new_var2 + 1;
}

/**
 * @brief Per-frame tick: reset/prep, render frame contents, advance frame
 *        counter, advance overlay state machine.
 *
 *  - @ref func_80142410  — frame prologue (likely OT reset / heap rewind).
 *  - @ref func_80141928  — main render pass for this overlay.
 *  - increments the global frame counter @c D_800F22AC.
 *  - @ref gname_update_state  — countdown / lerp / SFX trigger update.
 *
 * @param ctx Render context passed through to @ref func_80141928.
 *
 * @see https://decomp.me/scratch/yYkTM (100%)
 */
void gname_tick(s32 ctx)
{
    func_80142410();
    func_80141928(ctx);
    D_800F22AC += 1;
    gname_update_state();
}

/**
 * @brief Per-frame state-machine update: countdown, scalar lerp, input SFX.
 *
 *  - When the startup countdown @c g_startup_delay hits zero, hands off to
 *    @ref func_8014139C (the next stage); otherwise decrements it.
 *  - Lerps the scalar @c g_strip_width toward @c g_strip_width_target over
 *    @c g_strip_width_steps frames using the same `(target - current)/steps` shape
 *    as the RGB fade.
 *  - When input mask @c D_80122988 == 0x800 (a specific button bit), plays
 *    one of two SFX via @ref func_800A3938 (bank 0x80, sound 0x7E or 0x78)
 *    based on whether the cursor's current entry passes the
 *    @ref name_char_count / @ref name_is_blank validation pair, and on the
 *    "valid" path also kicks @c D_8014F7E4 = 5 to advance overlay state.
 *
 * @see https://decomp.me/scratch/g5Rx3 (100%)
 */
void gname_update_state(void)
{
    s32 steps;

    /* Startup delay countdown. */
    if (g_startup_delay == 0)
    {
        func_8014139C();
    }
    else
    {
        g_startup_delay--;
    }

    /* Lerp g_strip_width toward g_strip_width_target, snap when no steps remain. */
    steps = g_strip_width_steps;
    if (steps != 0)
    {
        g_strip_width_steps--;
        g_strip_width += (g_strip_width_target - g_strip_width) / steps;
    }
    else
    {
        g_strip_width = g_strip_width_target;
    }

    /* Confirm-button: play accept SFX on valid entry, reject SFX otherwise. */
    if (D_80122988 == 0x800)
    {
        if ((name_char_count(g_active_name) != 0) && (name_is_blank(g_active_name) == 0))
        {
            func_800A3938(0x7E, 0x80); /* accept */
            D_8014F7E4 = 5;
            return;
        }
        func_800A3938(0x78, 0x80); /* reject */
    }
}

/**
 * @brief Reset the overlay's run-state globals to their per-session defaults.
 *
 * Called from the boot path @ref gname_init. Zeros most counters/indices,
 * primes the lerp scalar (@c g_strip_width_steps = 5), seeds the cursor state
 * (@c D_8014F88C / @c D_8014F890 from frozen defaults @c D_8014F894 /
 * @c D_8014F89C), kicks @ref func_80140AB8 to compute initial @c D_8014F8AC,
 * and registers the overlay's per-character buffer with
 * @ref name_copy (`g_active_name`, `&D_8014F7E8`).
 *
 * @see https://decomp.me/scratch/FboaU (100%)
 */
void reset_run_state(void)
{
    D_8014F888 = 0xFF;
    D_8014F8AC = func_80140AB8(0, 0);
    D_8014F884 = 0;
    D_8014F8B4 = 0;
    D_8014F8C0 = 0;
    D_8014F8C4 = 0;
    D_8014F8D0 = 0;
    D_8014F850 = 0;
    D_8014F88C = D_8014F894;
    D_8014F890 = D_8014F89C;
    name_copy(g_active_name, &D_8014F7E8); /* matches 'la a1, D_8014F7E8' */
    g_strip_width = 0;
    func_80142928();
    g_strip_width_steps = 5;
    D_8014F8B0 = 0;
    D_8014F8B8 = 2;
    D_8014F848 = 0;
}

/**
 * decomp.me (85.93%) https://decomp.me/scratch/JHyuJ
 * WARNING. MIGHT NOT BE FUNCTIONALLY EQUIVALENT YET.
 */
s32 func_80140AB8(s32 arg0, s32 arg1)
{
    /* s-register mapped locals */
    s32 repeat;         /* s0: loop condition, init 0xFF */
    s32 reg_s1;         /* s1: arg0 */
    s32 reg_s2;         /* s2: arg1 */
    u32 reg_s6;         /* s6: (u32)(&D_80142F04) - 0x10 */
    const s32 five = 5; /* s7 */
    const s32 four = 4; /* s8 */

    /* Other locals */
    u8* new_var14;
    const s32 new_var5;
    s32 var_a0;
    void* var_a1;
    s32 temp_a0;
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v1;
    u32 new_var;
    s32 temp_v1_2;
    int new_var10;
    s32 temp_v1_3;
    s32 temp_v1_5;
    s32 var_v0;
    s32 var_v0_2;
    s32 var_v0_3;
    int new_var8;
    s32 var_v0_4;
    s32 var_v0_5;
    int new_var3;
    s32 var_v0_6;
    s32 var_v0_7;
    u8* new_var9;
    const s32 new_var2;
    s32 var_v0_8;
    u8* new_var13;
    s32 var_v1;
    void* temp_v1_4;
    u8* f00;
    u8* ef8;
    u32 f00_addr;
    u32 base_f04_addr;
    u8* new_var11;
    int new_var7;

    /* Prologue: initialize register-mapped locals */
    new_var14 = &D_8014F7B0;
    new_var11 = &D_8014F7E8;
    reg_s1 = arg0;
    reg_s2 = arg1;
    repeat = 0xFF;
    reg_s6 = (u32)(&D_80142F04) - 0x10;

    while (repeat == 0xFF)
    {
        if (reg_s1 < 0)
            goto block_62;
        if (reg_s1 < 4)
            goto s1_lt4_block;
        if (reg_s1 >= 8)
            goto block_62;
        else
            goto s14to8_block;
    s1_lt4_block:
        if (reg_s2 & 0x220)
        {
            D_8014F888 = reg_s1;
            if (reg_s1 != 1)
            {
                if (reg_s1 < 2)
                {
                    if (reg_s1 != 0)
                    {
                        repeat = 0;
                        continue;
                    }
                }
                else if (reg_s1 == 2)
                {
                    goto s1_eq2_code;
                }
                else if (reg_s1 == 3)
                {
                    goto s1_eq3_code;
                }
                else
                {
                    repeat = 0;
                    continue;
                }

                if ((name_char_count(g_active_name) != 0) && (name_is_blank(g_active_name) == 0))
                {
                    func_800A3938(0x7E, 0x80);
                    D_8014F7E4 = five;
                }
                else
                {
                    func_800A3938(0x78, 0x80);
                }

                repeat = 0;
                continue;
            }
            else
            {
                func_800A3938(0x7E, 0x80);
                name_pop_last_char(g_active_name);
                goto block_38;
            }

        s1_eq2_code:
            func_800A3938(0x7E, 0x80);
            if (D_8014F7E0 == four)
            {
                u16 half4;
                D_8014F850 = 0;
                base_f04_addr = func_80016F5C();
                var_v0_3 = base_f04_addr;
                temp_v1_2 = var_v0_3;
                if (temp_v1_2 < 0)
                {
                    var_v0_3 = temp_v1_2 + 0x7F;
                }
                {
                    u32 rem4 = temp_v1_2 - ((var_v0_3 >> 7) << 7);
                    half4 = *((u16*)((D_80142F04 + (rem4 * 2)) + reg_s6));
                    var_a1 = (void*)((D_80142F04 + half4) + reg_s6);
                }
                goto block_37;
            }
            else if (D_8014F7E0 == five)
            {
                u16 half5;
                u32 addr_f04;
                D_8014F850 = 0;
                var_v0_4 = func_80016F5C();
                temp_v1_3 = var_v0_4;
                if (temp_v1_3 < 0)
                {
                    var_v0_4 = temp_v1_3 + 0x7F;
                }
                {
                    u32 rem5 = temp_v1_3 - ((var_v0_4 >> 7) << 7);
                    addr_f04 = (u32)(&D_80142F04);
                    half5 = *((u16*)((((u32)D_80142F04) + (addr_f04 + (rem5 * 2))) + 0xF0));
                    var_a1 = (void*)((D_80142F04 + half5) + reg_s6);
                }
                goto block_37;
            }
            else if (D_8014F7E0 == 3)
            {
                u32 idx3;
                unsigned short half3a;
                u16 half3b;
                u32 rem3;
                D_8014F850 = 0;
                idx3 = D_8014F83C;
                new_var8 = idx3;
                if (new_var8 >= 0x81)
                {
                    var_a1 = &D_8014F7E8;
                    goto block_37;
                }
                new_var3 = new_var8 * 2;
                f00 = D_80142F00;
                f00_addr = (u32)(&D_80142F00);
                half3a = *((u16*)((f00 + new_var3) + reg_s6));
                name_copy(g_active_name, (void*)((f00 + half3a) + reg_s6));
                temp_v0 = func_80016F5C();
                var_v0_5 = temp_v0 >> 7;
                f00_addr = (u32)(&D_80142F00);
                if (temp_v0 < 0)
                {
                    var_v0_5 = ((s32)((((((temp_v0 & 0xFFFFFFFFFFFFFFFF) & 0xFFFFFFFFFFFFFFFF) & 0xFFFFFFFFFFFFFFFF) &
                                        0xFFFFFFFFFFFFFFFF) &
                                       0xFFFFFFFFFFFFFFFF) +
                                      0x7F)) >>
                               7;
                }
                rem3 = temp_v0 - (var_v0_5 << 7);
                var_v0_3 = rem3;
                f00 = D_80142F00;
                half3b = *((u16*)(((f00_addr + (var_v0_3 * 2)) + ((u32)f00)) + 0xF4));
                name_append(g_active_name, (void*)((f00 + half3b) + reg_s6), (s32)(new_var13 = f00));
                goto block_38;
            }
            else if (D_8014F7E0 == 1)
            {
                var_a1 = new_var14;
                goto block_36;
            }
        s1_eq3_code:
            func_800A3938(0x7E, 0x80);
            var_a1 = new_var11;
        block_36:
            D_8014F850 = 0;
        block_37:
            name_copy(g_active_name, var_a1);
        block_38:
            func_80142928();
            repeat = 0;
            g_strip_width_steps = five;
            repeat = 0;
            continue;
        }
        else
        {
            if (reg_s2 == 0)
                goto block_61;
            if (reg_s2 & 0x4000)
                goto block_51;
            if (reg_s2 & 0x8000)
            {
                var_v0 = 3;
                if (reg_s1 != 0)
                    var_v0 = reg_s1 - 1;
                goto block_55;
            }
            if (!(reg_s2 & 0x2000))
                goto block_61;
            var_v1 = 0;
            var_v0_2 = reg_s1 < 3;
            goto block_58;
        }
        goto end_if_s1_ge0;
    s14to8_block:
        if ((reg_s2 & 0x220) && ((D_8014F888 = reg_s1, temp_v1 = reg_s1 - 4, D_8014F848 != temp_v1)))
        {
            reg_s1 = 0x10;
            reg_s2 = 0;
            D_8014F8C0 = 0;
            D_8014F8B4 = 0;
            D_8014F848 = temp_v1;
            D_8014F8C4 = 0;
            D_8014F8D0 = 0;
            func_800A3938(0x7E, 0x80);
            continue;
        }
        else
        {
            if (reg_s2 != 0)
            {
                if (reg_s2 & 0x2000)
                {
                block_51:
                    reg_s1 = 0x10;
                    reg_s2 = 0;
                    continue;
                }
                if (reg_s2 & 0x1000)
                {
                    reg_s1 = (reg_s1 == four) ? 6 : reg_s1 - 1;
                    goto block_61;
                }
                if (reg_s2 & 0x4000)
                {
                    var_v1 = 4;
                    var_v0_2 = reg_s1 < 6;
                    goto block_58;
                }
            }
            goto block_61;
        }
        goto end_if_s1_ge0;
    block_55:
        reg_s1 = var_v0;
        goto block_61;
    block_58:
        if (var_v0_2 != 0)
            var_v1 = reg_s1 + 1;
        reg_s1 = var_v1;
    block_61:
        func_800A3938(0x7D, 0x80);
        temp_v1_4 = (void*)(((reg_s1 + 2) * 4) + ((u32)(&D_80142E0C)));
        repeat = 0;
        D_8014F894 = ((*((u32*)temp_v1_4)) & 0x1FF) - 8;
        new_var8 = (s32)(*((u8*)((u32)temp_v1_4 + 2)));
        D_8014F884 = five;
        D_8014F89C = new_var8;
        repeat = 0;
        continue;
    end_if_s1_ge0:;
    block_62:
        if ((reg_s2 & 0x220) && (((D_8014F8A0 * 0xA) + D_8014F898) >= D_8014F8D0))
        {
            if (D_8014F848 < 3)
            {
                if (name_char_count(g_active_name) < 0xA)
                {
                    u32 t1;
                    u16 hw;
                    D_8014F8B8 = 2;
                    {
                        u32 idx_lt3 = D_8014F8D0;
                        ef8 = D_80142EF8;
                        t1 = *((u32*)((D_8014F848 * 4) + ((u32)(&D_80142C98))));
                        hw = *((u16*)(((ef8 + (t1 * 2)) + (idx_lt3 * 2)) + reg_s6));
                        D_8014F8B0 = 0;
                        name_append(g_active_name, (void*)((D_80142EF8 + hw) + reg_s6), (s32)ef8);
                    }
                    func_80142928();
                    g_strip_width_steps = five;
                    func_800A3938(0x7D, 0x80);
                    repeat = 0;
                    continue;
                }
                else
                {
                    goto block_73;
                }
            }
            else if (D_8014F848 == 3)
            {
                u16 off;
                if ((*((u32*)((D_8014F8D0 * 4) + ((u32)(&D_80142CAC))))) != 0xFF)
                {
                    var_a0 = 0x7E;
                    D_8014F8C8 = D_8014F8D0;
                    D_8014F8C0 = 0;
                    D_8014F8B4 = 0;
                    D_8014F8C4 = 0;
                    D_8014F848 = four;
                    D_8014F894 = 0x54;
                    D_8014F89C = 0x68;
                    D_8014F884 = four;
                    temp_v1_5 = D_8014F8D0 * 2;
                    D_8014F8D0 = 0;
                    {
                        ef8 = D_80142EF8;
                        off = *((u16*)(((ef8 + (D_80142CA4 * 2)) + temp_v1_5) + reg_s6));
                        D_8014F84C = (void*)((ef8 + off) + reg_s6);
                    }
                    repeat = 0;
                    func_800A3938(var_a0, 0x80);
                }
                else
                {
                    repeat = 0;
                    continue;
                }
            }
            else if (D_8014F848 == 4)
            {
                if (name_char_count(g_active_name) < 0xA)
                {
                    u32 t1;
                    u32 t2;
                    u16 hw;
                    u8* efc;
                    new_var9 = D_80142EFC;
                    D_8014F8B8 = 2;
                    D_8014F8B0 = 0;
                    {
                        u32 idx4 = D_8014F8C8;
                        efc = new_var9;
                        t1 = *((u32*)((idx4 * 4) + ((u32)(&D_80142CAC))));
                        t2 = *((u32*)((t1 * 4) + ((u32)(&D_80142E40))));
                        hw = *((u16*)(((efc + (t2 * 2)) + (D_8014F8D0 * 2)) + reg_s6));
                        name_append(g_active_name, (void*)((efc + hw) + reg_s6), (s32)efc);
                    }
                    func_80142928();
                    g_strip_width_steps = five;
                    var_a0 = 0x7D;
                }
                else
                {
                block_73:
                    var_a0 = 0x78;
                }
                repeat = 0;
                func_800A3938(var_a0, 0x80);
            }
            else
            {
                repeat = 0;
                continue;
            }
        }
        else
        {
            if (reg_s2 != 0)
            {
                var_v0_6 = reg_s2 & 0x8000;
                if ((reg_s2 & 0x1000) && ((D_8014F8D0 / 10) == (((s32)D_8014F8D0) >> 0x1F)))
                {
                    reg_s1 = 0;
                    reg_s2 = 0;
                    continue;
                }
                var_v0_7 = reg_s2 & 0x1000;
                if (var_v0_6 != 0 && D_8014F8D0 == ((D_8014F8D0 / 10) * 0xA))
                {
                    reg_s1 = 4;
                    reg_s2 = 0;
                    continue;
                }
                if (var_v0_7 != 0 && (D_8014F8D0 / 10) != (((s32)D_8014F8D0) >> 0x1F))
                {
                    D_8014F8D0 = D_8014F8D0 - 0xA;
                }
                else if ((reg_s2 & 0x4000) && (D_8014F8D0 / 10) != D_8014F8A0)
                {
                    D_8014F8D0 = D_8014F8D0 + 0xA;
                }
                else if ((reg_s2 & 0x8000) && D_8014F8D0 != (D_8014F8D0 / 10) * 0xA)
                {
                    D_8014F8D0 = D_8014F8D0 - 1;
                }
                else if ((reg_s2 & 0x2000) && ((D_8014F8D0 / 10) * 0xA) != (D_8014F8D0 - 9))
                {
                    D_8014F8D0 = D_8014F8D0 + 1;
                }
                else
                {
                    repeat = 0;
                    continue;
                }
            }

            func_800A3938(0x7D, 0x80);

            D_8014F894 = ((D_8014F8D0 % 10) * 0x10) + 0x54;
            new_var10 = 4 * (4 * (D_8014F8D0 / 10));
            temp_a0 = new_var10;
            new_var7 = D_8014F8B4 - 0x68;
            temp_v0_2 = temp_a0 - new_var7;
            D_8014F89C = temp_v0_2;
            if (temp_v0_2 < 0x68)
            {
                D_8014F89C = 0x68;
                D_8014F8C0 = temp_a0;
                D_8014F8C4 = four;
            }
            if (D_8014F89C >= 0xA9)
            {
                D_8014F89C = 0xA8;
                D_8014F8C0 = temp_a0 - 0x40;
                D_8014F8C4 = four;
            }
            D_8014F884 = four;
            repeat = 0;
        }
    }
    return reg_s1;
}

/**
 * decomp.me (96.22%) https://decomp.me/scratch/ctu1w
 */
void func_8014139C(void)
{
    s8 sp10;
    s8 sp11;
    s8 sp12;
    s32 var_a0;
    s32 temp_a0;
    u8(*new_var2)[];
    s32 temp_a0_2;
    s32 temp_a1;
    s32 temp_v1;
    s32 temp_a1_2;
    s32 temp_s1;
    s32 temp_v0_3;
    s32* new_var;
    s32 temp_v1_2;
    s32 temp_v1_3;
    s32 var_v0;
    s8 temp_v0;
    u16 temp_v0_2;
    u8* base;
    u32 idx;
    u32 offset;
    u16 tmp;
    s32 original;
    s32 temp_c98;
    int new_var3;
    u8* ptr;
    u16 val;
    temp_a1 = D_80122988 & 0xF220;
    D_8014F888 = 0xFF;
    if (temp_a1 != 0)
    {
        D_8014F8AC = func_80140AB8(D_8014F8AC, temp_a1);
    }
    else if (D_80122988 & 1)
    {
        temp_s1 = name_pop_last_char(g_active_name);
        while (name_char_count(&D_8014F850) >= 0xB)
        {
            name_pop_last_char(&D_8014F850);
        }

        name_prepend_char(&D_8014F850, temp_s1 & 0xFFFF);
        func_80142928();
        g_strip_width_steps = 5;
        var_a0 = 0x7D;
        new_var3 = 0x80;
        func_800A3938(var_a0, new_var3);
    }
    else if (D_80122988 & 2)
    {
        if (name_char_count(g_active_name) < 0xA)
        {
            new_var2 = &D_8014F850;
            temp_v0 = name_pop_first_char(new_var2);
            temp_v0_2 = (u16)temp_v0;
            if (temp_v0_2 != 0)
            {
                sp10 = temp_v0;
                (&sp10)[1] = (s8)(temp_v0_2 >> 8);
                (&sp10)[2] = 0;
                name_append(g_active_name, &sp10);
                func_80142928();
                g_strip_width_steps = 5;
            }
            var_a0 = 0x7D;
            func_800A3938(var_a0, 0x80);
        }
        else
        {
            func_800A3938(0x78, 0x80);
        }
    }
    else if (D_80122988 & 0x40)
    {
        if (D_8014F838 != 0)
        {
            if (name_char_count(g_active_name) == 0)
            {
                D_8014F7E4 = 2;
                func_800A3938(0x7F, 0x80);
                return;
                if (!D_8014F894)
                {
                }
            }
        }
        func_800A3938(0x7F, 0x80);
        name_pop_last_char(g_active_name);
        func_80142928();
        g_strip_width_steps = 5;
    }
    if (((D_8014F8AC == 0x10) && (D_8014F848 == 4)) && (D_80122988 & 0xC))
    {
        func_800A3938(0x7D, 0x80);
        if (D_80122988 & 0xC)
        {
            do
            {
                if (D_80122988 & 4)
                {
                    temp_a0 = D_8014F8C8;
                    temp_v1 = temp_a0 - 0xA;
                    D_8014F8C8 = temp_v1;
                    if (temp_v1 == (-1))
                    {
                        D_8014F8C8 = 0;
                    }
                    else if (temp_v1 < 0)
                    {
                        D_8014F8C8 = temp_a0 + 0x29;
                    }
                }
                else
                {
                    original = D_8014F8C8;
                    temp_v1_2 = original + 10;
                    D_8014F8C8 = temp_v1_2;
                    if (temp_v1_2 == 0x32)
                    {
                        D_8014F8C8 = 9;
                    }
                    else if (temp_v1_2 >= 0x32)
                    {
                        D_8014F8C8 = original - 0x29;
                    }
                }
                offset = D_8014F8C8;
                if (D_80142CAC[offset] != 0xFF)
                {
                    D_8014F8C0 = (long)0;
                    D_8014F8B4 = 0;
                    var_v0 = D_80142C98[3];
                    D_8014F8C4 = 0;
                    D_8014F8D0 = 0;
                    D_8014F894 = 0x54;
                    D_8014F89C = 0x68;
                    D_8014F884 = 4;
                    temp_c98 = var_v0;
                    base = D_80142EF4;
                    idx = D_8014F8C8;
                    offset = (idx * 2) + ((temp_c98 * 2) + D_80142EF8);
                    tmp = *((u16*)(base + offset));
                    D_80122988 &= ~0xC;
                    D_8014F84C = (void*)((D_80142EF8 + tmp) + ((unsigned long)base));
                }
            } while (D_80122988 & 0xC);
        }
    }
    if (D_8014F884 != 0)
    {
        temp_a1_2 = ((s32)(D_8014F894 - D_8014F88C)) / ((s32)D_8014F884);
        temp_v1_3 = ((s32)(D_8014F89C - D_8014F890)) / ((s32)D_8014F884);
        D_8014F884 -= 1;
        D_8014F88C += temp_a1_2;
        D_8014F890 += temp_v1_3;
    }
    else
    {
        D_8014F88C = D_8014F894;
        D_8014F890 = D_8014F89C;
    }
    if (D_8014F8C4 != 0)
    {
        new_var = &D_8014F8B4;
        temp_v0_3 = ((s32)(D_8014F8C0 - (*new_var))) / ((s32)D_8014F8C4);
        D_8014F8C4 -= 1;
        D_8014F8B4 += temp_v0_3;
        return;
    }
    D_8014F8B4 = D_8014F8C0;
}

/**
 * decomp.me (83.21%) https://decomp.me/scratch/oXGkF
 */
void* func_80141848(void* arg0, s32* arg1, s16 arg2, s16 arg3)
{
    unsigned char* bp = (unsigned char*)arg0;
    unsigned int mask_lo;
    unsigned int mask_hi;
    unsigned char* D = g_glyph_table;
    unsigned int t0;
    unsigned int w0;
    unsigned int w1;
    *((unsigned int*)(bp + 4)) = 0x00808080U;
    bp[3] = 4;
    bp[7] = 0x64;
    *((s16*)(bp + 8)) = arg2;
    *((s16*)(bp + 10)) = arg3;
    bp[12] = D[0xa0];
    mask_lo = 0x00ffffff;
    bp[13] = D[0xa1];
    *((s16*)(bp + 16)) = (s16)D[0xa2];
    t0 = 0xe1000000;
    *((s16*)(bp + 18)) = (s16)D[0xa3];
    mask_hi = 0xff000000;
    w0 = *((unsigned int*)(&D[0xa4]));
    w1 = *((unsigned int*)bp);
    w0 = (w0 & 0x3f) | 0x7c80;
    *((s16*)(bp + 14)) = (s16)w0;
    w0 = *((unsigned int*)arg1);
    w1 = w1 & mask_hi;
    w0 = w0 & mask_lo;
    *((unsigned int*)bp) = w1 | w0;
    w1 = ((unsigned int)bp) & mask_lo;
    w0 = *((unsigned int*)arg1);
    bp += 0x14;
    w0 = w0 & mask_hi;
    *((unsigned int*)arg1) = w0 | w1;
    bp[3] = 1;
    w1 = *((unsigned int*)bp);
    t0 |= 5;
    *((unsigned int*)(bp + 4)) = t0;
    w0 = *((unsigned int*)arg1);
    w1 = w1 & mask_hi;
    w0 = w0 & mask_lo;
    *((unsigned int*)bp) = w1 | w0;
    w0 = *((unsigned int*)arg1);
    w1 = ((unsigned int)bp) & mask_lo;
    w0 = w0 & mask_hi;
    *((unsigned int*)arg1) = w0 | w1;
    return (void*)(bp + 8);
}

/**
 * decomp.me (82.68%) https://decomp.me/scratch/rQBi6
 */
void func_80141928(void* arg0)
{
    s32 new_var;
    s32 var_s0;
    s32 var_v1;
    s32* var_s2;
    void* var_t0;
    char* new_var2;
    void* temp_v0;
    void* var_t0_2;
    void* new_var4;
    unsigned char* var_s1;
    char* new_var3;
    var_s2 = &D_80142E14;
    var_s0 = 2;
    var_s1 = ((unsigned char*)(&D_80142E14)) + 2;
    var_t0 = *((void**)(((char*)arg0) + 0x4040));
    do
    {
        if (var_s0 != 9)
        {
            var_t0 = func_80142274(var_t0, ((char*)arg0) + 0x2C, var_s1[1], (*var_s2) & 0x1FF, ((s32)var_s1[0]) - 8, 1,
                                   (var_s0 - 2) == D_8014F888, 0);
        }
        var_s0 += 1;
        var_s1 += 4;
        var_s2++;
    } while (var_s0 < 0xD);
    new_var = D_8014F88C;
    new_var4 = arg0;
    temp_v0 = func_80141C34(
        emit_draw_mode_prim(func_80142B18(func_80142274(emit_draw_mode_prim(var_t0, ((char*)new_var4) + 0x2C),
                                                        ((char*)new_var4) + 0x34, 3U, 0xE8, 4, 0, 0, 0),
                                          arg0),
                            ((char*)new_var4) + 0x34),
        arg0);
    new_var3 = ((char*)temp_v0) + 0x14;
    *((s32*)(((char*)temp_v0) + 4)) = 0x808080;
    *((u8*)(((char*)temp_v0) + 7)) = 0x64;
    var_t0_2 = ((char*)temp_v0) + 0x1C;
    *((u8*)(((char*)temp_v0) + 3)) = 4;
    *((s16*)(((char*)temp_v0) + 8)) = new_var;
    {
        s32 tmp = D_8014F890;
        *((s16*)(((char*)temp_v0) + 10)) = tmp;
    }
    *((u8*)(((char*)temp_v0) + 12)) = g_glyph_table[0xA0];
    *((u8*)(((char*)temp_v0) + 13)) = g_glyph_table[0xA1];
    *((s16*)(((char*)temp_v0) + 16)) = (s16)g_glyph_table[0xA2];
    *((s16*)(((char*)temp_v0) + 18)) = (s16)g_glyph_table[0xA3];
    {
        u32 tmp = *((u32*)(g_glyph_table + 0xA4));
        *((s16*)(((char*)temp_v0) + 14)) = (s16)((tmp & 0x3F) | 0x7C80);
    }
    new_var2 = (char*)new_var4;
    {
        s32 old = *((s32*)temp_v0);
        *((s32*)temp_v0) = (old & 0xFF000000) | ((*((s32*)(new_var2 + 0x20))) & 0xFFFFFF);
    }
    *((s32*)(new_var2 + 0x20)) = ((*((s32*)(new_var2 + 0x20))) & 0xFF000000) | (((s32)temp_v0) & 0xFFFFFF);
    {
        void* temp_a0 = ((char*)temp_v0) + 0x14;
        *((u8*)(((char*)temp_a0) + 3)) = 1;
        *((u32*)(((char*)temp_a0) + 4)) = 0xE1000005;
        *((s32*)new_var3) = ((*((s32*)new_var3)) & 0xFF000000) | ((*((s32*)(new_var2 + 0x20))) & 0xFFFFFF);
        var_v1 = D_8014F8B4;
        *((s32*)(new_var2 + 0x20)) = ((*((s32*)(new_var2 + 0x20))) & 0xFF000000) | (((s32)temp_a0) & 0xFFFFFF);
    }
    new_var3 = new_var2 + 0x4040;
    if (D_8014F8B4 != 0)
    {
        var_t0_2 = func_80142274(var_t0_2, arg0, D_80142E0C[3], (*((s32*)(D_80142E0C + 0))) & 0x1FF, (s32)D_80142E0C[2],
                                 0, 0, 0);
    }
    if (D_8014F8A0 >= 5)
    {
        var_v1 = D_8014F8B4;
        if (var_v1 < 0)
        {
            var_v1 += 0xF;
        }
        if ((((var_v1 >> 1) >> 1) >> 2) != (D_8014F8A0 - 4))
        {
            var_t0_2 = func_80142274(var_t0_2, arg0, D_80142E0C[7], (*((s32*)(D_80142E0C + 4))) & 0x1FF,
                                     (s32)D_80142E0C[6], 0, 0, 0);
        }
    }
    *((void**)new_var3) = func_80141D64(emit_draw_mode_prim(var_t0_2, arg0), new_var2 + 0x24);
    func_80141F9C(arg0, D_8014F848);
    func_80141E04(new_var4, g_active_name, g_strip_width);
}

/**
 * decomp.me (62.11%) https://decomp.me/scratch/Yf7Ha
 */
s32 func_80141C34(s32 arg0, s32 arg1)
{
    s32 f8ac;
    s32 var_a0;
    u16 val16;
    u32* entry;
    u32 temp;
    u8* base;
    s32 var;
    s32 f848;
    void* ptr;

    f8ac = D_8014F8AC;
    var_a0 = arg0;

    if (f8ac < 8)
    {
        entry = (u32*)((u32)&D_80142EF8 + ((f8ac + 2) * 4));
        temp = *entry;
        base = ((u8*)&D_80142EF8) - 4;
        var = D_80142EF8; /* value, not address */
        val16 = *(u16*)(base + ((temp >> 8) & 0xFE) + var);
        ptr = base + val16 + var;
        var_a0 = func_800A88A0(var_a0, arg1, ptr, 1, 0xb0, 0xc8, 2);
    }
    else if (f8ac == 0x10)
    {
        f848 = D_8014F848;
        if (((u32)(f848 - 3)) < 2U)
        {
            base = ((u8*)&D_80142EF8) - 4;
            var = D_80142EF8;
            /* val16 from &D_80142EF8 + (f848*4) + 0x10 */
            val16 = *(u16*)((u8*)&D_80142EF8 + (f848 * 4) + 0x10);
            ptr = base + val16 + var;
            var_a0 = func_800A88A0(var_a0, arg1, ptr, 1, 0xb0, 0xc8, 2);
        }
        else
        {
            base = ((u8*)&D_80142EF8) - 4;
            /* val16 from &D_80142EF8 + (arg0*4) + 0x50 */
            val16 = *(u16*)((u8*)&D_80142EF8 + (arg0 * 4) + 0x50);
            ptr = base + val16 + (arg0 * 4);
            var_a0 = func_800A88A0(var_a0, arg1, ptr, 1, 0xb0, 0xc8, 2);
        }
    }

    return var_a0;
}

/**
 * decomp.me (43.25%) https://decomp.me/scratch/0GHRZ
 */
s32 func_80141D64(void)
{
    u32 n;
    void* var_a2;

    n = D_8014F848;
    if (n < 4)
    {
        u32 t0;
        char* v1; /* pointer arithmetic, no constant folding */
        u16 h;

        t0 = D_80142EF8;
        v1 = (char*)&D_80142EF8 - 4;    /* two addiu instructions */
        h = *(u16*)(v1 + (2 * n + t0)); /* lhu from (v1 + 2n + t0) */
        var_a2 = (void*)(t0 + (h + (u32)v1));
    }
    else
    {
        var_a2 = D_8014F84C;
    }

    return func_800A88A0(var_a2, 1, 0x23, 0x47, 2);
}

/**
 * @brief Append three GPU primitives to the render context's OT and reserve a
 *        right-edge VRAM strip for upload on the back page.
 *
 * Builds, in order:
 *   1. A 0x40-byte template packet copied from the inactive frame's reserve
 *      slot at `D_8014F840 + (alt_buf * 0x40C0) + 0x4064`.
 *   2. A textured sprite (tag 0x64) emitted via @ref func_800A88A0 using
 *      `tex_src` as its source data, then a Draw-Mode (GP0 0xE1) packet
 *      emitted via @ref emit_draw_mode_prim / @ref func_80142274.
 *   3. A 0x60-byte image-load packet built on the stack by
 *      @ref func_8001C56C describing a `strip_width x 32` rectangle at VRAM
 *      `(240 - strip_width, 24 | 256)` — i.e. right-aligned on whichever
 *      VRAM page is currently the back buffer (Y=0x18 vs 0x100).
 *
 * Each packet is spliced into the 24-bit OT at @c ctx->unk38 with the
 * standard `(top_byte | next_addr & 0xFFFFFF)` link idiom, and the heap
 * cursor @c ctx->unk4040 is advanced by 0x40 bytes past the last packet.
 *
 * @param ctx         Render context: OT head at unk38, primitive heap cursor
 *                    at unk4040, double-buffer parity at unk404C.
 * @param tex_src     Source data pointer for the sprite primitive (passed
 *                    through to func_800A88A0).
 * @param strip_width Width in pixels of the back-page VRAM upload strip; also
 *                    sets the strip's X position as `240 - strip_width`.
 *
 * @see https://decomp.me/scratch/LxujJ (99.26%)
 */
void func_80141E04(UnkStruct* ctx, s32 tex_src, s32 strip_width)
{
    s32* ot_head;   /* &ctx->unk38 — passed as the OT head pointer */
    s32* prim;      /* current primitive being emitted */
    s32* next_prim; /* heap cursor after the sprite/draw-mode pair */
    s32 vram_y;     /* VRAM Y of the back page (0x18 or 0x100) */
    u32 load_packet[0x19];
    s32 vram_x; /* VRAM X of the right-aligned strip */

    ot_head = (s32*)&ctx->unk38;
    prim = ctx->unk4040;
    next_prim = prim;

    /* 1. Copy template packet from the *other* frame's reserve slot, then
     *    splice it into the OT. */
    func_8001A5D4(prim, (void*)(D_8014F840 + ((ctx->unk404C ^ 1) * 0x40C0) + 0x4064));

    *prim = (*prim & 0xFF000000) | (ctx->unk38 & 0xFFFFFF);
    ctx->unk38 = (ctx->unk38 & 0xFF000000) | ((u32)prim & 0xFFFFFF);

    /* 2. Emit textured sprite (tag 0x64) wrapped by a Draw-Mode (0xE1) packet.
     *    Returns the heap cursor just past both packets. */
    next_prim = emit_draw_mode_prim(
        func_80142274(func_800A88A0(prim + 0x10, ot_head, tex_src, 1, 0x10, 8, 0), ot_head, 2, 0, 0, 0, 0, 0), ot_head);

    /* 3. Build a back-page VRAM upload RECT (W = strip_width, H = 32) at the
     *    right edge of whichever page is currently the back buffer. */
    vram_x = 0xF0 - strip_width;
    vram_y = 0x18;
    if (ctx->unk404C != 0)
    {
        vram_y = 0x100;
    }

    func_8001C56C(load_packet, vram_x, vram_y, strip_width, 0x20);
    func_8001A5D4(next_prim, load_packet);

    *next_prim = (*next_prim & 0xFF000000) | (ctx->unk38 & 0xFFFFFF);

    /* Advance heap cursor 0x40 bytes past the load packet. */
    ctx->unk4040 = next_prim + 0x10;

    ctx->unk38 = (ctx->unk38 & 0xFF000000) | ((u32)next_prim & 0xFFFFFF);
}

/**
 * decomp.me (77.64%) https://decomp.me/scratch/Glw7t
 */
void func_80141F9C(void* arg0, s32 arg1)
{
    u8 sp28[0x80];
    Obj* obj = (Obj*)arg0;
    u32* temp_s1 = obj->unk4040;
    u32* var_a2;
    u8* var_s4;
    u16* var_s3;
    s32 var_s0, var_s1, var_s2, var_s5;
    s32 temp_v1;

    /* First call and pointer setup */
    func_8001A5D4(temp_s1, (void*)(D_8014F840 + ((obj->unk404C ^ 1) * 0x40C0) + 0x4064));

    *temp_s1 = (*temp_s1 & 0xFF000000) | (obj->unk28 & 0x00FFFFFF);
    obj->unk28 = (obj->unk28 & 0xFF000000) | ((u32)temp_s1 & 0x00FFFFFF);

    /* var_a2 = temp_s1 + 0x40 (byte addition) */
    var_a2 = (u32*)((u8*)temp_s1 + 0x40);

    /* Determine var_s4, var_s1, var_s5, var_s2 based on D_8014F848 */
    if (D_8014F848 == 4)
    {
        var_s4 = (u8*)(D_80142EFC + ((u32)&D_80142EFC - 8));
        var_s1 = D_80142E40[D_80142CAC[D_8014F8C8]];
        var_s5 = D_80142E40[D_80142CAC[D_8014F8C8] + 1];
        var_s2 = 0;
    }
    else
    {
        var_s1 = D_80142C98[arg1][0];
        var_s5 = D_80142C98[arg1][1];
        var_s4 = (u8*)(D_80142EF8 + ((u32)&D_80142EF8 - 4));
        var_s2 = 0;
    }

    var_s0 = var_s2;
    var_s3 = (u16*)(var_s4 + (var_s1 * 2));

    /* Main loop */
    for (;;)
    {
        temp_v1 = (var_s2 * 0x10) - D_8014F8B4;
        if ((u32)(temp_v1 + 0x0B) < 0x5B)
        {
            var_a2 = func_800A88A0(var_a2, (void*)&obj->unk28, (void*)(var_s4 + *var_s3), 1, var_s0 * 0x10, temp_v1, 0);
        }
        var_s1++;
        var_s3++;
        if (var_s5 == var_s1)
            break;

        var_s0++;
        if (var_s0 == 10)
        {
            var_s0 = 0;
            var_s2++;
        }
    }

    /* After loop – final setup and second call */
    {
        u32 param_a2 = 0x68;
        D_8014F8A0 = var_s2;
        D_8014F898 = var_s0;
        if (obj->unk404C != 0)
        {
            param_a2 = 0x150;
        }

        func_8001C56C(sp28, 0x60, param_a2, 0xA0, 0x50);
        func_8001A5D4(var_a2, sp28);

        *var_a2 = (*var_a2 & 0xFF000000) | (obj->unk28 & 0x00FFFFFF);
        obj->unk28 = (obj->unk28 & 0xFF000000) | ((u32)var_a2 & 0x00FFFFFF);
        /* Byte addition of 0x40, not element addition */
        obj->unk4040 = (u32*)((u8*)var_a2 + 0x40);
    }
}

/**
 * @brief Emit a Draw-Mode (GP0 0xE1) primitive and link it to the OT.
 *
 * Writes an 8-byte packet at @p prim:
 *  - byte 3 = 1 (one-word payload).
 *  - bytes 4..7 = `0xE1000005` (GP0 Draw Mode: texpage default, abr=1,
 *    dither off, drawing-to-display-area enabled).
 * Then splices the packet into the 24-bit OT whose head is at @p ot_head
 * using the standard `(top_byte | next_addr & 0xFFFFFF)` chain idiom and
 * returns the heap cursor advanced by 8 bytes.
 *
 * @param prim    Destination address for the 8-byte packet (heap cursor).
 * @param ot_head Pointer to the 24-bit OT head pointer.
 * @return Heap cursor advanced past the packet (`prim + 8`).
 *
 * @see https://decomp.me/scratch/EyVeo (100%)
 */
void* emit_draw_mode_prim(void* arg0, s32* arg1)
{
    unsigned char* bytes = (unsigned char*)arg0;
    u32* words = (u32*)arg0;
    u32 temp1, temp2;

    setDrawTPage(bytes, 0, 0, 0x05);
    addPrim(arg1, arg0);

    return (void*)(bytes + 8);
}

/**
 * @brief Emit 1 or 2 glyph SPRT primitives and chain them onto an OT tag.
 *
 * Always writes a primary white-tinted (RGB=0x80) SPRT for glyph @p arg2
 * at @c (arg3 - arg5 + arg6, arg4 - arg5 + arg6). When @p arg5 != 0,
 * writes a second SPRT at @c (arg3 + tmp, arg4 + tmp) with
 * @c tmp = (arg5 - arg6) * 2. The second sprite's tint and code byte
 * depend on @p arg7:
 *   - @p arg7 != 0: opaque blue (RGB=(0,0,0xA0), code 0x64) — highlight.
 *   - @p arg7 == 0: semi-transparent black (RGB=0, code 0x66) — drop shadow.
 *
 * Both sprites pull u/v/w/h/clut from @c g_glyph_table[arg2] and are
 * appended to the linked list at @p arg1 via the standard addPrim sequence.
 *
 * @param arg0 Pointer to the next free byte in the primitive buffer.
 * @param arg1 Pointer to the OT head tag (addPrim "ot" arg).
 * @param arg2 Glyph ID (index into @c g_glyph_table).
 * @param arg3 Base X coordinate.
 * @param arg4 Base Y coordinate.
 * @param arg5 Shadow/highlight offset distance. When 0, only the primary
 *             sprite is emitted (the second SPRT block is skipped).
 * @param arg6 Origin compensation (subtracted from base for the primary
 *             sprite; combined into @c tmp for the secondary).
 * @param arg7 Secondary-sprite mode: non-zero = opaque blue highlight,
 *             zero = semi-transparent black drop shadow.
 * @return Pointer to the byte after the last emitted primitive.
 *
 * @see decomp.me (88.88%) https://decomp.me/scratch/UHlWz
 */
void* func_80142274(void* arg0, s32* arg1, u8 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, s32 arg7)
{
    u8* base = (u8*)arg0;
    u8* ptr = base;
    SPRT* sprt = (SPRT*)ptr;
    /* (offset) + (base) form so gcc emits `addu v1,v1,v0` (vs the reverse
       order from `&g_glyph_table[arg2]`). Also keeps arg2 live for the
       second SPRT's re-derivation below. */
    u8* entry = (u8*)((arg2 << 3) + (u32)g_glyph_table);
    u32 clut_word;
    s32 tmp2;

    /* Primary glyph SPRT — white tint, fully opaque. */
    *(u32*)&sprt->r0 = 0x808080;             /* r=g=b=0x80, code byte = 0 */
    setSprt(sprt);                           /* len=4, code=0x64 (free-size textured sprite) */
    sprt->x0 = (s16)(arg3 - arg5 + arg6);
    sprt->y0 = (s16)(arg4 - arg5 + arg6);
    sprt->u0 = entry[0];
    sprt->v0 = entry[1];
    sprt->w = (s16)entry[2];
    sprt->h = (s16)entry[3];
    /* Force a `lw` load on the clut word — accessing as u32 keeps the
       full word live so gcc doesn't shrink to `lhu`. */
    clut_word = *(u32*)(entry + 4);
    sprt->clut = (u16)((clut_word & 0x3F) | GLYPH_CLUT_PAGE_BITS);

    /* Chain the primary SPRT into the OT linked list. */
    addPrim((u_long*)arg1, (u_long*)arg0);

    ptr += 0x14;

    if (arg5 != 0)
    {
        /* Secondary SPRT — drop shadow (arg7==0) or highlight (arg7!=0).
           entry2 is re-derived (rather than reused) so gcc preserves arg2
           in $a2 across both SPRT blocks, matching the target codegen. */
        SPRT* sprt2 = (SPRT*)ptr;
        u8* entry2 = (u8*)((arg2 << 3) + (u32)g_glyph_table);
        u32 clut_word2;

        *(u32*)&sprt2->r0 = (arg7 != 0) ? 0xA00000 : 0;
        setlen(sprt2, 4);
        /* Default code, then conditionally overlay 0x66 for the
           semi-trans shadow. The "store-default-then-override" form
           matches the target's branch-and-delay-slot peephole; a single
           ternary `(arg7 == 0) ? 0x66 : 0x64` would compile differently. */
        setcode(sprt2, 0x64);
        if (arg7 == 0)
        {
            setcode(sprt2, 0x66);
        }

        tmp2 = (arg5 - arg6) * 2;
        sprt2->y0 = (s16)(arg4 + tmp2);
        sprt2->x0 = (s16)(arg3 + tmp2);
        sprt2->u0 = entry2[0];
        sprt2->v0 = entry2[1];
        sprt2->w = (s16)entry2[2];
        sprt2->h = (s16)entry2[3];
        clut_word2 = *(u32*)(entry2 + 4);
        sprt2->clut = (u16)((clut_word2 & 0x3F) | GLYPH_CLUT_PAGE_BITS);

        /* Chain the secondary SPRT into the OT linked list. */
        addPrim((u_long*)arg1, (u_long*)ptr);

        ptr += 0x14;
    }

    return (void*)ptr;
}

/**
 * @brief Build the name-entry cursor's per-frame sprite packet:
 *        a TexWindow bracket around @ref NAME_CURSOR_GLYPH_COUNT textured
 *        glyph sprites, terminated by a DrawMode.
 *
 * Walks @c D_8014F6B8 (a @ref GlyphSeqEntry array) one entry per glyph
 * cell, looks each glyph up in @c g_glyph_table, and emits a white
 * (RGB=0x80) free-size textured SPRT primitive (code 0x64). The chain is
 * wrapped with @c setTexWindow at both ends (rect @c {0,0,0xFF,0xFF} —
 * a no-op full-page window) and closed with @c setDrawTPage(0,0,5).
 * The buffer cursor at @c obj->unk4040 is advanced past the final primitive.
 *
 * @param arg0 Render context (@ref Obj). Reads/writes:
 *             - @c prim_ot at offset 0x3C — addPrim "ot" head.
 *             - @c unk4040 at offset 0x4040 — primitive scratch-pool cursor.
 *
 * @note Called by @ref gname_tick via the implicit-@c $a0 convention
 *       (the call site passes no args; the function reads whatever the
 *       caller left in @c $a0).
 *
 * @see decomp.me (100%) https://decomp.me/scratch/Q6WL2
 */
void func_80142410(Obj* arg0)
{
    RECT tw_rect;

    s32 i;

    u8* ptr_t1;
    u8* list_ptr;
    DR_TWIN* twin;
    SPRT* sprt;
    u8* drawmode;
    u_long* ptr;
    GlyphSeqEntry* seq;

    /* Two aliases of the same context pointer: gcc allocates them to t6/t2
       and uses t6 for the very first addPrim/buf access and t2 for every
       subsequent addPrim. This split is load-bearing for the asm match. */
    Obj* obj = arg0;
    Obj* obj2;
    u8* glyph_table_base;
    obj2 = obj;

    ptr_t1 = (u8*)obj->unk4040;

    /* First TexWindow init: source order is h, w, y, x. */
    tw_rect.h = 0xFF;
    tw_rect.w = 0xFF;
    tw_rect.y = 0;
    tw_rect.x = 0;

    /* Opening texture window. The first addPrim runs *before* seq/i/glyph_table_base
       are assigned so gcc materializes the 0x00FFFFFF mask at the very top
       of the prologue (the mask is the first non-arg constant used). */
    twin = (DR_TWIN*)ptr_t1;
    setTexWindow(twin, &tw_rect);
    addPrim(&obj->prim_ot, twin);

    seq = D_8014F6B8;
    i = 0;
    glyph_table_base = g_glyph_table;

    ptr_t1 += sizeof(DR_TWIN);

    /* Glyph sprites. list_ptr is a separate variable aliased to ptr_t1 so
       gcc keeps both pointers live across the loop (target uses t1 + a2 in
       parallel). */
    list_ptr = ptr_t1;
    do
    {
        u32 idx = seq->id;
        u32 xy;
        u8* glyph;

        sprt = (SPRT*)list_ptr;
        /* RGB only (high byte lands in `code`, immediately overwritten). */
        *(u32*)&sprt->r0 = 0x808080;
        setlen(sprt, 4);
        setcode(sprt, 0x64);

        xy = seq->xy;
        /* (offset) + (base) order forces gcc to emit `addu v1,v1,s0` (vs.
           the reverse order `s0,v1` you'd get from `&glyph_table[idx]`). */
        glyph = (u8*)((idx << 3) + (u32)glyph_table_base);
        *(u32*)&sprt->x0 = xy;

        sprt->u0 = glyph[0];
        sprt->v0 = glyph[1];
        sprt->w = glyph[2];
        {
            /* `i++` between the load and store of `h` so gcc schedules the
               counter increment into the slot the target asm uses. */
            u8 hh = glyph[3];
            i++;
            sprt->h = hh;
        }
        {
            /* Read clut as a full word (gcc would otherwise optimize this
               to `lhu` since only the low 16 bits affect the result). The
               `seq++` advance sits between read and store to match the
               target's instruction scheduling. */
            u32 clut_word = *(u32*)(glyph + 4);
            seq++;
            sprt->clut = (u16)((clut_word & 0x3F) | GLYPH_CLUT_PAGE_BITS);
        }

        addPrim(&obj2->prim_ot, sprt);
        list_ptr += sizeof(SPRT);
    } while (i < NAME_CURSOR_GLYPH_COUNT);
    ptr_t1 = list_ptr;

    /* Closing texture window. Field-assignment order differs from the
       opening call (w,h,x,y vs. h,w,y,x) — the original C wrote them in
       this exact order and gcc preserves it. */
    tw_rect.w = 0xFF;
    tw_rect.h = 0xFF;
    tw_rect.x = 0;
    tw_rect.y = 0;
    twin = (DR_TWIN*)ptr_t1;
    setTexWindow(twin, &tw_rect);
    addPrim(&obj2->prim_ot, twin);
    ptr_t1 += sizeof(DR_TWIN);

    /* DrawMode terminator: tpage 5, dfe=0, dtd=0. Writes only tag + 1 word.
       The buffer cursor is computed as `drawmode + 8` (not by mutating
       ptr_t1 first), which yields `addiu v0,t1,8; sw v0,0x4040(...)`. */
    drawmode = ptr_t1;
    setDrawTPage(drawmode, 0, 0, 5);
    addPrim(&obj2->prim_ot, drawmode);

    arg0->unk4040 = (u32*)(drawmode + 8);
}

/**
 * @brief Number of bytes in a name buffer, excluding the null terminator.
 *
 * Walks the variable-width encoding: each byte in [0x19, 0x20) consumes
 * two buffer bytes (DBCS lead + trail), every other byte consumes one.
 *
 * @param name Null-terminated name buffer.
 * @return Byte length excluding the terminator.
 * @see https://decomp.me/scratch/2QgjW (100%)
 */
s32 name_byte_length(u8* name)
{
    s32 byte_len;
    u8 c;
    u8* p;

    p = name;
    c = *p;
    byte_len = 0;
    if (c != 0)
    {
        do
        {
            if ((u32)(c - 0x19) < 7U) /* DBCS lead byte: 2-byte glyph */
            {
                p += 2;
                byte_len += 2;
            }
            else
            {
                p += 1;
                byte_len += 1;
            }
            c = *p;
        } while (c != 0);
    }
    return byte_len;
}

/**
 * @brief Number of logical glyphs in a name buffer.
 *
 * Like @ref name_byte_length but counts each DBCS pair as one glyph.
 *
 * @param name Null-terminated name buffer.
 * @return Glyph (character) count.
 * @see https://decomp.me/scratch/c8fPe (100%)
 */
s32 name_char_count(u8* name)
{
    s32 char_count;
    u8 c;
    u8* p;

    p = name;
    c = *p;
    char_count = 0;
    if (c != 0)
    {
        do
        {
            if ((u32)(c - 0x19) < 7U) /* DBCS lead byte: 2-byte glyph */
            {
                p += 2;
            }
            else
            {
                p += 1;
            }
            c = *p;
            char_count += 1;
        } while (c != 0);
    }
    return char_count;
}

/**
 * @brief Append @p src to the end of @p dst (in-place concatenation).
 *
 * Computes the byte lengths of both buffers (respecting the DBCS-style
 * encoding) and copies @p src's payload after @p dst's existing payload,
 * writing a fresh null terminator. Caller is responsible for ensuring
 * @p dst has room for both.
 *
 * @param dst Null-terminated name buffer; appended to in-place.
 * @param src Null-terminated source name to append.
 * @see https://decomp.me/scratch/1lsbD (100%)
 */
void name_append(u8* dst, u8* src)
{
    u8* p;
    s32 dst_len;
    s32 src_len;
    s32 saved_dst_len;
    u8 c;
    p = dst;
    dst_len = 0;
    if ((*dst) != 0)
    {
        do
        {
            c = *p;
            if (((u32)((unsigned char)(c - 0x19))) < 7U)
            {
                p += 2;
                dst_len += 2;
            }
            else
            {
                p += 1;
                dst_len += 1;
            }
        } while ((*p) != 0);
    }
    p = src;
    src_len = 0;
    saved_dst_len = dst_len;
    if ((*p) != 0)
    {
        do
        {
            c = *p;
            if (((u32)((unsigned char)(c - 0x19))) < 7U)
            {
                p += 2;
                src_len += 2;
            }
            else
            {
                p += 1;
                src_len += 1;
            }
        } while ((*p) != 0);
    }
    dst_len = 0;
    if (src_len > 0)
    {
        do
        {
            dst[saved_dst_len + dst_len] = src[dst_len];
            dst_len++;
        } while (dst_len < src_len);
    }
    dst[saved_dst_len + dst_len] = 0;
}

/**
 * @brief Remove the last glyph from @p name and return it.
 *
 * Walks the buffer keeping a one-glyph-behind pointer; on exit @c prev_pos
 * points at the last glyph and @c scan_pos at the null. The returned
 * @c s32 packs the glyph as `lead | (trail << 8)` for a DBCS pair, or just
 * the byte value for a 1-byte glyph. The buffer is truncated by writing
 * 0 at @c prev_pos. Empty names return 0 unchanged.
 *
 * @param name Null-terminated name buffer (truncated in-place).
 * @return Removed glyph packed as `lead | (trail << 8)`, or 0 if empty.
 * @see https://decomp.me/scratch/agZ8y (100%)
 */
s32 name_pop_last_char(u8* name)
{
    u8* prev_pos;
    u8* scan_pos;
    s32 last_char;

    prev_pos = name;
    scan_pos = prev_pos;
    if (*prev_pos != 0)
    {
        do
        {
            prev_pos = scan_pos;
            if ((u32)(*scan_pos - 0x19) < 7U)
            {
                scan_pos = prev_pos + 2;
            }
            else
            {
                scan_pos = prev_pos + 1;
            }
        } while (*scan_pos != 0);
    }
    last_char = prev_pos[0] | (prev_pos[1] << 8);
    if (prev_pos != scan_pos)
    {
        *prev_pos = 0;
    }
    return last_char;
}

/**
 * @brief Copy @p src into @p dst, including the null terminator.
 *
 * Computes the byte length of @p src walking the DBCS-style encoding, then
 * copies that many bytes and writes a terminator. Caller must ensure
 * @p dst has room.
 *
 * @param dst Destination name buffer.
 * @param src Null-terminated source name.
 * @see https://decomp.me/scratch/UeYRe (100%)
 */
void name_copy(u8* dst, u8* src)
{
    u8* p;
    s32 i;
    s32 src_len;
    p = src;
    src_len = 0;
    if ((*src) != 0)
    {
        do
        {
            if (((u32)((unsigned char)((*p) - 0x19))) < 7U)
            {
                p += 2;
                src_len += 2;
            }
            else
            {
                p += 1;
                src_len += 1;
            }
        } while ((*p) != 0);
    }
    i = 0;
    if (src_len > 0)
    {
        u8* dest; /* unused; preserved to match original codegen */
        do
        {
            *(&dst[i]) = src[i];
            i++;
            dest = &dst[i];
        } while (i < src_len);
    }
    dst[i] = 0;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/y0CgJ
 */
void func_80142928(void)
{
    UnkStruct2 sp10[16];
    UnkStruct2* new_var2;
    s16 new_var3;
    s32 count;
    s32 i;
    UnkStruct2* ptr;
    s32 new_var;
    s32* pSum;
    count = func_800644FC(sp10, g_active_name, 0);
    i = 0;
    new_var3 = new_var2->unk10;
    new_var = count;
    g_name_pixel_width = i;
    if (i < new_var)
    {
        if (!new_var)
        {
        }
        ptr = sp10;
        while (i < ((0, new_var)))
        {
            pSum = &g_name_pixel_width;
            new_var2 = ptr;
            new_var3 = new_var2->unk10;
            *pSum += new_var3;
            ptr++;
            i++;
        }
    }
    g_strip_width_target = g_name_pixel_width + 0x18;
}

/**
 * @brief Insert a glyph at the front of @p buffer (in-place).
 *
 * @p new_char packs the glyph as `lead | (trail << 8)`; the lead byte
 * decides whether it is a 1- or 2-byte glyph. The existing buffer
 * contents (including the null terminator) are shifted right by that
 * amount and the new glyph is written at offset 0. Caller must ensure
 * @p buffer has room.
 *
 * No-op if the lead byte is 0.
 *
 * @param buffer   Null-terminated name buffer.
 * @param new_char Glyph to prepend, packed `lead | (trail << 8)`.
 * @see https://decomp.me/scratch/VOLcD (100%)
 */
void name_prepend_char(u8* buffer, u16 header)
{
    u8* ptr;
    u32 len;
    u32 header_size;
    u32 move_count;
    u32 i;
    u16 h = header; // save header to match register usage

    if ((h & 0xFF) == 0)
        return;

    if (IS_DBSC_LEAD_BYTE(h & 0xFF))
    {
        header_size = 2;
    }
    else
    {
        header_size = 1;
    }

    ptr = buffer;
    len = 0;

    while (*ptr != 0)
    {

        if (IS_DBSC_LEAD_BYTE(*ptr))
        {
            ptr += 2;
            len += 2;
        }
        else
        {
            ptr += 1;
            len += 1;
        }
    }

    move_count = len + 1;
    for (i = move_count; i > 0; i--)
    {
        buffer[(header_size + i) - 1] = buffer[i - 1];
    }

    buffer[0] = (u8)(h & 0xFF);
    if (header_size == 2)
    {
        buffer[1] = (u8)(h >> 8);
    }
}

/**
 * @brief Remove the first glyph from @p name and return it.
 *
 * Reads one glyph at the head of the buffer (1 or 2 bytes per the
 * DBCS-style encoding), measures the rest of the buffer's byte length,
 * shifts the remaining bytes (plus null terminator) left by the glyph
 * size, and returns the removed glyph packed as `lead | (trail << 8)`.
 *
 * @param name Null-terminated name buffer (mutated in-place).
 * @return Removed glyph packed in low 16 bits, or 0 if @p name was empty.
 * @see https://decomp.me/scratch/ArXXq (100%)
 */
s32 name_pop_first_char(u8* name)
{
    u8 first;
    u32 width;
    u16 first_char;
    u8* p;
    s32 tail_len;
    s32 move_count;
    s32 i;
    u32 mask_u16;

    first = name[0];

    if (first == 0)
    {
        return 0;
    }

    if (IS_DBSC_LEAD_BYTE(first))
    {
        first_char = MAKE_DBCS_GLYPH(name[0], name[1]);
        width = 2;
    }
    else
    {
        first_char = name[0];
        width = 1;
    }

    /* Measure tail (everything after the first glyph) in bytes. */
    tail_len = 0;
    p = name + width;

    while (*p != 0)
    {
        if (IS_DBSC_LEAD_BYTE(*p))
        {
            p += 2;
            tail_len += 2;
        }
        else
        {
            p += 1;
            tail_len += 1;
        }
    }

    move_count = tail_len + 1; /* +1 to also shift the null terminator */
    mask_u16 = 0xFFFFU;
    for (i = 0; i < move_count; i++)
    {
        name[i] = name[i + width];
    }

    return (s32)(first_char & mask_u16);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/3TQG6
 */
s32 func_80142B18(s32 arg0, s32 arg1)
{
    u8 idx = D_8014F8B0;
    s32 result = arg0;
    u8* new_var2;
    s32 s3 = arg1;
    s32 i;
    u8* s1 = &D_8014F758[idx * 12];
    u8* s0 = s1 + 1;
    short new_var;
    for (i = 0; i < 3; i++, s0 += 4, s1 += 4)
    {
        s32 byte = s0[1];
        new_var = byte;
        if (new_var != 0)
        {
            result = func_80142274(result, s3 + 0x30, new_var, s1[0] + 0xE8, s0[0] + 4, 0, 0, 0);
        }
    }

    if (D_8014F8B8 != 0)
    {
        D_8014F8B8--;
        if (D_8014F8B8 == 0)
        {
            D_8014F8B0++;
            if (D_8014F8B0 == 7)
            {
                D_8014F8B0 = 0;
                D_8014F8B8 = 0;
                return result;
            }
            else
            {
                new_var2 = D_8014F758;
                D_8014F8B8 = new_var2[(D_8014F8B0 * 12) + 3];
            }
        }
    }
    return result;
}

/**
 * @brief Test whether a name buffer is empty or contains only blanks.
 *
 * Walks @p name byte-by-byte (note: not glyph-by-glyph). The buffer is
 * blank if every byte is either ASCII space (@ref CHAR_SPACE) or the
 * wide-space sentinel (@ref CHAR_WIDE_SPACE). An empty (immediate-null)
 * buffer also counts as blank.
 *
 * @param name Null-terminated name buffer.
 * @return 1 if blank, 0 otherwise.
 * @see https://decomp.me/scratch/rdbBA (100%)
 */
s32 name_is_blank(u8* name)
{
    while (*name != 0)
    {
        if (*name != CHAR_SPACE && *name != CHAR_WIDE_SPACE)
        {
            return FALSE;
        }

        name++;
    }

    return TRUE;
}