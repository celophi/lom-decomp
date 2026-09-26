/** @file field_scene_transition.c
 * @brief Scene changes: reading a scene file, installing its actors and resources, and the transition tiles.
 */

#include "field_scene_transition.h"
#include "field_text.h"
#include "cdrom.h"
#include "cd_resources.h"
#include "game_audio.h"
#include "akao_cmd.h"
#include "common.h"
#include "controller_internal.h"
#include "scene_state.h"
#include "field_actor_routes.h"
#include "field_actor_runtime.h"
#include "field_actor_tables.h"
#include "field_calls.h"
#include "field_interaction_start.h"
#include "field_state_ops.h"
#include "sdk/libgpu.h"
#include "sdk/libetc.h"
#include "sdk/libgte.h"

/** @brief World coordinates are 24.8 fixed point. */
#define FIELD_POSITION_SHIFT 8
/** @brief Scene resource selector bit kept in g_field_scene_mode_bit (the scene runs field_battle_setup). */
#define FIELD_SCENE_MODE_FLAG 0x8000
/** @brief Scene resource selector bits holding the scene id. */
#define FIELD_SCENE_ID_MASK 0x7FFF
/** @brief Where a scene file is read to. */
#define FIELD_SCENE_FILE ((u8*)0x80190000)
/** @brief Map word of a scene: map id, and in bit 15 the timer mode for field_initialize_actor_parts. */
#define FIELD_SCENE_MAP_ID_MASK 0x7FFF
#define FIELD_SCENE_PARTS_TIMER_SHIFT 15
/** @brief Music id that keeps the current song playing. */
#define FIELD_MUSIC_KEEP -1
/** @brief Music id that loads the fixed song instead of a scene song. */
#define FIELD_MUSIC_FIXED -2
/** @brief Frames of the song fade-out when the scene does not keep the entry music. */
#define FIELD_MUSIC_FADE_FRAMES 60
/** @brief Audio timer set when the entry music is kept. */
#define FIELD_ENTRY_MUSIC_TIMER 15
/** @brief Most images one actor description lists. */
#define FIELD_ACTOR_IMAGE_COUNT 6
/** @brief Image index that ends an actor description's image list. */
#define FIELD_ACTOR_IMAGE_END 0xFF
/** @brief Bytes of one scene portrait. */
#define FIELD_SCENE_PORTRAIT_SIZE 1184
/** @brief Resource entry of the built-in fallback geometry. */
#define FIELD_FALLBACK_RESOURCE 8
/** @brief VRAM slot of the fallback resource. */
#define FIELD_FALLBACK_SLOT 7
/** @brief Number of party palettes in g_field_party_palettes. */
#define FIELD_PARTY_PALETTE_COUNT 6
/** @brief First texture slot handed to the scene actors' images. */
#define FIELD_FIRST_IMAGE_SLOT 2
/** @brief Texture slots the allocator skips (FIELD_IMAGE_SLOT_SKIP_COUNT slots from this one). */
#define FIELD_IMAGE_SLOT_SKIP_FIRST 6
#define FIELD_IMAGE_SLOT_SKIP_COUNT 4
/** @brief Width of one CLUT row in VRAM halfwords. */
#define FIELD_CLUT_ROW_WIDTH 256
/** @brief Resource image slots from this one on use the second VRAM texture row. */
#define FIELD_IMAGE_SLOT_SECOND_ROW 10
/**
 * @brief Texture slot placement: slots are 64 halfwords wide and counted leftwards, slot n of the
 *        first row at x = 832 - 64n, slot n >= 10 at x = 960 - 64(n - 9) in the row at y = 256.
 */
#define FIELD_IMAGE_ROW0_X 832
#define FIELD_IMAGE_ROW1_X 960
#define FIELD_IMAGE_ROW1_Y 256
/** @brief First VRAM row of the actor CLUTs (one row per actor). */
#define FIELD_ACTOR_CLUT_VRAM_Y 500
/** @brief Scene that ignores the actor groups of its layout. */
#define FIELD_SCENE_NO_LAYOUT_GROUPS 0x13D
/** @brief Default spawn point when the scene has no spawn record. */
#define FIELD_DEFAULT_SPAWN_X 160
#define FIELD_DEFAULT_SPAWN_Z 224
/** @brief Spacing along X of the party before the spawn is placed. */
#define FIELD_PARTY_START_SPACING 40
/** @brief Actor animations per direction group (stand, walk, run, ...). */
#define FIELD_DIRECTION_ANIMATION_COUNT 5
/** @brief Mover footprints of a full-size and a small actor, and the step height they climb. */
#define FIELD_FOOTPRINT_LARGE_WIDTH 12
#define FIELD_FOOTPRINT_LARGE_DEPTH 8
#define FIELD_FOOTPRINT_WIDTH 9
#define FIELD_FOOTPRINT_DEPTH 6
#define FIELD_FOOTPRINT_STEP 16
/** @brief Full model scale of an object part (FieldObjectPart::scale_z). */
#define FIELD_PART_FULL_SCALE 0x40
/** @brief Transition tile image size and VRAM positions. */
#define FIELD_TRANSITION_TILE_SIZE 32
#define FIELD_TRANSITION_TILE_X 288
#define FIELD_TRANSITION_TILE_Y0 180
#define FIELD_TRANSITION_TILE_Y1 412
/** @brief Pixels in one transition tile image. */
#define FIELD_TRANSITION_TILE_PIXELS 0x800
/** @brief Bytes of the TIM file header in front of the CLUT block. */
#define FIELD_TIM_HEADER_SIZE 8
/** @brief Fade colour value of a white fade target. */
#define FIELD_FADE_WHITE 0x1FF
/** @brief Colour bits of a 15-bit VRAM pixel (without the semi-transparency bit). */
#define FIELD_PIXEL_COLOR_MASK 0x7FFF
/** @brief Transparent and opaque-white transition tile pixels. */
#define FIELD_PIXEL_TRANSPARENT 0
#define FIELD_PIXEL_WHITE 0xFFFF

/** @brief Facing of a spawn record: index into the direction tables. */
#define FIELD_SPAWN_FACING(spawn) ((spawn)->h.facing & 0xF)

