#include "common.h"
/** @brief Owner header and script-record fields addressed at a 12-byte stride. */
typedef struct
{
    u8 id, pad1[3], current_event, mode;
    u16 enabled_events;
    u16 scripts[16];
    u8 script_owner, pad29[3];
    s32 depth, pc, pad34, flags;
} Owner;
/** @brief Field state flag word controlling script selection. */
typedef struct
{
    u8 pad[0x400];
    s32 flags;
} State;
void akao_set_song_params(s32, s32, s32, s32);
void field_script_run(void *);
s32 func_80087EF0(s32);
/* Some callers forward a live owner ID without explicit argument setup. */
Owner *func_800C1B60();
s32 func_800C28F8(s32, s32);
extern State *D_80122B78;


/**
 * @brief Minimal command state used by func_800B286C.
 */
typedef struct FieldCommandState
{
    u8 unk0[4];
    u8 unk4;
    u8 unk5;
    u16 unk6;
} FieldCommandState;



/**
 * @brief Claims an enabled command slot when it is currently unassigned.
 *
 * @param arg0 Unused command context value.
 * @param arg1 Bit index and value stored in the claimed slot.
 * @param arg2 Secondary value stored in the claimed slot.
 * @return The low byte of @p arg1 on success, otherwise -1.
 */
s32 func_800B286C(s32 arg0, u8 arg1, s8 arg2)
{
    s32 index;
    FieldCommandState *state;

    state = (FieldCommandState *)func_800C1B60();
    index = arg1 & 0xFF;
    if (((s32)state->unk6 >> index) & 1)
    {
        if (state->unk4 == 0xFF)
        {
            state->unk4 = arg1;
            state->unk5 = arg2;
            return index;
        }
    }

    return -1;
}

/**
 * @brief Start an enabled owner event, preserving nested script depth.
 * @param owner_id Owner whose script context receives the event.
 * @param event_id Event index in the low byte; valid indices are zero through fifteen.
 * @param mode Execution mode copied into the owner for the script run.
 * @return Event index on success, or -1 for an unavailable event or depth overflow.
 * @note 100% match with GCC 2.8: 111 instructions, 444 bytes.
 */
s32 func_800B28E0(s32 owner_id, s32 event_id, s32 mode)
{
    s32 next_depth;
    s32 depth;
    u16 script_id;
    u32 event_index;
    Owner *owner;
    Owner *record;
    Owner *record_state;

    event_index = event_id & 0xFF;
    if (event_index < 0x10U)
    {
        owner = func_800C1B60(owner_id);
        if (((s32)owner->enabled_events >> event_index) & 1)
        {
            depth = owner->depth;
            owner->script_owner = owner_id;
            if (((Owner *)((u8 *)owner + ((depth * 3) << 2)))->pc != 0)
            {
                next_depth = depth + 1;
                owner->depth = next_depth;
                if (next_depth >= 8)
                {
                    owner->depth = 7;
                    akao_set_song_params(0x8001, 2, owner->id, event_index);
                    return -1;
                }
                goto select_script;
            }
        select_script:
            if ((D_80122B78->flags & 0x10000) && (owner_id < 3))
            {
                ((Owner *)((u8 *)owner + ((owner->depth * 3) << 2)))->pc =
                    func_800C28F8(owner_id, event_id & 0xFF);
                goto run_script;
            }
            script_id = owner->scripts[event_id & 0xFF];
            if (script_id != 0xFFFF)
            {
                ((Owner *)((u8 *)owner + ((owner->depth * 3) << 2)))->pc =
                    func_80087EF0(script_id & 0x7FFF);
            run_script:

                record = (Owner *)((u8 *)owner + ((owner->depth * 3) << 2));
                record->flags = (s32)(record->flags & ~1);
                record_state = (Owner *)((u8 *)owner + ((owner->depth * 3) << 2));
                record_state->flags = (s32)(record_state->flags & 1);
                owner->mode = mode;
                field_script_run((u8 *)owner + 0x28);
                owner->current_event = 0xFF;
                owner->mode = 0;
                return event_id & 0xFF;
            }
            return -1;
        }
        return -1;
    }
    return -1;
}
