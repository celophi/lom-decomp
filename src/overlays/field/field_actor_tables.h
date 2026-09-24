#ifndef FIELD_ACTOR_TABLES_H
#define FIELD_ACTOR_TABLES_H

#include "common.h"

/**
 * @file field_actor_tables.h
 * @brief Field actor record, object state, animation slot and binding tables
 *        shared by the actor command interpreters.
 *
 * The field keeps three parallel per-object tables indexed the same way:
 * g_field_actors (script and motion state), g_field_object_states (runtime
 * state; its key word identifies the object to scripts) and
 * g_field_object_parts (model part data). Animation actors live in the
 * separate g_field_actor_slots pool; g_field_actor_bindings ties up to three
 * of them to the party objects.
 */

/** @brief Number of objects in the parallel field object tables. */
#define FIELD_ACTOR_COUNT 13

/** @brief Number of general-purpose animation actor slots. */
#define FIELD_ACTOR_SLOT_COUNT 48

/** @brief Number of animation actor bindings (party members 0, 1 and everyone else). */
#define FIELD_ACTOR_BINDING_COUNT 3

/** @brief Sentinel returned by the actor lookups when no object matches. */
#define FIELD_ACTOR_NONE ((FieldActor*)-1)

/** @brief Script index of an actor without a running script. */
#define FIELD_SCRIPT_NONE 0xFF

/** @brief Script index of an actor running its object state's private script. */
#define FIELD_SCRIPT_OBJECT 0xFE

/** @brief Presence value of an unused actor record. */
#define FIELD_ACTOR_UNUSED 0xFF

/** @brief Presence value of a hidden actor record. */
#define FIELD_ACTOR_HIDDEN 0xFE

/** @brief Map size words of the fixed field map header block at 0x801ED400. */
typedef struct
{
    s16 width;
    u16 depth;
} FieldMapBounds;

/** @brief Fixed field render state block at 0x801ED600 (partial). */
typedef struct
{
    u8 unk0[0x91];
    u8 unk91;
    u8 unk92;
    u8 unk93[0x13E - 0x93];
    u8 leader_object_index;
    u8 unk13F;
    u8 unk140;
} FieldRenderState;

/** @brief Fixed address of the field map header block. */
#define FIELD_MAP_BOUNDS ((FieldMapBounds*)0x801ED400)

/** @brief Fixed address of the field render state block. */
#define FIELD_RENDER_STATE ((FieldRenderState*)0x801ED600)

/** @brief Word, halfword and bit views of an actor's control-mode word. */
typedef union
{
    u32 word;
    u16 half[2];
} FieldActorControl;

/** @brief Script, motion and animation state of one field object (0x54 bytes). */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
    u32 unkC;
    s16 unk10;
    s16 unk12;
    s16 unk14;
    s16 unk16;
    u8 unk18;
    u8 unk19;
    u8 unk1A;
    u8 unk1B;
    /** @brief Low nine bits hold the control mode; higher bits are flags. */
    FieldActorControl control;
    u8 unk20;
    /** @brief Low seven bits select the animation; bit 7 mirrors it. */
    u8 animation;
    u8 unk22;
    u8 unk23;
    u8 unk24;
    u8 presence;
    u8 unk26;
    u8 unk27;
    u8 script_index;
    u8 unk29;
    s16 command;
    u16 script_offset;
    u16 unk2E;
    u16 unk30;
    u8 unk32;
    u8 unk33;
    u8 unk34;
    u8 unk35;
    u8 unk36;
    s8 height;
    u8 unk38;
    u8 unk39;
    u8 object_index;
    u8 resource_index;
    u8 unk3C;
    u8 unk3D;
    u8 unk3E;
    u8 unk3F;
    s32 unk40;
    u32 unk44;
    u32 unk48;
    u32 unk4C;
    u8 unk50[4];
} FieldActor;

/** @brief A 24-bit value word whose top byte holds flags. */
typedef union
{
    s32 word;
    struct
    {
        u32 value : 24;
        u32 unk24 : 7;
        u32 flag31 : 1;
    } bits;
} FieldValueWord;

