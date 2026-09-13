#include "common.h"

/*
 * Consolidated FIELD actor-effect translation unit.
 *
 * Members in ascending address order:
 *   func_8009D95C (from field347.c)
 *   func_8009D9E0 (from func_8009D9E0.c)
 *   func_8009E66C (from func_8009E66C.c)
 *   func_8009FE54 (from func_8009FE54.c)
 *   func_800A0B0C (from func_800A0B0C.c)
 *   func_800A1344 (from func_800A1344.c)
 *
 * Note: func_8009D4D8 is deliberately NOT part of this TU; an interleaved unk
 * rodata blob sits between its jump table and this object's rodata, so it must
 * remain in its own file.
 *
 * D_801178D8 is read as u16 in func_8009E66C but as s32 in every other member,
 * so it is declared at block scope inside each user with that user's original
 * type and never at file scope (a file-scope copy would conflict).
 */

/* Shared camera globals, same type (s32) in every member that uses them. */
extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;

/**
 * @brief Look up a width/height pair for a field text-box style.
 *
 * Selects a fixed (@p arg1, @p arg2) size pair keyed by the style index
 * @p arg0 (0-6); out-of-range indices leave both outputs untouched.
 *
 * @param arg0 Style index (0-6).
 * @param arg1 Receives the first dimension.
 * @param arg2 Receives the second dimension.
 * @note Diff shows 99.70% in isolation; the only delta is the compiler-generated
 *       switch jump table being anonymous in the scratch vs the named
 *       jtbl_80051144 relocation, which resolves identically at link time.
 * @see decomp.me (100%) TODO
 */
void func_8009D95C(s32 arg0, s32 *arg1, s32 *arg2)
{
    switch (arg0)
    {
    case 0:
        *arg1 = 0x1E;
        *arg2 = 0x60;
        break;
    case 1:
        *arg1 = 0x40;
        *arg2 = 0x80;
        break;
    case 2:
        *arg1 = 0x10;
        *arg2 = 0x40;
        break;
    case 3:
        *arg1 = 0x1E;
        *arg2 = 0xC8;
        break;
    case 4:
        *arg1 = 0x1E;
        *arg2 = 0x40;
        break;
    case 5:
        *arg1 = 0;
        *arg2 = 0x64;
        break;
    case 6:
        *arg1 = 0x1E;
        *arg2 = 0x40;
        break;
    }
}

/* ---- func_8009D9E0 support declarations ---- */

#define M2C_FIELD(base, type, offset) (*(type)((unsigned char *)(base) + (offset)))
void func_8001CDAC(s32 *, s32 *);                /* extern */
extern void *D_800F2288;
/** @brief Per-actor effect state; unknown fields preserve the 0x23C stride. */
typedef struct
{
    u8 pad0[0x4A];
    u16 intensity;
    u8 pad4C[0x10];
    s32 angle;
    u8 pad60[0x114];
    union
    {
        u32 word;
        struct
        {
            u16 radius : 10;
            u16 flags : 6;
            u16 upper;
        } bits;
    } state;
    u8 pad178[0x18];
    s16 offsets[3][2];
    u8 pad19C[0xA0];
} EffectRecord;
extern EffectRecord D_80105AE0[];
extern void *g_pad_ctx;

/** @brief Actor fields used by the effect dispatcher. */
typedef struct
{
    s32 position[4];
    u8 pad10[0x11];
    u8 flags;
    u8 pad22[0x18];
    u8 slot;
} Actor;

/**
 * @brief Draw and advance an actor effect, including controller-driven offsets.
 * @param arg0 Actor supplying the position, facing flag, and effect slot.
 * @param arg1 Effect type, from 0 through 6.
 * @note 100% standalone; in this TU objdiff shows 99.975% solely because the
 *       two switch jump tables render as [.rodata]+0x20/+0x40 instead of the
 *       named jtbl_80051164/jtbl_80051184 (func_8009D95C owns rodata offset 0).
 *       Same registers and opcodes; a position artifact, links identically.
 */
