#include "wmap_sequence_runtime.h"
#include "common.h"

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

#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

extern WmapConfigA D_800DBE3C;
extern s32 D_8011CF54;
extern MATRIX D_8011D0E8;
extern s32 D_80139224;
extern WmapResource D_8013A180;
extern WmapMotion D_801AFBD0;
extern void func_800652A8(s32, s32);
extern void func_80066F9C(void *, s32, s32, s32, s32);

/** @brief Project and draw the effect actor, playing sounds at selected frames.
 * @return Actor motion state.
 */
s16 func_800999D0(void)
{
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
