#include "field_modal_runtime.h"
#include "field_scene_transition.h"
#include "field_text.h"
#include "cdrom.h"
#include "common.h"
#include "cd_resources.h"
#include "controller_internal.h"
#include "sdk/libgpu.h"
#include "sdk/libetc.h"

#define FIELD_SUBOVERLAY_ADDRESS ((void*)0x80140000)
#define FIELD_SAVED_CHARACTER_STRIDE 0x250
#define FIELD_INPUT_REPEAT_DELAY 15
#define FIELD_DUEL_PANEL_HOLD_FRAMES 90
#define FIELD_DUEL_PANEL_START_OFFSET 500

/**
 * @file field_modal_runtime.c
 * @brief Menu dispatch, modal overlay updates, and duel panels for FIELD.
 */

/** @brief Animation stages shared by the duel introduction and winner panels. */
typedef enum FieldDuelPanelPhase
{
    FIELD_DUEL_SLIDE_IN,
    FIELD_DUEL_HOLD,
    FIELD_DUEL_SLIDE_OUT,
    FIELD_DUEL_FINISHED
} FieldDuelPanelPhase;

/** @brief Party flags, weapon category, and action state in one 0x268-byte slot. */
typedef struct
{
    union
    {
        u16 word;
        struct
        {
            u8 flags, weapon_type;
        } bytes;
        struct
        {
            u16 active : 1;
            u16 selected : 1;
            u16 rest : 14;
        } bits;
    } head;
    u8 companion_id, companion_type;
    u8 unknown_0x004[0x250];
    u16 action_state;
    u8 action_index;
    u8 unknown_0x257;
    u8 weapon_flag;
    u8 unknown_0x259[0xF];
} FieldModalPartySlot;
/** @brief Actor flags within the 0x54-byte field record. */
typedef struct
{
    u8 unknown_0x000[0x1C];
    s32 flags;
    u8 unknown_0x020[0x34];
} FieldModalActor;
/** @brief Runtime health fields within the 0x23C-byte actor state. */
typedef struct
{
    s32 max_hp, hp, display_hp;
    u8 unknown_0x00c[0x230];
} FieldModalActorHealth;
/** @brief Eight-byte action descriptor with byte and halfword flag access. */
typedef struct
{
    s16 action_id;
    union
    {
        u16 word;
        struct
        {
            u8 low, high;
        } byte;
    } bits;
    s16 icon_id, texture_id;
} FieldModalAction;
/** @brief Selected item attributes at offsets 0x24 and 0x25. */
typedef struct
{
    u8 unknown_0x000[0x24];
    u8 instrument_type, spell;
} FieldModalInstrument;
/** @brief Accessed global settings and player fields within the saved context. */
typedef struct
{
    u8 unknown_0x000[0x18];
    u32 scene_config;
    s16 scene_option;
    s8 scene_sub_mode;
    u8 unknown_0x01f;
    s32 music_track;
    u16 scene_mode;
    u8 scene_flags;
    u8 layout_flags;
    u32 option_flags;
    u8 unknown_0x02c[0x5F0 - 0x2C];
    u8 name[24];
    u8 character_kind, character_id;
    u8 bound_actions[2];
    u8 equipped_abilities[4];
    u8 unknown_0x610[4];
    u16 max_hp;
    u8 unknown_0x616[0x1E];
    u16 duel_wins;
    u16 duel_losses;
    u8 unknown_0x638[8];
    u8 equipment_present_at_slot;
    u8 unknown_0x641[0x13];
    u32 equipment_flags;
    u8 unknown_0x658[0x1E8];
    u8 second_name[24];
    union
    {
        u32 word;
        u8 bytes[4];
    } second_character;
    u8 unknown_0x85c[0x884 - 0x85C];
    u16 second_duel_wins;
    u16 second_duel_losses;
    u8 unknown_0x888[0xCE0 - 0x888];
    u8 equipment_present;
    u8 unknown_0xce1[0x25E0 - 0xCE1];
    u8 item_counts[256];
} FieldModalSaveView;

/** @brief Unaligned little-endian offset into the field UI string table. */
typedef struct
{
    u8 low;
    u8 high;
} FieldModalStringOffset;

/** @brief Caller-owned block whose packet cursor lives at 0x40B8. */
typedef struct
{
    u8 unknown_0x000[0x40B8];
    s32 packet_cursor;
} FieldModalDrawContext;

s32 field_read_controller_buttons(s32);
void akao_cmd_98_9a_9c_9e(s32 context);

void func_800A3904(s32 context, s32 arg1, s32 arg2);
void field_draw_cd_error_text(s32 context);
void field_draw_actor_labels(s32 context);
void field_update_text_session(void);

void field_set_fade_target_only(s16 red, s16 green, s16 blue, s16 duration);
void func_800A3938(s32 sound_id, s32 pan);
void func_800AE9E0(void);
void* func_800A88A0(SPRT* cursor, s32* ot, u8* text, s32 color, s32 x, s32 y, s32 flags);
s32 field_name_byte_length(u8* context);
void field_copy_name(u8* dest, u8* src);
s32 func_800AEAC0(s32 packet_cursor, void* arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);
s32 func_800AF950(s32 packet_cursor, void* arg1, u8* str, s32 arg3, s32 x, s32 y, s32 arg6, s32 arg7, s32 arg8, s32 arg9, s32 arg10, s32 arg11);

void field_draw_empty_shop_notice(FieldModalDrawContext* context);
s32 field_draw_duel_intro(FieldModalDrawContext* context);
s32 field_draw_duel_result(FieldModalDrawContext* context);

extern u8 g_field_menu_controller_types[CONTROLLER_PORT_COUNT];
extern s32 g_field_primary_held_buttons;
extern s32 g_field_primary_repeat_delay;
extern s32 g_field_text_session_active;
extern s32 g_field_secondary_held_buttons;
extern s32 g_field_secondary_repeat_delay;
extern s32 D_8012291C;
extern s32 g_field_text_session_cd_error;
extern s32 g_field_buffered_input;
extern s32 g_pad_input;
extern s32 g_pad_input_inject;

extern s32 g_field_gosub_phase;
extern s32 D_801227F0;
extern s32 g_field_shop_active;
extern s32 g_gosub_result_count;

extern FieldModalStringOffset D_800EC400;
extern s32 g_field_duel_panel_phase;
extern s32 g_field_shop_notice_hidden;
extern s32 g_field_duel_panel_offset;
extern s32 g_field_duel_panel_hold_frames;

