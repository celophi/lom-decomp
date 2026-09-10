#include "common.h"

#define ACCESS(type, base, offset) (*(type *)((u8 *)(base) + (offset)))
/** @brief Selection descriptor containing the callback table index. */
typedef struct
{
    u8 pad[2];
    u8 filter;
} FilterSpec;
/** @brief Predicate for a source actor, candidate actor and caller argument. */
typedef s32 (*ActorFilter)(u8 *, u8 *, s32);
extern ActorFilter D_800EC2D8[];
extern u8 D_800FDF58[];
extern s32 D_800FE754;

/** @brief Fields consulted in a 0x23C-byte FIELD actor state. */
typedef struct
{
    u8 pad0[4];
    s32 active;
    u8 pad8[4];
    s32 flags_c;
    s32 category;
    u8 pad14[0x12C - 0x14];
    s32 valid;
    u8 pad130[0x174 - 0x130];
    s32 flags174;
    s32 flags178;
    u8 pad17c[0x23C - 0x17C];
} FieldState;
/** @brief Binding state and its owning actor in a 0x1C-byte record. */
typedef struct
{
    s32 state;
    u8 pad4[8];
    s32 owner;
    u8 pad10[12];
} Binding;
extern Binding D_80105880[];
extern FieldState D_80105AE0[];
extern s32 D_8010D020;

/**
 * @brief Collect eligible FIELD actor indices using a selected predicate.
 * @param source_index Actor excluded from the candidate list.
 * @param spec Descriptor selecting a predicate from D_800EC2D8.
 * @param group_mode Zero selects the opposite group; one selects the same group.
 * @param filter_arg Additional predicate argument.
 * @param output Destination for the accepted indices.
 * @return Number of indices written to output.
 * @note Unless D_8010D020 enables all slots, group_mode must be zero or one.
 */
s32 func_8009D1E4(s32 source_index, FilterSpec *spec, s32 group_mode, s32 filter_arg, s32 *output)
{
    s16 actor_state;
    s32 *output_cursor;
    s32 flags_or_offset;
    s32 is_party_slot;
    s32 index;
    s32 count;
    s32 end;
    s32 start;
    s32 owner_slot;
    s32 state_slot;
    s32 prior;
    u8 *actor_start;
    FieldState *state;
    FieldState *state_start;
    u8 *actor_state_ptr;
    u8 *actor;
    Binding *bindings;

    if (D_8010D020 != 0)
    {
        start = 0;
        end = 13;
    }
    else
    {
        switch (group_mode)
        {
        case 0:
            if (source_index < 3)
            {
                start = 3;
                end = 13;
            }
            else
            {
                start = 0;
                end = 3;
            }
            break;
        case 1:
            if (source_index < 3)
            {
                start = 0;
                end = 3;
            }
            else
            {
                start = 3;
                end = 13;
            }
            break;
        }
    }
    count = 0;
    index = start;
    actor_start = (index * 0x54) + D_800FDF58;
    state_start = &D_80105AE0[index];
    if (index < end)
    {
        bindings = D_80105880;
        state = state_start;
        actor_state_ptr = actor_start + 0x2A;
        actor = actor_start;
        output_cursor = output;
        do
        {
            if ((index != source_index) && (ACCESS(u8, actor_state_ptr, -5) != 0xFF) &&
                (state->active != 0))
            {
                flags_or_offset = state->flags178;
                if (!(flags_or_offset & 1) &&
                    ((D_800FE754 == (state->category & 0xF)) || (index < 3)) &&
                    !(flags_or_offset & 0x20))
                {
                    actor_state = ACCESS(s16, actor_state_ptr, 0);
                    if ((actor_state != 0x91) && (actor_state != 0xAE) && (actor_state != 0x87))
                    {
                        is_party_slot = index < 3;
                        if (!(flags_or_offset & 0x40))
                        {
                            owner_slot = index;
                            if (is_party_slot == 0)
                            {
                                owner_slot = 2;
                            }
                            if (bindings[owner_slot].owner == index)
                            {
                                state_slot = index;
                                if (is_party_slot == 0)
                                {
                                    state_slot = 2;
                                }
                                if (bindings[state_slot].state == 0)
                                {
                                    goto check_remaining_flags;
                                }
                            }
                            else
                            {
                                goto check_remaining_flags;
                            }
                        }
                        else
                        {
                        check_remaining_flags:
                            if (!(state->flags_c & 0x2280) && (state->valid != 0) &&
                                !(state->flags174 & 0x8000) && !(state->flags178 & 0x80))
                            {
                                /* Preserve the staged calculation of the 0x54-byte actor stride. */
                                flags_or_offset = source_index * 5;
                                flags_or_offset = flags_or_offset * 4 + source_index;
                                flags_or_offset *= 4;
                                if (D_800EC2D8[spec->filter](flags_or_offset + D_800FDF58, actor,
                                                             filter_arg) != 0)
                                {
                                    /* This empty scan is present in the original code. */
                                    for (prior = 0; prior < count; prior++)
                                    {
                                    }
                                    *output_cursor = index;
                                    output_cursor++;
                                    count += 1;
                                }
                            }
                        }
                    }
                }
            }
            actor += 0x54;
            index += 1;
            actor_state_ptr += 0x54;
            state++;
        } while (index < end);
    }
    return count;
}
