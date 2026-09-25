#include "cdrom.h"
#include "common.h"
#include "controller_internal.h"
#include "field_calls.h"
#include "field_actor_palette.h"
#include "field_actor_tables.h"
#include "field_mesh.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "tim.h"

/**
 * @file field_actor_slot_resources.c
 * @brief Field animation actor slots: slot allocation, built-in and streamed
 *        animation start, teardown and the per-object state resets.
 */

/** @brief First animation number with an entry in the built-in animation table. */
#define FIELD_BUILTIN_FIRST_ANIMATION 2
/** @brief Built-in animation (a special attack) that keeps playing unless the stop is forced. */
#define FIELD_PERSISTENT_ANIMATION 0x21

/** @brief Size of the actor heap set up by field_reset_actor_resources. */
#define FIELD_ACTOR_HEAP_SIZE 0x20000
/** @brief Heap owner tag of the shared mesh work buffers. */
#define FIELD_MESH_HEAP_TAG 4

/** @brief Image resource reloaded by field_reset_object_states, and where it goes in VRAM. */
#define FIELD_OBJECT_IMAGE_RESOURCE 0x5DA
#define FIELD_OBJECT_IMAGE_VRAM_X 0x3C0
#define FIELD_OBJECT_IMAGE_VRAM_Y 0x100
#define FIELD_OBJECT_IMAGE_CLUT_X 0x100
#define FIELD_OBJECT_IMAGE_CLUT_Y 0x1E0

/** @brief Header of the built-in animation resource; it overlays entry 0. */
typedef struct
{
    u32 unk0;
    /** @brief Offset of the frame data from the start of the resource. */
    u32 track_data_offset;
    /** @brief Offset of the FieldBuiltinAnimationTable. */
    u32 animation_table_offset;
    u32 unkC;
} FieldBuiltinHeader;

/** @brief Built-in animation entry (0x10 bytes), indexed by animation number. */
typedef struct
{
    u16 parts_offset;
    u16 curves_offset;
    u16 segments_offset;
    /** @brief Length in frames, unless the definition carries its own. */
    u16 duration;
    /** @brief Number of parts; 0 marks an unused entry. */
    u8 part_count;
    u8 unk9[7];
} FieldBuiltinEntry;

/** @brief Built-in animation resource: a header, then the entries. */
typedef union
{
    FieldBuiltinHeader header;
    FieldBuiltinEntry entries[1];
} FieldBuiltinResource;

/** @brief Definition index of each built-in animation, then the definitions. */
typedef struct
{
    u8 definition_index[0x100 - FIELD_BUILTIN_FIRST_ANIMATION];
    FieldAnimationDef definitions[1];
} FieldBuiltinAnimationTable;

extern FieldBuiltinResource g_field_resource_blob;
extern u8 g_field_resource_buffer[];
/** @brief Frame data of the built-in animations. */
extern u8* g_field_builtin_track_data;
/** @brief Built-in animation resource in use. */
extern FieldBuiltinResource* g_field_builtin_animations;
extern s32 g_field_camera_offset_x;
extern s32 g_field_camera_offset_y;
extern s32 g_field_camera_offset_z;
extern s32 D_8010CFD4;
extern u8* g_field_actor_heap;
extern s32 g_field_boss_hud_shake_frame;

void field_clear_actor_effects(FieldActorSlot* slot);
s32 field_load_vram_resource(s32 id, RECT* rect, s32 mode);
void field_clear_pending_binding_restarts(void);

/**
 * @brief Point the built-in animation globals at the loaded resource.
 */
void field_bind_builtin_animations(void)
{
    g_field_builtin_animations = &g_field_resource_blob;
    g_field_builtin_track_data = (u8*)&g_field_resource_blob + g_field_resource_blob.header.track_data_offset;
}

/**
 * @brief Upload the common effect texture and its CLUT rows from the loaded TIM.
 */
