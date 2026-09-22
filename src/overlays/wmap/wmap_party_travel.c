#include "wmap_map_display.h"
#include "wmap_party_travel.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
#include "cdrom.h"

void func_800584B4(void)
{
/* Partial WMAP decompilation: 83.588780% (gcc280_g0). */

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))

M2C_UNK akao_cmd_f1();                              /* extern */
M2C_UNK func_80059070__for_func_800584B4() __asm__("func_80059070");                            /* extern */
extern u8 D_800D9268;
extern s32 D_800DCEE8;
extern s32 D_8011CF44;
extern s32 D_80139230;
extern s32 D_801398D0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern s32 D_80182DF8;
extern u8 D_8019D248;
extern s32 D_801ADB00;

    void *var_a1;
    void *var_a3;
    s16 temp_a0;
    s16 temp_a0_2;
    s16 temp_a0_3;
    s16 temp_a0_6;
    s16 temp_a0_7;
    s16 temp_t0;
    s16 temp_v0;
    s16 temp_v0_2;
    s16 temp_v0_3;
    s16 temp_v1_4;
    s16 temp_v1_5;
    s16 var_a2;
    s16 var_v0;
    s16 var_v0_2;
    s32 temp_a0_4;
    s32 temp_v1_6;
    s32 temp_v1_7;
    s32 var_t1;
    s32 var_t2;
    u16 temp_v0_4;
    u16 temp_v1;
    u16 temp_v1_2;
    u16 temp_v1_3;
    void *temp_a0_5;
    void *temp_t6;

    var_t1 = 0;
    D_80139230 = 0;
    temp_t6 = (u8 *)&D_8019D248 + 0x36C;
    var_a3 = &D_800D9268;
    var_a1 = &D_8019D248;
    var_t2 = 0;
    do
    {
        temp_a0 = M2C_FIELD(var_a1, s16 *, 0xC);
        temp_v0 = M2C_FIELD(var_a1, s16 *, 0x10);
        temp_v1 = (u16) M2C_FIELD(var_a1, s16 *, 0xC);
        var_a2 = 0;
        if (temp_a0 != temp_v0)
        {
            var_a2 = 1;
            if (temp_v0 < temp_a0)
            {
                var_v0 = temp_v1 - 4;
            }
            else
            {
                var_v0 = temp_v1 + 4;
            }
            M2C_FIELD(var_a1, s16 *, 0xC) = var_v0;
        }
        temp_a0_2 = M2C_FIELD(var_a1, s16 *, 0xE);
        temp_v0_2 = M2C_FIELD(var_a1, s16 *, 0x12);
        temp_v1_2 = (u16) M2C_FIELD(var_a1, s16 *, 0xE);
        if (temp_a0_2 != temp_v0_2)
        {
            var_a2 = 1;
            if (temp_v0_2 < temp_a0_2)
            {
                var_v0_2 = temp_v1_2 - 4;
            }
            else
            {
                var_v0_2 = temp_v1_2 + 4;
            }
            M2C_FIELD(var_a1, s16 *, 0xE) = var_v0_2;
        }
        if (var_a2 == 0)
        {
            if (((M2C_FIELD(var_a1, s32 *, 0) != M2C_FIELD(var_a1, s16 *, 8)) || (M2C_FIELD(var_a1, s32 *, 4) != M2C_FIELD(var_a1, s16 *, 0xA))) && (var_t1 == 0))
            {
                D_800DCEE8 = 1;
            }
            temp_v0_3 = M2C_FIELD(var_a1, s16 *, 8);
            temp_a0_3 = M2C_FIELD(var_a1, s16 *, 0xA);
            M2C_FIELD(var_a1, s32 *, 0) = (s32) temp_v0_3;
            M2C_FIELD(var_a1, s32 *, 4) = (s32) temp_a0_3;
            if ((M2C_FIELD(var_a1, s32 *, 0x18) != temp_v0_3) || (M2C_FIELD(var_a1, s32 *, 0x1C) != temp_a0_3))
            {
                temp_t0 = M2C_FIELD(var_a1, s16 *, 8);
                var_a2 = M2C_FIELD(var_a1, s16 *, 0xA);
                temp_a0_4 = M2C_FIELD(var_a1, s32 *, 0x20) + 1;
                M2C_FIELD(var_a1, s32 *, 0x20) = temp_a0_4;
                temp_a0_5 = (temp_a0_4 * 4) + var_t2 + (u8 *)&D_8019D248;
                temp_v1_3 = M2C_FIELD(temp_a0_5, u16 *, 0x24);
                M2C_FIELD(var_a1, s16 *, 8) = (s16) temp_v1_3;
                M2C_FIELD(var_a1, s16 *, 0x10) = (s16) (((s16) temp_v1_3 - 1) * 0xA0);
                temp_v0_4 = M2C_FIELD(temp_a0_5, u16 *, 0xA4);
                M2C_FIELD(var_a1, s16 *, 0xA) = (s16) temp_v0_4;
                M2C_FIELD(var_a1, s16 *, 0x12) = (s16) (((s16) temp_v0_4 - 1) * 0xA0);
                if ((D_80182DF8 != 0) && (var_t1 == 3))
                {
                    D_801398D0 = 2;
                    D_80182D68 = (M2C_FIELD(temp_t6, s16 *, 8) - temp_t0) * 0x30;
                    D_80182D78 = (M2C_FIELD(temp_t6, s16 *, 0xA) - var_a2) * 0x30;
                }
                M2C_FIELD(var_a1, s32 *, 0x14) = 1;
            }
            else
            {
                M2C_FIELD(var_a1, s32 *, 0x14) = 0;
                M2C_FIELD(var_a3, s16 *, 0xE) = 0;
                if ((var_t1 == 3) && (D_80182DF8 != 0))
                {
                    D_80182DF8 = 0;
                }
            }
        }
        if (M2C_FIELD(var_a1, s32 *, 0x14) != 0)
        {
            if (M2C_FIELD(var_a1, s16 *, 0x10) == M2C_FIELD(var_a1, s16 *, 0xC))
            {
                temp_a0_6 = M2C_FIELD(var_a1, s16 *, 0x12);
                temp_v1_4 = M2C_FIELD(var_a1, s16 *, 0xE);
                if (temp_v1_4 < temp_a0_6)
                {
                    M2C_FIELD(var_a3, s16 *, 0xE) = 1;
                }
                else if (temp_a0_6 < temp_v1_4)
                {
                    M2C_FIELD(var_a3, s16 *, 0xE) = 4;
                }
            }
            if (M2C_FIELD(var_a1, s16 *, 0x12) == M2C_FIELD(var_a1, s16 *, 0xE))
            {
                temp_a0_7 = M2C_FIELD(var_a1, s16 *, 0x10);
                temp_v1_5 = M2C_FIELD(var_a1, s16 *, 0xC);
                if (temp_v1_5 < temp_a0_7)
                {
                    M2C_FIELD(var_a3, s16 *, 0xE) = 2;
                }
                else if (temp_a0_7 < temp_v1_5)
                {
                    M2C_FIELD(var_a3, s16 *, 0xE) = 3;
                }
            }
        }
        var_a3 += 0x2C;
        temp_v1_6 = M2C_FIELD(var_a1, s32 *, 0x14);
        var_a1 += 0x124;
        var_t2 += 0x124;
        var_t1 += 1;
        temp_v1_7 = D_80139230 | temp_v1_6;
        D_80139230 = temp_v1_7;
    } while (var_t1 < 4);
    if (temp_v1_7 != D_801ADB00)
    {
        if ((temp_v1_7 != 0) && (D_8011CF44 == 0))
        {
            func_800652A8(0x15, 0x80);
            D_801ADB00 = D_80139230;
        }
        if ((D_80139230 != D_801ADB00) && (D_80139230 == 0))
        {
            akao_cmd_f1();
            D_801ADB00 = D_80139230;
        }
    }
    if (D_800DCEE8 != 0)
    {
        func_80059070__for_func_800584B4();
        D_800DCEE8 = 0;
    }
}
#undef M2C_FIELD
#undef M2C_UNALIGNED32
#undef M2C_BITWISE

