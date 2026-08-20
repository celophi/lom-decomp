/**
 * @file field9.c
 * @brief Field actor slot helpers carved from the unk2 segment.
 */

#include "common.h"

typedef struct
{
    s16 x;
    s16 y;
} Vec2s;

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
    u8 pad26[0x30 - 0x26];
    u16 unk30; /* 0x30 */
    u8 pad32[0x44 - 0x32];
    u32 unk44; /* 0x44 */
    u32 unk48; /* 0x48 */
    u32 unk4C; /* 0x4C */
    u8 pad50[0x54 - 0x50];
} Struct_D800FDF58;

typedef struct
{
    u8 pad0[0x6C];
    s16 unk6C; /* 0x6C */
    s16 unk6E; /* 0x6E */
    u8 pad70[0x130 - 0x70];
    Vec2s unk130[4]; /* 0x130 */
    s16 unk140; /* 0x140 */
    s16 unk142; /* 0x142 */
    s16 unk144; /* 0x144 */
    s16 unk146; /* 0x146 */
    u8 pad148[0x23C - 0x148];
} Struct_D80105AE0;

typedef struct
{
    u8 pad0[0x22];
    u8 unk22;  /* 0x22 */
    u8 unk23;  /* 0x23 */
    u8 unk24;  /* 0x24 */
    u8 pad25[0x228 - 0x25];
    u8 unk228; /* 0x228 */
    u8 unk229[9]; /* 0x229 */
    u8 pad232[0x23A - 0x232];
    u8 unk23A; /* 0x23A */
    u8 unk23B; /* 0x23B */
    u8 pad23C[0x244 - 0x23C];
} FieldActorState;

typedef struct
{
    u8 pad0[0x24];
    u32 unk24; /* 0x24 */
    u32 unk28; /* 0x28 */
    u8 pad2C[0x34 - 0x2C];
    u32 unk34; /* 0x34 */
} FieldViewParams;

typedef struct
{
    s32 unk0; /* 0x00 */
    s32 unk4; /* 0x04 */
    s32 unk8; /* 0x08 */
} FieldPosOut;

extern FieldActorState g_field_actor_slots[80];
extern Struct_D800FDF58 D_800FDF58[];
extern Struct_D800FDF58 D_800FF658[];
extern Struct_D80105AE0 D_80105AE0[];
extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;
extern s32 g_field_track_index;
extern s32 D_800473F8;

/**
 * @see decomp.me (100%)
 */
void func_80073EAC(FieldActorState *actor)
{
    FieldActorState *slots;
    FieldActorState *slot;

    if (func_80073F10(actor->unk22) == 0)
    {
        slots = g_field_actor_slots;
        slot = &slots[actor->unk22];
        slot->unk24 = 0;
        slot->unk23B = 0;
        slot->unk23A = 0;
    }
}

/**
 * @see decomp.me (100%)
 */
s32 func_80073F10(s32 arg0)
{
    s32 i;
    s32 val;
    Struct_D800FDF58 *base;
    u8 *p;

    i = 0;
    val = 0xFF;
    base = D_800FF658;
    p = (u8 *)base + 0x22;
    while (i < 0x103)
    {
        if (p[3] != val && *p == arg0)
        {
            return 1;
        }
        i++;
        p += 0x54;
    }
    return 0;
}

/**
 * @see decomp.me (100%)
 */
void func_80073F60(s32 arg0, s32 arg1, s32 arg2)
{
    D_800473F8 = (arg0 << 0x10) | (arg1 << 8) | arg2;
}

/**
 * @brief Resolve a record's world-position source into arg2, dispatching on
 *        the record's unk1B mode selector.
 * @param arg0 Record whose unk1B selects the position source.
 * @param arg1 View/camera parameter block (rotation/flags read in modes 5/6/9).
 * @param arg2 Output position triple (unk0/unk4/unk8).
 * @note WIP - not yet byte-matching. Residual is register coloring: the
 *       per-case emit order of the g_field_actor_slots vs D_800FDF58 symbol
 *       addresses differs from the target, plus a few shared-tail block
 *       placements (block_40/41/42/45). The control-flow shape, CSE re-read
 *       structure, and field types are established; see the working notes.
 * @see decomp.me (92.87%)
 */