void field_upload_common_texture(void)
{
    RECT rect;
    TimPrefix* tim;
    u8* clut;
    u8* pixels;
    u32 clut_size;
    s32 width;
    s32 height;

    tim = (TimPrefix*)g_field_resource_buffer;
    clut = (u8*)tim->clut_data;
    rect.x = 0;
    rect.y = FIELD_EFFECT_CLUT_VRAM_Y;
    rect.w = CLUT_ENTRY_COUNT;
    rect.h = FIELD_EFFECT_CLUT_ROWS;
    clut_size = tim->clut_block.bnum;
    LoadImage(&rect, (u_long*)clut);
    pixels = clut + clut_size;
    width = ((TimBlock*)pixels - 1)->dimensions.width;
    height = ((TimBlock*)pixels - 1)->dimensions.height;
    rect.w = width;
    rect.h = height;
    rect.x = FIELD_EFFECT_TEXTURE_VRAM_X;
    rect.y = FIELD_EFFECT_TEXTURE_VRAM_Y;
    LoadImage(&rect, (u_long*)pixels);
}

/**
 * @brief Find a free animation actor slot not already claimed by a binding.
 * @param binding_index Owner object index (owners 2 and up share binding 2).
 * @param require_idle When non-zero, fail unless the owner's binding is idle.
 * @return Index of the first free, unclaimed actor slot, or -1 if none.
 */
inline s32 field_find_free_actor_slot(s32 binding_index, s32 require_idle)
{
    s32 i;
    s32 j;
    FieldActorBinding* bindings;

    if (require_idle != 0)
    {
        bindings = g_field_actor_bindings;
        if (bindings[FIELD_BINDING_INDEX(binding_index)].state != FIELD_BINDING_IDLE)
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
                if (g_field_actor_bindings[j].state != FIELD_BINDING_IDLE && g_field_actor_bindings[j].slot == i)
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
 * @brief Stop the animations a field object owns and its reserved sound channels.
 * @param actor Field actor whose object index selects the owned actor slots.
 * @param force Non-zero to stop the animations even while they hide objects.
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
            field_stop_actor_slot(actor, slot, force);
        }
    }
    field_release_sfx_group(actor->object_index);
}

/**
 * @brief Stop an animation slot and undo what the animation changed.
 * @param actor Associated field actor, unused.
 * @param slot Animation actor slot to stop.
 * @param force Non-zero stops the slot even while it hides objects or plays
 *        FIELD_PERSISTENT_ANIMATION.
 */
