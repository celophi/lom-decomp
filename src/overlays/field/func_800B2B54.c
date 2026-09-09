#include "common.h"

/** @brief Active-state word and applied-effect flags. */
typedef struct
{
    u32 pad;
    u32 active;
    u32 pad8;
    u32 flags;
} State;
/** @brief Actor fields used to validate and scale an effect. */
typedef struct
{
    u8 pad[0x10];
    State *state;
    u8 pad14[0x24];
    u8 immunity;
    u8 pad39[0x14];
    volatile u8 slots[3];
    u16 values[15];
} Actor;
extern u8 D_800F0B28[], D_800F0B38[];
extern s32 rand(void);
extern s32 func_800B2D34(Actor *, s32);
/**
 * @brief Apply a permitted effect with a chance check and scaled duration.
 * @param source Actor providing the offensive scale.
 * @param target Actor receiving the effect.
 * @param flags Bit 0 bypasses immunity; bit 1 permits an already active effect.
 * @param type Effect index, accepted when below 15.
 * @param chance Threshold compared against an eight-bit random value.
 * @param duration Base duration scaled by the actors and capped at 240.
 */
void func_800B2B54(Actor *source, Actor *target, s32 flags, s32 type, s32 chance, s32 duration)
{
    s32 mask, i, attack, defense, value;
    u8 *table, *scales;
    Actor *slot;
    if (type >= 15)
    {
        return;
    }
    if (target->state->active == 0)
    {
        return;
    }
    if (!(flags & 1))
    {
        table = D_800F0B28;
        if (target->immunity & table[type])
        {
            return;
        }
        mask = 0;
        i = mask;
        slot = (Actor *)((u8 *)target + i);
        do
        {
            if ((u32)(slot->slots[0] - 0x60) < 0x10)
            {
                mask |= table[slot->slots[0] - 0x60];
            }
            i++;
            slot = (Actor *)((u8 *)target + i);
        } while (i < 3);
        if (mask & D_800F0B28[type])
        {
            return;
        }
    }
    mask = 1 << type;
    if (!(flags & 2) && (target->state->flags & mask))
    {
        return;
    }
    if ((rand() & 0xFF) >= chance)
    {
        return;
    }
    target->state->flags |= mask;
    scales = &D_800F0B38[type];
    attack = func_800B2D34(source, *scales >> 4);
    defense = func_800B2D34(target, *scales & 0xF);
    value = duration * attack / defense;
    slot = (Actor *)((u8 *)target + type * 2);
    if (value > 0xF0)
    {
        value = 0xF0;
    }
    slot->values[0] = value;
}
