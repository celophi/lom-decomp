/** @file field_actor_script_ops.c
 * @brief Actor script interpreter: script entry, opcode dispatch, key-based action
 *        commands and key-based animation commands.
 *
 * One translation unit: the jump tables of func_80088198, func_80089980 and
 * func_8008B288 share one object's .rodata (the zero word at 0x8005095C is the
 * compiler's 8-byte alignment before func_8008B288's table).
 */

/* Script entry, current-command lookup, opcode dispatch, and busy-state selection. */


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
    u8 unk8;  /* 0x08 */
    u8 unk9;  /* 0x09 */
    u8 padA;
    u8 unkB;  /* 0x0B */
    u8 unkC;  /* 0x0C */
    u8 unkD;  /* 0x0D */
    u8 unkE;  /* 0x0E */
    u8 unkF;  /* 0x0F */
    u8 unk10; /* 0x10 */
    u8 unk11; /* 0x11 */
    u8 pad12[0x14 - 0x12];
    u32 unk14; /* 0x14 (overlaps unk16 at its upper halfword) */
    s16 unk18; /* 0x18 */
    u8 pad1A[0x23 - 0x1A];
    u8 unk23;  /* 0x23 */
    u32 unk24; /* 0x24 (overlaps byte writes at 0x24/0x25) */
    u32 unk28; /* 0x28 */
    u8 unk2C;  /* 0x2C */
    u8 pad2D;
    u8 unk2E; /* 0x2E */
    u8 pad2F[0x31 - 0x2F];
    u8 unk31;  /* 0x31 */
    u8 unk32;  /* 0x32 */
    u8 unk33;  /* 0x33 */
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
    u8 pad20[1];
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
    u8 unk32;  /* 0x32 */
    u8 unk33;  /* 0x33 */
    u8 unk34;  /* 0x34 */
    u8 unk35;  /* 0x35 */
    u8 unk36;  /* 0x36 */
    u8 unk37;  /* 0x37 */
    u8 unk38;  /* 0x38 */
    u8 pad39[0x3A - 0x39];
    u8 unk3A;  /* 0x3A */
    u8 unk3B;  /* 0x3B */
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
    u32 unkC;  /* 0x0C */
    u32 unk10; /* 0x10 */
    s32 unk14;
    u8 pad18[0x3C - 0x18];
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
    u8* unk168; /* 0x168 */
    u8 pad16C[3];
    u8 unk16F;
    u8 unk170;
    u8 unk171;
    u8 pad172[2];
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
    s16 unk1A4;
    s16 unk1A6;
    u8 tint_red; /* 0x1A8 */
    u8 tint_green; /* 0x1A9 */
    u8 tint_blue; /* 0x1AA */
    u8 pad1AB;
    s32 unk1AC;
    s32 unk1B0;
    u8 pad1B4[0x23C - 0x1B4];
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
    u8* start;     /* 0x00 */
    u8* end;       /* 0x04 */
    u8 unk8;       /* 0x08 */
    u8 slot_index; /* 0x09 */
    u8 padA[0xE - 0xA];
    s16 unkE;  /* 0x0E */
    u32 flags; /* 0x10 */
} FieldResourceEntry;

extern Struct_D800FDF58 g_field_actors[];
extern Struct_D80105AE0 g_field_object_states[];
extern D_800FD818_type g_field_player_records[];
extern FieldActorPartDef g_field_object_parts[];
extern FieldActorState g_field_actor_slots[];
extern FieldResourceEntry g_field_resource_entries[];
extern s32 g_field_direction_animation_modes[];
extern s32 g_field_actor_walk_animations[];
extern u8 g_field_actor_bindings[];
extern s32 D_8010A020[];
extern u16* g_field_actor_scripts;
extern u8 g_field_resource_actions[];
extern s32 D_8010AE54;
extern s32 D_8010AE58;

typedef s32 M2C_UNK;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8*)(expr) + (offset)))

typedef struct Query88198
{
    s32 x;
    s32 y;
    s32 z;
    u16 unkC;
    s16 unkE;
    u16 unk10;
} Query88198;

typedef struct OutPair88198
{
    s32 unk0;
    s32 unk4;
} OutPair88198;
typedef struct FieldA4D0Rect88198
{
    u16 x;
    u16 y;
    u16 w;
    u16 h;
} FieldA4D0Rect88198;

void field_start_actor_animation();
s32 func_80060F58();
void func_8006304C();
M2C_UNK field_initialize_actor_record();
void field_restart_actor_animation();
M2C_UNK field_restart_actor_animation_reverse();
s32 field_get_next_animation_frame_count();
s32 func_800839F8();
s32 field_object_has_active_actor_tracks();
s32 field_count_free_actor_slots();
s32 func_80083EEC();
s32 func_8008404C();
void func_8008A4D0();
void func_8008B870();
void func_8008BE38();
void func_8008BF88();
void func_8008C024();
s32 field_start_bound_action_animation();
M2C_UNK func_8009D4D8();
void func_800A3938();
M2C_UNK func_800A39A8();
M2C_UNK func_800A3A90();


void func_80088198(Struct_D800FDF58 *rec);

/**
 * @see decomp.me (100%) TODO
 */
void func_800880EC(Struct_D800FDF58 *rec)
{
    if (rec->unk28 != 0xFF)
    {
        if (rec->unk2A == 0)
        {
            func_80088198(rec);
        }
    }
}


/**
 * @brief Resolve the current command in a field object's active script.
 * @param object Field object whose active script and bytecode offset are resolved.
 * @return Pointer to the object's current script command.
 */
u8 *field_get_object_script_command(Struct_D800FDF58 *object)
{
    u8 *script_base;

    if (object->unk28 == 0xFE)
    {
        script_base = g_field_object_states[object->unk3A].unk168;
    }
    else
    {
        script_base = (u8 *)g_field_actor_scripts + g_field_actor_scripts[object->unk28];
    }
    return script_base + object->unk2C;
}


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
 * @see decomp.me (100%)
 */
