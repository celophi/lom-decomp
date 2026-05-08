#include "gname.h"

/**
 * @brief Reset the RGB fade state.
 *
 * Zeros the current color (@c D_8014F828) and the target color +
 * step count (@c D_8014F818). After this call the next
 * @ref func_80140410 tick will write a black tint (0,0,0) to the
 * primitive at @c arg0->unk4040.
 *
 * @see https://decomp.me/scratch/ld2aW (100%)
 */
void func_801403E0(void)
{
    D_8014F828.r = 0;
    D_8014F828.g = 0;
    D_8014F828.b = 0;
    D_8014F818.r = 0;
    D_8014F818.g = 0;
    D_8014F818.b = 0;
    D_8014F818.steps = 0;
}

/**
 * @brief Per-frame RGB fade tick: lerp current toward target and emit a
 *        full-screen tint quad + draw-mode pair into the OT.
 *
 * If `D_8014F818.steps` is non-zero, advances `D_8014F828` by one step of
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
void func_80140410(ArgStruct* ctx)
{
    u32* prim = (u32*)ctx->unk4040;     /* t4 — current primitive write pos */
    ArgStruct* arg = ctx;               /* t6 = t7 — preserves load order */
    s32 step_r, step_g, step_b;
    s32 abr_cmd;                        /* low byte of GP0 0xE1 (abr select) */

    /* Lerp current toward target, or snap if no steps remain. */
    if (D_8014F818.steps != 0)
    {
        step_r = (D_8014F818.r - D_8014F828.r) / D_8014F818.steps;
        step_g = (D_8014F818.g - D_8014F828.g) / D_8014F818.steps;
        step_b = (D_8014F818.b - D_8014F828.b) / D_8014F818.steps;
        D_8014F818.steps--;
        D_8014F828.r += step_r;
        D_8014F828.g += step_g;
        D_8014F828.b += step_b;
    }
    else
    {
        D_8014F828.r = D_8014F818.r;
        D_8014F828.g = D_8014F818.g;
        D_8014F828.b = D_8014F818.b;
    }

    /* Skip emit when fully transparent / identity tint. */
    if (!((D_8014F828.r == 0x100) && (D_8014F828.g == 0x100) && (D_8014F828.b == 0x100)))
    {
        /* Flat-quad RGB bytes at prim[4..6]. */
        if (D_8014F828.r >= 0x101)
        {
            /* Additive bias: subtract 1 so 0x101 → 0x00..0xFF. */
            ((u8*)prim)[4] = (u8)D_8014F828.r - 1;
            ((u8*)prim)[5] = (u8)D_8014F828.g - 1;
            ((u8*)prim)[6] = (u8)D_8014F828.b - 1;
        }
        else
        {
            /* Subtractive bias: bitwise NOT so 0xFF → 0x00, 0x00 → 0xFF. */
            ((u8*)prim)[4] = (D_8014F828.r == 0x100) ? 0 : ~(u8)D_8014F828.r;
            ((u8*)prim)[5] = (D_8014F828.g == 0x100) ? 0 : ~(u8)D_8014F828.g;
            ((u8*)prim)[6] = (D_8014F828.b == 0x100) ? 0 : ~(u8)D_8014F828.b;
        }

        /* Flat-shaded full-screen quad header: 3-word tag 0x62. */
        ((u8*)prim)[3] = 3;
        ((u8*)prim)[7] = 0x62;
        *((u16*)((u8*)prim +  8)) = 0;       /* x = 0   */
        *((u16*)((u8*)prim + 10)) = 0;       /* y = 0   */
        *((u16*)((u8*)prim + 12)) = 0x140;   /* w = 320 */
        *((u16*)((u8*)prim + 14)) = 0xF0;    /* h = 240 */

        /* Splice quad into OT. */
        *prim = (*prim & 0xFF000000) | (arg->unk0 & 0xFFFFFF);
        arg->unk0 = (arg->unk0 & 0xFF000000) | ((u32)prim & 0xFFFFFF);

        /* Choose blend mode by direction of tint. */
        prim = (u32*)((u8*)prim + 0x10);
        abr_cmd = (D_8014F828.r >= 0x101) ? 0x25 : 0x45;

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
 * `steps` ticks of @ref func_80140410 will lerp the current color toward
 * `(r, g, b)` and then snap on the final tick.
 *
 * @param r     Target red   (0..0x100 normal, >0x100 = additive).
 * @param g     Target green (0..0x100 normal, >0x100 = additive).
 * @param b     Target blue  (0..0x100 normal, >0x100 = additive).
 * @param steps Frames over which to interpolate. 0 means "snap immediately".
 *
 * @see https://decomp.me/scratch/jq3uD (100%)
 */
void func_801406F8(s32 r, s32 g, s32 b, s32 steps)
{
    D_8014F818.r = r;
    D_8014F818.g = g;
    D_8014F818.b = b;
    D_8014F818.steps = steps;
}

/**
 * @brief Overlay boot/reset entry: prep VRAM, load CLUT, init run state.
 *
 * Calls (in order):
 *  - @ref func_8014075C  — RECT build + handoff (likely VRAM clear or CLUT load).
 *  - @ref func_800AA02C  — engine helper (audio/SFX init).
 *  - sets @c D_8014F880 = 0x28 (40 — likely a startup delay countdown).
 *  - @ref func_8006441C  — engine helper.
 *  - @ref func_801409EC  — zero/seed all of the overlay's run-state globals.
 *  - @ref func_80063194  — engine helper.
 *
 * @see https://decomp.me/scratch/pnzC1 (100%)
 */
void func_80140714(void)
{
    volatile int dummy[2]; /* forces 0x20 stack frame, ra at 0x18(sp) */
    func_8014075C();
    func_800AA02C();
    D_8014F880 = 0x28;
    func_8006441C();
    func_801409EC();
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
 *  - @ref func_801408D0  — countdown / lerp / SFX trigger update.
 *
 * @param ctx Render context passed through to @ref func_80141928.
 *
 * @see https://decomp.me/scratch/yYkTM (100%)
 */
void func_80140888(s32 ctx)
{
    func_80142410();
    func_80141928(ctx);
    D_800F22AC += 1;
    func_801408D0();
}

/**
 * @brief Per-frame state-machine update: countdown, scalar lerp, input SFX.
 *
 *  - When the startup countdown @c D_8014F880 hits zero, hands off to
 *    @ref func_8014139C (the next stage); otherwise decrements it.
 *  - Lerps the scalar @c D_8014F8A8 toward @c D_8014F8BC over
 *    @c D_8014F8A4 frames using the same `(target - current)/steps` shape
 *    as the RGB fade.
 *  - When input mask @c D_80122988 == 0x800 (a specific button bit), plays
 *    one of two SFX via @ref func_800A3938 (bank 0x80, sound 0x7E or 0x78)
 *    based on whether the cursor's current entry passes the
 *    @ref func_80142720 / @ref func_80142C50 validation pair, and on the
 *    "valid" path also kicks @c D_8014F7E4 = 5 to advance overlay state.
 *
 * @see https://decomp.me/scratch/g5Rx3 (100%)
 */
void func_801408D0(void)
{
    s32 steps;

    /* Startup delay countdown. */
    if (D_8014F880 == 0)
    {
        func_8014139C();
    }
    else
    {
        D_8014F880--;
    }

    /* Lerp D_8014F8A8 toward D_8014F8BC, snap when no steps remain. */
    steps = D_8014F8A4;
    if (steps != 0)
    {
        D_8014F8A4--;
        D_8014F8A8 += (D_8014F8BC - D_8014F8A8) / steps;
    }
    else
    {
        D_8014F8A8 = D_8014F8BC;
    }

    /* Confirm-button: play accept SFX on valid entry, reject SFX otherwise. */
    if (D_80122988 == 0x800)
    {
        if ((func_80142720(D_8014F844) != 0) && (func_80142C50(D_8014F844) == 0))
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
 * Called from the boot path @ref func_80140714. Zeros most counters/indices,
 * primes the lerp scalar (@c D_8014F8A4 = 5), seeds the cursor state
 * (@c D_8014F88C / @c D_8014F890 from frozen defaults @c D_8014F894 /
 * @c D_8014F89C), kicks @ref func_80140AB8 to compute initial @c D_8014F8AC,
 * and registers the overlay's per-character buffer with
 * @ref func_801428A4 (`D_8014F844`, `&D_8014F7E8`).
 *
 * @see https://decomp.me/scratch/FboaU (100%)
 */
void func_801409EC(void)
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
    func_801428A4(D_8014F844, &D_8014F7E8); /* matches 'la a1, D_8014F7E8' */
    D_8014F8A8 = 0;
    func_80142928();
    D_8014F8A4 = 5;
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

                if ((func_80142720(D_8014F844) != 0) && (func_80142C50(D_8014F844) == 0))
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
                func_80142844(D_8014F844);
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
                func_801428A4(D_8014F844, (void*)((f00 + half3a) + reg_s6));
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
                func_80142764(D_8014F844, (void*)((f00 + half3b) + reg_s6), (s32)(new_var13 = f00));
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
            func_801428A4(D_8014F844, var_a1);
        block_38:
            func_80142928();
            repeat = 0;
            D_8014F8A4 = five;
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
                if (func_80142720(D_8014F844) < 0xA)
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
                        func_80142764(D_8014F844, (void*)((D_80142EF8 + hw) + reg_s6), (s32)ef8);
                    }
                    func_80142928();
                    D_8014F8A4 = five;
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
                if (func_80142720(D_8014F844) < 0xA)
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
                        func_80142764(D_8014F844, (void*)((efc + hw) + reg_s6), (s32)efc);
                    }
                    func_80142928();
                    D_8014F8A4 = five;
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
        temp_s1 = func_80142844(D_8014F844);
        while (func_80142720(&D_8014F850) >= 0xB)
        {
            func_80142844(&D_8014F850);
        }

        func_801429A0(&D_8014F850, temp_s1 & 0xFFFF);
        func_80142928();
        D_8014F8A4 = 5;
        var_a0 = 0x7D;
        new_var3 = 0x80;
        func_800A3938(var_a0, new_var3);
    }
    else if (D_80122988 & 2)
    {
        if (func_80142720(D_8014F844) < 0xA)
        {
            new_var2 = &D_8014F850;
            temp_v0 = func_80142A54(new_var2);
            temp_v0_2 = (u16)temp_v0;
            if (temp_v0_2 != 0)
            {
                sp10 = temp_v0;
                (&sp10)[1] = (s8)(temp_v0_2 >> 8);
                (&sp10)[2] = 0;
                func_80142764(D_8014F844, &sp10);
                func_80142928();
                D_8014F8A4 = 5;
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
            if (func_80142720(D_8014F844) == 0)
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
        func_80142844(D_8014F844);
        func_80142928();
        D_8014F8A4 = 5;
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
    unsigned char* D = D_80142CD4;
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
    temp_v0 = func_80141C34(func_80142220(func_80142B18(func_80142274(func_80142220(var_t0, ((char*)new_var4) + 0x2C),
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
    *((u8*)(((char*)temp_v0) + 12)) = D_80142CD4[0xA0];
    *((u8*)(((char*)temp_v0) + 13)) = D_80142CD4[0xA1];
    *((s16*)(((char*)temp_v0) + 16)) = (s16)D_80142CD4[0xA2];
    *((s16*)(((char*)temp_v0) + 18)) = (s16)D_80142CD4[0xA3];
    {
        u32 tmp = *((u32*)(D_80142CD4 + 0xA4));
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
    *((void**)new_var3) = func_80141D64(func_80142220(var_t0_2, arg0), new_var2 + 0x24);
    func_80141F9C(arg0, D_8014F848);
    func_80141E04(new_var4, D_8014F844, D_8014F8A8);
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
 *      emitted via @ref func_80142220 / @ref func_80142274.
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
    s32* ot_head;       /* &ctx->unk38 — passed as the OT head pointer */
    s32* prim;          /* current primitive being emitted */
    s32* next_prim;     /* heap cursor after the sprite/draw-mode pair */
    s32 vram_y;         /* VRAM Y of the back page (0x18 or 0x100) */
    u32 load_packet[0x19];
    s32 vram_x;         /* VRAM X of the right-aligned strip */

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
    next_prim = func_80142220(
        func_80142274(
            func_800A88A0(prim + 0x10, ot_head, tex_src, 1, 0x10, 8, 0),
            ot_head, 2, 0, 0, 0, 0, 0),
        ot_head);

    /* 3. Build a back-page VRAM upload RECT (W = strip_width, H = 32) at the
     *    right edge of whichever page is currently the back buffer. */
    vram_x = 0xF0 - strip_width;
    vram_y = 0x18;
    if (ctx->unk404C != 0) {
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
void* func_80142220(void* prim, s32* ot_head)
{
    unsigned char* bytes = (unsigned char*)prim;
    u32* words = (u32*)prim;
    u32 old_head, this_addr;

    bytes[3] = 1;
    *(u32*)(bytes + 4) = 0xE1000005;

    /* prim->next = old OT head (24-bit). */
    old_head = words[0];
    words[0] = (old_head & 0xFF000000) | ((u32)(*ot_head) & 0x00FFFFFF);

    /* OT head = prim (24-bit). */
    this_addr = (u32)(*ot_head);
    *ot_head = (this_addr & 0xFF000000) | ((u32)((unsigned long)prim) & 0x00FFFFFF);

    return (void*)(bytes + 8);
}

/**
 * decomp.me (88.88%) https://decomp.me/scratch/UHlWz
 */
void* func_80142274(void* arg0, s32* arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, s32 arg7)
{
    unsigned char* base = (unsigned char*)arg0;
    unsigned char* ptr = base;
    TableEntry* entry = &D_80142CD4[arg2];
    u32 temp;
    s32 tmp2;

    /* First structure at base */
    *(u32*)(ptr + 4) = 0x808080;
    *(ptr + 3) = 4;
    *(ptr + 7) = 0x64;
    *(u16*)(ptr + 8) = (u16)(arg3 - arg5 + arg6);
    *(u16*)(ptr + 10) = (u16)(arg4 - arg5 + arg6);
    *(ptr + 12) = entry->field0;
    *(ptr + 13) = entry->field1;
    *(u16*)(ptr + 16) = (u16)entry->field2;
    *(u16*)(ptr + 18) = (u16)entry->field3;
    *(u16*)(ptr + 14) = (u16)((entry->field4 & 0x3F) | 0x7C80);

    /* Update word at offset 0 and *arg1 */
    temp = *(u32*)ptr;
    *(u32*)ptr = (temp & 0xFF000000) | ((u32)(*arg1) & 0x00FFFFFF);
    *arg1 = (*arg1 & 0xFF000000) | ((u32)((unsigned long)arg0) & 0x00FFFFFF);

    ptr += 0x14;

    if (arg5 != 0)
    {
        /* Second structure at ptr */
        *(u32*)(ptr + 4) = (arg7 != 0) ? 0xA00000 : 0;
        *(ptr + 3) = 4;
        *(ptr + 7) = (arg7 == 0) ? 0x66 : 0x64;
        tmp2 = (arg5 - arg6) * 2;
        *(u16*)(ptr + 10) = (u16)(arg4 + tmp2);
        *(u16*)(ptr + 8) = (u16)(arg3 + tmp2);
        *(ptr + 12) = entry->field0;
        *(ptr + 13) = entry->field1;
        *(u16*)(ptr + 16) = (u16)entry->field2;
        *(u16*)(ptr + 18) = (u16)entry->field3;
        *(u16*)(ptr + 14) = (u16)((entry->field4 & 0x3F) | 0x7C80);

        /* Update second structure's word at offset 0 and *arg1 */
        temp = *(u32*)ptr;
        *(u32*)ptr = (temp & 0xFF000000) | ((u32)(*arg1) & 0x00FFFFFF);
        *arg1 = (*arg1 & 0xFF000000) | ((u32)((unsigned long)ptr) & 0x00FFFFFF);

        ptr += 0x14;
    }

    return (void*)ptr;
}