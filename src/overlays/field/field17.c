#include "common.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"

typedef struct
{
    s16 x;
    s16 y;
} Vec2s;

typedef struct
{
    s32 x;
    s32 y;
    s32 z;
} Vec3i;

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

/** @brief Effect/actor position record; array element stride 0x54. */
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

/** @brief Per-actor animation/geometry slot; array element stride 0x23C. */
typedef struct
{
    u8 pad0[0x130];
    Vec2s unk130[4]; /* 0x130 */
    s16 unk140;      /* 0x140 */
    s16 unk142;      /* 0x142 */
    s16 unk144;      /* 0x144 */
    s16 unk146;      /* 0x146 */
    Vec2s unk148[12]; /* 0x148 */
    u8 pad178[0x190 - 0x178];
    Vec2s unk190[3]; /* 0x190 */
    u8 pad19C[0x23C - 0x19C];
} Struct_D80105AE0;

/** @brief Actor part definition record. */
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
    u8 unk2C;  /* 0x2C */
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

/** @brief Actor animation table entry referenced through FieldActorState. */
typedef struct
{
    u8 pad0[0x10];
    u16 unk10; /* 0x10 */
    u16 unk12; /* 0x12 */
    u16 unk14; /* 0x14 */
} FieldActorAnimationDef;

/** @brief Field actor runtime state. */
typedef struct
{
    void *unk0;                   /* 0x000 */
    u8 pad4[0xC - 0x4];
    FieldActorAnimationDef *unkC; /* 0x00C */
    u8 pad10[0x1EC - 0x10];
    u16 unk1EC[9];                /* 0x1EC */
    u8 pad1FE[0x228 - 0x1FE];
    u8 owner_object_index;        /* 0x228 */
    u8 unk229[9];                 /* 0x229 */
    u8 pad232;                    /* 0x232 */
    u8 unk233;                    /* 0x233 */
    u8 pad234[0x244 - 0x234];
} FieldActorState;

extern Struct_D800FDF58 D_800FDF58[];
extern Struct_D800FDF58 D_800FF658[];
extern Struct_D80105AE0 D_80105AE0[];
extern s32 g_field_track_index;
extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;
extern u8 D_80104B58[];
extern u8 D_80105358[];

void func_8007E5FC(s16 *out, s32 mirror, u8 *item);
void func_800801F8(u8 *buf, s32 count, s32 flag);

/**
 * @brief Resolve the world-space anchor point for an actor part.
 * @param actor Owning actor state.
 * @param part Actor part definition selecting the anchor mode.
 * @param out Receives the resolved x/y/z anchor.
 * @param arg3 Secondary index used by the slot-relative anchor modes.
 * @see decomp.me (100.00%)
 */
