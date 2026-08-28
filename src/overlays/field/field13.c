/**
 * @file field13.c
 * @brief Field animation-frame audio/visual processor, carved from the top
 *        of the unk2 segment (the single-function slot right after
 *        field12.c's func_80077FB4).
 */

#include "common.h"
#include "psyq_compat/libgte.h"
#include "psyq_compat/libgpu.h"

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
    u16 x;
    u16 y;
} FieldScreenPair;

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

#include "psyq_compat/inline_c.h"
#include "psyq_compat/gte_dmpsx_compat.h"

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
 * @see decomp.me (100%)
 */
void func_800799C4(Struct_D800FDF58 *rec, u8 *primbuf, s32 *base)
{
    FieldScreenPair sp10;
    FieldSVector sp18;
    FieldVector sp20;
    FieldMatrix sp30;
    FieldActorPartDef *sp50;
    FieldSVector *sp54;
    FieldVector *sp58;
    s32 sp5C;
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
    s32 addr_mask;
    s32 high_mask;
    s32 temp_s0_2;
    s32 temp_v0_2;
    s32 temp_v1;
    s32 temp_v1_10;
    s32 temp_v1_12;
    s32 temp_v1_7;
    s32 temp_v1_8;
    s32 var_a0;
    s32 var_a1;
    s32 var_a0_2;
    s32 var_a0_3;
    s32 var_fp;
    s32 var_lo;
    s32 var_s5;
    s32 var_t0;
    s32 var_v0;
    s32 var_v0_3;
    s32 var_v1;
    s32 temp_s6;
    u8 *temp_s0;
    u8 *var_s1;

    var_s1 = primbuf;
    sp50 = &g_field_actor_slots[rec->unk22].unk0[rec->unk23];
    temp_s2 = &g_field_actor_slots[rec->unk22];
    func_8007D078(rec, sp50, &sp30, temp_s2);
    gte_SetRotMatrix(&sp30);

    var_a1 = D_800F22A0 / 256;
    sp10.x = (u16) (var_a1 + (rec->unk0 / 256 + 0xA0));
    sp10.y = (u16) (0x70 + D_800F22A4 / 256 + rec->unk4 / 256 - rec->unk8 / 512 - D_800F22A8 / 512);
    func_8007D8D8(temp_s2, rec, sp50, var_s1 + 4);
    setPolyG3(var_s1);
    setSemiTrans(var_s1, rec->unk1C & 0x800000);
    var_fp = 0;
    temp_s6 = rec->unk24;
    sp54 = &sp18;
    sp58 = &sp20;
    addr_mask = 0xFFFFFF;
    high_mask = 0xFF000000;
    var_s5 = var_fp;
    do
    {
        sp5C = (var_fp + 1) << 7;
        *(u16 *) (var_s1 + 0x10) = sp10.x;
        *(u16 *) (var_s1 + 0x12) = sp10.y;
        temp_lo = (rcos(var_s5) >> 6) * temp_s6;
        sp18.unk2 = 0;
        sp18.unk0 = (s16) (temp_lo >> 8);
        sp18.unk4 = (s16) ((s32) ((rsin(var_s5) >> 6) * temp_s6) >> 8);
        gte_ldv0(sp54);
        gte_rtv0();
        gte_stlvnl(sp58);
        temp_v1_2 = sp10.x + (u16) sp20.vx;
        *(s16 *) (var_s1 + 8) = temp_v1_2;
        *(s16 *) (var_s1 + 0x24) = temp_v1_2;
        temp_v1_3 = sp10.y + (u16) sp20.vy;
        *(s16 *) (var_s1 + 0xA) = temp_v1_3;
        *(s16 *) (var_s1 + 0x26) = temp_v1_3;
        if (var_fp == 0x1F)
        {
            var_lo = (rcos(0) >> 6) * temp_s6;
            var_a0_2 = 0;
            sp18.unk2 = 0;
            sp18.unk0 = (s16) (var_lo >> 8);
            sp18.unk4 = (s16) ((s32) ((rsin(var_a0_2) >> 6) * temp_s6) >> 8);
        }
        else
        {
            var_lo = (rcos(sp5C) >> 6) * temp_s6;
            var_a0_2 = sp5C;
            sp18.unk2 = 0;
            sp18.unk0 = (s16) (var_lo >> 8);
            sp18.unk4 = (s16) ((s32) ((rsin(var_a0_2) >> 6) * temp_s6) >> 8);
        }
        gte_ldv0(sp54);
        gte_rtv0();
        gte_stlvnl(sp58);
        temp_s0 = var_s1 + 0x1C;
        temp_v1_4 = sp10.x + (u16) sp20.vx;
        *(s16 *) (var_s1 + 0x18) = temp_v1_4;
        *(s16 *) (temp_s0 + 0x18) = temp_v1_4;
        *(s16 *) (var_s1 + 0x40) = temp_v1_4;
        temp_v0 = sp10.y + (u16) sp20.vy;
        *(s16 *) (var_s1 + 0x1A) = temp_v0;
        *(s16 *) (temp_s0 + 0x1A) = temp_v0;
        *(s16 *) (var_s1 + 0x42) = temp_v0;
        sp18.unk0 = (s16) (rcos(var_s5) >> 6);
        sp18.unk2 = 0;
        sp18.unk4 = (s16) (rsin(var_s5) >> 6);
        gte_ldv0(sp54);
        gte_rtv0();
        gte_stlvnl(sp58);
        temp_v1_5 = sp10.x + (u16) sp20.vx;
        *(s16 *) (temp_s0 + 0x10) = temp_v1_5;
        *(s16 *) (var_s1 + 0x48) = temp_v1_5;
        temp_v1_6 = sp10.y + (u16) sp20.vy;
        *(s16 *) (temp_s0 + 0x12) = temp_v1_6;
        *(s16 *) (var_s1 + 0x4A) = temp_v1_6;
        if (var_fp == 0x1F)
        {
            var_v0_3 = rcos(0);
            var_a0_3 = 0;
            sp18.unk0 = (s16) (var_v0_3 >> 6);
            sp18.unk2 = 0;
            sp18.unk4 = (s16) (rsin(var_a0_3) >> 6);
        }
        else
        {
            temp_s0_2 = var_s5 + 0x80;
            var_v0_3 = rcos(temp_s0_2);
            var_a0_3 = temp_s0_2;
            sp18.unk0 = (s16) (var_v0_3 >> 6);
            sp18.unk2 = 0;
            sp18.unk4 = (s16) (rsin(var_a0_3) >> 6);
        }
        gte_ldv0(sp54);
        gte_rtv0();
        gte_stlvnl(sp58);
        temp_v1_6 = (s16) (sp10.x + (u16) sp20.vx);
        *(s16 *) (var_s1 + 0x50) = temp_v1_6;
        temp_v0_2 = *(s32 *) (var_s1 + 4);
        temp_v0 = (s16) (sp10.y + (u16) sp20.vy);
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
        *(s32 *) (var_s1 + 0x58) = *(s32 *) (var_s1 + 4);
        *(s16 *) (var_s1 + 0x52) = temp_v0;
        temp_v1_8 = (s32) rec->unk8 >> 7;
        temp_a1 = (s32 *) (var_s1 + 0x1C);
        if (temp_v1_8 < 0)
        {
            (*(s32 *) (var_s1) = (*(s32 *) (var_s1) & high_mask) | (*(s32 *) (&base[0]) & addr_mask), *(s32 *) (&base[0]) = (*(s32 *) (&base[0]) & high_mask) | ((s32) (var_s1) & addr_mask));
            var_s1 = (u8 *) temp_a1;
        }
        else if (temp_v1_8 >= 0x1000)
        {
            (*(s32 *) (var_s1) = (*(s32 *) (var_s1) & high_mask) | (*(s32 *) (&base[0xFFF]) & addr_mask), *(s32 *) (&base[0xFFF]) = (*(s32 *) (&base[0xFFF]) & high_mask) | ((s32) (var_s1) & addr_mask));
            var_s1 = (u8 *) temp_a1;
        }
        else
        {
            (*(s32 *) (var_s1) = (*(s32 *) (var_s1) & high_mask) | (*(s32 *) ((s32 *) ((((s32) rec->unk8 >> 7) << 2) + (s32) base)) & addr_mask), *(s32 *) ((s32 *) ((((s32) rec->unk8 >> 7) << 2) + (s32) base)) = (*(s32 *) ((s32 *) ((((s32) rec->unk8 >> 7) << 2) + (s32) base)) & high_mask) | ((s32) (var_s1) & addr_mask));
            var_s1 = (u8 *) temp_a1;
        }
        temp_v1_10 = (s32) rec->unk8 >> 7;
        if (temp_v1_10 < 0)
        {
            (*(s32 *) (var_s1) = (*(s32 *) (var_s1) & high_mask) | (*(s32 *) (&base[0]) & addr_mask), *(s32 *) (&base[0]) = (*(s32 *) (&base[0]) & high_mask) | ((s32) (var_s1) & addr_mask));
            var_s1 += 0x1C;
        }
        else if (temp_v1_10 >= 0x1000)
        {
            (*(s32 *) (var_s1) = (*(s32 *) (var_s1) & high_mask) | (*(s32 *) (&base[0xFFF]) & addr_mask), *(s32 *) (&base[0xFFF]) = (*(s32 *) (&base[0xFFF]) & high_mask) | ((s32) (var_s1) & addr_mask));
            var_s1 += 0x1C;
        }
        else
        {
            (*(s32 *) (var_s1) = (*(s32 *) (var_s1) & high_mask) | (*(s32 *) ((s32 *) ((((s32) rec->unk8 >> 7) << 2) + (s32) base)) & addr_mask), *(s32 *) ((s32 *) ((((s32) rec->unk8 >> 7) << 2) + (s32) base)) = (*(s32 *) ((s32 *) ((((s32) rec->unk8 >> 7) << 2) + (s32) base)) & high_mask) | ((s32) (var_s1) & addr_mask));
            var_s1 += 0x1C;
        }
        temp_v1_12 = (s32) rec->unk8 >> 7;
        if (temp_v1_12 < 0)
        {
            (*(s32 *) (var_s1) = (*(s32 *) (var_s1) & high_mask) | (*(s32 *) (&base[0]) & addr_mask), *(s32 *) (&base[0]) = (*(s32 *) (&base[0]) & high_mask) | ((s32) (var_s1) & addr_mask));
            var_s1 += 0x1C;
        }
        else if (temp_v1_12 >= 0x1000)
        {
            (*(s32 *) (var_s1) = (*(s32 *) (var_s1) & high_mask) | (*(s32 *) (&base[0xFFF]) & addr_mask), *(s32 *) (&base[0xFFF]) = (*(s32 *) (&base[0xFFF]) & high_mask) | ((s32) (var_s1) & addr_mask));
            var_s1 += 0x1C;
        }
        else
        {
            (*(s32 *) (var_s1) = (*(s32 *) (var_s1) & high_mask) | (*(s32 *) ((s32 *) ((((s32) rec->unk8 >> 7) << 2) + (s32) base)) & addr_mask), *(s32 *) ((s32 *) ((((s32) rec->unk8 >> 7) << 2) + (s32) base)) = (*(s32 *) ((s32 *) ((((s32) rec->unk8 >> 7) << 2) + (s32) base)) & high_mask) | ((s32) (var_s1) & addr_mask));
            var_s1 += 0x1C;
        }
        var_s5 += 0x80;
        if (var_fp == 0x1F)
        {
            break;
        }
        var_fp += 1;
    } while (1);
    func_8007DA80(rec, sp50, var_s1, base);
}

