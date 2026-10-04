#ifndef FIELD_ACTOR_RECORDS_H
#define FIELD_ACTOR_RECORDS_H

/**
 * @file field_actor_records.h
 * @brief The FIELD actor, object and resource tables, shared by every FIELD file.
 */

#include "common.h"
#include "common/vector.h"

struct AkaoHeader;

/** @brief Resource table entry selected by an actor's resource index (0x14 bytes). */
typedef struct
{
    u8* start;
    u8* end;
    u8 unk8;
    u8 slot_index;
    /** @brief Depends on the resource: a character's palette or an effect's sound cue. */
    union
    {
        /** @brief CLUT row of the resource's images (field_set_party_palettes, field_update_scene). */
        u16 palette;
        /** @brief High nibble: cue kind; low 12 bits: sound id. */
        u16 sound_cue;
    } attribute;
    u16 unkC;
    /** @brief Passed to the animation starters; FIELD_REQUEST_BOUND selects a bound animation. */
    u16 bound_animation_flags;
    u32 flags;
} FieldResourceEntry;

extern FieldResourceEntry g_field_resource_entries[];

/** @brief Number of field object states (g_field_object_states). */
#define FIELD_OBJECT_COUNT 13

/** @brief Number of recorded positions in an object's route history. */
#define FIELD_ROUTE_HISTORY_LENGTH 48

/** @brief Number of waypoints the path finder can store for an object. */
#define FIELD_PATH_MAX_POINTS 18

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
    u8 bytes[4];
} FieldValueWord;

/** @brief Packed movement word: low ten bits are the effect scale, bit 15 a flag, the top half the resolved height. */
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
        /** @brief Movement flags; the low ten bits are the effect radius or percentage scale. */
        u16 flags;
        /** @brief Collision height resolved for the object. */
        s16 height;
    } half;
} FieldMovementWord;

