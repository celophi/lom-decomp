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
    u32 unk14; /* 0x14 (halfword view at 0x16) */
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
    u8 unk30;
    u8 unk31;
    u8 unk32;
    u8 unk33;
    u16 unk34; /* 0x34 */
    u16 pad36;
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
    u8 pad0[0x14];
    u8 unk14; /* 0x14 */
    u8 unk15; /* 0x15 */
    u8 pad16;
    u8 unk17; /* 0x17 */
    u16 unk18; /* 0x18 */
    u16 unk1A; /* 0x1A */
} FieldActorAnimationDef;

typedef struct
{
    FieldActorPartDef* unk0;
    u8 pad4[0xC - 4];
    FieldActorAnimationDef* unkC;
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
    u8 pad0[0xC];
    u32 unkC;    /* 0x0C */
    u8 pad10[0x14 - 0x10];
    s16 unk14;   /* 0x14 */
    u8 pad16[0x130 - 0x16];
    Vec2s unk130[4]; /* 0x130 */
    s16 unk140;  /* 0x140 */
    s16 unk142;  /* 0x142 */
    s16 unk144;  /* 0x144 */
    s16 unk146;  /* 0x146 */
    u8 pad148[0x174 - 0x148];
    u16 unk174;  /* 0x174 */
    u8 pad176[0x178 - 0x176];
    u32 unk178;  /* 0x178 (byte view at 0x17A) */
    u8 pad17C[0x18E - 0x17C];
    u8 unk18E;   /* 0x18E */
    u8 pad18F[0x190 - 0x18F];
    Vec2s unk190[3]; /* 0x190 */
    s32 unk19C;  /* 0x19C */
    s32 unk1A0;  /* 0x1A0 */
    u8 pad1A4[0x1A8 - 0x1A4];
    u8 unk1A8;   /* 0x1A8 */
    u8 unk1A9;   /* 0x1A9 */
    u8 unk1AA;   /* 0x1AA */
    u8 pad1AB[0x23C - 0x1AB];
} Struct_D80105AE0;

typedef struct
{
    s32 vx;
    s32 vy;
    s32 vz;
    s32 pad;
} FieldVector;

typedef struct
{
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
} FieldSVector;

typedef struct
{
    s16 m[3][3];
    s32 t[3];
} FieldMatrix;

/**
 * @brief Probe passed to func_8005B368 (see field_collision.c).
 */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
    u16 unkC;
    s16 unkE;
    u16 unk10;
} Query;

/**
 * @brief Mover state resolved by func_8005B6AC (see field_collision.c).
 */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
    s32 unk10;
    s32 unk14;
    s32 unk18;
    void* unk1C;
    s32 unk20;
    u16 unk24;
    s16 unk26;
    s32 unk28;
} Move_Mover;

typedef struct
{
    u8 pad0[0x244];
    s8 unk244;
    u8 pad245[0x268 - 0x245];
} D_800FD818_type;

typedef struct
{
    s16 unk0; /* 0x00 */
    s16 unk2; /* 0x02 */
    s16 unk4; /* 0x04 */
    u8 pad6[0xC - 6];
    s16 unkC; /* 0x0C */
} Struct_801ED400;

#include "psyq_compat/inline_c.h"
#include "psyq_compat/gte_dmpsx_compat.h"

extern s32 g_field_track_index;
extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;
extern FieldActorState g_field_actor_slots[80];
extern Struct_D800FDF58 D_800FDF58[];
extern Struct_D80105AE0 D_80105AE0[];
extern FieldVector D_80105778;
extern s32 D_80105760;
extern s32 D_800473F8;
extern s32 D_800FE754;
extern s32 D_80105764;
extern D_800FD818_type D_800FD818[];

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

/**
 * @brief Per-effect-record update: decrements the part's active-count table
 *        entry, dispatches an audio event for the owning actor, rolls a set
 *        of chained spawn effects selected by the part's flag/track fields,
 *        then syncs the owning actor's saved position/flag mirrors
 *        (D_800FDF58/D_80105AE0) when this record's part index matches one
 *        of the actor's per-slot animation-frame triggers.
 * @param rec Effect record being updated (owner/track/position fields).
 * @param part Part definition supplying flags, track selectors, and the
 *             chained-effect table used while rec->unk25 is being armed.
 * @see decomp.me (97.78%) TODO
 * @note WIP - 97.78%. Structure, control flow, and every field/array
 *       mapping are confirmed exact; the entire remaining residue is a
 *       single register-coloring swap: gcc assigns rec to s4 and the
 *       derived `&g_field_actor_slots[rec->unk22]` pointer to s3, where the
 *       target has them the other way around (s3 = rec, s4 = state). Both
 *       pseudos are close in gcc 2.7.2 global.c allocation priority
 *       (floor_log2(refs)*refs/live_len): rec came out at pri 2303 (41
 *       refs), state at pri 2388 (24 refs) - a ~3.7% gap - so state is
 *       allocated first and takes s3. The standard ALLOC-47 lever (a
 *       dedicated single-use constant local near the top, to shift live
 *       ranges) was tried with several constants/positions and always
 *       measured delta-exact 0 or negative; no C-level reshape found so far
 *       moves rec's priority above state's. A local `Struct_D80105AE0
 *       *slot` was introduced for the owner_object_index-indexed
 *       D_80105AE0 check (function-scope, block-local `slot =
 *       &D_80105AE0[state->owner_object_index];`) to stop the byte-view
 *       cast from folding the 0x17A offset into the array base constant;
 *       the same fix regresses the unk229[]-indexed sibling check three
 *       lines above it, so that one keeps the plain
 *       `((u8*)&D_80105AE0[...].unk178)[2]` cast form instead - the last
 *       (3-row) structural residue lives there.
 */