void func_8005880C(void)
{
/* Partial WMAP decompilation: 85.668540% (gcc280_g0). */

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))

M2C_UNK func_8005EB68(s32, s32, s32, s32, void *, void *); /* extern */
M2C_UNK func_800584B4__for_func_8005880C() __asm__("func_800584B4");                            /* static */
extern u8 D_8004FD04;
extern u8 D_800D9268;
extern s32 D_800DCEEC;
extern s32 D_800DCEF0;
extern s32 D_8011CF18;
extern s32 D_80129550;
extern s32 D_8013922C;
extern s32 D_80139230;
extern u8 D_80139290;
extern s32 D_8013986C;
extern u8 D_80139950;
extern u8 D_80139988;
extern s32 D_8013B27C;
extern s32 D_8013B294;
extern u8 D_8019D248;
extern u8 func_8009A420;

    SVECTOR position;
    s32 sp24;
    s32 sp20;
    void *var_s0;
    s32 temp_a2;
    s32 temp_a3;
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v0_3;
    s32 temp_v0_4;
    s32 var_s1;
    s32 var_s3;
    s32 var_s4;
    s32 var_s5;
    s32 var_s6;
    s32 var_v0;
    s32 var_v1;
    void *temp_s2;

    if (D_8013B27C != 0)
    {
        var_s3 = 0;
        var_s4 = 0x100;
        var_s0 = &D_8019D248;
        var_s6 = 0;
        var_s5 = 0;
        do
        {
            temp_s2 = var_s5 + (u8 *)&D_800D9268;
            if (M2C_FIELD(temp_s2, s16 *, 2) != -1)
            {
                func_8006CC4C(temp_s2, var_s6 + (u8 *)&D_80139988);
                if (D_8013986C == 0)
                {
                    temp_v0 = (sp20 & 0xFFFF0000) | M2C_FIELD(var_s0, u16 *, 0xC);
                    sp20 = temp_v0;
                    sp20 = (temp_v0 & 0xFFFF) | (M2C_FIELD(var_s0, u16 *, 0xE) << 0x10);
                    position.vx = (s16) ((s32) (((s16) M2C_FIELD(var_s0, u16 *, 0xC) - (((s32) (M2C_FIELD(&D_80139950, s32 *, 0) * 0x14000) / (s32) M2C_FIELD(&D_80139950, s32 *, 8)) - 0x14)) * 0x6000) / (s32) M2C_FIELD(&D_80139950, s32 *, 8));
                    position.vz = 0;
                    position.vy = (s16) ((s32) (((s16) M2C_FIELD(var_s0, u16 *, 0xE) - (((s32) (M2C_FIELD(&D_80139950, s32 *, 4) * 0x14000) / (s32) M2C_FIELD(&D_80139950, s32 *, 8)) - 0x14)) * 0x6000) / (s32) M2C_FIELD(&D_80139950, s32 *, 8));
                    gte_ldv0(&position);
    gte_rtps();
                    gte_stsxy(&sp20);
                    gte_stsz(&sp24);
                    sp24 >>= 2;
                    var_v0 = 0x1B91 - sp24;
                    if (var_v0 < 0)
                    {
                        var_v0 += 3;
                    }
                    temp_v0_2 = var_v0 >> 2;
                    var_s1 = temp_v0_2 + 0x2A;
                    if ((u32) (temp_v0_2 + 0xB) >= 0x90U)
                    {
                        var_s1 = 0x1F;
                    }
                    if (wmap_get_point_display_mode((((s16) M2C_FIELD(var_s0, u16 *, 0xC) * 0x30) / 160) + 0x30, (((s16) M2C_FIELD(var_s0, u16 *, 0xE) * 0x30) / 160) + 0x30, M2C_FIELD(&D_80139950, s32 *, 8)) != 0)
                    {
                        func_80066F9C(temp_s2, sp20, var_s3, var_s1, var_s4);
                    }
                }
                if (D_8013986C == 1)
                {
                    temp_v0_3 = *(s32 *)(((M2C_FIELD(var_s0, s32 *, 0) + (M2C_FIELD(var_s0, s32 *, 4) * 6)) * 4) + (u8 *)&D_8004FD04);
                    temp_v0_4 = (temp_v0_3 & 0xFFFF0000) | ((temp_v0_3 + 0xE) & 0xFFFF);
                    func_80066F9C(temp_s2, (temp_v0_4 & 0xFFFF) | (((temp_v0_4 >> 0x10) + 0x1C) << 0x10), var_s3, 0x1F, var_s4);
                }
            }
            var_s4 += 0x100;
            var_s0 += 0x124;
            var_s6 += 8;
            var_s3 += 1;
            var_s5 += 0x2C;
        } while (var_s3 < 4);
    }
    if ((D_8013922C & 0x40) && (D_80139230 == 0) && (D_80129550 == 0))
    {
        temp_a2 = (M2C_FIELD(&D_80139950, s32 *, 0) / 48) + D_800DCEEC;
        temp_a3 = (M2C_FIELD(&D_80139950, s32 *, 4) / 48) + D_800DCEF0;
        if ((M2C_FIELD(((temp_a3 * 0x28) + (temp_a2 * 0xF0) + (u8 *)&D_80139290), s16 *, 6) != 0) && (D_8013986C == 0) && (D_8011CF18 == 0))
        {
            if ((temp_a2 != M2C_FIELD(&D_8019D248, s32 *, 0)) || (var_v1 = 1, (temp_a3 != M2C_FIELD(&D_8019D248, s32 *, 4))))
            {
                var_v1 = 0;
            }
            D_8013B294 = var_v1;
            if (var_v1 == 0)
            {
                if (*((temp_a3 * 0x28) + (temp_a2 * 0xF0) + (u8 *)&D_80139290) == 0x18)
                {
                    func_8006CAC0(&func_8009A420);
                }
                else
                {
                    M2C_FIELD(&D_8019D248, s32 *, 0x18) = temp_a2;
                    M2C_FIELD(&D_8019D248, s32 *, 0x1C) = temp_a3;
                    if ((temp_a2 != M2C_FIELD(&D_8019D248, s32 *, 0)) || (temp_a3 != M2C_FIELD(&D_8019D248, s32 *, 4)))
                    {
                        func_8005EB68(M2C_FIELD(&D_8019D248, s32 *, 0), M2C_FIELD(&D_8019D248, s32 *, 4), temp_a2, temp_a3, (u8 *)&D_8019D248 + 0x24, (u8 *)&D_8019D248 + 0xA4);
                        M2C_FIELD(&D_8019D248, s32 *, 0x14) = 1;
                        M2C_FIELD(&D_8019D248, s32 *, 0x20) = 1;
                        M2C_FIELD(&D_8019D248, u16 *, 8) = (u16) M2C_FIELD(&D_8019D248, u16 *, 0x28);
                        M2C_FIELD(&D_8019D248, u16 *, 0xA) = (u16) M2C_FIELD(&D_8019D248, u16 *, 0xA8);
                        M2C_FIELD(&D_8019D248, s16 *, 0x10) = (s16) (((s16) M2C_FIELD(&D_8019D248, u16 *, 0x28) - 1) * 0xA0);
                        M2C_FIELD(&D_8019D248, s16 *, 0x12) = (s16) (((s16) M2C_FIELD(&D_8019D248, u16 *, 0xA8) - 1) * 0xA0);
                    }
                    else
                    {
                        M2C_FIELD(&D_8019D248, s32 *, 0x14) = 0;
                    }
                }
            }
        }
    }
    func_800584B4__for_func_8005880C();
}
#undef M2C_FIELD
#undef M2C_UNALIGNED32
#undef M2C_BITWISE

