#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8*)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;

/** @brief Actor animation fields in a 44-byte effect slot. */
typedef struct
{
    u8 pad_00[14];
    s16 sequence;
    s16 previous_sequence;
    u8 pad_12[2];
    u8* cursor;
    u8* sequence_start;
    u8* frame_data;
    s16 remaining;
    u8 pad_22[10];
} WmapActor;

/** @brief Radial particle motion and lifetime. */
typedef struct
{
    s16 state, angle;
    s32 velocity, radius;
    s16 lifetime, z;
    s32 angular_velocity;
} WmapMotion;

/** @brief Resource slot containing an animation-data pointer. */
typedef struct
{
    s32 field_00;
    u8* data;
} WmapResource;

/** @brief Position and velocity records, also used for particle bounds. */
typedef struct
{
    SVECTOR position;
    SVECTOR velocity;
} WmapMovingPoint;

extern s32 D_8011CF74;
extern s32 D_8013B278;
extern u16 D_8013B280;
extern s32 D_8013B284;
extern WmapMotion D_801AFBD0[];
extern s32 D_801B0FD0;
extern SVECTOR D_80139278;
extern SVECTOR D_801398C8;
extern VECTOR D_80182DC0;
extern VECTOR D_80182D48;
extern MATRIX D_8011D0E8;
extern WmapActor D_800D9268[];
extern u16 D_80139980;
extern WmapResource D_80139988[];
extern u8 D_800D9150;
extern u8 D_800DCEA8;
extern u8 D_800DCEB8;
extern s16 D_800DCEBA;
extern s16 D_800DCEBC;
extern u8 D_8011CF4C;
extern s32 D_8011D510;
extern s32 D_8011D530;
extern u8 D_80139200;
extern s32 D_80139204;
extern u16 D_80139210;
extern u16 D_80139212;
extern u16 D_80139214;
extern u8 D_80139950;
extern s32 D_80139968;
extern s32 D_8013996C;
extern s32 D_8013B29C;

s32 rand(void);

/**
 * @brief Update, project, and replenish a configured particle effect.
 * @param arg0 Actor records to update.
 * @param arg1 Address of the resource-slot bytes associated with the actors.
 * @param arg2 Number of actor records to process.
 * @param arg3 Effect parameter; semantics remain unresolved.
 * @param arg4_value Effect parameter stored in a halfword.
 * @param arg5_value Effect parameter stored in a halfword.
 * @param arg6 Particle behavior selector.
 * @param arg7 Particle effect configuration record.
 */
