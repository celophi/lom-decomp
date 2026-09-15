/** @file field_effect_primitives.c
 * @brief Build related ring, fan, trail, and burst effect primitives.
 */

/* field13 */
/**
 * @file field13.c
 * @brief Field animation-frame audio/visual processor, carved from the top
 *        of the unk2 segment (the single-function slot right after
 *        field12.c's func_80077FB4).
 */

#include "common.h"
#include "field_types.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

typedef struct
{
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
} FieldSVector;

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

#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

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
        field_resolve_effect_position(rec, part, &point);
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


/* field14 */
#include "common.h"
#include "field_types.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"









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
 * @see decomp.me (100%)
 */
u8 *func_8007AE2C(Struct_D800FDF58 *rec, u8 *primbuf, s32 *base)
{
    FieldActorState *state;
    FieldActorPartDef *part;
    s32 temp_v1_2;
    s32 first_d0;
    s32 temp_x;
    s32 raw_d4;

    part = &g_field_actor_slots[rec->unk22].unk0[rec->unk23];
    state = &g_field_actor_slots[rec->unk22];

    if (rec->unk3D != 0xFF && D_800FF658[rec->unk3D].unk25 != 0xFF)
    {
        first_d0 = D_800F22A0 / 256;
        temp_x = rec->unk0 / 256 + 0xA0;
        raw_d4 = D_800F22A4;
        *(u16 *) (primbuf + 0x8) = (u16) (first_d0 + temp_x);

        *(u16 *) (primbuf + 0xA) = (u16) (0x70 + raw_d4 / 256 + rec->unk4 / 256 - rec->unk8 / 512 - D_800F22A8 / 512);

        *(s32 *) (primbuf + 0x10) = *(s32 *) (primbuf + 0x8);
        *(u16 *) (primbuf + 0x8) = (u16) (*(u16 *) (primbuf + 0x8) - (u16) rec->unk44);
        *(u16 *) (primbuf + 0xA) = (u16) (*(u16 *) (primbuf + 0xA) - (u16) rec->unk48);
        *(u16 *) (primbuf + 0x10) = (u16) (*(u16 *) (primbuf + 0x10) + (u16) rec->unk44);
        *(u16 *) (primbuf + 0x12) = (u16) (*(u16 *) (primbuf + 0x12) + (u16) rec->unk48);

        *(u16 *) (primbuf + 0xC) = (u16) (D_800F22A0 / 256 + (D_800FF658[rec->unk3D].unk0 / 256 + 0xA0));

        *(u16 *) (primbuf + 0xE) = (u16) (0x70 + raw_d4 / 256 + D_800FF658[rec->unk3D].unk4 / 256 - D_800FF658[rec->unk3D].unk8 / 512 - D_800F22A8 / 512);

        *(s32 *) (primbuf + 0x14) = *(s32 *) (primbuf + 0xC);
        *(u16 *) (primbuf + 0xC) = (u16) (*(u16 *) (primbuf + 0xC) - (u16) D_800FF658[rec->unk3D].unk44);
        *(u16 *) (primbuf + 0xE) = (u16) (*(u16 *) (primbuf + 0xE) - (u16) D_800FF658[rec->unk3D].unk48);
        *(u16 *) (primbuf + 0x14) = (u16) (*(u16 *) (primbuf + 0x14) + (u16) D_800FF658[rec->unk3D].unk44);
        *(u16 *) (primbuf + 0x16) = (u16) (*(u16 *) (primbuf + 0x16) + (u16) D_800FF658[rec->unk3D].unk48);

        func_8007D8D8(state, rec, part, primbuf + 4);

        setPolyF4((POLY_F4 *) primbuf);
        setSemiTrans((POLY_F4 *) primbuf, rec->unk1C & 0x800000);

        temp_v1_2 = (s32) rec->unk8 >> 7;
        if (temp_v1_2 < 0)
        {
            addPrim(&base[0], (POLY_F4 *) primbuf);
            primbuf += sizeof(POLY_F4);
        }
        else if (temp_v1_2 >= 0x1000)
        {
            addPrim(&base[0xFFF], (POLY_F4 *) primbuf);
            primbuf += sizeof(POLY_F4);
        }
        else
        {
            addPrim(&base[(s32) rec->unk8 >> 7], (POLY_F4 *) primbuf);
            primbuf += sizeof(POLY_F4);
        }

        primbuf = func_8007DA80(rec, part, primbuf, base);
    }

    return primbuf;
}

