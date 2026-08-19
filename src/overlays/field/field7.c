/**
 * @file field7.c
 * @brief Field script effect/particle spawner.
 *
 * Holds func_8006D79C, the routine the field script's part stepper calls to
 * create one effect record. It claims a free slot in D_800FF658, seeds it from
 * the actor's part definition, and then dispatches on a 6-bit placement opcode
 * to work out where in the world the effect sits.
 *
 * The file deliberately declares none of its callees. field6.c does not declare
 * them either, and reproducing that implicit-int declaration state is required
 * to match: an explicit u8 return on the parameter-track helpers turns
 * "result != 0" from a single sltu into an andi plus a branch.
 */

#include "common.h"

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
    u8 unk60;   /* 0x60 */
    u8 pad61[0x12C - 0x61];
    u32 unk12C; /* 0x12C */
    Vec2s unk130[4]; /* 0x130 */
    s16 unk140; /* 0x140 */
    s16 unk142; /* 0x142 */
    s16 unk144; /* 0x144 */
    s16 unk146; /* 0x146 */
    u8 pad148[0x178 - 0x148];
    s32 unk178; /* 0x178 (byte views at 0x178 and 0x17A) */
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
} FieldActorPartDef;

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
    FieldActorPartDef* unk0;
    u8 pad4[0xC - 4];
    FieldActorAnimationDef* unkC;
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
} FieldActorState;

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
    s16 m[3][3];
    s32 t[3];
} FieldMatrix;

typedef struct
{
    u8 unk0;
    u8 pad1[0xC - 1];
    u8 unkC;
    u8 padD[0x1C - 0xD];
} Struct_D80105880;

extern Struct_D800FDF58 D_800FDF58[];
extern Struct_D800FDF58 D_800FF658[];
extern Struct_D80105AE0 D_80105AE0[];
extern FieldActorPartDef D_800FE3A0[];
extern FieldActorState g_field_actor_slots[];
extern Struct_D80105880 D_80105880[];
extern FieldVector D_80105778;
extern s32 D_800473F8;
extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;
extern s32 D_80105760;
extern s32 D_80105770;
extern u8* D_801058D4;
extern s32 g_field_track_index;

/*
 * Psy-Q inline GTE (COP2) macros. The original build used the vendor
 * inline_c.h forms, which are __asm__ volatile blocks rather than calls, so the
 * COP2 opcodes appear inline in the ROM. The two leading nops in GTE_RTV0 and
 * GTE_SQR0 are part of the vendor macro (GTE load latency), not padding.
 */
#define GTE_SET_ROT_MATRIX(m)                                                                      \
    __asm__ volatile("lw $12, 0(%0);lw $13, 4(%0);ctc2 $12, $0;ctc2 $13, $1;lw $12, 8(%0);"        \
                     "lw $13, 12(%0);lw $14, 16(%0);ctc2 $12, $2;ctc2 $13, $3;ctc2 $14, $4"        \
                     :                                                                             \
                     : "r"(m)                                                                      \
                     : "$12", "$13", "$14")

#define GTE_LDV0(v) __asm__ volatile("lwc2 $0, 0(%0);lwc2 $1, 4(%0)" : : "r"(v))
#define GTE_RTV0() __asm__ volatile("nop;nop;cop2 0x0486012")
#define GTE_LDLVL(v) __asm__ volatile("lwc2 $9, 0(%0);lwc2 $10, 4(%0);lwc2 $11, 8(%0)" : : "r"(v))
#define GTE_SQR0() __asm__ volatile("nop;nop;cop2 0x0a00428")
#define GTE_STLVNL(v) __asm__ volatile("swc2 $25, 0(%0);swc2 $26, 4(%0);swc2 $27, 8(%0)" : : "r"(v))

/**
 * @brief Spawn one field effect record for a part of an actor.
 *
 * Claims the first free slot in D_800FF658 (marked by unk25 == 0xFF), seeds it
 * from actor->unk0[part_index], derives its orientation through the GTE, and
 * then dispatches on the part's 6-bit placement opcode
 * ((part->unk28 >> 18) & 0x3F) to position it relative to the owner object, the
 * tracked object, the camera, or another already-spawned effect. Recurses for
 * chained parts and for the opcode-0x33 retry.
 *
 * @param actor Actor whose script is spawning the effect.
 * @param part_index Index of the part definition within actor->unk0.
 * @param start First D_800FF658 slot to consider when an opcode has to search
 *              for a previously spawned sibling effect; 0 on the outermost call.
 * @return Index of the slot that was filled, or -1 if no slot was free or the
 *         placement opcode rejected the spawn.
 *
 * @note WIP - 83.88% at time of writing. Structure, case order, shared-label
 *       placement and struct access widths are settled; the residue is two
 *       cross-jumped scan-loop tails plus a saved-register shortfall. Full
 *       handoff in working/func_8006D79C/STATUS.md.
 */