void func_8006A2FC(void* arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4_value, s32 arg5_value, u32 arg6, void* arg7)
{
    u16 arg4 = arg4_value;
    u16 arg5 = arg5_value;
    SVECTOR position;
    s32 sp20;
    M2C_UNK var_s4;
    s16* temp_s2_2;
    s16 temp_v0_6;
    s32 temp_a1;
    s32 temp_lo;
    s32 temp_s0;
    s32 temp_v0_3;
    s32 temp_v0_4;
    s32 temp_v1_2;
    s32 var_s1;
    s32 var_s1_2;
    s32 var_v0;
    u16 temp_v0;
    u16 temp_v0_2;
    u16 temp_v0_5;
    u8 var_v1;
    void* temp_s2;
    void* temp_v0_7;
    void* temp_v0_8;
    void* temp_v1;
    void* var_a0;
    void* var_s0;

    var_s4 = 4;
    var_s1 = 0;
    if (arg2 > 0)
    {
        var_s0 = arg0;
        do
        {
            temp_s2 = ((var_s1 + M2C_FIELD(arg7, s32*, 0x1C)) * 0x14) + (u8*)&D_801AFBD0;
            if (M2C_FIELD(temp_s2, s16*, 0) != 0)
            {
                position.vx = (s16)((s32)(((s32)M2C_FIELD(temp_s2, s32*, 8) >> 3) * (ccos(M2C_FIELD(temp_s2, s16*, 2)) >> 6)) >> 0xC);
                position.vy = (s16)((s32)(((s32)M2C_FIELD(temp_s2, s32*, 8) >> 3) * (csin(M2C_FIELD(temp_s2, s16*, 2)) >> 6)) >> 0xC);
                position.vz = M2C_FIELD(temp_s2, u16*, 0xE);
                gte_ldv0(&position);
                gte_rtps();
                switch (arg6)
                {
                case 5:
                    if (M2C_FIELD(var_s0, s16*, 0x24) >= arg3)
                    {
                        M2C_FIELD(var_s0, s16*, 0x22) = 0;
                    }
                    if ((M2C_FIELD(var_s0, s16*, 0x22) == 0) && (M2C_FIELD(var_s0, s16*, 0x24) < 4))
                    {
                        M2C_FIELD(temp_s2, s16*, 0) = 0;
                    }
                    /* fallthrough */
                case 0:
                    M2C_FIELD(temp_s2, u16*, 0xE) = (u16)(M2C_FIELD(temp_s2, u16*, 0xE) + M2C_FIELD(temp_s2, u16*, 4));
                    temp_v0 = M2C_FIELD(temp_s2, u16*, 0xC) - 1;
                    M2C_FIELD(temp_s2, u16*, 0xC) = temp_v0;
                    M2C_FIELD(temp_s2, s32*, 8) = (s32)(M2C_FIELD(temp_s2, s32*, 8) + M2C_FIELD(arg7, s32*, 0x18));
                    if ((temp_v0 << 0x10) == 0)
                    {
                        M2C_FIELD(temp_s2, s16*, 0) = 0;
                    }
                default:
                block_28:
                    var_v0 = var_s1 * 8;
                    break;
                case 1:
                    if (*(s32*)0x8013B264 < (s16)M2C_FIELD(temp_s2, u16*, 0xC))
                    {
                        M2C_FIELD(temp_s2, u16*, 0xE) = (u16)(M2C_FIELD(temp_s2, u16*, 0xE) + (*(u16*)0x8013B270 + ((s32)(rand() * D_8013B278) >> 0xF)));
                        M2C_FIELD(temp_s2, s32*, 8) = (s32)(M2C_FIELD(temp_s2, s32*, 8) + M2C_FIELD(arg7, s32*, 0x18));
                    }
                    else
                    {
                        temp_v0_2 = M2C_FIELD(temp_s2, u16*, 0xE) - (D_8013B280 + ((s32)(rand() * D_8013B284) >> 0xF));
                        M2C_FIELD(temp_s2, u16*, 0xE) = temp_v0_2;
                        if (temp_v0_2 & 0x8000)
                        {
                            M2C_FIELD(temp_s2, u16*, 0xE) = 0U;
                            M2C_FIELD(var_s0, s16*, 0x22) = 0;
                            M2C_FIELD(var_s0, s16*, 0x26) = 8;
                            if (M2C_FIELD(var_s0, s16*, 0x24) < 4)
                            {
                                M2C_FIELD(temp_s2, s16*, 0) = 0;
                            }
                        }
                    }
                    var_s4 = 0xC;
                    if ((u32)((u16)M2C_FIELD(temp_s2, s16*, 2) - 1) < 0x7FFU)
                    {
                        var_s4 = 0xA;
                    }
                    M2C_FIELD(temp_s2, u16*, 0xC) = (u16)(M2C_FIELD(temp_s2, u16*, 0xC) - 1);
                    goto block_28;
                case 2:
                    temp_v0_3 = (s16)M2C_FIELD(temp_s2, u16*, 0xC) - *(s32*)0x8013B264;
                    M2C_FIELD(temp_s2, u16*, 0xE) = (u16)(D_8013B280 + (*(u16*)0x8013B270 - ((s32)(temp_v0_3 * temp_v0_3) / (s32)D_8013B278)));
                    if ((s16)M2C_FIELD(temp_s2, u16*, 0xC) != 0)
                    {
                        M2C_FIELD(temp_s2, u16*, 0xC) = (u16)(M2C_FIELD(temp_s2, u16*, 0xC) - 1);
                        M2C_FIELD(temp_s2, s32*, 8) = (s32)(M2C_FIELD(temp_s2, s32*, 8) + M2C_FIELD(arg7, s32*, 0x18));
                    }
                    goto block_28;
                case 3:
                    temp_v0_4 = *(s32*)0x8013B264 - (s16)M2C_FIELD(temp_s2, u16*, 0xC);
                    M2C_FIELD(temp_s2, u16*, 0xE) = (u16)((s32)(temp_v0_4 * temp_v0_4) / (s32) * (u16*)0x8013B270);
                    goto block_28;
                case 4:
                    M2C_FIELD(temp_s2, u16*, 0xE) = (u16)(M2C_FIELD(temp_s2, u16*, 0xE) + M2C_FIELD(temp_s2, u16*, 4));
                    temp_v0_5 = M2C_FIELD(temp_s2, u16*, 0xC) - 1;
                    M2C_FIELD(temp_s2, u16*, 0xC) = temp_v0_5;
                    M2C_FIELD(temp_s2, s32*, 8) = (s32)(M2C_FIELD(temp_s2, s32*, 8) + M2C_FIELD(arg7, s32*, 0x18));
                    if ((temp_v0_5 << 0x10) == 0)
                    {
                        M2C_FIELD(temp_s2, s16*, 0) = 0;
                    }
                    goto block_28;
                case 7:
                    M2C_FIELD(temp_s2, u16*, 0xE) = (u16)(M2C_FIELD(temp_s2, u16*, 0xE) + M2C_FIELD(temp_s2, u16*, 4));
                    M2C_FIELD(temp_s2, s16*, 2) = (s16)((u16)M2C_FIELD(temp_s2, s16*, 2) + M2C_FIELD(arg7, u16*, 0x10));
                    M2C_FIELD(temp_s2, s32*, 8) = (s32)(M2C_FIELD(temp_s2, s32*, 8) + M2C_FIELD(arg7, s32*, 0x18));
                    /* fallthrough */
                case 6:
                    var_v0 = var_s1 * 8;
                    if ((s16)M2C_FIELD(temp_s2, u16*, 0xE) < 0)
                    {
                        M2C_FIELD(temp_s2, s16*, 0) = 0;
                        goto block_28;
                    }
                    break;
                }
                temp_a1 = M2C_FIELD((var_v0 + arg1), s32*, 4);
                temp_v0_6 = (s16)M2C_FIELD(var_s0, u16*, 0xE);
                if (M2C_FIELD(var_s0, s16*, 0x10) != temp_v0_6)
                {
                    M2C_FIELD(var_s0, s16*, 0x10) = (s16)M2C_FIELD(var_s0, u16*, 0xE);
                    M2C_FIELD(var_s0, s16*, 0x20) = 1;
                    temp_v1 = temp_a1 + *(s16*)((temp_v0_6 * 2) + temp_a1);
                    M2C_FIELD(var_s0, void**, 0x18) = temp_v1;
                    M2C_FIELD(var_s0, void**, 0x14) = temp_v1;
                }
                if (M2C_FIELD(var_s0, s16*, 0x20) != 0xFF)
                {
                    M2C_FIELD(var_s0, s16*, 0x20) = (s16)((u16)M2C_FIELD(var_s0, s16*, 0x20) - 1);
                }
                if (M2C_FIELD(var_s0, s16*, 0x20) == 0)
                {
                    temp_v0_7 = M2C_FIELD(var_s0, void**, 0x14);
                    var_v1 = M2C_FIELD(temp_v0_7, u8*, 0);
                    M2C_FIELD(var_s0, s16*, 0x20) = (s16)M2C_FIELD(temp_v0_7, u8*, 1);
                    if (var_v1 == 0xFF)
                    {
                        temp_v0_8 = M2C_FIELD(var_s0, void**, 0x18);
                        M2C_FIELD(var_s0, void**, 0x14) = temp_v0_8;
                        var_v1 = M2C_FIELD(temp_v0_8, u8*, 0);
                        M2C_FIELD(var_s0, s16*, 0x20) = (s16)M2C_FIELD(temp_v0_8, u8*, 1);
                    }
                    M2C_FIELD(var_s0, void**, 0x14) = (void*)(M2C_FIELD(var_s0, void**, 0x14) + 4);
                    M2C_FIELD(var_s0, s32*, 0x1C) = (s32)(temp_a1 + M2C_FIELD(((var_v1 * 2) + temp_a1), s16*, 0x40));
                }
                gte_stsxy(&sp20);
                func_80066F9C(var_s0, sp20, M2C_FIELD(arg7, s32*, 0x20), var_s4, 0);
            }
            var_s1 += 1;
            var_s0 += 0x2C;
        } while (var_s1 < arg2);
    }
    temp_v1_2 = M2C_FIELD(arg7, s32*, 0x14);
    if ((temp_v1_2 != -1) && (((s32)D_8011CF74 % temp_v1_2) == 0) && (var_s1_2 = 0, (arg2 > 0)))
    {
        var_a0 = arg0;
    loop_43:
        temp_s2_2 = ((var_s1_2 + M2C_FIELD(arg7, s32*, 0x1C)) * 0x14) + (u8*)&D_801AFBD0;
        var_s1_2 += 1;
        if (M2C_FIELD(temp_s2_2, s16*, 0) == 0)
        {
            M2C_FIELD(var_a0, s16*, 2) = 0;
            M2C_FIELD(var_a0, s8*, 6) = 0xF;
            M2C_FIELD(var_a0, s16*, 0x10) = -1;
            M2C_FIELD(var_a0, u16*, 0x22) = (u16)arg3;
            M2C_FIELD(var_a0, u16*, 0x24) = arg4;
            M2C_FIELD(var_a0, u16*, 0xE) = (u16)M2C_FIELD(arg7, u16*, 0x24);
            M2C_FIELD(var_a0, u16*, 0x26) = arg5;
            M2C_FIELD(temp_s2_2, s16*, 0) = 1;
            M2C_FIELD(temp_s2_2, s16*, 2) = rand();
            temp_s0 = M2C_FIELD(arg7, s32*, 0x28);
            if (temp_s0 < 0)
            {
                M2C_FIELD(temp_s2_2, s32*, 8) = (s32)(((s32)(rand() * ((temp_s0 & 0xFFFF) << 5)) >> 0xF) + ((s32)(temp_s0 & 0x7FFF0000) >> 0xE));
            }
            else
            {
                M2C_FIELD(temp_s2_2, s32*, 8) = temp_s0;
            }
            M2C_FIELD(temp_s2_2, s32*, 4) = (s32)(((s32)(rand() * M2C_FIELD(arg7, s32*, 8)) >> 0xF) + M2C_FIELD(arg7, s32*, 4));
            temp_lo = rand() * (s32)M2C_FIELD(arg7, u16*, 0x10);
            M2C_FIELD(temp_s2_2, s16*, 0xE) = 0;
            M2C_FIELD(temp_s2_2, s16*, 0xC) = (s16)(M2C_FIELD(arg7, u16*, 0xC) + (temp_lo >> 0xF));
            return;
        }
        var_a0 += 0x2C;
        if (var_s1_2 >= arg2)
        {
        }
        else
        {
            goto loop_43;
        }
    }
}

/**
 * @brief Advance and periodically spawn particles in the selected actor range.
 * @param arg0 Effect parameter; semantics remain unresolved.
 * @param arg1 Effect parameter; semantics remain unresolved.
 * @param arg2 Effect parameter; semantics remain unresolved.
 * @param arg3 Effect parameter; semantics remain unresolved.
 * @param arg4 Effect parameter; semantics remain unresolved.
 * @param arg5 Effect parameter; semantics remain unresolved.
 * @param arg6 Effect parameter; semantics remain unresolved.
 * @param arg7 Effect parameter; semantics remain unresolved.
 * @param arg8 Effect parameter; semantics remain unresolved.
 * @param arg9 Effect parameter; semantics remain unresolved.
 * @param arg10 Effect parameter; semantics remain unresolved.
 * @param arg11 Effect parameter; semantics remain unresolved.
 */
