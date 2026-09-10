#include "common.h"
/** @brief Position, state, timers, and slot in a 0x54-byte actor record. */
typedef struct
{
    s32 x, y;
    u8 pad8[0x19];
    u8 state;
    u8 pad22[2];
    u8 active;
    u8 pad25[2];
    u8 unknown27;
    u8 pad28[2];
    s16 value;
    u8 pad2c[2];
    u16 timer;
    u16 counter;
    u8 pad32[5];
    s8 height;
    u8 pad38[2];
    u8 slot;
    u8 tail[0x19];
} FieldReactionActor;
/** @brief Flags and reaction state in a 0x23C-byte actor slot. */
typedef struct
{
    u8 pad0[8];
    s32 value;
    u32 flags;
    u8 pad10[0x164];
    u32 options;
    u32 state;
    u8 pad17c[0x11];
    u8 unknown18d;
    u8 tail[0xAE];
} FieldReactionSlot;
/** @brief Reaction marker in a 0x268-byte player record. */
typedef struct
{
    u8 pad0[0x259];
    u8 state;
    u8 tail[0xE];
} FieldReactionPlayer;
/** @brief Active state and actor index in a 0x1C-byte selection record. */
typedef struct
{
    s32 active;
    u8 pad4[8];
    s32 actor_index;
    u8 tail[0xC];
} FieldReactionSelection;
extern FieldReactionPlayer D_800FD818[];
extern FieldReactionSlot D_80105AE0[];
extern FieldReactionSelection D_80105880[];
extern s32 D_8010A000;
extern void func_8006C3FC(FieldReactionActor *);
extern void func_800952DC(FieldReactionActor *, s32);
extern void func_8008BC5C(FieldReactionActor *);
extern void field_stop_actor_animations_for_object(FieldReactionActor *, s32);
extern void func_800A2DD8(s32);
extern s32 rand(void);

/**
 * @brief Set an actor reaction state and reset its motion and animation flags.
 * @param actor Actor receiving the reaction, subject to selection and state checks.
 * @param alternate Nonzero selects state 0x0B; zero randomly selects 0x14 or 0x15.
 * @note Temporarily remove the signed height offset while changing animations.
 */
void func_8008B870(FieldReactionActor *actor, s32 alternate)
{
    s32 offset;
    u16 value;
    s32 flags;
    FieldReactionSlot *slot;
    FieldReactionSlot *slots;
    FieldReactionSelection *selection;
    FieldReactionSelection *actor_selection;

    if (actor->slot < 3U)
    {
        D_800FD818[actor->slot].state = 5;
    }
    else if (D_80105AE0[actor->slot].value < 0)
    {
        D_8010A000 = 5;
    }
    if (!((D_80105AE0[actor->slot].state >> 6) & 1))
    {
        selection = D_80105880;
        if (actor->slot < 2U)
        {
            offset = actor->slot * 0x1C;
        }
        else
        {
            offset = 0x38;
        }
        if (((FieldReactionSelection *)((u8 *)selection + offset))->active != 0)
        {
            actor_selection = D_80105880;
            if (actor->slot < 2U)
            {
                offset = actor->slot * 0x1C;
            }
            else
            {
                offset = 0x38;
            }
            if (((FieldReactionSelection *)((u8 *)actor_selection + offset))->actor_index == actor->slot)
            {
                return;
            }
        }
    }
    value = actor->value;
    if ((u32)(value - 0x93) >= 2U && (s16)value != 0x90)
    {
        slots = D_80105AE0;
        slot = &slots[actor->slot];
        flags = slot->flags;
        if (!(flags & 0x200))
        {
            if ((*(u8 *)&slot->state & 1) || (flags & 0x23E4))
            {
                actor->timer = 1;
                actor->unknown27 = 0;
                actor->active = 1;
                actor->state &= 0x80;
                func_8006C3FC(actor);
                actor->value = 0x82;
                return;
            }
            if ((actor->state & 0x7F) != 0x44)
            {
                slot->state &= ~0x40;
                func_800952DC(actor, 0);
                actor->value = 0x82;
                actor->y -= actor->height << 8;
                D_80105AE0[actor->slot].unknown18d = 0;
                actor->counter = 0;
                if (alternate != 0)
                {
                    actor->state = (actor->state & 0x80) + 0xB;
                }
                else
                {
                    actor->state = (actor->state & 0x80) + 0x14;
                    actor->state += rand() & 1;
                }
                func_8008BC5C(actor);
                actor->timer = 1;
                actor->active = 1;
                actor->unknown27 = 0;
                D_80105AE0[actor->slot].options &= ~0x1800;
                func_8006C3FC(actor);
                actor->y += actor->height << 8;
                if (actor->y > 0)
                {
                    actor->y = 0;
                }
                D_80105AE0[actor->slot].flags &= ~0x4000;
                D_80105AE0[actor->slot].flags &= 0xFFFF7FFF;
                field_stop_actor_animations_for_object(actor, 0);
                if (actor->slot < 2U)
                {
                    func_800A2DD8(actor->slot);
                    D_80105AE0[actor->slot].unknown18d = 0;
                    actor->counter = 0;
                }
            }
        }
    }
}