extern FieldModalStringOffset D_800EC3E4;
extern FieldModalStringOffset D_800EC406;
extern FieldModalStringOffset D_800EC408;
extern FieldModalStringOffset D_800EC40A;
extern FieldModalStringOffset D_800EC40C;
extern s32 g_field_duel_winner;

extern void func_80140024(u32, s32);
extern s32 func_801405B0(s32);
extern s32 g_field_actor_bindings[];
extern u8 D_801226B8[], D_801226F0[];
extern s32 D_8011F424, D_801227D4, D_801229F4, g_active_script, g_script_repeat_count;
extern void akao_set_paused(s32);
extern void field_refresh_party_routes(void);
extern void func_80091438(s32);
extern void func_800A3D44(s32, u8);
extern void func_800A5174(s32, s32);
extern void func_800A54D0(void);
extern u8 g_field_action_animation_parameters[];
extern u8 D_800EB24C[];
extern FieldModalPartySlot g_field_player_records[];
extern FieldModalActor g_field_actors[];
extern FieldModalActorHealth g_field_object_states[];
extern FieldModalAction g_field_resource_actions[];
extern s32 g_field_scene_mode_bit;
extern u8* g_pad_ctx;
extern u32 g_field_modal_state;
extern void field_restore_fade_target(void);
extern void field_release_actor_resource_slot(s32);
extern s32 field_activate_actor_resource_slot(s32, s32, s32);
extern void func_80084240(void);
extern void func_800A39A8(s32, s32, s32, s32);
extern void func_800A7384(void);

extern s32 func_800B0888(void);
extern s32 func_801400C4(s32);
extern s32 func_801400D4(s32);
extern s32 func_801401F0(s32);
extern s32 func_801401F8(s32);
extern s32 func_80140370(s32);
extern s32 g_field_niki_addhero_state;
extern s32 g_field_card_overlay_mode;
extern s32 g_gosub_result_values;
extern s16 g_music_track_index;
extern u8 D_800FDF79;
/* GNAME and SHOP share this entry address but have different parameter lists. */
extern void func_80140004();

/** @brief True for a DBCS lead byte (0x19-0x1F), which owns the following byte. */
#define IS_DBCS(c) ((u32)((c) - 0x19) < 7)

/** @brief Write a signed decimal into @p record_text (minus-sign glyph from the string table). */
#define FORMAT_SIGNED(record_text, val)                                                                                                                        \
    {                                                                                                                                                          \
        u8* dst;                                                                                                                                               \
        s32 value;                                                                                                                                             \
        s32 double_byte_digits;                                                                                                                                \
        u8* minus;                                                                                                                                             \
        s32 low;                                                                                                                                               \
        s32 offset;                                                                                                                                            \
        s32 div;                                                                                                                                               \
        s32 started;                                                                                                                                           \
        s32 digit;                                                                                                                                             \
        dst = record_text;                                                                                                                                     \
        value = val;                                                                                                                                           \
        double_byte_digits = 0;                                                                                                                                \
        if (value < 0)                                                                                                                                         \
        {                                                                                                                                                      \
            value = -value;                                                                                                                                    \
            low = D_800EC3E4.low;                                                                                                                              \
            offset = (D_800EC3E4.high << 8) + (s32)((u8*)&D_800EC3E4 - 0x20);                                                                                  \
            minus = (u8*)(low + offset);                                                                                                                       \
            field_copy_name(dst, minus);                                                                                                                       \
            dst += field_name_byte_length(minus);                                                                                                              \
        }                                                                                                                                                      \
        div = 10000000;                                                                                                                                        \
        started = 0;                                                                                                                                           \
        do                                                                                                                                                     \
        {                                                                                                                                                      \
            digit = value / div;                                                                                                                               \
            if (digit != 0)                                                                                                                                    \
            {                                                                                                                                                  \
                started = 1;                                                                                                                                   \
            }                                                                                                                                                  \
            if (started || div == 1)                                                                                                                           \
            {                                                                                                                                                  \
                if (double_byte_digits)                                                                                                                        \
                {                                                                                                                                              \
                    *dst++ = 0x1D;                                                                                                                             \
                    *dst = digit;                                                                                                                              \
                }                                                                                                                                              \
                else                                                                                                                                           \
                {                                                                                                                                              \
                    *dst = digit + '0';                                                                                                                        \
                }                                                                                                                                              \
                dst++;                                                                                                                                         \
                value -= (value / div) * div;                                                                                                                  \
            }                                                                                                                                                  \
            div /= 10;                                                                                                                                         \
        } while (div != 0);                                                                                                                                    \
        *dst = 0;                                                                                                                                              \
    }

/** @brief Advance @p p to the terminator, accumulating the DBCS-aware byte length. */
#define STR_LEN_LOOP(p, len)                                                                                                                                   \
    while (*p != 0)                                                                                                                                            \
    {                                                                                                                                                          \
        if (IS_DBCS(*p))                                                                                                                                       \
        {                                                                                                                                                      \
            p += 2;                                                                                                                                            \
            len += 2;                                                                                                                                          \
        }                                                                                                                                                      \
        else                                                                                                                                                   \
        {                                                                                                                                                      \
            p += 1;                                                                                                                                            \
            len += 1;                                                                                                                                          \
        }                                                                                                                                                      \
    }

/** @brief Shared strcat body: append @p s_ after the last glyph of @p d_. */
#define STR_CAT_BODY(d_, s_, qsrc)                                                                                                                             \
    {                                                                                                                                                          \
        volatile u8* p = d_;                                                                                                                                   \
        s32 len_d = 0;                                                                                                                                         \
        volatile u8* q;                                                                                                                                        \
        s32 len_s;                                                                                                                                             \
        s32 append;                                                                                                                                            \
        s32 i;                                                                                                                                                 \
        STR_LEN_LOOP(p, len_d)                                                                                                                                 \
        q = qsrc;                                                                                                                                              \
        len_s = 0;                                                                                                                                             \
        append = len_d;                                                                                                                                        \
        STR_LEN_LOOP(q, len_s)                                                                                                                                 \
        for (i = 0; i < len_s; i++)                                                                                                                            \
        {                                                                                                                                                      \
            d_[i + append] = s_[i];                                                                                                                            \
        }                                                                                                                                                      \
        d_[i + append] = 0;                                                                                                                                    \
    }