/** @brief Fixed-point X and Z limits of the map; Z is stored at half resolution. */
#define FIELD_MAP_X_LIMIT(bounds) ((bounds)->width << FIELD_POSITION_SHIFT)
#define FIELD_MAP_Z_LIMIT(bounds) ((s16)(bounds)->depth << (FIELD_POSITION_SHIFT + 1))

/** @brief Scratchpad copy of the mover used by the collision code. */
#define FIELD_COLLISION_MOVER ((struct FieldCollisionMover*)0x1F800000)

/** @brief Address @p p rounded up to a word boundary, as an integer. */
#define FIELD_ALIGN4(p) (((s32)(p) + 3) & ~3)

/**
 * @brief Form a typed pointer from a byte offset plus a base address.
 * @note The integer sum keeps the offset as the first addu operand; pointer + int would not.
 */
#define OFFSET_FIRST_PTR(type, offset, base) ((type*)((offset) + (s32)(base)))

/**
 * @brief Header words of a scene file: byte offsets of its sections from the start of the file.
 *
 * The event scripts, strings, actor scripts and records sections are copied
 * back to back into g_field_scene_data_buffer; the actors section follows the
 * records and ends that range.
 */
enum
{
    FIELD_SCENE_LAYOUT = 0x00,
    FIELD_SCENE_EVENT_SCRIPTS = 0x04,
    FIELD_SCENE_STRINGS = 0x08,
    FIELD_SCENE_ACTOR_SCRIPTS = 0x0C,
    FIELD_SCENE_RECORDS = 0x10,
    FIELD_SCENE_ACTORS = 0x14,
    FIELD_SCENE_GEOMETRY = 0x18,
    FIELD_SCENE_IMAGES = 0x1C,
    FIELD_SCENE_PORTRAITS = 0x20,
    FIELD_SCENE_GROUP_BOUNDS = 0x24
};

/**
 * @brief Section of scene file @p file whose byte offset is stored in header word @p entry.
 * @note The header is read as plain words; reads through a header struct would be scheduled above the stores in between.
 */
#define FIELD_SCENE_SECTION(file, entry) ((file) + *(s32*)((file) + (entry)))

/** @brief Actor image list and variable-length action records in a scene file. */
typedef struct
{
    u8 unk0;
    /** @brief Bit 7: the resource has an action table (FIELD_RESOURCE_HAS_ACTIONS). */
    u8 flags;
    /** @brief Image indices, FIELD_ACTOR_IMAGE_END-terminated when shorter than six. */
    u8 images[FIELD_ACTOR_IMAGE_COUNT];
    u16 palette;
    u16 unkA;
    s32 action_count;
    FieldActionSlot actions[1];
} FieldSceneActorDescription;

/** @brief TIM block containing its length, VRAM rectangle, and pixel words. */
typedef struct
{
    u32 size;
    RECT rect;
    u_long pixels[1];
} FieldTimBlock;

/** @brief Paletted TIM file header followed by its CLUT block. */
typedef struct
{
    u32 magic;
    u32 flags;
    FieldTimBlock palette;
} FieldTimFile;

/** @brief Position in a TIM image: the file header, or the pixel block that follows the CLUT block. */
typedef union
{
    FieldTimFile file;
    FieldTimBlock block;
} FieldTimData;

/** @brief Spawn position and facing of a scene entrance. */
typedef union
{
    /** @brief Map coordinates and facing/flag bytes. */
    struct
    {
        s16 x, y, z;
        u8 facing;
        u8 flags;
    } h;
    /** @brief Packed spawn words with a four-bit facing selector. */
    struct
    {
        u32 xy;
        u32 z : 16;
        u32 facing : 4;
        u32 unk20 : 12;
    } bits;
} FieldSceneSpawn;

/** @brief Entry of g_field_direction_offsets: the (dx, dz) offset of one of the sixteen facings. */
typedef struct
{
    s16 dx;
    s16 dz;
} FieldDirectionOffset;

/** @brief Actor layout of a scene file: a count and the actors' action requests. */
typedef struct
{
    s32 count;
    FieldActionRequest requests[1];
} FieldSceneLayout;

/** @brief Mover handed to field_collision_move_mover (same layout as in field_collision.c). */
struct FieldCollisionMover
{
    s32 x;
    s32 height;
    s32 z;
    s32 move_x;
    s32 move_height;
    s32 move_z;
    s32 resolved_height;
    s32 collision_node;
    s32 flags;
    s16 footprint_width;
    s16 height_bias;
    union
    {
        s32 mode_flags;
        struct
        {
            unsigned footprint_depth : 16;
            unsigned airborne_low : 1;
            unsigned airborne_high : 1;
            unsigned unused : 14;
        } bits;
    } mode;
};

/** @brief Transition fade target color and duration. */
typedef struct
{
    s16 red;
    s16 green;
    s16 blue;
    s16 duration;
} FieldTransitionFade;

s32 akao_cmd_c1(s32 song_handle, s32 frames, s32 volume);
void* field_header_record_at(s32 index);
void field_restart_actor_animation(FieldActor* actor);
/* Unprototyped on purpose: the (s16, s8, s8, ...) definition would narrow the arguments here. */
void field_store_entry_settings();

extern FieldRenderHalf* g_field_render_context;
extern FieldActionRow g_field_resource_actions[];
extern s32 g_field_direction_animation_modes[];
extern FieldDirectionOffset g_field_direction_offsets[];
extern FieldTransitionFade g_field_fade_target;
/** @brief Built-in geometry of the fallback resource. */
extern u8 g_field_fallback_geometry[0x3C];
/** @brief CLUT row of the hero per party palette index. */
extern u16 g_field_party_palettes[FIELD_PARTY_PALETTE_COUNT];
/** @brief Transition tile images; field_prepare_transition_tiles recolours the first. */
extern u16 g_field_transition_tiles[FIELD_TRANSITION_TILE_PIXELS];
extern u16 g_field_transition_tiles_alt[FIELD_TRANSITION_TILE_PIXELS];
/** @brief Non-zero uploads g_field_transition_tiles_alt next; flips on every upload. */
extern s32 g_field_transition_tile_bank;
/** @brief Number of loaded actors; the next scene actor takes this object index. */
extern s32 g_field_loaded_actor_count;
/* TODO: Multiple-image resource flag also raises ability growth from 1 to 4. */
extern s32 D_80115890;
extern void* g_field_resource_cursor;

