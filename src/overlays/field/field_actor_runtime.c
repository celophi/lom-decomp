/**
 * @file field_actor_runtime.c
 * @brief Field actor runtime: dialog and game-over hooks, animation actor slots and their
 *        parameter tracks, actor resource packages and per-frame actor animation.
 */

#include "common.h"
#include "controller_internal.h"
#include "main.h"
#include "cdrom.h"
#include "cd_resources.h"
#include "display.h"
#include "game_audio.h"
#include "sdk/libgpu.h"
#include "field_calls.h"
#include "field_types.h"
#include "field_actor_tables.h"
#include "field_actor_runtime.h"
#include "field_ability_progression.h"
#include "field_effect_dispatch.h"
#include "field_effect_render_state.h"
#include "field_modal_runtime.h"
#include "field_scene_transition.h"
#include "field_text.h"

/** @brief Fade level of the normal field view. */
#define FIELD_FADE_NORMAL_LEVEL 0xC0
/** @brief Frames of the standard field fade. */
#define FIELD_FADE_FRAMES 5
/** @brief Frames of the fade out before the game-over overlay loads. */
#define FIELD_GOVER_FADE_FRAMES 60
/** @brief Frames of the fade out when the return-to-title prompt closes. */
#define FIELD_TITLE_PROMPT_FADE_FRAMES 8
/** @brief Music fade time requested when the game-over transition starts. */
#define FIELD_GOVER_MUSIC_FADE 120

/** @brief Load address of the sub-overlays (GOVER.BIN here). */
#define FIELD_SUBOVERLAY_ADDRESS ((void*)0x80140000)
/** @brief Work area handed to the game-over overlay. */
#define FIELD_GOVER_WORK_ADDRESS 0x80160000

/** @brief Backup of g_field_resource_buffer that party resources are relocated from. */
#define FIELD_RESOURCE_BACKUP ((u8*)0x80180000)
/** @brief Size of the resource buffer backup. */
#define FIELD_RESOURCE_BACKUP_SIZE 0x10000
/** @brief Number of resource entries in g_field_resource_entries. */
#define FIELD_RESOURCE_ENTRY_COUNT 9
/** @brief Resource entries that can hold relocatable data (the last one is fixed). */
#define FIELD_RESOURCE_MOVABLE_COUNT 8
/** @brief FieldResourceEntry::flags bit: the entry holds loaded data. */
#define FIELD_RESOURCE_LOADED 2
/** @brief FieldResourceEntry::resource_index value of actors whose data is not in the arena. */
#define FIELD_RESOURCE_FIXED 8
/** @brief Resource entries 3..5 carry the voice bank of their actors. */
#define FIELD_RESOURCE_VOICE_FIRST 3
#define FIELD_RESOURCE_VOICE_COUNT 3

/** @brief Animation targets are passed as the low byte of one word each. */
#define FIELD_TARGET_STRIDE 4

/** @brief Party record used for the companion. */
#define FIELD_COMPANION_INDEX 2

/** @brief Character ids of companions from this value on are drawn in a selectable colour. */
#define FIELD_COMPANION_COLOR_ID_MIN 0x41

/** @brief CD resource id bases of the party sprite packages. */
#define FIELD_RES_HERO_SPRITES 0xAEB
#define FIELD_RES_PARTNER_SPRITES 0xAEE
#define FIELD_RES_COMPANION_SPRITES 0xB02
/** @brief CD resource id bases of the weapon-specific sprite packages used in the scene mode. */
#define FIELD_RES_HERO_WEAPON_SPRITES 0xA0C
#define FIELD_RES_HERO_ALT_WEAPON_SPRITES 0xA17
#define FIELD_RES_PARTNER_WEAPON_SPRITES 0xA23
#define FIELD_RES_COMPANION_WEAPON_SPRITES 0xA4B
/** @brief CD resource id bases of the partner and companion voice banks. */
#define FIELD_RES_PARTNER_VOICES 0xA37
#define FIELD_RES_COMPANION_VOICES 0xA9B
/** @brief CD resource id of the portrait archive and the id base of resource entry packages. */
#define FIELD_RES_PORTRAITS 0x5E5
#define FIELD_RES_ENTRY_PACKAGES 0xB52

/** @brief First portrait of the companions in the portrait archive (partners start at 2). */
#define FIELD_PORTRAIT_PARTNER_BASE 2
#define FIELD_PORTRAIT_COMPANION_BASE 0xE
/** @brief Size of one cached portrait image. */
#define FIELD_PORTRAIT_SIZE 0x4A0

/** @brief Companion kind bits of PadContext::unkAA8 and the golem kind. */
#define COMPANION_KIND_MASK 0x7F
#define COMPANION_KIND_GOLEM 4

/** @brief Per-player block of the saved game and its control byte (bit 7: controller in use). */
#define FIELD_SAVED_PLAYER_STRIDE 0x250
#define FIELD_SAVED_PLAYER_CONTROL 0x608

/** @brief Control modes (low bits of FieldActor::control). */
#define FIELD_CONTROL_PAD 0
#define FIELD_CONTROL_FOLLOW 1
#define FIELD_CONTROL_SCRIPTED 2
/** @brief FieldActor::control bits 16-17: colour variant of a companion. */
#define FIELD_CONTROL_VARIANT_SHIFT 16
#define FIELD_CONTROL_VARIANT_MASK 0x30000
/** @brief FieldActor::control bits 19-22 and the value given to the partner's record when the partner is the hero. */
#define FIELD_CONTROL_UNK19_MASK 0x780000
#define FIELD_CONTROL_UNK19_PARTNER 0x500000
/** @brief FieldActor::control bit 23: the actor uses its own tint. */
#define FIELD_CONTROL_TINTED_SHIFT 23
#define FIELD_CONTROL_TINTED 0x800000
/** @brief FieldActor::control bits 15 and 28, set together on an effect that takes its owner's part colour. */
#define FIELD_CONTROL_UNK8000 0x8000
#define FIELD_CONTROL_UNK10000000 0x10000000
#define FIELD_CONTROL_OWNER_TINT (FIELD_CONTROL_UNK10000000 | FIELD_CONTROL_UNK8000)
/** @brief FieldActor::control bit 18, cleared when an actor is initialized. */
#define FIELD_CONTROL_UNK40000 0x40000

/** @brief Frame-status word bit (FIELD_ACTOR_FRAME_WORD): the displayed frame did not change this tick. */
#define FIELD_FRAME_UNCHANGED 0x1000000

/** @brief Initial X of an actor record that is not placed yet. */
#define FIELD_ACTOR_UNPLACED_X 0xFFFB0000

/** @brief Neutral tint of actors and parts. */
#define FIELD_TINT_NEUTRAL 0x80

/** @brief Object state flag bits used here (FieldObjectState::flags). */
#define FIELD_OBJECT_ANIMATION_FROZEN 0x2000
/** @brief Object state flags that suspend an object's input, following and commands. */
#define FIELD_OBJECT_CONTROL_BLOCKED 0x21E4

/** @brief FieldObjectState::contact bits 6 and 7 cleared when an actor is initialized. */
#define FIELD_CONTACT_UNK40 0x40


/** @brief FieldActor::presence value of an object hidden by an animation actor. */
#define FIELD_ANIMATION_HIDDEN FIELD_ACTOR_HIDDEN

/** @brief Status value of an owner-linked-less slot playing animation 0xC. */
#define FIELD_SLOT_STATUS_KEEP_EFFECTS_MASK 0xFFFF0001
#define FIELD_SLOT_STATUS_KEEP_EFFECTS 0xC0000
/** @brief Animation ids 0x1F..0x23 are the special attacks. */
#define FIELD_SPECIAL_ATTACK_FIRST 0x1F
#define FIELD_SPECIAL_ATTACK_END 0x24
/** @brief Flag ORed into the object index returned for a special attack. */
#define FIELD_SPECIAL_ATTACK_RESULT 0x200



/** @brief Packed red and green halves of a neutral colour scale. */
#define FIELD_COLOR_SCALE_NEUTRAL_RG 0x1000100UL

/** @brief Full-screen tint primitive: semi-transparent monochrome tile. */
#define FIELD_TINT_TILE_CODE 0x62

/** @brief Number of field effect records. */
#define FIELD_EFFECT_RECORD_COUNT 0x103
/** @brief FieldActor::presence of a free effect record. */
#define FIELD_EFFECT_FREE FIELD_ACTOR_UNUSED

/** @brief FieldObjectPart values: unk31 of a disabled part, unk31 of a part active from the first frame, unkB of a part without effects. */
#define FIELD_PART_DISABLED 0xFE
#define FIELD_PART_ALWAYS 0xFF
#define FIELD_PART_NO_EFFECT 0xFF
/** @brief FieldObjectPart::flags bit: the part is an attack sphere. */
#define FIELD_PART_ATTACK_SPHERE 0x04000000
/** @brief FieldObjectPart::unk14 bit: the part only runs on track 0. */
#define FIELD_PART_FIRST_TRACK_ONLY 0x4
/** @brief FieldObjectPart::unk28 bits: fixed effect count, spawn only when none is live, owner colour off, spawn cap. */
#define FIELD_PART_FIXED_COUNT_SHIFT 24
#define FIELD_PART_UNK25_SHIFT 25
#define FIELD_PART_SPAWN_CAP_SHIFT 30
/** @brief FieldObjectPart::unk0 bit 15: spawn effects one at a time until the count is reached. */
#define FIELD_PART_SPAWN_SINGLE_SHIFT 15
/** @brief FieldObjectPart::unk2C low five bits: effects per burst minus one; bits 5-7 must be clear for owner colour. */
#define FIELD_PART_BURST_MASK 0x1F
#define FIELD_PART_BURST_SHIFT 5
/** @brief FieldObjectPart::unk4 byte 3 low four bits: spawn period minus one. */
#define FIELD_PART_PERIOD_MASK 0xF
/** @brief FieldObjectPart::unk4 bits 4-5: the part animation loops (zero ends it). */
#define FIELD_PART_LOOP_SHIFT 4
/** @brief FieldObjectPart::unk4 bit 11: the part keeps its own colour. */
#define FIELD_PART_OWN_COLOR_SHIFT 11
/** @brief FieldObjectPart::unk4 bits 22-23: render mode. */
#define FIELD_PART_RENDER_MODE_SHIFT 22
#define FIELD_PART_RENDER_MODE_MASK 0xC00000
/** @brief Z/X scale of a part initialized with and without the timer mode. */
#define FIELD_PART_SCALE_SMALL 0x30
#define FIELD_PART_SCALE_FULL 0x40

/** @brief Sound event types and command kinds of an animation (FieldAnimationDef bytes 2..9). */
#define FIELD_SOUND_EVENT_START 1
#define FIELD_SOUND_EVENT_ONCE 5
#define FIELD_SOUND_EVENT_COUNT 2
#define FIELD_SOUND_ID_MASK 0x3FF
#define FIELD_SOUND_KIND_SHIFT 10
#define FIELD_SOUND_PLAYED 0x01
#define FIELD_SOUND_ONCE_PLAYED 0x80

/** @brief Pan range of actor sounds and the screen span mapped onto it. */
#define FIELD_PAN_LEFT 0x60
#define FIELD_PAN_RIGHT 0x9F
#define FIELD_PAN_SCREEN_LEFT 0x10
#define FIELD_PAN_SCREEN_RIGHT 0x131

/** @brief Screen centre that actor positions are projected around. */
#define FIELD_SCREEN_CENTER_X 0xA0
#define FIELD_SCREEN_CENTER_Y 0x70

/** @brief Parameter curve word: segment count, random flag; segment word: length and value. */
#define FIELD_CURVE_SEGMENT_COUNT_MASK 0x7F
#define FIELD_CURVE_RANDOM 0x80
#define FIELD_SEGMENT_LENGTH_MASK 0x3FF
#define FIELD_SEGMENT_VALUE_SHIFT 10

/** @brief Palette tracks interpolate 16 colours from four-bit palette selectors. */
#define FIELD_PALETTE_COLORS 16
#define FIELD_PALETTE_SELECTOR_BITS 4
#define FIELD_PALETTE_SELECTOR_END 16

/** @brief Actor animation resource: frame entries are two bytes, or four with height and motion. */
#define FIELD_ANIMATION_WRAP 0x1
#define FIELD_ANIMATION_MODE_OFFSET 0x40

/**
 * @brief Sound command of event @p event of an animation (FieldAnimationDef::sound_commands).
 * @note A byte offset from the animation; indexing sound_commands[] adds the operands the other way round.
 */
#define FIELD_SOUND_COMMAND(animation, event) (((u16*)((u8*)(animation) + ((event) << 1)))[4])

/** @brief Word view of FieldActor bytes 0x3C..0x3F; bit 24 is FIELD_FRAME_UNCHANGED. */
#define FIELD_ACTOR_FRAME_WORD(actor) (*(u32*)&(actor)->unk3C)

/** @brief Target and restore colours of the field fade (see field_fade.c). */
typedef struct
{
    s16 red;
    s16 green;
    s16 blue;
    s16 duration;
} FieldFade;

/** @brief Global colour scale applied to every actor; w views red and green together. */
typedef union
{
    struct
    {
        s16 red;
        s16 green;
        s16 blue;
    } s;
    u32 w;
} FieldColorScale;

/** @brief Direction offset used to place a copied actor next to the leader. */
typedef struct
{
    s16 distance;
    s16 unk2;
} FieldDirectionOffset;

/** @brief Staged actor resource package: section offsets, then section data. */
typedef struct FieldCdBuffer
{
    u32 offsets[4];
    u16 width;
    u16 height;
    u16 data;
} FieldCdBuffer;

/** @brief Action command map of one controller: command words and disable flags. */
typedef struct
{
    u16 commands[11];
    u8 disabled[11];
    u8 pad;
} FieldActionCommandMap;

/** @brief Action 1 and 5 commands restored by field_restore_default_action_animation_mappings(). */

void func_80140004(s32 work_address, s32 image_resource_index, s32 music_resource_index, s32 audio_clip_index);
void field_reset_input_repeat();
void field_draw_dialog_windows(s32);
void field_merge_dialog_items(void);
void akao_cmd_c1(s32, s32, s32);
void akao_cmd_a9(s32, s32);
FieldActor* field_lookup_actor(s32 key);

static void field_relocate_resource_buffer(s32 resource_index);
s32 field_get_actor_resource_id(s32 unused_slot_index, FieldPlayerRecord* player, s32 weapon_set);
static void field_load_actor_resource_slot(s32 resource_index, s32 slot_index, s32 resource_id, s32 alternate_layout);
void field_initialize_actor_record(s32 actor_index, s32 resource_entry_index);
void field_initialize_actor_part(s32 part_index, s32 timer_mode);
static void field_build_actor_render_commands(FieldRenderContext* render_ctx, s32 unused);
s32 field_evaluate_parameter_track(FieldActorSlot* slot, s32 curve_index);
void field_clear_actor_effects(FieldActorSlot* slot);
static void field_update_actor_effects(FieldActorSlot* slot);
static void field_update_actor_part_effects(FieldActorSlot* slot);
void field_dispatch_actor_audio_event(FieldActorSlot* slot, s32 event_type, s32 event_subtype);
static void field_reset_actor_track_mask(FieldActorSlot* slot);
void field_set_global_color_scale(s16 red, s16 green, s16 blue);
static void field_apply_global_color_scale(void);
static u8* field_advance_actor_animation_frame(FieldActor* actor);
u8* field_begin_actor_animation_forward(FieldActor* actor, u8* resource_base);
static u8* field_begin_actor_animation_reverse(FieldActor* actor, u8* resource_base);
static void field_settle_actor_vertical_offset(FieldActor* actor);
void field_load_resource_entry(s32 resource_slot_id, u8* resource_base, s32 entry_index);
void field_release_resource_entry(s32 entry_index);
void field_unpack_resource_package(FieldCdBuffer* buf, s32 size, s32 slot_index, s32 palette_row);
static void field_upload_resource_texture(FieldCdBuffer* buf, s32 slot_index, s32 texture_index, s32 palette_row);
static void field_append_resource_data(u32* src, s32 length, s32 slot_index);
static void field_refresh_actor_portraits();
s32* field_render_effect_frame16(FieldActor* actor, s32* packet, u32* ordering_table, s32 frame_data, s32 unused, FieldObjectPart* part);
s32* field_render_effect_frame8(FieldActor* actor, s32* packet, u32* ordering_table, s32 frame_data, s32 unused, FieldObjectPart* part);

extern FieldColorScale g_field_color_scale;
extern s8 g_field_color_scale_active;
extern s32 g_field_text_session_active;
extern s32 g_field_gover_image_resource_id;
extern s32 g_field_gover_music_resource_id;
extern s32 g_field_gover_load_countdown;
extern s32 g_field_gover_audio_clip_id;
extern s32 g_field_return_to_title_prompt_delay;
extern s32 g_field_return_to_title_prompt_state;
extern s32 D_8012291C;
extern s32 g_field_audio_timer;
extern s32 g_field_track_index;
extern s32 g_field_song_volume;
extern FieldFade g_field_fade_target;
extern FieldFade g_field_fade_restore_color;
extern s32 g_field_camera_offset_x;
extern s32 g_field_camera_offset_y;
extern s32 g_field_camera_offset_z;
extern u8 g_field_resource_buffer[];
extern FieldAnimationDef g_field_object_default_animation;
extern s32 D_800FE774;
extern void* g_field_resource_cursor;
extern s32 g_field_scene_mode_bit;
extern FieldDirectionOffset g_field_direction_offsets[];
extern s32 g_field_active_group;
extern s32 D_80122710;
extern s32 D_80122714;
extern s32 D_80122B20;
extern FieldActor g_field_effect_records[];
extern s32 D_80105770;
extern s32 D_800F2298;
extern s32 g_field_modal_state;
extern s32 D_80105760;
extern FieldCdBuffer* g_field_cd_buffer;
extern s32 g_field_dialog_screen_mode;
extern s32 g_field_duel_mode[];
extern FieldActionCommandMap g_field_action_command_maps[];