void func_8006A9C4(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, s32 arg7, s32 arg8, u16 arg9, s32 arg10, s32 arg11)
{
    SVECTOR position;
    s32 sp20;
    s16 temp_v0;
    s32 temp_a1;
    s32 temp_lo;
    s32 var_a1;
    s32 var_s2;
    s32 var_s2_2;
    s32 var_s4;
    u16 temp_v0_4;
    u8 var_v1;
    void* temp_v0_2;
    void* temp_v0_3;
    void* temp_v1;
    void* var_a0;
    void* var_s0;
    void* var_s0_2;
    void* var_s1;
    void* var_s3;

    var_s4 = 0;
    var_s2 = arg2;
    if (var_s2 < arg3)
    {
        var_s1 = (var_s2 * 0x14) + (u8*)&D_801AFBD0;
        var_s0 = (var_s2 * 0x2C) + arg0;
        var_s3 = (var_s2 * 8) + arg1;
        do
        {
            if (M2C_FIELD(var_s1, s16*, 0) != 0)
            {
                position.vx = (s16)((s32)(((s32)M2C_FIELD(var_s1, s32*, 8) >> 6) * (ccos(M2C_FIELD(var_s1, s16*, 2)) >> 6)) >> 0xC);
                position.vy = (s16)((s32)(((s32)M2C_FIELD(var_s1, s32*, 8) >> 6) * (csin(M2C_FIELD(var_s1, s16*, 2)) >> 6)) >> 0xC);
                position.vz = (s16)(0xC8 / (s16)M2C_FIELD(var_s1, s16*, 0xE));
                gte_ldv0(&position);
                gte_rtps();
                var_a1 = arg4 * 0x81;
                M2C_FIELD(var_s1, s16*, 0xE) = (s16)((u16)M2C_FIELD(var_s1, s16*, 0xE) + 1);
                M2C_FIELD(var_s1, s32*, 8) = (s32)(M2C_FIELD(var_s1, s32*, 8) + M2C_FIELD(var_s1, s32*, 4));
                M2C_FIELD(var_s0, s16*, 0x22) = 0;
                if (var_a1 < 0)
                {
                    var_a1 += 0xFF;
                }
                M2C_FIELD(var_s0, s16*, 0x24) = (s16)(var_a1 >> 8);
                gte_stsxy(&sp20);
                temp_a1 = M2C_FIELD(var_s3, s32*, 4);
                temp_v0 = M2C_FIELD(var_s0, s16*, 0xE);
                if (M2C_FIELD(var_s0, s16*, 0x10) != temp_v0)
                {
                    M2C_FIELD(var_s0, s16*, 0x10) = (s16)(u16)M2C_FIELD(var_s0, s16*, 0xE);
                    M2C_FIELD(var_s0, s16*, 0x20) = 1;
                    temp_v1 = temp_a1 + *(s16*)((temp_v0 * 2) + temp_a1);
                    M2C_FIELD(var_s0, void**, 0x18) = temp_v1;
                    M2C_FIELD(var_s0, void**, 0x14) = temp_v1;
                }
                if (M2C_FIELD(var_s0, s16*, 0x20) != 0xFF)
                {
                    M2C_FIELD(var_s0, s16*, 0x20) = (s16)((u16)M2C_FIELD(var_s0, s16*, 0x20) - 1);
                }
                if (M2C_FIELD(var_s0, s16*, 0x20) == 0)
                {
                    temp_v0_2 = M2C_FIELD(var_s0, void**, 0x14);
                    var_v1 = M2C_FIELD(temp_v0_2, u8*, 0);
                    M2C_FIELD(var_s0, s16*, 0x20) = (s16)M2C_FIELD(temp_v0_2, u8*, 1);
                    if (var_v1 == 0xFF)
                    {
                        temp_v0_3 = M2C_FIELD(var_s0, void**, 0x18);
                        M2C_FIELD(var_s0, void**, 0x14) = temp_v0_3;
                        var_v1 = M2C_FIELD(temp_v0_3, u8*, 0);
                        M2C_FIELD(var_s0, s16*, 0x20) = (s16)M2C_FIELD(temp_v0_3, u8*, 1);
                    }
                    M2C_FIELD(var_s0, void**, 0x14) = (void*)(M2C_FIELD(var_s0, void**, 0x14) + 4);
                    M2C_FIELD(var_s0, s32*, 0x1C) = (s32)(temp_a1 + M2C_FIELD(((var_v1 * 2) + temp_a1), s16*, 0x40));
                }
                func_80066F9C(var_s0, sp20, arg10, 0xA, 0);
                temp_v0_4 = M2C_FIELD(var_s1, u16*, 0xC) - 1;
                M2C_FIELD(var_s1, u16*, 0xC) = temp_v0_4;
                if ((temp_v0_4 << 0x10) == 0)
                {
                    M2C_FIELD(var_s1, s16*, 0) = 0;
                }
                var_s4 += 1;
            }
            var_s1 += 0x14;
            var_s0 += 0x2C;
            var_s2 += 1;
            var_s3 += 8;
        } while (var_s2 < arg3);
    }
    if ((((s32)D_8011CF74 % arg11) == 0) && (var_s2_2 = arg2, ((var_s2_2 < arg3) != 0)))
    {
        var_s0_2 = (var_s2_2 * 0x14) + (u8*)&D_801AFBD0;
        var_a0 = (var_s2_2 * 0x2C) + arg0;
    loop_20:
        var_s2_2 += 1;
        if (M2C_FIELD(var_s0_2, s16*, 0) == 0)
        {
            if (D_801B0FD0 >= var_s4)
            {
                M2C_FIELD(var_a0, s8*, 6) = 0xF;
                M2C_FIELD(var_a0, s16*, 2) = 0;
                M2C_FIELD(var_a0, s16*, 0xE) = 1;
                M2C_FIELD(var_a0, s16*, 0x10) = -1;
                M2C_FIELD(var_s0_2, s16*, 0) = 1;
                M2C_FIELD(var_s0_2, s16*, 2) = (s16)(rand() >> 3);
                M2C_FIELD(var_s0_2, s32*, 8) = 0;
                M2C_FIELD(var_s0_2, s32*, 4) = (s32)(((s32)(rand() * arg6) >> 0xF) + arg5);
                temp_lo = rand() * arg8;
                M2C_FIELD(var_s0_2, u16*, 0xE) = arg9;
                M2C_FIELD(var_s0_2, s16*, 0xC) = (s16)((temp_lo >> 0xF) + arg7);
            }
        }
        else
        {
            var_s0_2 += 0x14;
            var_a0 += 0x2C;
            if (var_s2_2 >= arg3)
            {
            }
            else
            {
                goto loop_20;
            }
        }
    }
}

/**
 * @brief Combine global and local transforms and install the resulting matrix.
 * @param translation Local translation applied to the base matrix.
 * @param rotation Local rotation composed with the base matrix.
 */
void func_8006ADD0(VECTOR* translation, SVECTOR* rotation)
{
    MATRIX base;
    MATRIX effect;
    SVECTOR combined_rotation;
    VECTOR combined_translation;

    combined_rotation.vx = D_80139278.vx + D_801398C8.vx;
    combined_rotation.vy = D_80139278.vy + D_801398C8.vy;
    combined_rotation.vz = D_80139278.vz + D_801398C8.vz;
    combined_translation.vx = D_80182DC0.vx + D_80182D48.vx;
    combined_translation.vy = D_80182DC0.vy + D_80182D48.vy;
    combined_translation.vz = D_80182DC0.vz + D_80182D48.vz;
    RotMatrix(&combined_rotation, &base);
    TransMatrix(&base, translation);
    SetRotMatrix(&base);
    SetTransMatrix(&base);
    RotMatrix(rotation, &effect);
    CompMatrix(&base, &effect, &effect);
    TransMatrix(&effect, &combined_translation);
    SetRotMatrix(&effect);
    SetTransMatrix(&effect);
}

/** @brief Combine the base and effect transforms and install the resulting matrix. */
void func_8006AEE0(void)
{
    SVECTOR rotation;
    VECTOR translation;

    rotation.vx = D_80139278.vx + D_801398C8.vx;
    rotation.vy = D_80139278.vy + D_801398C8.vy;
    rotation.vz = D_80139278.vz + D_801398C8.vz;
    translation.vx = D_80182DC0.vx + D_80182D48.vx;
    translation.vy = D_80182DC0.vy + D_80182D48.vy;
    translation.vz = D_80182DC0.vz + D_80182D48.vz;
    RotMatrix(&rotation, &D_8011D0E8);
    TransMatrix(&D_8011D0E8, &translation);
    SetRotMatrix(&D_8011D0E8);
    SetTransMatrix(&D_8011D0E8);
}