void func_80088198(Struct_D800FDF58* arg0)
{
    FieldA4D0Rect88198 sp10;
    Query88198 sp18;
    Query88198 sp30;
    FieldActorPartDef* temp_v0_19;
    FieldActorPartDef* casea5_base;
    FieldActorPartDef* temp_v0_20;
    FieldActorPartDef* casea6_base;
    s32 casea6_mask;
    FieldActorState* temp_v1_11;
    FieldActorState* temp_v1_9;
    Struct_D800FDF58* var_s0;
    Struct_D800FDF58* b9_actor_base;
    Struct_D80105AE0* temp_v0_13;
    Struct_D80105AE0* case83_base;
    Struct_D80105AE0* case9e_states;
    u8* case9e_entries;
    Struct_D80105AE0* casea9_states;
    Struct_D80105AE0* casea9_state;
    FieldActorState* casea9_slots;
    FieldActorState* track_slots;
    u8* casea9_entries;
    u8* casea9_check_entries;
    s32 shared_s1;
    Struct_D80105AE0* b9_state_base;
    Struct_D80105AE0* temp_v0_14;
    Struct_D80105AE0* temp_v0_16;
    Struct_D80105AE0* temp_v0_17;
    Struct_D80105AE0* temp_v0_3;
    Struct_D80105AE0* temp_v0_4;
    Struct_D80105AE0* temp_v0_5;
    Struct_D80105AE0* temp_v0_6;
    Struct_D80105AE0* temp_v0_7;
    Struct_D80105AE0* temp_v0_8;
    Struct_D80105AE0* temp_v1_2;
    Struct_D80105AE0* temp_v1_6;
    Struct_D80105AE0* var_s1;
    s16 var_v0;
    s16 var_v0_2;
    s32 offset_8d;
    s32 offset_query;
    s32 queryoff2;
    s32 queryoff3;
    s16 var_v1;
    u16 offset_88;
    s32* var_v1_4;
    s32 temp_a0_3;
    s32 temp_a0_4;
    s32 temp_a1_2;
    s32 temp_a1_5;
    s32 temp_a1_7;
    s32 temp_a2;
    s32 temp_a3_3;
    s32 temp_v0;
    s32 track_result;
    s32 temp_v0_11;
    s32 temp_v0_12;
    s32 temp_v1_5;
    s32 var_s2;
    s32 var_s6;
    s32 var_v0_3;
    s32 var_v0_5;
    s32 var_v0_6;
    s32 var_v0_7;
    u16* var_v1_3;
    u16 temp_a1_3;
    u16 temp_v1_3;
    u16 temp_v1_4;
    u8 temp_a0;
    u8 temp_a0_2;
    s32 temp_a1;
    u8 temp_a1_4;
    s32 temp_a1_6;
    s32 temp_a3;
    u8 temp_a3_2;
    s32 temp_s0_2;
    u8 temp_s5;
    u8 temp_v0_10;
    u8 temp_v0_2;
    s32 temp_v1;
    u8 temp_v1_8;
    u8 var_a1;
    void* temp_s0;
    void* temp_s2;
    void* temp_s3;
    void* temp_v1_10;
    Struct_D800FDF58* call_actor;
    FieldActorState* case83_slots;
    u8* case83_entries;
    u8* bc_entries;
    u8* track_entries;
    Struct_D80105AE0* update_states;
    s32 spawn_command;
    s32 parent_index;
    s32 advance_0;
    Struct_D80105AE0* query_base;
    Struct_D80105AE0* precheck_states;
    FieldActorState* bc_slots;
    s32 resource_offset;
    u8* action_row;
    s32 action_mask;
    s32 decoded_a4;
    s32 a4_old_flags;
    s32 a4_command_flags;
    s32 decoded_b2;
    s32 track_owner;
    s32 slot_offset_83;
    s32 slot_offset_a9;

    temp_a2 = 0x801ED400;

    do
    {
        if (arg0->unk28 == 0xFE)
        {
            temp_s3 = g_field_object_states[arg0->unk3A].unk168;
        }
        else
        {
            temp_s3 = (u8*)g_field_actor_scripts + g_field_actor_scripts[arg0->unk28];
        }
        temp_s3 += arg0->unk2C;
    } while (0);

    temp_a1 = M2C_FIELD(temp_s3, u8*, 0);

    switch (temp_a1)
    {
    case 0xFF:
        arg0->unk28 = 0xFF;
        arg0->unk10 = 0;
        arg0->unk2C += 1;
        return;
    case 0xA2:
        arg0->unk10 = 0;
        arg0->unk2C++;
        return;
    case 0xA3:
        arg0->unk10 = 1;
        arg0->unk2C++;
        return;
    case 0xAA:
        func_800A3938(M2C_FIELD(temp_s3, u8*, 1), 0x80);
        advance_0 = arg0->unk2C;
        advance_0 += 2;
        arg0->unk2C = advance_0;
        return;
    case 0xAB:
        if ((u8)arg0->unk3A < 3U)
        {
            func_800A3A90(M2C_FIELD(temp_s3, u8*, 1), 0x80, arg0->unk3A);
        }
        else if ((u8)arg0->unk3A < 6U)
        {
            temp_a3 = arg0->unk3A;
            func_800A39A8(M2C_FIELD(temp_s3, u8*, 1), 0x80, temp_a3 - 3, temp_a3);
        }
        arg0->unk2C = arg0->unk2C + 2;
        return;
    case 0xB9:

        do
        {
            do
            {
                var_s2 = 0xC;
                var_s6 = 1;
                b9_state_base = g_field_object_states;
                var_s1 = b9_state_base + 12;
                b9_actor_base = g_field_actors;
                var_s0 = b9_actor_base + 12;
            loop_15:
                temp_s5 = var_s0->unk25;
                if (temp_s5 == 0xFF)
                {
                    field_initialize_actor_record(var_s2, 3, temp_a2);
                    temp_a1_2 = var_s0->unk1C & ~0x1FF;
                    var_s0->unk0 = arg0->unk0;
                    var_s0->unk4 = arg0->unk4;
                    temp_a1_2 |= 2;
                    var_s0->unk8 = arg0->unk8;
                    var_s0->unk25 = 0xFE;
                    spawn_command = M2C_FIELD(temp_s3, u8*, 1);
                    var_s0->unk27 = 0;
                    var_s0->unk24 = var_s6;
                    var_s0->unk2A = 0xB8;
                    var_s0->unk21 = spawn_command;
                    parent_index = arg0->unk3A;
                    M2C_FIELD(var_s0, s8*, 0x3D) = 3;
                    var_s0->unk1C = temp_a1_2;
                    var_s0->unk28 = 0;
                    var_s0->unk10 = var_s6;
                    var_s0->pad20[0] = parent_index;
                    var_s1->unk10 = 0;
                    var_s1->unkC = 0;
                    M2C_FIELD(var_s1, s32*, 0x14) = var_s2;
                    M2C_FIELD(var_s1, s16*, 0x18) = 0;
                    temp_v1_5 = var_s1->u.unk178;
                    temp_v1_5 &= ~0x80;
                    temp_v1_5 &= ~1;
                    var_s1->u.unk178 = temp_v1_5;
                    field_restart_actor_animation(var_s0, temp_a1_2);
                    temp_v0 = func_800839F8(var_s2, 0);
                    if (temp_v0 != -1)
                    {
                        if (func_80083EEC(var_s2, temp_v0, 0xB0U) != 0)
                        {
                            field_start_actor_animation(temp_v0, 0, 0);
                        }
                    }
                    else
                    {
                        var_s0->unk25 = temp_s5;
                    }
                }
                else
                {
                    var_s1 -= 1;
                    var_s2 -= 1;
                    var_s0 -= 1;
                    if (var_s2 >= 3)
                    {
                        goto loop_15;
                    }
                }
            } while (0);
        } while (0);

        arg0->unk2C = arg0->unk2C + 2;
        return;
    case 0x81:
        arg0->unk2A = (s16)M2C_FIELD(temp_s3, u8*, 0);
        temp_v0_2 = M2C_FIELD(temp_s3, u8*, 1);
        arg0->unk2E = (u16)temp_v0_2;
        if (temp_v0_2 == 0)
        {
            arg0->unk2E = 1;
        }
        arg0->unk24 = 1;
        arg0->unk2C += 2;
        field_restart_actor_animation(arg0);
        return;
    case 0x83:
    case 0x84:
    case 0x85:
        case83_base = g_field_object_states;
        temp_v0_3 = &case83_base[arg0->unk3A];
        temp_v0_3->u.unk178 &= ~0x1C;
        arg0->unk2A = (s16)M2C_FIELD(temp_s3, u8*, 0);
        if ((u32)(temp_a1 - 0x83) < 2U)
        {
            arg0->unk2A = 0x85;
            case83_base[arg0->unk3A].unk16F = temp_a1 + 0x7D;
            var_v0_2 = (u16)arg0->unk2C + 1;
        }
        else
        {
            case83_base[arg0->unk3A].unk16F = M2C_FIELD(temp_s3, u8*, 1);
            var_v0_2 = (u16)arg0->unk2C + 2;
        }
        arg0->unk2C = var_v0_2;
        temp_a0 = arg0->unk3A;
        if ((g_field_object_states[temp_a0].unk16F == 2) && ((u16)arg0->unk30 != 0) && (temp_a0 < 2U))
        {
            arg0->unk21 = (arg0->unk21 & 0x80) + (u16)((u8)M2C_FIELD(arg0, s16*, 0x30) + 0x1F);
            if ((field_get_next_animation_frame_count(arg0) == 0) || ((u16)arg0->unk30 >= 5U))
            {
                g_field_object_states[arg0->unk3A].pad17C[0x11] = 0;
                arg0->unk30 = 0;
                temp_v0_4 = &g_field_object_states[arg0->unk3A];
                temp_v0_4->unkC &= 0xFFFF7FFF;
                arg0->unk21 &= 0x80;
                field_restart_actor_animation_reverse(arg0);
                arg0->unk2A = 0x95;
                arg0->pad20[0] = 0x14;
                return;
            }
            arg0->unk21 = (arg0->unk21 & 0x80) + 0x1F;
            temp_v0_5 = &g_field_object_states[arg0->unk3A];
            temp_v0_5->unkC |= 0x8000;
            goto block_36;
        }
        g_field_object_states[arg0->unk3A].pad17C[0x11] = 0;
        arg0->unk30 = 0;
    block_36:
        precheck_states = g_field_object_states;
        temp_v1_2 = &precheck_states[arg0->unk3A];
        if (temp_v1_2->unkC & 0x400)
        {
            arg0->unk2A = 0;
            return;
        }
        if (D_8010AE54 != 0 && (u32)(temp_v1_2->unk16F - 4) < 4U)
        {
            arg0->unk2A = 0;
            return;
        }
        {

            do
            {
                resource_offset = arg0->unk3B * 0x190;
                action_row = g_field_resource_actions + g_field_object_states[arg0->unk3A].unk16F * 8;
                temp_s0 = (void*)(resource_offset + (s32)action_row);
                if (!(M2C_FIELD(temp_s0, u16*, 2) & 0x400))
                {
                    if ((M2C_FIELD(temp_s0, u16*, 0) != 0) || (M2C_FIELD(temp_s0, u16*, 4) != 0))
                    {
                        if (M2C_FIELD(temp_s0, volatile u16*, 2) & 0x400)
                        {
                            goto block_43;
                        }
                        goto block_45;
                    }
                    goto block_57;
                }
            block_43:
                if ((field_object_has_active_actor_tracks(arg0->unk3A) == 0) && (field_count_free_actor_slots(arg0->unk3A) >= 3))
                {
                block_45:
                    if ((M2C_FIELD(temp_s0, u16*, 0) & 0x8000) && !(M2C_FIELD(temp_s0, u16*, 2) & 0x400))
                    {
                        if (((u8)arg0->unk3A < 3U) && (field_object_has_active_actor_tracks(arg0->unk3A) == 0) && (D_8010AE58 == 0) &&
                            (field_count_free_actor_slots(arg0->unk3A) >= 3))
                        {
                            temp_a0_2 = arg0->unk3A;
                            if ((g_field_object_states[temp_a0_2].unk48 == 0xFF) &&
                                (func_8008404C(temp_a0_2, (M2C_FIELD(temp_s0, u16*, 0) & 0x7FFF) + (u16)((g_field_player_records[temp_a0_2].u0.b.unk1 * 0x18) + 0x88)) !=
                                 0))
                            {
                                case83_slots = g_field_actor_slots;
                                case83_entries = g_field_actor_bindings;
                                if ((u8)arg0->unk3A >= 2U)
                                {
                                    var_v0_3 = 0x38;
                                }
                                else
                                {
                                    goto block_59;
                                }
                                goto block_61;
                            }
                        }
                        goto block_57;
                    }
                    temp_a1_3 = M2C_FIELD(temp_s0, u16*, 6);
                    if (temp_a1_3 & 0x8000)
                    {
                        if (func_8008404C(arg0->unk3A, temp_a1_3 & 0x3FF) == 0)
                        {
                        block_57:
                            arg0->unk2A = 0;
                            return;
                        }
                        case83_slots = g_field_actor_slots;
                        case83_entries = g_field_actor_bindings;
                        if ((u8)arg0->unk3A < 2U)
                        {
                        block_59:
                            var_v0_3 = arg0->unk3A * 0x1C;
                        }
                        else
                        {
                            var_v0_3 = 0x38;
                        }
                    block_61:
                        slot_offset_83 = (M2C_FIELD(&case83_entries[var_v0_3], s32*, 0x18)) * (s32)sizeof(FieldActorState);
                        ((FieldActorState*)((s32)case83_slots + slot_offset_83))->unk26 = g_field_object_states[arg0->unk3A].unk16F;
                        goto block_62;
                    }
                block_62:
                    if (M2C_FIELD(temp_s0, u16*, 2) & 0x400)
                    {
                        update_states = g_field_object_states;
                        temp_v0_6 = &update_states[arg0->unk3A];
                        temp_v0_6->u.unk178 |= 0x40;
                        temp_v0_7 = &g_field_object_states[arg0->unk3A];
                        action_mask = ~0x400;
                        temp_v0_7->unk174 = temp_v0_7->unk174 & action_mask;
                        g_field_object_states[arg0->unk3A].unk4A = 0;
                        temp_v0_8 = &g_field_object_states[arg0->unk3A];
                        action_mask = ~1;
                        temp_v0_8->unk4C = temp_v0_8->unk4C & action_mask;
                        temp_v1_3 = M2C_FIELD(temp_s0, u16*, 4);
                        if ((temp_v1_3 != 0xFFFF) && (temp_v1_3 != 0))
                        {
                            g_field_object_states[arg0->unk3A].unk3C = (s32)M2C_FIELD(temp_s0, u16*, 4);
                        }
                    }
                    else if (!(M2C_FIELD(temp_s0, u16*, 0) & 0x8000))
                    {
                        temp_v1_4 = M2C_FIELD(temp_s0, u16*, 4);
                        if ((temp_v1_4 != 0xFFFF) && (temp_v1_4 != 0))
                        {
                            shared_s1 = func_800839F8((s32)arg0->unk3A, 0);
                            if ((shared_s1 != -1) && (func_80083EEC((s32)arg0->unk3A, shared_s1, M2C_FIELD(temp_s0, u16*, 4)) != 0))
                            {
                                temp_a3_2 = arg0->unk3A;
                                g_field_actor_slots[shared_s1].unk26 = g_field_object_states[temp_a3_2].unk16F;
                                field_start_actor_animation(shared_s1, 0, 0);
                            }
                        }
                    }
                    func_8009D4D8(arg0, (u8)M2C_FIELD(temp_s0, u16*, 2));
                    return;
                }
                goto block_57;
            } while (0);
        }
        goto block_57;
    case 0xAD:
        arg0->unk33 = 1;
        /* fallthrough */
    case 0x88:
        arg0->unk2A = (s16)M2C_FIELD(temp_s3, u8*, 0);
        temp_a1_4 = M2C_FIELD(temp_s3, u8*, 1);
        arg0->unk1B = temp_a1_4;
        if (g_field_resource_entries[arg0->unk3B].flags & 1)
        {
            arg0->unk21 = g_field_actor_walk_animations[temp_a1_4 >> 5];
        }
        else
        {
            arg0->unk21 = g_field_direction_animation_modes[temp_a1_4 >> 5] + ((arg0->unk33 & 1) * 5) + 5;
        }
        call_actor = arg0;
        var_a1 = M2C_FIELD(temp_s3, u8*, 2);
        arg0->unk24 = 1;
        offset_88 = arg0->unk2C;
        offset_88 += 3;
        arg0->unk2E = (u16)var_a1;
        call_actor->unk2C = offset_88;
        field_restart_actor_animation(call_actor);
        return;
    case 0x8D:
        arg0->unk2A = (s16)M2C_FIELD(temp_s3, u8*, 0);
        temp_v0_10 = M2C_FIELD(temp_s3, u8*, 1) | (arg0->unk21 & 0x80);
        arg0->unk21 = temp_v0_10;
        if (temp_v0_10 & 0x80)
        {
            arg0->unk1B = 0;
        }
        else
        {
            arg0->unk1B = 0x80;
        }
        call_actor = arg0;
        var_a1 = M2C_FIELD(temp_s3, u8*, 2);
        arg0->unk24 = 1;
        offset_8d = arg0->unk2C;
        offset_8d += 3;
        arg0->unk2E = (u16)var_a1;
        call_actor->unk2C = offset_8d;
        field_restart_actor_animation(call_actor);
        return;
    case 0x8F:
        func_8008BF88(arg0, M2C_FIELD(temp_s3, u8*, 1), M2C_FIELD(temp_s3, u8*, 2), M2C_FIELD(temp_s3, u8*, 3));
        arg0->unk2C = (u16)arg0->unk2C + 4;
        return;
    case 0xB0:
    case 0xB1:
        temp_a1_5 = arg0->unk0;
        if ((temp_a1_5 < 0) || (temp_a3_3 = M2C_FIELD((void*)temp_a2, s16*, 0) << 8, ((temp_a1_5 < temp_a3_3) == 0)) ||
            (temp_v1_5 = arg0->unk8, (temp_v1_5 < 0)) || (temp_a2 = (s32)(M2C_FIELD((void*)temp_a2, u16*, 2) << 0x10) >> 7, ((temp_v1_5 < temp_a2) == 0)) ||
            (query_base = g_field_object_states, temp_v1_6 = &query_base[arg0->unk3A], temp_v0_11 = M2C_FIELD(temp_v1_6, s32*, 0x50), (temp_v0_11 < 0)) ||
            (temp_v0_11 >= temp_a3_3) || (temp_v0_12 = M2C_FIELD(temp_v1_6, s32*, 0x58), (temp_v0_12 < 0)) || (temp_v0_12 >= temp_a2))
        {
            call_actor = arg0;
            g_field_object_states[arg0->unk3A].unk1A6 = 0;
            temp_v0_13 = &g_field_object_states[arg0->unk3A];
            temp_v0_13->unk1AC = (s32)temp_v0_13->unk50;
            temp_v0_14 = &g_field_object_states[arg0->unk3A];
            temp_v0_14->unk1B0 = (s32)temp_v0_14->unk58;
            var_a1 = 1;
            g_field_object_states[arg0->unk3A].unk1A4 = 1;
            temp_a2 = M2C_FIELD(temp_s3, u8*, 0);
            call_actor->unk2E = 0xFF;
            call_actor->unk24 = 1;
            offset_query = (u16)call_actor->unk2C + 1;
            call_actor->unk2A = temp_a2;
            call_actor->unk2C = offset_query;
            field_restart_actor_animation(call_actor);
            return;
        }
        else
        {
            sp18.x = temp_a1_5;
            sp18.y = arg0->unk4;
            sp18.z = arg0->unk8;
            if (g_field_object_parts[arg0->unk3A].unk2E == 0x40)
            {
                sp18.unkC = 0xC;
                sp18.unk10 = 8;
                sp30.unkC = 0xC;
                sp30.unk10 = 8;
            }
            else
            {
                sp18.unkC = 9;
                sp18.unk10 = 6;
                sp30.unkC = 9;
                sp30.unk10 = 6;
            }
            sp18.unkE = 0x10;
            sp30.unkE = 0x10;
            func_8006304C(&sp18);
            sp30.x = g_field_object_states[arg0->unk3A].unk50;
            sp30.y = g_field_object_states[arg0->unk3A].unk54;
            sp30.z = g_field_object_states[arg0->unk3A].unk58;
            var_s2 = func_80060F58(&sp18, &sp30, (OutPair88198*)((u8*)&g_field_object_states[arg0->unk3A] + 0x1AC), 0);
            call_actor = arg0;
            if (var_s2 <= 0)
            {
                g_field_object_states[arg0->unk3A].unk1A6 = 0;
                temp_v0_16 = &g_field_object_states[arg0->unk3A];
                temp_v0_16->unk1AC = (s32)temp_v0_16->unk50;
                temp_v0_17 = &g_field_object_states[arg0->unk3A];
                temp_v0_17->unk1B0 = (s32)temp_v0_17->unk58;
                var_a1 = 1;
                g_field_object_states[arg0->unk3A].unk1A4 = 1;
                temp_a2 = M2C_FIELD(temp_s3, u8*, 0);
                call_actor->unk2E = 0xFF;
                call_actor->unk24 = 1;
                queryoff2 = call_actor->unk2C;
                queryoff2 += 1;
                call_actor->unk2A = temp_a2;
                call_actor->unk2C = queryoff2;
                field_restart_actor_animation(call_actor);
                return;
            }
            else
            {
                g_field_object_states[arg0->unk3A].unk1A4 = var_s2;
                g_field_object_states[arg0->unk3A].unk1A6 = 0;
                var_a1 = M2C_FIELD(temp_s3, u8*, 0);
                temp_a2 = M2C_FIELD(temp_s3, u8*, 0);
                call_actor->unk2E = 0xFF;
                call_actor->unk24 = 1;
                queryoff3 = (u16)call_actor->unk2C + 1;
                call_actor->unk2A = (s16)var_a1;
                call_actor->unk2C = queryoff3;
                field_restart_actor_animation(call_actor);
                return;
            }
        }

    case 0x89:
    case 0x8A:
    case 0x8B:
    case 0x8C:
    case 0xAC:
        arg0->unk2A = (s16)M2C_FIELD(temp_s3, u8*, 0);
        arg0->unk2E = (u16)M2C_FIELD(temp_s3, u8*, 1);
        arg0->unk2C = (u16)arg0->unk2C + 2;
        arg0->unk24 = 1;
        field_restart_actor_animation(arg0);
        return;
    case 0x9C:
    case 0x9D:
        arg0->unk2A = (s16)M2C_FIELD(temp_s3, u8*, 0);
        arg0->unk2E = 0xFF;
        arg0->unk21 = M2C_FIELD(temp_s3, u8*, 1) + (arg0->unk21 & 0x80);
        arg0->pad20[0] = M2C_FIELD(temp_s3, u8*, 2);
        arg0->pad26[0] = M2C_FIELD(temp_s3, u8*, 3);
        arg0->unk2C = (u16)arg0->unk2C + 4;
        arg0->unk24 = 1;
        field_restart_actor_animation(arg0);
        return;
    case 0x9E:
        case9e_entries = g_field_actor_bindings;
        if ((u8)arg0->unk3A < 2U)
        {
            var_v0_5 = arg0->unk3A * 0x1C;
        }
        else
        {
            var_v0_5 = 0x38;
        }
        if (M2C_FIELD(case9e_entries + var_v0_5, s32*, 0) == 0)
        {
            shared_s1 = M2C_FIELD(temp_s3, u8*, 1) + (M2C_FIELD(temp_s3, u8*, 2) << 8);
            temp_a1_6 = arg0->unk3A;
            arg0->unk2C = (u16)arg0->unk2C + 3;
            case9e_states = g_field_object_states;
            if (case9e_states[temp_a1_6].unkC & 0x400)
            {
                goto block_154;
            }
            if (D_8010AE54 != 0)
            {
                goto block_155;
            }
            if (func_8008404C(temp_a1_6, shared_s1) == 0)
            {
                goto block_155;
            }
            case9e_states[arg0->unk3A].unk174 |= 0x8000;
            return;
        }
        break;
    case 0xA9:
        casea9_check_entries = g_field_actor_bindings;
        if ((u8)arg0->unk3A < 2U)
        {
            var_v0_6 = arg0->unk3A * 0x1C;
        }
        else
        {
            var_v0_6 = 0x38;
        }
        if (M2C_FIELD(casea9_check_entries + var_v0_6, s32*, 0) == 0)
        {
            temp_s0_2 = M2C_FIELD(temp_s3, u8*, 3);
            shared_s1 = M2C_FIELD(temp_s3, u8*, 1) + (M2C_FIELD(temp_s3, u8*, 2) << 8);
            arg0->unk2C = (u16)arg0->unk2C + 4;
            casea9_states = g_field_object_states;
            casea9_states[arg0->unk3A].unk3C = 0xFFFF;
            if (D_8010AE54 != 0)
            {
                goto block_154;
            }
            if (func_8008404C(arg0->unk3A, shared_s1) == 0)
            {
                goto block_155;
            }
            temp_v1_8 = arg0->unk3A;
            casea9_state = &casea9_states[temp_v1_8];
            casea9_slots = g_field_actor_slots;
            casea9_entries = g_field_actor_bindings;
            if (temp_v1_8 < 2U)
            {
                var_v0_7 = temp_v1_8 * 0x1C;
            }
            else
            {
                var_v0_7 = 0x38;
            }
            slot_offset_a9 = (M2C_FIELD(&casea9_entries[var_v0_7], s32*, 0x18)) * (s32)sizeof(FieldActorState);
            ((FieldActorState*)((s32)casea9_slots + slot_offset_a9))->unk26 = temp_s0_2;
            casea9_state->unk16F = temp_s0_2;
            g_field_object_states[arg0->unk3A].unk174 |= 0x8000;
            return;
        }
        break;
    case 0xBC:
        shared_s1 = 2;
        if ((u8)arg0->unk3A < 2U)
        {
            shared_s1 = arg0->unk3A;
        }
        bc_entries = g_field_actor_bindings;
        temp_s2 = (shared_s1 * 0x1C) + bc_entries;
        temp_a0_3 = M2C_FIELD(temp_s2, s32*, 0);
        if (((u32)(temp_a0_3 - 1) < 2U) && (M2C_FIELD(temp_s2, s32*, 0xC) == arg0->unk3A))
        {
            var_s6 = 1;
            if (temp_a0_3 != var_s6)
            {
                bc_slots = g_field_actor_slots;
                temp_v1_9 = &bc_slots[M2C_FIELD(temp_s2, s32*, 0x18)];
                if (temp_v1_9->unk23A == 0)
                {
                    temp_v1_9->unkC = M2C_FIELD(temp_v1_9, FieldActorAnimationDef**, 0x10);
                    g_field_object_states[arg0->unk3A].u.b.pad2 = 0;
                    field_start_actor_animation(M2C_FIELD(temp_s2, s32*, 0x18), 0, 0);
                    g_field_object_states[shared_s1].u.b.pad[1] = (u8)M2C_FIELD(temp_s2, s32*, 0x18);
                    g_field_actor_slots[M2C_FIELD(temp_s2, s32*, 0x18)].unk2A = var_s6;
                    arg0->unk2A = 0xBC;
                }
                goto block_167;
            }
        }
        else
        {
            goto block_167;
        }
        break;
    case 0x9F:
        shared_s1 = 2;
        if ((u8)arg0->unk3A < 2U)
        {
            shared_s1 = arg0->unk3A;
        }
        track_entries = g_field_actor_bindings;
        temp_v1_10 = (shared_s1 * 0x1C) + track_entries;
        temp_a0_4 = M2C_FIELD(temp_v1_10, s32*, 0);
        if (((u32)(temp_a0_4 - 1) < 2U) && (track_owner = arg0->unk3A, temp_a1_7 = M2C_FIELD(temp_v1_10, s32*, 0xC), (temp_a1_7 == track_owner)))
        {
            if (temp_a0_4 != 1)
            {
                track_slots = g_field_actor_slots;
                temp_v1_11 = &track_slots[M2C_FIELD(temp_v1_10, s32*, 0x18)];
                if (temp_v1_11->unk23A == 0)
                {
                    var_s2 = 0;
                    if (M2C_FIELD(M2C_FIELD(temp_v1_11, void**, 0x10), u16*, 0xC) & 0x800)
                    {
                        temp_s0_2 = 0;
                        var_v1_3 = temp_v1_11->unk240;
                    loop_count_tracks:
                    {
                        if (*var_v1_3 != 0)
                        {
                            var_s2 += 1;
                        }
                        temp_s0_2 += 1;
                        var_v1_3 += 1;
                        if (temp_s0_2 < 3)
                        {
                            goto loop_count_tracks;
                        }
                    }
                        track_result = field_start_bound_action_animation(arg0->unk3A, 0, 0, ((var_s2 - 1) << 0xC) | 0x4400);
                    }
                    else
                    {
                        track_result = field_start_bound_action_animation(temp_a1_7, 0, 0, 0);
                    }
                    if (track_result != 0)
                    {
                        arg0->unk2A = 0xBC;
                        g_field_object_states[arg0->unk3A].u.b.pad2 = 0;
                    }
                }
                goto block_140;
            }
        }
        else
        {
        block_140:
            var_v0 = (u16)arg0->unk2C + 2;
            arg0->unk2C = var_v0;
            return;
        }
        break;
    case 0xA0:
    {
        Struct_D800FDF58* actor;
        u16 off;
        u8 value;
        actor = arg0;
        value = M2C_FIELD(temp_s3, u8*, 0);
        actor->unk2E = 0xFA;
        off = actor->unk2C;
        actor->unk2A = value;
        value = M2C_FIELD(temp_s3, u8*, 1);
        off += 2;
        actor->unk2C = off;
        actor->unk24 = 1;
        actor->pad20[0] = value;
        field_restart_actor_animation(actor);
        return;
    }
    case 0xA7:
    case 0xB6:
    {
        Struct_D800FDF58* actor;
        Struct_D80105AE0* states;
        u8 value;
        actor = arg0;
        temp_v1 = M2C_FIELD(temp_s3, u8*, 0);
        actor->unk2E = 0xFA;
        value = temp_v1;
        actor->unk2A = value;
        value = M2C_FIELD(temp_s3, u8*, 1);
        actor->unk1B = value;
        states = g_field_object_states;
        value = M2C_FIELD(temp_s3, u8*, 2);
        actor->pad20[0] = value;
        states[actor->unk3A].unk171 = M2C_FIELD(temp_s3, u8*, 3);
        var_a1 = M2C_FIELD(temp_s3, u8*, 4);
        var_v1 = actor->unk2C;
        actor->unk2E = 0xF0;
        actor->unk24 = 1;
        var_v1 += 5;
        actor->unk21 = var_a1;
        call_actor = actor;
        call_actor->unk2C = var_v1;
        field_restart_actor_animation(call_actor);
        return;
    }
    case 0x82:
        arg0->unk2C = (u16)arg0->unk2C + 1;
        func_8008B870(arg0, 0);
        return;
    case 0x8E:
        arg0->unk2C = (u16)arg0->unk2C + 1;
        func_8008BE38(arg0, 1);
        return;
    case 0x90:
        arg0->unk2C = (u16)arg0->unk2C + 1;
        func_8008C024(arg0, -1);
        return;
    case 0x97:
        temp_s3 += 1;
        arg0->unk2C = (u16)arg0->unk2C + 9;
        sp10.x = M2C_FIELD(temp_s3, u8*, 0) + (M2C_FIELD(temp_s3, u8*, 1) << 8);
        sp10.y = (s16)M2C_FIELD(temp_s3, u8*, 2);
        sp10.w = (s16)M2C_FIELD(temp_s3, u8*, 3);
        sp10.h = (s16)M2C_FIELD(temp_s3, u8*, 4);
        func_8008A4D0(arg0, &sp10, M2C_FIELD(temp_s3, u8*, 5) | (M2C_FIELD(temp_s3, u8*, 6) << 8), M2C_FIELD(temp_s3, u8*, 7));
        return;
    case 0xA1:
        shared_s1 = M2C_FIELD(temp_s3, u8*, 1) + (M2C_FIELD(temp_s3, u8*, 2) << 8);
        arg0->unk2C = (u16)arg0->unk2C + 3;
        if (D_8010AE54 != 0)
        {
            goto block_154;
        }
        if (func_8008404C(arg0->unk3A, shared_s1) == 0)
        {
            goto block_155;
        }
        var_v1_4 = D_8010A020;
        if ((u8)arg0->unk3A < 2U)
        {
            var_v1_4 += arg0->unk3A;
        }
        else
        {
            var_v1_4 += 2;
        }
        *var_v1_4 = 1;
        return;
    block_154:
    block_155:
        arg0->unk28 = 0xFF;
        arg0->unk10 = 0;
        return;
    case 0xA4:
        temp_a1 = 1;
        if ((u8)arg0->unk3A < 2U)
        {
            decoded_a4 = M2C_FIELD(temp_s3, u8*, 0);
            arg0->unk2E = temp_a1;
            arg0->unk2A = decoded_a4;
            a4_old_flags = arg0->unk21;
            a4_command_flags = M2C_FIELD(temp_s3, u8*, 1);
            arg0->unk24 = temp_a1;
            arg0->unk21 = a4_command_flags | (a4_old_flags & 0x80);
            field_restart_actor_animation(arg0);
        }
        arg0->unk2C = (u16)arg0->unk2C + 2;
        return;
    case 0xA5:
        casea5_base = g_field_object_parts;
        temp_v0_19 = &casea5_base[arg0->unk3A];
        temp_v0_19->unk34 |= 0x800000;
        goto block_167;
    case 0xA6:
        casea6_mask = 0xFF7FFFFF;
        casea6_base = g_field_object_parts;
        temp_v0_20 = &casea6_base[arg0->unk3A];
        temp_v0_20->unk34 &= casea6_mask;
        goto block_167;
    case 0xA8:
    {
        Struct_D800FDF58* actor;
        u16 off;
        u8 value;
        actor = arg0;
        actor->unk2A = M2C_FIELD(temp_s3, u8*, 0);
        value = M2C_FIELD(temp_s3, u8*, 2);
        actor->unk2E = value;
        off = actor->unk2C;
        value = M2C_FIELD(temp_s3, u8*, 1);
        off += 3;
        actor->unk2C = off;
        actor->unk27 = 0;
        actor->unk24 = 1;
        actor->unk21 = value;
        field_restart_actor_animation(actor);
        return;
    }
    case 0xB7:
    {
        Struct_D800FDF58* actor;
        u16 off;
        u8 value;
        actor = arg0;
        actor->unk2A = M2C_FIELD(temp_s3, u8*, 0);
        value = M2C_FIELD(temp_s3, u8*, 2);
        actor->unk2E = value;
        value = actor->unk21 = M2C_FIELD(temp_s3, u8*, 1);
        off = actor->unk2C;
        value = M2C_FIELD(temp_s3, u8*, 3);
        off += 4;
        actor->unk2C = off;
        actor->unk27 = 0;
        actor->unk24 = 1;
        actor->pad20[0] = value;
        field_restart_actor_animation_reverse(actor);
        return;
    }
    case 0xB2:
        decoded_b2 = M2C_FIELD(temp_s3, u8*, 0);
        arg0->pad20[0] = 0;
        var_v0 = (u16)arg0->unk2C + 1;
        arg0->unk2A = decoded_b2;
        arg0->unk2C = var_v0;
        return;
    case 0xB4:
        if (arg0->unk25 == 0xFE)
        {
            arg0->unk25 = 0;
        }
        else
        {
            arg0->unk25 = 0xFE;
        }
        goto block_167;
    case 0x0:
    block_167:
        arg0->unk2C++;
    default:
        return;
    }
}


