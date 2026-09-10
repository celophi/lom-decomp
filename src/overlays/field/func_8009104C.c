#include "common.h"

/** @brief Animation definition view with the original 0x1C-byte stride. */
typedef struct
{
    u8 pad0[0x12];
    u16 unk12;
    u8 pad14[2];
    u8 unk16;
    u8 pad17[5];
} AnimationDef;

/** @brief Actor slot view with the original 0x244-byte stride. */
typedef struct
{
    u8 pad0[0xC];
    AnimationDef *unkc;
    AnimationDef *unk10;
    u8 pad14[0x10];
    u8 unk24;
    u8 pad25[4];
    u8 unk29;
    u8 unk2a;
    u8 pad2b[0x222 - 0x2B];
    u16 unk222;
    u8 pad224[0xF];
    u8 unk233;
    u8 pad234[4];
    s16 unk238;
    u8 pad23a[0xA];
} ActorSlot;

/** @brief Pending actor-slot request, indexed with the original 0x1C-byte stride. */
typedef struct
{
    s32 unk0;
    u8 pad4[8];
    s32 unkc;
    u8 pad10[8];
    s32 unk18;
} SlotRequest;

/** @brief Actor state view containing the active slot identifier. */
typedef struct
{
    u8 pad0[0x179];
    u8 unk179;
    u8 pad17a[0x23C - 0x17A];
} ActorState;

void bcopy(void *, void *, s32);
void field_start_actor_animation(s32, s32, u8 *);
s32 func_800839F8(s32, s32);
extern SlotRequest D_80105880[];
extern ActorState D_80105AE0[];
extern ActorSlot g_field_actor_slots[];

/**
 * @brief Start the requested actor animation and initialize any extra layers.
 * @param actor_id Actor owning the pending slot request.
 * @param target_count Number of animation targets.
 * @param targets Animation target identifiers.
 * @param flags Layer-selection flags; other bit meanings are unknown.
 * @return One when the request was applied, or zero when it was not ready.
 * @note GCC 2.7.2 CDK currently matches 98.983406 percent of the target.
 */
s32 func_8009104C(s32 actor_id, s32 target_count, u8 *targets, s32 flags)
{
    u16 frame_value;
    ActorSlot *slot_base;
    s32 remaining_layers;
    s32 layer_offset;
    s32 result;
    s32 request_index;
    s32 new_slot_id;
    s32 layer_index;
    ActorSlot *slot;
    SlotRequest *request;
    ActorSlot *base_slot;
    AnimationDef *animation;
    ActorSlot *default_slot;
    ActorSlot *selected_slot;
    ActorSlot *layer_slot;
    ActorSlot *initialized_slot;

    request_index = actor_id;
    if (actor_id >= 3)
    {
        request_index = 2;
    }
    request = &D_80105880[request_index];
    if (request->unkc != actor_id)
    {
        return 0;
    }
    if (request->unk0 != 2)
    {
        return 0;
    }

    {
        if (flags & 0x4000)
        {
            if (!(flags & 0x400))
            {
                g_field_actor_slots[request->unk18].unk29 = (s8) ((flags >> 0xC) & 3);
                layer_slot = &g_field_actor_slots[request->unk18];
                layer_slot->unk222 = layer_slot->unk10[layer_slot->unk29].unk12;
            }
            else
            {
                g_field_actor_slots[request->unk18].unk29 = 0;
                base_slot = &g_field_actor_slots[request->unk18];
                remaining_layers = (flags >> 0xC) & 3;
                base_slot->unk222 = (u16) base_slot->unk10->unk12;
                layer_index = 1;
                g_field_actor_slots[request->unk18].unk2a = 1;
                if (remaining_layers != 0)
                {
                    slot_base = g_field_actor_slots;
                    layer_offset = 0x1C;
                    do
                    {
                        new_slot_id = func_800839F8(actor_id, 0);
                        if (new_slot_id != -1)
                        {
                            slot = &slot_base[new_slot_id];
                            bcopy(&slot_base[request->unk18], slot, 0x244);
                            animation = slot->unk10;
                            slot->unk233 = new_slot_id;
                            slot->unk29 = layer_index;
                            frame_value = animation->unk12;
                            slot->unkc = (AnimationDef *)((u8 *)animation + layer_offset);
                            slot->unk238 = 0;
                            result = 1;
                            slot->unk2a = result;
                            slot->unk24 = result;
                            slot->unk222 = frame_value;
                            field_start_actor_animation(new_slot_id, target_count, targets);
                        }
                        remaining_layers -= 1;
                        layer_offset += 0x1C;
                        layer_index += 1;
                    } while (remaining_layers != 0);
                }
            }
        }
        else
        {
            default_slot = &g_field_actor_slots[request->unk18];
            default_slot->unk222 = (u16) default_slot->unk10->unk12;
            g_field_actor_slots[request->unk18].unk29 = 0;
        }
        initialized_slot = &g_field_actor_slots[request->unk18];
        initialized_slot->unk238 = initialized_slot->unk10[initialized_slot->unk29].unk16;
        selected_slot = &g_field_actor_slots[request->unk18];
        selected_slot->unkc = selected_slot->unk10 + selected_slot->unk29;
        g_field_actor_slots[request->unk18].unk24 = 1;
        field_start_actor_animation(request->unk18, target_count, targets);
        D_80105AE0[actor_id].unk179 = (u8) request->unk18;
    }
    result = 1;
    return result;
}