extern s32 g_field_pending_spawn_id;
extern s32 g_field_pending_music_id;
extern s32 g_field_pending_secondary_music_id;
extern s32 g_field_pending_scene_id;
extern s32 g_field_pending_object_id;
extern s32 g_field_pending_sound_bank_id;
extern s32 g_field_scene_request_pending;
extern s32 g_field_scene_data_size;
extern s32 g_field_active_group;
extern u32 g_field_group_bounds[];
extern s32 g_field_group_bounds_count;
extern u16* g_field_event_scripts;
extern u16* g_field_actor_scripts;
extern u32* g_field_scene_record_table;
extern s32 g_field_hide_actor_panels;
extern s32 g_field_camera_target_x;
extern s32 g_field_camera_target_z;
extern s32 g_field_camera_follow_x;
extern s32 g_field_camera_follow_z;
/* TODO: Script command 0x44 selects alternate party control and triple HP. */
extern s32 g_field_duel_mode;
extern s32 g_field_scene_contact_latched;
extern u8 g_field_scene_data_buffer[];
extern s32 g_field_song_volume;
extern s32 g_field_preserve_entry_music;
extern s32 g_field_scene_mode_bit;
extern s32 g_field_previous_song_volume;
extern s32 g_field_party_palette_index;
extern u32 g_field_scene_portrait_count;
extern u8* g_field_scene_portraits;
extern u8* g_field_scene_strings;
extern s32 g_field_dialog_item_count;
extern s32 g_field_audio_timer;
/* Declared here, not through main.h: FIELD reads and writes g_scene_mode as a whole word. */
extern s32 g_scene_mode;
extern s32 g_layout_flag;
extern s32 g_layout_option;
extern s32 g_layout_sub_mode;

static void field_reset_fallback_resource(void);
static void field_upload_actor_image(FieldTimData* image, s32 image_slot, s32 actor_index, s32 upload_palette);
static void field_copy_scene_geometry(s32* src, s32* end);
static void field_load_scene_actors(FieldSceneLayout* layout);
static void field_refresh_actor_collisions(void);
static void field_upload_transition_tiles(void);
static void field_prepare_transition_tiles(void);

/**
 * @brief Queue a seek to the scene selected by the low fifteen selector bits.
 * @param scene_selector Scene resource selector, optionally carrying the high-bit mode flag.
 */
void field_seek_scene_resource(s32 scene_selector)
{
    cdrom_queue_seek((scene_selector & FIELD_SCENE_ID_MASK) + CD_RES_FIELD_SCENE_BASE);
}

/**
 * @brief Record the parameters for a pending scene change.
 * @param scene_id Scene id (with the high-bit mode flag preserved).
 * @param object_id Field object selected for the render context.
 * @param spawn_id Spawn id.
 * @param music_id Primary music resource; -1 retains music, -2 loads the fixed sequence.
 * @param sound_bank_id Sound-bank resource; -1 clears its header, -2 retains it.
 * @param secondary_music_id Secondary music resource, or -1 to retain it.
 */
void field_set_scene_parameters(s32 scene_id, s32 object_id, u32 spawn_id, s32 music_id, s32 sound_bank_id, s32 secondary_music_id)
{
    g_field_pending_scene_id = scene_id;
    g_field_pending_object_id = object_id;
    g_field_pending_spawn_id = spawn_id;
    g_field_pending_music_id = music_id;
    g_field_scene_request_pending = 1;
    g_field_pending_sound_bank_id = sound_bank_id;
    g_field_pending_secondary_music_id = secondary_music_id;
}

/**
 * @brief Complete a pending scene change and initialize its runtime resources.
 *
 * Reads the scene file, copies its scripts, strings, records and portraits,
 * installs the actor resources, images and action tables, loads the map and
 * the actor layout, switches the music and places the party at the spawn.
 * The transition tiles are re-uploaded between the steps.
 */
