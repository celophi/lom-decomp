#include "wmap_party_travel.h"
#include "wmap_travel_sequences.h"
#include "wmap_resource_support.h"
#include "wmap_sequence_runtime.h"
#include "wmap_sprite_render.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
#include "wmap_map_labels.h"
#include "wmap_effect_resources.h"
#include "cdrom.h"

/**
 * @brief Select the animation and resource bank from the current rotation.
 * @param advance Nonzero to advance the rotation by 32 units.
 * @return Actor animation state.
 */
WmapAnimation *func_80099754(s32 advance)
{
/* Partial WMAP decompilation: 97.459015% (gcc280_g0). */

/** @brief Actor animation state; only sequence and previous-sequence fields are changed. */

extern WmapAnimation D_800DBE3C;
typedef struct
{
    s16 field_00;
    u16 angle;
} WmapRotation;

extern WmapRotation D_801AFBD0;
extern s16 D_801AFBDE;
extern s16 D_801AFBE0;
extern u8 D_8011D538[];
extern u8 D_8011F538[];
extern u8 *D_8013A184;
extern s32 D_801B2C48;

    s32 angle;
    s32 rotated_angle;
    WmapAnimation *actor;
    s16 position;
    WmapRotation *rotation;

    actor = &D_800DBE3C;
    if (advance != 0)
    {
        rotation = &D_801AFBD0;
        angle = rotation->angle + 32;
    }
    else
    {
        rotation = &D_801AFBD0;
        angle = rotation->angle;
    }
    angle &= 0xFFF;
    rotation->angle = angle;
    rotated_angle = angle + 0x200;
    actor->sequence = (rotated_angle >> 9) & 3;
    if (rotated_angle & 0x800)
    {
        if (D_801B2C48 == 0)
        {
            actor->previous_sequence = -1;
        }
        D_8013A184 = D_8011F538;
        D_801B2C48 = 1;
    }
    else
    {
        if (D_801B2C48 != 0)
        {
            actor->previous_sequence = -1;
        }
        D_8013A184 = D_8011D538;
        D_801B2C48 = 0;
    }
    position = 50;
    if ((u32)(angle - 0x401) < 0x7FFU && D_801AFBDE < 40)
    {
        position = 61;
    }
    D_801AFBE0 = position;
    return actor;
}

/** @brief World-map step: reset the actor and its slot, register a callback, advance. */
void func_80099848(void)
{
/** @brief World-map actor record reset when this step spawns it. */
typedef struct
{
    u8 pad_00[2];
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
    u8 pad_28[2];
} WmapActor;

/** @brief World-map slot record populated when spawning this actor. */
typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    u8 pad_0C[2];
    s16 field_0E;
    s16 field_10;
    u8 pad_12[2];
} WmapSlotB;

extern WmapActor D_800DBE3C;
extern u8 D_80182E40[];
extern u8 D_8018B240[];
extern void *D_8013A184;
extern s32 D_8011D538;
extern WmapSlotB D_801AFBD0[];
extern s32 D_801B2C4C;
extern s32 D_801B2C50;
extern void func_800999D0__for_func_80099848(void) __asm__("func_800999D0");
extern void func_80099918__for_func_80099848(void) __asm__("func_80099918");

    WmapActor *actor = &D_800DBE3C;

    cdrom_wait_queue_empty();
    func_800651B4(D_80182E40);
    func_800651B4(D_8018B240);
    D_8013A184 = &D_8011D538;
    actor->field_10 = -1;
    actor->field_22 = 0x80;
    actor->field_24 = 0x80;
    actor->field_02 = 0;
    actor->field_06 = 0;
    actor->field_0E = 0;
    actor->field_26 = 0;
    D_801AFBD0[0].field_00 = 1;
    D_801AFBD0[0].field_08 = 0xC8;
    D_801AFBD0[0].field_0E = 0xA;
    D_801AFBD0[0].field_02 = 0;
    D_801AFBD0[0].field_10 = 0x3C;
    D_801AFBD0[0].field_04 = 0;
    func_8006CBD8(func_800999D0__for_func_80099848);
    D_801B2C50 = 0x3C;
    D_801B2C4C += 1;
    func_80099918__for_func_80099848();
}

/**
 * @brief World-map actor tick: advance timers, bump a wave index, and expire the step.
 */
