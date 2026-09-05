#include "common.h"

typedef struct
{
    u8 pad0[0x3A];
    u8 object_index;
    u8 pad3B[0x54 - 0x3B];
} FieldObjectState;

typedef struct
{
    u8 pad0[0x2E];
    u8 scale_z;
    u8 pad2F[0x33 - 0x2F];
    u8 scale_x;
    u8 pad34[0x48 - 0x34];
} FieldActorPartDef;

extern FieldActorPartDef D_800FE3A0[];

/**
 * @brief Set a field actor's horizontal model scale to half-size or full-size.
 * @param object Field object whose actor-part definition is updated.
 * @param half_scale Non-zero for half-size X/Z scale, zero for full-size scale.
 */
void field_set_actor_horizontal_scale(FieldObjectState *object, s32 half_scale)
{
    if (half_scale != 0)
    {
        D_800FE3A0[object->object_index].scale_z = 0x20;
        D_800FE3A0[object->object_index].scale_x = 0x20;
    }
    else
    {
        D_800FE3A0[object->object_index].scale_z = 0x40;
        D_800FE3A0[object->object_index].scale_x = 0x40;
    }
}
