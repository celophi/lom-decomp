/** @file field_event_dispatch.c
 * @brief Queue and start actor events through their script tables.
 */

#include "game_audio.h"
#include "common.h"
#include "field_calls.h"
#include "field_records.h"

extern FieldRuntimeContext* g_field_runtime;

void field_script_run(FieldScriptState* state);
u8* field_get_event_script(s32 script_id);
/* Declared without a prototype: func_800B286C forwards its own a0 without reloading it. */
FieldActorRecord* func_800C1B60();
u8* func_800C28F8(s32 owner_id, s32 event_index);

/**
 * @brief Queue an event on an actor when the event is enabled and nothing is pending.
 * @param owner_id Actor id; passed through to func_800C1B60 in a0.
 * @param event_id Event index and value stored as the pending event.
 * @param argument Event argument stored with the pending event.
 * @return Event index on success, otherwise -1.
 */
s32 func_800B286C(s32 owner_id, u8 event_id, s8 argument)
{
    s32 index;
    FieldActorRecord* actor;

    actor = func_800C1B60();
    index = event_id & 0xFF;
    if (((s32)actor->enabled_events >> index) & 1)
    {
        if (actor->event == FIELD_NO_EVENT)
        {
            actor->event = event_id;
            actor->event_argument = argument;
            return index;
        }
    }

    return -1;
}

/**
 * @brief Start an enabled actor event, pushing a script frame when one is running.
 * @param owner_id Actor whose script state runs the event.
 * @param event_id Event index in the low byte; valid indices are zero through fifteen.
 * @param mode Event argument stored in the actor while the script runs.
 * @return Event index on success, or -1 for an unavailable event or a frame overflow.
 */
s32 func_800B28E0(s32 owner_id, s32 event_id, s32 mode)
{
    s32 next_depth;
    s32 depth;
    u16 script_id;
    u32 event_index;
    FieldActorRecord* actor;

    event_index = event_id & 0xFF;
    if (event_index < FIELD_ACTOR_SCRIPT_COUNT)
    {
        actor = func_800C1B60(owner_id);
        if (((s32)actor->enabled_events >> event_index) & 1)
        {
            depth = actor->script.depth;
            actor->script.status.owner_id = owner_id;
            if (actor->script.frames[depth].pc != NULL)
            {
                next_depth = depth + 1;
                actor->script.depth = next_depth;
                if (next_depth >= FIELD_SCRIPT_FRAME_COUNT)
                {
                    actor->script.depth = FIELD_SCRIPT_FRAME_COUNT - 1;
                    record_game_diagnostic(0x8001, 2, actor->id, event_index);
                    return -1;
                }
            }
            if ((g_field_runtime->state.flags & 0x10000) && (owner_id < FIELD_PARTY_SIZE))
            {
                actor->script.frames[actor->script.depth].pc = func_800C28F8(owner_id, event_id & 0xFF);
            }
            else
            {
                script_id = actor->scripts[event_id & 0xFF];
                if (script_id == FIELD_NO_SCRIPT)
                {
                    return -1;
                }
                actor->script.frames[actor->script.depth].pc = field_get_event_script(script_id & 0x7FFF);
            }
            actor->script.frames[actor->script.depth].wait.bits.resume = 0;
            actor->script.frames[actor->script.depth].wait.bits.frames = 0;
            actor->event_argument = mode;
            field_script_run(&actor->script);
            actor->event = FIELD_NO_EVENT;
            actor->event_argument = 0;
            return event_id & 0xFF;
        }
        return -1;
    }
    return -1;
}