void func_80099918(void)
{
extern s8 *func_80099754__for_func_80099918(s32 arg0) __asm__("func_80099754");
extern u8 D_801AFBD0[];
extern s32 D_8011CF74;
extern s32 D_801B2C4C;
extern s32 D_801B2C50;

    s8 *obj;
    u8 *base;

    obj = func_80099754__for_func_80099918(1);
    base = D_801AFBD0;
    if (*(s16 *)(base + 0xE) < 100)
    {
        *(s16 *)(base + 0xE) += 1;
    }
    if (*(s32 *)(base + 0x8) < 0x1E0)
    {
        *(s32 *)(base + 0x8) += 4;
    }
    if ((D_8011CF74 & 3) == 0)
    {
        if (obj[6] < 0xF)
        {
            obj[6] += 1;
        }
    }
    if (--D_801B2C50 == 0)
    {
        D_801B2C4C += 1;
    }
}

/** @brief Project and draw the effect actor, playing sounds at selected frames.
 * @return Actor motion state.
 */
s16 func_800999D0(void)
{
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
    s16 field_10;
    s16 field_12;
} WmapMotion;

/** @brief Animation resource slot. */
typedef struct
{
    s32 field_00;
    void *resource;
} WmapResource;

extern WmapConfigA D_800DBE3C;
extern s32 D_8011CF54;
extern MATRIX D_8011D0E8;
extern s32 D_80139224;
extern WmapResource D_8013A180;
extern WmapMotion D_801AFBD0;

    SVECTOR position;
    s32 angle;
    WmapConfigA *actor = &D_800DBE3C;
    s32 animation_frame;

    angle = D_801AFBD0.angle;
    PushMatrix();
    SetRotMatrix(&D_8011D0E8);
    SetTransMatrix(&D_8011D0E8);
    position.vx = ((D_801AFBD0.z >> 4) * (ccos(angle) >> 4)) >> 6;
    position.vy = ((D_801AFBD0.z >> 4) * (csin(angle) >> 4)) >> 6;
    position.vz = D_801AFBD0.field_0E;
    gte_ldv0(&position);
    gte_rtps();
    animation_frame = func_8006CC4C(actor, &D_8013A180);
    if (D_80139224 != 0)
    {
        if (animation_frame == 12 || animation_frame == 22 || animation_frame == 0)
        {
            func_800652A8(51, 127);
        }
    }
    gte_stsxy(&D_8011CF54);
    if (D_801AFBD0.x != 0)
    {
        func_80066F9C(actor, D_8011CF54, 40, D_801AFBD0.x, 2);
    }
    else
    {
        func_80066F9C(actor, D_8011CF54, 40, D_801AFBD0.field_10, 2);
    }
    PopMatrix();
    return D_801AFBD0.state;
}

s32 func_80099B50(s32 arg3)
{
/* Partial WMAP decompilation: 82.702440% (gcc280_g0). */

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))

extern s32 D_800D9164;
extern s32 D_800DCED8;
extern s32 D_800DCEE4;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_801398D0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern s32 D_8019D240;
extern u8 func_80099E84__for_func_80099B50 __asm__("func_80099E84");
extern u8 func_80099F98__for_func_80099B50 __asm__("func_80099F98");

    void *var_a0_3;
    s32 *var_a1;
    s32 temp_a2;
    s32 temp_a3;
    s32 temp_t1;
    s32 temp_v1;
    s32 temp_v1_2;
    s32 var_a0;
    s32 var_a0_2;
    s32 var_a2;
    s32 var_a3;
    s32 var_v0;
    s32 var_v0_2;
    s32 var_v0_3;
    s32 var_v0_4;
    s32 var_v0_5;
    s32 var_v0_6;
    s32 var_v0_7;
    s32 var_v1;

    var_a3 = arg3;
    D_8019D240 = -1;
    D_800D9164 = 0;
    if (D_800DCEF8 == D_800DCED8)
    {
        if (D_800DCF00 == D_800DCEE4)
        {
            goto block_12;
        }
        goto block_4;
    }
    if (D_800DCF00 == D_800DCEE4)
    {
block_4:
        var_a1 = &D_800DCF00;
        temp_a2 = D_800DCED8 - D_800DCEF8;
        var_a0 = D_800DCEE4 - D_800DCF00;
        if (temp_a2 != 0)
        {
            var_v0 = temp_a2;
            if (temp_a2 < 0)
            {
                var_v0 = -var_v0;
            }
            var_a2 = (temp_a2 / var_v0) + 1;
        }
        else
        {
            var_a2 = 1;
        }
        if (var_a0 == 0)
        {
            var_a0_2 = 1;
        }
        else
        {
            goto block_21;
        }
        goto block_25;
    }