/**
 * @brief Field fan/cylinder mesh builder: generates a variable-segment
 *        circular fan of billboard primitives around an actor (segment
 *        count taken from the part definition, clamped to [2,32]),
 *        transforming each segment's five corner directions through the
 *        actor's rotation matrix via the GTE and inserting the resulting
 *        primitives into the ordering table.
 * @param rec Effect record.
 * @param primbuf Output primitive buffer; advanced by 4 primitives (0x70
 *                bytes) per ring segment.
 * @param base Ordering-table / primitive base array.
 * @see decomp.me (100%)
 */
void func_8007A104(Struct_D800FDF58 *rec, u8 *primbuf, s32 *base)
{
    FieldSVector sp10;
    FieldSVector sp18;
    FieldVector sp20;
    FieldMatrix sp30;
    FieldActorPartDef *sp50;
    s32 sp54;
    FieldSVector *sp58;
    FieldVector *sp5C;
    s32 sp60;
    s32 sp64;
    s32 sp68;
    s16 temp_v0;
    s16 temp_v0_2;
    s16 temp_v1_4;
    s16 temp_v1_5;
    s16 temp_v1_6;
    s16 temp_v1_7;
    s32 *temp_a1_2;
    FieldActorState *temp_s2;
    FieldActorState *slots;
    s32 *temp_v1_10;
    s32 *temp_v1_12;
    s32 *temp_v1_14;
    s32 *temp_v1_16;
    s32 temp_a0;
    s32 temp_a1copy;
    s32 temp_lo;
    s16 half_src;
    s32 temp_s0_2;
    s32 temp_s0_3;
    s32 temp_v0_3;
    s32 ot_word;
    s32 temp_v1_11;
    s32 temp_v1_13;
    s32 temp_v1_15;
    s32 temp_v1_8;
    s32 temp_v1_9;
    s32 var_a0_4;
    s32 var_s6;
    s32 var_s7;
    s32 var_v0_4;
    s32 var_v0_5;
    s32 var_v0_6;
    u16 ybase;
    u16 yoff;
    s32 var_v0_3;
    s32 limit1;
    s32 limit2;
    s32 limit3;
    s32 mask_low;
    s32 mask_high;
    u8 var_fp;
    u8 *temp_a0_2;
    u8 *temp_a1;
    u8 *temp_s0;
    u8 *var_s1;
    FieldActorPartDef *temp_v1;

    var_s1 = primbuf;
    slots = g_field_actor_slots;
    temp_s2 = &slots[rec->unk22];
    temp_v1 = &temp_s2->unk0[rec->unk23];
    sp50 = temp_v1;
    func_8007D078(rec, sp50, &sp30, temp_s2);
    gte_SetRotMatrix(&sp30);

    sp10.unk0 = (u16) ((D_800F22A0 / 0x100) + ((rec->unk0 / 0x100) + 0xA0));
    sp10.unk2 = (u16) (((((D_800F22A4 / 0x100) + 0x70) + (rec->unk4 / 0x100)) - (rec->unk8 / 0x200)) - (D_800F22A8 / 0x200));
    func_8007D8D8(temp_s2, rec, sp50, var_s1 + 4);
    *(s8 *) (var_s1 + 3) = 6;
    *(s8 *) (var_s1 + 7) = 0x30;
    temp_v1_15 = 0x800000;
    var_v0_3 = rec->unk1C & temp_v1_15;
    if (var_v0_3)
    {
        var_v0_3 = 0x32;
    }
    else
    {
        var_v0_3 = 0x30;
    }
    *(s8 *) (var_s1 + 7) = var_v0_3;
    var_fp = 2;
    var_v0_3 = sp50->unk8 - 2;
    if ((u32) var_v0_3 < 0x1FU)
    {
        var_fp = *(volatile u8 *) &sp50->unk8;
    }
    temp_lo = 0x1000 / (s32) var_fp;
    half_src = temp_lo;
    var_s6 = half_src;
    if ((0x1000 % (s32) var_fp) != 0)
    {
        var_s6 += 1;
    }
    var_s7 = 0;
    sp58 = &sp18;
    sp5C = &sp20;
    mask_low = 0xFFFFFF;
    mask_high = 0xFF000000;
    temp_a0 = temp_lo >> 1;
    sp54 = temp_a0;
    half_src = temp_a0;
    sp60 = half_src;
    sp64 = 0;
    sp68 = var_s6;
loop_19:
    {
        *(s32 *) (var_s1 + 0x10) = *(s32 *) &sp10;
        sp18.unk0 = (s16) (rcos(sp64) >> 8);
        sp18.unk2 = 0;
        sp18.unk4 = (s16) (rsin(sp64) >> 8);
        gte_ldv0(sp58);
        gte_rtv0();
        gte_stlvnl(sp5C);
        { u32 cb; u16 co;
            cb = (u16) sp10.unk0; co = (u16) sp20.vx; cb += co;
            *(s16 *) (var_s1 + 8) = cb;
            *(s16 *) (var_s1 + 0x24) = cb;
            cb = (u16) sp10.unk2; co = (u16) sp20.vy; cb += co;
            limit1 = var_fp - 1;
            *(s16 *) (var_s1 + 0xA) = cb;
            *(s16 *) (var_s1 + 0x26) = cb;
        }
        if (var_s7 == limit1)
        {
            var_v0_4 = rcos(0);
            sp18.unk0 = (s16) (var_v0_4 >> 8);
            sp18.unk2 = 0;
            sp18.unk4 = (s16) (rsin(0) >> 8);
        }
        else
        {
            var_v0_4 = rcos(sp68);
            sp18.unk0 = (s16) (var_v0_4 >> 8);
            sp18.unk2 = 0;
            sp18.unk4 = (s16) (rsin(sp68) >> 8);
        }
        gte_ldv0(sp58);
        gte_rtv0();
        gte_stlvnl(sp5C);
        temp_s0 = var_s1 + 0x1C;
        temp_a1 = var_s1 + 0x54;
        temp_v1_4 = sp10.unk0 + (u16) sp20.vx;
        *(s16 *) (var_s1 + 0x18) = temp_v1_4;
        *(s16 *) (temp_s0 + 0x18) = temp_v1_4;
        *(s16 *) (temp_a1 + 8) = temp_v1_4;
        *(s16 *) (var_s1 + 0x40) = temp_v1_4;
        temp_v0 = sp10.unk2 + (u16) sp20.vy;
        *(s16 *) (var_s1 + 0x1A) = temp_v0;
        *(s16 *) (temp_s0 + 0x1A) = temp_v0;
        *(s16 *) (temp_a1 + 0xA) = temp_v0;
        *(s16 *) (var_s1 + 0x42) = temp_v0;
        sp18.unk0 = (s16) (rcos(sp60) >> 6);
        sp18.unk2 = 0;
        sp18.unk4 = (s16) (rsin(sp60) >> 6);
        gte_ldv0(sp58);
        gte_rtv0();
        gte_stlvnl(sp5C);
        { u32 cb; u16 co;
            cb = (u16) sp10.unk0; co = (u16) sp20.vx; cb += co;
            *(s16 *) (temp_s0 + 0x10) = cb;
            *(s16 *) (var_s1 + 0x48) = cb;
            cb = (u16) sp10.unk2; co = (u16) sp20.vy; cb += co;
            limit2 = var_fp - 1;
            *(s16 *) (temp_s0 + 0x12) = cb;
            *(s16 *) (var_s1 + 0x4A) = cb;
        }
        if (var_s7 == limit2)
        {
            var_v0_5 = rcos(0);
            sp18.unk0 = (s16) (var_v0_5 >> 7);
            sp18.unk2 = 0;
            sp18.unk4 = (s16) (rsin(0) >> 7);
        }
        else
        {
            temp_s0_2 = sp64 + var_s6;
            var_v0_5 = rcos(temp_s0_2);
            sp18.unk0 = (s16) (var_v0_5 >> 7);
            sp18.unk2 = 0;
            sp18.unk4 = (s16) (rsin(temp_s0_2) >> 7);
        }
        gte_ldv0(sp58);
        gte_rtv0();
        gte_stlvnl(sp5C);
        temp_a0_2 = var_s1 + 0x38;
        temp_v0_2 = sp10.unk0 + (u16) sp20.vx;
        *(s16 *) (temp_a0_2 + 0x18) = temp_v0_2;
        *(s16 *) (var_s1 + 0x64) = temp_v0_2;
        temp_v1_9 = (u16) sp10.unk2 + (u16) sp20.vy;
        limit3 = var_fp - 1;
        *(s16 *) (temp_a0_2 + 0x1A) = temp_v1_9;
        *(s16 *) (var_s1 + 0x66) = temp_v1_9;
        if (var_s7 == limit3)
        {
            var_v0_6 = rcos(sp54);
            var_a0_4 = sp54;
            sp18.unk0 = (s16) (var_v0_6 >> 6);
            sp18.unk2 = 0;
            sp18.unk4 = (s16) (rsin(var_a0_4) >> 6);
        }
        else
        {
            temp_s0_3 = sp60 + var_s6;
            var_v0_6 = rcos(temp_s0_3);
            var_a0_4 = temp_s0_3;
            sp18.unk0 = (s16) (var_v0_6 >> 6);
            sp18.unk2 = 0;
            sp18.unk4 = (s16) (rsin(var_a0_4) >> 6);
        }
        gte_ldv0(sp58);
        gte_rtv0();
        gte_stlvnl(sp5C);
        *(s16 *) (var_s1 + 0x6C) = (s16) (sp10.unk0 + (u16) sp20.vx);
        temp_v0_3 = *(s32 *) (var_s1 + 4);
        ybase = sp10.unk2;
        yoff = (u16) sp20.vy;
        temp_v1_8 = *(s32 *) (var_s1 + 0);
        *(s32 *) (var_s1 + 0x68) = 0;
        *(s32 *) (var_s1 + 0x60) = 0;
        *(s32 *) (var_s1 + 0x4C) = 0;
        *(s32 *) (var_s1 + 0x44) = 0;
        *(s32 *) (var_s1 + 0x28) = 0;
        temp_a1copy = *(s32 *) (var_s1 + 4);
        *(s32 *) (var_s1 + 0x58) = temp_v0_3;
        *(s32 *) (var_s1 + 0x3C) = temp_v0_3;
        *(s32 *) (var_s1 + 0xC) = temp_v0_3;
        *(s32 *) (var_s1 + 0x30) = temp_v0_3;
        *(s32 *) (var_s1 + 0x20) = temp_v0_3;
        *(s32 *) (var_s1 + 0x14) = temp_v0_3;
        *(s32 *) (var_s1 + 0x1C) = temp_v1_8;
        *(s32 *) (var_s1 + 0x38) = temp_v1_8;
        *(s32 *) (var_s1 + 0x54) = temp_v1_8;
        *(s32 *) (var_s1 + 0x70) = temp_v1_8;
        *(s32 *) (var_s1 + 0x74) = temp_a1copy;
        *(s16 *) (var_s1 + 0x6E) = (s16) (ybase + yoff);
        temp_v1_9 = (s32) rec->unk8 >> 7;
        temp_a1_2 = (s32 *) (var_s1 + 0x1C);
        if (temp_v1_9 < 0)
        {
            *(s32 *) var_s1 = (*(s32 *) var_s1 & mask_high) | (base[0] & mask_low);
            base[0] = (base[0] & mask_high) | ((s32) var_s1 & mask_low);
            var_s1 = (u8 *) temp_a1_2;
        }
        else if (temp_v1_9 >= 0x1000)
        {
            *(s32 *) var_s1 = (*(s32 *) var_s1 & mask_high) | (base[0xFFF] & mask_low);
            base[0xFFF] = (base[0xFFF] & mask_high) | ((s32) var_s1 & mask_low);
            var_s1 = (u8 *) temp_a1_2;
        }
        else
        {
            *(s32 *) var_s1 = (*(s32 *) var_s1 & mask_high) | (base[temp_v1_9] & mask_low);
            temp_v1_10 = (s32 *) ((((s32) rec->unk8 >> 7) * 4) + (s32) base);
            ot_word = *temp_v1_10;
            *temp_v1_10 = (ot_word & mask_high) | ((s32) var_s1 & mask_low);
            var_s1 = (u8 *) temp_a1_2;
        }

        temp_v1_11 = (s32) rec->unk8 >> 7;
        if (temp_v1_11 < 0)
        {
            *(s32 *) var_s1 = (*(s32 *) var_s1 & mask_high) | (base[0] & mask_low);
            base[0] = (base[0] & mask_high) | ((s32) var_s1 & mask_low);
            var_s1 += 0x1C;
        }
        else if (temp_v1_11 >= 0x1000)
        {
            *(s32 *) var_s1 = (*(s32 *) var_s1 & mask_high) | (base[0xFFF] & mask_low);
            base[0xFFF] = (base[0xFFF] & mask_high) | ((s32) var_s1 & mask_low);
            var_s1 += 0x1C;
        }
        else
        {
            *(s32 *) var_s1 = (*(s32 *) var_s1 & mask_high) | (base[temp_v1_11] & mask_low);
            temp_v1_12 = (s32 *) ((((s32) rec->unk8 >> 7) * 4) + (s32) base);
            ot_word = *temp_v1_12;
            *temp_v1_12 = (ot_word & mask_high) | ((s32) var_s1 & mask_low);
            var_s1 += 0x1C;
        }

        temp_v1_13 = (s32) rec->unk8 >> 7;
        if (temp_v1_13 < 0)
        {
            *(s32 *) var_s1 = (*(s32 *) var_s1 & mask_high) | (base[0] & mask_low);
            base[0] = (base[0] & mask_high) | ((s32) var_s1 & mask_low);
            var_s1 += 0x1C;
        }
        else if (temp_v1_13 >= 0x1000)
        {
            *(s32 *) var_s1 = (*(s32 *) var_s1 & mask_high) | (base[0xFFF] & mask_low);
            base[0xFFF] = (base[0xFFF] & mask_high) | ((s32) var_s1 & mask_low);
            var_s1 += 0x1C;
        }
        else
        {
            *(s32 *) var_s1 = (*(s32 *) var_s1 & mask_high) | (base[temp_v1_13] & mask_low);
            temp_v1_14 = (s32 *) ((((s32) rec->unk8 >> 7) * 4) + (s32) base);
            ot_word = *temp_v1_14;
            *temp_v1_14 = (ot_word & mask_high) | ((s32) var_s1 & mask_low);
            var_s1 += 0x1C;
        }

        temp_v1_15 = (s32) rec->unk8 >> 7;
        if (temp_v1_15 < 0)
        {
            *(s32 *) var_s1 = (*(s32 *) var_s1 & mask_high) | (base[0] & mask_low);
            base[0] = (base[0] & mask_high) | ((s32) var_s1 & mask_low);
            var_s1 += 0x1C;
        }
        else if (temp_v1_15 >= 0x1000)
        {
            *(s32 *) var_s1 = (*(s32 *) var_s1 & mask_high) | (base[0xFFF] & mask_low);
            base[0xFFF] = (base[0xFFF] & mask_high) | ((s32) var_s1 & mask_low);
            var_s1 += 0x1C;
        }
        else
        {
            *(s32 *) var_s1 = (*(s32 *) var_s1 & mask_high) | (base[temp_v1_15] & mask_low);
            temp_v1_16 = (s32 *) ((((s32) rec->unk8 >> 7) * 4) + (s32) base);
            ot_word = *temp_v1_16;
            *temp_v1_16 = (ot_word & mask_high) | ((s32) var_s1 & mask_low);
            var_s1 += 0x1C;
        }
        if (var_s7 == (var_fp - 1))
        {
            goto loop_exit;
        }
        var_s7 += 1;
        sp60 += var_s6;
        sp64 += var_s6;
        sp68 += var_s6;
        goto loop_19;
    }
loop_exit:
    func_8007DA80(rec, sp50, var_s1, base);
}