void func_8009D9E0(Actor *arg0, u32 arg1)
{
    extern s32 D_801178D8;
    /*
     * Original per-file forward prototypes, kept block scope to preserve this
     * function's exact declaration environment (its baseline register
     * allocation depends on these void-pointer, s32-return forms, not the real
     * pointer-return signatures of the definitions later in this TU).
     */
    s32 func_8009E66C(void *, s32, s32 *, s32);
    s32 func_8009FE54(void *, s32, s32 *, s32);
    s32 func_800A0B0C(void *, s32, s32 *, s32, s32);
    s32 func_800A1344(void *, s32, s32 *, s32, s32);
    s32 vec[12];
    s16 screen[4];
    void *record_0;
    s32 count_0;
    void *record_1;
    s32 count_1;
    void *record_2;
    s32 count_2;
    void *record_3;
    s32 count_3;
    void *record_6;
    s32 count_6;
    s32 *position;
    u8 *pad = (u8 *)0x801ED600;
    s32 ratio;
    s32 facing;
    s32 limits[2];
    s16 temp_t0_2;
    s16 temp_v1_12;
    s32 temp_a0;
    s32 temp_a1;
    s32 temp_a1_2;
    s32 temp_a1_3;
    s32 temp_a1_4;
    s32 temp_a1_5;
    s32 temp_a3;
    s32 temp_s3;
    s32 temp_t0;
    s32 temp_t2;
    s32 temp_v1_10;
    s32 temp_v1_11;
    s32 temp_v1_9;
    s32 var_a0_2;
    s32 var_a0_3;
    s32 var_a0_4;
    s32 var_a1;
    s32 var_s0;
    s32 var_s2;
    s32 var_s6;
    s32 var_v0;
    s32 var_v0_2;
    s32 var_v0_3;
    s32 var_v0_4;
    s32 var_v0_5;
    s32 var_v0_6;
    s32 var_v0_7;
    s32 var_v1;
    s32 var_v1_2;
    s32 var_v1_3;
    u16 temp_v0_7;
    void *temp_a0_2;
    void *temp_a0_3;
    void *temp_a0_4;
    void *temp_s4;
    void *temp_v0;
    void *temp_v0_2;
    void *temp_v0_3;
    void *temp_v0_4;
    void *temp_v0_5;
    void *temp_v0_6;
    void *temp_v0_8;
    void *temp_v0_9;
    void *temp_v1;
    void *temp_v1_2;
    void *temp_v1_3;
    void *temp_v1_4;
    void *temp_v1_5;
    void *temp_v1_6;
    void *temp_v1_7;
    void *temp_v1_8;
    void *var_a0;

    temp_s4 = D_800F2288 + 0x40;
    var_s6 = M2C_FIELD(D_800F2288, s32 *, 0x40B8);
    {
        s32 *high;
        s32 *low;
        low = &limits[0];
        high = &limits[1];
        switch (arg1)
        {
        case 0:
            *low = 30;
            do
            {
                *high = 96;
            } while (0);
            break;
        case 1:
            *low = 64;
            *high = 128;
            break;
        case 2:
            *low = 16;
            *high = 64;
            break;
        case 3:
            *low = 30;
            *high = 200;
            break;
        case 5:
            limits[0] = 0;
            *high = 100;
            break;
        case 4:
        case 6:
            *low = 30;
            *high = 64;
            break;
        }
    }
    temp_s3 = ((EffectRecord *)((&D_80105AE0[arg0->slot])))->state.bits.radius;
    ratio = ((temp_s3 - limits[0]) << 8) / (limits[1] - limits[0]);
    facing = arg0->flags & 0x80;
    D_80105AE0[arg0->slot].intensity = ratio;
    position = arg0->position;
    if ((u16) ((EffectRecord *)((&D_80105AE0[arg0->slot])))->intensity >= 0x100U)
    {
        ((EffectRecord *)((&D_80105AE0[arg0->slot])))->intensity = 0xFFU;
    }
    var_s2 = 1;
    if (arg0->slot == 2)
    {
        if ((M2C_FIELD(g_pad_ctx, s32 *, 0xAA8) & 0x7F) == 4)
        {
            var_s2 = 0;
        }
    }
    D_801178D8 = ((EffectRecord *)(((void *)&D_80105AE0[arg0->slot])))->angle;
    switch (arg1)
    {
    case 0:
        if (var_s2 != 0)
        {
            var_s6 = func_8009E66C(temp_s4, var_s6, position, temp_s3);
        }
        D_80105AE0[arg0->slot].angle -= 0x80;
        record_0 = (void *)&D_80105AE0[arg0->slot];
        count_0 = ((EffectRecord *)(record_0))->state.bits.radius;
        if (count_0 < limits[1])
        {
            ((EffectRecord *)(record_0))->state.word = (((EffectRecord *)(record_0))->state.word & ~0x3FF) | ((count_0 + 2) & 0x3FF);
        }
        break;
    default:
        break;
    case 1:
        if (var_s2 != 0)
        {
            var_s6 = func_8009FE54(temp_s4, var_s6, position, temp_s3);
        }
        D_80105AE0[arg0->slot].angle -= 0x80;
        record_1 = (void *)&D_80105AE0[arg0->slot];
        count_1 = ((EffectRecord *)(record_1))->state.bits.radius;
        if (count_1 < limits[1])
        {
            ((EffectRecord *)(record_1))->state.word = (((EffectRecord *)(record_1))->state.word & ~0x3FF) | ((count_1 + 2) & 0x3FF);
        }
        break;
    case 2:
        if (var_s2 != 0)
        {
            var_s6 = func_800A1344(temp_s4, var_s6, position, temp_s3, facing);
        }
        D_80105AE0[arg0->slot].angle += 4;
        if (((EffectRecord *)((&D_80105AE0[arg0->slot])))->angle >= 0x50)
        {
            ((EffectRecord *)((&D_80105AE0[arg0->slot])))->angle = 0;
        }
        record_2 = (void *)&D_80105AE0[arg0->slot];
        temp_a1_2 = ((EffectRecord *)(record_2))->state.bits.radius;
        if (temp_a1_2 < limits[1])
        {
            var_v0_3 = (s32) ((EffectRecord *)(record_2))->state.word & ~0x3FF;
            var_v1 = temp_a1_2 + 1;
            ((EffectRecord *)(record_2))->state.word = var_v0_3 | (var_v1 & 0x3FF);
        }
        break;
    case 3:
        if (((EffectRecord *)((&D_80105AE0[arg0->slot])))->angle < 0)
        {
            ((EffectRecord *)((&D_80105AE0[arg0->slot])))->angle = 0;
        }
        if (var_s2 != 0)
        {
            var_s6 = func_800A0B0C(temp_s4, func_800A0B0C(temp_s4, var_s6, position, temp_s3, 0), position, temp_s3, 1);
        }
        D_80105AE0[arg0->slot].angle += 4;
        if (((EffectRecord *)((&D_80105AE0[arg0->slot])))->angle >= 0x50)
        {
            ((EffectRecord *)((&D_80105AE0[arg0->slot])))->angle = 0;
        }
        record_3 = (void *)&D_80105AE0[arg0->slot];
        count_3 = ((EffectRecord *)(record_3))->state.bits.radius;
        if (count_3 < limits[1])
        {
            ((EffectRecord *)(record_3))->state.word = (((EffectRecord *)(record_3))->state.word & ~0x3FF) | ((count_3 + 2) & 0x3FF);
        }
        break;
    case 4:
        var_s0 = 0;
        do
        {
            temp_a0 = var_s0 * 4;
            vec[0] = arg0->position[0] + (D_80105AE0[arg0->slot].offsets[var_s0][0] << 8);
            vec[1] = arg0->position[1];
            vec[2] = arg0->position[2] + (D_80105AE0[arg0->slot].offsets[var_s0][1] << 8);
            if (var_s2 != 0)
            {
                var_s6 = func_8009E66C(temp_s4, var_s6, &vec[0], temp_s3);
            }
            var_s0 += 1;
        } while (var_s0 < 3);
        temp_a1_3 = ((EffectRecord *)((&D_80105AE0[arg0->slot])))->state.bits.radius;
        if (temp_a1_3 < limits[1])
        {
            ((EffectRecord *)((&D_80105AE0[arg0->slot])))->state.word = (s32) (((s32) ((EffectRecord *)((&D_80105AE0[arg0->slot])))->state.word & ~0x3FF) | ((temp_a1_3 + 2) & 0x3FF));
        }
        ((EffectRecord *)(&D_80105AE0[arg0->slot]))->angle -= 0x80;
        break;
    case 5:
        vec[0] = arg0->position[0] + (((EffectRecord *)(((void *)&D_80105AE0[arg0->slot])))->offsets[0][0] << 8);
        vec[1] = arg0->position[1];
        vec[2] = arg0->position[2] + (((EffectRecord *)(((void *)&D_80105AE0[arg0->slot])))->offsets[0][1] << 8);
        if (var_s2 != 0)
        {
            var_s6 = func_8009E66C(temp_s4, var_s6, &vec[0], temp_s3);
        }
        D_80105AE0[arg0->slot].angle -= 0x80;
        temp_a1_4 = ((EffectRecord *)((&D_80105AE0[arg0->slot])))->state.bits.radius;
        if (temp_a1_4 < limits[1])
        {
            ((EffectRecord *)((&D_80105AE0[arg0->slot])))->state.word = (s32) (((s32) ((EffectRecord *)((&D_80105AE0[arg0->slot])))->state.word & ~0x3FF) | ((temp_a1_4 + 2) & 0x3FF));
            if (arg0->flags & 0x80)
            {
                ((EffectRecord *)((&D_80105AE0[arg0->slot])))->offsets[0][0] = (u16) (((EffectRecord *)((&D_80105AE0[arg0->slot])))->offsets[0][0] + 2);
            }
            else
            {
                ((EffectRecord *)((&D_80105AE0[arg0->slot])))->offsets[0][0] = (u16) (((EffectRecord *)((&D_80105AE0[arg0->slot])))->offsets[0][0] - 2);
            }
        }
        break;
    case 6:
        vec[0] = arg0->position[0] + (((EffectRecord *)(((void *)&D_80105AE0[arg0->slot])))->offsets[0][0] << 8);
        vec[1] = arg0->position[1];
        vec[2] = arg0->position[2] + (((EffectRecord *)(((void *)&D_80105AE0[arg0->slot])))->offsets[0][1] << 8);
        if (var_s2 != 0)
        {
            var_s6 = func_8009E66C(temp_s4, var_s6, &vec[0], temp_s3);
        }
        D_80105AE0[arg0->slot].angle -= 0x80;
        temp_a1_5 = ((EffectRecord *)((&D_80105AE0[arg0->slot])))->state.bits.radius;
        if (temp_a1_5 < limits[1])
        {
            ((EffectRecord *)((&D_80105AE0[arg0->slot])))->state.word = (s32) (((s32) ((EffectRecord *)((&D_80105AE0[arg0->slot])))->state.word & ~0x3FF) | ((temp_a1_5 + 1) & 0x3FF));
        }
        if ((u8) arg0->slot < 2U)
        {
            temp_v1_9 = arg0->slot * 0xAE;
            if ((u8) M2C_FIELD(pad, u8 *, temp_v1_9) >= 0xFEU)
            {
                var_a0_2 = 0;
            }
            else
            {
                temp_v0_7 = M2C_FIELD((temp_v1_9 + pad), u16 *, 2);
                var_a0_2 = (temp_v0_7 << 8) | (temp_v0_7 >> 8);
            }
            temp_v1_10 = ((u32) (var_a0_2 & 0x40) >> 1) | ((var_a0_2 & 0x20) * 2) | ((u32) (var_a0_2 & 0x80) >> 3) | ((var_a0_2 & 0x10) * 8) | (var_a0_2 & 0xFF0F);
            vec[2] = 0;
            vec[1] = 0;
            vec[0] = 0;
            if (temp_v1_10 & 0x2000)
            {
                vec[0] = 0x1000;
            }
            if (temp_v1_10 & 0x8000)
            {
                vec[0] -= 0x1000;
            }
            if (temp_v1_10 & 0x4000)
            {
                vec[1] = -0x1000;
            }
            if (temp_v1_10 & 0x1000)
            {
                vec[1] += 0x1000;
            }
            temp_v1_11 = arg0->slot * 0xAE;
            if (M2C_FIELD(pad, u8 *, temp_v1_11) != 0)
            {
                vec[0] += M2C_FIELD((temp_v1_11 + pad), s16 *, 0xC) * 0x10;
                temp_v1_11 = arg0->slot * 0xAE;
                vec[1] -= M2C_FIELD((temp_v1_11 + pad), s16 *, 0xE) * 0x10;
            }
            if ((vec[0] | vec[1]) != 0)
            {
                func_8001CDAC(&vec[0], &vec[4]);
                vec[8] = arg0->position[0] + ((((EffectRecord *)(&D_80105AE0[arg0->slot]))->offsets[0][0] + (vec[4] >> 10)) << 8);
                vec[9] = arg0->position[1];
                vec[10] = arg0->position[2] + ((((EffectRecord *)(&D_80105AE0[arg0->slot]))->offsets[0][1] + (vec[5] >> 10)) << 8);
                screen[0] = 0xA0 + D_800F22A0 / 256 + vec[8] / 256;
                screen[1] = 0x70 + D_800F22A4 / 256 + vec[9] / 256 - vec[10] / 512 - D_800F22A8 / 512;
                if ((screen[0] > 0 || vec[4] > 0) &&
                    (screen[1] > 0 || vec[5] < 0) &&
                    (screen[0] < 320 || vec[4] < 0) &&
                    (screen[1] < 224 || vec[5] > 0))
                {
                    temp_v0_8 = (void *)&D_80105AE0[arg0->slot];
                    ((EffectRecord *)(temp_v0_8))->offsets[0][0] = (u16) (((EffectRecord *)(temp_v0_8))->offsets[0][0] + (vec[4] >> 0xA));
                    temp_v0_9 = (void *)&D_80105AE0[arg0->slot];
                    ((EffectRecord *)(temp_v0_9))->offsets[0][1] = (u16) (((EffectRecord *)(temp_v0_9))->offsets[0][1] + (vec[5] >> 0xA));
                }
            }
        }
        break;
    }
    M2C_FIELD(D_800F2288, s32 *, 0x40B8) = var_s6;
}

