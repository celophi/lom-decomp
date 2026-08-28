/**
 * @file field12.c
 * @brief Field animation-frame audio/visual processor, carved from the top
 *        of the unk2 segment (the single-function slot right after
 *        field11.c's func_80075C88).
 */

#include "common.h"

typedef struct
{
    s16 x;
    s16 y;
} Vec2s;
typedef struct
{
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0; u16 clut;
    s16 x1, y1;
    u8 u1, v1; u16 tpage;
    s16 x2, y2;
    u8 u2, v2; u16 pad1;
    s16 x3, y3;
    u8 u3, v3; u16 pad2;
} FieldPolyFT4;


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
    u32 unk3C; /* 0x3C */
    s32 unk40; /* 0x40 */
    u32 unk44; /* 0x44 */
    u32 unk48; /* 0x48 */
    u32 unk4C; /* 0x4C */
    u8 pad50[0x54 - 0x50];
} Struct_D800FDF58;

/* This function needs the full 32-bit unk34 (masked against 0x100000), unlike
 * the u16 view used by field8.c/field10.c's local copy of the same struct. */
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
    u8 pad0[0x14];
    u8 unk14; /* 0x14 */
    u8 unk15; /* 0x15 */
    u8 pad16;
    u8 unk17; /* 0x17 */
    u16 unk18;
    u16 unk1A;
} FieldActorAnimationDef;