/*
 * Retail addresses of g_field_resource_entries[3].unk8, g_field_player_records[1]'s flag
 * word, weapon_type and character_kind, and g_field_player_records[2].character_id. They
 * stay separate symbols because the rebuilt overlay lays its data out differently.
 */
extern u8 D_800FF59C;
extern u16 D_800FDA80;
extern u8 D_800FDA81;
extern u8 D_800FDA83;
extern u8 D_800FDCEA;

/**
 * @brief Advance the active field dialog runtime and finish any pending text work.
 * @param update_mode Mode forwarded to the active dialog update helper.
 */
void field_update_dialog_runtime(s32 update_mode)
{
    if (g_field_dialog_screen_mode != 0)
    {
        field_update_battle_results_input();
        if (g_field_dialog_screen_mode != 0)
        {
            field_text_reset_scratch();
            if (g_field_dialog_screen_mode != 0)
            {
                field_draw_dialog_windows(update_mode);
            }
            field_text_upload_immediate_cache();
        }
    }
}

/**
 * @brief Restore the standard field fade and refresh the active dialog/menu state.
 */
void func_80068028(void)
{
    field_reset_input_repeat();
    g_field_fade_target.red = FIELD_FADE_NORMAL_LEVEL;
    g_field_fade_target.green = FIELD_FADE_NORMAL_LEVEL;
    g_field_fade_target.blue = FIELD_FADE_NORMAL_LEVEL;
    g_field_fade_target.duration = FIELD_FADE_FRAMES;
    if (g_field_duel_mode[0] == 0)
    {
        field_merge_dialog_items();
        field_advance_ability_progression();
        field_open_battle_results();
    }
    else
    {
        field_open_duel_results();
    }
}

/**
 * @brief Close the dialog screen: restore the fade and put both players back into their idle animation.
 */
void field_close_dialog_screen(void)
{
    s32 i;

    g_field_fade_target.red = g_field_fade_restore_color.red;
    g_field_fade_target.green = g_field_fade_restore_color.green;
    g_field_fade_target.blue = g_field_fade_restore_color.blue;
    g_field_fade_target.duration = FIELD_FADE_FRAMES;
    field_text_reset_windows();

    g_field_dialog_screen_mode = 0;

    for (i = 0; i < FIELD_PLAYER_COUNT; i++)
    {
        if (g_field_player_records[i].head.bytes.flags & FIELD_PLAYER_ACTIVE)
        {
            g_field_actors[i].command = FIELD_ACTOR_COMMAND_IDLE_AFTER_RELOAD;
            g_field_actors[i].animation_state = 1;
            g_field_actors[i].animation_frame = 0;
            g_field_actors[i].animation_active = 1;
            g_field_actors[i].control.word &= ~FIELD_CONTROL_MODE_MASK;
            g_field_actors[i].animation = (g_field_actors[i].animation & FIELD_ANIMATION_FACING) + 0x12;
            g_field_object_states[i].movement.word &= ~FIELD_MOVEMENT_SEQUENCE_MASK;
            /* Unprototyped call: the mask is still in $a1 and the target passes it along. */
            field_restart_actor_animation(&g_field_actors[i], (void*)~FIELD_CONTROL_MODE_MASK);
        }
    }

    field_restore_default_action_animation_mappings(0);
    D_8012291C = 0;
    field_camera_select_scroll_limits();
}

/**
 * @brief Script command: open the return-to-title confirmation prompt.
 * @param command_value Command operand; only zero opens the prompt.
 */
void field_request_return_to_title(s32 command_value)
{
    if (command_value == 0)
    {
        field_open_return_to_title_prompt();
    }
}

/**
 * @brief Start the timed transition that loads the game-over overlay.
 * @param image_resource_index Image resource passed to the game-over overlay.
 * @param music_resource_index Music resource passed to the game-over overlay.
 * @param audio_clip_index Audio clip passed to the game-over overlay.
 */
void field_begin_gover_transition(s32 image_resource_index, s32 music_resource_index, s32 audio_clip_index)
{
    s32 fade_time;

    if (g_field_gover_load_countdown == 0)
    {
        g_field_gover_image_resource_id = image_resource_index;
        g_field_gover_music_resource_id = music_resource_index;
        g_field_gover_audio_clip_id = audio_clip_index;
        field_reset_actor_resources();
        g_field_fade_target.red = 0;
        g_field_fade_restore_color.red = 0;
        g_field_fade_target.green = 0;
        g_field_fade_restore_color.green = 0;
        g_field_fade_target.blue = 0;
        g_field_fade_restore_color.blue = 0;
        fade_time = FIELD_GOVER_FADE_FRAMES;
        g_field_fade_target.duration = fade_time;
        cdrom_queue_seek(CD_RES_GOVER_BIN);
        g_field_gover_load_countdown = fade_time;
        akao_cmd_c1(0, FIELD_GOVER_MUSIC_FADE, 0);
        akao_cmd_a9(FIELD_GOVER_MUSIC_FADE, 0);
    }
}

/**
 * @brief Count down the game-over transition, then load and start the game-over overlay.
 * @see decomp.me (100%) https://decomp.me/scratch/9Ady0
 */
void field_update_gover_load(void)
{
    ControllerState* controller = CONTROLLER_STATE;

    if (g_field_gover_load_countdown == 0)
    {
        return;
    }

    if (--g_field_gover_load_countdown == 0)
    {
        controller->ports[1].actuator_control.fields.large_motor_command = 0;
        controller->ports[0].actuator_control.fields.large_motor_command = 0;
        controller->ports[1].small_motor_command = 0;
        controller->ports[0].small_motor_command = 0;
        cdrom_stream(CD_RES_GOVER_BIN, FIELD_SUBOVERLAY_ADDRESS);
        cdrom_wait_queue_empty();
        func_80140004(FIELD_GOVER_WORK_ADDRESS, g_field_gover_image_resource_id, g_field_gover_music_resource_id, g_field_gover_audio_clip_id);
        field_reset_actor_resources();
    }
}

/**
 * @brief Update and render the modal return-to-title confirmation prompt.
 * @param render_ctx Current field render context passed to the prompt renderer.
 * @see decomp.me (100%) https://decomp.me/scratch/Kws0l
 */
void field_update_return_to_title_prompt(s32 render_ctx)
{
    if (g_field_return_to_title_prompt_state != 0)
    {
        if (g_field_return_to_title_prompt_delay != 0)
        {
            g_field_return_to_title_prompt_delay--;
            if (g_field_return_to_title_prompt_delay == 0)
            {
                g_field_fade_target.red = FIELD_FADE_NORMAL_LEVEL;
                g_field_fade_target.green = FIELD_FADE_NORMAL_LEVEL;
                g_field_fade_target.blue = FIELD_FADE_NORMAL_LEVEL;
                g_field_fade_target.duration = FIELD_FADE_FRAMES;
            }
        }
        else
        {
            field_handle_return_to_title_prompt();
            if (g_field_return_to_title_prompt_state != 0)
            {
                field_text_reset_scratch();
                if (g_field_return_to_title_prompt_state != 0)
                {
                    field_draw_dialog_windows(render_ctx);
                }
                field_text_upload_immediate_cache();
            }
        }
    }
}

/**
 * @brief Initialize and open the return-to-title confirmation prompt.
 * @see decomp.me (100%) https://decomp.me/scratch/doJjR
 */
void field_open_return_to_title_prompt(void)
{
    field_reset_input_repeat();
    field_setup_return_to_title_prompt();
}

/**
 * @brief Begin closing the return-to-title prompt: fade to black and reset the text windows.
 * @see decomp.me (100%) https://decomp.me/scratch/b8yys
 */
void field_begin_return_to_title_prompt_close(void)
{
    g_field_fade_target.red = 0;
    g_field_fade_restore_color.red = 0;
    g_field_fade_target.green = 0;
    g_field_fade_restore_color.green = 0;
    g_field_fade_target.blue = 0;
    g_field_fade_restore_color.blue = 0;
    g_field_fade_target.duration = FIELD_TITLE_PROMPT_FADE_FRAMES;
    field_text_reset_windows();
    D_8012291C = 0;
}

/**
 * @brief Advance the deferred field audio timer and start playback when it expires.
 * @see decomp.me (100%) https://decomp.me/scratch/KDXt0
 */
void field_update_audio_timer(void)
{
    s32 remaining_frames;

    if (g_field_audio_timer != 0)
    {
        remaining_frames = g_field_audio_timer - 1;
        g_field_audio_timer = remaining_frames;
        if (remaining_frames == 0)
        {
            fade_out_current_song();
            field_play_song();
            field_fade_song(0, 1, g_field_song_volume);
        }
    }
}

/**
 * @brief Return the frame of the current track (g_field_track_index) modulo a divisor.
 * @param slot Animation actor slot.
 * @param divisor Divisor applied to the track frame.
 * @return Track frame modulo @p divisor.
 * @see decomp.me (100%) https://decomp.me/scratch/Xn30r
 */
s32 field_get_track_counter_modulo(FieldActorSlot* slot, s32 divisor)
{
    return slot->track_frames[g_field_track_index] % divisor;
}

/**
 * @brief Interpolate a 16-colour palette along a palette curve at the current track frame.
 * @param slot Animation actor slot whose curve segments time the palette steps.
 * @param palette_sequence Low four bits select the curve; the higher nibbles select palettes.
 * @param palette_table Base address of the 32-byte palettes.
 * @param output Destination for the 16 interpolated colours.
 * @see decomp.me (100%) https://decomp.me/scratch/X9uyL
 */
void field_interpolate_palette_track(FieldActorSlot* slot, s32 palette_sequence, s32 palette_table, s16* output)
{
    s32 segment_start_frame = 0;
    s32 selector_shift = FIELD_PALETTE_SELECTOR_BITS;
    s16* output_color;
    s32 color_index;
    s32 palette_selectors;
    s32 segments_remaining = (&slot->curves[palette_sequence & 0xF])->head.bytes.segment_count & FIELD_CURVE_SEGMENT_COUNT_MASK;
    u16* segment = &slot->curve_segments[(&slot->curves[palette_sequence & 0xF])->head.bytes.segment_offset];

    palette_selectors = palette_sequence;
    if (segments_remaining != 0)
    {
        do
        {
            u16 segment_word = *segment;
            s32 segment_end_frame = segment_start_frame + (segment_word & FIELD_SEGMENT_LENGTH_MASK);

            if (slot->track_frames[g_field_track_index] < segment_end_frame)
            {
                break;
            }
            segment_start_frame = segment_end_frame;
            segment++;
            selector_shift += FIELD_PALETTE_SELECTOR_BITS;
            segments_remaining--;
            if (selector_shift == FIELD_PALETTE_SELECTOR_END)
            {
                selector_shift = FIELD_PALETTE_SELECTOR_BITS;
            }
        } while (segments_remaining != 0);
    }
    if (segments_remaining != 0)
    {
        s32 selectors = palette_selectors & 0xFFFF;
        u16* start_palette = (u16*)(palette_table + (((selectors >> selector_shift) & 0xF) << 5));
        u16* end_palette;

        if ((selector_shift + FIELD_PALETTE_SELECTOR_BITS) != FIELD_PALETTE_SELECTOR_END)
        {
            end_palette = (u16*)(palette_table + (((selectors >> (selector_shift + FIELD_PALETTE_SELECTOR_BITS)) & 0xF) << 5));
        }
        else
        {
            end_palette = (u16*)(palette_table + ((selectors << 1) & 0x1E0));
        }
        output_color = output;
        color_index = 0;
        do
        {
            u16 start_color = *start_palette;
            u16 end_color = *end_palette;
            s32 start_red = start_color & 0x1F;
            s32 end_red = end_color & 0x1F;
            s32 red_range = end_red - start_red;
            s32 frame_offset = slot->track_frames[g_field_track_index] - segment_start_frame;
            s32 segment_length = (*segment) & FIELD_SEGMENT_LENGTH_MASK;
            s32 red_step = (red_range * frame_offset) / segment_length;
            s32 start_green = (start_color >> 5) & 0x1F;
            s32 end_green = (end_color >> 5) & 0x1F;
            s32 green_range = end_green - start_green;
            s32 green_step = (green_range * frame_offset) / segment_length;
            s32 start_blue = (start_color >> 10) & 0x1F;
            s32 end_blue = (end_color >> 10) & 0x1F;
            s32 blue_range = end_blue - start_blue;
            s32 blue_step = (blue_range * frame_offset) / segment_length;

            *output_color = (((start_color & 0x8000) | (start_red + red_step)) | ((start_green + green_step) << 5)) | ((start_blue + blue_step) << 10);
            output_color++;
            color_index++;
            start_palette++;
            end_palette++;
        } while (color_index < FIELD_PALETTE_COLORS);
    }
}

/**
 * @brief Evaluate an animation parameter curve at the current track frame.
 * @param slot Animation actor slot holding the curves and track frames.
 * @param curve_index Parameter curve index.
 * @return Curve value; random curves scale the value by rand() / 32768.
 * @see decomp.me (100%) https://decomp.me/scratch/t5bIj
 */
s32 field_evaluate_parameter_track(FieldActorSlot* slot, s32 curve_index)
{
    s32 segment_start_frame;
    FieldParameterCurve* curve;
    s32 start_delta;
    s32 value_range;
    u16* segment;
    s32 segments_remaining;
    u16 current_frame;
    u16 segment_word;
    s32 segment_end_frame;
    u16 final_segment_word;
    s32 random_value;

    segment_start_frame = 0;
    curve = slot->curves + curve_index;
    start_delta = segment_start_frame;
    segments_remaining = curve->head.bytes.segment_count & FIELD_CURVE_SEGMENT_COUNT_MASK;
    segment = slot->curve_segments + curve->head.bytes.segment_offset;
    if (segments_remaining != 0)
    {
        current_frame = slot->track_frames[g_field_track_index];
        do
        {
            /* Empty test: without it the loop is laid out differently. */
            if (!slot)
            {
            }
            segment_word = *segment;
            segment_end_frame = segment_start_frame + (segment_word & FIELD_SEGMENT_LENGTH_MASK);
            if (current_frame < segment_end_frame)
            {
                break;
            }
            segment_start_frame = segment_end_frame;
            start_delta = segment_word >> FIELD_SEGMENT_VALUE_SHIFT;
            segments_remaining--;
            segment++;
        } while (segments_remaining != 0);
    }
    value_range = curve->end_value - curve->start_value;
    start_delta = (value_range * start_delta) >> 5;
    if (segments_remaining == 0)
    {
        return curve->start_value;
    }
    if ((curve->head.word & FIELD_CURVE_RANDOM) && (g_field_text_session_active == 0))
    {
        random_value = rand();
        return curve->start_value +
               (((start_delta + (((((value_range * ((*segment) >> FIELD_SEGMENT_VALUE_SHIFT)) >> 5) - start_delta) * (slot->track_frames[g_field_track_index] - segment_start_frame)) /
                                 ((*segment) & FIELD_SEGMENT_LENGTH_MASK))) *
                 random_value) >>
                15);
    }
    else
    {
        final_segment_word = *segment;
        return (curve->start_value + start_delta) +
               (((((value_range * (final_segment_word >> FIELD_SEGMENT_VALUE_SHIFT)) >> 5) - start_delta) * (slot->track_frames[g_field_track_index] - segment_start_frame)) /
                (final_segment_word & FIELD_SEGMENT_LENGTH_MASK));
    }
}

/**
 * @brief End an animation actor slot once its animation has run its course.
 * @param slot Animation actor slot to check.
 * @return 1 when the slot finished (and was stopped or chained), otherwise 0.
 * @see decomp.me (100%) https://decomp.me/scratch/MCyYP
 */
