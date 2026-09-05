/**
 * @file field_queue_actor_animation_by_handle.c
 * @brief Resolve a runtime field-actor handle and queue an animation request.
 */

#include "common.h"

/** @brief Field object record paired with a runtime actor slot. */
typedef struct
{
    u8 _pad000[0x3A];
    u8 object_index;
    u8 _pad03B[0x54 - 0x3B];
} FieldObjectRecord;

/** @brief Runtime field-actor slot used to resolve actor handles. */
typedef struct
{
    u8 _pad000[0x14];
    s32 handle;
    u8 _pad018[0x23C - 0x18];
} FieldActorSlot;

/** @brief Partial view of the shared context referenced by g_pad_ctx. */
typedef struct
{
    u8 _pad000[0x3158];
    s32 unk3158;
} FieldPadContext;

extern FieldObjectRecord D_800FDF58[];
extern FieldActorSlot D_80105AE0[];
extern FieldPadContext *g_pad_ctx;

extern void func_8008C024(FieldObjectRecord *object, s32 animation_id);

/**
 * @brief Queue an animation for the field actor identified by a runtime handle.
 * @param actor_handle Handle to locate in the first 13 runtime actor slots.
 * @param animation_id Animation identifier forwarded to the actor update path.
 * @return 0 when the actor is found and updated, or -1 when no actor matches.
 */
s32 func_8008AE14(s32 actor_handle, s32 animation_id)
{
    FieldObjectRecord *object;
    FieldActorSlot *actor_slot;
    s32 context_counter;
    s32 actor_index;

    object = D_800FDF58;
    actor_slot = D_80105AE0;
    actor_index = 0;
scan_actor:
    if (actor_slot->handle != actor_handle)
    {
        actor_slot += 1;
        actor_index += 1;
        object += 1;
        if (actor_index >= 13)
        {
            object = (FieldObjectRecord *)-1;
        }
        else
        {
            goto scan_actor;
        }
    }

    if (object == (FieldObjectRecord *)-1)
    {
        return -1;
    }

    if (object->object_index >= 3)
    {
        context_counter = g_pad_ctx->unk3158;
        if (context_counter != -1)
        {
            g_pad_ctx->unk3158 = context_counter + 1;
        }
    }

    func_8008C024(object, animation_id);
    return 0;
}
