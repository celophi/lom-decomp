/**
 * @file field_effect_update.c
 * @brief Field actor effect spawning, per-frame update, lifetime, placement,
 *        target resolution, and reward collection (unk2 segment, gcc272_cdk).
 *
 * Merged translation unit consolidating the former field7.c (func_8006D79C),
 * field8.c, and field9.c. Function bodies are preserved verbatim.
 *
 * func_8006D79C keeps its original field7 declaration environment: it uses
 * private record views (Struct_D800FDF58, Struct_D80105AE0, and the FieldSpawn*
 * typedefs) and deliberately declares none of its callees, so they stay
 * implicitly declared - reproducing that state is required for the match. The
 * effect-typed views (FieldMotionRecord, FieldObjectPlacement, FieldActorState)
 * used by the remaining functions are pulled in by field_effect_types.h below,
 * after func_8006D79C, and the few symbols viewed under both layouts are
 * declared at block scope inside func_8006D79C.
 */

#include "common.h"
#include "field_types.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

typedef struct
{
    u8* start;
    u8* end;
    u8 unk8;
    u8 slot_index;
    u8 padA[0xE - 0xA];
    s16 unkE;
    u32 flags;
} FieldResourceEntry;

extern FieldResourceEntry g_field_resource_entries[];


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
    union { s32 word; u16 half[2]; struct { unsigned low:13; unsigned placement:2; unsigned bit15:1; unsigned group:2; unsigned bit18:1; unsigned kind:4; unsigned high:9; } bits; } flags;
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
    u8 pad3E[0x40 - 0x3E];
    s32 unk40; /* 0x40 */
    u32 unk44; /* 0x44 */
    u32 unk48; /* 0x48 */
    u32 unk4C; /* 0x4C */
    u8 pad50[0x54 - 0x50];
} Struct_D800FDF58;

typedef struct
{
    u8 pad0[0xC];
    u32 unkC;   /* 0x0C */
    u32 unk10;  /* 0x10 */
    u8 pad14[0x60 - 0x14];
    u8 unk60[0x12C - 0x60];
    u32 unk12C; /* 0x12C */
    Vec2s unk130[4]; /* 0x130 */
    s16 unk140; /* 0x140 */
    s16 unk142; /* 0x142 */
    s16 unk144; /* 0x144 */
    s16 unk146; /* 0x146 */
    u8 pad148[0x178 - 0x148];
    union { u32 word; u8 bytes[4]; } state;
    u8 pad17C[0x18E - 0x17C];
    u8 unk18E;  /* 0x18E */
    u8 pad18F[0x190 - 0x18F];
    Vec2s unk190[3]; /* 0x190 */
    s32 unk19C; /* 0x19C */
    s32 unk1A0; /* 0x1A0 */
    u8 pad1A4[0x1A8 - 0x1A4];
    u8 unk1A8;  /* 0x1A8 */
    u8 unk1A9;  /* 0x1A9 */
    u8 unk1AA;  /* 0x1AA */
    u8 pad1AB[0x23C - 0x1AB];
} Struct_D80105AE0;

typedef struct
{
    u32 unk0;  /* 0x00 */
    u32 unk4;  /* 0x04 (halfword view at 0x06) */
    u8 unk8;   /* 0x08 */
    u8 unk9;   /* 0x09 */
    u8 padA;
    u8 unkB;   /* 0x0B */
    u8 unkC;   /* 0x0C (also read as a word) */
    u8 unkD;   /* 0x0D */
    u8 unkE;   /* 0x0E */
    u8 unkF;   /* 0x0F */
    u8 unk10;  /* 0x10 */
    u8 unk11;  /* 0x11 */
    u8 pad12[0x14 - 0x12];
    u32 unk14; /* 0x14 (byte view at 0x14, halfword view at 0x16) */
    s16 unk18; /* 0x18 */
    u8 unk1A;  /* 0x1A */
    u8 pad1B;
    u32 unk1C; /* 0x1C */
    u8 unk20;  /* 0x20 */
    u8 unk21;  /* 0x21 */
    u8 unk22;  /* 0x22 */
    u8 unk23;  /* 0x23 */
    u32 unk24; /* 0x24 (halfword view at 0x26) */
    u32 unk28; /* 0x28 (byte view at 0x28) */
    u8 unk2C;  /* 0x2C (also read as a word) */
    u8 pad2D;
    u8 unk2E;  /* 0x2E */
    u8 unk2F;  /* 0x2F */
    u8 pad30;
    u8 unk31;  /* 0x31 */
    u8 pad32;
    u8 unk33;  /* 0x33 */
    u32 unk34; /* 0x34 */
    s16 unk38; /* 0x38 */
    s16 unk3A; /* 0x3A */
    s16 unk3C; /* 0x3C */
    s16 pad3E;
    s16 unk40; /* 0x40 */
    s16 unk42; /* 0x42 */
    s16 unk44; /* 0x44 */
    s16 unk46; /* 0x46 */
} FieldSpawnPartView;

typedef struct FieldSpawnAnimView
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
} FieldSpawnAnimView;

typedef struct
{
    FieldSpawnPartView* unk0;
    u8 pad4[0xC - 4];
    FieldSpawnAnimView* unkC;
    u8 pad10[0x14 - 0x10];
    u8* unk14;
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
} FieldSpawnActorView;

typedef struct
{
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
} FieldSpawnSVector;

typedef struct
{
    u32 unk0;
    u8 pad4[0xC - 4];
    u32 unkC;
    u8 pad10[0x1C - 0x10];
} Struct_D80105880;

extern FieldSpawnPartView D_800FE3A0[];
extern Struct_D80105880 D_80105880[];
extern FieldVector D_80105778;
extern s32 g_field_action_context;
extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;
extern s32 D_80105760;
extern s32 D_80105770;
extern u8* D_801058D4;
extern s32 g_field_track_index;

#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

#define GTE_SET_ROT_MATRIX(m) gte_SetRotMatrix(m)
#define GTE_LDV0(v) gte_ldv0(v)
#define GTE_RTV0() gte_rtv0()
#define GTE_LDLVL(v) gte_ldlvl(v)
#define GTE_SQR0() gte_sqr0()
#define GTE_STLVNL(v) gte_stlvnl(v)


/**
 * @brief Spawn one field effect record for a part of an actor.
 *
 * Claims the first free slot in g_field_effect_records (marked by unk25 == 0xFF), seeds it
 * from actor->unk0[part_index], derives its orientation through the GTE, and
 * then dispatches on the part's 6-bit placement opcode
 * ((part->unk28 >> 18) & 0x3F) to position it relative to the owner object, the
 * tracked object, the camera, or another already-spawned effect. Recurses for
 * chained parts and for the opcode-0x33 retry.
 *
 * @param actor Actor whose script is spawning the effect.
 * @param part_index Index of the part definition within actor->unk0.
 * @param start First g_field_effect_records slot to consider when an opcode has to search
 *              for a previously spawned sibling effect; 0 on the outermost call.
 * @return Index of the slot that was filled, or -1 if no slot was free or the
 *         placement opcode rejected the spawn.
 *
 * @see decomp.me (100%)
 */