static s32 field_finalize_actor_animation(FieldActorSlot* slot)
{
    s32 found;
    u8 state_value;
    s32 i;
    FieldActorSlot* other_slot;
    s16 owner_command;

    if (slot->duration == slot->track_frames[0])
    {
        if (slot->animation->unk18 & FIELD_ANIM_RESTART_AT_END)
        {
            for (i = FIELD_ACTOR_TRACK_COUNT - 1; i >= 0; i--)
            {
                slot->track_frames[i] = 0;
            }

            return 0;
        }
        slot->track_mask = 0;
    }
    if (slot->track_mask == 0)
    {
        slot->pending_track_mask = 0;
        if (slot->animation->unk18 & FIELD_ANIM_OWNER_VISIBILITY)
        {
            if (g_field_object_states[slot->owner_object_index].contact.bytes.controller_index == slot->slot_index)
            {
                owner_command = g_field_actors[slot->owner_object_index].command;
                if (((owner_command != FIELD_ACTOR_COMMAND_DEFEATED) && (owner_command != FIELD_ACTOR_COMMAND_DEFEAT_END)) ||
                    (g_field_object_states[slot->owner_object_index].flags & FIELD_OBJECT_FLAG_KNOCKED_OUT))
                {
                    g_field_actors[slot->owner_object_index].presence = 0;
                }
                g_field_object_states[slot->owner_object_index].contact.word &= ~FIELD_CONTACT_ANIMATION_HIDDEN;
            }
        }
        if (slot->animation->unk18 & FIELD_ANIM_TARGET_VISIBILITY)
        {
            for (found = 0; found < ((s32)slot->target_count); found++)
            {
                if (slot->targets[found] != FIELD_TARGET_NONE)
                {
                    state_value = g_field_object_states[slot->targets[found]].contact.word;
                    if ((state_value & FIELD_CONTACT_ANIMATION_HIDDEN) &&
                        (g_field_object_states[slot->targets[found]].contact.bytes.controller_index == slot->slot_index))
                    {
                        g_field_actors[slot->targets[found]].presence = 0;
                        g_field_object_states[slot->targets[found]].contact.word &= ~FIELD_CONTACT_ANIMATION_HIDDEN;
                    }
                }
            }
        }
        if (slot->animation->flags & FIELD_ANIM_CAMERA_OFFSET)
        {
            g_field_camera_offset_z = 0;
            g_field_camera_offset_y = 0;
            g_field_camera_offset_x = 0;
        }
        if ((slot->animation->flags >> 8) & (FIELD_ANIM_GLOBAL_COLOR >> 8))
        {
            s32 j;

            other_slot = g_field_actor_slots;
            for (j = 0; j < FIELD_ACTOR_SLOT_TOTAL; j++, other_slot++)
            {
                found = 0;
                if (((slot != other_slot) && (other_slot->active != 0)) && ((other_slot->animation->flags >> 8) & (FIELD_ANIM_GLOBAL_COLOR >> 8)))
                {
                    found = 1;
                    break;
                }
            }

            if (found == 0)
            {
                field_set_global_color_scale(FIELD_COLOR_SCALE_NEUTRAL, FIELD_COLOR_SCALE_NEUTRAL, FIELD_COLOR_SCALE_NEUTRAL);
            }
        }
        if ((slot->status.word & FIELD_SLOT_STATUS_KEEP_EFFECTS_MASK) != FIELD_SLOT_STATUS_KEEP_EFFECTS)
        {
            field_clear_actor_effects(slot);
        }
        if (!(slot->animation->flags & FIELD_ANIM_KEEP_ALIVE))
        {
            slot->duration = 0;
            slot->active = 0;
            if (slot->status.word & FIELD_SLOT_OWNER_LINKED)
            {
                field_release_actor_binding(slot->owner_object_index);
            }
            return 1;
        }
        state_value = slot->unk2A;
        if (state_value != 0)
        {
            slot->duration = 0;
            slot->active = 0;
            field_release_actor_binding(slot->owner_object_index);
            for (i = 0; i < FIELD_ACTOR_SLOT_TOTAL; i++)
            {
                if (((g_field_actor_slots[i].active != 0) && (g_field_actor_slots[i].owner_object_index == slot->owner_object_index)) &&
                    ((state_value = g_field_actor_slots[i].status.word) & FIELD_SLOT_OWNER_LINKED))
                {
                    g_field_actor_slots[i].track_mask = 0;
                    field_finalize_actor_animation(&g_field_actor_slots[i]);
                }
            }

            slot->unk2A = 0;
        }
        else
        {
            slot->duration = 0;
            slot->active = 0;
        }
        return 1;
    }
    return 0;
}

/**
 * @brief Evaluate an animation parameter curve at an explicit frame.
 * @param slot Animation actor slot holding the curves.
 * @param curve_index Parameter curve index.
 * @param frame Frame at which to evaluate the curve.
 * @return Curve value; random curves scale the value by rand() / 32768.
 * @see decomp.me (100%) https://decomp.me/scratch/7d7kv
 */
u32 field_evaluate_parameter_track_at_time(FieldActorSlot* slot, s32 curve_index, s32 frame)
{
    FieldParameterCurve* curve;
    s32 segment_mask;
    u16* segment;
    s32 segments_remaining;
    s32 segment_start_frame;
    s32 start_delta;
    s32 value_range;
    u16 segment_word;
    s32 randomized_result;
    s32 interpolated_delta;
    s32 segment_mask_copy;
    s32 result;
    s32 sample_frame;

    segment_start_frame = 0;
    start_delta = 0;
    sample_frame = frame;
    curve = &slot->curves[curve_index];

    segments_remaining = curve->head.bytes.segment_count & FIELD_CURVE_SEGMENT_COUNT_MASK;
    segment = &slot->curve_segments[curve->head.bytes.segment_offset];

    if (segments_remaining != 0)
    {
        do
        {
            u16 current = *segment;
            s32 segment_end_frame = segment_start_frame + (current & FIELD_SEGMENT_LENGTH_MASK);

            if (sample_frame < segment_end_frame)
            {
                break;
            }

            segment_start_frame = segment_end_frame;
            start_delta = current >> FIELD_SEGMENT_VALUE_SHIFT;
            segment++;
        } while ((--segments_remaining) != 0);
    }

    value_range = curve->end_value - curve->start_value;
    segment_mask = FIELD_SEGMENT_LENGTH_MASK;
    start_delta = (value_range * start_delta) >> 5;
    segment_mask_copy = segment_mask;

    if (segments_remaining == 0)
    {
        return curve->start_value;
    }

    if ((curve->head.word & FIELD_CURVE_RANDOM) && (g_field_text_session_active == 0))
    {
        s32 random_value = rand();

        segment_word = *segment;
        interpolated_delta = ((((value_range * (segment_word >> FIELD_SEGMENT_VALUE_SHIFT)) >> 5) - start_delta) * (sample_frame - segment_start_frame)) / (segment_word & segment_mask_copy);
        /* The unused second computation of the scaled delta keeps the target's register use. */
        randomized_result = curve->start_value + (((start_delta + interpolated_delta) * random_value) >> 15);
        result = ((start_delta + interpolated_delta) * random_value) >> 15;
        return randomized_result;
    }
    else
    {
        segment_word = *segment;
        interpolated_delta = ((((value_range * (segment_word >> FIELD_SEGMENT_VALUE_SHIFT)) >> 5) - start_delta) * (sample_frame - segment_start_frame)) / (segment_word & segment_mask);
        result = interpolated_delta;
        return (curve->start_value + start_delta) + result;
    }
}

/**
 * @brief Reset the track frames, frame counters and part effect counts of an animation actor slot.
 * @param slot Animation actor slot to reset.
 * @see decomp.me (100%) https://decomp.me/scratch/eRVUu
 */
static void field_reset_actor_track_state(FieldActorSlot* slot)
{
    s32 i;
    s32 j;

    for (j = FIELD_ACTOR_TRACK_COUNT - 1; j >= 0; j--)
    {
        slot->track_frames[j] = 0;
    }

    slot->frame_counter = 0;
    slot->unk234 = 0;

    for (i = 0; i < slot->part_count; i++)
    {
        slot->parts[i].unk32 = i;
        for (j = 0; j < FIELD_ACTOR_TRACK_COUNT; j++)
        {
            slot->unk2B[i] = (slot->effect_totals[j][i] = (slot->effect_counts[j][i] = 0));
        }
    }
}

/**
 * @brief Advance the active tracks of an animation actor slot and start the next track when due.
 * @param slot Animation actor slot to advance.
 * @see decomp.me (100%) https://decomp.me/scratch/PjqLA
 */
static void field_advance_actor_tracks(FieldActorSlot* slot)
{
    s32 track_index;

    if (field_finalize_actor_animation(slot) == 0)
    {
        for (track_index = 0; track_index < slot->target_count; track_index++)
        {
            if (((s32)slot->track_mask >> track_index) & 1)
            {
                slot->track_frames[track_index]++;
            }
        }

        slot->frame_counter++;

        if (slot->track_interval != 0 && slot->target_count != 0)
        {
            if ((slot->frame_counter % slot->track_interval) == 0)
            {
                s32 next_track_index = slot->frame_counter / slot->track_interval;

                if (next_track_index < slot->target_count)
                {
                    slot->track_mask |= (1 << next_track_index);
                }
            }
        }
    }
}

/**
 * @brief Initialize the animation actor slots and release every object from animation visibility control.
 * @see decomp.me (100%) https://decomp.me/scratch/3KrRM
 */
void field_initialize_actor_slots(void)
{
    s32 object_index;
    s32 slot_index;
    FieldActorSlot* slot;

    for (object_index = 0; object_index < FIELD_ACTOR_COUNT; object_index++)
    {
        g_field_object_states[object_index].contact.word &= ~FIELD_CONTACT_ANIMATION_HIDDEN;
    }
    slot = g_field_actor_slots;
    for (slot_index = 0; slot_index < FIELD_ACTOR_SLOT_TOTAL; slot_index++, slot++)
    {
        slot->slot_index = slot_index;
        slot->part_count = 0;
        slot->active = 0;
        slot->duration = 0;
        slot->track_interval = 0;
    }
}

/**
 * @brief Stop every animation actor slot.
 * @see decomp.me (100%) https://decomp.me/scratch/0JqYo
 */
void field_clear_actor_slots(void)
{
    s32 slot_index = 0;
    FieldActorSlot* slot = g_field_actor_slots;

    do
    {
        slot_index += 1;
        slot->active = 0;
        slot->target_count = 0;
        slot->track_mask = 0;
        slot->pending_track_mask = 0;
        slot->sound_flags[0] = 0;
        slot->sound_flags[1] = 0;
        slot++;
    } while (slot_index < FIELD_ACTOR_SLOT_TOTAL);
}

/**
 * @brief Start the animation of an animation actor slot, one track per target.
 * @param slot_index Animation actor slot to start.
 * @param target_count Number of entries in @p targets; zero runs one track without a target.
 * @param targets Target object indices, one per FIELD_TARGET_STRIDE bytes.
 * @see decomp.me (100%) https://decomp.me/scratch/BUS6C
 */
void field_start_actor_animation(s32 slot_index, s32 target_count, u8* targets)
{
    s32 i;
    s32 k;
    s32 j;
    s32 m;
    FieldActorSlot* slot;
    u8* target_cursor;

    m = slot_index;
    slot = &g_field_actor_slots[m];
    slot->sound_flags[1] = 0;
    slot->sound_flags[0] = 0;
    if (slot->part_count == 0)
    {
        return;
    }
    slot->animation->unk14 &= ~FIELD_ANIM_STARTED;
    slot->track_mask = 0;
    slot->pending_track_mask = 1;
    slot->targets[0] = 0;
    slot->target_count = target_count;
    slot->status.parts.hiding_objects = 0;
    if (target_count != 0)
    {
        target_cursor = targets;
        j = 0;
        i = 0;
        if (target_count > 0)
        {
            do
            {
                slot->targets[j] = *target_cursor;
                j++;
                target_cursor += FIELD_TARGET_STRIDE;
                i++;
            } while (i < target_count);
        }
        if (j != 0)
        {
            slot->target_count = j;
        }
        else
        {
            field_finalize_actor_animation(slot);
            return;
        }
    }
    else
    {
        slot->target_count = 1;
        slot->targets[0] = FIELD_TARGET_NONE;
    }
    field_clear_actor_effects(slot);
    for (i = FIELD_ACTOR_TRACK_COUNT - 1; i >= 0; i--)
    {
        slot->track_frames[i] = 0;
    }

    slot->frame_counter = 0;
    slot->unk234 = 0;
    for (k = 0; k < slot->part_count; k++)
    {
        slot->parts[k].unk32 = k;
        for (m = 0; m < FIELD_ACTOR_TRACK_COUNT; m++)
        {
            slot->unk2B[k] = (slot->effect_totals[m][k] = (slot->effect_counts[m][k] = 0));
        }
    }

    field_dispatch_actor_audio_event(slot, FIELD_SOUND_EVENT_START, 0);
    for (i = 0; i < slot->part_count; i++)
    {
        slot->parts[i].unk32 = i;
        for (j = 0; j < FIELD_ACTOR_TRACK_COUNT; j++)
        {
            slot->unk2B[i] = (slot->effect_totals[j][i] = (slot->effect_counts[j][i] = 0));
        }
    }
}

/**
 * @brief Play the sounds an animation attaches to an event.
 * @param slot Animation actor slot that raised the event.
 * @param event_type Event type (FIELD_SOUND_EVENT_START when the animation starts).
 * @param event_subtype Event value compared for events other than the start.
 * @see decomp.me (100%) https://decomp.me/scratch/XDOcQ
 */
void field_dispatch_actor_audio_event(FieldActorSlot* slot, s32 event_type, s32 event_subtype)
{
    FieldActor* actors;
    s32 i;
    u16 sound_command;
    s32 pan;
    FieldActor* owner;
    u8 resource_index;

    for (i = 0; i < FIELD_SOUND_EVENT_COUNT; i++)
    {
        if (slot->sound_flags[i] == 0)
        {
            if (slot->animation->sound_events[i] == event_type)
            {
                if (event_type != FIELD_SOUND_EVENT_START)
                {
                    if (slot->animation->sound_subtypes[i] != event_subtype)
                    {
                        continue;
                    }
                }
                pan = field_get_actor_sound_pan(slot->owner_object_index);
                sound_command = FIELD_SOUND_COMMAND(slot->animation, i);
                switch (sound_command >> FIELD_SOUND_KIND_SHIFT)
                {
                case 0:
                    field_play_sound(sound_command & FIELD_SOUND_ID_MASK, pan);
                    break;

                case 1:
                    if (slot->owner_object_index < FIELD_PLAYER_COUNT)
                    {
                        field_play_weapon_sfx(sound_command & FIELD_SOUND_ID_MASK, pan, slot->owner_object_index);
                    }
                    else
                    {
                        actors = g_field_actors;
                        owner = &actors[slot->owner_object_index];
                        resource_index = owner->resource_index;
                        if (resource_index >= FIELD_RESOURCE_VOICE_FIRST && resource_index < FIELD_RESOURCE_VOICE_FIRST + FIELD_RESOURCE_VOICE_COUNT)
                        {
                            if (D_800FF59C != 0)
                            {
                                field_play_set_sfx(sound_command & FIELD_SOUND_ID_MASK, pan, 0, slot->owner_object_index);
                            }
                            else
                            {
                                field_play_set_sfx(sound_command & FIELD_SOUND_ID_MASK, pan, owner->resource_index - FIELD_RESOURCE_VOICE_FIRST, slot->owner_object_index);
                            }
                        }
                    }
                    break;

                case 2:
                    if ((sound_command & FIELD_SOUND_ID_MASK) < 2)
                    {
                        field_play_sfx_buffer(slot->sound_params[sound_command & FIELD_SOUND_ID_MASK], pan, slot->owner_object_index);
                    }
                    break;
                }

                if (event_type == FIELD_SOUND_EVENT_ONCE)
                {
                    slot->sound_flags[i] |= FIELD_SOUND_ONCE_PLAYED;
                }
                slot->sound_flags[i] |= FIELD_SOUND_PLAYED;
            }
        }
    }
}

/**
 * @brief Check whether an animation actor slot is running or about to start.
 * @param slot_index Animation actor slot to inspect.
 * @return Nonzero while the slot has active or pending tracks.
 * @see decomp.me (100%) https://decomp.me/scratch/tXpD1
 */
s32 field_is_actor_animation_active(s32 slot_index)
{
    FieldActorSlot* slot;

    slot = &g_field_actor_slots[slot_index];
    return (slot->track_mask | slot->pending_track_mask) != 0;
}

/**
 * @brief Find an active, unlinked animation actor slot playing a special attack (animations 0x1F..0x23).
 * @return The owning object index ORed with FIELD_SPECIAL_ATTACK_RESULT, or zero if none.
 * @see decomp.me (100%) https://decomp.me/scratch/8lvHC
 */
s32 field_find_active_special_attack_actor(void)
{
    s32 animation_id_limit;
    FieldActorSlot* slot;
    s32 slot_index;
    s32 first_animation_id;
    u16 animation_id;

    slot = g_field_actor_slots;
    for (slot_index = 0; slot_index < FIELD_ACTOR_SLOT_TOTAL; slot_index++)
    {
        first_animation_id = FIELD_SPECIAL_ATTACK_FIRST;
        if ((slot->active != 0) && (!(slot->status.word & FIELD_SLOT_OWNER_LINKED)))
        {
            animation_id_limit = FIELD_SPECIAL_ATTACK_END;
            animation_id = slot->status.parts.animation_id;
            if ((animation_id < animation_id_limit) && (animation_id >= first_animation_id))
            {
                return slot->owner_object_index | FIELD_SPECIAL_ATTACK_RESULT;
            }
        }
        slot++;
    }

    return 0;
}

/**
 * @brief Advance every active animation actor slot by one frame.
 *
 * Runs the attack-sphere hit checks, frame sounds and part effects, advances the
 * track frames, then evaluates the two render-state tracks into the field render state.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/etUW8
 */
