#include "common.h"
#include "field_mesh.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

/*
 * Consolidated FIELD actor-slot resource TU (vram 0x80083948 .. 0x80084700).
 *
 * Merged from the following per-function sources (ascending by address):
 *   field18.c, field_upload_initial_vram_resource.c, field_find_free_actor_slot.c,
 *   field_actor_slot_queries.c, field_stop_actor_animations_for_object.c,
 *   func_80083BC0.c, func_80083EEC.c, func_8008404C.c, func_80084240.c,
 *   func_800842E0.c, field22.c, func_80084630.c
 *
 * Several extern records (g_field_actor_slots, D_80105880, D_80105AE0, D_801058D8)
 * are viewed through incompatible struct/pointer types by different functions, so
 * those externs (and the local record typedefs they use) are declared at BLOCK
 * scope inside each using function with that function's ORIGINAL type. GCC 2.7.2
 * accepts these conflicting block-scope declarations (warnings only) and emits
 * identical code. Only the func_80083BC0 record types and the two forward member
 * prototypes live at file scope.
 */

/** @brief Animation channel selectors and control flags used during teardown. */
typedef struct
{
    u8 unk0, unk1;
    u8 pad2[10];
    u16 unkC;
    u8 padE[10];
    u16 unk18;
} Animation;
/** @brief Teardown fields in a 0x244-byte FIELD animation actor slot. */
typedef struct
{
    u8 pad0[12];
    Animation *unkC;
    u8 pad10[0x24 - 0x10];
    u8 unk24;
    u8 pad25[5];
    u8 unk2A;
    u8 pad2B[0x224 - 0x2B];
    union
    {
        u32 word;
        u16 halves[2];
        u8 bytes[4];
    } status;
    u8 unk228;
    u8 unk229[9];
    u8 unk232;
    u8 pad233[7];
    u8 unk23A, unk23B;
    u8 pad23C[8];
} ActorState;
/** @brief FIELD object record with its animation marker and state ID. */
typedef struct
{
    u8 pad0[0x25];
    u8 unk25;
    u8 pad26[4];
    s16 unk2A;
    u8 pad2C[0x54 - 0x2C];
} Object;
/** @brief Object flags used to release ownership of animation state. */
typedef struct
{
    u8 pad0[12];
    s32 unkC;
    u8 pad10[0x178 - 0x10];
    s32 unk178;
    u8 pad17C[0x23C - 0x17C];
} ObjectState;
/**
 * @brief FIELD actor record whose object index selects the owned actor slots.
 * @note At file scope because it is a parameter type of
 *       field_stop_actor_animations_for_object.
 */
typedef struct
{
    u8 pad0[0x3A];
    u8 object_index;
} FieldActorRecord;

/* Forward prototypes for member functions defined later in this TU. */
void func_80083BC0(void *record, ActorState *actor, s32 force);
void func_80084424(s32 arg0);

/**
 * @brief Reset the field resource cursor pair to the base blob and its end.
 * @note D_801058D8 points at the blob base; D_801058D4 points one past the
 *       payload, using the length word stored at g_field_resource_blob[1].
 */
void func_80083948(void)
{
    extern u32 g_field_resource_blob[];
    extern u8 *D_801058D4;
    extern u32 *D_801058D8;

    D_801058D8 = g_field_resource_blob;
    D_801058D4 = (u8 *)g_field_resource_blob + g_field_resource_blob[1];
}

/**
 * @brief Upload the initial FIELD VRAM resource (palette strip and image) from
 *        the loaded resource buffer.
 */
void func_8008396C(void)
{
    extern u8 g_field_resource_buffer[];
    RECT rect;
    u8 *buf;
    u8 *base;
    u8 *data;
    u32 off;
    s32 w;
    s32 h;

    buf = g_field_resource_buffer;
    base = buf + 0x14;
    rect.x = 0;
    rect.y = 0x1EA;
    rect.w = 0x100;
    rect.h = 4;
    off = *(u32 *)(buf + 8);
    LoadImage(&rect, (u_long *)base);
    data = base + off;
    w = *(u16 *)(data - 4);
    h = *(u16 *)(data - 2);
    rect.w = w;
    rect.h = h;
    rect.x = 0x180;
    rect.y = 0;
    LoadImage(&rect, (u_long *)data);
}