/** @brief Append buffer @p s onto @p d. */
#define STR_CAT(d, s)                                                                                                                                          \
    {                                                                                                                                                          \
        u8* d_ = (d);                                                                                                                                          \
        u8* s_ = (s);                                                                                                                                          \
        STR_CAT_BODY(d_, s_, (s))                                                                                                                              \
    }

/** @brief Append string-table entry @p sym onto @p d. */
#define STR_CAT_ENTRY(d, sym, off)                                                                                                                             \
    {                                                                                                                                                          \
        u8* d_ = (d);                                                                                                                                          \
        u8* s_ = (u8*)(((sym).high << 8) + (sym).low);                                                                                                         \
        s_ += (s32)((u8*)&(sym) - (off));                                                                                                                      \
        STR_CAT_BODY(d_, s_, s_)                                                                                                                               \
    }

/** @brief Panel contents are drawn only while the panel is entering or leaving. */
#define DRAW_FLAG (g_field_duel_panel_phase == FIELD_DUEL_SLIDE_IN || g_field_duel_panel_phase == FIELD_DUEL_SLIDE_OUT)

/**
 * @brief Run the menu overlay and dispatch its requested follow-up screens.
 * @param render_buffers Pair of MENU render buffers.
 * @param input_source Caller input-source selector; unused by this routine.
 * @note Fixed entry addresses are reused by the overlays loaded before each call.
 */
void field_run_menu(void* render_buffers, s32 input_source)
{
    s32 menu_locked;
    ControllerState* controllers;
    s32 screen_id;

    controllers = CONTROLLER_STATE;
    menu_locked = D_8012291C;
    g_field_text_session_cd_error = 0;
    controllers->ports[0].small_motor_command = 0;
    controllers->ports[0].actuator_control.fields.large_motor_command = 0;
    controllers->ports[1].small_motor_command = 0;
    controllers->ports[1].actuator_control.fields.large_motor_command = 0;
    if (menu_locked != 0 || g_field_actor_bindings[0] != 0 || g_field_actor_bindings[7] != 0 || g_field_actor_bindings[14] != 0)
    {
        field_begin_text_session();
        return;
    }
    func_800A3938(0x80, 0x80);
    func_80084240();

    g_active_script = 0;

    for (;;)
    {
        cdrom_stream(CD_RES_MENU_BIN, FIELD_SUBOVERLAY_ADDRESS);
        cdrom_wait_queue_empty();
        screen_id = func_801405B0((s32)render_buffers);
        if (screen_id == 0)
        {
            field_rebuild_party_actions(1);
            break;
        }
        if (screen_id == 0xA)
        {
            field_rebuild_party_actions(1);
            cdrom_stream(CD_RES_GOLEM_BIN, FIELD_SUBOVERLAY_ADDRESS);
            cdrom_wait_queue_empty();
            func_80140024(0x80150000, 1);
            func_800C3BB0();
            func_80084240();
            field_text_reset_windows();
            g_active_script = screen_id;
            g_script_repeat_count = 0;
        }
        else
        {
            field_rebuild_party_actions(1);
            cdrom_stream(CD_RES_GNAME_BIN, FIELD_SUBOVERLAY_ADDRESS);
            cdrom_wait_queue_empty();
            if ((u32)(screen_id - 0xB) < 2U)
            {
                func_80140004((void*)0x80160000, D_801226F0, D_801227D4, 1, D_801229F4, D_801226B8, 0);
            }
            else
            {
                func_80140004((void*)0x80160000, D_801226F0, D_801227D4, screen_id, D_801229F4, D_801226B8, 0);
            }
            field_text_reset_windows();
            g_script_repeat_count = D_801229F4;
            if ((u32)(screen_id - 0xB) < 2U)
            {
                g_script_repeat_count = 0;
                g_active_script = screen_id;
            }
            else
            {
                g_active_script = D_8011F424 + 1;
            }
        }
    }
    func_80084240();
}

/**
 * @brief Open the field text session and reset both controller repeat timers.
 * @see decomp.me (100%) TODO
 */
void field_begin_text_session(void)
{
    g_field_text_session_active = 1;
    field_set_cd_error_fade_target();
    g_pad_input = 0;
    g_field_primary_held_buttons = field_read_controller_buttons(0);
    g_field_primary_repeat_delay = FIELD_INPUT_REPEAT_DELAY;
    g_pad_input_inject = 0;
    g_field_secondary_held_buttons = field_read_controller_buttons(1);
    g_field_secondary_repeat_delay = FIELD_INPUT_REPEAT_DELAY;
    g_field_buffered_input = 0;
    field_init_actor_labels();
}

/**
 * @brief Clear text-session flags and remember both controller device types.
 */
void field_reset_text_session(void)
{
    ControllerState* controllers = CONTROLLER_STATE;

    g_field_text_session_active = 0;
    D_8012291C = 0;
    g_field_menu_controller_types[0] = controllers->ports[0].published_sample.device_type;
    g_field_menu_controller_types[1] = controllers->ports[1].published_sample.device_type;
}

/**
 * @brief Update the active text session, draw its contents, and handle closing.
 * @param context Render context forwarded to the text renderer.
 * @see decomp.me (100%) TODO
 */
void field_update_modal_text_session(s32 context)
{
    if (g_field_text_session_active != 0)
    {
        field_update_text_session();
        if (g_field_text_session_active != 0)
        {
            field_text_reset_scratch();
            if (g_field_text_session_cd_error != 0)
            {
                field_draw_cd_error_text(context);
            }
            else
            {
                field_draw_actor_labels(context);
            }
            field_text_upload_immediate_cache();
            return;
        }
        field_text_reset_windows();
        akao_cmd_98_9a_9c_9e(2);
        func_800A3904(0, 0x3C, 0x7F);
    }
}

/**
 * @brief Apply saved player settings and rebuild the party action descriptors.
 * @param refresh_only Nonzero preserves the current party membership and health values.
 * @note Zero also reloads active-party data and palettes before refreshing actions.
 * @note Packed descriptor byte writes preserve the other flag byte.
 */
