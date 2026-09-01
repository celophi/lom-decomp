#include "common.h"
#include "field_types.h"
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
extern Struct_D800FDF58 D_800FF658[256];
extern FieldActorState g_field_actor_slots[80];

/**
 * @brief Field two-point trail/streak primitive builder: when the record has
 *        a linked previous record (rec->unk3D), projects both this record's
 *        and the linked record's world position to screen space (near/far
 *        pairs offset by rec->unk44/unk48), fills the primitive at primbuf,
 *        and blends its color from a depth-indexed palette in base[].
 * @param rec Effect record supplying the position/link/flag fields.
 * @param primbuf Output primitive buffer; advanced by one primitive (0x18
 *                bytes) when the record has an active link.
 * @param base Depth-indexed color/ordering-table array.
 * @return The advanced primbuf cursor (unchanged when rec has no active
 *         link).
 * @see decomp.me (100%)
 */
u8 *func_8007AE2C(Struct_D800FDF58 *rec, u8 *primbuf, s32 *base)
{
    FieldActorState *state;
    FieldActorPartDef *part;
    s32 temp_v1_2;
    s32 first_d0;
    s32 temp_x;
    s32 raw_d4;

    part = &g_field_actor_slots[rec->unk22].unk0[rec->unk23];
    state = &g_field_actor_slots[rec->unk22];

    if (rec->unk3D != 0xFF && D_800FF658[rec->unk3D].unk25 != 0xFF)
    {
        first_d0 = D_800F22A0 / 256;
        temp_x = rec->unk0 / 256 + 0xA0;
        raw_d4 = D_800F22A4;
        *(u16 *) (primbuf + 0x8) = (u16) (first_d0 + temp_x);

        *(u16 *) (primbuf + 0xA) = (u16) (0x70 + raw_d4 / 256 + rec->unk4 / 256 - rec->unk8 / 512 - D_800F22A8 / 512);

        *(s32 *) (primbuf + 0x10) = *(s32 *) (primbuf + 0x8);
        *(u16 *) (primbuf + 0x8) = (u16) (*(u16 *) (primbuf + 0x8) - (u16) rec->unk44);
        *(u16 *) (primbuf + 0xA) = (u16) (*(u16 *) (primbuf + 0xA) - (u16) rec->unk48);
        *(u16 *) (primbuf + 0x10) = (u16) (*(u16 *) (primbuf + 0x10) + (u16) rec->unk44);
        *(u16 *) (primbuf + 0x12) = (u16) (*(u16 *) (primbuf + 0x12) + (u16) rec->unk48);

        *(u16 *) (primbuf + 0xC) = (u16) (D_800F22A0 / 256 + (D_800FF658[rec->unk3D].unk0 / 256 + 0xA0));

        *(u16 *) (primbuf + 0xE) = (u16) (0x70 + raw_d4 / 256 + D_800FF658[rec->unk3D].unk4 / 256 - D_800FF658[rec->unk3D].unk8 / 512 - D_800F22A8 / 512);

        *(s32 *) (primbuf + 0x14) = *(s32 *) (primbuf + 0xC);
        *(u16 *) (primbuf + 0xC) = (u16) (*(u16 *) (primbuf + 0xC) - (u16) D_800FF658[rec->unk3D].unk44);
        *(u16 *) (primbuf + 0xE) = (u16) (*(u16 *) (primbuf + 0xE) - (u16) D_800FF658[rec->unk3D].unk48);
        *(u16 *) (primbuf + 0x14) = (u16) (*(u16 *) (primbuf + 0x14) + (u16) D_800FF658[rec->unk3D].unk44);
        *(u16 *) (primbuf + 0x16) = (u16) (*(u16 *) (primbuf + 0x16) + (u16) D_800FF658[rec->unk3D].unk48);

        func_8007D8D8(state, rec, part, primbuf + 4);

        setPolyF4((POLY_F4 *) primbuf);
        setSemiTrans((POLY_F4 *) primbuf, rec->unk1C & 0x800000);

        temp_v1_2 = (s32) rec->unk8 >> 7;
        if (temp_v1_2 < 0)
        {
            addPrim(&base[0], (POLY_F4 *) primbuf);
            primbuf += sizeof(POLY_F4);
        }
        else if (temp_v1_2 >= 0x1000)
        {
            addPrim(&base[0xFFF], (POLY_F4 *) primbuf);
            primbuf += sizeof(POLY_F4);
        }
        else
        {
            addPrim(&base[(s32) rec->unk8 >> 7], (POLY_F4 *) primbuf);
            primbuf += sizeof(POLY_F4);
        }

        primbuf = func_8007DA80(rec, part, primbuf, base);
    }

    return primbuf;
}

#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

/**
 * @brief Field radial fan primitive builder: transforms a ring of
 *        rec->unk24 (or 1, if unset) directions through the actor's
 *        rotation matrix via the GTE and emits one triangle primitive per
 *        segment connecting a shared hub vertex to each rim point,
 *        threading each into the depth-indexed ordering table in base[].
 * @param rec Effect record supplying the position/rotation/segment-count
 *            (unk24) fields.
 * @param primbuf Output primitive buffer; advanced by one primitive (0x10
 *                bytes) per fan segment.
 * @param base Depth-indexed ordering-table / primitive base array.
 * @return The advanced primbuf cursor.
 * @see decomp.me (100%)
 */
