#include "common.h"

typedef struct FieldActorAnimationDef
{
    u8 unk0[2];
    u8 pad2[0xC - 2];
    u16 unkC;
    u16 unkE;
    u16 unk10;
    u8 pad10[0x14 - 0x12];
    u8 unk14;
    u8 unk15;
    u8 pad16[0x18 - 0x16];
    u16 unk18;
} FieldActorAnimationDef;
typedef struct
{
    u32 unk0; /* 0x00 */
    u32 unk4; /* 0x04 */
    u8 unk8; /* 0x08 */
    u8 unk9; /* 0x09 */
    u8 padA;
    u8 unkB; /* 0x0B */
    u8 unkC; /* 0x0C */
    u8 unkD; /* 0x0D */
    u8 unkE; /* 0x0E */
    u8 unkF; /* 0x0F */
    u8 unk10; /* 0x10 */
    u8 unk11; /* 0x11 */
    u8 pad12[0x14 - 0x12];
    u32 unk14; /* 0x14 (overlaps unk16 at its upper halfword) */
    s16 unk18; /* 0x18 */
    u8 pad1A[0x23 - 0x1A];
    u8 unk23; /* 0x23 */
    u32 unk24; /* 0x24 (overlaps byte writes at 0x24/0x25) */
    u32 unk28; /* 0x28 */
    u8 unk2C; /* 0x2C */
    u8 pad2D;
    u8 unk2E; /* 0x2E */
    u8 pad2F[0x31 - 0x2F];
    u8 unk31; /* 0x31 */
    u8 unk32; /* 0x32 */
    u8 unk33; /* 0x33 */
    u32 unk34; /* 0x34 */
    u8 pad38[0x48 - 0x38];
} FieldActorPartDef;

typedef struct
{
    u8 pad0[0x91];
    u8 unk91;
    u8 unk92;
    u8 pad93[0x13F - 0x93];
    u8 unk13F;
    u8 unk140;
} Struct_801ED600;

typedef struct
{
    u8 unk0;
    u8 pad1;
    u8 unk2;
    u8 unk3[16];
    u8 unk13[9][16];
    u8 padA3;
    u16 unkA4[9][16];
    u16 unk1C4[9];
    u8 pad1D6[0x1FA - 0x1D6];
    u16 unk1FA;
    union
    {
        u32 unk1FC;
        struct
        {
            u16 lo;
            u16 animation_id;
        } h;
    } u1FC;
    u8 owner_object_index;
    u8 unk201[9];
    u8 unk20A;
    u8 unk20B;
    u16 unk20C;
    u16 unk20E;
    u16 unk210;
    u8 unk212;
    u8 unk213;
    u8 pad214[0x21C - 0x214];
} Struct_Unk28;

typedef struct
{
    FieldActorPartDef* unk0;
    u8 pad4[0xC - 4];
    FieldActorAnimationDef* unkC;
    u8 pad10[0x24 - 0x10];
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
    u8 pad1FE[0x222 - 0x1FE];
    u16 unk222;
    union
    {
        u32 unk224;
        struct
        {
            u16 lo;
            u16 animation_id;
        } h;
    } u224;
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
    s32 unk0; /* 0x00 */
    s32 unk4; /* 0x04 */
    s32 unk8; /* 0x08 */
    u32 unkC; /* 0x0C */
    s16 unk10; /* 0x10 */
    s16 unk12; /* 0x12 */
    s16 unk14; /* 0x14 */
    s16 unk16; /* 0x16 */
    u8 unk18; /* 0x18 */
    u8 unk19; /* 0x19 */
    u8 unk1A; /* 0x1A */
    u8 unk1B; /* 0x1B */
    s32 unk1C; /* 0x1C */
    u8 pad20[0x21 - 0x20];
    u8 unk21; /* 0x21 */
    u8 unk22; /* 0x22 */
    u8 unk23; /* 0x23 */
    u8 unk24; /* 0x24 */
    u8 unk25; /* 0x25 */
    u8 pad26[0x27 - 0x26];
    u8 unk27; /* 0x27 */
    u8 unk28; /* 0x28 */
    u8 pad29[0x2A - 0x29];
    s16 unk2A; /* 0x2A */
    u16 unk2C; /* 0x2C */
    u16 unk2E; /* 0x2E */
    s16 unk30; /* 0x30 */
    u8 unk32; /* 0x32 */
    u8 unk33; /* 0x33 */
    u8 unk34; /* 0x34 */
    u8 unk35; /* 0x35 */
    u8 unk36; /* 0x36 */
    u8 unk37; /* 0x37 */
    u8 unk38; /* 0x38 */
    u8 pad39[0x3A - 0x39];
    u8 unk3A; /* 0x3A */
    u8 unk3B; /* 0x3B */
    u32 unk3C; /* 0x3C */
    s32 unk40; /* 0x40 */
    u32 unk44; /* 0x44 */
    u32 unk48; /* 0x48 */
    u32 unk4C; /* 0x4C */
    u8 pad50[0x54 - 0x50];
} Struct_D800FDF58;

