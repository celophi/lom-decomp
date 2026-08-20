/**
 * @file field14.c
 * @brief Field effect-record primitive builder, carved from the top of the
 *        unk2 segment (the single-function slot right after field13.c's
 *        func_8007AA2C).
 */

#include "common.h"

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
    s32 unk1C; /* 0x1C (halfword view at 0x1E) */
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
    u32 unk1C; /* 0x1C */
    u8 unk20;
    u8 unk21;
    u8 unk22;
    u8 unk23;
    u32 unk24; /* 0x24 */
    u32 unk28; /* 0x28 */
    u8 unk2C;
    u8 unk2D;
    u8 unk2E;
    u8 unk2F;
    u8 pad30;
    u8 unk31;
    u8 pad32;
    u8 unk33;
    u32 unk34; /* 0x34 */
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
    FieldActorPartDef *unk0;
    u8 pad4[0x244 - 4];
} FieldActorState;

extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;
extern Struct_D800FDF58 D_800FF658[256];
extern FieldActorState g_field_actor_slots[80];

/**
 * @brief Field two-point trail/streak primitive builder: when the record has
 *        a linked previous record (rec->unk3D), projects both this record's
 *        and the linked record's world position to screen space (near/far
 *        pairs offset by rec->unk44/unk48), fills the primitive at primbuf,
 *        and blends its color from a depth-indexed palette in base[].
 * @param rec Effect record supplying the position/link/flag fields.
 * @param primbuf Output primitive buffer; advanced by one primitive (0x18
 *                bytes) when the record has an active link.
 * @param base Depth-indexed color/ordering-table array.
 * @return The advanced primbuf cursor (unchanged when rec has no active
 *         link).
 * @note WIP - 92.15% (234/284 exact rows). Structure and insn count already
 *       match the target exactly (no structural gaps); all residue is
 *       register-coloring / instruction-order noise, confirmed via
 *       sched_oracle (one genuinely VIOLATED emit-order constraint in the
 *       final else-branch color-merge block: target emits the `primbuf &
 *       0xFFFFFF` AND before the `&base[rec->unk8 >> 7]` address chain;
 *       tried naming it as a separate local both immediately before and at
 *       the top of the branch, and swapping the OR's operand order - all
 *       inert or negative, so the lever is still unknown).
 *       Two proven shapes (worth generalizing to idioms.md once solved):
 *       (1) [EXPAND-32]-style constant-first grouping is required for both
 *       screen coordinates - write `t + (X / 256 + K)` (constant grouped
 *       with the per-point division term, shared temp `t` added last), NOT
 *       `K + t + X / 256`. The `x / 256` / `x / 512` division form (not a
 *       manual sign-fix + shift) is required to match the target's
 *       instruction shape.
 *       (2) `part` must be computed from `g_field_actor_slots[rec->unk22]`
 *       BEFORE `state` (as two independent expressions, not `state = ...;
 *       part = &state->unk0[...];`) - this alone was +17 exact rows. It
 *       looks like gcc keeps the array's base address live across the first
 *       use and re-derives it fresh for the second, rather than caching it
 *       in a shared pointer local.
 *       (3) The linked record's `unk3D`-indexed chase into D_800FF658 must
 *       be written as a fresh `D_800FF658[rec->unk3D]` at every field access
 *       (never cached in a named pointer) - same CSE-18-style store-kills-
 *       cache pattern as elsewhere in this segment; the final else-branch
 *       color index (`rec->unk8 >> 7`) needs the same fresh-recompute
 *       treatment for its second use.
 *       Permuter false lead: two candidates (scores 2990/1720) scored higher
 *       on the permuter's own metric by aliasing `primbuf` into the
 *       `s32 *temp_v0` local WITHOUT a cast (`temp_v0 = primbuf;`), which
 *       silently turns `temp_v0 + 0x18` into `+0x60` (s32 pointer scaling) -
 *       a real semantic bug, not a register-allocation insight. Faithfully
 *       reproducing the aliasing with correct `(u8 *)` casts measured
 *       delta-exact 0; do not re-chase this class.
 * @see decomp.me WIP
 */