extern s32 D_800F22A0_A __asm__("D_800F22A0");
extern s32 D_800F22A0_B __asm__("D_800F22A0");
extern s32 D_800F22A4_A __asm__("D_800F22A4");
extern s32 D_800F22A4_B __asm__("D_800F22A4");
extern s32 D_800F22A8_A __asm__("D_800F22A8");
extern s32 D_800F22A8_B __asm__("D_800F22A8");

/**
 * @brief Field single-primitive marker builder: places one billboard
 *        primitive at the actor's camera-relative pan position, or (when
 *        the part's owner tracking flag is clear and its unk4 bit 3 is set)
 *        at a fixed local-space offset transformed through the actor's
 *        rotation matrix via the GTE.
 * @param rec Effect record.
 * @param primbuf Output primitive buffer; advanced by one primitive (0x14
 *                bytes).
 * @param base Ordering-table / primitive base array.
 * @see decomp.me (100%)
 */
void func_8007AA2C(Struct_D800FDF58 *rec, u8 *primbuf, s32 *base)
{
    FieldSVector pan;
    FieldSVector offset;
    FieldVector gte_out;
    FieldVector point;
    FieldMatrix matrix;
    FieldActorState *actor;
    FieldActorPartDef *part;

    part = &g_field_actor_slots[rec->unk22].unk0[rec->unk23];
    actor = &g_field_actor_slots[rec->unk22];
    func_8007D078(rec, part, &matrix, actor);
    gte_SetRotMatrix(&matrix);

    pan.unk0 = (s16)(0xA0 + D_800F22A0_A / 256 + rec->unk0 / 256);
    pan.unk2 = (s16)(0x70 + D_800F22A4_A / 256 + rec->unk4 / 256 - rec->unk8 / 512 - D_800F22A8_A / 512);

    func_8007D8D8(actor, rec, part, primbuf + 4);
    setLineG2((LINE_G2 *)primbuf);
    setSemiTrans((LINE_G2 *)primbuf, rec->unk1C & 0x800000);
    *(s32 *)(primbuf + 0xC) = 0;

    if (rec->unk1B != 0)
    {
        *(s32 *)(primbuf + 8) = *(s32 *)&pan;
        func_80073F7C(rec, part, &point);
        pan.unk0 = (s16)(0xA0 + D_800F22A0_B / 256 + point.vx / 256);
        pan.unk2 = (s16)(0x70 + D_800F22A4_B / 256 + point.vy / 256 - point.vz / 512 - D_800F22A8_B / 512);
        *(s32 *)(primbuf + 0x10) = *(s32 *)&pan;
    }
    else if ((part->unk4 >> 3) & 1)
    {
        offset.unk0 = -0x1E;
        offset.unk2 = 0;
        offset.unk4 = 0;
        gte_ldv0(&offset);
        gte_rtv0();
        gte_stlvnl(&gte_out);
        *(s16 *)(primbuf + 8) = pan.unk0 + (u16)gte_out.vx;
        {
            s16 y = pan.unk2 + (u16)gte_out.vy;
            *(s32 *)(primbuf + 0x10) = *(s32 *)&pan;
            *(s16 *)(primbuf + 0xA) = y;
        }
    }
    else
    {
        *(s32 *)(primbuf + 0x10) = *(s32 *)&pan;
        *(s32 *)(primbuf + 8) = *(s32 *)&pan;
    }

    {
        s32 index;
        index = rec->unk8 >> 7;
        if (index < 0)
        {
            *(s32 *)primbuf = (*(s32 *)primbuf & 0xFF000000) | (base[0] & 0xFFFFFF);
            base[0] = (base[0] & 0xFF000000) | ((s32)primbuf & 0xFFFFFF);
            primbuf += 0x14;
        }
        else if (index >= 0x1000)
        {
            *(s32 *)primbuf = (*(s32 *)primbuf & 0xFF000000) | (base[0xFFF] & 0xFFFFFF);
            base[0xFFF] = (base[0xFFF] & 0xFF000000) | ((s32)primbuf & 0xFFFFFF);
            primbuf += 0x14;
        }
        else
        {
            { s32 otval = base[index]; *(s32 *)primbuf = (*(s32 *)primbuf & 0xFF000000) | (otval & 0xFFFFFF); }
            base[rec->unk8 >> 7] = (base[rec->unk8 >> 7] & 0xFF000000) | ((s32)primbuf & 0xFFFFFF);
            primbuf += 0x14;
        }
    }
    func_8007DA80(rec, part, primbuf, base);
}