#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

/**
 * @brief Field radial fan primitive builder: transforms a ring of
 *        rec->unk24 (or 1, if unset) directions through the actor's
 *        rotation matrix via the GTE and emits one triangle primitive per
 *        segment connecting a shared hub vertex to each rim point,
 *        threading each into the depth-indexed ordering table in base[].
 * @param rec Effect record supplying the position/rotation/segment-count
 *            (unk24) fields.
 * @param primbuf Output primitive buffer; advanced by one primitive (0x10
 *                bytes) per fan segment.
 * @param base Depth-indexed ordering-table / primitive base array.
 * @return The advanced primbuf cursor.
 * @see decomp.me (100%)
 */
u8 *func_8007B29C(Struct_D800FDF58 *rec, u8 *primbuf, s32 *base)
{
    FieldMatrix *mtx;
    FieldSVector *dir;
    FieldActorPartDef *part;
    FieldVector *gte_out;
    s32 radius;
    Vec2s *base_screen;
    FieldActorState *state;
    u8 *var_s1;
    s32 *temp_v0;
    s32 angle;
    s32 segments;
    s32 hub_x;
    s32 i;
    s32 var_v0;
    s32 var_v1;
    s32 var_a0;
    s32 var_t0;
    s32 temp_v1;
    s8 var_v0_6;
    s8 var_v0_7;

    gte_out = (FieldVector *) 0x1F800010;
    base_screen = (Vec2s *) 0x1F800020;
    dir = (FieldSVector *) 0x1F800050;
    mtx = (FieldMatrix *) 0x1F800058;

    part = &g_field_actor_slots[rec->unk22].unk0[rec->unk23];
    state = &g_field_actor_slots[rec->unk22];

    base_screen->x = (s16) (0xA0 + D_800F22A0 / 0x100 + rec->unk0 / 0x100);
    base_screen->y = (s16) (0x70 + D_800F22A4 / 0x100 + rec->unk4 / 0x100 - rec->unk8 / 0x200 - D_800F22A8 / 0x200);

    func_8007D078(rec, part, mtx, state);
    gte_SetRotMatrix(mtx);

    var_v0 = 0xA0 + D_800F22A0 / 0x100 + rec->unk0 / 0x100;
    var_v1 = D_800F22A4;
    *(s16 *) (primbuf + 0x8) = (s16) var_v0;
    if (var_v1 < 0)
    {
        var_v1 += 0xFF;
    }
    *(s16 *) (primbuf + 0xA) = (s16) (0x70 + (var_v1 >> 8) + rec->unk4 / 0x100 - rec->unk8 / 0x200 - D_800F22A8 / 0x200);

    func_8007D8D8(state, rec, part, primbuf + 4);

    segments = 1;
    if (rec->unk24 != 0)
    {
        segments = rec->unk24;
    }

    radius = (u32) ((part->unk23 + 1) * 5) >> 4;

    dir->unk0 = (s16) ((u32) (rsin(0) * 5) >> 8);
    dir->unk2 = 0;
    dir->unk4 = (s16) ((s32) (rcos(0) * 0x50) >> 0xC);

    gte_ldv0(dir);
    gte_rtv0();
    gte_stlvnl(gte_out);

    *(s16 *) (primbuf + 0x8) = (s16) (base_screen->x + *(s16 *) &gte_out->vx);
    *(s16 *) (primbuf + 0xA) = (s16) (base_screen->y + *(s16 *) &gte_out->vy);
    hub_x = *(s32 *) (primbuf + 0x8);

    i = 1;
    if (i < segments)
    {
        do
        {
        var_s1 = primbuf + 0x14;
        do { angle = i << 12; } while (0);
        if (i & 1)
        {
            angle /= segments;
            dir->unk0 = (s16) ((rsin(angle) * radius) >> 0xC);
            dir->unk2 = 0;
            dir->unk4 = (s16) ((rcos(angle) * radius) >> 0xC);
        }
        else
        {
            angle /= segments;
            dir->unk0 = (s16) ((u32) (rsin(angle) * 5) >> 8);
            dir->unk2 = 0;
            dir->unk4 = (s16) ((s32) (rcos(angle) * 0x50) >> 0xC);
        }

            *(s8 *) (var_s1 - 0x11) = 3;
            *(s8 *) (var_s1 - 0xD) = 0x40;
            ((rec->unk1C & 0x800000) ?
             (*(u8 *)(var_s1 - 0xD) = *(u8 *)(var_s1 - 0xD) | 2) :
             (*(u8 *)(var_s1 - 0xD) = *(u8 *)(var_s1 - 0xD) & ~2));

            gte_ldv0(dir);
            gte_rtv0();
            gte_stlvnl(gte_out);

            *(s16 *) (var_s1 - 0x8) = (s16) (base_screen->x + *(s16 *) &gte_out->vx);
            *(s16 *) (var_s1 - 0x6) = (s16) (base_screen->y + *(s16 *) &gte_out->vy);
            *(s32 *) (primbuf + 0x18) = *(s32 *) (primbuf + 0x0C);
            *(s32 *) (primbuf + 0x14) = *(s32 *) (primbuf + 0x04);

            temp_v1 = (s32) rec->unk8 >> 7;
            if (temp_v1 < 0)
            {
                s32 addr;
                addr = (s32) primbuf & 0xFFFFFF;
                *(s32 *) (primbuf + 0) = (*(s32 *) (primbuf + 0) & 0xFF000000) | (base[0] & 0xFFFFFF);
                primbuf += 0x10;
                base[0] = (base[0] & 0xFF000000) | addr;
            }
            else if (temp_v1 >= 0x1000)
            {
                s32 addr;
                addr = (s32) primbuf & 0xFFFFFF;
                *(s32 *) (primbuf + 0) = (*(s32 *) (primbuf + 0) & 0xFF000000) | (base[0xFFF] & 0xFFFFFF);
                primbuf += 0x10;
                base[0xFFF] = (base[0xFFF] & 0xFF000000) | addr;
            }
            else
            {
                s32 addr;
                s32 *entry;
                addr = (s32) primbuf & 0xFFFFFF;
                *(s32 *) (primbuf + 0) = (*(s32 *) (primbuf + 0) & 0xFF000000) | (base[temp_v1] & 0xFFFFFF);
                entry = (s32 *) ((((s32) rec->unk8 >> 7) << 2) + (s32) base);
                primbuf += 0x10;
                *entry = (*entry & 0xFF000000) | addr;
            }
                i++;
        } while (i < segments);
    }

    *(s8 *) (primbuf + 3) = 3;
    *(s8 *) (primbuf + 7) = 0x40;
    ((rec->unk1C & 0x800000) ? (*(u8 *)(primbuf + 7) = *(u8 *)(primbuf + 7) | 2) : (*(u8 *)(primbuf + 7) = *(u8 *)(primbuf + 7) & ~2));
    *(s32 *) (primbuf + 0xC) = hub_x;

    temp_v1 = (s32) rec->unk8 >> 7;
    if (temp_v1 < 0)
    {
        s32 addr;
        addr = (s32) primbuf & 0xFFFFFF;
        *(s32 *) (primbuf + 0) = (*(s32 *) (primbuf + 0) & 0xFF000000) | (base[0] & 0xFFFFFF);
        primbuf += 0x10;
        base[0] = (base[0] & 0xFF000000) | addr;
    }
    else if (temp_v1 >= 0x1000)
    {
        s32 addr;
        addr = (s32) primbuf & 0xFFFFFF;
        *(s32 *) (primbuf + 0) = (*(s32 *) (primbuf + 0) & 0xFF000000) | (base[0xFFF] & 0xFFFFFF);
        primbuf += 0x10;
        base[0xFFF] = (base[0xFFF] & 0xFF000000) | addr;
    }
    else
    {
        s32 addr; s32 *entry; s32 srcval;
        addr = (s32) primbuf & 0xFFFFFF;
        srcval = base[temp_v1];
        *(s32 *) primbuf = (*(s32 *) primbuf & 0xFF000000) | (srcval & 0xFFFFFF);
        entry = (s32 *) ((((s32) rec->unk8 >> 7) << 2) + (s32) base);
        primbuf += 0x10;
        *entry = (*entry & 0xFF000000) | addr;
    }

    primbuf = func_8007DA80(rec, part, primbuf, base);

    return primbuf;
}


