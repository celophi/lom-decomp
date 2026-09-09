#include "common.h"

/** @brief Active field context position and packed script selector fields. */
typedef struct
{
    u8 pad0[0x58];
    s32 unk58;
    u8 pad5C[0x41C - 0x5C];
    s32 unk41C;
} Context;
extern Context *D_80122B78;
extern u8 D_800EF84C[];
extern void func_80087F44(u32, s32 *);
extern s32 func_8008B288(u32);
extern u8 *func_800C1B60(u32, Context *);
/**
 * @brief Convert the record selector sentinel to a signed invalid value.
 * @param record Record containing the selector at byte one.
 * @return Selector value, or -1 for the 0xFF sentinel.
 */
static inline s32 resolve_byte(u8 *record)
{
    s32 value = -1;
    if (record[1] != 0xFF)
    {
        value = record[1];
    }
    return value;
}
/**
 * @brief Resolve script actor, plane, effect, and selector operands in place.
 * @param actor_id Actor identifier; invalid identifiers are replaced with zero.
 * @param plane Plane selector or automatic-selection sentinel.
 * @param effect Effect selector, updated with facing flags or -1 when unavailable.
 * @param selector Selector index or current-context sentinel.
 */
void func_800B2654(s32 *actor_id, s32 *plane, s32 *effect, s32 *selector)
{
    s32 position[3];
    s32 plane_value;
    s32 effect_value;
    s32 facing_flag;
    s32 selector_value;
    u32 original_actor_id;

    original_actor_id = *actor_id;
    if (original_actor_id < 0x80U)
    {
        func_80087F44(original_actor_id, position);
    }
    else
    {
        *actor_id = 0;
    }
    plane_value = *plane;
    if (plane_value != 0xFF)
    {
        if (plane_value & 0x80)
        {
            facing_flag = 0;
        }
        else if (plane_value & 0x40)
        {
            facing_flag = 0x40;
        }
        else
        {
            facing_flag = ((u32)(func_8008B288(*actor_id) - 0x41) < 0x80U) << 6;
        }
        *plane &= 3;
    }
    else
    {
        if ((position[2] - D_80122B78->unk58) <= 0xBFFF)
        {
            *plane = 0;
        }
        else
        {
            *plane = 1;
        }
        facing_flag = ((u32)(func_8008B288(*actor_id) - 0x41) < 0x80U) << 6;
    }
    D_80122B78->unk41C = (s32)((D_80122B78->unk41C & ~0x300) | ((*plane & 3) << 8));
    effect_value = *effect;
    switch (effect_value)
    { /* irregular */
    case 0xFE:
        *effect = resolve_byte(func_800C1B60(original_actor_id, D_80122B78));
        break;
    case 0xFF:
        *effect = -1;
        break;
    }
    *effect |= facing_flag;
    selector_value = *selector;
    if (selector_value == 0xFF)
    {
        selector_value = (s32)(u8)D_80122B78->unk41C;
    }
    *selector = selector_value;
    if (!(((s32)D_800EF84C[selector_value] >> *plane) & 1))
    {
        *effect = -1;
    }
}
