#include "common.h"

/** @brief Actor-slot lookup entry from D_80105AE0. */
typedef struct
{
    u8 pad0[0x14];
    s32 lookup_key;
    u8 pad18[0x224];
} FieldActorSlotEntry;

/** @brief Runtime actor record from D_800FDF58. */
typedef struct
{
    u8 pad0[0x3A];
    u8 track_selector;
    u8 pad3B[0x19];
} FieldActorRecord;

s32 func_800839F8(s32 arg0, s32 arg1);
s32 func_80083EEC(u8 arg0, s32 arg1, s32 arg2);
void field_start_actor_animation(s32 slot_index, s32 target_count, u8 *targets);

extern FieldActorSlotEntry D_80105AE0[];
extern FieldActorRecord D_800FDF58[];

/**
 * @brief Collect matching actor selectors and start the requested actor animation.
 * @param lookup_key Value matched against the first 13 actor-slot entries.
 * @param animation_param Value forwarded to func_80083EEC.
 * @param repeat_count Number of repeated lookup passes used to collect target selectors.
 * @param unused_target Caller-provided target pointer; this routine does not consume it.
 * @return -1 if no actor record matches, 0 if the animation starts, or 1 if setup fails.
 */
s32 func_8008B5D0(s32 lookup_key, s32 animation_param, s32 repeat_count, s32 *unused_target)
{
    FieldActorSlotEntry *slot;
    FieldActorRecord *scan_record;
    FieldActorRecord *final_record;
    FieldActorRecord *found;
    s32 i;
    s32 repeat_index;
    s32 target_count;
    s32 animation_slot;
    s32 targets[16];

    repeat_index = 0;
    target_count = repeat_index;
    for (; repeat_index < repeat_count; repeat_index++)
    {
        scan_record = D_800FDF58;
        slot = D_80105AE0;
        for (i = 0; i < 13; i++, slot++, scan_record++)
        {
            if (slot->lookup_key == lookup_key)
            {
                found = scan_record;
                goto matched;
            }
        }
        found = (FieldActorRecord *)-1;
    matched:
        if (found != (FieldActorRecord *)-1)
        {
            targets[target_count] = found->track_selector;
            target_count++;
        }
    }

    final_record = D_800FDF58;
    slot = D_80105AE0;
    for (i = 0; i < 13; i++, slot++, final_record++)
    {
        if (slot->lookup_key == lookup_key)
        {
            goto found_it;
        }
    }
    found = (FieldActorRecord *)-1;
check:
    if (found != (FieldActorRecord *)-1)
    {
        goto body;
    }
    return -1;
found_it:
    found = final_record;
    goto check;
body:
    animation_slot = func_800839F8(found->track_selector, 0);
    if ((animation_slot != -1) && (func_80083EEC(found->track_selector, animation_slot, animation_param) != 0))
    {
        field_start_actor_animation(animation_slot, target_count, (u8 *)targets);
        return 0;
    }
    return 1;
}
