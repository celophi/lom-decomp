/**
 * @file field8.c
 * @brief Field per-part track-driven parameter roll: seeds one particle
 *        spawn record's timing/scale/rotation fields from a part definition's
 *        parameter tracks.
 */

#include "common.h"

typedef struct
{
    u32 unk0;  /* 0x00 */
    u32 unk4;  /* 0x04 */
    u8 unk8;   /* 0x08 */
    u8 unk9;   /* 0x09 */
    u8 unkA;   /* 0x0A */
    u8 unkB;   /* 0x0B */
    u8 unkC;   /* 0x0C */
    u8 unkD;   /* 0x0D */
    u8 unkE;   /* 0x0E */
    u8 unkF;   /* 0x0F */
    u8 unk10;  /* 0x10 */
    u8 unk11;  /* 0x11 */
    u8 pad12[0x14 - 0x12];
    u32 unk14; /* 0x14 */
    s16 unk18; /* 0x18 */
    u8 unk1A;  /* 0x1A */
    u8 pad1B;
    u16 unk1C; /* 0x1C */
    u8 unk1E; /* 0x1E */
    u8 pad1F;
    u8 unk20;
    u8 unk21;
    u8 unk22;
    u8 unk23;
    s32 unk24;
    u32 unk28;
    u8 unk2C;
    u8 pad2D;
    u8 unk2E;
    u8 unk2F;
    u8 pad30;
    u8 unk31;
    u8 unk32;
    u8 unk33;
    u32 unk34;
    s16 unk38;
    s16 unk3A;
    s16 unk3C;
    s16 pad3E;
    s16 unk40;
    s16 unk42;
    s16 unk44;
    s16 unk46;
} FieldActorPartDef;

typedef struct
{
    s16 x;
    s16 y;
} Vec2s;

typedef struct
{
    FieldActorPartDef* unk0;
    u8 pad4[0xC - 4];
    void* unkC;
    u8 pad10[0x14 - 0x10];
    u8* unk14;
    u8 pad18[0x24 - 0x18];
    u8 unk24;
    u8 unk25;
    u8 unk26;
    u8 unk27;
    u8 unk28;
    u8 unk29;
    u8 unk2A;
    u8 unk2B[16];
    u8 unk3B[9][16];
    u8 padCB;
    u16 unkCC[9][16];
    u16 unk1EC[9];
    Vec2s unk1FE[9];
    u16 unk222;
    u32 unk224;
    u8 owner_object_index;
    u8 unk229[9];
    u8 unk232;
    u8 unk233;
    u16 unk234;
    u16 unk236;
    u8 pad238[2];
    u8 unk23A;
    u8 unk23B;
    u8 pad23C[0x240 - 0x23C];
    u16* unk240;
} FieldActorState;

typedef struct
{
    u8 pad0[0x10];
    s16 unk10; /* 0x10 */
    s16 unk12; /* 0x12 */
    s16 unk14; /* 0x14 */
} FieldTrackResult;

typedef struct
{
    s32 unk0;  /* 0x00 */
    s32 unk4;  /* 0x04 */
    s32 unk8;  /* 0x08 */
    u32 unkC;  /* 0x0C */
    s16 unk10; /* 0x10 */
    s16 unk12; /* 0x12 */
    s16 unk14; /* 0x14 */
    s16 unk16; /* 0x16 */
    u8 unk18;  /* 0x18 */
    u8 unk19;  /* 0x19 */
    u8 unk1A;  /* 0x1A */
    u8 unk1B;  /* 0x1B */
    s32 unk1C; /* 0x1C */
    u8 unk20;  /* 0x20 */
    u8 unk21;  /* 0x21 */
    u8 unk22;  /* 0x22 */
    u8 unk23;  /* 0x23 */
    u8 unk24;  /* 0x24 */
    u8 unk25;  /* 0x25 */
    s8 unk26;  /* 0x26 */
    u8 unk27;  /* 0x27 */
    u8 unk28;  /* 0x28 */
    u8 unk29;  /* 0x29 */
    s16 unk2A; /* 0x2A */
    s16 unk2C; /* 0x2C */
    u16 unk2E; /* 0x2E */
    s16 unk30; /* 0x30 */
    u8 unk32;  /* 0x32 */
    u8 unk33;  /* 0x33 */
    u8 unk34;  /* 0x34 */
    u8 unk35;  /* 0x35 */
    u8 unk36;  /* 0x36 */
    u8 unk37;  /* 0x37 */
    u8 unk38;  /* 0x38 */
    u8 unk39;  /* 0x39 */
    u8 unk3A;  /* 0x3A */
    u8 unk3B;  /* 0x3B */
    u8 unk3C;  /* 0x3C */
    u8 unk3D;  /* 0x3D */
    u8 unk3E;  /* 0x3E */
    u8 pad3F;
    s32 unk40; /* 0x40 */
    u32 unk44; /* 0x44 */
    u32 unk48; /* 0x48 */
    u32 unk4C; /* 0x4C */
    u8 pad50[0x54 - 0x50];
} Struct_D800FDF58;

