/**
 * @file field11.c
 * @brief Field animation-frame audio/visual processor, carved from the top
 *        of the unk2 segment (the single-function slot right after
 *        field10.c's func_80074D7C).
 */

#include "common.h"

typedef struct
{
    s16 x;
    s16 y;
} Vec2s;

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
 * already used by field7.c/field8.c/field10.c for the same real struct. */
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
    u8 pad148[0x170 - 0x148];
    u8 unk170;  /* 0x170 */
    u8 pad171[0x174 - 0x171];
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
    u8 pad0[0x258];
    u8 unk258;
    u8 pad259[0x268 - 0x259];
} Struct_D800FD818;

typedef struct
{
    s32 unk0;
    u8 pad4[0xC - 4];
    s32 unkC;
    u8 pad10[0x1C - 0x10];
} Struct_D80105880;

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

#include "psyq/inline_c.h"

/*
 * inline_c.h's "Type 2" (no-operand) GTE macros, like gte_rtv0() below, emit
 * a raw `.word` value meant for Sony's original PSY-Q assembler to translate
 * into the real COP2 instruction word. This project assembles with plain GNU
 * `as` (via the maspsx syntax wrapper), which has no such translation step,
 * so override the one Type 2 op actually used here with the real ROM bytes,
 * as a `cop2` immediate, matching field7.c/field8.c.
 */
#define gte_rtv0() __asm__ volatile("nop;nop;cop2 0x0486012")

extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;
extern Struct_D800FD818 D_800FD818[];
extern Struct_D800FDF58 D_800FDF58[];
extern Struct_D80105AE0 D_80105AE0[];
extern Struct_D80105880 D_80105880[];
extern Struct_D80105768 D_80105768;
extern FieldActorState g_field_actor_slots[80];
extern FieldResourceEntry g_field_resource_entries[];

/**
 * @brief Field animation-frame audio/visual processor for the "negative
 *        item" resource variant, spawning per-frame billboard primitives and
 *        panning any attached sound cue relative to the camera.
 * @param rec Effect record.
 * @param cursor Vertex-buffer cursor pointer, threaded and returned.
 * @param base Ordering-table / primitive base array.
 * @param item Animation data blob for this frame.
 * @param flag Selects which of the actor's two audio channels to update.
 * @param part Part definition supplying flags and placement selectors.
 * @return Updated cursor pointer.
 * @note WIP - 77.57% (1020/2192 exact rows) at time of writing. This is one
 *       of the largest functions in the overlay (2251 target insns, 3 loops,
 *       35 calls, 4 raw GTE rtv0 blocks); see field10.c's func_800754B4 for
 *       the closest sibling shape (same GTE pan-vector macros, same dual
 *       int-pointer and byte-pointer cursor idiom). Frame is 0x18 bytes
 *       short of the target's (-0x90 vs -0xa8): the target keeps sxy/gte_out/
 *       dir/sp64/sp68/sp6C's scratchpad addresses in reloaded stack homes
 *       (heavy traffic at sp+0x58/0x60/0x64/0x68/0x6C) while this draft's
 *       register allocator keeps them live in registers instead, and shows
 *       extra spill traffic at sp+0x34-0x54 the target does not have.
 *       Advisory mechanisms: CSE-FOLD (target re-reads D_800F22A8,
 *       D_800FDF58, g_field_actor_slots where this draft reuses an
 *       already-computed value), FRAME, EXPAND-SHAPE. Also note: the four
 *       "irregular switch" placement-opcode dispatches (temp_v0_6/7/8/9 in
 *       the 0x7F0000-flags block) were rewritten from m2c's invalid
 *       case-label-outside-switch output into equivalent if/else chains;
 *       their exact statement shape is unverified against the target and is
 *       a likely source of remaining residue.
 * @see decomp.me WIP
 */