/**
 * @brief Marks the actor record matching @p key as busy and derives its next control state.
 * @param key Value compared against each slot's unk14.
 * @return 0 when a matching slot was found and updated, -1 otherwise.
 */
s32 func_80089980(s32 key)
{
    Struct_D80105AE0* e;
    Struct_D800FDF58* scan;
    Struct_D800FDF58* found;
    s32 i;
    s32 result;
    u8 state;

    scan = g_field_actors;
    e = g_field_object_states;
    i = 0;
loop:
    i++;
    if (e->unk14 == key)
    {
        goto found_label;
    }
    e++;
    scan++;
    if (i < 13)
    {
        goto loop;
    }
    found = (Struct_D800FDF58*)-1;
check:
    if (found != (Struct_D800FDF58*)-1)
    {
        goto body;
    }
    result = -1;
    goto done;
found_label:
    found = scan;
    goto check;
body:
    found->unk2A = 0xB2;
    state = found->unk21;
    if (state >= 0x8F)
    {
        goto set_default;
    }
    switch (state)
    {
    case 1:
    case 6:
    case 11:
        found->pad20[0] = 6;
        break;
    case 2:
    case 7:
    case 12:
        found->pad20[0] = 0xC;
        break;
    case 3:
    case 8:
    case 13:
        found->pad20[0] = 0x12;
        break;
    case 4:
    case 9:
    case 14:
    case 132:
    case 137:
    case 142:
        found->pad20[0] = 0x18;
        break;
    case 131:
    case 136:
    case 141:
        found->pad20[0] = 0x1E;
        break;
    case 130:
    case 135:
    case 140:
        found->pad20[0] = 0x24;
        break;
    case 129:
    case 134:
    case 139:
        found->pad20[0] = 0x2A;
        break;
    case 0:
        set_default:
        found->pad20[0] = 0;
        break;
    }
    result = 0;
done:
    return result;
}
/* Key-based actor action requests, interaction parameters, and action-state changes. */