s32 func_8006D79C(FieldSpawnActorView* actor, s32 part_index, s32 start)
{
    /* field7 views of shared pools; block scope keeps these record layouts
     * private to this function so field8/field9 can use the effect-typed
     * views of the same symbols at file scope. */
    extern Struct_D800FDF58 D_800FDF58[];
    extern Struct_D800FDF58 g_field_effect_records[];
    extern Struct_D80105AE0 D_80105AE0[];
    extern FieldSpawnActorView g_field_actor_slots[];
    s32 half_turn;
    FieldVector* vec = (FieldVector*)0x1F800000;
    FieldVector* sqr = (FieldVector*)0x1F800010;
    FieldSpawnSVector* dir = (FieldSpawnSVector*)0x1F800030;
    FieldMatrix* mtx = (FieldMatrix*)0x1F800040;
    Struct_D800FDF58* rec;
    Struct_D800FDF58* scan;
    Struct_D800FDF58* scanA;
    Struct_D800FDF58* scanB;
    Struct_D80105AE0* slot;
    Struct_D80105AE0* init_slot;
    Struct_D80105AE0* slots_base_init;
    Struct_D80105AE0* slot27;
    Struct_D80105AE0* slot27_guard;
    Struct_D80105AE0* slot28;
    Struct_D80105AE0* slot_place_check;
    Struct_D80105AE0* slot_place_attach;
    Struct_D80105AE0* slot_place_base;
    Struct_D80105AE0* slot_owner_valid;
    Struct_D800FDF58* src;
    Struct_D800FDF58* source33;
    Struct_D800FDF58* source_actor;
    Struct_D80105AE0* slot2_owner;
    Struct_D80105AE0* slot3_track;
    FieldSpawnSVector* scanA_rot;
    FieldSpawnSVector* scanB_rot;
    FieldSpawnPartView* part;
    s32 base_x;
    s32 count;
    s32 place_y;
    s32 width;
    s32 placement_x;
    s32 i;
    s32 n;
    s32 placement_index;
    s32 val;
    s32 direction_z;
    s32 kind;
    s32 sub;
    s32 nA;
    s32 nB;
    s32 subA;
    s32 pos27;
    s32 pos28;
    s32 pos37;
    u8 track_obj;
    u8* res;
    u8* drop_count_ptr;

    i = 0;
    count = 0xFF;
    source_actor = g_field_effect_records;
    scanA = source_actor;
find_slot:
    if (scanA->unk25 != count)
    {
        i++;
        scanA++;
        if (i < 0x100)
        {
            goto find_slot;
        }
    }
    if (i == 0x100)
    {
        g_field_action_context = 0x10101010;
        return -1;
    }

    rec = (Struct_D800FDF58*)((u32)(i * sizeof(*rec)) + (u32)source_actor);
    part = &actor->unk0[part_index];
    if ((s32)part->unk24 < 0)
    {
        rec->unk3D = count;
    }

    if ((u32)(((part->unk28 >> 18) & 0x3F) - 0x2A) < 8U &&
        (((part->unk28 >> 18) & 0x3F) - 0x22) == part_index)
    {
        return -1;
    }
    if ((u32)(((part->unk28 >> 18) & 0x3F) - 0x14) < 8U &&
        (((part->unk28 >> 18) & 0x3F) - 0x14) == part_index)
    {
        return -1;
    }
    if ((u32)(((part->unk28 >> 18) & 0x3F) - 0x37) < 8U &&
        (((part->unk28 >> 18) & 0x3F) - 0x37) == part_index)
    {
        return -1;
    }

    if (part->unk14 & 0xF0)
    {
        s32 parameter = *(u16*)((u8*)part + 0x26);
        rec->unk1B = parameter & 0xF;
    }
    else
    {
        rec->unk1B = (part->unk4 >> 8) & 7;
    }
    rec->flags.word = (rec->flags.word & 0xF8FFFFFF) | (((part->unk4 >> 13) & 7) << 24);
    rec->flags.word = (rec->flags.word & ~0x600) | ((part->unk20 >> 6) << 9);
    rec->unk28 = part->unkD;
    rec->unk2A = ((u16*)&part->unk14)[1];
    rec->unk2E = part->unk18;
    rec->unk29 = g_field_track_index;
    rec->flags.word = ((rec->flags.word & 0x9FFFFFFF) | ((*(u8*)&part->unk14 & 3) << 29)) & ~0x1000;
    rec->flags.word = (rec->flags.word & 0xF7FFFFFF) | (((part->unk34 >> 18) & 1) << 27);
    rec->flags.word &= ~0x6000;
    rec->flags.word &= 0xFFFBFFFF;
    half_turn = 128;
    rec->flags.word &= 0xFF87FFFF;
    if (part->unk24 & 0x800000)
    {
        s32 eval = field_evaluate_parameter_track_at_time(actor, (part->unk24 >> 25) & 0xF, 0) != 0;
        rec->flags.word = (rec->flags.word & 0xFF7FFFFF) | (eval << 23);
        goto bit23_done;
    scan_slot_found:
        rec->unk20 = n;
        goto scan_slots_done;
    }
    else
    {
        rec->flags.word = (rec->flags.word & 0xFF7FFFFF) | (((part->unk4 >> 1) & 1) << 23);
    }
bit23_done:
    rec->flags.word = rec->flags.word & 0xFFFCFFFF;
    if (rec->unk1B == 8)
    {
        s32 scan_ff;
        n = 0;
        val = n;
        scan_ff = 0xFF;
        count = actor->unk3B[g_field_track_index][part_index];
        scan = g_field_effect_records;
    scan_slots:
        if (scan->unk25 != scan_ff && scan->unk23 == part->unk46 && scan->unk22 == actor->unk233)
        {
            val = 1;
            if (count == 0)
            {
                goto scan_slot_found;
            }
            rec->unk20 = n;
            count--;
        }
        n++;
        scan++;
        if (n < 0x100)
        {
            goto scan_slots;
        }
    scan_slots_done:
        if (val == 0)
        {
            rec->unk25 = 0xFF;
            return -1;
        }
    }

    {
        s32 record_type = part->unkB;
        rec->unk27 = 0;
        rec->unk34 = 0;
        rec->unk25 = record_type;
    }
    rec->unk44 = part->unk40 << 8;
    rec->unk48 = part->unk42 << 8;
    rec->unk4C = part->unk44 << 8;
    if (((part->unk28 >> 8) & 1) && (*(u32*)&part->unk2C & 0x0F000000))
    {
        if (part->unk34 & 0x10000)
        {
            s32 limit = (*(u32*)&part->unk2C >> 24) & 0xF;
            n = 0;
            if (limit != 0)
            {
                val = limit;
                slots_base_init = D_80105AE0;
                do
                {
                    init_slot = &slots_base_init[actor->owner_object_index];
                    if (init_slot->unk60[n] == 0)
                    {
                        n++;
                    }
                    else
                    {
                        init_slot->unk60[n] = init_slot->unk60[n] - 1;
                        rec->unk21 = part->unk1A + n;
                        break;
                    }
                } while (n < val);
            }
            if (n == (part->unk2F & 0xF))
            {
                rec->unk25 = 0xFF;
                return -1;
            }
        }
        else
        {
            rec->unk21 = part->unk1A + (((part->unk2F & 0xF) * rand()) >> 15);
        }
    }
    else
    {
        rec->unk21 = part->unk1A;
    }

    switch ((s32)((part->unk28 >> 26) & 3))
    {
    case 0:
        rec->unk32 = part->unk21;
        break;
    case 1:
        rec->unk32 = field_evaluate_parameter_track(actor, part->unk21 & 0xF);
        break;
    case 2:
        rec->unk32 = field_evaluate_parameter_track_at_time(actor, part->unk21 & 0xF, 0);
        break;
    }
    switch ((s32)((part->unk28 >> 28) & 3))
    {
    case 0:
        rec->unk33 = part->unk22;
        break;
    case 1:
        rec->unk33 = field_evaluate_parameter_track(actor, part->unk22 & 0xF);
        break;
    case 2:
        rec->unk33 = field_evaluate_parameter_track_at_time(actor, part->unk22 & 0xF, 0);
        break;
    }
    if ((((part->unk28 >> 10) & 1) || (part->unk34 & 0x08000000)) && rec->unk1B == 0 &&
        !(D_800FDF58[actor->owner_object_index].unk21 & 0x80))
    {
        u8 angle;
        u8 original_angle;

        original_angle = rec->unk33;
        angle = original_angle;
        if (angle >= 64 && original_angle < 128)
        {
            rec->unk33 = 128 - original_angle;
        }
        else
        {
            original_angle -= half_turn;
            rec->unk33 = -original_angle;
        }
    }
    rec->unk2C = 0;
    actor->unk3B[g_field_track_index][part_index]++;
    rec->unk27 = 0;
    rec->unk23 = part_index;
    rec->unk22 = actor->unk233;
    if ((part->unk28 >> 25) & 1)
    {
        if ((part->unk4 >> 12) & 1)
        {
            rec->unk18 = field_evaluate_parameter_track(actor, (part->unk4 >> 16) & 0xF);
            rec->unk19 = field_evaluate_parameter_track(actor, (((((u16*)&part->unk4)[1] & 0xF) + 1) & 0xF));
            rec->unk1A = field_evaluate_parameter_track(actor, (((((u16*)&part->unk4)[1] & 0xF) + 2) & 0xF));
        }
        else
        {
            rec->unk18 = rec->unk19 = rec->unk1A =
                field_evaluate_parameter_track(actor, (part->unk4 >> 16) & 0xF);
        }
    }
    rec->flags.word = (rec->flags.word & 0xFFFF7FFF) | ((part->unk4 * 0x10) & 0x8000);
    rec->flags.word = (rec->flags.word & 0xEFFFFFFF) | (((part->unk28 >> 25) & 1) << 28);
    func_80070CB8(actor, part, rec);
    RotMatrix_gte((FieldSpawnSVector*)&rec->unk10, mtx);
    RotMatrixZ(rec->unk32 * 16, mtx);
    RotMatrixY(rec->unk33 * 16, mtx);
    dir->unk0 = 0;
    dir->unk2 = -0x1000;
    dir->unk4 = 0;
    GTE_SET_ROT_MATRIX(mtx);
    GTE_LDV0(dir);
    GTE_RTV0();
    GTE_STLVNL(vec);
    if (((part->unk4 >> 2) & 1) || rec->unk1B != 0)
    {
        rec->unk12 = ratan2(-vec->vz, vec->vx);
        GTE_LDLVL(vec);
        GTE_SQR0();
        GTE_STLVNL(sqr);
        rec->unk14 = ratan2(SquareRoot0(sqr->vx + sqr->vz), -vec->vy);
        rec->unk10 = 0;
        if (rec->unk14 < 0)
        {
            rec->unk14 = -rec->unk14;
        }
    }
    base_x = vec->vx;
    n = func_8007E754(actor, part);
    rec->flags.word = (rec->flags.word & ~0x1FF) | (n & 0x1FF);
    rec->unk0 = (n * vec->vx) >> 4;
    rec->unk4 = (n * vec->vy) >> 4;
    rec->unk8 = (n * vec->vz) >> 4;
    rec->unk24 = part->unk8;
    rec->unk3A = actor->owner_object_index;
    if (rec->unk25 == 0)
    {
        res = D_801058D4;
        goto call_res;
    }
    if (rec->unk25 == 1)
    {
        res = g_field_actor_slots[rec->unk22].unk14;
        if (res != 0)
        {
        call_res:
            field_begin_actor_animation_forward(rec, res);
            goto after_source;
        }
    }
    if (rec->unk25 == 2)
    {
        { Struct_D80105AE0 *owner_slots = D_80105AE0;
          u8 owner_index = actor->owner_object_index;
        slot2_owner = &owner_slots[owner_index]; }
        if (*(u8*)&slot2_owner->state.word & 1)
        {
            if (slot2_owner->state.bytes[2] != actor->unk233)
            {
                goto kill_rec;
            }
        }
        source_actor = &D_800FDF58[*(u8*)&actor->owner_object_index];
        if (!((part->unk4 >> 11) & 1) && !((part->unk28 >> 25) & 1) && (part->unk2C >> 5) == 0 &&
            (*(u32*)&part->unkC & 0xFFFF0000) == 0x80800000 && part->unk10 == 0x80)
        {
            rec->flags.word |= 0x10008000;
            rec->unk18 = D_800FE3A0[source_actor->unk3A].unkE;
            rec->unk19 = D_800FE3A0[source_actor->unk3A].unkF;
            rec->unk1A = D_800FE3A0[source_actor->unk3A].unk10;
        }
        rec->unk3A = D_800FDF58[actor->owner_object_index].unk3A;
        rec->unk3B = D_800FDF58[actor->owner_object_index].unk3B;
        rec->unkC = D_800FDF58[actor->owner_object_index].unkC;
        rec->unk21 |= D_800FDF58[actor->owner_object_index].unk21 & 0x80;
        rec->flags.word = (rec->flags.word & 0xFFFCFFFF) |
                     ((D_800FDF58[actor->owner_object_index].flags.half[1] & 3) << 16);
        rec->flags.word = (rec->flags.word & 0xFF87FFFF) | (D_800FDF58[actor->owner_object_index].flags.word & 0x780000);
        res = g_field_resource_entries[D_800FDF58[actor->owner_object_index].unk3B].start;
        if (res != 0)
        {
            field_restart_actor_animation(rec, res);
        }
        goto after_source;
    }
    if (rec->unk25 == 3)
    {
        track_obj = actor->unk229[g_field_track_index];
        if (track_obj == 0xFF)
        {
            rec->unk25 = track_obj;
            actor->unk3B[g_field_track_index][part_index]--;
            actor->unkCC[g_field_track_index][part_index]--;
            return -1;
        }
        if (!((D_80105AE0[actor->unk229[g_field_track_index]].state.word >> 6) & 1))
        {
            s32 offset;
            s32 offset2;
            s32 metadata_index;
            Struct_D80105880 *metadata = D_80105880;
            if (actor->unk229[g_field_track_index] < 2U)
            {
                offset = actor->unk229[g_field_track_index] * 0x1C;
            }
            else
            {
                offset = 0x38;
            }
            metadata_index = ((Struct_D80105880*)((u8*)metadata + offset))->unkC;
            if (metadata_index == actor->unk229[g_field_track_index])
            {
                Struct_D80105880 *next_metadata = D_80105880;
                if ((u32)(metadata_index & 0xFF) < 2U)
                {
                    offset2 = metadata_index * 0x1C;
                }
                else
                {
                    offset2 = 0x38;
                }
                if (((Struct_D80105880*)((u8*)next_metadata + offset2))->unk0 != 0)
                {
                    goto kill_rec;
                }
            }
        }
        {
            Struct_D80105AE0 *table_base = D_80105AE0;
            slot3_track = &table_base[actor->unk229[g_field_track_index]];
        }
        if (*(u8*)&slot3_track->state.word & 1)
        {
            if (slot3_track->state.bytes[2] != actor->unk233)
            {
            kill_rec:
                rec->unk25 = 0xFF;
                goto after_source;
            }
        }
        source_actor = &D_800FDF58[actor->unk229[g_field_track_index]];
        if (!((part->unk4 >> 11) & 1) && !((part->unk28 >> 25) & 1) && (part->unk2C >> 5) == 0 &&
            (*(u32*)&part->unkC & 0xFFFF0000) == 0x80800000 && part->unk10 == 0x80)
        {
            rec->flags.word |= 0x10008000;
            rec->unk18 = D_800FE3A0[source_actor->unk3A].unkE;
            rec->unk19 = D_800FE3A0[source_actor->unk3A].unkF;
            rec->unk1A = D_800FE3A0[source_actor->unk3A].unk10;
        }
        rec->unk3A = D_800FDF58[actor->unk229[g_field_track_index]].unk3A;
        rec->unk3B = D_800FDF58[actor->unk229[g_field_track_index]].unk3B;
        rec->unkC = D_800FDF58[actor->unk229[g_field_track_index]].unkC;
        rec->unk21 |= D_800FDF58[actor->unk229[g_field_track_index]].unk21 & 0x80;
        rec->flags.word = (rec->flags.word & 0xFFFCFFFF) |
                     ((D_800FDF58[actor->unk229[g_field_track_index]].flags.half[1] & 3) << 16);
        rec->flags.word =
            (rec->flags.word & 0xFF87FFFF) | (D_800FDF58[actor->unk229[g_field_track_index]].flags.word & 0x780000);
        res = g_field_resource_entries[D_800FDF58[actor->unk229[g_field_track_index]].unk3B].start;
        if (res != 0)
        {
            field_restart_actor_animation(rec, res);
        }
    }

after_source:
    if ((part->unk0 >> 13) & 1)
    {
        rec->unk2E = field_evaluate_parameter_track(actor, part->unk18 & 0xF);
    }
    if ((((part->unk28 >> 10) & 1) || (part->unk34 & 0x08000000)) && rec->unk1B != 0 &&
        !(D_800FDF58[actor->owner_object_index].unk21 & 0x80))
    {
        base_x = -base_x;
        rec->unk0 = -rec->unk0;
    }
    if ((rec->flags.word & 0x07000000) == 0x05000000)
    {
        rec->unk0 = 0;
        rec->unk4 = 0;
        rec->unk8 = 0;
    }
    if (part->unk24 & 0x60000000)
    {
        rec->unk0 = 0;
        rec->unk4 = 0;
        rec->unk8 = 0;
    }

    kind = part->unk28 >> 18;
    kind &= 0x3F;
    switch (kind)
    {
    case 0x0:
    case 0x1:
    case 0x2:
    case 0x3:
    case 0x4:
    case 0x5:
    case 0x6:
    case 0x7:
    case 0x8:
    case 0x9:
    case 0xA:
    case 0xB:
    case 0xC:
    case 0xD:
    case 0xE:
    case 0xF:
    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
        if (kind >= 0xA)
        {
            track_obj = actor->unk229[g_field_track_index];
            if (track_obj == 0xFF)
            {
                rec->unk25 = track_obj;
                actor->unk3B[g_field_track_index][part_index]--;
                actor->unkCC[g_field_track_index][part_index]--;
            return -1;
            }
            placement_index = actor->unk229[g_field_track_index];
            kind -= 0xA;
src = &D_800FDF58[placement_index]; slot = &D_80105AE0[placement_index];
        }
        else
        {
            {
                Struct_D80105AE0 *table_base = D_80105AE0;
                slot_place_check = &table_base[actor->owner_object_index];
            }
            if ((*(u8*)&slot_place_check->state.word & 1) && actor->unk233 >= 0x40U &&
                !(((u32)slot_place_check->state.word >> 5) & 1) && slot_place_check->state.bytes[2] != actor->unk233)
            {
                goto fail_slot;
            }
            placement_index = actor->owner_object_index;
src = &D_800FDF58[placement_index]; slot = &D_80105AE0[placement_index];
        }

        if ((part->unk28 >> 9) & 1)
        {
            width = abs(slot->unk144 - slot->unk140);
            part->unk2E = width * 2;
        }
        if ((part->unk28 >> 1) & 1)
        {
            nA = 0;
            width = abs(slot->unk146 - slot->unk142);
            part->unk33 = width * 2;
        } else { nA = 0; }
        count = nA;
        switch (kind)
        {
        case 1:
            count = (slot->unk144 + slot->unk140) >> 1;
            nA = (slot->unk146 + slot->unk142) >> 1;
            break;
        case 2:
            count = (slot->unk144 + slot->unk140) >> 1;
            nA = 0;
            break;
        case 3:
            count = (slot->unk144 + slot->unk140) >> 1;
            nA = slot->unk142;
            break;
        case 4:
            count = slot->unk140;
            nA = (slot->unk146 + slot->unk142) >> 1;
            break;
        case 5:
            count = slot->unk144;
            nA = (slot->unk146 + slot->unk142) >> 1;
            break;
        case 6:
            count = slot->unk140;
            nA = slot->unk142;
            break;
        case 7:
            count = slot->unk144;
            nA = slot->unk142;
            break;
        case 8:
            count = slot->unk140;
            nA = slot->unk146;
            break;
        case 9:
            count = slot->unk144;
            nA = slot->unk146;
            break;
        default:
            break;
        }
        count <<= 8;
        nA <<= 8;
        rec->unk0 += src->unk0 + count;
        rec->unk4 += src->unk4 + nA;
        rec->unk8 += src->unk8;
        if (rec->unk25 == 0xFD)
        {
            slot_place_base = D_80105AE0;
            rec->unk3A = src->unk3A;
            slot_place_attach = &slot_place_base[src->unk3A];
            if (!(*(u8*)&slot_place_attach->state.word & 1) || slot_place_attach->state.bytes[2] == actor->unk233)
            {
                rec->unkC = src->unkC;
                rec->unk3B = src->unk3B;
                rec->flags.word = (rec->flags.word & 0xFF87FFFF) | (src->flags.word & 0x780000);
                rec->flags.bits.group = src->flags.half[1];
                if (!((part->unk4 >> 11) & 1) && !((part->unk28 >> 25) & 1) && (part->unk2C >> 5) == 0 &&
                    (*(u32*)&part->unkC & 0xFFFF0000) == 0x80800000 && part->unk10 == 0x80)
                {
                    rec->flags.word |= 0x10008000;
                    rec->unk18 = D_800FE3A0[src->unk3A].unkE;
                    rec->unk19 = D_800FE3A0[src->unk3A].unkF;
                    rec->unk1A = D_800FE3A0[src->unk3A].unk10;
                }
                if (rec->unk21 == 0xFF)
                {
                    rec->unk21 = src->unk21;
                    rec->unk27 = src->unk27;
                    part->unk2E = D_800FE3A0[src->unk3A].unk2E;
                    part->unk33 = D_800FE3A0[src->unk3A].unk33;
                    rec->unk34 = src->unk34;
                    rec->unk35 = src->unk35;
                    rec->unk29 = src->unk29;
                    rec->unk36 = src->unk36;
                    rec->unk37 = src->unk37;
                    rec->unk38 = src->unk38;
                    rec->unk16 = src->unk16;
                }
                else
                {
                    rec->unk21 |= src->unk21 & 0x80;
                    res = g_field_resource_entries[src->unk3B].start;
                    if (res != 0)
                    {
                        field_restart_actor_animation(rec, res);
                    }
                }
            }
            else
            {
                goto mark_dead;
            }
        }
        break;
    case 0x14:
    case 0x15:
    case 0x16:
    case 0x17:
    case 0x18:
    case 0x19:
    case 0x1A:
    case 0x1B:
    case 0x2A:
    case 0x2B:
    case 0x2C:
    case 0x2D:
    case 0x2E:
    case 0x2F:
    case 0x30:
    case 0x31:
        if (kind >= 0x2A)
        {
            goto scanA_high;
        }
        subA = kind - 0x14;
        goto scanA_init;

    scanA_copy:
        rec->unk21 = g_field_effect_records[nA].unk21;
        rec->unk27 = g_field_effect_records[nA].unk27;
        rec->unk34 = g_field_effect_records[nA].unk34;
        rec->unk35 = g_field_effect_records[nA].unk35;
        rec->unk29 = g_field_effect_records[nA].unk29;
        rec->unk36 = g_field_effect_records[nA].unk36;
        rec->unk37 = g_field_effect_records[nA].unk37;
        rec->unk38 = g_field_effect_records[nA].unk38;
        goto scanA_done;

    scanA_high:
        subA = kind - 0x22;
    scanA_init:
        nA = start;
        if (nA < 0x100)
        {
            do
            {
                if (g_field_effect_records[nA].unk25 != 0xFF && g_field_effect_records[nA].unk23 == subA && g_field_effect_records[nA].unk22 == actor->unk233 &&
                    ((actor->unk0[subA].unk14 & 4) || g_field_effect_records[nA].unk29 == g_field_track_index))
                {
                    rec->unk0 += g_field_effect_records[nA].unk0;
                    rec->unk4 += g_field_effect_records[nA].unk4;
                    rec->unk8 += g_field_effect_records[nA].unk8;
                    rec->flags.word = (rec->flags.word & ~0x1000) | (g_field_effect_records[nA].flags.word & 0x1000);
                    if (part->unk1C & 0x08000000)
                    {
                        rec->unk10 = ((FieldSpawnSVector*)&g_field_effect_records[nA].unk10)->unk0;
                        rec->unk12 = ((FieldSpawnSVector*)&g_field_effect_records[nA].unk10)->unk2;
                        rec->unk14 = ((FieldSpawnSVector*)&g_field_effect_records[nA].unk10)->unk4;
                    }
                    rec->unk30 = nA;
                    if (rec->unk25 == 0xFD)
                    {
                        rec->unk3B = g_field_effect_records[nA].unk3B;
                        rec->unkC = g_field_effect_records[nA].unkC;
                        rec->unk25 = g_field_effect_records[nA].unk25;
                        rec->unk3A = g_field_effect_records[nA].unk3A;
                        rec->flags.word = (rec->flags.word & 0xFF87FFFF) | (((u32)g_field_effect_records[nA].flags.word >> 19 & 15) << 19);
                        rec->flags.word = (rec->flags.word & 0xFFFCFFFF) | ((((u32)g_field_effect_records[nA].flags.word >> 16) & 3) << 16);
                        if (rec->unk21 == 0xFF)
                        {
                            goto scanA_copy;
                        }
                        rec->unk3A = g_field_effect_records[nA].unk3A;
                        rec->unk25 = g_field_effect_records[nA].unk25;
                        rec->flags.word = (rec->flags.word & 0xFF87FFFF) | (((u32)g_field_effect_records[nA].flags.word >> 19 & 15) << 19);
                        rec->flags.word = (rec->flags.word & 0xFFFCFFFF) | ((((u32)g_field_effect_records[nA].flags.word >> 16) & 3) << 16);
                        rec->unk27 = g_field_effect_records[nA].unk27;
                        rec->unk21 = g_field_effect_records[nA].unk21;
                        res = g_field_resource_entries
                                  [D_800FDF58[g_field_actor_slots[rec->unk22].owner_object_index].unk3B]
                                      .start;
                        if (res != 0)
                        {
                            field_begin_actor_animation_forward(rec, res);
                        }
                    }
                    break;
                }
                nA++;
            } while (nA < 0x100);
        }
    scanA_done:
        if (nA == 0x100)
        {
            goto fail_slot;
        }
        func_8006D79C(actor, part_index, nA + 1);
        break;

    case 0x25:
        rec->unk0 += (part->unk38 << 8) - D_800F22A0;
        rec->unk4 += (part->unk3A << 8) - D_800F22A4;
        rec->unk8 += (part->unk3C << 8) - D_800F22A8;
        if (rec->unk25 == 0xFD)
        {
            rec->unk25 = 0xFE;
        }
        break;

    case 0x1C:
        rec->unk0 -= D_800F22A0;
        rec->unk4 -= D_800F22A4;
        rec->unk8 -= D_800F22A8;
        rec->flags.word |= 0x1000;
        break;

    case 0x1D:
        rec->unk4 -= 0x7000;
        rec->unk0 -= D_800F22A0;
        rec->unk8 -= D_800F22A8;
        rec->flags.word |= 0x1000;
        rec->unk4 -= D_800F22A4;
        break;

    case 0x1E:
        rec->unk4 += 0x7000;
        rec->unk0 -= D_800F22A0;
        rec->unk8 -= D_800F22A8;
        rec->flags.word |= 0x1000;
        rec->unk4 -= D_800F22A4;
        break;

    case 0x1F:
        rec->unk0 += 0xFFFF6000;
        rec->unk0 -= D_800F22A0;
        rec->unk4 -= D_800F22A4;
        rec->unk8 -= D_800F22A8;
        rec->flags.word |= 0x1000;
        break;

    case 0x20:
        rec->unk0 += 0xA000;
        rec->unk0 -= D_800F22A0;
        rec->unk4 -= D_800F22A4;
        rec->unk8 -= D_800F22A8;
        rec->flags.word |= 0x1000;
        break;

    case 0x21:
        rec->unk0 += 0xFFFF6000;
        rec->unk4 -= 0x7000;
        rec->unk0 -= D_800F22A0;
        rec->unk8 -= D_800F22A8;
        rec->flags.word |= 0x1000;
        rec->unk4 -= D_800F22A4;
        break;

    case 0x22:
        rec->unk0 += 0xA000;
        rec->unk4 -= 0x7000;
        rec->unk0 -= D_800F22A0;
        rec->unk8 -= D_800F22A8;
        rec->flags.word |= 0x1000;
        rec->unk4 -= D_800F22A4;
        break;

    case 0x23:
        rec->unk0 += 0xFFFF6000;
        rec->unk4 += 0x7000;
        rec->unk0 -= D_800F22A0;
        rec->unk8 -= D_800F22A8;
        rec->flags.word |= 0x1000;
        rec->unk4 -= D_800F22A4;
        break;

    case 0x24:
        rec->unk0 += 0xA000;
        rec->unk4 += 0x7000;
        rec->unk0 -= D_800F22A0;
        rec->unk8 -= D_800F22A8;
        rec->flags.word |= 0x1000;
        rec->unk4 -= D_800F22A4;
        break;

    case 0x27:
        {
            Struct_D80105AE0 *table_base = D_80105AE0;
            slot27_guard = &table_base[actor->owner_object_index];
        }
        if ((*(u8*)&slot27_guard->state.word & 1) && slot27_guard->state.bytes[2] != actor->unk233 &&
            actor->unk233 >= 0x40U && !(((u32)slot27_guard->state.word >> 5) & 1))
        {
            goto fail_slot;
        }
        src = &D_800FDF58[actor->owner_object_index];
        if (((part->unk28 >> 10) & 1) && !(src->unk21 & 0x80))
        {
            rec->unk0 += src->unk0 - (part->unk38 << 8);
        }
        else
        {
            rec->unk0 += src->unk0 + (part->unk38 << 8);
        }
        rec->unk4 += src->unk4 + (part->unk3A << 8);
        rec->unk8 += src->unk8 + (part->unk3C << 8);
        if (rec->unk25 == 0xFD && rec->unk21 == 0xFF)
        {
            rec->unk3A = src->unk3A;
            {
                Struct_D80105AE0 *table_base = D_80105AE0;
                slot27 = &table_base[src->unk3A];
            }
            if (*(u8*)&slot27->state.word & 1)
            {
                if (slot27->state.bytes[2] != actor->unk233)
                {
                    goto mark_dead;
                }
            }
            rec->unk3B = src->unk3B;
            rec->unkC = src->unkC;
            rec->unk21 = src->unk21;
            rec->unk27 = src->unk27;
            rec->flags.word = (rec->flags.word & 0xFF87FFFF) | (src->flags.word & 0x780000);
            rec->flags.word = (rec->flags.word & 0xFFFCFFFF) | ((src->flags.half[1] & 3) << 16);
            res = g_field_resource_entries[src->unk3B].start;
            if (res != 0)
            {
                field_restart_actor_animation(rec, res);
            }
            break;
        }
        break;

    case 0x28: {
        if (actor->unk229[g_field_track_index] == 0xFF)
        {
            rec->unk25 = 0xFF;
            actor->unk3B[g_field_track_index][part_index]--;
            actor->unkCC[g_field_track_index][part_index]--;
            return -1;
        }
        src = &D_800FDF58[actor->unk229[g_field_track_index]];
        if ((part->unk34 & 0x08000000) && !(D_800FDF58[actor->owner_object_index].unk21 & 0x80))
        {
            rec->unk0 += src->unk0 - (part->unk38 << 8);
        }
        else if (((part->unk28 >> 10) & 1) && !(src->unk21 & 0x80))
        {
            rec->unk0 += src->unk0 - (part->unk38 << 8);
        }
        else
        {
            rec->unk0 += src->unk0 + (part->unk38 << 8);
        }
        rec->unk4 += src->unk4 + (part->unk3A << 8);
        rec->unk8 += src->unk8 + (part->unk3C << 8);
        if (rec->unk25 == 0xFD && rec->unk21 == 0xFF)
        {
            rec->unk3A = src->unk3A;
            {
                Struct_D80105AE0 *table_base = D_80105AE0;
                slot28 = &table_base[src->unk3A];
            }
            if (*(u8*)&slot28->state.word & 1)
            {
                if (slot28->state.bytes[2] != actor->unk233)
                {
                    goto mark_dead;
                }
            }
            rec->unk3B = src->unk3B;
            rec->unkC = src->unkC;
            rec->unk21 = src->unk21;
            rec->unk27 = src->unk27;
            rec->flags.word = (rec->flags.word & 0xFF87FFFF) | (src->flags.word & 0x780000);
            rec->flags.word = (rec->flags.word & 0xFFFCFFFF) | ((src->flags.half[1] & 3) << 16);
            res = g_field_resource_entries[src->unk3B].start;
            if (res != 0)
            {
                field_restart_actor_animation(rec, res);
            }
            break;
        }
        break;

    }
    case 0x29:
        slot = &D_80105AE0[actor->owner_object_index];
        src = &D_800FDF58[actor->owner_object_index];
        rec->unk0 += src->unk0 + (slot->unk130[(part->unk24 >> 21) & 3].x << 8);
        rec->unk4 += src->unk4 + (slot->unk130[(part->unk24 >> 21) & 3].y << 8);
        rec->unk8 += src->unk8;
        if (rec->unk25 == 0xFD)
        {
            rec->unk3A = src->unk3A;
            rec->flags.word = (rec->flags.word & 0xFF87FFFF) | (src->flags.word & 0x780000);
            rec->flags.word = (rec->flags.word & 0xFFFCFFFF) | ((src->flags.half[1] & 3) << 16);
            {
                Struct_D80105AE0 *table_base = D_80105AE0;
                slot_owner_valid = &table_base[src->unk3A];
            }
            goto check_owner;
        }
        break;

    case 0x32:
        {
            Struct_D80105AE0 *table_base = D_80105AE0;
            slot = &table_base[actor->owner_object_index];
        }
        src = &D_800FDF58[actor->owner_object_index];
        rec->unk0 += src->unk0 + (slot->unk130[(part->unk24 >> 21) & 3].x << 8);
        rec->unk4 += src->unk4;
        rec->unk8 += src->unk8 + (part->unk3C << 8);
        if (((part->unk28 >> 10) & 1) && !(src->unk21 & 0x80))
        {
            rec->unk0 = rec->unk0 - (part->unk38 << 8);
        }
        else
        {
            rec->unk0 = rec->unk0 + (part->unk38 << 8);
        }
        if (rec->unk25 == 0xFD)
        {
            rec->flags.word = (rec->flags.word & 0xFF87FFFF) | (src->flags.word & 0x780000);
            rec->flags.word = (rec->flags.word & 0xFFFCFFFF) | ((src->flags.half[1] & 3) << 16);
            rec->unk3A = src->unk3A;
            {
                Struct_D80105AE0 *table_base = D_80105AE0;
                slot_owner_valid = &table_base[src->unk3A];
            }
        check_owner:
            if (!(*(u8*)&slot_owner_valid->state.word & 1) || slot_owner_valid->state.bytes[2] == actor->unk233)
            {
                rec->unk25 = 2;
                if (rec->unk21 == 0xFF)
                {
                    rec->unk3B = src->unk3B;
                    rec->unkC = src->unkC;
                    rec->unk21 = src->unk21;
                    rec->unk27 = src->unk27;
                }
                res = g_field_resource_entries[src->unk3B].start;
            maybe_attach:
                if (res != 0)
                {
                    field_restart_actor_animation(rec, res);
                }
                break;
            }
            goto mark_dead;
        }
        break;

    case 0x33:
        slot = &D_80105AE0[actor->owner_object_index];
        src = &D_800FDF58[actor->owner_object_index];
        rec->unk0 += src->unk0 + (slot->unk190[D_80105760].x << 8);
        rec->unk4 += src->unk4;
        rec->unk8 += src->unk8 + (slot->unk190[D_80105760].y << 8);
        rec->flags.bits.placement = *(u16*)&D_80105760;
        break;

    case 0x34: {
        if (actor->unk229[g_field_track_index] == 0xFF)
        {
            rec->unk25 = 0xFF;
            actor->unk3B[g_field_track_index][part_index]--;
            actor->unkCC[g_field_track_index][part_index]--;
            return -1;
        }
        src = &D_800FDF58[actor->unk229[g_field_track_index]];
        if (!(src->unk21 & 0x80))
        {
            rec->unk0 += src->unk0 + (actor->unk1FE[g_field_track_index].x << 8);
        }
        else
        {
            rec->unk0 += src->unk0 - (actor->unk1FE[g_field_track_index].x << 8);
        }
        rec->unk4 += src->unk4 + (actor->unk1FE[g_field_track_index].y << 8);
        rec->unk8 += src->unk8;
        break;

    }
    case 0x35:
        rec->unk0 += D_80105778.vx;
        rec->unk4 += D_80105778.vy;
        rec->unk8 += D_80105778.vz;
        break;

    case 0x36:
        rec->unk0 += part->unk38 << 8;
        rec->unk4 += part->unk3A << 8;
        rec->unk8 += part->unk3C << 8;
    check_dead:
        if (rec->unk25 == 0xFD)
        {
        mark_dead:
            rec->unk25 = 0xFE;
        }
        break;

    scanB_copy:
        rec->unk21 = g_field_effect_records[nB].unk21;
        rec->unk27 = g_field_effect_records[nB].unk27;
        rec->unk34 = g_field_effect_records[nB].unk34;
        rec->unk35 = g_field_effect_records[nB].unk35;
        rec->unk29 = g_field_effect_records[nB].unk29;
        rec->unk36 = g_field_effect_records[nB].unk36;
        rec->unk37 = g_field_effect_records[nB].unk37;
        rec->unk38 = g_field_effect_records[nB].unk38;
        goto scanB_done;

    case 0x37:
    case 0x38:
    case 0x39:
    case 0x3A:
    case 0x3B:
    case 0x3C:
    case 0x3D:
    case 0x3E:
        nB = start;
        count = kind - 0x37;
        if (nB < 0x100)
        {
            do
            {
                if (g_field_effect_records[nB].unk25 != 0xFF && g_field_effect_records[nB].unk23 == count && g_field_effect_records[nB].unk22 == actor->unk233 &&
                    ((actor->unk0[count].unk14 & 4) || g_field_effect_records[nB].unk29 == g_field_track_index))
                {
                    rec->unk0 += g_field_effect_records[nB].unk0;
                    rec->unk4 += g_field_effect_records[nB].unk4;
                    rec->unk8 += g_field_effect_records[nB].unk8;
                    if ((part->unk34 & 0x08000000) && !(D_800FDF58[actor->owner_object_index].unk21 & 0x80))
                    {
                        rec->unk0 -= part->unk38 << 8;
                    }
                    else if (((part->unk28 >> 10) & 1) && !(g_field_effect_records[nB].unk21 & 0x80))
                    {
                        rec->unk0 -= part->unk38 << 8;
                    }
                    else
                    {
                        rec->unk0 += part->unk38 << 8;
                    }
                    rec->unk4 += part->unk3A << 8;
                    rec->unk8 += part->unk3C << 8;
                    rec->flags.word = (rec->flags.word & ~0x1000) | (g_field_effect_records[nB].flags.word & 0x1000);
                    if (part->unk1C & 0x08000000)
                    {
                        rec->unk10 = ((FieldSpawnSVector*)&g_field_effect_records[nB].unk10)->unk0;
                        rec->unk12 = ((FieldSpawnSVector*)&g_field_effect_records[nB].unk10)->unk2;
                        rec->unk14 = ((FieldSpawnSVector*)&g_field_effect_records[nB].unk10)->unk4;
                    }
                    rec->unk30 = nB;
                    if (rec->unk25 == 0xFD)
                    {
                        rec->unk3B = g_field_effect_records[nB].unk3B;
                        rec->unkC = g_field_effect_records[nB].unkC;
                        rec->unk25 = g_field_effect_records[nB].unk25;
                        rec->unk3A = g_field_effect_records[nB].unk3A;
                        rec->flags.word = (rec->flags.word & 0xFF87FFFF) | (((u32)g_field_effect_records[nB].flags.word >> 19 & 15) << 19);
                        rec->flags.word = (rec->flags.word & 0xFFFCFFFF) | ((((u32)g_field_effect_records[nB].flags.word >> 16) & 3) << 16);
                        if (rec->unk21 == 0xFF)
                        {
                            goto scanB_copy;
                        }
                        rec->unk3A = g_field_effect_records[nB].unk3A;
                        rec->unk25 = g_field_effect_records[nB].unk25;
                        rec->flags.word = (rec->flags.word & 0xFF87FFFF) | (((u32)g_field_effect_records[nB].flags.word >> 19 & 15) << 19);
                        rec->flags.word = (rec->flags.word & 0xFFFCFFFF) | ((((u32)g_field_effect_records[nB].flags.word >> 16) & 3) << 16);
                        rec->unk27 = g_field_effect_records[nB].unk27;
                        rec->unk21 = g_field_effect_records[nB].unk21;
                        res = g_field_resource_entries
                                  [D_800FDF58[g_field_actor_slots[rec->unk22].owner_object_index].unk3B]
                                      .start;
                        if (res != 0)
                        {
                            field_begin_actor_animation_forward(rec, res);
                        }
                    }
                    break;
                }
                nB++;
            } while (nB < 0x100);
        }
    scanB_done:
        if (nB != 0x100)
        {
            goto scanB_recurse;
        }
    fail_slot:
        rec->unk25 = 0xFF;
        actor->unk3B[g_field_track_index][part_index]--;
    drop_slot:
        actor->unkCC[g_field_track_index][part_index]--;
        return -1;
    scanB_recurse:
        func_8006D79C(actor, part_index, nB + 1);
        break;
    }

        switch ((part->unk24 >> 29) & 3)
        {
        case 1:
            vec->vx = (D_800FDF58[actor->owner_object_index].unk0 - rec->unk0) >> 8;
            vec->vy = (D_800FDF58[actor->owner_object_index].unk4 - rec->unk4) >> 8;
            direction_z = (vec->vz = (D_800FDF58[actor->owner_object_index].unk8 - rec->unk8) >> 8);
            rec->unk12 = ratan2(-direction_z, vec->vx);
            GTE_LDLVL(vec);
            GTE_SQR0();
            GTE_STLVNL(sqr);
            rec->unk14 = ratan2(SquareRoot0(sqr->vx + sqr->vz), -vec->vy);
            rec->unk10 = 0;
            rec->unk0 = D_800FDF58[actor->owner_object_index].unk0;
            rec->unk4 = D_800FDF58[actor->owner_object_index].unk4;
            rec->unk8 = D_800FDF58[actor->owner_object_index].unk8;
            break;
        case 2:
            vec->vx = (D_800FDF58[actor->unk229[g_field_track_index]].unk0 - rec->unk0) >> 8;
            vec->vy = (D_800FDF58[actor->unk229[g_field_track_index]].unk4 - rec->unk4) >> 8;
            direction_z = (D_800FDF58[actor->unk229[g_field_track_index]].unk8 - rec->unk8) >> 8;
            vec->vz = direction_z;
            rec->unk12 = ratan2(-direction_z, vec->vx);
            GTE_LDLVL(vec);
            GTE_SQR0();
            GTE_STLVNL(sqr);
            rec->unk14 = ratan2(SquareRoot0(sqr->vx + sqr->vz), -vec->vy);
            rec->unk10 = 0;
            rec->unk0 = D_800FDF58[actor->unk229[g_field_track_index]].unk0;
            rec->unk4 = D_800FDF58[actor->unk229[g_field_track_index]].unk4;
            rec->unk8 = D_800FDF58[actor->unk229[g_field_track_index]].unk8;
            break;
        }
        if ((rec->flags.word & 0x07000000) == 0x05000000)
        {
            func_800A1D98(rec, func_8007E754(actor, part), (part->unk28 >> 15) & 1, D_80105770);
            rec->unk20 = 0;
            rec->unk39 = D_80105770;
            func_800A1D48(&rec->unk20, rec, D_80105770);
            D_80105770 = D_80105770 + 1;
            if (D_80105770 == 0x20)
            {
                D_80105770 = 0;
            }
        }
        if (part->unk1C & 0x10000000)
        {
            field_swap_effect_position_source(rec, part);
        }
        if (!(rec->flags.word & 0x07000000) && rec->unk1B != 0)
        {
            func_80070EF0(rec, part);
        }
        if ((part->unk28 >> 3) & 1)
        {
            if (part->unk34 & 0x80000)
            {
                rec->unk26 = rec->unk4 >> 8;
                rec->unk4 -= field_evaluate_parameter_track_at_time(actor, *(u8*)&part->unk28 >> 4, 0) << 8;
            }
            else
            {
                rec->unk4 = (-field_evaluate_parameter_track_at_time(actor, (part->unk28 >> 4) & 0xF, 0)) << 8;
            }
        }
        if (part->unk28 & 1)
        {
            rec->unk8 += 0x80;
        }
        switch ((s32)(((u32)rec->flags.word >> 29) & 3))
        {
        case 1:
            rec->unk2A = field_evaluate_parameter_track(actor, ((u16*)&part->unk14)[1] & 0xF);
            break;
        case 2:
            rec->unk2A = field_evaluate_parameter_track_at_time(actor, ((u16*)&part->unk14)[1] & 0xF, 0);
            break;
        case 3:
            field_resolve_effect_position(rec, part, vec);
            vec->vx -= rec->unk0;
            vec->vy -= rec->unk4;
            vec->vz -= rec->unk8;
            vec->vx >>= 8;
            vec->vy >>= 8;
            vec->vz >>= 8;
            GTE_LDLVL(vec);
            GTE_SQR0();
            GTE_STLVNL(sqr);
            { s32 distance = SquareRoot0(sqr->vx + sqr->vy + sqr->vz) << 8;
            if (((s16*)&part->unk14)[1] != 0)
            {
                rec->unk2A = (u32)(distance / ((s16*)&part->unk14)[1]) >> 2;
            }
            else
            {
                rec->unk2A = (u32)distance >> 2;
            }
            }
            break;
        }
        if (part->unk34 & 0x08000000)
        {
            rec->unk21 &= 0x7F;
        }
        if ((((part->unk28 >> 10) & 1) || (part->unk34 & 0x08000000)) &&
            !(D_800FDF58[actor->owner_object_index].unk21 & 0x80))
        {
            rec->unk21 ^= 0x80;
        }
        if ((part->unk28 >> 14) & 1)
        {
            if (base_x < 0)
            {
                rec->unk21 ^= 0x80;
            }
            if (part->unk9 == 0)
            {
                rec->unk21 = (rec->unk21 & 0x7F) | (D_800FDF58[actor->owner_object_index].unk21 & 0x80);
            }
        }
        if (part->unk34 & 0x200000)
        {
            rec->unk21 ^= 0x80;
        }
        if ((actor->unk224 & 0x1E) == 8 && ((part->unk28 >> 18) & 0x3F) == 0x33)
        {
            D_80105760 = D_80105760 + 1;
            if (D_80105760 < 3)
            {
                actor->unk3B[g_field_track_index][part_index]--;
                actor->unkCC[g_field_track_index][part_index]--;
                if (func_8006D79C(actor, part_index, 0) == -1)
                {
                    actor->unk3B[g_field_track_index][part_index]++;
                    actor->unkCC[g_field_track_index][part_index]++;
                }
            }
        }
        if ((u32)((rec->unk25 + 2) & 0xFF) >= 2U)
        {
            if (actor->unk229[g_field_track_index] != 0xFF)
            {
                if ((actor->unkC->unk18 & 0x14) == 0x14 && (actor->unkC->unk18 >> 12) == part_index)
                {
                    D_800FDF58[actor->unk229[g_field_track_index]].unk25 = 0xFE;
                    D_80105AE0[actor->unk229[g_field_track_index]].state.word |= 1;
                    D_80105AE0[actor->unk229[g_field_track_index]].state.bytes[2] = actor->unk233;
                    ((u8*)&actor->unk224)[1] = 1;
                }
            }
            if ((actor->unkC->unk18 & 0xA) == 0xA)
            {
                if (((actor->unkC->unk18 >> 8) & 0xF) == part_index)
                {
                    D_800FDF58[actor->owner_object_index].unk25 = 0xFE;
                    D_80105AE0[actor->owner_object_index].state.word |= 1;
                    D_80105AE0[actor->owner_object_index].state.bytes[2] = actor->unk233;
                }
            }
        }
        field_dispatch_actor_audio_event(actor, 2, part_index);
        field_dispatch_actor_audio_event(actor, 5, part_index);
        return i;


}