/** @brief Load map actor resources and restore the active map coordinates. */
void func_80058D9C(void)
{
/* Partial WMAP decompilation: 97.346664% (gcc280_g0). */

/** @brief Coordinate fields at the head of a 0x124-byte world-map record. */
typedef struct
{
    s32 x;
    s32 y;
    s16 grid_x;
    s16 grid_y;
    s16 position_x;
    s16 position_y;
    s16 target_x;
    s16 target_y;
    s32 field_14;
    s32 previous_x;
    s32 previous_y;
    u8 unknown_20[0x104];
} WmapCoordinateRecord;



/** @brief World-map actor configuration. */
typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad_04[2];
    u8 field_06;
    u8 pad_07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad_12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad_28[4];
} WmapConfigA;

/** @brief Per-actor motion and animation parameters. */
typedef struct
{
    s16 state;
    s16 angle;
    s32 x;
    s32 z;
    s16 scale;
    s16 field_0E;
    s32 field_10;
} WmapMotion;

/** @brief Animation resource slot. */
typedef struct
{
    s32 field_00;
    void *resource;
} WmapResource;

typedef struct
{
    s32 tile;
    u8 pad_04[36];
} WmapTile;
extern WmapCoordinateRecord D_8019D248[];
extern WmapConfigA D_800D9268[];
extern WmapResource D_80139988[];
extern WmapTile D_80139290[6][6];
extern u8 D_800DBE98[];
extern u8 D_800DC298[];
extern u8 D_800DC698[];
extern s16 D_800D926A;
extern s16 D_800D9296;
extern s16 D_800D92C2;
extern s32 func_8005D850(s32 *, u32 *);
extern void func_80058FF4__for_func_80058D9C(s32, s32, s32) __asm__("func_80058FF4");
extern s32 D_800D9224;
extern s32 D_800DBE78;
extern s32 D_8011CF20;
extern s32 D_8011CF50;
extern s32 D_8013922C;
extern s32 D_801398B8;
extern s32 D_801398C0;
extern s32 D_80182D5C;
extern s32 D_80182E1C;
extern s32 D_80182E34;

    s32 first_x, first_y, second_x, second_y;
    s32 i;
    s32 resource_id;
    u8 *resource;
    WmapCoordinateRecord *record;
    WmapConfigA *actor;

    cdrom_queue_read(0x10C9, D_800DBE98);
    cdrom_wait_queue_empty();
    resource = D_800DBE98;
    D_800D9268[0].field_02 = 0;
    for (i = 0; i < 4; i++)
    {
        record = &D_8019D248[i];
        actor = &D_800D9268[i];
        record->y = 1;
        record->x = 1;
        record->grid_y = 1;
        record->grid_x = 1;
        record->previous_y = 1;
        record->previous_x = 1;
        record->target_y = 0;
        record->target_x = 0;
        record->position_y = 0;
        record->position_x = 0;
        record->field_14 = 0;
        actor->field_06 = 15;
        actor->field_10 = -1;
        actor->field_22 = 128;
        actor->field_24 = 128;
        D_80139988[i].resource = resource;
        resource += 0x400;
    }
    D_80182D5C = func_8005D850(&D_8019D248[0].x, (u32 *)&D_8019D248[0].y);
    func_80058FF4__for_func_80058D9C(0, D_8019D248[0].x, D_8019D248[0].y);
    if (D_80139290[D_8019D248[0].x][D_8019D248[0].y].tile == 24)
    {
        D_8011CF50 = 1;
        D_801398C0 = 0;
        D_8013922C = 0;
        D_80182E34 = 3;
        D_800DBE78 = 3;
        D_8011CF20 = 1;
        D_800D926A = -1;
        D_800D9224++;
    }
    resource_id = 0x10CA;
    if (D_80182D5C != 0)
    {
        resource_id = 0x10CB;
    }
    func_80064F64(resource_id);
    if (D_801398B8 != 0)
    {
        cdrom_queue_read(0x10CC, D_800DC298);
        D_800D9296 = 1;
        func_8006D0F0(27, &first_x, &first_y);
        func_80058FF4__for_func_80058D9C(1, first_x, first_y);
        cdrom_wait_queue_empty();
    }
    if (D_80182E1C != 0)
    {
        cdrom_queue_read(0x10CD, D_800DC698);
        D_800D92C2 = 2;
        func_8006D0F0(3, &second_x, &second_y);
        func_80058FF4__for_func_80058D9C(2, second_x, second_y);
        cdrom_wait_queue_empty();
    }
}

