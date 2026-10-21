#ifndef FIELD_ACTOR_RECORDS_H
#define FIELD_ACTOR_RECORDS_H

/**
 * @file field_actor_records.h
 * @brief The FIELD actor, object and resource tables, shared by every FIELD file.
 */

#include "common.h"

#include "common/vector.h"

struct AkaoHeader;
struct FieldCollisionNode;

/** @brief Resource table entry selected by an actor's resource index (0x14 bytes). */
typedef struct
{
    u8* start;
    u8* end;
    /**
     * @brief Non-zero when the package spans more than one texture image (a large actor).
     * @note Scene loading sets it as image_count != 1, but never for two such actors in a row.
     *       Large actors keep FIELD_MOVEMENT_OVERLAPPING, get a wider shadow and their own contact rule,
     *       and voice entry FIELD_RESOURCE_VOICE_FIRST then plays its set sounds as set 0.
     */
    u8 multi_image;
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

/** @brief Shift of the cue kind in FieldResourceEntry::attribute.sound_cue. */
#define FIELD_SOUND_CUE_KIND_SHIFT 12
/** @brief Sound id bits of FieldResourceEntry::attribute.sound_cue. */
#define FIELD_SOUND_CUE_ID_MASK 0xFFF
/** @brief Sound cue kinds: play the sound once, or play it as a set effect bound to the resource. */
#define FIELD_SOUND_CUE_PLAY 0
#define FIELD_SOUND_CUE_SET_SFX 1

/** @brief FieldResourceEntry::flags bit: the resource has an action table and eight-direction animations. */
#define FIELD_RESOURCE_HAS_ACTIONS 1
/** @brief Number of resource entries in g_field_resource_entries. */
#define FIELD_RESOURCE_ENTRY_COUNT 9
/** @brief Resource entries that can hold relocatable data (the last one is fixed). */
#define FIELD_RESOURCE_MOVABLE_COUNT 8
/** @brief Resource index (FieldActor::resource_index) of actors whose data is not in the arena. */
#define FIELD_RESOURCE_FIXED 8
/** @brief Resource entries 3..5 carry the voice bank of their actors. */
#define FIELD_RESOURCE_VOICE_FIRST 3
#define FIELD_RESOURCE_VOICE_COUNT 3

/** @brief FieldResourceEntry::flags bit: the entry holds loaded data. */
#define FIELD_RESOURCE_LOADED 2
#define FIELD_RESOURCE_LOADED_SHIFT 1
/** @brief Non-zero when resource entry @p entry holds loaded data (FIELD_RESOURCE_LOADED). */
#define FIELD_RESOURCE_IS_LOADED(entry) (((entry).flags >> FIELD_RESOURCE_LOADED_SHIFT) & 1)

extern FieldResourceEntry g_field_resource_entries[];
/** @brief Buffer the loaded resources (FieldResourceEntry::start) are unpacked into. */
extern u8 g_field_resource_buffer[];

/** @brief Number of field object states (g_field_object_states). */
#define FIELD_OBJECT_COUNT 13

/** @brief World coordinates are 24.8 fixed point. */
#define FIELD_POSITION_SHIFT 8
/** @brief Shift from a GTE unit-vector component (12 fraction bits) times a length to world position units. */
#define FIELD_GTE_TO_POSITION_SHIFT (12 - FIELD_POSITION_SHIFT)
/** @brief Objects 0 and 1 are the two player-controlled party members; each has its own binding. */
#define FIELD_PLAYER_COUNT 2
/** @brief Objects 0 to 2 are the party: the two players and the companion. */
#define FIELD_PARTY_COUNT 3

/** @brief Number of recorded positions in an object's route history. */
#define FIELD_ROUTE_HISTORY_LENGTH 48

/** @brief Number of waypoints the path finder can store for an object. */
#define FIELD_PATH_MAX_POINTS 18

/** @brief A 24-bit value word whose top byte holds flags (only FieldObjectState::hp_display uses them). */
typedef union
{
    s32 word;
    struct
    {
        u32 value : 24;      /**< HP, or the HP the HUD gauge currently shows. */
        u32 panel_timer : 7; /**< Frames an enemy HUD panel stays up after an HP change. */
        u32 boss : 1;        /**< The object uses the boss HUD panel. */
    } bits;
    u8 bytes[4];
} FieldValueWord;

/** @brief Packed movement word: effect scale and action flags, with the resolved height in the top half. */
typedef union
{
    u32 word;
    struct
    {
        u32 scale : 10;
        /** @brief The action's charge animation has started (FIELD_MOVEMENT_CHARGED). */
        u32 charged : 1;
        u32 unk11 : 4;
        /** @brief The object cannot be hit or targeted (FIELD_MOVEMENT_INVULNERABLE). */
        u32 invulnerable : 1;
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

/** @brief FieldMovementWord low bits holding the effect scale (ground effect radius). */
#define FIELD_MOVEMENT_SCALE_MASK 0x3FF
/** @brief Object-state contact bits (the contact word of g_field_object_states). */
#define FIELD_CONTACT_ANIMATION_HIDDEN 0x01 /**< An animation actor hides the object; it cannot be targeted. */
#define FIELD_CONTACT_LINKED 0x02           /**< The object is linked to another object. */
#define FIELD_CONTACT_ACTION_BITS 0x1C      /**< Cleared before an action is prepared. */
#define FIELD_CONTACT_NO_HIT_TEST 0x20      /**< The object takes no hits and is never an action target. */
#define FIELD_CONTACT_ACTION_PENDING 0x40   /**< The object charges or waits for an action; hits land even while its own bound animation runs. */
#define FIELD_CONTACT_TARGETED 0x80         /**< The object is being targeted. */
/** @brief Bit positions of the contact bits that are tested one at a time. */
#define FIELD_CONTACT_LINKED_BIT 1
#define FIELD_CONTACT_NO_HIT_TEST_BIT 5
#define FIELD_CONTACT_ACTION_PENDING_BIT 6
#define FIELD_CONTACT_TARGETED_BIT 7

/** @brief FieldMovementWord bits holding the running action sequence (0, 0x800 or 0x1000). */
#define FIELD_MOVEMENT_SEQUENCE_MASK 0x1800
/** @brief FIELD_MOVEMENT_SEQUENCE_MASK value: an effect started the object's action animation without a contact. */
#define FIELD_MOVEMENT_SEQUENCE_LAUNCHED 0x800
/** @brief FIELD_MOVEMENT_SEQUENCE_MASK value: an effect contact resolved the object's pending action. */
#define FIELD_MOVEMENT_SEQUENCE_CONTACT 0x1000
/** @brief FieldObjectState::action_parameter value of an object without a pending action. */
#define FIELD_ACTION_PARAMETER_NONE 0xFFFF
/** @brief FieldObjectState::action_parameter bits 0-9: sequence id of the action's animation. */
#define FIELD_ACTION_PARAMETER_SEQUENCE_MASK 0x3FF
/** @brief FieldObjectState::action_parameter bits 12-13: animation index of the allocated actor. */
#define FIELD_ACTION_PARAMETER_ANIMATION_SHIFT 12
#define FIELD_ACTION_PARAMETER_ANIMATION_MASK 3
/** @brief FieldObjectState::action_parameter bit: the animation index field is valid. */
#define FIELD_ACTION_PARAMETER_SELECT_ANIMATION 0x4000
/** @brief FieldObjectState::action_parameter bit: the action allocated its own animation actor (released when the next sequence starts). */
#define FIELD_ACTION_PARAMETER_TRANSIENT_ACTOR 0x8000

/** @brief Full FieldObjectState::action_charge; below it an action still charges. */
#define FIELD_ACTION_CHARGE_FULL 0xFF

/** @brief Packed contact word: flag bits, the bound animation actor and the collected target count. */
typedef union
{
    u32 word;
    struct
    {
        /** @brief FIELD_CONTACT_ANIMATION_HIDDEN. */
        u32 animation_hidden : 1;
        /** @brief The object is linked to linked_object_index, which carries a link flag meanwhile. */
        u32 linked : 1;
        /** @brief Damage scale of the object's hits (0 counts as 1); cleared when an action starts. */
        u32 damage_scale : 3;
        /** @brief FIELD_CONTACT_NO_HIT_TEST. */
        u32 no_hit_test : 1;
        /** @brief FIELD_CONTACT_ACTION_PENDING. */
        u32 action_pending : 1;
        /** @brief FIELD_CONTACT_TARGETED. */
        u32 targeted : 1;
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
typedef struct FieldPathPoint
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

/** @brief FieldCollisionWord::word with the given centre offset and extent. */
#define FIELD_COLLISION_WORD(center_offset, extent) (((u32)(extent) << 16) | (u16)(center_offset))

/** @brief Reward kinds counted in FieldObjectState::reward_counters. */
#define FIELD_REWARD_KIND_COUNT 4
/** @brief FieldObjectState::idle_animation: no idle animation, and the idle animation that plays only while a reward is left. */
#define FIELD_IDLE_ANIMATION_NONE 0xFF
#define FIELD_IDLE_ANIMATION_REWARD 0x1F
/** @brief Ground effect points of an object (FieldObjectState::ground_effect_points). */
#define FIELD_GROUND_EFFECT_POINT_COUNT 3

/**
 * @brief Ground effects an action draws under its object (FieldActionFlags::target_filter).
 * @note The kind also selects the predicate in g_field_target_filters that picks the
 *       action's targets inside the effect, sized by the effect radius.
 */
typedef enum GroundEffectKind
{
    GROUND_EFFECT_DOME = 0,            /**< Rotating dome of arches. */
    GROUND_EFFECT_STRIPS = 1,          /**< Four rotating arched strips. */
    GROUND_EFFECT_SPIRAL = 2,          /**< Four strips wound around a tilted axis. */
    GROUND_EFFECT_CURVES = 3,          /**< Curved strips running out to both sides. */
    GROUND_EFFECT_SCATTERED_DOMES = 4, /**< Domes at three random ground points. */
    GROUND_EFFECT_DRIFTING_DOME = 5,   /**< Dome drifting in the facing direction. */
    GROUND_EFFECT_AIMED_DOME = 6       /**< Dome steered with the object's controller. */
} GroundEffectKind;

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
    /** @brief Drops left per reward kind; each drop effect takes one, FIELD_IDLE_ANIMATION_REWARD plays only while any is left. */
    u8 reward_counters[FIELD_REWARD_KIND_COUNT];
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
    /** @brief Built-in animation restarted when the actor goes idle; FIELD_IDLE_ANIMATION_NONE for none. */
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
    /**
     * @brief Ground effect centres as X/Z offsets from the object, in whole units.
     * @note The scattered domes use all of them, the drifting and aimed domes the first; spawned effects can be placed on them.
     */
    Vec2s ground_effect_points[FIELD_GROUND_EFFECT_POINT_COUNT];
    /** @brief Floor node cached between collision passes (FIELD_COLLISION_NODE_SEARCH asks for a fresh search). */
    struct FieldCollisionNode* collision_node;
    s32 collision_flags;
    u16 path_length;
    u16 path_index;
    u8 tint_red;
    u8 tint_green;
    u8 tint_blue;
    u8 tint_timer;
    FieldPathPoint path[FIELD_PATH_MAX_POINTS];
} FieldObjectState;

/** @brief FieldObjectState HP words: the HP value in the low 24 bits, HUD flags in the high byte of hp_display. */
#define FIELD_HP_VALUE_MASK 0xFFFFFF
#define FIELD_HP_FLAGS_MASK 0xFF000000

extern FieldObjectState g_field_object_states[FIELD_OBJECT_COUNT];

/**
 * @brief State record of field object @p object_index.
 * @param object_index Index into g_field_object_states.
 * @return The object's state record.
 */
static inline FieldObjectState* field_object_state(s32 object_index)
{
    FieldObjectState* states;

    states = g_field_object_states;
    return &states[object_index];
}

/** @brief FieldObjectState::contact.bytes.animation_actor_index value without an animation actor. */
#define FIELD_ANIMATION_ACTOR_NONE 0xFF

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

/** @brief Neutral value of an actor or part tint channel (no change to the colour). */
#define FIELD_TINT_NEUTRAL 0x80

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
            /** @brief FIELD_PART_BLEND_MODE. */
            u32 blend_mode : 2;
            u32 unk24 : 8;
        } bits;
    } behavior_flags;
    /** @brief FieldActor::animation_active given to each spawned effect; fan effects use it as their segment count. */
    u8 effect_animation_active;
    /** @brief Random pitch range of spawned effects (a track index with track_flags bit 11); zero also copies the owner's facing. */
    u8 pitch_spread;
    /** @brief Pitch added to spawned effects, times 8 (a track index with track_flags bit 12). */
    u8 pitch_offset;
    u8 effect_kind;
    /** @brief Effects kept alive per track: a count, or a parameter track index when placement_flags bit 24 is clear. */
    u8 effect_count;
    /** @brief Frames the part stays active from start_frame; also the lifetime of the effects it spawns. */
    u8 active_frames;
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
            /** @brief Default motion parameter of spawned effects, or its track index or distance divisor (FieldEffectFlags::parameter_mode). */
            s16 high;
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
    /** @brief Motion scale given to spawned effects, 0x100 for 1.0 (a track index with effect parameter mode 1). */
    s16 motion_scale;
    /** @brief FieldMotionRecord::facing_or_reward_kind of spawned effects (their animation or reward kind); the first of a range with
     * FIELD_PART_KIND_RANGE_BIT. */
    u8 facing_or_reward_kind;
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
            /**
             * @brief Linked part: the part spawned when effect_flags bit 31 is set (low nibble), or a lit
             *        mesh's first light source (FIELD_PART_LIGHT_NONE for none); ring effects read it as a radius scale.
             */
            u8 linked_part;
        } fields;
    } rotation_extent;
    union
    {
        u32 word;
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
            /** @brief Low nibble: CLUT column; high bits: effect CLUT row, FIELD_PALETTE_SELECTOR_SHARED and up for the shared CLUT. */
            u8 palette_selector;
            /** @brief Horizontal (X and Z) model scale, 0x40 for full size. */
            u8 scale_xz;
            u8 spawn_flags;
        } fields;
    } appearance;
    u8 pitch_acceleration;
    /** @brief Track frame the part becomes active on; FIELD_PART_ALWAYS from the first frame, FIELD_PART_DISABLED never. */
    u8 start_frame;
    /** @brief Index of the part in its slot's parts[]; selects its track_counters[] column. */
    u8 part_index;
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
    /** @brief Initial work_x/work_y/work_z of spawned effects in whole units: the target point offset of the offset position sources. */
    s16 target_offset_x;
    s16 target_offset_y;
    s16 target_offset_z;
    /** @brief Part whose live effect a FIELD_POSITION_LINKED_EFFECT effect follows. */
    s16 linked_effect_part;
} FieldObjectPart;