void func_80073F7C(Struct_D800FDF58 *arg0, FieldViewParams *arg1, FieldPosOut *arg2)
{
    FieldActorState *var_v0_7;
    Struct_D800FDF58 *var_a1;
    Struct_D800FDF58 *var_a2;
    Struct_D800FDF58 *var_v0;
    Struct_D800FDF58 *var_v0_5;
    Struct_D800FDF58 *var_v0_9;
    Struct_D800FDF58 *src;
    Struct_D80105AE0 *pe;
    u8 *ent;
    s32 var_v1;
    s32 val;
    s32 v;
    Struct_D800FDF58 *var_v1_4;

    switch (arg0->unk1B)
    {
    case 1:
        arg2->unk0 = D_800FDF58[g_field_actor_slots[arg0->unk22].unk229[g_field_track_index]].unk0;
        arg2->unk4 = D_800FDF58[g_field_actor_slots[arg0->unk22].unk229[g_field_track_index]].unk4;
        var_v1 = g_field_actor_slots[arg0->unk22].unk229[g_field_track_index];
        var_v0 = &D_800FDF58[var_v1];
        goto block_42;
    case 2:
        arg2->unk0 = D_800FDF58[g_field_actor_slots[arg0->unk22].unk228].unk0;
        arg2->unk4 = D_800FDF58[g_field_actor_slots[arg0->unk22].unk228].unk4;
        var_v0 = &D_800FDF58[g_field_actor_slots[arg0->unk22].unk228];
        goto block_42;
    case 3:
        arg2->unk0 = arg0->unk44 - D_800F22A0;
        arg2->unk4 = arg0->unk48 - D_800F22A4;
        arg2->unk8 = arg0->unk4C - D_800F22A8;
        return;
    case 4:
        var_a1 = D_800FF658;
        if (D_800FF658[arg0->unk30].unk25 != 0xFF)
        {
            arg2->unk0 = D_800FF658[arg0->unk30].unk0;
            arg2->unk4 = D_800FF658[arg0->unk30].unk4;
            var_v1 = arg0->unk30;
            goto block_41;
        }
        arg2->unk0 = arg0->unk0;
        arg2->unk4 = arg0->unk4;
        arg2->unk8 = arg0->unk8;
        return;
    case 5:
        src = &D_800FDF58[g_field_actor_slots[arg0->unk22].unk228];
        ent = (u8 *)&D_80105AE0[g_field_actor_slots[arg0->unk22].unk228];
        arg2->unk0 = src->unk0 + (*(s16 *)(ent + (((u32)arg1->unk24 >> 0x13) & 0xC) + 0x130) << 8);
        arg2->unk4 = src->unk4 + (*(s16 *)(ent + (((u32)arg1->unk24 >> 0x13) & 0xC) + 0x132) << 8);
        arg2->unk8 = src->unk8;
        return;
    case 6:
        val = ((u32)arg1->unk28 >> 0x12) & 0x3F;
        if (val >= 0xA)
        {
            if (val < 0x26)
            {
                var_v1 = g_field_actor_slots[arg0->unk22].unk229[g_field_track_index];
            }
            else
            {
                goto owner6;
            }
        }
        else
        {
owner6:
            var_v1 = g_field_actor_slots[arg0->unk22].unk228;
        }
        var_a2 = &D_800FDF58[var_v1];
        if (var_a2->unk21 & 0x80)
        {
            goto block_32;
        }
        v = var_a2->unk0 - arg0->unk44;
        goto block_33;
    case 7:
        arg2->unk0 = D_80105AE0[g_field_actor_slots[arg0->unk22].unk229[g_field_track_index]].unk6C << 8;
        arg2->unk4 = 0;
        arg2->unk8 = D_80105AE0[g_field_actor_slots[arg0->unk22].unk229[g_field_track_index]].unk6E << 8;
        return;
    case 8:
        arg2->unk0 = D_800FF658[arg0->unk20].unk0;
        arg2->unk4 = D_800FF658[arg0->unk20].unk4;
        var_v0 = &D_800FF658[arg0->unk20];
        goto block_42;
    case 9:
        val = ((u32)arg1->unk28 >> 0x12) & 0x3F;
        if (val < 0xA)
        {
            var_a2 = &D_800FDF58[g_field_actor_slots[arg0->unk22].unk228];
            var_v0_5 = &D_800FDF58[g_field_actor_slots[arg0->unk22].unk229[g_field_track_index]];
        }
        else
        {
            if (val < 0x26)
            {
                var_v1 = g_field_actor_slots[arg0->unk22].unk229[g_field_track_index];
            }
            else
            {
                var_v1 = g_field_actor_slots[arg0->unk22].unk228;
            }
            var_a2 = &D_800FDF58[var_v1];
            var_v0_5 = &D_800FDF58[g_field_actor_slots[arg0->unk22].unk228];
        }
        if (var_v0_5->unk0 < var_a2->unk0)
        {
            if (((arg1->unk28 >> 0xA) & 1) || (arg1->unk34 & 0x08000000))
            {
                arg0->unk21 = arg0->unk21 & 0x7F;
            }
            v = var_a2->unk0 - arg0->unk44;
        }
        else
        {
            if (((arg1->unk28 >> 0xA) & 1) || (arg1->unk34 & 0x08000000))
            {
                arg0->unk21 = arg0->unk21 | 0x80;
            }
block_32:
            v = arg0->unk44 + var_a2->unk0;
        }
block_33:
        arg2->unk0 = v;
        arg2->unk4 = arg0->unk48 + var_a2->unk4;
        arg2->unk8 = arg0->unk4C + var_a2->unk8;
        return;
    case 10:
        var_a1 = D_800FDF58;
        pe = &D_80105AE0[g_field_actor_slots[arg0->unk22].unk228];
        arg2->unk0 = D_800FDF58[g_field_actor_slots[arg0->unk22].unk228].unk0 + ((pe->unk144 + pe->unk140) << 7);
        arg2->unk4 = D_800FDF58[g_field_actor_slots[arg0->unk22].unk228].unk4 + ((pe->unk146 + pe->unk142) << 7);
        var_v0_7 = &g_field_actor_slots[arg0->unk22];
        goto block_40;
    case 11:
        var_a1 = D_800FDF58;
        pe = &D_80105AE0[g_field_actor_slots[arg0->unk22].unk229[g_field_track_index]];
        arg2->unk0 = D_800FDF58[g_field_actor_slots[arg0->unk22].unk229[g_field_track_index]].unk0 + ((pe->unk144 + pe->unk140) << 7);
        arg2->unk4 = D_800FDF58[g_field_actor_slots[arg0->unk22].unk229[g_field_track_index]].unk4 + ((pe->unk146 + pe->unk142) << 7);
        var_v1 = g_field_actor_slots[arg0->unk22].unk229[g_field_track_index];
        goto block_41;
    case 12:
    case 13:
        var_a1 = D_800FDF58;
        v = D_800FDF58[g_field_actor_slots[arg0->unk22].unk229[g_field_track_index]].unk0 - D_800FDF58[g_field_actor_slots[arg0->unk22].unk228].unk0;
        if (arg0->unk1B == 0xC)
        {
            arg2->unk0 = D_800FDF58[g_field_actor_slots[arg0->unk22].unk228].unk0 - v;
        }
        else
        {
            arg2->unk0 = D_800FDF58[g_field_actor_slots[arg0->unk22].unk229[g_field_track_index]].unk0 + v;
        }
        arg2->unk4 = D_800FDF58[g_field_actor_slots[arg0->unk22].unk228].unk4;
        var_v0_7 = &g_field_actor_slots[arg0->unk22];
block_40:
        var_v1 = var_v0_7->unk228;
block_41:
        var_v0 = &var_a1[var_v1];
block_42:
        arg2->unk8 = var_v0->unk8;
        return;
    case 14:
        arg2->unk0 = (D_800FDF58[g_field_actor_slots[arg0->unk22].unk229[g_field_track_index]].unk0 * 2) - D_800FDF58[g_field_actor_slots[arg0->unk22].unk228].unk0;
        arg2->unk4 = D_800FDF58[g_field_actor_slots[arg0->unk22].unk229[g_field_track_index]].unk4;
        var_v1_4 = &D_800FDF58[g_field_actor_slots[arg0->unk22].unk229[g_field_track_index]];
        var_v0_9 = &D_800FDF58[g_field_actor_slots[arg0->unk22].unk228];
block_45:
        arg2->unk8 = (var_v1_4->unk8 * 2) - var_v0_9->unk8;
    default:
        return;
    case 15:
        arg2->unk0 = (D_800FDF58[g_field_actor_slots[arg0->unk22].unk229[g_field_track_index]].unk0 * 2) - D_800FF658[arg0->unk20].unk0;
        arg2->unk4 = D_800FDF58[g_field_actor_slots[arg0->unk22].unk229[g_field_track_index]].unk4;
        var_v1 = g_field_actor_slots[arg0->unk22].unk229[g_field_track_index];
        var_v1_4 = &D_800FDF58[var_v1];
        var_v0_9 = &D_800FF658[arg0->unk20];
        goto block_45;
    }
}
