/** @file field_mesh_render.c
 * @brief Upload animated texture strips and emit textured and lighted actor mesh packets.
 */

#include "common.h"
#include "field_types.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

typedef struct
{
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
} FieldSVector;

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
    FieldActorPartDef *unk0;      /* 0x000 */
    u8 pad4[0xC - 0x4];
    FieldActorAnimationDef *unkC; /* 0x00C */
    u8 pad10[0x18 - 0x10];
    u8 *unk18;                    /* 0x018 */
    u8 pad1C[0x1EC - 0x1C];
    u16 unk1EC[9];                /* 0x1EC */
    u8 pad1FE[0x228 - 0x1FE];
    u8 owner_object_index;        /* 0x228 */
    u8 unk229[9];                 /* 0x229 */
    u8 pad232;                    /* 0x232 */
    u8 unk233;                    /* 0x233 */
    u8 pad234[0x244 - 0x234];
} FieldActorState;

extern Struct_D800FDF58 D_800FDF58[];
extern Struct_D800FDF58 g_field_effect_records[];
extern Struct_D80105AE0 D_80105AE0[];
extern s32 g_field_track_index;
extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;
extern u8 D_80104B58[];
extern u8 D_80105358[];

extern FieldActorState g_field_actor_slots[80];
extern s32 *D_80105790;
extern s32 *D_80105878;

void func_8007E5FC(s16 *out, s32 mirror, u8 *item);
void func_800801F8(u16 *buf, s32 count, s32 flag);
void field_resolve_effect_part_color(FieldActorState *actor, Struct_D800FDF58 *rec, FieldActorPartDef *part, u8 *out);
void func_800822A4(FieldActorState *actor, Struct_D800FDF58 *rec, FieldActorPartDef *part, s32 part_index);
s32 func_80082C90(FieldActorState *actor, Struct_D800FDF58 *rec, FieldActorPartDef *part, MATRIX *mtx, MATRIX *tmp);
s32 *field_render_effect_mesh(Struct_D800FDF58 *rec, s32 part_index, s32 *cursor, s32 *base);


#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

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
            func_800801F8((u16 *)(buf + 2), 0xF, (a2 >> 7) & 1);
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
        func_800801F8((u16 *)(buf + 2), 0xF, (a2 >> 7) & 1);
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
        func_800801F8((u16 *)(buf + 2), 0xFF, (actor->unkC->unk10 >> 7) & 1);
        rect.x = 0;
        rect.y = (actor->owner_object_index * 2) + 0x1EE;
        rect.w = 0x100;
        rect.h = 1;
    }
    else
    {
        buf = D_80105358;
        func_800801F8((u16 *)(buf + 2), 0xFF, (actor->unkC->unk10 >> 7) & 1);
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

/**
 * @brief Rotate a row of u16 cells by one element in place.
 * @param buf   Pointer to the first cell of the row.
 * @param count Number of cells in the row.
 * @param flag  Direction: nonzero rotates right (last cell wraps to the
 *              front); zero rotates left (first cell wraps to the back).
 * @see decomp.me (100%) https://decomp.me/scratch (func_800801F8)
 */
void func_800801F8(u16 *buf, s32 count, s32 flag)
{
    u16 temp;
    s32 i;

    if (flag)
    {
        temp = buf[count - 1];
        for (i = count - 1; i > 0; i--)
        {
            buf[i] = buf[i - 1];
        }
        buf[0] = temp;
    }
    else
    {
        temp = buf[0];
        for (i = 0; i < count - 1; i++)
        {
            buf[i] = buf[i + 1];
        }
        buf[i] = temp;
    }
}

/** @brief Clamp @p expr into 0..0xFF and store it into @p dst. */
#define CLAMP_TO(dst, expr) do { \
    s32 _v = (expr); \
    s32 _out; \
    if (_v >= 0) { \
        _out = 0xFF; \
        if (_v < 0x100) _out = _v; \
    } else { \
        _out = 0; \
    } \
    (dst) = _out; \
} while (0)

/**
 * @brief Emit ordering-table primitives for one mesh of an actor part.
 * @param rec Effect/actor position record driving the transform.
 * @param part_index Mesh index within the part (also selects the face stream).
 * @param cursor Current write position in the primitive buffer.
 * @param arg3_base Base of the ordering-table link array.
 * @return Advanced primitive-buffer cursor.
 * @see decomp.me (100%) https://decomp.me/scratch (field_render_effect_mesh)
 */