#undef M2C_FIELD

/* ---- func_8009E66C ---- */

#include "sdk/libgte.h"
#include "sdk/libgpu.h"

#define PROJECT_POINT(_poly, _vert, _tmp) \
    (_poly)->x##_vert = (s16)(0xA0 + D_800F22A0 / 0x100 + (_tmp).vx / 0x100); \
    (_poly)->y##_vert = (s16)(0x70 + D_800F22A4 / 0x100 + (_tmp).vy / 0x100 - (_tmp).vz / 0x200 - D_800F22A8 / 0x200)

#define POLY_AT(_off) ((POLY_G4 *)(primbuf + (_off)))

#define ADD_DEPTH_ADVANCE(_depth, _expr, _type) \
    if ((_depth) < 0) { \
        addPrim(&base[0], (_type *)primbuf); \
        primbuf += sizeof(_type); \
    } else if ((_depth) >= 0x1000) { \
        addPrim(&base[0xFFF], (_type *)primbuf); \
        primbuf += sizeof(_type); \
    } else { \
        addPrim(&base[(_expr)], (_type *)primbuf); \
        primbuf += sizeof(_type); \
    }

u8 *func_8009E66C(s32 *base, u8 *arg1, VECTOR *pos, s32 radius)
{
    extern u16 D_801178D8;
    VECTOR v[6];
    SVECTOR rot;
    MATRIX m0;
    MATRIX m1;
    s32 i;
    s32 x;
    s32 y;
    s32 initial_x;
    s32 initial_y;
    s32 depth;
    u32 center;
    u8 *primbuf;

    primbuf = arg1;

    rot.vx = 0;
    rot.vz = 0;
    rot.vy = D_801178D8;
    RotMatrix_gte(&rot, &m0);

    initial_x = (rcos(0) >> 4) * radius;
    initial_y = (rsin(0) >> 4) * radius;

    rot.vx = 0;
    rot.vz = 0;
    rot.vy = D_801178D8 + 0x180;
    RotMatrix_gte(&rot, &m1);

    v[0].vx = initial_x;
    v[0].vy = initial_y;
    v[0].vz = 0;
    ApplyMatrixLV(&m0, &v[0], &v[1]);
    v[0].vx = initial_x;
    v[0].vy = initial_y;
    v[0].vz = 0;
    ApplyMatrixLV(&m1, &v[0], &v[4]);

    v[0].vx = pos->vx + v[1].vx;
    v[0].vy = pos->vy + v[1].vy;
    v[0].vz = pos->vz + v[1].vz;
    PROJECT_POINT(POLY_AT(0x0), 0, v[0]);

    v[0].vx = pos->vx - v[1].vx;
    v[0].vy = pos->vy + v[1].vy;
    v[0].vz = pos->vz - v[1].vz;
    PROJECT_POINT(POLY_AT(0x24), 0, v[0]);

    v[0].vx = pos->vx - v[1].vz;
    v[0].vy = pos->vy + v[1].vy;
    v[0].vz = pos->vz + v[1].vx;
    PROJECT_POINT(POLY_AT(0x48), 0, v[0]);

    v[0].vx = pos->vx + v[1].vz;
    v[0].vy = pos->vy + v[1].vy;
    v[0].vz = pos->vz - v[1].vx;
    PROJECT_POINT(POLY_AT(0x6C), 0, v[0]);

    v[0].vx = pos->vx;
    v[0].vy = pos->vy;
    v[0].vz = pos->vz;
    PROJECT_POINT(POLY_AT(0x0), 1, v[0]);
    center = *(u32 *)&POLY_AT(0)->x1;
    *(u32 *)&POLY_AT(0)->x3 = center;
    *(u32 *)&POLY_AT(0x6C)->x1 = center;
    *(u32 *)&POLY_AT(0x6C)->x3 = center;
    *(u32 *)&POLY_AT(0x48)->x1 = center;
    *(u32 *)&POLY_AT(0x48)->x3 = center;
    *(u32 *)&POLY_AT(0x24)->x1 = center;
    *(u32 *)&POLY_AT(0x24)->x3 = center;

    v[0].vx = pos->vx + v[4].vx;
    v[0].vy = pos->vy + v[4].vy;
    v[0].vz = pos->vz + v[4].vz;
    PROJECT_POINT(POLY_AT(0x0), 2, v[0]);

    v[0].vx = pos->vx - v[4].vx;
    v[0].vy = pos->vy + v[4].vy;
    v[0].vz = pos->vz - v[4].vz;
    PROJECT_POINT(POLY_AT(0x24), 2, v[0]);

    v[0].vx = pos->vx - v[4].vz;
    v[0].vy = pos->vy + v[4].vy;
    v[0].vz = pos->vz + v[4].vx;
    PROJECT_POINT(POLY_AT(0x48), 2, v[0]);

    v[0].vx = pos->vx + v[4].vz;
    v[0].vy = pos->vy + v[4].vy;
    v[0].vz = pos->vz - v[4].vx;
    PROJECT_POINT(POLY_AT(0x6C), 2, v[0]);

    *(u32 *)&POLY_AT(0x0)->r0 = ((0xA0 - (v[1].vz >> 8)) << 16) & 0xFFFFFF;
    *(u32 *)&POLY_AT(0x24)->r0 = (((v[1].vz >> 8) + 0xA0) << 8) & 0xFF00;
    *(u32 *)&POLY_AT(0x48)->r0 = ((0xA0 - (v[1].vx >> 8)) << 8) & 0xFF00;
    *(u32 *)&POLY_AT(0x6C)->r0 = (((v[1].vx >> 8) + 0xA0) << 8) & 0xFF00;

    *(u32 *)&POLY_AT(0x6C)->r1 = 0xA000;
    *(u32 *)&POLY_AT(0x48)->r1 = 0xA000;
    *(u32 *)&POLY_AT(0x24)->r1 = 0xA000;
    *(u32 *)&POLY_AT(0x0)->r1 = 0xA000;
    *(u32 *)&POLY_AT(0x24)->r3 = 0;
    *(u32 *)&POLY_AT(0x24)->r2 = 0;
    *(u32 *)&POLY_AT(0x48)->r3 = 0;
    *(u32 *)&POLY_AT(0x48)->r2 = 0;
    *(u32 *)&POLY_AT(0x6C)->r3 = 0;
    *(u32 *)&POLY_AT(0x6C)->r2 = 0;
    *(u32 *)&POLY_AT(0x0)->r3 = 0;
    *(u32 *)&POLY_AT(0x0)->r2 = 0;

    SetPolyG4(POLY_AT(0x0));
    SetPolyG4(POLY_AT(0x24));
    SetPolyG4(POLY_AT(0x48));
    SetPolyG4(POLY_AT(0x6C));
    setSemiTrans(POLY_AT(0x0), 1);
    setSemiTrans(POLY_AT(0x24), 1);
    setSemiTrans(POLY_AT(0x48), 1);
    setSemiTrans(POLY_AT(0x6C), 1);

    depth = pos->vz >> 7;
    ADD_DEPTH_ADVANCE(depth, pos->vz >> 7, POLY_G4);
    depth = pos->vz >> 7;
    ADD_DEPTH_ADVANCE(depth, pos->vz >> 7, POLY_G4);
    depth = pos->vz >> 7;
    ADD_DEPTH_ADVANCE(depth, pos->vz >> 7, POLY_G4);
    depth = pos->vz >> 7;
    ADD_DEPTH_ADVANCE(depth, pos->vz >> 7, POLY_G4);

    setDrawTPage((DR_TPAGE *)primbuf, 0, 0, 0x25);
    depth = (pos->vz + v[1].vz) >> 7;
    ADD_DEPTH_ADVANCE(depth, (pos->vz + v[1].vz) >> 7, DR_TPAGE);

    i = 1;
    do {
        x = (rcos(i << 8) >> 4) * radius;
        y = -(rsin(i << 8) >> 4) * radius;
        v[0].vx = x;
        v[0].vy = y;
        v[0].vz = 0;
        ApplyMatrixLV(&m0, &v[0], &v[2]);
        v[0].vx = x;
        v[0].vy = y;
        v[0].vz = 0;
        ApplyMatrixLV(&m1, &v[0], &v[5]);

        v[0].vx = pos->vx + v[1].vx;
        v[0].vy = pos->vy + v[1].vy;
        v[0].vz = pos->vz + v[1].vz;
        PROJECT_POINT(POLY_AT(0), 0, v[0]);
        v[0].vx = pos->vx + v[2].vx;
        v[0].vy = pos->vy + v[2].vy;
        v[0].vz = pos->vz + v[2].vz;
        PROJECT_POINT(POLY_AT(0), 1, v[0]);
        v[0].vx = pos->vx + v[4].vx;
        v[0].vy = pos->vy + v[4].vy;
        v[0].vz = pos->vz + v[4].vz;
        PROJECT_POINT(POLY_AT(0), 2, v[0]);
        v[0].vx = pos->vx + v[5].vx;
        v[0].vy = pos->vy + v[5].vy;
        v[0].vz = pos->vz + v[5].vz;
        PROJECT_POINT(POLY_AT(0), 3, v[0]);

        *(u32 *)&POLY_AT(0)->r0 = ((0xA0 - (v[1].vz >> 8)) << 16) & 0xFFFFFF;
        *(u32 *)&POLY_AT(0)->r1 = ((0xA0 - (v[2].vz >> 8)) << 16) & 0xFFFFFF;
        *(u32 *)&POLY_AT(0)->r2 = 0;
        *(u32 *)&POLY_AT(0)->r3 = 0;
        SetPolyG4(POLY_AT(0));
        setSemiTrans(POLY_AT(0), 1);
        depth = (pos->vz + v[1].vz) >> 7;
        ADD_DEPTH_ADVANCE(depth, (pos->vz + v[1].vz) >> 7, POLY_G4);

        setDrawTPage((DR_TPAGE *)primbuf, 0, 0, 0x25);
        depth = (pos->vz + v[1].vz) >> 7;
        ADD_DEPTH_ADVANCE(depth, (pos->vz + v[1].vz) >> 7, DR_TPAGE);

        v[0].vx = pos->vx - v[1].vz;
        v[0].vy = pos->vy + v[1].vy;
        v[0].vz = pos->vz + v[1].vx;
        PROJECT_POINT(POLY_AT(0), 0, v[0]);
        v[0].vx = pos->vx - v[2].vz;
        v[0].vy = pos->vy + v[2].vy;
        v[0].vz = pos->vz + v[2].vx;
        PROJECT_POINT(POLY_AT(0), 1, v[0]);
        v[0].vx = pos->vx - v[4].vz;
        v[0].vy = pos->vy + v[4].vy;
        v[0].vz = pos->vz + v[4].vx;
        PROJECT_POINT(POLY_AT(0), 2, v[0]);
        v[0].vx = pos->vx - v[5].vz;
        v[0].vy = pos->vy + v[5].vy;
        v[0].vz = pos->vz + v[5].vx;
        PROJECT_POINT(POLY_AT(0), 3, v[0]);

        *(u32 *)&POLY_AT(0)->r0 = ((0xA0 - (v[1].vz >> 8)) << 16) & 0xFFFFFF;
        *(u32 *)&POLY_AT(0)->r1 = ((0xA0 - (v[2].vz >> 8)) << 16) & 0xFFFFFF;
        *(u32 *)&POLY_AT(0)->r2 = 0;
        *(u32 *)&POLY_AT(0)->r3 = 0;
        SetPolyG4(POLY_AT(0));
        setSemiTrans(POLY_AT(0), 1);
        depth = (pos->vz + v[1].vz) >> 7;
        ADD_DEPTH_ADVANCE(depth, (pos->vz + v[1].vz) >> 7, POLY_G4);

        setDrawTPage((DR_TPAGE *)primbuf, 0, 0, 0x25);
        depth = (pos->vz + v[1].vz) >> 7;
        ADD_DEPTH_ADVANCE(depth, (pos->vz + v[1].vz) >> 7, DR_TPAGE);

        i++;
        v[1].vx = v[2].vx;
        v[1].vy = v[2].vy;
        v[1].vz = v[2].vz;
        v[4].vx = v[5].vx;
        v[4].vy = v[5].vy;
        v[4].vz = v[5].vz;
    } while (i < 9);

    return primbuf;
}

