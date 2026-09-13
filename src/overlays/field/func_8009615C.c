#include "common.h"

/** @brief Animation record fields consumed by actor initialization. */
typedef struct
{
    u8 pad[0x12];
    u16 value;
    u8 pad14[2];
    u8 mode;
    u8 pad17[5];
} Animation;
/** @brief Actor slot fields initialized from a template. */
typedef struct
{
    u8 pad[0xC];
    Animation *current, *animations;
    u8 pad14[0x10];
    u8 active;
    u8 pad25[4];
    u8 animation;
    u8 pad_2a[0x1F8];
    u16 value;
    u8 pad224[15];
    u8 slot;
    u8 pad234[4];
    u16 mode;
    u8 pad_23a[10];
} Actor;
/** @brief Source actor template with its original stride. */
typedef struct
{
    u8 data[0x268];
} Template;
/** @brief Source record containing the assigned actor slot. */
typedef struct
{
    u8 pad[0x179];
    u8 slot;
    u8 tail[0xC2];
} Record;
/** @brief One of three actor-slot bindings. */
typedef struct
{
    u8 pad[0x18];
    s32 slot;
} Binding;
extern Actor g_field_actor_slots[];
extern Template D_800FD81C[];
extern Record D_80105AE0[];
extern Binding D_80105880[];
extern s32 func_800839F8(s32, s32);
extern void bcopy(void *, void *, s32);
/**
 * @brief Copy an actor template into a slot and select its initial animation.
 * @param index Source template and record index.
 * @param flags Animation override flag and two-bit animation index.
 * @return Assigned actor slot, or -1 when allocation fails.
 */
s32 func_8009615C(s32 index, s32 flags)
{
    s32 slot, binding;
    Actor *actor, *updated, *slots;
    Binding *bindings;
    slot = func_800839F8(index, 0);
    if (slot != -1)
    {
        actor = &g_field_actor_slots[slot];
        bcopy(&D_800FD81C[index], actor, 0x244);
        actor->active = 1;
        actor->slot = slot;
        if (flags & 0x4000)
        {
            actor->animation = (flags >> 12) & 3;
            actor->value = actor->animations[actor->animation].value;
        }
        else
        {
            actor->animation = 0;
            actor->value = actor->animations->value;
        }
        slots = g_field_actor_slots;
        updated = &slots[slot];
        binding = index;
        updated->mode = updated->animations[updated->animation].mode;
        updated->current = &updated->animations[updated->animation];
        D_80105AE0[binding].slot = slot;
        bindings = D_80105880;
        if (binding >= 3)
        {
            binding = 2;
        }
        bindings[binding].slot = slot;
    }
    else
    {
        D_80105AE0[index].slot = 0xFF;
    }
    return slot;
}
