#include "common.h"

#define FIELD_ANIMATION_SEQUENCE_MASK 0x7F
#define FIELD_ANIMATION_MIRROR_FLAG 0x80
#define FIELD_ANIMATION_HOLD_LAST_FRAME 0x800

/** @brief Field object state used by the actor-control animation handlers. */
typedef struct
{
    u8 pad00[0x1C];
    s32 state_flags;
    u8 pad20[1];
    u8 animation_sequence;
    u8 pad22[0x24 - 0x22];
    u8 frame_duration_scale;
    u8 pad25[0x27 - 0x25];
    u8 frame_index;
    u8 pad28[0x2E - 0x28];
    u16 animation_repeat_count;
    u8 pad30[0x3A - 0x30];
    u8 object_index;
    u8 pad3B[0x54 - 0x3B];
} FieldObjectRecord;

/** @brief Per-actor runtime slot in D_80105AE0. */
typedef struct
{
    u8 pad000[0x174];
    u32 state;
    u8 pad178[0x23C - 0x178];
} FieldActorSlot;

s32 func_80083EEC(u8 object_index, s32 actor_index, s32 animation_id);
void field_start_actor_animation(s32 actor_index, s32 arg1, s32 arg2);
void func_8006C3FC(FieldObjectRecord* object);
void func_80086C00(u8 object_index);

extern FieldActorSlot D_80105AE0[];

/**
 * @brief Handle actor control flag 0x40 becoming active for a field object.
 * @param object Field object whose actor slot and animation state are updated.
 * @param is_set Non-zero when actor control flag 0x40 is currently set.
 */
void func_800869FC(FieldObjectRecord* object, s32 is_set)
{
    u8 animation_sequence;

    if (is_set != 0)
    {
        func_80083EEC(object->object_index, object->object_index + 0x40, 9);
        field_start_actor_animation(object->object_index + 0x40, 0, 0);
        animation_sequence = object->animation_sequence;
        if ((animation_sequence & FIELD_ANIMATION_SEQUENCE_MASK) != 0x1B)
        {
            object->animation_sequence = (animation_sequence & FIELD_ANIMATION_MIRROR_FLAG) + 0x1B;
            object->animation_repeat_count = 1;
            object->frame_index = 0;
            object->frame_duration_scale = 1;
            D_80105AE0[object->object_index].state &= ~0x1800;
            func_8006C3FC(object);
            object->state_flags |= FIELD_ANIMATION_HOLD_LAST_FRAME;
        }
        func_80086C00(object->object_index);
    }
}
