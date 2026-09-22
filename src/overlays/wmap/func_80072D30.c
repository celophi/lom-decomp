#include "wmap_sequence_runtime.h"
/* Partial WMAP decompilation: 92.546150% (gcc280_g0). */
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
    s32 field_10;
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

extern WmapConfigA D_800D9370[];
extern WmapResource D_801399B8[];
extern WmapMotion *D_801B2560;
extern VECTOR D_8011CF60;
extern VECTOR D_80139870;
extern SVECTOR D_80139278;
extern SVECTOR D_801B24A0;
extern u16 D_80139980;
extern void func_80066F9C(void *, s32, s32, s32, s32);

/** @brief Compose the effect transform and project its twenty actors. */
void func_80072D30(void)
{
    MATRIX base_matrix;
    MATRIX effect_matrix;
    SVECTOR position;
    s32 screen_position;
    s32 i;
    s32 value;
    WmapMotion *motion;
    WmapConfigA *actor;
    WmapResource *resource;

    PushMatrix();
    RotMatrix(&D_80139278, &base_matrix);
    TransMatrix(&base_matrix, &D_80139870);
    SetRotMatrix(&base_matrix);
    SetTransMatrix(&base_matrix);
    RotMatrix(&D_801B24A0, &effect_matrix);
    TransMatrix(&effect_matrix, &D_8011CF60);
    CompMatrix(&base_matrix, &effect_matrix, &effect_matrix);
    SetRotMatrix(&effect_matrix);
    SetTransMatrix(&effect_matrix);
    resource = D_801399B8;
    D_801B24A0.vz -= 80;
    for (i = 0; i < 20; i++)
    {
        actor = &D_800D9370[i];
        position.vx = ((D_801B2560[i].z >> 6) * (ccos(D_801B2560[i].angle) >> 6)) >> 12;
        position.vy = ((D_801B2560[i].z >> 6) * (csin(D_801B2560[i].angle) >> 6)) >> 12;
        motion = &D_801B2560[i];
        position.vz = motion->field_0E;
        gte_ldv0(&position);
        gte_rtps();
        value = motion->z + motion->x;
        motion->z = value;
        if (value > 900000)
        {
            motion->z = 900000;
        }
        actor->field_22 = D_80139980;
        actor->field_24 = D_80139980;
        gte_stsxy(&screen_position);
        func_8006CC4C(actor, resource);
        resource++;
        func_80066F9C(actor, screen_position, 13, 31, 0);
    }
    PopMatrix();
}
