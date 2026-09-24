/** @file field_actor_lifecycle.c
 * @brief Refresh, trigger and reset the field actor records and party slots.
 */

#include "game_audio.h"
#include "common.h"
#include "field_calls.h"
#include "field_records.h"

extern FieldGameState* D_80122B74;
extern FieldRuntimeContext* D_80122B78;
extern FieldBattleContext* D_80123FB0;
extern s32 D_8010D020;
extern s32 g_layout_flag;

extern FieldStatusState* func_80087F0C(s32 actor_id);
extern s32 func_80087FC0(s32 party_index, s32 mode);
extern void func_800C1D14(s32 party_index, s32 mode);
extern void akao_cmd_c1(s32 arg0, s32 arg1, s32 arg2);

/**
 * @brief Refresh the actor objects of every non-party actor record.
 */
void func_800B4390(void)
{
    s32 i;

    for (i = FIELD_PARTY_SIZE; i < D_80122B78->state.actor_count; i++)
    {
        func_80087F0C(D_80122B78->actors[i].id);
    }
}

/**
 * @brief Start a trigger group: notify its actors and reset the party actor scripts.
 * @param group Trigger group matched against each actor's option bits.
 */
void func_800B4410(s32 group)
{
    s32 index;

    for (index = FIELD_PARTY_SIZE; index < D_80122B78->state.actor_count; index++)
    {
        if (D_80122B78->actors[index].flags.bits.trigger_group == group)
        {
            func_80087614(D_80122B78->actors[index].id, group);
            func_800B28E0(D_80122B78->actors[index].id, 0xD, 0);
        }
    }

    {
        FieldRuntimeContext* context = D_80122B78;

        context->state.flags |= 0x10000;
        context->state.bytes.trigger_group = group;
        func_800966F0(group, context);
    }

    for (index = 0; index < FIELD_PARTY_SIZE; index++)
    {
        if ((D_80122B74->characters[index].info.bytes[0] >> 7) != 0)
        {
            func_80087FC0(index, 0);
        }
        else
        {
            D_80122B78->actors[index].enabled_events = 0xFFFF;
            func_80087FC0(index, 2);
            func_800B28E0(D_80122B78->actors[index].id, 0xF, 0);
        }
    }

    func_800B28E0(0x80, 0xD, 0);
}

/**
 * @brief Suspend the battle and notify the actors of the active trigger group.
 */
void func_800B4584(void)
{
    s32 i;
    FieldStatusState* state;
    FieldBattleContext* battle;

    battle = D_80123FB0;
    battle->state.flags |= 0x80000000;
    func_800966F0(0, battle);
    func_800B28E0(0x80, 0xD, 3);

    for (i = FIELD_PARTY_SIZE; i < D_80122B78->state.actor_count; i++)
    {
        if (D_80122B78->actors[i].flags.bits.trigger_group == D_80122B78->state.bytes.trigger_group)
        {
            func_800B28E0(D_80122B78->actors[i].id, 0xD, 3);
        }
    }

    for (i = 0; i < FIELD_PARTY_SIZE; i++)
    {
        state = func_80087F0C(i);
        if (state != (FieldStatusState*)-1)
        {
            state->current = state->maximum;
        }
    }
}

/**
 * @brief End the trigger group, reset the party actors' event tables and restart every actor.
 * @note Sends audio command C1 for layouts 3, 34, 35, 37, 43, 45, 46, and 47.
 */
void func_800B4684(void)
{
    s32 i;
    s32 j;

    D_80123FB0 = NULL;
    D_80122B78->state.flags &= ~0x10000;
    D_80122B78->state.bytes.trigger_group = 0;
    i = 0;

    do
    {
        if (D_80122B74->characters[i].info.bytes[0] >> 7)
        {
            func_80087FC0(i, 0);
        }
        else
        {
            D_80122B78->actors[i].enabled_events = 0;
            for (j = 0; j < FIELD_ACTOR_SCRIPT_COUNT; j++)
            {
                D_80122B78->actors[i].scripts[j] = FIELD_NO_SCRIPT;
            }
            func_80087FC0(i, 1);
            func_800C1D14(i, 0);
        }
        i++;
    } while (i < FIELD_PARTY_SIZE);

    i = 0;
    if (D_80122B78->state.actor_count != 0)
    {
        do
        {
            func_800B28E0(D_80122B78->actors[i].id, 13, 1);
            i++;
        } while (i < D_80122B78->state.actor_count);
    }

    if (D_8010D020 != 0)
    {
        D_8010D020 = 0;
    }
    else
    {
        switch (g_layout_flag)
        {
        case 3:
        case 34:
        case 35:
        case 37:
        case 43:
        case 45:
        case 46:
        case 47:
            akao_cmd_c1(0, 0x40, 0);
            break;
        }
    }
}

/**
 * @brief Find an offset-table record with the requested byte identifier.
 *
 * The table starts with a record count followed by one byte offset per
 * record, relative to the table base. A failed search records a diagnostic.
 *
 * @param base Base of the count and record-offset table.
 * @param value Byte identifier to find at record offset 0x18.
 * @return Pointer to the matching record, or NULL when absent.
 */
u8* func_800B4844(u32* base, s32 value)
{
    u32 index;
    u32 count;
    u32 loaded_count;
    u32* offset;
    u8* record;
    s32 target;

    index = 0;
    loaded_count = *base;
    target = value & 0xFF;
    if (loaded_count != 0)
    {
        count = loaded_count;
        offset = base;
        do
        {
            record = (u8*)base + offset[1];
            if (record[0x18] != target)
            {
                index++;
                offset++;
            }
            else
            {
                return record;
            }
        } while (index < count);
    }

    record_game_diagnostic(0x8001, 0x67, target, -1);
    return NULL;
}