void field_update_actor_animations(void)
{
    ControllerState* controller = CONTROLLER_STATE;
    FieldActorSlot* slot;
    FieldAnimationDef* animation;
    s32 track_index;
    s32 slot_index;
    u8* vibration_curves;
    s32 i;
    s32 curve_index;
    u16 frame_sound;
    u32 frame_counter;
    u32 track_interval;

    slot = g_field_actor_slots;
    slot_index = 0;
    controller->ports[1].actuator_control.fields.large_motor_command = 0U;
    controller->ports[0].actuator_control.fields.large_motor_command = 0U;
    do
    {
        if (slot->duration != 0)
        {
            if (slot->pending_track_mask != 0)
            {
                field_reset_actor_track_mask(slot);
            }
            if (slot->track_mask != 0)
            {
                field_advance_actor_effects(slot);
                if (slot->animation->unk14 == FIELD_ANIM_ATTACK)
                {
                    if ((&slot->parts[slot->animation->unk15])->flags & FIELD_PART_ATTACK_SPHERE)
                    {
                        for (track_index = 0; track_index < slot->target_count; track_index++)
                        {
                            u16 track_frame = slot->track_frames[track_index];

                            if (((&slot->parts[slot->animation->unk15])->unk31 < track_frame) &&
                                (slot->track_frames[track_index] <
                                 ((&slot->parts[slot->animation->unk15])->unk31 + (&slot->parts[slot->animation->unk15])->unkD)))
                            {
                                g_field_track_index = track_index;
                                field_collect_attack_sphere_hits(slot, &slot->parts[slot->animation->unk15]);
                            }
                        }
                    }
                    else if ((&slot->parts[slot->animation->unk15])->unk31 == FIELD_PART_ALWAYS)
                    {
                        for (track_index = 0; track_index < slot->target_count; track_index++)
                        {
                            if ((&slot->parts[slot->animation->unk15])->unkD > slot->track_frames[track_index])
                            {
                                g_field_track_index = track_index;
                                field_collect_attack_sphere_hits(slot, &slot->parts[slot->animation->unk15]);
                            }
                        }
                    }
                    else if (((&slot->parts[slot->animation->unk15])->unk31 < slot->track_frames[0]) &&
                             (slot->track_frames[0] < ((&slot->parts[slot->animation->unk15])->unk31 + (&slot->parts[slot->animation->unk15])->unkD)))
                    {
                        g_field_track_index = 0;
                        field_collect_attack_sphere_hits(slot, &slot->parts[slot->animation->unk15]);
                    }
                }
                animation = slot->animation;
                frame_sound = animation->frame_sound.word;
                if ((frame_sound & FIELD_ANIM_FRAME_SOUND) && (animation->frame_sound.bytes[0] == slot->track_frames[0]))
                {
                    func_8005A67C((frame_sound >> 8) & 0x7F, 0);
                }
                field_update_actor_effects(slot);
                if (field_finalize_actor_animation(slot) == 0)
                {
                    for (i = 0; i < slot->target_count; i++)
                    {
                        if ((slot->track_mask >> i) & 1)
                        {
                            slot->track_frames[i] += 1;
                        }
                    }

                    slot->frame_counter += 1;
                    if ((slot->track_interval != 0) && (slot->target_count != 0))
                    {
                        frame_counter = slot->frame_counter;
                        track_interval = slot->track_interval;
                        if ((frame_counter % track_interval) == 0)
                        {
                            i = (frame_counter / track_interval) & 0xFFFF;
                            if (i < slot->target_count)
                            {
                                slot->track_mask |= 1 << i;
                            }
                        }
                    }
                }
                g_field_track_index = 0;
                for (track_index = 0; track_index < FIELD_VIBRATION_TRACK_COUNT; track_index++)
                {
                    u8 curve_selector = (vibration_curves = slot->animation->vibration_curves)[track_index];

                    if ((vibration_curves[track_index] != FIELD_CURVE_NONE) && (curve_selector < (u32)FIELD_CURVE_COUNT))
                    {
                        if (track_index != 0)
                        {
                            u8 combined_value = field_evaluate_parameter_track(slot, curve_selector & 0xF) | controller->ports[0].actuator_control.fields.large_motor_command;

                            controller->ports[0].actuator_control.fields.large_motor_command = combined_value;
                            controller->ports[1].actuator_control.fields.large_motor_command = combined_value;
                        }
                        else
                        {
                            u8 base_value = field_evaluate_parameter_track(slot, (curve_index = vibration_curves[0]) & 0xF);

                            controller->ports[0].small_motor_command = (controller->ports[1].small_motor_command = base_value);
                        }
                    }
                }
            }
            slot->sound_flags[0] &= ~FIELD_SOUND_PLAYED;
            slot->sound_flags[1] &= ~FIELD_SOUND_PLAYED;
        }
        slot_index += 1;
        slot += 1;
    } while (slot_index < FIELD_ACTOR_SLOT_TOTAL);
}

/**
 * @brief Start the tracks of a pending animation: the first one, or all of them without a track interval.
 * @param slot Animation actor slot to start.
 * @see decomp.me (100%) https://decomp.me/scratch/VZWgF
 */
static void field_reset_actor_track_mask(FieldActorSlot* slot)
{
    s32 i;
    u16 track_interval = slot->track_interval;

    /* Without this increment/decrement pair the lhu and the constant 1 swap registers. */
    track_interval++;
    track_interval--;

    slot->track_mask = 1;
    if (track_interval == 0)
    {
        slot->track_mask = 0;
        for (i = 0; i < slot->target_count; i++)
        {
            slot->track_mask |= 1 << i;
        }
    }
    slot->pending_track_mask = 0;
}

/**
 * @brief Render the field effects, then apply the animation actors' render effects.
 * @param render_context Field render context.
 * @param unused Forwarded to field_build_actor_render_commands().
 * @see decomp.me (100%) https://decomp.me/scratch/hvTSS
 */
void field_prepare_actor_render_commands(s32 render_context, s32 unused)
{
    field_render_effects((FieldRenderContext*)render_context);
    field_build_actor_render_commands((FieldRenderContext*)render_context, unused);
}

/**
 * @brief Apply animation actor render effects and emit full-screen tint packets.
 *
 * The first pass drives the owner and target object visibility from the animation
 * tracks, the second evaluates the camera offset tracks, and the third emits a
 * semi-transparent screen tint (or sets the global colour scale) for every active
 * slot with a colour track.
 *
 * @param render_ctx Field render context and packet cursor.
 * @param unused Unused; forwarded by field_prepare_actor_render_commands().
 * @see decomp.me (100%) https://decomp.me/scratch/Sgd61
 */
static void field_build_actor_render_commands(FieldRenderContext* render_ctx, s32 unused)
{
    u32 packed_color;
    u32* ordering_table = &render_ctx->ordering_table;
    u32* packet = (u32*)render_ctx->packet_cursor;
    FieldActorSlot* slot = g_field_actor_slots;
    s32 slot_index = 0;
    CVECTOR color;
    s32 blend_mode;
    s32 has_target_track;

    for (; slot_index < FIELD_ACTOR_SLOT_TOTAL; slot_index++, slot++)
    {
        if (slot->track_mask != 0)
        {
            u16 animation_flags = slot->animation->unk18;

            if ((animation_flags & FIELD_ANIM_OWNER_VISIBILITY) && !(slot->animation->unk18 & FIELD_ANIM_OWNER_TRACK_OFF))
            {
                u8 visibility;
                s32 track_value;

                g_field_track_index = 0;
                track_value = field_evaluate_parameter_track(slot, (slot->animation->unk18 >> FIELD_ANIM_OWNER_CURVE_SHIFT) & 0xF);
                visibility = 0;
                if (track_value != 0)
                {
                    visibility = FIELD_ANIMATION_HIDDEN;
                }
                if (visibility != 0)
                {
                    g_field_actors[slot->owner_object_index].presence = visibility;
                    g_field_object_states[slot->owner_object_index].contact.word |= FIELD_CONTACT_ANIMATION_HIDDEN;
                    g_field_object_states[slot->owner_object_index].contact.bytes.controller_index = slot->slot_index;
                }
                else
                {
                    u8 owner_index = slot->owner_object_index;
                    s16 owner_command = g_field_actors[owner_index].command;

                    if (((owner_command != FIELD_ACTOR_COMMAND_DEFEATED) && (owner_command != FIELD_ACTOR_COMMAND_DEFEAT_END)) ||
                        (g_field_object_states[owner_index].flags & FIELD_OBJECT_FLAG_KNOCKED_OUT))
                    {
                        g_field_actors[slot->owner_object_index].presence = visibility;
                    }
                }
            }
            has_target_track = slot->animation->unk18 & FIELD_ANIM_TARGET_VISIBILITY;
            if (has_target_track && !(slot->animation->unk18 & FIELD_ANIM_TARGET_TRACK_OFF))
            {
                s32 target_index;

                for (target_index = 0; target_index < slot->target_count; target_index++)
                {
                    u8 visibility;
                    s32 track_value;

                    g_field_track_index = target_index;
                    track_value = field_evaluate_parameter_track(slot, slot->animation->unk18 >> FIELD_ANIM_TARGET_CURVE_SHIFT);
                    visibility = 0;
                    if (track_value != 0)
                    {
                        visibility = FIELD_ANIMATION_HIDDEN;
                    }
                    if (slot->targets[target_index] != FIELD_TARGET_NONE)
                    {
                        if (visibility != 0)
                        {
                            g_field_actors[slot->targets[target_index]].presence = visibility;
                            g_field_object_states[slot->targets[target_index]].contact.word |= FIELD_CONTACT_ANIMATION_HIDDEN;
                            g_field_object_states[slot->targets[target_index]].contact.bytes.controller_index = slot->slot_index;
                            slot->status.parts.hiding_objects = 1;
                        }
                        else
                        {
                            g_field_actors[slot->targets[target_index]].presence = 0;
                        }
                    }
                }
            }
        }
    }

    g_field_track_index = 0;
    slot = g_field_actor_slots;
    slot_index = 0;
    for (; slot_index < FIELD_ACTOR_SLOT_TOTAL; slot_index++, slot++)
    {
        if (slot->track_mask != 0)
        {
            FieldAnimationDef* animation = slot->animation;

            if (animation->flags & FIELD_ANIM_CAMERA_OFFSET)
            {
                s32 offset_mode = animation->flags >> FIELD_ANIM_CAMERA_MODE_SHIFT;

                switch (offset_mode & 3)
                {
                case 0:
                    g_field_camera_offset_x = field_evaluate_parameter_track(slot, animation->unk10 >> FIELD_ANIM_CAMERA_CURVE_SHIFT);
                    break;

                case 1:
                    g_field_camera_offset_y = field_evaluate_parameter_track(slot, animation->unk10 >> FIELD_ANIM_CAMERA_CURVE_SHIFT);
                    break;

                case 2:
                    g_field_camera_offset_y = g_field_camera_offset_x = field_evaluate_parameter_track(slot, animation->unk10 >> FIELD_ANIM_CAMERA_CURVE_SHIFT);
                    break;

                case 3:
                    g_field_camera_offset_x = field_evaluate_parameter_track(slot, animation->unk10 >> FIELD_ANIM_CAMERA_CURVE_SHIFT);
                    g_field_camera_offset_y = field_evaluate_parameter_track(slot, ((slot->animation->unk10 >> FIELD_ANIM_CAMERA_CURVE_SHIFT) + 1) & 0xF);
                    break;
                }
            }
        }
    }

    slot = g_field_actor_slots;
    slot_index = 0;
    for (; slot_index < FIELD_ACTOR_SLOT_TOTAL; slot_index++, slot++)
    {
        if ((slot->track_mask != 0) && (slot->active != 0))
        {
            FieldAnimationDef* animation = slot->animation;
            u8 curve_selector = (u8)animation->flags;

            if ((u8)animation->flags < FIELD_CURVE_COUNT)
            {
                if (((animation->flags >> 8) & (FIELD_ANIM_COLOR_RGB >> 8)) != 0)
                {
                    color.r = field_evaluate_parameter_track(slot, (u8)animation->flags);
                    color.g = field_evaluate_parameter_track(slot, ((u8)slot->animation->flags + 1) & 0xF);
                    color.b = field_evaluate_parameter_track(slot, ((u8)slot->animation->flags + 2) & 0xF);
                }
                else
                {
                    color.r = color.g = color.b = field_evaluate_parameter_track(slot, curve_selector & 0xF);
                }
                if ((slot->animation->flags >> 8) & (FIELD_ANIM_GLOBAL_COLOR >> 8))
                {
                    field_set_global_color_scale(color.r * 2, color.g * 2, color.b * 2);
                }
                else
                {
                    TILE* tile;
                    DR_TPAGE* tpage;

                    if ((slot->animation->flags >> 8) & (FIELD_ANIM_COLOR_RGB >> 8))
                    {
                        if ((color.r == 0) && (color.g == 0) && (color.b == 0))
                        {
                            continue;
                        }
                    }
                    else if (color.r == 0)
                    {
                        continue;
                    }
                    tile = (TILE*)packet;
                    packed_color = *(u32*)&color;
                    setlen(tile, 3);
                    tile->w = SCREEN_WIDTH;
                    tile->h = SCREEN_HEIGHT;
                    tile->y0 = 0;
                    tile->x0 = 0;
                    *(u32*)&tile->r0 = packed_color;
                    setcode(tile, FIELD_TINT_TILE_CODE);
                    addPrim(ordering_table, tile);
                    blend_mode = (((slot->animation->flags >> FIELD_ANIM_BLEND_SHIFT) & 3) + 1) & 3;
                    packet += sizeof(TILE) / sizeof(u32);
                    tpage = (DR_TPAGE*)packet;
                    setDrawTPage(tpage, 0, 0, getTPage(0, blend_mode, 320, 0));
                    addPrim(ordering_table, tpage);
                    packet += sizeof(DR_TPAGE) / sizeof(u32);
                }
            }
        }
    }
    field_apply_global_color_scale();
    render_ctx->packet_cursor = (s32*)packet;
}

/**
 * @brief Reset the global actor colour scale to neutral.
 * @see decomp.me (100%) https://decomp.me/scratch/kl6PF
 */
void field_reset_global_color_scale(void)
{
    g_field_color_scale.s.red = FIELD_COLOR_SCALE_NEUTRAL;
    g_field_color_scale.s.green = FIELD_COLOR_SCALE_NEUTRAL;
    g_field_color_scale.s.blue = FIELD_COLOR_SCALE_NEUTRAL;
    g_field_color_scale_active = 0;
}

/**
 * @brief Set the global actor colour scale.
 * @param red Red scale component.
 * @param green Green scale component.
 * @param blue Blue scale component.
 * @see decomp.me (100%) https://decomp.me/scratch/iwb2J
 */
void field_set_global_color_scale(s16 red, s16 green, s16 blue)
{
    g_field_color_scale.s.red = red;
    g_field_color_scale.s.green = green;
    g_field_color_scale.s.blue = blue;
}

/**
 * @brief Push the global actor colour scale to the actors while it differs from neutral.
 * @see decomp.me (100%) https://decomp.me/scratch/eDBPu
 */
static void field_apply_global_color_scale(void)
{
    u8* active = &g_field_color_scale_active;

    if (*active != 0)
    {
        func_8005A0D0(-1, g_field_color_scale.s.red, g_field_color_scale.s.green, g_field_color_scale.s.blue);
        if (g_field_color_scale.w == FIELD_COLOR_SCALE_NEUTRAL_RG && g_field_color_scale.s.blue == FIELD_COLOR_SCALE_NEUTRAL)
        {
            *active = 0;
        }
    }
    else
    {
        FieldColorScale* scale = &g_field_color_scale;

        if (scale->w != FIELD_COLOR_SCALE_NEUTRAL_RG || scale->s.blue != FIELD_COLOR_SCALE_NEUTRAL)
        {
            func_8005A0D0(-1, scale->s.red, scale->s.green, scale->s.blue);
            *active = 1;
        }
    }
}

/**
 * @brief Forget the loaded sprite packages and cached portraits of the party and drop the companion.
 * @see decomp.me (100%) https://decomp.me/scratch/a1YEc
 */
void field_reset_actor_resource_slots(void)
{
    g_field_player_records[0].resource_id = 0;
    g_field_player_records[1].resource_id = 0;
    g_field_player_records[2].resource_id = 0;

    g_field_player_records[0].portrait_index = 0xFF;
    g_field_player_records[1].portrait_index = 0xFF;
    g_field_player_records[2].portrait_index = 0xFF;

    g_field_player_records[0].head.bits.alt_appearance = 0;
    g_field_player_records[1].head.bits.active = 0;
    g_field_player_records[2].head.bits.active = 0;
}

/**
 * @brief Reset the actor tables and load or relocate the sprite packages of the party.
 * @see decomp.me (100%) https://decomp.me/scratch/0wUsT
 */
