/**
 * @file field10.c
 * @brief Field particle-record render dispatch, carved from the top of the
 *        unk2 segment.
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
    u8 unk1E;  /* 0x1E */
    u8 pad1F;
    u8 unk20;
    u8 unk21;
    u8 unk22;
    u8 unk23;
    s32 unk24;
    u32 unk28;
    u8 unk2C;
    u8 unk2D;
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
    FieldActorPartDef *unk0;
    u8 pad4[0xC - 4];
    u8 *unkC;
    u8 pad10[0x14 - 0x10];
    u8 *unk14;
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
    u8 unk228;
    u8 unk229[9];
    u8 unk232;
    u8 unk233;
    u16 unk234;
    u16 unk236;
    u8 pad238[2];
    u8 unk23A;
    u8 unk23B;
    u8 pad23C[0x240 - 0x23C];
    u16 *unk240;
} FieldActorState;

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
    u8 pad148[0x16D - 0x148];
    s8 unk16D;   /* 0x16D */
    u8 pad16E[0x174 - 0x16E];
    u16 unk174;  /* 0x174 */
    u8 pad176[0x178 - 0x176];
    u8 unk178;   /* 0x178 */
    u8 pad179[0x18E - 0x179];
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
    u8 *start; /* 0x00 */
    u8 *end;   /* 0x04 */
    u8 unk8;   /* 0x08 */
    u8 slot_index; /* 0x09 */
    u8 padA[0xE - 0xA];
    s16 unkE;  /* 0x0E */
    u32 flags; /* 0x10 */
} FieldResourceEntry;

typedef struct
{
    u8 pad0[0x40];
    u32 unk40;   /* 0x40 */
    u8 pad44[0x40B8 - 0x44];
    s32 unk40B8; /* 0x40B8 */
} FieldRenderContext;

extern FieldActorState g_field_actor_slots[80];
extern Struct_D800FDF58 D_800FDF58[];
extern Struct_D800FDF58 D_800FF658[256];
extern Struct_D80105AE0 D_80105AE0[];
extern FieldResourceEntry g_field_resource_entries[];
extern Struct_D800FDF58 D_80104A58;
extern s32 g_field_track_index;
extern u8 *D_801058D4;
extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;
extern u16 D_80105758[];

/**
 * @brief Render every active field particle record, dispatching per record
 *        on its unk25 kind selector.
 * @param ctx Render context; unk40 is the vertex-buffer cursor base, unk40B8
 *            is the current cursor offset threaded through each call.
 * @note WIP - not yet byte-matching (90.36%, 374/465 exact rows). Residual
 *       is a global register-allocation priority puzzle among the three
 *       loop-invariant globals D_800FDF58/D_80105AE0/g_field_resource_entries
 *       referenced once per "track resolve" case (2, 3, 0xFD): the target
 *       hoists D_800FDF58 and D_80105AE0 to persistent saved registers and
 *       recomputes g_field_resource_entries per use, but every statement
 *       reorder tried here flips which two of the three win persistent
 *       status without landing on that exact pair. sched_oracle confirms
 *       only one genuine LUID violation in the loop-invariant preamble
 *       (addiu D_800FDF58 wanting to precede addiu D_80105AE0), separate
 *       from the case-body reorders already tried. See idioms.md
 *       [EXPAND-35] for the switch case-body-order fix that got this from
 *       10% to 87% in one edit.
 * @see decomp.me (90.36%)
 */
