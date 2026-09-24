#include "cdrom.h"
#include "common.h"
#include "field_calls.h"
#include "field_actor_tables.h"
#include "field_mesh.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

/**
 * @file field_actor_slot_resources.c
 * @brief Field animation actor slots: slot allocation, resource binding,
 *        teardown and the per-object animation state resets.
 */

/** @brief Binding index of an owner object: owners 2 and up share binding 2. */
#define FIELD_BINDING_INDEX(owner) ((owner) < 3 ? (owner) : 2)

/** @brief Resource entry in the field resource blob header (0x10 bytes). */
typedef struct
{
    u16 offset0;
    u16 offset2;
    u16 offset4;
    u16 fallback_value;
    u8 enabled;
    u8 unk9[7];
} FieldBlobEntry;

/**
 * @brief Byte view of FieldObjectState.unk4C, whose second byte holds the
 *        object's own index (0x23C bytes, overlays FieldObjectState).
 */
typedef struct
{
    u8 unk0[0x4D];
    s8 object_index;
    u8 unk4E[0x23C - 0x4E];
} FieldObjectIndexView;

extern u32 g_field_resource_blob[];
extern u8 g_field_resource_buffer[];
/** @brief End of the field resource blob payload. */
extern u8* D_801058D4;
/** @brief Base of the field resource blob. */
extern u8* D_801058D8;
extern s32 D_800F2278;
extern s32 D_800F227C;
extern s32 D_800F2280;
extern s32 D_8010CFD4;
extern s32 D_8010D034;
extern s32 g_field_boss_hud_shake_frame;

void func_80083BC0(FieldActor* actor, FieldActorSlot* slot, s32 force);
void func_80084424(s32 owner);
void field_clear_actor_effects(FieldActorSlot* slot);
void field_load_vram_resource(s32 id, s16* rect, s32 arg2);
void func_8008B724(void);
/* Defined as void (void) in field_actor_resource_unpack.c; the original passes load_id and tests the leftover $v0. */
s32 func_8009A364(s32 load_id);
void* func_8009CA54(s32 pool, s32 size, s32 tag);

/**
 * @brief Reset the field resource cursor pair to the base blob and its end.
 * @note D_801058D4 points one past the payload, using the length word stored
 *       at g_field_resource_blob[1].
 */
void func_80083948(void)
{
    D_801058D8 = (u8*)g_field_resource_blob;
    D_801058D4 = (u8*)g_field_resource_blob + g_field_resource_blob[1];
}

/**
 * @brief Upload the initial FIELD VRAM resource (palette strip and image) from
 *        the loaded resource buffer.
 */
void func_8008396C(void)
{
    RECT rect;
    u8* buf;
    u8* base;
    u8* data;
    u32 off;
    s32 w;
    s32 h;

    buf = g_field_resource_buffer;
    base = buf + 0x14;
    rect.x = 0;
    rect.y = 0x1EA;
    rect.w = 0x100;
    rect.h = 4;
    off = *(u32*)(buf + 8);
    LoadImage(&rect, (u_long*)base);
    data = base + off;
    w = *(u16*)(data - 4);
    h = *(u16*)(data - 2);
    rect.w = w;
    rect.h = h;
    rect.x = 0x180;
    rect.y = 0;
    LoadImage(&rect, (u_long*)data);
}

/**
 * @brief Find a free animation actor slot not already claimed by a binding.
 * @param binding_index Binding index (clamped to 2 when >= 3).
 * @param require_idle When non-zero, fail unless the binding is idle.
 * @return Index of the first free, unclaimed actor slot, or -1 if none.
 * @note Declared inline; func_8008404C expands it in place.
 */
inline s32 func_800839F8(s32 binding_index, s32 require_idle)
{
    s32 i;
    s32 j;
    FieldActorBinding* bindings;

    if (require_idle != 0)
    {
        bindings = g_field_actor_bindings;
        if (bindings[FIELD_BINDING_INDEX(binding_index)].state != 0)
        {
            return -1;
        }
    }

    for (i = 0; i < FIELD_ACTOR_SLOT_COUNT; i++)
    {
        if (g_field_actor_slots[i].active == 0)
        {
            for (j = 0; j < FIELD_ACTOR_BINDING_COUNT; j++)
            {
                if (g_field_actor_bindings[j].state != 0 && g_field_actor_bindings[j].slot == i)
                {
                    break;
                }
            }
            if (j == FIELD_ACTOR_BINDING_COUNT)
            {
                return i;
            }
        }
    }
    return -1;
}

/**
 * @brief Check whether a field object owns an actor with active animation tracks.
 * @param object_index Field-object index to match against each actor's owner.
 * @return 1 if a matching actor has active tracks, otherwise 0.
 */