/* field308 */
#include "common.h"

typedef struct
{
    u8 pad0[0x14];
    s32 unk14; /* 0x14 */
    u8 pad18[0x23C - 0x18];
} RecA80105AE0;

typedef struct
{
    u8 pad0[0x25];
    u8 unk25; /* 0x25 */
    u8 pad26[0x54 - 0x26];
} RecB800FDF58;




/**
 * @see decomp.me (100%)
 */
s32 func_80089A68(s32 arg0)
{
    RecA80105AE0 *ra;
    RecB800FDF58 *rb;
    RecB800FDF58 *found;
    s32 i;

    rb = ((RecB800FDF58 *)g_field_actors);
    ra = ((RecA80105AE0 *)g_field_object_states);
    for (i = 0; i < 0xD; i++, ra++, rb++)
    {
        if (ra->unk14 == arg0)
        {
            goto found_it;
        }
    }
    found = (RecB800FDF58 *) -1;
check:
    if (found != (RecB800FDF58 *) -1)
    {
        goto body;
    }
    return -1;
found_it:
    found = rb;
    goto check;
body:
    if (found->unk25 == 0xFE)
    {
        found->unk25 = 0;
    }
    else
    {
        found->unk25 = 0xFE;
    }
    return 0;
}


/* func_80089AE4 */
#include "common.h"

typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x224];
} StateB80089AE4;

typedef struct
{
    u8 pad0[0x10];
    s16 unk10;
    u8 pad12[0xA];
    s32 unk1C;
    u8 pad20[8];
    u8 unk28;
    u8 pad29[1];
    s16 unk2A;
    u8 pad2C[0xE];
    u8 unk3A;
    u8 pad3B[2];
    u8 unk3D;
    u8 pad3E[0x16];
} RecordB80089AE4;




s32 func_800839F8();

/**
 * @brief Find a matching field actor slot and update its animation state.
 * @param arg0 Field-state key to locate.
 * @param arg1 Animation argument, or -1 to skip animation dispatch.
 * @return Zero when a matching slot is updated, or -1 when no slot matches.
 */
s32 func_80089AE4(s32 arg0, s32 arg1)
{
    StateB80089AE4 *ra;
    RecordB80089AE4 *rb;
    s32 anim_id;
    RecordB80089AE4 *found;
    s32 i;

    rb = ((RecordB80089AE4 *)g_field_actors);
    ra = ((StateB80089AE4 *)g_field_object_states);
    for (i = 0; i < 0xD; i++, ra++, rb++)
    {
        if (ra->unk14 == arg0)
        {
            goto found_it;
        }
    }
    found = (RecordB80089AE4 *)-1;
check:
    do
    {
        if (found != (RecordB80089AE4 *)-1)
        {
            goto body;
        }
    } while (0);
    return -1;
found_it:
    found = rb;
    goto check;
body:
    if (arg1 != -1)
    {
        anim_id = func_800839F8(found->unk3A, 0, rb);
        if ((anim_id != -1) && (func_80083EEC(found->unk3A, anim_id, arg1) != 0))
        {
            field_start_actor_animation(anim_id, 0, 0);
        }
    }

    found->unk2A = 0xBB;
    found->unk3D = 2;
    found->unk10 = 1;
    found->unk28 = 0;
    found->unk1C = (found->unk1C & ~0x1FF) | 2;
    return 0;
}


/* func_80089BE8 */
#include "common.h"

/** @brief Per-actor slot in ((SlotA *)g_field_object_states); stride 0x23C. */
typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x224];
} SlotA;

/** @brief Per-actor record in ((EntryB *)g_field_actors); stride 0x54. */
typedef struct
{
    u8 pad0[0x3A];
    u8 unk3A;
    u8 pad3B[0x19];
} EntryB;

/** @brief Record in the g_field_player_records table indexed by EntryB::unk3A; stride 0x268. */
typedef struct
{
    u8 pad0[0x25E];
    s16 unk25E;
    s16 unk260;
    s16 unk262;
    s16 unk264;
    s16 unk266;
} RecFD818;





/**
 * @brief Update the indexed field record associated with an actor lookup key.
 * @param arg0 Actor lookup key.
 * @param arg1 Value written to the record's first configurable field.
 * @param arg2 Value written to the record's second configurable field.
 * @param arg3 Value written to the record's third configurable field.
 * @param arg4 Value written to the record's fourth configurable field.
 * @return Zero on success, or -1 when the actor or target record is unavailable.
 */
s32 func_80089BE8(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4)
{
    EntryB *scan;
    EntryB *found;
    SlotA *e;
    s32 i;

    scan = ((EntryB *)g_field_actors);
    e = ((SlotA *)g_field_object_states);
    i = 0;
loop:
    i++;
    if (e->unk14 == arg0)
    {
        goto found_label;
    }
    e++;
    scan++;
    if (i < 13)
    {
        goto loop;
    }
    found = (EntryB *)-1;
check:
    if (found == (EntryB *)-1)
    {
        return -1;
    }
    goto success;

found_label:
    found = scan;
    goto check;

success:
    if (found->unk3A >= 3)
    {
        return -1;
    }
    ((RecFD818 *)g_field_player_records)[found->unk3A].unk260 = arg4;
    ((RecFD818 *)g_field_player_records)[found->unk3A].unk25E = 0;
    ((RecFD818 *)g_field_player_records)[found->unk3A].unk262 = arg1;
    ((RecFD818 *)g_field_player_records)[found->unk3A].unk264 = arg2;
    ((RecFD818 *)g_field_player_records)[found->unk3A].unk266 = arg3;
    return 0;
}


/* func_80089D44 */
#include "common.h"

/**
 * @brief Actor record fields used by state reset and companion repositioning.
 */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u8 padC[4];
    s16 unk10;
    u8 pad12[0x21 - 0x12];
    u8 unk21;
    u8 pad22[2];
    u8 unk24;
    u8 unk25;
    u8 pad26;
    u8 unk27;
    u8 pad28[2];
    s16 unk2A;
    u8 pad2C[2];
    s16 unk2E;
    u8 pad30[0xA];
    u8 unk3A;
    u8 pad3B[0x54 - 0x3B];
} FieldRecord;

/**
 * @brief Runtime actor state with packed resource and animation fields.
 */
typedef struct
{
    u32 unk0;
    u32 unk4;
    u32 unk8;
    s32 unkC;
    u8 pad10[4];
    s32 unk14;
    u8 pad18[0x174 - 0x18];
    u32 unk174;
    /** @brief Word and byte views of the packed animation state. */
    union

    {
        u32 word;
        struct

        {
            u8 byte0;
            u8 animation;
            u8 byte2;
            u8 byte3;
        } bytes;
    } flags;
    u8 pad17C[0x1AB - 0x17C];
    u8 unk1AB;
    u8 pad1AC[0x23C - 0x1AC];
} FieldState;

/** @brief Camera displacement words used by the visible-area bounds checks. */
typedef struct
{
    s32 unused0;
    s32 x;
    s32 unused8;
    s32 z;
} CameraPosition;



s32 func_800839F8();
void func_800B48B8(s32);
void func_8008A0B0();

/**
 * @brief Reset a matching actor's state, animation resources and companion position.
 * @param key Runtime actor key to locate among thirteen slots.
 * @param requested_state New low state bits, or -1 to use state zero.
 * @param animation Animation request, or -1 to leave animation selection unchanged.
 * @param event_id Event to dispatch at value 0x80, or -1 to skip dispatch.
 * @return Zero on success, or -1 when no actor matches the key.
 * @note Keep repeated slot-index reads and the signed scan sentinel for matching.
 * @note GCC 2.7.2 CDK matches all 219 instructions (876 bytes).
 */