/** @brief Packed movement word: low ten bits are the effect scale, bit 15 a flag, the top half a height. */
typedef union
{
    u32 word;
    struct
    {
        u32 scale : 10;
        u32 unk10 : 5;
        u32 flag15 : 1;
        u32 unk16 : 16;
    } bits;
    struct
    {
        s16 lo;
        s16 hi;
    } half;
} FieldMovementWord;

/** @brief Packed contact word: flag bits, the bound animation actor and the collected target count. */
typedef union
{
    u32 word;
    struct
    {
        u32 flag0 : 1;
        u32 unk1 : 4;
        u32 flag5 : 1;
        u32 flag6 : 1;
        u32 flag7 : 1;
        u32 unk8 : 24;
    } bits;
    struct
    {
        u8 flags;
        u8 animation_actor_index;
        u8 controller_index;
        u8 target_count;
    } bytes;
} FieldContactWord;

/** @brief Collision footprint word with a halfword view of the extent. */
typedef union
{
    u32 word;
    struct
    {
        s16 center_offset;
        u16 extent;
    } half;
} FieldCollisionWord;

/** @brief Runtime state of one field object (0x23C bytes). */
typedef struct
{
    s32 unk0;
    FieldValueWord unk4;
    FieldValueWord unk8;
    u32 flags;
    s32 group_flags;
    s32 key;
    s16 unk18;
    u8 unk1A[0x3C - 0x1A];
    s32 unk3C;
    s32 unk40;
    s32 unk44;
    u16 unk48;
    s16 unk4A;
    s32 unk4C;
    s32 target_x;
    s32 target_y;
    s32 target_z;
    u8 unk5C[0x64 - 0x5C];
    s32 unk64;
    u8 unk68[0x12C - 0x68];
    FieldCollisionWord collision;
    u8 unk130[0x140 - 0x130];
    s16 unk140;
    s16 unk142;
    s16 unk144;
    s16 unk146;
    u8 unk148[0x168 - 0x148];
    u8* script;
    s8 unk16C;
    u8 unk16D;
    u8 unk16E;
    u8 action;
    u8 linked_object_index;
    u8 unk171;
    u8 unk172[2];
    FieldMovementWord movement;
    FieldContactWord contact;
    s32 unk17C;
    u8 targets[13];
    u8 retry_count;
    u8 unk18E;
    u8 unk18F[0x19C - 0x18F];
    s32 unk19C;
    s32 unk1A0;
    s16 path_length;
    s16 path_index;
    u8 tint_red;
    u8 tint_green;
    u8 tint_blue;
    u8 tint_timer;
    s32 path_x;
    s32 path_z;
    u8 unk1B4[0x23C - 0x1B4];
} FieldObjectState;

/** @brief Model part data of one field object (0x48 bytes). */
typedef struct
{
    u32 unk0;
    u32 unk4;
    u8 unk8;
    u8 unk9;
    u8 unkA;
    u8 unkB;
    u8 unkC;
    u8 unkD;
    u8 tint_red;
    u8 tint_green;
    u8 tint_blue;
    u8 unk11;
    u8 unk12[2];
    u32 unk14;
    s16 unk18;
    u8 unk1A[0x23 - 0x1A];
    u8 unk23;
    u32 unk24;
    u32 unk28;
    u8 unk2C;
    u8 unk2D;
    u8 footprint;
    u8 unk2F[2];
    u8 unk31;
    u8 unk32;
    u8 unk33;
    u32 flags;
    u8 unk38[0x48 - 0x38];
} FieldObjectPart;

/** @brief Per-party-member record (0x268 bytes). */
typedef struct
{
    u8 flags;
    u8 type;
    u8 unk2;
    u8 unk3;
    u8 unk4[0x254 - 4];
    u16 unk254;
    u8 unk256;
    u8 unk257[2];
    u8 hit_state;
    u8 unk25A;
    u8 unk25B;
    u8 unk25C;
    u8 unk25D;
    s16 unk25E;
    s16 unk260;
    s16 unk262;
    s16 unk264;
    s16 unk266;
} FieldPlayerRecord;