#include "field_effect_types.h"


typedef struct
{
    u8 pad0[0x10];
    s16 unk10; /* 0x10 */
    s16 unk12; /* 0x12 */
    s16 unk14; /* 0x14 */
} FieldTrackResult;

/** @brief Four-halfword GTE vector; the fourth halfword is padding. */
typedef struct
{
    s16 x;
    s16 y;
    s16 z;
    s16 pad;
} FieldSVector;

/**
 * @brief World-position footprint queried against field collision bounds.
 * @note Matches FieldCollisionQuery in field_collision.c: X/Z extents and Y tolerance.
 */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
    u16 width;
    s16 height_tolerance;
    u16 depth;
} FieldEffectCollisionQuery;

/**
 * @brief Position, step, and contact state resolved by func_8005B6AC.
 * @note The low halfword of mode is footprint depth, not a status code.
 * Bits 16/17 gate special resolution paths; their distinct meanings are unknown.
 */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
    s32 move_x;
    s32 move_y;
    s32 move_z;
    s32 resolved_height;
    void* collision_node;
    s32 contact_flags;
    u16 width;
    s16 height_tolerance;
    union
    {
        u32 word;
        struct
        {
            u32 depth : 16;
            u32 unknown_16 : 1;
            u32 unknown_17 : 1;
            u32 unknown_high : 14;
        } bits;
    } mode;
} FieldEffectCollisionMover;