void func_8007ECEC(FieldActorState *actor, FieldActorPartDef *part, Vec3i *out, s32 arg3)
{
    s32 kind;
    s32 n;
    Struct_D800FDF58 *rec;
    Struct_D80105AE0 *slot;
    s32 xoff;
    s32 yoff;
    s32 sub;
    s32 i;
    Struct_D800FDF58 *scan;

    kind = part->unk28 >> 18;
    kind &= 0x3F;
    switch (kind)
    {
    case 0x00: case 0x01: case 0x02: case 0x03: case 0x04:
    case 0x05: case 0x06: case 0x07: case 0x08: case 0x09:
    case 0x0A: case 0x0B: case 0x0C: case 0x0D: case 0x0E:
    case 0x0F: case 0x10: case 0x11: case 0x12: case 0x13:
    {
        if (kind >= 0xA)
        {
            n = actor->unk229[g_field_track_index];
            kind -= 0xA;
            rec = &D_800FDF58[n];
            slot = &D_80105AE0[n];
        }
        else
        {
            n = actor->owner_object_index;
            rec = &D_800FDF58[n];
            slot = &D_80105AE0[n];
        }
        if ((part->unk28 >> 9) & 1)
        {
            part->unk2E = (((u8 *)slot)[0x144] - ((u8 *)slot)[0x140]) * 2;
        }
        if ((part->unk28 >> 1) & 1)
        {
            i = 0;
            part->unk33 = (((u8 *)slot)[0x146] - ((u8 *)slot)[0x142]) * 2;
        }
        else
        {
            i = 0;
        }
        sub = i;
        switch (kind)
        {
        case 1:
            sub = (slot->unk144 + slot->unk140) >> 1;
            i = (slot->unk146 + slot->unk142) >> 1;
            break;
        case 2:
            sub = (slot->unk144 + slot->unk140) >> 1;
            i = 0;
            break;
        case 3:
            sub = (slot->unk144 + slot->unk140) >> 1;
            i = slot->unk142;
            break;
        case 4:
            sub = slot->unk140;
            i = (slot->unk146 + slot->unk142) >> 1;
            break;
        case 5:
            sub = slot->unk144;
            i = (slot->unk146 + slot->unk142) >> 1;
            break;
        case 6:
            sub = slot->unk140;
            i = slot->unk142;
            break;
        case 7:
            sub = slot->unk144;
            i = slot->unk142;
            break;
        case 8:
            sub = slot->unk140;
            i = slot->unk146;
            break;
        case 9:
            sub = slot->unk144;
            i = slot->unk146;
            break;
        }
        sub <<= 8;
        out->x = rec->unk0 + sub;
        i <<= 8;
        out->y = rec->unk4 + i;
        out->z = rec->unk8;
        return;
    }

    case 0x14: case 0x15: case 0x16: case 0x17:
    case 0x18: case 0x19: case 0x1A: case 0x1B:
    case 0x2A: case 0x2B: case 0x2C: case 0x2D:
    case 0x2E: case 0x2F: case 0x30: case 0x31:
        sub = kind - 0x14;
        if (kind >= 0x2A)
        {
            sub = kind - 0x22;
        }
        i = 0;
        do
        {
            if (D_800FF658[i].unk25 != 0xFF && D_800FF658[i].unk23 == sub && D_800FF658[i].unk22 == actor->unk233 && D_800FF658[i].unk29 == 0)
            {
                out->x = D_800FF658[i].unk0;
                out->y = D_800FF658[i].unk4;
                out->z = D_800FF658[i].unk8;
                return;
            }
            i++;
        } while (i < 0x100);
        return;

    case 0x25:
        out->x = (part->unk38 << 8) - D_800F22A0;
        out->y = (part->unk3A << 8) - D_800F22A4;
        out->z = (part->unk3C << 8) - D_800F22A8;
        return;
    case 0x1C:
        out->x = -D_800F22A0;
        out->y = -D_800F22A4;
        out->z = -D_800F22A8;
        return;
    case 0x1D:
        out->y = -0x7000;
        out->x = -D_800F22A0;
        out->y -= D_800F22A4;
        out->z = -D_800F22A8;
        return;
    case 0x1E:
        out->y = 0x7000;
        out->x = -D_800F22A0;
        out->y -= D_800F22A4;
        out->z = -D_800F22A8;
        return;
    case 0x1F:
        out->x = 0xFFFF6000;
        out->x -= D_800F22A0;
        out->y = -D_800F22A4;
        out->z = -D_800F22A8;
        return;
    case 0x20:
        out->x = 0xA000;
        out->x -= D_800F22A0;
        out->y = -D_800F22A4;
        out->z = -D_800F22A8;
        return;
    case 0x21:
        out->x = 0xFFFF6000;
        out->y = -0x7000;
        out->x -= D_800F22A0;
        out->y -= D_800F22A4;
        out->z = -D_800F22A8;
        return;
    case 0x22:
        out->x = 0xA000;
        out->y = -0x7000;
        out->x -= D_800F22A0;
        out->y -= D_800F22A4;
        out->z = -D_800F22A8;
        return;
    case 0x23:
        out->x = 0xFFFF6000;
        out->y = 0x7000;
        out->x -= D_800F22A0;
        out->y -= D_800F22A4;
        out->z = -D_800F22A8;
        return;
    case 0x24:
        out->x = 0xA000;
        out->y = 0x7000;
        out->x -= D_800F22A0;
        out->y -= D_800F22A4;
        out->z = -D_800F22A8;
        return;
    case 0x26:
        return;

    case 0x27:
        rec = &D_800FDF58[actor->owner_object_index];
        if ((((part->unk28 >> 10) & 1) || (part->unk34 & 0x08000000)) && !(rec->unk21 & 0x80))
        {
            out->x = rec->unk0 - (part->unk38 << 8);
        }
        else
        {
            out->x = rec->unk0 + (part->unk38 << 8);
        }
        out->y = rec->unk4 + (part->unk3A << 8);
        out->z = rec->unk8 + (part->unk3C << 8);
        return;

    case 0x28:
        rec = &D_800FDF58[actor->unk229[g_field_track_index]];
        if ((part->unk34 & 0x08000000) && !(D_800FDF58[actor->owner_object_index].unk21 & 0x80))
        {
            out->x = rec->unk0 - (part->unk38 << 8);
        }
        else if (((part->unk28 >> 10) & 1) && !(rec->unk21 & 0x80))
        {
            out->x = rec->unk0 - (part->unk38 << 8);
        }
        else
        {
            out->x = rec->unk0 + (part->unk38 << 8);
        }
        out->y = rec->unk4 + (part->unk3A << 8);
        out->z = rec->unk8 + (part->unk3C << 8);
        return;

    case 0x29:
        n = actor->owner_object_index;
        rec = &D_800FDF58[n];
        slot = &D_80105AE0[n];
        out->x = rec->unk0 + (slot->unk130[(part->unk24 >> 21) & 3].x << 8);
        out->y = rec->unk4 + (slot->unk130[(part->unk24 >> 21) & 3].y << 8);
        out->z = rec->unk8;
        return;

    case 0x32:
        n = actor->owner_object_index;
        rec = &D_800FDF58[n];
        slot = &D_80105AE0[n];
        out->x = rec->unk0 + (slot->unk130[(part->unk24 >> 21) & 3].x << 8);
        out->y = rec->unk4;
        out->z = rec->unk8 + (part->unk3C << 8);
        if (((part->unk28 >> 10) & 1) && !(rec->unk21 & 0x80))
        {
            out->x -= part->unk38 << 8;
        }
        else
        {
            out->x += part->unk38 << 8;
        }
        return;

    case 0x33:
        n = actor->owner_object_index;
        rec = &D_800FDF58[n];
        slot = &D_80105AE0[n];
        out->x = rec->unk0 + (slot->unk190[arg3].x << 8);
        out->y = rec->unk4;
        out->z = rec->unk8 + (slot->unk190[arg3].y << 8);
        return;

    case 0x36:
        out->x = part->unk38 << 8;
        out->y = part->unk3A << 8;
        out->z = part->unk3C << 8;
        return;

    case 0x37: case 0x38: case 0x39: case 0x3A:
    case 0x3B: case 0x3C: case 0x3D: case 0x3E:
        sub = kind - 0x37;
        i = 0;
        do
        {
            if (D_800FF658[i].unk25 != 0xFF && D_800FF658[i].unk23 == sub && D_800FF658[i].unk22 == actor->unk233 && D_800FF658[i].unk29 == 0)
            {
                out->x = D_800FF658[i].unk0;
                out->y = D_800FF658[i].unk4;
                out->z = D_800FF658[i].unk8;
            }
            i++;
        } while (i < 0x100);
        return;

    default:
        out->x = 0;
        out->y = 0;
        out->z = 0;
        return;
    }
}