void field_update_scene(void)
{
    FieldSceneSpawn default_spawn;
    RECT rect;
    s32 offset[3];
    u8* actor_description_base;
    u8* image_base;
    u8* geometry_base;
    FieldSceneLayout* layout;
    SceneState* scene_state;
    s32 actor_end;
    s32 object_id;
    s32 spawn_id;
    s32 sound_bank_id;
    s32 action_offset;
    u8* scene_file;
    ControllerState* controller;
    FieldSceneSpawn* spawn;
    s32* group_bounds_list;
    u32* group_bounds_source;
    u32* group_bounds_output;
    FieldActor* actor;
    u8* scene_cursor;
    s32 direction_flag;
    s32 scene_id;
    s32 previous_mode;
    s32 saved_scene_id;
    s32 action_index;
    s32 resource_offset;
    s32 image_index;
    s32 image_count;
    s32 i;
    s32 next_i;
    u8* resource_base;
    u8* resource_table;
    u8* action_base;
    FieldDirectionOffset* direction_offsets;
    s32* direction_modes;
    s8 animation;
    s32 top_image_slot;
    FieldActionSlot* action_ids;
    u8* portraits_section;
    u8* event_scripts_section;
    u32 portrait_count;
    u8* strings_section;
    u8* actor_scripts_section;
    u8* records_section;
    u8* actors_section;
    u8* section_source;
    u8* data_output;
    u8* image_cursor;
    FieldResourceEntry* resource_fields;
    FieldActionSlot* action;
    FieldResourceEntry* resource;
    FieldResourceEntry* inherited_resource;
    FieldSceneActorDescription* description;
    FieldActionSlot* action_fields;
    s32* geometry_offsets;
    s32* direction_entry;
    FieldResourceEntry* previous_resource;

    scene_state = SCENE_STATE;
    controller = CONTROLLER_STATE;
    if (g_field_scene_request_pending != 0)
    {
        controller->ports[1].small_motor_command = 0;
        controller->ports[0].small_motor_command = 0;
        controller->ports[1].actuator_control.fields.large_motor_command = 0;
        controller->ports[0].actuator_control.fields.large_motor_command = 0;
        if (g_field_audio_timer != 0)
        {
            fade_out_current_song();
            if (g_field_pending_music_id == FIELD_MUSIC_KEEP)
            {
                field_play_song();
                akao_cmd_c1(0, 1, g_field_song_volume);
            }
            g_field_audio_timer = 0;
        }
        field_prepare_transition_tiles();
        field_upload_transition_tiles();
        scene_id = g_field_pending_scene_id;
        g_field_scene_request_pending = 0;
        object_id = g_field_pending_object_id;
        spawn_id = g_field_pending_spawn_id;
        sound_bank_id = g_field_pending_sound_bank_id;
        DrawSync(0);
        previous_mode = g_field_scene_mode_bit;
        g_field_scene_mode_bit = scene_id & FIELD_SCENE_MODE_FLAG;
        scene_id = scene_id & FIELD_SCENE_ID_MASK;
        if (g_field_pending_music_id != FIELD_MUSIC_KEEP)
        {
            if (g_field_preserve_entry_music == 0)
            {
                akao_cmd_c1(0, FIELD_MUSIC_FADE_FRAMES, 0);
            }
        }
        scene_file = FIELD_SCENE_FILE;
        cdrom_queue_read((u16)(scene_id + CD_RES_FIELD_SCENE_BASE), scene_file);
        i = 0;
        cdrom_wait_queue_empty();
        field_upload_transition_tiles();
        g_scene_mode = scene_id;
        g_field_active_group = 0;
        g_field_duel_mode = 0;
        field_reset_battle_entry();
        field_clear_actor_texts();
        akao_cmd_f1();
        field_initialize_actor_system();
        field_upload_transition_tiles();
        for (; i < FIELD_PARTY_COUNT; i++)
        {
            g_field_actors[i].x = (FIELD_DEFAULT_SPAWN_X + i * FIELD_PARTY_START_SPACING) << FIELD_POSITION_SHIFT;
            g_field_actors[i].y = 0;
            g_field_actors[i].z = FIELD_DEFAULT_SPAWN_Z << FIELD_POSITION_SHIFT;
        }
        if (previous_mode != g_field_scene_mode_bit)
        {
            field_load_instrument_bank(g_field_scene_mode_bit == 0);
        }
        field_upload_transition_tiles();
        group_bounds_output = g_field_group_bounds;
        g_field_dialog_item_count = 0;
        scene_cursor = FIELD_SCENE_SECTION(scene_file, FIELD_SCENE_ACTORS);
        group_bounds_list = (s32*)(FIELD_SCENE_SECTION(scene_file, FIELD_SCENE_GROUP_BOUNDS));
        group_bounds_source = (u32*)(group_bounds_list + 1);
        i = group_bounds_list[0];
        actor_description_base = scene_cursor;
        actors_section = scene_cursor;
        g_field_group_bounds_count = i;
        geometry_base = FIELD_SCENE_SECTION(scene_file, FIELD_SCENE_GEOMETRY);
        image_base = FIELD_SCENE_SECTION(scene_file, FIELD_SCENE_IMAGES);
        layout = (FieldSceneLayout*)(FIELD_SCENE_SECTION(scene_file, FIELD_SCENE_LAYOUT));
        strings_section = FIELD_SCENE_SECTION(scene_file, FIELD_SCENE_STRINGS);
        event_scripts_section = FIELD_SCENE_SECTION(scene_file, FIELD_SCENE_EVENT_SCRIPTS);
        actor_scripts_section = FIELD_SCENE_SECTION(scene_file, FIELD_SCENE_ACTOR_SCRIPTS);
        records_section = FIELD_SCENE_SECTION(scene_file, FIELD_SCENE_RECORDS);
        portraits_section = FIELD_SCENE_SECTION(scene_file, FIELD_SCENE_PORTRAITS);
        while (i != 0)
        {
            *group_bounds_output++ = *group_bounds_source++;
            i--;
        }
        data_output = g_field_scene_data_buffer;
        g_field_scene_strings = g_field_scene_data_buffer;
        section_source = strings_section;
        while (section_source != actor_scripts_section)
        {
            *data_output++ = *section_source++;
        }
        g_field_event_scripts = (u16*)FIELD_ALIGN4(data_output);
        section_source = event_scripts_section;
        while (section_source != strings_section)
        {
            *data_output++ = *section_source++;
        }
        g_field_actor_scripts = (u16*)FIELD_ALIGN4(data_output);
        section_source = actor_scripts_section;
        while (section_source != records_section)
        {
            *data_output++ = *section_source++;
        }
        g_field_scene_record_table = (u32*)FIELD_ALIGN4(data_output);
        section_source = records_section;
        while (section_source != actors_section)
        {
            *data_output++ = *section_source++;
        }
        section_source = portraits_section + 4;
        i = 0;
        portrait_count = *(u32*)portraits_section;
        g_field_scene_portraits = (u8*)FIELD_ALIGN4(data_output);
        g_field_scene_portrait_count = portrait_count;
        while ((u32)i < g_field_scene_portrait_count)
        {
            /* image_count doubles as the byte counter of one portrait. */
            image_count = 0;
            do
            {
                image_count++;
                *data_output++ = *section_source++;
            } while (image_count < FIELD_SCENE_PORTRAIT_SIZE);
            i++;
        }
        g_field_scene_data_size = data_output - g_field_scene_data_buffer;
        g_field_previous_song_volume = g_field_song_volume;
        g_field_song_volume = *(u16*)scene_cursor;
        g_field_party_palette_index = *(u16*)(scene_cursor + 2);
        scene_cursor += 4;
        field_set_party_palettes();
        i = FIELD_PARTY_COUNT;
        field_initialize_actor_parts(*(u16*)scene_cursor >> FIELD_SCENE_PARTS_TIMER_SHIFT);
        g_field_hide_actor_panels = 0;
        field_upload_transition_tiles();
        scene_state->map_id = *(u16*)scene_cursor & FIELD_SCENE_MAP_ID_MASK;
        scene_cursor += 2;
        actor = &g_field_actors[FIELD_PARTY_COUNT];
        actor_end = *(u16*)scene_cursor + FIELD_PARTY_COUNT;
        scene_cursor += 2;
        for (; i < FIELD_ACTOR_COUNT; i++, actor++)
        {
            actor->presence = FIELD_ACTOR_UNUSED;
        }
        top_image_slot = FIELD_FIRST_IMAGE_SLOT;
        D_80115890 = 0;
        action_base = (u8*)g_field_resource_actions;
        field_reset_fallback_resource();
        i = 0;
        if ((actor_end - FIELD_PARTY_COUNT) > 0)
        {
            action_offset = FIELD_PARTY_COUNT * sizeof(FieldActionRow);
            geometry_offsets = (s32*)geometry_base;
            resource_offset = FIELD_PARTY_COUNT * sizeof(FieldResourceEntry);
            do
            {
                image_count = 0;
                description = (FieldSceneActorDescription*)(actor_description_base + *(s32*)scene_cursor);
                image_cursor = description->images;
                resource_table = (u8*)g_field_resource_entries;
                while (*image_cursor != FIELD_ACTOR_IMAGE_END)
                {
                    image_count++;
                    image_cursor++;
                    if (image_count == FIELD_ACTOR_IMAGE_COUNT)
                    {
                        break;
                    }
                }
                top_image_slot += image_count;
                if (top_image_slot >= FIELD_IMAGE_SLOT_SKIP_FIRST && top_image_slot < FIELD_IMAGE_SLOT_SKIP_FIRST + FIELD_IMAGE_SLOT_SKIP_COUNT)
                {
                    top_image_slot += FIELD_IMAGE_SLOT_SKIP_COUNT;
                }
                if (image_count != 0)
                {
                    resource_base = resource_table;
                    resource_fields = OFFSET_FIRST_PTR(FieldResourceEntry, resource_offset, resource_base);
                    resource_fields->palette = description->palette;
                    resource_fields->slot_index = top_image_slot;
                    if (D_80115890 != 0)
                    {
                        resource_fields->unk8 = 0;
                    }
                    else
                    {
                        resource_fields->unk8 = image_count != 1;
                    }
                    D_80115890 = image_count != 1;
                    resource_base = resource_table;
                    resource = OFFSET_FIRST_PTR(FieldResourceEntry, resource_offset, resource_base);
                    resource->start = g_field_resource_cursor;
                    resource->flags = (resource->flags & ~FIELD_RESOURCE_HAS_ACTIONS) | (description->flags >> 7);
                    field_copy_scene_geometry((s32*)(geometry_base + geometry_offsets[0]), (s32*)(geometry_base + geometry_offsets[1]));
                    resource->end = g_field_resource_cursor;
                    resource->flags |= FIELD_RESOURCE_LOADED;
                    action_ids = description->actions;
                    resource->unkE = description->unkA;
                    /* image_index holds the action count until the image upload below. */
                    image_index = description->action_count;
                    action_index = 0;
                    if (image_index > 0)
                    {
                        action_fields = description->actions;
                        do
                        {
                            action = OFFSET_FIRST_PTR(FieldActionSlot, action_offset + action_index * sizeof(FieldActionSlot), action_base);
                            action->flags.instrument = action_fields->flags.instrument;
                            action->command = action_ids->command;
                            action->flags.target_filter = action_fields->flags.target_filter;
                            action_index += 1;
                            action->animation = action_fields->animation;
                            action_ids++;
                            action->parameter = action_fields->parameter;
                            action->flags.target_group = action_fields->flags.target_group;
                            action_fields++;
                        } while (action_index < image_index);
                    }
                    image_index = 0;
                }
                else
                {
                    /* No images of its own: reuse the previous actor's CLUT and texture slot. */
                    rect.y = i + FIELD_ACTOR_CLUT_VRAM_Y + FIELD_PARTY_COUNT - 1;
                    rect.w = FIELD_CLUT_ROW_WIDTH;
                    rect.x = 0;
                    rect.h = 1;
                    MoveImage(&rect, 0, i + FIELD_ACTOR_CLUT_VRAM_Y + FIELD_PARTY_COUNT);
                    /* A byte offset: &g_field_resource_entries[i + 2] folds the + 2 into the base address. */
                    previous_resource = (FieldResourceEntry*)((u8*)g_field_resource_entries + (i + FIELD_PARTY_COUNT - 1) * sizeof(FieldResourceEntry));
                    resource_base = (u8*)g_field_resource_entries;
                    inherited_resource = OFFSET_FIRST_PTR(FieldResourceEntry, resource_offset, resource_base);
                    inherited_resource->palette = previous_resource->palette;
                    inherited_resource->slot_index = previous_resource->slot_index;
                    inherited_resource->unk8 = 0;
                    inherited_resource->start = g_field_resource_cursor;
                    inherited_resource->flags = (inherited_resource->flags & ~FIELD_RESOURCE_HAS_ACTIONS) | (description->flags >> 7);
                    field_copy_scene_geometry((s32*)(geometry_base + geometry_offsets[0]), (s32*)(geometry_base + geometry_offsets[1]));
                    inherited_resource->end = g_field_resource_cursor;
                    inherited_resource->flags |= FIELD_RESOURCE_LOADED;
                    action_ids = description->actions;
                    inherited_resource->unkE = description->unkA;
                    image_index = description->action_count;
                    action_index = 0;
                    if (image_count < image_index)
                    {
                        action_fields = description->actions;
                        do
                        {
                            action = OFFSET_FIRST_PTR(FieldActionSlot, action_offset + action_index * sizeof(FieldActionSlot), action_base);
                            action->flags.instrument = action_fields->flags.instrument;
                            action->command = action_ids->command;
                            action->flags.target_filter = action_fields->flags.target_filter;
                            action_index += 1;
                            action->animation = action_fields->animation;
                            action_ids++;
                            action->parameter = action_fields->parameter;
                            action->flags.target_group = action_fields->flags.target_group;
                            action_fields++;
                        } while (action_index < image_index);
                    }
                    DrawSync(0);
                    image_index = 0;
                }
                image_cursor = description->images;
                while (image_count != 0)
                {
                    field_upload_actor_image((FieldTimData*)(image_base + *(s32*)(image_base + (*image_cursor << 2))), top_image_slot - image_index,
                                             i + FIELD_PARTY_COUNT, image_index == 0);
                    image_cursor++;
                    image_count--;
                    image_index++;
                }
                /* Through a temporary: a plain i++ changes how the loop's offsets are reduced. */
                next_i = i + 1;
                geometry_offsets++;
                resource_offset += sizeof(FieldResourceEntry);
                i = next_i;
                action_offset += sizeof(FieldActionRow);
                scene_cursor += 4;
            } while (i < (actor_end - FIELD_PARTY_COUNT));
        }
        field_upload_transition_tiles();
        field_reset_object_tints();
        field_load_scene_actors(layout);
        if (g_field_pending_music_id != FIELD_MUSIC_KEEP)
        {
            field_capture_card_clock();
            if (g_field_preserve_entry_music == 0)
            {
                field_stop_song();
            }
            if (g_field_pending_music_id != FIELD_MUSIC_FIXED)
            {
                field_load_song(g_field_pending_music_id, 0);
                if ((g_field_preserve_entry_music == 0) && (g_field_pending_music_id != FIELD_MUSIC_FIXED))
                {
                    field_play_song();
                }
            }
            else
            {
                field_load_fixed_song();
            }
            g_layout_flag = g_field_pending_music_id;
        }
        else
        {
            akao_cmd_c1(0, FIELD_MUSIC_FADE_FRAMES, g_field_song_volume);
        }
        if (g_field_pending_secondary_music_id != FIELD_MUSIC_KEEP)
        {
            field_load_song(g_field_pending_secondary_music_id, 1);
            g_layout_sub_mode = g_field_pending_secondary_music_id;
        }
        field_upload_transition_tiles();
        if (g_field_preserve_entry_music != 0)
        {
            g_field_audio_timer = FIELD_ENTRY_MUSIC_TIMER;
        }
        else
        {
            g_field_audio_timer = 0;
        }
        g_field_preserve_entry_music = 0;
        if (g_field_scene_mode_bit != 0)
        {
            field_upload_resource_22_bank();
        }
        field_upload_transition_tiles();
        DrawSync(0);
        VSync(0);
        field_upload_transition_tiles();
        field_load_map(scene_state->map_id);
        field_upload_transition_tiles();
        field_init_ctx(g_field_render_context, object_id);
        field_upload_transition_tiles();
        field_text_reset_windows();
        field_camera_reset();
        if (g_field_scene_mode_bit != 0)
        {
            field_battle_setup(0);
        }
        spawn = field_header_record_at((u16)spawn_id);
        if (spawn == NULL)
        {
            default_spawn.h.z = FIELD_DEFAULT_SPAWN_Z;
            default_spawn.h.x = FIELD_DEFAULT_SPAWN_X;
            default_spawn.h.y = 0;
            default_spawn.bits.facing = 0;
            spawn = &default_spawn;
        }
        actor = g_field_actors;
        i = 0;
        direction_offsets = g_field_direction_offsets;
        direction_modes = g_field_direction_animation_modes;
        for (; i < FIELD_PARTY_COUNT; i++, actor++)
        {
            if (actor->presence != FIELD_ACTOR_UNUSED)
            {
                actor->x = spawn->h.x << FIELD_POSITION_SHIFT;
                actor->y = spawn->h.y << FIELD_POSITION_SHIFT;
                actor->z = spawn->h.z << FIELD_POSITION_SHIFT;
                offset[0] = direction_offsets[FIELD_SPAWN_FACING(spawn)].dx * i;
                offset[1] = 0;
                offset[2] = direction_offsets[FIELD_SPAWN_FACING(spawn)].dz * i;
                field_move_actor_position(actor, offset);
                actor->direction = (FIELD_SPAWN_FACING(spawn)) << 5;
                direction_entry = OFFSET_FIRST_PTR(s32, (FIELD_SPAWN_FACING(spawn)) * sizeof(s32), direction_modes);
                animation = (*direction_entry & ~FIELD_ANIMATION_FACING) % FIELD_DIRECTION_ANIMATION_COUNT;
                direction_flag = *(u8*)direction_entry & FIELD_ANIMATION_FACING;
                animation |= direction_flag;
                actor->animation = animation;
                actor->animation_frame = 0;
                actor->animation_active = 1;
                field_restart_actor_animation(actor);
            }
        }
        field_refresh_actor_collisions();
        field_reset_leader_position_history();
        field_camera_track_party();
        g_field_camera_follow_x = g_field_camera_target_x;
        g_field_camera_follow_z = g_field_camera_target_z;
        field_load_sfx_tables(sound_bank_id);
        g_layout_option = sound_bank_id;
        field_upload_transition_tiles();
        g_field_scene_contact_latched = 0;
        field_initialize_actor_slots();
        field_clear_actor_slots();
        field_reset_draw_state();
        field_reset_actor_resources();
        field_command_history_reset();
        field_clear_fade_prims();
        saved_scene_id = g_scene_mode;
        if (g_field_scene_mode_bit != 0)
        {
            saved_scene_id += FIELD_SCENE_MODE_FLAG;
        }
        field_store_entry_settings(saved_scene_id, object_id, g_layout_flag, spawn_id, g_layout_option, g_layout_sub_mode);
        field_upload_golem_palettes();
        field_refresh_party_routes();
        field_pair_indicators_reset();
        field_reset_item_menu();
        field_upload_transition_tiles();
        field_save_retry_snapshot();
    }
}