/** @brief Per-slot 0x268-byte view of byte counters selected by reward kind. */
typedef struct
{
    u8 pad0[0x244];
    u8 counters[0x24];
} FieldCounterView;

/** @brief Selected map dimensions used for effect movement bounds. */
typedef struct
{
    s16 width;
    s16 height;
} FieldEffectMapBounds;

/** @brief Word-wide camera translation at 0x801ED480, also used by FieldCamera. */
typedef struct
{
    u8 pad0[4];
    s32 x;
    s32 y;
    s32 z;
} FieldEffectCamera;

#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

extern s32 g_field_track_index;
extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;
extern FieldActorState g_field_actor_slots[80];
extern FieldMotionRecord D_800FDF58[];
extern FieldObjectPlacement D_80105AE0[];
extern FieldVector D_80105778;
extern s32 D_80105760;
/**
 * @brief Packed action context: action in low bits, source at bit 8, recipient at bit 16.
 * @note Also written on pool exhaustion and advanced during collision attempts;
 * the broader protocol of those writes is still unresolved.
 */
extern s32 g_field_action_context;
void field_set_action_context(s32 recipient_id, s32 source_id, s32 action);
extern s32 D_800FE754;
/** @brief Suppress repeated pickup audio until the next frame-command build. */
extern s32 g_field_pickup_sound_played;
extern FieldCounterView D_800FD818[];

/**
 * @brief Roll one particle spawn record's scale and rotation fields from a
 *        part's parameter tracks, falling back to fixed part values or a
 *        random roll where a track is not assigned.
 * @param arg0 Owning actor, indexes g_field_track_index into unkCC/unk3B.
 * @param arg1 Part definition supplying the track selectors and fallback
 *             values (unk1E divisor, unk32 timing-table index, unk9/unkA
 *             track selectors).
 * @param arg2 Output record to fill in (unk10/unk12/unk14).
 * @return Scaled track value added to arg2->unk14.
 */
s32 func_80070CB8(FieldActorState *arg0, FieldActorPartDef *arg1, FieldTrackResult *arg2)
{
    s16 var_v0;
    s32 var_lo;
    s32 track_value;
    s32 temp_s0;

    arg2->unk10 = 0;
    if (arg1->unknown_0x1e != 0)
    {
        var_v0 = ((0x1000 / arg1->unknown_0x1e) * arg0->track_counters[g_field_track_index][arg1->unknown_0x32] + 0x400) & 0xFFF;
        arg2->unk12 = var_v0;
    }
    else
    {
        var_v0 = (u32) rand() >> 3;
        arg2->unk12 = var_v0;
    }
    arg0->track_counters[g_field_track_index][arg1->unknown_0x32]++;

    if ((arg1->track_flags >> 0xB) & 1)
    {
        temp_s0 = field_evaluate_parameter_track(arg0, arg1->unknown_0x9 & 0xF);
        var_lo = temp_s0 * (rand() << 3);
    }
    else
    {
        var_lo = arg1->unknown_0x9 * (rand() << 3);
    }
    arg2->unk14 = var_lo >> 0xF;

    if ((arg1->track_flags >> 0xC) & 1)
    {
        track_value = field_evaluate_parameter_track(arg0, arg1->unknown_0xa & 0xF);
    }
    else
    {
        track_value = arg1->unknown_0xa;
    }
    track_value *= 8;
    arg2->unk14 += track_value;
    return track_value;
}

/**
 * @brief Move an effect to its source and preserve its old position as the new source.
 * @param rec Record whose position and position-source selection are exchanged.
 * @param part Part definition controlling the selected position source.
 * @see decomp.me (100%) TODO
 */
void field_swap_effect_position_source(FieldMotionRecord *rec, FieldActorPartDef *part)
{
    FieldVector new_pos;
    s32 unused[2];

    if (rec->position_source != FIELD_POSITION_NONE)
    {
        field_resolve_effect_position(rec, part, &new_pos);
        rec->position_source = FIELD_POSITION_SAVED;
        rec->work_x = rec->x + D_800F22A0;
        rec->work_y = rec->y + D_800F22A4;
        rec->work_z = rec->z + D_800F22A8;
        rec->x = new_pos.vx;
        rec->y = new_pos.vy;
        rec->z = new_pos.vz;
    }
}

/**
 * @brief Face a record toward the delta between its last stored position and
 *        a freshly rolled position, deriving both a horizontal-plane heading
 *        (unk12) and a vertical pitch (unk14).
 * @param rec Record whose heading/pitch (unk12/unk14) are updated.
 * @param part Passed through unchanged to field_resolve_effect_position.
 * @see decomp.me (100%) TODO
 */
void func_80070EF0(FieldMotionRecord *rec, FieldActorPartDef *part)
{
    FieldVector new_pos;
    FieldVector vec;
    FieldVector sqr;

    field_resolve_effect_position(rec, part, &new_pos);
    vec.vx = (new_pos.vx - rec->x) >> 8;
    vec.vy = (new_pos.vy - rec->y) >> 8;
    vec.vz = (new_pos.vz - rec->z) >> 8;

    gte_ldlvl(&vec);
    gte_sqr0();
    gte_stlvnl(&sqr);

    rec->heading = ratan2(-vec.vz, vec.vx);
    if (rec->heading < 0)
    {
        rec->heading += 0x1000;
    }

    if (vec.vy != 0)
    {
        rec->pitch = ratan2(SquareRoot0(sqr.vx + sqr.vz), -vec.vy);
    }
    else
    {
        rec->pitch = 0x400;
    }
    if (rec->pitch < 0)
    {
        rec->pitch += 0x1000;
    }
    rec->rotation_x = 0;
}

typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} RECT;

/** @brief Rotation member of successive 0x54-byte effect records, viewed from +0x10. */
typedef struct
{
    FieldSVector angles;
    u8 pad8[0x54 - 8];
} FieldEffectRotationEntry;

extern FieldMotionRecord g_field_effect_records[];
/** @brief Alias of g_field_effect_records + 0x10; preserves the original rotation-base relocation. */
extern FieldEffectRotationEntry D_800FF668[];
extern u8 D_80104B58[];
extern u8 D_80105358[];

void field_update_effect_record(FieldMotionRecord *rec, FieldActorPartDef *part, FieldActorState *actor);

/**
 * @brief Per-frame actor tick: advances every active effect record owned by
 *        this actor (culling off-screen ones, spawning chained effects on
 *        expiry), then refreshes the actor's palette-track texture pages.
 * @param arg0 Actor being ticked.
 * @see decomp.me (80.28%) TODO
 * @note WIP - 80.28% (212/312 rows). Two residues, both investigated at
 *       length without a clean fix:
 *       (1) the g_field_effect_records[rec->previous_effect_index] "previous record" address is
 *       recomputed from scratch by the target FOUR separate times (once per
 *       field read/write) in straight-line code with no intervening call or
 *       branch; plain C (even with freshly-named index locals each time)
 *       always lets gcc 2.7.2's cse.c merge these into one computation. The
 *       `FieldMotionRecord * volatile pv` reassigned before each use forces
 *       the recompute (pv is `volatile`, but the round-trip through its own
 *       stack home does not exactly match the target's register-only
 *       rederivation) - this closed most but not all of the gap.
 *       (2) the D_800F22A0/A4/A8 sign-fixup chain (building sx/sy) has a
 *       handful of residual sched1/regalloc-order rows that did not respond
 *       to reassociating the sums or reordering the fixups.
 *       A fresh source-model attempt, or sched_oracle on the sign-fixup
 *       block, would be the next lever - not attempted this session.
 */