/**
 * @brief Decode a 9-byte-per-entry mirrored offset table into an 8-slot output.
 * @param rec Effect record whose unk21 mirror flag selects the layout.
 * @param data Encoded entry list; first byte is the entry count.
 * @param out 8 halfword output slots.
 * @see decomp.me (100.00%)
 */
void func_8007F5D8(Struct_D800FDF58 *rec, u8 *data, s16 *out)
{
    s32 count;

    out[0] = out[1] = 0;
    out[2] = out[3] = 0;
    out[4] = out[5] = 0;
    out[6] = out[7] = 0;

    count = *data++;
    if (count != 0)
    {
        do
        {
            if (data[7] & 0x20)
            {
                if ((data[7] & 0xF) == 2)
                {
                    if (rec->unk21 & 0x80)
                    {
                        out[0] = -(s8)data[2];
                        out[1] = (s8)data[3];
                        out[2] = -(s8)data[0];
                        out[3] = (s8)data[1];
                        out[4] = -(s8)data[6];
                        out[5] = (s8)data[8];
                        out[6] = -(s8)data[4];
                        out[7] = (s8)data[5];
                    }
                    else
                    {
                        out[0] = (s8)data[0];
                        out[1] = (s8)data[1];
                        out[2] = (s8)data[2];
                        out[3] = (s8)data[3];
                        out[4] = (s8)data[4];
                        out[5] = (s8)data[5];
                        out[6] = (s8)data[6];
                        out[7] = (s8)data[8];
                    }
                }
            }
            data += 9;
            count--;
        } while (count != 0);
    }
}

