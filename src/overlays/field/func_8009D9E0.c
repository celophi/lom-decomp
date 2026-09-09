#include "common.h"

#define M2C_FIELD(base, type, offset) (*(type)((unsigned char *)(base) + (offset)))
void func_8001CDAC(s32 *, s32 *);                /* extern */
s32 func_8009E66C(void *, s32, s32 *, s32);         /* extern */
s32 func_8009FE54(void *, s32, s32 *, s32);         /* extern */
s32 func_800A0B0C(void *, s32, s32 *, s32, s32);    /* extern */
s32 func_800A1344(void *, s32, s32 *, s32, s32);    /* extern */
extern void *D_800F2288;
extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;
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
extern s32 D_801178D8;
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
 * @note Matches 99.966380% with gcc272_cdk: 803 instructions, 3212 bytes.
 * @note Seven register operand differences remain in the initial bounds switch.
 * @note Preserve scratch arrays and local lifetimes for matching code generation.
 */
void func_8009D9E0(Actor *arg0, u32 arg1)
{
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
        high = &limits[1];
        low = &limits[0];
        switch (arg1)
        {
        case 0:
            *low = 30;
            *high = 96;
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