/**
 * @brief Update radial particles and replenish inactive entries.
 * @param arg0 Effect parameter; semantics remain unresolved.
 * @param arg1 Effect parameter; semantics remain unresolved.
 * @param arg2 Effect parameter; semantics remain unresolved.
 * @param arg3 Effect parameter; semantics remain unresolved.
 * @param arg4 Effect parameter; semantics remain unresolved.
 * @param arg5 Effect parameter; semantics remain unresolved.
 * @param arg6 Effect parameter; semantics remain unresolved.
 * @param arg7 Effect parameter; semantics remain unresolved.
 */
void func_8006AFAC(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, u16 arg7)
{
    SVECTOR position;
    s32 sp20;
    s16 temp_v0;
    s32 temp_a1;
    s32 temp_lo;
    s32 var_s2;
    s32 var_s2_2;
    s32 var_s3;
    s32 var_s4;
    u16 temp_v0_4;
    u8 var_v1;
    void* temp_s0;
    void* temp_v0_2;
    void* temp_v0_3;
    void* temp_v1;
    void* var_s0;
    void* var_s1;
    void* var_s1_2;

    var_s3 = 0;
    var_s2 = arg0;
    if (var_s2 < arg1)
    {
        var_s1 = (var_s2 * 0x14) + (u8*)&D_801AFBD0;
        var_s4 = var_s2 * 0x2C;
        do
        {
            if (M2C_FIELD(var_s1, s16*, 0) != 0)
            {
                temp_s0 = var_s4 + (u8*)&D_800D9268;
                position.vx = (s16)((s32)(((s32)M2C_FIELD(var_s1, s32*, 8) >> 6) * (ccos(M2C_FIELD(var_s1, s16*, 2)) >> 6)) >> 0xC);
                temp_lo = ((s32)M2C_FIELD(var_s1, s32*, 8) >> 6) * (csin(M2C_FIELD(var_s1, s16*, 2)) >> 6);
                position.vz = 0;
                position.vy = (s16)(temp_lo >> 0xC);
                gte_ldv0(&position);
                gte_rtps();
                M2C_FIELD(var_s1, s32*, 8) = (s32)(M2C_FIELD(var_s1, s32*, 8) + M2C_FIELD(var_s1, s32*, 4));
                M2C_FIELD(temp_s0, u16*, 0x22) = (u16)D_80139980;
                M2C_FIELD(temp_s0, u16*, 0x24) = (u16)D_80139980;
                gte_stsxy(&sp20);
                temp_a1 = M2C_FIELD(((var_s2 * 8) + (u8*)&D_80139988), s32*, 4);
                temp_v0 = M2C_FIELD(temp_s0, s16*, 0xE);
                if (M2C_FIELD(temp_s0, s16*, 0x10) != temp_v0)
                {
                    M2C_FIELD(temp_s0, s16*, 0x10) = (s16)(u16)M2C_FIELD(temp_s0, s16*, 0xE);
                    M2C_FIELD(temp_s0, s16*, 0x20) = 1;
                    temp_v1 = temp_a1 + *(s16*)((temp_v0 * 2) + temp_a1);
                    M2C_FIELD(temp_s0, void**, 0x18) = temp_v1;
                    M2C_FIELD(temp_s0, void**, 0x14) = temp_v1;
                }
                if (M2C_FIELD(temp_s0, s16*, 0x20) != 0xFF)
                {
                    M2C_FIELD(temp_s0, s16*, 0x20) = (s16)((u16)M2C_FIELD(temp_s0, s16*, 0x20) - 1);
                }
                if (M2C_FIELD(temp_s0, s16*, 0x20) == 0)
                {
                    temp_v0_2 = M2C_FIELD(temp_s0, void**, 0x14);
                    var_v1 = M2C_FIELD(temp_v0_2, u8*, 0);
                    M2C_FIELD(temp_s0, s16*, 0x20) = (s16)M2C_FIELD(temp_v0_2, u8*, 1);
                    if (var_v1 == 0xFF)
                    {
                        temp_v0_3 = M2C_FIELD(temp_s0, void**, 0x18);
                        M2C_FIELD(temp_s0, void**, 0x14) = temp_v0_3;
                        var_v1 = M2C_FIELD(temp_v0_3, u8*, 0);
                        M2C_FIELD(temp_s0, s16*, 0x20) = (s16)M2C_FIELD(temp_v0_3, u8*, 1);
                    }
                    M2C_FIELD(temp_s0, void**, 0x14) = (void*)(M2C_FIELD(temp_s0, void**, 0x14) + 4);
                    M2C_FIELD(temp_s0, s32*, 0x1C) = (s32)(temp_a1 + M2C_FIELD(((var_v1 * 2) + temp_a1), s16*, 0x40));
                }
                func_80066F9C(temp_s0, sp20, arg2, arg3, 0);
                temp_v0_4 = M2C_FIELD(var_s1, u16*, 0xC) - 1;
                M2C_FIELD(var_s1, u16*, 0xC) = temp_v0_4;
                if ((temp_v0_4 << 0x10) == 0)
                {
                    M2C_FIELD(var_s1, s16*, 0) = 0;
                }
                var_s3 += 1;
            }
            var_s1 += 0x14;
            var_s2 += 1;
            var_s4 += 0x2C;
        } while (var_s2 < arg1);
    }
    var_s2_2 = arg0;
    if (var_s2_2 < arg1)
    {
        var_s0 = (var_s2_2 * 0x14) + (u8*)&D_801AFBD0;
        var_s1_2 = (var_s2_2 * 0x2C) + (u8*)&D_800D9268;
    loop_17:
        if (M2C_FIELD(var_s0, s16*, 0) == 0)
        {
            var_s3 += 1;
            if (D_801B0FD0 >= var_s3)
            {
                M2C_FIELD(var_s1_2, s16*, 2) = 0;
                M2C_FIELD(var_s1_2, s8*, 6) = 0xF;
                M2C_FIELD(var_s1_2, s16*, 0x10) = -1;
                M2C_FIELD(var_s1_2, u16*, 0xE) = arg7;
                M2C_FIELD(var_s0, s16*, 0) = 1;
                M2C_FIELD(var_s0, s16*, 2) = (s16)(rand() >> 3);
                M2C_FIELD(var_s0, s32*, 8) = 0;
                M2C_FIELD(var_s0, s32*, 4) = (s32)(arg4 + (rand() / arg5));
                M2C_FIELD(var_s0, s16*, 0xC) = (s16)(arg6 + (rand() & 0x3C));
                goto block_20;
            }
        }
        else
        {
        block_20:
            var_s0 += 0x14;
            var_s2_2 += 1;
            var_s1_2 += 0x2C;
            if (var_s2_2 < arg1)
            {
                goto loop_17;
            }
        }
    }
}

/**
 * @brief Update particle trails and initialize inactive entries from the effect parameters.
 * @param arg0 Effect parameter; semantics remain unresolved.
 * @param arg1 Effect parameter; semantics remain unresolved.
 * @param arg2 Effect parameter; semantics remain unresolved.
 * @param arg3 Effect parameter; semantics remain unresolved.
 * @param arg4 Effect parameter; semantics remain unresolved.
 * @param arg5 Effect parameter; semantics remain unresolved.
 * @param arg6 Effect parameter; semantics remain unresolved.
 * @param arg7 Effect parameter; semantics remain unresolved.
 * @param arg8 Effect parameter; semantics remain unresolved.
 * @param arg9 Effect parameter; semantics remain unresolved.
 * @param arg10 Effect parameter; semantics remain unresolved.
 * @param arg11 Effect parameter; semantics remain unresolved.
 * @param arg12 Effect parameter; semantics remain unresolved.
 * @param arg13 Effect parameter; semantics remain unresolved.
 * @param arg14 Effect parameter; semantics remain unresolved.
 * @param arg15 Effect parameter; semantics remain unresolved.
 * @param arg16 Effect parameter; semantics remain unresolved.
 */