typedef struct
{
    u8 pad0[0xC];
    u32 unkC; /* 0x0C */
    u32 unk10; /* 0x10 */
    u8 pad14[0x3C - 0x14];
    s32 unk3C; /* 0x3C */
    u8 pad40[0x48 - 0x40];
    u16 unk48; /* 0x48 */
    s16 unk4A; /* 0x4A */
    s32 unk4C; /* 0x4C */
    s32 unk50; /* 0x50 */
    s32 unk54; /* 0x54 */
    s32 unk58; /* 0x58 */
    u8 pad5C[0x12C - 0x5C];
    u32 unk12C; /* 0x12C */
    u8 pad130[0x140 - 0x130];
    s16 unk140; /* 0x140 */
    s16 unk142; /* 0x142 */
    s16 unk144; /* 0x144 */
    s16 unk146; /* 0x146 */
    u8 pad148[0x168 - 0x148];
    u8 *unk168; /* 0x168 */
    u8 pad16C[0x174 - 0x16C];
    s32 unk174; /* 0x174 */
    union
    {
        s32 unk178;
        struct
        {
            u8 pad[2];
            u8 unk17A;
            u8 pad2;
        } b;
    } u; /* 0x178 */
    u8 pad17C[0x18E - 0x17C];
    u8 unk18E; /* 0x18E */
    u8 pad18F[0x19C - 0x18F];
    s32 unk19C; /* 0x19C */
    s32 unk1A0; /* 0x1A0 */
    u8 pad1A4[0x1A8 - 0x1A4];
    u8 unk1A8; /* 0x1A8 */
    u8 unk1A9; /* 0x1A9 */
    u8 unk1AA; /* 0x1AA */
    u8 pad1AB[0x23C - 0x1AB];
} Struct_D80105AE0;
typedef struct
{
    union
    {
        u16 h; /* offset 0x00 as a halfword (flags) */
        struct
        {
            u8 unk0; /* offset 0x00 */
            u8 unk1; /* offset 0x01 */
        } b;
    } u0;
    u8 unk2;                /* offset 0x02 */
    u8 unk3;                /* offset 0x03 */
    u8 pad0[0x254 - 4];     /* 0x04 .. 0x253 */
    u16 unk254;             /* offset 0x254 */
    u8 unk256;              /* offset 0x256 */
    u8 pad1[0x268 - 0x257]; /* 0x257 .. 0x267 */
} D_800FD818_type;
typedef struct
{
    u8* start; /* 0x00 */
    u8* end;   /* 0x04 */
    u8 unk8;   /* 0x08 */
    u8 slot_index; /* 0x09 */
    u8 padA[0xE - 0xA];
    s16 unkE; /* 0x0E */
    u32 flags; /* 0x10 */
} FieldResourceEntry;

extern Struct_D800FDF58 D_800FDF58[];
extern Struct_D80105AE0 D_80105AE0[];
extern D_800FD818_type D_800FD818[];
extern FieldActorPartDef D_800FE3A0[];
extern FieldActorState g_field_actor_slots[];
extern FieldResourceEntry g_field_resource_entries[];
extern s32 D_800EB0A4[];
extern s32 D_800EB0C4[];
extern u8 D_80105880[];
extern s32 D_8010A020[];
extern u16 *D_8010A02C;
extern u8 D_8010A038[];
extern s32 D_8010AE54;
extern s32 D_8010AE58;

typedef s32 M2C_UNK;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))

typedef struct Query88198 {
    s32 x;
    s32 y;
    s32 z;
    u16 unkC;
    s16 unkE;
    u16 unk10;
} Query88198;

typedef struct OutPair88198 { s32 unk0; s32 unk4; } OutPair88198;
typedef struct FieldA4D0Rect88198 { u16 x; u16 y; u16 w; u16 h; } FieldA4D0Rect88198;

M2C_UNK field_start_actor_animation();
s32 func_80060F58();
void func_8006304C();
M2C_UNK func_8006B4D0();
M2C_UNK func_8006C3FC();
M2C_UNK func_8006C5FC();
s32 func_8006C7D8();
s32 func_800839F8();
s32 field_object_has_active_actor_tracks();
s32 field_count_free_actor_slots();
s32 func_80083EEC();
s32 func_8008404C();
void func_8008A4D0();
M2C_UNK func_8008B870();
M2C_UNK func_8008BE38();
M2C_UNK func_8008BF88();
M2C_UNK func_8008C024();
s32 func_8009104C();
M2C_UNK func_8009D4D8();
M2C_UNK func_800A3938();
M2C_UNK func_800A39A8();
M2C_UNK func_800A3A90();

/**
 * @brief Field actor primary opcode dispatch (opcodes 0x00, 0x80..0xBC).
 *
 * Reads the current command byte from the actor's active command stream and
 * dispatches to the matching handler: animation/motion setup, spawn of child
 * actors (0xB9), path/target queries (0xB0/0xB1), resource loads, and the many
 * one-shot state transitions. Advances the actor's stream offset (unk2C) before
 * returning.
 *
 * @param arg0 Pointer to the field actor record (Struct_D800FDF58 layout).
 * @note WIP m2c-derived match (89.32%); not yet byte-for-byte. Preserves original
 *       codegen forms (M2C_FIELD accesses, do/while(0) shells, phi temporaries);
 *       do not clean up.
 */