void field_rebuild_party_actions(s32 refresh_only)
{
    FieldModalPartySlot* initial_party;
    FieldModalPartySlot* party;
    FieldModalPartySlot* equipment_owner;
    s32 texture_value;
    s16 technique_action_id;
    u8* item_context;
    s32 context_offset;
    s32 record_offset;
    s32 absent;
    s32 ability_empty;
    FieldModalAction* record_base;
    u8** context_pointer;
    u8* pair_first;
    u8* pair_second;
    FieldModalSaveView* input_base;
    s32 character_kind;
    s32 controller_or_player_test;
    s32 equipment_offset;
    s32 equipment_index;
    s32 button_index;
    s32 action_offset;
    s32 saved_offset;
    s32 actor_stride_words;
    s32 player_index;
    s32 context_stride;
    s32 record_stride;
    s32 state_stride;
    s32 item_record_offset;
    u16 max_hp;
    u32 equipment_flags;
    u8 spell;
    u8 weapon_type;
    u8 ability_id;
    FieldModalActor* actor;
    FieldModalActor* actor_base;
    FieldModalAction* button_action;
    FieldModalInstrument* instrument;
    FieldModalAction* technique_action;
    FieldModalSaveView* saved_player;
    FieldModalAction* instrument_action;
    FieldModalSaveView* ability_start;
    FieldModalAction* empty_action;
    FieldModalSaveView* saved_member;
    FieldModalActorHealth* health;
    FieldModalSaveView* equipment;
    FieldModalSaveView* button;
    FieldModalSaveView* item_cursor;

    akao_set_paused((((u32)((FieldModalSaveView*)g_pad_ctx)->option_flags >> 1) & 1) ^ 1);
    cdrom_set_audio_volume(0x7F, ((u32)((FieldModalSaveView*)g_pad_ctx)->option_flags >> 1) & 1);
    controller_or_player_test = (s32)CONTROLLER_STATE;
    ((ControllerState*)controller_or_player_test)->ports[0].actuators_enabled = (s8)(*(volatile u32*)&((FieldModalSaveView*)g_pad_ctx)->option_flags & 1);
    if ((((FieldModalSaveView*)g_pad_ctx)->second_character.word & 0x80) && (((FieldModalSaveView*)g_pad_ctx)->second_name[0] != 0))
    {
        ((ControllerState*)controller_or_player_test)->ports[1].actuators_enabled = (s8)(*(volatile u32*)&((FieldModalSaveView*)g_pad_ctx)->option_flags & 1);
    }
    else
    {
        ((ControllerState*)controller_or_player_test)->ports[1].actuators_enabled = 0;
    }
    player_index = 0;
    if (refresh_only == 0)
    {
        do
        {
            saved_offset = player_index * FIELD_SAVED_CHARACTER_STRIDE;
            initial_party = &g_field_player_records[player_index];

            if (((FieldModalSaveView*)(g_pad_ctx + saved_offset))->name[0] != 0)
            {
                initial_party->head.bytes.weapon_type = 0xFF;
                saved_member = (FieldModalSaveView*)(g_pad_ctx + saved_offset);
                initial_party->head.bits.active = 1;
                button_index = saved_member->character_kind & 0x7F;
                character_kind = button_index;
                if (character_kind < 2)
                {
                    initial_party->head.bits.selected = character_kind;
                    initial_party->companion_type = 0;
                }
                else if (character_kind == 2)
                {
                    initial_party->head.bits.selected = 0;
                    initial_party->companion_id = saved_member->character_id;
                    initial_party->companion_type = 1;
                }
                else if (character_kind == 3)
                {
                    initial_party->head.bits.selected = 0;
                    initial_party->companion_id = saved_member->character_id;
                    initial_party->companion_type = 2;
                }
                else if (character_kind == 4)
                {
                    initial_party->head.bits.selected = 0;
                    initial_party->companion_id = saved_member->character_id + 0x41;
                    initial_party->companion_type = 2;
                }
            }
            else
            {
                initial_party->companion_type = 1;
                initial_party->companion_id = 0U;
                initial_party->head.bits.active = 0;
            }
            player_index += 1;

        } while (player_index < 3);
        player_index = 0;
        func_800A54D0();
    }
    player_index = 0;
    record_base = g_field_resource_actions;
    context_pointer = &g_pad_ctx;
    actor_stride_words = player_index;
    record_stride = player_index;
    context_stride = player_index;
    state_stride = player_index;
    do
    {
        party = &g_field_player_records[player_index];

        if (party->head.bytes.flags & 1)
        {
            actor_base = g_field_actors;
            actor = (FieldModalActor*)((actor_stride_words + player_index) * 4 + (u8*)actor_base);
            saved_player = (FieldModalSaveView*)((*context_pointer) + context_stride);
            actor->flags = (s32)((actor->flags & ~0x1FF) | (((u8)saved_player->character_kind >> 7) ^ 1));
            weapon_type = ((u32)saved_player->equipment_flags >> 0xA) & 0x3F;
            controller_or_player_test = player_index < 2;
            if (party->head.bytes.weapon_type != weapon_type)
            {
                party->head.bytes.weapon_type = weapon_type;
                if (controller_or_player_test != 0)
                {
                    func_80091438(player_index);
                }
                if (refresh_only == 0)
                {
                    health = (FieldModalActorHealth*)(state_stride + (u8*)g_field_object_states);
                    max_hp = ((FieldModalSaveView*)((*context_pointer) + context_stride))->max_hp;
                    health->display_hp = (s32)((health->display_hp & 0xFF000000) | max_hp);
                    health->max_hp = (s32)max_hp;
                    health->hp = (s32)max_hp;
                }
                if ((g_field_scene_mode_bit != 0) && (refresh_only != 0) && (controller_or_player_test != 0))
                {
                    func_800A3D44(player_index, party->head.bytes.weapon_type);
                }
            }
            party->weapon_flag = 0;
            if ((u32)(party->head.bytes.weapon_type - 1) < 2U)
            {
                party->weapon_flag = 1;
                equipment_index = 1;
                equipment_owner = party;
                equipment_offset = context_stride + 0x40;
                do
                {
                    equipment = (FieldModalSaveView*)((*context_pointer) + equipment_offset);
                    if (equipment->equipment_present_at_slot != 0)
                    {
                        equipment_flags = equipment->equipment_flags;
                        if ((((equipment_flags >> 8) & 3) == 1) && !((equipment_flags >> 0xA) & 0x3F))
                        {
                            equipment_owner->weapon_flag = 0;
                        }
                    }
                    equipment_index += 1;
                    equipment_offset += 0x40;
                } while (equipment_index < 4);
            }
            if (player_index == 2 && g_field_player_records[2].companion_type == player_index)
            {
                func_800A5174(2, g_field_player_records[2].companion_id + 0xA9B);
            }
            else
            {
                button_index = 0;
                pair_first = g_field_action_animation_parameters;
                pair_second = g_field_action_animation_parameters + 1;
                input_base = (FieldModalSaveView*)((*context_pointer) + context_stride);
                action_offset = record_stride;
                do
                {
                    button = (FieldModalSaveView*)((u8*)input_base + button_index);
                    button_action = (FieldModalAction*)(action_offset + (u32)record_base);
                    button_action->action_id = (s16)button->bound_actions[0];
                    button_action->icon_id = (s16) * ((button->bound_actions[0] * 2) + pair_first);
                    button_action->texture_id = (s16) * ((button->bound_actions[0] * 2) + pair_second);
                    action_offset += 8;
                    button_index += 1;
                } while (button_index < 2);
                context_offset = context_stride;
                absent = 0xFF;
                record_offset = record_stride;
                item_context = *context_pointer;
                ability_start = (FieldModalSaveView*)(item_context + context_offset);
                item_record_offset = 0x20;
                item_cursor = ability_start;
            rebuild_equipped_ability:

                ability_empty = item_cursor->equipped_abilities[0] == absent;
                if (ability_empty)
                {
                    empty_action = (FieldModalAction*)(item_record_offset + record_offset + (u32)record_base);
                    empty_action->bits.word = (u16)(empty_action->bits.word & 0xFBFF);
                    empty_action->bits.byte.low = absent;
                    empty_action->action_id = 0;
                    empty_action->icon_id = 0;
                    empty_action->texture_id = 0;
                    empty_action->bits.word = (u16)(empty_action->bits.word & 0xFCFF);
                }
                else
                {
                    ability_id = item_cursor->equipped_abilities[0];
                    if (ability_id & 0x80)
                    {
                        instrument_action = (FieldModalAction*)(item_record_offset + record_offset + (u32)record_base);
                        instrument_action->action_id = 0;
                        instrument_action->bits.word = (u16)(instrument_action->bits.word | 0x400);
                        instrument = (FieldModalInstrument*)(item_context + (context_offset + 0x5F0) + (((ability_id & 0x7F) << 6) + 0x150));
                        instrument_action->bits.byte.low = (s8)((u8)instrument->spell >> 1);
                        instrument_action->icon_id = (s16) * (instrument->instrument_type + D_800EB24C);
                        spell = instrument->spell;
                        texture_value = 0x8018;
                        texture_value += spell;
                        texture_value += instrument->instrument_type * 0xE;
                        if (!(spell & 1))
                        {
                            texture_value += 0x800;
                        }
                        instrument_action->texture_id = texture_value;
                        instrument_action->bits.word = (u16)(instrument_action->bits.word & 0xFCFF);
                    }
                    else
                    {
                        technique_action = (FieldModalAction*)(item_record_offset + record_offset + (u32)record_base);
                        technique_action->bits.word = (u16)(technique_action->bits.word & 0xFBFF);
                        ability_id = item_cursor->equipped_abilities[0];
                        technique_action_id = (s16)(ability_id | 0x8000);
                        technique_action->bits.byte.low = absent;
                        technique_action->icon_id = 2;
                        technique_action->action_id = technique_action_id;
                        ability_id = item_cursor->equipped_abilities[0];
                        weapon_type = party->head.bytes.weapon_type;
                        technique_action->bits.word = (u16)(technique_action->bits.word & 0xFCFF);
                        technique_action->texture_id = (s16)(((ability_id + 0x88) | ~0x7FFF) + (weapon_type * 0x18));
                    }
                }
                item_cursor = (FieldModalSaveView*)((u8*)item_cursor + 1);
                item_record_offset += 8;

                if ((s32)item_cursor < (s32)((u8*)ability_start + 4))
                {
                    goto rebuild_equipped_ability;
                }
            }
        }
        player_index += 1;
        record_stride += 0x190;
        context_stride += FIELD_SAVED_CHARACTER_STRIDE;
        actor_stride_words += 0x14;
        state_stride += 0x23C;

    } while (player_index < 3);
    field_refresh_party_routes();
}

