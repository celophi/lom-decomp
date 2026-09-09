#include "common.h"

/** @brief Record carrying the binding selector. */
typedef struct
{
    u8 pad[0x3A];
    u8 selector;
} Record;
/** @brief Binding state, owner index, and actor slot. */
typedef struct
{
    s32 state;
    u8 pad4[8];
    s32 owner;
    u8 pad10[8];
    s32 slot;
} Binding;
/** @brief Actor slot with two state bytes tested by the query. */
typedef struct
{
    u8 pad[0x23A];
    u8 first, second;
    u8 tail[8];
} Actor;
extern Binding D_80105880[];
extern Actor g_field_actor_slots[];
extern Record *func_80087C9C(s32);
/**
 * @brief Classify an actor record using its binding and slot state.
 * @param index Actor identifier passed to the record lookup.
 * @return -1 for a missing record, otherwise a state code from 0 through 4.
 */
s32 func_800878B4(s32 index)
{
    Record *record;
    u8 *base;
    Actor *actors, *actor;
    s32 offset, owner;
    record = func_80087C9C(index);
    if (record == (Record *)-1)
    {
        return -1;
    }
    base = (u8 *)D_80105880;
    if (record->selector < 2)
    {
        offset = record->selector * 28;
    }
    else
    {
        offset = 56;
    }
    if (((Binding *)(base + offset))->state == 0)
    {
        return 0;
    }
    base = (u8 *)D_80105880;
    if (record->selector < 2)
    {
        offset = record->selector * 28;
    }
    else
    {
        offset = 56;
    }
    if (record->selector != (owner = ((Binding *)(base + offset))->owner))
    {
        return 1;
    }
    base = (u8 *)D_80105880;
    if ((u8)owner < 2)
    {
        offset = owner * 28;
    }
    else
    {
        offset = 56;
    }
    if (((Binding *)(base + offset))->state == 1)
    {
        return 2;
    }
    actors = g_field_actor_slots;
    base = (u8 *)D_80105880;
    if (record->selector < 2)
    {
        offset = record->selector * 28;
    }
    else
    {
        offset = 56;
    }
    actor = (Actor *)((u8 *)actors + ((Binding *)(base + offset))->slot * 0x244);
    if (actor->first != 0)
    {
        return 3;
    }
    actors = g_field_actor_slots;
    base = (u8 *)D_80105880;
    if (record->selector < 2)
    {
        offset = record->selector * 28;
    }
    else
    {
        offset = 56;
    }
    actor = (Actor *)((u8 *)actors + ((Binding *)(base + offset))->slot * 0x244);
    if (actor->second != 0)
    {
        return 3;
    }
    return 4;
}