#undef PROJECT_POINT
#undef POLY_AT
#undef ADD_DEPTH_ADVANCE

/* ---- func_8009FE54 ---- */

#define PROJECT_POINT(_poly, _vert, _tmp) \
    (_poly)->x##_vert = (s16)(0xA0 + D_800F22A0 / 0x100 + (_tmp).vx / 0x100); \
    (_poly)->y##_vert = (s16)(0x70 + D_800F22A4 / 0x100 + (_tmp).vy / 0x100 - (_tmp).vz / 0x200 - D_800F22A8 / 0x200)

#define POLY_AT(_off) ((POLY_G4 *)(primbuf + (_off)))

#define ADD_DEPTH_ADVANCE(_depth, _expr, _type) \
    if ((_depth) < 0) \
    { \
        addPrim(&base[0], (_type *)primbuf); \
        primbuf += sizeof(_type); \
    } \
    else if ((_depth) >= 0x1000) \
    { \
        addPrim(&base[0xFFF], (_type *)primbuf); \
        primbuf += sizeof(_type); \
    } \
    else \
    { \
        addPrim(&base[(_expr)], (_type *)primbuf); \
        primbuf += sizeof(_type); \
    }

/**
 * @brief Append four rotating strips of shaded, translucent quads.
 * @param base Ordering table with 4096 depth buckets.
 * @param arg1 Next free primitive-buffer byte.
 * @param pos World-space center in fixed-point coordinates.
 * @param radius Radius used to construct the strips.
 * @return First free byte after the appended primitives.
 * @note Matches 100% with gcc272_cdk: 814 instructions, 3256 bytes.
 * @note Keep the vector array and unused matrix to preserve the stack layout.
 */