void field_stop_actor_slot(FieldActor* actor, FieldActorSlot* slot, s32 force)
{
    s16 command;
    s32 target_index;
    u8 owner_index;
    FieldObjectState* target_state;
    FieldAnimationDef* animation;
    ControllerState* controller = CONTROLLER_STATE;

    if (force == 0)
    {
        if (slot->status.parts.hiding_objects != 0)
        {
            return;
        }
        if (!(slot->status.word & FIELD_SLOT_OWNER_LINKED))
        {
            if (slot->status.parts.animation_id == FIELD_PERSISTENT_ANIMATION)
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
    if (slot->animation->flags & FIELD_ANIM_CAMERA_OFFSET)
    {
        g_field_camera_offset_z = 0;
        g_field_camera_offset_y = 0;
        g_field_camera_offset_x = 0;
    }
    if (slot->animation->unk18 & FIELD_ANIM_OWNER_VISIBILITY)
    {
        owner_index = slot->owner_object_index;
        command = g_field_actors[owner_index].command;
        if ((command != FIELD_ACTOR_COMMAND_DEFEATED && command != FIELD_ACTOR_COMMAND_DEFEAT_END) ||
            (g_field_object_states[owner_index].flags & FIELD_OBJECT_FLAG_KNOCKED_OUT))
        {
            g_field_actors[slot->owner_object_index].presence = 0;
        }
        g_field_object_states[slot->owner_object_index].contact.word &= ~FIELD_CONTACT_ANIMATION_HIDDEN;
    }
    if (slot->animation->unk18 & FIELD_ANIM_TARGET_VISIBILITY)
    {
        for (target_index = 0; target_index < slot->target_count; target_index++)
        {
            if (slot->targets[target_index] != FIELD_TARGET_NONE)
            {
                g_field_actors[slot->targets[target_index]].presence = 0;
                target_state = &g_field_object_states[slot->targets[target_index]];
                target_state->contact.word &= ~FIELD_CONTACT_ANIMATION_HIDDEN;
            }
        }
    }
    animation = slot->animation;
    /* The colour curve is the low byte of flags, read as a byte. */
    if (*(u8*)&animation->flags < FIELD_CURVE_COUNT && ((animation->flags >> 8) & (FIELD_ANIM_GLOBAL_COLOR >> 8)))
    {
        field_set_global_color_scale(FIELD_COLOR_SCALE_NEUTRAL, FIELD_COLOR_SCALE_NEUTRAL, FIELD_COLOR_SCALE_NEUTRAL);
    }
    if (slot->animation->vibration_curves[1] != FIELD_CURVE_NONE)
    {
        controller->ports[1].actuator_control.fields.large_motor_command = 0;
        controller->ports[0].actuator_control.fields.large_motor_command = 0;
    }
    if (slot->animation->vibration_curves[0] != FIELD_CURVE_NONE)
    {
        controller->ports[1].small_motor_command = 0;
        controller->ports[0].small_motor_command = 0;
    }
    if (!(slot->animation->flags & FIELD_ANIM_KEEP_ALIVE))
    {
        slot->active = 0;
        if (slot->status.word & FIELD_SLOT_OWNER_LINKED)
        {
            field_release_actor_binding(slot->owner_object_index);
        }
    }
    else
    {
        slot->active = 0;
        field_release_actor_binding(slot->owner_object_index);
        slot->unk2A = 0;
    }
}

/**
 * @brief Start a built-in animation in an actor slot.
 * @param object_index Owner object index stored in the slot.
 * @param slot_index Actor slot to start.
 * @param animation_id Built-in animation number; 0 clears the slot.
 * @return 1 when the animation exists and was started, otherwise 0.
 */
s32 field_start_builtin_animation(s32 object_index, s32 slot_index, s32 animation_id)
{
    FieldBuiltinResource* header_base;
    FieldBuiltinResource* resource_base;
    FieldBuiltinEntry* entry;
    FieldBuiltinAnimationTable* table;
    FieldAnimationDef* animation;
    FieldActorSlot* slot;
    u8 definition_index;
    u8 part_count;

    slot = &g_field_actor_slots[slot_index];
    if (animation_id == 0)
    {
        slot->part_count = 0;
        return 0;
    }

    header_base = g_field_builtin_animations;
    entry = &header_base->entries[animation_id];
    table = (FieldBuiltinAnimationTable*)((u8*)header_base + header_base->header.animation_table_offset);
    part_count = entry->part_count;
    slot->part_count = part_count;
    if (part_count == 0)
    {
        return 0;
    }

    slot->status.parts.hiding_objects = 0;
    slot->active = 1;
    resource_base = g_field_builtin_animations;
    slot->status.word |= FIELD_SLOT_ELEMENT_BITS;
    slot->parts = (FieldObjectPart*)((u8*)resource_base + entry->parts_offset);
    slot->curves = (FieldParameterCurve*)((u8*)resource_base + entry->curves_offset);
    slot->curve_segments = (u16*)((u8*)resource_base + entry->segments_offset);
    slot->status.word &= ~FIELD_SLOT_OWNER_LINKED;
    slot->status.parts.animation_id = animation_id;

    definition_index = table->definition_index[animation_id - FIELD_BUILTIN_FIRST_ANIMATION];
    /* The definitions follow the index array; the original adds that offset last. */
    animation = (FieldAnimationDef*)((u8*)table + definition_index * sizeof(FieldAnimationDef) + sizeof(table->definition_index));
    slot->animation = animation;
    if (animation->flags & FIELD_ANIM_OWN_DURATION)
    {
        slot->duration = animation->duration;
    }
    else
    {
        slot->duration = entry->duration;
    }

    slot->track_interval = 0;
    slot->animation->flags &= ~FIELD_ANIM_KEEP_ALIVE;
    slot->animation_index = 0;
    slot->unk2A = 0;
    slot->owner_object_index = object_index;
    slot->actor_type = g_field_object_states[object_index].action;
    return 1;
}

/**
 * @brief Reserve a free actor slot for an owner and queue its streamed animation.
 * @param owner Owner object index; owners 2 and up share binding 2.
 * @param resource_id Streamed animation number.
 * @return 1 when the read was queued, 0 when busy or out of slots.
 */
s32 field_start_streamed_animation(s32 owner, s32 resource_id)
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
    if (bindings[FIELD_BINDING_INDEX(owner)].state != FIELD_BINDING_IDLE)
    {
        return 0;
    }
    free_slot = field_find_free_actor_slot(owner, 1);
    if (free_slot == -1)
    {
        return 0;
    }
    binding = &g_field_actor_bindings[FIELD_BINDING_INDEX(owner)];
    load_id = resource_id + FIELD_ANIMATION_RESOURCE_BASE;
    binding->load_id = load_id;
    if (field_request_resource_read(load_id) != 0)
    {
        return 0;
    }
    slots = g_field_actor_slots;
    slot = &slots[free_slot];
    slot->active = 1;
    slot->status.parts.hiding_objects = 0;
    slot->target_count = 0;
    slot->track_mask = 0;
    slot->pending_track_mask = 0;
    slot->status.word |= FIELD_SLOT_ELEMENT_BITS;
    slot->actor_type = g_field_object_states[owner].action;
    binding->slot = free_slot;
    binding->state = FIELD_BINDING_LOADING;
    binding->resource_id = resource_id;
    binding->owner = owner;
    return 1;
}

/**
 * @brief Reset the animation bindings and the resource queue, and set up the
 *        actor heap with the shared mesh work buffers.
 */
void field_reset_actor_resources(void)
{
    field_clear_pending_binding_restarts();
    g_field_actor_bindings[2].state = FIELD_BINDING_IDLE;
    g_field_actor_bindings[1].state = FIELD_BINDING_IDLE;
    g_field_actor_bindings[0].state = FIELD_BINDING_IDLE;
    g_field_actor_bindings[2].load_id = 0;
    g_field_actor_bindings[1].load_id = 0;
    g_field_actor_bindings[0].load_id = 0;
    field_clear_resource_queue();
    field_block_pool_init(g_field_actor_heap, FIELD_ACTOR_HEAP_SIZE);
    g_field_mesh_transformed_normals = field_block_alloc(g_field_actor_heap, FIELD_MESH_VERTEX_MAX * sizeof(SVECTOR), FIELD_MESH_HEAP_TAG);
    g_field_mesh_screen_vertices = field_block_alloc(g_field_actor_heap, FIELD_MESH_VERTEX_MAX * sizeof(s32), FIELD_MESH_HEAP_TAG);
    g_field_mesh_depth_offsets = field_block_alloc(g_field_actor_heap, FIELD_MESH_VERTEX_MAX * sizeof(s32), FIELD_MESH_HEAP_TAG);
}

/**
 * @brief Finish a streamed animation whose read has completed, or issue the
 *        next queued read when no binding is loading.
 */
void field_poll_streamed_animations(void)
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
        if (binding->state == FIELD_BINDING_LOADING && binding->load_id == field_get_loading_resource())
        {
            if (cdrom_can_queue_resource((u16)binding->load_id) != 0)
            {
                binding->load_id = 0;
                slot = &g_field_actor_slots[binding->slot];
                field_unpack_actor_resource(binding->owner, (struct FieldActorState*)slot);
                if (slot->part_count != 0)
                {
                    slot->owner_object_index = binding->owner;
                    slot->status.word |= FIELD_SLOT_OWNER_LINKED;
                    binding->state = FIELD_BINDING_READY;
                }
                else
                {
                    slot->active = 0;
                    field_release_actor_binding(g_field_actor_bindings[0].owner);
                }
                break;
            }
        }
        else
        {
            idle_count++;
        }
    }
    if (idle_count == FIELD_ACTOR_BINDING_COUNT)
    {
        field_issue_next_resource_read();
    }
}