/** @brief FieldObjectPart::spawn_flags bit 23: the object moves without map collision (script opcodes 0xA5/0xA6 toggle it). */
#define FIELD_PART_IGNORE_MAP_COLLISION_BIT 23
#define FIELD_PART_IGNORE_MAP_COLLISION (1 << FIELD_PART_IGNORE_MAP_COLLISION_BIT)

/** @brief FieldObjectPart::effect_flags bit 20: the part is a solid body; it collides with the map, casts a footprint and its sprite hit quads act. */
#define FIELD_PART_SOLID 0x00100000

/** @brief FieldObjectPart::effect_flags bit 31: spawning the part also spawns its linked part (rotation_extent.fields.linked_part). */
#define FIELD_PART_SPAWN_LINKED 0x80000000

/** @brief FieldObjectPart::track_flags bit 21: the part draws with its owner's animated CLUT row, which its palette track fills. */
#define FIELD_PART_ANIMATED_PALETTE_BIT 21
#define FIELD_PART_HAS_ANIMATED_PALETTE(part) (((part)->track_flags.word >> FIELD_PART_ANIMATED_PALETTE_BIT) & 1)

/** @brief FieldObjectPart::behavior_flags bits 22-23 (bits.blend_mode): GPU blend mode of the part's sprites and mesh (getTPage abr). */
#define FIELD_PART_BLEND_MODE_SHIFT 22
#define FIELD_PART_BLEND_MODE_MASK 0xC00000
#define FIELD_PART_BLEND_MODE(part) (((part)->behavior_flags.word >> FIELD_PART_BLEND_MODE_SHIFT) & 3)