u8 *func_8009FE54(s32 *base, u8 *arg1, VECTOR *pos, s32 radius)
{
    extern s32 D_801178D8;
    VECTOR v[6];
    SVECTOR rot;
    MATRIX m0;
    MATRIX m1;
    VECTOR p0;
    VECTOR p1;
    s32 distance;
    s32 i;
    s32 j;
    s32 x;
    s32 y;
    s32 inner_x;
    s32 inner_y;
    s32 angle;
    s32 depth;
    u8 *primbuf;

    primbuf = arg1;
    i = 0;
    do
    {
        distance = (radius >> 1) + 0x40;
        p0.vx = pos->vx;
        p0.vy = pos->vy;
        p0.vz = pos->vz;
        p1.vx = pos->vx;
        p1.vy = pos->vy;
        p1.vz = pos->vz;
        angle = i << 10;
        x = (rcos(angle - D_801178D8) >> 4) * distance;
        y = (rsin(angle - D_801178D8) >> 4) * distance;
        p0.vx += x;
        p0.vz += y;
        x = (rcos(angle - D_801178D8 - 0x100) >> 4) * distance;
        y = (rsin(angle - D_801178D8 - 0x100) >> 4) * distance;
        p1.vx += x;
        p1.vz += y;
        rot.vx = 0;
        rot.vz = 0;
        rot.vy = D_801178D8 + angle;
        RotMatrix_gte(&rot, &m0);
        x = ((rcos(0) >> 4) * radius) >> 1;
        y = ((rsin(0) >> 4) * radius) >> 1;
        v[0].vx = x;
        v[0].vy = y;
        v[0].vz = 0;
        ApplyMatrixLV(&m0, &v[0], &v[1]);
        v[0].vx = -radius * 0x80;
        v[0].vy = 0;
        v[0].vz = 0;
        ApplyMatrixLV(&m0, &v[0], &v[2]);
        v[0].vx = p0.vx + v[1].vx;
        v[0].vy = p0.vy + v[1].vy;
        v[0].vz = p0.vz + v[1].vz;
        PROJECT_POINT(POLY_AT(0), 0, v[0]);
        v[0].vx = p0.vx + v[2].vx;
        v[0].vy = p0.vy + v[2].vy;
        v[0].vz = p0.vz + v[2].vz;
        PROJECT_POINT(POLY_AT(0), 1, v[0]);
        v[0].vx = p1.vx + v[1].vx;
        v[0].vy = p1.vy + v[1].vy;
        v[0].vz = p1.vz + v[1].vz;
        PROJECT_POINT(POLY_AT(0), 2, v[0]);
        v[0].vx = p1.vx + v[2].vx;
        v[0].vy = p1.vy + v[2].vy;
        v[0].vz = p1.vz + v[2].vz;
        PROJECT_POINT(POLY_AT(0), 3, v[0]);
        *(u32 *)&POLY_AT(0)->r0 = ((0xA0 - (v[1].vz >> 8)) << 8) & 0xFF00;
        *(u32 *)&POLY_AT(0)->r1 = ((0xA0 - (v[2].vz >> 8)) << 8) & 0xFF00;
        *(u32 *)&POLY_AT(0)->r2 = 0;
        *(u32 *)&POLY_AT(0)->r3 = 0;
        SetPolyG4(POLY_AT(0));
        setSemiTrans(POLY_AT(0), 1);
        depth = (p0.vz + v[1].vz) >> 7;
        ADD_DEPTH_ADVANCE(depth, (p0.vz + v[1].vz) >> 7, POLY_G4);
        setDrawTPage((DR_TPAGE *)primbuf, 0, 0, 0x25);
        depth = (p0.vz + v[1].vz) >> 7;
        ADD_DEPTH_ADVANCE(depth, (p0.vz + v[1].vz) >> 7, DR_TPAGE);
        j = 1;
        do
        {
            inner_x = ((rcos(j << 8) >> 4) * radius) >> 1;
            inner_y = (-(rsin(j << 8) >> 4) * radius) >> 1;
            v[0].vx = inner_x;
            v[0].vy = inner_y;
            v[0].vz = 0;
            ApplyMatrixLV(&m0, &v[0], &v[2]);
            v[0].vx = p0.vx + v[1].vx;
            v[0].vy = p0.vy + v[1].vy;
            v[0].vz = p0.vz + v[1].vz;
            PROJECT_POINT(POLY_AT(0), 0, v[0]);
            v[0].vx = p0.vx + v[2].vx;
            v[0].vy = p0.vy + v[2].vy;
            v[0].vz = p0.vz + v[2].vz;
            PROJECT_POINT(POLY_AT(0), 1, v[0]);
            v[0].vx = p1.vx + v[1].vx;
            v[0].vy = p1.vy + v[1].vy;
            v[0].vz = p1.vz + v[1].vz;
            PROJECT_POINT(POLY_AT(0), 2, v[0]);
            v[0].vx = p1.vx + v[2].vx;
            v[0].vy = p1.vy + v[2].vy;
            v[0].vz = p1.vz + v[2].vz;
            PROJECT_POINT(POLY_AT(0), 3, v[0]);
            *(u32 *)&POLY_AT(0)->r0 = ((0xA0 - (v[1].vz >> 8)) << 16) & 0xFFFFFF;
            *(u32 *)&POLY_AT(0)->r1 = ((0xA0 - (v[2].vz >> 8)) << 16) & 0xFFFFFF;
            *(u32 *)&POLY_AT(0)->r2 = 0;
            *(u32 *)&POLY_AT(0)->r3 = 0;
            SetPolyG4(POLY_AT(0));
            setSemiTrans(POLY_AT(0), 1);
            depth = (p0.vz + v[1].vz) >> 7;
            ADD_DEPTH_ADVANCE(depth, (p0.vz + v[1].vz) >> 7, POLY_G4);
            setDrawTPage((DR_TPAGE *)primbuf, 0, 0, 0x25);
            depth = (p0.vz + v[1].vz) >> 7;
            ADD_DEPTH_ADVANCE(depth, (p0.vz + v[1].vz) >> 7, DR_TPAGE);
            j++;
            v[1].vx = v[2].vx;
            v[1].vy = v[2].vy;
            v[1].vz = v[2].vz;
        } while (j < 9);
        i++;
    } while (i < 4);
    return primbuf;
}

#undef PROJECT_POINT
#undef POLY_AT
#undef ADD_DEPTH_ADVANCE