void func_80074D7C(FieldRenderContext *ctx)
{
    Struct_D800FDF58 *rec;
    Struct_D80105AE0 *slot;
    FieldActorPartDef *part;
    u32 *base;
    u8 *resource_ptr;
    s32 cursor;
    s32 item;
    s32 track;
    s32 flags;

    rec = &D_80104A58;
    base = &ctx->unk40;
    cursor = ctx->unk40B8;

    if (D_800FF658 != rec)
    {
        rec = D_800FF658;
        do
        {
            g_field_track_index = rec->unk29;
            if (rec->unk25 != 0xFE && rec->unk25 != 0xFF)
            {
                switch (rec->unk25)
                {
                case 0xF6:
                    cursor = func_8007B9FC(rec, cursor, base);
                    break;

                case 0xF7:
                case 0xF8:
                case 0xF9:
                    if (g_field_actor_slots[rec->unk22].unk0[rec->unk23].unk23 < 8)
                    {
                        cursor = func_80081098(rec, 0xF9 - rec->unk25, cursor, base);
                    }
                    else
                    {
                        cursor = func_80080274(rec, 0xF9 - rec->unk25, cursor, base);
                    }
                    break;

                case 0xFA:
                    cursor = func_800799C4(rec, cursor, base);
                    break;

                case 0xFB:
                    cursor = func_8007A104(rec, cursor, base);
                    break;

                case 0xFC:
                    cursor = func_8007AA2C(rec, cursor, base);
                    break;

                case 0xFD:
                    part = &g_field_actor_slots[rec->unk22].unk0[rec->unk23];
                    flags = ((u32) part->unk28 >> 0x12) & 0x3F;
                    if (((u32) (flags - 0xA) < 0xA) || flags == 0x28)
                    {
                        track = g_field_actor_slots[rec->unk22].unk229[rec->unk29];
                    }
                    else
                    {
                        track = g_field_actor_slots[rec->unk22].unk228;
                    }
                    flags = D_800FDF58[track].unk3B;
                    slot = &D_80105AE0[track];
                    resource_ptr = g_field_resource_entries[flags].start;
                    slot->unk16D = -(s8) (rec - D_800FF658);
                    if (resource_ptr != 0)
                    {
                        item = func_8006C854(rec, resource_ptr);
                        if (item != 0)
                        {
                            if (item >= 0)
                            {
                                cursor = func_80077FB4(rec, cursor, base, item, (slot->unk178 & 1) ^ 1, part);
                            }
                            else
                            {
                                cursor = func_80075C88(rec, cursor, base, item, (slot->unk178 & 1) ^ 1, part);
                            }
                        }
                    }
                    break;

                case 0:
                    item = func_8006C854(rec, D_801058D4);
                    if (item != 0)
                    {
                        cursor = func_800754B4(rec, cursor, base, item);
                    }
                    break;

                case 1:
                    resource_ptr = g_field_actor_slots[rec->unk22].unk14;
                    if (resource_ptr != 0)
                    {
                        item = func_8006C854(rec, resource_ptr);
                        if (item != 0)
                        {
                            cursor = func_800754B4(rec, cursor, base, item);
                        }
                    }
                    break;

                case 2:
                    part = &g_field_actor_slots[rec->unk22].unk0[rec->unk23];
                    track = g_field_actor_slots[rec->unk22].unk228;
                    slot = &D_80105AE0[track];
                    flags = D_800FDF58[track].unk3B;
                    resource_ptr = g_field_resource_entries[flags].start;
                    slot->unk16D = -(s8) (rec - D_800FF658);
                    if (resource_ptr != 0)
                    {
                        item = func_8006C854(rec, resource_ptr);
                        if (item != 0)
                        {
                            if (item >= 0)
                            {
                                cursor = func_80077FB4(rec, cursor, base, item, (slot->unk178 & 1) ^ 1, part);
                            }
                            else
                            {
                                cursor = func_80075C88(rec, cursor, base, item, (slot->unk178 & 1) ^ 1, part);
                            }
                        }
                    }
                    break;

                case 3:
                    part = &g_field_actor_slots[rec->unk22].unk0[rec->unk23];
                    track = g_field_actor_slots[rec->unk22].unk229[rec->unk29];
                    slot = &D_80105AE0[track];
                    flags = D_800FDF58[track].unk3B;
                    resource_ptr = g_field_resource_entries[flags].start;
                    slot->unk16D = -(s8) (rec - D_800FF658);
                    if (resource_ptr != 0)
                    {
                        item = func_8006C854(rec, resource_ptr);
                        if (item != 0)
                        {
                            if (item >= 0)
                            {
                                cursor = func_80077FB4(rec, cursor, base, item, (slot->unk178 & 1) ^ 1, part);
                            }
                            else
                            {
                                cursor = func_80075C88(rec, cursor, base, item, (slot->unk178 & 1) ^ 1, part);
                            }
                        }
                    }
                    break;

                case 4:
                    cursor = func_8007B29C(rec, cursor, base);
                    break;

                case 5:
                    cursor = func_8007AE2C(rec, cursor, base);
                    break;

                case 6:
                    cursor = func_8007C3F8(rec, cursor, base);
                    break;
                }
            }
            rec++;
        } while (rec != &D_80104A58);
    }

    ctx->unk40B8 = cursor;
}

