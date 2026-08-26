/**
 * @file field27.c
 * @brief Field HUD gauge/bar renderer, carved from the middle of the unk2_f
 *        fragment (the single-function slot between func_80084700 and
 *        func_80085D30).
 */

#include "common.h"

typedef struct {
    u32 unk0;
    u32 unk4;
    u32 unk8;
    u8 pad0C[0x48 - 0x0C];
    u16 unk48;
    u16 unk4A;
    union {
        u32 unk4C;
        struct { u8 b4C; u8 unk4D; u8 b4E; u8 b4F; } bytes;
    } u4C;
    u8 pad50[0x23C - 0x50];
} State23C;

typedef struct {
    u8 pad0[0x259];
    u8 unk259;
    u8 pad25A[0x25E - 0x25A];
    s16 unk25E;
    s16 unk260;
    u8 pad262[0x268 - 0x262];
} Entry268;

typedef struct {
    u8 pad0[0x2A];
    s16 unk2A;
    u8 pad2C[0x54 - 0x2C];
} Rec54;

#define B(p,o)   (*(u8 *)((u8 *)(p) + (o)))
#define H(p,o)   (*(s16 *)((u8 *)(p) + (o)))
#define UH(p,o)  (*(u16 *)((u8 *)(p) + (o)))
#define W(p,o)   (*(u32 *)((u8 *)(p) + (o)))
#define PTR(p,o) (*(u8 **)((u8 *)(p) + (o)))

u8 *func_80085D30(u8 *, u8 *, u32 *, u16, s32, s32);
u8 *func_80085E84(u8 *, u8 *, u32, s16 *);
u8 *func_80086030(u8 *, s32, u8 *);
u8 *func_800860CC(u8 *, s32, u8 *);
u8 *func_80086184(u8 *, u8 *, u8, s16 *);
s32 rand(void);

extern u32 D_800EAFD8[];
extern u32 D_800EAFEC[];
extern u32 D_800EAFF4[];
extern u32 D_800EAFFC[];
extern u32 D_800EB004[];
extern s16 D_800EB058[];
extern Entry268 D_800FD818[];
extern u8 D_800FDCEA;
extern Rec54 D_800FDF58[];
extern State23C D_80105AE0[];
extern s32 D_801077F0[];
extern s32 D_8010A000;
extern s32 D_8010A004;
extern s32 D_8010A008;
extern s32 D_8010A00C;
extern s32 D_8010A010;
extern u8 D_80117EC8[];
extern s32 g_frame_counter;

/**
 * @brief Emit the per-track HUD gauge primitives (background bar, fill bar,
 *        delta/tick markers and the closing draw-mode primitive) for one
 *        actor slot, animating the gauge value toward its target.
 * @param arg0 Base screen x for the gauge group.
 * @param var_s6 Base screen y (adjusted by the per-slot shake table).
 * @param arg2 Track/slot index (0..2 use the extended blink/shake path).
 * @param arg3 Pointer to the primitive-buffer cursor cell; updated on return.
 * @param arg4 Gauge full-scale denominator used for value-to-width scaling.
 * @note WIP - 94.46% (822/1034 exact rows). Body is raw m2c output kept
 *       verbatim to preserve the verified match; brace style will be
 *       normalised to Allman when the function reaches 100%. Remaining
 *       residue is CSE-FOLD: the target re-reads the D_8010A00C/D_8010A010/
 *       D_8010A004 layout globals where this draft folds them into earlier
 *       loads. Callees func_80085D30/func_80086030/func_800860CC/func_80086184
 *       are still asm in the neighbouring unk2_f / unk2_g fragments.
 * @see decomp.me WIP
 */
