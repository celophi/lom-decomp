#include "common.h"

/** @brief Partial 0x54-byte actor layout used by the action dispatcher. */
typedef struct
{
    u8 pad0[0x21];
    u8 unk21;
    u8 pad22[2];
    u8 unk24;
    u8 pad25[2];
    u8 unk27;
    u8 pad28[2];
    union
    {
        u16 word;
        u8 bytes[2];
    } state;
    u8 pad2c[2];
    u16 unk2E, unk30;
    u8 pad32[8];
    u8 unk3A, unk3B;
    u8 pad3c[0x54 - 0x3C];
} Actor;
/** @brief Partial 0x23C-byte actor-slot layout and overlapping status bytes. */
typedef struct
{
    u8 pad0[0x3C];
    s32 unk3C;
    u8 pad40[8];
    u16 unk48;
    u8 pad4a[2];
    s32 unk4C;
    u8 pad50[0x16F - 0x50];
    u8 unk16F;
    u8 pad170[4];
    s32 unk174;
    union
    {
        s32 word;
        u8 bytes[4];
    } flags;
    u8 pad17c[0x23C - 0x17C];
} Slot;
/** @brief Eight-byte resource action descriptor. */
typedef struct
{
    u16 unk0, unk2, unk4, unk6;
} Action;
/** @brief Resource directory entry containing the availability flags. */
typedef struct
{
    u8 pad0[0x10];
    s32 flags;
} Resource;
/** @brief Party entry with the action-table bank selector at offset one. */
typedef struct
{
    u8 unk0, unk1;
    u8 pad2[0x268 - 2];
} Party;
extern Slot D_80105AE0[];
extern Action D_8010A038[];
extern Party D_800FD818[];
extern Resource g_field_resource_entries[];
extern s32 D_8010AE58;
s32 field_object_has_active_actor_tracks(s32);
s32 field_count_free_actor_slots(s32);
s32 func_8008404C(s32, s32);
void func_800A3938(s32, s32);
void func_8006C3FC(Actor *);
s32 func_800839F8(s32, s32);
s32 func_80083EEC(s32, s32, s32);
void field_start_actor_animation(s32, s32, u8 *);
void func_8009D4D8(Actor *, s32);
/**
 * @brief Validate and dispatch the pending resource action for an actor.
 *
 * The high command byte selects a resource descriptor. Failed availability or
 * slot checks clear the pending command; accepted actions update slot flags,
 * optionally start an animation, then apply the descriptor's action flags.
 *
 * @param actor Actor whose pending command and action state are updated.
 */
void func_8008E690(Actor *actor)
{
    s32 animation_slot;
    s32 resource_offset;
    s32 party_offset;
    s32 action_index;
    u16 requirement;
    u16 animation;
    u8 actor_index;
    u8 mode;
    Slot *slot;
    Action *action;

    if (actor->state.bytes[0] == 0x85)
    {
        D_80105AE0[actor->unk3A].flags.word = (s32)(D_80105AE0[actor->unk3A].flags.word & ~0x1C);
        if (g_field_resource_entries[actor->unk3B].flags & 1)
        {
            D_80105AE0[actor->unk3A].unk16F = (s8)(actor->state.word >> 8);
            /* Each resource owns 50 eight-byte descriptors. */
            resource_offset = actor->unk3B * 0x190;
            actor->state.word = actor->state.bytes[0];
            slot = &D_80105AE0[actor->unk3A];
            mode = slot->unk16F;
            action = (Action *)(resource_offset + (s32)&D_8010A038[mode]);
            if (mode == 0xB)
            {
                slot->unk16F = *(u8 *)&action->unk0;
            }
            if (!(action->unk2 & 0x400))
            {
                if (action->unk0 == 0 && action->unk4 == 0)
                {
                    goto play_failure;
                }
                /* Preserve the descriptor re-read present in the original path. */
                if (!(((volatile Action *)action)->unk2 & 0x400))
                {
                    goto check_action;
                }
            }
            if ((field_object_has_active_actor_tracks(actor->unk3A) == 0) &&
                (field_count_free_actor_slots(actor->unk3A) >= 3) && (actor->unk30 == 0))
            {
            check_action:
                D_80105AE0[actor->unk3A].flags.word =
                    (s32)(D_80105AE0[actor->unk3A].flags.word & ~2);
                if (((u16)action->unk0 & 0x8000) && !(action->unk2 & 0x400))
                {
                    if ((field_object_has_active_actor_tracks(actor->unk3A) == 0) &&
                        (D_8010AE58 == 0) && (field_count_free_actor_slots(actor->unk3A) >= 3))
                    {
                        actor_index = actor->unk3A;
                        if (D_80105AE0[actor_index].unk48 != 0xFF)
                        {
                        play_failure:
                            func_800A3938(0x78, 0x80);
                        cancel_action:
                            actor->state.word = 0;
                            return;
                        }
                        action_index = action->unk0 & 0x7fff;
                        party_offset = (D_800FD818[actor_index].unk1 * 0x18) + 0x88;
                        if (func_8008404C(actor_index, action_index + party_offset) != 0)
                        {
                            D_80105AE0[actor->unk3A].unk174 =
                                (s32)(D_80105AE0[actor->unk3A].unk174 | 0x8000);
                            goto start_action;
                        }
                        goto cancel_action;
                    }
                    goto cancel_action;
                }
                requirement = action->unk6;
                if (!(requirement & 0x8000) ||
                    (func_8008404C(actor->unk3A, requirement & 0x3FF) != 0))
                {
                start_action:
                    if (action->unk2 & 0x400)
                    {
                        D_80105AE0[actor->unk3A].unk4C = (s32)(D_80105AE0[actor->unk3A].unk4C & ~1);
                        D_80105AE0[actor->unk3A].flags.word =
                            (s32)(D_80105AE0[actor->unk3A].flags.word | 0x40);
                        actor->unk2E = 1;
                        actor->unk24 = 1;
                        actor->unk27 = 0;
                        actor->unk21 = (u8)((actor->unk21 & 0x80) + 0x10);
                        D_80105AE0[actor->unk3A].unk174 =
                            (s32)(D_80105AE0[actor->unk3A].unk174 & ~0x1800);
                        func_8006C3FC(actor);
                        if (action->unk4 != 0)
                        {
                            D_80105AE0[actor->unk3A].unk3C = (s32)action->unk4;
                        }
                        D_80105AE0[actor->unk3A].unk174 =
                            (s32)(D_80105AE0[actor->unk3A].unk174 & ~0x400);
                    }
                    else if (!((u16)action->unk0 & 0x8000))
                    {
                        animation = action->unk4;
                        if ((animation != 0xFFFF) && (animation != 0))
                        {
                            animation_slot = func_800839F8(actor->unk3A, 0);
                            if ((animation_slot != -1) &&
                                (func_80083EEC(actor->unk3A, animation_slot, action->unk4) != 0))
                            {
                                field_start_actor_animation(animation_slot, 0, 0);
                                D_80105AE0[actor->unk3A].flags.bytes[1] = animation_slot;
                            }
                        }
                    }
                    func_8009D4D8(actor, *(u8 *)&action->unk2);
                }
                else
                {
                    goto cancel_action;
                }
            }
            else
            {
                goto cancel_action;
            }
        }
        else
        {
            goto cancel_action;
        }
    }
}