/**
 * @brief Release an owner's animation binding once its slot has stopped.
 * @param owner Owner object index; owners 2 and up share binding 2.
 */
void field_release_actor_binding(s32 owner)
{
    FieldActorBinding* bindings;
    FieldActorSlot* slots;

    bindings = g_field_actor_bindings;
    if (bindings[FIELD_BINDING_INDEX(owner)].owner == owner)
    {
        slots = g_field_actor_slots;
        if (slots[bindings[FIELD_BINDING_INDEX(owner)].slot].active == 0)
        {
            field_release_sfx_group(owner);
            bindings[FIELD_BINDING_INDEX(owner)].state = FIELD_BINDING_IDLE;
            D_8010CFD4 = 0;
            field_free_owner_resources(FIELD_BINDING_INDEX(owner));
        }
    }
}

/**
 * @brief Reset every object's state to its scene-start values and reload the
 *        object image into VRAM.
 */
void field_reset_object_states(void)
{
    RECT rect;
    s32 i;

    for (i = 0; i < FIELD_ACTOR_COUNT; i++)
    {
        g_field_object_states[i].action_charge = 0;
        g_field_object_states[i].technique_gauge = 0;
        g_field_object_states[i].key = i;
        g_field_object_states[i].unk18 = 0;
        g_field_object_states[i].flags = 0;
        g_field_object_states[i].unk18E = 0;
        g_field_object_states[i].unk8.bits.value = g_field_object_states[i].unk4.bits.value;
        g_field_object_states[i].unk8.bits.unk24 = 0;
        g_field_object_states[i].unk8.bits.flag31 = 0;
        g_field_object_states[i].hud.word |= 1;
        g_field_object_states[i].movement.bits.scale = 50;
        g_field_object_states[i].movement.half.hi = 0;
        g_field_object_states[i].contact.bits.flag0 = 0;
        g_field_object_states[i].hud.bytes.object_index = i;
        g_field_object_states[i].contact.bits.flag5 = 0;
        g_field_object_states[i].contact.bits.flag6 = 0;
    }

    rect.x = FIELD_OBJECT_IMAGE_VRAM_X;
    rect.y = FIELD_OBJECT_IMAGE_VRAM_Y;
    rect.w = FIELD_OBJECT_IMAGE_CLUT_X;
    rect.h = FIELD_OBJECT_IMAGE_CLUT_Y;
    field_load_vram_resource(FIELD_OBJECT_IMAGE_RESOURCE, &rect, 0);
    DrawSync(0);
}

/**
 * @brief Reset the object tints and flags and the party HUD shake to their
 *        scene-start values.
 */
void field_reset_object_tints(void)
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
        g_field_object_states[i].previous_flags = 0;
        g_field_object_states[i].action_parameter = 0;
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
    hit_state = FIELD_HUD_SHAKE_IDLE;
    for (i = FIELD_PARTY_COUNT - 1; i >= 0; i--)
    {
        g_field_player_records[i].hit_state = hit_state;
    }
    g_field_boss_hud_shake_frame = FIELD_HUD_SHAKE_IDLE;
}