void func_8007100C(FieldActorState *arg0_param)
{
    FieldActorState *arg0;
    FieldMotionRecord *rec;
    FieldActorPartDef *part;
    void *buf;
    s32 i;
    s32 count;
    s32 newslot;
    FieldMotionRecord *newrec;
    s32 v0, v1, a0, v0_2, a0_2, v1_2;
    s16 sx, sy;
    u8 t;
    s32 dx, dy, dz;
    FieldMotionRecord * volatile pv;
    s32 unused_pad[2];
    s32 sp20, sp24, sp28;
    s32 out_vec[3];

    arg0 = arg0_param;
    for (rec = g_field_effect_records; rec != &g_field_effect_records[256]; rec++)
    {
        if (rec->actor_index == arg0->actor_index && rec->state != 0xFF)
        {
            part = &arg0->parts[rec->part_index];
            g_field_track_index = rec->track_index;
            field_update_effect_record(rec, part, arg0);
            rec->age++;
            if (((part->behavior_flags >> 4) & 3) == 1 && (u16) rec->age == rec->lifetime)
            {
                t = rec->state;
                rec->state = 0xFF;
                rec->saved_state = t;
            }
            if (((part->behavior_flags >> 4) & 3) == 3)
            {
                v0 = D_800F22A0;
                if (v0 < 0)
                {
                    v0 += 0xFF;
                }
                v1 = rec->x;
                if (v1 < 0)
                {
                    v1 += 0xFF;
                }
                a0 = D_800F22A4;
                sx = (v0 >> 8) + (v1 >> 8) + 0xA0;
                if (a0 < 0)
                {
                    a0 += 0xFF;
                }
                v0_2 = rec->y;
                if (v0_2 < 0)
                {
                    v0_2 += 0xFF;
                }
                a0_2 = rec->z;
                if (a0_2 < 0)
                {
                    a0_2 += 0x1FF;
                }
                v1_2 = D_800F22A8;
                if (v1_2 < 0)
                {
                    v1_2 += 0x1FF;
                }
                sy = ((a0 >> 8) + (v0_2 >> 8) + 0x70) - (a0_2 >> 9) - (v1_2 >> 9);
                if ((u32) ((sx + 0x140) & 0xFFFF) >= 0x3C1 || sy >= 0x1E1 || sy < -0xF0)
                {
                    t = rec->state;
                    rec->state = 0xFF;
                    rec->saved_state = t;
                }
            }
            if (rec->state == 0xFF)
            {
                func_80071500(rec, part);
            }
            if (part->effect_flags < 0 && rec->state != 0xFF)
            {
                newslot = func_8006D79C(arg0, part->unknown_0x23 & 0xF, 0);
                if (newslot != -1)
                {
                    newrec = &g_field_effect_records[newslot];
                    newrec->x = rec->x;
                    newrec->y = rec->y;
                    newrec->z = rec->z;
                    pv = &g_field_effect_records[rec->previous_effect_index];
                    dx = rec->x - pv->x;
                    sp20 = dx;
                    pv = &g_field_effect_records[rec->previous_effect_index];
                    dy = rec->y - pv->y;
                    sp24 = dy;
                    pv = &g_field_effect_records[rec->previous_effect_index];
                    dz = rec->z - pv->z;
                    sp28 = dz;
                    pv = &g_field_effect_records[rec->previous_effect_index];
                    pv->next_effect_index = newslot;
                    sp28 = 0;
                    sx = (s16) (dx >> 8);
                    v0_2 = dz >> 9;
                    sy = (s16) ((dy >> 8) - v0_2);
                    sp20 = sy;
                    sp24 = -(s32) sx;
                    func_8001CDAC(&sp20, out_vec, v0_2);
                    newrec->rotation_x = (s16) (out_vec[0] >> 6);
                    newrec->heading = (s16) (out_vec[1] >> 6);
                    newrec->work_x = 0;
                    newrec->work_y = 0;
                    newrec->pitch = (s16) (out_vec[2] >> 6);
                    newrec->next_effect_index = 0xFF;
                    newrec->previous_effect_index = rec->previous_effect_index;
                    rec->previous_effect_index = newslot;
                }
            }
        }
    }

    if (arg0->owner_object_index < 2)
    {
        buf = &D_80104B58[arg0->owner_object_index << 0xA];
    }
    else
    {
        buf = D_80105358;
    }
    count = 0;
    newslot = 0;
    if (arg0->part_count != 0)
    {
        i = 0;
        do
        {
            part = &arg0->parts[i];
            if ((part->track_flags >> 0x15) & 1)
            {
                newslot++;
                field_interpolate_palette_track(arg0, part->unknown_0x1c, buf, (u8 *) buf + 0x200);
            }
            count++;
            i++;
        } while (count < arg0->part_count);
    }
    if (newslot != 0)
    {
        RECT rect;

        if (arg0->owner_object_index < 2)
        {
            rect.x = 0;
            rect.w = 0x10;
            rect.h = 1;
            rect.y = (arg0->owner_object_index * 2) + 0x1EF;
        }
        else
        {
            rect.y = 0x1F3;
            rect.w = 0x10;
            rect.x = 0;
            rect.h = 1;
        }
        LoadImage(&rect, (u8 *) buf + 0x200);
    }
    func_8007FFC8(arg0);
    func_8008332C(arg0, arg0->parts, arg0->part_count);
}

/**
 * @brief Per-effect-record update: decrements the part's active-count table
 *        entry, dispatches an audio event for the owning actor, rolls a set
 *        of chained spawn effects selected by the part's flag/track fields,
 *        then syncs the owning actor's saved position/flag mirrors
 *        (D_800FDF58/D_80105AE0) when this record's part index matches one
 *        of the actor's per-slot animation-frame triggers.
 * @param rec Effect record being updated (owner/track/position fields).
 * @param part Part definition supplying flags, track selectors, and the
 *             chained-effect table used while rec->state is being armed.
 * @see decomp.me (100%)
 */
void func_80071500(FieldMotionRecord* rec, FieldActorPartDef* part)
{
    FieldActorState* state;
    FieldActorPartDef* track_part;
    FieldMotionRecord* newrec;
    FieldMotionRecord* candidate;
    FieldMotionRecord* pool;
    FieldObjectPlacement* slot;
    FieldObjectPlacement* objects;
    FieldVector new_pos;
    FieldVector vec;
    FieldVector sqr;
    s32 bit, mask;
    s32 newslot;
    s32 spawn_count;
    s32 spawn_kind;
    s16 anim_flags;
    u8 t;

    g_field_actor_slots[rec->actor_index].active_counts[rec->track_index][part->unknown_0x32]--;

    state = &g_field_actor_slots[rec->actor_index];
    field_dispatch_actor_audio_event(state, 3, rec->part_index);

    if (*(u32*)&part->unknown_0x2c & 0xF0000000)
    {
        if (rec->height_or_retired_state == -1)
        {
            rec->state = 0xFE;
        }
        else
        {
            rec->state = rec->height_or_retired_state;
        }

        pool = g_field_effect_records;
        for (bit = 0, mask = 1; bit < 4; bit++, mask <<= 1)
        {
            if ((*(u32*)&part->unknown_0x2c >> 0x1C) & mask)
            {
                D_80105778.vx = rec->x;
                D_80105778.vy = rec->y;
                D_80105760 = 0;
                D_80105778.vz = rec->z;

                track_part = &g_field_actor_slots[rec->actor_index]
                                  .parts[(part->spawn_flags.halves.part_selectors >> (bit * 4)) & 0xF];
                spawn_count = 1;
                spawn_kind = 0x35;
                if (((track_part->placement_flags >> 0x12) & 0x3F) == spawn_kind)
                {
                    if (track_part->unknown_0xc != 0)
                    {
                        spawn_count = track_part->unknown_0xc;
                    }
                }

                if (spawn_count != 0)
                {
                    do
                    {
                        newslot = func_8006D79C(&g_field_actor_slots[rec->actor_index],
                                                (part->spawn_flags.halves.part_selectors >> (bit * 4)) & 0xF, 0);
                        if (newslot != -1)
                        {
                            candidate = (FieldMotionRecord*)(newslot * (s32)sizeof(FieldMotionRecord) + (s32)pool);
                            if (!(((u8*)&candidate->flags)[3] & 7) && (candidate->position_source != 0))
                            {
                                newrec = candidate;
                                field_resolve_effect_position(newrec, part, &new_pos);
                                vec.vx = (new_pos.vx - newrec->x) >> 8;
                                vec.vy = (new_pos.vy - newrec->y) >> 8;
                                vec.vz = (new_pos.vz - newrec->z) >> 8;

                                gte_ldlvl(&vec);
                                gte_sqr0();
                                gte_stlvnl(&sqr);

                                newrec->heading = ratan2(-vec.vz, vec.vx);
                                if (newrec->heading < 0)
                                {
                                    newrec->heading += 0x1000;
                                }

                                if (vec.vy != 0)
                                {
                                    newrec->pitch = ratan2(SquareRoot0(sqr.vx + sqr.vz), -vec.vy);
                                }
                                else
                                {
                                    newrec->pitch = 0x400;
                                }
                                if (newrec->pitch < 0)
                                {
                                    newrec->pitch += 0x1000;
                                }
                                newrec->rotation_x = 0;
                            }
                        }
                        spawn_count--;
                    } while (spawn_count != 0);
                }
            }
        }
    }
    rec->state = 0xFF;

    if (((state->animation->sync_flags & 0x14) == 0x14) && ((state->animation->sync_parts >> 0xD) == rec->part_index))
    {
        if (state->track_object_indices[rec->track_index] != 0xFF)
        {
            objects = D_80105AE0;
            slot = &objects[state->track_object_indices[rec->track_index]];
            if (((u8*)&slot->state_flags)[2] == state->actor_index)
            {
                anim_flags = D_800FDF58[state->track_object_indices[rec->track_index]].motion_parameter;
                if ((anim_flags != 0x90 && anim_flags != 0x94) || (slot->object_flags & 0x200))
                {
                    D_800FDF58[state->track_object_indices[rec->track_index]].state = 0;
                }
                else
                {
                    D_800FDF58[state->track_object_indices[rec->track_index]].state = 0xFE;
                }
                D_80105AE0[state->track_object_indices[rec->track_index]].state_flags &= ~1;
            }
        }
    }

    if (((state->animation->sync_flags & 0xA) == 0xA) &&
        (((state->animation->sync_parts >> 0xA) & 7) == rec->part_index))
    {
        slot = &D_80105AE0[state->owner_object_index];
        if (((u8*)&slot->state_flags)[2] == state->actor_index)
        {
            anim_flags = D_800FDF58[state->owner_object_index].motion_parameter;
            if ((anim_flags != 0x90 && anim_flags != 0x94) || (slot->object_flags & 0x200))
            {
                D_800FDF58[state->owner_object_index].state = 0;
            }
            else
            {
                D_800FDF58[state->owner_object_index].state = 0xFE;
            }
            D_80105AE0[state->owner_object_index].state_flags &= ~1;
        }
    }

    if (rec->part_index == ((state->animation->sync_parts & 0x1F) - 1))
    {
        D_800FDF58[state->owner_object_index].x = rec->x;
        D_800FDF58[state->owner_object_index].y = rec->y;
        D_800FDF58[state->owner_object_index].z = rec->z;
        D_800FDF58[state->owner_object_index].y = 0;
        D_800FDF58[state->owner_object_index].facing_or_reward_kind =
            (D_800FDF58[state->owner_object_index].facing_or_reward_kind & 0x7F) | (rec->facing_or_reward_kind & 0x80);
    }

    if (rec->part_index == (((state->animation->sync_parts >> 5) & 0x1F) - 1))
    {
        if (state->track_object_indices[rec->track_index] != 0xFF)
        {
            D_800FDF58[state->track_object_indices[rec->track_index]].x = rec->x;
            D_800FDF58[state->track_object_indices[rec->track_index]].y = rec->y;
            D_800FDF58[state->track_object_indices[rec->track_index]].z = rec->z;
            D_800FDF58[state->track_object_indices[rec->track_index]].y = 0;
            D_800FDF58[state->track_object_indices[rec->track_index]].facing_or_reward_kind =
                (D_800FDF58[state->track_object_indices[rec->track_index]].facing_or_reward_kind & 0x7F) |
                (rec->facing_or_reward_kind & 0x80);
        }
    }

    if (rec->height_or_retired_state == 5)
    {
        t = rec->next_effect_index;
        if (t != 0xFF)
        {
            FieldMotionRecord* records = g_field_effect_records;
            s32 next_index = rec->next_effect_index;
            records[next_index].previous_effect_index = 0xFF;
        }
    }
}

/**
 * @brief Placement selector within the part flags word.
 */
typedef struct
{
    unsigned int lower : 18;
    unsigned int opcode : 6;
    unsigned int upper : 8;
} FieldPlacementBits;

/** @brief High-halfword actor action kinds handled as collectible rewards. */
typedef enum
{
    FIELD_PICKUP_EXPERIENCE_OR_CURRENCY = 31,
    FIELD_PICKUP_ITEM = 32,
    FIELD_PICKUP_RESTORE_QUARTER = 33,
    FIELD_PICKUP_RESTORE_HALF = 34
} FieldPickupAction;

/** @brief Bit positions in the part's behavior_flags word (+0x04). */
#define FIELD_PART_PITCH_ACCELERATION_BIT 2
#define FIELD_PART_GROUND_BOUNCE_BIT 6

/** @brief Masks in the part's effect_flags word (+0x24). */
#define FIELD_PART_MAP_COLLISION 0x00100000

/** @brief Bit positions in the part's placement_flags word (+0x28). */
#define FIELD_PART_HEIGHT_TRACK_BIT 3

/** @brief Masks in the part's spawn_flags word (+0x34). */
#define FIELD_PART_HEIGHT_FROM_BASE 0x00080000
#define FIELD_PART_SKIP_MAP_COLLISION 0x00800000
#define FIELD_PART_CAMERA_BOUNDS 0x01000000
#define FIELD_PART_GROUND_STOP 0x02000000
#define FIELD_PART_RETIRE_ON_COLLISION 0x10000000

#define FIELD_PICKUP_DELAY 0x10U
#define FIELD_PICKUP_DISTANCE 5
#define FIELD_PICKUP_SOUND 0x1F
#define FIELD_EFFECT_MOTION_KIND_MASK 0x07000000
#define FIELD_EFFECT_MOTION_PATH 0x05000000
#define FIELD_EFFECT_MOTION_HOMING 0x01000000
#define FIELD_EFFECT_DISTANCE_MASK 0x1FF
#define FIELD_EFFECT_ORIENTATION_LOCK 8

/** @brief Angle units used by the field rotation helpers. */
#define FIELD_ANGLE_TURN 0x1000
#define FIELD_ANGLE_HALF_TURN 0x800
#define FIELD_ANGLE_QUARTER_TURN 0x400
#define FIELD_ANGLE_MASK 0xFFF
#define FIELD_EFFECT_TURN_THRESHOLD 0x200
#define FIELD_EFFECT_TURN_STEP 0x80
#define FIELD_EFFECT_COLLISION_WIDTH 0xC
#define FIELD_EFFECT_COLLISION_DEPTH 8
#define FIELD_EFFECT_COLLISION_HEIGHT 0x10
/** @brief Skip initial surface lookup/inheritance; ordinary movement collisions still run. */
#define FIELD_EFFECT_SKIP_SURFACE_PREPASS ((void *) -2)
#define FIELD_EFFECT_CAMERA_X_MIN 0x500
#define FIELD_EFFECT_CAMERA_X_MAX 0x13B00
#define FIELD_EFFECT_CAMERA_Z_SPAN 0x1E800
#define FIELD_EFFECT_MAP_BOUNDS_ADDRESS 0x801ED400
#define FIELD_EFFECT_CAMERA_ADDRESS 0x801ED480

/** @brief Scratchpad locations shared by the effect update's GTE/collision phases. */
#define FIELD_EFFECT_ORIGIN_ADDRESS 0x1F800000
#define FIELD_EFFECT_VECTOR_ADDRESS 0x1F800010
#define FIELD_EFFECT_TARGET_ADDRESS 0x1F800020
#define FIELD_EFFECT_LOCAL_VECTOR_ADDRESS 0x1F800030
#define FIELD_EFFECT_ROTATED_VECTOR_ADDRESS 0x1F800038
#define FIELD_EFFECT_MATRIX_ADDRESS 0x1F800040
#define FIELD_EFFECT_MOVER_ADDRESS 0x1F800080
#define FIELD_EFFECT_QUERY_ADDRESS 0x1F8000C0

/**
 * @brief Advance an actor-owned effect's placement, motion, pickups, and hit contacts.
 * @param rec Active effect record; positions use eight fractional bits.
 * @param part Packed definition selecting parameter tracks and placement behavior.
 * @param actor Owner supplying track/object bindings and animation action state.
 * @note Called once per active record by func_8007100C before it increments age.
 * Attached effects rebuild their world position; free effects integrate a rotated
 * step, resolve collision, and optionally steer toward a position source.
 * @note Some work scalars serve disjoint phases to preserve original allocation.
 * @see decomp.me (100%) https://decomp.me/scratch/i1ZHZ
 */