/** @brief Packed contact word: flag bits, the bound animation actor and the collected target count. */
typedef union
{
    u32 word;
    struct
    {
        u32 flag0 : 1;
        /** @brief The object is linked to linked_object_index, which carries a link flag meanwhile. */
        u32 linked : 1;
        /** @brief Damage scale of the object's hits (0 counts as 1); cleared when an action starts. */
        u32 damage_scale : 3;
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

/** @brief One recorded X/Z position (whole units) of an object's route history. */
typedef struct FieldRoutePoint
{
    s16 x;
    s16 z;
} FieldRoutePoint;

/** @brief One fixed-point X/Z waypoint written by the path finder. */
typedef struct
{
    s32 x;
    s32 z;
} FieldPathPoint;

/** @brief Collision footprint word with unsigned and signed views of the extent. */
typedef union
{
    u32 word;
    struct
    {
        s16 center_offset;
        u16 extent;
    } half;
    struct
    {
        s16 center_offset;
        s16 extent;
    } signed_half;
} FieldCollisionWord;

/**
 * @brief Runtime state of one field object (0x23C bytes).
 * @note Indices correspond to g_field_actors, not the animation-slot pool.
 */
typedef struct FieldObjectState
{
    s32 maximum_hp;
    FieldValueWord current_hp;
    /** @brief Low 24 bits track the animated HP gauge; the high byte carries HUD flags. */
    FieldValueWord hp_display;
    u32 flags;
    s32 group_flags;
    s32 key;
    /** @brief Event bits the object's scripts react to (FieldLayoutRecord::enabled_events); the low two enable touch and action-button interactions. */
    u16 enabled_events;
    /** @brief Interaction scripts, event scripts and actor parameters copied from FieldLayoutRecord::scripts. */
    u16 scripts[16];
    u8 unk3A[2];
    s32 action_parameter;
    s32 sequence_id;
    s32 sequence_position;
    /** @brief Special attack gauge, 0xFF when full; the effect code reads it as a status intensity. */
    u16 technique_gauge;
    u16 action_charge;
    /** @brief Bit 0 selects the special attack gauge on the HUD; the second byte is the object's own index. */
    union
    {
        s32 word;
        struct
        {
            u8 flags;
            u8 object_index;
            u8 unk4E;
            u8 unk4F;
        } bytes;
    } hud;
    s32 target_x;
    s32 target_y;
    s32 target_z;
    s32 effect_angle;
    /** @brief Drops left per reward kind; each drop effect takes one, idle animation 0x1F plays only while any is left. */
    u8 reward_counters[4];
    /** @brief Display name of the object, NULL when it has no label. */
    u8* name;
    /** @brief Stat-derived footprint strength, saturated to 255 during setup. */
    u16 effect_footprint_strength;
    u8 unk6A[2];
    FieldRoutePoint route_history[FIELD_ROUTE_HISTORY_LENGTH];
    FieldCollisionWord collision;
    Vec2s attachment_points[4];
    /** @brief Projected bounds with a packed-word view for empty-box tests. */
    union
    {
        struct
        {
            s16 left;
            s16 top;
            s16 right;
            s16 bottom;
        } half;
        s32 words[2];
    } bounds;
    /** @brief Projected effect corners, also consumed as packed XY pairs. */
    union
    {
        Vec2s points[8];
        s32 words[8];
    } effect_vertices;
    u8* script;
    /** @brief Built-in animation restarted when the actor goes idle; 0xFF for none. */
    u8 idle_animation;
    /** @brief Effect record the object's HUD panel follows while it is bound to an animation actor. */
    u8 linked_effect_index;
    /** @brief Entry of the leader's route history this follower walks towards. */
    u8 route_index;
    u8 action;
    u8 linked_object_index;
    u8 command_timer;
    u16 sequence;
    FieldMovementWord movement;
    FieldContactWord contact;
    /** @brief Flags as of the last handler run, compared with flags to find changes. */
    s32 previous_flags;
    u8 targets[FIELD_OBJECT_COUNT];
    /** @brief Counted while object flag 0x8000 is set; a party member recovers at its type's limit. */
    u8 retry_count;
    /** @brief Non-zero when the player can interact with the object; FIELD_INTERACTION_ITEM for an item to pick up. */
    u8 interaction_kind;
    u8 unk18F;
    Vec2s ground_attachment_points[3];
    /** @brief Floor node cached between collision passes (-1 asks for a fresh search). */
    s32 collision_node;
    s32 collision_flags;
    u16 path_length;
    u16 path_index;
    u8 tint_red;
    u8 tint_green;
    u8 tint_blue;
    u8 tint_timer;
    FieldPathPoint path[FIELD_PATH_MAX_POINTS];
} FieldObjectState;

extern FieldObjectState g_field_object_states[FIELD_OBJECT_COUNT];

/** @brief Tracks an animation actor runs (one per target it follows). */
#define FIELD_ACTOR_TRACK_COUNT 9

/** @brief Parts an animation actor can drive. */
#define FIELD_ACTOR_PART_COUNT 16

/** @brief Vibration tracks of an animation: 0 drives the small motor, 1 the large motor. */
#define FIELD_VIBRATION_TRACK_COUNT 2

/** @brief Packed part flags accessed as a word, halfwords or bytes. */
typedef union
{
    u32 word;
    struct
    {
        u16 low;
        u16 high;
    } halves;
    struct
    {
        u8 low;
        u8 middle_low;
        u8 middle_high;
        u8 high;
    } bytes;
} FieldPartFlags;

/**
 * @brief Part definition of a field object or animation actor (0x48 bytes).
 * @note Controls tracks, placement and effect motion; packed selector words have byte and halfword views.
 */
typedef struct FieldObjectPart
{
    union
    {
        u32 word;
        struct
        {
            u16 low;
            u16 high;
        } halves;
        struct
        {
            u8 low;
            u8 middle_low;
            u8 middle_high;
            u8 high;
        } bytes;
        struct
        {
            u32 unk0 : 24;
            u32 mode : 2;
            u32 unk26 : 6;
        } bits;
    } track_flags;
    union
    {
        u32 word;
        struct
        {
            u16 low;
            u16 high;
        } halves;
        struct
        {
            u8 low;
            u8 middle_low;
            u8 middle_high;
            u8 high;
        } bytes;
        struct
        {
            u32 unk0 : 11;
            u32 flag11 : 1;
            u32 unk12 : 10;
            u32 render_mode : 2;
            u32 unk24 : 8;
        } bits;
    } behavior_flags;
    u8 unk8;
    u8 unk9;
    u8 unkA;
    u8 effect_kind;
    u8 unkC;
    u8 unkD;
    /** @brief Tint of the part; effect parts may use these bytes to select tracks instead. */
    u8 tint_red;
    u8 tint_green;
    u8 tint_blue;
    u8 turn_end_age;
    /** @brief Frames per mesh texture frame; 0 for none. */
    u8 texture_frame_period;
    u8 rotation_y_16;
    /** @brief Orientation selectors; also read through its upper halfword. */
    union
    {
        u32 word;
        struct
        {
            u16 low;
            u16 high;
        } halves;
        struct
        {
            u8 low;
            u8 middle_low;
            u8 middle_high;
            u8 high;
        } bytes;
        struct
        {
            u32 unk0 : 4;
            u32 mode : 4;
            u32 unk8 : 24;
        } bits;
    } orientation_flags;
    s16 unk18;
    u8 unk1A;
    u8 unk1B;
    /** @brief Palette track, angular divisions, footprint flags, and extent low bits. */
    union
    {
        u32 word;
        struct
        {
            u16 palette_track;
            u8 angular_divisions;
            u8 scale_extent_flags;
        } fields;
    } palette_extent;
    /** @brief Extent high bits/mode followed by rotation and effect selectors. */
    union
    {
        u32 word;
        struct
        {
            u8 extent_value_mode;
            u8 rotation_z_track;
            u8 rotation_y_track;
            u8 unk23;
        } fields;
    } rotation_extent;
    union
    {
        s32 word;
        u8 bytes[4];
    } effect_flags;
    FieldPartFlags placement_flags;
    /** @brief Color-track flags, palette selector, horizontal scale, and spawn controls. */
    union
    {
        u32 word;
        struct
        {
            u8 color_track_flags;
            u8 palette_selector;
            /** @brief Horizontal (X and Z) model scale, 0x40 for full size. */
            u8 scale_xz;
            u8 spawn_flags;
        } fields;
    } appearance;
    u8 pitch_acceleration;
    u8 unk31;
    u8 unk32;
    /** @brief Vertical model scale, 0x40 for full size. */
    u8 scale_y;
    /** @brief Halfword selectors and packed word flags. */
    union
    {
        u32 word;
        struct
        {
            u16 part_selectors;
            u16 motion_flags;
        } halves;
    } spawn_flags;
    s16 offset_x;
    s16 offset_y;
    s16 offset_z;
    s16 unk3E;
    s16 unk40;
    s16 unk42;
    s16 unk44;
    s16 unk46;
} FieldObjectPart;

/**
 * @brief Animation definition played by an animation actor slot (0x1C bytes).
 * @note An actor can select one of up to three consecutive definitions.
 */
typedef struct FieldAnimationDef
{
    /** @brief Parameter curves driving the small and large vibration motors, FIELD_CURVE_NONE for none. */
    u8 vibration_curves[FIELD_VIBRATION_TRACK_COUNT];
    /** @brief Two sound events: value compared by non-start events, event type, sound command. */
    u8 sound_subtypes[2];
    u16 sound_events[2];
    u16 sound_commands[2];
    /** @brief FIELD_ANIM_* and FIELD_ANIMATION_HAS_* bits. */
    u16 flags;
    /** @brief Bit 15 plays sound bits 8-14 when track 0 reaches the frame in the low byte. */
    union
    {
        u16 word;
        u8 bytes[2];
    } frame_sound;
    /** @brief Palette animation word; bits 12-15 also select the camera offset curve. */
    u16 palette_animation;
    /** @brief Length in frames when FIELD_ANIM_OWN_DURATION is set. */
    u16 duration;
    /** @brief FieldHitTestMode, plus the FIELD_ANIM_ATTACK and FIELD_ANIM_STARTED values and bits. */
    u8 hit_test_mode;
    /** @brief Part whose attack sphere is tested. */
    u8 hit_test_part;
    /** @brief Frames between the starts of consecutive tracks, 0 to start them all at once. */
    u8 track_interval;
    u8 hit_radius;
    /** @brief FIELD_ANIM_OWNER_* and FIELD_ANIM_TARGET_* visibility, track and curve bits. */
    u16 sync_flags;
    u16 sync_parts;
} FieldAnimationDef;

/** @brief Parameter curve of an animation: segment count, random flag and first segment, then the value range. */
typedef struct
{
    union
    {
        u16 word;
        struct
        {
            u8 segment_count;
            u8 segment_offset;
        } bytes;
    } head;
    s16 end_value;
    s16 start_value;
} FieldParameterCurve;

/** @brief Status word of an animation actor slot. */
typedef union
{
    u32 word;
    u16 half[2];
    u8 bytes[4];
    struct
    {
        /** @brief Bit 0: FIELD_SLOT_OWNER_LINKED; bits 1-4: the action element (FIELD_SLOT_ELEMENT_BITS). */
        u8 flags;
        /** @brief Non-zero while the animation hides its owner or targets. */
        u8 hiding_objects;
        /** @brief Animation resource the slot plays. */
        u16 animation_id;
    } parts;
} FieldActorSlotStatus;

/**
 * @brief Animation actor slot (0x244 bytes): part definitions, per-track counters and object bindings.
 * @note slot_index identifies the slot in g_field_actor_slots; object indices refer to g_field_actors and g_field_object_states.
 */
typedef struct FieldActorSlot
{
    FieldObjectPart* parts;
    FieldParameterCurve* curves;
    /** @brief Curve segments: ten-bit length in frames, six-bit value. */
    u16* curve_segments;
    FieldAnimationDef* animation;
    /** @brief The actor's animation definitions; animation_index selects the one playing. */
    FieldAnimationDef* animations;
    u8* track_data;
    u8* mesh_data;
    /** @brief Sound-effect buffers of the resource, played by kind 2 sound commands. */
    struct AkaoHeader* sound_data[2];
    u8 active;
    /** @brief Number of entries in parts[]. */
    u8 part_count;
    /** @brief Owner's action when the animation started; applied to the targets it hits. */
    u8 hit_reaction;
    /** @brief Per sound event: bit 0 once it has played, bit 7 for a type 5 event. */
    u8 sound_flags[2];
    /** @brief Entry of animations[] that is playing. */
    u8 animation_index;
    u8 sequence_active;
    /** @brief Per-part mesh texture animation frame. */
    u8 mesh_texture_frames[FIELD_ACTOR_PART_COUNT];
    /** @brief Live effects per track and part: spawning one adds one, retiring one takes it away. */
    u8 active_counts[FIELD_ACTOR_TRACK_COUNT][FIELD_ACTOR_PART_COUNT];
    u8 unkCB;
    /** @brief Effects per track and part counted against the part's spawn cap. */
    u16 track_counters[FIELD_ACTOR_TRACK_COUNT][FIELD_ACTOR_PART_COUNT];
    /** @brief Frame counter of each track. */
    u16 track_frames[FIELD_ACTOR_TRACK_COUNT];
    Vec2s track_offsets[FIELD_ACTOR_TRACK_COUNT];
    /** @brief Length of the animation in frames. */
    u16 duration;
    FieldActorSlotStatus status;
    u8 owner_object_index;
    /** @brief Object each track follows; hit contacts add more. */
    u8 targets[FIELD_ACTOR_TRACK_COUNT];
    /** @brief Entries of targets[], including tracks started by new hit contacts. */
    u8 target_count;
    u8 slot_index;
    u16 unk234;
    /** @brief Frames since the animation started. */
    u16 frame_counter;
    /** @brief Frames between the starts of consecutive tracks, 0 to start them all at once. */
    u16 track_interval;
    u8 track_mask;
    u8 pending_track_mask;
    u8 unk23C[4];
    /** @brief Per animation, the mask of parts it drives. */
    u16* part_masks;
} FieldActorSlot;

extern FieldActorSlot g_field_actor_slots[];

#endif