block_12:
    var_a2 = D_800DCED8;
    temp_v1 = var_a2 - D_800DCEF8;
    var_a0 = D_800DCEE4 - D_800DCF00;
    var_a1 = (s32 *) temp_v1;
    if (temp_v1 < 0)
    {
        var_a1 = (s32 *) -(s32) var_a1;
    }
    var_v0_2 = var_a0;
    if (var_a0 < 0)
    {
        var_v0_2 = -var_v0_2;
    }
    if (var_a1 == var_v0_2)
    {
        if (temp_v1 != 0)
        {
            var_a2 = (temp_v1 / (s32) var_a1) + 1;
        }
        else
        {
            var_a2 = 1;
        }
        if (var_a0 != 0)
        {
block_21:
            var_v0_3 = var_a0;
            if (var_a0 < 0)
            {
                var_v0_3 = -var_v0_3;
            }
            var_a0_2 = (var_a0 / var_v0_3) + 1;
        }
        else
        {
            var_a0_2 = 1;
        }
block_25:
        D_8019D240 = var_a2 + (var_a0_2 * 3);
    }
    if (D_8019D240 != -1)
    {
        D_801398D0 = 2;
        var_a0_3 = &func_80099E84__for_func_80099B50;
    }
    else
    {
        var_a2 = D_800DCED8 - D_800DCEF8;
        temp_t1 = D_800DCEE4 - D_800DCF00;
        var_v1 = var_a2;
        if (var_a2 < 0)
        {
            var_v1 = -var_v1;
        }
        var_v0_4 = temp_t1;
        if (temp_t1 < 0)
        {
            var_v0_4 = -var_v0_4;
        }
        temp_a3 = var_v1 - var_v0_4;
        if (temp_a3 > 0)
        {
            var_v0_5 = 0;
            if (var_a2 != 0)
            {
                var_v0_5 = -1;
                if (var_a2 > 0)
                {
                    var_v0_5 = 1;
                }
            }
            var_a1 = &D_80182D68;
            D_80182D78 = 0;
            var_a3 = temp_a3 * var_v0_5;
            D_80182D68 = var_a3 * 0x30;
            D_800DCEF8 += var_a3;
            if (var_a2 > 0)
            {
                var_v0_6 = 5;
            }
            else
            {
                var_v0_6 = 3;
            }
        }
        else
        {
            temp_v1_2 = D_800DCF00 - D_800DCEE4;
            var_v0_7 = 0;
            if (temp_v1_2 != 0)
            {
                var_v0_7 = -1;
                if (temp_v1_2 > 0)
                {
                    var_v0_7 = 1;
                }
            }
            var_a1 = &D_80182D78;
            D_80182D68 = 0;
            var_a3 = temp_a3 * var_v0_7;
            D_80182D78 = var_a3 * 0x30;
            D_800DCF00 += var_a3;
            if (temp_t1 > 0)
            {
                var_v0_6 = 7;
            }
            else
            {
                var_v0_6 = 1;
            }
        }
        D_8019D240 = var_v0_6;
        var_a0_3 = &func_80099F98__for_func_80099B50;
    }
    func_8006CBD8(var_a0_3);
    return 0;
}
#undef M2C_FIELD
#undef M2C_UNALIGNED32
#undef M2C_BITWISE

/**
 * @brief Start map movement after the actor faces its route, then wait for completion.
 * @return One while movement is pending, zero after arrival.
 */
s32 func_80099E84(void)
{
extern void func_80099754__for_func_80099E84(s32) __asm__("func_80099754");
extern s16 D_800D6508[];
extern s32 D_800D9164;
extern s32 D_800DCED8;
extern s32 D_800DCEE4;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_801398AC;
extern s32 D_801398D0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern s32 D_8019D240;
extern s16 D_801AFBD2;

    if (D_800D9164 == 0)
    {
        func_80099754__for_func_80099E84(1);
        if (D_801AFBD2 == D_800D6508[D_8019D240])
        {
            D_801398D0 = 2;
            D_80182D68 = (D_800DCED8 - D_800DCEF8) * 0x30;
            D_80182D78 = (D_800DCEE4 - D_800DCF00) * 0x30;
            D_800D9164 += 1;
        }
        return 1;
    }
    if (D_801398D0 == 2)
    {
        return 1;
    }
    D_801398AC = 0;
    D_800DCEF8 = D_800DCED8;
    D_800DCF00 = D_800DCEE4;
    return 0;
}

/** @brief Wait for facing to settle, then select the next movement direction.
 * @return One while waiting, zero after scheduling movement.
 */
s32 func_80099F98(void)