/**
 * @brief Load GNAME.BIN and run the name-entry screen.
 *
 * Argument order follows gname_run, which receives the 0x80160000 render
 * buffers first and allow_empty_cancel = 0 last.
 *
 * @param initial_name Name shown when the screen opens.
 * @param active_name Name buffer edited by the UI.
 * @param source_mode Random-name source selector.
 * @param history_index History-list entry selector.
 * @param custom_name Custom random-name source.
 * @see decomp.me (100%) TODO
 */
void field_run_name_entry(s32 initial_name, s32 active_name, s32 source_mode, s32 history_index, s32 custom_name)
{
    func_80084240();
    cdrom_stream(CD_RES_GNAME_BIN, FIELD_SUBOVERLAY_ADDRESS);
    cdrom_wait_queue_empty();
    func_80140004((void*)0x80160000, initial_name, active_name, source_mode, history_index, custom_name, 0);
    field_text_reset_windows();
    func_80084240();
}

/**
 * @brief Load ZUKAN.BIN and run its entry point at 0x80140E00.
 * @param context Forwarded to the ZUKAN entry point; meaning not yet established.
 */
void field_run_zukan(s32 context)
{
    func_80084240();
    cdrom_stream(CD_RES_ZUKAN_BIN, FIELD_SUBOVERLAY_ADDRESS);
    cdrom_wait_queue_empty();
    func_80140E00((void*)0x80160000, context);
    func_80084240();
}

/**
 * @brief Load GOSUB.BIN and open a screen sequence, unless a sub-overlay is already active.
 *
 * Sets g_field_modal_state to 2 for the duration and clears the gosub result count.
 *
 * @param screen_sequence Terminated s32 array passed to gosub_open_screen_sequence.
 * @see decomp.me (100%) TODO
 */
void field_open_gosub_screen_sequence(void* screen_sequence)
{
    if (g_field_modal_state == FIELD_MODAL_NONE)
    {
        D_801227F0 = 1;
        g_gosub_result_count = 0;
        func_80084240();
        cdrom_stream(CD_RES_GOSUB_BIN, FIELD_SUBOVERLAY_ADDRESS);
        cdrom_wait_queue_empty();
        g_field_modal_state = FIELD_MODAL_GOSUB;
        g_field_gosub_phase = 2;
        func_80140080((void*)0x80175000, screen_sequence);
    }
}

/**
 * @brief Open the selling screen when inventory is present, otherwise show a notice.
 * @param shop_options Value forwarded to the shop; meaning not yet established.
 * @note The equipment loop repeatedly checks the first record, as in the original.
 * @see decomp.me (100%) TODO
 */