/* field15 */
#include "common.h"
#include "field_types.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"









extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;
extern FieldActorState g_field_actor_slots[80];

#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

/**
 * @brief Field radial burst primitive builder: builds a fan of segments
 *        random-rotation matrices in the scratchpad, transforms a shared
 *        direction vector through each of them, and emits a connected strip
 *        of LINE_F2 primitives from the actor's origin outwards, threading
 *        each one into the depth-indexed ordering table in base[].
 * @param rec Effect record supplying the position/flag/segment-count (unk24)
 *            fields.
 * @param primbuf Output primitive buffer; advanced by one primitive (0x10
 *                bytes) per emitted segment.
 * @param base Depth-indexed ordering-table / primitive base array.
 * @return The advanced primbuf cursor (as returned by func_8007DA80).
 * @note WIP - all 639 instruction positions and stack accesses match.
 *       Five remaining operand differences use a1 rather than a2 for the
 *       final scratchpad Z value. The long-lived origin pointer preserves
 *       the six independent scratchpad loads. Packet address bitfields and
 *       the initial cur uses recover the target saved-register allocation.
 *       Current evidence and rejected probes are in working/func_8007B9FC/.
 * @see decomp.me (99.95%) WIP
 */
u8 *func_8007B9FC(Struct_D800FDF58 *rec, u8 *primbuf, s32 *base)
{
    FieldActorPartDef *part;
    FieldActorState *state;
    FieldVector *gte_out;
    FieldVector *ptr_a;
    FieldVector *ptr_b;
    FieldVector *ptr_c;
    FieldSVector *dir;
    FieldMatrix *cur;
    u8 *p2;
    s32 segments;
    s32 i;
    s32 amp;
    s32 step;
    s32 temp_v1;
    FieldVector *origin;

    gte_out = (FieldVector *) 0x1F800010;
    ptr_a = (FieldVector *) 0x1F800020;
    ptr_b = (FieldVector *) 0x1F800030;
    ptr_c = (FieldVector *) 0x1F800040;
    dir = (FieldSVector *) 0x1F800050;

    part = &g_field_actor_slots[rec->unk22].unk0[rec->unk23];
    state = &g_field_actor_slots[rec->unk22];

    cur = (FieldMatrix *)0x1F800058;
    func_8007D078(rec, part, cur, state);
    gte_SetRotMatrix(cur);

    {
        s32 first_d0;
        s32 temp_x;
        s32 raw_d4;
        first_d0 = D_800F22A0 / 256;
        temp_x = rec->unk0 / 256;
        raw_d4 = D_800F22A4;
        *(s16 *) (primbuf + 0x8) = first_d0 + (s16) (temp_x + 0xA0);
        if (raw_d4 < 0)
        {
            raw_d4 += 255;
        }
        *(s16 *) (primbuf + 0xA) = 0x70 + (raw_d4 >> 8) + rec->unk4 / 256 - rec->unk8 / 512 - D_800F22A8 / 512;
    }

    func_8007D8D8(state, rec, part, primbuf + 4);

    *(s8 *) (primbuf + 3) = 3;
    *(s8 *) (primbuf + 7) = 0x40;
    ((rec->unk1C & 0x800000) ? (*(u8 *) (primbuf + 7) = *(u8 *) (primbuf + 7) | 2) : (*(u8 *) (primbuf + 7) = *(u8 *) (primbuf + 7) & ~2));

    cur = (FieldMatrix *) 0x1F800058;

    segments = 0x14;
    if (rec->unk24 < 0x14)
    {
        segments = rec->unk24;
    }
    if (segments <= 0)
    {
        segments = 1;
    }
    step = 0x800 / segments;

    field_resolve_effect_position(rec, part, (void *) 0x1F800000);

    i = segments - 1;

    origin = (FieldVector *)0x1F800000;
    ptr_a->vx = (origin->vx + rec->unk0) >> 1;
    ptr_a->vy = origin->vy;
    ptr_a->vz = (origin->vz + rec->unk8) >> 1;
    ptr_b->vx = (origin->vx - rec->unk0) >> 1;
    ptr_b->vy = rec->unk4 - origin->vy;
    ptr_b->vz = (origin->vz - rec->unk8) >> 1;

    if (i > 0)
    {
        do
        {
            dir->unk0 = 0;
            dir->unk2 = (s16) ((rand() << 12) >> 15);
            dir->unk4 = (s16) ((rand() << 12) >> 16);
            RotMatrix_gte(dir, cur);
            i--;
            cur++;
        } while (i > 0);
    }

    cur = (FieldMatrix *) 0x1F800058;

    if ((*(u8 *) &part->unk4) >> 7)
    {
        dir->unk0 = 0;
        dir->unk2 = (s16) ((part->unk4 >> 28) << 8);
        dir->unk4 = 0;
    }
    else
    {
        *(s32 *) &dir->unk4 = 0;
        *(s32 *) &dir->unk0 = 0;
    }

    if (((part->unk0 >> 6) & 3) != 0)
    {
        amp = (part->unk0 >> 26) << 9;
    }
    else
    {
        amp = 0;
    }

    i = segments - 1;
    if (i > 0)
    {
        p2 = primbuf + 0x10;
        do
        {
            *(s32 *) (p2 + 0x4) = *(s32 *) (p2 - 0xC);
            *(s8 *) (p2 - 0xD) = 3;
            *(s8 *) (p2 - 0x9) = 0x40;
            ((rec->unk1C & 0x800000) ? (*(u8 *) (p2 - 0x9) = *(u8 *) (p2 - 0x9) | 2) : (*(u8 *) (p2 - 0x9) = *(u8 *) (p2 - 0x9) & ~2));

            gte_SetRotMatrix(cur);
            gte_ldv0(dir);
            gte_rtv0();
            gte_stlvnl(ptr_c);

            if (amp != 0)
            {
                gte_out->vy = ptr_a->vy + (ptr_b->vy * i) / segments - ((s32) (((part->unk0 >> 26) << 9) * rsin(i * step)) >> 12) + ptr_c->vy;
            }
            else
            {
                gte_out->vy = ptr_a->vy + (ptr_b->vy * i) / segments + ptr_c->vy;
            }

            gte_out->vx = ((ptr_b->vx * rcos(i * step)) >> 12) + ptr_a->vx + ptr_c->vx;
            gte_out->vz = ((ptr_b->vz * rcos(i * step)) >> 12) + ptr_a->vz + ptr_c->vz;

            {
                s32 first_d0;
                s32 temp_x;
                s32 raw_d4;
                first_d0 = D_800F22A0 / 256;
                temp_x = gte_out->vx / 256;
                raw_d4 = D_800F22A4;
                *(s16 *) (p2 - 0x4) = first_d0 + (s16) (temp_x + 0xA0);
                if (raw_d4 < 0)
                {
                    raw_d4 += 255;
                }
                *(s16 *) (p2 - 0x2) = 0x70 + (raw_d4 >> 8) + gte_out->vy / 256 - gte_out->vz / 512 - D_800F22A8 / 512;
            }
            *(s32 *) (p2 + 0x8) = *(s32 *) (p2 - 0x4);

            temp_v1 = (s32) rec->unk8 >> 7;
            if (temp_v1 < 0)
            {
                s32 addr;
                p2 += 0x10;
                addr = (s32) primbuf & 0xFFFFFF;
                ((P_TAG *)primbuf)->addr = ((P_TAG *)&base[0])->addr;
                primbuf += 0x10;
                base[0] = (base[0] & 0xFF000000) | addr;
            }
            else if (temp_v1 >= 0x1000)
            {
                s32 addr;
                p2 += 0x10;
                addr = (s32) primbuf & 0xFFFFFF;
                ((P_TAG *)primbuf)->addr = ((P_TAG *)&base[0xFFF])->addr;
                primbuf += 0x10;
                base[0xFFF] = (base[0xFFF] & 0xFF000000) | addr;
            }
            else
            {
                s32 addr;
                s32 *entry;
                p2 += 0x10;
                addr = (s32) primbuf & 0xFFFFFF;
                ((P_TAG *)primbuf)->addr = ((P_TAG *)&base[temp_v1])->addr;
                entry = (s32 *) ((((s32) rec->unk8 >> 7) << 2) + (s32) base);
                primbuf += 0x10;
                *entry = (*entry & 0xFF000000) | addr;
            }

            i--;
            cur++;
        } while (i > 0);
    }

    *(s8 *) (primbuf + 3) = 3;
    *(s8 *) (primbuf + 7) = 0x40;
    ((rec->unk1C & 0x800000) ? (*(u8 *) (primbuf + 7) = *(u8 *) (primbuf + 7) | 2) : (*(u8 *) (primbuf + 7) = *(u8 *) (primbuf + 7) & ~2));

    {
        s32 first_d0;
        s32 temp_x;
        s32 raw_d4;
        first_d0 = D_800F22A0 / 256;
        temp_x = origin->vx / 256;
        raw_d4 = D_800F22A4;
        *(s16 *) (primbuf + 0xC) = first_d0 + (s16) (temp_x + 0xA0);
        if (raw_d4 < 0)
        {
            raw_d4 += 255;
        }
        raw_d4 = 0x70 + (raw_d4 >> 8) + origin->vy / 256;
        raw_d4 -= origin->vz / 512;
        *(s16 *) (primbuf + 0xE) = raw_d4 - D_800F22A8 / 512;
    }

    temp_v1 = (s32) rec->unk8 >> 7;
    if (temp_v1 < 0)
    {
        s32 addr;
        addr = (s32) primbuf & 0xFFFFFF;
        ((P_TAG *)primbuf)->addr = ((P_TAG *)&base[0])->addr;
        primbuf += 0x10;
        base[0] = (base[0] & 0xFF000000) | addr;
    }
    else if (temp_v1 >= 0x1000)
    {
        s32 addr;
        addr = (s32) primbuf & 0xFFFFFF;
        ((P_TAG *)primbuf)->addr = ((P_TAG *)&base[0xFFF])->addr;
        primbuf += 0x10;
        base[0xFFF] = (base[0xFFF] & 0xFF000000) | addr;
    }
    else
    {
        s32 addr;
        s32 *entry;
        addr = (s32) primbuf & 0xFFFFFF;
        ((P_TAG *)primbuf)->addr = ((P_TAG *)&base[temp_v1])->addr;
        entry = (s32 *) ((((s32) rec->unk8 >> 7) << 2) + (s32) base);
        primbuf += 0x10;
        *entry = (*entry & 0xFF000000) | addr;
    }

    primbuf = func_8007DA80(rec, part, primbuf, base);

    return primbuf;
}
