#include "field_scene_transition.h"
#include "field_text.h"
#include "cdrom.h"
#include "cd_resources.h"
#include "game_audio.h"
#include "common.h"
#include "field_interaction_start.h"
#include "sdk/libgpu.h"
#include "sdk/libetc.h"
#include "sdk/libgte.h"

#define U8_AT(p, o) (*(u8*)((u8*)(p) + (o)))
#define U16_AT(p, o) (*(u16*)((u8*)(p) + (o)))
#define S16_AT(p, o) (*(s16*)((u8*)(p) + (o)))
#define S32_AT(p, o) (*(s32*)((u8*)(p) + (o)))
#define U32_AT(p, o) (*(u32*)((u8*)(p) + (o)))

/** @brief Geometry span, image allocation, and flags for an actor resource. */
typedef struct
{
    s32 geometry_start;
    s32 geometry_end;
    u8 multiple_images;
    u8 image_page;
    u16 palette;
    u16 unknown_0x0c;
    u16 unknown_0x0e;
    s32 flags;
} FieldSceneResource;

/** @brief Eight-byte action descriptor copied from an actor description. */
typedef struct
{
    u16 id;
    union
    {
        u16 word;
        struct
        {
            unsigned short low : 8;
            unsigned short mode : 2;
            unsigned short special : 1;
            unsigned short unused : 5;
        } bits;
        struct
        {
            u8 low;
            u8 high;
        } bytes;
    } flags;
    u16 animation;
    u16 requirement;
} FieldSceneAction;

/** @brief Actor image list and variable-length action records in a scene file. */
typedef struct
{
    u8 unknown_0x00;
    u8 flags;
    u8 images[6];
    u16 palette;
    u16 unknown_0x0a;
    s32 action_count;
    FieldSceneAction actions[1];
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

/** @brief Position prefix of the actor record passed to the mover. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
} MoverPosition;

/** @brief Map bounds stored in the field scratch area. */
typedef struct
{
    s16 width;
    s16 height;
    s16 unknown_0x04;
    u8 _pad6[0xC - 6];
    s16 unknown_0x0c;
} MoverBounds;

/** @brief Scratchpad position, motion, and collision resolver state. */
typedef struct
{
    s32 position[3];
    s32 motion[3];
    s32 height;
    s32 contact;
    s32 surface;
    s16 radius;
    s16 depth;
    union
    {
        s32 word;
        struct
        {
            s16 status;
            s16 flags;
        } halves;
    } mode;
} ScratchMover;

/** @brief Default scene spawn position and packed facing selector. */
typedef union
{
    /** @brief Signed map coordinates and facing/flag bytes. */
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
        u32 direction : 4;
        u32 unused : 12;
    } bits;
} FieldSceneSpawn;

/** @brief Controller actuator bytes reset when changing scenes. */
typedef struct
{
    u8 _pad0[0x91];
    u8 port1_a;
    u8 port1_b;
    u8 _pad93[0x13F - 0x93];
    u8 port2_a;
    u8 port2_b;
} FieldTransitionController;

/** @brief Actor position prefix with the original 0x54-byte record stride. */
typedef struct
{
    s32 x, y, z;
    u8 _pad0c[15];
    u8 facing;
    u8 _pad1c[5];
    u8 animation_mode;
    u8 _pad22[2];
    u8 active;
    u8 presence;
    u8 unknown_0x26;
    u8 animation_timer;
    u8 _pad28[44];
} FieldSceneActorPosition;

/** @brief Fallback actor resource view. */
typedef struct
{
    u8 _pad0[0xA0];
    u8* geometry_start;
    u8* geometry_end;
    u8 multiple_images;
    u8 image_page;
    s16 palette;
    u8 _padac[2];
    s16 unknown_0x0e;
    s32 flags;
} FieldFallbackResourceTable;

/** @brief Position, flags, and visual slot in a 0x54-byte actor record. */
typedef struct
{
    s32 x, y, z;
    u8 _padc[0x10];
    union
    {
        u32 word;
        struct
        {
            unsigned low : 16;
            unsigned group : 2;
            unsigned _pad18 : 1;
            unsigned render : 4;
            unsigned high : 9;
        } bits;
    } mode;
    u8 _pad20[5];
    u8 presence;
    u8 _pad26[0x14];
    u8 slot;
    u8 tail[0x19];
} FieldLoadedActor;

/** @brief Packed links, parameters, and state in a 0x23C-byte actor slot. */
typedef struct
{
    u32 base;
    u32 link;
    u32 tag;
    u32 _padc;
    u32 flags;
    s32 index;
    u16 id;
    u16 params[16];
    u8 _pad3a[0x13A];
    u32 options;
    union
    {
        u32 word;
        struct
        {
            unsigned low : 5;
            unsigned bit5 : 1;
            unsigned bit6 : 1;
            unsigned bit7 : 1;
            unsigned high : 24;
        } bits;
    } state;
    u8 _pad17c[0x12];
    u8 unknown18e;
    u8 _pad18f[0x19];
    u8 red, green, blue, alpha;
    u8 tail[0x90];
} FieldLoadedActorSlot;

/** @brief Color bytes in a 0x48-byte field visual record. */
typedef struct
{
    u8 _pade[0xE];
    u8 red, green, blue;
    u8 tail[0x37];
} FieldLoadedActorVisual;