s32 func_80089D44(s32 key, s32 requested_state, s32 animation, s32 event_id)
{
    FieldRecord *scan_record;
    FieldState *scan_state;
    FieldState *lookup_state;
    FieldRecord *record;
    FieldRecord *candidate;
    s32 state;
    s32 slot;
    s32 i;
    s32 unavailable;
    s32 animation_slot;
    CameraPosition *camera;
    FieldState *runtime_flags;
    FieldState *resources;
    FieldState *runtime_mode;

    camera = (CameraPosition *)0x801ED480;
    state = requested_state;
    scan_record = ((FieldRecord *)g_field_actors);
    lookup_state = ((FieldState *)g_field_object_states);
    for (i = 0; i < 0xD; i++, lookup_state++, scan_record++)

    {
        if (lookup_state->unk14 == key)

        {
            goto found_it;
        }
    }
    record = (FieldRecord *)-1;
check:
    if (record != (FieldRecord *)-1)

    {
        goto body;
    }
    return (s32)record;
found_it:
    record = scan_record;
    goto check;
body:
    record->unk25 = 0;
    record->unk2A = 0;
    record->unk10 = 0;
    record->unk2E = 1;
    if (state == -1)
    {
        state = 0;
    }
    record->unk27 = 0;
    record->unk24 = 1;
    record->unk21 = (u8) ((record->unk21 & 0x80) | state);
    field_restart_actor_animation(record);
    if (animation != -1)
    {
        animation_slot = func_800839F8(record->unk3A, 0);
        if ((animation_slot != -1) && (func_80083EEC(record->unk3A, animation_slot, animation) != 0))
        {
            field_start_actor_animation(animation_slot, 0, 0);
            ((FieldState *)g_field_object_states)[record->unk3A].flags.bytes.animation = animation_slot;
        }
    }
    if (event_id != -1)
    {
        func_800A3938(event_id, 0x80);
    }
    func_800B48B8(((FieldState *)g_field_object_states)[record->unk3A].unk14);
    ((FieldState *)g_field_object_states)[record->unk3A].unkC = 0;
    runtime_flags = &((FieldState *)g_field_object_states)[record->unk3A];
    runtime_flags->flags.word = (s32) (runtime_flags->flags.word & ~0x20);
    resources = &((FieldState *)g_field_object_states)[record->unk3A];
    resources->unk8 = (s32) ((resources->unk8 & 0xFF000000) | (resources->unk0 & 0xFFFFFF));
    resources->unk4 = (s32) (resources->unk0 & 0xFFFFFF);
    ((FieldState *)g_field_object_states)[record->unk3A].unk1AB = 0x3C;
    runtime_mode = &((FieldState *)g_field_object_states)[record->unk3A];
    runtime_mode->unk174 = (s32) (runtime_mode->unk174 | 0x8000);
    if (record->unk3A < 3U &&
        (record->unk0 <= -camera->x + 0xA00 ||
         record->unk0 >= -camera->x + 0x13600 ||
         record->unk8 <= -camera->z + 0xA00 ||
         record->unk8 >= -camera->z + 0x1B600))
         {
        slot = 0;
        unavailable = 0xFF;
        scan_state = ((FieldState *)g_field_object_states);
        candidate = ((FieldRecord *)g_field_actors);
        for (; slot < 3; scan_state++, slot++, candidate++)

        {
            if (candidate->unk25 != unavailable && scan_state->unk4 != 0 && record->unk3A != slot)

            {
                break;
            }
        }
        if (slot != 3)
        {
            func_8008A0B0(record, slot, 1);
        }
    }
    return 0;
}


/* func_8008A0B0 */
#include "common.h"

/** @brief Field actor record with position, animation state, and runtime index. */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u8 padc[0x18];
    u8 unk24;
    u8 pad25[5];
    s16 unk2a;
    u8 pad2c[2];
    s16 unk2e;
    u8 pad30[10];
    u8 unk3a;
    u8 pad3b[0x19];
} CommandView5_FieldRecord;

/** @brief Runtime actor state containing position and collision path fields. */
typedef struct
{
    u8 pad0[0x50];
    s32 unk50;
    s32 unk54;
    s32 unk58;
    u8 pad5c[0x1A4 - 0x5C];
    s16 unk1a4;
    s16 unk1a6;
    u8 pad1a8[4];
    s32 unk1ac;
    s32 unk1b0;
    u8 pad1b4[0x23C - 0x1B4];
} CommandView5_FieldState;

/** @brief Position view at runtime state offset 0x50, with the 0x23C-byte stride. */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u8 padc[0x23C - 12];
} StatePosition;

/** @brief Actor part definition view used to select the collision footprint. */
typedef struct
{
    u8 pad0[0x2E];
    u8 unk2e;
    u8 pad2f[0x48 - 0x2F];
} PartDef;

/** @brief Map width and depth fields at the fixed map-header address. */
typedef struct
{
    s16 unk0;
    u16 unk2;
} MapBounds;

/** @brief Collision probe position, horizontal footprint, and height tolerance. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
    u16 width;
    s16 height;
    u16 depth;
} CollisionQuery;




extern StatePosition D_80105B30[];
s32 func_80060F58(CollisionQuery *, CollisionQuery *, void *, s32);
void func_8006304C(CollisionQuery *);

/**
 * @brief Compute a path from an actor to another record, with direct fallback.
 * @param record Actor record whose runtime path should be updated.
 * @param source_index Record supplying the destination coordinates.
 * @param update_animation Nonzero to reset and apply the actor animation.
 */
void func_8008A0B0(CommandView5_FieldRecord *record, s32 source_index, s32 update_animation)
{
    CollisionQuery start;
    CollisionQuery goal;
    s32 path_length;
    MapBounds *bounds;
    s32 x;
    s32 map_depth;
    s32 map_width;
    s32 state_x;
    s32 state_z;
    s32 z;
    CommandView5_FieldRecord *source;

    bounds = (MapBounds *)0x801ED400;
    x = record->unk0;
    if ((x < 0) ||
        (map_width = bounds->unk0 << 8, ((x < map_width) == 0)) ||
        (z = record->unk8, (z < 0)) ||
        (map_depth = (s32) (bounds->unk2 << 0x10) >> 7, ((z < map_depth) == 0)) ||
        (state_x = ((CommandView5_FieldState *)g_field_object_states)[record->unk3a].unk50, (state_x < 0)) ||
        (state_x >= map_width) ||
        (state_z = ((CommandView5_FieldState *)g_field_object_states)[record->unk3a].unk58, (state_z < 0)) ||
        (state_z >= map_depth))
    {
        (&((CommandView5_FieldState *)g_field_object_states)[record->unk3a])->unk1a6 = 0;
        (&((CommandView5_FieldState *)g_field_object_states)[record->unk3a])->unk1ac = ((CommandView5_FieldRecord *)g_field_actors)[source_index].unk0;
        (&((CommandView5_FieldState *)g_field_object_states)[record->unk3a])->unk1b0 = ((CommandView5_FieldRecord *)g_field_actors)[source_index].unk8;
        ((CommandView5_FieldState *)g_field_object_states)[record->unk3a].unk1a4 = 1;
    }
    else
    {
        start.x = x;
        start.y = record->unk4;
        start.z = record->unk8;
        if ((&((PartDef *)g_field_object_parts)[record->unk3a])->unk2e == 0x40)
        {
            start.width = 0xC;
            start.depth = 8;
            goal.width = 0xC;
            goal.depth = 8;
        }
        else
        {
            start.width = 9;
            start.depth = 6;
            goal.width = 9;
            goal.depth = 6;
        }
        start.height = 0x10;
        goal.height = 0x10;
        func_8006304C(&start);
        D_80105B30[record->unk3a].unk0 = ((CommandView5_FieldRecord *)g_field_actors)[source_index].unk0;
        source = &((CommandView5_FieldRecord *)g_field_actors)[source_index];
        (&D_80105B30[record->unk3a])->unk4 = (s32) source->unk4;
        (&D_80105B30[record->unk3a])->unk8 = (s32) source->unk8;
        goal.x = source->unk0;
        goal.y = source->unk4;
        goal.z = source->unk8;
        path_length = func_80060F58(&start, &goal, (u8 *)&D_80105B30[record->unk3a] + 0x15C, 0);
        if (path_length <= 0)
        {
            (&((CommandView5_FieldState *)((u8 *)D_80105B30 - 0x50))[record->unk3a])->unk1a6 = 0;
            (&((CommandView5_FieldState *)((u8 *)D_80105B30 - 0x50))[record->unk3a])->unk1ac = (s32) source->unk0;
            (&((CommandView5_FieldState *)((u8 *)D_80105B30 - 0x50))[record->unk3a])->unk1b0 = (s32) source->unk8;
            ((CommandView5_FieldState *)((u8 *)D_80105B30 - 0x50))[record->unk3a].unk1a4 = 1;
        }
        else
        {
            (&((CommandView5_FieldState *)((u8 *)D_80105B30 - 0x50))[record->unk3a])->unk1a4 = path_length;
            (&((CommandView5_FieldState *)((u8 *)D_80105B30 - 0x50))[record->unk3a])->unk1a6 = 0;
        }
    }
    if (update_animation != 0)
    {
        record->unk2a = 0xB5;
        record->unk2e = 0xFF;
        record->unk24 = 1;
        field_restart_actor_animation(record);
    }
}


/* func_8008A4D0 */
#include "common.h"

typedef struct {
    u8 pad0[0xC];
    s32 unkC;
} FieldA4D0State;

typedef struct {
    u16 x;
    u16 y;
    u16 w;
    u16 h;
} FieldA4D0Rect;

void func_8008A4D0(FieldA4D0State *arg0, FieldA4D0Rect *rect, s32 arg2, s32 arg3)
{
    s32 xoff;
    s32 yoff;
    s32 mode;

    mode = arg0->unkC;
    if (mode >= 2)
    {
        yoff = 0;
        if (mode >= 9)
        {
            yoff = 0x100;
            xoff = 0x3C0 - ((mode - 9) << 6);
        }
        else
        {
            xoff = 0x340 - (mode << 6);
        }
    }
    else
    {
        yoff = 0;
        xoff = 0x380 - (mode << 7);
    }

    rect->x = (s16)rect->x >> 2;
    rect->x += xoff;
    rect->w = (s16)rect->w >> 2;
    rect->y += yoff;
    MoveImage2(rect, (arg2 >> 2) + xoff, arg3 + yoff);
}


/* func_8008A580 */
#include "common.h"

/** @brief Per-actor animation/geometry slot; array element stride 0x23C. */
typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x3C - 0x18];
    u32 unk3C;
    u8 pad40[0x23C - 0x40];
} FieldActorSlot;

/** @brief Parallel per-actor record; array element stride 0x54. */
typedef struct
{
    u8 pad0[0x3A];
    u8 unk3A;
    u8 pad3B[0x54 - 0x3B];
} FieldActorRecord;



extern s32 D_8010A020[];

s32 func_8008404C();

/**
 * @brief Find an actor by key and request its slot update.
 * @param key Actor-slot lookup key.
 * @param arg1 Value forwarded to the actor update helper; meaning unknown.
 * @return -1 when absent, 1 when the helper returns zero, or 0 after marking the slot.
 * @see decomp.me (100%) TODO
 */
s32 func_8008A580(s32 key, s32 arg1)
{
    FieldActorRecord *scan;
    FieldActorRecord *found;
    FieldActorSlot *e;
    s32 i;
    s32 *slot;
    s32 *table;

    scan = ((FieldActorRecord *)g_field_actors);
    e = ((FieldActorSlot *)g_field_object_states);
    i = 0;
loop:
    i++;
    if (e->unk14 == key)
    {
        goto found_label;
    }
    e++;
    scan++;
    if (i < 13)
    {
        goto loop;
    }
    found = (FieldActorRecord *)-1;
check:
    if (found == (FieldActorRecord *)-1)
    {
        return (s32)found;
    }
    goto body;
found_label:
    found = scan;
    goto check;
body:
    if (func_8008404C(found->unk3A, arg1) != 0)
    {
        table = D_8010A020;
        if (found->unk3A < 2)
        {
            slot = &table[found->unk3A];
        }
        else
        {
            slot = table + 2;
        }
        *slot = 1;
        ((FieldActorSlot *)g_field_object_states)[found->unk3A].unk3C = 0xFFFF;
    }
    else
    {
        return 1;
    }
    return 0;
}


/* func_8008A678 */
#include "common.h"

/** @brief Field state prefix and queued interaction targets in a 0x23C-byte slot. */
typedef struct
{
    u8 pad0[4];
    s32 active;
    u8 pad8[0x14 - 8];
    s32 actor;
    u8 pad18[0x16F - 0x18];
    u8 action;
    u8 pad170[8];
    union
    {
        s32 flags;
        struct
        {
            u8 pad[3];
            u8 count;
        } bytes;
    } state;
    u8 pad17c[4];
    u8 targets[0x23C - 0x180];
} CommandView8_FieldState;