void func_8006B328(s32 arg0, s32 arg1, s32 arg2, s16 arg3, s32 arg4, s32 arg5, u16 arg6, s32 arg7, s32 arg8, s32 arg9, s32 arg10, s32 arg11, s32 arg12,
                   u16 arg13, u16 arg14, u16 arg15, s32 arg16)
{
    SVECTOR position;
    s32 sp20;
    s16 temp_v0_2;
    s32* temp_a0;
    s32* temp_a1_3;
    s32* temp_v0;
    s32* temp_v1_3;
    s32 temp_a1;
    s32 temp_a1_2;
    s32 temp_v1_2;
    s32 var_s2;
    s32 var_s3;
    s32 var_s4;
    u16 temp_v0_5;
    u8 var_v1;
    void* temp_s0;
    void* temp_s1;
    void* temp_v0_3;
    void* temp_v0_4;
    void* temp_v1;

    temp_a1 = arg16 * 4;
    temp_v0 = temp_a1 + (u8*)&D_800D9150;
    var_s2 = arg0;
    *temp_v0 -= 1;
    if (var_s2 < arg1)
    {
        var_s4 = var_s2 * 0x2C;
        var_s3 = var_s2 * 0x14;
        do
        {
            temp_s1 = var_s3 + (u8*)&D_801AFBD0;
            temp_s0 = var_s4 + (u8*)&D_800D9268;
            if (M2C_FIELD(temp_s1, s16*, 0) != 0)
            {
                if (arg3 != -1)
                {
                    M2C_FIELD(temp_s0, s16*, 0x24) = arg3;
                    M2C_FIELD(temp_s0, s16*, 0x22) = arg3;
                }
                position.vx = M2C_FIELD(temp_s1, u16*, 0x10);
                position.vy = M2C_FIELD(temp_s1, u16*, 0x12);
                position.vz = M2C_FIELD(temp_s1, u16*, 0xE);
                gte_ldv0(&position);
                gte_rtps();
                temp_a1_2 = M2C_FIELD(((var_s2 * 8) + (u8*)&D_80139988), s32*, 4);
                temp_v0_2 = M2C_FIELD(temp_s0, s16*, 0xE);
                if (M2C_FIELD(temp_s0, s16*, 0x10) != temp_v0_2)
                {
                    M2C_FIELD(temp_s0, s16*, 0x10) = (s16)(u16)M2C_FIELD(temp_s0, s16*, 0xE);
                    M2C_FIELD(temp_s0, s16*, 0x20) = 1;
                    temp_v1 = temp_a1_2 + *(s16*)((temp_v0_2 * 2) + temp_a1_2);
                    M2C_FIELD(temp_s0, void**, 0x18) = temp_v1;
                    M2C_FIELD(temp_s0, void**, 0x14) = temp_v1;
                }
                if (M2C_FIELD(temp_s0, s16*, 0x20) != 0xFF)
                {
                    M2C_FIELD(temp_s0, s16*, 0x20) = (s16)((u16)M2C_FIELD(temp_s0, s16*, 0x20) - 1);
                }
                if (M2C_FIELD(temp_s0, s16*, 0x20) == 0)
                {
                    temp_v0_3 = M2C_FIELD(temp_s0, void**, 0x14);
                    var_v1 = M2C_FIELD(temp_v0_3, u8*, 0);
                    M2C_FIELD(temp_s0, s16*, 0x20) = (s16)M2C_FIELD(temp_v0_3, u8*, 1);
                    if (var_v1 == 0xFF)
                    {
                        temp_v0_4 = M2C_FIELD(temp_s0, void**, 0x18);
                        M2C_FIELD(temp_s0, void**, 0x14) = temp_v0_4;
                        var_v1 = M2C_FIELD(temp_v0_4, u8*, 0);
                        M2C_FIELD(temp_s0, s16*, 0x20) = (s16)M2C_FIELD(temp_v0_4, u8*, 1);
                    }
                    M2C_FIELD(temp_s0, void**, 0x14) = (void*)(M2C_FIELD(temp_s0, void**, 0x14) + 4);
                    M2C_FIELD(temp_s0, s32*, 0x1C) = (s32)(temp_a1_2 + M2C_FIELD(((var_v1 * 2) + temp_a1_2), s16*, 0x40));
                }
                gte_stsxy(&sp20);
                func_80066F9C(temp_s0, sp20, arg7, 4, 0);
                temp_v0_5 = M2C_FIELD(temp_s1, u16*, 0xE) - M2C_FIELD(temp_s1, u16*, 4);
                M2C_FIELD(temp_s1, u16*, 0xE) = temp_v0_5;
                if ((s16)temp_v0_5 < arg12)
                {
                    M2C_FIELD(temp_s0, s16*, 0x22) = 0;
                }
                if ((s16)M2C_FIELD(temp_s1, u16*, 0xE) < 0)
                {
                    M2C_FIELD(temp_s1, u16*, 0xE) = 0U;
                }
                if ((M2C_FIELD(temp_s0, s16*, 0x22) == 0) && (M2C_FIELD(temp_s0, s16*, 0x24) < 5))
                {
                    M2C_FIELD(temp_s1, s16*, 0) = 0;
                }
            }
            else
            {
                temp_a1_3 = temp_a1 + (u8*)&D_800D9150;
                if (*temp_a1_3 == 0)
                {
                    temp_a0 = temp_a1 + (u8*)&D_800DCEA8;
                    temp_v1_2 = *temp_a0;
                    if (temp_v1_2 != 0)
                    {
                        if (temp_v1_2 == 1)
                        {
                            *temp_a1_3 = arg2;
                        }
                        else
                        {
                            *temp_a0 = temp_v1_2 - 1;
                        }
                        M2C_FIELD(temp_s1, s16*, 0) = 1;
                        M2C_FIELD(temp_s1, u16*, 0xE) = arg6;
                        M2C_FIELD(temp_s1, u16*, 0x10) = (u16)(((s32)(rand() * arg9) >> 0xF) + arg8);
                        M2C_FIELD(temp_s1, u16*, 0x12) = (u16)(((s32)(rand() * arg11) >> 0xF) + arg10);
                        M2C_FIELD(temp_s1, u16*, 4) = (s32)(((s32)(rand() * arg5) >> 0xF) + arg4);
                        M2C_FIELD(temp_s0, s16*, 0x22) = (s16)arg13;
                        M2C_FIELD(temp_s0, s16*, 0x24) = (s16)arg14;
                        M2C_FIELD(temp_s0, u16*, 0x26) = arg15;
                    }
                }
            }
            var_s4 += 0x2C;
            var_s2 += 1;
            var_s3 += 0x14;
        } while (var_s2 < arg1);
    }
    temp_v1_3 = (arg16 * 4) + (u8*)&D_800D9150;
    if (*temp_v1_3 == 0)
    {
        *temp_v1_3 = arg2;
    }
}

/**
 * @brief Project moving particles, advance their animation, and draw by depth.
 * @param first First actor index.
 * @param end Exclusive last actor index.
 * @param frame Sprite frame passed to the renderer.
 * @param z_step Per-update change in particle Z.
 * @param depth Base ordering-table depth.
 */
void func_8006B6EC(s32 first, s32 end, s32 frame, s32 z_step, s32 depth)
{
    SVECTOR position;
    s32 screen_position;
    s32 i;
    WmapMotion* motion;
    WmapActor* actor;
    u8* data;
    u8* cursor;
    s16* offsets;
    s32 animation_frame;
    s32 offset;

    for (i = first; i < end; i++)
    {
        actor = &D_800D9268[i];
        motion = &D_801AFBD0[i];
        if (motion->state != 0)
        {
            position.vx = ((motion->radius >> 6) * (ccos(motion->angle) >> 6)) >> 12;
            position.vy = ((motion->radius >> 6) * (csin(motion->angle) >> 6)) >> 12;
            position.vz = motion->z;
            gte_ldv0(&position);
            gte_rtps();
            motion->z += z_step;
            motion->radius += motion->velocity;
            motion->angle = (motion->angle + motion->angular_velocity) & 4095;
            gte_stsxy(&screen_position);
            offsets = (s16*)D_80139988[i].data;
            data = (u8*)offsets;
            if (actor->previous_sequence != actor->sequence)
            {
                actor->previous_sequence = (u16)actor->sequence;
                offset = offsets[actor->sequence];
                actor->remaining = 1;
                cursor = (u8*)offsets + offset;
                actor->sequence_start = cursor;
                actor->cursor = cursor;
            }
            if (actor->remaining != 255)
            {
                actor->remaining--;
            }
            if (actor->remaining == 0)
            {
                cursor = actor->cursor;
                animation_frame = cursor[0];
                actor->remaining = cursor[1];
                if (animation_frame == 255)
                {
                    cursor = actor->sequence_start;
                    actor->cursor = cursor;
                    animation_frame = cursor[0];
                    actor->remaining = cursor[1];
                }
                actor->cursor += 4;
                actor->frame_data = data + ((s16*)(animation_frame * 2 + data))[32];
            }
            if ((u32)((u16)motion->angle - 1025) < 2047U)
            {
                func_80066F9C(actor, screen_position, frame, depth + 2, 0);
            }
            else
            {
                func_80066F9C(actor, screen_position, frame, depth - 2, 0);
            }
            motion->lifetime--;
            if (motion->lifetime == 0)
            {
                motion->state = 0;
            }
        }
    }
}