/**
 * @brief Reset the fallback actor resource to the built-in geometry.
 */
static void field_reset_fallback_resource(void)
{
    g_field_resource_entries[FIELD_FALLBACK_RESOURCE].slot_index = FIELD_FALLBACK_SLOT;
    g_field_resource_entries[FIELD_FALLBACK_RESOURCE].start = g_field_fallback_geometry;
    g_field_resource_entries[FIELD_FALLBACK_RESOURCE].end = g_field_fallback_geometry + sizeof(g_field_fallback_geometry);
    g_field_resource_entries[FIELD_FALLBACK_RESOURCE].flags &= ~FIELD_RESOURCE_HAS_ACTIONS;
    g_field_resource_entries[FIELD_FALLBACK_RESOURCE].palette = 0;
    g_field_resource_entries[FIELD_FALLBACK_RESOURCE].unk8 = 0;
    g_field_resource_entries[FIELD_FALLBACK_RESOURCE].unkE = 0;
    g_field_resource_entries[FIELD_FALLBACK_RESOURCE].flags |= FIELD_RESOURCE_LOADED;
}

/**
 * @brief Upload an actor TIM image and optionally its palette to field VRAM.
 * @param image Paletted TIM file.
 * @param image_slot Destination VRAM texture slot.
 * @param actor_index Actor whose CLUT row receives the palette.
 * @param upload_palette Nonzero to upload the TIM palette before the image.
 */
