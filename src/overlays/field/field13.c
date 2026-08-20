/**
 * @file field13.c
 * @brief Field animation-frame audio/visual processor, carved from the top
 *        of the unk2 segment (the single-function slot right after
 *        field12.c's func_80077FB4).
 */

#include "common.h"

typedef struct
{
    s16 m[3][3];
    s32 t[3];
} FieldMatrix;

typedef struct
{
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
} FieldSVector;

typedef struct
{
    s32 vx;
    s32 vy;
    s32 vz;
    s32 pad;
} FieldVector;

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

#include "psyq/inline_c.h"

/*
 * inline_c.h's "Type 2" (no-operand) GTE macros, like gte_rtv0() below, emit
 * a raw `.word` value meant for Sony's original PSY-Q assembler to translate
 * into the real COP2 instruction word. This project assembles with plain GNU
 * `as` (via the maspsx syntax wrapper), which has no such translation step,
 * so override the one Type 2 op actually used here with the real ROM bytes,
 * as a `cop2` immediate, matching field7.c/field8.c/field11.c/field12.c.
 */
#define gte_rtv0() __asm__ volatile("nop;nop;cop2 0x0486012")

extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;
extern FieldActorState g_field_actor_slots[80];

/**
 * @brief Field ring/cylinder mesh builder: generates a 32-segment circular
 *        strip of billboard primitives around an actor, transforming each
 *        segment's local-space direction through the actor's rotation
 *        matrix via the GTE and inserting the resulting primitives into the
 *        ordering table.
 * @param rec Effect record.
 * @param primbuf Output primitive buffer; advanced by 3 primitives (0x54
 *                bytes) per ring segment.
 * @param base Ordering-table / primitive base array.
 * @note WIP - 72.35% (172/461 exact rows) at time of writing. m2c could not
 *       resolve the delay-slot stores of the camera-relative pan base (X to
 *       sp+0x10, Y to sp+0x12) or the GTE output's Y component (sp+0x24);
 *       all three were recovered from the raw target asm and are spelled
 *       here as ordinary locals (sp10, sp12, and inline reads of the local
 *       GTE output struct) rather than m2c's placeholder `subroutine_arg4`.
 *       The GTE dir/output vectors and rotation matrix are genuine stack
 *       locals in this function (sp18/sp20/sp30), unlike field11.c/
 *       field12.c's fixed 0x1F800000-range scratchpad addresses. Frame is
 *       0x10 bytes LARGER than the target's (-0x98 vs -0x88, the opposite
 *       direction from field11.c/field12.c's residue) and the GTE locals
 *       land at different stack offsets than the target's sp+0x18/0x20 -
 *       likely a local-variable-set/declaration-order difference rather
 *       than a structural one, since exact rows already cover most of the
 *       per-vertex fill logic.
 * @see decomp.me WIP
 */