/**
 * @brief Advance and project animated particles, bouncing at the supplied bounds.
 * @param first First actor index.
 * @param end Exclusive last actor index; this point contains the bounds.
 * @param point_data Packed position/velocity records.
 * @param frame Sprite frame passed to the renderer.
 * @param depth Ordering-table depth.
 */
void func_8006B998(s32 first, s32 end, void* point_data, s32 frame, s32 depth)
{
    WmapMovingPoint* points = point_data;
    s32 screen_position;
    s32 i;
    WmapMovingPoint* point;
    WmapMovingPoint* bounds;
    WmapActor* actor;
    u8* data;
    s16* offsets;
    u8* cursor;
    s32 offset;
    s32 animation_frame;

    point = &points[first];
    bounds = &points[end];
    for (i = first; i < end; i++, point++)
    {
        actor = &D_800D9268[i];
        gte_ldv0(&point->position);
        gte_rtps();
        offsets = (s16*)D_80139988[i].data;
        data = (u8*)offsets;
        if (actor->previous_sequence != actor->sequence)
        {
            actor->previous_sequence = (u16)actor->sequence;
            offset = offsets[actor->sequence];
            actor->remaining = 1;
            cursor = (u8*)offsets + offset;
            actor->sequence_start = cursor;
            actor->cursor = cursor;
        }
        if (actor->remaining != 255)
        {
            actor->remaining--;
        }
        if (actor->remaining == 0)
        {
            cursor = actor->cursor;
            animation_frame = cursor[0];
            actor->remaining = cursor[1];
            if (animation_frame == 255)
            {
                cursor = actor->sequence_start;
                actor->cursor = cursor;
                animation_frame = cursor[0];
                actor->remaining = cursor[1];
            }
            actor->cursor += 4;
            actor->frame_data = data + ((s16*)(animation_frame * 2 + data))[32];
        }
        gte_stsxy(&screen_position);
        func_80066F9C(actor, screen_position, frame, depth, 0);
        point->position.vx += point->velocity.vx;
        point->position.vy += point->velocity.vy;
        point->position.vz += point->velocity.vz;
        if (point->position.vx < bounds->position.vx || bounds->velocity.vx < point->position.vx)
        {
            point->velocity.vx = -point->velocity.vx;
        }
        if (point->position.vy < bounds->position.vy || bounds->velocity.vy < point->position.vy)
        {
            point->velocity.vy = -point->velocity.vy;
        }
        if (point->position.vz < bounds->position.vz || bounds->velocity.vz < point->position.vz)
        {
            point->velocity.vz = -point->velocity.vz;
        }
    }
}

/**
 * @brief Advance a particle sequence and draw its active animation frames.
 * @param arg0 Effect parameter; semantics remain unresolved.
 * @param arg1 Effect parameter; semantics remain unresolved.
 * @param arg2 Effect parameter; semantics remain unresolved.
 * @param arg3 Effect parameter; semantics remain unresolved.
 */
