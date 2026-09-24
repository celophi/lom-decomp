/** @file field_record_lookup_ops.c
 * @brief Actor record lookup and allocation, reward pickup and resource lookup.
 */

#include "game_audio.h"
#include "common.h"
#include "field_records.h"

FieldStatusState* func_80087F0C(s32 actor_id);
void saturating_counter_add(FieldStatusState* state, s32 delta);
void func_8008BD88(s32 actor_id);
u32* func_800875B4(void);

/*
 * Declared without a prototype: func_800C1B60 forwards its caller's a0 to
 * func_800C1B98 by calling it with no arguments, which a prototype would
 * reject. The definition below carries the real signature.
 */
FieldActorRecord* func_800C1B98();
FieldActorRecord* func_800C1B60();

extern FieldRuntimeContext* D_80122B78;

/**
 * @brief Battle reward entry, 0x44 bytes from the table base.
 * @note Entry 0's count field holds the number of entries; each entry's item
 *       starts at offset 8 and runs into the next entry.
 */
typedef struct
{
    u16 unk0;
    u16 count;
    s32 key;
    u8 item[0x3C];
} FieldRewardEntry;

void func_800C2138();
u8* field_find_free_inventory_record(void);
void field_copy_inventory_record(u8* dst, u8* src);
void field_append_dialog_item(u8* arg0, u8 arg1);

extern FieldBattleContext* D_80123FB0;
extern u16 D_800F0E98[];

/**
 * @brief Hand an actor's pickup to the party: an item from the reward table or a counter.
 * @param unused Unused.
 * @param owner_id Actor whose pickup code is resolved.
 */
void func_800C1A18(void* unused, s32 owner_id)
{
    FieldActorRecord* actor;
    u16 code;
    s32 index;
    s32 count;
    FieldRewardEntry* table;
    FieldRewardEntry* cursor;
    s32 offset;
    s32 old_offset;
    u8* found;
    u8* handle;

    actor = func_800C1B60(owner_id);
    code = actor->pickup;
    index = code & 0xFF;
    if (!(code & 0x8000))
    {
        s32 key;

        table = (FieldRewardEntry*)D_80123FB0->resources;
        /* Kept: each do-while(0) below changes register allocation; all are required. */
        do
        {
            found = NULL;
        } while (0);
        count = table->count;
        do
        {
            index = 0;
            if (count != 0)
            {
                s32 n;
                FieldRewardEntry* base;

                key = code & 0xFFFF;
                base = table;
                n = count;
                cursor = base;
                offset = 0;
                do
                {
                    do
                    {
                        do
                        {
                            old_offset = offset;
                        } while (0);
                    } while (0);
                    if (cursor->key == key)
                    {
                        found = (u8*)(old_offset + (s32)base + 8);
                        break;
                    }
                    do
                    {
                        cursor++;
                    } while (0);
                    offset = old_offset + sizeof(FieldRewardEntry);
                    index += 1;
                } while (index < n);
            }
        } while (0);
        if (found == NULL)
        {
            return;
        }
        handle = field_find_free_inventory_record();
        if (handle == NULL)
        {
            return;
        }
        field_copy_inventory_record(handle, found);
        field_append_dialog_item(handle, 0);
        return;
    }
    /* Kept: passing actor keeps it in a1 as in the original. */
    func_800C2138(index, actor);
    field_append_dialog_item((u8*)D_800F0E98 + D_800F0E98[index], 1);
}

/**
 * @brief Restore a fraction of an actor's maximum capacity, capped at that maximum.
 * @param record_id Identifier of an existing runtime actor.
 * @param fraction_256 Fraction in units of 1/256; pickups use 64 or 128.
 * @note The amount comes from maximum capacity, not the current value.
 */
void field_restore_actor_capacity_fraction(s32 record_id, s32 fraction_256)
{
    FieldStatusState* state;
    s32 scaled_capacity;

    state = func_80087F0C(record_id);
    scaled_capacity = state->maximum * fraction_256;
    saturating_counter_add(state, (u32)scaled_capacity >> 8);
}

/**
 * @brief Look up an actor record, falling back to the second event record.
 *
 * Takes no formal parameters so that the caller's a0 flows unchanged into
 * func_800C1B98; callers pass the record id in that slot.
 *
 * @return The matching record, or the fallback record when none matched.
 */