static void field_upload_actor_image(FieldTimData* image, s32 image_slot, s32 actor_index, s32 upload_palette)
{
    RECT rect;
    u16 width;
    u16 height;
    s32 size;

    width = image->file.palette.rect.w;
    height = image->file.palette.rect.h;
    size = image->file.palette.size;
    if (upload_palette != 0)
    {
        setRECT(&rect, 0, actor_index + FIELD_ACTOR_CLUT_VRAM_Y, width * height, 1);
        LoadImage(&rect, image->file.palette.pixels);
    }
    image = (FieldTimData*)((u8*)image + size + FIELD_TIM_HEADER_SIZE);
    width = image->block.rect.w;
    height = image->block.rect.h;
    if (image_slot >= FIELD_IMAGE_SLOT_SECOND_ROW)
    {
        setRECT(&rect, FIELD_IMAGE_ROW1_X - ((image_slot - (FIELD_IMAGE_SLOT_SECOND_ROW - 1)) << 6), FIELD_IMAGE_ROW1_Y, width, height);
    }
    else
    {
        setRECT(&rect, FIELD_IMAGE_ROW0_X - (image_slot << 6), 0, width, height);
    }
    LoadImage(&rect, image->block.pixels);
    DrawSync(0);
}

/**
 * @brief Copy a word-aligned byte range to the field resource cursor and advance it.
 * @param src Start of the source range.
 * @param end End of the source range (exclusive, rounded up to a word).
 */