typedef struct
{
    FieldActorPartDef *unk0;
    u8 pad4[0xC - 4];
    FieldActorAnimationDef *unkC;
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

/* Local view of the shared per-track "slot" record; only the fields this
 * function touches are named, matching the per-file minimal-view convention
 * already used by field7.c/field8.c/field10.c/field11.c for the same real
 * struct. */
typedef struct
{
    u8 pad0[0xC];
    s32 unkC;   /* 0x0C */
    u8 pad10[0x3C - 0x10];
    s32 unk3C;  /* 0x3C */
    u8 pad40[0x12C - 0x40];
    s16 unk12C; /* 0x12C */
    s16 unk12E; /* 0x12E */
    s16 unk130; /* 0x130 */
    s16 unk132; /* 0x132 */
    s16 unk134; /* 0x134 */
    s16 unk136; /* 0x136 */
    s16 unk138; /* 0x138 */
    s16 unk13A; /* 0x13A */
    s16 unk13C; /* 0x13C */
    s16 unk13E; /* 0x13E */
    s16 unk140; /* 0x140 */
    s16 unk142; /* 0x142 */
    s16 unk144; /* 0x144 */
    s16 unk146; /* 0x146 */
    u8 pad148[0x174 - 0x148];
    s32 unk174; /* 0x174 */
    s32 unk178; /* 0x178 (byte views at 0x179/0x17A/0x17B) */
    u8 pad17C[0x180 - 0x17C];
    u8 unk180;  /* 0x180 */
    u8 pad181[0x18D - 0x181];
    u8 unk18D;  /* 0x18D */
    u8 unk18E;  /* 0x18E */
    u8 pad18F[0x23C - 0x18F];
} Struct_D80105AE0;

typedef struct
{
    s16 unk0;
    u8 pad2[2];
    s16 unk4;
} Struct_D80105768;

typedef struct
{
    u8 *start;
    u8 *end;
    u8 unk8;
    u8 slot_index;
    u16 unkA;
    u8 padC[2];
    s16 unkE;
    u32 flags;
} FieldResourceEntry;

/* Output of func_80097150: resolved track/actor index plus the screen-space
 * x/y it computed for it. */
typedef struct
{
    s32 index;
    s16 x;
    s16 y;
} TrackPlacement;

#include "psyq_compat/inline_c.h"
#include "psyq_compat/gte_dmpsx_compat.h"

extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;
extern Struct_D800FDF58 D_800FDF58[];
extern Struct_D80105AE0 D_80105AE0[];
extern Struct_D80105768 D_80105768;
extern FieldActorState g_field_actor_slots[80];
extern FieldResourceEntry g_field_resource_entries[];

/**
 * @brief Field animation-frame audio/visual processor for the "non-negative
 *        item" resource variant, spawning per-frame billboard primitives and
 *        panning any attached sound cue relative to the camera.
 * @param rec Effect record.
 * @param cursor Vertex-buffer cursor pointer, threaded and returned.
 * @param base Ordering-table / primitive base array.
 * @param item Animation data blob for this frame.
 * @param flag Selects which of the actor's two audio channels to update.
 * @param part Part definition supplying flags and placement selectors.
 * @return Updated cursor pointer.
 * @note WIP - 97.36% (1557/1668 exact rows, gcc272_cdk). Frame now matches
 *       (-0xa8 both sides). Sibling of field11.c's func_80075C88 (called for the
 *       item < 0 case there, this one for item >= 0); same struct types, GTE
 *       pan-vector macros, and dual int-pointer/byte-pointer cursor idiom, but a
 *       simpler placement-opcode dispatch (no 0x7F0000-flags branch tree) and a
 *       wider 11-byte (0xB) per-frame record with 16-bit byte-pair deltas
 *       instead of func_80075C88's 8-bit signed deltas. Residue: spill-slot
 *       traffic at sp+0x068/0x078/0x0BC.
 * @note The @c gte_dmpsx_compat.h include is required for the tree build: it
 *       redefines the Psy-Q inline_c.h GTE macros to emit the real COP2 words
 *       GNU as accepts, matching the sibling field files. To diff, the target
 *       needs .include "gte_macros.inc" after its glabel (see idioms TOOL-11);
 *       working/func_80077FB4/target_test.s carries that.
 * @see decomp.me WIP
 */
s32 *func_80077FB4(Struct_D800FDF58 *rec, s32 *cursor, s32 *base, u8 *item, s32 flag, FieldActorPartDef *part)
{
    FieldMatrix *mtx = (FieldMatrix *) 0x1F800000;
    Vec2s *sxy = (Vec2s *) 0x1F800040;
    s32 sp5C;
    FieldVector *gte_out = (FieldVector *) 0x1F800044;
    FieldSVector *dir = (FieldSVector *) 0x1F800054;
    s16 *sp60 = (s16 *) 0x1F800064;
    Vec2s *sp64;
    Vec2s *sp68;
    FieldActorState *actor;
    s32 sp70;
    s32 sp74;
    s32 sp78;
    TrackPlacement sp28;
    struct { s32 sp30; u8 pad34[0x24]; } scratch;
    u8 *var_s1;
    FieldPolyFT4 *poly;
    s16 *temp_v0_13;
    s16 *var_a0_11;
    s16 *var_a1_3;
    s16 *var_a3_loop;
    s16 temp_v1_24;
    s32 var_s0;
    s32 *var_a2;
    s32 temp_a0_3;
    s32 temp_a1_2;
    s32 temp_s6;
    s32 temp_t0_call;
    s32 temp_t2;
    s32 temp_v0_5;
    s32 temp_v1_10;
    s32 temp_v1_11;
    s32 temp_v1_12;
    s32 temp_v1_13;
    s32 temp_v1_15;
    s32 temp_v1_17;
    s32 temp_v1_20;
    s32 temp_v1_23;
    s32 temp_v1_2;
    s32 var_a0;
    s32 var_a0_2;
    s32 var_a0_7;
    s32 var_a0_8;
    s32 var_a0_9;
    s32 var_a1_2;
    s32 var_t0;
    s32 var_v0_2;
    s32 var_v0_3;
    s32 var_v0_5;
    s32 var_v0_6;
    s32 var_v0_7;
    s32 var_v0_8;
    s32 var_v0_9;
    s32 var_v1;
    s32 var_a1;
    s32 var_v1_2;
    s32 var_v1_3;
    s32 var_v1_4;
    int new_var;
    s32 var_v1_5;
    u16 temp_a1;
    u16 temp_v0_7;
    u16 temp_v0_8;
    u16 temp_v1_21;
    u16 temp_v1_22;
    u16 var_a0_10;
    u16 var_v1_6;
    u16 var_v1_7;
    u32 temp_v1_16;
    u32 temp_v1_25;
    u32 temp_v1_3;
    u8 temp_a0;
    u8 temp_a2;
    s32 temp_s4;
    s32 temp_t1;
    u8 temp_v0;
    u8 temp_v0_2;
    u8 temp_v0_3;
    u8 temp_v0_4;
    u8 temp_v1;
    u8 temp_v1_18;
    u8 temp_v1_4;
    s32 temp_v1_5;
    s32 temp_v1_6;
    s32 temp_v1_7;
    s32 temp_v1_8;
    u8 temp_v1_9;
    u8 var_a0_3;
    u8 var_a0_4;
    u8 var_a0_5;
    u8 var_a0_6;
    s32 var_v0_4;
    Struct_D80105AE0 *slot;
    Struct_D80105AE0 *temp_v0_6;
    Struct_D800FDF58 *temp_v1_19;

    sp74 = 0;
    sp64 = (Vec2s *) 0x1F800080;
    sp68 = (Vec2s *) 0x1F800094;
    slot = &D_80105AE0[rec->unk3A];
    actor = &g_field_actor_slots[rec->unk22];
    *(s32 *) &slot->unk12C = 0;
    {
        s32 *zero_ptr = (s32 *) ((u8 *) slot + 0x1C);
        var_a1 = 7;
        do
        {
            *(s32 *) ((u8 *) zero_ptr + 0x148) = 0;
            var_a1 -= 1;
            zero_ptr -= 1;
        } while (var_a1 >= 0);
    }
    *(s32 *) &slot->unk144 = 0;
    *(s32 *) &slot->unk140 = 0;
    func_8007D078(rec, part, mtx, actor);
    gte_SetRotMatrix(mtx);

    var_v0_2 = D_800F22A0;
    if (var_v0_2 < 0)
    {
        var_v0_2 += 0xFF;
    }
    var_v1 = rec->unk0;
    var_a1_2 = var_v0_2 >> 8;
    if (var_v1 < 0)
    {
        var_v1 += 0xFF;
    }
    var_a0 = D_800F22A4;
    sxy->x = (u16) ((var_v0_2 >> 8) + ((var_v1 >> 8) + 0xA0));
    if (var_a0 < 0)
    {
        var_a0 += 0xFF;
    }
    var_v0_3 = rec->unk4;
    var_v1_2 = var_a0 >> 8;
    if (var_v0_3 < 0)
    {
        var_v0_3 += 0xFF;
    }
    var_a0_2 = rec->unk8;
    var_v0_5 = var_v0_3 >> 8;
    var_v0_5 += 0x70;
    var_a1_2 = var_v1_2 + var_v0_5;
    if (var_a0_2 < 0)
    {
        var_a0_2 += 0x1FF;
    }
    var_v1_3 = D_800F22A8;
    var_v0_5 = var_a0_2 >> 9;
    var_a0_2 = var_a1_2 - var_v0_5;
    if (var_v1_3 < 0)
    {
        var_v1_3 += 0x1FF;
    }
    var_v0_5 = var_v1_3 >> 9;
    temp_a1 = var_a0_2 - var_v0_5;
    sxy->y = temp_a1;
    temp_v1 = rec->unk37;
    temp_a0 = rec->unk38;
    if ((temp_v1 | temp_a0) != 0)
    {
        temp_v1_2 = (s8) temp_v1 + ((s32) (((s8) temp_a0 - (s8) temp_v1) * rec->unk34) / (s32) rec->unk35);
        sp70 = temp_v1_2;
        sxy->y = (u16) (temp_a1 - temp_v1_2);
    }
    else
    {
        sp70 = 0;
    }
    func_8007F7A0(rec, item, sp60);
    temp_v1_3 = part->unk34;
    if (temp_v1_3 & 0x100000)
    {
        func_8007F864(rec, sxy, sp60, mtx, (temp_v1_3 >> 0x14) & 1);
    }
    slot->unk140 = (u16) sp60[0];
    slot->unk142 = (u16) sp60[1];
    slot->unk144 = (u16) sp60[4];
    slot->unk146 = (u16) sp60[5];
    temp_t1 = *item;
    sp5C = (s32) temp_t1;
    item += 1;
    if (temp_t1 != 0)
    {
        var_s1 = item + 0x11;
        sp78 = 0xFFFFFF;
        poly = (FieldPolyFT4 *) cursor;
        do
        {
            temp_v1_4 = var_s1[-0xA];
            if (!(temp_v1_4 & 0x20))
            {
                func_8007D8D8(actor, rec, part, (s32 *) ((u8 *) cursor + 4));
                do
                {
                    do
                    {
                        ((u8 *) &poly->tag)[new_var = 3] = 9;
                        poly->code = 0x2CU;
                        var_v0_4 = rec->unk1C;
                        if (!(var_v0_4 & 0x800000))
                        {
                            var_v0_4 = 0x2C;
                        }
                        else
                        {
                            var_v0_4 = 0x2E;
                        }
                        poly->code = var_v0_4;
                    } while (0);
                } while (0);
                temp_s4 = var_s1[-0xD];
                temp_s6 = var_s1[-0xC] - 1;
                temp_t0_call = (s32) (s16) (var_s1[-0x10] + (var_s1[-0x7] << 8));
                var_s0 = (s16) (*item + (var_s1[-0x8] << 8));
                if (rec->unk21 & 0x80)
                {
                    var_s0 = -var_s0 - temp_s4;
                }
                temp_s4 -= 1;
                func_8007DB98(rec, sxy, cursor, temp_s4, temp_s6, (s32) var_s0, temp_t0_call, item, mtx);
                if ((var_s1[-0xA] ^ ((u8) rec->unk21 >> 1)) & 0x40)
                {
                    temp_v0 = var_s1[-0xF];
                    poly->u1 = temp_v0;
                    poly->u3 = temp_v0;
                    temp_v1_5 = poly->u1 + temp_s4;
                    var_a0_3 = 0xFF;
                    if (temp_v1_5 != 0x100)
                    {
                        var_a0_3 = temp_v1_5;
                    }
                    poly->u2 = var_a0_3;
                    poly->u0 = var_a0_3;
                }
                else
                {
                    temp_v0_2 = var_s1[-0xF];
                    poly->u0 = temp_v0_2;
                    poly->u2 = temp_v0_2;
                    temp_v1_6 = poly->u0 + temp_s4;
                    var_a0_4 = 0xFF;
                    if (temp_v1_6 != 0x100)
                    {
                        var_a0_4 = temp_v1_6;
                    }
                    poly->u3 = var_a0_4;
                    poly->u1 = var_a0_4;
                }
                if (var_s1[-0xA] & 0x80)
                {
                    temp_v0_3 = var_s1[-0xE];
                    poly->v2 = temp_v0_3;
                    poly->v3 = temp_v0_3;
                    temp_v1_7 = poly->v2 + temp_s6;
                    var_a0_5 = 0xFF;
                    if (temp_v1_7 != 0x100)
                    {
                        var_a0_5 = temp_v1_7;
                    }
                    poly->v1 = var_a0_5;
                    poly->v0 = var_a0_5;
                }
                else
                {
                    temp_v0_4 = var_s1[-0xE];
                    poly->v0 = temp_v0_4;
                    poly->v1 = temp_v0_4;
                    temp_v1_8 = poly->v0 + temp_s6;
                    var_a0_6 = 0xFF;
                    if (temp_v1_8 != 0x100)
                    {
                        var_a0_6 = temp_v1_8;
                    }
                    poly->v3 = var_a0_6;
                    poly->v2 = var_a0_6;
                }
                var_a1 = rec->unkC;
                temp_a0_3 = var_a1 << 7;
                if (var_a1 >= 2)
                {
                    temp_a0_3 = var_a1 << 6;
                    if (var_a1 >= 9)
                    {
                        {
                            s32 v0;
                            s32 v1;
                            s32 a0;
                            v0 = var_a1 - 9;
                            v0 <<= 6;
                            v1 = var_s1[-0xA];
                            v1 &= new_var;
                            v1 <<= 6;
                            v1 += 0x3C0;
                            v1 -= v0;
                            v1 &= 0x3FF;
                            a0 = part->unk4;
                            v1 = (s32) v1 >> 6;
                            a0 = (s32) ((u32) a0 >> 0x11);
                            a0 &= 0x60;
                            a0 |= 0x10;
                            a0 |= v1;
                            poly->tpage = (s16) a0;
                        }
                    }
                    else
                    {
                        var_v1_3 = var_s1[-0xA];
                        var_v1_3 &= new_var;
                        var_v1_3 <<= 6;
                        var_v1_3 += 0x340;
                        var_v1_3 -= temp_a0_3;
                        var_v1_3 = (var_v1_3 & 0x3FF) >> 6;
                        var_v0_5 = ((u32) part->unk4 >> 0x11) & 0x60;
                        goto block_48;
                    }
                }
                else
                {
                    temp_a0_3 = 0x380 - temp_a0_3;
                    var_v1_3 = ((u32) part->unk4 >> 0x11) & 0x60;
                    if (var_s1[-0xA] & new_var)
                    {
                        var_v0_6 = (temp_a0_3 + 0x40) & 0x3FF;
                    }
                    else
                    {
                        var_v0_6 = temp_a0_3 & 0x3FF;
                    }
                    var_v0_5 = var_v0_6 >> 6;
block_48:
                    poly->tpage = (s16) (var_v1_3 | var_v0_5);
                }
                if (((u32) part->unk28 >> 0xC) & 3)
                {
                    temp_v1_9 = part->unk2D;
                    if (temp_v1_9 >= 0x40U)
                    {
                        temp_t0_call = 0x1F2;
                        if ((u8) actor->unk228 < 2U)
                        {
                            temp_t0_call = (actor->unk228 * 2) + 0x1EE;
                        }
                    }
                    else
                    {
                        temp_t0_call = (temp_v1_9 >> 4) + 0x1EA;
                    }
                    temp_v1_10 = ((u32) part->unk28 >> 0xC) & 3;
                    if (temp_v1_10 != 1)
                    {
                        if (temp_v1_10 == 2)
                        {
                            if ((u8) actor->unk228 >= 3U)
                            {
                                goto block_57;
                            }
                            goto block_58;
                        }
                    }
                    else
                    {
block_57:
                        temp_v1_10 = temp_t0_call << 6;
                        temp_v0_5 = part->unk2D & 0xF;
                        temp_v1_10 |= temp_v0_5;
                        poly->clut = (s16) temp_v1_10;
                    }
                }
                else
                {
block_58:
                    if (var_s1[-0xB] == 0xB)
                    {
                        poly->code = (u8) (poly->code | 2);
                    }
                    temp_v0_5 = rec->unk3B;
                    temp_v1_10 = var_s1[-0xB];
                    temp_v0_5 += 0x1F4;
                    temp_v0_5 <<= 6;
                    temp_v1_10 &= 0x3F;
                    poly->clut = (s16) (temp_v0_5 | temp_v1_10);
                }
                temp_v1_11 = (s32) rec->unk8 >> 7;
                if (temp_v1_11 < 0)
                {
                    *cursor = (*cursor & 0xFF000000) | (base[0] & sp78);
                    temp_v1_12 = (s32) cursor & sp78;
                    poly += 1;
                    cursor = (s32 *) ((u8 *) cursor + 0x28);
                    base[0] = (s32) ((base[0] & 0xFF000000) | temp_v1_12);
                }
                else if (temp_v1_11 >= 0x1000)
                {
                    *cursor = (*cursor & 0xFF000000) | (base[0xFFF] & sp78);
                    temp_v1_13 = (s32) cursor & sp78;
                    poly += 1;
                    cursor = (s32 *) ((u8 *) cursor + 0x28);
                    base[0xFFF] = (s32) ((base[0xFFF] & 0xFF000000) | temp_v1_13);
                }
                else
                {
                    poly += 1;
                    *cursor = (*cursor & 0xFF000000) | (base[temp_v1_11] & sp78);
                    {
                        s32 *p = &base[(s32) rec->unk8 >> 7];
                        temp_a0_3 = (s32) cursor & sp78;
                        cursor = (s32 *) ((u8 *) cursor + 0x28);
                        *p = (*p & 0xFF000000) | temp_a0_3;
                    }
                }
                var_s1 += 0xB;
                item += 0xB;
            }
            else
            {
                temp_v1_15 = temp_v1_4 & 0xF;
                if ((flag == 0) || (temp_v1_15 == 2) || ((slot->unk178 & 1) && (temp_v1_15 == 0)))
                {
                    temp_v1_16 = var_s1[-0xA] & 0xF;
                    switch (temp_v1_16)
                    {
                    case 0:
                        func_8007FC74(rec, slot, item, 0, sxy, dir, gte_out);
                        break;
                    case 3:
                        func_8007FC74(rec, slot, item, 4, sxy, dir, gte_out);
                        break;
                    case 1:
                        if (part->unk24 & 0x100000)
                        {
                            if (rec->unk21 & 0x80)
                            {
                                dir->unk0 = (s16) (var_s1[-0xF] + (var_s1[-0xE] << 8));
                                dir->unk2 = 0;
                                dir->unk4 = (s16) -(*item + (var_s1[-0x10] << 8));
                            }
                            else
                            {
                                dir->unk0 = (s16) (var_s1[-0xF] + (var_s1[-0xE] << 8));
                                dir->unk2 = 0;
                                dir->unk4 = (s16) (*item + (var_s1[-0x10] << 8));
                            }
                            gte_ldv0(dir);
                            gte_rtv0();
                            gte_stlvnl(gte_out);
                            sp64[0].x = (s16) (sxy->x + gte_out->vx);
                            sp64[0].y = (s16) (sxy->y + gte_out->vy);
                            if (rec->unk21 & 0x80)
                            {
                                dir->unk0 = (s16) (var_s1[-0xB] + (var_s1[-0x9] << 8));
                                dir->unk2 = 0;
                                dir->unk4 = (s16) -(var_s1[-0xD] + (var_s1[-0xC] << 8));
                            }
                            else
                            {
                                dir->unk0 = (s16) (var_s1[-0xB] + (var_s1[-0x9] << 8));
                                dir->unk2 = 0;
                                dir->unk4 = (s16) (var_s1[-0xD] + (var_s1[-0xC] << 8));
                            }
                            gte_ldv0(dir);
                            gte_rtv0();
                            gte_stlvnl(gte_out);
                            sp64[1].x = (s16) (sxy->x + gte_out->vx);
                            sp64[1].y = (s16) (sxy->y + gte_out->vy);
                            if (rec->unk21 & 0x80)
                            {
                                dir->unk0 = (s16) (var_s1[-0x6] + (var_s1[-0x5] << 8));
                                dir->unk2 = 0;
                                dir->unk4 = (s16) -(var_s1[-0x8] + (var_s1[-0x7] << 8));
                            }
                            else
                            {
                                dir->unk0 = (s16) (var_s1[-0x6] + (var_s1[-0x5] << 8));
                                dir->unk2 = 0;
                                dir->unk4 = (s16) (var_s1[-0x8] + (var_s1[-0x7] << 8));
                            }
                            gte_ldv0(dir);
                            gte_rtv0();
                            gte_stlvnl(gte_out);
                            sp64[2].x = (s16) (sxy->x + gte_out->vx);
                            sp64[2].y = (s16) (sxy->y + gte_out->vy);
                            if (rec->unk21 & 0x80)
                            {
                                dir->unk0 = (s16) (var_s1[-0x2] + (var_s1[-0x1] << 8));
                                dir->unk2 = 0;
                                dir->unk4 = (s16) -(var_s1[-0x4] + (var_s1[-0x3] << 8));
                            }
                            else
                            {
                                dir->unk0 = (s16) (var_s1[-0x2] + (var_s1[-0x1] << 8));
                                dir->unk2 = 0;
                                dir->unk4 = (s16) (var_s1[-0x4] + (var_s1[-0x3] << 8));
                            }
                            gte_ldv0(dir);
                            gte_rtv0();
                            gte_stlvnl(gte_out);
                            sp64[3].x = (s16) (sxy->x + gte_out->vx);
                            sp64[3].y = (s16) (sxy->y + gte_out->vy);
                            if (!(slot->unk174 & 0x1800) && (((temp_v1_17 = slot->unk3C, (temp_v1_17 != 0xFFFF)) && (temp_v1_17 != 0)) || (actor->unkC->unk14 == new_var)))
                            {
                                var_s0 = func_80097150(sp64, rec, &sp28);
                                if (var_s0 == 1)
                                {
                                    {
                                        Struct_D80105AE0 *slots_base = D_80105AE0;
                                        temp_v0_6 = &slots_base[sp28.index];
                                    }
                                    temp_v0_6->unkC = (s32) (temp_v0_6->unkC & ~0x400);
                                    *((u8 *) slot + 0x180 + ((u8 *) &slot->unk178)[3]) = (u8) sp28.index;
                                    temp_v1_18 = ((u8 *) &slot->unk178)[3];
                                    if (temp_v1_18 < 9U)
                                    {
                                        ((u8 *) &slot->unk178)[3] = (u8) (temp_v1_18 + 1);
                                    }
                                    if (actor->unkC->unk14 == new_var)
                                    {
                                        actor->unk23A = (u8) (actor->unk23A | (var_s0 << actor->unk232));
                                        actor->unk229[actor->unk232] = (u8) sp28.index;
                                        {
                                            s32 a0;
                                            s32 a1;
                                            s32 v0;
                                            Struct_D800FDF58 *entry;

                                            a0 = D_800F22A0;
                                            if (a0 < 0)
                                            {
                                                a0 += 0xFF;
                                            }
                                            var_v1_3 = D_800FDF58[sp28.index].unk0;
                                            a1 = a0 >> 8;
                                            if (var_v1_3 < 0)
                                            {
                                                var_v1_3 += 0xFF;
                                            }
                                            a0 = D_800F22A4;
                                            v0 = var_v1_3 >> 8;
                                            v0 += 0xA0;
                                            sp68->x = (u16) (a1 + v0);
                                            if (a0 < 0)
                                            {
                                                a0 += 0xFF;
                                            }
                                            entry = &D_800FDF58[sp28.index];
                                            v0 = entry->unk4;
                                            a1 = a0 >> 8;
                                            if (v0 < 0)
                                            {
                                                v0 += 0xFF;
                                            }
                                            a0 = entry->unk8;
                                            v0 >>= 8;
                                            v0 += 0x70;
                                            a1 += v0;
                                            if (a0 < 0)
                                            {
                                                a0 += 0x1FF;
                                            }
                                            var_v1_3 = D_800F22A8;
                                            v0 = a0 >> 9;
                                            a0 = a1 - v0;
                                            if (var_v1_3 < 0)
                                            {
                                                var_v1_3 += 0x1FF;
                                            }
                                            v0 = var_v1_3 >> 9;
                                            sp68->y = (u16) (a0 - v0);
                                        }
                                        if (D_800FDF58[sp28.index].unk21 & 0x80)
                                        {
                                            var_a0_10 = sp28.x;
                                            var_v1_6 = (u16) sp68->x;
                                            var_v0_8 = actor->unk232;
                                        }
                                        else
                                        {
                                            var_v1_6 = sp28.x;
                                            var_a0_10 = (u16) sp68->x;
                                            var_v0_8 = actor->unk232;
                                        }
                                        actor->unk1FE[var_v0_8].x = (s16) (var_v1_6 - var_a0_10);
                                        actor->unk1FE[actor->unk232].y = (s16) (sp28.y - sp68->y);
                                        actor->unk232 = (u8) (actor->unk232 + 1);
                                        func_8008A840(actor->unk228, sp28.index);
                                        do
                                        {
                                            do
                                            {
                                                temp_v0_7 = sp28.x - sxy->x;
                                                slot->unk13C = temp_v0_7;
                                                slot->unk138 = temp_v0_7;
                                                slot->unk134 = temp_v0_7;
                                                slot->unk130 = temp_v0_7;
                                                temp_v0_8 = sxy->y;
                                                temp_v0_8 = sp28.y - temp_v0_8;
                                                slot->unk174 = (s32) ((slot->unk174 & ~0x1800) | 0x1000);
                                                slot->unk13E = temp_v0_8;
                                                slot->unk13A = temp_v0_8;
                                                slot->unk136 = temp_v0_8;
                                                slot->unk132 = temp_v0_8;
                                            } while (0);
                                        } while (0);
                                    }
                                    else
                                    {
                                        if ((func_8008A840(rec->unk3A, sp28.index) == var_s0) && ((temp_v1_20 = slot->unk3C, (temp_v1_20 == var_s0)) || (temp_v1_20 == 3) || (temp_v1_20 == 0x10)))
                                        {
                                            slot->unk3C = 0x1E;
                                        }
                                        var_a1_2 = 1;
                                        if (slot->unk3C & 0x8000)
                                        {
                                            scratch.sp30 = sp28.index;
                                            var_a2 = &scratch.sp30;
block_176:
                                            if (func_8009104C(rec->unk3A, var_a1_2, var_a2, slot->unk3C) != 0)
                                            {
                                                slot->unk174 = (s32) ((slot->unk174 & ~0x1800) | 0x1000);
                                            }
                                        }
                                        else
                                        {
                                            var_s0 = func_800839F8(rec->unk3A, 0);
                                            if (var_s0 != -1)
                                            {
                                                if (func_80083EEC(rec->unk3A, var_s0, slot->unk3C) != 0)
                                                {
                                                    ((u8 *) &slot->unk178)[1] = var_s0;
                                                    scratch.sp30 = sp28.index;
                                                    field_start_actor_animation(var_s0, 1, &scratch.sp30);
                                                    temp_v1_21 = sp28.x - sxy->x;
                                                    slot->unk13C = temp_v1_21;
                                                    slot->unk138 = temp_v1_21;
                                                    slot->unk134 = temp_v1_21;
                                                    slot->unk130 = temp_v1_21;
                                                    var_v1_7 = sp28.y - sxy->y;
                                                    goto block_126;
                                                }
                                            }
                                        }
                                        goto block_182;
                                    }
                                }
                                else
                                {
                                    if (var_s0 == 2)
                                    {
                                        slot->unk3C = 0x1E;
                                        var_s0 = func_800839F8(rec->unk3A, 0);
                                        if (var_s0 != -1)
                                        {
                                            if (func_80083EEC(rec->unk3A, var_s0, slot->unk3C) != 0)
                                            {
                                                ((u8 *) &slot->unk178)[1] = var_s0;
                                                scratch.sp30 = sp28.index;
                                                field_start_actor_animation(var_s0, 1, &scratch.sp30);
                                                temp_v1_22 = sp28.x - sxy->x;
                                                slot->unk13C = temp_v1_22;
                                                slot->unk138 = temp_v1_22;
                                                slot->unk134 = temp_v1_22;
                                                slot->unk130 = temp_v1_22;
                                                var_v1_7 = sp28.y - sxy->y;
block_126:
                                                slot->unk13E = var_v1_7;
                                                slot->unk13A = var_v1_7;
                                                slot->unk136 = var_v1_7;
                                                slot->unk132 = var_v1_7;
                                            }
                                        }
block_182:
                                        slot->unk3C = 0xFFFF;
                                        goto block_183;
                                    }
                                    if (var_s0 == new_var)
                                    {
                                        slot->unk3C = 0xFFFF;
block_183:
                                        slot->unk174 = (s32) ((slot->unk174 & ~0x1800) | 0x1000);
                                    }
                                }
                            }
                        }
                        break;
                    case 6:
                        temp_v1_23 = slot->unk3C;
                        if (!(temp_v1_23 & 0x8000) && (temp_v1_23 != 0xFFFF) && (((temp_v1_24 = rec->unk2A, (temp_v1_24 == 0x91)) || (temp_v1_24 == 0x85) || (temp_v1_24 == 0x86) || (temp_v1_24 == 0x98))) && !(rec->unk3C & 0x01000000))
                        {
                            var_s0 = func_800839F8(rec->unk3A, 0);
                            if ((var_s0 != -1) && (func_80083EEC(rec->unk3A, var_s0, slot->unk3C) != 0))
                            {
                                ((u8 *) &slot->unk178)[1] = var_s0;
                                field_start_actor_animation(var_s0, 0, NULL);
                                if (rec->unk21 & 0x80)
                                {
                                    dir->unk0 = (s16) (var_s1[-0xF] + (var_s1[-0xE] << 8));
                                    dir->unk2 = 0;
                                    dir->unk4 = (s16) -(*item + (var_s1[-0x10] << 8));
                                }
                                else
                                {
                                    dir->unk0 = (s16) (var_s1[-0xF] + (var_s1[-0xE] << 8));
                                    dir->unk2 = 0;
                                    dir->unk4 = (s16) (*item + (var_s1[-0x10] << 8));
                                }
                                gte_ldv0(dir);
                                gte_rtv0();
                                gte_stlvnl(gte_out);
                                slot->unk130 = (u16) gte_out->vx;
                                slot->unk132 = (u16) gte_out->vy;
                                if (rec->unk21 & 0x80)
                                {
                                    dir->unk0 = (s16) (var_s1[-0xB] + (var_s1[-0x9] << 8));
                                    dir->unk2 = 0;
                                    dir->unk4 = (s16) -(var_s1[-0xD] + (var_s1[-0xC] << 8));
                                }
                                else
                                {
                                    dir->unk0 = (s16) (var_s1[-0xB] + (var_s1[-0x9] << 8));
                                    dir->unk2 = 0;
                                    dir->unk4 = (s16) (var_s1[-0xD] + (var_s1[-0xC] << 8));
                                }
                                gte_ldv0(dir);
                                gte_rtv0();
                                gte_stlvnl(gte_out);
                                slot->unk134 = (u16) gte_out->vx;
                                slot->unk136 = (u16) gte_out->vy;
                                if (rec->unk21 & 0x80)
                                {
                                    dir->unk0 = (s16) (var_s1[-0x6] + (var_s1[-0x5] << 8));
                                    dir->unk2 = 0;
                                    dir->unk4 = (s16) -(var_s1[-0x8] + (var_s1[-0x7] << 8));
                                }
                                else
                                {
                                    dir->unk0 = (s16) (var_s1[-0x6] + (var_s1[-0x5] << 8));
                                    dir->unk2 = 0;
                                    dir->unk4 = (s16) (var_s1[-0x8] + (var_s1[-0x7] << 8));
                                }
                                gte_ldv0(dir);
                                gte_rtv0();
                                gte_stlvnl(gte_out);
                                slot->unk138 = (u16) gte_out->vx;
                                slot->unk13A = (u16) gte_out->vy;
                                if (rec->unk21 & 0x80)
                                {
                                    dir->unk0 = (s16) (var_s1[-0x2] + (var_s1[-0x1] << 8));
                                    dir->unk2 = 0;
                                    dir->unk4 = (s16) -(var_s1[-0x4] + (var_s1[-0x3] << 8));
                                }
                                else
                                {
                                    dir->unk0 = (s16) (var_s1[-0x2] + (var_s1[-0x1] << 8));
                                    dir->unk2 = 0;
                                    dir->unk4 = (s16) (var_s1[-0x4] + (var_s1[-0x3] << 8));
                                }
                                gte_ldv0(dir);
                                gte_rtv0();
                                gte_stlvnl(gte_out);
                                slot->unk13C = (u16) gte_out->vx;
                                slot->unk13E = (u16) gte_out->vy;
                            }
                            goto block_183;
                        }
                        break;
                    case 5:
                        if ((rec->unk34 == 0) && !(rec->unk3C & 0x01000000))
                        {
                            FieldResourceEntry *resources = g_field_resource_entries;
                            u8 idx = rec->unk3B;
                            temp_v1_25 = (u16) resources[idx].unkA >> 0xC;
                            if (temp_v1_25 != 1)
                            {
                                if ((s32) temp_v1_25 < 2)
                                {
                                    if (temp_v1_25 == 0)
                                    {
                                        func_800A3938(resources[rec->unk3B].unkA & 0xFFF, func_8006CE70(rec->unk3A, rec));
                                    }
                                }
                            }
                            else
                            {
                                func_800A39A8(resources[rec->unk3B].unkA & 0xFFF, func_8006CE70(rec->unk3A, rec), rec->unk3B - new_var, rec->unk3A);
                            }
                        }
                        break;
                    case 2:
                        if (part->unk24 & 0x100000)
                        {
                            func_8007E5FC(sp60, rec->unk21 & 0x80, item);
                            sp74 += 1;
                        }
                        break;
                    case 4:
                        if ((slot->unk3C != 0xFFFF) && !(slot->unk174 & 0x1800))
                        {
                            if (rec->unk21 & 0x80)
                            {
                                dir->unk0 = (s16) (var_s1[-0xF] + (var_s1[-0xE] << 8));
                                dir->unk2 = 0;
                                dir->unk4 = (s16) -(*item + (var_s1[-0x10] << 8));
                            }
                            else
                            {
                                dir->unk0 = (s16) (var_s1[-0xF] + (var_s1[-0xE] << 8));
                                dir->unk2 = 0;
                                dir->unk4 = (s16) (*item + (var_s1[-0x10] << 8));
                            }
                            gte_ldv0(dir);
                            gte_rtv0();
                            gte_stlvnl(gte_out);
                            slot->unk130 = (u16) gte_out->vx;
                            slot->unk132 = (u16) (gte_out->vy - sp70);
                            if (rec->unk21 & 0x80)
                            {
                                dir->unk0 = (s16) (var_s1[-0xB] + (var_s1[-0x9] << 8));
                                dir->unk2 = 0;
                                dir->unk4 = (s16) -(var_s1[-0xD] + (var_s1[-0xC] << 8));
                            }
                            else
                            {
                                dir->unk0 = (s16) (var_s1[-0xB] + (var_s1[-0x9] << 8));
                                dir->unk2 = 0;
                                dir->unk4 = (s16) (var_s1[-0xD] + (var_s1[-0xC] << 8));
                            }
                            gte_ldv0(dir);
                            gte_rtv0();
                            gte_stlvnl(gte_out);
                            slot->unk134 = (u16) gte_out->vx;
                            slot->unk136 = (u16) (gte_out->vy - sp70);
                            if (rec->unk21 & 0x80)
                            {
                                dir->unk0 = (s16) (var_s1[-0x6] + (var_s1[-0x5] << 8));
                                dir->unk2 = 0;
                                dir->unk4 = (s16) -(var_s1[-0x8] + (var_s1[-0x7] << 8));
                            }
                            else
                            {
                                dir->unk0 = (s16) (var_s1[-0x6] + (var_s1[-0x5] << 8));
                                dir->unk2 = 0;
                                dir->unk4 = (s16) (var_s1[-0x8] + (var_s1[-0x7] << 8));
                            }
                            gte_ldv0(dir);
                            gte_rtv0();
                            gte_stlvnl(gte_out);
                            slot->unk138 = (u16) gte_out->vx;
                            slot->unk13A = (u16) (gte_out->vy - sp70);
                            if (rec->unk21 & 0x80)
                            {
                                dir->unk0 = (s16) (var_s1[-0x2] + (var_s1[-0x1] << 8));
                                dir->unk2 = 0;
                                dir->unk4 = (s16) -(var_s1[-0x4] + (var_s1[-0x3] << 8));
                            }
                            else
                            {
                                dir->unk0 = (s16) (var_s1[-0x2] + (var_s1[-0x1] << 8));
                                dir->unk2 = 0;
                                dir->unk4 = (s16) (var_s1[-0x4] + (var_s1[-0x3] << 8));
                            }
                            gte_ldv0(dir);
                            gte_rtv0();
                            gte_stlvnl(gte_out);
                            slot->unk13C = (u16) gte_out->vx;
                            slot->unk13E = (u16) (gte_out->vy - sp70);
                            if (slot->unk3C & 0x8000)
                            {
                                if (func_8009104C(rec->unk3A, 0, NULL, slot->unk3C) != 0)
                                {
                                    slot->unk174 = (s32) ((slot->unk174 & ~0x1800) | 0x1000);
                                }
                            }
                            else
                            {
                                var_s0 = func_800839F8(rec->unk3A, 0);
                                if (var_s0 != -1)
                                {
                                    if (func_80083EEC(rec->unk3A, var_s0, slot->unk3C) != 0)
                                    {
                                        ((u8 *) &slot->unk178)[1] = var_s0;
                                        field_start_actor_animation(var_s0, 0, NULL);
                                    }
                                }
                            }
                            slot->unk3C = 0xFFFF;
                            slot->unk174 = (s32) ((slot->unk174 & ~0x1800) | 0x1000);
                        }
                        break;
                    }
                }
                var_s1 += 0x11;
                item += 0x11;
            }
            sp5C -= 1;
        } while (sp5C != 0);
    }
    if (sp74 != 0)
    {
        Struct_D80105768 *scale;
        s32 width_tmp;
        var_a1_3 = sp60;
        temp_v0_13 = var_a1_3 + 8;
        if (var_a1_3 != temp_v0_13)
        {
            scale = &D_80105768;
            var_a3_loop = temp_v0_13;
            var_a0_11 = var_a1_3 + 1;
            do
            {
                *var_a1_3 = (s16) ((s32) (((s32) (*var_a1_3 * part->unk2E) >> 6) * scale->unk0) >> 0xC);
                var_a1_3 += 2;
                *var_a0_11 = (s16) ((s32) (((s32) (*var_a0_11 * part->unk33) >> 6) * scale->unk4) >> 0xC);
                var_a0_11 += 2;
            } while (var_a1_3 != var_a3_loop);
        }
        width_tmp = sp60[0];
        var_v0_9 = sp60[2] - width_tmp;
        if (var_v0_9 < 0)
        {
            var_v0_9 = -var_v0_9;
        }
        slot->unk12E = (s16) ((var_v0_9 * 7) / 10);
        slot->unk12C = (s16) ((s32) (sp60[2] + sp60[0]) >> 1);
        cursor = func_800871A0(rec, cursor, base, sp60);
    }
    return cursor;
}