void field_initialize_actor_system(void)
{
    s32 i;
    s32 j;
    u32 work_value;
    s32 companion_index;
    u8* slot_base;
    FieldAnimationDef* default_animation;
    FieldObjectPart* part;
    u8* slot_column_base;
    FieldActorSlot* slot;
    u8* table_cursor;
    u8* row_cursor;
    s32 control_flags;
    u8* scratch_base;
    s32 column_offset;
    u8* byte_cursor;
    u8* pad_base;
    s32 control_flags_alt;
    u8* player_block;
    u8* slot_base_alias;
    u32 dest_addr;

    g_field_text_session_active = 0;
    D_8012291C = 0;

    for (i = 0; i < FIELD_ACTOR_COUNT; i++)
    {
        g_field_actors[i].presence = FIELD_ACTOR_UNUSED;
        g_field_actors[i].x = FIELD_ACTOR_UNPLACED_X;
        g_field_actors[i].y = 0;
        g_field_actors[i].z = 0;
    }

    bcopy(g_field_resource_buffer, FIELD_RESOURCE_BACKUP, FIELD_RESOURCE_BACKUP_SIZE);
    D_800FE774 = 0;
    g_field_resource_cursor = g_field_resource_buffer;

    for (i = 0; i < FIELD_RESOURCE_ENTRY_COUNT; i++)
    {
        g_field_resource_entries[i].flags &= ~FIELD_RESOURCE_LOADED;
    }

    if (g_field_scene_mode_bit != 0)
    {
        for (j = 0; j < FIELD_PARTY_COUNT; j++)
        {
            if (g_field_player_records[j].head.bytes.flags & FIELD_PLAYER_ACTIVE)
            {
                D_800FE774++;
                if (j == FIELD_COMPANION_INDEX)
                {
                    work_value = field_get_actor_resource_id(FIELD_COMPANION_INDEX, &g_field_player_records[FIELD_COMPANION_INDEX], 1);
                    i = work_value;
                }
                else
                {
                    i = field_get_actor_resource_id(j, &g_field_player_records[j], 0);
                }
                if (i != g_field_player_records[j].resource_id)
                {
                    g_field_player_records[j].resource_id = i;
                    if (j == FIELD_COMPANION_INDEX)
                    {
                        field_load_actor_resource_slot(FIELD_COMPANION_INDEX, FIELD_COMPANION_INDEX, i, 1);
                    }
                    else
                    {
                        field_load_actor_resource_slot(j, j, i, 0);
                    }
                }
                else
                {
                    field_relocate_resource_buffer(j);
                    companion_index = FIELD_COMPANION_INDEX;
                    if (j == companion_index)
                    {
                        g_field_resource_entries[companion_index].flags = g_field_resource_entries[companion_index].flags | FIELD_RESOURCE_HAS_ACTIONS;
                    }
                }
                field_initialize_actor_record(j, j);

                pad_base = (u8*)g_pad_ctx;
                control_flags = g_field_actors[j].control.word & (~FIELD_CONTROL_MODE_MASK);
                player_block = pad_base + (j * FIELD_SAVED_PLAYER_STRIDE);
                g_field_actors[j].control.word = control_flags | ((player_block[FIELD_SAVED_PLAYER_CONTROL] >> 7) ^ 1);

                if (j < FIELD_PLAYER_COUNT)
                {
                    field_load_weapon_sfx_table(j, g_field_player_records[j].head.bytes.weapon_type);
                }
            }
        }
    }
    else
    {
        for (j = 0; j < FIELD_PARTY_COUNT; j++)
        {
            if ((g_field_player_records[j].head.bytes.flags & FIELD_PLAYER_ACTIVE) != 0)
            {
                D_800FE774++;
                i = field_get_actor_resource_id(j, &g_field_player_records[j], 0);
                if (i != g_field_player_records[j].resource_id)
                {
                    g_field_player_records[j].resource_id = i;
                    field_load_actor_resource_slot(j, j, i, 0);
                }
                else
                {
                    field_relocate_resource_buffer(j);
                }
                field_initialize_actor_record(j, j);

                pad_base = (u8*)g_pad_ctx;
                control_flags_alt = g_field_actors[j].control.word & (~FIELD_CONTROL_MODE_MASK);
                player_block = pad_base + (j * FIELD_SAVED_PLAYER_STRIDE);
                g_field_actors[j].control.word = control_flags_alt | ((player_block[FIELD_SAVED_PLAYER_CONTROL] >> 7) ^ 1);
            }
        }
    }

    field_apply_weapon_action_params(0);
    if (D_800FDA80 & FIELD_PLAYER_ACTIVE)
    {
        field_apply_weapon_action_params(1);
    }

    g_field_object_default_animation.unk14 = 0;
    i = 0;
    slot_base = (u8*)g_field_actor_slots;
    default_animation = &g_field_object_default_animation;
    part = g_field_object_parts;
    j = FIELD_OBJECT_SLOT_BASE * sizeof(FieldActorSlot);

    for (; i < FIELD_ACTOR_COUNT; i++)
    {
        slot = (FieldActorSlot*)(((u32)j) + ((u32)slot_base));
        slot->parts = part;
        slot->animation = default_animation;
        field_initialize_actor_part(i, 0);
        part++;
        j += sizeof(FieldActorSlot);
    }

    field_restore_default_action_animation_mappings(0);

    /* Clears the part effect counts of the last animation slot (g_field_actor_slots[79]). */
    for (i = 0; i < FIELD_ACTOR_PART_COUNT; i++)
    {
        j = 0;
        slot_column_base = ((u8*)g_field_actor_slots) + i;
        byte_cursor = slot_column_base + 0xB327;
        column_offset = i * 2;
        work_value = 0xB3C8;

        {
            u8* base = (u8*)g_field_actor_slots;

            slot_base_alias = base;
            scratch_base = slot_base_alias;
            table_cursor = scratch_base;
            row_cursor = table_cursor;
        }

        for (; j < FIELD_ACTOR_TRACK_COUNT; j++)
        {
            scratch_base = row_cursor + 0xB337;
            dest_addr = column_offset;
            dest_addr += (u32)(table_cursor + work_value);
            *((u16*)dest_addr) = (*(i + scratch_base) = 0);
            row_cursor += FIELD_ACTOR_PART_COUNT;
            table_cursor += FIELD_ACTOR_PART_COUNT * sizeof(u16);
            *byte_cursor = 0;
        }
    }

    /* Unprototyped call: the four loop registers are passed along in $a0-$a3. */
    field_refresh_actor_portraits(table_cursor, row_cursor, column_offset, byte_cursor);
}

/**
 * @brief Copy a party member's sprite package back from the resource backup to the arena cursor.
 * @param resource_index Resource entry (and actor) of the party member.
 */
static void field_relocate_resource_buffer(s32 resource_index)
{
    u32 resource_size;
    FieldActor* actor;
    u8* old_start;
    void** cursor_ref;
    u8* source_base;
    u8* buffer_base;

    buffer_base = g_field_resource_buffer;
    /* Without this increment/decrement pair the buffer base is not kept in its own register. */
    buffer_base++;
    buffer_base--;
    source_base = FIELD_RESOURCE_BACKUP;
    cursor_ref = &g_field_resource_cursor;
    /* A no-op store the target performs (a word load and store of the entry start). */
    *(u32*)&g_field_resource_entries[resource_index].start += 0;
    bcopy((void*)(source_base - (u32)buffer_base + (u32)g_field_resource_entries[resource_index].start), *cursor_ref,
          g_field_resource_entries[resource_index].end - g_field_resource_entries[resource_index].start);
    resource_size = (u32)g_field_resource_entries[resource_index].end;
    old_start = g_field_resource_entries[resource_index].start;
    resource_size -= (u32)old_start;
    g_field_resource_entries[resource_index].start = (u8*)g_field_resource_cursor;
    g_field_resource_entries[resource_index].end = ((u8*)g_field_resource_cursor) + resource_size;
    g_field_resource_entries[resource_index].flags &= ~FIELD_RESOURCE_HAS_ACTIONS;
    g_field_resource_entries[resource_index].slot_index = resource_index;
    g_field_resource_entries[resource_index].flags |= FIELD_RESOURCE_LOADED;
    g_field_resource_cursor = g_field_resource_entries[resource_index].end;
    actor = &g_field_actors[resource_index];
    /* Unprototyped call: the old start is still in $a1 and the target passes it along. */
    field_restart_actor_animation(actor, old_start);
}

/**
 * @brief Restore the default commands of actions 1 and 5 in both action command maps and enable them.
 * @see decomp.me (100%) https://decomp.me/scratch/oaoFZ
 */
void field_restore_default_action_animation_mappings(void)
{
    g_field_action_command_maps[0].commands[1] = FIELD_ACTION_COMMAND(1);
    g_field_action_command_maps[1].commands[1] = FIELD_ACTION_COMMAND(1);
    g_field_action_command_maps[0].disabled[1] = 0;
    g_field_action_command_maps[1].disabled[1] = 0;
    g_field_action_command_maps[0].disabled[5] = 0;
    g_field_action_command_maps[0].commands[5] = FIELD_ACTION_COMMAND(5);
    g_field_action_command_maps[1].disabled[5] = 0;
    g_field_action_command_maps[1].commands[5] = FIELD_ACTION_COMMAND(5);
}

/**
 * @brief Resolve the CD resource id of a party member's sprite package.
 * @param unused_slot_index Party index, unused.
 * @param player Party member record.
 * @param weapon_set Non-zero selects the weapon-specific package set.
 * @return CD resource id of the package.
 */
s32 field_get_actor_resource_id(s32 unused_slot_index, FieldPlayerRecord* player, s32 weapon_set)
{
    if (weapon_set != 0)
    {
        switch (player->character_kind)
        {
        case FIELD_PLAYER_KIND_HERO:
            if (player->head.bits.alt_appearance)
            {
                return player->head.bytes.weapon_type + FIELD_RES_HERO_ALT_WEAPON_SPRITES;
            }
            return player->head.bytes.weapon_type + FIELD_RES_HERO_WEAPON_SPRITES;

        case FIELD_PLAYER_KIND_PARTNER:
            return player->character_id + FIELD_RES_PARTNER_WEAPON_SPRITES;

        case FIELD_PLAYER_KIND_COMPANION:
        default:
            return player->character_id + FIELD_RES_COMPANION_WEAPON_SPRITES;
        }
    }
    else
    {
        switch (player->character_kind)
        {
        case FIELD_PLAYER_KIND_HERO:
            return player->head.bits.alt_appearance + FIELD_RES_HERO_SPRITES;

        case FIELD_PLAYER_KIND_PARTNER:
            return player->character_id + FIELD_RES_PARTNER_SPRITES;

        case FIELD_PLAYER_KIND_COMPANION:
        default:
            return player->character_id + FIELD_RES_COMPANION_SPRITES;
        }
    }
}

/**
 * @brief Reset the model parts of every field object.
 * @param timer_mode Non-zero gives the parts the small scale.
 */
void field_initialize_actor_parts(s32 timer_mode)
{
    s32 i;

    for (i = 0; i < FIELD_ACTOR_COUNT; i++)
    {
        field_initialize_actor_part(i, timer_mode);
    }
}

/**
 * @brief Load one sprite package into the resource arena and bind it to a resource entry.
 * @param resource_index Resource entry to fill.
 * @param slot_index Actor (and texture slot) using the package.
 * @param resource_id CD resource id of the package.
 * @param alternate_layout Low bit selects the alternate texture layout.
 */
static void field_load_actor_resource_slot(s32 resource_index, s32 slot_index, s32 resource_id, s32 alternate_layout)
{
    g_field_resource_entries[resource_index].unkE = 0x2F;
    g_field_resource_entries[resource_index].slot_index = slot_index;
    g_field_resource_entries[resource_index].unk8 = 0;
    g_field_resource_entries[resource_index].flags = (g_field_resource_entries[resource_index].flags & ~FIELD_RESOURCE_HAS_ACTIONS) | (alternate_layout & 1);
    g_field_resource_entries[resource_index].start = g_field_resource_cursor;
    /* Unprototyped call with a fourth argument the definition does not take. */
    field_load_resource_package(resource_id, slot_index, resource_index, alternate_layout & 1);
    g_field_actors[slot_index].animation &= FIELD_ANIMATION_FACING;
    field_restart_actor_animation(&g_field_actors[slot_index]);
    g_field_resource_entries[resource_index].end = g_field_resource_cursor;
    g_field_resource_entries[resource_index].flags |= FIELD_RESOURCE_LOADED;
}

/**
 * @brief Finish reloading a party member: refresh the players' control modes and the party state.
 * @param actor_slot Party index; values two and above do nothing.
 */
void field_finish_party_slot_reload(s32 actor_slot)
{
    s32 i;
    u8* pad_base;
    u8* player_block;
    u32 control_flags;
    s32 work_value;

    if (actor_slot < FIELD_PLAYER_COUNT)
    {
        func_800B08FC(0, actor_slot);
        if (func_800B0850() == 0)
        {
            pad_base = (u8*)g_pad_ctx;

            for (i = 0; i < FIELD_PLAYER_COUNT; i++)
            {
                if (g_field_player_records[i].head.bytes.flags & FIELD_PLAYER_ACTIVE)
                {
                    work_value = ~FIELD_CONTROL_MODE_MASK;
                    control_flags = g_field_actors[i].control.word & work_value;
                    work_value = i * FIELD_SAVED_PLAYER_STRIDE;
                    player_block = pad_base + work_value;
                    g_field_actors[i].control.word = control_flags | ((player_block[FIELD_SAVED_PLAYER_CONTROL] >> 7) ^ 1);
                }
            }

            field_refresh_party_routes();
            field_battle_end();
            field_reset_actor_resources();
            func_800B01FC();
        }
    }
}

/**
 * @brief Remove the partner or companion and compact the resource arena.
 * @param slot_index_minus_one Party index minus one (0 = partner, 1 = companion).
 */
void field_release_actor_resource_slot(s32 slot_index_minus_one)
{
    s32 slot_index;
    s32 i;
    u8* src;
    u8* dst;
    s32 size;
    FieldActor* actor;
    FieldActor* actors;
    FieldActor* member;
    FieldResourceEntry* entry;

    slot_index = slot_index_minus_one;
    slot_index += 1;

    if (g_field_player_records[slot_index].head.bytes.flags & FIELD_PLAYER_ACTIVE)
    {
        entry = &g_field_resource_entries[slot_index];

        /* A single-pass loop: the target fetches the actor pointer inside its own block. */
        while (g_field_player_records[slot_index].head.bytes.flags & FIELD_PLAYER_ACTIVE)
        {
            actors = g_field_actors;
            member = &actors[slot_index];
            break;
        }

        src = entry->end;
        size = src - entry->start;
        dst = entry->start;
        member->presence = FIELD_ACTOR_UNUSED;

        g_field_player_records[slot_index].portrait_index = 0xFF;

        g_field_player_records[slot_index].head.bits.active = 0;

        while (src != g_field_resource_cursor)
        {
            *dst = *src;
            src++;
            dst++;
        }

        for (i = 0; i < FIELD_RESOURCE_MOVABLE_COUNT; i++)
        {
            if (g_field_resource_entries[i].start > g_field_resource_entries[slot_index].start)
            {
                g_field_resource_entries[i].start -= size;
                g_field_resource_entries[i].end -= size;
            }
        }

        i = 0;
        actor = g_field_actors;
        do
        {
            if ((actor->presence != FIELD_ACTOR_UNUSED) && (actor->resource_index != FIELD_RESOURCE_FIXED))
            {
                if ((actor->frame_data | 0x80000000) > (((u32)g_field_resource_entries[slot_index].start) | 0x80000000))
                {
                    actor->frame_data -= size;
                }
            }

            i++;
            actor++;
        } while (i < FIELD_ACTOR_COUNT);

        g_field_resource_cursor = ((u8*)g_field_resource_cursor) - size;
        g_field_resource_entries[slot_index].flags &= ~FIELD_RESOURCE_LOADED;
    }
}

/**
 * @brief Add the partner or companion to the party and load its sprite package.
 * @param source_selector Key of an actor to take the place of, -1 to start unplaced, or -2 to copy the hero.
 * @param resource_variant Character id of the new member.
 * @param slot_index_minus_one Party index minus one (0 = partner, 1 = companion).
 * @return 1 on activation, 0 when the member is already present, or -1 when the source actor is not found.
 */
s32 field_activate_actor_resource_slot(s32 source_selector, s32 resource_variant, s32 slot_index_minus_one)
{
    s32 slot = slot_index_minus_one + 1;
    FieldActor* source_actor;
    FieldPlayerRecord* player;
    u8* pad_context;
    s32 resource_id;
    s32 control_flags;
    u8* player_block;

    if (g_field_player_records[slot].head.bytes.flags & FIELD_PLAYER_ACTIVE)
    {
        return 0;
    }

    if (source_selector >= 0)
    {
        source_actor = field_lookup_actor(source_selector);
        if (source_actor == FIELD_ACTOR_NONE)
        {
            return -1;
        }
    }

    g_field_player_records[slot].head.bits.active = 1;
    if (source_selector == -2)
    {
        g_field_player_records[slot].character_kind = FIELD_PLAYER_KIND_HERO;
    }
    else
    {
        g_field_player_records[slot].character_kind = slot;
    }

    g_field_player_records[slot].character_id = resource_variant;

    player = &g_field_player_records[slot];
    switch (player->character_kind)
    {
    case FIELD_PLAYER_KIND_HERO:
        resource_id = player->head.bits.alt_appearance + FIELD_RES_HERO_SPRITES;
        break;

    case FIELD_PLAYER_KIND_PARTNER:
        resource_id = player->character_id + FIELD_RES_PARTNER_SPRITES;
        break;

    case FIELD_PLAYER_KIND_COMPANION:
    default:
        resource_id = player->character_id + FIELD_RES_COMPANION_SPRITES;
        break;
    }

    g_field_player_records[slot].resource_id = resource_id;
    field_load_actor_resource_slot(slot, slot, resource_id, 0);
    field_initialize_actor_record(slot, slot);

    control_flags = g_field_actors[slot].control.word;
    pad_context = (u8*)g_pad_ctx;
    player_block = pad_context + slot * FIELD_SAVED_PLAYER_STRIDE;
    g_field_actors[slot].control.word = (control_flags & ~FIELD_CONTROL_MODE_MASK) | ((player_block[FIELD_SAVED_PLAYER_CONTROL] >> 7) ^ 1);

    if ((slot == FIELD_COMPANION_INDEX) && (resource_variant >= FIELD_COMPANION_COLOR_ID_MIN))
    {
        FieldActor* companion = &g_field_actors[FIELD_COMPANION_INDEX];

        companion->control.word = (companion->control.word & ~FIELD_CONTROL_VARIANT_MASK) |
                                  ((((u8)((PadContext*)pad_context)->large_history_index + 1) & 3) << FIELD_CONTROL_VARIANT_SHIFT);
    }
    else
    {
        g_field_actors[slot].control.word &= ~FIELD_CONTROL_VARIANT_MASK;
    }

    if (source_selector == -1)
    {
        g_field_actors[slot].x = 0;
        g_field_actors[slot].y = 0;
        g_field_actors[slot].z = 0;
        g_field_actors[slot].animation = 0;
    }
    else if (source_selector == -2)
    {
        FieldActor* member;
        FieldActor* hero;
        FieldDirectionOffset* offset;
        FieldDirectionOffset* offsets;
        s32 motion[3];

        hero = g_field_actors;
        member = &g_field_actors[slot];
        member->x = hero->x;
        member->y = hero->y;
        member->z = hero->z;
        offsets = g_field_direction_offsets;
        offset = &offsets[hero->direction >> 5];
        motion[0] = offset->distance;
        motion[1] = 0;
        motion[2] = offset->distance;
        field_move_actor_position(member, motion);
        member->animation = 0;
    }
    else
    {
        g_field_actors[slot].x = source_actor->x;
        g_field_actors[slot].y = source_actor->y;
        g_field_actors[slot].z = source_actor->z;
        g_field_actors[slot].animation = 0;
        source_actor->presence = FIELD_ACTOR_UNUSED;
        D_800FE774--;
    }

    g_field_actors[slot].presence = 0;
    g_field_actors[slot].command = FIELD_ACTOR_COMMAND_NONE;
    g_field_actors[slot].unk10 = 0;
    g_field_actors[slot].script_index = FIELD_SCRIPT_NONE;
    field_restart_actor_animation(&g_field_actors[slot]);
    field_rebuild_party_actions(0);
    field_refresh_actor_portraits();

    if ((g_field_scene_mode_bit != 0) && (slot == 1))
    {
        field_load_weapon_sfx_table(1, D_800FDA81);
    }

    field_refresh_party_routes();
    field_set_party_palettes();

    switch (g_field_player_records[slot].character_kind)
    {
    case FIELD_PLAYER_KIND_PARTNER:
        func_800A5174(1, g_field_player_records[slot].character_id + FIELD_RES_PARTNER_VOICES);
        field_apply_weapon_action_params(1);
        break;

    case FIELD_PLAYER_KIND_COMPANION:
        func_800A5174(2, g_field_player_records[slot].character_id + FIELD_RES_COMPANION_VOICES);
        break;
    }

    return 1;
}