s32 func_8006D79C(FieldActorState* actor, s32 part_index, s32 start)
{
    FieldVector* vec = (FieldVector*)0x1F800000;
    FieldVector* sqr = (FieldVector*)0x1F800010;
    FieldSVector* dir = (FieldSVector*)0x1F800030;
    FieldMatrix* mtx = (FieldMatrix*)0x1F800040;
    Struct_D800FDF58* rec;
    Struct_D800FDF58* src;
    Struct_D800FDF58* src27;
    Struct_D800FDF58* src28;
    Struct_D800FDF58* scan;
    Struct_D800FDF58* scanA;
    Struct_D800FDF58* scanB;
    Struct_D80105AE0* slot;
    Struct_D80105AE0* slot27;
    Struct_D80105AE0* slot28;
    FieldSVector* scanA_rot;
    FieldSVector* scanB_rot;
    FieldActorPartDef* part;
    s32 base_x;
    s32 speed;
    s32 count;
    s32 i;
    s32 n;
    s32 j;
    s32 val;
    s32 kind;
    s32 sub;
    s32 nA;
    s32 nB;
    s32 subA;
    s32 subB;
    s32 pos27;
    s32 pos28;
    s32 pos37;
    u8 track_obj;
    u8* res;

    i = 0;
    scan = D_800FF658;
find_slot:
    if (scan->unk25 != 0xFF)
    {
        i++;
        scan++;
        if (i < 0x100)
        {
            goto find_slot;
        }
    }
    if (i == 0x100)
    {
        D_800473F8 = 0x10101010;
        return -1;
    }

    part = &actor->unk0[part_index];
    rec = &D_800FF658[i];
    if ((s32)part->unk24 < 0)
    {
        rec->unk3D = 0xFF;
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
        rec->unk1B = ((u16*)&part->unk24)[1] & 0xF;
    }
    else
    {
        rec->unk1B = (part->unk4 >> 8) & 7;
    }
    rec->unk1C = (rec->unk1C & 0xF8FFFFFF) | (((part->unk4 >> 13) & 7) << 24);
    rec->unk1C = (rec->unk1C & ~0x600) | ((part->unk20 >> 6) << 9);
    rec->unk28 = part->unkD;
    rec->unk2A = ((u16*)&part->unk14)[1];
    rec->unk2E = part->unk18;
    rec->unk29 = g_field_track_index;
    rec->unk1C = ((rec->unk1C & 0x9FFFFFFF) | ((*(u8*)&part->unk14 & 3) << 29)) & ~0x1000;
    rec->unk1C =
        (((rec->unk1C & 0xF7FFFFFF) | (((part->unk34 >> 18) & 1) << 27)) & ~0x6000) & 0xFFFBFFFF & 0xFF87FFFF;
    if (part->unk24 & 0x800000)
    {
        rec->unk1C =
            (rec->unk1C & 0xFF7FFFFF) |
            ((field_evaluate_parameter_track_at_time(actor, (part->unk24 >> 25) & 0xF, 0) != 0) << 23);
    }
    else
    {
        rec->unk1C = (rec->unk1C & 0xFF7FFFFF) | (((part->unk4 >> 1) & 1) << 23);
    }
    rec->unk1C = rec->unk1C & 0xFFFCFFFF;
    if (rec->unk1B == 8)
    {
        n = 0;
        count = 0;
        scan = D_800FF658;
        j = actor->unk3B[g_field_track_index][part_index];
    scan_slots:
        if (scan->unk25 != 0xFF && scan->unk23 == part->unk46 && scan->unk22 == actor->unk233)
        {
            count = 1;
            if (j != 0)
            {
                rec->unk20 = n;
                j--;
                goto next_slot;
            }
            rec->unk20 = n;
        }
        else
        {
        next_slot:
            n++;
            scan++;
            if (n < 0x100)
            {
                goto scan_slots;
            }
        }
        if (count == 0)
        {
            rec->unk25 = 0xFF;
            return -1;
        }
    }

    rec->unk27 = 0;
    rec->unk34 = 0;
    rec->unk25 = part->unkB;
    rec->unk44 = part->unk40 << 8;
    rec->unk48 = part->unk42 << 8;
    rec->unk4C = part->unk44 << 8;
    if (((part->unk28 >> 8) & 1) && (*(u32*)&part->unk2C & 0x0F000000))
    {
        if (part->unk34 & 0x10000)
        {
            val = (*(u32*)&part->unk2C >> 24) & 0xF;
            n = 0;
            if (val != 0)
            {
                do
                {
                    slot = &D_80105AE0[actor->owner_object_index];
                    if ((&slot->unk60)[n] == 0)
                    {
                        n++;
                    }
                    else
                    {
                        (&slot->unk60)[n] = (&slot->unk60)[n] - 1;
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

    switch ((part->unk28 >> 26) & 3)
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
    switch ((part->unk28 >> 28) & 3)
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
        rec->unk33 = 0x80 - rec->unk33;
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
            rec->unk19 = field_evaluate_parameter_track(actor, (((u16*)&part->unk4)[0] & 0xF) + 1);
            rec->unk1A = field_evaluate_parameter_track(actor, (((u16*)&part->unk4)[0] & 0xF) + 2);
        }
        else
        {
            rec->unk1A = rec->unk19 = rec->unk18 =
                field_evaluate_parameter_track(actor, (part->unk4 >> 16) & 0xF);
        }
    }
    rec->unk1C = (rec->unk1C & 0xFFFF7FFF) | ((part->unk4 * 0x10) & 0x8000);
    rec->unk1C = (rec->unk1C & 0xEFFFFFFF) | (((part->unk28 >> 25) & 1) << 28);
    func_80070CB8(actor, part, rec);
    RotMatrix_gte((FieldSVector*)&rec->unk10, mtx);
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
    speed = func_8007E754(actor, part);
    rec->unk1C = (rec->unk1C & ~0x1FF) | (speed & 0x1FF);
    rec->unk0 = (speed * vec->vx) >> 4;
    rec->unk4 = (speed * vec->vy) >> 4;
    rec->unk8 = (speed * vec->vz) >> 4;
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
            func_8006C460(rec, res);
            goto after_source;
        }
    }
    if (rec->unk25 == 2)
    {
        slot = &D_80105AE0[actor->owner_object_index];
        if (*(u8*)&slot->unk178 & 1)
        {
            if (((u8*)&slot->unk178)[2] != actor->unk233)
            {
                goto kill_rec;
            }
        }
        src = &D_800FDF58[actor->owner_object_index];
        if (!((part->unk4 >> 11) & 1) && !((part->unk28 >> 25) & 1) && (part->unk2C >> 5) == 0 &&
            (*(u32*)&part->unkC & 0xFFFF0000) == 0x80800000 && part->unk10 == 0x80)
        {
            rec->unk1C |= 0x10008000;
            rec->unk18 = D_800FE3A0[src->unk3A].unkE;
            rec->unk19 = D_800FE3A0[src->unk3A].unkF;
            rec->unk1A = D_800FE3A0[src->unk3A].unk10;
        }
        rec->unk3A = D_800FDF58[actor->owner_object_index].unk3A;
        rec->unk3B = D_800FDF58[actor->owner_object_index].unk3B;
        rec->unkC = D_800FDF58[actor->owner_object_index].unkC;
        rec->unk21 |= D_800FDF58[actor->owner_object_index].unk21 & 0x80;
        rec->unk1C = (rec->unk1C & 0xFFFCFFFF) |
                     ((((u16*)&D_800FDF58[actor->owner_object_index].unk1C)[1] & 3) << 16);
        rec->unk1C = (rec->unk1C & 0xFF87FFFF) | (D_800FDF58[actor->owner_object_index].unk1C & 0x780000);
        src = &D_800FDF58[actor->owner_object_index];
        goto attach_res;
    }
    if (rec->unk25 == 3)
    {
        track_obj = actor->unk229[g_field_track_index];
        if (track_obj == 0xFF)
        {
            rec->unk25 = track_obj;
            actor->unk3B[g_field_track_index][part_index]--;
            goto drop_slot;
        }
        if (!((D_80105AE0[actor->unk229[g_field_track_index]].unk178 >> 6) & 1))
        {
            if (actor->unk229[g_field_track_index] < 2U)
            {
                n = actor->unk229[g_field_track_index] * 0x1C;
            }
            else
            {
                n = 0x38;
            }
            val = ((Struct_D80105880*)((u8*)D_80105880 + n))->unkC;
            if (val == actor->unk229[g_field_track_index])
            {
                if ((u32)(val & 0xFF) < 2U)
                {
                    j = val * 0x1C;
                }
                else
                {
                    j = 0x38;
                }
                if (((Struct_D80105880*)((u8*)D_80105880 + j))->unk0 != 0)
                {
                    goto kill_rec;
                }
            }
        }
        slot = &D_80105AE0[actor->unk229[g_field_track_index]];
        if (*(u8*)&slot->unk178 & 1)
        {
            if (((u8*)&slot->unk178)[2] != actor->unk233)
            {
            kill_rec:
                rec->unk25 = 0xFF;
                goto after_source;
            }
        }
        src = &D_800FDF58[actor->unk229[g_field_track_index]];
        if (!((part->unk4 >> 11) & 1) && !((part->unk28 >> 25) & 1) && (part->unk2C >> 5) == 0 &&
            (*(u32*)&part->unkC & 0xFFFF0000) == 0x80800000 && part->unk10 == 0x80)
        {
            rec->unk1C |= 0x10008000;
            rec->unk18 = D_800FE3A0[src->unk3A].unkE;
            rec->unk19 = D_800FE3A0[src->unk3A].unkF;
            rec->unk1A = D_800FE3A0[src->unk3A].unk10;
        }
        rec->unk3A = D_800FDF58[actor->unk229[g_field_track_index]].unk3A;
        rec->unk3B = D_800FDF58[actor->unk229[g_field_track_index]].unk3B;
        rec->unkC = D_800FDF58[actor->unk229[g_field_track_index]].unkC;
        rec->unk21 |= D_800FDF58[actor->unk229[g_field_track_index]].unk21 & 0x80;
        rec->unk1C = (rec->unk1C & 0xFFFCFFFF) |
                     ((((u16*)&D_800FDF58[actor->unk229[g_field_track_index]].unk1C)[1] & 3) << 16);
        rec->unk1C =
            (rec->unk1C & 0xFF87FFFF) | (D_800FDF58[actor->unk229[g_field_track_index]].unk1C & 0x780000);
        src = &D_800FDF58[actor->unk229[g_field_track_index]];
    attach_res:
        res = g_field_resource_entries[src->unk3B].start;
        if (res != 0)
        {
            func_8006C3FC(rec, res);
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
    if ((rec->unk1C & 0x07000000) == 0x05000000)
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

    kind = (part->unk28 >> 18) & 0x3F;
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
                goto drop_slot;
            }
            n = actor->unk229[g_field_track_index];
            kind -= 0xA;
        }
        else
        {
            slot = &D_80105AE0[actor->owner_object_index];
            if ((*(u8*)&slot->unk178 & 1) && actor->unk233 >= 0x40U &&
                !((slot->unk178 >> 5) & 1) && ((u8*)&slot->unk178)[2] != actor->unk233)
            {
                goto fail_slot;
            }
            n = actor->owner_object_index;
        }
        src = &D_800FDF58[n];
        slot = &D_80105AE0[n];
        if ((part->unk28 >> 9) & 1)
        {
            val = slot->unk144 - slot->unk140;
            if (val < 0)
            {
                val = -val;
            }
            part->unk2E = val * 2;
        }
        j = 0;
        if ((part->unk28 >> 1) & 1)
        {
            val = slot->unk146 - slot->unk142;
            if (val < 0)
            {
                val = -val;
            }
            part->unk33 = val * 2;
        }
        count = 0;
        switch (kind)
        {
        case 1:
            count = (slot->unk144 + slot->unk140) >> 1;
            j = (slot->unk146 + slot->unk142) >> 1;
        default:
        shift_count:
            n = count << 8;
            break;
        case 2:
            j = 0;
        mid_x:
            count = (slot->unk144 + slot->unk140) >> 1;
            goto shift_count;
        case 3:
            j = slot->unk142;
            goto mid_x;
        case 4:
            count = slot->unk140;
            val = slot->unk146 + slot->unk142;
        mid_y:
            j = val >> 1;
            goto shift_count;
        case 5:
            count = slot->unk144;
            val = slot->unk146 + slot->unk142;
            goto mid_y;
        case 6:
            j = slot->unk142;
            n = slot->unk140 << 8;
            break;
        case 7:
            j = slot->unk142;
            n = slot->unk144 << 8;
            break;
        case 8:
            count = slot->unk140;
        bot_y:
            j = slot->unk146;
            goto shift_count;
        case 9:
            count = slot->unk144;
            goto bot_y;
        }
        rec->unk0 += src->unk0 + n;
        rec->unk4 += src->unk4 + (j << 8);
        rec->unk8 += src->unk8;
        if (rec->unk25 == 0xFD)
        {
            rec->unk3A = src->unk3A;
            slot = &D_80105AE0[src->unk3A];
            if (!(*(u8*)&slot->unk178 & 1) || ((u8*)&slot->unk178)[2] == actor->unk233)
            {
                rec->unkC = src->unkC;
                rec->unk3B = src->unk3B;
                rec->unk1C = (rec->unk1C & 0xFF87FFFF) | (src->unk1C & 0x780000);
                rec->unk1C = (rec->unk1C & 0xFFFCFFFF) | ((((u16*)&src->unk1C)[1] & 3) << 16);
                if (!((part->unk4 >> 11) & 1) && !((part->unk28 >> 25) & 1) && (part->unk2C >> 5) == 0 &&
                    (*(u32*)&part->unkC & 0xFFFF0000) == 0x80800000 && part->unk10 == 0x80)
                {
                    rec->unk1C |= 0x10008000;
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
                    goto maybe_attach;
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
        if (kind < 0x2A)
        {
            subA = kind - 0x14;
        }
        else
        {
            subA = kind - 0x22;
        }
        nA = start;
        if (nA < 0x100)
        {
            scanA = &D_800FF658[nA];
            scanA_rot = (FieldSVector*)&D_800FF658[nA].unk10;
            do
            {
                if (scanA->unk25 != 0xFF && scanA->unk23 == subA && scanA->unk22 == actor->unk233 &&
                    ((actor->unk0[subA].unk14 & 4) || scanA->unk29 == g_field_track_index))
                {
                    rec->unk0 += scanA->unk0;
                    rec->unk4 += scanA->unk4;
                    rec->unk8 += scanA->unk8;
                    rec->unk1C = (rec->unk1C & ~0x1000) | (scanA->unk1C & 0x1000);
                    if (part->unk1C & 0x08000000)
                    {
                        rec->unk10 = scanA_rot->unk0;
                        rec->unk12 = scanA_rot->unk2;
                        rec->unk14 = scanA_rot->unk4;
                    }
                    rec->unk30 = nA;
                    if (rec->unk25 == 0xFD)
                    {
                        rec->unk3B = scanA->unk3B;
                        rec->unkC = scanA->unkC;
                        rec->unk25 = scanA->unk25;
                        rec->unk3A = scanA->unk3A;
                        rec->unk1C = (rec->unk1C & 0xFF87FFFF) | (scanA->unk1C & 0x780000);
                        rec->unk1C = (rec->unk1C & 0xFFFCFFFF) | ((((u16*)&scanA->unk1C)[1] & 3) << 16);
                        if (rec->unk21 != 0xFF)
                        {
                            rec->unk3A = scanA->unk3A;
                            rec->unk25 = scanA->unk25;
                            rec->unk1C = (rec->unk1C & 0xFF87FFFF) | (scanA->unk1C & 0x780000);
                            rec->unk1C = (rec->unk1C & 0xFFFCFFFF) | ((((u16*)&scanA->unk1C)[1] & 3) << 16);
                            rec->unk27 = scanA->unk27;
                            rec->unk21 = scanA->unk21;
                            res = g_field_resource_entries
                                      [D_800FDF58[g_field_actor_slots[rec->unk22].owner_object_index].unk3B]
                                          .start;
                            if (res != 0)
                            {
                                func_8006C460(rec, res);
                            }
                        }
                        else
                        {
                            rec->unk21 = scanA->unk21;
                            rec->unk27 = scanA->unk27;
                            rec->unk34 = scanA->unk34;
                            rec->unk35 = scanA->unk35;
                            rec->unk29 = scanA->unk29;
                            rec->unk36 = scanA->unk36;
                            rec->unk37 = scanA->unk37;
                            rec->unk38 = scanA->unk38;
                        }
                    }
                    break;
                }
                scanA_rot = (FieldSVector*)((u8*)scanA_rot + 0x54);
                nA++;
                scanA++;
            } while (nA < 0x100);
        }
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
        goto check_dead;

    case 0x1C:
        rec->unk4 -= D_800F22A4;
        rec->unk0 -= D_800F22A0;
        goto sub_z;

    case 0x1D:
        val = rec->unk4 - 0x7000;
        goto set_y;

    case 0x1E:
        val = rec->unk4 + 0x7000;
    set_y:
        rec->unk4 = val;
        rec->unk8 -= D_800F22A8;
        rec->unk0 -= D_800F22A0;
        rec->unk1C |= 0x1000;
        rec->unk4 -= D_800F22A4;
        break;

    case 0x1F:
        j = 0xFFFF6000;
        goto add_x;

    case 0x20:
        j = 0xA000;
    add_x:
        rec->unk0 = rec->unk0 + j;
        rec->unk0 -= D_800F22A0;
        rec->unk4 -= D_800F22A4;
    sub_z:
        rec->unk8 -= D_800F22A8;
        rec->unk1C |= 0x1000;
        break;

    case 0x21:
        val = rec->unk0 + 0xFFFF6000;
        n = rec->unk4 - 0x7000;
        goto set_xy;

    case 0x22:
        val = rec->unk0 + 0xA000;
        n = rec->unk4 - 0x7000;
        goto set_xy;

    case 0x23:
        j = 0xFFFF6000;
        goto add_xy;

    case 0x24:
        j = 0xA000;
    add_xy:
        val = rec->unk0 + j;
        n = rec->unk4 + 0x7000;
    set_xy:
        rec->unk4 = n;
        rec->unk0 = val;
        rec->unk1C |= 0x1000;
        rec->unk8 -= D_800F22A8;
        rec->unk0 -= D_800F22A0;
        rec->unk4 -= D_800F22A4;
        break;

    case 0x27:
        slot27 = &D_80105AE0[actor->owner_object_index];
        if ((*(u8*)&slot27->unk178 & 1) && ((u8*)&slot27->unk178)[2] != actor->unk233 &&
            actor->unk233 >= 0x40U && !((slot27->unk178 >> 5) & 1))
        {
            goto fail_slot;
        }
        src27 = &D_800FDF58[actor->owner_object_index];
        if (((part->unk28 >> 10) & 1) && !(src27->unk21 & 0x80))
        {
            pos27 = src27->unk0 - (part->unk38 << 8);
        }
        else
        {
            pos27 = src27->unk0 + (part->unk38 << 8);
        }
        rec->unk0 += pos27;
        rec->unk4 += src27->unk4 + (part->unk3A << 8);
        rec->unk8 += src27->unk8 + (part->unk3C << 8);
        if (rec->unk25 == 0xFD && rec->unk21 == 0xFF)
        {
            rec->unk3A = src27->unk3A;
            slot27 = &D_80105AE0[src27->unk3A];
            if (*(u8*)&slot27->unk178 & 1)
            {
                if (((u8*)&slot27->unk178)[2] != actor->unk233)
                {
                    goto mark_dead;
                }
            }
            rec->unk3B = src27->unk3B;
            rec->unkC = src27->unkC;
            rec->unk21 = src27->unk21;
            rec->unk27 = src27->unk27;
            rec->unk1C = (rec->unk1C & 0xFF87FFFF) | (src27->unk1C & 0x780000);
            rec->unk1C = (rec->unk1C & 0xFFFCFFFF) | ((((u16*)&src27->unk1C)[1] & 3) << 16);
            res = g_field_resource_entries[src27->unk3B].start;
            goto maybe_attach;
        }
        break;

    case 0x28:
        track_obj = actor->unk229[g_field_track_index];
        if (track_obj == 0xFF)
        {
            rec->unk25 = track_obj;
            actor->unk3B[g_field_track_index][part_index]--;
            goto drop_slot;
        }
        src28 = &D_800FDF58[actor->unk229[g_field_track_index]];
        if ((part->unk34 & 0x08000000) && !(D_800FDF58[actor->owner_object_index].unk21 & 0x80))
        {
            pos28 = src28->unk0 - (part->unk38 << 8);
        }
        else if (((part->unk28 >> 10) & 1) && !(src28->unk21 & 0x80))
        {
            pos28 = src28->unk0 - (part->unk38 << 8);
        }
        else
        {
            pos28 = src28->unk0 + (part->unk38 << 8);
        }
        rec->unk0 += pos28;
        rec->unk4 += src28->unk4 + (part->unk3A << 8);
        rec->unk8 += src28->unk8 + (part->unk3C << 8);
        if (rec->unk25 == 0xFD && rec->unk21 == 0xFF)
        {
            rec->unk3A = src28->unk3A;
            slot28 = &D_80105AE0[src28->unk3A];
            if (*(u8*)&slot28->unk178 & 1)
            {
                if (((u8*)&slot28->unk178)[2] != actor->unk233)
                {
                    goto mark_dead;
                }
            }
            rec->unk3B = src28->unk3B;
            rec->unkC = src28->unkC;
            rec->unk21 = src28->unk21;
            rec->unk27 = src28->unk27;
            rec->unk1C = (rec->unk1C & 0xFF87FFFF) | (src28->unk1C & 0x780000);
            rec->unk1C = (rec->unk1C & 0xFFFCFFFF) | ((((u16*)&src28->unk1C)[1] & 3) << 16);
            res = g_field_resource_entries[src28->unk3B].start;
            goto maybe_attach;
        }
        break;

    case 0x29:
        slot = &D_80105AE0[actor->owner_object_index];
        src = &D_800FDF58[actor->owner_object_index];
        rec->unk0 += src->unk0 + (slot->unk130[(part->unk24 >> 21) & 3].x << 8);
        rec->unk4 += src->unk4 + (slot->unk130[(part->unk24 >> 21) & 3].y << 8);
        rec->unk8 += src->unk8;
        if (rec->unk25 == 0xFD)
        {
            rec->unk3A = src->unk3A;
            rec->unk1C = (rec->unk1C & 0xFF87FFFF) | (src->unk1C & 0x780000);
            rec->unk1C = (rec->unk1C & 0xFFFCFFFF) | ((((u16*)&src->unk1C)[1] & 3) << 16);
            slot = &D_80105AE0[src->unk3A];
            goto check_owner;
        }
        break;

    case 0x32:
        slot = &D_80105AE0[actor->owner_object_index];
        src = &D_800FDF58[actor->owner_object_index];
        rec->unk0 += src->unk0 + (slot->unk130[(part->unk24 >> 21) & 3].x << 8);
        rec->unk4 += src->unk4;
        rec->unk8 += src->unk8 + (part->unk3C << 8);
        if (((part->unk28 >> 10) & 1) && !(src->unk21 & 0x80))
        {
            val = rec->unk0 - (part->unk38 << 8);
        }
        else
        {
            val = rec->unk0 + (part->unk38 << 8);
        }
        rec->unk0 = val;
        if (rec->unk25 == 0xFD)
        {
            rec->unk1C = (rec->unk1C & 0xFF87FFFF) | (src->unk1C & 0x780000);
            rec->unk1C = (rec->unk1C & 0xFFFCFFFF) | ((((u16*)&src->unk1C)[1] & 3) << 16);
            rec->unk3A = src->unk3A;
            slot = &D_80105AE0[src->unk3A];
        check_owner:
            if (!(*(u8*)&slot->unk178 & 1) || ((u8*)&slot->unk178)[2] == actor->unk233)
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
                    func_8006C3FC(rec, res);
                }
                break;
            }
            goto mark_dead;
        }
        break;

    case 0x33:
        src = &D_800FDF58[actor->owner_object_index];
        slot = &D_80105AE0[actor->owner_object_index];
        rec->unk0 += src->unk0 + (slot->unk190[D_80105760].x << 8);
        rec->unk4 += src->unk4;
        rec->unk1C = (rec->unk1C & ~0x6000) | ((D_80105760 & 3) << 13);
        rec->unk8 += src->unk8 + (slot->unk190[D_80105760].y << 8);
        break;

    case 0x34:
        track_obj = actor->unk229[g_field_track_index];
        if (track_obj == 0xFF)
        {
            rec->unk25 = track_obj;
            actor->unk3B[g_field_track_index][part_index]--;
            goto drop_slot;
        }
        src = &D_800FDF58[actor->unk229[g_field_track_index]];
        if (!(src->unk21 & 0x80))
        {
            val = src->unk0 + (actor->unk1FE[g_field_track_index].x << 8);
        }
        else
        {
            val = src->unk0 - (actor->unk1FE[g_field_track_index].x << 8);
        }
        rec->unk0 += val;
        rec->unk4 += src->unk4 + (actor->unk1FE[g_field_track_index].y << 8);
        val = rec->unk8 + src->unk8;
        goto set_z;

    case 0x35:
        rec->unk0 += D_80105778.vx;
        rec->unk4 += D_80105778.vy;
        val = rec->unk8 + D_80105778.vz;
    set_z:
        rec->unk8 = val;
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

    case 0x37:
    case 0x38:
    case 0x39:
    case 0x3A:
    case 0x3B:
    case 0x3C:
    case 0x3D:
    case 0x3E:
        nB = start;
        subB = kind - 0x37;
        if (nB < 0x100)
        {
            scanB = &D_800FF658[nB];
            scanB_rot = (FieldSVector*)&D_800FF658[nB].unk10;
            do
            {
                if (scanB->unk25 != 0xFF && scanB->unk23 == subB && scanB->unk22 == actor->unk233 &&
                    ((actor->unk0[subB].unk14 & 4) || scanB->unk29 == g_field_track_index))
                {
                    rec->unk0 += scanB->unk0;
                    rec->unk4 += scanB->unk4;
                    rec->unk8 += scanB->unk8;
                    if ((part->unk34 & 0x08000000) && !(D_800FDF58[actor->owner_object_index].unk21 & 0x80))
                    {
                        pos37 = rec->unk0 - (part->unk38 << 8);
                    }
                    else if (((part->unk28 >> 10) & 1) && !(scanB->unk21 & 0x80))
                    {
                        pos37 = rec->unk0 - (part->unk38 << 8);
                    }
                    else
                    {
                        pos37 = rec->unk0 + (part->unk38 << 8);
                    }
                    rec->unk0 = pos37;
                    rec->unk4 += part->unk3A << 8;
                    rec->unk8 += part->unk3C << 8;
                    rec->unk1C = (rec->unk1C & ~0x1000) | (scanB->unk1C & 0x1000);
                    if (part->unk1C & 0x08000000)
                    {
                        rec->unk10 = scanB_rot->unk0;
                        rec->unk12 = scanB_rot->unk2;
                        rec->unk14 = scanB_rot->unk4;
                    }
                    rec->unk30 = nB;
                    if (rec->unk25 == 0xFD)
                    {
                        rec->unk3B = scanB->unk3B;
                        rec->unkC = scanB->unkC;
                        rec->unk25 = scanB->unk25;
                        rec->unk3A = scanB->unk3A;
                        rec->unk1C = (rec->unk1C & 0xFF87FFFF) | (scanB->unk1C & 0x780000);
                        rec->unk1C = (rec->unk1C & 0xFFFCFFFF) | ((((u16*)&scanB->unk1C)[1] & 3) << 16);
                        if (rec->unk21 != 0xFF)
                        {
                            rec->unk3A = scanB->unk3A;
                            rec->unk25 = scanB->unk25;
                            rec->unk1C = (rec->unk1C & 0xFF87FFFF) | (scanB->unk1C & 0x780000);
                            rec->unk1C = (rec->unk1C & 0xFFFCFFFF) | ((((u16*)&scanB->unk1C)[1] & 3) << 16);
                            rec->unk27 = scanB->unk27;
                            rec->unk21 = scanB->unk21;
                            res = g_field_resource_entries
                                      [D_800FDF58[g_field_actor_slots[rec->unk22].owner_object_index].unk3B]
                                          .start;
                            if (res != 0)
                            {
                                func_8006C460(rec, res);
                            }
                        }
                        else
                        {
                            rec->unk21 = scanB->unk21;
                            rec->unk27 = scanB->unk27;
                            rec->unk34 = scanB->unk34;
                            rec->unk35 = scanB->unk35;
                            rec->unk29 = scanB->unk29;
                            rec->unk36 = scanB->unk36;
                            rec->unk37 = scanB->unk37;
                            rec->unk38 = scanB->unk38;
                        }
                    }
                    break;
                }
                scanB_rot = (FieldSVector*)((u8*)scanB_rot + 0x54);
                nB++;
                scanB++;
            } while (nB < 0x100);
        }
        if (nB == 0x100)
        {
            goto fail_slot;
        }
        func_8006D79C(actor, part_index, nB + 1);
        break;

    fail_slot:
        rec->unk25 = 0xFF;
        actor->unk3B[g_field_track_index][part_index]--;
    drop_slot:
        actor->unkCC[g_field_track_index][part_index]--;
        return -1;
    }

        switch ((part->unk24 >> 29) & 3)
        {
        case 1:
            vec->vx = (D_800FDF58[actor->owner_object_index].unk0 - rec->unk0) >> 8;
            vec->vy = (D_800FDF58[actor->owner_object_index].unk4 - rec->unk4) >> 8;
            val = (D_800FDF58[actor->owner_object_index].unk8 - rec->unk8) >> 8;
            vec->vz = val;
            rec->unk12 = ratan2(-val, vec->vx);
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
            val = (D_800FDF58[actor->unk229[g_field_track_index]].unk8 - rec->unk8) >> 8;
            vec->vz = val;
            rec->unk12 = ratan2(-val, vec->vx);
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
        if ((rec->unk1C & 0x07000000) == 0x05000000)
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
            func_80070E4C(rec, part);
        }
        if (!(rec->unk1C & 0x07000000) && rec->unk1B != 0)
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
                rec->unk4 = field_evaluate_parameter_track_at_time(actor, (part->unk28 >> 4) & 0xF, 0) * -0x100;
            }
        }
        if (part->unk28 & 1)
        {
            rec->unk8 += 0x80;
        }
        switch ((rec->unk1C >> 29) & 3)
        {
        case 1:
            rec->unk2A = field_evaluate_parameter_track(actor, ((u16*)&part->unk14)[1] & 0xF);
            break;
        case 2:
            rec->unk2A = field_evaluate_parameter_track_at_time(actor, ((u16*)&part->unk14)[1] & 0xF, 0);
            break;
        case 3:
            func_80073F7C(rec, part, vec);
            vec->vx -= rec->unk0;
            vec->vy -= rec->unk4;
            vec->vx = vec->vx >> 8;
            vec->vz -= rec->unk8;
            vec->vz = vec->vz >> 8;
            vec->vy = vec->vy >> 8;
            GTE_LDLVL(vec);
            GTE_SQR0();
            GTE_STLVNL(sqr);
            val = SquareRoot0(sqr->vx + sqr->vy + sqr->vz) << 8;
            if (((s16*)&part->unk14)[1] != 0)
            {
                val = val / ((s16*)&part->unk14)[1];
            }
            rec->unk2A = (u32)val >> 2;
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
                    D_80105AE0[actor->unk229[g_field_track_index]].unk178 |= 1;
                    ((u8*)&D_80105AE0[actor->unk229[g_field_track_index]].unk178)[2] = actor->unk233;
                    ((u8*)&actor->unk224)[1] = 1;
                }
            }
            if ((actor->unkC->unk18 & 0xA) == 0xA)
            {
                if (((actor->unkC->unk18 >> 8) & 0xF) == part_index)
                {
                    D_800FDF58[actor->owner_object_index].unk25 = 0xFE;
                    D_80105AE0[actor->owner_object_index].unk178 |= 1;
                    ((u8*)&D_80105AE0[actor->owner_object_index].unk178)[2] = actor->unk233;
                }
            }
        }
        field_dispatch_actor_audio_event(actor, 2, part_index);
        field_dispatch_actor_audio_event(actor, 5, part_index);
        return i;


}