/** @brief Animation definition referenced by an animation actor slot. */
typedef struct
{
    u8 unk0;
    u8 unk1;
    u8 unk2[0xC - 2];
    u16 flags;
    u16 unkE;
    u16 unk10;
    u16 unk12;
    u8 unk14;
    u8 unk15;
    u8 unk16[2];
    u16 unk18;
} FieldAnimationDef;

/** @brief Status word of an animation actor slot. */
typedef union
{
    u32 word;
    u16 half[2];
    u8 bytes[4];
} FieldActorSlotStatus;

/** @brief Animation actor bound to a party object (0x1C bytes). */
typedef struct
{
    s32 state;
    s32 unk4;
    s32 unk8;
    s32 owner;
    s32 unk10;
    s32 unk14;
    s32 slot;
} FieldActorBinding;

/** @brief Animation actor slot (0x244 bytes). */
typedef struct
{
    void* resource0;
    void* resource4;
    void* resource8;
    FieldAnimationDef* animation;
    FieldAnimationDef* default_animation;
    u8 unk14[0x24 - 0x14];
    u8 active;
    u8 enabled;
    u8 actor_type;
    u8 unk27[2];
    u8 unk29;
    u8 unk2A;
    u8 unk2B[0x222 - 0x2B];
    u16 unk222;
    FieldActorSlotStatus status;
    u8 owner_object_index;
    u8 targets[9];
    u8 target_count;
    u8 unk233[0x238 - 0x233];
    u16 timer;
    u8 track_mask;
    u8 pending_track_mask;
    u8 unk23C[4];
    u16* tracks;
} FieldActorSlot;

/** @brief Resource table entry selected by an actor's resource index (0x14 bytes). */
typedef struct
{
    u8* start;
    u8* end;
    u8 unk8;
    u8 slot_index;
    u8 unkA[0xE - 0xA];
    u16 unkE;
    u32 flags;
} FieldResourceEntry;

extern FieldActor g_field_actors[];
extern FieldObjectState g_field_object_states[];
extern FieldActorBinding g_field_actor_bindings[];
extern FieldActorSlot g_field_actor_slots[];
/** @brief Reserved animation slots of the field objects (g_field_actor_slots[64] onwards). */
extern FieldActorSlot D_800FB3C8[];
extern FieldResourceEntry g_field_resource_entries[];
extern FieldObjectPart g_field_object_parts[];
extern FieldPlayerRecord g_field_player_records[];

/**
 * @brief Find the field actor whose object state carries @p key.
 * @param key Object key compared against each object state's key word.
 * @return The matching actor record, or FIELD_ACTOR_NONE.
 */
static inline FieldActor* field_find_actor(s32 key)
{
    FieldObjectState* state;
    FieldActor* actor;
    s32 i;

    actor = g_field_actors;
    state = g_field_object_states;
    for (i = 0; i < FIELD_ACTOR_COUNT; i++, state++, actor++)
    {
        if (state->key == key)
        {
            return actor;
        }
    }
    return FIELD_ACTOR_NONE;
}

/**
 * @brief Binding used by @p actor (objects 2 and up share the last binding).
 * @param actor Actor whose object index selects the binding.
 * @return The object's animation actor binding.
 */
static inline FieldActorBinding* field_actor_binding(FieldActor* actor)
{
    FieldActorBinding* bindings;
    s32 offset;

    bindings = g_field_actor_bindings;
    if (actor->object_index < 2)
    {
        offset = actor->object_index * sizeof(FieldActorBinding);
    }
    else
    {
        offset = 2 * sizeof(FieldActorBinding);
    }
    return (FieldActorBinding*)((u8*)bindings + offset);
}

/**
 * @brief Binding used by object @p object_index (objects 2 and up share the last binding).
 * @param object_index Object index; only its low byte selects the binding.
 * @return The object's animation actor binding.
 */
static inline FieldActorBinding* field_object_binding(s32 object_index)
{
    FieldActorBinding* bindings;
    s32 offset;

    bindings = g_field_actor_bindings;
    if ((u8)object_index < 2)
    {
        offset = object_index * sizeof(FieldActorBinding);
    }
    else
    {
        offset = 2 * sizeof(FieldActorBinding);
    }
    return (FieldActorBinding*)((u8*)bindings + offset);
}

#endif