/**
 * @brief Variable-stride variant that delegates matching entries to func_8007E5FC.
 * @param rec Effect record whose unk21 mirror flag is passed through.
 * @param data Encoded entry list; first byte is the entry count.
 * @param out 8 halfword output slots.
 * @see decomp.me (100.00%)
 */
void func_8007F7A0(Struct_D800FDF58 *rec, u8 *data, s16 *out)
{
    s32 count;

    out[0] = out[1] = 0;
    out[2] = out[3] = 0;
    out[4] = out[5] = 0;
    out[6] = out[7] = 0;

    count = *data++;
    if (count != 0)
    {
        do
        {
            if (data[7] & 0x20)
            {
                if ((data[7] & 0xF) == 2)
                {
                    func_8007E5FC(out, rec->unk21 & 0x80, data);
                }
                data += 0x11;
            }
            else
            {
                data += 0xB;
            }
            count--;
        } while (count != 0);
    }
}

#include "psyq/inline_c.h"
#include "psyq/gte_dmpsx_compat.h"

/**
 * @brief Apply the actor rotation to a bounding-box centre and accumulate it
 *        into the screen-space position.
 * @param rec Effect record whose unk21 mirror flag negates the depth offset.
 * @param sxy Screen-space position accumulator.
 * @param bounds Bounding-box extents (indices 0/1/2/5 used in mode 1).
 * @param base Depth offset used in the default mode.
 * @param mode 0 skips, 1 derives the offset from bounds, other uses base.
 * @see decomp.me (100.00%)
 */
void func_8007F864(Struct_D800FDF58 *rec, DVECTOR *sxy, s16 *bounds, s32 base, s32 mode)
{
    SVECTOR v;
    VECTOR out;
    s32 xoff;
    s32 zoff = base;

    if (mode != 0)
    {
        if (mode == 1)
        {
            zoff = (bounds[2] + bounds[0]) / 2;
            xoff = (bounds[5] + bounds[1]) / 2;
        }

        v.vx = -xoff;
        v.vy = 0;
        v.vz = -zoff;
        gte_ldv0(&v);
        gte_rtv0();
        gte_stlvnl(&out);

        if (rec->unk21 & 0x80)
        {
            zoff = -zoff;
        }
        sxy->vx += (s16)out.vx + zoff;
        sxy->vy += (s16)out.vy + xoff;
    }
}

/**
 * @brief Rotate four single-byte direction vectors and store the transformed
 *        screen positions into the actor slot's vertex ring.
 * @param rec Effect record whose unk21 mirror flag negates the depth component.
 * @param slot Actor slot receiving the transformed vertices.
 * @param item Encoded direction bytes (pairs consumed three at a time).
 * @param n Base vertex index into slot->unk148.
 * @param sxy Screen-space origin added to every transformed vertex.
 * @param dir Scratch direction vector fed to the GTE.
 * @param gte_out Scratch GTE output vector.
 * @see decomp.me (100.00%)
 */