/**
 * @brief Find a free field actor slot not already claimed by one of the three
 *        active tracks.
 * @param arg0 Track index (clamped to 2 when >= 3).
 * @param arg1 When non-zero, require the (clamped) track's D_80105880 entry to
 *             be idle; otherwise fail early.
 * @return Index of the first free, unclaimed actor slot, or -1 if none.
 */
s32 func_800839F8(s32 arg0, s32 arg1)
{
    typedef struct
    {
        u32 unk0;
        u8 pad4[0x18 - 4];
        s32 unk18;
    } Struct_D80105880;

    typedef struct
    {
        u8 pad0[0x24];
        u8 unk24;
        u8 pad25[0x244 - 0x25];
    } FieldActorState;

    extern Struct_D80105880 D_80105880[];
    extern FieldActorState g_field_actor_slots[];

    s32 i;
    s32 j;
    Struct_D80105880 *entry;

    if (arg1 != 0)
    {
        entry = D_80105880;
        if (arg0 >= 3)
        {
            arg0 = 2;
        }
        if (entry[arg0].unk0 != 0)
        {
            return -1;
        }
    }

    for (i = 0; i < 0x30; i++)
    {
        if (g_field_actor_slots[i].unk24 == 0)
        {
            for (j = 0; j < 3; j++)
            {
                if (D_80105880[j].unk0 != 0 && D_80105880[j].unk18 == i)
                {
                    break;
                }
            }
            if (j == 3)
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
    typedef struct
    {
        u8 pad0[0x24];
        u8 is_active;
        u8 pad25[0x228 - 0x25];
        u8 owner_object_index;
        u8 pad229[0x23A - 0x229];
        u8 active_track_mask;
        u8 pad23B[0x244 - 0x23B];
    } FieldActorState;

    extern FieldActorState g_field_actor_slots[];

    s32 slot_index;

    slot_index = 0;
    do
    {
        if ((g_field_actor_slots[slot_index].owner_object_index == object_index) && (g_field_actor_slots[slot_index].active_track_mask != 0))
        {
            return 1;
        }
        slot_index++;
    } while (slot_index < 0x30);
    return 0;
}

/**
 * @brief Count free actor slots in the first 48-slot actor pool.
 * @return Number of actor slots whose active flag is clear.
 */
s32 field_count_free_actor_slots(void)
{
    typedef struct
    {
        u8 pad0[0x24];
        u8 is_active;
        u8 pad25[0x228 - 0x25];
        u8 owner_object_index;
        u8 pad229[0x23A - 0x229];
        u8 active_track_mask;
        u8 pad23B[0x244 - 0x23B];
    } FieldActorState;

    extern FieldActorState g_field_actor_slots[];

    s32 free_count;
    s32 slot_index;

    free_count = 0;
    for (slot_index = 0; slot_index < 0x30; slot_index++)
    {
        if (g_field_actor_slots[slot_index].is_active == 0)
        {
            free_count++;
        }
    }

    return free_count;
}

/**
 * @brief Stop actor animations and reserved SFX channels for a field object.
 * @param record Field actor record whose object index selects the owned actor slots.
 * @param force Non-zero to force teardown of matching actor animations.
 */
void field_stop_actor_animations_for_object(FieldActorRecord *record, s32 force)
{
    typedef struct
    {
        u8 pad0[0x228];
        u8 owner_object_index;
        u8 pad229[0x244 - 0x229];
    } FieldActorState;

    extern FieldActorState g_field_actor_slots[];
    void func_800A3B78(s32 object_index);

    FieldActorState *actor;
    s32 actor_index;

    actor = g_field_actor_slots;
    for (actor_index = 0; actor_index < 48; actor_index++, actor++)
    {
        if (actor->owner_object_index == record->object_index)
        {
            func_80083BC0(record, actor, force);
        }
    }
    func_800A3B78(record->object_index);
}

/**
 * @brief Stop an actor animation and release its object and render state.
 * @param record Associated FIELD object record, unused by this routine.
 * @param actor Animation actor slot to stop.
 * @param force Nonzero bypasses the normal animation status checks.
 */
void func_80083BC0(void *record, ActorState *actor, s32 force)
{
    extern Object D_800FDF58[];
    extern ObjectState D_80105AE0[];
    extern s32 D_800F2278, D_800F227C, D_800F2280;
    void field_set_global_color_scale(s32, s32, s32);
    void field_clear_actor_effects(ActorState *);

    s16 object_state;
    s32 target_index;
    u8 owner_index;
    u8 target_slot;
    ObjectState *owner_state;
    ObjectState *target_state;
    Animation *animation;
    u8 *target_ptr;
    u8 *render = (u8 *)0x801ED600;

    if (force == 0)
    {
        if (actor->status.bytes[1] == 0)
        {
            if (!(actor->status.word & 1))
            {
                if (actor->status.halves[1] != 0x21)
                {
                    goto stop_animation;
                }
            }
            else
            {
                goto clear_tracks;
            }
        }
    }
    else
    {
    stop_animation:
    clear_tracks:
        actor->unk23A = 0;
        actor->unk23B = 0;
        field_clear_actor_effects(actor);
        if (actor->unk24 != 0)
        {
            if (actor->unkC->unkC & 0x1000)
            {
                D_800F2280 = 0;
                D_800F227C = 0;
                D_800F2278 = 0;
            }
            if (actor->unkC->unk18 & 2)
            {
                owner_index = actor->unk228;
                object_state = D_800FDF58[owner_index].unk2A;
                if (((object_state != 0x90) && (object_state != 0x94)) ||
                    (D_80105AE0[owner_index].unkC & 0x200))
                {
                    D_800FDF58[actor->unk228].unk25 = 0;
                }
                D_80105AE0[actor->unk228].unk178 &= ~1;
            }
            if (actor->unkC->unk18 & 4)
            {
                for (target_index = 0; target_index < actor->unk232; target_index++)
                {
                    /* Keep this initial read and the subsequent direct array accesses. */
                    target_slot = actor->unk229[target_index];
                    if (actor->unk229[target_index] != 0xFF)
                    {
                        D_800FDF58[actor->unk229[target_index]].unk25 = 0;
                        target_state = &D_80105AE0[actor->unk229[target_index]];
                        target_state->unk178 = (s32)(target_state->unk178 & ~1);
                    }
                }
            }
            animation = actor->unkC;
            if ((*(u8 *)&animation->unkC < 0x10U) && (((u16)animation->unkC >> 8) & 4))
            {
                field_set_global_color_scale(0x100, 0x100, 0x100);
            }
            if (actor->unkC->unk1 != 0xFF)
            {
                render[0x140] = 0;
                render[0x92] = 0;
            }
            if (actor->unkC->unk0 != 0xFF)
            {
                render[0x13F] = 0;
                render[0x91] = 0;
            }
            if (!(actor->unkC->unkC & 0x800))
            {
                actor->unk24 = 0U;
                if (actor->status.word & 1)
                {
                    func_80084424(actor->unk228);
                }
            }
            else
            {
                actor->unk24 = 0U;
                func_80084424(actor->unk228);
                actor->unk2A = 0;
            }
        }
    }
}

/**
 * @brief Initialize a field actor slot from a resource entry.
 * @param object_index Actor metadata index stored in the slot.
 * @param slot_index Field actor slot to initialize.
 * @param resource_index Resource entry index; zero disables the slot.
 * @return 1 when the resource entry is enabled and initialized, otherwise 0.
 */
s32 func_80083EEC(s32 object_index, s32 slot_index, s32 resource_index)
{
    typedef struct
    {
        u16 offset0;
        u16 offset2;
        u16 offset4;
        u16 fallback_value;
        u8 enabled;
        u8 pad9[7];
    } FieldActorResourceEntry;

    typedef struct
    {
        u8 pad0[0xC];
        u16 flags;
        u8 padE[4];
        u16 value12;
        u8 pad14[0x1C - 0x14];
    } FieldActorAnimation;

    typedef struct
    {
        u8 pad0[0x16F];
        u8 actor_type;
        u8 pad170[0x23C - 0x170];
    } FieldActorMetadata;

    typedef union
    {
        u32 value;
        struct
        {
            u8 low;
            u8 transient;
            u16 resource_index;
        } parts;
    } FieldActorFlags;

    typedef struct
    {
        void* resource0;
        void* resource4;
        void* resource8;
        FieldActorAnimation* animation;
        u8 pad10[0x24 - 0x10];
        u8 active;
        u8 enabled;
        u8 actor_type;
        u8 pad27[0x29 - 0x27];
        u8 unk29;
        u8 unk2A;
        u8 pad2B[0x222 - 0x2B];
        u16 value222;
        FieldActorFlags flags;
        u8 owner_object_index;
        u8 pad229[0x238 - 0x229];
        u16 timer238;
        u8 pad23A[0x244 - 0x23A];
    } FieldActorState;

    extern u8* D_801058D8;
    extern FieldActorMetadata D_80105AE0[];
    extern FieldActorState g_field_actor_slots[];

    u8* header_base;
    u8* resource_base;
    FieldActorResourceEntry* entry;
    u8* animation_table;
    u8* animation_index_ptr;
    FieldActorAnimation* animation;
    FieldActorState* actor;
    u8 animation_index;
    u8 enabled;

    actor = &g_field_actor_slots[slot_index];
    if (resource_index == 0)
    {
        actor->enabled = 0;
        return 0;
    }

    header_base = D_801058D8;
    entry = (FieldActorResourceEntry*)(header_base + resource_index * 0x10);
    animation_table = header_base + *(s32*)(header_base + 8);
    enabled = entry->enabled;
    actor->enabled = enabled;
    if (enabled == 0)
    {
        return 0;
    }

    actor->flags.parts.transient = 0;
    actor->active = 1;
    resource_base = D_801058D8;
    actor->flags.value |= 0x1E;
    actor->resource0 = resource_base + entry->offset0;
    actor->resource4 = resource_base + entry->offset2;
    actor->resource8 = resource_base + entry->offset4;
    actor->flags.value &= ~1;
    actor->flags.parts.resource_index = resource_index;

    animation_index_ptr = animation_table;
    animation_index_ptr += resource_index;
    animation_index = animation_index_ptr[-2];
    animation = (FieldActorAnimation*)(animation_table + animation_index * 0x1C + 0xFE);
    actor->animation = animation;
    if (animation->flags & 0x8000)
    {
        actor->value222 = animation->value12;
    }
    else
    {
        actor->value222 = entry->fallback_value;
    }

    actor->timer238 = 0;
    actor->animation->flags &= 0xF7FF;
    actor->unk29 = 0;
    actor->unk2A = 0;
    actor->owner_object_index = object_index;
    actor->actor_type = D_80105AE0[object_index].actor_type;
    return 1;
}

/**
 * @brief Reserve an unused actor slot and initialize its resource binding.
 * @param actor_id Owner actor identifier; binding indices above two use binding two.
 * @param resource_id Resource identifier to load.
 * @return One on success, or zero if unavailable or loading fails.
 */
s32 func_8008404C(s32 actor_id, s32 resource_id)
{
    typedef struct
    {
        s32 unk0, unk4, unk8, unkC;
        u8 pad10[8];
        s32 unk18;
    } Binding;
    typedef struct
    {
        u8 pad0[0x24];
        u8 unk24;
        u8 pad25;
        u8 unk26;
        u8 pad27[0x224 - 0x27];
        union
        {
            s32 flags;
            u8 bytes[4];
        } state;
        u8 pad228[10];
        u8 unk232;
        u8 pad233[7];
        u8 unk23A, unk23B;
        u8 pad23C[8];
    } Slot;
    typedef struct
    {
        u8 pad0[0x16F];
        u8 unk16F;
        u8 pad170[0x23C - 0x170];
    } Actor;

    extern Binding D_80105880[];
    extern Slot g_field_actor_slots[];
    extern Actor D_80105AE0[];
    extern s32 func_800B0850(void);
    extern s32 func_8009A364(s32);

    Binding *binding_base;
    Binding *search_base;
    Binding *binding_cursor;
    s32 binding_index;
    Slot *slot_cursor;
    s32 load_id;
    s32 free_slot;
    s32 first_index;
    s32 second_index;
    s32 slot_index;
    s32 owner_index;
    Binding *binding;
    Slot *slot;
    Slot *slot_base;

    if (func_800B0850() == 0)
    {
        first_index = actor_id;
        binding_base = D_80105880;

        if (actor_id >= 3)
        {
            first_index = 2;
        }
        if (binding_base[first_index].unk0 == 0)
        {
            second_index = actor_id;
            if (actor_id >= 3)
            {
                second_index = 2;
            }
            free_slot = -1;
            if (binding_base[second_index].unk0 == 0)
            {
                slot_index = 0;
                search_base = binding_base;
                slot_cursor = g_field_actor_slots;
            scan_slot:
                binding_index = 0;
                if (slot_cursor->unk24 == 0)
                {
                    binding_cursor = search_base;
                    for (; binding_index < 3; binding_index++, binding_cursor++)
                    {
                        if ((binding_cursor->unk0 != 0) && (binding_cursor->unk18 == slot_index))
                        {
                            break;
                        }
                    }
                    if (binding_index != 3)
                    {
                        goto next_slot;
                    }
                    free_slot = slot_index;
                }
                else
                {
                next_slot:
                    slot_index += 1;
                    slot_cursor++;
                    if (slot_index >= 0x30)
                    {
                        free_slot = -1;
                    }
                    else
                    {
                        goto scan_slot;
                    }
                }
            }
            if (free_slot != -1)
            {
                owner_index = actor_id;
                if (actor_id >= 3)
                {
                    owner_index = 2;
                }
                binding = &D_80105880[owner_index];
                load_id = resource_id + 0x2DC;
                binding->unk4 = load_id;
                if (func_8009A364(load_id) == 0)
                {
                    slot_base = g_field_actor_slots;
                    slot = &slot_base[free_slot];
                    slot->unk24 = 1;
                    slot->state.bytes[1] = 0;
                    slot->unk232 = 0;
                    slot->unk23A = 0;
                    slot->unk23B = 0;
                    slot->state.flags = (s32)(slot->state.flags | 0x1E);
                    slot->unk26 = (u8)D_80105AE0[actor_id].unk16F;
                    binding->unk18 = free_slot;
                    binding->unk0 = 1;
                    binding->unk8 = resource_id;
                    binding->unkC = actor_id;
                    return 1;
                }
                return 0;
            }
            return 0;
        }
        return 0;
    }
    return 0;
}

/**
 * @brief Initialize FIELD resource-load state and allocate the streaming buffers.
 */
void func_80084240(void)
{
    typedef struct
    {
        s32 unk0;
        s32 unk4;
        u8 pad8[0x14];
        s32 unk1C;
        s32 unk20;
        u8 pad24[0x14];
        s32 unk38;
        s32 unk3C;
    } FieldInitState;

    extern FieldInitState D_80105880;
    extern s32 D_8010D034;

    void func_8008B724(void);
    void func_8009A384(void);
    void func_8009CA08(s32 arg0, s32 arg1);
    s32 func_8009CA54(s32 arg0, s32 arg1, s32 arg2);

    func_8008B724();
    D_80105880.unk38 = 0;
    D_80105880.unk1C = 0;
    D_80105880.unk0 = 0;
    D_80105880.unk3C = 0;
    D_80105880.unk20 = 0;
    D_80105880.unk4 = 0;
    func_8009A384();
    func_8009CA08(D_8010D034, 0x20000);
    g_field_mesh_transformed_normals = func_8009CA54(D_8010D034, 0x1800, 4);
    g_field_mesh_screen_vertices = func_8009CA54(D_8010D034, 0xC00, 4);
    g_field_mesh_depth_offsets = func_8009CA54(D_8010D034, 0xC00, 4);
}

/**
 * @brief Retire completed resource-load slots and finish processing when all slots are idle.
 */
void func_800842E0(void)
{
    typedef struct
    {
        s32 unk0;
        s32 unk4;
        u8 pad8[0xC - 8];
        s32 unkC;
        u8 pad10[0x18 - 0x10];
        s32 unk18;
    } Struct_D80105880;

    typedef struct
    {
        u8 pad0[0x24];
        u8 unk24;
        u8 unk25;
        u8 pad26[0x224 - 0x26];
        s32 unk224;
        u8 unk228;
        u8 pad229[0x244 - 0x229];
    } FieldActorState;

    extern Struct_D80105880 D_80105880[];
    extern FieldActorState g_field_actor_slots[];

    s32 func_8009A390(void);
    void func_8009A3E8(void);
    void func_8009A4CC(s32 arg0, void *arg1);
    s32 cdrom_can_queue_resource(s32 resource_index);

    s32 i;
    s32 idle_count;
    Struct_D80105880 *entry;
    FieldActorState *actor;

    entry = D_80105880;
    i = 0;
    idle_count = 0;
    do
    {
        if (entry->unk0 == 1 && entry->unk4 == func_8009A390())
        {
            if (cdrom_can_queue_resource((u16)entry->unk4) == 0)
            {
                i++;
                goto next;
            }
            i++;
            entry->unk4 = 0;
            actor = (FieldActorState *)((u8 *)g_field_actor_slots + entry->unk18 * 0x244);
            func_8009A4CC(entry->unkC, actor);
            if (actor->unk25 != 0)
            {
                actor->unk228 = (u8)entry->unkC;
                actor->unk224 |= 1;
                entry->unk0 = 2;
            }
            else
            {
                actor->unk24 = 0;
                func_80084424(D_80105880[0].unkC);
            }
            goto done;
        }
        else
        {
            idle_count++;
            i++;
        }
    next:
        entry++;
    } while (i < 3);
done:
    if (idle_count == 3)
    {
        func_8009A3E8();
    }
}

/**
 * @brief Tear down the actor bound to a track once its slot has gone idle.
 * @param arg0 Track index (clamped to 2 when >= 3 for the D_80105880 lookup).
 * @note Only acts when the track's stored value matches arg0 and the bound
 *       actor slot is free; then it releases the actor, clears the track and
 *       D_8010CFD4, and notifies func_8009A4A0.
 */
void func_80084424(s32 arg0)
{
    typedef struct
    {
        s32 unk0;
        u8 pad4[0xC - 4];
        s32 unkC;
        u8 pad10[0x18 - 0x10];
        s32 unk18;
    } Struct_D80105880;

    typedef struct
    {
        u8 pad0[0x24];
        u8 unk24;
        u8 pad25[0x244 - 0x25];
    } FieldActorState;

    extern Struct_D80105880 D_80105880[];
    extern FieldActorState g_field_actor_slots[];
    extern s32 D_8010CFD4;

    void func_800A3B78(s32 arg0);
    void func_8009A4A0(s32 arg0);

    s32 value;
    s32 index;
    s32 call_index;
    s32 small;
    Struct_D80105880 *base;
    FieldActorState *actors;

    value = arg0;
    base = D_80105880;
    small = value < 3;
    index = value;
    if (small == 0)
    {
        index = 2;
    }

    if (base[index].unkC == value)
    {
        index = value;
        actors = g_field_actor_slots;
        if (value >= 3)
        {
            index = 2;
        }

        if (actors[base[index].unk18].unk24 == 0)
        {
            func_800A3B78(value);
            index = value;
            if (value >= 3)
            {
                index = 2;
            }

            call_index = value;
            base[index].unk0 = 0;
            D_8010CFD4 = 0;
            if (value >= 3)
            {
                call_index = 2;
            }
            func_8009A4A0(call_index);
        }
    }
}

/**
 * @brief Reset all 13 field animation slots to their idle defaults and reload
 *        the shared VRAM resource block.
 * @note Each slot's animation cursor, timers and flag bits are cleared; the
 *       run-position field unk8 is reseeded from unk4's low 24 bits.
 */
void func_80084524(void)
{
    typedef union
    {
        s32 w;
        struct
        {
            u32 low24 : 24;
            u32 mid7 : 7;
            u32 top1 : 1;
        } b;
    } Word8;

    typedef union
    {
        s32 w;
        struct
        {
            u8 b0;
            s8 b1;
            u8 b2;
            u8 b3;
        } b;
    } Word4C;

    typedef union
    {
        s32 w;
        struct
        {
            u32 low10 : 10;
            u32 rest6 : 6;
            u32 hi16 : 16;
        } b;
        struct
        {
            s16 lo;
            s16 hi;
        } h;
    } Word174;

    typedef union
    {
        s32 w;
        struct
        {
            u32 b0 : 1;
            u32 p1 : 4;
            u32 b5 : 1;
            u32 b6 : 1;
            u32 rest : 25;
        } b;
    } Word178;

    typedef struct
    {
        u8 pad0[4];
        Word8 unk4;
        Word8 unk8;
        s32 unkC;
        u8 pad10[4];
        s32 unk14;
        s16 unk18;
        u8 pad1A[0x48 - 0x1A];
        s16 unk48;
        s16 unk4A;
        Word4C unk4C;
        u8 pad50[0x174 - 0x50];
        Word174 unk174;
        Word178 unk178;
        u8 pad17C[0x18E - 0x17C];
        u8 unk18E;
        u8 pad18F[0x23C - 0x18F];
    } Slot;

    extern Slot D_80105AE0[];
    void field_load_vram_resource(s32 id, s16 *rect, s32 arg2);

    s16 rect[4];
    s32 i;

    i = 0;
    do
    {
        D_80105AE0[i].unk4A = 0;
        D_80105AE0[i].unk48 = 0;
        D_80105AE0[i].unk14 = i;
        D_80105AE0[i].unk18 = 0;
        D_80105AE0[i].unkC = 0;
        D_80105AE0[i].unk18E = 0;
        D_80105AE0[i].unk8.b.low24 = D_80105AE0[i].unk4.b.low24;
        D_80105AE0[i].unk8.b.mid7 = 0;
        D_80105AE0[i].unk8.b.top1 = 0;
        D_80105AE0[i].unk4C.w |= 1;
        D_80105AE0[i].unk174.b.low10 = 0x32;
        D_80105AE0[i].unk174.h.hi = 0;
        D_80105AE0[i].unk178.b.b0 = 0;
        D_80105AE0[i].unk4C.b.b1 = (s8)i;
        D_80105AE0[i].unk178.b.b5 = 0;
        D_80105AE0[i].unk178.b.b6 = 0;
        i++;
    } while (i < 13);

    rect[0] = 0x3C0;
    rect[1] = 0x100;
    rect[2] = 0x100;
    rect[3] = 0x1E0;
    field_load_vram_resource(0x5DA, rect, 0);
    DrawSync(0);
}

/**
 * @brief Reset the 13 field animation slots and the reserved SFX handles to
 *        their power-on defaults.
 */
void func_80084630(void)
{
    typedef union { s32 w; struct { u32 low15:15; u32 b15:1; u32 hi16:16; } b; } W174;
    typedef union { s32 w; struct { u32 b0:1; u32 p1:4; u32 b5:1; u32 b6:1; u32 b7:1; u32 rest:24; } b; } W178;
    typedef struct {
     u8 pad0[0xC]; s32 unkC; u8 pad10[0x3C - 0x10]; s32 unk3C; u8 pad40[0x64 - 0x40]; s32 unk64;
     u8 pad68[0x16F - 0x68]; u8 unk16F; u8 pad170[0x174 - 0x170]; W174 unk174; W178 unk178; s32 unk17C;
     u8 pad180[0x18E - 0x180]; u8 unk18E; u8 pad18F[0x1A8 - 0x18F]; u8 unk1A8,unk1A9,unk1AA,unk1AB;
     u8 pad1AC[0x23C - 0x1AC];
    } Slot;
    typedef struct {u8 pad0[0xE]; u8 unkE,unkF,unk10; u8 pad11[0x48 - 0x11];} Part;
    typedef struct {u8 pad0[0x259]; u8 unk259; u8 pad25A[0x268 - 0x25A];} FD;
    extern Slot D_80105AE0[]; extern Part D_800FE3A0[]; extern FD D_800FD818[]; extern s32 D_8010A000;

    s32 i; u8 v; u8 ff;
    i=0;
    do {
      D_80105AE0[i].unk1A8 = D_800FE3A0[i].unkE;
      D_80105AE0[i].unk1A9 = D_800FE3A0[i].unkF;
      v = D_800FE3A0[i].unk10;
      D_80105AE0[i].unk1AB = 0;
      D_80105AE0[i].unkC = 0;
      D_80105AE0[i].unk17C = 0;
      D_80105AE0[i].unk3C = 0;
      D_80105AE0[i].unk64 = 0;
      D_80105AE0[i].unk16F = 0;
      D_80105AE0[i].unk18E = 0;
      D_80105AE0[i].unk174.b.b15 = 0;
      D_80105AE0[i].unk178.b.b7 = 0;
      D_80105AE0[i].unk178.b.b0 = 0;
      D_80105AE0[i].unk1AA = v;
      D_80105AE0[i].unk178.b.b5 = 0;
      D_80105AE0[i].unk178.b.b6 = 0;
      i++;
    } while (i<13);
    ff=0xFF;
    i=2;
    do { D_800FD818[i].unk259=ff; i--; } while(i>=0);
    D_8010A000=0xFF;
}
