#include "common.h"

typedef struct
{
    u8 pad0[0x24];
    u8 is_active;
    u8 pad25[0x228 - 0x25];
    u8 owner_object_index;
    u8 pad229[0x23A - 0x229];
    u8 active_track_mask;
    u8 pad23B[0x244 - 0x23B];
} FieldActorState;

extern FieldActorState g_field_actor_slots[];

/**
 * @brief Check whether a field object owns an actor with active animation tracks.
 * @param object_index Field-object index to match against each actor's owner.
 * @return 1 if a matching actor has active tracks, otherwise 0.
 */
s32 field_object_has_active_actor_tracks(s32 object_index)
{
    s32 slot_index;

    slot_index = 0;
    do
    {
        if ((g_field_actor_slots[slot_index].owner_object_index == object_index) && (g_field_actor_slots[slot_index].active_track_mask != 0))
        {
            return 1;
        }
        slot_index++;
    } while (slot_index < 0x30);
    return 0;
}

/**
 * @brief Count free actor slots in the first 48-slot actor pool.
 * @return Number of actor slots whose active flag is clear.
 */
s32 field_count_free_actor_slots(void)
{
    s32 free_count;
    s32 slot_index;

    free_count = 0;
    for (slot_index = 0; slot_index < 0x30; slot_index++)
    {
        if (g_field_actor_slots[slot_index].is_active == 0)
        {
            free_count++;
        }
    }

    return free_count;
}