#include "psyq/inline_c.h"

/**
 * @brief Render one field particle record's animation frames into the display
 *        list, spawning per-frame billboard primitives.
 * @param rec Effect record.
 * @param cursor Vertex-buffer cursor pointer, threaded and returned.
 * @param base Ordering-table / primitive base array.
 * @param item Animation data blob for this frame.
 * @return Updated cursor pointer.
 * @note WIP - not yet byte-matching (72.24%, 166/501 exact rows). The whole
 *       prologue/register-save shape disagrees with the target from the
 *       first instruction: the target keeps a0/a1/a2/a3 live in s-registers
 *       through the gte_SetRotMatrix call and defers most callee-save stores,
 *       while this draft copies straight into t1/s3/t2/fp and stores saves
 *       up front. Residue is spread almost uniformly across the whole body
 *       (EXPAND-SHAPE, +9 insns overall) rather than isolated to one region,
 *       which points at a source-level register-role/variable-boundary
 *       mismatch rather than a local statement-order fix. Needs a fresh
 *       match_context pull (m2c-regvars view) before further iteration.
 * @see decomp.me (72.24%)
 */
s32 *func_800754B4(Struct_D800FDF58 *rec, s32 *cursor, s32 *base, u8 *item)
{
    Vec2s *sxy = (Vec2s *) 0x1F800040;
    s16 *scratch = (s16 *) 0x1F800064;
    s32 sp4C;
    s32 sp48;
    s32 *sp44;
    s32 *sp40;
    s32 sp38;
    s32 sp34;
    s32 sp30;
    s32 sp28;
    Struct_D800FDF58 *var_t1;
    s32 *var_s3;
    s32 *var_t2;
    FieldActorState *slot;
    FieldActorPartDef *part;
    u8 *var_fp;
    u8 *var_s2;
    u8 *var_s1;
    s32 count;
    s32 mode;
    s16 tmp;
    s8 var_s4;
    s8 var_s0;
    s8 var_v1_3;
    s8 var_t0;
    s32 var_t0_2;
    s32 v0;
    s32 v1;
    s32 a0;
    s32 a1;
    s32 x0;
    s32 x1;
    u16 *tbl;

    var_t1 = rec;
    var_s3 = cursor;
    var_t2 = base;
    sp34 = 0;
    sp38 = 2;
    slot = &g_field_actor_slots[rec->unk22];
    part = &slot->unk0[rec->unk23];
    if (slot->unk228 < 2)
    {
        sp38 = slot->unk228;
    }
    sp40 = (s32 *) var_t1;
    sp44 = var_t2;
    sp48 = (s32) slot;
    func_8007D078(var_t1, part, 0x1F800000, slot);
    gte_SetRotMatrix((void *) 0x1F800000);

    v0 = D_800F22A0;
    if (v0 < 0)
    {
        v0 += 0xFF;
    }
    v1 = var_t1->unk0;
    if (v1 < 0)
    {
        v1 += 0xFF;
    }
    a0 = D_800F22A4;
    sxy->x = (v0 >> 8) + ((v1 >> 8) + 0xA0);
    if (a0 < 0)
    {
        a0 += 0xFF;
    }
    v0 = var_t1->unk4;
    if (v0 < 0)
    {
        v0 += 0xFF;
    }
    a1 = var_t1->unk8;
    if (a1 < 0)
    {
        a1 += 0x1FF;
    }
    v1 = D_800F22A8;
    if (v1 < 0)
    {
        v1 += 0x1FF;
    }
    sxy->y = (((a0 >> 8) + ((v0 >> 8) + 0x70)) - (a1 >> 9)) - (v1 >> 9);

    var_fp = item + 1;
    sp40 = (s32 *) var_t1;
    sp44 = var_t2;
    sp48 = (s32) slot;
    sp30 = *item;
    func_8007D8D8(slot, var_t1, part, &sp28);

    if (sp30 != 0)
    {
        tbl = D_80105758;
        var_s2 = var_fp + 8;
        var_s1 = (u8 *) (var_s3 + 4);
        do
        {
            u8 flags = var_s2[-1];
            if (!(flags & 0x20))
            {
                var_s1[-1] = 9;
                *(s32 *) var_s1 = sp28;
                var_s1[3] = 0x2C;
                if (var_t1->unk1C & 0x800000)
                {
                    var_s1[3] = 0x2E;
                }
                else
                {
                    var_s1[3] = 0x2C;
                }
                var_s0 = var_s2[-4];
                var_s4 = var_s2[-3] - 1;
                var_t1->unk3C = var_s4;
                var_t0 = var_s2[-7];
                if (!(var_t1->unk21 & 0x80))
                {
                    var_v1_3 = *item;
                }
                else
                {
                    var_v1_3 = -*item - var_s0;
                }
                var_s0 -= 1;
                if (((Vec2s *) part)->y & 1)
                {
                    var_t0 -= 0x70;
                }
                sp40 = (s32 *) var_t1;
                sp44 = var_t2;
                sp48 = (s32) slot;
                func_8007DB98(var_t1, sxy, var_s3, var_s0, var_s4, var_v1_3, var_t0, item, 0x1F800000);

                if ((var_s2[-1] ^ (var_t1->unk21 >> 1)) & 0x40)
                {
                    v0 = var_s2[-6];
                    var_s1[0x20] = v0;
                    var_s1[0x10] = v0;
                    v0 += var_s0;
                    var_s1[0x18] = v0;
                    var_s1[8] = v0;
                }
                else
                {
                    v0 = var_s2[-6];
                    var_s1[0x18] = v0;
                    var_s1[8] = v0;
                    v0 += var_s0;
                    var_s1[0x20] = v0;
                    var_s1[0x10] = v0;
                }
                if (var_s2[-1] & 0x80)
                {
                    v0 = var_s2[-5];
                    var_s1[0x21] = v0;
                    var_s1[0x19] = v0;
                    v0 += var_s4;
                    var_s1[0x11] = v0;
                    var_s1[9] = v0;
                }
                else
                {
                    v0 = var_s2[-5];
                    var_s1[0x11] = v0;
                    var_s1[9] = v0;
                    v0 += var_s4;
                    var_s1[0x21] = v0;
                    var_s1[0x19] = v0;
                }
                mode = var_s2[-1] & 3;
                if (mode == 2)
                {
                    var_t0_2 = 0x1F2;
                    if (slot->unk228 < 2)
                    {
                        tmp = ((tbl[sp38] & 3) << 7) | (((u32) part->unk4 >> 0x11) & 0x60) | 0x10 | ((((slot->unk228 << 6) + 0x340) & 0x3FF) >> 6);
                        *(s16 *) (var_s1 + 0x12) = tmp;
                        var_t0_2 = (slot->unk228 * 2) + 0x1EE;
                    }
                    else
                    {
                        tmp = ((tbl[sp38] & 3) << 7) | (((u32) part->unk4 >> 0x11) & 0x60) | 5;
                        *(s16 *) (var_s1 + 0x12) = tmp;
                    }
                }
                else
                {
                    var_t0_2 = (mode * 2) + 0x1EA;
                    tmp = (((u32) part->unk4 >> 0x11) & 0x60) | ((((mode << 6) + 0x180) & 0x3FF) >> 6);
                    *(s16 *) (var_s1 + 0x12) = tmp;
                }

                if (((u32) part->unk0 >> 0x15) & 1)
                {
                    *(s16 *) (var_s1 + 0xA) = (var_t0_2 + 1) << 6;
                }
                else
                {
                    switch (((u32) part->unk28 >> 0xC) & 3)
                    {
                    case 1:
                        v0 = part->unk2D;
                        if (v0 >= 0x10)
                        {
                            x0 = (var_t0_2 + 1) << 6;
                        }
                        else
                        {
                            x0 = var_t0_2 << 6;
                        }
                        *(s16 *) (var_s1 + 0xA) = x0 | (v0 & 0xF);
                        break;
                    case 2:
                        x1 = var_t0_2 << 6;
                        if (slot->unk228 >= 3)
                        {
                            v0 = part->unk2D & 0x3F;
                        }
                        else
                        {
                            x1 = var_t0_2 << 6;
                            v0 = var_s2[-2] & 0x3F;
                        }
                        *(s16 *) (var_s1 + 0xA) = x1 | v0;
                        break;
                    case 0:
                        x1 = var_t0_2 << 6;
                        v0 = var_s2[-2] & 0x3F;
                        *(s16 *) (var_s1 + 0xA) = x1 | v0;
                        break;
                    }
                }

                if ((var_t1->unk1C & 0x1000) || ((v1 = var_t1->unk8 >> 7), v1 < 0))
                {
                    var_s1 += 0x28;
                    *var_s3 = (*var_s3 & 0xFF000000) | (var_t2[0] & 0xFFFFFF);
                    v1 = (s32) var_s3 & 0xFFFFFF;
                    var_s3 += 0x28;
                    var_t2[0] = (var_t2[0] & 0xFF000000) | v1;
                }
                else if (v1 >= 0x1000)
                {
                    var_s1 += 0x28;
                    *var_s3 = (*var_s3 & 0xFF000000) | (var_t2[0xFFF] & 0xFFFFFF);
                    v1 = (s32) var_s3 & 0xFFFFFF;
                    var_s3 += 0x28;
                    var_t2[0xFFF] = (var_t2[0xFFF] & 0xFF000000) | v1;
                }
                else
                {
                    var_s1 += 0x28;
                    *var_s3 = (*var_s3 & 0xFF000000) | (var_t2[v1] & 0xFFFFFF);
                    a0 = (s32) var_s3 & 0xFFFFFF;
                    {
                        s32 *p = &var_t2[var_t1->unk8 >> 7];
                        var_s3 += 0x28;
                        *p = (*p & 0xFF000000) | a0;
                    }
                }
            }
            else if (((flags & 0xF) == 2) && (part->unk24 & 0x100000))
            {
                scratch[0] = (*item * part->unk2E) >> 6;
                scratch[1] = ((s8) var_s2[-7] * part->unk33) >> 6;
                scratch[2] = ((s8) var_s2[-6] * part->unk2E) >> 6;
                scratch[3] = ((s8) var_s2[-5] * part->unk33) >> 6;
                scratch[4] = ((s8) var_s2[-4] * part->unk2E) >> 6;
                scratch[5] = ((s8) var_s2[-3] * part->unk33) >> 6;
                scratch[6] = ((s8) var_s2[-2] * part->unk2E) >> 6;
                sp34 += 1;
                scratch[7] = ((s8) var_s2[0] * part->unk33) >> 6;
            }
            var_s2 += 9;
            var_fp += 9;
            sp30 -= 1;
            *(s32 *) var_s1 = *(s32 *) (var_s1 - 0x28);
        } while (sp30 != 0);
    }

    if (sp34 != 0)
    {
        var_s3 = func_800871A0(var_t1, var_s3, var_t2, 0x1F800064);
    }
    return var_s3;
}
