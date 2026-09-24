/** @file field_actor_record_ops.c
 * @brief Spawn, release and query dynamic actor records.
 */

#include "game_audio.h"
#include "common.h"
#include "field_calls.h"
#include "field_records.h"

/** @brief Three-dimensional field position. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
} FieldPosition;

/** @brief Actor identifier paired with its distance from a reference position. */
typedef struct
{
    s32 object_id;
    s32 distance;
} FieldDistanceEntry;

/** @brief Sortable list of actor identifiers and their distances. */
typedef struct
{
    s32 count;
    FieldDistanceEntry entries[16];
} FieldDistanceList;

FieldActorRecord* func_800C1C50(s32 id);
/* Declared without a prototype: callers forward their own a0. */
FieldActorRecord* func_800C1B60();
void func_80087F44(s32 index, FieldPosition* position);
void func_800C1F28(u32* arg0);
s32 func_800C1FBC(FieldPosition* arg0, FieldPosition* arg1);

extern FieldRuntimeContext* D_80122B78;

/**
 * @brief Spawn an actor record from the event table of actor record 4, or release it.
 * @param actor_id Actor identifier.
 * @param argument Argument for the spawn event 7, or 0xFF to release the record.
 */
void func_800C2640(s32 actor_id, s32 argument)
{
    FieldActorRecord* actor;
    s32 i;

    if (argument != 0xFF)
    {
        actor = func_800C1C50(actor_id);
        if (actor == NULL)
        {
            record_game_diagnostic(0x8001, 1, 1, 1);
            return;
        }
        actor->flags.bits.spawned = 1;
        actor->enabled_events = D_80122B78->actors[4].enabled_events;
        i = 0;
        do
        {
            actor->scripts[i] = D_80122B78->actors[4].scripts[i];
            i++;
        } while (i < FIELD_ACTOR_SCRIPT_COUNT);
        func_800B28E0(actor_id, 7, argument & 0xFF);
        return;
    }

    actor = func_800C1B60(actor_id);
    actor->flags.bits.active = 0;
    actor->flags.bits.spawned = 0;
}

/**
 * @brief Select the nearest eligible actor to actor six.
 * @return Object ID of the nearest eligible actor, or 0xFF when none qualify.
 */
s32 func_800C2724(void)
{
    FieldDistanceList list;
    FieldPosition reference_position;
    FieldPosition actor_position;
    s32 actor_index;
    s32 flags;
    FieldActorRecord* actor;

    func_80087F44(6, &reference_position);
    actor_index = FIELD_PARTY_SIZE;
    list.count = 0;
    do
    {
        actor = &D_80122B78->actors[actor_index];
        flags = actor->flags.word;
        if (flags < 0 && actor->flags.bits.spawned && func_80087770(6, actor->id) == 1)
        {
            func_80087F44(actor->id, &actor_position);
            list.entries[list.count].object_id = actor->id;
            list.entries[list.count].distance = func_800C1FBC(&reference_position, &actor_position);
            list.count += 1;
        }
        actor_index += 1;
    } while (actor_index < FIELD_ACTOR_RECORD_COUNT);

    func_800C1F28((u32*)&list);
    if (list.count != 0)
    {
        return list.entries[0].object_id;
    }
    return 0xFF;
}

/* Declared without a prototype: func_800C2848 forwards its own a0. */
extern FieldActorRecord* func_800C1B98();
extern void func_800C1D14(s32 actor_id, s32 flags);
extern void func_80087D8C(s32 actor_id, s32 arg1, s32 arg2, s32 arg3);

/**
 * @brief Switch an actor to script-only mode and stop its script.
 * @param actor_id Actor identifier; passed through to func_800C1B98 in a0.
 * @param flags Bit 0 forwarded to func_800C1D14; bit 1 also calls func_80087D8C.
 */
void func_800C2848(s32 actor_id, s32 flags)
{
    FieldActorRecord* actor = func_800C1B98();

    if (actor != NULL)
    {
        actor->flags.bits.script_only = 1;
        func_800C1D14(actor->id, flags);
        if (flags & 2)
        {
            func_80087D8C(actor_id, -0x400, 0, 0);
        }
    }
}

/**
 * @brief Leave script-only mode and stop the actor's script.
 * @note Takes the actor id in a0 from its caller; func_800C1B60 reads it from there.
 */
void func_800C28B8(void)
{
    FieldActorRecord* actor;
    s32 actor_id;

    actor = func_800C1B60();
    actor_id = actor->id;
    actor->flags.bits.script_only = 0;
    func_800C1D14(actor_id, 0);
}