static void field_copy_scene_geometry(s32* src, s32* end)
{
    s32 size;
    s32 words;
    s32* dst;

    size = (u8*)end - (u8*)src + 3;
    words = size >> 2;
    dst = g_field_resource_cursor;
    g_field_resource_cursor = (u8*)dst + (size & ~3);
    while (words != 0)
    {
        *dst = *src;
        dst++;
        src++;
        words--;
    }
}

/**
 * @brief Install the scene's layout actions and place its active actors.
 * @param layout Actor layout of the scene file.
 * @note Active entries fill consecutive actor records from FIELD_PARTY_COUNT on.
 */
static void field_load_scene_actors(FieldSceneLayout* layout)
{
    FieldActor* actor = &g_field_actors[FIELD_PARTY_COUNT];
    FieldObjectState* state = &g_field_object_states[FIELD_PARTY_COUNT];
    FieldActionRequest* entry = layout->requests;
    s32 loaded_count = 0;
    s32 index = 0;
    s32 count = layout->count;
    s32 i;
    u32 value_word;
    u32 base_value;

    g_field_loaded_actor_count = FIELD_PARTY_COUNT;
    for (; index < count; index++, entry++)
    {
        state->unk8.word &= 0x7FFFFFFF;
        field_install_actor_action(entry, index);
        if (entry->control.bits.active)
        {
            field_initialize_actor_record(loaded_count + FIELD_PARTY_COUNT, entry->source.actor + FIELD_PARTY_COUNT);
            if (entry->control.bits.hidden)
            {
                actor->presence = FIELD_ACTOR_HIDDEN;
            }
            else
            {
                actor->presence = 0;
            }
            actor->control.bits.group = entry->control.bits.group;
            if ((g_field_pending_scene_id & FIELD_SCENE_ID_MASK) == FIELD_SCENE_NO_LAYOUT_GROUPS)
            {
                actor->control.bits.group = 0;
            }
            actor->control.bits.palette = entry->control.bits.palette;
            actor->x = entry->position.bits.x << FIELD_POSITION_SHIFT;
            actor->z = entry->position.bits.z << FIELD_POSITION_SHIFT;
            actor->y = entry->position.bits.y << FIELD_POSITION_SHIFT;
            state->contact.bits.flag6 = 0;
            state->contact.bits.flag7 = 0;
            state->movement.word &= ~FIELD_MOVEMENT_TINT_FLASH;
            state->tint_red = g_field_object_parts[actor->object_index].tint_red;
            state->tint_green = g_field_object_parts[actor->object_index].tint_green;
            state->tint_blue = g_field_object_parts[actor->object_index].tint_blue;
            state->tint_timer = 0;
            state->interaction_kind = 0;
            value_word = state->unk8.word & 0x80FFFFFF;
            state->contact.bits.flag5 = 0;
            state->unk8.word = value_word;
            state->unk8.word = (value_word & 0xFF000000) | (state->unk0 & 0xFFFFFF);
            base_value = state->unk0;
            state->key = index + FIELD_PARTY_COUNT;
            state->unk4.word = base_value & 0xFFFFFF;
            state->enabled_events = entry->enabled_events;
            state->group_flags = entry->control.flags;
            for (i = 0; i < FIELD_ACTION_SCRIPT_COUNT; i++)
            {
                state->scripts[i] = entry->scripts[i];
            }
            field_restart_actor_animation(actor);
            state++;
            actor++;
            loaded_count++;
            g_field_loaded_actor_count++;
        }
    }
}

/**
 * @brief Settle every present field actor on the map collision.
 * @note Actors outside the map bounds get no contact instead.
 */