/** @brief Position and presence fields of a 0x54-byte field actor. */
typedef struct
{
    s32 x, y, z;
    u8 pad_c[0x19];
    u8 presence;
    u8 _pad26[0x2E];
} FieldCollisionActor;

/** @brief Collision result fields in a 0x23C-byte actor slot. */
typedef struct
{
    u8 pad[0x176];
    s16 height;
    u8 _pad178[0x24];
    s32 contact, surface;
    u8 _pad1a4[0x98];
} FieldCollisionSlot;

/** @brief Scratchpad collision request and resolved position. */
typedef struct
{
    s32 x, y, z, dx, dy, dz, height, contact, surface;
    s16 radius, depth;
    union
    {
        s32 flags;
        struct
        {
            unsigned step : 16;
            unsigned bit16 : 1;
            unsigned bit17 : 1;
            unsigned high : 14;
        } bits;
    } mode;
} FieldCollisionRequest;

/** @brief Map dimensions used to validate actor coordinates. */
typedef struct
{
    s16 width;
    u16 height;
} FieldCollisionBounds;

/** @brief Palette field in a field resource record. */
typedef struct
{
    u8 _pad0[0xA];
    u16 palette;
    u8 _padc[0x14 - 0xC];
} FieldPaletteResource;

/** @brief Party resource selector. */
typedef struct
{
    u8 _pad0[3];
    u8 resource_kind;
    u8 _pad4[0x268 - 4];
} FieldPartyResource;

/** @brief Transition fade target color and duration. */
typedef struct
{
    s16 red;
    s16 green;
    s16 blue;
    s16 duration;
} FieldTransitionFade;

static void field_reset_fallback_resource(void);
static void field_upload_actor_image(void* image_data, s32 image_page, s32 actor_slot, s32 upload_palette);
static void field_copy_scene_geometry(s32* src, s32* end);
static void field_load_scene_actors(s32* data);
static void field_refresh_actor_collisions(void);
static void field_upload_transition_tiles(void);
static void field_prepare_transition_tiles(void);

/**
 * @brief Queue a seek to the scene selected by the low fifteen selector bits.
 * @param scene_selector Scene resource selector, optionally carrying the high-bit mode flag.
 * @see decomp.me (100.00%)
 */