/** @brief FieldObjectPart::behavior_flags bit 3 and placement_flags bits 2 and 11: how the part's matrix is oriented. */
#define FIELD_PART_ORIENTED_SHIFT 3
#define FIELD_PART_STRETCH_TO_TARGET_SHIFT 2
#define FIELD_PART_AIM_AT_TARGET_SHIFT 11

/** @brief FieldObjectPart::orientation_flags bit 2: the part only runs on track 0, and its effects serve every track. */
#define FIELD_PART_FIRST_TRACK_ONLY 0x4

/** @brief FieldObjectPart::placement_flags bit 25: a new effect takes its colour from parameter tracks instead of the part tint. */
#define FIELD_PART_TRACK_COLOR_BIT 25
#define FIELD_PART_TRACK_COLOR (1 << FIELD_PART_TRACK_COLOR_BIT)

/** @brief FieldObjectPart::palette_extent bits: the owner's footprint strength scales the part's horizontal (scale_xz) and vertical (scale_y) size. */
#define FIELD_PART_FOOTPRINT_SCALES_XZ 0x02000000
#define FIELD_PART_FOOTPRINT_SCALES_Y 0x04000000

/** @brief A part size reduced in ten-percent steps by the footprint strength of object @p owner_index. */
#define FIELD_FOOTPRINT_SCALED_SIZE(size, owner_index)                                                                                                         \
    ((size) - (((size) * ((s32)(FIELD_OBJECT_FOOTPRINT_STRENGTH_RANGE - g_field_object_states[owner_index].effect_footprint_strength) >> 6)) / 10))