void field_open_shop_mode_0(s32 shop_options)
{
    s32 count;
    s32 i;

    if (g_field_modal_state == FIELD_MODAL_NONE)
    {
        count = 0;
        for (i = 0; i < 0x64; i++)
        {
            if (((FieldModalSaveView*)g_pad_ctx)->equipment_present != 0)
            {
                count++;
                break;
            }
        }
        for (i = 0; i < 0x100; i++)
        {
            if (((FieldModalSaveView*)g_pad_ctx)->item_counts[i] != 0)
            {
                count++;
                break;
            }
        }
        if (count == 0)
        {
            field_begin_empty_shop_notice(0);
        }
        else
        {
            func_80084240();
            cdrom_stream(CD_RES_SHOP_BIN, FIELD_SUBOVERLAY_ADDRESS);
            cdrom_wait_queue_empty();
            g_field_shop_active = 1;
            g_field_modal_state = FIELD_MODAL_SHOP;
            func_80140004((void*)0x80150000, 0, 0, 0, 0, shop_options);
        }
    }
}

/**
 * @brief Open the shop with a caller-supplied entry list.
 * @param entry_count Number of entries in the supplied list.
 * @param entries Address of the eight-byte shop entry array.
 * @param list_options Additional list data forwarded to the shop; meaning unresolved.
 * @param shop_options Value forwarded as the final shop argument.
 * @see decomp.me (100%) TODO
 */
void field_open_shop_mode_1(s32 entry_count, s32 entries, s32 list_options, s32 shop_options)
{
    if (g_field_modal_state == FIELD_MODAL_NONE)
    {
        func_80084240();
        cdrom_stream(CD_RES_SHOP_BIN, FIELD_SUBOVERLAY_ADDRESS);
        cdrom_wait_queue_empty();
        g_field_shop_active = 1;
        g_field_modal_state = FIELD_MODAL_SHOP;
        func_80140004((void*)0x80150000, 1, entry_count, entries, list_options, shop_options);
    }
}

/**
 * @brief Advance the active modal overlay and restore field input on completion.
 * @param context_or_delay Render context address, reused for the input repeat delay.
 */
void field_update_modal(s32 context_or_delay)
{
    FieldModalPartySlot* slot;
    s32 result;
    s32 index_or_zero;

    switch (g_field_modal_state)
    {
    case FIELD_MODAL_SHOP:
        if ((g_field_shop_active != 0) && (func_801400D4(context_or_delay) != 0))
        {
            g_field_shop_active = 0;
            g_field_modal_state = FIELD_MODAL_NONE;
            func_80084240();
            return;
        }
    case FIELD_MODAL_NONE:
        return;
    case FIELD_MODAL_GOSUB:
        if (g_field_gosub_phase != 0)
        {
            if (g_field_gosub_phase >= 2)
            {
                if (func_801400C4(context_or_delay) != 0)
                {
                    DrawSync(0);
                    g_field_gosub_phase = 1;
                    func_80084240();
                    return;
                }
            }
            else
            {
                DrawSync(0);
                field_text_reset_windows();
                D_801227F0 = 2;
                g_field_modal_state = FIELD_MODAL_NONE;
                g_field_gosub_phase = 0;
                return;
            }
        }
        return;
    case FIELD_MODAL_CARDA:
        if ((g_field_card_overlay_mode != 0) && (func_80140370(context_or_delay) != 0))
        {
            func_80084240();
            switch (g_field_card_overlay_mode)
            {
            case 2:
                field_rebuild_party_actions(0);
                index_or_zero = 0;
                slot = g_field_player_records;
                do
                {
                    slot = &g_field_player_records[index_or_zero];
                    slot->action_state = 0;
                    slot->action_index = 0xFF;
                    index_or_zero += 1;
                } while (index_or_zero < 3);
                g_music_track_index = (s16) * (volatile s32*)&((FieldModalSaveView*)g_pad_ctx)->music_track;
                field_set_scene_parameters(((FieldModalSaveView*)g_pad_ctx)->scene_mode, ((FieldModalSaveView*)g_pad_ctx)->scene_flags,
                                           ((FieldModalSaveView*)g_pad_ctx)->scene_config & 0x01FFFFFF, ((FieldModalSaveView*)g_pad_ctx)->layout_flags,
                                           (s32)((FieldModalSaveView*)g_pad_ctx)->scene_option, (s32)((FieldModalSaveView*)g_pad_ctx)->scene_sub_mode);
                field_set_fade_target_only(0x100, 0x100, 0x100, 8);
                break;
            case 6:
            case 7:
                g_gosub_result_values = 6;
                /* fall through */
            default:
            case 3:
            case 4:
            case 5:
                break;
            }
            g_gosub_result_count = 1;
            D_801227F0 = 2;
            g_field_card_overlay_mode = 0;
            g_field_modal_state = FIELD_MODAL_NONE;
            return;
        }
        return;
    case FIELD_MODAL_NIKI:
        if ((g_field_niki_addhero_state != 0) && (func_801401F0(context_or_delay) != 0))
        {
            func_80084240();
            g_field_niki_addhero_state = 0;
            g_field_modal_state = FIELD_MODAL_NONE;
            return;
        }
        return;
    case FIELD_MODAL_ADDHERO:
        if (g_field_niki_addhero_state != 0)
        {
            result = func_801401F8(context_or_delay);
            switch (result)
            {
            case 1:
                g_field_player_records[1].head.bits.active = 0;
                g_field_player_records[1].head.bits.selected = ((FieldModalSaveView*)g_pad_ctx)->second_character.bytes[0] & 1;
                g_field_player_records[1].companion_type = 0;
                field_activate_actor_resource_slot(-2, 0, 0);
                func_80084240();
                g_field_niki_addhero_state = 0;
                g_field_modal_state = FIELD_MODAL_NONE;
                return;
            case 3:
                func_80084240();
                g_field_niki_addhero_state = 0;
                g_field_modal_state = FIELD_MODAL_NONE;
                return;
            case 2:
                field_release_actor_resource_slot(0);
                func_80084240();
                g_field_niki_addhero_state = 0;
                g_field_modal_state = FIELD_MODAL_NONE;
                return;
            }
        }
        return;
    case FIELD_MODAL_EMPTY_SHOP:
        if (g_pad_input & (PADRright | PADi))
        {
            field_text_reset_windows();
            index_or_zero = 0;
            g_field_modal_state = FIELD_MODAL_NONE;
            g_pad_input = 0;
            break;
        }
        field_text_reset_scratch();
        field_draw_empty_shop_notice((void*)context_or_delay);
        field_text_upload_immediate_cache();
        return;
    case FIELD_MODAL_DUEL_INTRO:
        if (field_draw_duel_intro((void*)context_or_delay) == 0)
        {
            return;
        }
        func_800A39A8(0, 0x80, 0, 3);
        field_text_reset_windows();
        index_or_zero = 0;
        g_field_modal_state = FIELD_MODAL_NONE;
        g_pad_input = 0;
        break;
    case FIELD_MODAL_DUEL_RESULT:
        if (field_draw_duel_result((void*)context_or_delay) == 0 || func_800B0888() != 0)
        {
            return;
        }
        func_800A7384();
        field_text_reset_windows();
        index_or_zero = 0;
        g_field_modal_state = FIELD_MODAL_NONE;
        g_pad_input = 0;
        break;
    default:
        return;
    }
    g_field_primary_held_buttons = field_read_controller_buttons(index_or_zero);
    context_or_delay = FIELD_INPUT_REPEAT_DELAY;
    g_field_primary_repeat_delay = context_or_delay;
    g_pad_input_inject = 0;
    g_field_secondary_held_buttons = field_read_controller_buttons(1);
    g_field_secondary_repeat_delay = context_or_delay;
    g_field_buffered_input = 0;
    field_restore_fade_target();
}