typedef struct
{
    s32 vx;
    s32 vy;
    s32 vz;
    s32 pad;
} FieldVector;

#include "psyq/inline_c.h"

/*
 * inline_c.h's "Type 2" (no-operand) GTE macros, like gte_sqr0() above, emit
 * a raw `.word` value. Sony's original PSY-Q assembler recognized that value
 * as a GTE opcode token and translated it into the real COP2 instruction
 * word. This project assembles with plain GNU `as` (via the `maspsx` syntax
 * wrapper), which has no such translation step, so the `.word` bits get
 * written out unchanged - the wrong instruction, not an assembler error.
 * The Type 1/Type 3 (load/store) macros above are plain lwc2/swc2/mtc2
 * mnemonics with no such trick, so they assemble correctly as-is.
 * Override just SQR (sf=0) with the real ROM bytes instead, as a `cop2`
 * immediate.
 */
#define gte_sqr0() __asm__ volatile("nop;nop;cop2 0x0a00428")

extern s32 g_field_track_index;
extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;

/**
 * @brief Roll one particle spawn record's scale and rotation fields from a
 *        part's parameter tracks, falling back to fixed part values or a
 *        random roll where a track is not assigned.
 * @param arg0 Owning actor, indexes g_field_track_index into unkCC/unk3B.
 * @param arg1 Part definition supplying the track selectors and fallback
 *             values (unk1E divisor, unk32 timing-table index, unk9/unkA
 *             track selectors).
 * @param arg2 Output record to fill in (unk10/unk12/unk14).
 * @see decomp.me (99.80%) TODO
 * @note Residual is a single register-coloring swap (v0/v1) on the final
 *       `arg2->unk14 += part * 8` accumulate; insn count and every other row
 *       already match. See idioms.md ALLOC-15/ALLOC-14 for the mechanism;
 *       local_alloc_oracle confirms the swap needs either an explicit copy
 *       insertion or a birth-order change that no plain C reshape reaches.
 */
void func_80070CB8(FieldActorState *arg0, FieldActorPartDef *arg1, FieldTrackResult *arg2)
{
    s16 var_v0;
    s32 var_lo;
    s32 var_v0_2;
    s32 temp_s0;

    arg2->unk10 = 0;
    if (arg1->unk1E != 0)
    {
        var_v0 = ((0x1000 / arg1->unk1E) * arg0->unkCC[g_field_track_index][arg1->unk32] + 0x400) & 0xFFF;
        arg2->unk12 = var_v0;
    }
    else
    {
        var_v0 = (u32) rand() >> 3;
        arg2->unk12 = var_v0;
    }
    arg0->unkCC[g_field_track_index][arg1->unk32]++;

    if ((arg1->unk0 >> 0xB) & 1)
    {
        temp_s0 = field_evaluate_parameter_track(arg0, arg1->unk9 & 0xF);
        var_lo = temp_s0 * (rand() << 3);
    }
    else
    {
        var_lo = arg1->unk9 * (rand() << 3);
    }
    arg2->unk14 = var_lo >> 0xF;

    if ((arg1->unk0 >> 0xC) & 1)
    {
        var_v0_2 = field_evaluate_parameter_track(arg0, arg1->unkA & 0xF);
    }
    else
    {
        var_v0_2 = arg1->unkA;
    }
    arg2->unk14 += ((long long) var_v0_2) * 8;
}

/**
 * @brief When a record is due a position refresh, snapshot its current
 *        position as a delta-tracking anchor, roll in a new position via
 *        func_80073F7C, and re-arm the guard.
 * @param arg0 Record whose position (unk0/unk4/unk8) is refreshed.
 * @param arg1 Passed through unchanged to func_80073F7C.
 * @see decomp.me (100%) TODO
 */