FieldActorRecord* func_800C1B60()
{
    FieldActorRecord* actor;

    actor = func_800C1B98();
    if (actor == NULL)
    {
        actor = &D_80122B78->events[1];
    }
    return actor;
}

/**
 * @brief Find the record for an actor id.
 * @param id Ids below 3 index directly, 3..0x7F search active records, 0x80+ map to event records.
 * @return The record, or NULL when a searched id is not present.
 */
FieldActorRecord* func_800C1B98(s32 id)
{
    s32 i;

    if (id < FIELD_PARTY_SIZE)
    {
        return &D_80122B78->actors[id];
    }
    if (id < 0x80)
    {
        for (i = 0; i < FIELD_ACTOR_RECORD_COUNT; i++)
        {
            if (D_80122B78->actors[i].flags.bits.active && (D_80122B78->actors[i].id == id))
            {
                goto found;
            }
        }
        return NULL;
    }
    return &D_80122B78->actors[id - 0x70];
/* Kept: the found return is emitted last; returning inside the loop moves it. */
found:
    return &D_80122B78->actors[i];
}

/**
 * @brief Claim the first free actor record for an actor id and reset it.
 * @param id Actor id to store in the record.
 * @return The claimed record, or NULL when all 16 records are active.
 */
FieldActorRecord* func_800C1C50(s32 id)
{
    s32 i;
    s32 j;

    for (i = 0; i < FIELD_ACTOR_RECORD_COUNT; i++)
    {
        if (!D_80122B78->actors[i].flags.bits.active)
        {
            D_80122B78->actors[i].id = id;
            D_80122B78->actors[i].event = FIELD_NO_EVENT;
            D_80122B78->actors[i].enabled_events = 0xFFFF;
            D_80122B78->actors[i].flags.bits.trigger_group = 0;
            D_80122B78->actors[i].flags.bits.spawned = 0;
            D_80122B78->actors[i].flags.bits.script_only = 0;
            D_80122B78->actors[i].flags.bits.active = 1;
            for (j = 0; j < FIELD_ACTOR_SCRIPT_COUNT; j++)
            {
                D_80122B78->actors[i].scripts[j] = FIELD_NO_SCRIPT;
            }
            return &D_80122B78->actors[i];
        }
    }
    return NULL;
}

/**
 * @brief Stop an actor's script, optionally notifying func_8008BD88 first.
 * @param actor_id Actor id.
 * @param flags Bit 0 set requests the func_8008BD88 notification.
 */
void func_800C1D14(s32 actor_id, s32 flags)
{
    FieldActorRecord* actor;

    if (flags & 1)
    {
        func_8008BD88(actor_id);
    }
    actor = func_800C1B60(actor_id);
    actor->script.depth = 0;
    actor->script.frames[0].pc = NULL;
    actor->script.status.word &= 0x7FFFFFFF;
}

/**
 * @brief Stop every actor that is not script-only and disable its event 8.
 */
void func_800C1D68(void)
{
    s32 i;

    for (i = 0; i < D_80122B78->state.actor_count; i++)
    {
        if (!D_80122B78->actors[i].flags.bits.script_only)
        {
            D_80122B78->actors[i].enabled_events &= 0xFEFF;
            func_800C1D14(D_80122B78->actors[i].id, 1);
        }
    }
}

/**
 * @brief Empty loop over the actor count; the body was compiled away.
 */
void func_800C1E08(void)
{
    s32 i;
    u16 count;

    i = 0;
    count = D_80122B78->state.actor_count;
    if (count != 0)
    {
        do
        {
            i += 1;
        } while (i < (s32)count);
    }
}

/**
 * @brief Find the resource record whose leading halfword equals @p resource_id.
 * @param resource_id Resource id to look for.
 * @return The record, or NULL after reporting the failed lookup.
 */
u16* func_800C1E40(s32 resource_id)
{
    u32* base;
    u16* record;
    s32 i;
    u32 count;

    base = func_800875B4();
    count = base[0] >> 2;
    for (i = 0; i < (s32)count; i++)
    {
        record = (u16*)((u8*)base + base[i]);
        if (*record == resource_id)
        {
            return record;
        }
    }
    record_game_diagnostic(0x8001, 0x6B, resource_id, 0);
    return NULL;
}