/* ---- func_800A0B0C ---- */

/**
 * @brief Draw animated curved quad strips extending from a fixed-point position.
 * @param ordering_table Ordering table with 0x1000 depth entries.
 * @param primitive_buffer Destination for the generated GPU packets.
 * @param position World position in signed fixed-point coordinates.
 * @param extent Maximum horizontal extent, tested after each completed strip.
 * @param forward Nonzero extends toward positive X; zero extends toward negative X.
 * @return First byte after the emitted primitives and draw-page command.
 * @note Emits at least one strip and at most four, with nine quads per strip.
 * @note Unused local workspace storage preserves the recovered frame layout.
 * @note WIP: 80.566540% standalone; unchanged instruction stream in this TU
 *       (the merged diff drops one exact row solely because an intra-object
 *       j func_800A0B0C+0x2b8 renders at a shifted address - a position
 *       artifact, byte-identical and links identically).
 */
u8 *func_800A0B0C(s32 *ordering_table, u8 *primitive_buffer, VECTOR *position, s32 extent, s32 forward)
{
    extern s32 D_801178D8;
    s32 step;
    s32 strip_index;
    s32 first_xy;
    s32 second_xy;
    /* Only element 1 is accessed; other recovered workspace roles are unknown. */
    volatile VECTOR projection_workspace[13];
    SVECTOR reserved_workspace;
    s32 *temp_v0;
    s32 *temp_v1_4;
    s32 *temp_v1_8;
    u8 *next_primitive;
    u8 *primitive;
    s32 temp_a0;
    s32 temp_a0_2;
    s32 temp_a1;
    s32 temp_a1_2;
    s32 temp_t0;
    s32 temp_t0_2;
    s32 temp_v1;
    s32 temp_v1_2;
    s32 temp_v1_3;
    s32 temp_v1_5;
    s32 temp_v1_6;
    s32 temp_v1_7;
    s32 temp_v1_9;
    s32 var_a0;
    s32 var_a0_2;
    s32 var_a0_3;
    s32 var_a0_4;
    s32 var_a0_5;
    s32 var_a0_6;
    s32 var_a0_7;
    s32 var_a0_8;
    s32 angle;
    s32 offset;
    s32 var_v0;
    s32 var_v0_10;
    s32 var_v0_11;
    s32 var_v0_12;
    s32 var_v0_2;
    s32 var_v0_3;
    s32 var_v0_4;
    s32 var_v0_5;
    s32 var_v0_6;
    s32 var_v0_7;
    s32 var_v0_8;
    s32 var_v0_9;
    s32 var_v1;
    s32 var_v1_2;
    s32 var_v1_3;
    s32 var_v1_4;
    s32 var_v1_5;
    s32 var_v1_6;
    s32 var_v1_7;
    s32 var_v1_8;
    u8 *strip_code;
    u8 *outer_code;

    primitive = primitive_buffer;
    outer_code = primitive + 7;
    strip_index = 0;
    offset = (D_801178D8 % 40) << 8;
next_strip:
    /* Save both initial packed XY values to close the strip after eight steps. */
    if (forward != 0)
    {
        var_v0 = position->vx + offset;
    }
    else
    {
        var_v0 = position->vx - offset;
    }
    projection_workspace[1].vx = var_v0;
    projection_workspace[1].vy = position->vy;
    projection_workspace[1].vz = position->vz + 0x2000;
    *(s16 *)(outer_code + 1) = 160 + projection_workspace[1].vx / 256 + D_800F22A0 / 256;
    *(s16 *)(outer_code + 3) = 112 + projection_workspace[1].vy / 256 + D_800F22A4 / 256 - projection_workspace[1].vz / 512 - D_800F22A8 / 512;
    first_xy = (s32) *(s32 *)(outer_code + (1));
    if (forward != 0)
    {
        var_v0_4 = position->vx + offset + 0x1400;
    }
    else
    {
        var_v0_4 = (position->vx - offset) - 0x1400;
    }
    projection_workspace[1].vx = var_v0_4;
    projection_workspace[1].vy = position->vy;
    projection_workspace[1].vz = position->vz + 0x2000;
    *(s16 *)(outer_code + 9) = 160 + projection_workspace[1].vx / 256 + D_800F22A0 / 256;
    *(s16 *)(outer_code + 11) = 112 + projection_workspace[1].vy / 256 + D_800F22A4 / 256 - projection_workspace[1].vz / 512 - D_800F22A8 / 512;
    angle = 0x100;
    strip_code = primitive + 7;
    step = 1;
    second_xy = (s32) *(s32 *)(outer_code + (9));
    do
    {
        if (forward != 0)
        {
            var_v0_7 = position->vx + offset + angle;
        }
        else
        {
            var_v0_7 = (position->vx - offset) - angle;
        }
        projection_workspace[1].vx = var_v0_7;
        projection_workspace[1].vy = position->vy - (rsin(angle) * 2);
        projection_workspace[1].vz = position->vz + (rcos(angle) * 2);
        *(s16 *)(strip_code + 17) = 160 + projection_workspace[1].vx / 256 + D_800F22A0 / 256;
        *(s16 *)(strip_code + 19) = 112 + projection_workspace[1].vy / 256 + D_800F22A4 / 256 - projection_workspace[1].vz / 512 - D_800F22A8 / 512;
        *(s32 *)(strip_code + (37)) = (s32) *(s32 *)(strip_code + (17));
        if (forward != 0)
        {
            var_v0_10 = position->vx + offset + angle + 0x1400;
        }
        else
        {
            var_v0_10 = ((position->vx - offset) - angle) - 0x1400;
        }
        projection_workspace[1].vx = var_v0_10;
        projection_workspace[1].vy = position->vy - (rsin(angle) * 2);
        projection_workspace[1].vz = position->vz + (rcos(angle) * 2);
        *(s16 *)(strip_code + 25) = 160 + projection_workspace[1].vx / 256 + D_800F22A0 / 256;
        *(s16 *)(strip_code + 27) = 112 + projection_workspace[1].vy / 256 + D_800F22A4 / 256 - projection_workspace[1].vz / 512 - D_800F22A8 / 512;
        /* Fade the curved strip from black to blue. */
        *(s32 *)(strip_code + (-3)) = 0;
        *(s32 *)(strip_code + (5)) = 0xA00000;
        *(s32 *)(strip_code + (13)) = 0;
        *(s32 *)(strip_code + (21)) = 0xA00000;
        *(s32 *)(strip_code + (45)) = (s32) *(s32 *)(strip_code + (25));
        SetPolyG4((POLY_G4 *)primitive);
        *(u8 *)(strip_code + (0)) = (u8) (*(u8 *)(strip_code + (0)) | 2);
        temp_v1 = (s32) position->vz >> 7;
        if (temp_v1 < 0)
        {
            strip_code += 0x24;
            outer_code += 0x24;
            temp_v1_2 = (s32) primitive & 0xFFFFFF;
            *(s32 *)primitive = (*(s32 *)primitive & 0xFF000000) | (ordering_table[0] & 0xFFFFFF);
            primitive += 0x24;
            ordering_table[0] = (s32) ((ordering_table[0] & 0xFF000000) | temp_v1_2);
        }
        else if (temp_v1 >= 0x1000)
        {
            strip_code += 0x24;
            outer_code += 0x24;
            temp_v1_3 = (s32) primitive & 0xFFFFFF;
            *(s32 *)primitive = (*(s32 *)primitive & 0xFF000000) | (ordering_table[0xFFF] & 0xFFFFFF);
            primitive += 0x24;
            ordering_table[0xFFF] = (s32) ((ordering_table[0xFFF] & 0xFF000000) | temp_v1_3);
        }
        else
        {
            strip_code += 0x24;
            outer_code += 0x24;
            *(s32 *)primitive = (*(s32 *)primitive & 0xFF000000) | (ordering_table[temp_v1] & 0xFFFFFF);
            temp_a0 = (s32) primitive & 0xFFFFFF;
            temp_v1_4 = &ordering_table[(s32) position->vz >> 7];
            primitive += 0x24;
            *temp_v1_4 = (*temp_v1_4 & 0xFF000000) | temp_a0;
        }
        angle += 0x100;
        temp_t0 = step + 1;
        step = temp_t0;
        } while (temp_t0 < 9);
        *(s32 *)(outer_code + (17)) = first_xy;
        *(s32 *)(outer_code + (25)) = second_xy;
        *(s32 *)(outer_code + (-3)) = 0;
        *(s32 *)(outer_code + (5)) = 0xA000;
        *(s32 *)(outer_code + (13)) = 0;
        *(s32 *)(outer_code + (21)) = 0xA000;
        SetPolyG4((POLY_G4 *)primitive);
        *(u8 *)(outer_code + (0)) = (u8) (*(u8 *)(outer_code + (0)) | 2);
        temp_v1_5 = (s32) position->vz >> 7;
        if (temp_v1_5 < 0)
        {
            outer_code += 0x24;
            *(s32 *)primitive = (*(s32 *)primitive & 0xFF000000) | (ordering_table[0] & 0xFFFFFF);
            temp_v1_6 = (s32) primitive & 0xFFFFFF;
            primitive += 0x24;
            ordering_table[0] = (s32) ((ordering_table[0] & 0xFF000000) | temp_v1_6);
        }
        else if (temp_v1_5 >= 0x1000)
        {
            outer_code += 0x24;
            *(s32 *)primitive = (*(s32 *)primitive & 0xFF000000) | (ordering_table[0xFFF] & 0xFFFFFF);
            temp_v1_7 = (s32) primitive & 0xFFFFFF;
            primitive += 0x24;
            ordering_table[0xFFF] = (s32) ((ordering_table[0xFFF] & 0xFF000000) | temp_v1_7);
        }
        else
        {
            outer_code += 0x24;
            *(s32 *)primitive = (*(s32 *)primitive & 0xFF000000) | (ordering_table[temp_v1_5] & 0xFFFFFF);
            temp_a0_2 = (s32) primitive & 0xFFFFFF;
            temp_v1_8 = &ordering_table[(s32) position->vz >> 7];
            primitive += 0x24;
            *temp_v1_8 = (*temp_v1_8 & 0xFF000000) | temp_a0_2;
        }
        offset += 0x2800;
        if (offset < (extent << 8) && ++strip_index < 4)
        {
            goto next_strip;
        }
        /* Draw-page command follows all polygons at the same clamped depth. */
        *(u8 *)(primitive + (3)) = 1;
        *(s32 *)(primitive + (4)) = 0xE1000025;
        temp_v1_9 = (s32) position->vz >> 7;
        if (temp_v1_9 < 0)
        {
            *(s32 *)(primitive + (0)) = (*(s32 *)(primitive + (0)) & 0xFF000000) | (ordering_table[0] & 0xFFFFFF);
            next_primitive = primitive + 8;
            ordering_table[0] = (s32) ((ordering_table[0] & 0xFF000000) | ((s32) primitive & 0xFFFFFF));
        }
        else if (temp_v1_9 >= 0x1000)
        {
            *(s32 *)(primitive + (0)) = (*(s32 *)(primitive + (0)) & 0xFF000000) | (ordering_table[0xFFF] & 0xFFFFFF);
            next_primitive = primitive + 8;
            ordering_table[0xFFF] = (s32) ((ordering_table[0xFFF] & 0xFF000000) | ((s32) primitive & 0xFFFFFF));
        }
        else
        {
            *(s32 *)(primitive + (0)) = (*(s32 *)(primitive + (0)) & 0xFF000000) | (ordering_table[temp_v1_9] & 0xFFFFFF);
            temp_v0 = &ordering_table[(s32) position->vz >> 7];
            next_primitive = primitive + 8;
            *temp_v0 = (*temp_v0 & 0xFF000000) | ((s32) primitive & 0xFFFFFF);
        }
        return next_primitive;
    }