/**
 * @brief Open the empty-inventory notice with a red fade.
 * @param hidden Nonzero suppresses the notice text.
 */
void field_begin_empty_shop_notice(s32 hidden)
{
    g_field_modal_state = FIELD_MODAL_EMPTY_SHOP;
    field_set_fade_target_only(0xC0, 0x80, 0x80, 8);
    func_800A3938(0xC7, 0x80);
    g_field_shop_notice_hidden = hidden;
}

/**
 * @brief Draw the empty-inventory notice unless its text is suppressed.
 * @param context Field drawing context and packet cursor.
 */
void field_draw_empty_shop_notice(FieldModalDrawContext* context)
{
    s32 packet_cursor;
    s32 low;
    s32 offset;
    u8* base;

    packet_cursor = context->packet_cursor;
    if (g_field_shop_notice_hidden == 0)
    {
        low = D_800EC400.low;
        offset = (D_800EC400.high << 8) + (s32)(base = (u8*)&D_800EC400 - 0x3C);
        packet_cursor = (s32)func_800A88A0((SPRT*)packet_cursor, (s32*)context, (u8*)(low + offset), 4, 0xA0, 0x68, 2);
    }
    context->packet_cursor = packet_cursor;
}

/**
 * @brief Start the duel introduction panels sliding in from off-screen.
 */
void field_begin_duel_intro(void)
{
    g_field_modal_state = FIELD_MODAL_DUEL_INTRO;
    field_set_fade_target_only(0xC0, 0xC0, 0xC0, 8);
    func_800AE9E0();
    func_800A3938(0x125, 0x80);
    g_field_duel_panel_phase = FIELD_DUEL_SLIDE_IN;
    g_field_duel_panel_hold_frames = 0;
    g_field_duel_panel_offset = FIELD_DUEL_PANEL_START_OFFSET;
}

/**
 * @brief Select the duel winner, update win/loss records, and start the result panel.
 */
void field_begin_duel_result(void)
{
    g_field_modal_state = FIELD_MODAL_DUEL_RESULT;
    field_set_fade_target_only(0xC0, 0xC0, 0xC0, 8);
    func_800AE9E0();
    func_800A3938(0x126, 0x80);
    g_field_duel_panel_phase = FIELD_DUEL_SLIDE_IN;
    g_field_duel_panel_hold_frames = 0;
    g_field_duel_panel_offset = FIELD_DUEL_PANEL_START_OFFSET;

    if ((D_800FDF79 & 0x7F) == 0x1D)
    {
        g_field_duel_winner = 1;
        ((FieldModalSaveView*)g_pad_ctx)->duel_losses = ((FieldModalSaveView*)g_pad_ctx)->duel_losses + 1;
        if ((u32)(((FieldModalSaveView*)g_pad_ctx)->second_character.bytes[0] & 0x7F) < 2)
        {
            ((FieldModalSaveView*)g_pad_ctx)->second_duel_wins = ((FieldModalSaveView*)g_pad_ctx)->second_duel_wins + 1;
        }
    }
    else
    {
        g_field_duel_winner = 0;
        ((FieldModalSaveView*)g_pad_ctx)->duel_wins = ((FieldModalSaveView*)g_pad_ctx)->duel_wins + 1;
        if ((u32)(((FieldModalSaveView*)g_pad_ctx)->second_character.bytes[0] & 0x7F) < 2)
        {
            ((FieldModalSaveView*)g_pad_ctx)->second_duel_losses = ((FieldModalSaveView*)g_pad_ctx)->second_duel_losses + 1;
        }
    }
}

/**
 * @brief Animate and draw the opposing players and their duel records.
 * @param context Field drawing context and packet cursor.
 * @return Nonzero after the panels have slid out.
 */