void func_80071500(Struct_D800FDF58 *rec, FieldActorPartDef *part)
{
    FieldActorState *state;
    FieldActorPartDef *track_part;
    Struct_D800FDF58 *newrec;
    Struct_D80105AE0 *slot;
    FieldVector new_pos;
    FieldVector vec;
    FieldVector sqr;
    s32 bit, mask;
    s32 newslot;
    s32 spawn_count;
    s16 anim_flags;
    u8 t;

    g_field_actor_slots[rec->unk22].unk3B[rec->unk29][part->unk32]--;

    state = &g_field_actor_slots[rec->unk22];
    field_dispatch_actor_audio_event(state, 3, rec->unk23);

    if (*(u32 *) &part->unk2C & 0xF0000000)
    {
        rec->unk25 = (rec->unk26 == -1) ? 0xFE : (u8) rec->unk26;

        for (bit = 0, mask = 1; bit < 4; bit++, mask <<= 1)
        {
            if ((*(u32 *) &part->unk2C >> 0x1C) & mask)
            {
                D_80105778.vx = rec->unk0;
                D_80105778.vy = rec->unk4;
                D_80105760 = 0;
                D_80105778.vz = rec->unk8;

                track_part = &g_field_actor_slots[rec->unk22].unk0[(part->unk34 >> (bit * 4)) & 0xF];
                spawn_count = 1;
                if (((track_part->unk28 >> 0x12) & 0x3F) == 0x35)
                {
                    if (track_part->unkC != 0)
                    {
                        spawn_count = track_part->unkC;
                    }
                }

                if (spawn_count != 0)
                {
                    do
                    {
                        newslot = func_8006D79C(&g_field_actor_slots[rec->unk22], (part->unk34 >> (bit * 4)) & 0xF, 0);
                        if (newslot != -1)
                        {
                            newrec = &D_800FF658[newslot];
                            if (!(((u8 *) &newrec->unk1C)[3] & 7) && (newrec->unk1B != 0))
                            {
                                func_80073F7C(newrec, part, &new_pos);
                                vec.vx = (new_pos.vx - newrec->unk0) >> 8;
                                vec.vy = (new_pos.vy - newrec->unk4) >> 8;
                                vec.vz = (new_pos.vz - newrec->unk8) >> 8;

                                gte_ldlvl(&vec);
                                gte_sqr0();
                                gte_stlvnl(&sqr);

                                newrec->unk12 = ratan2(-vec.vz, vec.vx);
                                if (newrec->unk12 < 0)
                                {
                                    newrec->unk12 += 0x1000;
                                }

                                if (vec.vy != 0)
                                {
                                    newrec->unk14 = ratan2(SquareRoot0(sqr.vx + sqr.vz), -vec.vy);
                                }
                                else
                                {
                                    newrec->unk14 = 0x400;
                                }
                                if (newrec->unk14 < 0)
                                {
                                    newrec->unk14 += 0x1000;
                                }
                                newrec->unk10 = 0;
                            }
                        }
                        spawn_count--;
                    } while (spawn_count != 0);
                }
            }
        }
    }
    rec->unk25 = 0xFF;

    if (((state->unkC->unk18 & 0x14) == 0x14) && ((state->unkC->unk1A >> 0xD) == rec->unk23))
    {
        if (state->unk229[rec->unk29] != 0xFF)
        {
            if (((u8 *) &D_80105AE0[state->unk229[rec->unk29]].unk178)[2] == state->unk233)
            {
                anim_flags = D_800FDF58[state->unk229[rec->unk29]].unk2A;
                if ((anim_flags != 0x90 && anim_flags != 0x94) || (D_80105AE0[state->unk229[rec->unk29]].unkC & 0x200))
                {
                    D_800FDF58[state->unk229[rec->unk29]].unk25 = 0;
                }
                else
                {
                    D_800FDF58[state->unk229[rec->unk29]].unk25 = 0xFE;
                }
                D_80105AE0[state->unk229[rec->unk29]].unk178 &= ~1;
            }
        }
    }

    if (((state->unkC->unk18 & 0xA) == 0xA) && (((state->unkC->unk1A >> 0xA) & 7) == rec->unk23))
    {
        slot = &D_80105AE0[state->owner_object_index];
        if (((u8 *) &slot->unk178)[2] == state->unk233)
        {
            anim_flags = D_800FDF58[state->owner_object_index].unk2A;
            if ((anim_flags != 0x90 && anim_flags != 0x94) || (slot->unkC & 0x200))
            {
                D_800FDF58[state->owner_object_index].unk25 = 0;
            }
            else
            {
                D_800FDF58[state->owner_object_index].unk25 = 0xFE;
            }
            D_80105AE0[state->owner_object_index].unk178 &= ~1;
        }
    }

    if (rec->unk23 == ((state->unkC->unk1A & 0x1F) - 1))
    {
        D_800FDF58[state->owner_object_index].unk0 = rec->unk0;
        D_800FDF58[state->owner_object_index].unk4 = rec->unk4;
        D_800FDF58[state->owner_object_index].unk8 = rec->unk8;
        D_800FDF58[state->owner_object_index].unk4 = 0;
        D_800FDF58[state->owner_object_index].unk21 =
            (D_800FDF58[state->owner_object_index].unk21 & 0x7F) | (rec->unk21 & 0x80);
    }

    if (rec->unk23 == (((state->unkC->unk1A >> 5) & 0x1F) - 1))
    {
        if (state->unk229[rec->unk29] != 0xFF)
        {
            D_800FDF58[state->unk229[rec->unk29]].unk0 = rec->unk0;
            D_800FDF58[state->unk229[rec->unk29]].unk4 = rec->unk4;
            D_800FDF58[state->unk229[rec->unk29]].unk8 = rec->unk8;
            D_800FDF58[state->unk229[rec->unk29]].unk4 = 0;
            D_800FDF58[state->unk229[rec->unk29]].unk21 =
                (D_800FDF58[state->unk229[rec->unk29]].unk21 & 0x7F) | (rec->unk21 & 0x80);
        }
    }

    if (rec->unk26 == 5)
    {
        t = rec->unk3E;
        if (t != 0xFF)
        {
            D_800FF658[t].unk3D = 0xFF;
        }
    }
}