void func_80084D08(s32 arg0, s32 var_s6, s32 arg2, u8 *arg3, u32 arg4) {
    s16 *scratch = (s16 *)0x1F800000;
    u8 *ctx;
    u8 *call_ctx;
    u8 **arg3p;
    u8 *temp_ctx;
    u32 sp18;
    u32 *var_a2_2;
    s16 temp_a0_5;
    s16 temp_a0_6;
    s16 temp_a0_8;
    s16 temp_a1_2;
    s16 temp_a1_3;
    s16 temp_a1_4;
    s16 temp_v0_6;
    s16 temp_v0_7;
    s32 temp_v1_10;
    s32 temp_v1_11;
    s16 temp_v1_16;
    s16 temp_v1_19;
    s16 temp_v1_5;
    s16 temp_v1_6;
    s16 var_v0_10;
    s16 var_v0_7;
    s16 var_v0_9;
    s16 var_v1_2;
    s32 *temp_a0_7;
    s32 *temp_s1;
    s32 *temp_v1;
    s32 *temp_v1_2;
    s32 *temp_v1_8;
    s32 temp_a0_2;
    s32 temp_a3;
    s32 temp_a3_2;
    s32 temp_a3_3;
    s32 special_dx;
    s32 temp_v0_2;
    s32 temp_v0_5;
    s32 temp_v1_12;
    s32 temp_v1_13;
    s32 temp_v1_9;
    s32 var_a0_2;
    s32 var_a0_4;
    s32 var_a0_5;
    s32 var_v0_2;
    s32 var_v0_3;
    s32 var_v0_4;
    s32 var_v0_5;
    s32 var_v0_6;
    s32 var_v0_8;
    s32 var_v1_3;
    s32 var_v1_4;
    u16 var_a3;
    u32 temp_a0_3;
    u32 temp_a0_9;
    u32 temp_a1_5;
    u32 temp_lo;
    u32 temp_lo_2;
    u32 temp_v0;
    u32 temp_v0_4;
    u32 temp_v1_14;
    u32 temp_v1_15;
    u32 temp_v1_17;
    u32 temp_v1_18;
    u32 temp_v1_20;
    u32 temp_v1_3;
    u32 temp_v1_7;
    u32 var_a2;
    u32 mask24;
    u32 temp_t0;
    u8 *var_v1;
    u8 temp_a0;
    u8 temp_v0_3;
    s32 temp_v1_21;
    u8 temp_v1_4;
    u8 var_v0;
    Entry268 *temp_a0_4;
    Entry268 *entry_base;
    Entry268 *temp_a1;
    u8 *temp_s0;
    State23C *temp_s2;
    u8 *var_a0;
    u8 *var_a0_3;
    u8 *var_a0_6;
    u8 *var_a0_7;
    u8 *var_a0_8;
    u8 *var_a2_3;
    u8 *var_a2_4;
    u8 *var_s0;
    u8 *var_t1;

    if (arg2 < 3) {
        if (D_800FD818[arg2].unk259 < 6U) {
            var_s6 += D_800EB058[D_800FD818[arg2].unk259];
            if (D_800FD818[arg2].unk259 != 0) {
                D_800FD818[arg2].unk259--;
            } else {
                D_800FD818[arg2].unk259 = 0xFF;
            }
        }
    }
    temp_s2 = &D_80105AE0[arg2];
    temp_v0 = (u32) temp_s2->unk8 >> 0x1F;
    sp18 = temp_v0;
    if ((temp_v0 != 0) && (D_8010A000 < 6)) {
        var_s6 += D_800EB058[D_8010A000];
        if (D_8010A000 != 0) {
            D_8010A000--;
        } else {
            D_8010A000 = 0xFF;
        }
    }
    arg3p = &arg3;
    temp_ctx = *arg3p;
    var_a0 = PTR(temp_ctx, 0x40B8);
    ctx = temp_ctx;
    if (temp_s2->u4C.bytes.unk4D < 3U) {
        D_8010A00C = 0x1D;
        D_8010A010 = 9;
        scratch[0] = arg0;
        scratch[1] = var_s6;
        var_a0 = func_80086184(var_a0, ctx + 0xC, temp_s2->u4C.bytes.unk4D, scratch);
        D_8010A008 = 0x36;
    } else if (sp18 != 0) {
        D_8010A00C = 0xF;
        D_8010A010 = 6;
        D_8010A008 = 0xE3;
    } else {
        D_8010A00C = 4;
        D_8010A010 = 6;
        D_8010A008 = 0x36;
    }
    D_8010A004 = 3;
    var_s0 = var_a0;
    if (arg2 < 3) {
        temp_v1 = D_801077F0;
        temp_v1 += arg2;
        if (*temp_v1 < 6) {
            (*temp_v1)++;
            if (*temp_v1 < 6) {
                goto build_first_prim;
            }
        }
        for (var_a0_2 = 0; var_a0_2 < 8; var_a0_2++) {
            if (D_80117EC8[var_a0_2] == 0xFF) {
                break;
            }
            if (D_80117EC8[var_a0_2] == arg2) {
                D_801077F0[arg2] = 0;
                break;
            }
        }
        temp_s1 = &D_801077F0[arg2];
        if ((*temp_s1 != 0) && !(rand() & 0x3F)) {
            *temp_s1 = 0;
        }
build_first_prim:
        W(var_s0, 0x4) = 0x808080;
        B(var_s0, 0x3) = 4;
        B(var_s0, 0x7) = 0x64;
        H(var_s0, 0x8) = (s16) (arg0 + 0x22);
        H(var_s0, 0xA) = (s16) (var_s6 - 6);
        temp_v1_2 = &D_801077F0[arg2];
        if (*temp_v1_2 < 6) {
            var_v1_2 = ((u16) *temp_v1_2 * 0x10) + 0x2058;
        } else {
            var_v1_2 = 0x2058;
        }
        W(var_s0, 0x10) = 0x100010;
        H(var_s0, 0xC) = var_v1_2;
        H(var_s0, 0xE) = 0x7850;
        W(var_s0, 0x0) = (s32) ((W(var_s0, 0x0) & 0xFF000000) | (W(ctx, 0xC) & 0xFFFFFF));
        W(ctx, 0xC) = (s32) ((W(ctx, 0xC) & 0xFF000000) | ((s32) var_s0 & 0xFFFFFF));
        var_s0 += 0x14;
        scratch[0] = arg0 + 0x30;
        temp_a0_3 = temp_s2->unk4;
        scratch[1] = var_s6;
        temp_v1_3 = temp_s2->unk0;
        var_a2 = temp_a0_3 * 0x64;
        if (temp_v1_3 != 0) {
            var_a2 = var_a2 / temp_v1_3;
        }
        if ((var_a2 == 0) && (temp_a0_3 != 0)) {
            var_a2 = 1;
        }
        var_s0 = func_80085E84(var_s0, ctx, var_a2, scratch);
    }
    temp_v1_4 = temp_s2->u4C.bytes.unk4D;
    var_a0_3 = var_s0;
    if (temp_v1_4 < 2U) {
        if (temp_s2->u4C.unk4C & 1) {
            if (temp_s2->unk48 == 0xFF) {
                if (g_frame_counter & 8) {
                    var_a2 = (u32) D_800EAFFC;
                } else {
                    var_a2 = (u32) D_800EAFEC;
                }
            } else {
                var_a2 = (u32) D_800EAFEC;
            }
            var_a3 = temp_s2->unk48;
            call_ctx = ctx;
        } else {
            call_ctx = ctx;
            var_a2 = (u32) D_800EAFF4;
            var_a3 = temp_s2->unk4A;
        }
        var_a0_3 = func_80085D30(var_a0_3, call_ctx, (u32 *) var_a2, var_a3, (s32) arg0, (s32) var_s6);
    } else if ((temp_v1_4 == 2) && ((u8) D_800FDCEA >= 0x41U)) {
        var_a2 = (u32) D_800EB004;
        var_a0_3 = func_80085D30(var_a0_3, ctx, (u32 *) var_a2, temp_s2->unk48, (s32) arg0, (s32) var_s6);
    }
    var_t1 = var_a0_3;
    if (arg2 < 3) {
        entry_base = D_800FD818;
        temp_a0_4 = &entry_base[arg2];
        if ((temp_a0_4->unk260 != 0) && (D_800FDF58[arg2].unk2A == 0x8E)) {
            W(var_t1, 0x4) = 0x202020;
            W(var_t1, 0x14) = 0x202020;
            B(var_t1, 0x3) = 8;
            B(var_t1, 0x7) = 0x38;
            W(var_t1, 0xC) = 0xFFFFFF;
            W(var_t1, 0x1C) = 0xFFFFFF;
            var_a2 = (s16) temp_a0_4->unk260;
            var_a2 = (s32) (D_8010A008 * temp_a0_4->unk25E) / (s32) var_a2;
            temp_a1_2 = (u16) D_8010A010 + var_s6;
            H(var_t1, 0x12) = temp_a1_2;
            H(var_t1, 0xA) = temp_a1_2;
            temp_v1_5 = (u16) D_8010A00C + arg0;
            temp_t0 = (u16) D_8010A004;
            var_a0_2 = temp_a1_2 + temp_t0;
            H(var_t1, 0x22) = var_a0_2;
            H(var_t1, 0x1A) = var_a0_2;
            H(var_t1, 0x8) = temp_v1_5;
            H(var_t1, 0x18) = (s16) (temp_v1_5 - 3);
            temp_v1_6 = temp_v1_5 + var_a2;
            H(var_t1, 0x10) = temp_v1_6;
            H(var_t1, 0x20) = (s16) (temp_v1_6 - 3);
            temp_a3 = (s32) var_t1 & 0xFFFFFF;
            W(var_t1, 0x0) = (s32) ((W(var_t1, 0x0) & 0xFF000000) | (W(ctx, 0xC) & 0xFFFFFF));
            var_t1 += 0x24;
            var_v0_5 = (W(ctx, 0xC) & 0xFF000000) | temp_a3;
            goto block_64;
        }
    }
    temp_v1_7 = temp_s2->unk4;
    if (temp_v1_7 != 0) {
        temp_lo = temp_v1_7 / arg4;
        var_a0_2 = temp_lo & 3;
        if (temp_lo >= 3U) {
            var_a0_2 |= 2;
        }
        temp_t0 = 0xFFFFFF;
        if ((s32) ((temp_s2->unk8 & temp_t0) - temp_v1_7) < (s32) arg4) {
            temp_v1_8 = &D_800EAFD8[var_a0_2];
            W(var_t1, 0x4) = (s32) (*temp_v1_8 & 0x3F3F3F);
            W(var_t1, 0xC) = (s32) (*temp_v1_8 & 0x7F7F7F);
            temp_v1_9 = *temp_v1_8;
            B(var_t1, 0x3) = 8;
            B(var_t1, 0x7) = 0x38;
            W(var_t1, 0x1C) = temp_v1_9;
            W(var_t1, 0x14) = temp_v1_9;
            temp_v1_11 = (s32) (D_8010A008 * (temp_v1_7 % arg4)) / (s32) arg4;
            temp_a1_3 = (u16) D_8010A010 + var_s6;
            H(var_t1, 0x12) = temp_a1_3;
            H(var_t1, 0xA) = temp_a1_3;
            temp_v1_10 = (u16) D_8010A00C + arg0;
            var_a2 = (u16) D_8010A004;
            temp_a0_6 = temp_a1_3 + var_a2;
            H(var_t1, 0x22) = temp_a0_6;
            H(var_t1, 0x1A) = temp_a0_6;
            H(var_t1, 0x8) = temp_v1_10;
            H(var_t1, 0x18) = (s16) (temp_v1_10 - 3);
            temp_v1_11 += temp_v1_10;
            H(var_t1, 0x10) = temp_v1_11;
            H(var_t1, 0x20) = (s16) (temp_v1_11 - 3);
            W(var_t1, 0x0) = (s32) ((W(var_t1, 0x0) & 0xFF000000) | (W(ctx, 0xC) & temp_t0));
            temp_v1_12 = (s32) var_t1 & temp_t0;
            var_t1 += 0x24;
            W(ctx, 0xC) = (s32) ((W(ctx, 0xC) & 0xFF000000) | temp_v1_12);
        }
        temp_v1_14 = (u32) temp_s2->unk4 / arg4;
        temp_v0_4 = temp_v1_14 - 1;
        if (temp_v1_14 != 0) {
            var_a0_5 = temp_v0_4 & 3;
            if (temp_v0_4 >= 3U) {
                var_a0_5 |= 2;
            }
            temp_a0_7 = &D_800EAFD8[var_a0_5];
            W(var_t1, 0x4) = (s32) (*temp_a0_7 & 0x3F3F3F);
            B(var_t1, 0x3) = 8;
            B(var_t1, 0x7) = 0x38;
            W(var_t1, 0x14) = (s32) (*temp_a0_7 & 0x7F7F7F);
            temp_v0_5 = *temp_a0_7;
            W(var_t1, 0x1C) = temp_v0_5;
            W(var_t1, 0xC) = temp_v0_5;
            temp_a0_8 = (u16) D_8010A00C;
            temp_v0_6 = (u16) D_8010A008;
            temp_a1_4 = (u16) D_8010A010;
            temp_v0_7 = (u16) D_8010A004;
            temp_a0_8 += arg0;
            temp_v0_6 += temp_a0_8;
            H(var_t1, 0x10) = temp_v0_6;
            temp_v0_6 -= 3;
            temp_a1_4 += var_s6;
            H(var_t1, 0x20) = temp_v0_6;
            H(var_t1, 0x12) = temp_a1_4;
            temp_v0_7 += temp_a1_4;
            H(var_t1, 0x8) = temp_a0_8;
            temp_a0_8 -= 3;
            H(var_t1, 0x18) = temp_a0_8;
            H(var_t1, 0xA) = temp_a1_4;
            H(var_t1, 0x22) = temp_v0_7;
            H(var_t1, 0x1A) = temp_v0_7;
            W(var_t1, 0x0) = (s32) ((W(var_t1, 0x0) & 0xFF000000) | (W(ctx, 0xC) & 0xFFFFFF));
            temp_v1_13 = (s32) var_t1 & 0xFFFFFF;
            var_t1 += 0x24;
            var_v0_5 = (W(ctx, 0xC) & 0xFF000000) | temp_v1_13;
block_64:
            W(ctx, 0xC) = var_v0_5;
        }
    }
    mask24 = 0xFFFFFF;
    temp_a1_5 = temp_s2->unk8;
    temp_v1_14 = temp_s2->unk4;
    temp_a0_9 = temp_a1_5 & mask24;
    if (temp_a0_9 < temp_v1_14) {
        if ((u32) (temp_v1_14 - temp_a0_9) >= 3U) {
            var_v1_3 = temp_a1_5 & 0xFF000000;
            var_v0_6 = temp_a0_9 + ((temp_v1_14 - temp_a0_9) / 3);
        } else {
            var_v1_3 = temp_a1_5 & 0xFF000000;
            var_v0_6 = temp_a0_9 + 1;
        }
        *(volatile u32 *)&temp_s2->unk8 = (u32) (var_v1_3 | (var_v0_6 & mask24));
        temp_v1_15 = *(volatile u32 *)&temp_s2->unk4;
        if ((temp_v1_15 / arg4) == ((s32) (temp_s2->unk8 & 0xFFFFFF) / (s32) arg4)) {
            temp_a3_2 = (u16) D_8010A00C + arg0;
            H(var_t1, 0xC) = (s16) (temp_a3_2 + ((u32) (D_8010A008 * (temp_v1_15 % arg4)) / arg4));
            var_a2_3 = ctx + 8;
            var_a0_6 = var_t1;
            H(var_a0_6, 0x8) = (s16) (temp_a3_2 + ((s32) (D_8010A008 * ((s32) (temp_s2->unk8 & 0xFFFFFF) % (s32) arg4)) / (s32) arg4));
        } else {
            temp_v1_16 = (u16) D_8010A00C + arg0;
            H(var_t1, 0xC) = temp_v1_16;
            H(var_t1, 0x8) = (s16) (temp_v1_16 + ((s32) (D_8010A008 * ((s32) (temp_s2->unk8 & 0xFFFFFF) % (s32) arg4)) / (s32) arg4));
            var_a0_6 = func_800860CC(var_t1, var_s6, ctx + 8);
            temp_v1_17 = temp_s2->unk4;
            if ((u32) ((temp_s2->unk8 & 0xFFFFFF) - temp_v1_17) < arg4) {
                var_v0_7 = (u16) D_8010A00C + arg0 + ((u32) (D_8010A008 * (temp_v1_17 % arg4)) / arg4);
            } else {
                var_v0_7 = (u16) D_8010A00C + arg0;
            }
            H(var_a0_6, 0xC) = var_v0_7;
            var_a2_3 = ctx + 8;
            H(var_a0_6, 0x8) = (s16) ((u16) D_8010A008 + ((u16) D_8010A00C + arg0));
        }
        var_a0_7 = func_800860CC(var_a0_6, var_s6, var_a2_3);
        goto block_88;
    }
    if (temp_v1_14 < temp_a0_9) {
        if ((u32) (temp_a0_9 - temp_v1_14) >= 4U) {
            var_v1_4 = temp_a1_5 & 0xFF000000;
            var_v0_8 = temp_a0_9 - ((temp_a0_9 - temp_v1_14) / 3);
        } else {
            var_v1_4 = temp_a1_5 & 0xFF000000;
            var_v0_8 = temp_a0_9 - 1;
        }
        *(volatile u32 *)&temp_s2->unk8 = (u32) (var_v1_4 | (var_v0_8 & mask24));
        temp_v1_18 = *(volatile u32 *)&temp_s2->unk4;
        if ((temp_v1_18 / arg4) == ((s32) (temp_s2->unk8 & 0xFFFFFF) / (s32) arg4)) {
            temp_a3_3 = (u16) D_8010A00C + arg0;
            H(var_t1, 0xC) = (s16) (temp_a3_3 + ((u32) (D_8010A008 * (temp_v1_18 % arg4)) / arg4));
            var_a2_4 = ctx + 8;
            var_a0_8 = var_t1;
            H(var_a0_8, 0x8) = (s16) (temp_a3_3 + ((s32) (D_8010A008 * ((s32) (temp_s2->unk8 & 0xFFFFFF) % (s32) arg4)) / (s32) arg4));
        } else {
            temp_v1_19 = (u16) D_8010A00C + arg0;
            H(var_t1, 0xC) = temp_v1_19;
            H(var_t1, 0x8) = (s16) (temp_v1_19 + ((s32) (D_8010A008 * ((s32) (temp_s2->unk8 & 0xFFFFFF) % (s32) arg4)) / (s32) arg4));
            var_a0_8 = func_80086030(var_t1, var_s6, ctx + 8);
            temp_v1_20 = temp_s2->unk4;
            if ((u32) ((temp_s2->unk8 & 0xFFFFFF) - temp_v1_20) < arg4) {
                var_v0_9 = (u16) D_8010A00C + arg0 + ((u32) (D_8010A008 * (temp_v1_20 % arg4)) / arg4);
            } else {
                var_v0_9 = (u16) D_8010A00C + arg0;
            }
            H(var_a0_8, 0xC) = var_v0_9;
            var_a2_4 = ctx + 8;
            H(var_a0_8, 0x8) = (s16) ((u16) D_8010A008 + ((u16) D_8010A00C + arg0));
        }
        var_a0_7 = func_80086030(var_a0_8, var_s6, var_a2_4);
block_88:
        var_t1 = var_a0_7;
    }
    var_s0 = var_t1;
    W(var_s0, 0x4) = 0x808080;
    B(var_s0, 0x3) = 4;
    B(var_s0, 0x7) = 0x66;
    W(var_s0, 0x8) = (s32) ((var_s6 << 0x10) + arg0);
    temp_v1_21 = temp_s2->u4C.bytes.unk4D;
    switch (temp_v1_21) {
    case 0:
    case 1:
        W(var_s0, 0x10) = 0x180058;
        H(var_s0, 0xC) = 0x1000;
        break;
    case 2:
        if ((u8) D_800FDCEA >= 0x41U) {
            W(var_s0, 0x10) = 0x180058;
            H(var_s0, 0xC) = 0x1000;
        } else {
            W(var_s0, 0x10) = 0x180058;
            H(var_s0, 0xC) = 0x2800;
        }
        break;
    default:
        if (sp18 != 0) {
            W(var_s0, 0x10) = 0x100100;
            H(var_s0, 0xC) = 0;
        } else {
            W(var_s0, 0x10) = 0x100040;
            H(var_s0, 0xC) = 0x4000;
        }
        break;
    }
    H(var_s0, 0xE) = 0x7810;
    W(var_s0, 0x0) = (s32) ((W(var_s0, 0x0) & 0xFF000000) | (W(ctx, 0xC) & 0xFFFFFF));
    W(ctx, 0xC) = (s32) ((W(ctx, 0xC) & 0xFF000000) | ((s32) var_s0 & 0xFFFFFF));
    var_s0 += 0x14;
    B(var_s0, 0x3) = 1;
    W(var_s0, 0x4) = 0xE100001F;
    W(var_s0, 0x0) = (s32) ((W(var_s0, 0x0) & 0xFF000000) | (W(ctx, 0xC) & 0xFFFFFF));
    W(ctx, 0xC) = (s32) ((W(ctx, 0xC) & 0xFF000000) | ((s32) var_s0 & 0xFFFFFF));
    PTR(arg3, 0x40B8) = var_s0 + 8;
}