void func_8006BC44(s32 arg0, s32 arg1, void* arg2, s32 arg3)
{
    SVECTOR position;
    u32 sp20;
    s16 temp_v0_4;
    s16 temp_v1_2;
    s32 temp_a1_2;
    s32 temp_v0_3;
    s32 var_s2;
    s32 var_s2_2;
    s32 var_s4_2;
    u8 var_v1;
    void* temp_a1;
    void* temp_s1;
    void* temp_v0;
    void* temp_v0_2;
    void* temp_v0_5;
    void* temp_v0_6;
    void* temp_v1;
    void* var_a0;
    void* var_s0;
    void* var_s0_2;
    void* var_s4;
    void* var_s5;
    void* var_v0;

    M2C_FIELD(arg2, s32*, 8) = (s32)(M2C_FIELD(arg2, s32*, 8) - 1);
    var_s2 = arg0;
    if (var_s2 < (var_s2 + arg1))
    {
        var_s5 = (var_s2 * 8) + (u8*)&D_80139988;
        var_s4 = (var_s2 * 0x14) + (u8*)&D_801AFBD0;
        var_s0 = (var_s2 * 0x2C) + (u8*)&D_800D9268;
        do
        {
            position.vx = (s16)((s32)(((s32)M2C_FIELD(var_s4, s32*, 8) >> 3) * (ccos(M2C_FIELD(var_s4, s16*, 2)) >> 6)) >> 0xC);
            position.vy = (s16)((s32)(((s32)M2C_FIELD(var_s4, s32*, 8) >> 3) * (csin(M2C_FIELD(var_s4, s16*, 2)) >> 6)) >> 0xC);
            position.vz = M2C_FIELD(var_s4, u16*, 0xE);
            gte_ldv0(&position);
            gte_rtps();
            M2C_FIELD(var_s4, s32*, 8) = (s32)(M2C_FIELD(var_s4, s32*, 8) - M2C_FIELD(arg2, s32*, 0x10));
            M2C_FIELD(var_s4, s16*, 2) = (s16)((u16)M2C_FIELD(var_s4, s16*, 2) + M2C_FIELD(arg2, u16*, 0x14));
            gte_stsxy(&sp20);
            M2C_FIELD(var_s4, u16*, 0x10) = sp20;
            M2C_FIELD(var_s4, u16*, 0x12) = (u16)M2C_FIELD(&sp20, u16*, 2);
            if (M2C_FIELD(arg2, s32*, 8) == 0)
            {
                var_a0 = var_s0;
                if (M2C_FIELD(arg2, s32*, 0) != -1)
                {
                    temp_a1 = ((var_s2 + M2C_FIELD(arg2, s32*, 4)) * 0x2C) + (u8*)&D_800D9268;
                    var_v0 = temp_a1;
                    do
                    {
                        M2C_FIELD(var_v0, s32*, 0) = (s32)M2C_FIELD(var_a0, s32*, 0);
                        M2C_FIELD(var_v0, s32*, 4) = (s32)M2C_FIELD(var_a0, s32*, 4);
                        M2C_FIELD(var_v0, s32*, 8) = (s32)M2C_FIELD(var_a0, s32*, 8);
                        M2C_FIELD(var_v0, s32*, 0xC) = (s32)M2C_FIELD(var_a0, s32*, 0xC);
                        var_a0 += 0x10;
                        var_v0 += 0x10;
                    } while (var_a0 != (var_s0 + 0x20));
                    M2C_FIELD(var_v0, s32*, 0) = (s32)M2C_FIELD(var_a0, s32*, 0);
                    M2C_FIELD(var_v0, s32*, 4) = (s32)M2C_FIELD(var_a0, s32*, 4);
                    M2C_FIELD(var_v0, s32*, 8) = (s32)M2C_FIELD(var_a0, s32*, 8);
                    temp_v0 = ((var_s2 + M2C_FIELD(arg2, s32*, 4)) * 0x14) + (u8*)&D_801AFBD0;
                    M2C_FIELD(temp_v0, s32*, 0) = (s32)M2C_FIELD(var_s4, s32*, 0);
                    M2C_FIELD(temp_v0, s32*, 4) = (s32)M2C_FIELD(var_s4, s32*, 4);
                    M2C_FIELD(temp_v0, s32*, 8) = (s32)M2C_FIELD(var_s4, s32*, 8);
                    M2C_FIELD(temp_v0, s32*, 0xC) = (s32)M2C_FIELD(var_s4, s32*, 0xC);
                    M2C_FIELD(temp_v0, s32*, 0x10) = (s32)M2C_FIELD(var_s4, u16*, 0x10);
                    temp_v0_2 = ((var_s2 + M2C_FIELD(arg2, s32*, 4)) * 8) + (u8*)&D_80139988;
                    M2C_FIELD(temp_v0_2, s32*, 0) = (s32)M2C_FIELD(var_s5, s32*, 0);
                    M2C_FIELD(temp_v0_2, s32*, 4) = (s32)M2C_FIELD(var_s5, s32*, 4);
                    M2C_FIELD(temp_a1, u16*, 0x26) = (u16)M2C_FIELD(arg2, u16*, 0x18);
                    M2C_FIELD(temp_a1, u16*, 0x22) = (u16)M2C_FIELD(arg2, u16*, 0x1C);
                    if (arg3 == 0)
                    {
                        M2C_FIELD(temp_a1, s16*, 0xE) = 1;
                    }
                }
            }
            var_s5 += 0x78;
            var_s4 += 0x12C;
            var_s2 += 0xF;
            var_s0 += 0x294;
        } while (var_s2 < (arg0 + arg1));
    }
    if (M2C_FIELD(arg2, s32*, 8) == 0)
    {
        temp_v0_3 = M2C_FIELD(arg2, s32*, 4) + 1;
        M2C_FIELD(arg2, s32*, 4) = temp_v0_3;
        M2C_FIELD(arg2, s32*, 8) = (s32)M2C_FIELD(arg2, s32*, 0);
        if (temp_v0_3 >= 0xF)
        {
            M2C_FIELD(arg2, s32*, 4) = 1;
        }
    }
    var_s2_2 = arg0;
    if (var_s2_2 < (var_s2_2 + arg1))
    {
        var_s4_2 = var_s2_2 * 0x14;
        var_s0_2 = (var_s2_2 * 0x2C) + (u8*)&D_800D9268;
        do
        {
            temp_s1 = var_s4_2 + (u8*)&D_801AFBD0;
            if (M2C_FIELD(temp_s1, s16*, 0) != 0)
            {
                temp_a1_2 = M2C_FIELD(((var_s2_2 * 8) + (u8*)&D_80139988), s32*, 4);
                temp_v0_4 = M2C_FIELD(var_s0_2, s16*, 0xE);
                if (M2C_FIELD(var_s0_2, s16*, 0x10) != temp_v0_4)
                {
                    M2C_FIELD(var_s0_2, s16*, 0x10) = (s16)(u16)M2C_FIELD(var_s0_2, s16*, 0xE);
                    M2C_FIELD(var_s0_2, s16*, 0x20) = 1;
                    temp_v1 = temp_a1_2 + *(s16*)((temp_v0_4 * 2) + temp_a1_2);
                    M2C_FIELD(var_s0_2, void**, 0x18) = temp_v1;
                    M2C_FIELD(var_s0_2, void**, 0x14) = temp_v1;
                }
                if (M2C_FIELD(var_s0_2, s16*, 0x20) != 0xFF)
                {
                    M2C_FIELD(var_s0_2, s16*, 0x20) = (s16)((u16)M2C_FIELD(var_s0_2, s16*, 0x20) - 1);
                }
                if (M2C_FIELD(var_s0_2, s16*, 0x20) == 0)
                {
                    temp_v0_5 = M2C_FIELD(var_s0_2, void**, 0x14);
                    var_v1 = M2C_FIELD(temp_v0_5, u8*, 0);
                    M2C_FIELD(var_s0_2, s16*, 0x20) = (s16)M2C_FIELD(temp_v0_5, u8*, 1);
                    if (var_v1 == 0xFF)
                    {
                        temp_v0_6 = M2C_FIELD(var_s0_2, void**, 0x18);
                        M2C_FIELD(var_s0_2, void**, 0x14) = temp_v0_6;
                        var_v1 = M2C_FIELD(temp_v0_6, u8*, 0);
                        M2C_FIELD(var_s0_2, s16*, 0x20) = (s16)M2C_FIELD(temp_v0_6, u8*, 1);
                    }
                    M2C_FIELD(var_s0_2, void**, 0x14) = (void*)(M2C_FIELD(var_s0_2, void**, 0x14) + 4);
                    M2C_FIELD(var_s0_2, s32*, 0x1C) = (s32)(temp_a1_2 + M2C_FIELD(((var_v1 * 2) + temp_a1_2), s16*, 0x40));
                }
                func_80066F9C(var_s0_2, M2C_FIELD(temp_s1, s32*, 0x10), M2C_FIELD(arg2, s32*, 0x24), M2C_FIELD(arg2, s32*, 0x20), 0);
                if ((arg3 != 0) && ((temp_v1_2 = M2C_FIELD(var_s0_2, s16*, 0x24), ((temp_v1_2 < 0xFD) == 0)) || (temp_v1_2 < 4)))
                {
                    M2C_FIELD(temp_s1, s16*, 0) = 0;
                }
            }
            var_s4_2 += 0x14;
            var_s2_2 += 1;
            var_s0_2 += 0x2C;
        } while (var_s2_2 < (arg0 + arg1));
    }
}

/**
 * @brief Advance the map transform effect toward its configured position.
 * @return Sequence continuation flag.
 */
s32 func_8006C0EC(void)
{
    SVECTOR position;
    s16 var_v0;
    s16 var_v0_2;
    s16 var_v0_3;
    s32 var_v0_4;
    s32 var_v0_5;

    func_8006AEE0();
    position.vz = 0;
    position.vx =
        (s16)((s32)((((D_8011D510 - 1) * 0xA0) - ((s32)(M2C_FIELD(&D_80139950, s32*, 0) * 0x14000) / (s32)M2C_FIELD(&D_80139950, s32*, 8))) * 0x6000) /
              (s32)M2C_FIELD(&D_80139950, s32*, 8));
    position.vy =
        (s16)((s32)((((D_8011D530 - 1) * 0xA0) - ((s32)(M2C_FIELD(&D_80139950, s32*, 4) * 0x14000) / (s32)M2C_FIELD(&D_80139950, s32*, 8))) * 0x6000) /
              (s32)M2C_FIELD(&D_80139950, s32*, 8));
    gte_ldv0(&position);
    gte_rtps();
    if (M2C_FIELD(&D_800DCEB8, s16*, 0) != M2C_FIELD(&D_801398C8, s16*, 0))
    {
        if (M2C_FIELD(&D_800DCEB8, s16*, 0) < M2C_FIELD(&D_801398C8, s16*, 0))
        {
            var_v0 = (u16)M2C_FIELD(&D_801398C8, s16*, 0) - D_80139210;
        }
        else
        {
            var_v0 = (u16)M2C_FIELD(&D_801398C8, s16*, 0) + D_80139210;
        }
        M2C_FIELD(&D_801398C8, s16*, 0) = var_v0;
    }
    if (D_800DCEBA != M2C_FIELD(&D_801398C8, s16*, 2))
    {
        if (D_800DCEBA < M2C_FIELD(&D_801398C8, s16*, 2))
        {
            var_v0_2 = (u16)M2C_FIELD(&D_801398C8, s16*, 2) - D_80139212;
        }
        else
        {
            var_v0_2 = (u16)M2C_FIELD(&D_801398C8, s16*, 2) + D_80139212;
        }
        M2C_FIELD(&D_801398C8, s16*, 2) = var_v0_2;
    }
    if (D_800DCEBC != M2C_FIELD(&D_801398C8, s16*, 4))
    {
        if (D_800DCEBC < M2C_FIELD(&D_801398C8, s16*, 4))
        {
            var_v0_3 = (u16)M2C_FIELD(&D_801398C8, s16*, 4) - D_80139214;
        }
        else
        {
            var_v0_3 = (u16)M2C_FIELD(&D_801398C8, s16*, 4) + D_80139214;
        }
        M2C_FIELD(&D_801398C8, s16*, 4) = var_v0_3;
    }
    if (M2C_FIELD(&D_80139200, s32*, 0) != M2C_FIELD(&D_80182D48, s32*, 0))
    {
        if (M2C_FIELD(&D_80139200, s32*, 0) < M2C_FIELD(&D_80182D48, s32*, 0))
        {
            var_v0_4 = M2C_FIELD(&D_80182D48, s32*, 0) - D_80139968;
        }
        else
        {
            var_v0_4 = M2C_FIELD(&D_80182D48, s32*, 0) + D_80139968;
        }
        M2C_FIELD(&D_80182D48, s32*, 0) = var_v0_4;
    }
    if (D_80139204 != M2C_FIELD(&D_80182D48, s32*, 4))
    {
        if (D_80139204 < M2C_FIELD(&D_80182D48, s32*, 4))
        {
            var_v0_5 = M2C_FIELD(&D_80182D48, s32*, 4) - D_8013996C;
        }
        else
        {
            var_v0_5 = M2C_FIELD(&D_80182D48, s32*, 4) + D_8013996C;
        }
        M2C_FIELD(&D_80182D48, s32*, 4) = var_v0_5;
    }
    if (D_8013B29C != 0)
    {
        gte_stsxy(&D_8011CF4C);
        return D_8013B29C;
    }
    M2C_FIELD(&D_801398C8, s16*, 0) = 0;
    M2C_FIELD(&D_801398C8, s16*, 2) = 0;
    M2C_FIELD(&D_801398C8, s16*, 4) = 0;
    M2C_FIELD(&D_80182D48, s32*, 0) = 0;
    M2C_FIELD(&D_80182D48, s32*, 4) = 0;
    M2C_FIELD(&D_80182D48, s32*, 8) = 0;
    M2C_FIELD(&D_80139200, s32*, 0) = 0;
    M2C_FIELD(&D_80139200, s32*, 4) = 0;
    M2C_FIELD(&D_80139200, s32*, 8) = 0;
    M2C_FIELD(&D_800DCEB8, s16*, 0) = 0;
    M2C_FIELD(&D_800DCEB8, s16*, 2) = 0;
    M2C_FIELD(&D_800DCEB8, s16*, 4) = 0;
    M2C_FIELD(&D_8011CF4C, s16*, 0) = 0xA4;
    M2C_FIELD(&D_8011CF4C, s16*, 2) = 0x69;
    return 0;
}