/** @brief FieldObjectPart::appearance scale_xz of a full-size object; smaller objects use the small collision footprint. */
#define FIELD_PART_FULL_SCALE 0x40
/** @brief Shift that applies a FieldObjectPart scale (FIELD_PART_FULL_SCALE = 1.0) to a length. */
#define FIELD_PART_SCALE_SHIFT 6

/**
 * @brief Animation definition played by an animation actor slot (0x1C bytes).
 * @note An actor can select one of up to three consecutive definitions.
 */
typedef struct FieldAnimationDef
{
    /** @brief Parameter curves driving the small and large vibration motors, FIELD_CURVE_NONE for none. */
    u8 vibration_curves[FIELD_VIBRATION_TRACK_COUNT];
    /** @brief Two sound events: the value non-start events compare, then the event types and their sound commands. */
    u8 sound_subtypes[2];
    /** @brief Entries 0-1: event type of each sound event; entries 2-3: its sound command (FIELD_SOUND_COMMAND). */
    u16 sound_events[4];
    /** @brief FIELD_ANIM_* and FIELD_ANIMATION_HAS_ALTERNATES bits. */
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

/** @brief FieldAnimationDef::flags bits. */
#define FIELD_ANIM_COLOR_CURVE_MASK 0xFF
#define FIELD_ANIM_COLOR_RGB 0x100
#define FIELD_ANIM_BLEND_SHIFT 9
#define FIELD_ANIM_GLOBAL_COLOR 0x400
/** @brief Two alternate definitions and per-definition part masks follow; the slot stays active between definitions. */
#define FIELD_ANIMATION_HAS_ALTERNATES 0x800
#define FIELD_ANIM_CAMERA_OFFSET 0x1000
#define FIELD_ANIM_CAMERA_MODE_SHIFT 13
/** @brief Camera offset modes (FieldAnimationDef::flags bits 13-14): the curve moves X, Y, both, or X with the next curve moving Y. */
#define FIELD_ANIM_CAMERA_MODE_MASK 3
#define FIELD_ANIM_CAMERA_X 0
#define FIELD_ANIM_CAMERA_Y 1
#define FIELD_ANIM_CAMERA_XY 2
#define FIELD_ANIM_CAMERA_X_THEN_Y 3
/** @brief FieldAnimationDef::flags bit 15: the definition carries its own duration. */
#define FIELD_ANIM_OWN_DURATION 0x8000
/** @brief Test a FieldAnimationDef::flags bit from bit 8 up through the flags' high byte. */
#define FIELD_ANIM_HIGH_FLAG(flags, flag) (((flags) >> 8) & ((flag) >> 8))
/** @brief FieldAnimationDef::frame_sound: bit 15 fires a sound at a frame; bits 8-14 are the sound, the low byte the frame. */
#define FIELD_ANIM_FRAME_SOUND 0x8000
/** @brief Sound (bits 8-14) of a FieldAnimationDef::frame_sound word. */
#define FIELD_ANIM_FRAME_SOUND_ID(word) (((word) >> 8) & 0x7F)
/** @brief FieldAnimationDef::palette_animation bits 12-15: curve of the camera offset track. */
#define FIELD_ANIM_CAMERA_CURVE_SHIFT 12
/** @brief FieldAnimationDef::hit_test_mode values and bits. */
#define FIELD_ANIM_ATTACK 2
#define FIELD_ANIM_STARTED 0x80
/** @brief FieldAnimationDef::sync_flags bits. */
#define FIELD_ANIM_OWNER_VISIBILITY 0x2
#define FIELD_ANIM_TARGET_VISIBILITY 0x4
#define FIELD_ANIM_OWNER_TRACK_OFF 0x8
#define FIELD_ANIM_TARGET_TRACK_OFF 0x10
#define FIELD_ANIM_RESTART_AT_END 0x20
#define FIELD_ANIM_OWNER_CURVE_SHIFT 8
#define FIELD_ANIM_TARGET_CURVE_SHIFT 12
#define FIELD_ANIM_CURVE_MASK 0xF
/** @brief Visibility without a curve: the part named in the curve bits hides the owner or the targets while its effects live. */
#define FIELD_ANIM_OWNER_PART_HIDES (FIELD_ANIM_OWNER_VISIBILITY | FIELD_ANIM_OWNER_TRACK_OFF)
#define FIELD_ANIM_TARGET_PART_HIDES (FIELD_ANIM_TARGET_VISIBILITY | FIELD_ANIM_TARGET_TRACK_OFF)

/** @brief Target entry of an animation track without a target object. */
#define FIELD_TARGET_NONE 0xFF

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
        struct
        {
            u8 segment_count : 7;
            u8 random : 1;
        } bits;
    } head;
    s16 end_value;
    s16 start_value;
} FieldParameterCurve;