/** @brief Seven-word interaction request consumed by func_800B5534. */
typedef struct
{
    s32 actor;
    s32 action;
    s32 param;
    s32 target;
    s32 unk10;
    s32 unk14;
    s32 mode;
} Request;


s32 func_800B5534();

/**
 * @brief Process queued actor interactions and clear the source queue count.
 * @param index Source field-state slot index.
 * @note Preserve the loop-carried stride and address casts for matching codegen.
 */
void func_8008A678(s32 index)
{
    Request request;
    CommandView8_FieldState *base;
    CommandView8_FieldState *loop_base;
    CommandView8_FieldState *source;
    CommandView8_FieldState *saved_source;
    CommandView8_FieldState *current;
    CommandView8_FieldState *target;
    CommandView8_FieldState *first_target;
    s32 i;
    s32 stride8;

    base = ((CommandView8_FieldState *)g_field_object_states);
    stride8 = index * 8;
    source = (CommandView8_FieldState *)((u8 *)base + ((stride8 + index) * 16 - index) * 4);
    i = 0;
    if (source->state.bytes.count != 0)
    {
        /* Keep separate copies for values that must survive the request call. */
        loop_base = base;
        saved_source = source;
        stride8 = index * 8;
        do
        {
            /* Integer address arithmetic preserves the target operand order. */
            current = (CommandView8_FieldState *)((((stride8 + index) * 16 - index) * 4) + (u32)loop_base);
            first_target = (CommandView8_FieldState *)(current->targets[i] * 0x23C + (u32)loop_base);
            first_target->state.flags &= ~0x80;
            target = (CommandView8_FieldState *)(current->targets[i] * 0x23C + (u32)loop_base);
            if (!(((u32)target->state.flags >> 5) & 1))
            {
                stride8 = index * 8;
                if (target->active != 0)
                {
                    request.actor = current->actor;
                    if ((u8)current->action < 0xB)
                    {
                        request.action = current->action;
                    }
                    else
                    {
                        request.action = 0xA;
                    }
                    request.target = loop_base[saved_source->targets[i]].actor;
                    request.param = 0;
                    request.unk10 = 0;
                    request.unk14 = 0;
                    request.mode = 1;
                    func_800B5534(&request);
                    goto stride_update;
                }
            }
            else
            {
stride_update:
                stride8 = index * 8;
            }
            i++;
        } while (i < ((CommandView8_FieldState *)((u8 *)loop_base + (((stride8 + index) * 16 - index) * 4)))->state.bytes.count);
    }
    ((CommandView8_FieldState *)g_field_object_states)[index].state.bytes.count = 0;
}


/* func_8008A840 */
#include "common.h"

typedef struct
{
    u8 pad0[0x2A];
    s16 unk2A;
    u8 pad2C[0x30 - 0x2C];
    u16 unk30;
    u8 pad32[0x54 - 0x32];
} Rec54;

typedef struct
{
    u8 pad0[4];
    s32 unk4;
    u8 pad8[0x14 - 8];
    s32 unk14;
    u8 pad18[0x16F - 0x18];
    u8 unk16F;
    u8 pad170[0x178 - 0x170];
    s32 unk178;
    u8 pad17C[0x23C - 0x17C];
} State23C;

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
    s32 unk10;
    s32 unk14;
    s32 unk18;
} ArgBlock;




void func_8008C4A8();
s32 func_800B5534();

/**
 * @brief Configure an interaction request between two field-state slots.
 * @param arg0 Source slot index.
 * @param arg1 Destination slot index.
 * @return Result returned by func_800B5534, or zero when the request cannot be started.
 */
s32 func_8008A840(s32 arg0, s32 arg1)
{
    ArgBlock arg_block;
    State23C *slot;
    State23C *other;
    State23C *base;
    s32 mask;

    if ((((Rec54 *)g_field_actors)[arg0].unk2A == 0x91) || (((Rec54 *)g_field_actors)[arg0].unk2A == 0x87))
    {
        return 0;
    }
    base = ((State23C *)g_field_object_states);
    other = &base[arg1];
    other->unk178 = other->unk178 & ~0x80;
    if (other->unk4 == 0)
    {
        return 0;
    }
    slot = &base[arg0];
    arg_block.unk0 = slot->unk14;
    if ((u8)slot->unk16F < 0xB)
    {
        arg_block.unk4 = (s32)slot->unk16F;
    }
    else
    {
        arg_block.unk4 = 0;
    }
    arg_block.unkC = ((State23C *)g_field_object_states)[arg1].unk14;
    if (arg0 < 2)
    {
        arg_block.unk8 = (s32)((Rec54 *)g_field_actors)[arg0].unk30;
    }
    else
    {
        arg_block.unk8 = 0;
    }
    arg_block.unk10 = 0;
    arg_block.unk14 = 0;
    mask = ((u32)((State23C *)g_field_object_states)[arg0].unk178 >> 2) & 7;
    if (mask == 0)
    {
        mask = 1;
    }
    arg_block.unk18 = mask;
    func_8008C4A8(arg0);
    return func_800B5534(&arg_block);
}


/* func_8008A9D8 */
#include "common.h"

typedef struct
{
    u8 pad0[0x2A];
    s16 unk2A;
    u8 pad2C[0x54 - 0x2C];
} CommandView10_Rec54;

typedef struct
{
    u8 pad0[4];
    s32 unk4;
    u8 pad8[0x14 - 0x8];
    s32 unk14;
    u8 pad18[0x178 - 0x18];
    s32 unk178;
    u8 pad17C[0x23C - 0x17C];
} CommandView10_State23C;

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
    s32 unk10;
    s32 unk14;
    s32 unk18;
} CommandView10_ArgBlock;




/**
 * @brief Update an actor slot and dispatch the associated field action.
 * @param arg0 Source actor index.
 * @param arg1 Destination slot index.
 * @param arg2 Action argument.
 * @return Result from the dispatched action, or 0 when no action is performed.
 */
s32 func_8008A9D8(s32 arg0, s32 arg1, s32 arg2)
{
    CommandView10_State23C *base;
    CommandView10_State23C *slot;
    CommandView10_ArgBlock arg_block;

    if ((((CommandView10_Rec54 *)g_field_actors)[arg0].unk2A != 0x91) && (((CommandView10_Rec54 *)g_field_actors)[arg0].unk2A != 0x87))
    {
        base = ((CommandView10_State23C *)g_field_object_states);
        slot = &base[arg1];
        slot->unk178 = slot->unk178 & ~0x80;
        if (slot->unk4 != 0)
        {
            arg_block.unk0 = base[arg0].unk14;
            arg_block.unk4 = arg2;
            arg_block.unkC = slot->unk14;
            arg_block.unk8 = 0;
            arg_block.unk10 = 0;
            arg_block.unk14 = 0;
            arg_block.unk18 = 1;
            func_8008C4A8(arg0, slot, arg2);
            return func_800B5534(&arg_block);
        }
    }
    return 0;
}


/* func_8008AABC */
#include "common.h"

/**
 * @brief Per-actor animation/geometry slot; array element stride 0x23C.
 */
typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x23C - 0x18];
} Blk8008AABC_Struct_D80105AE0;

/**
 * @brief Argument block passed by pointer to func_800B5F60.
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
    s32 unk1C;
} ArgB5F60;


void func_800B5F60(ArgB5F60 *);

/**
 * @brief Builds a two-actor argument block and dispatches func_800B5F60.
 *
 * Copies the 0x14 field of actors @p a and @p b into a stack argument block,
 * marks it active (unk18 = 1), and passes it to func_800B5F60.
 */
void func_8008AABC(s32 a, s32 b)
{
    ArgB5F60 s;

    s.unk0 = ((Blk8008AABC_Struct_D80105AE0 *)g_field_object_states)[a].unk14;
    s.unkC = ((Blk8008AABC_Struct_D80105AE0 *)g_field_object_states)[b].unk14;
    s.unk18 = 1;
    func_800B5F60(&s);
}


/* func_8008AB2C */
#include "common.h"

/** @brief Field entry with animation state and actor index. */
typedef struct
{
    u8 pad0[0x2A];
    s16 unk2A;
    u8 pad2C[14];
    u8 unk3A;
    u8 pad3B[0x54 - 0x3B];
} Entry;
/** @brief Actor record with lookup ID, state flags, and action byte. */
typedef struct
{
    s32 unk0, unk4, unk8, unkC, unk10, unk14;
    u8 pad18[0x16F - 0x18];
    u8 unk16F;
    u8 pad170[8];
    s32 unk178;
    u8 pad17C[0x23C - 0x17C];
} Actor;
/** @brief Actor data record containing the pending action byte. */
typedef struct
{
    u8 pad0[0x259];
    u8 unk259;
    u8 pad25A[14];
} Data;
/** @brief Input context view containing the actor-zero event counter. */
typedef struct
{
    u8 pad0[0x3154];
    s32 unk3154;
} Pad;



extern Pad *g_pad_ctx;
extern s32 g_field_boss_hud_shake_frame;
/**
 * @brief Mark an actor for an action and dispatch its state-dependent update.
 * @param arg0 Actor ID to locate among the field records.
 * @param arg1 Reaction-mode selector forwarded to func_8008B870.
 * @return Zero when found, or -1 when no record has the requested ID.
 */
s32 func_8008AB2C(s32 arg0, s32 arg1)
{
    Entry *entry_cursor;
    Actor *actor_cursor;
    Entry *entry;
    s16 action;
    s32 pad_counter;
    s32 entry_count;
    Actor *actor;
    u8 *actor_base;

    entry_cursor = ((Entry *)g_field_actors);
    actor_cursor = ((Actor *)g_field_object_states);
    entry_count = 0;
loop_1:
    entry_count += 1;
    if (actor_cursor->unk14 != arg0)
    {
        actor_cursor++;
        entry_cursor++;
        if (entry_count >= 0xD)
        {
            entry = (Entry *)-1;
        }
        else
        {
            goto loop_1;
        }
    }
    else
    {
        goto found;
    }
check:
    if (entry != (Entry *)-1)
    {
        goto body;
    }
    return -1;
found:
    entry = entry_cursor;
    goto check;
body:
    if (entry->unk3A == 0)
    {
        pad_counter = g_pad_ctx->unk3154;
        if (pad_counter != -1)
        {
            g_pad_ctx->unk3154 = (s32)(pad_counter + 1);
        }
    }
    actor_base = (u8 *)((Actor *)g_field_object_states);
    actor = (Actor *)(actor_base + entry->unk3A * 0x23C);
    actor->unkC = (s32)(actor->unkC | 0x10000000);
    if ((u8)entry->unk3A < 3U)
    {
        ((Data *)g_field_player_records)[entry->unk3A].unk259 = 5;
    }
    else if (((Actor *)(actor_base + entry->unk3A * 0x23C))->unk8 < 0)
    {
        g_field_boss_hud_shake_frame = 5;
    }
    action = entry->unk2A;
    if (action == 0x87)
    {
        return 0;
    }
    if (action >= 0x88)
    {
        if (action != 0x91)
        {
            func_8008B870(entry, arg1);
        }
        return 0;
    }
    if (action >= 0x85)
    {
        switch (((Actor *)((u8 *)((Actor *)g_field_object_states) + entry->unk3A * 0x23C))->unk16F)
        {
        case 0:
        case 1:
        case 2:
        case 3:
        case 8:
        case 9:
        case 10:
            break;
        default:
            if (!(((u32)((Actor *)((u8 *)((Actor *)g_field_object_states) + entry->unk3A * 0x23C))->unk178 >> 6) & 1))
            {
                return 0;
            }
            break;
        }
    }
    func_8008B870(entry, arg1);
    return 0;
}
/* Key-based animation commands, actor slot queries, facing changes, and pending animation restarts. */

/* func_8008AD44 */
#include "common.h"

/**
 * @brief Per-actor animation/geometry slot; array element stride 0x23C.
 */
typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x23C - 0x18];
} Blk8008AD44_Struct_D80105AE0;

/**
 * @brief Parallel per-actor record; array element stride 0x54.
 */
typedef struct
{
    u8 data[0x54];
} Blk8008AD44_Struct_D800FDF58;




/**
 * @brief Finds the actor slot matching @p key and activates its record.
 *
 * Scans the first 13 g_field_object_states slots for one whose 0x14 field equals @p key.
 * On a hit, activates the parallel g_field_actors record (func_8008BE38 with 1) and
 * returns 0; if no slot matches, returns -1.
 */
s32 func_8008AD44(s32 key)
{
    Blk8008AD44_Struct_D800FDF58 *p = ((Blk8008AD44_Struct_D800FDF58 *)g_field_actors);
    Blk8008AD44_Struct_D80105AE0 *e = ((Blk8008AD44_Struct_D80105AE0 *)g_field_object_states);
    s32 i;

    i = 0;
    while (i < 13)
    {
        if (e->unk14 == key)
        {
            goto found;
        }
        i++;
        e++;
        p++;
    }
    p = (Blk8008AD44_Struct_D800FDF58 *)-1;
found:
    if (p == (Blk8008AD44_Struct_D800FDF58 *)-1)
    {
        return -1;
    }
    func_8008BE38(p, 1);
    return 0;
}