s32 *field_render_effect_mesh(Struct_D800FDF58 *rec, s32 part_index, s32 *cursor, s32 *arg3_base)
{
    volatile s32 pad[2];
    MATRIX mtx;
    MATRIX tmp;
    MATRIX *mp;
    u8 color[4];
    s32 opz;
    FieldActorState *actor;
    FieldActorPartDef *part;
    u8 *face;
    s32 *sxy;
    s32 *depths;
    s16 *screen;
    s32 kind;
    s32 mesh_off;
    s32 sx, sy, rx, ry, rz, sz;
    s32 sx8, sy8, ybase;
    s32 *base;

    mp = &tmp;
    base = arg3_base;
    mp = &mtx;
    { FieldActorState *actor_base = g_field_actor_slots; part = &actor_base[rec->unk22].unk0[rec->unk23]; actor = &actor_base[rec->unk22]; }

    func_80082C90(actor, rec, part, mp, &tmp);
    mtx.t[2] = 0;
    mtx.t[1] = 0;
    mtx.t[0] = 0;
    field_resolve_effect_part_color(actor, rec, part, color);
    screen = (s16 *)0x1F800000;
    gte_SetRotMatrix(mp);
    gte_SetTransMatrix(mp);
    func_800822A4(actor, rec, part, part_index);

    sxy = D_80105790;
    depths = D_80105878;
    sx = D_800F22A0;
    mesh_off = part_index * 0x18;
    { s32 fa = mesh_off; fa += (s32)actor->unk18; face = *(u8 **)(fa + 0x14); }
    screen[0] = 0xA0 + D_800F22A0 / 256 + rec->unk0 / 256;
    screen[1] = 0x70 + D_800F22A4 / 256 + rec->unk4 / 256 - rec->unk8 / 512 - D_800F22A8 / 512;
    kind = (face[6] >> 1) & 0xF;

    switch (kind) {
    case 0:
    {
        s32 count0;
        volatile u8 *p;
        s32 prim_code;
        s32 lowmask;
        s32 highmask;
        { s32 ca = mesh_off; ca += (s32)actor->unk18; count0 = *(u16 *)ca; }
        if (count0 != 0) {
            p = (u8 *)cursor + 0xE;
            prim_code = 0x24;
            do { lowmask = 0xFFFFFF; } while (0);
            do { do { do { highmask = 0xFF000000; } while (0); } while (0); } while (0);
            do {
                gte_ldsxy3(sxy[0], sxy[1], sxy[2]);
                gte_nclip();
                gte_stopz(&opz);
                if (opz > 0) {
                    rx = *(s32 *)color;
                    p[-11] = 7;
                    *(volatile s32 *)(p - 10) = rx;
                    p[-7] = prim_code;
                    if (rec->unk1C & 0x800000) p[-7] = 0x26;
                    *(volatile s32 *)(p - 6) = sxy[0];
                    *(volatile s32 *)(p + 2) = sxy[1];
                    *(volatile s32 *)(p + 10) = sxy[2];
                    *(volatile u16 *)(p - 6) += (u16)screen[0];
                    *(volatile u16 *)(p - 4) += (u16)screen[1];
                    *(volatile u16 *)(p + 2) += (u16)screen[0];
                    *(volatile u16 *)(p + 4) += (u16)screen[1];
                    *(volatile u16 *)(p + 10) += (u16)screen[0];
                    *(volatile u16 *)(p + 12) += (u16)screen[1];
                    *(volatile u16 *)(p - 2) = *(u16 *)face;
                    do {
                    *(volatile u16 *)(p + 6) = *(u16 *)(face + 2);
                    *(volatile u16 *)(p + 14) = *(u16 *)(face + 4);
                    } while (0);
                    if (actor->owner_object_index < 2) {
                        { s32 av; s32 pv; av=actor->owner_object_index; pv=part->unk2D; *(volatile u16 *)(p + 0) = ((av << 7) + 0x7B80) | (pv & 0x3F); }
                        *(s16 *)((u8 *)cursor + (p - (volatile u8 *)cursor) + 8) = ((part->unk34 >> 15) & 0x80) | ((part->unk4 >> 17) & 0x60) | 0x10 | ((((actor->owner_object_index << 6) + 0x340) & 0x3FF) >> 6);
                    } else {
                        *(volatile u16 *)(p + 0) = (part->unk2D & 0x3F) | 0x7C80;
                        *(volatile s16 *)(p + 8) = ((part->unk34 >> 15) & 0x80) | ((part->unk4 >> 17) & 0x60) | 5;
                    }
                    if ((part->unk0 >> 21) & 1) {
                        *(volatile u16 *)p = (*(volatile u16 *)p & 0xFFC0) + 0x40;
                    }
                    {
                        s32 d = rec->unk8 >> 7;
                        s32 off = *depths;
                        s32 idx = d + off;
                        if (idx < 0) {
                            s32 old;
                            s32 addr;
                            p += 0x20;
                            old = *cursor;
                            *cursor = (old & highmask) | (base[0] & lowmask);
                            addr = (s32)cursor & lowmask;
                            cursor = (s32 *)((u8 *)cursor + 0x20);
                            base[0] = (base[0] & highmask) | addr;
                        } else if (idx >= 0x1000) {
                            s32 old;
                            s32 addr;
                            p += 0x20;
                            old = *cursor;
                            *cursor = (old & highmask) | (base[0xFFF] & lowmask);
                            addr = (s32)cursor & lowmask;
                            cursor = (s32 *)((u8 *)cursor + 0x20);
                            base[0xFFF] = (base[0xFFF] & highmask) | addr;
                        } else {
                            s32 addr;
                            s32 *entry;
                            p += 0x20;
                            addr = (s32)cursor & lowmask;
                            *cursor = (*cursor & highmask) | (*((s32 *)((off << 2) + ((d << 2) + (s32)base))) & lowmask);
                            { s32 rd = rec->unk8 >> 7; s32 roff = *depths; entry = (s32 *)((roff << 2) + ((rd << 2) + (s32)base)); }
                            cursor = (s32 *)((u8 *)cursor + 0x20);
                            *entry = (*entry & highmask) | addr;
                        }
                    }
                }
                face += 0x10;
                count0--;
                sxy += 3;
                depths++;
            } while (count0 != 0);
        }
        return cursor;
    }
    case 1:
    {
        s32 count1;
        s32 *p = cursor;
        u8 *fc;
        s32 d, off, idx;
        s32 prim_code;
        s32 lowmask;
        s32 highmask;
        { s32 mb = (s32)actor->unk18; count1 = *(u16 *)(mesh_off + mb); }
        if (count1 != 0) {
            prim_code = 0x20;
            lowmask = 0xFFFFFF;
            highmask = 0xFF000000;
            do {
                gte_ldsxy3(sxy[0], sxy[1], sxy[2]);
                gte_nclip();
                gte_stopz(&opz);
                if (opz > 0) {
                    fc = face + 2;
                    ((u8 *)p)[4] = face[0] + color[0] - 0x80;
                    ((u8 *)p)[5] = fc[-1] + color[1] - 0x80;
                    ((u8 *)p)[6] = fc[0] + color[2] - 0x80;
                    *(s32 *)((u8 *)p + 8) = sxy[0];
                    *(s32 *)((u8 *)p + 12) = sxy[1];
                    *(s32 *)((u8 *)p + 16) = sxy[2];
                    *(u16 *)((u8 *)p + 8) += *(u16 *)screen;
                    *(u16 *)((u8 *)p + 10) += *(u16 *)((u8 *)screen + 2);
                    *(u16 *)((u8 *)p + 12) += *(u16 *)screen;
                    *(u16 *)((u8 *)p + 14) += *(u16 *)((u8 *)screen + 2);
                    *(u16 *)((u8 *)p + 16) += *(u16 *)screen;
                    *(u16 *)((u8 *)p + 18) += *(u16 *)((u8 *)screen + 2);
                    ((u8 *)p)[3] = 4;
                    ((u8 *)p)[7] = prim_code;
                    if (rec->unk1C & 0x800000) ((u8 *)p)[7] = 0x22;

                    off = *depths;
                    d = rec->unk8 >> 7;
                    idx = d + off;
                    if (idx < 0) {
                        *p = (*p & highmask) | (base[0] & lowmask);
                        base[0] = (base[0] & highmask) | ((s32)p & lowmask);
                        p = (s32 *)((u8 *)p + 0x14);
                    } else if (idx >= 0x1000) {
                        *p = (*p & highmask) | (base[0xFFF] & lowmask);
                        base[0xFFF] = (base[0xFFF] & highmask) | ((s32)p & lowmask);
                        p = (s32 *)((u8 *)p + 0x14);
                    } else {
                        *p = (*p & highmask) | (*((s32 *)((off << 2) + ((d << 2) + (s32)base))) & lowmask);
                        {
                            s32 rd = rec->unk8 >> 7;
                            s32 roff = *depths;
                            s32 *entry = (s32 *)((roff << 2) + ((rd << 2) + (s32)base));
                            *entry = (*entry & highmask) | ((s32)p & lowmask);
                        }
                        p = (s32 *)((u8 *)p + 0x14);
                    }

                    ((u8 *)p)[3] = 1;
                    {
                        s32 mode0 = part->unk34 >> 15;
                        s32 mode1 = part->unk4 >> 17;
                        mode1 &= 0x60;
                        mode0 &= 0x80;
                        mode0 |= mode1;
                        mode0 |= 0xE1000000;
                        *(s32 *)((u8 *)p + 4) = mode0;
                    }
                    off = *depths;
                    d = rec->unk8 >> 7;
                    idx = d + off;
                    {
                        s32 *next;
                        if (idx < 0) {
                            next = (s32 *)((u8 *)p + 8);
                            *p = (*p & highmask) | (base[0] & lowmask);
                            base[0] = (base[0] & highmask) | ((s32)p & lowmask);
                        } else if (idx >= 0x1000) {
                            next = (s32 *)((u8 *)p + 8);
                            *p = (*p & highmask) | (base[0xFFF] & lowmask);
                            base[0xFFF] = (base[0xFFF] & highmask) | ((s32)p & lowmask);
                        } else {
                            next = (s32 *)((u8 *)p + 8);
                            *p = (*p & highmask) | (*((s32 *)((off << 2) + ((d << 2) + (s32)base))) & lowmask);
                            {
                                s32 rd = rec->unk8 >> 7;
                                s32 roff = *depths;
                                s32 *entry = (s32 *)((roff << 2) + ((rd << 2) + (s32)base));
                                *entry = (*entry & highmask) | ((s32)p & lowmask);
                            }
                        }
                        p = next;
                    }
                }
                face += 0x10;
                do { count1--; } while (0);
                sxy += 3;
                depths++;
            } while (count1 != 0);
        }
        cursor = p;
        break;
    }
    case 2:
    {
        s32 count2;
        u8 *basecur;
        s32 lowmask;
        s32 highmask;
        u8 code2;
        { s32 ca = mesh_off; ca += (s32)actor->unk18; count2 = *(u16 *)ca; }
        do { do { do { do { do { basecur=(u8*)cursor; } while (0); } while (0); } while (0); } while (0); } while (0);
        if(count2!=0){
            do { do { do { lowmask=0xFFFFFF; } while (0); } while (0); } while (0);
            do { do { do { do { do { do { highmask=0xFF000000; } while (0); } while (0); } while (0); } while (0); } while (0); } while (0);
            do {
                s32 a=sxy[0],b=sxy[1],c=sxy[2];
                gte_ldsxy3(a,b,c); gte_nclip(); gte_stopz(&opz);
                if(opz>0){
                    CLAMP_TO(basecur[4], face[7]+color[0]-0x80);
                    CLAMP_TO(basecur[5], face[8]+color[1]-0x80);
                    CLAMP_TO(basecur[6], face[9]+color[2]-0x80);
                    CLAMP_TO(basecur[16], face[10]+color[0]-0x80);
                    CLAMP_TO(basecur[17], face[11]+color[1]-0x80);
                    CLAMP_TO(basecur[18], face[12]+color[2]-0x80);
                    CLAMP_TO(basecur[28], face[13]+color[0]-0x80);
                    CLAMP_TO(basecur[29], face[14]+color[1]-0x80);
                    CLAMP_TO(basecur[(basecur[3] = 9, code2 = 0x34, 30)], face[15]+color[2]-0x80);
                    basecur[7]=code2; setSemiTrans(basecur, rec->unk1C & 0x800000);
                    do { *(s32 *)(basecur+8)=sxy[0]; *(s32 *)(basecur+20)=sxy[1]; *(s32 *)(basecur+32)=sxy[2]; } while (0);
                    *(u16 *)(basecur+8)+=(u16)screen[0]; *(u16 *)(basecur+10)+=(u16)screen[1];
                    *(u16 *)(basecur+20)+=(u16)screen[0]; *(u16 *)(basecur+22)+=(u16)screen[1];
                    *(u16 *)(basecur+32)+=(u16)screen[0]; *(u16 *)(basecur+34)+=(u16)screen[1];
                    *(u16 *)(basecur+12)=*(u16*)face; *(u16 *)(basecur+24)=*(u16 *)(face+2); *(u16 *)(basecur+36)=*(u16 *)(face+4);
                    if(actor->owner_object_index<2){
                        { s32 av; s32 pv; av=actor->owner_object_index; pv=part->unk2D; *(u16*)(basecur+14)=((av<<7)+0x7B80)|(pv&0x3F); }
                        *(s16*)(basecur+26)=((part->unk34>>15)&0x80)|((part->unk4>>17)&0x60)|0x10|((((actor->owner_object_index<<6)+0x340)&0x3FF)>>6);
                    } else {
                        *(u16*)(basecur+14)=(part->unk2D&0x3F)|0x7C80;
                        *(s16*)(basecur+26)=((part->unk34>>15)&0x80)|((part->unk4>>17)&0x60)|5;
                    }
                    if((part->unk0>>21)&1) *(u16*)(basecur+14)=(*(u16*)(basecur+14)&0xFFC0)+0x40;
                    {
                        s32 off = *depths;
                        s32 d = rec->unk8 >> 7;
                        s32 idx = d + off;
                        if (idx < 0) {
                            s32 addr;
                            *(s32 *)basecur = (*(s32 *)basecur & highmask) | (base[0] & lowmask);
                            addr = (s32)basecur & lowmask;
                            basecur += 0x28;
                            base[0] = (base[0] & highmask) | addr;
                        } else if (idx >= 0x1000) {
                            s32 addr;
                            *(s32 *)basecur = (*(s32 *)basecur & highmask) | (base[0xFFF] & lowmask);
                            addr = (s32)basecur & lowmask;
                            basecur += 0x28;
                            base[0xFFF] = (base[0xFFF] & highmask) | addr;
                        } else {
                            s32 addr;
                            s32 *entry;
                            addr = (s32)basecur & lowmask;
                            *(s32 *)basecur = (*(s32 *)basecur & highmask) | (*((s32 *)((off << 2) + ((d << 2) + (s32)base))) & lowmask);
                            { s32 rd2; s32 roff2; rd2 = rec->unk8 >> 7; roff2 = *depths;
                                entry = (s32 *)((roff2 << 2) + ((rd2 << 2) + (s32)base)); }
                            basecur += 0x28;
                            *entry = (*entry & highmask) | addr;
                        }
                    }
                }
                face+=0x10; do { do { do { count2--; } while (0); } while (0); } while (0); sxy+=3; depths++;
            }while(count2!=0);
        }
        cursor=(s32*)basecur;
        break;
    }
    default:
        return cursor;
    }
    return cursor;
}

