#include "common.h"

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

/**
 * @brief Initialize a field actor slot from a resource entry.
 * @param object_index Actor metadata index stored in the slot.
 * @param slot_index Field actor slot to initialize.
 * @param resource_index Resource entry index; zero disables the slot.
 * @return 1 when the resource entry is enabled and initialized, otherwise 0.
 */
s32 func_80083EEC(s32 object_index, s32 slot_index, s32 resource_index)
{
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