s32 field_object_has_active_actor_tracks(s32 object_index)
{
    s32 slot_index;

    for (slot_index = 0; slot_index < FIELD_ACTOR_SLOT_COUNT; slot_index++)
    {
        if (g_field_actor_slots[slot_index].owner_object_index == object_index && g_field_actor_slots[slot_index].track_mask != 0)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Count free actor slots in the general-purpose actor pool.
 * @return Number of actor slots whose active flag is clear.
 */
s32 field_count_free_actor_slots(void)
{
    s32 free_count;
    s32 slot_index;

    free_count = 0;
    for (slot_index = 0; slot_index < FIELD_ACTOR_SLOT_COUNT; slot_index++)
    {
        if (g_field_actor_slots[slot_index].active == 0)
        {
            free_count++;
        }
    }

    return free_count;
}

/**
 * @brief Stop actor animations and reserved SFX channels for a field object.
 * @param actor Field actor whose object index selects the owned actor slots.
 * @param force Non-zero to force teardown of matching actor animations.
 */
void field_stop_actor_animations_for_object(FieldActor* actor, s32 force)
{
    FieldActorSlot* slot;
    s32 slot_index;

    slot = g_field_actor_slots;
    for (slot_index = 0; slot_index < FIELD_ACTOR_SLOT_COUNT; slot_index++, slot++)
    {
        if (slot->owner_object_index == actor->object_index)
        {
            func_80083BC0(actor, slot, force);
        }
    }
    func_800A3B78(actor->object_index);
}

/**
 * @brief Stop an actor animation and release its object and render state.
 * @param actor Associated field actor, unused by this routine.
 * @param slot Animation actor slot to stop.
 * @param force Nonzero bypasses the normal animation status checks.
 */
void func_80083BC0(FieldActor* actor, FieldActorSlot* slot, s32 force)
{
    s16 command;
    s32 target_index;
    u8 owner_index;
    FieldObjectState* target_state;
    FieldAnimationDef* animation;
    FieldRenderState* render = FIELD_RENDER_STATE;

    if (force == 0)
    {
        if (slot->status.bytes[1] != 0)
        {
            return;
        }
        if (!(slot->status.word & 1))
        {
            if (slot->status.half[1] == 0x21)
            {
                return;
            }
        }
    }

    slot->track_mask = 0;
    slot->pending_track_mask = 0;
    field_clear_actor_effects(slot);
    if (slot->active == 0)
    {
        return;
    }
    if (slot->animation->flags & 0x1000)
    {
        D_800F2280 = 0;
        D_800F227C = 0;
        D_800F2278 = 0;
    }
    if (slot->animation->unk18 & 2)
    {
        owner_index = slot->owner_object_index;
        command = g_field_actors[owner_index].command;
        if ((command != 0x90 && command != 0x94) || (g_field_object_states[owner_index].flags & 0x200))
        {
            g_field_actors[slot->owner_object_index].presence = 0;
        }
        g_field_object_states[slot->owner_object_index].contact.word &= ~1;
    }
    if (slot->animation->unk18 & 4)
    {
        for (target_index = 0; target_index < slot->target_count; target_index++)
        {
            if (slot->targets[target_index] != 0xFF)
            {
                g_field_actors[slot->targets[target_index]].presence = 0;
                target_state = &g_field_object_states[slot->targets[target_index]];
                target_state->contact.word &= ~1;
            }
        }
    }
    animation = slot->animation;
    if (*(u8*)&animation->flags < 0x10U && ((animation->flags >> 8) & 4))
    {
        field_set_global_color_scale(0x100, 0x100, 0x100);
    }
    if (slot->animation->unk1 != 0xFF)
    {
        render->unk140 = 0;
        render->unk92 = 0;
    }
    if (slot->animation->unk0 != 0xFF)
    {
        render->unk13F = 0;
        render->unk91 = 0;
    }
    if (!(slot->animation->flags & 0x800))
    {
        slot->active = 0;
        if (slot->status.word & 1)
        {
            func_80084424(slot->owner_object_index);
        }
    }
    else
    {
        slot->active = 0;
        func_80084424(slot->owner_object_index);
        slot->unk2A = 0;
    }
}

/**
 * @brief Initialize a field actor slot from a resource entry.
 * @param object_index Owner object index stored in the slot.
 * @param slot_index Field actor slot to initialize.
 * @param resource_index Resource entry index; zero disables the slot.
 * @return 1 when the resource entry is enabled and initialized, otherwise 0.
 */
s32 func_80083EEC(s32 object_index, s32 slot_index, s32 resource_index)
{
    u8* header_base;
    u8* resource_base;
    FieldBlobEntry* entry;
    u8* animation_table;
    FieldAnimationDef* animation;
    FieldActorSlot* slot;
    u8 animation_index;
    u8 enabled;

    slot = &g_field_actor_slots[slot_index];
    if (resource_index == 0)
    {
        slot->enabled = 0;
        return 0;
    }

    header_base = D_801058D8;
    entry = &((FieldBlobEntry*)header_base)[resource_index];
    animation_table = header_base + *(s32*)(header_base + 8);
    enabled = entry->enabled;
    slot->enabled = enabled;
    if (enabled == 0)
    {
        return 0;
    }

    slot->status.bytes[1] = 0;
    slot->active = 1;
    resource_base = D_801058D8;
    slot->status.word |= 0x1E;
    slot->resource0 = resource_base + entry->offset0;
    slot->resource4 = resource_base + entry->offset2;
    slot->resource8 = resource_base + entry->offset4;
    slot->status.word &= ~1;
    slot->status.half[1] = resource_index;

    animation_index = (animation_table + resource_index)[-2];
    animation = (FieldAnimationDef*)(animation_table + animation_index * 0x1C + 0xFE);
    slot->animation = animation;
    if (animation->flags & 0x8000)
    {
        slot->unk222 = animation->unk12;
    }
    else
    {
        slot->unk222 = entry->fallback_value;
    }

    slot->timer = 0;
    slot->animation->flags &= 0xF7FF;
    slot->unk29 = 0;
    slot->unk2A = 0;
    slot->owner_object_index = object_index;
    slot->actor_type = g_field_object_states[object_index].action;
    return 1;
}

/**
 * @brief Reserve an unused actor slot and start loading its resource.
 * @param owner Owner object index; indices above two use binding two.
 * @param resource_id Resource identifier to load.
 * @return One on success, or zero if unavailable or loading fails.
 */
s32 func_8008404C(s32 owner, s32 resource_id)
{
    s32 load_id;
    s32 free_slot;
    FieldActorBinding* bindings;
    FieldActorBinding* binding;
    FieldActorSlot* slots;
    FieldActorSlot* slot;

    if (func_800B0850() != 0)
    {
        return 0;
    }
    bindings = g_field_actor_bindings;
    if (bindings[FIELD_BINDING_INDEX(owner)].state != 0)
    {
        return 0;
    }
    free_slot = func_800839F8(owner, 1);
    if (free_slot == -1)
    {
        return 0;
    }
    binding = &g_field_actor_bindings[FIELD_BINDING_INDEX(owner)];
    load_id = resource_id + 0x2DC;
    binding->unk4 = load_id;
    if (func_8009A364(load_id) != 0)
    {
        return 0;
    }
    slots = g_field_actor_slots;
    slot = &slots[free_slot];
    slot->active = 1;
    slot->status.bytes[1] = 0;
    slot->target_count = 0;
    slot->track_mask = 0;
    slot->pending_track_mask = 0;
    slot->status.word |= 0x1E;
    slot->actor_type = g_field_object_states[owner].action;
    binding->slot = free_slot;
    binding->state = 1;
    binding->unk8 = resource_id;
    binding->owner = owner;
    return 1;
}

/**
 * @brief Initialize FIELD resource-load state and allocate the streaming buffers.
 */
void func_80084240(void)
{
    func_8008B724();
    g_field_actor_bindings[2].state = 0;
    g_field_actor_bindings[1].state = 0;
    g_field_actor_bindings[0].state = 0;
    g_field_actor_bindings[2].unk4 = 0;
    g_field_actor_bindings[1].unk4 = 0;
    g_field_actor_bindings[0].unk4 = 0;
    func_8009A384();
    func_8009CA08((u32*)D_8010D034, 0x20000);
    g_field_mesh_transformed_normals = func_8009CA54(D_8010D034, 0x1800, 4);
    g_field_mesh_screen_vertices = func_8009CA54(D_8010D034, 0xC00, 4);
    g_field_mesh_depth_offsets = func_8009CA54(D_8010D034, 0xC00, 4);
}

/**
 * @brief Retire completed resource-load slots and finish processing when all slots are idle.
 */
void func_800842E0(void)
{
    s32 i;
    s32 idle_count;
    FieldActorBinding* binding;
    FieldActorSlot* slot;

    binding = g_field_actor_bindings;
    i = 0;
    idle_count = 0;
    for (; i < FIELD_ACTOR_BINDING_COUNT; i++, binding++)
    {
        if (binding->state == 1 && binding->unk4 == func_8009A390())
        {
            if (cdrom_can_queue_resource((u16)binding->unk4) != 0)
            {
                binding->unk4 = 0;
                slot = &g_field_actor_slots[binding->slot];
                func_8009A4CC(binding->owner, (struct FieldActorState*)slot);
                if (slot->enabled != 0)
                {
                    slot->owner_object_index = binding->owner;
                    slot->status.word |= 1;
                    binding->state = 2;
                }
                else
                {
                    slot->active = 0;
                    func_80084424(g_field_actor_bindings[0].owner);
                }
                break;
            }
        }
        else
        {
            idle_count++;
        }
    }
    if (idle_count == 3)
    {
        func_8009A3E8();
    }
}

/**
 * @brief Tear down the actor bound to an owner once its slot has gone idle.
 * @param owner Owner object index (clamped to 2 when >= 3 for the binding lookup).
 * @note Only acts when the binding's owner matches and the bound actor slot
 *       is free; then it releases the binding, clears D_8010CFD4 and
 *       notifies func_8009A4A0.
 */
void func_80084424(s32 owner)
{
    FieldActorBinding* bindings;
    FieldActorSlot* slots;

    bindings = g_field_actor_bindings;
    if (bindings[FIELD_BINDING_INDEX(owner)].owner == owner)
    {
        slots = g_field_actor_slots;
        if (slots[bindings[FIELD_BINDING_INDEX(owner)].slot].active == 0)
        {
            func_800A3B78(owner);
            bindings[FIELD_BINDING_INDEX(owner)].state = 0;
            D_8010CFD4 = 0;
            func_8009A4A0(FIELD_BINDING_INDEX(owner));
        }
    }
}

/**
 * @brief Reset the field object animation state and reload the shared VRAM
 *        resource block.
 * @note Each object's animation cursor, timers and flag bits are cleared; the
 *       value in unk8 is reseeded from unk4.
 */
void func_80084524(void)
{
    s16 rect[4];
    s32 i;

    for (i = 0; i < FIELD_ACTOR_COUNT; i++)
    {
        g_field_object_states[i].unk4A = 0;
        g_field_object_states[i].unk48 = 0;
        g_field_object_states[i].key = i;
        g_field_object_states[i].unk18 = 0;
        g_field_object_states[i].flags = 0;
        g_field_object_states[i].unk18E = 0;
        g_field_object_states[i].unk8.bits.value = g_field_object_states[i].unk4.bits.value;
        g_field_object_states[i].unk8.bits.unk24 = 0;
        g_field_object_states[i].unk8.bits.flag31 = 0;
        g_field_object_states[i].unk4C |= 1;
        g_field_object_states[i].movement.bits.scale = 0x32;
        g_field_object_states[i].movement.half.hi = 0;
        g_field_object_states[i].contact.bits.flag0 = 0;
        ((FieldObjectIndexView*)g_field_object_states)[i].object_index = i;
        g_field_object_states[i].contact.bits.flag5 = 0;
        g_field_object_states[i].contact.bits.flag6 = 0;
    }

    rect[0] = 0x3C0;
    rect[1] = 0x100;
    rect[2] = 0x100;
    rect[3] = 0x1E0;
    field_load_vram_resource(0x5DA, rect, 0);
    DrawSync(0);
}

/**
 * @brief Reset the field object tints and flags and the party hit states to
 *        their power-on defaults.
 */
void func_80084630(void)
{
    s32 i;
    u8 tint_blue;
    u8 hit_state;

    for (i = 0; i < FIELD_ACTOR_COUNT; i++)
    {
        g_field_object_states[i].tint_red = g_field_object_parts[i].tint_red;
        g_field_object_states[i].tint_green = g_field_object_parts[i].tint_green;
        tint_blue = g_field_object_parts[i].tint_blue;
        g_field_object_states[i].tint_timer = 0;
        g_field_object_states[i].flags = 0;
        g_field_object_states[i].unk17C = 0;
        g_field_object_states[i].unk3C = 0;
        g_field_object_states[i].unk64 = 0;
        g_field_object_states[i].action = 0;
        g_field_object_states[i].unk18E = 0;
        g_field_object_states[i].movement.bits.flag15 = 0;
        g_field_object_states[i].contact.bits.flag7 = 0;
        g_field_object_states[i].contact.bits.flag0 = 0;
        g_field_object_states[i].tint_blue = tint_blue;
        g_field_object_states[i].contact.bits.flag5 = 0;
        g_field_object_states[i].contact.bits.flag6 = 0;
    }
    hit_state = 0xFF;
    for (i = 2; i >= 0; i--)
    {
        g_field_player_records[i].hit_state = hit_state;
    }
    g_field_boss_hud_shake_frame = 0xFF;
}