static void field_refresh_actor_collisions(void)
{
    FieldActor* actor;
    FieldObjectState* state;
    FieldMapBounds* bounds = FIELD_MAP_BOUNDS;
    struct FieldCollisionMover* mover = FIELD_COLLISION_MOVER;
    s32 i;

    actor = g_field_actors;
    state = g_field_object_states;
    for (i = 0; i < FIELD_ACTOR_COUNT; i++, state++, actor++)
    {
        if (actor->presence != FIELD_ACTOR_UNUSED)
        {
            if (actor->x >= 0 && actor->x < FIELD_MAP_X_LIMIT(bounds) && actor->z >= 0 && actor->z < FIELD_MAP_Z_LIMIT(bounds))
            {
                mover->x = actor->x;
                mover->height = actor->y;
                mover->z = actor->z;
                mover->footprint_width = FIELD_FOOTPRINT_LARGE_WIDTH;
                mover->mode.bits.footprint_depth = FIELD_FOOTPRINT_LARGE_DEPTH;
                mover->move_x = 0;
                mover->move_height = 0;
                mover->move_z = 0;
                mover->height_bias = FIELD_FOOTPRINT_STEP;
                mover->collision_node = -1;
                mover->flags = 0;
                mover->mode.bits.airborne_high = 0;
                mover->mode.bits.airborne_low = 0;
                field_collision_move_mover(mover);
                state->collision_node = mover->collision_node;
                state->collision_flags = mover->flags;
                state->movement.half.hi = mover->resolved_height >> 8;
                actor->x = mover->x;
                actor->z = mover->z;
                actor->y = mover->height;
            }
            else
            {
                state->movement.half.hi = 0;
                state->collision_node = -1;
                state->collision_flags = 0;
            }
        }
    }
}

/**
 * @brief Apply collision-constrained motion to an actor position.
 * @param actor Actor to move; outside the map bounds it stays where it is.
 * @param motion X, Y and Z motion in fixed-point units.
 */
void field_move_actor_position(FieldActor* actor, s32* motion)
{
    FieldMapBounds* bounds = FIELD_MAP_BOUNDS;
    struct FieldCollisionMover* mover = FIELD_COLLISION_MOVER;
    s32 position_z;
    s32 position_x;

    position_x = actor->x;
    if (position_x < 0)
    {
        return;
    }
    if (position_x >= FIELD_MAP_X_LIMIT(bounds))
    {
        return;
    }

    position_z = actor->z;
    if (position_z < 0)
    {
        return;
    }
    if (position_z >= FIELD_MAP_Z_LIMIT(bounds))
    {
        return;
    }

    mover->x = position_x;
    mover->height = actor->y;
    mover->z = actor->z;
    mover->move_x = motion[0];
    mover->move_height = motion[1];
    mover->move_z = motion[2];

    if (g_field_object_parts[0].scale_z >= FIELD_PART_FULL_SCALE)
    {
        mover->footprint_width = FIELD_FOOTPRINT_LARGE_WIDTH;
        mover->mode.bits.footprint_depth = FIELD_FOOTPRINT_LARGE_DEPTH;
    }
    else
    {
        mover->footprint_width = FIELD_FOOTPRINT_WIDTH;
        mover->mode.bits.footprint_depth = FIELD_FOOTPRINT_DEPTH;
    }

    mover->height_bias = FIELD_FOOTPRINT_STEP;
    mover->collision_node = -1;
    mover->flags = 0;
    mover->mode.bits.airborne_high = 0;
    mover->mode.bits.airborne_low = 0;

    field_collision_move_mover(mover);

    actor->x = mover->x;
    actor->z = mover->z;
    actor->y = mover->height;
}

/**
 * @brief Set the CLUT rows of the two player resources from the scene's party palette.
 */
void field_set_party_palettes(void)
{
    s32 i;

    if (g_field_party_palette_index >= FIELD_PARTY_PALETTE_COUNT)
    {
        g_field_party_palette_index = FIELD_PARTY_PALETTE_COUNT - 1;
    }
    for (i = 0; i < FIELD_PLAYER_COUNT; i++)
    {
        if (g_field_player_records[i].character_kind == FIELD_PLAYER_KIND_HERO)
        {
            g_field_resource_entries[i].palette = g_field_party_palettes[g_field_party_palette_index];
        }
        else
        {
            g_field_resource_entries[i].palette = 0;
        }
    }
}

/**
 * @brief Upload the two field transition tiles to VRAM and flip the source bank.
 */
static void field_upload_transition_tiles(void)
{
    RECT rect;
    u16* src;

    setRECT(&rect, FIELD_TRANSITION_TILE_X, FIELD_TRANSITION_TILE_Y0, FIELD_TRANSITION_TILE_SIZE, FIELD_TRANSITION_TILE_SIZE);
    if (g_field_transition_tile_bank != 0)
    {
        src = g_field_transition_tiles_alt;
    }
    else
    {
        src = g_field_transition_tiles;
    }
    LoadImage(&rect, (u_long*)src);

    setRECT(&rect, FIELD_TRANSITION_TILE_X, FIELD_TRANSITION_TILE_Y1, FIELD_TRANSITION_TILE_SIZE, FIELD_TRANSITION_TILE_SIZE);
    if (g_field_transition_tile_bank != 0)
    {
        src = g_field_transition_tiles_alt;
    }
    else
    {
        src = g_field_transition_tiles;
    }
    LoadImage(&rect, (u_long*)src);

    DrawSync(0);
    g_field_transition_tile_bank = g_field_transition_tile_bank == 0;
}

/**
 * @brief Recolor the transition tile pixels for a white or a black fade target.
 */
static void field_prepare_transition_tiles(void)
{
    s32 i;
    u16* p;

    g_field_transition_tile_bank = 0;
    if (g_field_fade_target.red == FIELD_FADE_WHITE && g_field_fade_target.green == g_field_fade_target.red &&
        g_field_fade_target.blue == g_field_fade_target.green)
    {
        p = g_field_transition_tiles;
        for (i = 0; i < FIELD_TRANSITION_TILE_PIXELS; i++, p++)
        {
            if ((*p & FIELD_PIXEL_COLOR_MASK) == FIELD_PIXEL_TRANSPARENT)
            {
                *p = FIELD_PIXEL_WHITE;
            }
        }
    }
    else
    {
        p = g_field_transition_tiles;
        for (i = 0; i < FIELD_TRANSITION_TILE_PIXELS; i++, p++)
        {
            if (*p == FIELD_PIXEL_WHITE)
            {
                *p = FIELD_PIXEL_TRANSPARENT;
            }
        }
    }
}