/**
 * @brief Reuse the resource entry that already holds @p resource_slot_id, or the first free one, and load it.
 * @param resource_slot_id Resource slot identifier to find.
 * @param resource_base Package number forwarded to the entry loader.
 */
void field_find_or_load_resource_entry(s32 resource_slot_id, s32 resource_base)
{
    s32 i;

    for (i = 0; i < FIELD_RESOURCE_ENTRY_COUNT; i++)
    {
        if (((g_field_resource_entries[i].flags >> 1) & 1) && g_field_resource_entries[i].slot_index == resource_slot_id)
        {
            break;
        }
    }

    if (i == FIELD_RESOURCE_ENTRY_COUNT)
    {
        for (i = 0; i < FIELD_RESOURCE_ENTRY_COUNT; i++)
        {
            if (!((g_field_resource_entries[i].flags >> 1) & 1))
            {
                break;
            }
        }
    }

    field_load_resource_entry(resource_slot_id, (u8*)resource_base, i);
}

/**
 * @brief Replace a resource entry with a new package and restart the actors that use it.
 * @param resource_slot_id Resource slot identifier stored in the entry.
 * @param resource_base Package number; the CD resource id is FIELD_RES_ENTRY_PACKAGES plus this value.
 * @param entry_index Resource entry to replace.
 */
void field_load_resource_entry(s32 resource_slot_id, u8* resource_base, s32 entry_index)
{
    s32 i;
    FieldResourceEntry* entry;
    FieldResourceEntry* entries;

    field_release_resource_entry(entry_index);

    entries = g_field_resource_entries;
    entry = &entries[entry_index];
    entry->slot_index = resource_slot_id;
    entry->unk8 = 0;
    field_set_party_palettes();
    entry->unkE = 0;
    entry->flags &= ~FIELD_RESOURCE_HAS_ACTIONS;
    entry->start = g_field_resource_cursor;
    field_load_resource_package(resource_base + FIELD_RES_ENTRY_PACKAGES, resource_slot_id, entry_index);
    entry->end = g_field_resource_cursor;
    entry->flags |= FIELD_RESOURCE_LOADED;

    {
        FieldActor* actor;
        actor = g_field_actors;
        for (i = 0; i < FIELD_ACTOR_COUNT; i++, actor++)
        {
            if (actor->presence != FIELD_ACTOR_UNUSED && actor->resource_index == entry_index)
            {
                field_restart_actor_animation(actor);
            }
        }
    }
}

/**
 * @brief Release a resource entry and compact the resource arena behind it.
 * @param entry_index Resource entry to release.
 */
void field_release_resource_entry(s32 entry_index)
{
    s32 i;
    u8* src;
    u8* dst;
    s32 size;
    FieldActor* actor;

    if ((g_field_resource_entries[entry_index].flags >> 1) & 1)
    {
        src = g_field_resource_entries[entry_index].end;
        size = src - g_field_resource_entries[entry_index].start;
        dst = g_field_resource_entries[entry_index].start;

        while (src != g_field_resource_cursor)
        {
            *dst = *src;
            src++;
            dst++;
        }

        for (i = 0; i < FIELD_RESOURCE_MOVABLE_COUNT; i++)
        {
            if (((g_field_resource_entries[i].flags >> 1) & 1) && g_field_resource_entries[i].start > g_field_resource_entries[entry_index].start)
            {
                g_field_resource_entries[i].start -= size;
                g_field_resource_entries[i].end -= size;
            }
        }

        actor = g_field_actors;
        for (i = 0; i < FIELD_ACTOR_COUNT; i++)
        {
            if ((actor->presence != FIELD_ACTOR_UNUSED) && (actor->resource_index != FIELD_RESOURCE_FIXED))
            {
                if ((actor->frame_data | 0x80000000) > (((u32)g_field_resource_entries[entry_index].start) | 0x80000000))
                {
                    actor->frame_data -= size;
                }
            }
            actor++;
        }

        g_field_resource_cursor = ((u8*)g_field_resource_cursor) - size;
    }
}

/**
 * @brief Clear an actor record and bind it to a resource entry.
 * @param actor_index Actor record to initialize.
 * @param resource_entry_index Resource entry used by the actor.
 */
void field_initialize_actor_record(s32 actor_index, s32 resource_entry_index)
{
    u32 i;
    u8* bytes;
    s32 control_flags;
    s32 initial_flags;
    s16* velocity;
    u32* unk44;

    bytes = (u8*)&g_field_actors[actor_index];
    i = 0;
    do
    {
        *bytes = 0;
        i++;
        bytes++;
    } while (i < sizeof(FieldActor));

    g_field_object_states[actor_index].collision_node = -1;
    g_field_object_states[actor_index].collision_flags = 0;
    g_field_object_states[actor_index].interaction_kind = 0;
    g_field_object_states[actor_index].contact.word &= ~FIELD_CONTACT_TARGETED;
    g_field_object_states[actor_index].contact.word &= ~FIELD_CONTACT_UNK40;

    g_field_actors[actor_index].owner_slot = (s8)(actor_index + FIELD_OBJECT_SLOT_BASE);
    g_field_actors[actor_index].script_index = FIELD_SCRIPT_NONE;

    g_field_actors[actor_index].object_index = actor_index;
    g_field_actors[actor_index].animation_active = 0;
    g_field_actors[actor_index].presence = 0;
    g_field_actors[actor_index].animation_frame = 0;
    g_field_actors[actor_index].command = FIELD_ACTOR_COMMAND_NONE;
    g_field_actors[actor_index].script_offset = 0;
    g_field_actors[actor_index].animation_state = 0;
    g_field_actors[actor_index].variant = 0;
    g_field_actors[actor_index].unk32 = 0;
    g_field_actors[actor_index].running = 0;
    g_field_actors[actor_index].y = 0;
    g_field_actors[actor_index].z = 0;
    initial_flags = (g_field_actors[actor_index].control.word & ~FIELD_CONTROL_MODE_MASK) | FIELD_CONTROL_SCRIPTED;
    g_field_actors[actor_index].control.word = initial_flags;
    g_field_actors[actor_index].x = FIELD_ACTOR_UNPLACED_X;
    control_flags = g_field_actors[actor_index].control.word;
    control_flags &= ~FIELD_CONTROL_UNK8000;
    control_flags &= ~FIELD_CONTROL_UNK10000000;
    control_flags &= ~FIELD_CONTROL_UNK40000;
    g_field_actors[actor_index].control.word = control_flags;
    g_field_actors[actor_index].x = FIELD_ACTOR_UNPLACED_X;

    if (actor_index == 1 && D_800FDA83 == FIELD_PLAYER_KIND_HERO)
    {
        FieldActor* partner = &g_field_actors[1];

        partner->control.word = (partner->control.word & ~FIELD_CONTROL_UNK19_MASK) | FIELD_CONTROL_UNK19_PARTNER;
    }
    else
    {
        g_field_actors[actor_index].control.word &= ~FIELD_CONTROL_UNK19_MASK;
    }

    g_field_actors[actor_index].control.word &= ~FIELD_CONTROL_PLAY_ONCE;
    g_field_actors[actor_index].unkC = g_field_resource_entries[resource_entry_index].slot_index;
    g_field_actors[actor_index].resource_index = resource_entry_index;
    g_field_actors[actor_index].animation = 0;
    velocity = &g_field_actors[actor_index].unk10;
    velocity[0] = 0;
    velocity[1] = 0;
    velocity[2] = 0;
    g_field_actors[actor_index].frame_timer = 1;
    g_field_actors[actor_index].frame_ticks = 0;
    unk44 = &g_field_actors[actor_index].unk44;
    unk44[0] = 0;
    unk44[1] = 0;
    unk44[2] = 0;
    g_field_actors[actor_index].tint_blue = FIELD_TINT_NEUTRAL;
    g_field_actors[actor_index].tint_green = FIELD_TINT_NEUTRAL;
    g_field_actors[actor_index].tint_red = FIELD_TINT_NEUTRAL;

    if (actor_index == FIELD_COMPANION_INDEX && D_800FDCEA >= FIELD_COMPANION_COLOR_ID_MIN)
    {
        FieldActor* companion = &g_field_actors[FIELD_COMPANION_INDEX];

        companion->control.word =
            (companion->control.word & ~FIELD_CONTROL_VARIANT_MASK) | ((((u8)g_pad_ctx->large_history_index + 1) & 3) << FIELD_CONTROL_VARIANT_SHIFT);
        return;
    }

    g_field_actors[actor_index].control.word &= ~FIELD_CONTROL_VARIANT_MASK;
}

/**
 * @brief Reset one object's model part to its default render state.
 * @param part_index Object part to initialize.
 * @param timer_mode Non-zero gives the part the small scale.
 */
void field_initialize_actor_part(s32 part_index, s32 timer_mode)
{
    s32 words_remaining;
    u32* word_cursor;

    words_remaining = sizeof(g_field_object_parts[part_index]) / sizeof(*word_cursor);
    word_cursor = (u32*)&g_field_object_parts[part_index];
    do
    {
        *word_cursor = 0;
        words_remaining--;
        word_cursor++;
    } while (words_remaining != 0);

    g_field_object_parts[part_index].tint_blue = FIELD_TINT_NEUTRAL;
    g_field_object_parts[part_index].tint_green = FIELD_TINT_NEUTRAL;
    g_field_object_parts[part_index].tint_red = FIELD_TINT_NEUTRAL;
    g_field_object_parts[part_index].unk8 = 1;
    g_field_object_parts[part_index].unk9 = 0xFF;
    g_field_object_parts[part_index].unkD = 8;
    g_field_object_parts[part_index].unk14.half.hi = 20;
    g_field_object_parts[part_index].unk24.bytes[1] = 8;
    g_field_object_parts[part_index].unk24.bytes[0] = 8;
    g_field_object_parts[part_index].unk23 = 8;
    g_field_object_parts[part_index].unk18 = 0x100;
    g_field_object_parts[part_index].unk4.bits.flag11 = 1;
    g_field_object_parts[part_index].unk4.bits.render_mode = 1;
    g_field_object_parts[part_index].unk0.bits.mode = 2;

    if (timer_mode != 0)
    {
        g_field_object_parts[part_index].scale_z = FIELD_PART_SCALE_SMALL;
        g_field_object_parts[part_index].scale_x = FIELD_PART_SCALE_SMALL;
    }
    else
    {
        g_field_object_parts[part_index].scale_z = FIELD_PART_SCALE_FULL;
        g_field_object_parts[part_index].scale_x = FIELD_PART_SCALE_FULL;
    }

    g_field_object_parts[part_index].unk11 = 0xFF;
    g_field_object_parts[part_index].unk28.word |= 0x2000000;
    g_field_object_parts[part_index].unk14.bits.mode = 2;
    g_field_object_parts[part_index].unk24.word |= 0x100000;
}

/**
 * @brief Apply one tint and render mode to every field object and its part.
 * @param red Red component.
 * @param green Green component.
 * @param blue Blue component.
 * @param color_flag Low bit becomes the actors' FIELD_CONTROL_TINTED bit.
 * @param render_mode Two-bit render mode of the parts.
 */
void field_set_all_actor_render_state(s32 red, s32 green, s32 blue, s32 color_flag, s32 render_mode)
{
    s32 i;
    u32 mode_bits;
    u32 color_flag_bits;

    mode_bits = (render_mode & 3) << FIELD_PART_RENDER_MODE_SHIFT;
    for (i = 0; i < FIELD_ACTOR_COUNT; i++)
    {
        g_field_object_states[i].tint_red = red;
        g_field_object_states[i].tint_green = green;
        g_field_object_states[i].tint_blue = blue;
        g_field_object_parts[i].tint_red = red;
        g_field_object_parts[i].tint_green = green;
        g_field_object_parts[i].tint_blue = blue;
        g_field_object_parts[i].unk4.word = (g_field_object_parts[i].unk4.word & ~FIELD_PART_RENDER_MODE_MASK) | mode_bits;
    }

    for (i = 0; i < FIELD_ACTOR_COUNT; i++)
    {
        color_flag_bits = (color_flag & 1) << FIELD_CONTROL_TINTED_SHIFT;
        g_field_actors[i].control.word = (g_field_actors[i].control.word & ~FIELD_CONTROL_TINTED) | color_flag_bits;
    }
}

/**
 * @brief Apply a tint and render mode to the field object with the given key.
 * @param red Red component.
 * @param green Green component.
 * @param blue Blue component.
 * @param color_flag Low bit becomes the actor's FIELD_CONTROL_TINTED bit.
 * @param render_mode Two-bit render mode of the part.
 * @param actor_selector Key of the object.
 * @return 0 on success, or -1 when no object has the key.
 */
s32 field_set_actor_render_state(s32 red, s32 green, s32 blue, s32 color_flag, s32 render_mode, s32 actor_selector)
{
    FieldActor* actor;

    actor = field_lookup_actor(actor_selector);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }

    g_field_object_states[actor->object_index].tint_red = red;
    g_field_object_states[actor->object_index].tint_green = green;
    g_field_object_states[actor->object_index].tint_blue = blue;
    g_field_object_parts[actor->object_index].tint_red = red;
    g_field_object_parts[actor->object_index].tint_green = green;
    g_field_object_parts[actor->object_index].tint_blue = blue;
    g_field_object_parts[actor->object_index].unk4.word =
        (g_field_object_parts[actor->object_index].unk4.word & ~FIELD_PART_RENDER_MODE_MASK) | ((render_mode & 3) << FIELD_PART_RENDER_MODE_SHIFT);
    actor->control.word = (actor->control.word & ~FIELD_CONTROL_TINTED) | ((color_flag & 1) << FIELD_CONTROL_TINTED_SHIFT);
    return 0;
}

/**
 * @brief Read an actor texture resource and upload its palette and image to VRAM.
 * @param resource_id CD resource id to read.
 * @param slot_index Actor texture slot.
 * @param texture_column Texture column within the actor VRAM area.
 * @param narrow_layout Non-zero selects the narrow half-height layout.
 * @param palette_row Palette row offset.
 */
static void field_load_actor_texture_set(s32 resource_id, s32 slot_index, s32 texture_column, s32 narrow_layout, s32 palette_row)
{
    RECT rect;
    FieldCdBuffer* buf;
    s32 image_offset;
    s32 full_width;

    buf = g_field_cd_buffer;
    full_width = 0x100;
    cdrom_queue_read(resource_id & 0xFFFF, buf);
    cdrom_wait_queue_empty();
    image_offset = buf->offsets[2];
    buf->data = 0;

    if (texture_column == 0 || narrow_layout != 0)
    {
        if (narrow_layout != 0)
        {
            setRECT(&rect, 0xC0, palette_row + 0x1F4, 0x40, 1);
        }
        else
        {
            rect.y = palette_row + 0x1F4;
            rect.w = full_width;
            rect.x = 0;
            rect.h = 1;
        }
        LoadImage(&rect, (u_long*)&buf->data);
    }

    if (slot_index >= 2)
    {
        s32 base = (texture_column << 6) + 0x340;
        s32 offset = slot_index << 6;

        rect.x = base - offset;
        rect.w = 0x40;
        rect.y = 0;
        rect.h = 0x100;
    }
    else if (narrow_layout != 0)
    {
        s32 base = (texture_column << 6) + 0x380;

        setRECT(&rect, base - (slot_index << 7), 0x80, 0x40, 0x80);
    }
    else
    {
        s32 base = (texture_column << 6) + 0x380;
        s32 offset = slot_index << 7;

        rect.x = base - offset;
        rect.w = 0x40;
        rect.y = 0;
        rect.h = 0x100;
    }

    LoadImage(&rect, (u_long*)(image_offset + (s32)buf + 0x14));
    DrawSync(0);
}

/**
 * @brief Run one frame of every field object: animation, effects, and input, following or script commands.
 */
