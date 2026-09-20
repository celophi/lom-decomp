#include "field_text.h"
/** @file field_actor_input_actions.c
 * @brief Map controller input to actor actions, animation IDs, and input-state changes.
 */

/* func_80091728 */
#include "common.h"

typedef struct
{
    u8 unk0;
    u8 pad1;
    u16 unk2;
} PadState;

typedef struct
{
    u8 pad0[0x1C];
    u32 unk1C;
    u8 pad20[0x3A - 0x20];
    u8 unk3A;
} Arg2Struct;

extern u8 D_800EB23C[];
extern u8 D_800EB244[];
extern u8 *g_pad_ctx;

/**
 * @brief Build the active input mask for a field menu entry.
 * @param arg0 Controller slot index.
 * @param arg1 Input mapping index.
 * @param arg2 Field menu entry state.
 * @return The mapped active-button mask, or zero when input is unavailable.
 */
s32 func_80091728(s32 arg0, s32 arg1, Arg2Struct *arg2)
{
    s32 mask;
    s32 i;
    u8 want;
    volatile PadState *pad;
    u32 bits;
    u32 pad_base;
    u8 *rec_base;

    pad_base = 0x801ED600;
    if (arg2->unk3A < 3)
    {
        if ((arg2->unk1C & 0x1FF) == 0)
        {
            mask = 0;
            i = 0;
            want = D_800EB244[arg1];
            rec_base = g_pad_ctx + arg0 * 0x250;
            do
            {
                if (want == *(rec_base + i + 0x638))
                {
                    mask |= D_800EB23C[i];
                }
                i += 1;
            } while (i < 8);
            pad = (volatile PadState *)(pad_base + arg0 * 0xAE);
            if (pad->unk0 < 0xFE)
            {
                bits = (u32)pad->unk2 >> 8;
                return (((bits >> 1) & 0x20) | ((bits & 0x20) * 2) | ((bits >> 3) & 0x10) | ((bits & 0x10) * 8) | (bits & 0xF)) & mask;
            }
            return 0;
        }
        return 0;
    }
    return 0;
}


/* func_8009184C */
#include "common.h"

typedef struct {
    u8 pad0[0x2A];
    s16 unk2A;
    u8 pad2C[0x54-0x2C];
} Entry;

extern Entry g_field_actors[];
extern s32 g_field_active_group;
extern s32 D_800F229C;
extern s32 D_8010AE78;
extern s32 g_field_buffered_input;

void field_probe_actor_interaction(Entry *arg0);

void func_8009184C(void)
{
    s32 a;
    s32 b;

    if ((g_field_active_group == 0) && (g_field_actors[0].unk2A == 0) &&
        (D_800F229C == 0) && (D_8010AE78 == 0) && (g_field_buffered_input & 0x220))
    {
        a = field_text_get_status(0);
        b = field_text_get_status(1);
        if ((g_field_active_group == 0) && (a == -1) && (b == a))
        {
            field_probe_actor_interaction(g_field_actors);
        }
    }
}


/* func_80091914 */
#include "common.h"

/** @brief Partial actor state used to choose the next action animation. */
typedef struct
{
    u8 pad[0x21];
    u8 mode;
    u8 pad22[14];
    s16 counter;
    u8 pad32[8];
    u8 slot;
} Actor;
/** @brief Partial 0x23C-byte actor slot with action-control flags. */
typedef struct
{
    u8 pad[12];
    u32 flags;
    u8 pad10[0x17D];
    u8 active;
    u8 pad18E[0xAE];
} Slot;
/** @brief Eleven animation IDs followed by their disabled flags. */
typedef struct
{
    u16 animations[11];
    u8 disabled[11];
    u8 pad;
} ActionMap;
extern Slot g_field_object_states[];
extern u8 g_field_action_animation_maps[];
extern s32 g_field_action_context;
extern s32 D_8010AE54;
s32 func_800A29F8(s32, s32, s32);
void func_800A3938(s32, s32);

/**
 * @brief Resolve an actor action into an enabled animation ID.
 * @param actor Actor state to query and update.
 * @param map_index Action-animation map passed to the action selector.
 * @return Enabled animation ID, or zero when no animation is available.
 * @note Materialize map_base before applying the map stride; the ordering is
 * required for the exact address setup. Keep the repeated animation load.
 * @note GCC 2.7.2 CDK: 100% match, 109 instructions (436 bytes).
 */
u16 func_80091914(Actor *actor, s32 map_index)
{
    s32 action;
    s32 in_range;
    ActionMap *map;
    u16 *animation;
    s32 map_base;

    action = func_800A29F8(map_index, (actor->mode >> 7) ^ 1, 0);
    if (action != 0xFF)
    {
        g_field_action_context = (g_field_action_context & ~0xFF) | action;
    }
    else if ((actor->mode & 0x7F) != 0x3D)
    {
        actor->counter = 0;
        g_field_object_states[actor->slot].flags &= 0xFFFF7FFF;
        g_field_object_states[actor->slot].active = 0;
    }
    if (g_field_object_states[actor->slot].flags & 0x400)
    {
        if ((u32)(action - 2) >= 2)
        {
            if (action != 0xFF)
            {
                func_800A3938(0x78, 0x80);
                return 0;
            }
            goto no_animation;
        }
        goto check_range;
    }
check_range:
    if (action < 11)
    {
        if (D_8010AE54 == 0 || action < 4)
        {
            map_base = (s32)g_field_action_animation_maps;
            map = (ActionMap *)(map_index * 0x22 + map_base);
            animation = (u16 *)(action * 2 + (s32)map);
            if (*animation != 0)
            {
                if (map->disabled[action] == 0)
                {
                    return *animation;
                }
                return 0;
            }
            goto no_animation;
        }
        return 0;
    }
no_animation:
    return 0;
}


/* func_80091AC8 */
#include "common.h"

typedef struct
{
    u8 state;
    u8 pad1;
    u16 input;
    u8 pad4[0xAE - 4];
} FieldInputRecord91AC8;

typedef struct
{
    u8 pad0[0x24];
    u8 unk24;
    u8 pad25[0x33 - 0x25];
    u8 unk33;
    u8 pad34[0x3B - 0x34];
    u8 resource_index;
} FieldActor91AC8;

typedef struct
{
    u8 pad0[0x10];
    u32 flags;
} FieldResourceEntry91AC8;

extern FieldResourceEntry91AC8 g_field_resource_entries[];

#define FIELD_INPUT_RECORDS ((FieldInputRecord91AC8 *)0x801ED600)

/**
 * @brief Update an actor's input state from its field input record.
 * @param actor Actor state to update.
 * @param index Input-record index to read.
 */
void func_80091AC8(FieldActor91AC8 *actor, s32 index)
{
    u16 raw;
    s32 input;
    FieldInputRecord91AC8 *records;

    records = FIELD_INPUT_RECORDS;
    if (!(g_field_resource_entries[actor->resource_index].flags & 1))
    {
        input = 0;
        if (records[index].state < 0xFE)
        {
            raw = records[index].input;
            input = ((raw << 8) & 0xFF00) | (raw >> 8);
        }

        input = ((u32)(input & 0x40) >> 1) | ((input & 0x20) * 2) | ((u32)(input & 0x80) >> 3) | ((input & 0x10) * 8) | (input & ~0xF0);
        if (input & 0x44)
        {
            if (actor->unk33 == 0)
            {
                actor->unk33 = 1;
                actor->unk24 = 0;
            }
        }
        else if (actor->unk33 != 0)
        {
            actor->unk33 = 0;
            actor->unk24 = 0;
        }
    }
}