void func_80070E4C(Struct_D800FDF58 *arg0, s32 arg1)
{
    s32 new_pos[3];
    s32 unused[2];

    if (arg0->unk1B != 0)
    {
        func_80073F7C(arg0, arg1, new_pos);
        arg0->unk1B = 3;
        arg0->unk44 = arg0->unk0 + D_800F22A0;
        arg0->unk48 = arg0->unk4 + D_800F22A4;
        arg0->unk4C = arg0->unk8 + D_800F22A8;
        arg0->unk0 = new_pos[0];
        arg0->unk4 = new_pos[1];
        arg0->unk8 = new_pos[2];
    }
}

/**
 * @brief Face a record toward the delta between its last stored position and
 *        a freshly rolled position, deriving both a horizontal-plane heading
 *        (unk12) and a vertical pitch (unk14).
 * @param rec Record whose heading/pitch (unk12/unk14) are updated.
 * @param part Passed through unchanged to func_80073F7C.
 * @see decomp.me (100%) TODO
 */
void func_80070EF0(Struct_D800FDF58 *rec, FieldActorPartDef *part)
{
    FieldVector new_pos;
    FieldVector vec;
    FieldVector sqr;

    func_80073F7C(rec, part, &new_pos);
    vec.vx = (new_pos.vx - rec->unk0) >> 8;
    vec.vy = (new_pos.vy - rec->unk4) >> 8;
    vec.vz = (new_pos.vz - rec->unk8) >> 8;

    gte_ldlvl(&vec);
    gte_sqr0();
    gte_stlvnl(&sqr);

    rec->unk12 = ratan2(-vec.vz, vec.vx);
    if (rec->unk12 < 0)
    {
        rec->unk12 += 0x1000;
    }

    if (vec.vy != 0)
    {
        rec->unk14 = ratan2(SquareRoot0(sqr.vx + sqr.vz), -vec.vy);
    }
    else
    {
        rec->unk14 = 0x400;
    }
    if (rec->unk14 < 0)
    {
        rec->unk14 += 0x1000;
    }
    rec->unk10 = 0;
}

typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} RECT;

extern Struct_D800FDF58 D_800FF658[];
extern u8 D_80104B58[];
extern u8 D_80105358[];

/**
 * @brief Per-frame actor tick: advances every active effect record owned by
 *        this actor (culling off-screen ones, spawning chained effects on
 *        expiry), then refreshes the actor's palette-track texture pages.
 * @param arg0 Actor being ticked.
 * @see decomp.me (80.28%) TODO
 * @note WIP - 80.28% (212/312 rows). Two residues, both investigated at
 *       length without a clean fix:
 *       (1) the D_800FF658[rec->unk3D] "previous record" address is
 *       recomputed from scratch by the target FOUR separate times (once per
 *       field read/write) in straight-line code with no intervening call or
 *       branch; plain C (even with freshly-named index locals each time)
 *       always lets gcc 2.7.2's cse.c merge these into one computation. The
 *       `Struct_D800FDF58 * volatile pv` reassigned before each use forces
 *       the recompute (pv is `volatile`, but the round-trip through its own
 *       stack home does not exactly match the target's register-only
 *       rederivation) - this closed most but not all of the gap.
 *       (2) the D_800F22A0/A4/A8 sign-fixup chain (building sx/sy) has a
 *       handful of residual sched1/regalloc-order rows that did not respond
 *       to reassociating the sums or reordering the fixups.
 *       A fresh source-model attempt, or sched_oracle on the sign-fixup
 *       block, would be the next lever - not attempted this session.
 */
