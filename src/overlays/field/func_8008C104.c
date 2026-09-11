#include "common.h"

/** @brief Actor state and animation fields reset by this transition. */
typedef struct
{
    u32 pad0, word4;
    u8 pad8[0x14];
    u32 flags;
    u8 pad20, state, pad22[2], active, pad25[2], byte27, byte28, pad29;
    s16 animation;
    u16 pad_2c, value;
    u8 pad30[10], selector, resource;
} Actor;
/** @brief Actor record flags with the original 0x23C stride. */
typedef struct
{
    u8 pad[0xC];
    u32 flags;
    u8 pad10[0x164];
    u32 flags174;
    u8 flags178;
    u8 tail[0xC3];
} Record;
/** @brief Resource descriptor containing animation eligibility flags. */
typedef struct
{
    u8 pad[0xE];
    u16 flags;
    u8 tail[4];
} Resource;
extern Record D_80105AE0[];
extern u8 D_800FB3C8[];
extern Resource g_field_resource_entries[];
extern void func_8008BC5C(void), func_8008BCF8(Actor *), func_8006C3FC(Actor *);
extern void field_stop_actor_animations_for_object(Actor *, s32);
extern void func_80083BC0(Actor *, void *, s32), func_800952DC(Actor *, s32), func_80084424(s32),
    func_80083EEC(s32, s32, u16), field_start_actor_animation(s32, s32, s32);
/**
 * @brief Reset actor state and select the appropriate transition animation.
 * @param actor Actor whose state and animation resources are updated.
 * @return Unspecified value; callers do not consume the result.
 */
s32 func_8008C104(Actor *actor)
{
    Record *records, *record, *final_records;
    Resource *resource, *resources;
    s32 selector;
    func_8008BC5C();
    func_8008BCF8(actor);
    actor->animation = 0x8E;
    actor->byte28 = 0xFF;
    actor->value = 1;
    actor->active = 1;
    actor->word4 = 0;
    actor->byte27 = 0;
    actor->state = (actor->state & 0x80) + 0x1D;
    records = D_80105AE0;
    record = &records[actor->selector];
    record->flags174 &= ~0x1800;
    func_8006C3FC(actor);
    field_stop_actor_animations_for_object(actor, 1);
    func_80083BC0(actor, D_800FB3C8 + actor->selector * 0x244, 1);
    actor->flags |= 0x800;
    func_800952DC(actor, 0);
    func_80084424(actor->selector);
    selector = actor->selector;
    if (!(records[selector].flags178 & 1))
    {
        resources = g_field_resource_entries;
        resource = &resources[actor->resource];
        if (resource->flags & 0x8000)
        {
            actor->animation = 0x92;
        }
        else
        {
            func_80083EEC(selector, selector + 0x40, resource->flags);
            field_start_actor_animation(actor->selector + 0x40, 0, 0);
        }
    }
    final_records = D_80105AE0;
    if (!(final_records[actor->selector].flags & 0x200))
    {
        if (actor->animation == 0x92)
        {
            actor->animation = 0x93;
        }
        else
        {
            actor->animation = 0x90;
        }
    }
}