s32 *func_80075C88(Struct_D800FDF58 *rec, s32 *cursor, s32 *base, u8 *item, s32 flag, FieldActorPartDef *part)
{
    FieldMatrix *mtx = (FieldMatrix *) 0x1F800000;
    Vec2s *sxy = (Vec2s *) 0x1F800040;
    FieldVector *gte_out = (FieldVector *) 0x1F800044;
    FieldSVector *dir = (FieldSVector *) 0x1F800054;
    s16 *sp64 = (s16 *) 0x1F800064;
    Vec2s *sp68 = (Vec2s *) 0x1F800080;
    Vec2s *sp6C = (Vec2s *) 0x1F800094;
    s32 sp7C;
    s32 sp78;
    s32 sp74;
    s32 sp5C;
    s32 sp30;
    TrackPlacement sp28;
    u8 *var_s5;
    u8 *var_s6;
    s32 var_a3;
    s16 temp_v0_24;
    s16 var_a0_12;
    s16 var_a1;
    u8 temp_a0_4;
    s16 var_v0_10;
    s16 var_v0_27;
    s16 var_v0_28;
    s32 *temp_v1_16;
    s32 temp_a0_2;
    s32 temp_a1_2;
    s32 temp_a1_3;
    s32 temp_a3_2;
    s32 temp_s7;
    s32 temp_t1_2;
    s32 temp_v0_10;
    s32 temp_v0_12;
    s32 temp_v0_18;
    s32 temp_v0_6;
    s32 temp_v0_7;
    s32 temp_v0_8;
    s32 temp_v0_9;
    s32 temp_v1_13;
    s32 temp_v1_14;
    s32 temp_v1_15;
    s32 temp_v1_17;
    s32 temp_v1_19;
    s32 temp_v1_20;
    s32 temp_v1_23;
    s32 temp_v1_26;
    s32 temp_v1_27;
    s32 temp_v1_28;
    s32 temp_v1_2;
    s32 temp_v1_34;
    s32 var_a0;
    s32 var_a0_2;
    s32 var_a0_7;
    s32 var_a0_8;
    s32 var_a0_9;
    s32 var_s1;
    s32 var_s1_2;
    s32 var_s1_3;
    s32 var_s1_4;
    s32 var_s1_5;
    s32 var_s2;
    s32 var_v0_15;
    s32 var_v0_20;
    s32 var_v0_21;
    s32 var_v0_22;
    s32 var_v0_2;
    s32 var_v0_3;
    s32 var_v0_5;
    s32 var_v0_6;
    s32 var_v0_7;
    s32 var_v0_8;
    s32 var_v1;
    s32 var_v1_10;
    s32 var_v1_11;
    s32 var_v1_2;
    s32 var_v1_3;
    s32 var_v1_4;
    s32 var_v1_5;
    s32 var_v1_6;
    s32 var_v1_7;
    s32 var_v1_8;
    s8 var_s0;
    s8 var_v0_11;
    s8 var_v0_12;
    s8 var_v0_13;
    s8 var_v0_14;
    s8 var_v0_16;
    s8 var_v0_17;
    s8 var_v0_18;
    s8 var_v0_19;
    s8 var_v0_23;
    s8 var_v0_24;
    s8 var_v0_25;
    s8 var_v0_26;
    u16 temp_a1;
    u16 temp_a3;
    u16 temp_v0_15;
    u16 temp_v0_16;
    u16 temp_v1_29;
    u16 temp_v1_30;
    u16 temp_v1_31;
    u16 temp_v1_32;
    u16 var_a0_10;
    u16 var_v1_9;
    u32 temp_a0_3;
    u32 temp_v1_18;
    u32 temp_v1_33;
    u32 temp_v1_3;
    u32 var_v0_9;
    u8 temp_a0;
    u8 temp_a2;
    u8 temp_s2;
    u8 temp_t1;
    u8 temp_v0_11;
    u8 temp_v0_21;
    u8 temp_v0_22;
    u8 temp_v0_23;
    u8 temp_v0_2;
    u8 temp_v0_3;
    u8 temp_v0_4;
    u8 temp_v0_5;
    u8 temp_v1;
    u8 temp_v1_10;
    u8 temp_v1_11;
    u8 temp_v1_12;
    u8 temp_v1_21;
    u8 temp_v1_4;
    u8 temp_v1_5;
    u8 temp_v1_6;
    u8 temp_v1_7;
    u8 temp_v1_8;
    u8 temp_v1_9;
    u8 var_a0_11;
    u8 var_a0_3;
    u8 var_a0_4;
    u8 var_a0_5;
    u8 var_a0_6;
    u8 var_v0_4;
    FieldActorState *actor;
    Struct_D80105AE0 *slot;
    Struct_D80105AE0 *temp_v0_13;
    Struct_D80105AE0 *temp_v0_14;
    Struct_D80105AE0 *temp_v0_17;
    Struct_D80105AE0 *temp_v0_20;
    Struct_D800FDF58 *temp_v0_19;
    Struct_D800FDF58 *temp_v1_22;
    Struct_D80105AE0 *temp_v1_24;
    FieldActorState *temp_v1_25;

    sp78 = 0;
    slot = &D_80105AE0[rec->unk3A];
    *(s32 *) &slot->unk12C = 0;
    if (flag == 0)
    {
        s32 *zero_ptr = (s32 *) ((u8 *) slot + 0x1C + 0x148);
        var_v1 = 7;
        do
        {
            *zero_ptr = 0;
            var_v1 -= 1;
            zero_ptr -= 1;
        } while (var_v1 >= 0);
        *(s32 *) &slot->unk144 = 0;
        *(s32 *) &slot->unk140 = 0;
    }
    actor = &g_field_actor_slots[rec->unk22];
    func_8007D078(rec, part, mtx, actor);
    gte_SetRotMatrix(mtx);

    var_v0_2 = D_800F22A0;
    if (var_v0_2 < 0)
    {
        var_v0_2 += 0xFF;
    }
    var_v1_2 = rec->unk0;
    if (var_v1_2 < 0)
    {
        var_v1_2 += 0xFF;
    }
    var_a0 = D_800F22A4;
    sxy->x = (u16) ((var_v0_2 >> 8) + ((var_v1_2 >> 8) + 0xA0));
    if (var_a0 < 0)
    {
        var_a0 += 0xFF;
    }
    var_v0_3 = rec->unk4;
    if (var_v0_3 < 0)
    {
        var_v0_3 += 0xFF;
    }
    var_a0_2 = rec->unk8;
    if (var_a0_2 < 0)
    {
        var_a0_2 += 0x1FF;
    }
    var_v1_3 = D_800F22A8;
    if (var_v1_3 < 0)
    {
        var_v1_3 += 0x1FF;
    }
    temp_a1 = (((var_a0 >> 8) + ((var_v0_3 >> 8) + 0x70)) - (var_a0_2 >> 9)) - (var_v1_3 >> 9);
    sxy->y = temp_a1;
    temp_v1 = rec->unk37;
    temp_a0 = rec->unk38;
    if ((temp_v1 | temp_a0) != 0)
    {
        temp_v1_2 = (s8) temp_v1 + ((s32) (((s8) temp_a0 - (s8) temp_v1) * rec->unk34) / (s32) rec->unk35);
        sp74 = temp_v1_2;
        sxy->y = (u16) (temp_a1 - temp_v1_2);
    }
    else
    {
        sp74 = 0;
    }
    func_8007F5D8(rec, item, sp64);
    temp_v1_3 = part->unk34;
    if (temp_v1_3 & 0x100000)
    {
        func_8007F864(rec, sxy, sp64, mtx, (temp_v1_3 >> 0x14) & 1);
    }
    slot->unk140 = (u16) sp64[0];
    slot->unk142 = (u16) sp64[1];
    slot->unk144 = (u16) sp64[4];
    slot->unk146 = (u16) sp64[5];
    temp_t1 = *item;
    item += 1;
    sp5C = (s32) temp_t1;
    if (temp_t1 != 0)
    {
        var_s5 = item + 6;
        sp7C = 0xFFFFFF;
        var_s6 = (u8 *) (cursor + 0xE);
        do
        {
            temp_v1_4 = var_s5[1];
            if (!(temp_v1_4 & 0x20))
            {
                if ((u8) rec->unk3A < 3U)
                {
                    if (((temp_v1_4 & 3) == 2) && (var_s5[0] == 2))
                    {
                        if (D_800FD818[rec->unk3A].unk258 == 0)
                        {
                            goto block_28;
                        }
                    }
                    else
                    {
                        goto block_29;
                    }
                }
                else
                {
block_28:
block_29:
                    func_8007D8D8(actor, rec, part, cursor + 4);
                    var_s6[-0xB] = 9;
                    var_s6[-7] = 0x2CU;
                    var_v0_4 = 0x2E;
                    if (!(rec->unk1C & 0x800000))
                    {
                        var_v0_4 = 0x2C;
                    }
                    var_s6[-7] = var_v0_4;
                    temp_s2 = var_s5[-2];
                    temp_s7 = var_s5[-1] - 1;
                    if (!(rec->unk21 & 0x80))
                    {
                        var_s0 = (s8) *item;
                        var_s1 = temp_s2 - 1;
                    }
                    else
                    {
                        var_s0 = -(s8) *item - temp_s2;
                        var_s1 = temp_s2 - 1;
                    }
                    func_8007DB98(rec, sxy, cursor, var_s1, temp_s7, (s32) var_s0, (s32) var_s5[-5], item, mtx);
                    if ((var_s5[1] ^ ((u8) rec->unk21 >> 1)) & 0x40)
                    {
                        temp_v0_2 = var_s5[-4];
                        var_s6[6] = temp_v0_2;
                        var_s6[0x16] = temp_v0_2;
                        temp_v1_5 = var_s6[6] + var_s1;
                        var_a0_3 = 0xFF;
                        if (temp_v1_5 != 0x100)
                        {
                            var_a0_3 = temp_v1_5;
                        }
                        var_s6[0xE] = var_a0_3;
                        var_s6[-2] = var_a0_3;
                    }
                    else
                    {
                        temp_v0_3 = var_s5[-4];
                        var_s6[-2] = temp_v0_3;
                        var_s6[0xE] = temp_v0_3;
                        temp_v1_6 = var_s6[-2] + var_s1;
                        var_a0_4 = 0xFF;
                        if (temp_v1_6 != 0x100)
                        {
                            var_a0_4 = temp_v1_6;
                        }
                        var_s6[0x16] = var_a0_4;
                        var_s6[6] = var_a0_4;
                    }
                    if (var_s5[1] & 0x80)
                    {
                        temp_v0_4 = var_s5[-3];
                        var_s6[0xF] = temp_v0_4;
                        var_s6[0x17] = temp_v0_4;
                        temp_v1_7 = var_s6[0xF] + temp_s7;
                        var_a0_5 = 0xFF;
                        if (temp_v1_7 != 0x100)
                        {
                            var_a0_5 = temp_v1_7;
                        }
                        var_s6[7] = var_a0_5;
                        var_s6[-1] = var_a0_5;
                    }
                    else
                    {
                        temp_v0_5 = var_s5[-3];
                        var_s6[-1] = temp_v0_5;
                        var_s6[7] = temp_v0_5;
                        temp_v1_8 = var_s6[-1] + temp_s7;
                        var_a0_6 = 0xFF;
                        if (temp_v1_8 != 0x100)
                        {
                            var_a0_6 = temp_v1_8;
                        }
                        var_s6[0x17] = var_a0_6;
                        var_s6[0xF] = var_a0_6;
                    }
                    if ((var_s5[1] & 3) == 2)
                    {
                        var_s6[-1] = (u8) (var_s6[-1] | 0x80);
                        var_s6[7] = (u8) (var_s6[7] | 0x80);
                        var_s6[0xF] = (u8) (var_s6[0xF] | 0x80);
                        var_s6[0x17] = (u8) (var_s6[0x17] | 0x80);
                    }
                    temp_a1_2 = rec->unkC;
                    if (temp_a1_2 >= 2)
                    {
                        if (temp_a1_2 >= 9)
                        {
                            *(s16 *) (var_s6 + 8) = (s16) ((((u32) part->unk4 >> 0x11) & 0x60) | 0x10 | ((s32) (((((var_s5[1] & 3) << 6) + 0x3C0) - ((temp_a1_2 - 9) << 6)) & 0x3FF) >> 6));
                        }
                        else
                        {
                            var_v1_4 = (s32) (((((var_s5[1] & 3) << 6) + 0x340) - (temp_a1_2 << 6)) & 0x3FF) >> 6;
                            var_v0_5 = ((u32) part->unk4 >> 0x11) & 0x60;
                            goto block_58;
                        }
                    }
                    else
                    {
                        temp_a0_2 = 0x380 - (temp_a1_2 << 7);
                        var_v1_4 = ((u32) part->unk4 >> 0x11) & 0x60;
                        if (var_s5[1] & 3)
                        {
                            var_v0_6 = (temp_a0_2 + 0x40) & 0x3FF;
                        }
                        else
                        {
                            var_v0_6 = temp_a0_2 & 0x3FF;
                        }
                        var_v0_5 = var_v0_6 >> 6;
block_58:
                        *(s16 *) (var_s6 + 8) = (s16) (var_v1_4 | var_v0_5);
                    }
                    temp_a0_3 = rec->unk1C;
                    if ((temp_a0_3 & 0x7F0000) && (var_s5[0] == 0) && ((var_s5[1] & 3) != 2))
                    {
                        if (temp_a0_3 & 0x40000)
                        {
                            if (((u32) part->unk28 >> 0xC) & 3)
                            {
                                temp_v1_9 = part->unk2D;
                                if (temp_v1_9 >= 0x40U)
                                {
                                    var_v0_7 = 0x1F2;
                                }
                                else
                                {
                                    var_v0_7 = (temp_v1_9 >> 4) + 0x1EA;
                                }
                                temp_v0_6 = ((u32) part->unk28 >> 0xC) & 3;
                                if (temp_v0_6 == 1)
                                { /* switch 1 case 1 */
                                    var_v1_5 = var_v0_7 << 6;
                                    var_v0_7 = part->unk2D & 0xF;
                                    goto block_116;
                                }
                                if (temp_v0_6 == 2)
                                { /* switch 1 case 2 */
                                    *(s16 *) (var_s6 + 0) = (s16) (((rec->unk3B + 0x1F4) << 6) | 9);
                                }
                            }
                            else
                            { /* switch 1 case 0 (shares case 2's body) */
                                *(s16 *) (var_s6 + 0) = (s16) (((rec->unk3B + 0x1F4) << 6) | 9);
                            }
                        }
                        else if (temp_a0_3 & 0x780000)
                        {
                            if (!(((u32) part->unk28 >> 0xC) & 3))
                            {
                                var_v0_7 = (temp_a0_3 >> 0x13) & 0xF;
                                var_v1_5 = (rec->unk3B + 0x1F4) << 6;
                                goto block_116;
                            }
                            temp_v1_10 = part->unk2D;
                            if (temp_v1_10 >= 0x40U)
                            {
                                var_v0_8 = 0x1F2;
                            }
                            else
                            {
                                var_v0_8 = (temp_v1_10 >> 4) + 0x1EA;
                            }
                            temp_v0_7 = ((u32) part->unk28 >> 0xC) & 3;
                            switch (temp_v0_7)
                            { /* switch 2; irregular */
                            case 1: /* switch 2 */
                                var_v1_5 = var_v0_8 << 6;
                                var_v0_7 = part->unk2D & 0xF;
                                goto block_116;
                            case 2: /* switch 2 */
                                var_v0_8 = (rec->unk3B + 0x1F4) << 6;
                                var_v1_6 = ((u32) rec->unk1C >> 0x13) & 0xF;
                                goto block_112;
                            }
                        }
                        else
                        {
                            var_v0_9 = temp_a0_3 >> 0x10;
                            if (((u32) part->unk28 >> 0xC) & 3)
                            {
                                temp_v1_11 = part->unk2D;
                                if (temp_v1_11 >= 0x40U)
                                {
                                    var_v0_7 = 0x1F2;
                                }
                                else
                                {
                                    var_v0_7 = (temp_v1_11 >> 4) + 0x1EA;
                                }
                                temp_v0_8 = ((u32) part->unk28 >> 0xC) & 3;
                                switch (temp_v0_8)
                                { /* switch 3; irregular */
                                case 1: /* switch 3 */
                                    var_v1_5 = var_v0_7 << 6;
                                    var_v0_7 = part->unk2D & 0xF;
                                    goto block_116;
                                case 2: /* switch 3 */
                                    var_v0_9 = (u32) *(u16 *) ((u8 *) rec + 0x1E);
                                    goto block_92;
                                }
                            }
                            else
                            {
block_92:
                                *(s16 *) (var_s6 + 0) = (s16) ((((var_v0_9 & 3) + 0x1EF) << 6) | 0x10);
                            }
                        }
                    }
                    else if (!(((u32) part->unk28 >> 0xC) & 3))
                    {
                        if ((var_s5[1] & 3) != 2)
                        {
                            if (rec->unk3B == 8)
                            {
                                var_v0_10 = (var_s5[0] & 0x3F) | 0x7A80;
                            }
                            else
                            {
                                var_v0_10 = ((rec->unk3B + 0x1F4) << 6) | (var_s5[0] & 0x3F);
                            }
                            *(s16 *) (var_s6 + 0) = var_v0_10;
                            if (var_s5[0] == 0xB)
                            {
                                var_s6[-7] = (u8) (var_s6[-7] | 2);
                            }
                        }
                        else
                        {
                            goto block_113;
                        }
                    }
                    else
                    {
                        temp_v1_12 = part->unk2D;
                        if (temp_v1_12 >= 0x40U)
                        {
                            var_s1_4 = 0x1F2;
                            if ((u8) actor->unk228 < 2U)
                            {
                                var_s1_4 = (actor->unk228 * 2) + 0x1EE;
                            }
                        }
                        else
                        {
                            var_s1_4 = (temp_v1_12 >> 4) + 0x1EA;
                        }
                        temp_v0_9 = ((u32) part->unk28 >> 0xC) & 3;
                        switch (temp_v0_9)
                        { /* switch 4; irregular */
                        case 1: /* switch 4 */
                            var_v1_5 = var_s1_4 << 6;
                            var_v0_7 = part->unk2D & 0xF;
block_116:
                            *(s16 *) (var_s6 + 0) = (s16) (var_v1_5 | var_v0_7);
                            break;
                        case 2: /* switch 4 */
                            if ((u8) actor->unk228 >= 3U)
                            {
                                var_v1_5 = var_s1_4 << 6;
                                var_v0_7 = part->unk2D & 0xF;
                                goto block_116;
                            }
                            if ((var_s5[1] & 3) != 2)
                            {
                                var_v0_8 = (rec->unk3B + 0x1F4) << 6;
                                var_v1_6 = var_s5[0] & 0x3F;
block_112:
                                *(s16 *) (var_s6 + 0) = (s16) (var_v0_8 | var_v1_6);
                            }
                            else
                            {
block_113:
                                if (var_s5[0] == 1)
                                {
                                    var_s6[-7] = (u8) (var_s6[-7] | 2);
                                }
                                var_v1_5 = (rec->unk3B + 0x1F4) << 6;
                                var_v0_7 = ((s32) ((var_s5[0] * 0x10) + 0xC0) >> 4) & 0x3F;
                                goto block_116;
                            }
                            break;
                        }
                    }
                    temp_v1_13 = (s32) rec->unk8 >> 7;
                    if (temp_v1_13 < 0)
                    {
                        *cursor = (*cursor & 0xFF000000) | (base[0] & sp7C);
                        temp_v1_14 = (s32) cursor & sp7C;
                        var_s6 += 0x28;
                        cursor += 0x28;
                        base[0] = (s32) ((base[0] & 0xFF000000) | temp_v1_14);
                    }
                    else if (temp_v1_13 >= 0x1000)
                    {
                        *cursor = (*cursor & 0xFF000000) | (base[0xFFF] & sp7C);
                        temp_v1_15 = (s32) cursor & sp7C;
                        var_s6 += 0x28;
                        cursor += 0x28;
                        base[0xFFF] = (s32) ((base[0xFFF] & 0xFF000000) | temp_v1_15);
                    }
                    else
                    {
                        var_s6 += 0x28;
                        *cursor = (*cursor & 0xFF000000) | (base[temp_v1_13] & sp7C);
                        temp_v1_16 = &base[(s32) rec->unk8 >> 7];
                        temp_v0_10 = (*temp_v1_16 & 0xFF000000) | ((s32) cursor & sp7C);
                        cursor += 0x28;
                        *temp_v1_16 = temp_v0_10;
                    }
                }
                goto block_298;
            }
            temp_v1_17 = temp_v1_4 & 0xF;
            if ((flag == 0) || (temp_v1_17 == 2) || ((slot->unk178 & 1) && (temp_v1_17 == 0)))
            {
                temp_v1_18 = var_s5[1] & 0xF;
                switch (temp_v1_18)
                { /* switch 5 */
                case 0: /* switch 5 */
                    var_a3 = 0;
block_130:
                    func_8007F938(rec, slot, item, var_a3, sxy, dir, gte_out);
                    var_s5 += 9;
                    break;
                case 3: /* switch 5 */
                    var_a3 = 4;
                    goto block_130;
                case 6: /* switch 5 */
                    temp_v1_19 = slot->unk3C;
                    if (!(temp_v1_19 & 0x8000) && !(slot->unk174 & 0x1800) && (temp_v1_19 != 0xFFFF) && (((temp_a0_4 = rec->unk2A, (temp_a0_4 == 0x91)) && ((rec->unk21 & 0x7F) == 0x2E)) || (temp_a0_4 == 0x85) || (temp_a0_4 == 0x98)) && !(rec->unk3C & 0x01000000))
                    {
                        temp_v0_11 = func_800839F8(rec->unk3A, 0);
                        if (temp_v0_11 != -1U)
                        {
                            if (func_80083EEC(rec->unk3A, temp_v0_11, slot->unk3C) != 0)
                            {
                                slot->unk18D = temp_v0_11;
                                field_start_actor_animation(temp_v0_11, 0, NULL);
                                if (rec->unk21 & 0x80)
                                {
                                    dir->unk2 = 0;
                                    dir->unk0 = (s16) (s8) (u8) var_s5[-5];
                                    var_v0_11 = -(s8) *item;
                                }
                                else
                                {
                                    dir->unk2 = 0;
                                    dir->unk0 = (s16) (s8) (u8) var_s5[-5];
                                    var_v0_11 = (s8) *item;
                                }
                                dir->unk4 = (s16) var_v0_11;
                                gte_ldv0(dir);
                                gte_rtv0();
                                gte_stlvnl(gte_out);
                                slot->unk130 = (u16) gte_out->vx;
                                slot->unk132 = (u16) gte_out->vy;
                                if (rec->unk21 & 0x80)
                                {
                                    dir->unk2 = 0;
                                    dir->unk0 = (s16) (s8) var_s5[-3];
                                    var_v0_12 = -(s8) var_s5[-4];
                                }
                                else
                                {
                                    dir->unk2 = 0;
                                    dir->unk0 = (s16) (s8) var_s5[-3];
                                    var_v0_12 = (s8) var_s5[-4];
                                }
                                dir->unk4 = (s16) var_v0_12;
                                gte_ldv0(dir);
                                gte_rtv0();
                                gte_stlvnl(gte_out);
                                slot->unk134 = (u16) gte_out->vx;
                                slot->unk136 = (u16) gte_out->vy;
                                if (rec->unk21 & 0x80)
                                {
                                    dir->unk2 = 0;
                                    dir->unk0 = (s16) (s8) var_s5[-1];
                                    var_v0_13 = -(s8) var_s5[-2];
                                }
                                else
                                {
                                    dir->unk2 = 0;
                                    dir->unk0 = (s16) (s8) var_s5[-1];
                                    var_v0_13 = (s8) var_s5[-2];
                                }
                                dir->unk4 = (s16) var_v0_13;
                                gte_ldv0(dir);
                                gte_rtv0();
                                gte_stlvnl(gte_out);
                                slot->unk138 = (u16) gte_out->vx;
                                slot->unk13A = (u16) gte_out->vy;
                                if (rec->unk21 & 0x80)
                                {
                                    dir->unk2 = 0;
                                    dir->unk0 = (s16) (s8) var_s5[2];
                                    var_v0_14 = -(s8) var_s5[0];
                                }
                                else
                                {
                                    dir->unk2 = 0;
                                    dir->unk0 = (s16) (s8) var_s5[2];
                                    var_v0_14 = (s8) var_s5[0];
                                }
                                dir->unk4 = (s16) var_v0_14;
                                gte_ldv0(dir);
                                gte_rtv0();
                                gte_stlvnl(gte_out);
                                slot->unk13C = (u16) gte_out->vx;
                                slot->unk13E = (u16) gte_out->vy;
                            }
                        }
                        slot->unk3C = 0xFFFF;
                        var_v0_15 = (slot->unk174 & ~0x1800) | 0x1000;
block_297:
                        slot->unk174 = var_v0_15;
                    }
                    goto block_298;
                case 1: /* switch 5 */
                    if ((part->unk24 & 0x100000) && !(slot->unk174 & 0x1800) && ((((u8 *) &slot->unk178)[2] == 0) || (rec->unk2A == 0x91)) && (((temp_v1_20 = slot->unk3C, (temp_v1_20 != 0xFFFF)) && (temp_v1_20 != 0)) || (actor->unkC->unk14 == 3)))
                    {
                        if (rec->unk21 & 0x80)
                        {
                            dir->unk2 = 0;
                            dir->unk0 = (s16) (s8) (u8) var_s5[-5];
                            var_v0_16 = -(s8) *item;
                        }
                        else
                        {
                            dir->unk2 = 0;
                            dir->unk0 = (s16) (s8) (u8) var_s5[-5];
                            var_v0_16 = (s8) *item;
                        }
                        dir->unk4 = (s16) var_v0_16;
                        gte_ldv0(dir);
                        gte_rtv0();
                        gte_stlvnl(gte_out);
                        sp68[0].x = (s16) (sxy->x + gte_out->vx);
                        sp68[0].y = (s16) (sxy->y + gte_out->vy);
                        if (rec->unk21 & 0x80)
                        {
                            dir->unk2 = 0;
                            dir->unk0 = (s16) (s8) var_s5[-3];
                            var_v0_17 = -(s8) var_s5[-4];
                        }
                        else
                        {
                            dir->unk2 = 0;
                            dir->unk0 = (s16) (s8) var_s5[-3];
                            var_v0_17 = (s8) var_s5[-4];
                        }
                        dir->unk4 = (s16) var_v0_17;
                        gte_ldv0(dir);
                        gte_rtv0();
                        gte_stlvnl(gte_out);
                        sp68[1].x = (s16) (sxy->x + gte_out->vx);
                        sp68[1].y = (s16) (sxy->y + gte_out->vy);
                        if (rec->unk21 & 0x80)
                        {
                            dir->unk2 = 0;
                            dir->unk0 = (s16) (s8) var_s5[-1];
                            var_v0_18 = -(s8) var_s5[-2];
                        }
                        else
                        {
                            dir->unk2 = 0;
                            dir->unk0 = (s16) (s8) var_s5[-1];
                            var_v0_18 = (s8) var_s5[-2];
                        }
                        dir->unk4 = (s16) var_v0_18;
                        gte_ldv0(dir);
                        gte_rtv0();
                        gte_stlvnl(gte_out);
                        sp68[2].x = (s16) (sxy->x + gte_out->vx);
                        sp68[2].y = (s16) (sxy->y + gte_out->vy);
                        if (rec->unk21 & 0x80)
                        {
                            dir->unk2 = 0;
                            dir->unk0 = (s16) (s8) var_s5[2];
                            var_v0_19 = -(s8) var_s5[0];
                        }
                        else
                        {
                            dir->unk2 = 0;
                            dir->unk0 = (s16) (s8) var_s5[2];
                            var_v0_19 = (s8) var_s5[0];
                        }
                        dir->unk4 = (s16) var_v0_19;
                        gte_ldv0(dir);
                        gte_rtv0();
                        gte_stlvnl(gte_out);
                        temp_a3 = (u16) gte_out->vx;
                        sp68[3].x = (s16) (sxy->x + temp_a3);
                        sp68[3].y = (s16) (sxy->y + gte_out->vy);
                        temp_v0_12 = func_80097150(sp68, rec, &sp28, temp_a3);
                        if (temp_v0_12 == 1)
                        {
                            temp_v0_13 = &D_80105AE0[sp28.index];
                            temp_v0_13->unkC = (s32) (temp_v0_13->unkC & ~0x400);
                            (slot + ((u8 *) &slot->unk178)[3])->unk180 = (u8) sp28.index;
                            temp_v1_21 = ((u8 *) &slot->unk178)[3];
                            if (temp_v1_21 < 9U)
                            {
                                ((u8 *) &slot->unk178)[3] = (u8) (temp_v1_21 + 1);
                            }
                            if (actor->unkC->unk14 == 3)
                            {
                                actor->unk23A = (u8) (actor->unk23A | (1 << actor->unk232));
                                actor->unk229[actor->unk232] = (u8) sp28.index;
                                var_a0_7 = D_800F22A0;
                                if (var_a0_7 < 0)
                                {
                                    var_a0_7 += 0xFF;
                                }
                                var_v1_7 = D_800FDF58[sp28.index].unk0;
                                if (var_v1_7 < 0)
                                {
                                    var_v1_7 += 0xFF;
                                }
                                var_a0_8 = D_800F22A4;
                                sp6C->x = (u16) ((var_a0_7 >> 8) + ((var_v1_7 >> 8) + 0xA0));
                                if (var_a0_8 < 0)
                                {
                                    var_a0_8 += 0xFF;
                                }
                                temp_v1_22 = &D_800FDF58[sp28.index];
                                var_v0_20 = temp_v1_22->unk4;
                                if (var_v0_20 < 0)
                                {
                                    var_v0_20 += 0xFF;
                                }
                                var_a0_9 = temp_v1_22->unk8;
                                if (var_a0_9 < 0)
                                {
                                    var_a0_9 += 0x1FF;
                                }
                                var_v1_8 = D_800F22A8;
                                if (var_v1_8 < 0)
                                {
                                    var_v1_8 += 0x1FF;
                                }
                                sp6C->y = (u16) ((((var_a0_8 >> 8) + ((var_v0_20 >> 8) + 0x70)) - (var_a0_9 >> 9)) - (var_v1_8 >> 9));
                                if (D_800FDF58[sp28.index].unk21 & 0x80)
                                {
                                    var_a0_10 = sp28.x;
                                    var_v1_9 = (u16) sp6C->x;
                                    var_v0_21 = actor->unk232;
                                }
                                else
                                {
                                    var_v1_9 = sp28.x;
                                    var_a0_10 = (u16) sp6C->x;
                                    var_v0_21 = actor->unk232;
                                }
                                actor->unk1FE[var_v0_21].x = (s16) (var_v1_9 - var_a0_10);
                                actor->unk1FE[actor->unk232].y = (s16) (sp28.y - sp6C->y);
                                actor->unk232 = (u8) (actor->unk232 + 1);
                                temp_v0_14 = &D_80105AE0[sp28.index];
                                temp_v0_14->unk178 = (s32) (temp_v0_14->unk178 | 0x80);
                                var_s5 += 9;
                                func_8008A840(actor->unk228, sp28.index);
                                temp_v0_15 = sp28.x - sxy->x;
                                slot->unk13C = temp_v0_15;
                                slot->unk138 = temp_v0_15;
                                slot->unk134 = temp_v0_15;
                                slot->unk130 = temp_v0_15;
                                slot->unk174 = (s32) ((slot->unk174 & ~0x1800) | 0x1000);
                                temp_v0_16 = sp28.y - sxy->y;
                                slot->unk13E = temp_v0_16;
                                slot->unk13A = temp_v0_16;
                                slot->unk136 = temp_v0_16;
                                slot->unk132 = temp_v0_16;
                            }
                            else
                            {
                                temp_v1_23 = slot->unk3C;
                                if (temp_v1_23 == 0x24)
                                {
                                    if (D_800FDF58[sp28.index].unk25 == 0)
                                    {
                                        slot->unk178 = (u32) (slot->unk178 | 2);
                                        slot->unk170 = (u8) sp28.index;
                                        temp_v0_17 = &D_80105AE0[sp28.index];
                                        temp_v0_17->unk178 = (s32) (temp_v0_17->unk178 | 0x2000);
                                        goto block_236;
                                    }
                                }
                                else
                                {
                                    if ((temp_v1_23 == 0x59) || (temp_v1_23 == 0x66) || (temp_v1_23 == 0x2B))
                                    {
                                        if (!(((u32) D_80105AE0[sp28.index].unk178 >> 6) & 1))
                                        {
                                            var_v1_10 = sp28.index;
                                            temp_a1_3 = sp28.index < 3;
                                            if (temp_a1_3 == 0)
                                            {
                                                var_v1_10 = 2;
                                            }
                                            temp_v0_18 = D_80105880[var_v1_10].unkC;
                                            if (temp_v0_18 == sp28.index)
                                            {
                                                var_v1_11 = temp_v0_18;
                                                if (temp_a1_3 == 0)
                                                {
                                                    var_v1_11 = 2;
                                                }
                                                if (D_80105880[var_v1_11].unk0 == 0)
                                                {
                                                    goto block_208;
                                                }
                                                goto block_209;
                                            }
                                            goto block_208;
                                        }
block_208:
                                        if (D_80105AE0[sp28.index].unk178 & 1)
                                        {
block_209:
                                            slot->unk3C = 0;
                                        }
                                        else
                                        {
                                            temp_v0_19 = &D_800FDF58[sp28.index];
                                            temp_v0_19->unk21 = (u8) (temp_v0_19->unk21 & 0x7F);
                                            if (sp28.index < 2)
                                            {
                                                func_800A2DD8(sp28.index);
                                                D_80105AE0[sp28.index].unk18D = 0;
                                                D_800FDF58[sp28.index].unk30 = 0;
                                            }
                                        }
                                        goto block_236;
                                    }
                                    temp_v1_24 = &D_80105AE0[sp28.index];
                                    var_s1_5 = 1;
                                    if (!(temp_v1_24->unk178 & 1) || ((temp_v1_25 = &g_field_actor_slots[((u8 *) &temp_v1_24->unk178)[2]], (temp_v1_25->unk24 != 0)) && (temp_v1_25->unk228 == rec->unk3A)))
                                    {
                                        temp_v1_26 = rec->unk21 & 0x7F;
                                        switch (temp_v1_26)
                                        { /* switch 6; irregular */
                                        case 0x48: /* switch 6 */
                                            var_s1_5 = func_8008A9D8(rec->unk3A, sp28.index, 0x10);
                                            break;
                                        case 0x49: /* switch 6 */
                                            var_s1_5 = func_8008A9D8(rec->unk3A, sp28.index, 0x11);
                                            break;
                                        case 0x3E: /* switch 6 */
                                            var_s1_5 = func_8008A9D8(rec->unk3A, sp28.index, 0x19);
                                            break;
                                        case 0x45: /* switch 6 */
                                            var_s1_5 = func_8008A9D8(rec->unk3A, sp28.index, 0x1A);
                                            break;
                                        default: /* switch 6 */
                                            var_s1_5 = func_8008A840(rec->unk3A, sp28.index);
                                            break;
                                        }
                                    }
                                    if (sp28.index < 2)
                                    {
                                        if (!(D_800FDF58[sp28.index].unk1C & 0x1FF))
                                        {
                                            func_800A2DD8(sp28.index);
                                        }
                                    }
                                    if (var_s1_5 == 1)
                                    {
                                        temp_v1_27 = slot->unk3C;
                                        if ((temp_v1_27 == 1) || (temp_v1_27 == 3) || (var_v0_22 = temp_v1_27 & 0x8000, (temp_v1_27 == 0x10)))
                                        {
                                            slot->unk3C = 0x1E;
                                            goto block_236;
                                        }
                                    }
                                    else
                                    {
block_236:
                                        var_v0_22 = slot->unk3C & 0x8000;
                                    }
                                    if (var_v0_22 != 0)
                                    {
                                        sp30 = sp28.index;
                                        if (rec->unk2A == 0x91)
                                        {
                                            if (slot->unk18D != 0xFF)
                                            {
                                                temp_v0_20 = &D_80105AE0[sp28.index];
                                                temp_v0_20->unk178 = (s32) (temp_v0_20->unk178 | 0x80);
                                                var_a0_11 = slot->unk18D;
                                                goto block_253;
                                            }
                                        }
                                        else if (func_8009104C(rec->unk3A, 1, &sp30, slot->unk3C) != 0)
                                        {
                                            slot->unk174 = (s32) ((slot->unk174 & ~0x1800) | 0x1000);
                                        }
                                        goto block_254;
                                    }
                                    if (slot->unk3C != 0)
                                    {
                                        temp_v0_21 = func_800839F8(rec->unk3A, 0);
                                        if (temp_v0_21 != -1U)
                                        {
                                            temp_v1_28 = ((u32) slot->unk178 >> 2) & 7;
                                            switch (temp_v1_28)
                                            { /* switch 7; irregular */
                                            case 2: /* switch 7 */
                                                slot->unk3C = 0x74;
                                                break;
                                            case 4: /* switch 7 */
                                                slot->unk3C = 0x75;
                                                break;
                                            }
                                            var_a0_11 = temp_v0_21;
                                            if (func_80083EEC(rec->unk3A, temp_v0_21, slot->unk3C) != 0)
                                            {
                                                slot->unk18D = var_a0_11;
                                                sp30 = sp28.index;
block_253:
                                                field_start_actor_animation(var_a0_11, 1, &sp30);
block_254:
                                                temp_v1_29 = sp28.x - sxy->x;
                                                slot->unk13C = temp_v1_29;
                                                slot->unk138 = temp_v1_29;
                                                slot->unk134 = temp_v1_29;
                                                slot->unk130 = temp_v1_29;
                                                temp_v1_30 = sp28.y - sxy->y;
                                                slot->unk13E = temp_v1_30;
                                                slot->unk13A = temp_v1_30;
                                                slot->unk136 = temp_v1_30;
                                                slot->unk132 = temp_v1_30;
                                            }
                                        }
                                    }
                                    slot->unk3C = 0xFFFF;
                                    var_v0_15 = (slot->unk174 & ~0x1800) | 0x1000;
                                    goto block_297;
                                }
                                goto block_298;
                            }
                        }
                        else
                        {
                            if (temp_v0_12 == 2)
                            {
                                slot->unk3C = 0x1E;
                                temp_v0_22 = func_800839F8(rec->unk3A, 0);
                                if (temp_v0_22 != -1U)
                                {
                                    if (func_80083EEC(rec->unk3A, temp_v0_22, slot->unk3C) != 0)
                                    {
                                        slot->unk18D = temp_v0_22;
                                        sp30 = sp28.index;
                                        field_start_actor_animation(temp_v0_22, 1, &sp30);
                                        temp_v1_31 = sp28.x - sxy->x;
                                        slot->unk13C = temp_v1_31;
                                        slot->unk138 = temp_v1_31;
                                        slot->unk134 = temp_v1_31;
                                        slot->unk130 = temp_v1_31;
                                        temp_v1_32 = sp28.y - sxy->y;
                                        slot->unk13E = temp_v1_32;
                                        slot->unk13A = temp_v1_32;
                                        slot->unk136 = temp_v1_32;
                                        slot->unk132 = temp_v1_32;
                                    }
                                }
                                slot->unk3C = 0xFFFF;
                                var_v0_15 = (slot->unk174 & ~0x1800) | 0x1000;
                                goto block_297;
                            }
                            if (temp_v0_12 == 3)
                            {
                                slot->unk3C = 0xFFFF;
                                var_v0_15 = (slot->unk174 & ~0x1800) | 0x1000;
                                goto block_297;
                            }
                            goto block_298;
                        }
                    }
                    else
                    {
                        goto block_298;
                    }
                    break;
                case 5: /* switch 5 */
                    if ((rec->unk34 == 0) && !(rec->unk3C & 0x01000000))
                    {
                        temp_v1_33 = (u16) g_field_resource_entries[rec->unk3B].unkA >> 0xC;
                        if (temp_v1_33 != 1)
                        {
                            if (((s32) temp_v1_33 < 2) && (temp_v1_33 == 0))
                            {
                                var_s5 += 9;
                                func_800A3938(g_field_resource_entries[rec->unk3B].unkA & 0xFFF, func_8006CE70(rec->unk3A));
                            }
                            else
                            {
                                goto block_298;
                            }
                        }
                        else
                        {
                            var_s5 += 9;
                            temp_a2 = rec->unk3B;
                            func_800A39A8(g_field_resource_entries[temp_a2].unkA & 0xFFF, func_8006CE70(rec->unk3A), temp_a2 - 3, rec->unk3A);
                        }
                    }
                    else
                    {
                        goto block_298;
                    }
                    break;
                case 2: /* switch 5 */
                    if (part->unk24 & 0x100000)
                    {
                        sp78 += 1;
                        func_8007E4A8(sp64, rec->unk21 & 0x80, item);
                        var_s5 += 9;
                    }
                    else
                    {
                        goto block_298;
                    }
                    break;
                case 4: /* switch 5 */
                    if ((slot->unk174 & 0x1800) != 0x800)
                    {
                        temp_v1_34 = slot->unk3C;
                        if ((temp_v1_34 != 0xFFFF) && (temp_v1_34 != 0))
                        {
                            if (rec->unk21 & 0x80)
                            {
                                dir->unk2 = 0;
                                dir->unk0 = (s16) (s8) (u8) var_s5[-5];
                                var_v0_23 = -(s8) *item;
                            }
                            else
                            {
                                dir->unk2 = 0;
                                dir->unk0 = (s16) (s8) (u8) var_s5[-5];
                                var_v0_23 = (s8) *item;
                            }
                            dir->unk4 = (s16) var_v0_23;
                            gte_ldv0(dir);
                            gte_rtv0();
                            gte_stlvnl(gte_out);
                            slot->unk130 = (u16) gte_out->vx;
                            slot->unk132 = (u16) (gte_out->vy - sp74);
                            if (rec->unk21 & 0x80)
                            {
                                dir->unk2 = 0;
                                dir->unk0 = (s16) (s8) var_s5[-3];
                                var_v0_24 = -(s8) var_s5[-4];
                            }
                            else
                            {
                                dir->unk2 = 0;
                                dir->unk0 = (s16) (s8) var_s5[-3];
                                var_v0_24 = (s8) var_s5[-4];
                            }
                            dir->unk4 = (s16) var_v0_24;
                            gte_ldv0(dir);
                            gte_rtv0();
                            gte_stlvnl(gte_out);
                            slot->unk134 = (u16) gte_out->vx;
                            slot->unk136 = (u16) (gte_out->vy - sp74);
                            if (rec->unk21 & 0x80)
                            {
                                dir->unk2 = 0;
                                dir->unk0 = (s16) (s8) var_s5[-1];
                                var_v0_25 = -(s8) var_s5[-2];
                            }
                            else
                            {
                                dir->unk2 = 0;
                                dir->unk0 = (s16) (s8) var_s5[-1];
                                var_v0_25 = (s8) var_s5[-2];
                            }
                            dir->unk4 = (s16) var_v0_25;
                            gte_ldv0(dir);
                            gte_rtv0();
                            gte_stlvnl(gte_out);
                            slot->unk138 = (u16) gte_out->vx;
                            slot->unk13A = (u16) (gte_out->vy - sp74);
                            if (rec->unk21 & 0x80)
                            {
                                dir->unk2 = 0;
                                dir->unk0 = (s16) (s8) var_s5[2];
                                var_v0_26 = -(s8) var_s5[0];
                            }
                            else
                            {
                                dir->unk2 = 0;
                                dir->unk0 = (s16) (s8) var_s5[2];
                                var_v0_26 = (s8) var_s5[0];
                            }
                            dir->unk4 = (s16) var_v0_26;
                            gte_ldv0(dir);
                            gte_rtv0();
                            gte_stlvnl(gte_out);
                            slot->unk13C = (u16) gte_out->vx;
                            temp_a3_2 = slot->unk3C;
                            slot->unk13E = (u16) (gte_out->vy - sp74);
                            if (temp_a3_2 & 0x8000)
                            {
                                if (func_8009104C(rec->unk3A, 0, NULL, temp_a3_2) != 0)
                                {
                                    slot->unk174 = (s32) ((slot->unk174 & ~0x1800) | 0x1000);
                                }
                            }
                            else
                            {
                                temp_v0_23 = func_800839F8(rec->unk3A, 0);
                                if (temp_v0_23 != -1U)
                                {
                                    if (func_80083EEC(rec->unk3A, temp_v0_23, slot->unk3C) != 0)
                                    {
                                        slot->unk18D = temp_v0_23;
                                        field_start_actor_animation(temp_v0_23, 0, NULL);
                                    }
                                }
                            }
                            slot->unk3C = 0xFFFF;
                            var_v0_15 = (slot->unk174 & ~0x1800) | 0x800;
                            goto block_297;
                        }
                    }
                    goto block_298;
                default:
                    goto block_298;
                }
            }
            else
            {
block_298:
                var_s5 += 9;
            }
            temp_t1_2 = sp5C - 1;
            item += 9;
            sp5C = temp_t1_2;
        } while (temp_t1_2 != 0);
    }
    if (sp78 != 0)
    {
        var_a1 = sp64[0];
        temp_v0_24 = sp64[0x10];
        if (var_a1 != temp_v0_24)
        {
            var_a0_12 = sp64[2];
            do
            {
                *sp64 = (s16) ((s32) (((s32) (*sp64 * part->unk2E) >> 6) * D_80105768.unk0) >> 0xC);
                sp64 += 4;
                *(sp64 + 2) = (s16) ((s32) (((s32) (*(sp64 + 2) * part->unk33) >> 6) * D_80105768.unk4) >> 0xC);
            } while (*sp64 != temp_v0_24);
        }
        slot->unk12C = (s16) ((s32) (sp64[4] + sp64[0]) >> 1);
        var_v0_27 = sp64[4] - sp64[0];
        if (var_v0_27 < 0)
        {
            var_v0_27 = -var_v0_27;
        }
        slot->unk12E = var_v0_27;
        if ((var_v0_27 << 0x10) == 0)
        {
            var_v0_28 = (s16) sp64[8] - sp64[0];
            if (var_v0_28 < 0)
            {
                var_v0_28 = -var_v0_28;
            }
            slot->unk12E = var_v0_28;
            slot->unk12C = (s16) ((s32) ((s16) sp64[8] + sp64[0]) >> 1);
        }
        cursor = func_800871A0(rec, cursor, base, sp64);
    }
    return cursor;
}