void field_update_effect_record(FieldMotionRecord *rec, FieldActorPartDef *part, FieldActorState *actor)
{
    FieldVector *target_position;
    FieldEffectCamera *camera;
    FieldEffectMapBounds *map_bounds;
    FieldEffectCollisionMover *mover;
    FieldEffectCollisionQuery *query;
    FieldVector *work_vector;
    FieldSVector *local_vector;
    FieldSVector *rotated_vector;
    FieldMatrix *rotation;
    FieldVector *placement_origin;
    FieldMotionRecord *reference_record;
    FieldObjectPlacement *reference_object;
    u32 flags;
    u32 record_flags;
    s32 selector;
    s32 x, y, z;
    s32 offset_or_angle;
    s32 current_angle;
    s32 dx, dy;
    void *initial_surface;
    s32 state_or_delta;
    s32 flags_byte;
    s32 slot;
    s32 recipient_index;
    u8 reference_state;
    u8 retired_state;

    camera = (FieldEffectCamera *) FIELD_EFFECT_CAMERA_ADDRESS;
    map_bounds = (FieldEffectMapBounds *) FIELD_EFFECT_MAP_BOUNDS_ADDRESS;
    mover = (FieldEffectCollisionMover *) FIELD_EFFECT_MOVER_ADDRESS;
    query = (FieldEffectCollisionQuery *) FIELD_EFFECT_QUERY_ADDRESS;
    work_vector = (FieldVector *) FIELD_EFFECT_VECTOR_ADDRESS;
    target_position = (FieldVector *) FIELD_EFFECT_TARGET_ADDRESS;
    local_vector = (FieldSVector *) FIELD_EFFECT_LOCAL_VECTOR_ADDRESS;
    rotated_vector = (FieldSVector *) FIELD_EFFECT_ROTATED_VECTOR_ADDRESS;
    rotation = (FieldMatrix *) FIELD_EFFECT_MATRIX_ADDRESS;
    placement_origin = (FieldVector *) FIELD_EFFECT_ORIGIN_ADDRESS;

    /* Sample transparency and motion tracks; the caller advances age afterward. */
    flags = part->effect_flags;
    if (flags & FIELD_EFFECT_SEMITRANSPARENT)
    {
        s32 semitransparent = field_evaluate_parameter_track_at_time(actor, (flags >> 0x19) & 0xF, (u16) ((u16) rec->age)) != 0;
        rec->flags = (rec->flags & ~FIELD_EFFECT_SEMITRANSPARENT) | (semitransparent << 0x17);
    }
    if ((rec->flags & 0x60000000) == 0x40000000)
    {
        rec->motion_parameter = field_evaluate_parameter_track_at_time(actor, ((s16 *) &part->orientation_flags)[1] & 0xF, ((u16) rec->age));
    }

    /* Linked segment records update their projected work vector and return early. */
    if (rec->state == 5)
    {
        field_build_effect_part_matrix(rec, part, rotation, actor);
        gte_SetRotMatrix(rotation);
        {
            u16 segment_x = rec->heading;
            local_vector->y = 0;
            local_vector->x = segment_x;
        }
        local_vector->z = rec->rotation_x;
        gte_ldv0(local_vector);
        gte_rtv0();
        gte_stlvnl(work_vector);
        rec->work_x = work_vector->vx;
        rec->work_y = work_vector->vy;
        return;
    }

    /* Path mode advances a wrapping byte parameter and interpolates world X/Z. */
    record_flags = rec->flags;
    if ((record_flags & FIELD_EFFECT_MOTION_KIND_MASK) == FIELD_EFFECT_MOTION_PATH)
    {
        rec->position_data.path_time = rec->position_data.path_time + rec->motion_parameter;
        func_800A1D48(&rec->position_data.path_time, rec, rec->path_group);
    }

    /* Attached effects rebuild an origin, then add the rotated local displacement. */
    else if ((u32) ((record_flags >> 0x18) & 7) >= 2U)
    {
        if ((((u32) part->placement_flags >> 0x1A) & 3) == 2)
        {
            rec->rotation_z_16 = field_evaluate_parameter_track_at_time(actor, part->rotation_z_track & 0xF, ((u16) rec->age));
        }
        if ((((u32) part->placement_flags >> 0x1C) & 3) == 2)
        {
            rec->rotation_y_16 = field_evaluate_parameter_track_at_time(actor, part->rotation_y_track & 0xF, ((u16) rec->age));
        }
        if ((rec->flags & 0x600) == 0x400)
        {
            u32 selector_high = *(u32 *) &part->unknown_0x1c >> 29;
            local_vector->y = -field_evaluate_parameter_track_at_time(actor, ((((*(u32 *) &part->unknown_0x20) & 0x3F) * 8) | selector_high) & 0xF, (u16) rec->age);
        }
        else
        {
            local_vector->y = -((u16) rec->flags & FIELD_EFFECT_DISTANCE_MASK);
        }
        if ((*(u32 *) &part->unknown_0x1c) & 0x01000000)
        {
            local_vector->y = (s16) ((D_80105AE0[actor->owner_object_index].scale_percent & 0x3FF) * (s16) (u16) local_vector->y / 100);
        }
        {
            u32 bounds_flags;
            s32 placement_kind;
            bounds_flags = part->placement_flags;
            placement_kind = (bounds_flags >> 0x12) & 0x3F;
            if (placement_kind < 0x14)
            {
                if ((bounds_flags >> 0x10) & 1)
                {
                    s32 previous_distance = local_vector->y;
                    if ((u32) (placement_kind - 0xA) >= 0x1CU)
                    {
                        dy = (D_80105AE0[actor->owner_object_index].bounds_right - D_80105AE0[actor->owner_object_index].bounds_left) >> 1;
                    }
                    else
                    {
                        dy = (D_80105AE0[actor->track_object_indices[g_field_track_index]].bounds_right - D_80105AE0[actor->track_object_indices[g_field_track_index]].bounds_left) >> 1;
                    }
                    dy = abs(dy);
                    local_vector->y = previous_distance - dy;
                    bounds_flags = part->placement_flags;
                }
                if ((bounds_flags >> 0x11) & 1)
                {
                    s32 previous_distance = local_vector->y;
                    if ((u32) (((bounds_flags >> 0x12) & 0x3F) - 0xA) >= 0x1CU)
                    {
                        dy = (D_80105AE0[actor->owner_object_index].bounds_bottom - D_80105AE0[actor->owner_object_index].bounds_top) >> 1;
                    }
                    else
                    {
                        dy = (D_80105AE0[actor->track_object_indices[g_field_track_index]].bounds_bottom - D_80105AE0[actor->track_object_indices[g_field_track_index]].bounds_top) >> 1;
                    }
                    dy = abs(dy);
                    local_vector->y = previous_distance - dy;
                }
            }
        }
        local_vector->x = 0;
        local_vector->z = 0;
        rec->heading = rec->heading + rec->motion_parameter;
        RotMatrix_gte((FieldSVector *) &rec->rotation_x, rotation);
        RotMatrixZ(rec->rotation_z_16 * 0x10, rotation);
        RotMatrixY(rec->rotation_y_16 * 0x10, rotation);
        gte_SetRotMatrix(rotation);
        gte_ldv0(local_vector);
        gte_rtv0();
        gte_stsv(rotated_vector);

        /* 0..9 select owner bounds; 10..19 select the current track object bounds. */
        selector = ((FieldPlacementBits *) &part->placement_flags)->opcode;
        switch (selector)
        {
            case 0x0: case 0x1: case 0x2: case 0x3: case 0x4:
            case 0x5: case 0x6: case 0x7: case 0x8: case 0x9:
            case 0xA: case 0xB: case 0xC: case 0xD: case 0xE:
            case 0xF: case 0x10: case 0x11: case 0x12: case 0x13:
            {
                s32 offset_y;
                FieldObjectPlacement *owner;
                FieldObjectPlacement *owner_base;

                if ((s32) selector >= 0xA)
                {
                    slot = actor->track_object_indices[g_field_track_index];
                    selector -= 0xA;
                    reference_record = &D_800FDF58[slot];
                    reference_object = &D_80105AE0[slot];
                }
                else
                {
                    slot = actor->owner_object_index;
                    reference_record = &D_800FDF58[slot];
                    reference_object = &D_80105AE0[slot];
                }
                owner_base = D_80105AE0;
                owner = &owner_base[actor->owner_object_index];
                flags_byte = *(u8 *) &owner->state_flags;
                if ((flags_byte & 1) && ((u8) actor->actor_index >= 0x40U))
                {
                    if (!(((u32) owner->state_flags >> 5) & 1))
                    {
                        offset_y = 0x800000;
                        offset_or_angle = offset_y;
                        selector = -1;
                    }
                    else
                    {
                        offset_y = 0;
                        offset_or_angle = offset_y;
                    }
                }
                else
                {
                    offset_y = 0;
                    offset_or_angle = offset_y;
                }
                switch (selector)
                {
                    case 1:
                        offset_or_angle = (reference_object->bounds_right + reference_object->bounds_left) >> 1;
                        offset_y = (reference_object->bounds_bottom + reference_object->bounds_top) >> 1;
                        break;
                    case 2:
                        offset_y = 0;
                        offset_or_angle = (reference_object->bounds_right + reference_object->bounds_left) >> 1;
                        break;
                    case 3:
                        offset_y = reference_object->bounds_top;
                        offset_or_angle = (reference_object->bounds_right + reference_object->bounds_left) >> 1;
                        break;
                    case 4:
                        offset_or_angle = reference_object->bounds_left;
                        offset_y = (reference_object->bounds_bottom + reference_object->bounds_top) >> 1;
                        break;
                    case 5:
                        offset_or_angle = reference_object->bounds_right;
                        offset_y = (reference_object->bounds_bottom + reference_object->bounds_top) >> 1;
                        break;
                    case 6:
                        offset_or_angle = reference_object->bounds_left;
                        offset_y = reference_object->bounds_top;
                        break;
                    case 7:
                        offset_or_angle = reference_object->bounds_right;
                        offset_y = reference_object->bounds_top;
                        break;
                    case 8:
                        offset_or_angle = reference_object->bounds_left;
                        offset_y = reference_object->bounds_bottom;
                        break;
                    case 9:
                        offset_or_angle = reference_object->bounds_right;
                        offset_y = reference_object->bounds_bottom;
                        break;
                }
                offset_or_angle <<= 8;
                placement_origin->vx = reference_record->x + offset_or_angle;
                offset_y <<= 8;
                placement_origin->vy = reference_record->y + offset_y;
                placement_origin->vz = reference_record->z;
                break;
            }
            case 0x14: case 0x15: case 0x16: case 0x17: case 0x18:
            case 0x19: case 0x1A: case 0x1B: case 0x2A: case 0x2B:
            case 0x2C: case 0x2D: case 0x2E: case 0x2F: case 0x30:
            case 0x31:
            {
                FieldMotionRecord *effect;
                FieldMotionRecord *effect_base;
                effect_base = g_field_effect_records;
                effect = &effect_base[(u16) rec->reference_index];
                reference_state = effect->state;
                if (reference_state != FIELD_EFFECT_RETIRED)
                {
                    placement_origin->vx = effect->x;
                    placement_origin->vy = g_field_effect_records[((u16) rec->reference_index)].y;
                    placement_origin->vz = g_field_effect_records[((u16) rec->reference_index)].z;
                    if (!(part->orientation_flags & FIELD_EFFECT_ORIENTATION_LOCK))
                    {
                        RotMatrix_gte((FieldSVector *) &g_field_effect_records[((u16) rec->reference_index)].rotation_x, rotation);
                        gte_SetRotMatrix(rotation);
                        gte_ldv0(rotated_vector);
                        gte_rtv0();
                        gte_stsv(local_vector);
                        rotated_vector->x = local_vector->x;
                        rotated_vector->y = local_vector->y;
                        rotated_vector->z = local_vector->z;
                    }
                    break;
                }
                rec->state = reference_state;
                return;
            }
            case 0x25:
                placement_origin->vx = (part->offset_x << 8) - D_800F22A0;
                placement_origin->vy = (part->offset_y << 8) - D_800F22A4;
                placement_origin->vz = (part->offset_z << 8) - D_800F22A8;
                break;
            case 0x1D:
                placement_origin->vx = 0;
                placement_origin->vy = -0x7000;
                placement_origin->vz = 0;
                break;
            case 0x1E:
                placement_origin->vx = 0;
                placement_origin->vy = 0x7000;
                placement_origin->vz = 0;
                break;
            case 0x1F:
                placement_origin->vx = 0xFFFF6000;
                placement_origin->vy = 0;
                placement_origin->vz = 0;
                break;
            case 0x20:
                placement_origin->vx = 0xA000;
                placement_origin->vy = 0;
                placement_origin->vz = 0;
                break;
            case 0x21:
                placement_origin->vx = 0xFFFF6000;
                placement_origin->vy = -0x7000;
                placement_origin->vz = 0;
                break;
            case 0x22:
                placement_origin->vx = 0xA000;
                placement_origin->vy = -0x7000;
                placement_origin->vz = 0;
                break;
            case 0x23:
                placement_origin->vx = 0xFFFF6000;
                placement_origin->vy = 0x7000;
                placement_origin->vz = 0;
                break;
            case 0x24:
                placement_origin->vx = 0xA000;
                placement_origin->vy = 0x7000;
                placement_origin->vz = 0;
                break;
            case 0x27:
                reference_record = &D_800FDF58[actor->owner_object_index];
                if ((((u32) part->placement_flags >> 0xA) & 1) && !(reference_record->facing_or_reward_kind & 0x80))
                {
                    placement_origin->vx = reference_record->x - (part->offset_x << 8);
                }
                else
                {
                    placement_origin->vx = reference_record->x + (part->offset_x << 8);
                }
                placement_origin->vy = reference_record->y + (part->offset_y << 8);
                placement_origin->vz = reference_record->z + (part->offset_z << 8);
                break;
            case 0x28:
                reference_record = &D_800FDF58[actor->track_object_indices[g_field_track_index]];
                if ((part->spawn_flags.word & 0x08000000) && !(D_800FDF58[actor->owner_object_index].facing_or_reward_kind & 0x80))
                {
                    placement_origin->vx = reference_record->x - (part->offset_x << 8);
                }
                else if ((((u32) part->placement_flags >> 0xA) & 1) && !(reference_record->facing_or_reward_kind & 0x80))
                {
                    placement_origin->vx = reference_record->x - (part->offset_x << 8);
                }
                else
                {
                    placement_origin->vx = reference_record->x + (part->offset_x << 8);
                }
                placement_origin->vy = reference_record->y + (part->offset_y << 8);
                placement_origin->vz = reference_record->z + (part->offset_z << 8);
                break;
            case 0x29:
                reference_record = &D_800FDF58[actor->owner_object_index];
                reference_object = &D_80105AE0[actor->owner_object_index];
                placement_origin->vx = reference_record->x;
                placement_origin->vy = reference_record->y + (reference_object->attachment_points[((u32) part->effect_flags >> 0x15) & 3].y << 8);
                placement_origin->vz = reference_record->z;
                placement_origin->vx += reference_object->attachment_points[((u32) part->effect_flags >> 0x15) & 3].x << 8;
                break;
            case 0x32:
                reference_record = &D_800FDF58[actor->owner_object_index];
                reference_object = &D_80105AE0[actor->owner_object_index];
                placement_origin->vx = reference_record->x + (reference_object->attachment_points[((u32) part->effect_flags >> 0x15) & 3].x << 8);
                placement_origin->vy = reference_record->y;
                placement_origin->vz = reference_record->z + (part->offset_z << 8);
                if ((((u32) part->placement_flags >> 0xA) & 1) && !(reference_record->facing_or_reward_kind & 0x80))
                {
                    placement_origin->vx -= part->offset_x << 8;
                }
                else
                {
                    placement_origin->vx += part->offset_x << 8;
                }
                break;
            case 0x33:
                reference_record = &D_800FDF58[actor->owner_object_index];
                reference_object = &D_80105AE0[actor->owner_object_index];
                placement_origin->vx = reference_record->x + (reference_object->ground_attachment_points[((u32) rec->flags >> 0xD) & 3].x << 8);
                placement_origin->vy = reference_record->y;
                placement_origin->vz = reference_record->z + (reference_object->ground_attachment_points[((u32) rec->flags >> 0xD) & 3].y << 8);
                break;
            case 0x34:
                reference_record = &D_800FDF58[actor->track_object_indices[g_field_track_index]];
                if (!(reference_record->facing_or_reward_kind & 0x80))
                {
                    placement_origin->vx = reference_record->x + (actor->track_offsets[g_field_track_index].x << 8);
                }
                else
                {
                    placement_origin->vx = reference_record->x - (actor->track_offsets[g_field_track_index].x << 8);
                }
                placement_origin->vy = reference_record->y + (actor->track_offsets[g_field_track_index].y << 8);
                placement_origin->vz = reference_record->z;
                break;
            case 0x36:
                placement_origin->vx = part->offset_x << 8;
                placement_origin->vy = part->offset_y << 8;
                placement_origin->vz = part->offset_z << 8;
                break;
            case 0x37: case 0x38: case 0x39: case 0x3A: case 0x3B:
            case 0x3C: case 0x3D: case 0x3E:
            {
                FieldMotionRecord *effect;
                FieldMotionRecord *effect_base;
                effect_base = g_field_effect_records;
                effect = &effect_base[(u16) rec->reference_index];
                reference_state = effect->state;
                if (reference_state == FIELD_EFFECT_RETIRED)
                {
                    rec->state = reference_state;
                    return;
                }
                placement_origin->vx = effect->x;
                placement_origin->vy = g_field_effect_records[(u16) rec->reference_index].y;
                placement_origin->vz = g_field_effect_records[(u16) rec->reference_index].z;
                if ((part->spawn_flags.word & 0x08000000) && !(D_800FDF58[actor->owner_object_index].facing_or_reward_kind & 0x80))
                {
                    placement_origin->vx -= part->offset_x << 8;
                }
                else if ((((u32) part->placement_flags >> 0xA) & 1) && !(g_field_effect_records[(u16) rec->reference_index].facing_or_reward_kind & 0x80))
                {
                    placement_origin->vx -= part->offset_x << 8;
                }
                else
                {
                    placement_origin->vx += part->offset_x << 8;
                }
                placement_origin->vy += part->offset_y << 8;
                placement_origin->vz += part->offset_z << 8;
                if (!(part->orientation_flags & FIELD_EFFECT_ORIENTATION_LOCK))
                {
                    RotMatrix_gte(&D_800FF668[(u16) rec->reference_index].angles, rotation);
                    gte_SetRotMatrix(rotation);
                    gte_ldv0(rotated_vector);
                    gte_rtv0();
                    gte_stsv(local_vector);
                    rotated_vector->x = local_vector->x;
                    rotated_vector->y = local_vector->y;
                    rotated_vector->z = local_vector->z;
                }
                break;
            }
            default:
                placement_origin->pad = 0;
                placement_origin->vz = 0;
                placement_origin->vy = 0;
                placement_origin->vx = 0;
                break;
            case 0x26:
                break;
        }
        rec->x = ((s16) rotated_vector->x << 8) + placement_origin->vx;
        rec->y = (((s32) (rotated_vector->y << 0x10)) >> 8) + placement_origin->vy;
        z = (((s32) (rotated_vector->z << 0x10)) >> 8) + placement_origin->vz;
        rec->z = z;
        if (part->placement_flags & 1)
        {
            rec->z = z + 0x80;
        }
        flags = part->placement_flags;
        if ((flags >> FIELD_PART_HEIGHT_TRACK_BIT) & 1)
        {
            if (part->spawn_flags.word & FIELD_PART_HEIGHT_FROM_BASE)
            {
                rec->y = (rec->height_or_retired_state - field_evaluate_parameter_track_at_time(actor, (flags >> 4) & 0xF, ((u16) rec->age))) << 8;
            }
            else
            {
                rec->y = -field_evaluate_parameter_track_at_time(actor, (flags >> 4) & 0xF, ((u16) rec->age)) << 8;
            }
        }
        state_or_delta = ((u8 *) &rec->flags)[3] & 7;
        switch (state_or_delta)
        {
            case 3:
                rec->y = rec->y + (((u16) rec->age) << 9);
                break;
            case 4:
                rec->y = rec->y - (((u16) rec->age) << 9);
                break;
        }
        if ((((u32) part->behavior_flags >> FIELD_PART_PITCH_ACCELERATION_BIT) & 1) && ((rec->pitch + part->pitch_acceleration) < 0x800))
        {
            rec->pitch = (u16) rec->pitch + part->pitch_acceleration;
        }
    }
    else
    {
        {
            u16 speed = rec->motion_parameter;
            local_vector->z = 0;
            local_vector->x = 0;
            local_vector->y = -speed << 2;
        }
        RotMatrix_gte((FieldSVector *) &rec->rotation_x, rotation);
        if (!(((u32) part->behavior_flags >> FIELD_PART_PITCH_ACCELERATION_BIT) & 1) && (rec->position_source == 0))
        {
            RotMatrixZ(rec->rotation_z_16 * 0x10, rotation);
            RotMatrixY(rec->rotation_y_16 * 0x10, rotation);
        }
        gte_SetRotMatrix(rotation);
        gte_ldv0(local_vector);
        gte_rtv0();
        gte_stsv(rotated_vector);

        /* Free movement uses a rotated step and a fresh collision probe each update. */
        initial_surface = FIELD_EFFECT_SKIP_SURFACE_PREPASS;
        if ((((rec->flags & 0x60000000) != 0x40000000) || ((s16) rec->motion_parameter != 0)) && ((((u32) part->behavior_flags >> FIELD_PART_PITCH_ACCELERATION_BIT) & 1) || ((s32) part->placement_flags < 0)))
        {
            s32 pitch_cosine;
            s16 adjusted_pitch;

            pitch_cosine = rcos(rec->pitch) - (part->pitch_acceleration * 8);
            adjusted_pitch = ratan2(rsin(rec->pitch), pitch_cosine);
            rec->pitch = adjusted_pitch;
            if (adjusted_pitch < 0x400)
            {
                rec->motion_parameter = (((s16) rec->motion_parameter * 0xF) >> 4) - 1;
            }
            else
            {
                rec->motion_parameter = (((s16) rec->motion_parameter << 5) / 30) + 1;
            }
            if (((s16) rec->motion_parameter < 0x14) && (rec->pitch < 0x400))
            {
                rec->pitch = 0x800 - (u16) rec->pitch;
                rec->motion_parameter = 0x14;
            }
        }

        if ((part->effect_flags & FIELD_PART_MAP_COLLISION) && !(part->spawn_flags.word & FIELD_PART_SKIP_MAP_COLLISION))
        {
            z = rec->x;
            if ((z < 0) || (z >= (map_bounds->width << 8)) || ((z = rec->z), (z < 0)) || (z >= ((s32) (map_bounds->height << 0x10) >> 7)))
            {
                if (!(part->spawn_flags.word & FIELD_PART_RETIRE_ON_COLLISION))
                {
                    rotated_vector->x = 0;
                    rotated_vector->z = 0;
                }
                else
                {
                    rec->state = FIELD_EFFECT_RETIRED;
                }
            }
            else
            {
                s32 position_z;
                s32 position_x;
                /* Advance the packed context during a collision attempt; protocol remains unresolved. */
                g_field_action_context += 0x100;
                mover->x = rec->x;
                mover->y = 0;
                mover->z = rec->z;
                mover->move_x = rotated_vector->x;
                mover->move_y = 0;
                mover->move_z = rotated_vector->z;
                mover->width = FIELD_EFFECT_COLLISION_WIDTH;
                mover->mode.bits.depth = FIELD_EFFECT_COLLISION_DEPTH;
                mover->height_tolerance = FIELD_EFFECT_COLLISION_HEIGHT;
                mover->collision_node = initial_surface;
                mover->contact_flags = 0;
                mover->mode.bits.unknown_17 = 0;
                mover->mode.bits.unknown_16 = 0;
                if (func_8005B6AC(mover) & 3)
                {
                    rotated_vector->z = 0;
                    rotated_vector->x = 0;
                    if (part->spawn_flags.word & FIELD_PART_RETIRE_ON_COLLISION)
                    {
                        rec->state = FIELD_EFFECT_RETIRED;
                    }
                }
                else if (!(part->spawn_flags.word & FIELD_PART_RETIRE_ON_COLLISION))
                {
                    position_x = mover->x;
                    query->width = FIELD_EFFECT_COLLISION_WIDTH;
                    query->height_tolerance = FIELD_EFFECT_COLLISION_HEIGHT;
                    /* This scope preserves the depth register's allocation priority. */
                    do
                    {
                        query->depth = FIELD_EFFECT_COLLISION_DEPTH;
                    } while (0);
                    query->x = position_x;
                    position_z = mover->z;
                    {
                        s32 query_y = rec->y;
                        query->z = position_z;
                        query->y = query_y;
                    }
                    if ((D_800FE754 != 0) && (func_8005B368(query) != -1))
                    {
                        rotated_vector->z = 0;
                        rotated_vector->x = 0;
                    }
                    else
                    {
                        rotated_vector->x = (u16) mover->x - (u16) rec->x;
                        rotated_vector->z = (u16) mover->z - (u16) rec->z;
                    }
                }
            }
        }

        if (part->spawn_flags.word & FIELD_PART_CAMERA_BOUNDS)
        {
            s32 camera_x;
            s32 next_z;
            dx = rec->x + (s16) rotated_vector->x;
            camera_x = -camera->x;
            if (((camera_x + FIELD_EFFECT_CAMERA_X_MIN) < dx) && (dx < (camera_x + FIELD_EFFECT_CAMERA_X_MAX)))
            {
                next_z = rec->z + (s16) rotated_vector->z;
                x = -camera->z;
                if ((x < next_z) && (next_z < (x + FIELD_EFFECT_CAMERA_Z_SPAN)))
                {
                    rec->x = dx;
                    rec->z = rec->z + (s16) rotated_vector->z;
                }
            }
        }
        else
        {
            rec->x = rec->x + (s16) rotated_vector->x;
            rec->z = rec->z + (s16) rotated_vector->z;
        }
        {
            u32 ground_flags;
            y = rec->y + (s16) rotated_vector->y;
            rec->y = y;
            if ((part->spawn_flags.word & FIELD_PART_GROUND_STOP) && (y >= 0) && (((ground_flags = rec->flags, state_or_delta = ground_flags & 0x60000000), (state_or_delta == 0)) || (state_or_delta == 0x40000000)))
            {
                rec->flags = ground_flags & 0x9FFFFFFF;
                rec->y = 0;
                rec->motion_parameter = 0;
            }
        }
        flags = part->placement_flags;
        if ((flags >> FIELD_PART_HEIGHT_TRACK_BIT) & 1)
        {
            if (part->spawn_flags.word & FIELD_PART_HEIGHT_FROM_BASE)
            {
                rec->y = (rec->height_or_retired_state - field_evaluate_parameter_track_at_time(actor, (flags >> 4) & 0xF, ((u16) rec->age))) << 8;
            }
            else
            {
                rec->y = -field_evaluate_parameter_track_at_time(actor, (flags >> 4) & 0xF, ((u16) rec->age)) << 8;
            }
        }

        /* Resolve a target and either stop nearby or steer the heading and pitch. */
        if (rec->position_source != 0)
        {
            s32 target_dy, target_dz;

            field_resolve_effect_position(rec, part, target_position);
            work_vector->vx = (rec->x - target_position->vx) >> 8;
            work_vector->vy = (rec->y - target_position->vy) >> 8;
            target_dz = (rec->z - target_position->vz) >> 8;
            work_vector->vz = target_dz;
            if (((u32) (work_vector->vx + 0xF) < 0x1FU) && (target_dz >= -0xF) && (target_dz < 0x10))
            {
                target_dy = work_vector->vy;
                if ((target_dy >= -0xF) && (work_vector->vy < 0x10))
                {
                    if ((((u32) part->behavior_flags >> 4) & 3) == 2)
                    {
                        retired_state = rec->state;
                        rec->state = FIELD_EFFECT_RETIRED;
                        rec->height_or_retired_state = (s8) retired_state;
                        return;
                    }
                    if (!(part->orientation_flags & FIELD_EFFECT_ORIENTATION_LOCK))
                    {
                        rec->motion_parameter = 0;
                        rec->flags = rec->flags & 0x9FFFFFFF;
                    }
                }
            }
            if (!(rec->flags & FIELD_EFFECT_MOTION_KIND_MASK) && !(part->orientation_flags & FIELD_EFFECT_ORIENTATION_LOCK))
            {
                FieldVector new_pos;
                FieldVector delta;
                FieldVector delta_squared;
                s16 target_heading;

                field_resolve_effect_position(rec, part, &new_pos);
                delta.vx = (new_pos.vx - rec->x) >> 8;
                delta.vy = (new_pos.vy - rec->y) >> 8;
                delta.vz = (new_pos.vz - rec->z) >> 8;
                gte_ldlvl(&delta);
                gte_sqr0();
                gte_stlvnl(&delta_squared);
                target_heading = ratan2(-delta.vz, delta.vx);
                rec->heading = target_heading;
                if (target_heading < 0)
                {
                    rec->heading = target_heading + FIELD_ANGLE_TURN;
                }
                if (delta.vy != 0)
                {
                    rec->pitch = ratan2(SquareRoot0(delta_squared.vx + delta_squared.vz), -delta.vy);
                }
                else
                {
                    rec->pitch = FIELD_ANGLE_QUARTER_TURN;
                }
                if (rec->pitch < 0)
                {
                    rec->pitch = (u16) rec->pitch + FIELD_ANGLE_TURN;
                }
                rec->rotation_x = 0;
            }
            if ((rec->flags & FIELD_EFFECT_MOTION_KIND_MASK) == FIELD_EFFECT_MOTION_HOMING)
            {

                s16 next_heading;

                gte_ldlvl(work_vector);
                gte_sqr0();
                gte_stlvnl(target_position);
                offset_or_angle = ratan2(work_vector->vz, -work_vector->vx);
                if (offset_or_angle < 0)
                {
                    offset_or_angle += FIELD_ANGLE_TURN;
                }
                if (((u16) rec->age) == part->turn_end_age)
                {
                    rec->flags = rec->flags & 0xF8FFFFFF;
                    if (!(part->orientation_flags & FIELD_EFFECT_ORIENTATION_LOCK))
                    {
                        rec->heading = (u16) offset_or_angle;
                        if (work_vector->vz != 0)
                        {
                            rec->pitch = ratan2(SquareRoot0(target_position->vx + target_position->vz), work_vector->vy);
                        }
                        else
                        {
                            rec->pitch = FIELD_ANGLE_QUARTER_TURN;
                        }
                        if (rec->pitch < 0)
                        {
                            rec->pitch = (u16) rec->pitch + FIELD_ANGLE_TURN;
                        }
                    }
                }
                else
                {
                    current_angle = (s16) rec->heading;
                    selector = (offset_or_angle - current_angle) & FIELD_ANGLE_MASK;
                    if ((u32) (selector - 8) >= 0xFF1U)
                    {
                        rec->heading = (u16) offset_or_angle;
                    }
                    else
                    {
                        if (selector >= 0x801)
                        {
                            s32 reverse_turn = FIELD_ANGLE_TURN - selector;
                            if (reverse_turn < FIELD_EFFECT_TURN_THRESHOLD)
                            {
                                next_heading = current_angle - (reverse_turn >> 2);
                            }
                            else
                            {
                                next_heading = current_angle - FIELD_EFFECT_TURN_STEP;
                            }
                        }
                        else if (selector < FIELD_EFFECT_TURN_THRESHOLD)
                        {
                            next_heading = current_angle + (selector >> 2);
                        }
                        else
                        {
                            next_heading = current_angle + FIELD_EFFECT_TURN_STEP;
                        }
                        rec->heading = next_heading;
                    }
                    if ((s16) rec->heading < 0)
                    {
                        rec->heading = (u16) rec->heading + FIELD_ANGLE_TURN;
                    }
                    {
                        offset_or_angle = ratan2(SquareRoot0(target_position->vx + target_position->vz), work_vector->vy);
                        current_angle = rec->pitch;
                        if (offset_or_angle < current_angle)
                        {
                            state_or_delta = current_angle - offset_or_angle;
                            if (state_or_delta < FIELD_EFFECT_TURN_THRESHOLD)
                            {
                                rec->pitch = current_angle - (state_or_delta >> 2);
                            }
                            else
                            {
                                rec->pitch = current_angle - FIELD_EFFECT_TURN_STEP;
                            }
                        }
                        else if (current_angle < offset_or_angle)
                        {
                            state_or_delta = offset_or_angle - current_angle;
                            if (state_or_delta < FIELD_EFFECT_TURN_THRESHOLD)
                            {
                                rec->pitch = current_angle + (state_or_delta >> 2);
                            }
                            else
                            {
                                rec->pitch = current_angle + FIELD_EFFECT_TURN_STEP;
                            }
                        }
                        if (rec->pitch < 0)
                        {
                            rec->pitch = (u16) rec->pitch + FIELD_ANGLE_TURN;
                        }
                        rec->rotation_x = 0;
                    }
                }
            }
        }
    }
    /* After the pickup delay, find a nearby recipient and consume the reward effect. */
    if (!(actor->action_flags & 1))
    {
        state_or_delta = ((u16 *) &actor->action_flags)[1];
        switch (state_or_delta)
        {
            case FIELD_PICKUP_EXPERIENCE_OR_CURRENCY:
                if ((u16) actor->track_ages[0] >= FIELD_PICKUP_DELAY)
                {
                    recipient_index = func_8009980C(rec, FIELD_PICKUP_DISTANCE, actor, 0);
                    if (recipient_index != -1)
                    {
                        FieldObjectPlacement *recipient_object;
                        FieldObjectPlacement *object_base;
                        s32 counter_slot;
                        FieldCounterView *counter_base;
                        u32 counter_index;

                        if (g_field_pickup_sound_played == 0)
                        {
                            func_800A3938(FIELD_PICKUP_SOUND, field_get_actor_sound_pan(rec->source_object_index));
                        }
                        g_field_pickup_sound_played = 1;
                        rec->state = FIELD_EFFECT_RETIRED;
                        object_base = D_80105AE0;
                        recipient_object = &object_base[recipient_index];
                        field_set_action_context(recipient_object->record_id, object_base[actor->owner_object_index].record_id, rec->facing_or_reward_kind - 0x16);
                        counter_base = D_800FD818;
                        counter_index = rec->facing_or_reward_kind;
                        counter_slot = recipient_index < 3 ? recipient_index : 2;
                        counter_base[counter_slot].counters[counter_index] = counter_base[recipient_index < 3 ? recipient_index : 2].counters[counter_index] + 1;
                        func_800C0B40(recipient_object->record_id, object_base[actor->owner_object_index].record_id, rec->facing_or_reward_kind - 0x16);
                        field_release_actor_if_no_effects(rec);
                        return;
                    }
                }
                break;
            case FIELD_PICKUP_ITEM:
                if ((u16) actor->track_ages[0] >= FIELD_PICKUP_DELAY)
                {
                    recipient_index = func_8009980C(rec, FIELD_PICKUP_DISTANCE, actor, 0);
                    if (recipient_index != -1)
                    {
                        FieldObjectPlacement *recipient_object;
                        FieldObjectPlacement *object_base;

                        if (g_field_pickup_sound_played == 0)
                        {
                            func_800A3938(FIELD_PICKUP_SOUND, field_get_actor_sound_pan(rec->source_object_index));
                        }
                        g_field_pickup_sound_played = 1;
                        rec->state = FIELD_EFFECT_RETIRED;
                        object_base = D_80105AE0;
                        recipient_object = &object_base[recipient_index];
                        field_set_action_context(recipient_object->record_id, object_base[actor->owner_object_index].record_id, 4);
                        func_800C0B40(recipient_object->record_id, object_base[actor->owner_object_index].record_id, 4);
                        field_release_actor_if_no_effects(rec);
                        return;
                    }
                }
                break;
            /* Restore 64/256 of maximum capacity, then play the quarter-restore animation. */
            case FIELD_PICKUP_RESTORE_QUARTER:
                if ((u16) actor->track_ages[0] >= FIELD_PICKUP_DELAY)
                {
                    recipient_index = func_8009980C(rec, FIELD_PICKUP_DISTANCE, actor, 0);
                    if (recipient_index != -1)
                    {
                        FieldObjectPlacement *recipient_object;
                        FieldObjectPlacement *object_base;

                        if (g_field_pickup_sound_played == 0)
                        {
                            func_800A3938(FIELD_PICKUP_SOUND, field_get_actor_sound_pan(rec->source_object_index));
                        }
                        g_field_pickup_sound_played = 1;
                        rec->state = FIELD_EFFECT_RETIRED;
                        object_base = D_80105AE0;
                        recipient_object = &object_base[recipient_index];
                        field_set_action_context(recipient_object->record_id, object_base[actor->owner_object_index].record_id, 5);
                        func_800C0B40(recipient_object->record_id, object_base[actor->owner_object_index].record_id, 5);
                        func_80092C24(&D_800FDF58[recipient_index], 0x2C);
                        field_release_actor_if_no_effects(rec);
                        return;
                    }
                }
                break;
            /* Restore 128/256 of maximum capacity. */
            case FIELD_PICKUP_RESTORE_HALF:
                if ((u16) actor->track_ages[0] >= FIELD_PICKUP_DELAY)
                {
                    recipient_index = func_8009980C(rec, FIELD_PICKUP_DISTANCE, actor, 0);
                    if (recipient_index != -1)
                    {
                        FieldObjectPlacement *recipient_object;
                        FieldObjectPlacement *object_base;

                        if (g_field_pickup_sound_played == 0)
                        {
                            func_800A3938(FIELD_PICKUP_SOUND, field_get_actor_sound_pan(rec->source_object_index));
                        }
                        g_field_pickup_sound_played = 1;
                        rec->state = FIELD_EFFECT_RETIRED;
                        object_base = D_80105AE0;
                        recipient_object = &object_base[recipient_index];
                        field_set_action_context(recipient_object->record_id, object_base[actor->owner_object_index].record_id, 6);
                        func_800C0B40(recipient_object->record_id, object_base[actor->owner_object_index].record_id, 6);
                        func_80092C24(&D_800FDF58[recipient_index], 0x2D);
                        field_release_actor_if_no_effects(rec);
                        return;
                    }
                }
                break;
            default:
                break;
        }
    }
    /* Collect effect-centered hits, reflect below-ground motion, then scale speed. */
    {
        FieldActorAnimationDef *anim = actor->animation;
        if ((anim->hit_test_mode == FIELD_HIT_TEST_EFFECT_BOUNDS) && (anim->hit_test_part == rec->part_index))
        {
            field_collect_effect_hits(rec, anim->hit_radius, actor);
        }
    }
    if (((u32) part->behavior_flags >> FIELD_PART_GROUND_BOUNCE_BIT) & 1)
    {
        s32 ground_y = rec->y;
        if (ground_y > 0)
        {
            s32 old_pitch;
            s16 old_motion;

            rec->y = -ground_y;
            old_pitch = (u16) rec->pitch;
            old_motion = rec->motion_parameter;
            rec->pitch = FIELD_ANGLE_HALF_TURN - old_pitch;
            rec->motion_parameter = old_motion / 2;
            field_dispatch_actor_audio_event(actor, 4, rec->part_index, old_pitch);
        }
    }
    rec->motion_parameter = (rec->motion_parameter * rec->motion_scale) >> 8;
}