void field_update_actor_objects(void)
{
    FieldActor* actor;
    FieldObjectState* state;
    s32 i;
    s32 mode;
    s32 count;
    s32 follower_index;

    actor = &g_field_actors[0];
    state = &g_field_object_states[0];

    if (D_80122714 == 0)
    {
        field_poll_leader_interaction();
        if (D_80122714 == 0 && g_field_active_group == 0)
        {
            field_check_marker_contact(g_field_actors);
        }
    }

    field_update_object_tints();
    follower_index = 0;
    i = 0;
    count = -1;

    do
    {
        if (actor->presence != FIELD_ACTOR_UNUSED)
        {
            if (!(actor->control.word & FIELD_CONTROL_MODE_MASK) && count <= 0)
            {
                count++;
            }

            if (!(state->flags & FIELD_OBJECT_ANIMATION_FROZEN))
            {
                actor->frame_data = (s32)field_advance_actor_animation_frame(actor);
            }
            else
            {
                FIELD_ACTOR_FRAME_WORD(actor) |= FIELD_FRAME_UNCHANGED;
            }

            mode = state->group_flags & FIELD_OBJECT_GROUP_MASK;
            if (g_field_active_group == mode || mode == 0)
            {
                field_update_object_effects(i);
                if (!(state->contact.word & (FIELD_CONTACT_TARGETED | FIELD_CONTACT_ANIMATION_HIDDEN)))
                {
                    if (g_field_dialog_screen_mode == 0 && g_field_return_to_title_prompt_state == 0)
                    {
                        field_step_actor_script(actor);
                    }

                    mode = actor->control.word & FIELD_CONTROL_MODE_MASK;
                    if (mode == FIELD_CONTROL_PAD)
                    {
                        if (D_80122714 == 0 && D_80122710 == 0 && g_field_dialog_screen_mode == 0)
                        {
                            if (!(state->flags & FIELD_OBJECT_CONTROL_BLOCKED))
                            {
                                field_update_actor_input(actor, count);
                            }
                            else
                            {
                                field_settle_actor_vertical_offset(actor);
                            }
                        }
                    }
                    else if (mode == FIELD_CONTROL_FOLLOW)
                    {
                        if (g_field_dialog_screen_mode == 0 && g_field_return_to_title_prompt_state == 0)
                        {
                            if (!(state->flags & FIELD_OBJECT_CONTROL_BLOCKED))
                            {
                                /* Unprototyped call: the third value rides along in $a2. */
                                field_follow_leader_route(actor, follower_index, 0x600 + (follower_index * 0x400));
                                actor->control.word &= ~FIELD_CONTROL_PLAY_ONCE;
                            }
                            else
                            {
                                field_settle_actor_vertical_offset(actor);
                            }
                        }
                        follower_index++;
                    }
                    else if (mode == FIELD_CONTROL_SCRIPTED)
                    {
                        if (g_field_return_to_title_prompt_state == 0 && g_field_dialog_screen_mode == 0 && D_80122B20 == 0)
                        {
                            if (actor->command != FIELD_ACTOR_COMMAND_DEFEAT_WAIT && actor->command != FIELD_ACTOR_COMMAND_DEFEAT_END && actor->command != FIELD_ACTOR_COMMAND_DEFEATED &&
                                actor->command != FIELD_ACTOR_COMMAND_DEFEAT_DELAY && actor->command != FIELD_ACTOR_COMMAND_KNOCKED_DOWN &&
                                actor->command != FIELD_ACTOR_COMMAND_ATTACHED)
                            {
                                field_update_spawned_actor(actor);
                            }
                        }

                        if (!(state->flags & FIELD_OBJECT_CONTROL_BLOCKED))
                        {
                            field_update_actor_command(actor);
                            if (g_field_return_to_title_prompt_state == 0 && g_field_dialog_screen_mode == 0)
                            {
                                actor->control.word |= FIELD_CONTROL_PLAY_ONCE;
                            }
                        }
                        else
                        {
                            field_settle_actor_vertical_offset(actor);
                        }
                    }

                    field_record_actor_position(actor);
                }
            }
        }

        i++;
        actor++;
        state++;
    } while (i < FIELD_ACTOR_COUNT);

    field_camera_update();
}

/**
 * @brief Move a raised actor (negative Y) back down towards the ground by one step.
 * @param actor Actor to update.
 */
static void field_settle_actor_vertical_offset(FieldActor* actor)
{
    if (actor->y < 0)
    {
        actor->y += 0x800;
        if (actor->y > 0)
        {
            actor->y = 0;
        }
    }
}

/**
 * @brief Emit the render packets of every visible field object.
 * @param render_context Field render context and packet cursor.
 */
void field_render_actor_objects(FieldRenderContext* render_context)
{
    FieldActor* actor;
    FieldObjectState* state;
    u32* ordering_table;
    s32* packet_cursor;
    s32 i;

    actor = &g_field_actors[0];
    ordering_table = &render_context->ordering_table;
    i = 0;
    state = &g_field_object_states[0];
    packet_cursor = render_context->packet_cursor;

    do
    {
        if (actor->presence != FIELD_ACTOR_HIDDEN && actor->presence != FIELD_ACTOR_UNUSED)
        {
            if (actor->frame_data >= 0)
            {
                packet_cursor = field_render_effect_frame16(actor, packet_cursor, ordering_table, actor->frame_data, 0, &g_field_object_parts[i]);
            }
            else
            {
                packet_cursor = field_render_effect_frame8(actor, packet_cursor, ordering_table, actor->frame_data, 0, &g_field_object_parts[i]);
            }
        }
        else if (actor->presence == FIELD_ACTOR_HIDDEN)
        {
            state->collision.word = 0x100000;
            state->unk140 = -8;
            state->unk142 = -0xF;
            state->unk144 = 8;
            state->unk146 = 0;
        }
        else
        {
            state->collision.word = 0;
        }

        i++;
        actor++;
        state++;
    } while (i < FIELD_ACTOR_COUNT);

    render_context->packet_cursor = packet_cursor;
}

/**
 * @brief Read one CD resource to @p destination and advance the resource arena cursor past it.
 * @param resource_id CD resource id to read.
 * @param destination Buffer receiving the data.
 */
static void field_stream_resource_to_buffer(u16 resource_id, void* destination)
{
    s32 size;

    size = cdrom_queue_read(resource_id, destination);
    cdrom_wait_queue_empty();
    g_field_resource_cursor += (size + 3) & ~3;
}

/**
 * @brief Advance an actor's animation by one tick and return the data of the displayed frame.
 *
 * An animation resource starts with the offset of its frame table (bytes 2-3), the
 * animation count (byte 4) and a mode byte (byte 5), followed by one offset per
 * animation. Each animation holds its frame count and then two-byte entries
 * (frame, duration), or four-byte entries that add a height and a motion value.
 *
 * @param actor Actor whose animation advances.
 * @return Frame data of the displayed frame.
 */
static u8* field_advance_actor_animation_frame(FieldActor* actor)
{
    u8* resource_base;
    u8* entry;
    u8* frame_offsets;
    u32 default_motion;
    s32 wrap;
    s32 entry_shift;
    s32 animation_index;
    s32 frame;
    s32 at_end;
    u8 animation_byte;
    u8 next_frame;
    s32 duration;
    s32 offset_low;
    u32 offset_high;

    resource_base = g_field_resource_entries[actor->resource_index].start;
    entry = resource_base + 4;
    default_motion = entry[1] >> 1;
    wrap = entry[1] & FIELD_ANIMATION_WRAP;
    if (default_motion & FIELD_ANIMATION_MODE_OFFSET)
    {
        default_motion -= FIELD_ANIMATION_MODE_OFFSET;
    }

    animation_byte = actor->animation;
    animation_index = animation_byte & FIELD_ANIMATION_INDEX_MASK;
    if (animation_index >= (s32)resource_base[4])
    {
        entry = resource_base + 6;
        actor->animation = animation_byte & FIELD_ANIMATION_FACING;
    }
    else
    {
        entry = entry + (animation_index * 2 + 2);
    }

    offset_low = entry[0];
    offset_high = entry[1];
    entry = resource_base + offset_low + ((offset_high & 0x7F) << 8);
    entry_shift = (offset_high >> 7) + 1;
    FIELD_ACTOR_FRAME_WORD(actor) |= FIELD_FRAME_UNCHANGED;

    if (actor->animation_active != 0)
    {
        actor->frame_timer--;
        actor->frame_ticks++;
    }

    if (actor->frame_timer == 0 && actor->animation_active != 0)
    {
        next_frame = actor->animation_frame + 1;
        actor->animation_frame = next_frame;
        if (next_frame >= entry[0])
        {
            if (actor->animation_state == 0 || --actor->animation_state == 0)
            {
                if (actor->control.word & FIELD_CONTROL_PLAY_ONCE)
                {
                    actor->frame_timer = 1;
                    actor->frame_ticks = 1;
                    actor->speed_accumulator = 0;
                    actor->animation_frame--;
                    return (u8*)actor->frame_data;
                }
            }
            actor->animation_frame = 0;
        }

        frame = actor->animation_frame;
        FIELD_ACTOR_FRAME_WORD(actor) &= ~FIELD_FRAME_UNCHANGED;
        at_end = (frame + 1) >= (s32)entry[0];
        entry = entry + ((frame << entry_shift) + 1);
        duration = entry[1];
        actor->frame_timer = duration;
        actor->frame_length = duration;
        if (actor->frame_timer == 0)
        {
            actor->frame_timer++;
            actor->frame_length++;
        }
        actor->frame_ticks = 0;
        if (entry_shift == 2)
        {
            actor->height = entry[2];
            actor->speed_accumulator = entry[3];
            if (at_end)
            {
                actor->next_height = actor->height;
            }
            else
            {
                actor->next_height = entry[6];
            }
        }
        else
        {
            actor->next_height = 0;
            actor->height = 0;
            actor->speed_accumulator = default_motion;
        }
    }
    else
    {
        entry = entry + ((actor->animation_frame << entry_shift) + 1);
    }

    frame_offsets = resource_base + resource_base[2] + (resource_base[3] << 8) + (entry[0] * 2 + 2);
    if (wrap)
    {
        return (u8*)((s32)(resource_base + frame_offsets[0] + (frame_offsets[1] << 8)) & 0x7FFFFFFF);
    }
    return resource_base + frame_offsets[0] + (frame_offsets[1] << 8);
}

/**
 * @brief Restart an actor's animation from its first frame.
 * @param actor Actor to restart.
 */
void field_restart_actor_animation(FieldActor* actor)
{
    actor->animation_frame = 0;
    actor->control.word &= ~FIELD_CONTROL_PLAY_ONCE;
    actor->frame_data = (s32)field_begin_actor_animation_forward(actor, g_field_resource_entries[actor->resource_index].start);
}

/**
 * @brief Set up the first frame of an actor's animation.
 * @param actor Actor to set up; animation_frame selects the frame.
 * @param resource_base Animation resource.
 * @return Frame data of the frame.
 */
u8* field_begin_actor_animation_forward(FieldActor* actor, u8* resource_base)
{
    u8* entry;
    u8* frame_offsets;
    u32 default_motion;
    s32 wrap;
    s32 entry_shift;
    s32 animation_index;
    s32 at_end;
    s32 duration;
    s32 offset_low;
    u32 offset_high;
    u8 animation_byte;

    entry = resource_base + 4;
    default_motion = entry[1] >> 1;
    wrap = entry[1] & FIELD_ANIMATION_WRAP;
    if (default_motion & FIELD_ANIMATION_MODE_OFFSET)
    {
        default_motion -= FIELD_ANIMATION_MODE_OFFSET;
    }

    animation_byte = actor->animation;
    animation_index = animation_byte & FIELD_ANIMATION_INDEX_MASK;
    if (animation_index >= (s32)resource_base[4])
    {
        entry = resource_base + 6;
        actor->animation = animation_byte & FIELD_ANIMATION_FACING;
    }
    else
    {
        entry = entry + (animation_index * 2 + 2);
    }

    offset_low = entry[0];
    offset_high = entry[1];
    entry = resource_base + offset_low + ((offset_high & 0x7F) << 8);
    entry_shift = (offset_high >> 7) + 1;

    at_end = (actor->animation_frame + 1) >= (s32)entry[0];
    entry = entry + 1;
    duration = entry[1] * actor->animation_active;
    actor->frame_timer = duration;
    actor->frame_length = duration;
    if (actor->frame_timer == 0)
    {
        actor->frame_timer++;
        actor->frame_length++;
    }
    actor->frame_ticks = 0;
    if (entry_shift == 2)
    {
        actor->height = entry[2];
        actor->speed_accumulator = entry[3];
        if (at_end)
        {
            actor->next_height = 0;
        }
        else
        {
            actor->next_height = entry[6];
        }
    }
    else
    {
        actor->next_height = 0;
        actor->height = 0;
        actor->speed_accumulator = default_motion;
    }

    if (actor->frame_length == 0)
    {
        actor->frame_length = 1;
    }
    FIELD_ACTOR_FRAME_WORD(actor) &= ~FIELD_FRAME_UNCHANGED;

    frame_offsets = resource_base + resource_base[2] + (resource_base[3] << 8) + (entry[0] * 2 + 2);
    if (wrap)
    {
        return (u8*)((s32)(resource_base + frame_offsets[0] + (frame_offsets[1] << 8)) & 0x7FFFFFFF);
    }
    return resource_base + frame_offsets[0] + (frame_offsets[1] << 8);
}

/**
 * @brief Show the last frame of an actor's animation and hold it there.
 * @param actor Actor to update.
 */
void field_restart_actor_animation_reverse(FieldActor* actor)
{
    actor->control.word |= FIELD_CONTROL_PLAY_ONCE;
    actor->frame_data = (s32)field_begin_actor_animation_reverse(actor, g_field_resource_entries[actor->resource_index].start);
}

/**
 * @brief Set up the last frame of an actor's animation.
 * @param actor Actor to set up.
 * @param resource_base Animation resource.
 * @return Frame data of the last frame.
 */
static u8* field_begin_actor_animation_reverse(FieldActor* actor, u8* resource_base)
{
    u8* entry;
    u8* frame_offsets;
    u32 default_motion;
    s32 wrap;
    s32 entry_shift;
    s32 animation_index;
    s32 offset_low;
    u32 offset_high;
    u8 animation_byte;
    s32 duration;

    entry = resource_base + 4;
    default_motion = entry[1] >> 1;
    wrap = entry[1] & FIELD_ANIMATION_WRAP;
    if (default_motion & FIELD_ANIMATION_MODE_OFFSET)
    {
        default_motion -= FIELD_ANIMATION_MODE_OFFSET;
    }

    animation_byte = actor->animation;
    animation_index = animation_byte & FIELD_ANIMATION_INDEX_MASK;
    if (animation_index >= (s32)resource_base[4])
    {
        entry = resource_base + 6;
        actor->animation = animation_byte & FIELD_ANIMATION_FACING;
    }
    else
    {
        entry = entry + (animation_index * 2 + 2);
    }

    offset_low = entry[0];
    offset_high = entry[1];
    entry = resource_base + offset_low + ((offset_high & 0x7F) << 8);
    entry_shift = (offset_high >> 7) + 1;

    actor->animation_frame = entry[0] - 1;
    entry = entry + ((actor->animation_frame << entry_shift) + 1);
    duration = entry[1] * actor->animation_active;
    actor->frame_timer = duration;
    actor->frame_length = duration;
    if (actor->frame_timer == 0)
    {
        actor->frame_timer++;
        actor->frame_length++;
    }
    actor->frame_ticks = 0;
    if (entry_shift == 2)
    {
        actor->height = entry[2];
        actor->speed_accumulator = entry[3];
        actor->next_height = 0;
    }
    else
    {
        actor->next_height = 0;
        actor->height = 0;
        actor->speed_accumulator = default_motion;
    }

    FIELD_ACTOR_FRAME_WORD(actor) &= ~FIELD_FRAME_UNCHANGED;

    frame_offsets = resource_base + resource_base[2] + (resource_base[3] << 8) + (entry[0] * 2 + 2);
    if (wrap)
    {
        return (u8*)((s32)(resource_base + frame_offsets[0] + (frame_offsets[1] << 8)) & 0x7FFFFFFF);
    }
    return resource_base + frame_offsets[0] + (frame_offsets[1] << 8);
}

/**
 * @brief Return the frame count of the animation after the actor's current one.
 * @param actor Actor whose animation is looked up.
 * @return Frame count of the next animation, or zero when there is none.
 */
u8 field_get_next_animation_frame_count(FieldActor* actor)
{
    u8* resource_base;
    u8* entry;
    s32 animation_index;

    /* The target loads the resource start in a block of its own; a plain assignment schedules differently. */
    do
    {
        resource_base = g_field_resource_entries[actor->resource_index].start;
    } while (0);
    entry = resource_base + 4;
    animation_index = (actor->animation & FIELD_ANIMATION_INDEX_MASK) + 1;
    if (animation_index >= (s32)resource_base[4])
    {
        return 0;
    }
    entry = entry + (animation_index * 2 + 2);
    entry = resource_base + entry[0] + ((entry[1] & 0x7F) << 8);
    /* The frame count goes back through the actor register; returning entry[0] directly allocates differently. */
    actor = (FieldActor*)(u32)entry[0];
    return (u32)actor;
}

/**
 * @brief Advance an effect record's part animation by one tick and return the displayed frame data.
 * @param actor Effect record whose animation advances.
 * @param resource_base Animation resource.
 * @return Frame data of the displayed frame, or 0 when a non-looping part animation ended.
 */
