#include "common.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

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
 * @note WIP - not yet byte-matching. Residual is concentrated in two places:
 *       (1) the ptr_a/ptr_b scratchpad fill below: the target re-loads
 *       *(s32 *)0x1F800000/4/8 for the ptr_b group, this source CSEs them
 *       across the ptr_a stores. Root cause is established from the gcc
 *       2.7.2 cse.c note_mem_written rule - a varying-address store only
 *       sets writes->all when the MEM is neither MEM_IN_STRUCT_P nor a PLUS
 *       address, so the FieldVector field stores here (mem/s + PLUS) leave
 *       the plain scalar constant-address loads in the table. Writing the
 *       stores through a plain `s32 *` does force the reloads (measured: 6
 *       loads instead of 3) but then CSE shares the address constants in
 *       registers instead of folding them into each load, which the target
 *       does not do. A spelling that defeats both has not been found yet.
 *       (2) a 6-way callee-saved register rotation that follows from (1):
 *       target has s2=0xFFFFFF, s3=i, s4=segments, s5=cur, s6=angle,
 *       s7=dir; this source has s2=i, s3=segments, s4=0xFFFFFF, s5=angle,
 *       s6=dir, s7=cur.
 *       Established and measured: frame size, every sp slot, the p2 cursor
 *       bias (primbuf + 0x10; +0x18/+0x1C/+0x20/0 are equivalent, +0x4/+0xC
 *       are not), local declaration order (it drives spill-slot numbering),
 *       and the statement order in the loop preheader.
 *       The decomp-permuter cannot be used on this function: pycparser
 *       rejects the inline-asm GTE macros.
 * @see decomp.me (95.43%) WIP
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
    s32 first_d0;
    s32 temp_x;
    s32 raw_d4;

    gte_out = (FieldVector *) 0x1F800010;
    ptr_a = (FieldVector *) 0x1F800020;
    ptr_b = (FieldVector *) 0x1F800030;
    ptr_c = (FieldVector *) 0x1F800040;
    dir = (FieldSVector *) 0x1F800050;

    part = &g_field_actor_slots[rec->unk22].unk0[rec->unk23];
    state = &g_field_actor_slots[rec->unk22];

    func_8007D078(rec, part, (FieldMatrix *) 0x1F800058, state);
    gte_SetRotMatrix((FieldMatrix *) 0x1F800058);

    first_d0 = D_800F22A0 / 256;
    temp_x = rec->unk0 / 256 + 0xA0;
    raw_d4 = D_800F22A4 / 256;
    *(s16 *) (primbuf + 0x8) = (s16) (first_d0 + temp_x);
    *(s16 *) (primbuf + 0xA) = (s16) (0x70 + raw_d4 + rec->unk4 / 256 - rec->unk8 / 512 - D_800F22A8 / 512);

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

    func_80073F7C(rec, part, (void *) 0x1F800000);

    i = segments - 1;

    ptr_a->vx = (*(s32 *) 0x1F800000 + rec->unk0) >> 1;
    ptr_a->vy = *(s32 *) 0x1F800004;
    ptr_a->vz = (*(s32 *) 0x1F800008 + rec->unk8) >> 1;
    ptr_b->vx = (*(s32 *) 0x1F800000 - rec->unk0) >> 1;
    ptr_b->vy = rec->unk4 - *(s32 *) 0x1F800004;
    ptr_b->vz = (*(s32 *) 0x1F800008 - rec->unk8) >> 1;

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

            first_d0 = D_800F22A0 / 256;
            temp_x = gte_out->vx / 256 + 0xA0;
            raw_d4 = D_800F22A4 / 256;
            *(s16 *) (p2 - 0x4) = (s16) (first_d0 + temp_x);
            *(s16 *) (p2 - 0x2) = (s16) (0x70 + raw_d4 + gte_out->vy / 256 - gte_out->vz / 512 - D_800F22A8 / 512);
            *(s32 *) (p2 + 0x8) = *(s32 *) (p2 - 0x4);

            temp_v1 = (s32) rec->unk8 >> 7;
            if (temp_v1 < 0)
            {
                s32 addr;
                p2 += 0x10;
                addr = (s32) primbuf & 0xFFFFFF;
                *(s32 *) (primbuf + 0) = (*(s32 *) (primbuf + 0) & 0xFF000000) | (base[0] & 0xFFFFFF);
                primbuf += 0x10;
                base[0] = (base[0] & 0xFF000000) | addr;
            }
            else if (temp_v1 >= 0x1000)
            {
                s32 addr;
                p2 += 0x10;
                addr = (s32) primbuf & 0xFFFFFF;
                *(s32 *) (primbuf + 0) = (*(s32 *) (primbuf + 0) & 0xFF000000) | (base[0xFFF] & 0xFFFFFF);
                primbuf += 0x10;
                base[0xFFF] = (base[0xFFF] & 0xFF000000) | addr;
            }
            else
            {
                s32 addr;
                s32 *entry;
                p2 += 0x10;
                addr = (s32) primbuf & 0xFFFFFF;
                *(s32 *) (primbuf + 0) = (*(s32 *) (primbuf + 0) & 0xFF000000) | (base[temp_v1] & 0xFFFFFF);
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

    first_d0 = D_800F22A0 / 256;
    temp_x = *(s32 *) 0x1F800000 / 256 + 0xA0;
    raw_d4 = D_800F22A4 / 256;
    *(s16 *) (primbuf + 0xC) = (s16) (first_d0 + temp_x);
    *(s16 *) (primbuf + 0xE) = (s16) (0x70 + raw_d4 + *(s32 *) 0x1F800004 / 256 - *(s32 *) 0x1F800008 / 512 - D_800F22A8 / 512);

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

    primbuf = func_8007DA80(rec, part, primbuf, base);

    return primbuf;
}