/** @brief FieldParameterCurve::head segment_count bits, and the random flag above them. */
#define FIELD_CURVE_SEGMENT_COUNT_MASK 0x7F
#define FIELD_CURVE_RANDOM 0x80

/** @brief FieldActorSlot::status bit 0: the slot plays a streamed animation bound to its owner. */
#define FIELD_SLOT_OWNER_LINKED 0x1
/** @brief FieldActorSlot::status bits holding the starting action's ground effect (GroundEffectKind), shifted by one; all set means none. */
#define FIELD_SLOT_GROUND_EFFECT_BITS 0x1E
#define FIELD_SLOT_GROUND_EFFECT_SHIFT 1

/** @brief Status word of an animation actor slot. */
typedef union
{
    u32 word;
    u16 half[2];
    u8 bytes[4];
    struct
    {
        /** @brief Bit 0: FIELD_SLOT_OWNER_LINKED; bits 1-4: the action's ground effect (FIELD_SLOT_GROUND_EFFECT_BITS). */
        u8 flags;
        /** @brief Non-zero while the animation hides its owner or targets. */
        u8 hiding_objects;
        /** @brief Animation resource the slot plays. */
        u16 animation_id;
    } parts;
} FieldActorSlotStatus;

/** @brief Sound-effect buffers a FieldActorSlot can hold. */
#define FIELD_ACTOR_SOUND_COUNT 2

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
    /** @brief Mesh table of the resource (see FieldMeshResource in field_mesh.h). */
    struct FieldMeshResource* meshes;
    /** @brief Sound-effect buffers of the resource, played by kind 2 sound commands. */
    struct AkaoHeader* sound_data[FIELD_ACTOR_SOUND_COUNT];
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
    /**
     * @brief Cleared with frame_counter whenever the tracks reset (field_reset_actor_track_state); never read.
     * @note Write-only in US and JP: no load of slot offset 0x234 exists, so its purpose is unknown.
     */
    u16 unused234;
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
/** @brief Frame data of the built-in animations. */
extern u8* g_field_builtin_track_data;