void field_seek_scene_resource(s32 scene_selector)
{

    cdrom_queue_seek((scene_selector & 0x7FFF) + CD_RES_FIELD_SCENE_BASE);
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
    extern s32 g_field_pending_spawn_id;
    extern s32 g_field_pending_music_id;
    extern s32 g_field_pending_secondary_music_id;
    extern s32 g_field_pending_scene_id;
    extern s32 g_field_pending_object_id;
    extern s32 g_field_pending_sound_bank_id;
    extern s32 g_field_scene_request_pending;

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
 * @note Coordinates disc reads, audio transitions, resource installation, map state, and actor placement.
 * @note The scene-file header at 0x80190000 stores byte offsets relative to that buffer.
 * @see decomp.me (100%)
 */
void field_update_scene(void)
{

    void akao_cmd_c1();
    void akao_cmd_f1();

    void field_clear_actor_slots();
    void field_init_ctx();
    void field_initialize_actor_slots();
    void field_initialize_actor_system();
    void field_load_map();
    void field_stop_song();

    u8* func_800630BC(u16);
    void func_80067AA4();
    void field_initialize_actor_parts();
    void field_restart_actor_animation();
    void func_80084240();
    void func_80084630();
    void func_80086F20();
    void func_8008C730();
    void func_8008C7A8();
    void func_80091BC8();
    void func_80092394();
    void func_800A255C();
    void func_800A2DFC();
    void func_800A35F4();
    void func_800A3654();
    void func_800A368C();
    void func_800A3728();
    void func_800A380C();
    void func_800A3BE8();
    void func_800A54D0();
    void func_800A6204();
    void func_800A6EEC();
    void func_800A8D10();
    void func_800AF8C4();
    void func_800B0094(void);
    void func_800B01FC();
    void func_800B34D0();
    extern s32 g_field_scene_data_size;
    extern u8 g_field_direction_animation_modes[];
    extern u8 g_field_direction_offsets[];
    extern s32 g_field_render_context;
    extern u8 g_field_actors[];
    extern u8 g_field_scene_actors[];
    extern s32 g_field_active_group;
    extern u8 g_field_group_bounds[];
    extern s32 g_field_group_bounds_count;
    extern s32 g_field_event_scripts;
    extern s32 g_field_actor_scripts;
    extern s32 g_field_scene_record_table;
    extern u8 g_field_resource_actions[];
    extern s32 g_field_hide_actor_panels;
    extern s32 g_field_camera_target_x;
    extern s32 g_field_camera_target_z;
    extern s32 g_field_camera_follow_x;
    extern s32 g_field_camera_follow_z;
    /* TODO: Script command 0x44 selects alternate party control and triple HP. */
    extern s32 D_8010D020;
    extern s32 g_field_scene_contact_latched;
    extern u8 g_field_scene_data_buffer[];
    extern s32 g_field_pending_spawn_id;
    extern s32 g_field_song_volume;
    /* TODO: Multiple-image resource flag also raises ability growth from 1 to 4. */
    extern s32 D_80115890;
    extern s32 g_field_preserve_entry_music;
    extern s32 g_field_pending_music_id;
    extern s32 g_field_pending_secondary_music_id;
    extern s32 g_field_scene_mode_bit;
    extern s32 g_field_previous_song_volume;
    extern s32 g_field_pending_scene_id;
    extern s32 g_field_party_palette_index;
    extern s32 g_field_pending_object_id;
    extern s32 g_field_pending_sound_bank_id;
    extern u32 g_field_scene_portrait_count;
    extern s32 g_field_scene_portraits;
    extern u8* g_field_scene_strings;
    extern s32 g_field_dialog_item_count;

    extern s32 g_field_audio_timer;
    extern s32 g_field_resource_cursor;
    extern u8 g_field_resource_entries[];
    extern s32 g_field_scene_request_pending;
    extern s32 g_layout_flag;
    extern s32 g_layout_option;
    extern s32 g_layout_sub_mode;
    extern s32 g_scene_mode;

    FieldSceneSpawn spawn;
    RECT rect;
    VECTOR offset;
    u8* actor_description_base;
    u8* image_base;
    u8* geometry_base;
    u8* layout_base;
    u16* persistent_map;
    s32 actor_end;
    s32 object_id;
    s32 spawn_id;
    s32 sound_bank_id;
    s32 action_offset;
    u8* scene_buffer;
    FieldTransitionController* controller;
    FieldSceneSpawn* spawn_record;
    u8* list_header;
    u8* direction_entry;
    u8* list_source;
    u8* list_output;
    FieldSceneActorPosition* actor_flags;
    FieldSceneActorPosition* actor_position;
    u8* scene_cursor;
    FieldSceneActorPosition* initial_position;
    s32 direction_flag;
    s32 first_image;
    s32 scene_id;
    s32 previous_mode;
    s32 resource_id;
    s32 saved_scene_id;
    s32 image_palette;
    s32 action_index;
    s32 resource_offset;
    s32 image_index;
    s32 image_count;
    s32 actor_index;
    s32 next_actor_index;
    u8* resource_base;
    u8* resource_table;
    u8* action_base;
    u8* spacing_base;
    u8* direction_base;
    s8 actor_mode;
    s32 palette_index;
    FieldSceneAction* action_ids;
    u8* section_20;
    u8* section_04;
    u32 portrait_count;
    u8* section_08;
    u8* section_0c;
    u8* section_10;
    u8* section_end;
    u8* section_source;
    u8* resource_output;
    u8* image_cursor;
    u8 section_byte;
    u8 second_section_byte;
    u8 third_section_byte;
    u8 fourth_section_byte;
    u8 portrait_byte;
    FieldSceneResource* resource_fields;
    FieldSceneAction* action;
    FieldSceneResource* resource;
    FieldSceneResource* inherited_resource;
    FieldSceneActorDescription* description;
    FieldSceneResource* previous_resource;
    FieldSceneAction* action_fields;
    s32* geometry_offsets;

    persistent_map = (u16*)0x801ED480;
    controller = (FieldTransitionController*)0x801ED600;
    if (g_field_scene_request_pending != 0)
    {
        controller->port2_a = 0;
        controller->port1_a = 0;
        controller->port2_b = 0;
        controller->port1_b = 0;
        if (g_field_audio_timer != 0)
        {
            fade_out_current_song();
            if (g_field_pending_music_id == -1)
            {
                func_800A380C();
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
        g_field_scene_mode_bit = scene_id & 0x8000;
        scene_id = scene_id & 0x7FFF;
        if (g_field_pending_music_id != -1)
        {
            if (g_field_preserve_entry_music == 0)
            {
                akao_cmd_c1(0, 0x3C, 0);
            }
        }
        scene_buffer = (u8*)0x80190000;
        cdrom_queue_read((scene_id + CD_RES_FIELD_SCENE_BASE) & 0xFFFF, scene_buffer);
        actor_index = 0;
        cdrom_wait_queue_empty();
        field_upload_transition_tiles();
        g_scene_mode = scene_id;
        g_field_active_group = 0;
        D_8010D020 = 0;
        func_800B01FC();
        func_800A6204();
        akao_cmd_f1();
        field_initialize_actor_system();
        field_upload_transition_tiles();
        initial_position = (FieldSceneActorPosition*)g_field_actors;
        do
        {
            initial_position->x = 0xA000 + (actor_index << 8) * 40;
            initial_position->y = 0;
            initial_position->z = 0xE000;
            actor_index += 1;
            initial_position = (FieldSceneActorPosition*)g_field_actors + actor_index;
        } while (actor_index < 3);
        if (previous_mode != g_field_scene_mode_bit)
        {
            func_800A35F4(g_field_scene_mode_bit == 0);
        }
        field_upload_transition_tiles();
        list_output = g_field_group_bounds;
        g_field_dialog_item_count = 0;
        scene_cursor = scene_buffer + S32_AT(scene_buffer, 0x14);
        list_header = scene_buffer + S32_AT(scene_buffer, 0x24);
        list_source = list_header + 4;
        actor_index = S32_AT(list_header, 0);
        actor_description_base = scene_cursor;
        section_end = scene_cursor;
        g_field_group_bounds_count = actor_index;
        geometry_base = scene_buffer + S32_AT(scene_buffer, 0x18);
        image_base = scene_buffer + S32_AT(scene_buffer, 0x1C);
        layout_base = scene_buffer + S32_AT(scene_buffer, 0x00);
        section_08 = scene_buffer + S32_AT(scene_buffer, 0x08);
        section_04 = scene_buffer + S32_AT(scene_buffer, 0x04);
        section_0c = scene_buffer + S32_AT(scene_buffer, 0x0C);
        section_10 = scene_buffer + S32_AT(scene_buffer, 0x10);
        section_20 = scene_buffer + S32_AT(scene_buffer, 0x20);
        if (actor_index != 0)
        {
            do
            {
                resource_id = S32_AT(list_source, 0);
                list_source += 4;
                actor_index -= 1;
                S32_AT(list_output, 0) = resource_id;
                list_output += 4;
            } while (actor_index != 0);
        }
        resource_output = g_field_scene_data_buffer;
        g_field_scene_strings = g_field_scene_data_buffer;
        section_source = section_08;
        if (section_source != section_0c)
        {
            do
            {
                section_byte = *section_source;
                section_source += 1;
                *resource_output = section_byte;
                resource_output += 1;
            } while (section_source != section_0c);
        }
        g_field_event_scripts = (s32)(resource_output + 3) & ~3;
        section_source = section_04;
        if (section_source != section_08)
        {
            do
            {
                second_section_byte = *section_source;
                section_source += 1;
                *resource_output = second_section_byte;
                resource_output += 1;
            } while (section_source != section_08);
        }
        g_field_actor_scripts = (s32)(resource_output + 3) & ~3;
        section_source = section_0c;
        if (section_source != section_10)
        {
            do
            {
                third_section_byte = *section_source;
                section_source += 1;
                *resource_output = third_section_byte;
                resource_output += 1;
            } while (section_source != section_10);
        }
        g_field_scene_record_table = (s32)(resource_output + 3) & ~3;
        section_source = section_10;
        if (section_source != section_end)
        {
            do
            {
                fourth_section_byte = *section_source;
                section_source += 1;
                *resource_output = fourth_section_byte;
                resource_output += 1;
            } while (section_source != section_end);
        }
        section_source = section_20 + 4;
        actor_index = 0;
        portrait_count = U32_AT(section_20, 0);
        g_field_scene_portraits = (s32)(resource_output + 3) & ~3;
        g_field_scene_portrait_count = portrait_count;
        if (portrait_count != 0)
        {
            do
            {
                image_count = 0;
                do
                {
                    image_count += 1;
                    portrait_byte = *section_source;
                    section_source += 1;
                    *resource_output = portrait_byte;
                    resource_output += 1;
                } while (image_count < 0x4A0);
                actor_index += 1;
            } while ((u32)actor_index < (u32)g_field_scene_portrait_count);
        }
        g_field_scene_data_size = resource_output - g_field_scene_data_buffer;
        g_field_previous_song_volume = g_field_song_volume;
        g_field_song_volume = (s32)U16_AT(scene_cursor, 0x0);
        g_field_party_palette_index = (s32)U16_AT(scene_cursor, 0x2);
        scene_cursor += 4;
        field_set_party_palettes();
        actor_index = 3;
        field_initialize_actor_parts((u16)U16_AT(scene_cursor, 0) >> 0xF);
        g_field_hide_actor_panels = 0;
        field_upload_transition_tiles();
        *persistent_map = U16_AT(scene_cursor, 0) & 0x7FFF;
        scene_cursor += 2;
        actor_position = (FieldSceneActorPosition*)g_field_scene_actors;
        actor_end = U16_AT(scene_cursor, 0) + 3;
        scene_cursor = scene_cursor + 2;
        do
        {
            actor_position->presence = 0xFF;
            actor_index += 1;
            actor_position++;
        } while (actor_index < 0xD);
        palette_index = 2;
        D_80115890 = 0;
        action_base = g_field_resource_actions;
        field_reset_fallback_resource();
        actor_index = 0;
        if ((actor_end - 3) > 0)
        {
            action_offset = 0x4B0;
            geometry_offsets = (s32*)geometry_base;
            resource_offset = 0x3C;
            do
            {
                image_count = 0;
                description = (FieldSceneActorDescription*)(actor_description_base + S32_AT(scene_cursor, 0));
                image_cursor = description->images;
                resource_table = g_field_resource_entries;
                while (*image_cursor != 0xFF)
                {
                    image_count++;
                    image_cursor++;
                    if (image_count == 6)
                    {
                        break;
                    }
                }
                palette_index += image_count;
                if ((u32)(palette_index - 6) < 4U)
                {
                    palette_index += 4;
                }
                if (image_count != 0)
                {
                    resource_base = resource_table;
                    resource_fields = (FieldSceneResource*)(resource_offset + (s32)resource_base);
                    resource_fields->palette = (u16)description->palette;
                    resource_fields->image_page = palette_index;
                    if (D_80115890 != 0)
                    {
                        resource_fields->multiple_images = 0;
                    }
                    else
                    {
                        resource_fields->multiple_images = (s8)(image_count != 1);
                    }
                    D_80115890 = image_count != 1;
                    resource_base = resource_table;
                    resource = (FieldSceneResource*)(resource_offset + (s32)resource_base);
                    resource->geometry_start = (s32)g_field_resource_cursor;
                    resource->flags = (s32)((resource->flags & ~1) | ((u8)description->flags >> 7));
                    field_copy_scene_geometry((s32*)(geometry_base + geometry_offsets[0]), (s32*)(geometry_base + geometry_offsets[1]));
                    resource->geometry_end = (s32)g_field_resource_cursor;
                    resource->flags = (s32)(resource->flags | 2);
                    action_ids = description->actions;
                    resource->unknown_0x0e = (u16)description->unknown_0x0a;
                    image_index = description->action_count;
                    action_index = 0;
                    if (image_index > 0)
                    {
                        action_fields = description->actions;

                        do
                        {
                            action = (FieldSceneAction*)(action_offset + action_index * sizeof(FieldSceneAction) + (s32)action_base);
                            action->flags.bits.special = action_fields->flags.bits.special;
                            action->id = (u16)action_ids->id;

                            action->flags.bytes.low = (u8)action_fields->flags.word;
                            action_index += 1;
                            action->animation = (u16)action_fields->animation;
                            action_ids++;
                            action->requirement = (u16)action_fields->requirement;
                            action->flags.bits.mode = action_fields->flags.bits.mode;
                            action_fields++;
                        } while (action_index < image_index);
                        image_index = 0;
                    }
                    else
                    {
                        image_index = 0;
                    }
                }
                else
                {
                    rect.y = actor_index + 0x1F6;
                    rect.w = 0x100;
                    rect.x = 0;
                    rect.h = 1;
                    MoveImage(&rect, 0, actor_index + 0x1F7);
                    previous_resource = (FieldSceneResource*)(((actor_index + 2) * 0x14) + g_field_resource_entries);
                    resource_base = g_field_resource_entries;
                    inherited_resource = (FieldSceneResource*)(resource_offset + (s32)resource_base);
                    inherited_resource->palette = (u16)previous_resource->palette;
                    inherited_resource->image_page = (u8)previous_resource->image_page;
                    inherited_resource->multiple_images = 0;
                    inherited_resource->geometry_start = (s32)g_field_resource_cursor;
                    inherited_resource->flags = (s32)((inherited_resource->flags & ~1) | ((u8)description->flags >> 7));
                    field_copy_scene_geometry((s32*)(geometry_base + geometry_offsets[0]), (s32*)(geometry_base + geometry_offsets[1]));
                    inherited_resource->geometry_end = (s32)g_field_resource_cursor;
                    inherited_resource->flags = (s32)(inherited_resource->flags | 2);
                    action_ids = description->actions;
                    inherited_resource->unknown_0x0e = (u16)description->unknown_0x0a;
                    image_index = description->action_count;
                    action_index = 0;
                    if (image_count < image_index)
                    {
                        action_fields = description->actions;

                        do
                        {
                            action = (FieldSceneAction*)(action_offset + action_index * sizeof(FieldSceneAction) + (s32)action_base);
                            action->flags.bits.special = action_fields->flags.bits.special;
                            action->id = (u16)action_ids->id;

                            action->flags.bytes.low = (u8)action_fields->flags.word;
                            action_index += 1;
                            action->animation = (u16)action_fields->animation;
                            action_ids++;
                            action->requirement = (u16)action_fields->requirement;
                            action->flags.bits.mode = action_fields->flags.bits.mode;
                            action_fields++;
                        } while (action_index < image_index);
                    }
                    DrawSync(0);
                    image_index = 0;
                }
                image_cursor = description->images;
                if (image_count != 0)
                {
                    image_palette = palette_index - image_index;
                    do
                    {
                        first_image = image_index == 0;
                        field_upload_actor_image(image_base + *(s32*)((s32)image_base + (*image_cursor << 2)), image_palette, actor_index + 3, first_image);
                        image_cursor++;
                        image_count--;
                        image_index++;
                        image_palette = palette_index - image_index;
                    } while (image_count != 0);
                }
                next_actor_index = actor_index + 1;
                geometry_offsets++;
                resource_offset += 0x14;
                actor_index = next_actor_index;
                action_offset += 0x190;
                scene_cursor += 4;
            } while (actor_index < (actor_end - 3));
        }
        field_upload_transition_tiles();
        func_80084630();
        field_load_scene_actors((s32*)layout_base);
        if (g_field_pending_music_id != -1)
        {
            func_800B0094();
            if (g_field_preserve_entry_music == 0)
            {
                field_stop_song();
            }
            if (g_field_pending_music_id != -2)
            {
                func_800A368C(g_field_pending_music_id, 0);
                if ((g_field_preserve_entry_music == 0) && (g_field_pending_music_id != -2))
                {
                    func_800A380C();
                }
            }
            else
            {
                func_800A3728();
            }
            g_layout_flag = g_field_pending_music_id;
        }
        else
        {
            akao_cmd_c1(0, 0x3C, g_field_song_volume);
        }
        if (g_field_pending_secondary_music_id != -1)
        {
            func_800A368C(g_field_pending_secondary_music_id, 1);
            g_layout_sub_mode = g_field_pending_secondary_music_id;
        }
        field_upload_transition_tiles();
        if (g_field_preserve_entry_music != 0)
        {
            g_field_audio_timer = 0xF;
        }
        else
        {
            g_field_audio_timer = 0;
        }
        g_field_preserve_entry_music = 0;
        if (g_field_scene_mode_bit != 0)
        {
            func_800A3654();
        }
        field_upload_transition_tiles();
        DrawSync(0);
        VSync(0);
        field_upload_transition_tiles();
        field_load_map(*persistent_map);
        field_upload_transition_tiles();
        field_init_ctx(g_field_render_context, (u16)object_id);
        field_upload_transition_tiles();
        field_text_reset_windows();
        func_80092394();
        if (g_field_scene_mode_bit != 0)
        {
            func_800B34D0(0);
        }
        spawn_record = (FieldSceneSpawn*)func_800630BC((u16)spawn_id);
        if (spawn_record == NULL)
        {
            spawn.h.z = 0xE0;
            spawn.h.x = 0xA0;
            spawn.h.y = 0;
            spawn.bits.direction = 0;
            spawn_record = &spawn;
        }
        actor_position = (FieldSceneActorPosition*)g_field_actors;
        actor_index = 0;
        spacing_base = g_field_direction_offsets;
        direction_base = g_field_direction_animation_modes;
        do
        {
            actor_flags = actor_position;
            if (actor_flags->presence != 0xFF)
            {
                actor_position->x = spawn_record->h.x << 8;
                actor_flags->y = (s32)(spawn_record->h.y << 8);
                actor_flags->z = (s32)(spawn_record->h.z << 8);
                offset.vx = S16_AT(spacing_base, (spawn_record->h.facing & 0xF) * 4) * actor_index;
                offset.vy = 0;
                offset.vz = S16_AT(spacing_base, (spawn_record->h.facing & 0xF) * 4 + 2) * actor_index;
                field_move_actor_position(actor_position, &offset.vx);
                actor_flags->facing = (s8)((spawn_record->h.facing & 0xF) << 5);
                direction_entry = (u8*)(((spawn_record->h.facing & 0xF) * 4) + (s32)direction_base);
                actor_mode = (S32_AT(direction_entry, 0) & ~0x80) % 5;
                direction_flag = U8_AT(direction_entry, 0) & 0x80;
                actor_mode |= direction_flag;
                actor_flags->animation_mode = actor_mode;
                actor_flags->animation_timer = 0;
                actor_flags->active = 1;
                field_restart_actor_animation(actor_position);
            }
            actor_index += 1;
            actor_position++;
        } while (actor_index < 3);
        field_refresh_actor_collisions();
        func_8008C730();
        func_80091BC8();
        g_field_camera_follow_x = g_field_camera_target_x;
        g_field_camera_follow_z = g_field_camera_target_z;
        func_800A3BE8(sound_bank_id);
        g_layout_option = sound_bank_id;
        field_upload_transition_tiles();
        g_field_scene_contact_latched = 0;
        field_initialize_actor_slots();
        field_clear_actor_slots();
        func_80067AA4();
        func_80084240();
        func_800A255C();
        func_80086F20();
        saved_scene_id = g_scene_mode;
        if (g_field_scene_mode_bit != 0)
        {
            saved_scene_id += 0x8000;
        }
        func_800A8D10(saved_scene_id, object_id, g_layout_flag, spawn_id, g_layout_option, g_layout_sub_mode);
        func_800A54D0();
        func_8008C7A8();
        func_800A2DFC();
        func_800AF8C4();
        field_upload_transition_tiles();
        func_800A6EEC();
    }
}

/**
 * @brief Reset actor resource eight to the built-in fallback geometry.
 */
static void field_reset_fallback_resource(void)
{

    extern FieldFallbackResourceTable g_field_resource_entries;
    extern u8 D_800EB274[];

    s32 flags;

    g_field_resource_entries.image_page = 7;
    g_field_resource_entries.geometry_start = D_800EB274;
    g_field_resource_entries.geometry_end = D_800EB274 + 0x3C;
    flags = g_field_resource_entries.flags & ~1;
    g_field_resource_entries.flags = flags;
    g_field_resource_entries.palette = 0;
    g_field_resource_entries.multiple_images = 0;
    g_field_resource_entries.unknown_0x0e = 0;
    g_field_resource_entries.flags = flags | 2;
}

/**
 * @brief Upload an actor TIM image and optionally its palette to field VRAM.
 * @param image_data Paletted TIM file.
 * @param image_page Destination texture page allocation index.
 * @param actor_slot Actor slot selecting the destination palette row.
 * @param upload_palette Nonzero to upload the TIM palette before the image.
 */
static void field_upload_actor_image(void* image_data, s32 image_page, s32 actor_slot, s32 upload_palette)
{
    RECT rect;
    u16 width;
    u16 height;
    s32 size;

    width = ((FieldTimFile*)image_data)->palette.rect.w;
    height = ((FieldTimFile*)image_data)->palette.rect.h;
    size = ((FieldTimFile*)image_data)->palette.size;
    if (upload_palette != 0)
    {
        setRECT(&rect, 0, actor_slot + 500, width * height, 1);
        LoadImage(&rect, ((FieldTimFile*)image_data)->palette.pixels);
    }
    image_data = (u8*)image_data + size + 8;
    width = ((FieldTimBlock*)image_data)->rect.w;
    height = ((FieldTimBlock*)image_data)->rect.h;
    if (image_page >= 10)
    {
        rect.x = 960 - ((image_page - 9) << 6);
        rect.y = 256;
    }
    else
    {
        rect.x = 832 - (image_page << 6);
        rect.y = 0;
    }
    do
    {
        rect.w = width;
    } while (0);
    rect.h = height;
    LoadImage(&rect, ((FieldTimBlock*)image_data)->pixels);
    DrawSync(0);
}

/**
 * @brief Copy a word-aligned byte range into the field resource cursor.
 * @param src Start of the source range.
 * @param end End of the source range (exclusive, rounded up to a word).
 */
static void field_copy_scene_geometry(s32* src, s32* end)
{
    extern void* g_field_resource_cursor;

    s32 size;
    s32 words;
    s32* dst;

    size = (char*)end - (char*)src + 3;
    words = size >> 2;
    dst = g_field_resource_cursor;
    g_field_resource_cursor = (char*)dst + (size & ~3);
    while (words != 0)
    {
        *dst = *src;
        dst++;
        src++;
        words--;
    }
}

/**
 * @brief Initialize active actors from a count-prefixed field entry list.
 * @param data Entry count followed by 0x30-byte actor load records.
 * @note Active entries fill consecutive actor slots beginning at slot three.
 */
static void field_load_scene_actors(s32* data)
{

    extern FieldLoadedActor g_field_scene_actors[];
    extern FieldLoadedActorSlot D_80106194[];
    extern FieldLoadedActorVisual D_800FE3A0[];
    extern s32 D_800FE774;
    extern s32 g_field_pending_scene_id;
    extern void field_initialize_actor_record(s32, s32);
    extern void field_restart_actor_animation(FieldLoadedActor*);

    FieldLoadedActor* actor = g_field_scene_actors;
    FieldLoadedActorSlot* slot = D_80106194;
    FieldActionRequest* entry = (FieldActionRequest*)(data + 1);
    s32 active = 0;
    s32 index = active;
    s32 count = *data;
    s32 i;
    u32 tag;
    u32 slot_base;
    u8 blue;

    D_800FE774 = 3;
    if (count > 0)
    {
        do
        {
            slot->tag &= 0x7FFFFFFF;
            field_install_actor_action(entry, index);
            if (entry->control.flags < 0)
            {
                field_initialize_actor_record(active + 3, entry->source.actor + 3);
                if (((u32)entry->control.flags >> 30) & 1)
                {
                    actor->presence = 0xFE;
                }
                else
                {
                    actor->presence = 0;
                }
                actor->mode.bits.group = (u32)entry->control.flags >> 28;
                if ((g_field_pending_scene_id & 0x7FFF) == 0x13D)
                {
                    actor->mode.bits.group = 0;
                }
                actor->mode.bits.render = (u32)entry->control.flags >> 24;
                actor->x = entry->position.halves.x << 8;
                actor->z = (entry->position.halves.z & 0x7FF) << 8;
                actor->y = (entry->position.word >> 30) << 8;
                slot->state.bits.bit6 = 0;
                slot->state.bits.bit7 = 0;
                slot->options &= 0xFFFF7FFF;
                slot->red = D_800FE3A0[actor->slot].red;
                slot->green = D_800FE3A0[actor->slot].green;
                blue = D_800FE3A0[actor->slot].blue;
                slot->alpha = 0;
                slot->unknown18e = 0;
                tag = slot->tag & 0x80FFFFFF;
                slot->state.bits.bit5 = 0;
                slot->tag = tag;
                slot->blue = blue;
                slot->tag = (tag & 0xFF000000) | (slot->base & 0xFFFFFF);
                i = 0;
                do
                {
                    slot_base = slot->base;
                    slot->index = index + 3;
                } while (0);

                slot->link = slot_base & 0xFFFFFF;
                slot->id = entry->enabled_events;
                slot->flags = entry->control.flags;
                do
                {
                    slot->params[i] = entry->scripts[i];
                    i++;
                } while (i < FIELD_ACTION_SCRIPT_COUNT);
                field_restart_actor_animation(actor);
                slot++;
                actor++;
                active++;
                D_800FE774++;
            }
            do
            {
                index++;
            } while (0);

            entry++;
        } while (index < count);
    }
}

/**
 * @brief Refresh collision state and positions for the thirteen field actors.
 * @note Uses the scratchpad mover at 0x1F800000 and bounds at 0x801ED400.
 */
static void field_refresh_actor_collisions(void)
{

    extern FieldCollisionActor g_field_actors[];
    extern FieldCollisionSlot g_field_object_states[];
    extern s32 func_8005B6AC(FieldCollisionRequest*);

    FieldCollisionActor* actor;
    FieldCollisionSlot* slot;
    FieldCollisionBounds* bounds = (FieldCollisionBounds*)0x801ED400;
    FieldCollisionRequest* mover = (FieldCollisionRequest*)0x1F800000;
    s32 i, x, z;
    actor = g_field_actors;
    slot = g_field_object_states;
    for (i = 0; i < 13; i++, slot++, actor++)
    {
        if (actor->presence != 0xFF)
        {
            x = actor->x;
            if (x >= 0 && x < (bounds->width << 8) && (z = actor->z) >= 0 && z < ((s32)(bounds->height << 16) >> 7))
            {
                mover->x = x;
                mover->y = actor->y;
                mover->z = actor->z;
                mover->radius = 12;
                mover->mode.bits.step = 8;
                mover->dx = 0;
                mover->dy = 0;
                mover->dz = 0;
                mover->depth = 16;
                mover->contact = -1;
                mover->surface = 0;
                mover->mode.bits.bit17 = 0;
                mover->mode.bits.bit16 = 0;
                func_8005B6AC(mover);
                slot->contact = mover->contact;
                slot->surface = mover->surface;
                slot->height = mover->height >> 8;
                actor->x = mover->x;
                actor->z = mover->z;
                actor->y = mover->y;
            }
            else
            {
                slot->height = 0;
                slot->contact = -1;
                slot->surface = 0;
            }
        }
    }
}

/**
 * @brief Apply collision-constrained motion to an actor position.
 * @param actor Actor record beginning with fixed-point X, Y, Z coordinates.
 * @param motion Three 32-bit X, Y, and Z motion components.
 */
void field_move_actor_position(void* actor, void* motion)
{
    extern u8 D_800FE3CE;
    extern s32 func_8005B6AC(ScratchMover * mover);

    MoverBounds* bounds = (MoverBounds*)0x801ED400;
    ScratchMover* mover = (ScratchMover*)0x1F800000;
    s32 position_z;
    s32 position_x;
    s32 mode_flags;

    position_x = ((MoverPosition*)actor)->x;
    if (position_x < 0)
    {
        return;
    }
    if (position_x >= (bounds->width << 8))
    {
        return;
    }

    position_z = ((MoverPosition*)actor)->z;
    if (position_z < 0)
    {
        return;
    }
    if (position_z >= ((s32)(bounds->height << 16) >> 7))
    {
        return;
    }

    mover->position[0] = position_x;
    mover->position[1] = ((MoverPosition*)actor)->y;
    mover->position[2] = ((MoverPosition*)actor)->z;
    mover->motion[0] = ((s32*)motion)[0];
    mover->motion[1] = ((s32*)motion)[1];
    mover->motion[2] = ((s32*)motion)[2];

    if ((u8)D_800FE3CE >= 0x40)
    {
        mover->radius = 0xC;
        mover->mode.halves.status = 8;
    }
    else
    {
        mover->radius = 9;
        mover->mode.halves.status = 6;
    }

    ((volatile ScratchMover*)mover)->depth = 0x10;
    mode_flags = ((volatile ScratchMover*)mover)->mode.word;
    mover->contact = -1;
    mover->surface = 0;
    mode_flags &= 0xFFFDFFFF;
    mode_flags &= 0xFFFEFFFF;
    mover->mode.word = mode_flags;

    func_8005B6AC(mover);

    ((MoverPosition*)actor)->x = mover->position[0];
    ((MoverPosition*)actor)->z = mover->position[2];
    ((MoverPosition*)actor)->y = mover->position[1];
}

/**
 * @brief Assign per-slot palette values for the two persistent field entries.
 * @see decomp.me (100%) TODO
 */
void field_set_party_palettes(void)
{

    extern FieldPaletteResource g_field_resource_entries[];
    extern FieldPartyResource D_800FD818[];
    extern u16 D_800EB2B4[];
    extern s32 g_field_party_palette_index;

    s32 i;

    if (g_field_party_palette_index >= 6)
    {
        g_field_party_palette_index = 5;
    }
    i = 0;
    do
    {
        if (D_800FD818[i].resource_kind == 0)
        {
            g_field_resource_entries[i].palette = D_800EB2B4[g_field_party_palette_index];
        }
        else
        {
            g_field_resource_entries[i].palette = 0;
        }
        i += 1;
    } while (i < 2);
}

/**
 * @brief Refresh the two field fade actions in VRAM and flip the source bank.
 */
static void field_upload_transition_tiles(void)
{
    extern s32 D_801178B8;
    extern u16 D_800EB2D4[];
    extern u16 D_800EBAD4[];

    RECT rect;
    u16* src;

    setRECT(&rect, 0x120, 0xB4, 0x20, 0x20);
    if (D_801178B8 != 0)
    {
        src = D_800EBAD4;
    }
    else
    {
        src = D_800EB2D4;
    }
    LoadImage(&rect, (u_long*)src);

    setRECT(&rect, 0x120, 0x19C, 0x20, 0x20);
    if (D_801178B8 != 0)
    {
        src = D_800EBAD4;
    }
    else
    {
        src = D_800EB2D4;
    }
    LoadImage(&rect, (u_long*)src);

    DrawSync(0);
    D_801178B8 = D_801178B8 == 0;
}

/**
 * @brief Replace black pixels in the transition actions for a white fade target.
 */
static void field_prepare_transition_tiles(void)
{

    extern s32 D_801178B8;
    extern FieldTransitionFade g_field_fade_target;
    extern u16 D_800EB2D4[];

    s32 i;
    u16* p;

    D_801178B8 = 0;
    if (g_field_fade_target.red == 0x1FF && g_field_fade_target.green == g_field_fade_target.red && g_field_fade_target.blue == g_field_fade_target.green)
    {
        p = D_800EB2D4;
        for (i = 0; i < 0x800; i++, p++)
        {
            if ((*p & 0x7FFF) == 0)
                *p = 0xFFFF;
        }
    }
    else
    {
        p = D_800EB2D4;
        for (i = 0; i < 0x800; i++, p++)
        {
            if (*p == 0xFFFF)
                *p = 0;
        }
    }
}