void func_8007F938(Struct_D800FDF58 *rec, Struct_D80105AE0 *slot, u8 *item, s32 n, Vec2s *sxy, FieldSVector *dir, FieldVector *gte_out)
{
    s16 v;

    if (rec->unk21 & 0x80)
    {
        dir->unk0 = (s8)item[1];
        dir->unk2 = 0;
        v = -(s8)item[0];
    }
    else
    {
        dir->unk0 = (s8)item[1];
        dir->unk2 = 0;
        v = (s8)item[0];
    }
    dir->unk4 = v;
    gte_ldv0(dir);
    gte_rtv0();
    gte_stlvnl(gte_out);
    if (n == 0)
    {
        slot->unk140 = (u16)gte_out->vx;
        slot->unk142 = (u16)gte_out->vy - (s8)rec->unk37;
    }
    slot->unk148[n].x = (u16)gte_out->vx + (u16)sxy->x;
    slot->unk148[n].y = (u16)gte_out->vy + (u16)sxy->y;

    if (rec->unk21 & 0x80)
    {
        dir->unk0 = (s8)item[3];
        dir->unk2 = 0;
        v = -(s8)item[2];
    }
    else
    {
        dir->unk0 = (s8)item[3];
        dir->unk2 = 0;
        v = (s8)item[2];
    }
    dir->unk4 = v;
    gte_ldv0(dir);
    gte_rtv0();
    gte_stlvnl(gte_out);
    slot->unk148[n + 1].x = (u16)gte_out->vx + (u16)sxy->x;
    slot->unk148[n + 1].y = (u16)gte_out->vy + (u16)sxy->y;

    if (rec->unk21 & 0x80)
    {
        dir->unk0 = (s8)item[5];
        dir->unk2 = 0;
        v = -(s8)item[4];
    }
    else
    {
        dir->unk0 = (s8)item[5];
        dir->unk2 = 0;
        v = (s8)item[4];
    }
    dir->unk4 = v;
    gte_ldv0(dir);
    gte_rtv0();
    gte_stlvnl(gte_out);
    if (n == 0)
    {
        slot->unk144 = (u16)gte_out->vx;
        slot->unk146 = (u16)gte_out->vy - (s8)rec->unk37;
    }
    slot->unk148[n + 2].x = (u16)gte_out->vx + (u16)sxy->x;
    slot->unk148[n + 2].y = (u16)gte_out->vy + (u16)sxy->y;

    if (rec->unk21 & 0x80)
    {
        dir->unk0 = (s8)item[8];
        dir->unk2 = 0;
        v = -(s8)item[6];
    }
    else
    {
        dir->unk0 = (s8)item[8];
        dir->unk2 = 0;
        v = (s8)item[6];
    }
    dir->unk4 = v;
    gte_ldv0(dir);
    gte_rtv0();
    gte_stlvnl(gte_out);
    slot->unk148[n + 3].x = (u16)gte_out->vx + (u16)sxy->x;
    slot->unk148[n + 3].y = (u16)gte_out->vy + (u16)sxy->y;
}

/**
 * @brief Rotate four little-endian halfword direction vectors and store the
 *        transformed screen positions into the actor slot's vertex ring.
 * @param rec Effect record whose unk21 mirror flag negates the depth component.
 * @param slot Actor slot receiving the transformed vertices.
 * @param item Encoded direction halfwords.
 * @param n Base vertex index into slot->unk148.
 * @param sxy Screen-space origin added to every transformed vertex.
 * @param dir Scratch direction vector fed to the GTE.
 * @param gte_out Scratch GTE output vector.
 * @see decomp.me (100.00%)
 */
void func_8007FC74(Struct_D800FDF58 *rec, Struct_D80105AE0 *slot, u8 *item, s32 n, Vec2s *sxy, FieldSVector *dir, FieldVector *gte_out)
{
    if (rec->unk21 & 0x80)
    {
        dir->unk0 = item[2] + (item[3] << 8);
        dir->unk2 = 0;
        dir->unk4 = -(item[0] + (item[1] << 8));
    }
    else
    {
        dir->unk0 = item[2] + (item[3] << 8);
        dir->unk2 = 0;
        dir->unk4 = item[0] + (item[1] << 8);
    }
    gte_ldv0(dir);
    gte_rtv0();
    gte_stlvnl(gte_out);
    if (n == 0)
    {
        slot->unk140 = (u16)gte_out->vx;
        slot->unk142 = (u16)gte_out->vy;
    }
    slot->unk148[n].x = (u16)gte_out->vx + (u16)sxy->x;
    slot->unk148[n].y = (u16)gte_out->vy + (u16)sxy->y;

    if (rec->unk21 & 0x80)
    {
        dir->unk0 = item[6] + (item[8] << 8);
        dir->unk2 = 0;
        dir->unk4 = -(item[4] + (item[5] << 8));
    }
    else
    {
        dir->unk0 = item[6] + (item[8] << 8);
        dir->unk2 = 0;
        dir->unk4 = item[4] + (item[5] << 8);
    }
    gte_ldv0(dir);
    gte_rtv0();
    gte_stlvnl(gte_out);
    slot->unk148[n + 1].x = (u16)gte_out->vx + (u16)sxy->x;
    slot->unk148[n + 1].y = (u16)gte_out->vy + (u16)sxy->y;

    if (rec->unk21 & 0x80)
    {
        dir->unk0 = item[0xB] + (item[0xC] << 8);
        dir->unk2 = 0;
        dir->unk4 = -(item[9] + (item[0xA] << 8));
    }
    else
    {
        dir->unk0 = item[0xB] + (item[0xC] << 8);
        dir->unk2 = 0;
        dir->unk4 = item[9] + (item[0xA] << 8);
    }
    gte_ldv0(dir);
    gte_rtv0();
    gte_stlvnl(gte_out);
    if (n == 0)
    {
        slot->unk144 = (u16)gte_out->vx;
        slot->unk146 = (u16)gte_out->vy;
    }
    slot->unk148[n + 2].x = (u16)gte_out->vx + (u16)sxy->x;
    slot->unk148[n + 2].y = (u16)gte_out->vy + (u16)sxy->y;

    if (rec->unk21 & 0x80)
    {
        dir->unk0 = item[0xF] + (item[0x10] << 8);
        dir->unk2 = 0;
        dir->unk4 = -(item[0xD] + (item[0xE] << 8));
    }
    else
    {
        dir->unk0 = item[0xF] + (item[0x10] << 8);
        dir->unk2 = 0;
        dir->unk4 = item[0xD] + (item[0xE] << 8);
    }
    gte_ldv0(dir);
    gte_rtv0();
    gte_stlvnl(gte_out);
    slot->unk148[n + 3].x = (u16)gte_out->vx + (u16)sxy->x;
    slot->unk148[n + 3].y = (u16)gte_out->vy + (u16)sxy->y;
}