{
/* Partial WMAP decompilation: 99.894740% (gcc280_g0). */

extern s16 D_800D6508[];
extern s32 D_800D9164;
extern s32 D_800DCED8;
extern s32 D_800DCEE4;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_801398D0;
extern s32 D_8019D240;
extern s16 D_801AFBD2;
extern s32 func_80099E84__for_func_80099F98(void) __asm__("func_80099E84");
extern void func_80099754__for_func_80099F98(s32) __asm__("func_80099754");

    s32 delta_y;
    s32 delta_x;
    s32 magnitude_x;
    s32 magnitude_y;

    if (D_800D9164 == 0)
   
  {
        func_80099754__for_func_80099F98(1);
        if (D_801AFBD2 == D_800D6508[D_8019D240])
       
      {
            D_801398D0 = 2;
            D_800D9164++;
        }
        return 1;
    }
    if (D_801398D0 == 2)
   
  {
        return 1;
    }
    D_800D9164 = 0;
    delta_x = D_800DCED8 - D_800DCEF8;
    delta_y = D_800DCEE4 - D_800DCF00;
    if (delta_x != 0)
    {
        magnitude_x = delta_x;
        if (delta_x < 0)
        {
            magnitude_x = -magnitude_x;
        }
        delta_x = (delta_x / magnitude_x) + 1;
    }
    else
    {
        delta_x = 1;
    }
    if (delta_y != 0)
    {
        magnitude_y = delta_y;
        if (delta_y < 0)
        {
            magnitude_y = -magnitude_y;
        }
        delta_y = (delta_y / magnitude_y) + 1;
    }
    else
    {
        delta_y = 1;
    }
    D_8019D240 = delta_x + (delta_y * 3);
    func_8006CBD8(func_80099E84__for_func_80099F98);
    return 0;
}

/** @brief Save the projection state and set the next effect's map-relative position. */
void func_8009A114(void)
{
/** @brief Four-word world-map transform state. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
    s32 pad;
} WmapTransform;

extern WmapTransform D_800DCEC8;
extern WmapTransform D_80139950;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_8011CF50;
extern s32 D_800DCEC0;
extern s32 D_801B2C48;
extern u8 D_8011D538[];
extern s32 D_801398D0;
extern s32 D_8013B208;
extern s32 D_8013B288;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern s32 D_801B2C54;
extern void func_8009ABB0__for_func_8009A114(void) __asm__("func_8009ABB0");

    D_8013B288 = 0;
    D_8011CF50 = 1;
    D_801B2C48 = 0;
    D_8013B208 = 1;
    func_8005FF88(-1);
    D_800DCEC0 = 0;
    D_800DCEC8 = D_80139950;
    cdrom_queue_read(0x1145, D_8011D538);
    cdrom_queue_read(0x1146, D_8011D538 + 0x2000);
    func_800A8AA8(0x1147);
    func_800A8AF0(0x1148);
    func_8006D0F0(24, &D_800DCEF8, &D_800DCF00);
    D_801398D0 = 2;
    D_80182D68 = ((D_800DCEF8 - 1) * 48) - D_80139950.x;
    D_80182D78 = ((D_800DCF00 - 1) * 48) - D_80139950.y;
    D_801B2C54++;
    func_8009ABB0__for_func_8009A114();
}

/** @brief World-map step: reset the actor and its slot, register a callback, advance. */
void func_8009A258(void)
{
/** @brief World-map actor record reset when this step spawns it. */
typedef struct
{
    u8 pad_00[2];
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
    u8 pad_28[2];
} WmapActor;

/** @brief World-map slot record populated when spawning this actor. */
typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    u8 pad_0C[2];
    s16 field_0E;
    s16 field_10;
    u8 pad_12[2];
} WmapSlotB;

extern WmapActor D_800DBE3C;
extern u8 D_80182E40[];
extern u8 D_8018B240[];
extern void *D_8013A184;
extern s32 D_8011D538;
extern WmapSlotB D_801AFBD0[];
extern s32 D_801B2C54;
extern s32 D_801B2C58;
extern void func_800999D0__for_func_8009A258(void) __asm__("func_800999D0");
extern void func_8009A328__for_func_8009A258(void) __asm__("func_8009A328");

    WmapActor *actor = &D_800DBE3C;

    cdrom_wait_queue_empty();
    func_800651B4(D_80182E40);
    func_800651B4(D_8018B240);
    D_8013A184 = &D_8011D538;
    actor->field_10 = -1;
    actor->field_22 = 0x80;
    actor->field_24 = 0x80;
    actor->field_02 = 0;
    actor->field_06 = 0;
    actor->field_0E = 0;
    actor->field_26 = 0;
    D_801AFBD0[0].field_00 = 1;
    D_801AFBD0[0].field_08 = 0xC8;
    D_801AFBD0[0].field_0E = 0xA;
    D_801AFBD0[0].field_02 = 0;
    D_801AFBD0[0].field_10 = 0x3C;
    D_801AFBD0[0].field_04 = 0;
    func_8006CBD8(func_800999D0__for_func_8009A258);
    D_801B2C58 = 0x3C;
    D_801B2C54 += 1;
    func_8009A328__for_func_8009A258();
}