u8 *func_8007B29C(Struct_D800FDF58 *rec, u8 *primbuf, s32 *base)
{
    FieldMatrix *mtx;
    FieldSVector *dir;
    FieldActorPartDef *part;
    FieldVector *gte_out;
    s32 radius;
    Vec2s *base_screen;
    FieldActorState *state;
    u8 *var_s1;
    s32 *temp_v0;
    s32 angle;
    s32 segments;
    s32 hub_x;
    s32 i;
    s32 var_v0;
    s32 var_v1;
    s32 var_a0;
    s32 var_t0;
    s32 temp_v1;
    s8 var_v0_6;
    s8 var_v0_7;

    gte_out = (FieldVector *) 0x1F800010;
    base_screen = (Vec2s *) 0x1F800020;
    dir = (FieldSVector *) 0x1F800050;
    mtx = (FieldMatrix *) 0x1F800058;

    part = &g_field_actor_slots[rec->unk22].unk0[rec->unk23];
    state = &g_field_actor_slots[rec->unk22];

    base_screen->x = (s16) (0xA0 + D_800F22A0 / 0x100 + rec->unk0 / 0x100);
    base_screen->y = (s16) (0x70 + D_800F22A4 / 0x100 + rec->unk4 / 0x100 - rec->unk8 / 0x200 - D_800F22A8 / 0x200);

    func_8007D078(rec, part, mtx, state);
    gte_SetRotMatrix(mtx);

    var_v0 = 0xA0 + D_800F22A0 / 0x100 + rec->unk0 / 0x100;
    var_v1 = D_800F22A4;
    *(s16 *) (primbuf + 0x8) = (s16) var_v0;
    if (var_v1 < 0)
    {
        var_v1 += 0xFF;
    }
    *(s16 *) (primbuf + 0xA) = (s16) (0x70 + (var_v1 >> 8) + rec->unk4 / 0x100 - rec->unk8 / 0x200 - D_800F22A8 / 0x200);

    func_8007D8D8(state, rec, part, primbuf + 4);

    segments = 1;
    if (rec->unk24 != 0)
    {
        segments = rec->unk24;
    }

    radius = (u32) ((part->unk23 + 1) * 5) >> 4;

    dir->unk0 = (s16) ((u32) (rsin(0) * 5) >> 8);
    dir->unk2 = 0;
    dir->unk4 = (s16) ((s32) (rcos(0) * 0x50) >> 0xC);

    gte_ldv0(dir);
    gte_rtv0();
    gte_stlvnl(gte_out);

    *(s16 *) (primbuf + 0x8) = (s16) (base_screen->x + *(s16 *) &gte_out->vx);
    *(s16 *) (primbuf + 0xA) = (s16) (base_screen->y + *(s16 *) &gte_out->vy);
    hub_x = *(s32 *) (primbuf + 0x8);

    i = 1;
    if (i < segments)
    {
        do
        {
        var_s1 = primbuf + 0x14;
        do { angle = i << 12; } while (0);
        if (i & 1)
        {
            angle /= segments;
            dir->unk0 = (s16) ((rsin(angle) * radius) >> 0xC);
            dir->unk2 = 0;
            dir->unk4 = (s16) ((rcos(angle) * radius) >> 0xC);
        }
        else
        {
            angle /= segments;
            dir->unk0 = (s16) ((u32) (rsin(angle) * 5) >> 8);
            dir->unk2 = 0;
            dir->unk4 = (s16) ((s32) (rcos(angle) * 0x50) >> 0xC);
        }

            *(s8 *) (var_s1 - 0x11) = 3;
            *(s8 *) (var_s1 - 0xD) = 0x40;
            ((rec->unk1C & 0x800000) ?
             (*(u8 *)(var_s1 - 0xD) = *(u8 *)(var_s1 - 0xD) | 2) :
             (*(u8 *)(var_s1 - 0xD) = *(u8 *)(var_s1 - 0xD) & ~2));

            gte_ldv0(dir);
            gte_rtv0();
            gte_stlvnl(gte_out);

            *(s16 *) (var_s1 - 0x8) = (s16) (base_screen->x + *(s16 *) &gte_out->vx);
            *(s16 *) (var_s1 - 0x6) = (s16) (base_screen->y + *(s16 *) &gte_out->vy);
            *(s32 *) (primbuf + 0x18) = *(s32 *) (primbuf + 0x0C);
            *(s32 *) (primbuf + 0x14) = *(s32 *) (primbuf + 0x04);

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
                i++;
        } while (i < segments);
    }

    *(s8 *) (primbuf + 3) = 3;
    *(s8 *) (primbuf + 7) = 0x40;
    ((rec->unk1C & 0x800000) ? (*(u8 *)(primbuf + 7) = *(u8 *)(primbuf + 7) | 2) : (*(u8 *)(primbuf + 7) = *(u8 *)(primbuf + 7) & ~2));
    *(s32 *) (primbuf + 0xC) = hub_x;

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
        s32 addr; s32 *entry; s32 srcval;
        addr = (s32) primbuf & 0xFFFFFF;
        srcval = base[temp_v1];
        *(s32 *) primbuf = (*(s32 *) primbuf & 0xFF000000) | (srcval & 0xFFFFFF);
        entry = (s32 *) ((((s32) rec->unk8 >> 7) << 2) + (s32) base);
        primbuf += 0x10;
        *entry = (*entry & 0xFF000000) | addr;
    }

    primbuf = func_8007DA80(rec, part, primbuf, base);

    return primbuf;
}