void func_8007100C(FieldActorState *arg0_param)
{
    FieldActorState *arg0;
    Struct_D800FDF58 *rec;
    FieldActorPartDef *part;
    void *buf;
    s32 i;
    s32 count;
    s32 newslot;
    Struct_D800FDF58 *newrec;
    s32 v0, v1, a0, v0_2, a0_2, v1_2;
    s16 sx, sy;
    u8 t;
    s32 dx, dy, dz;
    Struct_D800FDF58 * volatile pv;
    s32 unused_pad[2];
    s32 sp20, sp24, sp28;
    s32 out_vec[3];

    arg0 = arg0_param;
    for (rec = D_800FF658; rec != &D_800FF658[256]; rec++)
    {
        if (rec->unk22 == arg0->unk233 && rec->unk25 != 0xFF)
        {
            part = &arg0->unk0[rec->unk23];
            g_field_track_index = rec->unk29;
            func_80071D40(rec, part, arg0);
            rec->unk2C++;
            if (((part->unk4 >> 4) & 3) == 1 && (u16) rec->unk2C == rec->unk28)
            {
                t = rec->unk25;
                rec->unk25 = 0xFF;
                rec->unk27 = t;
            }
            if (((part->unk4 >> 4) & 3) == 3)
            {
                v0 = D_800F22A0;
                if (v0 < 0)
                {
                    v0 += 0xFF;
                }
                v1 = rec->unk0;
                if (v1 < 0)
                {
                    v1 += 0xFF;
                }
                a0 = D_800F22A4;
                sx = (v0 >> 8) + (v1 >> 8) + 0xA0;
                if (a0 < 0)
                {
                    a0 += 0xFF;
                }
                v0_2 = rec->unk4;
                if (v0_2 < 0)
                {
                    v0_2 += 0xFF;
                }
                a0_2 = rec->unk8;
                if (a0_2 < 0)
                {
                    a0_2 += 0x1FF;
                }
                v1_2 = D_800F22A8;
                if (v1_2 < 0)
                {
                    v1_2 += 0x1FF;
                }
                sy = ((a0 >> 8) + (v0_2 >> 8) + 0x70) - (a0_2 >> 9) - (v1_2 >> 9);
                if ((u32) ((sx + 0x140) & 0xFFFF) >= 0x3C1 || sy >= 0x1E1 || sy < -0xF0)
                {
                    t = rec->unk25;
                    rec->unk25 = 0xFF;
                    rec->unk27 = t;
                }
            }
            if (rec->unk25 == 0xFF)
            {
                func_80071500(rec, part);
            }
            if (part->unk24 < 0 && rec->unk25 != 0xFF)
            {
                newslot = func_8006D79C(arg0, part->unk23 & 0xF, 0);
                if (newslot != -1)
                {
                    newrec = &D_800FF658[newslot];
                    newrec->unk0 = rec->unk0;
                    newrec->unk4 = rec->unk4;
                    newrec->unk8 = rec->unk8;
                    pv = &D_800FF658[rec->unk3D];
                    dx = rec->unk0 - pv->unk0;
                    sp20 = dx;
                    pv = &D_800FF658[rec->unk3D];
                    dy = rec->unk4 - pv->unk4;
                    sp24 = dy;
                    pv = &D_800FF658[rec->unk3D];
                    dz = rec->unk8 - pv->unk8;
                    sp28 = dz;
                    pv = &D_800FF658[rec->unk3D];
                    pv->unk3E = newslot;
                    sp28 = 0;
                    sx = (s16) (dx >> 8);
                    v0_2 = dz >> 9;
                    sy = (s16) ((dy >> 8) - v0_2);
                    sp20 = sy;
                    sp24 = -(s32) sx;
                    func_8001CDAC(&sp20, out_vec, v0_2);
                    newrec->unk10 = (s16) (out_vec[0] >> 6);
                    newrec->unk12 = (s16) (out_vec[1] >> 6);
                    newrec->unk44 = 0;
                    newrec->unk48 = 0;
                    newrec->unk14 = (s16) (out_vec[2] >> 6);
                    newrec->unk3E = 0xFF;
                    newrec->unk3D = rec->unk3D;
                    rec->unk3D = newslot;
                }
            }
        }
    }

    if (arg0->owner_object_index < 2)
    {
        buf = &D_80104B58[arg0->owner_object_index << 0xA];
    }
    else
    {
        buf = D_80105358;
    }
    count = 0;
    newslot = 0;
    if (arg0->unk25 != 0)
    {
        i = 0;
        do
        {
            part = &arg0->unk0[i];
            if ((part->unk0 >> 0x15) & 1)
            {
                newslot++;
                field_interpolate_palette_track(arg0, part->unk1C, buf, (u8 *) buf + 0x200);
            }
            count++;
            i++;
        } while (count < arg0->unk25);
    }
    if (newslot != 0)
    {
        RECT rect;

        if (arg0->owner_object_index < 2)
        {
            rect.x = 0;
            rect.w = 0x10;
            rect.h = 1;
            rect.y = (arg0->owner_object_index * 2) + 0x1EF;
        }
        else
        {
            rect.y = 0x1F3;
            rect.w = 0x10;
            rect.x = 0;
            rect.h = 1;
        }
        LoadImage(&rect, (u8 *) buf + 0x200);
    }
    func_8007FFC8(arg0);
    func_8008332C(arg0, arg0->unk0, arg0->unk25);
}