/**
 * @brief Upload an actor's animation strip to VRAM, dispatching on the anim
 *        mode nibble through a computed-goto jump table.
 * @param actor Owning actor state selecting the animation definition and slot.
 * @return TODO: return value unused by callers.
 * @see decomp.me (100.00%)
 */
s32 func_8007FFC8(FieldActorState *actor)
{
    RECT rect;
    u8 *buf;
    u16 anim;
    u32 idx;
    static void *const jt[6] = {&&done, &&done, &&case2, &&case3, &&done, &&done};

    anim = actor->unkC->unk10;
    if (!(anim & 0xF))
    {
        goto done;
    }
    if (((u32)actor->unk1EC[0] % (u32)((u8)actor->unkC->unk10 & 0xF)) != 0)
    {
        goto done;
    }
    idx = (anim >> 4) & 7;
    if (idx >= 6)
    {
        goto done;
    }
    idx <<= 2;
    idx += (u32)jt;
    goto *(*(void **)idx);

case2:
    if (actor->owner_object_index < 2)
    {
        s32 owner_off = actor->owner_object_index;
        FieldActorAnimationDef *anim_def = actor->unkC;
        owner_off <<= 10;
        {
            u16 a2 = anim_def->unk10;
            u8 *base = &D_80104B58[(a2 >> 3) & 0x1E0];
            buf = (u8 *)(owner_off + (s32)base);
            func_800801F8(buf + 2, 0xF, (a2 >> 7) & 1);
        }
        rect.x = (actor->unkC->unk10 >> 4) & 0xF0;
        rect.y = (actor->owner_object_index * 2) + 0x1EE;
        rect.w = 0x10;
        rect.h = 1;
    }
    else
    {
        u16 a2 = actor->unkC->unk10;
        buf = &D_80105358[(a2 >> 3) & 0x1E0];
        func_800801F8(buf + 2, 0xF, (a2 >> 7) & 1);
        rect.x = (actor->unkC->unk10 >> 4) & 0xF0;
        rect.y = 0x1F2;
        rect.w = 0x10;
        rect.h = 1;
    }
    goto load;

case3:
    if (actor->owner_object_index < 2)
    {
        buf = &D_80104B58[actor->owner_object_index << 10];
        func_800801F8(buf + 2, 0xFF, (actor->unkC->unk10 >> 7) & 1);
        rect.x = 0;
        rect.y = (actor->owner_object_index * 2) + 0x1EE;
        rect.w = 0x100;
        rect.h = 1;
    }
    else
    {
        buf = D_80105358;
        func_800801F8(buf + 2, 0xFF, (actor->unkC->unk10 >> 7) & 1);
        rect.y = 0x1F2;
        rect.w = 0x100;
        rect.x = 0;
        rect.h = 1;
    }
load:
    LoadImage(&rect, (u_long *)buf);
done:
    ;
}
