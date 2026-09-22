/* Partial WMAP decompilation: 97.459015% (gcc280_g0). */
#include "common.h"

/** @brief Actor animation state; only sequence and previous-sequence fields are changed. */
typedef struct
{
    u8 pad_00[0xE];
    s16 sequence;
    s16 previous_sequence;
} WmapAnimation;

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

/**
 * @brief Select the animation and resource bank from the current rotation.
 * @param advance Nonzero to advance the rotation by 32 units.
 * @return Actor animation state.
 */
WmapAnimation *func_80099754(s32 advance)
{
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