/**
 * @brief Initialize grid coordinates and corresponding pixel positions.
 * @param index World-map record index.
 * @param x Grid X coordinate.
 * @param y Grid Y coordinate.
 */
void func_80058FF4(s32 index, s32 x, s32 y)
{
/** @brief Coordinate fields at the head of a 0x124-byte world-map record. */
typedef struct
{
    s32 x;
    s32 y;
    s16 grid_x;
    s16 grid_y;
    s16 position_x;
    s16 position_y;
    s16 target_x;
    s16 target_y;
    u8 unknown_14[4];
    s32 previous_x;
    s32 previous_y;
    u8 unknown_20[0x104];
} WmapCoordinateRecord;

extern WmapCoordinateRecord D_8019D248[];

    s16 pixel_x;
    s16 pixel_y;
    WmapCoordinateRecord *record;
    WmapCoordinateRecord *base;

    base = D_8019D248;
    record = &base[index];
    record->x = x;
    record->previous_x = x;
    record->grid_x = (s16) x;
    pixel_x = ((s16) x - 1) * 0xA0;
    record->y = y;
    record->previous_y = y;
    record->grid_y = (s16) y;
    record->target_x = pixel_x;
    record->position_x = pixel_x;
    pixel_y = ((s16) y - 1) * 0xA0;
    record->target_y = pixel_y;
    record->position_y = pixel_y;
}

/** @brief Refresh world-map state and cache its saved seven-bit value. */
void func_80059070(void)
{
extern void func_8005B58C(void);
extern s32 func_8005D494(void);
extern s32 D_8013B274;

    func_8005B58C();
    D_8013B274 = func_8005D494();
}