/* field254 */
#include "common.h"

typedef struct
{
    u8 pad0[0x14];
    s32 unk14; /* 0x14 */
    u8 pad18[0x23C - 0x18];
} Blk8008ADB4_RecA80105AE0;

typedef struct
{
    u8 pad0[0x21];
    u8 unk21; /* 0x21 */
    u8 pad22[0x54 - 0x22];
} Blk8008ADB4_RecB800FDF58;




s32 func_8008ADB4(s32 arg0)
{
    Blk8008ADB4_RecA80105AE0 *ra;
    Blk8008ADB4_RecB800FDF58 *rb;
    Blk8008ADB4_RecB800FDF58 *found;
    s32 i;

    rb = ((Blk8008ADB4_RecB800FDF58 *)g_field_actors);
    ra = ((Blk8008ADB4_RecA80105AE0 *)g_field_object_states);
    for (i = 0; i < 0xD; i++, ra++, rb++)
    {
        if (ra->unk14 == arg0)
        {
            goto found_it;
        }
    }
    found = (Blk8008ADB4_RecB800FDF58 *) -1;
check:
    if (found != (Blk8008ADB4_RecB800FDF58 *) -1)
    {
        return found->unk21 & 0x7F;
    }
    return -1;
found_it:
    found = rb;
    goto check;
}


/* field_queue_actor_animation_by_handle */
/**
 * @file field_queue_actor_animation_by_handle.c
 * @brief Resolve a runtime field-actor handle and queue an animation request.
 */

#include "common.h"

/** @brief Field object record paired with a runtime actor slot. */
typedef struct
{
    u8 _pad000[0x3A];
    u8 object_index;
    u8 _pad03B[0x54 - 0x3B];
} FieldObjectRecord;

/** @brief Runtime field-actor slot used to resolve actor handles. */
typedef struct
{
    u8 _pad000[0x14];
    s32 handle;
    u8 _pad018[0x23C - 0x18];
} Blk8008AE14_FieldActorSlot;

/** @brief Partial view of the shared context referenced by g_pad_ctx. */
typedef struct
{
    u8 _pad000[0x3158];
    s32 unk3158;
} FieldPadContext;






/**
 * @brief Queue an animation for the field actor identified by a runtime handle.
 * @param actor_handle Handle to locate in the first 13 runtime actor slots.
 * @param animation_id Animation identifier forwarded to the actor update path.
 * @return 0 when the actor is found and updated, or -1 when no actor matches.
 */
s32 func_8008AE14(s32 actor_handle, s32 animation_id)
{
    FieldObjectRecord *object;
    Blk8008AE14_FieldActorSlot *actor_slot;
    s32 context_counter;
    s32 actor_index;

    object = ((FieldObjectRecord *)g_field_actors);
    actor_slot = ((Blk8008AE14_FieldActorSlot *)g_field_object_states);
    actor_index = 0;
scan_actor:
    if (actor_slot->handle != actor_handle)
    {
        actor_slot += 1;
        actor_index += 1;
        object += 1;
        if (actor_index >= 13)
        {
            object = (FieldObjectRecord *)-1;
        }
        else
        {
            goto scan_actor;
        }
    }

    if (object == (FieldObjectRecord *)-1)
    {
        return -1;
    }

    if (object->object_index >= 3)
    {
        context_counter = ((FieldPadContext *)g_pad_ctx)->unk3158;
        if (context_counter != -1)
        {
            ((FieldPadContext *)g_pad_ctx)->unk3158 = context_counter + 1;
        }
    }

    func_8008C024(object, animation_id);
    return 0;
}


/* func_8008AEB0 */
#include "common.h"

typedef struct { u8 pad0[0x14]; s32 unk14; u8 pad18[0x224]; } ActorAEB0;
typedef struct { u8 pad0[0x1C]; u32 unk1C; u8 pad20[0xA]; s16 unk2A; u8 pad2C[0x28]; } RecAEB0;



s32 func_8008AEB0(s32 id)
{
    s32 v;
    s32 result;
    s32 masked;
    RecAEB0 *rec;
    ActorAEB0 *actor;

    rec = ((RecAEB0 *)g_field_actors);
    actor = ((ActorAEB0 *)g_field_object_states);
    v = 0;
    while (v < 13) {
        if (actor->unk14 == id) {
            v = (s32)rec;
            goto found_done;
        }
        v++;
        actor++;
        rec++;
    }
    v = -1;
found_done:
    if (v == -1) {
        return -1;
    }
    if (((*(u16 *)&((RecAEB0 *)v)->unk1C) & 0x1FF) < 2) {
        result = 0;
        if (((RecAEB0 *)v)->unk2A == 0) {
            masked = ((RecAEB0 *)v)->unk1C & 0x600;
            result = masked == 0;
        }
    } else {
        result = 0;
        if ((((RecAEB0 *)v)->unk2A == 0x81) || (((RecAEB0 *)v)->unk2A == 0)) {
            result = 1;
        }
    }
    return result;
}


/* func_8008AF68 */
#include "common.h"

/**
 * @brief Per-actor animation/geometry slot; array element stride 0x23C.
 */
typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x23C - 0x18];
} CommandView5_Struct_D80105AE0;

/**
 * @brief Parallel per-actor record; array element stride 0x54.
 */
typedef struct
{
    u8 data[0x54];
} CommandView5_Struct_D800FDF58;




/**
 * @brief Finds the actor slot matching @p key and notifies its record.
 *
 * Scans the first 13 ((CommandView5_Struct_D80105AE0 *)g_field_object_states) slots for one whose 0x14 field equals @p key;
 * on a hit, forwards the parallel ((CommandView5_Struct_D800FDF58 *)g_field_actors) record and the remaining caller
 * arguments to func_8008BF88 and returns 0, otherwise returns -1.
 *
 * @note gcc272_cdk, 100% match. Preserving arg1-arg3 through the scan is what
 *       keeps the loop key/record temporaries in t0/t1 as in the target.
 */
s32 func_8008AF68(s32 key, s32 arg1, s32 arg2, s32 arg3)
{
    CommandView5_Struct_D800FDF58 *p = ((CommandView5_Struct_D800FDF58 *)g_field_actors);
    CommandView5_Struct_D80105AE0 *e = ((CommandView5_Struct_D80105AE0 *)g_field_object_states);
    s32 i;

    i = 0;
    while (i < 13)
    {
        if (e->unk14 == key)
        {
            goto found;
        }
        i++;
        e++;
        p++;
    }
    p = (CommandView5_Struct_D800FDF58 *)-1;
found:
    if (p == (CommandView5_Struct_D800FDF58 *)-1)
    {
        return -1;
    }
    func_8008BF88(p, arg1, arg2, arg3);
    return 0;
}


/* func_8008AFD8 */
#include "common.h"

/** @brief Blk8008AFD8_Actor position, direction, and animation fields in the 0x54-byte entry. */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u8 pad0C[0x21 - 0xC];
    u8 unk21;
    u8 pad22[2];
    s8 unk24;
    u8 pad25[2];
    s8 unk27;
    u8 pad28[6];
    s16 unk2E;
    u8 pad30[10];
    u8 unk3A;
    u8 unk3B;
    u8 pad3C[0x54 - 0x3C];
} Blk8008AFD8_Entry;
/** @brief Blk8008AFD8_Actor identity and movement flags in the 0x23C-byte record. */
typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x174 - 0x18];
    s32 unk174;
    u8 pad178[0x23C - 0x178];
} Blk8008AFD8_Actor;
/** @brief Resource flags selecting the direction mode. */
typedef struct
{
    u8 pad0[0x10];
    s32 unk10;
} Resource;



extern s32 ratan2(s32 y, s32 x);
/**
 * @brief Turn the source actor toward the target and reset its movement state.
 * @param source_id Source actor identifier.
 * @param target_id Target actor identifier.
 * @return Zero on success, or -1 if either actor is absent.
 */
s32 func_8008AFD8(s32 source_id, s32 target_id)
{
    Blk8008AFD8_Entry *first;
    Blk8008AFD8_Entry *second;
    Blk8008AFD8_Entry *entry;
    Blk8008AFD8_Actor *actor;
    Blk8008AFD8_Entry *entry2;
    Blk8008AFD8_Actor *actor2;
    Blk8008AFD8_Actor *base;
    Blk8008AFD8_Actor *slot;
    s32 i;
    s32 angle;

    entry = ((Blk8008AFD8_Entry *)g_field_actors);
    actor = ((Blk8008AFD8_Actor *)g_field_object_states);
    for (i = 0; i < 13; i++, actor++, entry++)
    {
        if (actor->unk14 == source_id)
        {
            goto first_found;
        }
    }
    first = (Blk8008AFD8_Entry *)-1;
first_check:
    if (first != (Blk8008AFD8_Entry *)-1)
    {
        goto second_start;
    }
    return -1;
first_found:
    first = entry;
    goto first_check;
second_found:
    second = entry2;
    goto second_check;
second_start:
    entry2 = ((Blk8008AFD8_Entry *)g_field_actors);
    actor2 = ((Blk8008AFD8_Actor *)g_field_object_states);
    for (i = 0; i < 13; i++, actor2++, entry2++)
    {
        if (actor2->unk14 == target_id)
        {
            goto second_found;
        }
    }
    second = (Blk8008AFD8_Entry *)-1;
second_check:
    if (second == (Blk8008AFD8_Entry *)-1)
    {
        return -1;
    }

    angle = ratan2(first->unk8 - second->unk8, second->unk0 - first->unk0);
    if (!(((Resource *)g_field_resource_entries)[first->unk3B].unk10 & 1))
    {
        if (angle < -0x700)
        {
            first->unk21 = 2;
        }
        else if (angle < -0x500)
        {
            first->unk21 = 3;
        }
        else if (angle < -0x300)
        {
            first->unk21 = 4;
        }
        else if (angle < -0x100)
        {
            first->unk21 = 0x83;
        }
        else if (angle < 0x100)
        {
            first->unk21 = 0x82;
        }
        else if (angle < 0x300)
        {
            first->unk21 = 0x81;
        }
        else if (angle < 0x500)
        {
            first->unk21 = 0;
        }
        else if (angle < 0x700)
        {
            first->unk21 = 1;
        }
        else
        {
            first->unk21 = 2;
        }
    }
    else
    {
        if ((u32)(angle - 0x401) < 0x7FFU)
        {
            first->unk21 = 0;
        }
        else if (angle < -0x400)
        {
            if (angle < -0xBFF)
            {
                first->unk21 = 0x80;
            }
            else
            {
                first->unk21 = 0;
            }
        }
        else
        {
            first->unk21 = 0x80;
        }
    }

    base = ((Blk8008AFD8_Actor *)g_field_object_states);
    first->unk2E = 1;
    first->unk27 = 0;
    first->unk24 = 1;
    slot = &base[first->unk3A];
    slot->unk174 &= ~0x1800;
    field_restart_actor_animation(first);
    return 0;
}


/* func_8008B1C8 */
#include "common.h"

typedef struct {
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x23C - 0x18];
} ActorSlot;

typedef struct {
    u8 pad0[0x21];
    u8 unk21;
    u8 pad22[2];
    s8 unk24;
    u8 pad25[2];
    s8 unk27;
    u8 pad28[6];
    s16 unk2E;
    u8 pad30[0xA];
    u8 unk3A;
    u8 pad3B[0x54 - 0x3B];
} CommandView7_Entry;

typedef struct {
    u8 pad0[0x174];
    s32 unk174;
    u8 pad178[0x23C - 0x178];
} ActorRecord;




s32 func_8008B1C8(s32 arg0, u8 arg1)
{
    ActorSlot *ra;
    CommandView7_Entry *rb;
    CommandView7_Entry *found;
    s32 i;
    ActorRecord *base;
    ActorRecord *slot;

    rb = ((CommandView7_Entry *)g_field_actors);
    ra = ((ActorSlot *)g_field_object_states);
    for (i = 0; i < 0xD; i++, ra++, rb++)
    {
        if (ra->unk14 == arg0)
        {
            goto found_it;
        }
    }
    found = (CommandView7_Entry *)-1;
check:
    if (found == (CommandView7_Entry *)-1)
    {
        goto fail;
    }
    base = (ActorRecord *)((ActorSlot *)g_field_object_states);
    found->unk21 = arg1;
    found->unk2E = 1;
    found->unk27 = 0;
    found->unk24 = 1;
    slot = &base[found->unk3A];
    slot->unk174 &= ~0x1800;
    field_restart_actor_animation(found);
    return 0;
found_it:
    found = rb;
    goto check;
fail:
    return -1;
}


/* func_8008B288 */
#include "common.h"