extern SVECTOR D_800FF668;
extern SVECTOR *D_80105870;

#define CLAMP_LIGHTED_COLOR_TO(dst, expr) { \
    s32 _v = (expr); \
    s32 _out; \
    if (_v >= 0) { \
        _out = 0xFF; \
        if (_v < 0x100) _out = _v; \
    } else { \
        _out = 0; \
    } \
    (dst) = _out; \
}

/**
 * @brief Emit lit ordering-table primitives for one mesh of an actor part.
 *
 * Sibling of field_render_effect_mesh that additionally builds a 3-source light and
 * color matrix pair from the part's light ids (searching g_field_effect_records for the
 * owning record of each light), then emits per-face prims in three variants:
 * case 0 = textured tri with normal-driven shading (ncs), case 2 = gouraud
 * tri with clamped per-vertex colors, case 1 = flat tri plus a DR_TPAGE
 * chaser packet.
 *
 * @param rec Effect/actor position record driving the transform.
 * @param part_index Mesh index within the part (also selects the face stream).
 * @param cursor Current write position in the primitive buffer.
 * @param arg3_base Base of the ordering-table link array.
 * @return Advanced primitive-buffer cursor.
 * @see decomp.me (100%) https://decomp.me/scratch/F2Z4z
 */
s32 *field_render_lit_effect_mesh(Struct_D800FDF58 *rec, s32 part_index, s32 *cursor, s32 *arg3_base)
{
    s32 pad[2];
    s32 *prim_cursor;
    MATRIX mtx;
    MATRIX tmp;
    MATRIX *mp;
    MATRIX *rotation_matrix;
    u8 color[4];
    MATRIX light_mtx;
    MATRIX color_mtx;
    SVECTOR dir;
    SVECTOR out;
    SVECTOR *rotation_table;
    SVECTOR *rotation_base;
    s32 opz;
    FieldActorState *actor;
    FieldActorPartDef *part;
    Struct_D800FDF58 *scan;
    s32 light_index;
    s32 scan_off;
    s32 *sxy;
    SVECTOR *normals;
    s32 *depths;
    s16 *screen;
    u8 *face;
    s32 mesh_off;
    s32 mesh_bytes;
    s32 count;
    s32 kind;
    FieldActorState *actor_base;

    prim_cursor = cursor;
    mp = &mtx;
    actor_base = g_field_actor_slots;
    part = &actor_base[rec->unk22].unk0[rec->unk23];
    actor = &actor_base[rec->unk22];

    func_80082C90(actor, rec, part, mp, &tmp);
    mtx.t[2] = 0;
    mtx.t[1] = 0;
    mtx.t[0] = 0;
    field_resolve_effect_part_color(actor, rec, part, color);
    screen = (s16 *)0x1F800000;
    gte_SetRotMatrix(mp);
    gte_SetTransMatrix(mp);
    func_800822A4(actor, rec, part, part_index);
    func_800829A0(actor, rec, part, part_index, &tmp);

    light_index = 0;
    rotation_base = &D_800FF668;

    do {
        count = 0;
        rotation_table = rotation_base;
        if (((u8 *)part)[light_index + 0x23] < 8) {
            do {
                {
                    scan = &g_field_effect_records[count];
                    scan_off = count * 0x54;
                }
                if (((u8 *)part)[light_index + 0x23] == scan->unk23 && rec->unk22 == scan->unk22) {
                    rotation_matrix = &tmp;
                    RotMatrix_gte((SVECTOR *)(scan_off + (s32)rotation_table), rotation_matrix);
                    RotMatrixZ(rec->unk32 * 0x10, rotation_matrix);
                    RotMatrixY(rec->unk33 * 0x10, rotation_matrix);
                    dir.vz = 0;
                    dir.vx = 0;
                    dir.vy = -0x1000;
                    gte_SetRotMatrix(rotation_matrix);
                    gte_ldv0(&dir);
                    gte_rtv0();
                    gte_stsv(&out);
                    light_mtx.m[light_index][0] = out.vx;
                    light_mtx.m[light_index][1] = out.vy;
                    light_mtx.m[light_index][2] = out.vz;
                    color_mtx.m[0][light_index] = g_field_actor_slots[rec->unk22].unk0[scan->unk23].unkE * 0x10;
                    color_mtx.m[1][light_index] = g_field_actor_slots[rec->unk22].unk0[scan->unk23].unkF * 0x10;
                    color_mtx.m[2][light_index] = g_field_actor_slots[rec->unk22].unk0[scan->unk23].unk10 * 0x10;
                    break;
                }
                count++;
            } while (count < 0x100);
            if (count == 0x100) {
                color_mtx.m[2][light_index] = 0;
                color_mtx.m[1][light_index] = 0;
                color_mtx.m[0][light_index] = 0;
                light_mtx.m[light_index][0] = 0;
            }
        } else {
            color_mtx.m[2][light_index] = 0;
            color_mtx.m[1][light_index] = 0;
            color_mtx.m[0][light_index] = 0;
            light_mtx.m[light_index][0] = 0;
        }
        light_index++;
    } while (light_index < 3);

    gte_SetLightMatrix(&light_mtx);
    gte_SetColorMatrix(&color_mtx);
    sxy = D_80105790;
    normals = D_80105870;
    depths = D_80105878;
    mesh_bytes = part_index * 0x18;
    mesh_off = mesh_bytes;
    { s32 fa = mesh_off; fa += (s32)actor->unk18; face = *(u8 **)(fa + 0x14); }
    screen[0] = 0xA0 + D_800F22A0 / 256 + rec->unk0 / 256;
    screen[1] = 0x70 + D_800F22A4 / 256 + rec->unk4 / 256 - rec->unk8 / 512 - D_800F22A8 / 512;
    kind = (face[6] >> 1) & 0xF;

    switch (kind) {
    case 0:
    {
        u8 *p;
        s32 lowmask;
        s32 highmask;
        s32 prim_code;
        s32 d, off, idx;

        gte_SetBackColor(color[0], color[1], color[2]);
        if (actor->owner_object_index < 2) {
            { u8 actor_index; s32 palette; lowmask = actor->owner_object_index; actor_index = lowmask; palette = part->unk2D;
              *(u16 *)((u8 *)prim_cursor + 0xE) = ((actor_index << 7) + 0x7B80) | (palette & 0x3F); }
            *(s16 *)((u8 *)prim_cursor + 0x16) = ((part->unk34 >> 15) & 0x80) | ((part->unk4 >> 17) & 0x60) | 0x10 | ((((actor->owner_object_index << 6) + 0x340) & 0x3FF) >> 6);
        } else {
            *(u16 *)((u8 *)prim_cursor + 0xE) = (part->unk2D & 0x3F) | 0x7C80;
            *(s16 *)((u8 *)prim_cursor + 0x16) = ((part->unk34 >> 15) & 0x80) | ((part->unk4 >> 17) & 0x60) | 5;
        }
        if ((part->unk0 >> 21) & 1)
            *(u16 *)((u8 *)prim_cursor + 0xE) = (*(u16 *)((u8 *)prim_cursor + 0xE) & 0xFFC0) + 0x40;

        count = *(u16 *)(actor->unk18 + part_index * 0x18);
        if (count != 0) {
            prim_code = 0x24;
            lowmask = 0xFFFFFF;
            highmask = 0xFF000000;
            do {
                p = (u8 *)prim_cursor + 0x36;
                gte_ldsxy3(sxy[0], sxy[1], sxy[2]);
                gte_nclip();
                gte_stopz(&opz);
                if (opz > 0) {
                    gte_ldv0(normals);
                    gte_ncs();
                    gte_strgb((u8 *)prim_cursor + 4);
                    *(s32 *)(p - 0x2E) = sxy[0];
                    *(s32 *)(p - 0x26) = sxy[1];
                    *(s32 *)(p - 0x1E) = sxy[2];
                    *(u16 *)(p - 0x2E) += (u16)screen[0];
                    *(u16 *)(p - 0x2C) += (u16)screen[1];
                    *(u16 *)(p - 0x26) += (u16)screen[0];
                    *(u16 *)(p - 0x24) += (u16)screen[1];
                    *(u16 *)(p - 0x1E) += (u16)screen[0];
                    *(u16 *)(p - 0x1C) += (u16)screen[1];
                    p[-0x33] = 7;
                    p[-0x2F] = prim_code;
                    if (rec->unk1C & 0x800000) p[-0x2F] = 0x26;
                    else p[-0x2F] = prim_code;
                    *(u16 *)(p - 0x2A) = *(u16 *)face;
                    *(u16 *)(p - 0x22) = *(u16 *)(face + 2);
                    *(u16 *)(p - 0x1A) = *(u16 *)(face + 4);
                    *(u16 *)(p - 8) = *(u16 *)(p - 0x28);
                    *(u16 *)((u8 *)prim_cursor + 0x36) = *(u16 *)(p - 0x20);

                    off = *depths;
                    d = rec->unk8 >> 7;
                    idx = d + off;
                    if (idx < 0) {
                        *prim_cursor = (*prim_cursor & highmask) | (arg3_base[0] & lowmask);
                        arg3_base[0] = (arg3_base[0] & highmask) | ((s32)prim_cursor & lowmask);
                        prim_cursor = (s32 *)((u8 *)prim_cursor + 0x20);
                    } else if (idx >= 0x1000) {
                        *prim_cursor = (*prim_cursor & highmask) | (arg3_base[0xFFF] & lowmask);
                        arg3_base[0xFFF] = (arg3_base[0xFFF] & highmask) | ((s32)prim_cursor & lowmask);
                        prim_cursor = (s32 *)((u8 *)prim_cursor + 0x20);
                    } else {
                        s32 *entry;
                        *prim_cursor = (*prim_cursor & highmask) | (*((s32 *)((off << 2) + ((d << 2) + (s32)arg3_base))) & lowmask);
                        { s32 rd = rec->unk8 >> 7; s32 roff = *depths; entry = (s32 *)((roff << 2) + ((rd << 2) + (s32)arg3_base)); }
                        *entry = (*entry & highmask) | ((s32)prim_cursor & lowmask);
                        prim_cursor = (s32 *)((u8 *)prim_cursor + 0x20);
                    }
                }
                face += 0x10;
                count--;
                sxy += 3;
                normals++;
                depths++;
            } while (count != 0);
        }
        return prim_cursor;
    }
    case 2:
    {
        u8 *basecur;
        { s32 ca = mesh_off; ca += (s32)actor->unk18; count = *(u16 *)ca; }
        basecur = (u8 *)prim_cursor;
        if(count!=0){
            do {
                gte_ldsxy3(sxy[0],sxy[1],sxy[2]); gte_nclip(); gte_stopz(&opz);
                if(opz>0){
                    CLAMP_LIGHTED_COLOR_TO(basecur[4], face[7]+color[0]-0x80);
                    CLAMP_LIGHTED_COLOR_TO(basecur[5], face[8]+color[1]-0x80);
                    CLAMP_LIGHTED_COLOR_TO(basecur[6], face[9]+color[2]-0x80);
                    CLAMP_LIGHTED_COLOR_TO(basecur[16], face[10]+color[0]-0x80);
                    CLAMP_LIGHTED_COLOR_TO(basecur[17], face[11]+color[1]-0x80);
                    CLAMP_LIGHTED_COLOR_TO(basecur[18], face[12]+color[2]-0x80);
                    CLAMP_LIGHTED_COLOR_TO(basecur[28], face[13]+color[0]-0x80);
                    CLAMP_LIGHTED_COLOR_TO(basecur[29], face[14]+color[1]-0x80);
                    { s32 blue;
                        CLAMP_LIGHTED_COLOR_TO(blue, face[15]+color[2]-0x80);
                        setlen(basecur, 9);
                        basecur[30] = blue;
                    }
                    setcode(basecur, 0x34);
                    setSemiTrans(basecur, rec->unk1C & 0x800000);
                    *(s32 *)(basecur+8)=sxy[0]; *(s32 *)(basecur+20)=sxy[1]; *(s32 *)(basecur+32)=sxy[2];
                    *(u16 *)(basecur+8)+=(u16)screen[0]; *(u16 *)(basecur+10)+=(u16)screen[1];
                    *(u16 *)(basecur+20)+=(u16)screen[0]; *(u16 *)(basecur+22)+=(u16)screen[1];
                    *(u16 *)(basecur+32)+=(u16)screen[0]; *(u16 *)(basecur+34)+=(u16)screen[1];
                    *(u16 *)(basecur+12)=*(u16*)face; *(u16 *)(basecur+24)=*(u16 *)(face+2); *(u16 *)(basecur+36)=*(u16 *)(face+4);
                    if(actor->owner_object_index<2){
                        { s32 av; s32 pv; av=actor->owner_object_index; pv=part->unk2D; *(u16*)(basecur+14)=((av<<7)+0x7B80)|(pv&0x3F); }
                        *(s16*)(basecur+26)=((part->unk34>>15)&0x80)|((part->unk4>>17)&0x60)|0x10|((((actor->owner_object_index<<6)+0x340)&0x3FF)>>6);
                    } else {
                        *(u16*)(basecur+14)=(part->unk2D&0x3F)|0x7C80;
                        *(s16*)(basecur+26)=((part->unk34>>15)&0x80)|((part->unk4>>17)&0x60)|5;
                    }
                    if((part->unk0>>21)&1) *(u16*)(basecur+14)=(*(u16*)(basecur+14)&0xFFC0)+0x40;
                    {
                        s32 off = *depths;
                        s32 d = rec->unk8 >> 7;
                        s32 idx = d + off;
                        if (idx < 0) {
                        addPrim(arg3_base, basecur);
                        basecur += 0x28;
                    } else {
                        if (idx >= 0x1000) {
                            addPrim(&arg3_base[0xFFF], basecur);
                            basecur += 0x28;
                        } else {
                            { s32 rd; s32 roff;
                            addPrim((rd = rec->unk8 >> 7, roff = *depths, (s32 *)((roff << 2) + ((rd << 2) + (s32)arg3_base))), basecur); }
                            basecur += 0x28;
                        }
                    }
                }
                }
                face += 0x10; count--; sxy += 3; depths++;
            }while(count!=0);
        }
        prim_cursor=(s32*)basecur;
        break;
    }
    case 1:
    {
        s32 *p = prim_cursor;
        u8 *fc;
        s32 d, off, idx;
        s32 prim_code;
        s32 lowmask;
        s32 highmask;
        { s32 mb = (s32)actor->unk18; count = *(u16 *)(mesh_off + mb); }
        if (count != 0) {
            prim_code = 0x20;
            lowmask = 0xFFFFFF;
            highmask = 0xFF000000;
            do {
                gte_ldsxy3(sxy[0], sxy[1], sxy[2]);
                gte_nclip();
                gte_stopz(&opz);
                if (opz > 0) {
                    fc = face + 2;
                    gte_SetBackColor(face[0] + color[0] - 0x80,
                                     fc[-1] + color[1] - 0x80,
                                     fc[0] + color[2] - 0x80);
                    gte_ldv0(normals);
                    gte_ncs();
                    gte_strgb((u8 *)p + 4);
                    *(s32 *)((u8 *)p + 8) = sxy[0];
                    *(s32 *)((u8 *)p + 12) = sxy[1];
                    *(s32 *)((u8 *)p + 16) = sxy[2];
                    *(u16 *)((u8 *)p + 8) += *(u16 *)screen;
                    *(u16 *)((u8 *)p + 10) += *(u16 *)((u8 *)screen + 2);
                    *(u16 *)((u8 *)p + 12) += *(u16 *)screen;
                    *(u16 *)((u8 *)p + 14) += *(u16 *)((u8 *)screen + 2);
                    *(u16 *)((u8 *)p + 16) += *(u16 *)screen;
                    *(u16 *)((u8 *)p + 18) += *(u16 *)((u8 *)screen + 2);
                    ((u8 *)p)[3] = 4;
                    ((u8 *)p)[7] = prim_code;
                    if (rec->unk1C & 0x800000) ((u8 *)p)[7] = 0x22;
                    else ((u8 *)p)[7] = prim_code;

                    off = *depths;
                    d = rec->unk8 >> 7;
                    idx = d + off;
                    if (idx < 0) {
                        *p = (*p & highmask) | (arg3_base[0] & lowmask);
                        arg3_base[0] = (arg3_base[0] & highmask) | ((s32)p & lowmask);
                        p = (s32 *)((u8 *)p + 0x14);
                    } else if (idx >= 0x1000) {
                        *p = (*p & highmask) | (arg3_base[0xFFF] & lowmask);
                        arg3_base[0xFFF] = (arg3_base[0xFFF] & highmask) | ((s32)p & lowmask);
                        p = (s32 *)((u8 *)p + 0x14);
                    } else {
                        *p = (*p & highmask) | (*((s32 *)((off << 2) + ((d << 2) + (s32)arg3_base))) & lowmask);
                        {
                            s32 rd = rec->unk8 >> 7;
                            s32 roff = *depths;
                            s32 *entry = (s32 *)((roff << 2) + ((rd << 2) + (s32)arg3_base));
                            *entry = (*entry & highmask) | ((s32)p & lowmask);
                        }
                        p = (s32 *)((u8 *)p + 0x14);
                    }

                    ((u8 *)p)[3] = 1;
                    {
                        s32 mode0 = part->unk34 >> 15;
                        s32 mode1 = part->unk4 >> 17;
                        mode1 &= 0x60;
                        mode0 &= 0x80;
                        mode0 |= mode1;
                        mode0 |= 0xE1000000;
                        *(s32 *)((u8 *)p + 4) = mode0;
                    }
                    off = *depths;
                    d = rec->unk8 >> 7;
                    idx = d + off;
                    {
                        s32 *next;
                        if (idx < 0) {
                            next = (s32 *)((u8 *)p + 8);
                            *p = (*p & highmask) | (arg3_base[0] & lowmask);
                            arg3_base[0] = (arg3_base[0] & highmask) | ((s32)p & lowmask);
                        } else if (idx >= 0x1000) {
                            next = (s32 *)((u8 *)p + 8);
                            *p = (*p & highmask) | (arg3_base[0xFFF] & lowmask);
                            arg3_base[0xFFF] = (arg3_base[0xFFF] & highmask) | ((s32)p & lowmask);
                        } else {
                            next = (s32 *)((u8 *)p + 8);
                            *p = (*p & highmask) | (*((s32 *)((off << 2) + ((d << 2) + (s32)arg3_base))) & lowmask);
                            {
                                s32 rd = rec->unk8 >> 7;
                                s32 roff = *depths;
                                s32 *entry = (s32 *)((roff << 2) + ((rd << 2) + (s32)arg3_base));
                                *entry = (*entry & highmask) | ((s32)p & lowmask);
                            }
                        }
                        p = next;
                    }
                }
                face += 0x10;
                count--;
                sxy += 3;
                normals++;
                depths++;
            } while (count != 0);
        }
        prim_cursor = p;
        break;
    }
    default:
        return prim_cursor;
    }
    return prim_cursor;
}