extern FieldActorState g_field_actor_slots[80];
extern FieldMotionRecord D_800FDF58[];
extern FieldMotionRecord g_field_effect_records[];
extern FieldObjectPlacement D_80105AE0[];
extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;
extern s32 g_field_track_index;
extern s32 g_field_action_context;

/**
 * @brief Release an effect's owning actor after its last live effect retires.
 * @param effect Motion record whose actor_index identifies the owning actor slot.
 * @note Callers retire the effect before this check. Other live effects keep the
 * actor active; this function does not retire records or free their storage.
 * @see decomp.me (100%)
 */
void field_release_actor_if_no_effects(FieldMotionRecord *effect)
{
    FieldActorState *slots;
    FieldActorState *slot;

    if (field_actor_has_live_effects(effect->actor_index) == 0)
    {
        slots = g_field_actor_slots;
        slot = &slots[effect->actor_index];
        slot->is_active = 0;
        slot->unknown_0x23b = 0;
        slot->active_track_mask = 0;
    }
}

/**
 * @brief Check the effect pool for an unretired record owned by an actor.
 * @param actor_index Actor slot index to compare with each record's owner.
 * @return 1 if a matching live effect exists, otherwise 0.
 * @see decomp.me (100%)
 */
s32 field_actor_has_live_effects(s32 actor_index)
{
    s32 effect_index;
    s32 retired_state;
    FieldMotionRecord *effect;

    effect_index = 0;
    retired_state = FIELD_EFFECT_RETIRED;
    effect = g_field_effect_records;
    for (; effect_index < FIELD_EFFECT_POOL_COUNT; effect_index++)
    {
        if (effect->state != retired_state && effect->actor_index == actor_index)
        {
            return 1;
        }
        effect++;
    }
    return 0;
}