/**
 * @brief Initialize and draw the particle range described by an effect record.
 * @param arg0 Effect parameter; semantics remain unresolved.
 */
void func_8006C448(void* arg0)
{
    SVECTOR position;
    s32 sp20;
    s16 temp_v0_2;
    s32 temp_a0;
    s32 temp_a2;
    s32 temp_v0;
    s32 temp_v1;
    s32 var_s2;
    s32 var_s2_2;
    u8 var_v1;
    void* temp_a0_2;
    void* temp_s0;
    void* temp_s1;
    void* temp_v0_3;
    void* temp_v0_4;
    void* temp_v1_2;
    void* temp_v1_3;

    var_s2 = 0;
    if (M2C_FIELD(arg0, s32*, 0) > 0)
    {
    loop_2:
        temp_v1 = M2C_FIELD(arg0, s32*, 4) + var_s2;
        temp_a0 = temp_v1 * 0x2C;
        temp_s1 = (temp_v1 * 0x14) + (u8*)&D_801AFBD0;
        temp_s0 = temp_a0 + (u8*)&D_800D9268;
        if (M2C_FIELD(temp_s1, s16*, 0) == 0)
        {
            M2C_FIELD(temp_s1, s16*, 0) = 1;
            M2C_FIELD(temp_s1, s16*, 0x10) = (s16)(M2C_FIELD(arg0, u16*, 8) + ((s32)(rand() * M2C_FIELD(arg0, s32*, 0xC)) >> 0xF));
            M2C_FIELD(temp_s1, s16*, 0xE) = (s16)(M2C_FIELD(arg0, u16*, 0x10) + ((s32)(rand() * M2C_FIELD(arg0, s32*, 0x14)) >> 0xF));
            M2C_FIELD(temp_s1, s16*, 0x12) = (s16)(M2C_FIELD(arg0, u16*, 0x18) + ((s32)(rand() * M2C_FIELD(arg0, s32*, 0x1C)) >> 0xF));
            M2C_FIELD(temp_s0, s16*, 2) = 0;
            M2C_FIELD(temp_s0, s8*, 6) = 0xF;
            M2C_FIELD(temp_s0, s16*, 0x10) = -1;
            M2C_FIELD(temp_s0, u16*, 0xE) = (u16)M2C_FIELD(arg0, u16*, 0x34);
            M2C_FIELD(temp_s0, u16*, 0x22) = (u16)M2C_FIELD(arg0, u16*, 0x20);
            M2C_FIELD(temp_s0, u16*, 0x24) = (u16)M2C_FIELD(arg0, u16*, 0x24);
            M2C_FIELD(temp_s0, u16*, 0x26) = (u16)M2C_FIELD(arg0, u16*, 0x28);
        }
        else
        {
            var_s2 += 1;
            if (var_s2 < M2C_FIELD(arg0, s32*, 0))
            {
                goto loop_2;
            }
        }
    }
    var_s2_2 = 0;
    if (M2C_FIELD(arg0, s32*, 0) > 0)
    {
        do
        {
            temp_v0 = M2C_FIELD(arg0, s32*, 4) + var_s2_2;
            temp_v1_2 = (temp_v0 * 0x14) + (u8*)&D_801AFBD0;
            temp_a0_2 = (temp_v0 * 0x2C) + (u8*)&D_800D9268;
            if (M2C_FIELD(temp_v1_2, s16*, 0) != 0)
            {
                position.vx = M2C_FIELD(temp_v1_2, u16*, 0x10);
                position.vy = M2C_FIELD(temp_v1_2, u16*, 0xE);
                position.vz = M2C_FIELD(temp_v1_2, u16*, 0x12);
                gte_ldv0(&position);
                gte_rtps();
                temp_a2 = M2C_FIELD((((var_s2_2 + M2C_FIELD(arg0, s32*, 4)) * 8) + (u8*)&D_80139988), s32*, 4);
                temp_v0_2 = (s16)M2C_FIELD(temp_a0_2, u16*, 0xE);
                if (M2C_FIELD(temp_a0_2, s16*, 0x10) != temp_v0_2)
                {
                    M2C_FIELD(temp_a0_2, s16*, 0x10) = (s16)M2C_FIELD(temp_a0_2, u16*, 0xE);
                    M2C_FIELD(temp_a0_2, s16*, 0x20) = 1;
                    temp_v1_3 = temp_a2 + *(s16*)((temp_v0_2 * 2) + temp_a2);
                    M2C_FIELD(temp_a0_2, void**, 0x18) = temp_v1_3;
                    M2C_FIELD(temp_a0_2, void**, 0x14) = temp_v1_3;
                }
                if (M2C_FIELD(temp_a0_2, s16*, 0x20) != 0xFF)
                {
                    M2C_FIELD(temp_a0_2, s16*, 0x20) = (s16)((u16)M2C_FIELD(temp_a0_2, s16*, 0x20) - 1);
                }
                if (M2C_FIELD(temp_a0_2, s16*, 0x20) == 0)
                {
                    temp_v0_3 = M2C_FIELD(temp_a0_2, void**, 0x14);
                    var_v1 = M2C_FIELD(temp_v0_3, u8*, 0);
                    M2C_FIELD(temp_a0_2, s16*, 0x20) = (s16)M2C_FIELD(temp_v0_3, u8*, 1);
                    if (var_v1 == 0xFF)
                    {
                        temp_v0_4 = M2C_FIELD(temp_a0_2, void**, 0x18);
                        M2C_FIELD(temp_a0_2, void**, 0x14) = temp_v0_4;
                        var_v1 = M2C_FIELD(temp_v0_4, u8*, 0);
                        M2C_FIELD(temp_a0_2, s16*, 0x20) = (s16)M2C_FIELD(temp_v0_4, u8*, 1);
                    }
                    M2C_FIELD(temp_a0_2, void**, 0x14) = (void*)(M2C_FIELD(temp_a0_2, void**, 0x14) + 4);
                    M2C_FIELD(temp_a0_2, s32*, 0x1C) = (s32)(temp_a2 + M2C_FIELD(((var_v1 * 2) + temp_a2), s16*, 0x40));
                }
                gte_stsxy(&sp20);
                func_80066F9C(temp_a0_2, sp20, M2C_FIELD(arg0, s32*, 0x30), M2C_FIELD(arg0, s32*, 0x2C), 0);
            }
            var_s2_2 += 1;
        } while (var_s2_2 < M2C_FIELD(arg0, s32*, 0));
    }
}