/**
 * @brief Object-state flags bits 0-8: status ailments, in the order of the status text and immunity tables.
 * @note Bit n is status effect n (field_apply_status_effect); g_field_object_flag_handlers starts each one's looping animation.
 */
#define FIELD_OBJECT_FLAG_POISON 0x0001
#define FIELD_OBJECT_FLAG_DARKNESS 0x0002
#define FIELD_OBJECT_FLAG_PARALYSIS 0x0004
#define FIELD_OBJECT_FLAG_CONFUSION 0x0008
#define FIELD_OBJECT_FLAG_FLAME_BURST 0x0010
#define FIELD_OBJECT_FLAG_HYPNOSIS 0x0020
#define FIELD_OBJECT_FLAG_SLEEP 0x0040
#define FIELD_OBJECT_FLAG_PETRIFIED 0x0080
#define FIELD_OBJECT_FLAG_FROZEN 0x0100
/** @brief Object-state flags bit set while the object is knocked out. */
#define FIELD_OBJECT_FLAG_KNOCKED_OUT 0x0200
/** @brief Object-state flags bit set while another object is linked to this one. */
#define FIELD_OBJECT_FLAG_LINK_TARGET 0x2000
/**
 * @brief Object-state flags bit: the Mini status (effect 10) - the object is shrunk to half size and its actions are restricted.
 * @note Pad input may then only start actions 2 and 3; scripted actions and techniques
 *       do not start at all. The bit is cleared when the object is hit.
 */
#define FIELD_OBJECT_FLAG_MINI 0x0400
/** @brief Object-state flags bit set while a dash ability (Lunge, Retreat, Defensive Lunge, Slide) plays. */
#define FIELD_OBJECT_FLAG_DASH 0x4000
/** @brief Object-state flags bit: an action chain is running (cleared with the retry count). */
#define FIELD_OBJECT_CHAINING 0x8000
/** @brief Object-state flags bit set for one frame when the object is hit; tints the actor red. */
#define FIELD_OBJECT_FLAG_HIT_FLASH 0x10000000

/** @brief Object-state flag bits that make an object an invalid target. */
#define FIELD_OBJECT_UNTARGETABLE_FLAGS (FIELD_OBJECT_FLAG_PETRIFIED | FIELD_OBJECT_FLAG_KNOCKED_OUT | FIELD_OBJECT_FLAG_LINK_TARGET)

/** @brief Presence value of a visible actor record. */
#define FIELD_ACTOR_VISIBLE 0

/** @brief Presence value of an unused actor record. */
#define FIELD_ACTOR_UNUSED 0xFF

/** @brief Presence value of a hidden actor record. */
#define FIELD_ACTOR_HIDDEN 0xFE

/** @brief Low seven bits of FieldActor::animation select the animation. */
#define FIELD_ANIMATION_INDEX_MASK 0x7F
/** @brief FieldActor::animation bit that mirrors the animation. */
#define FIELD_ANIMATION_FACING 0x80
/** @brief Shift of FIELD_ANIMATION_FACING. */
#define FIELD_ANIMATION_FACING_SHIFT 7

/**
 * @brief Abilities a player assigns to the action buttons (FieldActionSlot::command).
 * @note An ability's code is also the animation the actor plays while using it
 *       (FieldActor::animation without the facing bit). The names follow the
 *       game's command name table (g_field_command_names), which gives each
 *       unused code below an ability the same name as that ability.
 */