/**
 * @brief Per-effect-record parameter-track/placement update: rolls angle and
 *        scale tracks, resolves the effect's world position through one of a
 *        large table of placement opcodes (owner/tracked-object relative,
 *        camera-relative, chained-effect-relative, or absolute), applies
 *        motion/collision snapping through func_8005B6AC and func_8005B368,
 *        derives heading/pitch from the resulting delta, and on completion
 *        dispatches any chained spawn/audio side effects for the owning
 *        actor.
 * @param rec Effect record being updated.
 * @param part Part definition supplying flags, placement opcode, and track
 *             selectors.
 * @param actor Actor that owns the part/track data referenced by rec.
 * @see decomp.me WIP
 * @note WIP - 73.35% at time of writing (2139-insn target, one of the
 *       largest functions in this overlay). Structure is broadly right
 *       (5 GTE rotate/translate blocks via RotMatrix_gte/gte_SetRotMatrix/
 *       gte_rtv0, the ~30-way placement-opcode switch, the nested nudge
 *       switch on 31-34) but two classes of residue remain unresolved:
 *       (1) the stack frame is 0x20 bytes larger than the target's
 *       (-0x98 vs -0x78), and the saved-register home slots are shifted to
 *       match - some locals (likely the func_80073F7C output triplets
 *       around `new_pos`/`sp40`) need reshaping into the target's tighter
 *       layout, not chasing register colors first.
 *       (2) several CSE-FOLD spots where the target re-reads
 *       D_80105AE0/D_800FF658/D_800FDF58 through a fresh idx*stride
 *       computation where this draft reuses an already-computed pointer
 *       (same mechanism documented on func_80071500 above) - likely present
 *       in the placement-opcode switch's shared tail and the case 31-34
 *       nudge handlers.
 *       Not yet attempted: permuter, sched_oracle/crossjump_oracle on the
 *       large structural runs, or idiom_harvest.
 */
