/**
 * @file field29.c
 * @brief Field actor ground-shadow primitive emitter, carved from the head of
 *        the unk2_i fragment (the single-function slot between func_80086FB8
 *        and func_80087564). Called from the field animation processors
 *        (e.g. field11.c's func_80075C88 shadow path).
 */

#include "common.h"

typedef struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u8 padC[0x37 - 0xC];
    u8 unk37;
    u8 pad38[0x3A - 0x38];
    u8 unk3A;
} Rec871A0;

typedef struct {
    u32 unk0;
    u32 unk4;
    s16 unk8;
    s16 unkA;
    u8 unkC;
    u8 unkD;
    u16 unkE;
    s16 unk10;
    s16 unk12;
    u8 unk14;
    u8 unk15;
    u16 unk16;
    s16 unk18;
    s16 unk1A;
    u8 unk1C;
    u8 unk1D;
    u16 unk1E;
    s16 unk20;
    s16 unk22;
    u8 unk24;
    u8 unk25;
    u16 unk26;
} Prim871A0;

typedef struct {
    u16 unk0;
    u16 unk2;
    u16 unk4;
} Off871A0;

typedef struct {
    u16 unk0;
    u16 unk2;
} Screen871A0;

typedef struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
} Scratch871A0;

typedef struct {
    u8 pad0[0x176];
    s16 unk176;
    u8 pad178[0x23C - 0x178];
} State871A0;

typedef struct {
    u8 *start;
    u8 *end;
    u8 unk8;
    u8 slot_index;
    u8 padA[4];
    s16 unkE;
    u32 flags;
} Res871A0;

extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;
extern State871A0 D_80105AE0[];
extern Res871A0 g_field_resource_entries[];

/**
 * @brief Compute an actor's screen-space ground shadow and emit its POLY_FT4
 *        primitive into the ordering table.
 * @param arg0 Effect record supplying world x/z and the shadow selectors at
 *             @c unk37 / @c unk3A.
 * @param arg1 Primitive cursor to fill; returned advanced past the emitted
 *             primitive when one is produced.
 * @param arg2 Ordering-table base array (0x1000 entries) linked into.
 * @param arg3 Shadow footprint half-extents (x at @c unk0, y at @c unk4).
 * @return The (possibly advanced) primitive cursor.
 * @note WIP - 90.73% (101/241 exact rows). Body is raw m2c output kept
 *       verbatim to preserve the verified match; brace style will be
 *       normalised to Allman when the function reaches 100%. Residue is
 *       concentrated in argdiff rows (register coloring) around the two
 *       footprint-projection arms and the OT-link tail.
 * @see decomp.me WIP
 */
