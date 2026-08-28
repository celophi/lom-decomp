#include "common.h"
#include "psyq_compat/libgte.h"
#include "psyq_compat/libgpu.h"
#include "psyq_compat/rand.h"

typedef struct
{
    s16 m[3][3];
    s32 t[3];
} FieldMatrix;

typedef struct
{
    s16 vx;
    s16 vy;
    s16 vz;
    s16 pad;
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
    u32 unk14; /* 0x14 */
    s16 unk18; /* 0x18 */
    u8 unk1A;  /* 0x1A */
    u8 pad1B;
    u32 unk1C; /* 0x1C */
    u8 unk20;  /* 0x20 */
    u8 unk21;  /* 0x21 */
    u8 unk22;  /* 0x22 */
    u8 unk23;  /* 0x23 */
    u32 unk24; /* 0x24 */
    u32 unk28; /* 0x28 */
    u8 unk2C;  /* 0x2C (also read as a whole word) */
    u8 unk2D;  /* 0x2D */
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

typedef struct
{
    FieldActorPartDef *unk0; /* 0x000 */
    u8 pad4[0x228 - 0x004];
    u8 unk228;               /* 0x228 */
    u8 unk229[9];            /* 0x229 */
    u8 pad232[0x244 - 0x232];
} FieldActorState;

typedef struct
{
    u8 pad0[0x68];
    u16 unk68; /* 0x68 */
    u8 pad6A[0x140 - 0x6A];
    s16 unk140; /* 0x140 */
    s16 unk142; /* 0x142 */
    s16 unk144; /* 0x144 */
    s16 unk146; /* 0x146 */
    u8 pad148[0x174 - 0x148];
    u16 unk174; /* 0x174 */
    u8 pad176[0x23C - 0x176];
} Struct_D80105AE0;

typedef struct
{
    s16 unk0;
    s16 unk2;
    s16 unk4;
} Struct_D80105768;

extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;
extern u8 D_800EC37C[];
extern u16 D_800EC388[];
extern Struct_D800FDF58 D_800FDF58[];
extern Struct_D80105AE0 D_80105AE0[];
extern Struct_D80105768 D_80105768;
extern FieldActorState g_field_actor_slots[80];
extern s32 g_field_track_index;

s32 field_evaluate_parameter_track(FieldActorState *actor, s32 track);
s32 field_evaluate_parameter_track_at_time(FieldActorState *actor, u32 track, u16 time);
void func_80073F7C(Struct_D800FDF58 *rec, FieldActorPartDef *part, FieldVector *out);
s32 func_8007D078(Struct_D800FDF58 *rec, FieldActorPartDef *part, FieldMatrix *mtx, FieldActorState *actor);
void func_8007D8D8(FieldActorState *actor, Struct_D800FDF58 *rec, FieldActorPartDef *part, u8 *out);
u8 *func_8007DA80(Struct_D800FDF58 *rec, FieldActorPartDef *part, u8 *primbuf, s32 *base);

#include "psyq_compat/inline_c.h"
#include "psyq_compat/gte_dmpsx_compat.h"

/**
 * @brief Field ribbon/trail primitive builder: derives the orientation matrix
 *        for the actor part, seeds a chain of randomly rotated matrices in the
 *        scratchpad, then walks the chain emitting textured quads that sweep
 *        from the effect origin out to the target point returned by
 *        func_80073F7C, threading each quad into the depth-indexed ordering
 *        table in base[].
 * @param rec Effect record supplying position, flags and segment count (unk24).
 * @param primbuf Output primitive buffer; advanced 0x28 bytes per emitted quad.
 * @param base Depth-indexed ordering-table / primitive base array.
 * @return The advanced primbuf cursor.
 * @note WIP - not yet byte-matching. Residue is spread across the emit loop:
 *       98 argdiff rows plus a four-slot shuffle of the sp+0x1C..0x28 spill
 *       block, i.e. the callee-saved/spill assignment for the loop-carried
 *       values differs from the target rather than the codegen shape.
 * @see decomp.me (95.42%) WIP
 */
u8 *func_8007C3F8(Struct_D800FDF58 *rec, u8 *primbuf, s32 *base)
{
    FieldActorPartDef *part;
    FieldActorState *state;
    FieldVector *gte_out;
    FieldVector *ptr_a;
    FieldVector *ptr_b;
    FieldVector *ptr_c;
    FieldMatrix *cur;
    s32 segments;
    s32 i;
    s32 amp;
    s32 step;
    s32 angle;
    s32 off_x;
    FieldSVector *dir;
    s32 temp_v1;
    u8 uvflags;
    u8 prim_code;

    gte_out = (FieldVector *) 0x1F800010;
    ptr_a = (FieldVector *) 0x1F800020;
    ptr_b = (FieldVector *) 0x1F800030;
    ptr_c = (FieldVector *) 0x1F800040;
    dir = (FieldSVector *) 0x1F800050;

    part = &g_field_actor_slots[rec->unk22].unk0[rec->unk23];
    state = &g_field_actor_slots[rec->unk22];
    func_8007D078(rec, part, (FieldMatrix *) 0x1F800058, state);

    cur = (FieldMatrix *) 0x1F800058;
    uvflags = D_800EC37C[((u16) rec->unk2C) % 12];
    *(u16 *) (primbuf + 0xC) = D_800EC388[(uvflags & 1) * 4 + 0];
    *(u16 *) (primbuf + 0x14) = D_800EC388[(uvflags & 1) * 4 + 1];
    *(u16 *) (primbuf + 0x1C) = D_800EC388[(uvflags & 1) * 4 + 2];
    *(u16 *) (primbuf + 0x24) = D_800EC388[(uvflags & 1) * 4 + 3];

    if (uvflags & 0x80)
    {
        i = *(u8 *) (primbuf + 0xC);
        *(u8 *) (primbuf + 0xC) = *(u8 *) (primbuf + 0x14);
        *(u8 *) (primbuf + 0x14) = i;
        i = *(u8 *) (primbuf + 0x1C);
        *(u8 *) (primbuf + 0x1C) = *(u8 *) (primbuf + 0x24);
        *(u8 *) (primbuf + 0x24) = i;
    }
    if (uvflags & 0x40)
    {
        i = *(u8 *) (primbuf + 0xD);
        *(u8 *) (primbuf + 0xD) = *(u8 *) (primbuf + 0x1D);
        *(u8 *) (primbuf + 0x1D) = i;
        i = *(u8 *) (primbuf + 0x15);
        *(u8 *) (primbuf + 0x15) = *(u8 *) (primbuf + 0x25);
        *(u8 *) (primbuf + 0x25) = i;
    }

    {
        u32 part_unk4 = part->unk4;
        *(u16 *) (primbuf + 0xE) = 0x7B05;
        *(u16 *) (primbuf + 0x16) = (u16) (((part_unk4 >> 17) & 0x60) | 7);
    }

    gte_SetRotMatrix(cur);

    {
        s32 first_d0;
        s32 temp_x;
        s32 raw_d4;
        first_d0 = D_800F22A0 / 256;
        temp_x = rec->unk0 / 256 + 0xA0;
        raw_d4 = D_800F22A4;
        *(s16 *) (primbuf + 8) = (s16) (first_d0 + temp_x);
        if (raw_d4 < 0)
        {
            raw_d4 += 0xFF;
        }
        *(s16 *) (primbuf + 0xA) = (s16) (0x70 + (raw_d4 >> 8) + rec->unk4 / 256 - rec->unk8 / 512 - D_800F22A8 / 512);
    }

    func_8007D8D8(state, rec, part, primbuf + 4);
    *(s8 *) (primbuf + 3) = 9;
    *(s8 *) (primbuf + 7) = 0x2C;
    ((rec->unk1C & 0x800000) ? (*(u8 *) (primbuf + 7) = *(u8 *) (primbuf + 7) | 2) : (*(u8 *) (primbuf + 7) = *(u8 *) (primbuf + 7) & ~2));

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

    func_80073F7C(rec, part, (FieldVector *) 0x1F800000);
    {
        volatile s32 *scratch = (volatile s32 *) 0x1F800000;
        ptr_a->vx = (scratch[0] + rec->unk0) >> 1;
        ptr_a->vy = scratch[1];
        ptr_a->vz = (rec->unk8 + scratch[2]) >> 1;
        ptr_c->vx = *(s32 *) 0x1F800000 - rec->unk0;
        ptr_b->vx = ptr_c->vx >> 1;
        ptr_c->vy = rec->unk4 - *(s32 *) 0x1F800004;
        ptr_b->vy = ptr_c->vy;
        i = segments - 1;
        ptr_c->vz = *(s32 *) 0x1F800008 - rec->unk8;
        ptr_b->vz = ptr_c->vz >> 1;
    }

    if (i > 0)
    {
        do
        {
            dir->vx = 0;
            dir->vy = (s16) ((rand() << 12) >> 15);
            dir->vz = (s16) ((rand() << 12) >> 16);
            RotMatrix_gte(dir, cur);
            i--;
            cur++;
        } while (i > 0);
    }

    cur = (FieldMatrix *) 0x1F800058;

    if ((*(u8 *) &part->unk4) >> 7)
    {
        dir->vx = 0;
        dir->vy = (s16) ((part->unk4 >> 28) << 8);
        dir->vz = 0;
    }
    else
    {
        *(s32 *) &dir->vz = 0;
        *(s32 *) &dir->vx = 0;
    }

    if (((part->unk0 >> 6) & 3) != 0)
    {
        amp = (part->unk0 >> 26) << 10;
    }
    else
    {
        amp = 0;
    }
    amp = (amp * rand()) >> 15;
    if (rand() & 1)
    {
        amp = -amp;
    }

    angle = ratan2(ptr_c->vx, ptr_c->vy + (ptr_c->vz >> 1));
    off_x = (rcos(angle) * part->unk33) >> 12;
    angle = (rsin(angle) * part->unk33) >> 12;
    if (uvflags & 1)
    {
        angle *= 2;
        off_x *= 2;
    }

    i = segments - 1;
    *(s16 *) (primbuf + 0x18) = *(s16 *) (primbuf + 8) + off_x;
    *(s16 *) (primbuf + 0x1A) = *(s16 *) (primbuf + 0xA) + angle;
    *(s16 *) (primbuf + 0xA) -= angle;
    *(s16 *) (primbuf + 8) -= off_x;

    if (i > 0)
    {
        s32 angle_step;
        s32 addr_mask = 0xFFFFFF;
        s32 tag_mask = 0xFF000000;
        angle_step = i * step;
        do
        {
            temp_v1 = *(s32 *) (primbuf + 0x4);
            *(s8 *) (primbuf + 0x3) = 9;
            *(s8 *) (primbuf + 0x7) = 0x2C;
            *(s32 *) (primbuf + 0x2C) = temp_v1;
            prim_code = 0x2E;
            if (!(rec->unk1C & 0x800000))
            {
                prim_code = 0x2C;
            }
            *(u8 *) (primbuf + 7) = prim_code;

            gte_SetRotMatrix(cur);
            gte_ldv0(dir);
            gte_rtv0();
            gte_stlvnl(ptr_c);

            if (amp != 0)
            {
                gte_out->vy = ptr_a->vy + (ptr_b->vy * i) / segments - ((amp * rsin(angle_step)) >> 12) + ptr_c->vy;
            }
            else
            {
                gte_out->vy = ptr_a->vy + (ptr_b->vy * i) / segments + ptr_c->vy;
            }
            gte_out->vx = ((ptr_b->vx * rcos(angle_step)) >> 12) + ptr_a->vx + ptr_c->vx;
            gte_out->vz = ((ptr_b->vz * rcos(angle_step)) >> 12) + ptr_a->vz + ptr_c->vz;

            {
                s32 first_d0;
                s32 temp_x;
                s32 raw_d4;
                first_d0 = D_800F22A0 / 256;
                temp_x = gte_out->vx / 256 + 0xA0;
                raw_d4 = D_800F22A4;
                *(s16 *) (primbuf + 0x10) = (s16) (first_d0 + temp_x);
                if (raw_d4 < 0)
                {
                    raw_d4 += 0xFF;
                }
                *(s16 *) (primbuf + 0x12) = (s16) (0x70 + (raw_d4 >> 8) + gte_out->vy / 256 - gte_out->vz / 512 - D_800F22A8 / 512);
            }
            *(s16 *) (primbuf + 0x20) = *(s16 *) (primbuf + 0x10) + off_x;
            *(s16 *) (primbuf + 0x22) = *(s16 *) (primbuf + 0x12) + angle;
            *(s16 *) (primbuf + 0x12) -= angle;
            *(s16 *) (primbuf + 0x10) -= off_x;

            {
                u16 copy_a;
                u16 copy_e;
                copy_a = *(u16 *) (primbuf + 0xC);
                *(u16 *) (primbuf + 0x4C) = *(u16 *) (primbuf + 0x24);
                *(u16 *) (primbuf + 0x34) = copy_a;
                *(u16 *) (primbuf + 0x3E) = *(u16 *) (primbuf + 0x16);
                *(s32 *) (primbuf + 0x40) = *(s32 *) (primbuf + 0x20);
                *(s32 *) (primbuf + 0x30) = *(s32 *) (primbuf + 0x10);
                copy_e = *(u16 *) (primbuf + 0xE);
                *(u16 *) (primbuf + 0x3C) = *(u16 *) (primbuf + 0x14);
                *(u16 *) (primbuf + 0x44) = *(u16 *) (primbuf + 0x1C);
                *(u16 *) (primbuf + 0x36) = copy_e;
            }

            temp_v1 = (s32) rec->unk8 >> 7;
            if (temp_v1 < 0)
            {
                s32 addr = (s32) primbuf & addr_mask;
                *(s32 *) primbuf = (*(s32 *) primbuf & tag_mask) | (base[0] & addr_mask);
                primbuf += 0x28;
                base[0] = (base[0] & tag_mask) | addr;
            }
            else if (temp_v1 >= 0x1000)
            {
                s32 addr = (s32) primbuf & addr_mask;
                *(s32 *) primbuf = (*(s32 *) primbuf & tag_mask) | (base[0xFFF] & addr_mask);
                primbuf += 0x28;
                base[0xFFF] = (base[0xFFF] & tag_mask) | addr;
            }
            else
            {
                s32 addr;
                s32 *entry;
                addr = (s32) primbuf & addr_mask;
                *(s32 *) primbuf = (*(s32 *) primbuf & tag_mask) | (base[temp_v1] & addr_mask);
                entry = (s32 *) ((((s32) rec->unk8 >> 7) << 2) + (s32) base);
                primbuf += 0x28;
                *entry = (*entry & tag_mask) | addr;
            }
            angle_step -= step;
            i--;
            cur++;
        } while (i > 0);
    }

    *(s8 *) (primbuf + 3) = 9;
    *(s8 *) (primbuf + 7) = 0x2C;
    ((rec->unk1C & 0x800000) ? (*(u8 *) (primbuf + 7) = *(u8 *) (primbuf + 7) | 2) : (*(u8 *) (primbuf + 7) = *(u8 *) (primbuf + 7) & ~2));

    {
        s32 first_d0;
        s32 temp_x;
        s32 raw_d4;
        first_d0 = D_800F22A0 / 256;
        temp_x = *(s32 *) 0x1F800000 / 256 + 0xA0;
        raw_d4 = D_800F22A4;
        *(s16 *) (primbuf + 0x10) = (s16) (first_d0 + temp_x);
        if (raw_d4 < 0)
        {
            raw_d4 += 0xFF;
        }
        *(s16 *) (primbuf + 0x12) = (s16) (0x70 + (raw_d4 >> 8) + *(s32 *) 0x1F800004 / 256 - *(s32 *) 0x1F800008 / 512 - D_800F22A8 / 512);
    }
    *(s16 *) (primbuf + 0x20) = *(s16 *) (primbuf + 0x10) + off_x;
    *(s16 *) (primbuf + 0x22) = *(s16 *) (primbuf + 0x12) + angle;
    *(s16 *) (primbuf + 0x12) -= angle;
    *(s16 *) (primbuf + 0x10) -= off_x;

    temp_v1 = (s32) rec->unk8 >> 7;
    if (temp_v1 < 0)
    {
        s32 addr = (s32) primbuf & 0xFFFFFF;
        *(s32 *) primbuf = (*(s32 *) primbuf & 0xFF000000) | (base[0] & 0xFFFFFF);
        primbuf += 0x28;
        base[0] = (base[0] & 0xFF000000) | addr;
    }
    else if (temp_v1 >= 0x1000)
    {
        s32 addr = (s32) primbuf & 0xFFFFFF;
        *(s32 *) primbuf = (*(s32 *) primbuf & 0xFF000000) | (base[0xFFF] & 0xFFFFFF);
        primbuf += 0x28;
        base[0xFFF] = (base[0xFFF] & 0xFF000000) | addr;
    }
    else
    {
        s32 addr;
        s32 *entry;
        addr = (s32) primbuf & 0xFFFFFF;
        *(s32 *) primbuf = (*(s32 *) primbuf & 0xFF000000) | (base[temp_v1] & 0xFFFFFF);
        entry = (s32 *) ((((s32) rec->unk8 >> 7) << 2) + (s32) base);
        primbuf += 0x28;
        *entry = (*entry & 0xFF000000) | addr;
    }
    return primbuf;
}

/**
 * @brief Build the full orientation/scale matrix for one animated actor part.
 *
 * Starts from identity, applies the track-driven rotation selected by the
 * two-bit mode in part->unk0, then the fixed billboard corrections, the
 * optional aim-at-target orientation, the stretch-to-target scale and finally
 * the per-part and track-driven scales.
 *
 * @param rec Effect record supplying position, angles and the animation time
 *            (unk2C) used to sample parameter tracks.
 * @param part Part definition holding the rotation/scale mode bitfields.
 * @param mtx Destination matrix (scratchpad); overwritten with identity first.
 * @param actor Owning actor state, used as the parameter-track source.
 * @return Unspecified; the assembly leaves v0 undefined and no caller uses it.
 * @see decomp.me (100%)
 */
s32 func_8007D078(Struct_D800FDF58 *rec, FieldActorPartDef *part, FieldMatrix *mtx, FieldActorState *actor)
{
    FieldSVector *dir = (FieldSVector *) 0x1F8000C0;
    FieldVector *scale = (FieldVector *) 0x1F8000C8;
    FieldVector *delta = (FieldVector *) 0x1F8000D8;
    FieldVector *sqr = (FieldVector *) 0x1F8000E8;
    s32 angle;
    s32 final_angle;
    s32 axis;
    s32 mode;
    u32 dist;
    s32 temp;
    u32 flags;

    ((u32 *) mtx)[4] = 0x1000;
    ((u32 *) mtx)[2] = 0x1000;
    ((u32 *) mtx)[0] = 0x1000;
    ((u32 *) mtx)[7] = 0;
    ((u32 *) mtx)[6] = 0;
    ((u32 *) mtx)[5] = 0;
    ((u32 *) mtx)[3] = 0;
    ((u32 *) mtx)[1] = 0;

    flags = part->unk0;
    temp = (flags >> 6) & 3;
    if (temp == 0)
    {
        goto after_track_rotation;
    }
    switch (temp)
    {
    case 1:
        angle = field_evaluate_parameter_track_at_time(actor, flags >> 26, rec->unk2C) << 4;
        axis = ((u8 *) part)[3];
        axis &= 3;
        break;
    case 2:
        angle = rec->unk12;
        axis = 1;
        break;
    case 3:
        angle = (((flags >> 26) * rec->unk2C) << 4) & 0xFFF;
        axis = flags >> 24;
        axis &= 3;
        break;
    default:
        break;
    }

    if (part->unk34 & 0x20000)
    {
        if (!(D_800FDF58[actor->unk228].unk21 & 0x80))
        {
            angle = -angle;
        }
    }

    switch (axis)
    {
    case 0:
        RotMatrixX(angle, (MATRIX *) mtx);
        break;
    case 1:
        RotMatrixY(angle, (MATRIX *) mtx);
        break;
    case 2:
        RotMatrixZ(angle, (MATRIX *) mtx);
        break;
    case 3:
        RotMatrixZ(angle, (MATRIX *) mtx);
        RotMatrixY(angle, (MATRIX *) mtx);
        RotMatrixX(angle, (MATRIX *) mtx);
        break;
    }

after_track_rotation:
    if ((part->unk0 >> 17) & 1)
    {
        RotMatrixX(0x400, (MATRIX *) mtx);
    }
    if (((u8 *) part)[0x13] != 0)
    {
        RotMatrixY(((u8 *) part)[0x13] << 4, (MATRIX *) mtx);
    }

    if ((part->unk4 >> 3) & 1)
    {
        RotMatrixZ(0x400, (MATRIX *) mtx);
        RotMatrixY(0x400, (MATRIX *) mtx);
        if ((part->unk28 >> 11) & 1)
        {
            func_80073F7C(rec, part, scale);
            temp = scale->vz;
            angle = ratan2(rec->unk8 - temp, scale->vx - rec->unk0);
            delta->vx = (scale->vx - rec->unk0) >> 8;
            delta->vy = (scale->vy - rec->unk4) >> 8;
            delta->vz = (scale->vz - rec->unk8) >> 8;
            gte_ldlvl(delta);
            gte_sqr0();
            gte_stlvnl(sqr);
            dist = SquareRoot0(sqr->vx + sqr->vz + sqr->vy);
            temp = scale->vy;
            RotMatrixZ(ratan2(dist << 8, rec->unk4 - temp), (MATRIX *) mtx);
            RotMatrixY(angle, (MATRIX *) mtx);
            final_angle = 0x155;
        }
        else
        {
            RotMatrixZ(rec->unk14, (MATRIX *) mtx);
            RotMatrixY(rec->unk12, (MATRIX *) mtx);
            RotMatrixZ(rec->unk32 << 4, (MATRIX *) mtx);
            RotMatrixY(rec->unk33 << 4, (MATRIX *) mtx);
            final_angle = 0x100;
        }
    }
    else
    {
        RotMatrixY(0x400, (MATRIX *) mtx);
        final_angle = 0x400;
    }
    RotMatrixX(final_angle, (MATRIX *) mtx);

    if ((part->unk28 >> 2) & 1)
    {
        func_80073F7C(rec, part, scale);
        delta->vx = (scale->vx - rec->unk0) >> 8;
        delta->vy = (scale->vy - rec->unk4) >> 8;
        delta->vz = (scale->vz - rec->unk8) >> 8;
        gte_ldlvl(delta);
        gte_sqr0();
        gte_stlvnl(sqr);
        dist = SquareRoot0(sqr->vx + sqr->vy + sqr->vz);
        if (dist == 0)
        {
            dist = 1;
        }
        scale->vy = 0x1000;
        scale->vz = 0x1000;
        if (rec->unk3C != 0)
        {
            scale->vx = (dist << 12) / rec->unk3C;
        }
        else
        {
            scale->vx = (dist << 12) >> 6;
        }
        ScaleMatrix((MATRIX *) mtx, (VECTOR *) scale);
    }

    if ((part->unk0 >> 23) & 1)
    {
        if ((part->unk4 >> 3) & 1)
        {
            dir->vx = -0x1000;
            dir->vy = 0;
            dir->vz = 0;
            gte_SetRotMatrix(mtx);
            gte_ldv0(dir);
            gte_rtv0();
            gte_stlvnl(scale);
            dir->vx = 0;
            dir->vy = 0x400;
            dir->vz = 0x400;
            RotMatrix_gte((SVECTOR *) dir, (MATRIX *) mtx);
            RotMatrixZ(ratan2(scale->vy, scale->vx) + 0x400, (MATRIX *) mtx);
            gte_ldlvl(scale);
            gte_sqr0();
            gte_stlvnl(sqr);
            scale->vx = SquareRoot0(sqr->vx + sqr->vy);
            scale->vz = 0x1000;
            scale->vy = 0x1000;
            ScaleMatrix((MATRIX *) mtx, (VECTOR *) scale);
        }
    }

    scale->vy = 0x1000;
    if (part->unk1C & 0x02000000)
    {
        scale->vz = (part->unk2E - ((part->unk2E * ((s32) (0x100 - D_80105AE0[actor->unk228].unk68) >> 6)) / 10)) << 6;
    }
    else
    {
        scale->vz = part->unk2E << 6;
    }
    if (part->unk1C & 0x04000000)
    {
        scale->vx = (part->unk33 - ((part->unk33 * ((s32) (0x100 - D_80105AE0[actor->unk228].unk68) >> 6)) / 10)) << 6;
    }
    else
    {
        scale->vx = part->unk33 << 6;
    }
    ScaleMatrix((MATRIX *) mtx, (VECTOR *) scale);

    D_80105768.unk0 = 0x1000;
    D_80105768.unk2 = 0x1000;
    D_80105768.unk4 = 0x1000;

    if ((part->unk4 >> 7) & 1)
    {
        scale->vz = 0x1000;
        scale->vy = 0x1000;
        scale->vx = 0x1000;
        flags = part->unk4;
        mode = (flags >> 20) & 3;
        switch (mode)
        {
        case 0:
            scale->vz = field_evaluate_parameter_track_at_time(actor, flags >> 28, rec->unk2C) << 4;
            break;
        case 1:
            scale->vx = field_evaluate_parameter_track_at_time(actor, flags >> 28, rec->unk2C) << 4;
            break;
        case 2:
        {
            u32 value = field_evaluate_parameter_track_at_time(actor, flags >> 28, rec->unk2C) << 4;
            scale->vz = value;
            scale->vy = value;
            scale->vx = value;
            break;
        }
        case 3:
            scale->vz = field_evaluate_parameter_track_at_time(actor, flags >> 28, rec->unk2C) << 4;
            temp = (part->unk4 >> 28) + 1;
            scale->vx = field_evaluate_parameter_track_at_time(actor, temp % 16, rec->unk2C) << 4;
            break;
        }
        D_80105768.unk0 = (u16) scale->vx;
        D_80105768.unk2 = (u16) scale->vy;
        D_80105768.unk4 = (u16) scale->vz;
        ScaleMatrix((MATRIX *) mtx, (VECTOR *) scale);
    }
}

/**
 * @brief Resolve the RGB triple for one part into the primitive colour bytes.
 *
 * Depending on the record flags the colour is taken from the parameter tracks
 * named by the part definition, copied verbatim from the record, or broadcast
 * from a single track sample to all three channels.
 *
 * @param actor Owning actor state, used as the parameter-track source.
 * @param rec Effect record supplying the flag word (unk1C), the literal colour
 *            at unk18 and the animation time (unk2C).
 * @param part Part definition naming the per-channel parameter tracks.
 * @param out Destination for the three colour bytes (r, g, b).
 * @see decomp.me (100%)
 */
void func_8007D8D8(FieldActorState *actor, Struct_D800FDF58 *rec, FieldActorPartDef *part, u8 *out)
{
    s32 flags;
    u8 value;

    flags = rec->unk1C;
    if (!(flags & 0x8000))
    {
        if ((*(u32 *) &part->unk2C >> 5) & 4)
        {
            u32 eval_temp;
            do
            {
                eval_temp = field_evaluate_parameter_track_at_time(actor, part->unkE & 0xF, rec->unk2C);
            } while (0);
            out[0] = eval_temp;
        }
        else
        {
            out[0] = part->unkE;
        }
        if ((*(u32 *) &part->unk2C >> 5) & 2)
        {
            u32 eval_temp;
            do
            {
                eval_temp = field_evaluate_parameter_track_at_time(actor, part->unkF & 0xF, rec->unk2C);
            } while (0);
            out[1] = eval_temp;
        }
        else
        {
            out[1] = part->unkF;
        }
        if ((*(u32 *) &part->unk2C >> 5) & 1)
        {
            out[2] = field_evaluate_parameter_track_at_time(actor, part->unk10 & 0xF, rec->unk2C);
            return;
        }
        out[2] = part->unk10;
        return;
    }

    if (flags & 0x10000000)
    {
        *(u32 *) out = *(u32 *) &rec->unk18;
        return;
    }

    {
        u32 part_flags = part->unk4;
        if ((part_flags >> 12) & 1)
        {
            out[0] = field_evaluate_parameter_track_at_time(actor, (part_flags >> 16) & 0xF, rec->unk2C);
            out[1] = field_evaluate_parameter_track_at_time(actor, (((u16 *) &part->unk4)[1] & 0xF) + 1, rec->unk2C);
            out[2] = field_evaluate_parameter_track_at_time(actor, (((u16 *) &part->unk4)[1] & 0xF) + 2, rec->unk2C);
            return;
        }

        value = field_evaluate_parameter_track_at_time(actor, (part_flags >> 16) & 0xF, rec->unk2C);
        out[0] = out[1] = out[2] = value;
    }
}

/**
 * @brief Emit the trailing DR_MODE packet that restores the texture page and
 *        thread it into the depth-indexed ordering table.
 * @param rec Effect record supplying the depth key (unk8 >> 7).
 * @param part Part definition supplying the semi-transparency bits (unk4).
 * @param primbuf Output primitive buffer.
 * @param base Depth-indexed ordering-table / primitive base array.
 * @return The primbuf cursor advanced past the 8-byte packet.
 * @see decomp.me (100%)
 */
u8 *func_8007DA80(Struct_D800FDF58 *rec, FieldActorPartDef *part, u8 *primbuf, s32 *base)
{
    s32 index;
    s32 *entry;
    s32 srcval;

    primbuf[3] = 1;
    *(s32 *) (primbuf + 4) = (((part->unk4 >> 17) & 0x60) | 0xE1000005);

    index = rec->unk8 >> 7;
    if (index < 0)
    {
        *(s32 *) primbuf = (*(s32 *) primbuf & 0xFF000000) | (base[0] & 0xFFFFFF);
        base[0] = (base[0] & 0xFF000000) | ((s32) primbuf & 0xFFFFFF);
        primbuf += 8;
    }
    else if (index >= 0x1000)
    {
        *(s32 *) primbuf = (*(s32 *) primbuf & 0xFF000000) | (base[0xFFF] & 0xFFFFFF);
        base[0xFFF] = (base[0xFFF] & 0xFF000000) | ((s32) primbuf & 0xFFFFFF);
        primbuf += 8;
    }
    else
    {
        srcval = base[index];
        *(s32 *) primbuf = (*(s32 *) primbuf & 0xFF000000) | (srcval & 0xFFFFFF);
        entry = (s32 *) (((rec->unk8 >> 7) << 2) + (s32) base);
        *entry = (*entry & 0xFF000000) | ((s32) primbuf & 0xFFFFFF);
        primbuf += 8;
    }
    return primbuf;
}

/**
 * @brief Project the four corners of a billboarded sprite quad into screen
 *        space and write them into the primitive packet.
 *
 * With no tilt (item[8] == 0) each corner is rotated straight through the
 * caller matrix. Otherwise a second matrix is built from the tilt angle and
 * the corners are chained as origin plus rotated edge vectors, with the corner
 * order mirrored when the record is facing the other way.
 *
 * @param rec Effect record; only the facing flag in unk21 is read.
 * @param origin Screen-space origin (x, y) the quad is anchored to.
 * @param packet Primitive packet receiving the four vertex pairs.
 * @param width Quad width in world units.
 * @param height Quad height in world units.
 * @param x World-space x offset of the quad from the anchor.
 * @param y World-space y offset of the quad from the anchor.
 * @param item Sprite item record; item[8] holds the tilt angle (0 = untilted).
 * @param mtx Caller-supplied rotation matrix applied to every corner.
 * @see decomp.me (100%)
 */
void func_8007DB98(Struct_D800FDF58 *rec, u16 *origin, u8 *packet, s32 width, s32 height, s32 x, s32 y, u8 *item, FieldMatrix *mtx)
{
    FieldSVector *tmp = (FieldSVector *) 0x1F800100;
    FieldSVector *vec = (FieldSVector *) 0x1F800108;
    FieldVector *out = (FieldVector *) 0x1F800110;
    FieldMatrix *rot = (FieldMatrix *) 0x1F800120;
    u16 *p = (u16 *) packet;
    u16 *o = (u16 *) out;

    if (item[8] == 0)
    {
        tmp->vx = y;
        tmp->vy = 0;
        tmp->vz = x;
        gte_ldv0(tmp);
        gte_rtv0();
        gte_stlvnl(out);
        p[4] = origin[0] + o[0];
        p[5] = origin[1] + o[2];

        tmp->vx = y;
        tmp->vy = 0;
        tmp->vz = x + width;
        gte_ldv0(tmp);
        gte_rtv0();
        gte_stlvnl(out);
        p[8] = origin[0] + o[0];
        p[9] = origin[1] + o[2];

        tmp->vx = y + height;
        tmp->vy = 0;
        tmp->vz = x;
        gte_ldv0(tmp);
        gte_rtv0();
        gte_stlvnl(out);
        p[12] = origin[0] + o[0];
        p[13] = origin[1] + o[2];

        tmp->vx = y + height;
        tmp->vy = 0;
        tmp->vz = x + width;
        gte_ldv0(tmp);
        gte_rtv0();
        gte_stlvnl(out);
        p[16] = origin[0] + o[0];
        p[17] = origin[1] + o[2];
        return;
    }

    if (rec->unk21 & 0x80)
    {
        ((u32 *) rot)[4] = 0x1000;
        ((u32 *) rot)[2] = 0x1000;
        ((u32 *) rot)[0] = 0x1000;
        ((u32 *) rot)[7] = 0;
        ((u32 *) rot)[6] = 0;
        ((u32 *) rot)[5] = 0;
        ((u32 *) rot)[3] = 0;
        ((u32 *) rot)[1] = 0;

        vec->vx = y;
        vec->vy = 0;
        vec->vz = x + width;
        gte_SetRotMatrix(rot);
        gte_ldv0(vec);
        gte_rtv0();
        gte_stsv(tmp);
        gte_SetRotMatrix(mtx);
        gte_ldv0(tmp);
        gte_rtv0();
        gte_stlvnl(out);
        p[8] = origin[0] + o[0];
        p[9] = origin[1] + o[2];

        vec->vx = 0;
        vec->vy = -(item[8] << 4);
        vec->vz = 0;
        RotMatrix_gte((SVECTOR *) vec, (MATRIX *) rot);

        vec->vx = 0;
        vec->vy = 0;
        vec->vz = -width;
        gte_SetRotMatrix(rot);
        gte_ldv0(vec);
        gte_rtv0();
        gte_stsv(tmp);
        gte_SetRotMatrix(mtx);
        gte_ldv0(tmp);
        gte_rtv0();
        gte_stlvnl(out);
        p[4] = p[8] + o[0];
        p[5] = p[9] + o[2];

        vec->vx = height;
        vec->vy = 0;
        vec->vz = 0;
        gte_SetRotMatrix(rot);
        gte_ldv0(vec);
        gte_rtv0();
        gte_stsv(tmp);
        gte_SetRotMatrix(mtx);
        gte_ldv0(tmp);
        gte_rtv0();
        gte_stlvnl(out);
        p[16] = p[8] + o[0];
        p[17] = p[9] + o[2];

        vec->vx = height;
        vec->vy = 0;
        vec->vz = -width;
        gte_SetRotMatrix(rot);
        gte_ldv0(vec);
        gte_rtv0();
        gte_stsv(tmp);
        gte_SetRotMatrix(mtx);
        gte_ldv0(tmp);
        gte_rtv0();
        gte_stlvnl(out);
        p[12] = p[8] + o[0];
        p[13] = p[9] + o[2];
    }
    else
    {
        ((u32 *) rot)[4] = 0x1000;
        ((u32 *) rot)[2] = 0x1000;
        ((u32 *) rot)[0] = 0x1000;
        ((u32 *) rot)[7] = 0;
        ((u32 *) rot)[6] = 0;
        ((u32 *) rot)[5] = 0;
        ((u32 *) rot)[3] = 0;
        ((u32 *) rot)[1] = 0;

        vec->vx = y;
        vec->vy = 0;
        vec->vz = x;
        gte_SetRotMatrix(rot);
        gte_ldv0(vec);
        gte_rtv0();
        gte_stsv(tmp);
        gte_SetRotMatrix(mtx);
        gte_ldv0(tmp);
        gte_rtv0();
        gte_stlvnl(out);
        p[4] = origin[0] + o[0];
        p[5] = origin[1] + o[2];

        vec->vx = 0;
        vec->vy = item[8] << 4;
        vec->vz = 0;
        RotMatrix_gte((SVECTOR *) vec, (MATRIX *) rot);

        vec->vx = 0;
        vec->vy = 0;
        vec->vz = width;
        gte_SetRotMatrix(rot);
        gte_ldv0(vec);
        gte_rtv0();
        gte_stsv(tmp);
        gte_SetRotMatrix(mtx);
        gte_ldv0(tmp);
        gte_rtv0();
        gte_stlvnl(out);
        p[8] = p[4] + o[0];
        p[9] = p[5] + o[2];

        vec->vx = height;
        vec->vy = 0;
        vec->vz = 0;
        gte_SetRotMatrix(rot);
        gte_ldv0(vec);
        gte_rtv0();
        gte_stsv(tmp);
        gte_SetRotMatrix(mtx);
        gte_ldv0(tmp);
        gte_rtv0();
        gte_stlvnl(out);
        p[12] = p[4] + o[0];
        p[13] = p[5] + o[2];

        vec->vx = height;
        vec->vy = 0;
        vec->vz = width;
        gte_SetRotMatrix(rot);
        gte_ldv0(vec);
        gte_rtv0();
        gte_stsv(tmp);
        gte_SetRotMatrix(mtx);
        gte_ldv0(tmp);
        gte_rtv0();
        gte_stlvnl(out);
        p[16] = p[4] + o[0];
        p[17] = p[5] + o[2];
    }
}

/**
 * @brief Unpack the four signed byte-pair UV corners of a sprite item,
 *        optionally mirrored horizontally.
 * @param out Destination for the eight signed UV components.
 * @param flip Non-zero to mirror the quad horizontally.
 * @param item Sprite item record holding the packed signed byte corners.
 * @see decomp.me (100%)
 */
void func_8007E4A8(s16 *out, s32 flip, u8 *item)
{
    if (flip)
    {
        out[0] = -(s8) item[2];
        out[1] = (s8) item[3];
        out[2] = -(s8) item[0];
        out[3] = (s8) item[1];
        out[4] = -(s8) item[6];
        out[5] = (s8) item[8];
        out[6] = -(s8) item[4];
        out[7] = (s8) item[5];
    }
    else
    {
        out[0] = (s8) item[0];
        out[1] = (s8) item[1];
        out[2] = (s8) item[2];
        out[3] = (s8) item[3];
        out[4] = (s8) item[4];
        out[5] = (s8) item[5];
        out[6] = (s8) item[6];
        out[7] = (s8) item[8];
    }
}

/**
 * @brief Wide variant of func_8007E4A8: unpack the four 16-bit little-endian
 *        corners of a sprite item, optionally mirrored horizontally.
 * @param out Destination for the eight 16-bit UV components.
 * @param mirror Non-zero to mirror the quad horizontally.
 * @param item Sprite item record holding the packed 16-bit corners.
 * @see decomp.me (100%)
 */
void func_8007E5FC(s16 *out, s32 mirror, u8 *item)
{
    if (mirror != 0)
    {
        out[0] = -(item[4] + (item[5] << 8));
        out[1] = item[6] + (item[8] << 8);
        out[2] = -(item[0] + (item[1] << 8));
        out[3] = item[2] + (item[3] << 8);
        out[4] = -(item[0xD] + (item[0xE] << 8));
        out[5] = item[0xF] + (item[0x10] << 8);
        out[6] = -(item[9] + (item[0xA] << 8));
        out[7] = item[0xB] + (item[0xC] << 8);
    }
    else
    {
        out[0] = item[0] + (item[1] << 8);
        out[1] = item[2] + (item[3] << 8);
        out[2] = item[4] + (item[5] << 8);
        out[3] = item[6] + (item[8] << 8);
        out[4] = item[9] + (item[0xA] << 8);
        out[5] = item[0xB] + (item[0xC] << 8);
        out[6] = item[0xD] + (item[0xE] << 8);
        out[7] = item[0xF] + (item[0x10] << 8);
    }
}

/**
 * @brief Resolve the effective distance/extent value for one animated part.
 *
 * The base value comes from the 9-bit field split across part->unk20 and the
 * top of part->unk1C, either literally or sampled from a parameter track. It is
 * then optionally scaled by the distance to the linked actor, by a random
 * factor, and by the owning actor stat, and finally has the half-extents of one
 * or two bounding boxes added in, chosen per the category in part->unk28.
 *
 * @param actor Owning actor state; unk228 is its own record index and
 *              unk229[] the linked record indices.
 * @param part Part definition holding the value, mode and category bitfields.
 * @return The resolved value.
 * @see decomp.me (100%)
 */
s32 func_8007E754(FieldActorState *actor, FieldActorPartDef *part)
{
    FieldVector delta;
    FieldVector sqr;
    s32 speed;
    s32 part_index;
    s32 temp;
    u32 flags;
    FieldActorState *actor2;
    s32 track;

    {
        u32 init_flags = *(u32 *) &part->unk20;
        s32 init_mode = (init_flags >> 6) & 3;
        actor2 = actor;
        switch (init_mode)
        {
        case 0:
        {
            s32 hi = (u32) part->unk1C >> 29;
            speed = ((init_flags & 0x3F) << 3) | hi;
            break;
        }
        case 1:
        {
            u32 hi = part->unk1C >> 29;
            speed = field_evaluate_parameter_track(actor2, (((init_flags & 0x3F) << 3) | hi) & 0xF);
            break;
        }
        case 2:
        {
            u32 hi = part->unk1C;
            speed = field_evaluate_parameter_track_at_time(actor2, (((init_flags & 0x3F) << 3) | (hi >> 29)) & 0xF, 0);
            break;
        }
        }
    }

    if (part->unk24 & 0x01000000)
    {
        delta.vx = (D_800FDF58[actor2->unk229[g_field_track_index]].unk0 - D_800FDF58[actor2->unk228].unk0) >> 8;
        delta.vy = (D_800FDF58[actor2->unk229[g_field_track_index]].unk4 - D_800FDF58[actor2->unk228].unk4) >> 8;
        delta.vz = (D_800FDF58[actor2->unk229[g_field_track_index]].unk8 - D_800FDF58[actor2->unk228].unk8) >> 8;
        gte_ldlvl(&delta);
        gte_sqr0();
        gte_stlvnl(&sqr);
        speed = (SquareRoot0(sqr.vx + sqr.vy + sqr.vz) * speed) / 100;
    }

    if ((part->unk28 >> 15) & 1)
    {
        speed = (speed * rand()) >> 15;
    }

    if (part->unk1C & 0x01000000)
    {
        speed = ((D_80105AE0[actor2->unk228].unk174 & 0x3FF) * speed) / 100;
    }

    flags = part->unk28;
    {
        s32 category = (flags >> 18) & 0x3F;
        if (category < 0x14)
        {
            if ((flags >> 16) & 1)
            {
                if ((u32) (category - 0xA) >= 0x1C)
                {
                    temp = (D_80105AE0[actor2->unk228].unk144 - D_80105AE0[actor2->unk228].unk140) >> 1;
                }
                else
                {
                    temp = (D_80105AE0[actor2->unk229[g_field_track_index]].unk144 - D_80105AE0[actor2->unk229[g_field_track_index]].unk140) >> 1;
                }
                if (temp < 0)
                {
                    temp = -temp;
                }
                speed += temp;
            }
            if ((part->unk28 >> 17) & 1)
            {
                if ((u32) (((part->unk28 >> 18) & 0x3F) - 0xA) >= 0x1C)
                {
                    temp = (D_80105AE0[actor2->unk228].unk146 - D_80105AE0[actor2->unk228].unk142) >> 1;
                }
                else
                {
                    temp = (D_80105AE0[actor2->unk229[g_field_track_index]].unk146 - D_80105AE0[actor2->unk229[g_field_track_index]].unk142) >> 1;
                }
                if (temp < 0)
                {
                    temp = -temp;
                }
                speed += temp;
            }
        }
    }

    {
        s32 category = (part->unk28 >> 18) & 0x3F;
        if ((u32) (category - 0x2A) < 8 || (u32) (category - 0x14) < 8 || (u32) (category - 0x37) < 8)
        {
            s32 category2 = (part->unk28 >> 18) & 0x3F;
            if (category2 < 0x37)
            {
                part_index = category2 - 0x14;
                if (category2 >= 0x2A)
                {
                    part_index = category2 - 0x22;
                }
            }

            if ((((u16 *) &part->unk28)[1]) & 1)
            {
                if (((u8 *) actor2->unk0)[(part_index * 0x48) + 0xB] == 2)
                {
                    temp = (D_80105AE0[actor2->unk228].unk144 - D_80105AE0[actor2->unk228].unk140) >> 1;
                }
                else
                {
                    temp = (D_80105AE0[actor2->unk229[g_field_track_index]].unk144 - D_80105AE0[actor2->unk229[g_field_track_index]].unk140) >> 1;
                }
                if (temp < 0)
                {
                    temp = -temp;
                }
                speed += temp;
            }

            if ((part->unk28 >> 17) & 1)
            {
                if (((u8 *) actor2->unk0)[(part_index * 0x48) + 0xB] == 2)
                {
                    temp = (D_80105AE0[actor2->unk228].unk146 - D_80105AE0[actor2->unk228].unk142) >> 1;
                }
                else
                {
                    temp = (D_80105AE0[actor2->unk229[g_field_track_index]].unk146 - D_80105AE0[actor2->unk229[g_field_track_index]].unk142) >> 1;
                }
                if (temp < 0)
                {
                    temp = -temp;
                }
                speed += temp;
            }
        }
    }

    return speed;
}