/* ---- func_800A1344 ---- */

#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

/**
 * @brief Vector and matrix scratch area used by the GTE strip renderer.
 * @note The input is at workspace+0x30 and the active matrix at+0x78;
 *       unused vector/matrix slots preserve the original scratch layout.
 */
typedef struct
{
    VECTOR rotated;
    VECTOR world;
    VECTOR unused;
    SVECTOR input;
    MATRIX matrices[5];
} FieldStripWorkspace;

#define ADD_PACKET(depth)                                                                                                                                      \
    (*(s32*)current_packet = (*(s32*)current_packet & tag_mask) | (ordering_table[depth] & addr_mask),                                                         \
     ordering_table[depth] = (ordering_table[depth] & tag_mask) | ((s32)current_packet & addr_mask))

/**
 * @brief Draw four rotating strips of Gouraud-shaded quads around a position.
 * @param ordering_table Depth ordering table with 4096 entries.
 * @param packet First free primitive packet.
 * @param position Fixed-point world-space origin of the effect.
 * @param slope Direction parameter converted into a Y rotation angle.
 * @param facing Selects addition or subtraction of the rotated X offset.
 * @return First free packet following all quads and the draw-page command.
 * @note Full word copies carry packed X/Y pairs into the next segment and
 *       close the strip. The strip cursor points at the primitive code byte.
 * @see decomp.me (100%)
 */