Prim871A0 *func_800871A0(Rec871A0 *arg0, Prim871A0 *arg1, s32 *arg2, Off871A0 *arg3)
{
    s32 temp_t3;
    s16 temp_v0_3;
    s16 temp_v0_4;
    s16 temp_v0_5;
    s16 temp_v1;
    s16 var_v0_3;
    s16 var_v0_4;
    s32 *temp_v0_6;
    s32 temp_a0;
    s32 temp_a0_2;
    s32 temp_a0_3;
    s32 temp_a2_2;
    s32 temp_t1;
    s32 temp_v0;
    s32 temp_v1_2;
    s32 temp_v1_3;
    s32 temp_v1_4;
    s32 temp_v1_5;
    s32 var_a0;
    s32 var_a1;
    s32 var_a1_2;
    s32 var_v0;
    s32 var_v0_2;
    s32 var_v1;
    s32 var_v1_2;
    s32 temp_t4;
    u8 temp_a1;
    s32 temp_v0_2;
    s32 cam_x_q;
    s32 rec_x_q;
    s32 cam_y_q;
    s32 rec_z_q;
    s32 screen_y_base;
    s32 work_v1;
    s32 work_a0;
    s32 temp_t7;
    Screen871A0 *screen;
    Scratch871A0 *scratch;
    Prim871A0 *var_t0;
    s32 *var_t6;

    var_t0 = arg1;
    var_t6 = arg2;
    screen = (Screen871A0 *)0x1F8000C0;
    scratch = (Scratch871A0 *)0x1F8000C4;
    var_a1 = D_800F22A0;
    temp_v0 = arg0->unk0;
    scratch->unk4 = 0;
    scratch->unk0 = temp_v0;
    temp_a2_2 = arg0->unk8;
    scratch->unk8 = temp_a2_2;
    if (var_a1 < 0) {
        var_a1 += 0xFF;
    }
    var_a0 = temp_v0;
    cam_x_q = var_a1 >> 8;
    if (var_a0 < 0) {
        var_a0 += 0xFF;
    }
    var_a1_2 = D_800F22A4;
    rec_x_q = var_a0 >> 8;
    rec_x_q += 0xA0;
    temp_t4 = cam_x_q + rec_x_q;
    screen->unk0 = temp_t4;
    if (var_a1_2 < 0) {
        var_a1_2 += 0xFF;
    }
    var_a0 = temp_a2_2;
    cam_y_q = var_a1_2 >> 8;
    screen_y_base = cam_y_q + 0x70;
    if (var_a0 < 0) {
        var_a0 += 0x1FF;
    }
    var_v1 = D_800F22A8;
    rec_z_q = var_a0 >> 9;
    screen_y_base -= rec_z_q;
    if (var_v1 < 0) {
        var_v1 += 0x1FF;
    }
    screen->unk2 = (u16)(screen_y_base - (var_v1 >> 9));
    temp_v0_2 = arg0->unk37;
    temp_a1 = arg0->unk3A;
    temp_t1 = temp_v0_2 << 0x18;
    temp_t3 = D_80105AE0[temp_a1].unk176;
    temp_t7 = temp_t1 >> 24;
    if (g_field_resource_entries[temp_a1].unk8 != 0) {
        temp_a2_2 = temp_t3 << 8;
        var_a1 = temp_t1 >> 0x1A;
        var_v0 = arg0->unk4;
        work_a0 = (u16)arg3->unk0;
        var_v0 -= temp_a2_2;
        var_v0 >>= 0xB;
        var_v0 += var_a1;
        work_v1 = var_v0 << 2;
        var_v0 += work_v1;
        work_v1 = temp_t4 + work_a0;
        if (var_v0 < 0) {
            var_v0 += 3;
        }
        var_v0 >>= 2;
        var_v0 = work_v1 - var_v0;
        var_t0->unk8 = var_v0;
        var_t0->unk18 = var_v0;

        var_v0_2 = arg0->unk4;
        work_v1 = (u16)arg3->unk4;
        var_v0_2 -= temp_a2_2;
        var_v0_2 >>= 0xB;
        var_v0_2 -= var_a1;
        work_a0 = var_v0_2 << 2;
        var_a1 = screen->unk0;
        var_v0_2 += work_a0;
        var_a1 += work_v1;
        if (var_v0_2 < 0) {
            var_v0_2 += 3;
        }
        var_v0_2 >>= 2;
        var_v0_3 = var_a1 + var_v0_2;
    } else {
        temp_a2_2 = temp_t3 << 8;
        var_a1 = temp_t1 >> 0x1A;
        work_v1 = (u16)arg3->unk0;
        var_v0 = arg0->unk4;
        work_v1 += temp_t4;
        var_v0 -= temp_a2_2;
        var_v0 >>= 0xB;
        work_v1 -= var_v0;
        work_v1 += var_a1;
        var_t0->unk8 = work_v1;
        var_t0->unk18 = work_v1;

        var_v0_2 = screen->unk0;
        work_a0 = (u16)arg3->unk4;
        work_v1 = arg0->unk4;
        var_v0_2 += work_a0;
        work_v1 -= temp_a2_2;
        work_v1 >>= 0xB;
        var_v0_2 += work_v1;
        var_v0_3 = var_v0_2 - var_a1;
    }
    var_t0->unk10 = var_v0_3;
    var_t0->unk20 = var_v0_3;
    var_a0 = temp_t3 << 8;
    var_v1 = (s16)arg3->unk0;
    var_v0 = (s16)arg3->unk4;
    var_a1 = var_t0->unk8;
    var_v1 -= var_v0;
    var_v1 >>= 1;
    var_v0 = arg0->unk4;
    if (var_v1 < 0) {
        var_v1 = -var_v1;
    }
    var_v0 -= var_a0;
    var_v0 >>= 0xB;
    var_v1 += var_v0;
    var_a0 = temp_t7 >> 2;
    var_v0 = var_t0->unk10;
    var_v0 = var_v0 < var_a1;
    var_v1 -= var_a0;
    if (var_v0 == 0) {
        var_v0 = var_v1 < 2;
        if (var_v0 == 0) {
            if (temp_t3 != 0) {
                var_v0 = screen->unk2;
                var_v1 >>= 1;
                var_v0 -= var_v1;
                var_v0 += temp_t3;
                var_t0->unk12 = var_v0;
                var_t0->unkA = var_v0;
                var_v0 = screen->unk2;
                var_v0 += var_v1;
                var_v0 += temp_t3;
            } else {
                var_v0 = screen->unk2;
                var_v1 >>= 1;
                var_v0 -= var_v1;
                var_t0->unk12 = var_v0;
                var_t0->unkA = var_v0;
                var_v0 = screen->unk2;
                var_v0 += var_v1;
            }
            var_t0->unk22 = var_v0;
            var_t0->unk1A = var_v0;
            var_t0->unk4 = 0x808080;
            ((u8 *)var_t0)[3] = 9;
            ((u8 *)var_t0)[7] = 0x2E;
            var_t0->unk24 = 0x40;
            var_t0->unk14 = 0x40;
            var_t0->unk15 = 0x50;
            var_t0->unkD = 0x50;
            var_t0->unk25 = 0x70;
            var_t0->unk1D = 0x70;
            var_t0->unk16 = 0x5F;
            var_t0->unk1C = 0;
            var_t0->unkC = 0;
            var_t0->unkE = 0x7850;
            temp_v1_5 = (s32)arg0->unk8 >> 7;
            {
                typedef struct { unsigned addr:24; unsigned len:8; } LocalTag;
                if (temp_v1_5 < 0) {
                    ((LocalTag *)var_t0)->addr = ((LocalTag *)&var_t6[0])->addr;
                    ((LocalTag *)&var_t6[0])->addr = (u32)var_t0;
                    var_t0 = (Prim871A0 *)((u8 *)var_t0 + 0x28);
                } else if (temp_v1_5 >= 0x1000) {
                    ((LocalTag *)var_t0)->addr = ((LocalTag *)&var_t6[0xFFF])->addr;
                    ((LocalTag *)&var_t6[0xFFF])->addr = (u32)var_t0;
                    var_t0 = (Prim871A0 *)((u8 *)var_t0 + 0x28);
                } else {
                    ((LocalTag *)var_t0)->addr = ((LocalTag *)&var_t6[temp_v1_5])->addr;
                    ((LocalTag *)&var_t6[(s32)arg0->unk8 >> 7])->addr = (u32)var_t0;
                    var_t0 = (Prim871A0 *)((u8 *)var_t0 + 0x28);
                }
            }
        }
    }
    return var_t0;
}