typedef enum FieldAbility
{
    FIELD_ABILITY_JUMP = 0x08,
    /** Guards; held with the button. */
    FIELD_ABILITY_DEFEND = 0x0A,
    FIELD_ABILITY_LUNGE = 0x2F,
    FIELD_ABILITY_RETREAT = 0x30,
    /** Held with the button. Defense counts as 0 while it plays; also the pose after an instrument or a duel knock-down. */
    FIELD_ABILITY_CROUCH = 0x31,
    FIELD_ABILITY_PUSH = 0x32,
    FIELD_ABILITY_CHEER = 0x33,
    FIELD_ABILITY_SPIN = 0x34,
    FIELD_ABILITY_GRAPPLE = 0x35,
    FIELD_ABILITY_WHIRL = 0x36,
    FIELD_ABILITY_BASH = 0x37,
    /** Never touched by effect quads. */
    FIELD_ABILITY_SOMERSAULT = 0x38,
    /** Never touched by effect quads. */
    FIELD_ABILITY_BACK_ROLL = 0x39,
    FIELD_ABILITY_BACK_FLIP = 0x3A,
    FIELD_ABILITY_MOONSAULT = 0x3B,
    /** A player using it is not hit by attack spheres. */
    FIELD_ABILITY_HIGH_JUMP = 0x3C,
    /** A player using it keeps its height while moving. */
    FIELD_ABILITY_DOUBLE_JUMP = 0x3D,
    FIELD_ABILITY_TACKLE = 0x3E,
    /** Answers an effect quad with action 2. */
    FIELD_ABILITY_COUNTERATTACK = 0x3F,
    /** Answers an effect quad with action 3. */
    FIELD_ABILITY_COUNTERSTRIKE = 0x40,
    FIELD_ABILITY_TAUNT = 0x41,
    FIELD_ABILITY_EVADE = 0x42,
    FIELD_ABILITY_TOSS = 0x43,
    /** Ignores hits. */
    FIELD_ABILITY_DEFENSIVE_LUNGE = 0x44,
    FIELD_ABILITY_SLIDE = 0x45,
    /** Follow-up attack with no name of its own; nothing in the field code starts it. */
    FIELD_ABILITY_FOLLOW_UP_4E = 0x4E,
    FIELD_ABILITY_FLIP_KICK = 0x4F,
    /** Follow-up attack Spin chains into while button 2 is held. */
    FIELD_ABILITY_FOLLOW_UP_50 = 0x50,
    /** Follow-up attack Spin chains into while button 3 is held. */
    FIELD_ABILITY_FOLLOW_UP_51 = 0x51
} FieldAbility;

/** @brief Player animations outside FieldAbility (the command name table has no distinct names for them). */
/** @brief Landing pose after a Jump or Double Jump; the landing effect plays once it is reached. */
#define FIELD_ANIMATION_JUMP_LANDING 0x09
/** @brief Power Attack (the default command of FIELD_RESOURCE_ACTION_POWER_ATTACK). */
#define FIELD_ANIMATION_POWER_ATTACK 0x25
/** @brief Aerial moves that end with the landing effect, like Back Flip and Moonsault. */
#define FIELD_ANIMATION_AERIAL_46 0x46
#define FIELD_ANIMATION_AERIAL_47 0x47
/** @brief Air attack that drifts forward while it falls; its hits resolve as battle action 0x10. */
#define FIELD_ANIMATION_AIR_ATTACK 0x48
/** @brief Power Attack started in the air from a jump; drifts forward while it falls, hits resolve as battle action 0x11. */
#define FIELD_ANIMATION_AIR_POWER_ATTACK 0x49

/** @brief FieldObjectPart::placement_flags bits 12-13: where a sprite effect takes its CLUT (FIELD_PART_PALETTE_*). */
#define FIELD_PART_PALETTE_SOURCE_SHIFT 12
#define FIELD_PART_PALETTE_SOURCE_MASK 3
#define FIELD_PART_PALETTE_SOURCE(part) (((part)->placement_flags.word >> FIELD_PART_PALETTE_SOURCE_SHIFT) & FIELD_PART_PALETTE_SOURCE_MASK)
/** @brief Palette sources: the resource's CLUT row, the part's palette selector, or the sprite frame's own entry. */
#define FIELD_PART_PALETTE_RESOURCE 0
#define FIELD_PART_PALETTE_SELECTOR 1
#define FIELD_PART_PALETTE_FRAME 2

/** @brief Palette selector bits of the CLUT column (in 16-entry steps). */
#define FIELD_PALETTE_SELECTOR_COLUMN_MASK 0xF
/** @brief Shift of the effect CLUT row in a palette selector. */
#define FIELD_PALETTE_SELECTOR_ROW_SHIFT 4
/** @brief Palette selectors from this value up use the shared CLUT row. */
#define FIELD_PALETTE_SELECTOR_SHARED 0x40

/** @brief FieldActor::direction headings are 256 steps per turn (ratan2/rcos use 4096); heading 0 faces +x. */
#define FIELD_HEADING_STEPS 256
#define FIELD_HEADING_QUARTER_TURN (FIELD_HEADING_STEPS / 4)
#define FIELD_HEADING_HALF_TURN (FIELD_HEADING_STEPS / 2)
#define FIELD_HEADING_THREE_QUARTER_TURN (FIELD_HEADING_QUARTER_TURN * 3)
/** @brief Heading of facing sector @p sector (0-7), one eighth of a turn each. */
#define FIELD_HEADING_SECTOR(sector) ((sector) * (FIELD_HEADING_STEPS / 8))