u8* field_advance_actor_part_animation_frame(FieldActor* actor, u8* resource_base)
{
    u8* entry;
    u8* frame_offsets;
    FieldObjectPart* part;
    u32 default_motion;
    s32 wrap;
    s32 entry_shift;
    s32 animation_index;
    s32 frame;
    s32 at_end;
    s32 duration;
    s32 offset_low;
    u32 offset_high;
    u8 next_frame;

    entry = resource_base + 4;
    default_motion = entry[1] >> 1;
    wrap = entry[1] & FIELD_ANIMATION_WRAP;
    if (default_motion & FIELD_ANIMATION_MODE_OFFSET)
    {
        default_motion -= FIELD_ANIMATION_MODE_OFFSET;
    }

    animation_index = actor->animation & FIELD_ANIMATION_INDEX_MASK;
    if (animation_index >= (s32)resource_base[4])
    {
        entry = resource_base + 6;
    }
    else
    {
        entry = entry + (animation_index * 2 + 2);
    }

    offset_low = entry[0];
    offset_high = entry[1];
    entry = resource_base + offset_low + ((offset_high & 0x7F) << 8);
    FIELD_ACTOR_FRAME_WORD(actor) |= FIELD_FRAME_UNCHANGED;
    entry_shift = (offset_high >> 7) + 1;

    if (D_800F2298 == 0 && g_field_modal_state == 0 && g_field_text_session_active == 0 && actor->animation_active != 0)
    {
        actor->frame_timer--;
        actor->frame_ticks++;
    }

    if (actor->frame_timer == 0 && actor->animation_active != 0)
    {
        next_frame = actor->animation_frame + 1;
        actor->animation_frame = next_frame;
        if (next_frame >= entry[0])
        {
            part = &g_field_actor_slots[actor->owner_slot].parts[actor->owner_part];
            if (!((part->unk4.word >> FIELD_PART_LOOP_SHIFT) & 3))
            {
                field_retire_effect(actor, part);
                return 0;
            }
            actor->animation_frame = 0;
        }

        frame = actor->animation_frame;
        FIELD_ACTOR_FRAME_WORD(actor) &= ~FIELD_FRAME_UNCHANGED;
        at_end = (frame + 1) == (s32)entry[0];
        entry = entry + ((frame << entry_shift) + 1);
        duration = entry[1] * actor->animation_active;
        actor->frame_ticks = 0;
        actor->frame_timer = duration;
        actor->frame_length = duration;
        if (entry_shift == 2)
        {
            actor->height = entry[2];
            actor->speed_accumulator = entry[3];
            if (at_end)
            {
                actor->next_height = 0;
            }
            else
            {
                actor->next_height = entry[6];
            }
        }
        else
        {
            actor->next_height = 0;
            actor->height = 0;
            actor->speed_accumulator = default_motion;
        }
    }
    else
    {
        entry = entry + ((actor->animation_frame << entry_shift) + 1);
    }

    frame_offsets = resource_base + resource_base[2] + (resource_base[3] << 8) + (entry[0] * 2 + 2);
    if (wrap)
    {
        return (u8*)((s32)(resource_base + frame_offsets[0] + (frame_offsets[1] << 8)) & 0x7FFFFFFF);
    }
    return resource_base + frame_offsets[0] + (frame_offsets[1] << 8);
}

/**
 * @brief Read an actor resource package from the CD and unpack it.
 * @param resource_id CD resource id to read.
 * @param slot_index Texture slot of the package.
 * @param resource_entry_index Resource entry receiving the package (also the palette row).
 */
void field_load_resource_package(u16 resource_id, s32 slot_index, s32 resource_entry_index)
{
    FieldCdBuffer* buf;
    s32 size;

    buf = g_field_cd_buffer;
    size = cdrom_queue_read(resource_id, buf);
    cdrom_wait_queue_empty();
    field_unpack_resource_package(buf, size, slot_index, resource_entry_index);
}

/**
 * @brief Split a staged actor resource package into its animation data and textures.
 * @param buf Staged package; the first offset is also the header size (one word per section).
 * @param size Total package size in bytes.
 * @param slot_index Texture slot and resource entry receiving the package.
 * @param palette_row Palette row used for the texture upload.
 */
void field_unpack_resource_package(FieldCdBuffer* buf, s32 size, s32 slot_index, s32 palette_row)
{
    switch (buf->offsets[0] >> 2)
    {
    case 2:
        field_append_resource_data((u32*)((u8*)buf + buf->offsets[1]), size - buf->offsets[1], slot_index);
        field_upload_resource_texture((FieldCdBuffer*)((u8*)buf + buf->offsets[0]), slot_index, 0, palette_row);
        break;
    case 3:
        field_append_resource_data((u32*)((u8*)buf + buf->offsets[2]), size - buf->offsets[2], slot_index);
        field_upload_resource_texture((FieldCdBuffer*)((u8*)buf + buf->offsets[0]), slot_index, 0, palette_row);
        field_upload_resource_texture((FieldCdBuffer*)((u8*)buf + buf->offsets[1]), slot_index, 1, palette_row);
        break;
    case 4:
        field_append_resource_data((u32*)((u8*)buf + buf->offsets[3]), size - buf->offsets[3], slot_index);
        field_upload_resource_texture((FieldCdBuffer*)((u8*)buf + buf->offsets[0]), slot_index, 0, palette_row);
        field_upload_resource_texture((FieldCdBuffer*)((u8*)buf + buf->offsets[1]), slot_index, 1, palette_row);
        field_upload_resource_texture((FieldCdBuffer*)((u8*)buf + buf->offsets[2]), slot_index, 2, palette_row);
        break;
    }
}

/**
 * @brief Upload one texture section of a staged actor resource package to VRAM.
 * @param buf Texture section: palette block, then the image block at offsets[2].
 * @param slot_index Actor texture slot.
 * @param texture_index Texture section index (1 has no palette, 2 is the narrow layout).
 * @param palette_row Palette row used by the upload.
 */
static void field_upload_resource_texture(FieldCdBuffer* buf, s32 slot_index, s32 texture_index, s32 palette_row)
{
    RECT rect;
    s32 image_offset;

    image_offset = buf->offsets[2];

    if (texture_index == 2)
    {
        setRECT(&rect, 0xC0, palette_row + 0x1F4, 0x40, 1);
    }
    else
    {
        rect.y = palette_row + 0x1F4;
        rect.w = 0x100;
        rect.x = 0;
        rect.h = 1;
    }

    if (texture_index != 1)
    {
        LoadImage(&rect, (u_long*)&buf->data);
    }

    if (slot_index >= 2)
    {
        s32 base = 0x340;
        s32 offset = slot_index << 6;

        rect.x = base - offset;
        rect.w = 0x40;
        rect.y = 0;
        rect.h = 0x100;
    }
    else
    {
        FieldCdBuffer* image = (FieldCdBuffer*)(image_offset + (s32)buf);
        s32 w = image->width;
        s32 h = image->height;

        if (texture_index == 2)
        {
            setRECT(&rect, 0x3C0 - (slot_index << 7), 0x80, 0x40, 0x80);
        }
        else if (texture_index == 1)
        {
            setRECT(&rect, 0x3C0 - (slot_index << 7), 0, w, h);
        }
        else
        {
            s32 base = (texture_index << 6) + 0x380;
            s32 offset = slot_index << 7;

            rect.x = base - offset;
            rect.w = 0x40;
            rect.y = 0;
            rect.h = 0x100;
        }
    }

    LoadImage(&rect, (u_long*)(image_offset + (s32)buf + 0x14));
}

/**
 * @brief Copy animation data to the resource arena cursor and point a resource entry at it.
 * @param src Source words.
 * @param length Number of source bytes.
 * @param slot_index Resource entry that receives the data.
 */
static void field_append_resource_data(u32* src, s32 length, s32 slot_index)
{
    u32* dst;
    u32 n;

    dst = g_field_resource_cursor;
    n = (u32)(length + 3) >> 2;
    while (n != 0)
    {
        *dst = *src;
        src++;
        n--;
        dst++;
    }

    g_field_resource_entries[slot_index].start = g_field_resource_cursor;
    g_field_resource_cursor += (length + 3) & ~3;
}

/**
 * @brief Map an actor's screen X position to the sound pan range.
 * @param actor_index Field object index.
 * @return Pan value clamped to FIELD_PAN_LEFT..FIELD_PAN_RIGHT.
 */
s32 field_get_actor_sound_pan(s32 actor_index)
{
    Vec2s pos;
    s32 screen_x;

    pos.x = FIELD_SCREEN_CENTER_X + g_field_view_offset_x / 256 + g_field_actors[actor_index].x / 256;
    pos.y = FIELD_SCREEN_CENTER_Y + g_field_view_offset_y / 256 + g_field_actors[actor_index].y / 256 - g_field_actors[actor_index].z / 512 - g_field_view_offset_z / 512;

    screen_x = pos.x;
    if (screen_x >= FIELD_PAN_SCREEN_LEFT)
    {
        if (screen_x >= FIELD_PAN_SCREEN_RIGHT)
        {
            return FIELD_PAN_RIGHT;
        }
        screen_x = ((screen_x - FIELD_PAN_SCREEN_LEFT) * (FIELD_PAN_RIGHT - FIELD_PAN_LEFT)) / (FIELD_PAN_SCREEN_RIGHT - FIELD_PAN_SCREEN_LEFT - 1);
        return screen_x + FIELD_PAN_LEFT;
    }
    return FIELD_PAN_LEFT;
}

/**
 * @brief Reload the cached portraits of the party when any member's portrait changed.
 */
static void field_refresh_actor_portraits(void)
{
    u8* partner_portrait;
    u8* companion_portrait;
    s32 partner_portrait_index;
    s32 golem_index;

    if (g_field_player_records[1].character_kind != FIELD_PLAYER_KIND_HERO)
    {
        partner_portrait_index = g_field_player_records[1].character_id + FIELD_PORTRAIT_PARTNER_BASE;
    }
    else
    {
        partner_portrait_index = g_field_player_records[1].head.bits.alt_appearance;
    }

    if (g_field_player_records[0].portrait_index != g_field_player_records[0].head.bits.alt_appearance ||
        g_field_player_records[1].portrait_index != partner_portrait_index ||
        g_field_player_records[2].portrait_index != g_field_player_records[2].character_id + FIELD_PORTRAIT_COMPANION_BASE)
    {
        if (g_field_player_records[1].portrait_index != partner_portrait_index)
        {
            if (g_field_player_records[1].character_kind != FIELD_PLAYER_KIND_HERO)
            {
                func_800A5174(1, g_field_player_records[1].character_id + FIELD_RES_PARTNER_VOICES);
            }
            else
            {
                func_800A5174(1, FIELD_RES_PARTNER_VOICES);
            }
        }
        if (g_field_player_records[2].portrait_index != g_field_player_records[2].character_id + FIELD_PORTRAIT_COMPANION_BASE)
        {
            func_800A5174(2, g_field_player_records[2].character_id + FIELD_RES_COMPANION_VOICES);
        }

        cdrom_stream(FIELD_RES_PORTRAITS, g_field_cd_buffer);
        cdrom_wait_queue_empty();

        g_field_player_records[0].portrait_index = g_field_player_records[0].head.bits.alt_appearance;
        bcopy((u8*)g_field_cd_buffer + g_field_cd_buffer->offsets[g_field_player_records[0].portrait_index + 1], g_prim_rect_buf, FIELD_PORTRAIT_SIZE);

        partner_portrait = g_prim_rect_buf + FIELD_PORTRAIT_SIZE;
        g_field_player_records[1].portrait_index = partner_portrait_index;
        bcopy((u8*)g_field_cd_buffer + g_field_cd_buffer->offsets[g_field_player_records[1].portrait_index + 1], partner_portrait, FIELD_PORTRAIT_SIZE);

        companion_portrait = g_prim_rect_buf + FIELD_PORTRAIT_SIZE * 2;
        g_field_player_records[2].portrait_index = g_field_player_records[2].character_id + FIELD_PORTRAIT_COMPANION_BASE;
        bcopy((u8*)g_field_cd_buffer + g_field_cd_buffer->offsets[g_field_player_records[2].portrait_index + 1], companion_portrait, FIELD_PORTRAIT_SIZE);

        if (g_field_player_records[1].character_kind == FIELD_PLAYER_KIND_HERO)
        {
            field_copy_portrait_palette(partner_portrait, g_field_player_records[1].head.bits.alt_appearance);
        }

        if (g_pad_ctx->gname_name[0] != 0 && (g_pad_ctx->unkAA8 & COMPANION_KIND_MASK) == COMPANION_KIND_GOLEM)
        {
            golem_index = g_pad_ctx->large_history_index;
            if (golem_index < LARGE_HISTORY_RECORD_COUNT)
            {
                func_800A55E4(companion_portrait, g_pad_ctx->large_history_records[golem_index].unknown_0x48);
            }
        }
    }

    field_reset_actor_resources();
}

/**
 * @brief Free every field effect record and reset the effect allocator.
 */
void field_reset_effect_pool(void)
{
    s32 i;
    s32 free_presence;

    free_presence = FIELD_EFFECT_FREE;
    for (i = FIELD_EFFECT_RECORD_COUNT - 1; i >= 0; i--)
    {
        g_field_effect_records[i].presence = free_presence;
    }

    D_80105770 = 0;
}

/**
 * @brief Free every effect record emitted by an animation actor slot.
 * @param slot Animation actor slot whose effects are freed.
 */
void field_clear_actor_effects(FieldActorSlot* slot)
{
    s32 i;
    s32 owner_slot;
    s32 free_presence;
    FieldActor* effects;

    owner_slot = slot->slot_index;
    i = 0;
    free_presence = FIELD_EFFECT_FREE;
    effects = g_field_effect_records;
    while (i < FIELD_EFFECT_RECORD_COUNT)
    {
        if (effects->presence != free_presence && effects->owner_slot == owner_slot)
        {
            effects->presence = free_presence;
        }
        i++;
        effects++;
    }
}

/**
 * @brief Run the part effects of every active track of an animation actor slot.
 * @param slot Animation actor slot to process.
 */
static void field_update_actor_effects(FieldActorSlot* slot)
{
    s32 i;

    if (slot->target_count != 0)
    {
        for (i = 0; i < slot->target_count; i++)
        {
            if ((slot->track_mask >> i) & 1)
            {
                g_field_track_index = i;
                field_update_actor_part_effects(slot);
            }
        }
    }
    else
    {
        g_field_track_index = 0;
        field_update_actor_part_effects(slot);
    }
}

/**
 * @brief Spawn the effects of each part of an animation actor slot for the current track.
 * @param slot Animation actor slot to process.
 */
static void field_update_actor_part_effects(FieldActorSlot* slot)
{
    FieldObjectPart* part;
    s32 i;
    s32 target_count;
    s32 previous_count;
    s32 spawn_count;
    u8 start_frame;
    u32 flags;

    part = slot->parts;
    i = 0;
    for (; i < slot->part_count; i++, part++)
    {
        start_frame = part->unk31;
        if (start_frame != FIELD_PART_DISABLED &&
            (!(slot->animation->flags & FIELD_ANIM_KEEP_ALIVE) || ((slot->part_masks[slot->animation_index] >> i) & 1)) &&
            part->unkB != FIELD_PART_NO_EFFECT && (!(part->unk14.word & FIELD_PART_FIRST_TRACK_ONLY) || g_field_track_index == 0))
        {
            if (slot->targets[g_field_track_index] == FIELD_TARGET_NONE)
            {
                if (start_frame == FIELD_PART_ALWAYS || (part->flags & FIELD_PART_ATTACK_SPHERE))
                {
                    continue;
                }
                if (part->unk31 > slot->track_frames[g_field_track_index])
                {
                    continue;
                }
            }
            else if (start_frame != FIELD_PART_ALWAYS)
            {
                if (part->unk31 > slot->track_frames[g_field_track_index])
                {
                    continue;
                }
            }

            if (part->unk28.bytes[3] & 1)
            {
                target_count = part->unkC;
            }
            else
            {
                target_count = field_evaluate_parameter_track(slot, part->unkC);
            }

            flags = part->unk28.word;
            if (((flags >> FIELD_PART_SPAWN_CAP_SHIFT) & 1) && slot->effect_totals[g_field_track_index][i] >= target_count)
            {
                continue;
            }

            if ((part->unk0.word >> FIELD_PART_SPAWN_SINGLE_SHIFT) & 1)
            {
                if (((flags >> FIELD_PART_FIXED_COUNT_SHIFT) & 1) && slot->effect_counts[g_field_track_index][i] != 0)
                {
                    continue;
                }
                if (field_get_track_counter_modulo(slot, (part->unk4.bytes[3] & FIELD_PART_PERIOD_MASK) + 1) != 0)
                {
                    continue;
                }
                if (slot->effect_counts[g_field_track_index][i] >= target_count)
                {
                    continue;
                }
                do
                {
                    previous_count = slot->effect_counts[g_field_track_index][i];
                    D_80105760 = 0;
                    if (field_spawn_actor_effect(slot, i, 0) == -1)
                    {
                        break;
                    }
                    if (slot->effect_counts[g_field_track_index][i] == previous_count)
                    {
                        break;
                    }
                } while (slot->effect_counts[g_field_track_index][i] < target_count);
            }
            else
            {
                if (slot->effect_counts[g_field_track_index][i] < target_count &&
                    field_get_track_counter_modulo(slot, (part->unk4.bytes[3] & FIELD_PART_PERIOD_MASK) + 1) == 0)
                {
                    spawn_count = (part->unk2C & FIELD_PART_BURST_MASK) + 1;
                    while (spawn_count != 0)
                    {
                        if (slot->effect_counts[g_field_track_index][i] >= target_count)
                        {
                            break;
                        }
                        D_80105760 = 0;
                        if (field_spawn_actor_effect(slot, i, 0) == -1)
                        {
                            break;
                        }
                        spawn_count--;
                    }
                }
            }
        }
    }
}

/**
 * @brief Give a spawned effect its owner's part tint when the emitting part has a plain neutral colour.
 * @param effect Spawned effect record.
 * @param part Emitting part.
 * @param owner Field object that owns the emitter.
 */
static void field_apply_actor_part_color(FieldActor* effect, FieldObjectPart* part, FieldActor* owner)
{
    if (!((part->unk4.word >> FIELD_PART_OWN_COLOR_SHIFT) & 1) && !((part->unk28.word >> FIELD_PART_UNK25_SHIFT) & 1) &&
        (part->unk2C >> FIELD_PART_BURST_SHIFT) == 0 && (*(u32*)&part->unkC & 0xFFFF0000) == 0x80800000 &&
        part->tint_blue == FIELD_TINT_NEUTRAL)
    {
        effect->control.word |= FIELD_CONTROL_OWNER_TINT;
        effect->tint_red = g_field_object_parts[owner->object_index].tint_red;
        effect->tint_green = g_field_object_parts[owner->object_index].tint_green;
        effect->tint_blue = g_field_object_parts[owner->object_index].tint_blue;
    }
}
