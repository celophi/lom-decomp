#include "common.h"

typedef struct
{
    u8 pad0[0x3A];
    u8 object_index;
} FieldActorRecord;

typedef struct
{
    u8 pad0[0x228];
    u8 owner_object_index;
    u8 pad229[0x244 - 0x229];
} FieldActorState;

extern FieldActorState g_field_actor_slots[];

void func_80083BC0(FieldActorRecord *record, FieldActorState *actor, s32 force);
void func_800A3B78(s32 object_index);

/**
 * @brief Stop actor animations and reserved SFX channels for a field object.
 * @param record Field actor record whose object index selects the owned actor slots.
 * @param force Non-zero to force teardown of matching actor animations.
 */
void field_stop_actor_animations_for_object(FieldActorRecord *record, s32 force)
{
    FieldActorState *actor;
    s32 actor_index;

    actor = g_field_actor_slots;
    for (actor_index = 0; actor_index < 48; actor_index++, actor++)
    {
        if (actor->owner_object_index == record->object_index)
        {
            func_80083BC0(record, actor, force);
        }
    }
    func_800A3B78(record->object_index);
}