/**
 * @brief Pack recipient, source, and action identifiers into shared action context.
 * @param recipient_id Identifier placed starting at bit 16.
 * @param source_id Identifier placed starting at bit 8.
 * @param action Action/reward selector placed in the low bits.
 * @note Inputs are not masked here; preserve their original word-wide behavior.
 * @see decomp.me (100%)
 */
void field_set_action_context(s32 recipient_id, s32 source_id, s32 action)
{
    g_field_action_context = (recipient_id << 0x10) | (source_id << 8) | action;
}

/**
 * @brief Resolve an effect's selected target/attachment position in world coordinates.
 * @param rec Record selecting a source and supplying saved positions or effect links.
 * @param part Part definition controlling anchor choice, placement, and facing updates.
 * @param out Destination X/Y/Z; its pad word is untouched.
 * @note Sources 0 and values above 15 leave out unchanged. Source 9 can update
 * the record's facing bit. References at +0x30 use an unsigned halfword read.
 * The dispatch table retains its trailing null word at jtbl_80050144.
 * @see decomp.me (100%)
 */
void field_resolve_effect_position(FieldMotionRecord *rec, FieldActorPartDef *part, FieldVector *out)
{
    FieldMotionRecord *source_record;
    FieldMotionRecord *opposite_record;
    FieldMotionRecord *track_record;
    FieldMotionRecord *owner_record;
    FieldMotionRecord *linked_record;
    FieldObjectPlacement *source_object;
    FieldActorState *slots;
    FieldActorState *owner_slots;
    s32 object_index;
    s32 source_index;
    s32 owner_index;
    s32 placement;
    s32 delta_x;
    s32 position_x;
    s32 x_term;
    s32 offset_x;
    s32 dispatch;
    static void *const dispatch_table[] =
    {
        &&track_object, &&owner_object, &&saved, &&reference_effect,
        &&owner_attachment, &&facing_offset, &&track_stored_xz, &&linked_effect,
        &&relative_side_offset, &&owner_bounds_center, &&track_bounds_center,
        &&reflect_track_x, &&reflect_track_x, &&extend_track_xz, &&extend_link_xz, 0
    };

    dispatch = rec->position_source - FIELD_POSITION_TRACK_OBJECT;
    if ((u32) dispatch >= FIELD_POSITION_EXTEND_LINK_XZ)
    {
        return;
    }
    goto *dispatch_table[dispatch];

track_object:
    slots = g_field_actor_slots;
    out->vx = D_800FDF58[slots[rec->actor_index].track_object_indices[g_field_track_index]].x;
    out->vy = D_800FDF58[slots[rec->actor_index].track_object_indices[g_field_track_index]].y;
    object_index = slots[rec->actor_index].track_object_indices[g_field_track_index];
    do
    {
        track_record = &D_800FDF58[object_index];
    } while (0);
    out->vz = track_record->z;
    return;
owner_object:
    owner_slots = g_field_actor_slots;
    out->vx = D_800FDF58[owner_slots[rec->actor_index].owner_object_index].x;
    out->vy = D_800FDF58[g_field_actor_slots[rec->actor_index].owner_object_index].y;
    owner_index = owner_slots[rec->actor_index].owner_object_index;
    do
    {
        owner_record = &D_800FDF58[owner_index];
    } while (0);
    out->vz = owner_record->z;
    return;
saved:
    out->vx = rec->work_x - D_800F22A0;
    out->vy = rec->work_y - D_800F22A4;
    out->vz = rec->work_z - D_800F22A8;
    return;
reference_effect:
    if (g_field_effect_records[(u16) rec->reference_index].state != FIELD_EFFECT_RETIRED)
    {
        out->vx = g_field_effect_records[(u16) rec->reference_index].x;
        out->vy = g_field_effect_records[(u16) rec->reference_index].y;
        out->vz = g_field_effect_records[(u16) rec->reference_index].z;
        return;
    }
    out->vx = rec->x;
    out->vy = rec->y;
    out->vz = rec->z;
    return;
owner_attachment:
    source_record = &D_800FDF58[g_field_actor_slots[rec->actor_index].owner_object_index];
    source_object = &D_80105AE0[g_field_actor_slots[rec->actor_index].owner_object_index];
    out->vx = source_record->x + (source_object->attachment_points[((u32) part->effect_flags >> 0x15) & 3].x << 8);
    out->vy = source_record->y + (source_object->attachment_points[((u32) part->effect_flags >> 0x15) & 3].y << 8);
    out->vz = source_record->z;
    return;
facing_offset:
    placement = ((u32) part->placement_flags >> 0x12) & 0x3F;
    if (placement >= 0xA)
    {
        if (placement < 0x26)
        {
            source_index = g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index];
        }
        else
        {
            source_index = g_field_actor_slots[rec->actor_index].owner_object_index;
        }
    }
    else
    {
        source_index = g_field_actor_slots[rec->actor_index].owner_object_index;
    }
    source_record = &D_800FDF58[source_index];
    if (source_record->facing_or_reward_kind & 0x80)
    {
        position_x = rec->work_x;
        x_term = source_record->x;
        position_x = position_x + x_term;
    }
    else
    {
        x_term = rec->work_x;
        position_x = -x_term;
        position_x += source_record->x;
    }
    out->vx = position_x;
    out->vy = rec->work_y + source_record->y;
    out->vz = rec->work_z + source_record->z;
    return;
track_stored_xz:
    out->vx = D_80105AE0[g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index]].unknown_0x6c << 8;
    out->vz = D_80105AE0[g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index]].unknown_0x6e << 8;
    out->vy = 0;
    return;
linked_effect:
    out->vx = g_field_effect_records[rec->position_data.linked_effect_index].x;
    out->vy = g_field_effect_records[rec->position_data.linked_effect_index].y;
    linked_record = &g_field_effect_records[rec->position_data.linked_effect_index];
    out->vz = linked_record->z;
    return;
relative_side_offset:
    placement = ((u32) part->placement_flags >> 0x12) & 0x3F;
    if (placement < 0xA)
    {
        source_record = &D_800FDF58[g_field_actor_slots[rec->actor_index].owner_object_index];
        opposite_record = &D_800FDF58[g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index]];
    }
    else
    {
        if (placement < 0x26)
        {
            object_index = g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index];
            source_record = &D_800FDF58[object_index];
        }
        else
        {
            object_index = g_field_actor_slots[rec->actor_index].owner_object_index;
            source_record = &D_800FDF58[object_index];
        }
        opposite_record = &D_800FDF58[g_field_actor_slots[rec->actor_index].owner_object_index];
    }
    if (source_record->x > opposite_record->x)
    {
        if (((part->placement_flags >> 0xA) & 1) || (part->spawn_flags.word & 0x08000000))
        {
            rec->facing_or_reward_kind = rec->facing_or_reward_kind & 0x7F;
        }
        offset_x = rec->work_x;
        position_x = source_record->x - offset_x;
    }
    else
    {
        if (((part->placement_flags >> 0xA) & 1) || (part->spawn_flags.word & 0x08000000))
        {
            rec->facing_or_reward_kind = rec->facing_or_reward_kind | 0x80;
        }
        position_x = rec->work_x;
        x_term = source_record->x;
        position_x = position_x + x_term;
    }
    out->vx = position_x;
    out->vy = rec->work_y + source_record->y;
    out->vz = rec->work_z + source_record->z;
    return;
owner_bounds_center:
    source_object = &D_80105AE0[g_field_actor_slots[rec->actor_index].owner_object_index];
    out->vx = D_800FDF58[g_field_actor_slots[rec->actor_index].owner_object_index].x + ((source_object->bounds_right + source_object->bounds_left) << 7);
    out->vy = D_800FDF58[g_field_actor_slots[rec->actor_index].owner_object_index].y + ((source_object->bounds_bottom + source_object->bounds_top) << 7);
    out->vz = D_800FDF58[g_field_actor_slots[rec->actor_index].owner_object_index].z;
    return;
track_bounds_center:
    source_object = &D_80105AE0[g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index]];
    out->vx = D_800FDF58[g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index]].x + ((source_object->bounds_right + source_object->bounds_left) << 7);
    out->vy = D_800FDF58[g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index]].y + ((source_object->bounds_bottom + source_object->bounds_top) << 7);
    out->vz = D_800FDF58[g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index]].z;
    return;
reflect_track_x:
    delta_x = D_800FDF58[g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index]].x - D_800FDF58[g_field_actor_slots[rec->actor_index].owner_object_index].x;
    if (rec->position_source == FIELD_POSITION_REFLECT_TRACK_X)
    {
        out->vx = D_800FDF58[g_field_actor_slots[rec->actor_index].owner_object_index].x - delta_x;
    }
    else
    {
        out->vx = D_800FDF58[g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index]].x + delta_x;
    }
    out->vy = D_800FDF58[g_field_actor_slots[rec->actor_index].owner_object_index].y;
    out->vz = D_800FDF58[g_field_actor_slots[rec->actor_index].owner_object_index].z;
    return;
extend_track_xz:
    out->vx = (D_800FDF58[g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index]].x * 2) - D_800FDF58[g_field_actor_slots[rec->actor_index].owner_object_index].x;
    out->vy = D_800FDF58[g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index]].y;
    out->vz = (D_800FDF58[g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index]].z * 2) - D_800FDF58[g_field_actor_slots[rec->actor_index].owner_object_index].z;
    return;
extend_link_xz:
    out->vx = (D_800FDF58[g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index]].x * 2) - g_field_effect_records[rec->position_data.linked_effect_index].x;
    out->vy = D_800FDF58[g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index]].y;
    out->vz = (D_800FDF58[g_field_actor_slots[rec->actor_index].track_object_indices[g_field_track_index]].z * 2) - g_field_effect_records[rec->position_data.linked_effect_index].z;
    return;
}