/** @brief Per-actor slot in ((Blk8008B288_SlotA *)g_field_object_states); stride 0x23C. */
typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x224];
} Blk8008B288_SlotA;

/** @brief Per-actor record in ((Blk8008B288_EntryB *)g_field_actors); stride 0x54. */
typedef struct
{
    u8 pad0[0x21];
    u8 unk21;
    u8 pad22[0x3B - 0x22];
    u8 unk3B;
    u8 pad3C[0x54 - 0x3C];
} Blk8008B288_EntryB;

typedef struct
{
    u8* start;
    u8* end;
    u8 unk8;
    u8 slot_index;
    u8 padA[0xE - 0xA];
    s16 unkE;
    u32 flags;
} Blk8008B288_FieldResourceEntry;





/**
 * @brief Derive a control value for the actor slot matching the requested key.
 * @param arg0 Actor-slot lookup key.
 * @return Derived control value, or -1 when no matching actor slot exists.
 */
s32 func_8008B288(s32 arg0)
{
    Blk8008B288_EntryB* scan;
    Blk8008B288_EntryB* found;
    Blk8008B288_SlotA* e;
    s32 i;
    s32 result;
    s32 state;

    scan = ((Blk8008B288_EntryB *)g_field_actors);
    e = ((Blk8008B288_SlotA *)g_field_object_states);
    i = 0;
loop:
    i++;
    if (e->unk14 == arg0)
    {
        goto found_label;
    }
    e++;
    scan++;
    if (i < 13)
    {
        goto loop;
    }
    found = (Blk8008B288_EntryB*)-1;
check:
    if (found != (Blk8008B288_EntryB*)-1)
    {
        goto lookup;
    }
    result = -1;
    goto done;
found_label:
    found = scan;
    goto check;
lookup:
    if (((Blk8008B288_FieldResourceEntry *)g_field_resource_entries)[found->unk3B].flags & 1)
    {
        goto special;
    }
    {
        state = found->unk21;
        if ((state & 0x7F) < 0xF)
        {
            switch (found->unk21)
            {
            case 1:
            case 6:
            case 11:
                result = 0x60;
                break;
            case 2:
            case 7:
            case 12:
                result = 0x80;
                break;
            case 3:
            case 8:
            case 13:
                result = 0xA0;
                break;
            case 4:
            case 9:
            case 14:
                result = 0xC0;
                break;
            case 129:
            case 134:
            case 139:
                result = 0x20;
                break;
            case 130:
            case 135:
            case 140:
                result = 0;
                break;
            case 131:
            case 136:
            case 141:
                result = 0xE0;
                break;
            case 132:
            case 137:
            case 142:
                result = 0xC0;
                break;
            case 0:
            case 5:
            case 10:
            case 128:
            case 133:
            case 138:
            default:
                result = 0x40;
                break;
            }
        }
        else
        {
            result = 0x40;
        }
    }
done:
    return result;
special:
    result = found->unk21 & 0x80;
    goto done;
}


/* field_actor_slot_lookup */
#include "common.h"

/*
 * Actor slot lookups keyed by the word at 0x14 of ((CommandView9_SlotA *)g_field_object_states). The first 13
 * slots are scanned in parallel with the ((CommandView9_EntryB *)g_field_actors) records, so a hit in one
 * array selects the same index in the other.
 */

/** @brief Per-actor slot in ((CommandView9_SlotA *)g_field_object_states); stride 0x23C. */
typedef struct
{
    u8 pad0[0x14];
    s32 unk14;   /* 0x14 lookup key */
    u8 pad18[0x224];
} CommandView9_SlotA;

/** @brief Per-actor record in ((CommandView9_EntryB *)g_field_actors); stride 0x54. */
typedef struct
{
    u8 pad0[0x3A];
    u8 unk3A;    /* 0x3A object index / track selector */
    u8 pad3B[0x19];
} CommandView9_EntryB;

s32 func_800839F8();





/**
 * @brief Finds the track value associated with the actor slot matching @p key.
 *
 * Scans the first 13 actor slots in parallel with ((CommandView9_EntryB *)g_field_actors). On a hit, the
 * record's track selector at 0x3A chooses one of the three 0x1C-byte entries
 * in g_field_actor_bindings; selectors >= 2 clamp to the third entry.
 *
 * @param key Value compared against each slot's unk14.
 * @return The selected g_field_actor_bindings word, or -1 when no slot matches.
 */
s32 func_8008B398(s32 key)
{
    CommandView9_EntryB *scan;
    CommandView9_EntryB *found;
    CommandView9_SlotA *e;
    s32 i;
    s32 offset;
    s32 result;
    u8 *base;

    scan = ((CommandView9_EntryB *)g_field_actors);
    e = ((CommandView9_SlotA *)g_field_object_states);
    i = 0;
loop:
    i++;
    if (e->unk14 == key)
    {
        goto found_label;
    }
    e++;
    scan++;
    if (i < 13)
    {
        goto loop;
    }
    found = (CommandView9_EntryB *)-1;
check:
    if (found != (CommandView9_EntryB *)-1)
    {
        goto lookup;
    }
    result = -1;
    goto done;
found_label:
    found = scan;
    goto check;
lookup:
    base = (u8 *)((s32 *)g_field_actor_bindings);
    if (found->unk3A < 2)
    {
        offset = found->unk3A * 0x1C;
    }
    else
    {
        offset = 0x38;
    }
    result = *(s32 *)(base + offset);
done:
    return result;
}

/**
 * @brief Start an animation on the actor whose slot key matches, resolving it through the record's own index.
 * @param arg0 Slot key compared against unk14.
 * @param arg1 Forwarded to func_80083EEC as its third argument.
 * @return -1 when no slot matches, 0 when the animation started, 1 otherwise.
 */
s32 func_8008B42C(s32 arg0, s32 arg1)
{
    CommandView9_SlotA *ra;
    CommandView9_EntryB *rb;
    CommandView9_EntryB *found;
    s32 i;
    s32 anim;

    rb = ((CommandView9_EntryB *)g_field_actors);
    ra = ((CommandView9_SlotA *)g_field_object_states);
    for (i = 0; i < 0xD; i++, ra++, rb++)
    {
        if (ra->unk14 == arg0)
        {
            goto found_it;
        }
    }
    found = (CommandView9_EntryB *) -1;
check:
    if (found != (CommandView9_EntryB *) -1)
    {
        goto body;
    }
    return -1;
found_it:
    found = rb;
    goto check;
body:
    anim = func_800839F8(found->unk3A, 0, rb);
    if ((anim != -1) && (func_80083EEC(found->unk3A, anim, arg1) != 0))
    {
        field_start_actor_animation(anim, 0, 0);
        return 0;
    }
    return 1;
}

/**
 * @brief Start an animation on the actor whose slot key matches, resolving it with index 0.
 * @param arg0 Slot key compared against unk14.
 * @param arg1 Forwarded to func_80083EEC as its third argument.
 * @return -1 when no slot matches, 0 when the animation started, 1 otherwise.
 */
s32 func_8008B500(s32 arg0, s32 arg1)
{
    CommandView9_SlotA *ra;
    CommandView9_EntryB *rb;
    CommandView9_EntryB *found;
    s32 i;
    s32 anim;

    rb = ((CommandView9_EntryB *)g_field_actors);
    ra = ((CommandView9_SlotA *)g_field_object_states);
    for (i = 0; i < 0xD; i++, ra++, rb++)
    {
        if (ra->unk14 == arg0)
        {
            goto found_it;
        }
    }
    found = (CommandView9_EntryB *) -1;
check:
    if (found != (CommandView9_EntryB *) -1)
    {
        goto body;
    }
    return -1;
found_it:
    found = rb;
    goto check;
body:
    anim = func_800839F8(0, 0, rb);
    if ((anim != -1) && (func_80083EEC(found->unk3A, anim, arg1) != 0))
    {
        field_start_actor_animation(anim, 0, 0);
        return 0;
    }
    return 1;
}


/* func_8008B5D0 */
#include "common.h"

/** @brief Actor-slot lookup entry from ((FieldActorSlotEntry *)g_field_object_states). */
typedef struct
{
    u8 pad0[0x14];
    s32 lookup_key;
    u8 pad18[0x224];
} FieldActorSlotEntry;

/** @brief Runtime actor record from ((Blk8008B5D0_FieldActorRecord *)g_field_actors). */
typedef struct
{
    u8 pad0[0x3A];
    u8 track_selector;
    u8 pad3B[0x19];
} Blk8008B5D0_FieldActorRecord;

s32 func_800839F8();




/**
 * @brief Collect matching actor selectors and start the requested actor animation.
 * @param lookup_key Value matched against the first 13 actor-slot entries.
 * @param animation_param Value forwarded to func_80083EEC.
 * @param repeat_count Number of repeated lookup passes used to collect target selectors.
 * @param unused_target Caller-provided target pointer; this routine does not consume it.
 * @return -1 if no actor record matches, 0 if the animation starts, or 1 if setup fails.
 */
s32 func_8008B5D0(s32 lookup_key, s32 animation_param, s32 repeat_count, s32 *unused_target)
{
    FieldActorSlotEntry *slot;
    Blk8008B5D0_FieldActorRecord *scan_record;
    Blk8008B5D0_FieldActorRecord *final_record;
    Blk8008B5D0_FieldActorRecord *found;
    s32 i;
    s32 repeat_index;
    s32 target_count;
    s32 animation_slot;
    s32 targets[16];

    repeat_index = 0;
    target_count = repeat_index;
    for (; repeat_index < repeat_count; repeat_index++)
    {
        scan_record = ((Blk8008B5D0_FieldActorRecord *)g_field_actors);
        slot = ((FieldActorSlotEntry *)g_field_object_states);
        for (i = 0; i < 13; i++, slot++, scan_record++)
        {
            if (slot->lookup_key == lookup_key)
            {
                found = scan_record;
                goto matched;
            }
        }
        found = (Blk8008B5D0_FieldActorRecord *)-1;
    matched:
        if (found != (Blk8008B5D0_FieldActorRecord *)-1)
        {
            targets[target_count] = found->track_selector;
            target_count++;
        }
    }

    final_record = ((Blk8008B5D0_FieldActorRecord *)g_field_actors);
    slot = ((FieldActorSlotEntry *)g_field_object_states);
    for (i = 0; i < 13; i++, slot++, final_record++)
    {
        if (slot->lookup_key == lookup_key)
        {
            goto found_it;
        }
    }
    found = (Blk8008B5D0_FieldActorRecord *)-1;
check:
    if (found != (Blk8008B5D0_FieldActorRecord *)-1)
    {
        goto body;
    }
    return -1;
found_it:
    found = final_record;
    goto check;
body:
    animation_slot = func_800839F8(found->track_selector, 0);
    if ((animation_slot != -1) && (func_80083EEC(found->track_selector, animation_slot, animation_param) != 0))
    {
        field_start_actor_animation(animation_slot, target_count, (u8 *)targets);
        return 0;
    }
    return 1;
}


/* field36 */
#include "common.h"

typedef struct
{
    /* 0x0 */ s32 unk0;
    /* 0x4 */ s32 unk4;
    /* 0x8 */ s32 unk8;
} UnkStruct_8008B724;

extern s32 D_8010A020[];

void func_8008B724(void)
{
    D_8010A020[2] = 0;
    D_8010A020[1] = 0;
    D_8010A020[0] = 0;
}


/* func_8008B73C */
#include "common.h"

extern s32 D_8010A020[];





/**
 * @brief Restart animations for three pending actor slots and clear their update flags.
 * @see decomp.me (100%)
 */
void func_8008B73C(void)
{
    s32 i;
    s32 *flag;
    u8 *rec;
    s32 scaled_index;
    s32 index;
    u8 *slot;
    s32 slot_offset;
    u8 *actor_base;
    u8 *ae0_base;

    i = 0;
    actor_base = ((u8 *)g_field_actor_slots);
    ae0_base = ((u8 *)g_field_object_states);
    flag = D_8010A020;
    rec = ((u8 *)g_field_actor_bindings);
restart_slots:
    {
        if (*flag != 0 && *(s32 *)(rec + 0x0) == 2)
        {
            slot_offset = *(s32 *)(rec + 0x18) * 0x244;
            *(s32 *)(slot_offset + (u32)actor_base + 0xC) = *(s32 *)(slot_offset + (u32)actor_base + 0x10);
            field_start_actor_animation(*(s32 *)(rec + 0x18), 0, 0);
            index = *(s32 *)(rec + 0xC);
            if (index >= 3)
            {
                index = 2;
            }
            scaled_index = index * 8;
            (ae0_base + (((scaled_index + index) * 0x10) - index) * 4)[0x179] = *(u8 *)(rec + 0x18);
            *flag = 0;
            slot = (u8 *)(*(s32 *)(rec + 0x18) * 0x244 + (u32)actor_base);
            slot[0x2A] = 1;
        }
        flag += 1;
        i += 1;
        rec += 0x1C;
    }
    if (i < 3)
    {
        goto restart_slots;
    }
}