s32 field_draw_duel_intro(FieldModalDrawContext* context)
{
    u8 record_text[0x38];
    u8 loss_text[0x38];
    s32 packet_cursor;
    void* draw_context;

    packet_cursor = context->packet_cursor;
    draw_context = context;
    switch (g_field_duel_panel_phase)
    {
    case FIELD_DUEL_SLIDE_IN:
        g_field_duel_panel_offset /= 2;
        if (g_field_duel_panel_offset == 0)
        {
            g_field_duel_panel_phase = FIELD_DUEL_HOLD;
            g_field_duel_panel_hold_frames = FIELD_DUEL_PANEL_HOLD_FRAMES;
        }
        break;
    case FIELD_DUEL_HOLD:
        g_field_duel_panel_hold_frames--;
        if (g_field_duel_panel_hold_frames == 0)
        {
            func_800A3938(0x127, 0x80);
            g_field_duel_panel_phase = FIELD_DUEL_SLIDE_OUT;
            g_field_duel_panel_offset = -1;
        }
        break;
    case FIELD_DUEL_SLIDE_OUT:
        g_field_duel_panel_offset *= 2;
        if (g_field_duel_panel_offset < -0x64)
        {
            g_field_duel_panel_phase = FIELD_DUEL_FINISHED;
        }
        break;
    case FIELD_DUEL_FINISHED:
        return 1;
    }

    packet_cursor = func_800AEAC0(packet_cursor, draw_context, 0, g_field_duel_panel_offset + 0x32, 0x22, 1);
    packet_cursor = func_800AF950(packet_cursor, draw_context, &((FieldModalSaveView*)g_pad_ctx)->name[0], 4, g_field_duel_panel_offset + 0x6C, 0x32, 0, 5,
                                  0x180, 0x180, -4, DRAW_FLAG);

    FORMAT_SIGNED(record_text, ((FieldModalSaveView*)g_pad_ctx)->duel_wins);
    STR_CAT_ENTRY(record_text, D_800EC408, 0x44);
    FORMAT_SIGNED(loss_text, ((FieldModalSaveView*)g_pad_ctx)->duel_losses);
    STR_CAT(record_text, loss_text);
    STR_CAT_ENTRY(record_text, D_800EC40A, 0x46);

    packet_cursor = func_800AF950(packet_cursor, draw_context, record_text, 4, g_field_duel_panel_offset + 0x6C, 0x42, 0, 6, 0x180, 0x180, -4, DRAW_FLAG);
    {
        s32 low = D_800EC406.low;
        s32 offset = (D_800EC406.high << 8) + (s32)((u8*)&D_800EC406 - 0x42);
        packet_cursor = func_800AF950(packet_cursor, draw_context, (u8*)(low + offset), 4, 0xA0, 0x64, 2, 7, 0x180, 0x180, -4, DRAW_FLAG);
    }
    packet_cursor = func_800AEAC0(packet_cursor, draw_context, 1, 0xDE - g_field_duel_panel_offset, 0x86, 0);
    packet_cursor = func_800AF950(packet_cursor, draw_context, &((FieldModalSaveView*)g_pad_ctx)->second_name[0], 4, 0xD4 - g_field_duel_panel_offset, 0x96, 1,
                                  8, 0x180, 0x180, -4, DRAW_FLAG);

    if ((u32)(((FieldModalSaveView*)g_pad_ctx)->second_character.bytes[0] & 0x7F) < 2)
    {
        FORMAT_SIGNED(record_text, ((FieldModalSaveView*)g_pad_ctx)->second_duel_wins);
        STR_CAT_ENTRY(record_text, D_800EC408, 0x44);
        FORMAT_SIGNED(loss_text, ((FieldModalSaveView*)g_pad_ctx)->second_duel_losses);
        STR_CAT(record_text, loss_text);
        STR_CAT_ENTRY(record_text, D_800EC40A, 0x46);
        packet_cursor = func_800AF950(packet_cursor, draw_context, record_text, 4, 0xD4 - g_field_duel_panel_offset, 0xA6, 1, 9, 0x180, 0x180, -4, DRAW_FLAG);
    }

    context->packet_cursor = packet_cursor;
    return 0;
}

/**
 * @brief Animate and draw the winner and their duel record.
 * @param context Field drawing context and packet cursor.
 * @return Nonzero after the panel has slid out.
 */
s32 field_draw_duel_result(FieldModalDrawContext* context)
{
    u8 record_text[0x38];
    u8 loss_text[0x38];
    s32 packet_cursor;
    void* draw_context;
    FieldModalSaveView* winner;

    packet_cursor = context->packet_cursor;
    draw_context = context;
    switch (g_field_duel_panel_phase)
    {
    case FIELD_DUEL_SLIDE_IN:
        g_field_duel_panel_offset /= 2;
        if (g_field_duel_panel_offset == 0)
        {
            g_field_duel_panel_phase = FIELD_DUEL_HOLD;
            g_field_duel_panel_hold_frames = FIELD_DUEL_PANEL_HOLD_FRAMES;
        }
        break;
    case FIELD_DUEL_HOLD:
        g_field_duel_panel_hold_frames--;
        if (g_field_duel_panel_hold_frames == 0)
        {
            func_800A3938(0x127, 0x80);
            g_field_duel_panel_phase = FIELD_DUEL_SLIDE_OUT;
            g_field_duel_panel_offset = -1;
        }
        break;
    case FIELD_DUEL_SLIDE_OUT:
        g_field_duel_panel_offset *= 2;
        if (g_field_duel_panel_offset < -0x64)
        {
            g_field_duel_panel_phase = FIELD_DUEL_FINISHED;
        }
        break;
    case FIELD_DUEL_FINISHED:
        return 1;
    }

    {
        s32 low = D_800EC40C.low;
        s32 offset = (D_800EC40C.high << 8) + (s32)((u8*)&D_800EC40C - 0x48);
        packet_cursor = func_800AF950(packet_cursor, draw_context, (u8*)(low + offset), 4, 0xA0, 0x34, 2, 5, 0x200, 0x200, -4, DRAW_FLAG);
    }
    packet_cursor = func_800AEAC0(packet_cursor, draw_context, g_field_duel_winner, g_field_duel_panel_offset + 0x32, 0x54, 1);
    packet_cursor = func_800AF950(packet_cursor, draw_context, g_pad_ctx + (g_field_duel_winner * FIELD_SAVED_CHARACTER_STRIDE + 0x5F0), 4,
                                  g_field_duel_panel_offset + 0x6C, 0x64, 0, 6, 0x1C0, 0x1C0, -4, DRAW_FLAG);

    winner = (FieldModalSaveView*)(g_pad_ctx + g_field_duel_winner * FIELD_SAVED_CHARACTER_STRIDE);
    if ((u32)(winner->character_kind & 0x7F) < 2)
    {
        FieldModalSaveView* winner_record;

        FORMAT_SIGNED(record_text, winner->duel_wins);
        STR_CAT_ENTRY(record_text, D_800EC408, 0x44);
        winner_record = (FieldModalSaveView*)(g_pad_ctx + g_field_duel_winner * FIELD_SAVED_CHARACTER_STRIDE);
        FORMAT_SIGNED(loss_text, winner_record->duel_losses);
        STR_CAT(record_text, loss_text);
        STR_CAT_ENTRY(record_text, D_800EC40A, 0x46);
        packet_cursor = func_800AF950(packet_cursor, draw_context, record_text, 4, g_field_duel_panel_offset + 0x8C, 0x84, 0, 7, 0x180, 0x180, -4, DRAW_FLAG);
    }

    context->packet_cursor = packet_cursor;
    return 0;
}