void func_800799C4(Struct_D800FDF58 *rec, u8 *primbuf, s32 *base)
{
    FieldMatrix sp30;
    FieldSVector sp18;
    FieldVector sp20;
    s32 sp5C;
    FieldVector *sp58;
    FieldSVector *sp54;
    FieldActorPartDef *sp50;
    u16 sp12;
    u16 sp10;
    s16 temp_v0;
    s16 temp_v1_2;
    s16 temp_v1_3;
    s16 temp_v1_4;
    s16 temp_v1_5;
    s16 temp_v1_6;
    s32 *temp_a1;
    FieldActorState *temp_s2;
    s32 *temp_v1_11;
    s32 *temp_v1_13;
    s32 *temp_v1_9;
    s32 *var_s1_2;
    s32 temp_lo;
    s32 temp_s0_2;
    s32 temp_v0_2;
    s32 temp_v1;
    s32 temp_v1_10;
    s32 temp_v1_12;
    s32 temp_v1_7;
    s32 temp_v1_8;
    s32 var_a0;
    s32 var_a0_2;
    s32 var_a0_3;
    s32 var_fp;
    s32 var_lo;
    s32 var_s5;
    s32 var_t0;
    s32 var_t1;
    s32 var_v0;
    s32 var_v0_3;
    s32 var_v1;
    s8 var_v0_2;
    u8 temp_s6;
    u8 *temp_s0;
    u8 *var_s1;

    var_s1 = primbuf;
    temp_s2 = &g_field_actor_slots[rec->unk22];
    temp_v1 = (s32) &temp_s2->unk0[rec->unk23];
    sp50 = (FieldActorPartDef *) temp_v1;
    func_8007D078(rec, sp50, &sp30, temp_s2);
    gte_SetRotMatrix(&sp30);

    var_a0 = D_800F22A0;
    if (var_a0 < 0)
    {
        var_a0 += 0xFF;
    }
    var_v0 = rec->unk0;
    if (var_v0 < 0)
    {
        var_v0 += 0xFF;
    }
    sp10 = (u16) ((var_a0 >> 8) + ((var_v0 >> 8) + 0xA0));
    var_a0 = D_800F22A4;
    if (var_a0 < 0)
    {
        var_a0 += 0xFF;
    }
    var_v0 = rec->unk4;
    if (var_v0 < 0)
    {
        var_v0 += 0xFF;
    }
    var_v1 = rec->unk8;
    if (var_v1 < 0)
    {
        var_v1 += 0x1FF;
    }
    var_t0 = D_800F22A8;
    if (var_t0 < 0)
    {
        var_t0 += 0x1FF;
    }
    sp12 = (u16) ((((var_a0 >> 8) + ((var_v0 >> 8) + 0x70)) - (var_v1 >> 9)) - (var_t0 >> 9));
    func_8007D8D8(temp_s2, rec, sp50, var_s1 + 4, sp12);
    *(s8 *) (var_s1 + 3) = 6;
    *(s8 *) (var_s1 + 7) = 0x30;
    var_v0_2 = 0x32;
    if (!(rec->unk1C & 0x800000))
    {
        var_v0_2 = 0x30;
    }
    *(s8 *) (var_s1 + 7) = var_v0_2;
    var_fp = 0;
    temp_s6 = rec->unk24;
    sp54 = &sp18;
    var_s5 = 0;
    sp58 = &sp20;
    var_t1 = 0x80;
    do
    {
        sp5C = var_t1;
        *(u16 *) (var_s1 + 0x10) = sp10;
        *(u16 *) (var_s1 + 0x12) = sp12;
        temp_lo = (rcos(var_s5) >> 6) * temp_s6;
        sp18.unk4 = 0;
        sp18.unk0 = (s16) (temp_lo >> 8);
        sp18.unk2 = (s16) ((s32) ((rsin(var_s5) >> 6) * temp_s6) >> 8);
        gte_ldv0(sp54);
        gte_rtv0();
        gte_stlvnl(sp58);
        temp_v1_2 = sp10 + (u16) sp20.vx;
        *(s16 *) (var_s1 + 8) = temp_v1_2;
        *(s16 *) (var_s1 + 0x24) = temp_v1_2;
        temp_v1_3 = sp12 + (u16) sp20.vy;
        *(s16 *) (var_s1 + 0xA) = temp_v1_3;
        *(s16 *) (var_s1 + 0x26) = temp_v1_3;
        if (var_fp == 0x1F)
        {
            var_lo = (rcos(0) >> 6) * temp_s6;
            var_a0_2 = 0;
        }
        else
        {
            var_lo = (rcos(sp5C) >> 6) * temp_s6;
            var_a0_2 = sp5C;
        }
        sp18.unk4 = 0;
        sp18.unk0 = (s16) (var_lo >> 8);
        sp18.unk2 = (s16) ((s32) ((rsin(var_a0_2) >> 6) * temp_s6) >> 8);
        gte_ldv0(sp54);
        gte_rtv0();
        gte_stlvnl(sp58);
        temp_s0 = var_s1 + 0x1C;
        temp_v1_4 = sp10 + (u16) sp20.vx;
        *(s16 *) (var_s1 + 0x18) = temp_v1_4;
        *(s16 *) (temp_s0 + 0x18) = temp_v1_4;
        *(s16 *) (var_s1 + 0x40) = temp_v1_4;
        temp_v0 = sp12 + (u16) sp20.vy;
        *(s16 *) (var_s1 + 0x1A) = temp_v0;
        *(s16 *) (temp_s0 + 0x1A) = temp_v0;
        *(s16 *) (var_s1 + 0x42) = temp_v0;
        sp18.unk0 = (s16) (rcos(var_s5) >> 6);
        sp18.unk4 = 0;
        sp18.unk2 = (s16) (rsin(var_s5) >> 6);
        gte_ldv0(sp54);
        gte_rtv0();
        gte_stlvnl(sp58);
        temp_v1_5 = sp10 + (u16) sp20.vx;
        *(s16 *) (temp_s0 + 0x10) = temp_v1_5;
        *(s16 *) (var_s1 + 0x48) = temp_v1_5;
        temp_v1_6 = sp12 + (u16) sp20.vy;
        *(s16 *) (temp_s0 + 0x12) = temp_v1_6;
        *(s16 *) (var_s1 + 0x4A) = temp_v1_6;
        if (var_fp == 0x1F)
        {
            var_v0_3 = rcos(0);
            var_a0_3 = 0;
        }
        else
        {
            temp_s0_2 = var_s5 + 0x80;
            var_v0_3 = rcos(temp_s0_2);
            var_a0_3 = temp_s0_2;
        }
        sp18.unk0 = (s16) (var_v0_3 >> 6);
        sp18.unk4 = 0;
        sp18.unk2 = (s16) (rsin(var_a0_3) >> 6);
        gte_ldv0(sp54);
        gte_rtv0();
        gte_stlvnl(sp58);
        *(s16 *) (var_s1 + 0x50) = (s16) (sp10 + (u16) sp20.vx);
        temp_v0_2 = *(s32 *) (var_s1 + 4);
        temp_v1_7 = *(s32 *) (var_s1 + 0);
        *(s32 *) (var_s1 + 0xC) = 0;
        *(s32 *) (var_s1 + 0x4C) = 0;
        *(s32 *) (var_s1 + 0x44) = 0;
        *(s32 *) (var_s1 + 0x28) = 0;
        *(s32 *) (var_s1 + 0x3C) = temp_v0_2;
        *(s32 *) (var_s1 + 0x30) = temp_v0_2;
        *(s32 *) (var_s1 + 0x20) = temp_v0_2;
        *(s32 *) (var_s1 + 0x14) = temp_v0_2;
        *(s32 *) (var_s1 + 0x1C) = temp_v1_7;
        *(s32 *) (var_s1 + 0x38) = temp_v1_7;
        *(s32 *) (var_s1 + 0x54) = temp_v1_7;
        *(s32 *) (var_s1 + 0x58) = temp_v0_2;
        *(s16 *) (var_s1 + 0x52) = (s16) (sp12 + (u16) sp20.vy);
        temp_v1_8 = (s32) rec->unk8 >> 7;
        temp_a1 = (s32 *) (var_s1 + 0x1C);
        if (temp_v1_8 < 0)
        {
            *(s32 *) (var_s1 + 0) = (s32) ((*(s32 *) (var_s1 + 0) & 0xFF000000) | (base[0] & 0xFFFFFF));
            base[0] = (s32) ((base[0] & 0xFF000000) | ((s32) var_s1 & 0xFFFFFF));
        }
        else if (temp_v1_8 >= 0x1000)
        {
            *(s32 *) (var_s1 + 0) = (s32) ((*(s32 *) (var_s1 + 0) & 0xFF000000) | (base[0xFFF] & 0xFFFFFF));
            base[0xFFF] = (s32) ((base[0xFFF] & 0xFF000000) | ((s32) var_s1 & 0xFFFFFF));
        }
        else
        {
            *(s32 *) (var_s1 + 0) = (s32) ((*(s32 *) (var_s1 + 0) & 0xFF000000) | (*((temp_v1_8 * 4) + base) & 0xFFFFFF));
            temp_v1_9 = (((s32) rec->unk8 >> 7) * 4) + base;
            *temp_v1_9 = (*temp_v1_9 & 0xFF000000) | ((s32) var_s1 & 0xFFFFFF);
        }
        temp_v1_10 = (s32) rec->unk8 >> 7;
        if (temp_v1_10 < 0)
        {
            *temp_a1 = (*temp_a1 & 0xFF000000) | (base[0] & 0xFFFFFF);
            var_s1_2 = temp_a1 + 7;
            base[0] = (s32) ((base[0] & 0xFF000000) | ((s32) temp_a1 & 0xFFFFFF));
        }
        else if (temp_v1_10 >= 0x1000)
        {
            *temp_a1 = (*temp_a1 & 0xFF000000) | (base[0xFFF] & 0xFFFFFF);
            var_s1_2 = temp_a1 + 7;
            base[0xFFF] = (s32) ((base[0xFFF] & 0xFF000000) | ((s32) temp_a1 & 0xFFFFFF));
        }
        else
        {
            *temp_a1 = (*temp_a1 & 0xFF000000) | (*((temp_v1_10 * 4) + base) & 0xFFFFFF);
            temp_v1_11 = (((s32) rec->unk8 >> 7) * 4) + base;
            var_s1_2 = temp_a1 + 7;
            *temp_v1_11 = (*temp_v1_11 & 0xFF000000) | ((s32) temp_a1 & 0xFFFFFF);
        }
        temp_v1_12 = (s32) rec->unk8 >> 7;
        if (temp_v1_12 < 0)
        {
            *var_s1_2 = (*var_s1_2 & 0xFF000000) | (base[0] & 0xFFFFFF);
            var_s1 = (u8 *) (var_s1_2 + 7);
            base[0] = (s32) ((base[0] & 0xFF000000) | ((s32) var_s1_2 & 0xFFFFFF));
        }
        else if (temp_v1_12 >= 0x1000)
        {
            *var_s1_2 = (*var_s1_2 & 0xFF000000) | (base[0xFFF] & 0xFFFFFF);
            var_s1 = (u8 *) (var_s1_2 + 7);
            base[0xFFF] = (s32) ((base[0xFFF] & 0xFF000000) | ((s32) var_s1_2 & 0xFFFFFF));
        }
        else
        {
            *var_s1_2 = (*var_s1_2 & 0xFF000000) | (*((temp_v1_12 * 4) + base) & 0xFFFFFF);
            temp_v1_13 = (((s32) rec->unk8 >> 7) * 4) + base;
            var_s1 = (u8 *) (var_s1_2 + 7);
            *temp_v1_13 = (*temp_v1_13 & 0xFF000000) | ((s32) var_s1_2 & 0xFFFFFF);
        }
        var_s5 += 0x80;
        var_fp += 1;
        var_t1 = sp5C + 0x80;
    } while (var_fp != 0x20);
    func_8007DA80(rec, sp50, var_s1, base);
}