u8* func_800A1344(s32* ordering_table, u8* packet, VECTOR* position, s32 slope, s32 facing)
{
    extern s32 D_801178D8;
    s32 screen_x;
    s32 packet_addr;
    FieldStripWorkspace work;
    s32 strip_index;
    s32 angle;
    s32 first_xy;
    s32 second_xy;
    MATRIX* matrix;
    u8* current_packet;
    s32 segment_depth;
    s32 closing_depth;
    s32 drawpage_depth;
    s32 camera_y;
    s32 addr_mask;
    s32 tag_mask;
    s32 segment_index;
    s32 radius;
    s32 radius_sum;
    s32 first_x;
    s32 segment_outer_x;
    s32 second_x;
    s32 segment_inner_x;
    u8* segment_packet;
    u8* strip_code;

    current_packet = packet;
    angle = ratan2(slope, 0x64);
    strip_index = 0;
    matrix = &work.matrices[2];
    addr_mask = 0xFFFFFF;
    tag_mask = 0xFF000000;
    radius = D_801178D8;
    strip_code = current_packet + 7;
next_strip:
    {
        ((s32*)&work.matrices[2])[4] = 0x1000;
        ((s32*)&work.matrices[2])[2] = 0x1000;
        ((s32*)&work.matrices[2])[0] = 0x1000;
        ((s32*)&work.matrices[2])[7] = 0;
        ((s32*)&work.matrices[2])[6] = 0;
        ((s32*)&work.matrices[2])[5] = 0;
        ((s32*)&work.matrices[2])[3] = 0;
        ((s32*)&work.matrices[2])[1] = 0;
        RotMatrixY(angle, matrix);
        work.input.vx = (s16)radius;
        work.input.vy = 0;
        work.input.vz = 0;
        gte_SetRotMatrix(matrix);
        gte_ldv0(&work.input);
        gte_rtv0();
        gte_stlvnl(&work.rotated);
        if (facing != 0)
        {
            first_x = position->vx + (work.rotated.vx << 8);
        }
        else
        {
            first_x = position->vx - (work.rotated.vx << 8);
        }
        do
        {
            work.world.vx = first_x;
            work.world.vy = position->vy + (work.rotated.vy << 8);
            work.world.vz = position->vz + (work.rotated.vz << 8);
        } while (0);
        screen_x = 160 + D_800F22A0 / 256 + ((volatile VECTOR*)&work.world)->vx / 256;
        camera_y = D_800F22A4;
        *(s16*)(strip_code + 1) = screen_x;
        if (camera_y < 0)
        {
            camera_y += 255;
        }
        *(s16*)(strip_code + 3) = 112 + (camera_y >> 8) + ((volatile VECTOR*)&work.world)->vy / 256 - ((volatile VECTOR*)&work.world)->vz / 512 - D_800F22A8 / 512;
        first_xy = *(s32*)(strip_code + 0x1);
        work.input.vx = radius + 0x14;
        work.input.vy = 0;
        work.input.vz = 0;
        gte_SetRotMatrix(matrix);
        gte_ldv0(&work.input);
        gte_rtv0();
        gte_stlvnl(&work.rotated);
        if (facing != 0)
        {
            second_x = position->vx + (work.rotated.vx << 8);
        }
        else
        {
            second_x = position->vx - (work.rotated.vx << 8);
        }
        do
        {
            work.world.vx = second_x;
            work.world.vy = position->vy + (work.rotated.vy << 8);
            work.world.vz = position->vz + (work.rotated.vz << 8);
        } while (0);
        screen_x = 160 + D_800F22A0 / 256 + ((volatile VECTOR*)&work.world)->vx / 256;
        camera_y = D_800F22A4;
        *(s16*)(strip_code + 9) = screen_x;
        if (camera_y < 0)
        {
            camera_y += 255;
        }
        *(s16*)(strip_code + 11) = 112 + (camera_y >> 8) + ((volatile VECTOR*)&work.world)->vy / 256 - ((volatile VECTOR*)&work.world)->vz / 512 - D_800F22A8 / 512;
        segment_index = 1;
        radius_sum = radius;
        segment_packet = current_packet;
        second_xy = *(s32*)(strip_code + 0x9);
        do
        {
            RotMatrixX(-0x100, matrix);
            work.input.vx = radius + (radius_sum >> 5);
            work.input.vy = 0;
            work.input.vz = 0;
            gte_SetRotMatrix(matrix);
            gte_ldv0(&work.input);
            gte_rtv0();
            gte_stlvnl(&work.rotated);
            if (facing != 0)
            {
                segment_inner_x = position->vx + (work.rotated.vx << 8);
            }
            else
            {
                segment_inner_x = position->vx - (work.rotated.vx << 8);
            }
            do
            {
                work.world.vx = segment_inner_x;
                work.world.vy = position->vy + (work.rotated.vy << 8);
                work.world.vz = position->vz + (work.rotated.vz << 8);
            } while (0);
            screen_x = 160 + D_800F22A0 / 256 + ((volatile VECTOR*)&work.world)->vx / 256;
            camera_y = D_800F22A4;
            *(s16*)(segment_packet + 24) = screen_x;
            if (camera_y < 0)
            {
                camera_y += 255;
            }
            *(s16*)(segment_packet + 26) =
                112 + (camera_y >> 8) + ((volatile VECTOR*)&work.world)->vy / 256 - ((volatile VECTOR*)&work.world)->vz / 512 - D_800F22A8 / 512;
            *(s32*)(segment_packet + 44) = *(s32*)(segment_packet + 24);
            work.input.vx = radius + (radius_sum >> 5) + 0x14;
            work.input.vy = 0;
            work.input.vz = 0;
            gte_SetRotMatrix(matrix);
            gte_ldv0(&work.input);
            gte_rtv0();
            gte_stlvnl(&work.rotated);
            if (facing != 0)
            {
                segment_outer_x = position->vx + (work.rotated.vx << 8);
            }
            else
            {
                segment_outer_x = position->vx - (work.rotated.vx << 8);
            }
            do
            {
                work.world.vx = segment_outer_x;
                work.world.vy = position->vy + (work.rotated.vy << 8);
                work.world.vz = position->vz + (work.rotated.vz << 8);
            } while (0);
            screen_x = 160 + D_800F22A0 / 256 + ((volatile VECTOR*)&work.world)->vx / 256;
            camera_y = D_800F22A4;
            *(s16*)(segment_packet + 32) = screen_x;
            if (camera_y < 0)
            {
                camera_y += 255;
            }
            *(s16*)(segment_packet + 34) =
                112 + (camera_y >> 8) + ((volatile VECTOR*)&work.world)->vy / 256 - ((volatile VECTOR*)&work.world)->vz / 512 - D_800F22A8 / 512;
            *(s32*)(segment_packet + 4) = 0;
            *(s32*)(segment_packet + 12) = 0xA00000;
            *(s32*)(segment_packet + 20) = 0;
            *(s32*)(segment_packet + 28) = 0xA00000;
            *(s32*)(segment_packet + 52) = *(s32*)(segment_packet + 32);
            SetPolyG4((POLY_G4*)current_packet);
            do
            {
                *(u8*)(segment_packet + 7) = (u8)(*(u8*)(segment_packet + 7) | 2);
                segment_depth = position->vz >> 7;
                if (segment_depth < 0)
                {
                    ADD_PACKET(0);
                    current_packet += 0x24;
                    segment_packet += 0x24;
                    strip_code += 0x24;
                }
                else if (segment_depth >= 0x1000)
                {
                    ADD_PACKET(0xFFF);
                    current_packet += 0x24;
                    segment_packet += 0x24;
                    strip_code += 0x24;
                }
                else
                {
                    ADD_PACKET(position->vz >> 7);
                    current_packet += 0x24;
                    segment_packet += 0x24;
                    strip_code += 0x24;
                }
                segment_index += 1;
                radius_sum += radius;
            } while (0);
        } while (segment_index < 9);
        *(s32*)(strip_code + (17)) = first_xy;
        *(s32*)(strip_code + (25)) = second_xy;
        *(s32*)(strip_code + (-3)) = 0;
        *(s32*)(strip_code + (5)) = 0xA000;
        *(s32*)(strip_code + (13)) = 0;
        *(s32*)(strip_code + (21)) = 0xA000;
        SetPolyG4((POLY_G4*)current_packet);
        *(u8*)(strip_code + (0)) = (u8)(*(u8*)(strip_code + (0)) | 2);
        closing_depth = position->vz >> 7;
        if (closing_depth < 0)
        {
            strip_code += 0x24;
            do
            {
                *(s32*)current_packet = (*(s32*)current_packet & tag_mask) | (ordering_table[0] & addr_mask);
                packet_addr = (s32)current_packet & addr_mask;
            } while (0);
            ordering_table[0] = (ordering_table[0] & tag_mask) | packet_addr;
            current_packet += 0x24;
        }
        else if (closing_depth >= 0x1000)
        {
            ADD_PACKET(0xFFF);
            current_packet += 0x24;
            strip_code += 0x24;
        }
        else
        {
            ADD_PACKET(position->vz >> 7);
            current_packet += 0x24;
            strip_code += 0x24;
        }
        radius += 0x50;
        do
        {
            if (radius >= 0x140)
            {
                radius -= 0x140;
            }
        } while (0);
        strip_index++;
    }
    if (strip_index < 4)
    {
        goto next_strip;
    }
    *(u8*)(current_packet + (3)) = 1;
    *(s32*)(current_packet + (4)) = 0xE1000025;
    drawpage_depth = position->vz >> 7;
    if (drawpage_depth < 0)
    {
        addPrim(&ordering_table[0], current_packet);
        current_packet += 8;
    }
    else if (drawpage_depth >= 0x1000)
    {
        addPrim(&ordering_table[0xFFF], current_packet);
        current_packet += 8;
    }
    else
    {
        addPrim(&ordering_table[position->vz >> 7], current_packet);
        current_packet += 8;
    }
    return current_packet;
}

#undef ADD_PACKET