void func_80071D40(Struct_D800FDF58 *rec, FieldActorPartDef *part, FieldActorState *actor)
{
    Struct_801ED400 *sp48 = (Struct_801ED400 *) 0x801ED480;
    Struct_801ED400 *sp44 = (Struct_801ED400 *) 0x801ED400;
    s32 *sp40 = (s32 *) 0x1F800020;
    FieldVector *vec = (FieldVector *) 0x1F800000;
    FieldVector *sqr = (FieldVector *) 0x1F800010;
    FieldSVector *dir = (FieldSVector *) 0x1F800030;
    FieldSVector *dest = (FieldSVector *) 0x1F800038;
    FieldMatrix *mtx = (FieldMatrix *) 0x1F800040;
    Move_Mover *mover = (Move_Mover *) 0x1F800080;
    Query *query = (Query *) 0x1F8000C0;
    Struct_D800FDF58 *frec;
    Struct_D80105AE0 *fslot;
    s32 flags;
    s32 opcode;
    s32 x, y, z;
    s16 heading;
    s16 pitch;
    s32 dx, dy, dz;
    s32 vsub;
    s32 kind;
    s32 slot;
    u8 old25;
    u8 tflags;

    flags = part->unk24;
    if (flags & 0x800000)
    {
        rec->unk1C = (rec->unk1C & 0xFF7FFFFF) | ((field_evaluate_parameter_track_at_time(actor, (flags >> 0x19) & 0xF, rec->unk2C) != 0) << 0x17);
    }
    if ((rec->unk1C & 0x60000000) == 0x40000000)
    {
        rec->unk2A = field_evaluate_parameter_track_at_time(actor, ((s16 *) &part->unk14)[1] & 0xF, rec->unk2C);
    }

    if (rec->unk25 == 5)
    {
        func_8007D078(rec, part, mtx, actor);
        gte_SetRotMatrix(mtx);
        dir->unk2 = 0;
        dir->unk0 = rec->unk12;
        dir->unk4 = rec->unk10;
        gte_ldv0(dir);
        gte_rtv0();
        gte_stlvnl(sqr);
        rec->unk44 = sqr->vx;
        rec->unk48 = sqr->vy;
        return;
    }

    flags = rec->unk1C;
    if ((flags & 0x07000000) == 0x05000000)
    {
        rec->unk20 = rec->unk20 + rec->unk2A;
        func_800A1D48(&rec->unk20, rec, rec->unk39);
        goto block_229;
    }

    if ((u32) ((flags >> 0x18) & 7) >= 2U)
    {
        if ((((u32) part->unk28 >> 0x1A) & 3) == 2)
        {
            rec->unk32 = field_evaluate_parameter_track_at_time(actor, part->unk21 & 0xF, rec->unk2C);
        }
        if ((((u32) part->unk28 >> 0x1C) & 3) == 2)
        {
            rec->unk33 = field_evaluate_parameter_track_at_time(actor, part->unk22 & 0xF, rec->unk2C);
        }
        if ((rec->unk1C & 0x600) == 0x400)
        {
            heading = -field_evaluate_parameter_track_at_time(actor, (((part->unk20 & 0x3F) * 8) | (*(u32 *) &part->unk1C >> 0x1D)) & 0xF, rec->unk2C);
        }
        else
        {
            heading = -((u16) rec->unk1C & 0x1FF);
        }
        dir->unk2 = heading;
        if (part->unk1C & 0x01000000)
        {
            dir->unk2 = (s16) ((D_80105AE0[actor->owner_object_index].unk174 & 0x3FF) * (s16) (u16) dir->unk2 / 100);
        }
        flags = part->unk28;
        kind = (flags >> 0x12) & 0x3F;
        if (kind < 0x14)
        {
            vsub = flags >> 0x11;
            if ((flags >> 0x10) & 1)
            {
                if ((u32) (kind - 0xA) >= 0x1CU)
                {
                    slot = actor->owner_object_index;
                }
                else
                {
                    slot = actor->unk229[g_field_track_index];
                }
                fslot = &D_80105AE0[slot];
                dy = (fslot->unk144 - fslot->unk140) >> 1;
                if (dy < 0)
                {
                    dy = -dy;
                }
                dir->unk2 = (s16) dir->unk2 - dy;
                vsub = (u32) part->unk28 >> 0x11;
            }
            if (vsub & 1)
            {
                if ((u32) (((part->unk28 >> 0x12) & 0x3F) - 0xA) >= 0x1CU)
                {
                    slot = actor->owner_object_index;
                }
                else
                {
                    slot = actor->unk229[g_field_track_index];
                }
                fslot = &D_80105AE0[slot];
                dy = (fslot->unk146 - fslot->unk142) >> 1;
                if (dy < 0)
                {
                    dy = -dy;
                }
                dir->unk2 = (s16) dir->unk2 - dy;
            }
        }
        dir->unk0 = 0;
        dir->unk4 = 0;
        rec->unk12 = rec->unk12 + rec->unk2A;
        RotMatrix_gte((FieldSVector *) &rec->unk10, mtx);
        RotMatrixZ(rec->unk32 * 0x10, mtx);
        RotMatrixY(rec->unk33 * 0x10, mtx);
        gte_SetRotMatrix(mtx);
        gte_ldv0(dir);
        gte_rtv0();
        gte_stsv(dest);

        opcode = ((u32) part->unk28 >> 0x12) & 0x3F;
        switch (opcode)
        {
            case 0x0: case 0x1: case 0x2: case 0x3: case 0x4:
            case 0x5: case 0x6: case 0x7: case 0x8: case 0x9:
            case 0xA: case 0xB: case 0xC: case 0xD: case 0xE:
            case 0xF: case 0x10: case 0x11: case 0x12: case 0x13:
            {
                s32 sub;
                s32 a2v, a3v;

                if ((s32) opcode >= 0xA)
                {
                    slot = actor->unk229[g_field_track_index];
                    sub = opcode - 0xA;
                }
                else
                {
                    slot = actor->owner_object_index;
                    sub = opcode;
                }
                frec = &D_800FDF58[slot];
                fslot = &D_80105AE0[slot];
                a3v = 0;
                a2v = 0;
                if ((((&D_80105AE0[actor->owner_object_index])->unk178 & 1)) && ((u8) actor->unk233 >= 0x40U))
                {
                    a2v = 0;
                    if (!((((u32) (&D_80105AE0[actor->owner_object_index])->unk178 >> 5) & 1)))
                    {
                        a3v = 0x800000;
                        a2v = 0x800000;
                        sub = -1;
                    }
                }
                switch (sub)
                {
                    case 1:
                        a2v = (fslot->unk144 + fslot->unk140) >> 1;
                        a3v = (fslot->unk146 + fslot->unk142) >> 1;
                        break;
                    case 2:
                        a3v = 0;
                        a2v = (fslot->unk144 + fslot->unk140) >> 1;
                        break;
                    case 3:
                        a3v = fslot->unk142;
                        a2v = (fslot->unk144 + fslot->unk140) >> 1;
                        break;
                    case 4:
                        a2v = fslot->unk140;
                        a3v = (fslot->unk146 + fslot->unk142) >> 1;
                        break;
                    case 5:
                        a2v = fslot->unk144;
                        a3v = (fslot->unk146 + fslot->unk142) >> 1;
                        break;
                    case 6:
                        a2v = fslot->unk140;
                        a3v = fslot->unk142;
                        break;
                    case 7:
                        a2v = fslot->unk144;
                        a3v = fslot->unk142;
                        break;
                    case 8:
                        a2v = fslot->unk140;
                        a3v = fslot->unk146;
                        break;
                    case 9:
                        a2v = fslot->unk144;
                        a3v = fslot->unk146;
                        break;
                }
                vec->vx = frec->unk0 + (a2v << 8);
                vec->vy = frec->unk4 + (a3v << 8);
                vec->vz = frec->unk8;
                goto case_0x26;
            }
            case 0x26:
            case_0x26:
block_114:
                rec->unk0 = ((s16) dest->unk0 << 8) + vec->vx;
                rec->unk4 = (((s32) (dest->unk2 << 0x10)) >> 8) + vec->vy;
                z = (((s32) (dest->unk4 << 0x10)) >> 8) + vec->vz;
                rec->unk8 = z;
                if (part->unk28 & 1)
                {
                    rec->unk8 = z + 0x80;
                }
                flags = part->unk28;
                if ((flags >> 3) & 1)
                {
                    if (part->unk34 & 0x80000)
                    {
                        rec->unk4 = (rec->unk26 - field_evaluate_parameter_track_at_time(actor, (flags >> 4) & 0xF, rec->unk2C)) << 8;
                    }
                    else
                    {
                        rec->unk4 = field_evaluate_parameter_track_at_time(actor, (flags >> 4) & 0xF, rec->unk2C) * -0x100;
                    }
                }
                kind = ((u8 *) &rec->unk1C)[3] & 7;
                switch (kind)
                {
                    case 3:
                        rec->unk4 = rec->unk4 + (rec->unk2C << 9);
                        break;
                    case 4:
                        rec->unk4 = rec->unk4 - (rec->unk2C << 9);
                        break;
                }
                if ((((u32) part->unk4 >> 2) & 1) && ((rec->unk14 + part->unk30) < 0x800))
                {
                    rec->unk14 = (u16) rec->unk14 + part->unk30;
                }
                goto block_229;
            case 0x14: case 0x15: case 0x16: case 0x17: case 0x18:
            case 0x19: case 0x1A: case 0x1B: case 0x2A: case 0x2B:
            case 0x2C: case 0x2D: case 0x2E: case 0x2F: case 0x30:
            case 0x31:
                frec = &D_800FF658[rec->unk30];
                old25 = frec->unk25;
                if (old25 != 0xFF)
                {
                    vec->vx = frec->unk0;
                    vec->vy = D_800FF658[rec->unk30].unk4;
                    vec->vz = D_800FF658[rec->unk30].unk8;
                    if (!(part->unk14 & 8))
                    {
                        RotMatrix_gte((FieldSVector *) &D_800FF658[rec->unk30].unk10, mtx);
                        gte_SetRotMatrix(mtx);
                        gte_ldv0(dest);
                        gte_rtv0();
                        gte_stsv(dir);
                        dest->unk0 = dir->unk0;
                        dest->unk2 = dir->unk2;
                        dest->unk4 = dir->unk4;
                    }
                    goto block_114;
                }
                rec->unk25 = old25;
                return;
            case 0x25:
                vec->vx = (part->unk38 << 8) - D_800F22A0;
                vec->vy = (part->unk3A << 8) - D_800F22A4;
                vec->vz = (part->unk3C << 8) - D_800F22A8;
                goto block_114;
            case 0x1D:
                vec->vx = 0;
                vec->vy = -0x7000;
                vec->vz = 0;
                goto block_114;
            case 0x1E:
                vec->vx = 0;
                vec->vy = 0x7000;
                vec->vz = 0;
                goto block_114;
            case 0x1F:
                vec->vx = 0xFFFF6000;
                vec->vy = 0;
                vec->vz = 0;
                goto block_114;
            case 0x20:
                vec->vx = 0xA000;
                vec->vy = 0;
                vec->vz = 0;
                goto block_114;
            case 0x21:
                vec->vx = 0xFFFF6000;
                vec->vy = -0x7000;
                vec->vz = 0;
                goto block_114;
            case 0x22:
                vec->vx = 0xA000;
                vec->vy = -0x7000;
                vec->vz = 0;
                goto block_114;
            case 0x23:
                vec->vx = 0xFFFF6000;
                vec->vy = 0x7000;
                vec->vz = 0;
                goto block_114;
            case 0x24:
                vec->vx = 0xA000;
                vec->vy = 0x7000;
                vec->vz = 0;
                goto block_114;
            case 0x27:
                frec = &D_800FDF58[actor->owner_object_index];
                if ((((u32) part->unk28 >> 0xA) & 1) && !(frec->unk21 & 0x80))
                {
                    vec->vx = frec->unk0 - (part->unk38 << 8);
                }
                else
                {
                    vec->vx = frec->unk0 + (part->unk38 << 8);
                }
                vec->vy = frec->unk4 + (part->unk3A << 8);
                vec->vz = frec->unk8 + (part->unk3C << 8);
                goto block_114;
            case 0x28:
                frec = &D_800FDF58[actor->unk229[g_field_track_index]];
                if ((part->unk34 & 0x08000000) && !(D_800FDF58[actor->owner_object_index].unk21 & 0x80))
                {
                    vec->vx = frec->unk0 - (part->unk38 << 8);
                }
                else if ((((u32) part->unk28 >> 0xA) & 1) && !(frec->unk21 & 0x80))
                {
                    vec->vx = frec->unk0 - (part->unk38 << 8);
                }
                else
                {
                    vec->vx = frec->unk0 + (part->unk38 << 8);
                }
                vec->vy = frec->unk4 + (part->unk3A << 8);
                vec->vz = frec->unk8 + (part->unk3C << 8);
                goto block_114;
            case 0x29:
                frec = &D_800FDF58[actor->owner_object_index];
                fslot = &D_80105AE0[actor->owner_object_index];
                vec->vx = frec->unk0;
                vec->vy = frec->unk4 + (fslot->unk130[(part->unk24 >> 0x15) & 3].y << 8);
                vec->vz = frec->unk8;
                vec->vx += fslot->unk130[(part->unk24 >> 0x15) & 3].x << 8;
                goto block_114;
            case 0x32:
                frec = &D_800FDF58[actor->owner_object_index];
                fslot = &D_80105AE0[actor->owner_object_index];
                vec->vx = frec->unk0 + (fslot->unk130[(part->unk24 >> 0x15) & 3].x << 8);
                vec->vy = frec->unk4;
                vec->vz = frec->unk8 + (part->unk3C << 8);
                if ((((u32) part->unk28 >> 0xA) & 1) && !(frec->unk21 & 0x80))
                {
                    vec->vx -= part->unk38 << 8;
                }
                else
                {
                    vec->vx += part->unk38 << 8;
                }
                goto block_114;
            case 0x33:
                frec = &D_800FDF58[actor->owner_object_index];
                fslot = &D_80105AE0[actor->owner_object_index];
                vec->vx = frec->unk0 + (fslot->unk190[(rec->unk1C >> 0xD) & 3].x << 8);
                vec->vy = frec->unk4;
                vec->vz = frec->unk8 + (fslot->unk190[(rec->unk1C >> 0xD) & 3].y << 8);
                goto block_114;
            case 0x34:
                frec = &D_800FDF58[actor->unk229[g_field_track_index]];
                if (!(frec->unk21 & 0x80))
                {
                    vec->vx = frec->unk0 + (actor->unk1FE[g_field_track_index].x << 8);
                }
                else
                {
                    vec->vx = frec->unk0 - (actor->unk1FE[g_field_track_index].x << 8);
                }
                vec->vy = frec->unk4 + (actor->unk1FE[g_field_track_index].y << 8);
                vec->vz = frec->unk8;
                goto block_114;
            case 0x36:
                vec->vx = part->unk38 << 8;
                vec->vy = part->unk3A << 8;
                vec->vz = part->unk3C << 8;
                goto block_114;
            case 0x37: case 0x38: case 0x39: case 0x3A: case 0x3B:
            case 0x3C: case 0x3D: case 0x3E:
                frec = &D_800FF658[rec->unk30];
                old25 = frec->unk25;
                if (old25 == 0xFF)
                {
                    rec->unk25 = old25;
                    return;
                }
                vec->vx = frec->unk0;
                vec->vy = frec->unk4;
                vec->vz = frec->unk8;
                if ((part->unk34 & 0x08000000) && !(D_800FDF58[actor->owner_object_index].unk21 & 0x80))
                {
                    vec->vx -= part->unk38 << 8;
                }
                else if ((((u32) part->unk28 >> 0xA) & 1) && !(frec->unk21 & 0x80))
                {
                    vec->vx -= part->unk38 << 8;
                }
                else
                {
                    vec->vx += part->unk38 << 8;
                }
                vec->vy += part->unk3A << 8;
                vec->vz += part->unk3C << 8;
                if (!(part->unk14 & 8))
                {
                    RotMatrix_gte((FieldSVector *) &D_800FF658[rec->unk30].unk10, mtx);
                    gte_SetRotMatrix(mtx);
                    gte_ldv0(dir);
                    gte_rtv0();
                    gte_stsv(dir);
                    dest->unk0 = dir->unk0;
                    dest->unk2 = dir->unk2;
                    dest->unk4 = dir->unk4;
                    goto block_114;
                }
                goto block_114;
            default:
                vec->pad = 0;
                vec->vz = 0;
                vec->vy = 0;
                vec->vx = 0;
                goto block_114;
        }
    }
    else
    {
        dest->unk4 = 0;
        dest->unk0 = 0;
        dest->unk2 = (s16) (rec->unk2A * -4);
        RotMatrix_gte((FieldSVector *) &rec->unk10, mtx);
        if (!(((u32) part->unk4 >> 2) & 1) && (rec->unk1B == 0))
        {
            RotMatrixZ(rec->unk32 * 0x10, mtx);
            RotMatrixY(rec->unk33 * 0x10, mtx);
        }
        gte_SetRotMatrix(mtx);
        gte_ldv0(dest);
        gte_rtv0();
        gte_stsv(dest);

        if ((((rec->unk1C & 0x60000000) != 0x40000000) || ((s16) rec->unk2A != 0)) && ((((u32) part->unk4 >> 2) & 1) || ((s32) part->unk28 < 0)))
        {
            s32 temp_s0;
            s16 new_heading;
            s32 new_scale;

            temp_s0 = rcos(rec->unk14) - (part->unk30 * 8);
            new_heading = ratan2(rsin(rec->unk14), temp_s0);
            rec->unk14 = new_heading;
            if (new_heading < 0x400)
            {
                new_scale = (((s16) rec->unk2A * 0xF) >> 4) - 1;
            }
            else
            {
                new_scale = (((s16) rec->unk2A << 5) / 30) + 1;
            }
            rec->unk2A = new_scale;
            if (((s16) rec->unk2A < 0x14) && (rec->unk14 < 0x400))
            {
                rec->unk14 = 0x800 - (u16) rec->unk14;
                rec->unk2A = 0x14;
            }
        }

        if ((part->unk24 & 0x100000) && !(part->unk34 & 0x800000))
        {
            x = rec->unk0;
            if ((x < 0) || (x >= (sp48->unk0 << 8)) || ((z = rec->unk8), (z < 0)) || (z >= ((s32) (sp48->unk2 << 0x10) >> 7)))
            {
                if (!(part->unk34 & 0x10000000))
                {
                    dest->unk0 = 0;
                    dest->unk4 = 0;
                }
                else
                {
                    goto block_152;
                }
            }
            else
            {
                mover->unk4 = 0;
                mover->unk0 = rec->unk0;
                dz = rec->unk8;
                heading = dest->unk0;
                D_800473F8 += 0x100;
                mover->unk28 = 8;
                mover->unk10 = 0;
                mover->unk24 = 0xC;
                mover->unk26 = 0x10;
                mover->unk20 = 0;
                mover->unk14 = dest->unk4;
                mover->unkC = heading;
                mover->unk1C = (void *) -2;
                mover->unk8 = dz;
                mover->unk28 = mover->unk28 & 0xFFFDFFFF & 0xFFFEFFFF;
                if (func_8005B6AC(mover, &D_800473F8, heading, dz) & 3)
                {
                    dest->unk4 = 0;
                    dest->unk0 = 0;
                    if (part->unk34 & 0x10000000)
                    {
block_152:
                        rec->unk25 = 0xFF;
                    }
                }
                else if (!(part->unk34 & 0x10000000))
                {
                    query->unkC = 0xC;
                    query->unkE = 0x10;
                    query->unk10 = 8;
                    query->x = mover->unk0;
                    query->z = mover->unk8;
                    query->y = rec->unk4;
                    if ((D_800FE754 != 0) && (func_8005B368(query) != -1))
                    {
                        dest->unk4 = 0;
                        dest->unk0 = 0;
                    }
                    else
                    {
                        dest->unk0 = (u16) mover->unk0 - (u16) rec->unk0;
                        dest->unk4 = (u16) mover->unk8 - (u16) rec->unk8;
                    }
                }
            }
        }

        if (part->unk34 & 0x01000000)
        {
            dx = rec->unk0 + (s16) dest->unk0;
            dz = -sp44->unk4;
            if (((dz + 0x500) < dx) && (dx < (dz + 0x13B00)))
            {
                dz = rec->unk8 + (s16) dest->unk4;
                x = -sp44->unkC;
                if ((x < dz) && (dz < (x + 0x1E800)))
                {
                    rec->unk0 = dx;
                    goto block_165;
                }
            }
        }
        else
        {
            rec->unk0 = rec->unk0 + (s16) dest->unk0;
block_165:
            rec->unk8 = rec->unk8 + (s16) dest->unk4;
        }
        y = rec->unk4 + (s16) dest->unk2;
        rec->unk4 = y;
        if ((part->unk34 & 0x02000000) && (y >= 0) && (((flags = rec->unk1C, kind = flags & 0x60000000), (kind == 0)) || (kind == 0x40000000)))
        {
            rec->unk1C = flags & 0x9FFFFFFF;
            rec->unk4 = 0;
            rec->unk2A = 0;
        }
        flags = part->unk28;
        if ((flags >> 3) & 1)
        {
            if (part->unk34 & 0x80000)
            {
                rec->unk4 = (rec->unk26 - field_evaluate_parameter_track_at_time(actor, (flags >> 4) & 0xF, rec->unk2C)) << 8;
            }
            else
            {
                rec->unk4 = field_evaluate_parameter_track_at_time(actor, (flags >> 4) & 0xF, rec->unk2C) * -0x100;
            }
        }

        if (rec->unk1B != 0)
        {
            s32 vy2, vz2;

            func_80073F7C(rec, part, sp40);
            sqr->vx = (rec->unk0 - sp40[0]) >> 8;
            sqr->vy = (rec->unk4 - sp40[1]) >> 8;
            vz2 = (rec->unk8 - sp40[2]) >> 8;
            sqr->vz = vz2;
            if (((u32) (sqr->vx + 0xF) < 0x1FU) && (vz2 >= -0xF) && (vz2 < 0x10))
            {
                vy2 = sqr->vy;
                if ((vy2 >= -0xF) && (vy2 < 0x10))
                {
                    if ((((u32) part->unk4 >> 4) & 3) == 2)
                    {
                        tflags = rec->unk25;
                        rec->unk25 = 0xFF;
                        rec->unk26 = (s8) tflags;
                        return;
                    }
                    if (!(part->unk14 & 8))
                    {
                        rec->unk2A = 0;
                        rec->unk1C = rec->unk1C & 0x9FFFFFFF;
                    }
                    goto block_185;
                }
            }
block_185:
            if (!(rec->unk1C & 0x07000000) && !(part->unk14 & 8))
            {
                s32 new_pos[3];
                FieldVector delta;
                s16 v0_5;

                func_80073F7C(rec, part, new_pos);
                delta.vx = (new_pos[0] - rec->unk0) >> 8;
                delta.vy = (new_pos[1] - rec->unk4) >> 8;
                delta.vz = (new_pos[2] - rec->unk8) >> 8;
                gte_ldlvl(&delta);
                gte_sqr0();
                gte_stlvnl(sqr);
                v0_5 = ratan2(-delta.vz, delta.vx);
                rec->unk12 = v0_5;
                if (v0_5 & 0x8000)
                {
                    rec->unk12 = v0_5 + 0x1000;
                }
                if (delta.vy != 0)
                {
                    rec->unk14 = ratan2(SquareRoot0(sqr->vx + sqr->vz), -delta.vy);
                }
                else
                {
                    rec->unk14 = 0x400;
                }
                if (rec->unk14 < 0)
                {
                    rec->unk14 = (u16) rec->unk14 + 0x1000;
                }
                rec->unk10 = 0;
            }
            if ((rec->unk1C & 0x07000000) == 0x01000000)
            {
                s16 base_heading;
                s32 diff;
                s16 new_pitch;

                gte_ldlvl(sqr);
                gte_sqr0();
                gte_stlvnl((FieldVector *) sp40);
                pitch = ratan2(sqr->vz, -sqr->vx);
                if (pitch < 0)
                {
                    pitch += 0x1000;
                }
                if (rec->unk2C == part->unk11)
                {
                    rec->unk1C = rec->unk1C & 0xF8FFFFFF;
                    if (!(part->unk14 & 8))
                    {
                        rec->unk12 = (u16) pitch;
                        if (sqr->vz != 0)
                        {
                            rec->unk14 = ratan2(SquareRoot0(sp40[0] + sp40[2]), sqr->vy);
                        }
                        else
                        {
                            rec->unk14 = 0x400;
                        }
                        if (rec->unk14 < 0)
                        {
                            rec->unk14 = (u16) rec->unk14 + 0x1000;
                        }
                    }
                }
                else
                {
                    base_heading = (s16) rec->unk12;
                    diff = (pitch - base_heading) & 0xFFF;
                    if ((u32) (diff - 8) >= 0xFF1U)
                    {
                        rec->unk12 = (u16) pitch;
                    }
                    else
                    {
                        if (diff >= 0x801)
                        {
                            s32 back = 0x1000 - diff;
                            if (back < 0x200)
                            {
                                new_pitch = base_heading - (back >> 2);
                            }
                            else
                            {
                                new_pitch = base_heading - 0x80;
                            }
                        }
                        else if (diff < 0x200)
                        {
                            new_pitch = base_heading + (diff >> 2);
                        }
                        else
                        {
                            new_pitch = base_heading + 0x80;
                        }
                        rec->unk12 = new_pitch;
                    }
                    if ((s16) rec->unk12 < 0)
                    {
                        rec->unk12 = (u16) rec->unk12 + 0x1000;
                    }
                    {
                        s16 target_pitch;
                        s16 cur_pitch;
                        s32 pd;

                        target_pitch = ratan2(SquareRoot0(sp40[0] + sp40[2]), sqr->vy);
                        cur_pitch = rec->unk14;
                        if (target_pitch < cur_pitch)
                        {
                            pd = cur_pitch - target_pitch;
                            if (pd < 0x200)
                            {
                                rec->unk14 = cur_pitch - (pd >> 2);
                            }
                            else
                            {
                                rec->unk14 = cur_pitch - 0x80;
                            }
                        }
                        else if (cur_pitch < target_pitch)
                        {
                            pd = target_pitch - cur_pitch;
                            if (pd < 0x200)
                            {
                                rec->unk14 = cur_pitch + (pd >> 2);
                            }
                            else
                            {
                                rec->unk14 = cur_pitch + 0x80;
                            }
                        }
                        if (rec->unk14 < 0)
                        {
                            rec->unk14 = (u16) rec->unk14 + 0x1000;
                        }
                        rec->unk10 = 0;
                    }
                }
            }
            goto block_229;
        }
block_229:
        if (!(actor->unk224 & 1))
        {
            kind = ((u16 *) &actor->unk224)[1];
            switch (kind)
            {
                case 31:
                    if ((u16) actor->unk1EC[0] >= 0x10U)
                    {
                        s32 newidx = func_8009980C(rec, 5, actor, 0);
                        if (newidx != -1)
                        {
                            Struct_D80105AE0 *ns;
                            s32 idxlo, idxlo2;
                            s32 useidx;
                            s32 useidx2;

                            if (D_80105764 == 0)
                            {
                                func_800A3938(0x1F, func_8006CE70(rec->unk3A));
                            }
                            D_80105764 = 1;
                            rec->unk25 = 0xFF;
                            ns = &D_80105AE0[newidx];
                            func_80073F60(ns->unk14, D_80105AE0[actor->owner_object_index].unk14, rec->unk21 - 0x16);
                            idxlo = newidx < 3 ? newidx : 2;
                            idxlo2 = idxlo;
                            useidx = newidx;
                            useidx2 = newidx < 3 ? newidx : 2;
                            D_800FD818[idxlo2].unk244 = (&D_800FD818[useidx2].unk244)[rec->unk21] + 1;
                            func_800C0B40(ns->unk14, D_80105AE0[actor->owner_object_index].unk14, rec->unk21 - 0x16, D_800FD818);
                            func_80073EAC(rec);
                            return;
                        }
                    }
                    goto block_264;
                case 32:
                    if ((u16) actor->unk1EC[0] >= 0x10U)
                    {
                        s32 newidx = func_8009980C(rec, 5, actor, 0);
                        if (newidx != -1)
                        {
                            Struct_D80105AE0 *ns;

                            if (D_80105764 == 0)
                            {
                                func_800A3938(0x1F, func_8006CE70(rec->unk3A));
                            }
                            D_80105764 = 1;
                            rec->unk25 = 0xFF;
                            ns = &D_80105AE0[newidx];
                            func_80073F60(ns->unk14, D_80105AE0[actor->owner_object_index].unk14, 4);
                            func_800C0B40(ns->unk14, D_80105AE0[actor->owner_object_index].unk14, 4, D_800FD818);
                            func_80073EAC(rec);
                            return;
                        }
                    }
                    goto block_264;
                case 33:
                    if ((u16) actor->unk1EC[0] >= 0x10U)
                    {
                        s32 newidx = func_8009980C(rec, 5, actor, 0);
                        if (newidx != -1)
                        {
                            Struct_D80105AE0 *ns;

                            if (D_80105764 == 0)
                            {
                                func_800A3938(0x1F, func_8006CE70(rec->unk3A));
                            }
                            D_80105764 = 1;
                            rec->unk25 = 0xFF;
                            ns = &D_80105AE0[newidx];
                            func_80073F60(ns->unk14, D_80105AE0[actor->owner_object_index].unk14, 5);
                            func_800C0B40(ns->unk14, D_80105AE0[actor->owner_object_index].unk14, 5, D_800FD818);
                            func_80092C24(&D_800FDF58[newidx], 0x2C);
                            func_80073EAC(rec);
                            return;
                        }
                    }
                    goto block_264;
                case 34:
                    if ((u16) actor->unk1EC[0] >= 0x10U)
                    {
                        s32 newidx = func_8009980C(rec, 5, actor, 0);
                        if (newidx != -1)
                        {
                            Struct_D80105AE0 *ns;

                            if (D_80105764 == 0)
                            {
                                func_800A3938(0x1F, func_8006CE70(rec->unk3A));
                            }
                            D_80105764 = 1;
                            rec->unk25 = 0xFF;
                            ns = &D_80105AE0[newidx];
                            func_80073F60(ns->unk14, D_80105AE0[actor->owner_object_index].unk14, 6);
                            func_800C0B40(ns->unk14, D_80105AE0[actor->owner_object_index].unk14, 6, D_800FD818);
                            func_80092C24(&D_800FDF58[newidx], 0x2D);
                            func_80073EAC(rec);
                            return;
                        }
                    }
                    goto block_264;
                default:
                    goto block_264;
            }
        }
        else
        {
block_264:
            {
                FieldActorAnimationDef *anim = actor->unkC;
                if ((anim->unk14 == 1) && (anim->unk15 == rec->unk23))
                {
                    func_80099018(rec, anim->unk17, actor);
                }
            }
            if (((u32) part->unk4 >> 6) & 1)
            {
                s32 v4 = rec->unk4;
                if (v4 > 0)
                {
                    u16 old14;
                    s16 old2A;

                    rec->unk4 = -v4;
                    old14 = (u16) rec->unk14;
                    old2A = rec->unk2A;
                    rec->unk14 = 0x800 - old14;
                    rec->unk2A = (s16) (old2A + ((u32) (old2A << 0x10) >> 0x1F)) >> 1;
                    field_dispatch_actor_audio_event(actor, 4, rec->unk23, old14);
                }
            }
            rec->unk2A = (s16) (rec->unk2A * rec->unk2E) >> 8;
        }
    }
}