/**
 * @brief World-map actor tick: advance timers, bump a wave index, and expire the step.
 */
void func_8009A328(void)
{
extern s8 *func_80099754__for_func_8009A328(s32 arg0) __asm__("func_80099754");
extern u8 D_801AFBD0[];
extern s32 D_8011CF74;
extern s32 D_801B2C54;
extern s32 D_801B2C58;

    s8 *obj;
    u8 *base;

    obj = func_80099754__for_func_8009A328(1);
    base = D_801AFBD0;
    if (*(s16 *)(base + 0xE) < 100)
    {
        *(s16 *)(base + 0xE) += 1;
    }
    if (*(s32 *)(base + 0x8) < 0x1E0)
    {
        *(s32 *)(base + 0x8) += 4;
    }
    if ((D_8011CF74 & 3) == 0)
    {
        if (obj[6] < 0xF)
        {
            obj[6] += 1;
        }
    }
    if (--D_801B2C58 == 0)
    {
        D_801B2C54 += 1;
    }
}

/**
 * @brief Update the world map and clear its flag when the tracked value reaches 0x3E0.
 * @return Zero at the target value, otherwise one.
 */
s32 func_8009A3E0(void)
{
extern void func_80099754__for_func_8009A3E0(s32) __asm__("func_80099754");
extern s32 D_800D9164;
extern s16 D_801AFBD2;

    func_80099754__for_func_8009A3E0(1);
    if (D_801AFBD2 == 0x3E0)
    {
        D_800D9164 = 0;
        return 0;
    }
    return 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009A420(s32 arg0)
{
extern u32 D_801B2C4C;
extern s32 D_801B2C50;
extern void (*D_800D651C[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2C4C = 1;
        D_801B2C50 = 1;
        return 1;
    }

    if (D_801B2C4C < 0x14)
    {
        D_800D651C[D_801B2C4C]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009A498(void)
{
extern u32 D_801B2C4C;
extern s32 D_801B2C50;
extern void (*D_800D651C[])(void);

    D_801B2C4C = 1;
    D_801B2C50 = 1;
}

/** @brief Save the projection state and queue resources for the next sequence. */
void func_8009A4B0(void)
{
/** @brief Four-word world-map transform state. */
typedef struct
{
    s32 words[4];
} WmapTransform;

extern s32 D_800DCEC0;
extern WmapTransform D_800DCEC8;
extern s32 D_8011CF50;
extern u8 D_8011D538[];
extern s32 D_80139224;
extern WmapTransform D_80139950;
extern s32 D_8013B208;
extern s32 D_8013B288;
extern s32 D_801B2C48;
extern s32 D_801B2C4C;
extern s32 D_801B2C50;

    D_80139224 = 1;
    D_8013B288 = 0;
    D_8011CF50 = 1;
    D_801B2C48 = 0;
    D_8013B208 = 1;
    func_8005FF88(-1);
    D_800DCEC0 = 0;
    D_800DCEC8 = D_80139950;
    cdrom_queue_read(0x1145, D_8011D538);
    cdrom_queue_read(0x1146, D_8011D538 + 0x2000);
    func_800A8AA8(0x1147);
    func_800A8AF0(0x1148);
    D_801B2C50 = 1;
    D_801B2C4C += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009A588(void)
{
extern s32 D_801B2C50;
extern s32 D_801B2C4C;

    if (--D_801B2C50 == 0)
    {
        D_801B2C4C += 1;
    }
}

/** @brief Calculate the selected map position relative to the current projection origin. */
void func_8009A5BC(void)
{
extern void func_8009A668__for_func_8009A5BC(void) __asm__("func_8009A668");
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_801398D0;
extern s32 D_80139950[];
extern s32 D_80182D68;
extern s32 D_80182D78;
extern s32 D_801B2C4C;

    func_8006D0F0(2, &D_800DCEF8, &D_800DCF00);
    D_801398D0 = 2;
    D_80182D68 = ((D_800DCEF8 - 1) * 0x30) - D_80139950[0];
    D_80182D78 = ((D_800DCF00 - 1) * 0x30) - D_80139950[1];
    D_801B2C4C += 1;
    func_8009A668__for_func_8009A5BC();
}

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void func_8009A668(void)
{
extern s32 D_801398D0;
extern s32 D_801B2C4C;
extern void func_80099848__for_func_8009A668(void) __asm__("func_80099848");

    if (D_801398D0 != 2)
    {
        D_801B2C4C += 1;
        func_80099848__for_func_8009A668();
    }
}

/** @brief Schedule the selected cell effect unless its value is two. */
void func_8009A6A8(void)
{
/** @brief First word of a 40-byte world-map cell. */
typedef struct
{
    s32 value;
    u8 unknown_04[36];
} WmapValueRecord;

extern s16 D_801AFBE0;
extern WmapValueRecord D_80139290[][6];
extern s32 D_801398AC;
extern s32 D_800DCED8;
extern s32 D_800DCEE4;
extern s32 D_801B2C4C;
extern void func_80099B50__for_func_8009A6A8(void) __asm__("func_80099B50");
extern void func_8009A75C__for_func_8009A6A8(void) __asm__("func_8009A75C");

    D_801AFBE0 = 0x3B;
    if (D_80139290[g_wmap_travelers[0].cell_x][g_wmap_travelers[0].cell_y].value == 2)
    {
        D_801398AC = 0;
    }
    else
    {
        D_800DCED8 = g_wmap_travelers[0].cell_x;
        D_800DCEE4 = g_wmap_travelers[0].cell_y;
        D_801398AC = 1;
        func_8006CBD8(func_80099B50__for_func_8009A6A8);
    }
    D_801B2C4C++;
    func_8009A75C__for_func_8009A6A8();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8009A75C(void)
{
extern s32 D_801398AC;
extern s32 D_801B2C4C;
extern void func_8009A798__for_func_8009A75C(void) __asm__("func_8009A798");
extern void func_8009A7D0__for_func_8009A75C(void) __asm__("func_8009A7D0");
extern s32 D_801B2C50;

    if (D_801398AC == 0)
    {
        D_801B2C4C += 1;
        func_8009A798__for_func_8009A75C();
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8009A798(void)
{
extern s32 D_801398AC;
extern s32 D_801B2C4C;
extern void func_8009A798(void);
extern void func_8009A7D0__for_func_8009A798(void) __asm__("func_8009A7D0");
extern s32 D_801B2C50;

    D_801B2C50 = 0x12C;
    D_801B2C4C += 1;
    func_8009A7D0__for_func_8009A798();
}

/** @brief Update the effect and advance its sequence on completion or timeout. */
void func_8009A7D0(void)
{
extern void func_80099754__for_func_8009A7D0(s32) __asm__("func_80099754");
extern s16 D_800D926A;
extern s16 D_801AFBD2;
extern s32 D_801B2C4C;
extern s32 D_801B2C50;

    s32 remaining_ticks;

    func_80099754__for_func_8009A7D0(1);
    if (D_801AFBD2 == 0x200)
    {
        D_800D926A = -1;
        D_801B2C50 = 0;
        D_801B2C4C += 1;
    }
    remaining_ticks = D_801B2C50 - 1;
    D_801B2C50 = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B2C4C += 1;
    }
}

/** @brief Set initial map coordinates, register the callback, and advance the sequence. */
void func_8009A854(void)
{

extern void func_8009A8D0__for_func_8009A854(void) __asm__("func_8009A8D0");
extern s32 D_800DCED8;
extern s32 D_800DCEE4;
extern s32 D_801398AC;
extern s32 D_801B2C4C;
extern s32 func_80099B50__for_func_8009A854(s32) __asm__("func_80099B50");

    func_8006D0F0(0x18, &D_800DCED8, &D_800DCEE4);
    wmap_set_traveler_position(0, D_800DCED8, D_800DCEE4);
    D_801398AC = 1;
    func_8006CBD8(&func_80099B50__for_func_8009A854);
    D_801B2C4C += 1;
    func_8009A8D0__for_func_8009A854();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8009A8D0(void)
{
extern s32 D_801398AC;
extern s32 D_801B2C4C;
extern void func_8009A90C__for_func_8009A8D0(void) __asm__("func_8009A90C");
extern void func_8009A944__for_func_8009A8D0(void) __asm__("func_8009A944");
extern s32 D_801B2C50;

    if (D_801398AC == 0)
    {
        D_801B2C4C += 1;
        func_8009A90C__for_func_8009A8D0();
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8009A90C(void)
{
extern s32 D_801398AC;
extern s32 D_801B2C4C;
extern void func_8009A90C(void);
extern void func_8009A944__for_func_8009A90C(void) __asm__("func_8009A944");
extern s32 D_801B2C50;

    D_801B2C50 = 0x78;
    D_801B2C4C += 1;
    func_8009A944__for_func_8009A90C();
}

/** @brief Advance a world-map sub-state, clearing a flag when a marker hits its cap. */
void func_8009A944(void)
{
extern s16 D_801AFBD2;
extern s16 D_800D926A;
extern s32 D_801B2C4C;
extern s32 D_801B2C50;
extern void func_80099754__for_func_8009A944(s32 arg) __asm__("func_80099754");

    func_80099754__for_func_8009A944(1);
    if (D_801AFBD2 == 0x400)
    {
        D_800D926A = 0;
    }
    if (--D_801B2C50 == 0)
    {
        D_801B2C4C += 1;
    }
}

/** @brief World-map step handler: install a callback, advance the step counter, chain to the next step. */
void func_8009A9A8(void)
{
extern void func_8009A3E0__for_func_8009A9A8(void) __asm__("func_8009A3E0");
extern void func_8009A9F0__for_func_8009A9A8(void) __asm__("func_8009A9F0");
extern s32 D_800D9164;
extern s32 D_801B2C4C;

    D_800D9164 = 1;
    func_8006CAC0(func_8009A3E0__for_func_8009A9A8);
    D_801B2C4C += 1;
    func_8009A9F0__for_func_8009A9A8();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8009A9F0(void)
{
extern s32 D_800D9164;
extern s32 D_801B2C4C;
extern void func_8009AA2C__for_func_8009A9F0(void) __asm__("func_8009AA2C");

    if (D_800D9164 == 0)
    {
        D_801B2C4C += 1;
        func_8009AA2C__for_func_8009A9F0();
    }
}

/**
 * @brief Reset a world-map animation flag and schedule the next step.
 */
void func_8009AA2C(void)
{
extern u16 D_801AFBD0;
extern s32 D_801B2C4C;
extern s32 D_801B2C50;
extern void func_8009AA6C__for_func_8009AA2C(void) __asm__("func_8009AA6C");

    D_801AFBD0 = 0;
    D_801B2C50 = 0x78;
    D_801B2C4C += 1;
    func_8009AA6C__for_func_8009AA2C();
}

/**
 * @brief World-map step: build a sprite, decrement a shared budget, expire the timer.
 * @note Best match ~89.03% (gcc280_g0); residual is prologue scheduling of the
 *       s0 save versus the first call (permuter territory).
 */
void func_8009AA6C(void)
{
/* Partial WMAP decompilation: 89.027020% (gcc280_g0). */

extern u8 D_800DBE3C[];
extern s32 D_8013A180;
extern s32 D_8011CF54;
extern s16 D_801AFBE0;
extern s32 D_801B2C4C;
extern s32 D_801B2C50;
extern void func_80099754__for_func_8009AA6C(s32 arg) __asm__("func_80099754");

    func_80099754__for_func_8009AA6C(0);
    func_8006CC4C(D_800DBE3C, &D_8013A180);
    func_80066F9C(D_800DBE3C, D_8011CF54, 0x28, D_801AFBE0, 2);
    *(s16 *)&D_8011CF54 = *(u16 *)&D_8011CF54 - 6;
    if (--D_801B2C50 == 0)
    {
        D_801B2C4C += 1;
    }
}

/** @brief Set the world-map ready flag and bump the wave index. */
void func_8009AB00(void)
{
extern s32 D_8013B294;
extern s32 D_801B2C4C;

    D_8013B294 = 1;
    D_801B2C4C += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009AB20(s32 arg0)
{
extern u32 D_801B2C54;
extern s32 D_801B2C58;
extern void (*D_800D656C[])(void);
extern s32 D_801398D0;
extern void func_8009A258__for_func_8009AB20(void) __asm__("func_8009A258");

    s32 result;

    if (arg0 != 0)
    {
        D_801B2C54 = 1;
        D_801B2C58 = 1;
        return 1;
    }

    if (D_801B2C54 < 0xE)
    {
        D_800D656C[D_801B2C54]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009AB98(void)
{
extern u32 D_801B2C54;
extern s32 D_801B2C58;
extern void (*D_800D656C[])(void);
extern s32 D_801398D0;
extern void func_8009A258__for_func_8009AB98(void) __asm__("func_8009A258");

    D_801B2C54 = 1;
    D_801B2C58 = 1;
}

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void func_8009ABB0(void)
{
extern u32 D_801B2C54;
extern s32 D_801B2C58;
extern void (*D_800D656C[])(void);
extern s32 D_801398D0;
extern void func_8009A258__for_func_8009ABB0(void) __asm__("func_8009A258");

    if (D_801398D0 != 2)
    {
        D_801B2C54 += 1;
        func_8009A258__for_func_8009ABB0();
    }
}

/** @brief Set initial map coordinates, register the callback, and advance the sequence. */
void func_8009ABF0(void)
{

extern s32 func_80099B50__for_func_8009ABF0(s32) __asm__("func_80099B50");
extern void func_8009AC6C__for_func_8009ABF0(void) __asm__("func_8009AC6C");
extern s32 D_800DCED8;
extern s32 D_800DCEE4;
extern s32 D_801398AC;
extern s32 D_801B2C54;

    func_8006D0F0(0, &D_800DCED8, &D_800DCEE4);
    wmap_set_traveler_position(0, D_800DCED8, D_800DCEE4);
    D_801398AC = 1;
    func_8006CBD8(func_80099B50__for_func_8009ABF0);
    D_801B2C54 += 1;
    func_8009AC6C__for_func_8009ABF0();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8009AC6C(void)
{
extern s32 D_801398AC;
extern s32 D_801B2C54;
extern void func_8009ACA8__for_func_8009AC6C(void) __asm__("func_8009ACA8");
extern void func_8009ACE0__for_func_8009AC6C(void) __asm__("func_8009ACE0");
extern s32 D_801B2C58;

    if (D_801398AC == 0)
    {
        D_801B2C54 += 1;
        func_8009ACA8__for_func_8009AC6C();
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8009ACA8(void)
{
extern s32 D_801398AC;
extern s32 D_801B2C54;
extern void func_8009ACA8(void);
extern void func_8009ACE0__for_func_8009ACA8(void) __asm__("func_8009ACE0");
extern s32 D_801B2C58;

    D_801B2C58 = 0x78;
    D_801B2C54 += 1;
    func_8009ACE0__for_func_8009ACA8();
}

/** @brief Advance a world-map sub-state, clearing a flag when a marker hits its cap. */
void func_8009ACE0(void)
{
extern s16 D_801AFBD2;
extern s16 D_800D926A;
extern s32 D_801B2C54;
extern s32 D_801B2C58;
extern void func_80099754__for_func_8009ACE0(s32 arg) __asm__("func_80099754");

    func_80099754__for_func_8009ACE0(1);
    if (D_801AFBD2 == 0x400)
    {
        D_800D926A = 0;
    }
    if (--D_801B2C58 == 0)
    {
        D_801B2C54 += 1;
    }
}

/** @brief World-map step handler: install a callback, advance the step counter, chain to the next step. */
void func_8009AD44(void)
{
extern void func_8009A3E0__for_func_8009AD44(void) __asm__("func_8009A3E0");
extern void func_8009AD8C__for_func_8009AD44(void) __asm__("func_8009AD8C");
extern s32 D_800D9164;
extern s32 D_801B2C54;

    D_800D9164 = 1;
    func_8006CAC0(func_8009A3E0__for_func_8009AD44);
    D_801B2C54 += 1;
    func_8009AD8C__for_func_8009AD44();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8009AD8C(void)
{
extern s32 D_800D9164;
extern s32 D_801B2C54;
extern void func_8009ADC8__for_func_8009AD8C(void) __asm__("func_8009ADC8");

    if (D_800D9164 == 0)
    {
        D_801B2C54 += 1;
        func_8009ADC8__for_func_8009AD8C();
    }
}

/**
 * @brief Reset a world-map animation flag and schedule the next step.
 */
void func_8009ADC8(void)
{
extern u16 D_801AFBD0;
extern s32 D_801B2C54;
extern s32 D_801B2C58;
extern void func_8009AE08__for_func_8009ADC8(void) __asm__("func_8009AE08");

    D_801AFBD0 = 0;
    D_801B2C58 = 0x78;
    D_801B2C54 += 1;
    func_8009AE08__for_func_8009ADC8();
}

/**
 * @brief World-map step: build a sprite, decrement a shared budget, expire the timer.
 * @note Best match ~89.03% (gcc280_g0); residual is prologue scheduling of the
 *       s0 save versus the first call (permuter territory).
 */
void func_8009AE08(void)
{
/* Partial WMAP decompilation: 89.027020% (gcc280_g0). */

extern u8 D_800DBE3C[];
extern s32 D_8013A180;
extern s32 D_8011CF54;
extern s16 D_801AFBE0;
extern s32 D_801B2C54;
extern s32 D_801B2C58;
extern void func_80099754__for_func_8009AE08(s32 arg) __asm__("func_80099754");

    func_80099754__for_func_8009AE08(0);
    func_8006CC4C(D_800DBE3C, &D_8013A180);
    func_80066F9C(D_800DBE3C, D_8011CF54, 0x28, D_801AFBE0, 2);
    *(s16 *)&D_8011CF54 = *(u16 *)&D_8011CF54 - 4;
    if (--D_801B2C58 == 0)
    {
        D_801B2C54 += 1;
    }
}

/** @brief Set the world-map ready flag and bump the wave index. */
void func_8009AE9C(void)
{
extern s32 D_8013B294;
extern s32 D_801B2C54;

    D_8013B294 = 1;
    D_801B2C54 += 1;
}