u8 *func_8007AE2C(Struct_D800FDF58 *rec, u8 *primbuf, s32 *base)
{
    FieldActorState *state;
    FieldActorPartDef *part;
    u8 *var_s1;
    s32 *temp_v0;
    s32 temp_t3;
    s32 temp_t4;
    s8 var_v0_5;
    s32 temp_v1_2;

    part = &g_field_actor_slots[rec->unk22].unk0[rec->unk23];
    state = &g_field_actor_slots[rec->unk22];

    if (rec->unk3D != 0xFF && D_800FF658[rec->unk3D].unk25 != 0xFF)
    {
        temp_t3 = D_800F22A0 / 256;
        *(u16 *) (primbuf + 0x8) = (u16) (temp_t3 + (rec->unk0 / 256 + 0xA0));

        temp_t4 = D_800F22A4 / 256;
        *(u16 *) (primbuf + 0xA) = (u16) (0x70 + temp_t4 + rec->unk4 / 256 - rec->unk8 / 512 - D_800F22A8 / 512);

        *(s32 *) (primbuf + 0x10) = *(s32 *) (primbuf + 0x8);
        *(u16 *) (primbuf + 0x8) = (u16) (*(u16 *) (primbuf + 0x8) - (u16) rec->unk44);
        *(u16 *) (primbuf + 0xA) = (u16) (*(u16 *) (primbuf + 0xA) - (u16) rec->unk48);
        *(u16 *) (primbuf + 0x10) = (u16) (*(u16 *) (primbuf + 0x10) + (u16) rec->unk44);
        *(u16 *) (primbuf + 0x12) = (u16) (*(u16 *) (primbuf + 0x12) + (u16) rec->unk48);

        *(u16 *) (primbuf + 0xC) = (u16) (temp_t3 + (D_800FF658[rec->unk3D].unk0 / 256 + 0xA0));

        *(u16 *) (primbuf + 0xE) = (u16) (0x70 + temp_t4 + D_800FF658[rec->unk3D].unk4 / 256 - D_800FF658[rec->unk3D].unk8 / 512 - D_800F22A8 / 512);

        *(s32 *) (primbuf + 0x14) = *(s32 *) (primbuf + 0xC);
        *(u16 *) (primbuf + 0xC) = (u16) (*(u16 *) (primbuf + 0xC) - (u16) D_800FF658[rec->unk3D].unk44);
        *(u16 *) (primbuf + 0xE) = (u16) (*(u16 *) (primbuf + 0xE) - (u16) D_800FF658[rec->unk3D].unk48);
        *(u16 *) (primbuf + 0x14) = (u16) (*(u16 *) (primbuf + 0x14) + (u16) D_800FF658[rec->unk3D].unk44);
        *(u16 *) (primbuf + 0x16) = (u16) (*(u16 *) (primbuf + 0x16) + (u16) D_800FF658[rec->unk3D].unk48);

        func_8007D8D8(state, rec, part, primbuf + 4);

        *(s8 *) (primbuf + 3) = 5;
        *(s8 *) (primbuf + 7) = 0x28;
        var_v0_5 = 0x2A;
        if (!(rec->unk1C & 0x800000))
        {
            var_v0_5 = 0x28;
        }
        *(s8 *) (primbuf + 7) = var_v0_5;

        temp_v1_2 = (s32) rec->unk8 >> 7;
        if (temp_v1_2 < 0)
        {
            *(s32 *) (primbuf + 0) = (*(s32 *) (primbuf + 0) & 0xFF000000) | (base[0] & 0xFFFFFF);
            var_s1 = primbuf + 0x18;
            base[0] = (base[0] & 0xFF000000) | ((s32) primbuf & 0xFFFFFF);
        }
        else if (temp_v1_2 >= 0x1000)
        {
            *(s32 *) (primbuf + 0) = (*(s32 *) (primbuf + 0) & 0xFF000000) | (base[0xFFF] & 0xFFFFFF);
            var_s1 = primbuf + 0x18;
            base[0xFFF] = (base[0xFFF] & 0xFF000000) | ((s32) primbuf & 0xFFFFFF);
        }
        else
        {
            *(s32 *) (primbuf + 0) = (*(s32 *) (primbuf + 0) & 0xFF000000) | (base[temp_v1_2] & 0xFFFFFF);
            temp_v0 = &base[(s32) rec->unk8 >> 7];
            var_s1 = primbuf + 0x18;
            *temp_v0 = (*temp_v0 & 0xFF000000) | ((s32) primbuf & 0xFFFFFF);
        }

        primbuf = func_8007DA80(rec, part, var_s1, base);
    }

    return primbuf;
}