void func_80088198(Struct_D800FDF58 *arg0) {
    FieldA4D0Rect88198 sp10;
    Query88198 sp18;
    Query88198 sp30;
    FieldActorPartDef *temp_v0_19;
    FieldActorPartDef *casea5_base;
    FieldActorPartDef *temp_v0_20;
    FieldActorPartDef *casea6_base;
    s32 casea4_one;
    s32 casea6_mask;
    FieldActorState *temp_v1_11;
    FieldActorState *temp_v1_9;
    Struct_D800FDF58 *var_s0;
    Struct_D800FDF58 *b9_actor_base;
    Struct_D80105AE0 *temp_v0_13;
    Struct_D80105AE0 *case83_base;
    Struct_D80105AE0 *case9e_states;
    u8 *case9e_entries;
    Struct_D80105AE0 *casea9_states;
    Struct_D80105AE0 *casea9_state;
    FieldActorState *casea9_slots;
    u8 *casea9_entries;
    u8 *casea9_check_entries;
    s32 shared_s1;
    Struct_D80105AE0 *b9_state_base;
    Struct_D80105AE0 *temp_v0_14;
    Struct_D80105AE0 *temp_v0_16;
    Struct_D80105AE0 *temp_v0_17;
    Struct_D80105AE0 *temp_v0_18;
    Struct_D80105AE0 *temp_v0_3;
    Struct_D80105AE0 *temp_v0_4;
    Struct_D80105AE0 *temp_v0_5;
    Struct_D80105AE0 *temp_v0_6;
    Struct_D80105AE0 *temp_v0_7;
    Struct_D80105AE0 *temp_v0_8;
    Struct_D80105AE0 *temp_v1_2;
    Struct_D80105AE0 *temp_v1_6;
    Struct_D80105AE0 *temp_v1_7;
    Struct_D80105AE0 *var_s1;
    s16 temp_v0_15;
    s16 var_v0;
    s16 var_v0_2;
    s16 var_v0_4;
    s16 var_v1;
    s16 var_v1_2;
    s32 *var_v1_4;
    s32 temp_a0_3;
    s32 temp_a0_4;
    s32 temp_a1_2;
    s32 temp_a1_5;
    s32 temp_a1_7;
    s32 temp_a2;
    s32 temp_a3_3;
    s32 temp_v0;
    s32 temp_v0_11;
    s32 temp_v0_12;
    s32 temp_v1_5;
    s32 var_a3;
    s32 var_s0_2;
    s32 var_s2;
    s32 var_s2_2;
    s32 var_s6;
    s32 var_v0_3;
    s32 var_v0_5;
    s32 var_v0_6;
    s32 var_v0_7;
    u16 *var_v1_3;
    u16 temp_a1_3;
    u16 temp_v1_3;
    u16 temp_v1_4;
    u8 temp_a0;
    u8 temp_a0_2;
    s32 temp_a1;
    u8 temp_a1_4;
    s32 temp_a1_6;
    u8 temp_a3;
    u8 temp_a3_2;
    u8 temp_s0_2;
    u8 temp_s5;
    u8 temp_v0_10;
    u8 temp_v0_2;
    s32 temp_v1;
    u8 temp_v1_8;
    u8 var_a0;
    u8 var_a1;
    void *temp_s0;
    void *temp_s2;
    void *temp_s3;
    void *temp_s3_2;
    void *temp_v1_10;
    u16 phi_v0;

    temp_a2 = 0x801ED400;
    do {
    do {
    do {
    do {
    do {
        if (arg0->unk28 == 0xFE) {
        temp_s3 = D_80105AE0[arg0->unk3A].unk168;
    } else {
        temp_s3 = (u8 *)D_8010A02C + D_8010A02C[arg0->unk28];
    }
    temp_s3 += arg0->unk2C;
    } while (0);
    } while (0);
    } while (0);
    } while (0);
    } while (0);
        do {
    do {
    do {
    do {
    temp_a1 = M2C_FIELD(temp_s3, u8 *, 0);
    } while (0);
    } while (0);
    } while (0);
    } while (0);
    switch (temp_a1) {
    case 0xFF:
        arg0->unk28 = 0xFF;
        arg0->unk10 = 0;
        arg0->unk2C += 1;
        return;
    case 0xA2:
        phi_v0 = (u16) arg0->unk2C;
        arg0->unk10 = 0;
        goto block_168;
    case 0xA3:
        phi_v0 = (u16) arg0->unk2C;
        arg0->unk10 = 1;
        goto block_168;
    case 0xAA:
        func_800A3938(M2C_FIELD(temp_s3, u8 *, 1), 0x80);
        var_v0 = arg0->unk2C + 2;
        goto block_169;
    case 0xAB:
        if ((u8)arg0->unk3A < 3U) {
            func_800A3A90(M2C_FIELD(temp_s3, u8 *, 1), 0x80, arg0->unk3A);
        } else if ((u8)arg0->unk3A < 6U) {
            temp_a3 = arg0->unk3A;
            func_800A39A8(M2C_FIELD(temp_s3, u8 *, 1), 0x80, temp_a3 - 3, temp_a3);
        }
        var_v0 = arg0->unk2C + 2;
        goto block_169;
    case 0xB9:
        do {
        do {
        do {
        var_s2 = 0xC;
        var_s6 = 1;
        b9_state_base = D_80105AE0;
        var_s1 = b9_state_base + 12;
        b9_actor_base = D_800FDF58;
        var_s0 = b9_actor_base + 12;
loop_15:
        temp_s5 = var_s0->unk25;
        if (temp_s5 == 0xFF) {
            func_8006B4D0(var_s2, 3, temp_a2);
            temp_a1_2 = var_s0->unk1C;
            var_s0->unk0 = arg0->unk0;
            temp_a1_2 &= ~0x1FF;
            var_s0->unk4 = arg0->unk4;
            temp_a1_2 |= 2;
            var_s0->unk8 = arg0->unk8;
            var_s0->unk25 = 0xFE;
            var_s0->unk27 = 0;
            var_s0->unk24 = var_s6;
            var_s0->unk2A = 0xB8;
            var_s0->unk21 = M2C_FIELD(temp_s3, u8 *, 1);
            M2C_FIELD(var_s0, s8 *, 0x3D) = 3;
            var_s0->unk1C = temp_a1_2;
            var_s0->unk28 = 0;
            var_s0->unk10 = var_s6;
            var_s0->pad20[0] = arg0->unk3A;
            var_s1->unk10 = 0;
            var_s1->unkC = 0;
            M2C_FIELD(var_s1, s32 *, 0x14) = var_s2;
            M2C_FIELD(var_s1, s16 *, 0x18) = 0;
            temp_v1_5 = var_s1->u.unk178;
        temp_v1_5 &= ~0x80;
        temp_v1_5 &= ~1;
        var_s1->u.unk178 = temp_v1_5;
            func_8006C3FC(var_s0, temp_a1_2);
            temp_v0 = func_800839F8(var_s2, 0);
            if (temp_v0 != -1) {
                if (func_80083EEC(var_s2, temp_v0, 0xB0U) != 0) {
                    field_start_actor_animation(temp_v0, 0, 0);
                }
            } else {
                var_s0->unk25 = temp_s5;
            }
        } else {
            var_s1 -= 1;
            var_s2 -= 1;
            var_s0 -= 1;
            if (var_s2 >= 3) {
                goto loop_15;
            }
        }
        } while (0);
        } while (0);
        } while (0);
        var_v0 = arg0->unk2C + 2;
        goto block_169;
    case 0x81:
        arg0->unk2A = (s16) M2C_FIELD(temp_s3, u8 *, 0);
        temp_v0_2 = M2C_FIELD(temp_s3, u8 *, 1);
        arg0->unk2E = (u16) temp_v0_2;
        if (temp_v0_2 == 0) {
            arg0->unk2E = 1;
        }
        arg0->unk24 = 1;
        arg0->unk2C += 2;
        func_8006C3FC(arg0);
        return;
    case 0x83:
    case 0x84:
    case 0x85:
        case83_base = D_80105AE0;
        temp_v0_3 = &case83_base[arg0->unk3A];
        temp_v0_3->u.unk178 &= ~0x1C;
        arg0->unk2A = (s16) M2C_FIELD(temp_s3, u8 *, 0);
        if ((u32) (temp_a1 - 0x83) < 2U) {
            arg0->unk2A = 0x85;
            case83_base[arg0->unk3A].pad148[0x27] = temp_a1 + 0x7D;
            var_v0_2 = (u16) arg0->unk2C + 1;
        } else {
            case83_base[arg0->unk3A].pad148[0x27] = M2C_FIELD(temp_s3, u8 *, 1);
            var_v0_2 = (u16) arg0->unk2C + 2;
        }
        arg0->unk2C = var_v0_2;
        temp_a0 = arg0->unk3A;
        if ((D_80105AE0[temp_a0].pad148[0x27] == 2) && ((u16) arg0->unk30 != 0) && (temp_a0 < 2U)) {
            arg0->unk21 = (arg0->unk21 & 0x80) + (u16)((u8) M2C_FIELD(arg0, s16 *, 0x30) + 0x1F);
            if ((func_8006C7D8(arg0) == 0) || ((u16) arg0->unk30 >= 5U)) {
                D_80105AE0[arg0->unk3A].pad17C[0x11] = 0;
                arg0->unk30 = 0;
                temp_v0_4 = &D_80105AE0[arg0->unk3A];
                temp_v0_4->unkC &= 0xFFFF7FFF;
                arg0->unk21 &= 0x80;
                func_8006C5FC(arg0);
                arg0->unk2A = 0x95;
                arg0->pad20[0] = 0x14;
                return;
            }
            arg0->unk21 = (arg0->unk21 & 0x80) + 0x1F;
            temp_v0_5 = &D_80105AE0[arg0->unk3A];
            temp_v0_5->unkC |= 0x8000;
            goto block_36;
        }
        D_80105AE0[arg0->unk3A].pad17C[0x11] = 0;
        arg0->unk30 = 0;
block_36:
        temp_v1_2 = &D_80105AE0[arg0->unk3A];
        if (!(temp_v1_2->unkC & 0x400) && ((D_8010AE54 == 0) || ((u32) (temp_v1_2->pad148[0x27] - 4) >= 4U))) {
            do {
do {
do {
            temp_s0 = (arg0->unk3B * 0x190) + ((D_80105AE0[arg0->unk3A].pad148[0x27] * 8) + D_8010A038);
            if (!(M2C_FIELD(temp_s0, u16 *, 2) & 0x400)) {
                if ((M2C_FIELD(temp_s0, u16 *, 0) != 0) || (M2C_FIELD(temp_s0, u16 *, 4) != 0)) {
                    if (M2C_FIELD(temp_s0, u16 *, 2) & 0x400) {
                        goto block_43;
                    }
                    goto block_45;
                }
                goto block_57;
            }
block_43:
            if ((field_object_has_active_actor_tracks(arg0->unk3A) == 0) && (field_count_free_actor_slots() >= 3)) {
block_45:
                if ((M2C_FIELD(temp_s0, u16 *, 0) & 0x8000) && !(M2C_FIELD(temp_s0, u16 *, 2) & 0x400)) {
                    if (((u8) arg0->unk3A < 3U) && (field_object_has_active_actor_tracks(arg0->unk3A) == 0) && (D_8010AE58 == 0) &&
                        (field_count_free_actor_slots() >= 3)) {
                        temp_a0_2 = arg0->unk3A;
                        if ((D_80105AE0[temp_a0_2].unk48 == 0xFF) && (func_8008404C(temp_a0_2, (M2C_FIELD(temp_s0, u16 *, 0) & 0x7FFF) + (u16)((D_800FD818[temp_a0_2].u0.b.unk1 * 0x18) + 0x88)) != 0)) {
                            if ((u8) arg0->unk3A >= 2U) {
                                var_v0_3 = 0x38;
                            } else {
                                goto block_59;
                            }
                            goto block_61;
                        }
                    }
                    goto block_57;
                }
                temp_a1_3 = M2C_FIELD(temp_s0, u16 *, 6);
                if (temp_a1_3 & 0x8000) {
                    if (func_8008404C(arg0->unk3A, temp_a1_3 & 0x3FF) == 0) {
                        goto block_57;
                    }
                    if ((u8) arg0->unk3A < 2U) {
block_59:
                        var_v0_3 = arg0->unk3A * 0x1C;
                    } else {
                        var_v0_3 = 0x38;
                    }
block_61:
                    g_field_actor_slots[M2C_FIELD(&D_80105880[var_v0_3], s32 *, 0x18)].unk26 = D_80105AE0[arg0->unk3A].pad148[0x27];
                    goto block_62;
                }
block_62:
                if (M2C_FIELD(temp_s0, u16 *, 2) & 0x400) {
                    temp_v0_6 = &D_80105AE0[arg0->unk3A];
                    temp_v0_6->u.unk178 |= 0x40;
                    temp_v0_7 = &D_80105AE0[arg0->unk3A];
                    temp_v0_7->unk174 &= ~0x400;
                    D_80105AE0[arg0->unk3A].unk4A = 0;
                    temp_v0_8 = &D_80105AE0[arg0->unk3A];
                    temp_v0_8->unk4C = (s32) (temp_v0_8->unk4C & ~1);
                    temp_v1_3 = M2C_FIELD(temp_s0, u16 *, 4);
                    if ((temp_v1_3 != 0xFFFF) && (temp_v1_3 != 0)) {
                        D_80105AE0[arg0->unk3A].unk3C = (s32) M2C_FIELD(temp_s0, u16 *, 4);
                    }
                } else if (!(M2C_FIELD(temp_s0, u16 *, 0) & 0x8000)) {
                    temp_v1_4 = M2C_FIELD(temp_s0, u16 *, 4);
                    if ((temp_v1_4 != 0xFFFF) && (temp_v1_4 != 0)) {
                        shared_s1 = func_800839F8((s32) arg0->unk3A, 0);
                        if ((shared_s1 != -1) && (func_80083EEC((s32) arg0->unk3A, shared_s1, M2C_FIELD(temp_s0, u16 *, 4)) != 0)) {
                            temp_a3_2 = arg0->unk3A;
                            g_field_actor_slots[shared_s1].unk26 = D_80105AE0[temp_a3_2].pad148[0x27];
                            field_start_actor_animation(shared_s1, 0, 0);
                        }
                    }
                }
                func_8009D4D8(arg0, (u8) M2C_FIELD(temp_s0, u16 *, 2));
                return;
            }
            goto block_57;
            } while (0);
} while (0);
} while (0);
        }
block_57:
        arg0->unk2A = 0;
        return;
    case 0xAD:
        arg0->unk33 = 1;
        /* fallthrough */
    case 0x88:
        arg0->unk2A = (s16) M2C_FIELD(temp_s3, u8 *, 0);
        temp_a1_4 = M2C_FIELD(temp_s3, u8 *, 1);
        arg0->unk1B = temp_a1_4;
        if (g_field_resource_entries[arg0->unk3B].flags & 1) {
            arg0->unk21 = D_800EB0C4[temp_a1_4 >> 5];
        } else {
            arg0->unk21 = D_800EB0A4[temp_a1_4 >> 5] + ((arg0->unk33 & 1) * 5) + 5;
        }
        var_a1 = M2C_FIELD(temp_s3, u8 *, 2);
        arg0->unk24 = 1;
        var_v1 = (u16) arg0->unk2C + 3;
        arg0->unk2E = (u16) var_a1;
        goto block_143;
    case 0x8D:
        arg0->unk2A = (s16) M2C_FIELD(temp_s3, u8 *, 0);
        temp_v0_10 = M2C_FIELD(temp_s3, u8 *, 1) | (arg0->unk21 & 0x80);
        arg0->unk21 = temp_v0_10;
        if (temp_v0_10 & 0x80) {
            arg0->unk1B = 0;
        } else {
            arg0->unk1B = 0x80;
        }
        var_a1 = M2C_FIELD(temp_s3, u8 *, 2);
        arg0->unk24 = 1;
        var_v1 = (u16) arg0->unk2C + 3;
        arg0->unk2E = (u16) var_a1;
        goto block_143;
    case 0x8F:
        func_8008BF88(arg0, M2C_FIELD(temp_s3, u8 *, 1), M2C_FIELD(temp_s3, u8 *, 2), M2C_FIELD(temp_s3, u8 *, 3));
        var_v0 = (u16) arg0->unk2C + 4;
        goto block_169;
    case 0xB0:
    case 0xB1:
        temp_a1_5 = arg0->unk0;
        if ((temp_a1_5 < 0) || (temp_a3_3 = M2C_FIELD((void *)temp_a2, s16 *, 0) << 8, ((temp_a1_5 < temp_a3_3) == 0)) || (temp_v1_5 = arg0->unk8, (temp_v1_5 < 0)) || (temp_a2 = (s32) (M2C_FIELD((void *)temp_a2, u16 *, 2) << 0x10) >> 7, ((temp_v1_5 < temp_a2) == 0)) || (temp_v1_6 = &D_80105AE0[arg0->unk3A], temp_v0_11 = M2C_FIELD(temp_v1_6, s32 *, 0x50), (temp_v0_11 < 0)) || (temp_v0_11 >= temp_a3_3) || (temp_v0_12 = M2C_FIELD(temp_v1_6, s32 *, 0x58), (temp_v0_12 < 0)) || (temp_v0_12 >= temp_a2)) {
            M2C_FIELD(&D_80105AE0[arg0->unk3A], s16 *, 0x1A6) = 0;
            temp_v0_13 = &D_80105AE0[arg0->unk3A];
            M2C_FIELD(temp_v0_13, s32 *, 0x1AC) = (s32) M2C_FIELD(temp_v0_13, s32 *, 0x50);
            temp_v0_14 = &D_80105AE0[arg0->unk3A];
            M2C_FIELD(temp_v0_14, s32 *, 0x1B0) = (s32) M2C_FIELD(temp_v0_14, s32 *, 0x58);
            var_a1 = 1;
            M2C_FIELD(&D_80105AE0[arg0->unk3A], s16 *, 0x1A4) = 1;
            arg0->unk2E = 0xFF;
            arg0->unk24 = 1;
            var_v1 = (u16) arg0->unk2C + 1;
            arg0->unk2A = (s16) M2C_FIELD(temp_s3, u8 *, 0);
        } else {
            sp18.x = temp_a1_5;
            sp18.y = arg0->unk4;
            sp18.z = arg0->unk8;
            var_v1_2 = 9;
            if (D_800FE3A0[arg0->unk3A].unk2E == 0x40) {
                var_v1_2 = 0xC;
                var_v0_4 = 8;
            } else {
                var_v0_4 = 6;
            }
            sp18.unkC = var_v1_2;
            sp18.unk10 = var_v0_4;
            sp30.unkC = var_v1_2;
            sp30.unk10 = var_v0_4;
            sp18.unkE = 0x10;
            sp30.unkE = 0x10;
            func_8006304C(&sp18);
            sp30.x = D_80105AE0[arg0->unk3A].unk50;
            sp30.y = D_80105AE0[arg0->unk3A].unk54;
            sp30.z = D_80105AE0[arg0->unk3A].unk58;
            temp_v0_15 = func_80060F58(&sp18, &sp30, (OutPair88198 *)((u8 *)&D_80105AE0[arg0->unk3A] + 0x1AC), 0);
            if (temp_v0_15 <= 0) {
                M2C_FIELD(&D_80105AE0[arg0->unk3A], s16 *, 0x1A6) = 0;
                temp_v0_16 = &D_80105AE0[arg0->unk3A];
                M2C_FIELD(temp_v0_16, s32 *, 0x1AC) = (s32) M2C_FIELD(temp_v0_16, s32 *, 0x50);
                temp_v0_17 = &D_80105AE0[arg0->unk3A];
                M2C_FIELD(temp_v0_17, s32 *, 0x1B0) = (s32) M2C_FIELD(temp_v0_17, s32 *, 0x58);
                var_a1 = 1;
                M2C_FIELD(&D_80105AE0[arg0->unk3A], s16 *, 0x1A4) = 1;
                arg0->unk2E = 0xFF;
                arg0->unk24 = 1;
                var_v1 = (u16) arg0->unk2C + 1;
                arg0->unk2A = (s16) M2C_FIELD(temp_s3, u8 *, 0);
            } else {
                M2C_FIELD(&D_80105AE0[arg0->unk3A], s16 *, 0x1A4) = temp_v0_15;
                M2C_FIELD(&D_80105AE0[arg0->unk3A], s16 *, 0x1A6) = 0;
                var_a1 = M2C_FIELD(temp_s3, u8 *, 0);
                arg0->unk2E = 0xFF;
                arg0->unk24 = 1;
                var_v1 = (u16) arg0->unk2C + 1;
                arg0->unk2A = (s16) var_a1;
            }
        }
        goto block_143;
    case 0x89:
    case 0x8A:
    case 0x8B:
    case 0x8C:
    case 0xAC:
        arg0->unk2A = (s16) M2C_FIELD(temp_s3, u8 *, 0);
        arg0->unk2C = (u16) arg0->unk2C + 2;
        arg0->unk24 = 1;
        arg0->unk2E = (u16) M2C_FIELD(temp_s3, u8 *, 1);
        func_8006C3FC(arg0);
        return;
    case 0x9C:
    case 0x9D:
        arg0->unk2E = 0xFF;
        arg0->unk2A = (s16) M2C_FIELD(temp_s3, u8 *, 0);
        arg0->unk21 = M2C_FIELD(temp_s3, u8 *, 1) + (arg0->unk21 & 0x80);
        arg0->pad20[0] = M2C_FIELD(temp_s3, u8 *, 2);
        arg0->unk2C = (u16) arg0->unk2C + 4;
        arg0->unk24 = 1;
        arg0->pad26[0] = M2C_FIELD(temp_s3, u8 *, 3);
        func_8006C3FC(arg0);
        return;
    case 0x9E:
        case9e_entries = D_80105880;
        if ((u8) arg0->unk3A < 2U) {
            var_v0_5 = arg0->unk3A * 0x1C;
        } else {
            var_v0_5 = 0x38;
        }
        if (M2C_FIELD(case9e_entries + var_v0_5, s32 *, 0) == 0) {
            shared_s1 = M2C_FIELD(temp_s3, u8 *, 1) + (M2C_FIELD(temp_s3, u8 *, 2) << 8);
            temp_a1_6 = arg0->unk3A;
            arg0->unk2C = (u16) arg0->unk2C + 3;
            case9e_states = D_80105AE0;
            if (case9e_states[temp_a1_6].unkC & 0x400) {
                goto block_154;
            }
            if (D_8010AE54 != 0) {
                goto block_155;
            }
            if (func_8008404C(temp_a1_6, shared_s1) == 0) {
                goto block_155;
            }
            case9e_states[arg0->unk3A].unk174 |= 0x8000;
            return;
        }
        break;
    case 0xA9:
        casea9_check_entries = D_80105880;
        if ((u8) arg0->unk3A < 2U) {
            var_v0_6 = arg0->unk3A * 0x1C;
        } else {
            var_v0_6 = 0x38;
        }
        if (M2C_FIELD(casea9_check_entries + var_v0_6, s32 *, 0) == 0) {
            temp_s0_2 = M2C_FIELD(temp_s3, u8 *, 3);
            shared_s1 = M2C_FIELD(temp_s3, u8 *, 1) + (M2C_FIELD(temp_s3, u8 *, 2) << 8);
            arg0->unk2C = (u16) arg0->unk2C + 4;
            casea9_states = D_80105AE0;
            casea9_states[arg0->unk3A].unk3C = 0xFFFF;
            if (D_8010AE54 != 0) {
                goto block_154;
            }
            if (func_8008404C(arg0->unk3A, shared_s1) == 0) {
                goto block_155;
            }
            temp_v1_8 = arg0->unk3A;
            casea9_state = &casea9_states[temp_v1_8];
            casea9_slots = g_field_actor_slots;
            casea9_entries = D_80105880;
            if (temp_v1_8 < 2U) {
                var_v0_7 = temp_v1_8 * 0x1C;
            } else {
                var_v0_7 = 0x38;
            }
            casea9_slots[M2C_FIELD(&casea9_entries[var_v0_7], s32 *, 0x18)].unk26 = temp_s0_2;
            casea9_state->pad148[0x27] = temp_s0_2;
            casea9_states[arg0->unk3A].unk174 |= 0x8000;
            return;
        }
        break;
    case 0xBC:
        shared_s1 = 2;
        if ((u8) arg0->unk3A < 2U) {
            shared_s1 = arg0->unk3A;
        }
        temp_s2 = (shared_s1 * 0x1C) + D_80105880;
        temp_a0_3 = M2C_FIELD(temp_s2, s32 *, 0);
        if (((u32) (temp_a0_3 - 1) < 2U) && (M2C_FIELD(temp_s2, s32 *, 0xC) == arg0->unk3A)) {
            var_s6 = 1;
            if (temp_a0_3 != var_s6) {
                temp_v1_9 = &g_field_actor_slots[M2C_FIELD(temp_s2, s32 *, 0x18)];
                if (temp_v1_9->unk23A == 0) {
                    temp_v1_9->unkC = M2C_FIELD(temp_v1_9, FieldActorAnimationDef **, 0x10);
                    D_80105AE0[arg0->unk3A].u.b.pad2 = 0;
                    field_start_actor_animation(M2C_FIELD(temp_s2, s32 *, 0x18), 0, 0);
                    D_80105AE0[shared_s1].u.b.pad[1] = (u8) M2C_FIELD(temp_s2, s32 *, 0x18);
                    g_field_actor_slots[M2C_FIELD(temp_s2, s32 *, 0x18)].unk2A = var_s6;
                    arg0->unk2A = 0xBC;
                }
                goto block_167;
            }
        } else {
            goto block_167;
        }
        break;
    case 0x9F:
        shared_s1 = 2;
        if ((u8) arg0->unk3A < 2U) {
            shared_s1 = arg0->unk3A;
        }
        temp_v1_10 = (shared_s1 * 0x1C) + D_80105880;
        temp_a0_4 = M2C_FIELD(temp_v1_10, s32 *, 0);
        if (((u32) (temp_a0_4 - 1) < 2U) && (temp_a1_7 = M2C_FIELD(temp_v1_10, s32 *, 0xC), (temp_a1_7 == arg0->unk3A))) {
            if (temp_a0_4 != 1) {
                temp_v1_11 = &g_field_actor_slots[M2C_FIELD(temp_v1_10, s32 *, 0x18)];
                if (temp_v1_11->unk23A == 0) {
                    var_s2_2 = 0;
                    if (M2C_FIELD(M2C_FIELD(temp_v1_11, void **, 0x10), u16 *, 0xC) & 0x800) {
                        var_s0_2 = 0;
                        var_v1_3 = temp_v1_11->unk240;
                        do {
                            if (*var_v1_3 != 0) {
                                var_s2_2 += 1;
                            }
                            var_s0_2 += 1;
                            var_v1_3 += 1;
                        } while (var_s0_2 < 3);
                        var_a0 = arg0->unk3A;
                        var_a3 = ((var_s2_2 - 1) << 0xC) | 0x4400;
                    } else {
                        var_a0 = (u8) temp_a1_7;
                        var_a3 = 0;
                    }
                    if (func_8009104C((s32) var_a0, 0, 0, var_a3) != 0) {
                        arg0->unk2A = 0xBC;
                        D_80105AE0[arg0->unk3A].u.b.pad2 = 0;
                    }
                }
                goto block_140;
            }
        } else {
block_140:
            var_v0 = (u16) arg0->unk2C + 2;
            goto block_169;
        }
        break;
    case 0xA0: {
        Struct_D800FDF58 *actor;
        u16 off;
        u8 value;
        actor = arg0;
        value = M2C_FIELD(temp_s3, u8 *, 0);
        actor->unk2E = 0xFA;
        off = actor->unk2C;
        actor->unk2A = value;
        value = M2C_FIELD(temp_s3, u8 *, 1);
        off += 2;
        actor->unk2C = off;
        actor->unk24 = 1;
        actor->pad20[0] = value;
        func_8006C3FC(actor);
        return;
    }
    case 0xA7:
    case 0xB6: {
        Struct_D800FDF58 *actor;
        Struct_D80105AE0 *states;
        u8 value;
        actor = arg0;
        actor->unk2E = 0xFA;
        value = M2C_FIELD(temp_s3, u8 *, 0);
        actor->unk2A = value;
        value = M2C_FIELD(temp_s3, u8 *, 1);
        actor->unk1B = value;
        states = D_80105AE0;
        value = M2C_FIELD(temp_s3, u8 *, 2);
        actor->pad20[0] = value;
        states[actor->unk3A].pad148[0x29] = M2C_FIELD(temp_s3, u8 *, 3);
        var_a1 = M2C_FIELD(temp_s3, u8 *, 4);
        var_v1 = actor->unk2C;
        actor->unk2E = 0xF0;
        actor->unk24 = 1;
        var_v1 += 5;
        actor->unk21 = var_a1;
block_143:
        actor->unk2C = var_v1;
        func_8006C3FC(actor);
        return;
    }
    case 0x82:
        arg0->unk2C = (u16) arg0->unk2C + 1;
        func_8008B870(arg0, 0);
        return;
    case 0x8E:
        arg0->unk2C = (u16) arg0->unk2C + 1;
        func_8008BE38(arg0, 1);
        return;
    case 0x90:
        arg0->unk2C = (u16) arg0->unk2C + 1;
        func_8008C024(arg0, -1);
        return;
    case 0x97:
        temp_s3 += 1;
        arg0->unk2C = (u16) arg0->unk2C + 9;
        sp10.x = M2C_FIELD(temp_s3, u8 *, 0) + (M2C_FIELD(temp_s3, u8 *, 1) << 8);
        sp10.y = (s16) M2C_FIELD(temp_s3, u8 *, 2);
        sp10.w = (s16) M2C_FIELD(temp_s3, u8 *, 3);
        sp10.h = (s16) M2C_FIELD(temp_s3, u8 *, 4);
        func_8008A4D0(arg0, &sp10, M2C_FIELD(temp_s3, u8 *, 5) | (M2C_FIELD(temp_s3, u8 *, 6) << 8), M2C_FIELD(temp_s3, u8 *, 7));
        return;
    case 0xA1:
        shared_s1 = M2C_FIELD(temp_s3, u8 *, 1) + (M2C_FIELD(temp_s3, u8 *, 2) << 8);
        arg0->unk2C = (u16) arg0->unk2C + 3;
        if (D_8010AE54 != 0) {
            goto block_154;
        }
        if (func_8008404C(arg0->unk3A, shared_s1) == 0) {
            goto block_155;
        }
        if ((u8) arg0->unk3A < 2U) {
            var_v1_4 = &D_8010A020[arg0->unk3A];
        } else {
            var_v1_4 = D_8010A020 + 8;
        }
        *var_v1_4 = 1;
        return;
block_154:
block_155:
        arg0->unk28 = 0xFF;
        arg0->unk10 = 0;
        return;
    case 0xA4:
        casea4_one = 1;
        if ((u8) arg0->unk3A < 2U) {
            arg0->unk2E = casea4_one;
            arg0->unk2A = (s16) M2C_FIELD(temp_s3, u8 *, 0);
            arg0->unk24 = casea4_one;
            arg0->unk21 = M2C_FIELD(temp_s3, u8 *, 1) | (arg0->unk21 & 0x80);
            func_8006C3FC(arg0);
        }
        var_v0 = (u16) arg0->unk2C + 2;
        goto block_169;
    case 0xA5:
        casea5_base = D_800FE3A0;
        temp_v0_19 = &casea5_base[arg0->unk3A];
        temp_v0_19->unk34 |= 0x800000;
        goto block_167;
    case 0xA6:
        casea6_mask = 0xFF7FFFFF;
        casea6_base = D_800FE3A0;
        temp_v0_20 = &casea6_base[arg0->unk3A];
        temp_v0_20->unk34 &= casea6_mask;
        goto block_167;
    case 0xA8: {
        Struct_D800FDF58 *actor;
        u16 off;
        u8 value;
        actor = arg0;
        actor->unk2A = M2C_FIELD(temp_s3, u8 *, 0);
        value = M2C_FIELD(temp_s3, u8 *, 2);
        actor->unk2E = value;
        off = actor->unk2C;
        value = M2C_FIELD(temp_s3, u8 *, 1);
        actor->unk27 = 0;
        off += 3;
        actor->unk2C = off;
        actor->unk24 = 1;
        actor->unk21 = value;
        func_8006C3FC(actor);
        return;
    }
    case 0xB7: {
        Struct_D800FDF58 *actor;
        u16 off;
        u8 value;
        actor = arg0;
        actor->unk2A = M2C_FIELD(temp_s3, u8 *, 0);
        value = M2C_FIELD(temp_s3, u8 *, 2);
        actor->unk2E = value;
        value = M2C_FIELD(temp_s3, u8 *, 1);
        actor->unk21 = value;
        off = actor->unk2C;
        value = M2C_FIELD(temp_s3, u8 *, 3);
        actor->unk27 = 0;
        off += 4;
        actor->unk2C = off;
        actor->unk24 = 1;
        actor->pad20[0] = value;
        func_8006C5FC(actor);
        return;
    }
    case 0xB2:
        arg0->pad20[0] = 0;
        var_v0 = (u16) arg0->unk2C + 1;
        arg0->unk2A = (s16) M2C_FIELD(temp_s3, u8 *, 0);
        goto block_169;
    case 0xB4:
        if (arg0->unk25 == 0xFE) {
            arg0->unk25 = 0;
        } else {
            arg0->unk25 = 0xFE;
        }
        goto block_167;
    case 0x0:
block_167:
        phi_v0 = (u16) arg0->unk2C;
block_168:
        var_v0 = phi_v0 + 1;
block_169:
        arg0->unk2C = var_v0;
    default:
        return;
    }
}