/** @brief Actor commands (FieldActor::command). */
enum
{
    FIELD_ACTOR_COMMAND_NONE = 0,
    FIELD_ACTOR_COMMAND_BASE = 0x81,
    FIELD_ACTOR_COMMAND_STEP = 0x81,
    FIELD_ACTOR_COMMAND_HIT = 0x82,
    FIELD_ACTOR_COMMAND_ACTION = 0x85,
    FIELD_ACTOR_COMMAND_RECOVER = 0x86,
    FIELD_ACTOR_COMMAND_INSTRUMENT = 0x87,
    FIELD_ACTOR_COMMAND_WALK = 0x88,
    FIELD_ACTOR_COMMAND_RISE = 0x89,
    FIELD_ACTOR_COMMAND_SINK = 0x8A,
    FIELD_ACTOR_COMMAND_WALK_TO_TARGET = 0x8B,
    FIELD_ACTOR_COMMAND_WALK_FROM_TARGET = 0x8C,
    FIELD_ACTOR_COMMAND_SEQUENCE_STEP = 0x8D,
    FIELD_ACTOR_COMMAND_KNOCKED_DOWN = 0x8E,
    FIELD_ACTOR_COMMAND_JUMP = 0x8F,
    FIELD_ACTOR_COMMAND_DEFEATED = 0x90,
    FIELD_ACTOR_COMMAND_TECHNIQUE = 0x91,
    FIELD_ACTOR_COMMAND_DEFEAT_BOUND = 0x92,
    FIELD_ACTOR_COMMAND_DEFEAT_WAIT = 0x93,
    FIELD_ACTOR_COMMAND_DEFEAT_END = 0x94,
    FIELD_ACTOR_COMMAND_SEQUENCE_WAIT = 0x95,
    /** Follow-up attack started from a jump ability while action button 3 is held; no further chaining. */
    FIELD_ACTOR_COMMAND_AIR_FOLLOW_UP = 0x96,
    /** Runs like FIELD_ACTOR_COMMAND_SECOND_JUMP and counts as an action for hits; nothing in the C code sets it. */
    FIELD_ACTOR_COMMAND_98 = 0x98,
    FIELD_ACTOR_COMMAND_IDLE_AFTER_ANIMATION = 0x99,
    FIELD_ACTOR_COMMAND_IDLE_AFTER_RELOAD = 0x9A,
    /** Second jump of a Double Jump, started while recovering from the first. */
    FIELD_ACTOR_COMMAND_SECOND_JUMP = 0x9B,
    /** Raise the actor by command_param each frame for unk26 frames. */
    FIELD_ACTOR_COMMAND_TIMED_LIFT = 0x9C,
    /** Lower the actor by command_param each frame until it reaches the ground (or unk26 runs out). */
    FIELD_ACTOR_COMMAND_DESCEND_TO_GROUND = 0x9D,
    FIELD_ACTOR_COMMAND_APPROACH_TARGET = 0xA0,
    FIELD_ACTOR_COMMAND_ACTION_END = 0xA4,
    FIELD_ACTOR_COMMAND_TIMED_WALK = 0xA7,
    FIELD_ACTOR_COMMAND_WAIT_ANIMATION = 0xA8,
    FIELD_ACTOR_COMMAND_RUN_TO_TARGET = 0xAC,
    FIELD_ACTOR_COMMAND_RUN = 0xAD,
    FIELD_ACTOR_COMMAND_DEFEAT_DELAY = 0xAE,
    FIELD_ACTOR_COMMAND_FOLLOW_LEADER = 0xAF,
    FIELD_ACTOR_COMMAND_WALK_PATH = 0xB0,
    FIELD_ACTOR_COMMAND_RUN_PATH = 0xB1,
    FIELD_ACTOR_COMMAND_TURN = 0xB2,
    FIELD_ACTOR_COMMAND_STOP = 0xB3,
    FIELD_ACTOR_COMMAND_LEAVE_PATH = 0xB5,
    FIELD_ACTOR_COMMAND_TIMED_SLIDE = 0xB6,
    FIELD_ACTOR_COMMAND_WAIT = 0xB7,
    FIELD_ACTOR_COMMAND_ATTACHED = 0xB8,
    /** Leave the actor and its command untouched (dropped items wait in it until removed). */
    FIELD_ACTOR_COMMAND_HOLD = 0xBA,
    FIELD_ACTOR_COMMAND_RETIRE = 0xBB,
    FIELD_ACTOR_COMMAND_WAIT_BOUND_ACTOR = 0xBC,
    FIELD_ACTOR_COMMAND_INSTRUMENT_END = 0xBD
};

#endif
