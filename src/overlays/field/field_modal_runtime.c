/** @file field_modal_runtime.c
 * @brief Immediate text, actor labels, field input and the modal overlays.
 *
 * Covers 0x800A88A0 .. 0x800AD030: the immediate text drawer and number
 * formatter, saved names and inventory helpers, the actor-label text session,
 * controller input with key repeat, and the menu, shop, GOSUB, CARDA, NIKI,
 * ADDHERO and duel modal screens.
 */

#include "field_text.h"
#include "common.h"
#include "field_actor_routes.h"
#include "field_actor_runtime.h"
#include "field_calls.h"
#include "field_menu_element.h"
#include "gpu_packet.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "field_modal_runtime.h"
#include "field_effect_render_state.h"
#include "cdrom.h"
#include "saved_game.h"
#include "vector.h"
#include "sdk/libetc.h"
#include "field_scene_transition.h"
#include "cd_resources.h"
#include "controller_internal.h"
#include "field_records.h"
#include "field_actor_tables.h"
#include "field_runtime.h"
#include "display.h"
#include "game_state.h"

/** @brief Text lead bytes 0x19 to 0x1F start a two-byte glyph. */
#define IS_DBCS_LEAD_BYTE(byte) (((byte) >= 0x19) && ((byte) <= 0x1F))
#define NAME_GLYPH_SIZE_SINGLE 1
#define NAME_GLYPH_SIZE_DOUBLE 2
/** @brief Lead byte of the double-byte digit glyphs (followed by the digit value). */
#define FIELD_TEXT_DIGIT_LEAD 0x1D

/** @brief Largest number of entries in the dialog item list. */
#define FIELD_DIALOG_ITEM_LIMIT 10

/** @brief Frames a newly pressed button waits before it repeats. */
#define FIELD_PAD_REPEAT_DELAY 15
/** @brief Frames between two repeats of a held button (it fires every third frame). */
#define FIELD_PAD_REPEAT_INTERVAL 2
#define FIELD_PAD_DIRECTIONS (PADLup | PADLdown | PADLleft | PADLright)
/** @brief Buttons whose presses count as still holding the repeating buttons. */
#define FIELD_PAD_HOLD_BUTTONS (PADh | PADi | PADselect | PADRdown | PADRright | PADL1 | PADL2 | PADR1 | PADR2)
/** @brief Button combination that resets the game to the title screen. */
#define FIELD_INPUT_RESET_COMBINATION (PADh | PADselect | PADL1 | PADL2 | PADR1 | PADR2)
/** @brief Analog stick offsets beyond this count as a direction press. */
#define FIELD_PAD_STICK_THRESHOLD 1

/** @brief Ordering table entry of the field text; the selected label goes one entry in front. */
#define FIELD_TEXT_OT_INDEX 15
/** @brief field_draw_text alignment flags and the flag that adds the glyph shadow pass. */
#define FIELD_TEXT_ALIGN_MASK 0x7F
#define FIELD_TEXT_ALIGN_RIGHT 1
#define FIELD_TEXT_ALIGN_CENTER 2
#define FIELD_TEXT_SHADOW 0x80
/** @brief field_draw_text text colours. */
#define FIELD_TEXT_COLOR_NORMAL 4
#define FIELD_TEXT_COLOR_DIM 5

/** @brief Scale given to the part of the selected label's actor (0x40 is full size). */
#define FIELD_LABEL_SELECTED_SCALE 0x80
#define FIELD_LABEL_MIN_Y 50
#define FIELD_LABEL_MAX_Y 176
/** @brief Half width of one label glyph in pixels. */
#define FIELD_LABEL_GLYPH_HALF_WIDTH 6
/** @brief Glyphs of the brackets drawn around the selected label (func_800AD524). */
#define FIELD_LABEL_LEFT_BRACKET 12
#define FIELD_LABEL_RIGHT_BRACKET 13
/** @brief Smallest gap between a label and the left screen edge. */
#define FIELD_LABEL_MARGIN 8
/** @brief func_800AD524 / func_800AD208 flags of the label glyphs: outline plus palette 1 or 2. */
#define FIELD_LABEL_SELECTED_DIGITS (FIELD_TEXT_SHADOW | 1)
#define FIELD_LABEL_NORMAL_DIGITS (FIELD_TEXT_SHADOW | 2)
/** @brief Screen row of the field origin (actors are drawn relative to it). */
#define FIELD_SCREEN_CENTER_Y 112
/** @brief Held-button hints: eight hint buttons, portrait and text position (32 pixels per player). */
#define FIELD_HINT_BUTTON_COUNT 8
#define FIELD_HINT_PORTRAIT_X 96
#define FIELD_HINT_PORTRAIT_Y 60
#define FIELD_HINT_TEXT_X 128
#define FIELD_HINT_TEXT_Y 64
/** @brief Byte offset of a member (the classic offsetof). */
#define FIELD_OFFSET_OF(type, member) ((s32) & ((type*)0)->member)

#define FIELD_SOUND_ACTION_REFUSED 0x78
#define FIELD_SOUND_CURSOR 0x7D
#define FIELD_SOUND_SELECT 0x7E
#define FIELD_SOUND_MENU_OPEN 0x80
#define FIELD_SOUND_LOW_HP 0xA6
#define FIELD_SOUND_EMPTY_SHOP 0xC7
#define FIELD_SOUND_DUEL_INTRO 0x125
#define FIELD_SOUND_DUEL_RESULT 0x126
#define FIELD_SOUND_DUEL_PANEL_OUT 0x127
#define FIELD_SOUND_PAN_CENTRE 0x80

/** @brief FieldPlayerRecord weapon type before the first party update. */
#define FIELD_WEAPON_TYPE_UNSET 0xFF
/** @brief FieldPlayerRecord::portrait_index when no portrait is cached. */
#define FIELD_PORTRAIT_NONE 0xFF
/** @brief Party record of the companion. */
#define FIELD_COMPANION_INDEX 2
/** @brief Golem companions use character ids from this value on. */
#define FIELD_COMPANION_GOLEM_ID_BASE 0x41
/** @brief Resource id base of the companion action packages (field_load_party_script_page). */
#define FIELD_RES_COMPANION_ACTIONS 0xA9B
/** @brief Action slots of a party member's row: two commands, then four skills from slot 4. */
#define FIELD_COMMAND_SLOT_COUNT 2
/** @brief Button action of the first command: button actions 2 and 3 run resource action slots 0 and 1. */
#define FIELD_COMMAND_BUTTON_ACTION 2
#define FIELD_SKILL_SLOT_COUNT 4
#define FIELD_SKILL_ACTION_BASE 4
/** @brief Saved skill values: none, or an instrument (plus its item record index). */
#define FIELD_SKILL_NONE 0xFF
#define FIELD_SKILL_INSTRUMENT 0x80
/** @brief FieldActionFlags::target_filter of an action without a target predicate. */
#define FIELD_ACTION_TARGET_NONE 0xFF
/** @brief Technique actions: command flag and sequence ids (FIELD_TECHNIQUE_SEQUENCE_BASE + weapon type * 24 + technique). */
#define FIELD_ACTION_TECHNIQUE 0x8000
#define FIELD_ACTION_TECHNIQUE_MASK 0x7FFF
#define FIELD_TECHNIQUES_PER_WEAPON 24
#define FIELD_TECHNIQUE_SEQUENCE_BASE 0x88
/** @brief Action animation of a technique. */
#define FIELD_TECHNIQUE_ANIMATION 2

/** @brief Load address of the sub-overlays (MENU, GOLEM, GNAME, ZUKAN, GOSUB, SHOP). */
#define FIELD_SUBOVERLAY_ADDRESS ((void*)0x80140000)
/** @brief Work buffers handed to the sub-overlays. */
#define FIELD_MENU_RENDER_BUFFERS ((void*)0x80170000)
#define FIELD_GOLEM_WORK_BUFFER 0x80150000
#define FIELD_SHOP_WORK_BUFFER ((void*)0x80150000)
#define FIELD_GNAME_WORK_BUFFER ((void*)0x80160000)
#define FIELD_GOSUB_WORK_BUFFER ((void*)0x80175000)

/** @brief Screens the MENU overlay returns: 0 closes the menu, others open GNAME with that mode. */
#define FIELD_MENU_CLOSED 0
#define FIELD_MENU_GOLEM 10
#define FIELD_MENU_NAME_ENTRY_B 11
#define FIELD_MENU_NAME_ENTRY_C 12

/** @brief g_field_gosub_phase: idle, closing (the screens finished), running. */
#define FIELD_GOSUB_IDLE 0
#define FIELD_GOSUB_CLOSING 1
#define FIELD_GOSUB_RUNNING 2

/** @brief func_800AF950 scales (1/256 units) and lower-edge slant of the duel panel text. */
#define FIELD_DUEL_TEXT_SCALE 0x180
#define FIELD_DUEL_TITLE_SCALE 0x200
#define FIELD_DUEL_WINNER_SCALE 0x1C0
#define FIELD_DUEL_TEXT_SLANT (-4)
#define FIELD_DUEL_PANEL_HOLD_FRAMES 90
#define FIELD_DUEL_PANEL_START_OFFSET 500
/** @brief The duel panel is gone once it has slid this far past the screen edge. */
#define FIELD_DUEL_PANEL_END_OFFSET (-100)
/** @brief Animation of the losing hero; decides the duel winner. */
#define FIELD_ANIMATION_DUEL_LOST 0x1D

/** @brief The duel panels' text is drawn only while they slide in or out. */
#define FIELD_DUEL_PANEL_MOVING (g_field_duel_panel_phase == FIELD_DUEL_SLIDE_IN || g_field_duel_panel_phase == FIELD_DUEL_SLIDE_OUT)

/** @brief Animation stages shared by the duel introduction and winner panels. */
typedef enum FieldDuelPanelPhase
{
    FIELD_DUEL_SLIDE_IN,
    FIELD_DUEL_HOLD,
    FIELD_DUEL_SLIDE_OUT,
    FIELD_DUEL_FINISHED
} FieldDuelPanelPhase;

/**
 * @brief Entry of the dialog text bank at D_800EC3C4: a little-endian byte
 *        offset from the start of the bank to one string.
 */
typedef struct
{
    u8 low;
    u8 high;
} FieldTextOffset;

/* Dialog text bank entries (FieldTextOffset). */
extern FieldTextOffset D_800EC3D2;
extern FieldTextOffset D_800EC3D4;
extern FieldTextOffset D_800EC3E4;
extern FieldTextOffset D_800EC400;
extern FieldTextOffset D_800EC406;
extern FieldTextOffset D_800EC408;
extern FieldTextOffset D_800EC40A;
extern FieldTextOffset D_800EC40C;
extern FieldTextOffset D_800EC3E0;
extern FieldTextOffset D_800EC3E6;
extern FieldTextOffset D_800EC3E8;
/* Dialog text bank (starts with FieldTextOffset entries). */
extern u8 D_800EC3C4[];
/* Offset tables of the ability and technique names (u16 offsets from the table start). */
extern u8 g_field_command_names[];
extern u8 g_field_technique_names[];
extern FieldActionRow g_field_resource_actions[];
/* Action slot bound to each hint button (index into FieldCharacterRecord::button_actions). */
extern u8 g_field_hint_button_map[];
/* Icon and texture parameters of the action animations, two bytes per action. */
extern u8 g_field_action_animation_parameters[];
/* Action icons of the instrument types. */
extern u8 g_field_instrument_icons[];

/*
 * Saved game (g_saved_game) as FIELD reads it; main.h declares the same
 * pointer as PadContext, so this file does not include main.h and declares
 * the main executable globals it uses itself.
 */
extern FieldGameState* g_pad_ctx;
extern s32 g_pad_input;
extern s32 g_pad_input_inject;
extern s32 g_save_slot_index;
extern u16 g_music_track_index;
extern s32 g_frame_counter;
extern s32 g_pending_game_state;
extern s32 g_active_script;
extern s32 g_script_repeat_count;

/* Dialog item list shown by the result screens. */
extern s32 g_field_dialog_item_texts[];
extern s32 g_field_dialog_item_count;
extern u8 g_field_dialog_item_quantities[];

/* Actor-label text session. */
extern u8 g_field_selected_actor_label;
extern u8 g_field_label_actor_indices[];
extern u8 g_field_label_actor_count;
/* Saved FieldObjectPart scale_z and scale_x of each labelled actor. */
extern u8 g_field_label_saved_scale_x[];
extern u8 g_field_label_saved_scale_y[];
extern s32 g_field_text_session_active;
extern s32 g_field_text_session_cd_error;
extern s32 g_field_draw_count;

/* Input repeat state of both controllers. */
extern s32 g_field_primary_held_buttons;
extern s32 g_field_primary_repeat_delay;
extern s32 g_field_secondary_held_buttons;
extern s32 g_field_secondary_repeat_delay;
extern s32 g_field_buffered_input;
extern u8 g_field_menu_controller_types[CONTROLLER_PORT_COUNT];

/* Field state that blocks the menu. */
extern s32 g_field_active_group;
extern s32 g_field_interaction_active;
extern s32 D_80122710;
extern s32 D_80122714;
extern s32 D_800F2298;
extern s32 g_field_dialog_screen_mode;
extern s32 g_field_return_to_title_prompt_state;
extern s32 D_8012291C;
extern s32 D_80122980;
extern s32 g_field_scene_mode_bit;

/* Modal overlays. */
extern s32 g_field_modal_state;
extern s32 g_field_gosub_phase;
extern s32 D_801227F0;
extern s32 g_field_shop_active;
extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values;
extern s32 g_field_niki_addhero_state;
extern s32 g_field_card_overlay_mode;
extern s32 g_field_shop_notice_hidden;
extern s32 g_field_duel_panel_phase;
extern s32 g_field_duel_panel_offset;
extern s32 g_field_duel_panel_hold_frames;
extern s32 g_field_duel_winner;

/* Item rename request left by the MENU overlay for GNAME: the item record, its old name, extra text, source selector and item category. */
extern u8 g_field_rename_custom_name[];
extern u8 g_field_rename_initial_name[];
extern s32 g_field_rename_item_category;
extern u8* g_field_rename_target;
extern s32 g_field_rename_source;

void akao_stop_sfx_by_id(s32 id);
void akao_cmd_99_9b_9d_9f(s32 arg0);
void akao_cmd_98_9a_9c_9e(s32 arg0);
void akao_set_paused(s32 mode);

/* Sub-overlay entry points, valid once their overlay is loaded at FIELD_SUBOVERLAY_ADDRESS. */
/* GNAME and SHOP share this entry address but have different parameter lists. */
void func_80140004();
void func_80140024(u32 work, s32 mode);
s32 func_801400C4(FieldRenderHalf* render);
s32 func_801400D4(FieldRenderHalf* render);
s32 func_801401F0(FieldRenderHalf* render);
s32 func_801401F8(FieldRenderHalf* render);
s32 func_80140370(FieldRenderHalf* render);
s32 func_801405B0(s32 render_buffers);
void func_80140080(void* work, void* screen_sequence);
void func_80140E00(void* work, s32 context);

s32 field_party_reload_reading(void);
void* field_emit_actor_portrait(SPRT* cursor, u32* ot, s32 index, u32* position);
void* func_800AD208(s32* ot, void* cursor, s32 value, s32 digits, u16* position, s32 flags);
void* func_800AD524(u8* cursor, s32* ot, s32 glyph, s32* position, s32 flags);
SPRT* func_800AD658(s32* ot, SPRT* sprite_cursor, s32 count);
s32 field_draw_player_icon(s32 packet_cursor, u_long* ot, s32 player, s32 x, s32 y, s32 flip);
s32 func_800AF950(s32 packet_cursor, u_long* ot, u8* text, s32 color, s32 x, s32 y, s32 align, s32 slot, s32 scale_x, s32 scale_y, s32 arg10, s32 visible);

void* field_draw_text(SPRT* sprite_cursor, s32* ot, u8* text, s32 text_color, s32 x, s32 y, s32 flags);
void field_format_number(u8* text, s32 number, s32 wide_request);
s32 field_name_byte_length(u8* name);
void field_copy_name(u8* destination, u8* source);
s32 field_read_controller_buttons(s32 index);
void field_reset_input_repeat(void);
static void field_init_actor_labels(void);
static void field_draw_cd_error_text(FieldRenderHalf* render);
static void field_draw_actor_labels(FieldRenderHalf* render);
static s32 field_play_low_hp_warning(void);
static void field_update_text_session(void);
static void field_run_menu(void* render_buffers, s32 controller);
static void field_begin_text_session(void);
static void field_update_modal_text_session(FieldRenderHalf* render);
static void field_begin_empty_shop_notice(s32 hidden);
static void field_draw_empty_shop_notice(FieldRenderHalf* render);
static s32 field_draw_duel_intro(FieldRenderHalf* render);
static s32 field_draw_duel_result(FieldRenderHalf* render);

/**
 * @brief Address of the string a dialog text bank entry points at.
 * @param entry Bank entry.
 * @param index Position of @p entry in the bank; its offset counts from the bank start.
 * @return The entry's string.
 */
static inline u8* field_dialog_text(FieldTextOffset* entry, s32 index)
{
    return (u8*)(entry->low + ((entry->high << 8) + (s32)((u8*)entry - index * sizeof(FieldTextOffset))));
}

/**
 * @brief Build the glyph sprites of a text line and add them to the ordering table.
 * @param sprite_cursor First free sprite; the glyph sprites are built here.
 * @param ot Ordering table entry receiving the sprites.
 * @param text Encoded text; an empty string draws nothing.
 * @param text_color Text colour passed to the glyph builder.
 * @param x Left edge, or the right edge / centre for FIELD_TEXT_ALIGN_RIGHT / FIELD_TEXT_ALIGN_CENTER.
 * @param y Top edge.
 * @param flags Alignment in FIELD_TEXT_ALIGN_MASK, plus FIELD_TEXT_SHADOW for the black outline copies.
 * @return First free primitive after the text and its closing DR_TPAGE.
 */
void* field_draw_text(SPRT* sprite_cursor, s32* ot, u8* text, s32 text_color, s32 x, s32 y, s32 flags)
{
    s32 glyph_count;
    s32 remaining;
    s32 i;
    s32 advance;
    SPRT* sprite;
    DR_TPAGE* tpage;

    if (*text == 0)
    {
        return sprite_cursor;
    }

    glyph_count = field_text_build_sprites(sprite_cursor, text, text_color);
    remaining = glyph_count;

    if ((flags & FIELD_TEXT_ALIGN_MASK) != FIELD_TEXT_ALIGN_RIGHT)
    {
        if ((flags & FIELD_TEXT_ALIGN_MASK) == FIELD_TEXT_ALIGN_CENTER)
        {
            sprite = sprite_cursor;
            for (i = 0; i < remaining; i++)
            {
                x -= sprite[i].w >> 1;
            }
        }
    }
    else
    {
        sprite = sprite_cursor;
        for (i = 0; i < remaining; i++)
        {
            x -= sprite[i].w;
        }
    }

    advance = 0;

    if (remaining != 0)
    {
        do
        {
            sprite = sprite_cursor;
            SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
            setSprt(sprite);
            setXY0(sprite, x + advance, y);
            advance += sprite->w;

            addPrim(ot, sprite);
            sprite_cursor++;
            remaining--;
        } while (remaining != 0);
    }

    if (flags & FIELD_TEXT_SHADOW)
    {
        sprite_cursor = func_800AD658(ot, sprite_cursor, glyph_count);
    }

    tpage = (DR_TPAGE*)sprite_cursor;
    setDrawTPage(tpage, 0, 0, 0x1F);
    addPrim(ot, tpage);

    return tpage + 1;
}

/**
 * @brief Draw a signed decimal number as text.
 * @param ot Ordering table entry receiving the sprites.
 * @param sprite_cursor First free sprite.
 * @param value Number to draw.
 * @param text_color Text colour.
 * @param position X and y of the text.
 * @param flags field_draw_text alignment and shadow flags.
 * @return First free primitive after the text.
 */
void* field_draw_number(s32* ot, SPRT* sprite_cursor, s32 value, s32 text_color, s16* position, s32 flags)
{
    u8 text[64];

    field_format_number(text, value, 0);
    return field_draw_text(sprite_cursor, ot, text, text_color, position[0], position[1], flags);
}

/**
 * @brief Draw a signed decimal number as text, asking the formatter for double-byte digits.
 * @param ot Ordering table entry receiving the sprites.
 * @param sprite_cursor First free sprite.
 * @param value Number to draw.
 * @param text_color Text colour.
 * @param position X and y of the text.
 * @param flags field_draw_text alignment and shadow flags.
 * @return First free primitive after the text.
 * @note field_format_number ignores the request, so the digits come out as in field_draw_number.
 */
void* field_draw_number_wide(s32* ot, SPRT* sprite_cursor, s32 value, s32 text_color, s16* position, s32 flags)
{
    u8 text[64];

    field_format_number(text, value, 1);
    return field_draw_text(sprite_cursor, ot, text, text_color, position[0], position[1], flags);
}

/**
 * @brief Format a signed decimal number (up to eight digits) as encoded text.
 * @param text Buffer receiving the text and its terminator.
 * @param number Number to format; a negative number starts with the bank's minus sign.
 * @param wide_request Double-byte digits requested by field_draw_number_wide; ignored.
 * @note The double-byte branch tests a local that is always zero, so the digits are always single-byte.
 */
inline void field_format_number(u8* text, s32 number, s32 wide_request)
{
    u8* cursor;
    s32 value;
    s32 double_byte;
    u8* minus;
    s32 divisor;
    s32 started;
    s32 digit;

    cursor = text;
    value = number;
    double_byte = 0;
    if (value < 0)
    {
        value = -value;
        minus = field_dialog_text(&D_800EC3E4, 16);
        field_copy_name(cursor, minus);
        cursor += field_name_byte_length(minus);
    }
    divisor = 10000000;
    started = 0;
    do
    {
        digit = value / divisor;
        if (digit != 0)
        {
            started = 1;
        }
        if (started || divisor == 1)
        {
            if (double_byte)
            {
                *cursor++ = FIELD_TEXT_DIGIT_LEAD;
                *cursor = digit;
            }
            else
            {
                *cursor = digit + '0';
            }
            cursor++;
            value -= (value / divisor) * divisor;
        }
        divisor /= 10;
    } while (divisor != 0);
    *cursor = 0;
}

/**
 * @brief Bind the field input and inventory context to the loaded saved game.
 */
void field_bind_saved_game_context(void)
{
    g_pad_ctx = (FieldGameState*)g_saved_game.bytes;
}

/**
 * @brief Save the field entry settings, music track, and current save slot.
 * @param scene_mode Scene entry mode.
 * @param field_flags Field entry flags.
 * @param layout_flags Field layout flags.
 * @param entry_config Packed entry configuration; only the low 25 bits are replaced.
 * @param option_id Entry option identifier.
 * @param sub_mode Entry submode.
 */
void field_store_entry_settings(s16 scene_mode, s8 field_flags, s8 layout_flags, s32 entry_config, s32 option_id, s32 sub_mode)
{
    g_pad_ctx->scene_mode = scene_mode;
    g_pad_ctx->field_flags = field_flags;
    g_pad_ctx->layout_flags = layout_flags;
    g_pad_ctx->entry_config = entry_config;
    g_pad_ctx->option_id = option_id;
    g_pad_ctx->sub_mode = sub_mode;
    g_pad_ctx->music_track = g_music_track_index;
    g_pad_ctx->save_slot = g_save_slot_index;
}

/**
 * @brief Append text and its quantity to the field dialog item list.
 * @param text Address of the encoded item text.
 * @param quantity Quantity displayed alongside the text.
 */
void field_append_dialog_item(s32 text, u8 quantity)
{
    s32 index = g_field_dialog_item_count;

    if (index < FIELD_DIALOG_ITEM_LIMIT)
    {
        g_field_dialog_item_texts[index] = text;
        g_field_dialog_item_quantities[index] = quantity;
        g_field_dialog_item_count++;
    }
}

/**
 * @brief Count encoded name bytes, excluding the terminator.
 * @param name NUL-terminated name; lead bytes 0x19 through 0x1F begin two-byte glyphs.
 * @return Number of bytes before the terminator.
 */
s32 field_name_byte_length(u8* name)
{
    s32 count;
    u8 character;

    count = 0;
    character = *name;
    if (character != 0)
    {
        do
        {
            if (IS_DBCS_LEAD_BYTE(character))
            {
                name += NAME_GLYPH_SIZE_DOUBLE;
                count += NAME_GLYPH_SIZE_DOUBLE;
            }
            else
            {
                name += NAME_GLYPH_SIZE_SINGLE;
                count += NAME_GLYPH_SIZE_SINGLE;
            }
            character = *name;
        } while (character != 0);
    }
    return count;
}

/**
 * @brief Copy an encoded name, including its terminator.
 * @param destination Buffer receiving the name.
 * @param source NUL-terminated encoded name.
 */
void field_copy_name(u8* destination, u8* source)
{
    u8* cursor;
    s32 byte_count;
    s32 byte_index;

    cursor = source;
    byte_count = 0;
    while (*cursor != 0)
    {
        if (IS_DBCS_LEAD_BYTE(*cursor))
        {
            cursor += NAME_GLYPH_SIZE_DOUBLE;
            byte_count += NAME_GLYPH_SIZE_DOUBLE;
        }
        else
        {
            cursor += NAME_GLYPH_SIZE_SINGLE;
            byte_count += NAME_GLYPH_SIZE_SINGLE;
        }
    }
    for (byte_index = 0; byte_index < byte_count; byte_index++)
    {
        destination[byte_index] = source[byte_index];
    }
    destination[byte_index] = 0;
}

/**
 * @brief Append a NUL-terminated name onto another, honouring DBCS glyph widths.
 * @param destination Existing name; the source is appended after its last glyph.
 * @param source Name to append.
 */
inline void field_append_name(u8* destination, const u8* source)
{
    const u8* destination_cursor;
    s32 destination_byte_count;
    const u8* source_cursor;
    s32 source_byte_count;
    s32 append_offset;
    s32 byte_index;

    destination_cursor = destination;
    destination_byte_count = 0;
    while (*destination_cursor)
    {
        if (IS_DBCS_LEAD_BYTE(*destination_cursor))
        {
            destination_cursor += NAME_GLYPH_SIZE_DOUBLE;
            destination_byte_count += NAME_GLYPH_SIZE_DOUBLE;
        }
        else
        {
            destination_cursor += NAME_GLYPH_SIZE_SINGLE;
            destination_byte_count += NAME_GLYPH_SIZE_SINGLE;
        }
    }

    source_cursor = source;
    source_byte_count = 0;
    append_offset = destination_byte_count;
    while (*source_cursor)
    {
        if (IS_DBCS_LEAD_BYTE(*source_cursor))
        {
            source_cursor += NAME_GLYPH_SIZE_DOUBLE;
            source_byte_count += NAME_GLYPH_SIZE_DOUBLE;
        }
        else
        {
            source_cursor += NAME_GLYPH_SIZE_SINGLE;
            source_byte_count += NAME_GLYPH_SIZE_SINGLE;
        }
    }

    for (byte_index = 0; byte_index < source_byte_count; byte_index++)
    {
        destination[byte_index + append_offset] = source[byte_index];
    }
    destination[byte_index + append_offset] = 0;
}

/**
 * @brief Append a dialog text bank string onto a name.
 * @param destination Name to extend.
 * @param entry Bank entry of the string.
 * @param index Position of @p entry in the bank.
 * @note Adds the offset before the bank start, unlike field_dialog_text; the other order changes the generated code.
 */
static inline void field_append_dialog_text(u8* destination, FieldTextOffset* entry, s32 index)
{
    u8* text;

    text = (u8*)((entry->high << 8) + entry->low);
    text += (s32)((u8*)entry - index * sizeof(FieldTextOffset));
    field_append_name(destination, text);
}

/**
 * @brief Copy one saved inventory record.
 * @param destination Buffer receiving the record.
 * @param source Record to copy.
 */
void field_copy_inventory_record(u8* destination, u8* source)
{
    u32 byte_index;

    byte_index = 0;
    do
    {
        byte_index++;
        *destination++ = *source++;
    } while (byte_index < sizeof(FieldItemRecord));
}

/**
 * @brief Close gaps in the saved inventory and mark the remaining slots free.
 * @note Record data beyond the occupancy byte is retained in free slots.
 */
void field_compact_inventory(void)
{
    s32 record_index;
    FieldItemRecord* write_record;
    FieldItemRecord* read_record;

    record_index = 0;
    write_record = g_pad_ctx->items;
    read_record = write_record;
    do
    {
        if (read_record->kind != 0)
        {
            if (read_record != write_record)
            {
                bcopy(read_record, write_record, sizeof(*read_record));
                read_record->kind = 0;
            }
            write_record++;
        }
        record_index += 1;
        read_record++;
    } while (record_index < FIELD_ITEM_COUNT);
    while (write_record < &g_pad_ctx->items[FIELD_ITEM_COUNT])
    {
        write_record->kind = 0;
        write_record++;
    }
}

/**
 * @brief Find a free slot in the saved inventory.
 * @return The first free record, or NULL when the inventory is full.
 */
FieldItemRecord* field_find_free_inventory_record(void)
{
    s32 record_index;
    FieldItemRecord* record;

    record = g_pad_ctx->items;
    for (record_index = 0; record_index < FIELD_ITEM_COUNT; record_index++)
    {
        if (record->kind == 0)
        {
            return record;
        }
        record++;
    }
    return NULL;
}

/**
 * @brief Draw the CD error text or the actor labels, then upload the text cache.
 * @param render Render half receiving the text primitives.
 */
inline void field_draw_text_session(FieldRenderHalf* render)
{
    field_text_reset_scratch();

    if (g_field_text_session_cd_error)
    {
        field_draw_cd_error_text(render);
    }
    else
    {
        field_draw_actor_labels(render);
    }

    field_text_upload_immediate_cache();
}

/**
 * @brief Restore the part scales saved before displaying actor labels.
 */
inline void field_restore_label_actor_parts(void)
{
    s32 index;
    FieldObjectPart* part;

    akao_stop_sfx_by_id(FIELD_SOUND_SELECT);
    for (index = 0; index < g_field_label_actor_count; index++)
    {
        part = &g_field_object_parts[g_field_label_actor_indices[index]];
        part->scale_z = g_field_label_saved_scale_x[index];
        part->scale_x = g_field_label_saved_scale_y[index];
    }
}

/**
 * @brief Collect the living named actors of the active group as labels and save their part scales.
 */
static void field_init_actor_labels(void)
{
    FieldActor* actor;
    FieldObjectState* state;
    s32 actor_index;

    field_reset_input_repeat();
    akao_cmd_99_9b_9d_9f(2);
    field_fade_song(0, 60, 0);
    akao_stop_sfx_by_id(FIELD_SOUND_SELECT);
    actor = &g_field_actors[FIELD_PARTY_COUNT];
    actor_index = FIELD_PARTY_COUNT;
    state = &g_field_object_states[FIELD_PARTY_COUNT];
    g_field_label_actor_count = 0;
    g_field_selected_actor_label = 0;
    do
    {
        if (actor->presence != FIELD_ACTOR_UNUSED && state->unk4.word != 0 && g_field_active_group == (state->group_flags & FIELD_OBJECT_GROUP_MASK) &&
            state->name != NULL)
        {
            g_field_label_actor_indices[g_field_label_actor_count] = actor_index;
            g_field_label_saved_scale_x[g_field_label_actor_count] = g_field_object_parts[actor_index].scale_z;
            g_field_label_saved_scale_y[g_field_label_actor_count] = g_field_object_parts[actor_index].scale_x;
            g_field_label_actor_count += 1;
        }
        actor_index += 1;
        state++;
        actor++;
    } while (actor_index < FIELD_ACTOR_COUNT);
}

/**
 * @brief Draw the status text of the current CD-ROM error.
 * @param render Render half receiving the text.
 */
static void field_draw_cd_error_text(FieldRenderHalf* render)
{
    SPRT* primitive;
    u_long* ordering_table;
    Vec2s unused[2]; /* Never used; the original frame has room for it. */

    primitive = (SPRT*)render->primitive_cursor;
    ordering_table = &render->ordering_table[FIELD_TEXT_OT_INDEX];
    /* 2 is CdErrorStatus CD_ERROR_STATUS_DISC_CHECK_PENDING (private to cdrom.c). */
    if (cdrom_get_error_status() == 2)
    {
        primitive = field_draw_text(primitive, (s32*)ordering_table, field_dialog_text(&D_800EC3D2, 7), FIELD_TEXT_COLOR_NORMAL, SCREEN_WIDTH / 2, 100,
                                    FIELD_TEXT_SHADOW | FIELD_TEXT_ALIGN_CENTER);
    }
    else
    {
        primitive = field_draw_text(primitive, (s32*)ordering_table, field_dialog_text(&D_800EC3D4, 8), FIELD_TEXT_COLOR_NORMAL, SCREEN_WIDTH / 2, 100,
                                    FIELD_TEXT_SHADOW | FIELD_TEXT_ALIGN_CENTER);
    }
    render->primitive_cursor = (u8*)primitive;
}

/**
 * @brief Draw each player's held-button action hint and the labels of the selectable actors.
 * @param render Render half receiving the text.
 * @note A player's hint shows the action bound to the first held button among the eight hint buttons.
 */
static void field_draw_actor_labels(FieldRenderHalf* render)
{
    DVECTOR point;
    FieldTextOffset* bank_entry;
    s32 text_color;
    u16 text_offset;
    s32 work;          /* Held buttons, then the player's save base, then a text index or skill. */
    s32 text_or_state; /* Hint text in the first loop, the label actor's object state in the second. */
    s32 bank;
    s32 record_offset;
    s32 screen_y;
    s32 screen_x;
    s32 ot;
    s32 label_half_width;
    s32 number_ot;
    s32 camera_x;
    s32 camera_y;
    s32 text_ot;
    s32 left_glyph_ot;
    s32 right_glyph_ot;
    s32 button_or_x; /* Hint button index in the first loop; camera x in pixels, then the label x, in the second. */
    s32 index;
    s32 cursor;
    s32 action_offset;
    s32 label_y;
    u16 raw_buttons;
    s32 bit_or_actor; /* Hint button bit in the first loop, label actor index in the second. */
    s32 action;
    s32 secondary_action;
    s32 secondary_action_alt;
    s32 text_value;
    FieldObjectPart* highlight_part;
    FieldActor* actor;
    FieldObjectPart* normal_part;
    ControllerPortState* port;
    s32 text_part;
    ControllerPortState* ports;
    s32 record_base_offset;

    cursor = (s32)render->primitive_cursor;
    ot = (s32)&render->ordering_table[FIELD_TEXT_OT_INDEX];
    index = 0;
    ports = CONTROLLER_STATE->ports;
    do
    {
        action_offset = index * sizeof(FieldActionRow);
        if ((g_field_player_records[index].head.bytes.flags & FIELD_PLAYER_ACTIVE) &&
            ((port = &ports[index])->published_sample.device_type < CONTROLLER_DEVICE_CONFIGURING))
        {
            bit_or_actor = 1;
            button_or_x = 0;
            bank = (s32)D_800EC3C4;
            record_offset = index * sizeof(FieldCharacterRecord);
            raw_buttons = port->published_sample.held_buttons;
            record_base_offset = index * sizeof(FieldCharacterRecord) + FIELD_OFFSET_OF(FieldGameState, characters);
            work = ((raw_buttons << 8) & 0xFF00) | (raw_buttons >> 8);
            work = (((u32)(work & 0x40) >> 1) | ((work & 0x20) * 2) | ((u32)(work & 0x80) >> 3) | ((work & 0x10) * 8) | (work & 0xFF0F));
            do
            {
                if (work & bit_or_actor)
                {
                    /* characters[0] of this shifted base is characters[index]; indexing directly changes the loop hoisting. */
                    work = (s32)g_pad_ctx + record_offset;
                    action = ((FieldGameState*)work)->characters[0].button_actions[g_field_hint_button_map[button_or_x]];
                    switch (action)
                    {
                    case FIELD_COMMAND_BUTTON_ACTION: /* The two commands: resource action slots 0 and 1. */
                    case FIELD_COMMAND_BUTTON_ACTION + 1:
                        text_part = (s32)&g_field_resource_actions->slots[-FIELD_COMMAND_BUTTON_ACTION] + action_offset;
                        action *= sizeof(FieldActionSlot);
                        work = ((FieldActionSlot*)(text_part + action))->command & FIELD_ACTION_TECHNIQUE_MASK;
                        text_value = (s32)g_field_command_names;
                        text_part = ((u16*)text_value)[work];
                        text_or_state = text_part + text_value;
                        break;

                    case 0: /* Guests 5 and 8 have no attack or guard. */
                        if (((((FieldGameState*)work)->characters[0].info.bytes[0] & FIELD_CHARACTER_TYPE_MASK) == FIELD_CHARACTER_GUEST) &&
                            ((secondary_action = ((FieldGameState*)work)->characters[0].info.bytes[1], (secondary_action == 5)) || (secondary_action == 8)))
                        {
                            text_offset = D_800EC3E0.high << 8;
                            text_part = text_offset + bank;
                            text_value = D_800EC3E0.low;
                            text_or_state = text_part + text_value;
                        }
                        else
                        {
                            bank_entry = &D_800EC3E6;
                            text_part = bank_entry->high;
                            text_value = D_800EC3E6.low;
                            text_part <<= 8;
                            text_part += bank;
                            text_or_state = text_part + text_value;
                        }

                        break;
                    case 1:
                        if (((((FieldGameState*)work)->characters[0].info.bytes[0] & FIELD_CHARACTER_TYPE_MASK) == FIELD_CHARACTER_GUEST) &&
                            ((secondary_action_alt = ((FieldGameState*)work)->characters[0].info.bytes[1], (secondary_action_alt == 5)) ||
                             (secondary_action_alt == 8)))
                        {
                            text_offset = D_800EC3E0.high << 8;
                            text_part = text_offset + bank;
                            text_value = D_800EC3E0.low;
                            text_or_state = text_part + text_value;
                        }
                        else
                        {
                            text_part = D_800EC3E8.high;
                            text_part <<= 8;
                            text_part += bank;
                            text_value = D_800EC3E8.low;
                            text_or_state = text_part + text_value;
                        }
                        break;
                    default:
                        /* Dead store (overwritten below); without it the technique bank's high half is not hoisted as in the original. */
                        text_value = (s32)g_field_technique_names;
                        work = ((FieldGameState*)((u8*)g_pad_ctx + record_offset))->characters[0].info.bytes[action];
                        if (work == FIELD_SKILL_NONE)
                        {
                            text_offset = D_800EC3E0.high << 8;
                            text_part = text_offset + bank;
                            text_value = D_800EC3E0.low;
                            text_or_state = text_part + text_value;
                        }
                        else
                        {
                            if (work & FIELD_SKILL_INSTRUMENT)
                            {
                                text_offset = work & ~FIELD_SKILL_INSTRUMENT;
                                text_or_state = (s32) & ((FieldCharacterRecord*)((u8*)g_pad_ctx + record_base_offset))->unk150[text_offset];
                            }
                            else
                            {
                                action = (s32)&g_field_resource_actions->slots[action];
                                work = ((FieldActionSlot*)(index * sizeof(FieldActionRow) + action))->command;
                                work &= FIELD_ACTION_TECHNIQUE_MASK;
                                text_value = g_field_player_records[index].head.bytes.weapon_type;
                                work += text_value * FIELD_TECHNIQUES_PER_WEAPON;

                                text_value = (s32)g_field_technique_names;
                                text_part = ((u16*)text_value)[work];
                                text_or_state = text_part + text_value;
                            }
                        }

                        break;
                    }
                    label_y = index;
                    point.vx = FIELD_HINT_PORTRAIT_X;
                    label_y <<= 5;
                    point.vy = label_y + FIELD_HINT_PORTRAIT_Y;
                    cursor = (s32)field_emit_actor_portrait((SPRT*)cursor, (u32*)ot, index, (u32*)&point);
                    cursor = (s32)field_draw_text((SPRT*)cursor, (s32*)ot, (u8*)text_or_state, FIELD_TEXT_COLOR_NORMAL, FIELD_HINT_TEXT_X,
                                                  label_y + FIELD_HINT_TEXT_Y, FIELD_TEXT_SHADOW);
                    break;
                }
                else
                {
                    button_or_x += 1;
                    bit_or_actor *= 2;
                }
            } while (button_or_x < FIELD_HINT_BUTTON_COUNT);
        }
        index += 1;
    } while (index < FIELD_PLAYER_COUNT);
    index = 0;
    if (g_field_label_actor_count != 0)
    {
        do
        {
            bit_or_actor = g_field_label_actor_indices[index];
            camera_x = g_field_view_offset_x;
            text_or_state = (s32)&g_field_object_states[bit_or_actor];
            actor = &g_field_actors[bit_or_actor];
            button_or_x = camera_x / 256;
            screen_x = actor->x / 256 + SCREEN_WIDTH / 2;
            point.vx = button_or_x + screen_x;
            camera_y = g_field_view_offset_y / 256;
            screen_y = actor->y / 256 + FIELD_SCREEN_CENTER_Y;
            point.vy = ((camera_y + screen_y) - actor->z / 512) - g_field_view_offset_z / 512;
            label_half_width = field_count_text_glyphs(((FieldObjectState*)text_or_state)->name) * FIELD_LABEL_GLYPH_HALF_WIDTH;
            if ((point.vx + label_half_width) > SCREEN_WIDTH)
            {
                point.vx = SCREEN_WIDTH - label_half_width;
            }
            if (((point.vx - label_half_width) - FIELD_LABEL_MARGIN) < 0)
            {
                point.vx = label_half_width + FIELD_LABEL_MARGIN;
            }
            if (point.vy > FIELD_LABEL_MAX_Y)
            {
                point.vy = FIELD_LABEL_MAX_Y;
            }
            if (point.vy < FIELD_LABEL_MIN_Y)
            {
                point.vy = FIELD_LABEL_MIN_Y;
            }
            button_or_x = point.vx;
            text_ot = ot;
            if (g_field_selected_actor_label == index)
            {
                text_ot = ot - sizeof(u_long);
            }
            text_color = FIELD_TEXT_COLOR_DIM;
            if (g_field_selected_actor_label == index)
            {
                text_color = FIELD_TEXT_COLOR_NORMAL;
            }
            cursor = (s32)field_draw_text((SPRT*)cursor, (s32*)text_ot, ((FieldObjectState*)text_or_state)->name, text_color, button_or_x, (s32)point.vy,
                                          FIELD_TEXT_SHADOW | FIELD_TEXT_ALIGN_CENTER);
            left_glyph_ot = ot;
            point.vy = (u16)point.vy - 8;
            if (g_field_selected_actor_label == index)
            {
                left_glyph_ot = ot - sizeof(u_long);
            }
            cursor = (s32)func_800AD524((u8*)cursor, (s32*)left_glyph_ot, FIELD_LABEL_LEFT_BRACKET, (s32*)&point,
                                        g_field_selected_actor_label == index ? FIELD_LABEL_SELECTED_DIGITS : FIELD_LABEL_NORMAL_DIGITS);
            right_glyph_ot = ot;
            point.vx = (u16)point.vx + 8;
            if (g_field_selected_actor_label == index)
            {
                right_glyph_ot = ot - sizeof(u_long);
            }
            cursor = (s32)func_800AD524((u8*)cursor, (s32*)right_glyph_ot, FIELD_LABEL_RIGHT_BRACKET, (s32*)&point,
                                        g_field_selected_actor_label == index ? FIELD_LABEL_SELECTED_DIGITS : FIELD_LABEL_NORMAL_DIGITS);
            number_ot = ot;
            point.vx = (u16)point.vx + 8;
            if (g_field_selected_actor_label == index)
            {
                number_ot -= sizeof(u_long);
            }
            cursor = (s32)func_800AD208((s32*)number_ot, (void*)cursor, ((FieldObjectState*)text_or_state)->hud.bytes.flags >> 1, 2, (u16*)&point,
                                        g_field_selected_actor_label == index ? FIELD_LABEL_SELECTED_DIGITS : FIELD_LABEL_NORMAL_DIGITS);
            if ((g_field_selected_actor_label == index) && (((FieldObjectState*)text_or_state)->unk8.word >= 0))
            {
                highlight_part = &g_field_object_parts[bit_or_actor];
                highlight_part->scale_z = FIELD_LABEL_SELECTED_SCALE;
                highlight_part->scale_x = FIELD_LABEL_SELECTED_SCALE;
            }
            else
            {
                normal_part = &g_field_object_parts[bit_or_actor];
                normal_part->scale_z = (u8)g_field_label_saved_scale_x[index];
                normal_part->scale_x = (u8)g_field_label_saved_scale_y[index];
            }
            index += 1;
        } while (index < (s32)g_field_label_actor_count);
    }
    render->primitive_cursor = (u8*)cursor;
}

/**
 * @brief Combine quantities for repeated dialog text entries and close the gaps.
 */
void field_merge_dialog_items(void)
{
    s32 index;
    s32 previous;
    u8 amount;
    s32 shift_index;

    index = 0;
    if (g_field_dialog_item_count > 0)
    {
        do
        {
            if (g_field_dialog_item_quantities[index] != 0)
            {
                for (previous = 0; previous < index; previous++)
                {
                    amount = g_field_dialog_item_quantities[previous];
                    if ((amount != 0) && (g_field_dialog_item_texts[index] == g_field_dialog_item_texts[previous]))
                    {
                        shift_index = index;
                        g_field_dialog_item_quantities[previous] = amount + g_field_dialog_item_quantities[index];
                        while (shift_index < (g_field_dialog_item_count - 1))
                        {
                            g_field_dialog_item_quantities[shift_index] = g_field_dialog_item_quantities[shift_index + 1];
                            g_field_dialog_item_texts[shift_index] = g_field_dialog_item_texts[shift_index + 1];
                            shift_index += 1;
                        }
                        index -= 1;
                        g_field_dialog_item_count -= 1;
                        break;
                    }
                }
            }
            index += 1;
        } while (index < g_field_dialog_item_count);
    }
}

/**
 * @brief Move the actor-label selection, or close the text session on START (or once the CD error is gone).
 */
static void field_update_text_session(void)
{
    if ((g_field_text_session_cd_error && !cdrom_get_error_status()) ||
        (!g_field_text_session_cd_error && (g_pad_input == PADh || ((g_pad_ctx->characters[1].info.word & FIELD_CHARACTER_AI) &&
                                                                    g_pad_ctx->characters[1].name[0] && g_pad_input_inject == PADh))))
    {
        g_field_draw_count = 0;
        g_field_text_session_active = 0;
        field_restore_fade_target();
        field_restore_label_actor_parts();
        field_reset_input_repeat();
    }
    else
    {
        g_field_draw_count = 1;
        if (g_field_label_actor_count >= 2U)
        {
            g_pad_input |= g_pad_input_inject;
            if (g_pad_input & (PADLup | PADLleft))
            {
                g_field_selected_actor_label = g_field_selected_actor_label ? g_field_selected_actor_label - 1 : g_field_label_actor_count - 1;
                akao_stop_sfx_by_id(FIELD_SOUND_CURSOR);
            }
            else if (g_pad_input & (PADLdown | PADLright))
            {
                g_field_selected_actor_label = g_field_selected_actor_label == g_field_label_actor_count - 1 ? 0 : g_field_selected_actor_label + 1;
                akao_stop_sfx_by_id(FIELD_SOUND_CURSOR);
            }
        }
    }
}

/**
 * @brief Read a controller's buttons in field bit order, with the left stick as directions.
 * @param index Controller port.
 * @return Button mask, or zero when the controller is missing or still being configured.
 * @note The packet's button bytes are swapped and bits 4-7 reordered into the PAD* layout.
 */
s32 field_read_controller_buttons(s32 index)
{
    ControllerPortState* ports;
    ControllerPortState* port;
    u16 raw_buttons;
    s32 buttons;
    s16 stick;
    s32 port_offset;

    ports = CONTROLLER_STATE->ports;
    port = &ports[index];
    if (port->published_sample.device_type >= CONTROLLER_DEVICE_CONFIGURING)
    {
        return 0;
    }

    raw_buttons = port->published_sample.held_buttons;
    buttons = (raw_buttons >> 8) | ((raw_buttons & 0xFF) << 8);
    buttons = ((u32)(buttons & 0x40) >> 1) | ((buttons & 0x20) << 1) | ((u32)(buttons & 0x80) >> 3) | ((buttons & 0x10) << 3) | (buttons & 0xFF0F);

    if (port->published_sample.device_type != CONTROLLER_DEVICE_DIGITAL)
    {
        stick = port->current_sample.left_stick_x;
        if (stick < -FIELD_PAD_STICK_THRESHOLD)
        {
            buttons |= PADLleft;
        }
        else if (stick > FIELD_PAD_STICK_THRESHOLD)
        {
            buttons |= PADLright;
        }

        port_offset = index * sizeof(*ports);
        stick = ((ControllerPortState*)((u8*)ports + port_offset))->current_sample.left_stick_y;
        if (stick < -FIELD_PAD_STICK_THRESHOLD)
        {
            buttons |= PADLup;
        }
        else if (stick > FIELD_PAD_STICK_THRESHOLD)
        {
            buttons |= PADLdown;
        }
    }

    return buttons;
}

/**
 * @brief Read both controllers and apply the initial delay and key repeat to their buttons.
 * @note A new button fires at once, waits FIELD_PAD_REPEAT_DELAY frames, then repeats every third frame.
 * @note While a modal screen is open, held directions repeat on their own without the other buttons.
 */
void field_update_input_repeat(void)
{
    s32 directions;
    s32 buttons;

    buttons = field_read_controller_buttons(0);
    g_pad_input = 0;
    g_field_buffered_input = 0;
    if (((buttons == g_field_primary_held_buttons) ||
         ((g_field_primary_held_buttons != 0) && (buttons & (g_field_primary_held_buttons | FIELD_PAD_HOLD_BUTTONS)))) &&
        buttons != 0)
    {
        directions = buttons & FIELD_PAD_DIRECTIONS;

        if ((directions != 0) && (g_field_modal_state != 0))
        {
            buttons = directions;
        }
        if (g_field_primary_repeat_delay == 0)
        {
            g_pad_input = buttons;
            g_field_primary_repeat_delay = FIELD_PAD_REPEAT_INTERVAL;
        }
        else
        {
            g_field_primary_repeat_delay -= 1;
            g_pad_input = 0;
        }
    }
    else if (buttons == 0)
    {
        g_field_primary_repeat_delay = 0;
        g_field_primary_held_buttons = 0;
    }
    else
    {
        g_pad_input = buttons;
        g_field_primary_held_buttons = buttons;
        g_field_primary_repeat_delay = FIELD_PAD_REPEAT_DELAY;
    }
    buttons = field_read_controller_buttons(1);
    g_pad_input_inject = 0;
    if (((buttons == g_field_secondary_held_buttons) ||
         ((g_field_secondary_held_buttons != 0) && (buttons & (g_field_secondary_held_buttons | FIELD_PAD_HOLD_BUTTONS)))) &&
        buttons != 0)
    {
        directions = buttons & FIELD_PAD_DIRECTIONS;

        if ((directions != 0) && (g_field_modal_state != 0))
        {
            buttons = directions;
        }
        if (g_field_secondary_repeat_delay == 0)
        {
            g_pad_input_inject = buttons;
            g_field_secondary_repeat_delay = FIELD_PAD_REPEAT_INTERVAL;
        }
        else
        {
            g_field_secondary_repeat_delay -= 1;
            g_pad_input_inject = 0;
        }
    }
    else if (buttons == 0)
    {
        g_field_secondary_repeat_delay = 0;
        g_field_secondary_held_buttons = 0;
    }
    else
    {
        g_pad_input_inject = buttons;
        g_field_secondary_held_buttons = buttons;
        g_field_secondary_repeat_delay = FIELD_PAD_REPEAT_DELAY;
    }
    g_field_buffered_input = g_pad_input;
}

/**
 * @brief Clear delivered input and restart both controllers' held-button delays.
 */
inline void field_reset_input_repeat(void)
{
    g_pad_input = 0;
    g_field_primary_held_buttons = field_read_controller_buttons(0);
    g_field_primary_repeat_delay = FIELD_PAD_REPEAT_DELAY;
    g_pad_input_inject = 0;
    g_field_secondary_held_buttons = field_read_controller_buttons(1);
    g_field_secondary_repeat_delay = FIELD_PAD_REPEAT_DELAY;
    g_field_buffered_input = 0;
}

/**
 * @brief Handle the soft reset, the text session, and the menu, CD error and item-drop buttons.
 * @param render Render half; the text session draws into it.
 * @note Unplugging a controller opens the menu for that controller.
 */
void field_process_input(FieldRenderHalf* render)
{
    ControllerPortState* ports = CONTROLLER_STATE->ports;
    u32 buttons;
    s32 actor_count;
    s32 actor_index;

    buttons = ports[0].published_sample.held_buttons;
    buttons = (buttons >> 8) | ((buttons & 0xFF) << 8);
    buttons = ((buttons & 0x40) >> 1) | ((buttons & 0x20) << 1) | ((buttons & 0x80) >> 3) | ((buttons & 0x10) << 3) | (buttons & 0xFF0F);
    if (g_field_modal_state != 0)
    {
        return;
    }
    if (buttons == FIELD_INPUT_RESET_COMBINATION)
    {
        g_pending_game_state = GAME_STATE_RETURN_TO_TITLE;
        ports[0].small_motor_command = 0;
        ports[0].actuator_control.fields.large_motor_command = 0;
        ports[1].small_motor_command = 0;
        ports[1].actuator_control.fields.large_motor_command = 0;
        akao_cmd_98_9a_9c_9e(0);
        field_fade_song(0, 60, 127);
        return;
    }
    if (g_field_interaction_active != 0)
    {
        D_80122710 = 1;
        return;
    }
    if (D_80122710 != 0)
    {
        D_80122710 = 0;
        field_refresh_party_routes();
    }
    if (g_field_text_session_active != 0)
    {
        field_update_modal_text_session(render);
        return;
    }
    if (!(g_field_actors[0].control.word & FIELD_CONTROL_MODE_MASK))
    {
        for (actor_count = 0; actor_count < FIELD_PARTY_COUNT; actor_count++)
        {
            if (g_field_actors[actor_count].presence != FIELD_ACTOR_UNUSED && g_field_actors[actor_count].command == FIELD_ACTOR_COMMAND_IDLE_AFTER_RELOAD)
            {
                return;
            }
        }
        if (field_text_get_status(0) == -1 && D_800F2298 == 0 && g_field_dialog_screen_mode == 0 && g_field_return_to_title_prompt_state == 0 &&
            D_80122714 == 0 && field_party_reload_pending() == 0)
        {
            if (g_field_menu_controller_types[0] != CONTROLLER_DEVICE_DISCONNECTED && ports[0].published_sample.device_type == CONTROLLER_DEVICE_DISCONNECTED)
            {
                field_run_menu(FIELD_MENU_RENDER_BUFFERS, 0);
            }
            if (g_field_menu_controller_types[1] != CONTROLLER_DEVICE_DISCONNECTED && ports[1].published_sample.device_type == CONTROLLER_DEVICE_DISCONNECTED)
            {
                field_run_menu(FIELD_MENU_RENDER_BUFFERS, 1);
            }
            if (cdrom_get_error_status() != 0)
            {
                g_field_text_session_cd_error = 1;
                ports[0].small_motor_command = 0;
                ports[0].actuator_control.fields.large_motor_command = 0;
                ports[1].small_motor_command = 0;
                ports[1].actuator_control.fields.large_motor_command = 0;
                field_begin_text_session();
                return;
            }
            g_field_menu_controller_types[0] = ports[0].published_sample.device_type;
            g_field_menu_controller_types[1] = ports[1].published_sample.device_type;
            if (D_8012291C != 0)
            {
                if (field_is_scene_fading() == 0)
                {
                    if (g_pad_input == PADh ||
                        ((g_pad_ctx->characters[1].info.word & FIELD_CHARACTER_AI) && g_pad_ctx->characters[1].name[0] && g_pad_input_inject == PADh))
                    {
                        field_run_menu(FIELD_MENU_RENDER_BUFFERS, 0);
                    }
                }
            }
            else
            {
                if (g_pad_input == PADh || g_pad_input == PADRup ||
                    ((g_pad_ctx->characters[1].info.word & FIELD_CHARACTER_AI) && g_pad_ctx->characters[1].name[0] &&
                     (g_pad_input_inject == PADh || g_pad_input_inject == PADRup)))
                {
                    field_run_menu(FIELD_MENU_RENDER_BUFFERS, 0);
                }
            }
            field_play_low_hp_warning();
            if (g_pad_input & PADRleft)
            {
                actor_count = 0;
                if (D_80122980 != 0)
                {
                    for (actor_index = 0; actor_index < FIELD_ACTOR_COUNT; actor_index++)
                    {
                        if (g_field_actors[actor_index].presence != FIELD_ACTOR_UNUSED && g_field_object_states[actor_index].key >= 0x14)
                        {
                            actor_count++;
                        }
                    }
                    if (actor_count < 5)
                    {
                        field_open_item_drop_menu();
                        return;
                    }
                    field_play_sound(FIELD_SOUND_ACTION_REFUSED, FIELD_SOUND_PAN_CENTRE);
                }
            }
        }
    }
}

/**
 * @brief Play the low-HP warning for each player below a quarter of their maximum HP.
 * @return Undefined: declared s32 but falls off the end (v0 stays live at the exit, which keeps the delay slots empty).
 * @note The two players' warnings alternate every sixteen frames; player 2 only warns while pad controlled.
 */
static s32 field_play_low_hp_warning(void)
{
    if (g_field_active_group != 0)
    {
        if (!(g_frame_counter & 0x1F))
        {
            if ((g_field_object_states[0].unk4.word != 0) && ((u32)(g_field_object_states[0].unk4.word * 4) < (u32)g_field_object_states[0].unk0))
            {
                field_play_sound(FIELD_SOUND_LOW_HP, FIELD_SOUND_PAN_CENTRE);
            }
        }

        if (!((g_frame_counter + 0x10) & 0x1F) && !(g_field_actors[1].control.word & FIELD_CONTROL_MODE_MASK) && (g_field_object_states[1].unk4.word != 0) &&
            ((u32)(g_field_object_states[1].unk4.word * 4) < (u32)g_field_object_states[1].unk0))
        {
            field_play_sound(FIELD_SOUND_LOW_HP, FIELD_SOUND_PAN_CENTRE);
        }
    }
}

/**
 * @brief Run the MENU overlay and the GOLEM or GNAME screens it asks for, until the menu closes.
 * @param render_buffers Render buffers handed to MENU.
 * @param controller Controller that opened the menu; unused.
 * @note While D_8012291C is set, or an animation binding is busy, the actor-label session opens instead.
 */
static void field_run_menu(void* render_buffers, s32 controller)
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
    if (menu_locked != 0 || g_field_actor_bindings[0].state != FIELD_BINDING_IDLE || g_field_actor_bindings[1].state != FIELD_BINDING_IDLE ||
        g_field_actor_bindings[2].state != FIELD_BINDING_IDLE)
    {
        field_begin_text_session();
        return;
    }
    field_play_sound(FIELD_SOUND_MENU_OPEN, FIELD_SOUND_PAN_CENTRE);
    field_reset_actor_resources();

    g_active_script = 0;

    for (;;)
    {
        cdrom_stream(CD_RES_MENU_BIN, FIELD_SUBOVERLAY_ADDRESS);
        cdrom_wait_queue_empty();
        screen_id = func_801405B0((s32)render_buffers);
        if (screen_id == FIELD_MENU_CLOSED)
        {
            field_rebuild_party_actions(1);
            break;
        }
        if (screen_id == FIELD_MENU_GOLEM)
        {
            field_rebuild_party_actions(1);
            cdrom_stream(CD_RES_GOLEM_BIN, FIELD_SUBOVERLAY_ADDRESS);
            cdrom_wait_queue_empty();
            func_80140024(FIELD_GOLEM_WORK_BUFFER, 1);
            field_golem_rebuild_current_grid();
            field_reset_actor_resources();
            field_text_reset_windows();
            g_active_script = screen_id;
            g_script_repeat_count = 0;
        }
        else
        {
            field_rebuild_party_actions(1);
            cdrom_stream(CD_RES_GNAME_BIN, FIELD_SUBOVERLAY_ADDRESS);
            cdrom_wait_queue_empty();
            if ((screen_id == FIELD_MENU_NAME_ENTRY_B) || (screen_id == FIELD_MENU_NAME_ENTRY_C))
            {
                func_80140004(FIELD_GNAME_WORK_BUFFER, g_field_rename_initial_name, g_field_rename_target, 1, g_field_rename_source, g_field_rename_custom_name, 0);
            }
            else
            {
                func_80140004(FIELD_GNAME_WORK_BUFFER, g_field_rename_initial_name, g_field_rename_target, screen_id, g_field_rename_source, g_field_rename_custom_name, 0);
            }
            field_text_reset_windows();
            g_script_repeat_count = g_field_rename_source;
            if ((screen_id == FIELD_MENU_NAME_ENTRY_B) || (screen_id == FIELD_MENU_NAME_ENTRY_C))
            {
                g_script_repeat_count = 0;
                g_active_script = screen_id;
            }
            else
            {
                g_active_script = g_field_rename_item_category + 1;
            }
        }
    }
    field_reset_actor_resources();
}

/**
 * @brief Open the actor-label text session.
 */
static void field_begin_text_session(void)
{
    g_field_text_session_active = 1;
    field_set_cd_error_fade_target();
    field_reset_input_repeat();
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
 * @brief Run one frame of the text session and restore the field when it closes.
 * @param render Render half receiving the text.
 */
static void field_update_modal_text_session(FieldRenderHalf* render)
{
    if (g_field_text_session_active != 0)
    {
        field_update_text_session();
        if (g_field_text_session_active != 0)
        {
            field_draw_text_session(render);
            return;
        }
        field_text_reset_windows();
        akao_cmd_98_9a_9c_9e(2);
        field_fade_song(0, 60, 127);
    }
}

/**
 * @brief Apply the saved sound and vibration options and rebuild the party's action slots.
 * @param refresh_only Nonzero keeps the party membership and HP; zero also reloads them from the saved game.
 * @note Action slots 0-1 are the two commands, 4-7 the skills: a technique, an instrument
 *       (FIELD_SKILL_INSTRUMENT plus an item record) or none (FIELD_SKILL_NONE).
 * @note The FieldGameState pointers here are g_pad_ctx advanced by a character's offset, so their
 *       characters[0] is that character, the way the original addresses the per-character data.
 */
void field_rebuild_party_actions(s32 refresh_only)
{
    FieldPlayerRecord* member;
    FieldPlayerRecord* player;
    FieldPlayerRecord* equipment_player;
    s32 parameter;
    s16 technique_action_id;
    u8* save_base;
    u8* instrument_icons;
    s32 skill_character_offset;
    s32 skill_row_offset;
    s32 skill_none;
    s32 skill_empty;
    u8* animation_params;
    u8* parameter_params;
    FieldGameState* command_save;
    s32 character_kind;
    s32 controllers_or_is_player;
    s32 equipment_offset;
    s32 equipment_index;
    s32 slot_or_type;
    s32 command_action_offset;
    s32 member_offset;
    s32 actor_words;
    s32 player_index;
    s32 character_offset;
    s32 row_offset;
    s32 state_offset;
    s32 skill_action_offset;
    u16 hp;
    u32 equipment_info;
    u8 spell;
    u8 weapon_type;
    u8 skill;
    FieldActor* actor;
    FieldActor* actor_base;
    FieldActionSlot* command_action;
    FieldItemRecord* instrument;
    FieldActionSlot* technique_action;
    FieldGameState* player_save;
    FieldActionSlot* instrument_action;
    FieldGameState* skills_start;
    FieldActionSlot* empty_action;
    FieldGameState* member_save;
    FieldObjectState* health;
    FieldGameState* equipment;
    FieldGameState* command_view;
    FieldGameState* skill_cursor;

    akao_set_paused(g_pad_ctx->mono_sound ^ 1);
    cdrom_set_audio_volume(0x7F, g_pad_ctx->mono_sound);
    controllers_or_is_player = (s32)CONTROLLER_STATE;
    ((ControllerState*)controllers_or_is_player)->ports[0].actuators_enabled = g_pad_ctx->vibration;
    if ((g_pad_ctx->characters[1].info.word & FIELD_CHARACTER_AI) && (g_pad_ctx->characters[1].name[0] != 0))
    {
        ((ControllerState*)controllers_or_is_player)->ports[1].actuators_enabled = g_pad_ctx->vibration;
    }
    else
    {
        ((ControllerState*)controllers_or_is_player)->ports[1].actuators_enabled = 0;
    }
    if (refresh_only == 0)
    {
        player_index = 0;
        do
        {
            member_offset = player_index * sizeof(FieldCharacterRecord);
            member = &g_field_player_records[player_index];

            if (((FieldGameState*)((u8*)g_pad_ctx + member_offset))->characters[0].name[0] != 0)
            {
                member->head.bytes.weapon_type = FIELD_WEAPON_TYPE_UNSET;
                member_save = (FieldGameState*)((u8*)g_pad_ctx + member_offset);
                member->head.bits.active = 1;
                slot_or_type = member_save->characters[0].info.bytes[0] & FIELD_CHARACTER_TYPE_MASK;
                character_kind = slot_or_type;
                if (character_kind < FIELD_CHARACTER_GUEST)
                {
                    member->head.bits.alt_appearance = character_kind;
                    member->character_kind = FIELD_PLAYER_KIND_HERO;
                }
                else if (character_kind == FIELD_CHARACTER_GUEST)
                {
                    member->head.bits.alt_appearance = 0;
                    member->character_id = member_save->characters[0].info.bytes[1];
                    member->character_kind = FIELD_PLAYER_KIND_PARTNER;
                }
                else if (character_kind == FIELD_CHARACTER_COMPANION)
                {
                    member->head.bits.alt_appearance = 0;
                    member->character_id = member_save->characters[0].info.bytes[1];
                    member->character_kind = FIELD_PLAYER_KIND_COMPANION;
                }
                else if (character_kind == FIELD_CHARACTER_GOLEM)
                {
                    member->head.bits.alt_appearance = 0;
                    member->character_id = member_save->characters[0].info.bytes[1] + FIELD_COMPANION_GOLEM_ID_BASE;
                    member->character_kind = FIELD_PLAYER_KIND_COMPANION;
                }
            }
            else
            {
                member->character_kind = FIELD_PLAYER_KIND_PARTNER;
                member->character_id = 0;
                member->head.bits.active = 0;
            }
            player_index += 1;

        } while (player_index < FIELD_PARTY_COUNT);
        field_upload_golem_palettes();
    }
    player_index = 0;
    do
    {
        player = (FieldPlayerRecord*)((u8*)g_field_player_records + (player_index << 9) + player_index * 0x68);
        state_offset = player_index * sizeof(FieldObjectState);
        character_offset = player_index * sizeof(FieldCharacterRecord);
        row_offset = player_index * sizeof(FieldActionRow);
        actor_words = player_index * 0x14;

        if (player->head.bytes.flags & FIELD_PLAYER_ACTIVE)
        {
            actor_base = g_field_actors;
            actor = (FieldActor*)((actor_words + player_index) * 4 + (u8*)actor_base);
            player_save = (FieldGameState*)((u8*)g_pad_ctx + character_offset);
            actor->control.word = ((actor->control.word & ~FIELD_CONTROL_MODE_MASK) | ((player_save->characters[0].info.bytes[0] >> 7) ^ 1));
            weapon_type = FIELD_ITEM_TYPE(player_save->characters[0].equipment[FIELD_WEAPON_SLOT].info.word);
            controllers_or_is_player = player_index < FIELD_PLAYER_COUNT;
            if (player->head.bytes.weapon_type != weapon_type)
            {
                player->head.bytes.weapon_type = weapon_type;
                if (controllers_or_is_player != 0)
                {
                    field_apply_weapon_action_params(player_index);
                }
                if (refresh_only == 0)
                {
                    health = &g_field_object_states[player_index];
                    hp = ((FieldGameState*)((u8*)g_pad_ctx + character_offset))->characters[0].hp;
                    health->unk8.word = (health->unk8.word & 0xFF000000) | hp;
                    health->unk0 = hp;
                    health->unk4.word = hp;
                }
                if ((g_field_scene_mode_bit != 0) && (refresh_only != 0) && (controllers_or_is_player != 0))
                {
                    field_load_weapon_sfx_table(player_index, player->head.bytes.weapon_type);
                }
            }
            player->unk258 = 0;
            if ((u32)(player->head.bytes.weapon_type - 1) < 2U)
            {
                player->unk258 = 1;
                equipment_index = 1;
                equipment_player = player;
                equipment_offset = character_offset + sizeof(FieldItemRecord);
                do
                {
                    equipment = (FieldGameState*)((u8*)g_pad_ctx + equipment_offset);
                    if (equipment->characters[0].equipment[0].kind != 0)
                    {
                        equipment_info = equipment->characters[0].equipment[0].info.word;
                        if ((FIELD_ITEM_CATEGORY(equipment_info) == FIELD_ITEM_CATEGORY_ARMOR) && !FIELD_ITEM_TYPE(equipment_info))
                        {
                            equipment_player->unk258 = 0;
                        }
                    }
                    equipment_index += 1;
                    equipment_offset += sizeof(FieldItemRecord);
                } while (equipment_index < FIELD_EQUIPMENT_SLOT_COUNT);
            }
            if (player_index == FIELD_COMPANION_INDEX && g_field_player_records[FIELD_COMPANION_INDEX].character_kind == player_index)
            {
                field_load_party_script_page(2, g_field_player_records[FIELD_COMPANION_INDEX].character_id + FIELD_RES_COMPANION_ACTIONS);
            }
            else
            {
                slot_or_type = 0;
                animation_params = g_field_action_animation_parameters;
                parameter_params = g_field_action_animation_parameters + 1;
                command_save = (FieldGameState*)((u8*)g_pad_ctx + character_offset);
                command_action_offset = row_offset;
                do
                {
                    command_view = (FieldGameState*)((u8*)command_save + slot_or_type);
                    command_action = (FieldActionSlot*)(command_action_offset + (u32)g_field_resource_actions);
                    command_action->command = command_view->characters[0].info.actions.commands[0];
                    command_action->animation = animation_params[command_view->characters[0].info.actions.commands[0] * 2];
                    command_action->parameter = parameter_params[command_view->characters[0].info.actions.commands[0] * 2];
                    command_action_offset += sizeof(FieldActionSlot);
                    slot_or_type += 1;
                } while (slot_or_type < FIELD_COMMAND_SLOT_COUNT);
                skill_character_offset = character_offset;
                skill_none = FIELD_SKILL_NONE;
                skill_row_offset = row_offset;
                save_base = (u8*)g_pad_ctx;
                skills_start = (FieldGameState*)(save_base + skill_character_offset);
                skill_action_offset = FIELD_SKILL_ACTION_BASE * sizeof(FieldActionSlot);
                skill_cursor = skills_start;
                do
                {
                    /* Net-zero writes: they keep loop.c from hoisting and strength-reducing these offsets, which the original loop does not do. */
                    skill_character_offset = ~skill_character_offset;
                    skill_character_offset = ~skill_character_offset;
                    skill_action_offset = (~(u32)skill_action_offset);
                    skill_action_offset = (~(u32)skill_action_offset);
                    skill_row_offset = (~(u32)skill_row_offset);
                    skill_row_offset = (~(u32)skill_row_offset);
                    skill_none = (~(u32)skill_none);
                    skill_none = (~(u32)skill_none);
                    skills_start = (FieldGameState*)(~(u32)skills_start);
                    skills_start = (FieldGameState*)(~(u32)skills_start);
                    save_base = (u8*)(~(u32)save_base);
                    save_base = (u8*)(~(u32)save_base);
                    skill_cursor = (FieldGameState*)(~(u32)skill_cursor);
                    skill_cursor = (FieldGameState*)(~(u32)skill_cursor);
                    skill_empty = skill_cursor->characters[0].info.actions.skills[0] == skill_none;
                    if (skill_empty)
                    {
                        empty_action = (FieldActionSlot*)(skill_action_offset + skill_row_offset + (u32)g_field_resource_actions);
                        empty_action->flags.instrument = 0;
                        empty_action->flags.target_filter = skill_none;
                        empty_action->command = 0;
                        empty_action->animation = 0;
                        empty_action->parameter = 0;
                        empty_action->flags.target_group = 0;
                    }
                    else
                    {
                        skill = skill_cursor->characters[0].info.actions.skills[0];
                        if (skill & FIELD_SKILL_INSTRUMENT)
                        {
                            instrument_action = (FieldActionSlot*)(skill_action_offset + skill_row_offset + (u32)g_field_resource_actions);
                            instrument_action->command = 0;
                            instrument_action->flags.instrument = 1;
                            instrument = (FieldItemRecord*)(save_base + (skill_character_offset + FIELD_OFFSET_OF(FieldGameState, characters)) +
                                                            (((skill & ~FIELD_SKILL_INSTRUMENT) << 6) + FIELD_OFFSET_OF(FieldCharacterRecord, unk150)));
                            instrument_action->flags.target_filter = instrument->derived.bytes[1] >> 1;
                            instrument_icons = g_field_instrument_icons;
                            instrument_action->animation = instrument_icons[instrument->derived.bytes[0]];
                            instrument_icons = 0;
                            spell = instrument->derived.bytes[1];
                            parameter = 0x8018;
                            parameter += spell;
                            parameter += instrument->derived.bytes[0] * 14;
                            if (!(spell & 1))
                            {
                                parameter += 0x800;
                            }
                            instrument_action->parameter = parameter;
                            instrument_action->flags.target_group = 0;
                        }
                        else
                        {
                            technique_action = (FieldActionSlot*)(skill_action_offset + skill_row_offset + (u32)g_field_resource_actions);
                            technique_action->flags.instrument = 0;
                            skill = skill_cursor->characters[0].info.actions.skills[0];
                            technique_action_id = skill | FIELD_ACTION_TECHNIQUE;
                            technique_action->flags.target_filter = skill_none;
                            technique_action->animation = FIELD_TECHNIQUE_ANIMATION;
                            technique_action->command = technique_action_id;
                            skill = skill_cursor->characters[0].info.actions.skills[0];
                            weapon_type = player->head.bytes.weapon_type;
                            technique_action->flags.target_group = 0;
                            technique_action->parameter =
                                (((skill + FIELD_TECHNIQUE_SEQUENCE_BASE) | ~FIELD_ACTION_TECHNIQUE_MASK) + (weapon_type * FIELD_TECHNIQUES_PER_WEAPON));
                        }
                    }
                    skill_cursor = (FieldGameState*)((u8*)skill_cursor + 1);
                    skill_action_offset += sizeof(FieldActionSlot);
                } while ((s32)skill_cursor < (s32)((u8*)skills_start + FIELD_SKILL_SLOT_COUNT));
            }
        }
        player_index += 1;

    } while (player_index < FIELD_PARTY_COUNT);
    field_refresh_party_routes();
}

/**
 * @brief Load GNAME.BIN and run the name-entry screen.
 *
 * Argument order follows gname_run, which receives its work buffer first and
 * allow_empty_cancel = 0 last.
 *
 * @param initial_name Name shown when the screen opens.
 * @param active_name Name buffer edited by the UI.
 * @param source_mode Random-name source selector.
 * @param history_index History-list entry selector.
 * @param custom_name Custom random-name source.
 */
void field_run_name_entry(u8* initial_name, u8* active_name, s32 source_mode, s32 history_index, s32 custom_name)
{
    field_reset_actor_resources();
    cdrom_stream(CD_RES_GNAME_BIN, FIELD_SUBOVERLAY_ADDRESS);
    cdrom_wait_queue_empty();
    func_80140004(FIELD_GNAME_WORK_BUFFER, initial_name, active_name, source_mode, history_index, custom_name, 0);
    field_text_reset_windows();
    field_reset_actor_resources();
}

/**
 * @brief Load ZUKAN.BIN and run the encyclopedia screen.
 * @param context Forwarded to the ZUKAN entry point; meaning not yet established.
 */
void field_run_zukan(s32 context)
{
    field_reset_actor_resources();
    cdrom_stream(CD_RES_ZUKAN_BIN, FIELD_SUBOVERLAY_ADDRESS);
    cdrom_wait_queue_empty();
    func_80140E00(FIELD_GNAME_WORK_BUFFER, context);
    field_reset_actor_resources();
}

/**
 * @brief Load GOSUB.BIN and open a screen sequence, unless a sub-overlay is already active.
 *
 * Sets g_field_modal_state to FIELD_MODAL_GOSUB for the duration and clears the gosub result count.
 *
 * @param screen_sequence Terminated s32 array passed to gosub_open_screen_sequence.
 */
void field_open_gosub_screen_sequence(void* screen_sequence)
{
    if (g_field_modal_state == FIELD_MODAL_NONE)
    {
        D_801227F0 = 1;
        g_gosub_result_count = 0;
        field_reset_actor_resources();
        cdrom_stream(CD_RES_GOSUB_BIN, FIELD_SUBOVERLAY_ADDRESS);
        cdrom_wait_queue_empty();
        g_field_modal_state = FIELD_MODAL_GOSUB;
        g_field_gosub_phase = FIELD_GOSUB_RUNNING;
        func_80140080(FIELD_GOSUB_WORK_BUFFER, screen_sequence);
    }
}

/**
 * @brief Open the selling screen when inventory is present, otherwise show a notice.
 * @param shop_options Value forwarded to the shop; meaning not yet established.
 * @note The inventory loop tests items[0] on every pass, so only the first record counts.
 */
void field_open_shop_mode_0(s32 shop_options)
{
    s32 count;
    s32 i;

    if (g_field_modal_state == FIELD_MODAL_NONE)
    {
        count = 0;
        for (i = 0; i < FIELD_ITEM_COUNT; i++)
        {
            if (g_pad_ctx->items[0].kind != 0)
            {
                count++;
                break;
            }
        }
        for (i = 0; i < FIELD_ITEM_KIND_COUNT; i++)
        {
            if (g_pad_ctx->item_counts[i] != 0)
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
            field_reset_actor_resources();
            cdrom_stream(CD_RES_SHOP_BIN, FIELD_SUBOVERLAY_ADDRESS);
            cdrom_wait_queue_empty();
            g_field_shop_active = 1;
            g_field_modal_state = FIELD_MODAL_SHOP;
            func_80140004(FIELD_SHOP_WORK_BUFFER, 0, 0, 0, 0, shop_options);
        }
    }
}

/**
 * @brief Open the shop with a caller-supplied entry list.
 * @param entry_count Number of entries in the supplied list.
 * @param entries Address of the eight-byte shop entry array.
 * @param list_options Additional list data forwarded to the shop; meaning unresolved.
 * @param shop_options Value forwarded as the final shop argument.
 */
void field_open_shop_mode_1(s32 entry_count, s32 entries, s32 list_options, s32 shop_options)
{
    if (g_field_modal_state == FIELD_MODAL_NONE)
    {
        field_reset_actor_resources();
        cdrom_stream(CD_RES_SHOP_BIN, FIELD_SUBOVERLAY_ADDRESS);
        cdrom_wait_queue_empty();
        g_field_shop_active = 1;
        g_field_modal_state = FIELD_MODAL_SHOP;
        func_80140004(FIELD_SHOP_WORK_BUFFER, 1, entry_count, entries, list_options, shop_options);
    }
}

/**
 * @brief Run one frame of the open modal screen and give the input back to the field when it closes.
 * @param render Render half the modal screens draw into.
 */
void field_update_modal(FieldRenderHalf* render)
{
    s32 result;
    s32 index; /* Also the controller port read by the closing code (0). */

    switch (g_field_modal_state)
    {
    case FIELD_MODAL_SHOP:
        if ((g_field_shop_active != 0) && (func_801400D4(render) != 0))
        {
            g_field_shop_active = 0;
            g_field_modal_state = FIELD_MODAL_NONE;
            field_reset_actor_resources();
            return;
        }
    case FIELD_MODAL_NONE:
        return;
    case FIELD_MODAL_GOSUB:
        if (g_field_gosub_phase != FIELD_GOSUB_IDLE)
        {
            if (g_field_gosub_phase >= FIELD_GOSUB_RUNNING)
            {
                if (func_801400C4(render) != 0)
                {
                    DrawSync(0);
                    g_field_gosub_phase = FIELD_GOSUB_CLOSING;
                    field_reset_actor_resources();
                    return;
                }
            }
            else
            {
                DrawSync(0);
                field_text_reset_windows();
                D_801227F0 = 2;
                g_field_modal_state = FIELD_MODAL_NONE;
                g_field_gosub_phase = FIELD_GOSUB_IDLE;
                return;
            }
        }
        return;
    case FIELD_MODAL_CARDA:
        if ((g_field_card_overlay_mode != 0) && (func_80140370(render) != 0))
        {
            field_reset_actor_resources();
            switch (g_field_card_overlay_mode)
            {
            case 2:
                field_rebuild_party_actions(0);
                for (index = 0; index < FIELD_PARTY_COUNT; index++)
                {
                    g_field_player_records[index].resource_id = 0;
                    g_field_player_records[index].portrait_index = FIELD_PORTRAIT_NONE;
                }
                g_music_track_index = g_pad_ctx->music_track;
                field_set_scene_parameters(g_pad_ctx->scene_mode, g_pad_ctx->field_flags, g_pad_ctx->entry_config, g_pad_ctx->layout_flags,
                                           g_pad_ctx->option_id, g_pad_ctx->sub_mode);
                field_set_fade_target_only(FIELD_COLOR_SCALE_NEUTRAL, FIELD_COLOR_SCALE_NEUTRAL, FIELD_COLOR_SCALE_NEUTRAL, 8);
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
        if ((g_field_niki_addhero_state != 0) && (func_801401F0(render) != 0))
        {
            field_reset_actor_resources();
            g_field_niki_addhero_state = 0;
            g_field_modal_state = FIELD_MODAL_NONE;
            return;
        }
        return;
    case FIELD_MODAL_ADDHERO:
        if (g_field_niki_addhero_state != 0)
        {
            result = func_801401F8(render);
            switch (result)
            {
            case 1:
                g_field_player_records[1].head.bits.active = 0;
                g_field_player_records[1].head.bits.alt_appearance = g_pad_ctx->characters[1].info.bytes[0] & 1;
                g_field_player_records[1].character_kind = FIELD_PLAYER_KIND_HERO;
                field_activate_actor_resource_slot(-2, 0, 0);
                field_reset_actor_resources();
                g_field_niki_addhero_state = 0;
                g_field_modal_state = FIELD_MODAL_NONE;
                return;
            case 3:
                field_reset_actor_resources();
                g_field_niki_addhero_state = 0;
                g_field_modal_state = FIELD_MODAL_NONE;
                return;
            case 2:
                field_release_actor_resource_slot(0);
                field_reset_actor_resources();
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
            index = 0;
            g_field_modal_state = FIELD_MODAL_NONE;
            g_pad_input = 0;
            break;
        }
        field_text_reset_scratch();
        field_draw_empty_shop_notice(render);
        field_text_upload_immediate_cache();
        return;
    case FIELD_MODAL_DUEL_INTRO:
        if (field_draw_duel_intro(render) == 0)
        {
            return;
        }
        field_play_set_sfx(0, FIELD_SOUND_PAN_CENTRE, 0, 3);
        field_text_reset_windows();
        index = 0;
        g_field_modal_state = FIELD_MODAL_NONE;
        g_pad_input = 0;
        break;
    case FIELD_MODAL_DUEL_RESULT:
        if (field_draw_duel_result(render) == 0 || field_party_reload_reading() != 0)
        {
            return;
        }
        field_close_battle_results();
        field_text_reset_windows();
        index = 0;
        g_field_modal_state = FIELD_MODAL_NONE;
        g_pad_input = 0;
        break;
    default:
        return;
    }
    g_field_primary_held_buttons = field_read_controller_buttons(index);
    g_field_primary_repeat_delay = FIELD_PAD_REPEAT_DELAY;
    g_pad_input_inject = 0;
    g_field_secondary_held_buttons = field_read_controller_buttons(1);
    g_field_secondary_repeat_delay = FIELD_PAD_REPEAT_DELAY;
    g_field_buffered_input = 0;
    field_restore_fade_target();
}

/**
 * @brief Open the empty-inventory notice with a red fade.
 * @param hidden Nonzero suppresses the notice text.
 */
static void field_begin_empty_shop_notice(s32 hidden)
{
    g_field_modal_state = FIELD_MODAL_EMPTY_SHOP;
    field_set_fade_target_only(0xC0, 0x80, 0x80, 8);
    field_play_sound(FIELD_SOUND_EMPTY_SHOP, FIELD_SOUND_PAN_CENTRE);
    g_field_shop_notice_hidden = hidden;
}

/**
 * @brief Draw the empty-inventory notice unless its text is suppressed.
 * @param render Render half receiving the notice.
 */
static void field_draw_empty_shop_notice(FieldRenderHalf* render)
{
    SPRT* packet_cursor;

    packet_cursor = (SPRT*)render->primitive_cursor;
    if (g_field_shop_notice_hidden == 0)
    {
        packet_cursor = field_draw_text(packet_cursor, (s32*)render->ordering_table, field_dialog_text(&D_800EC400, 30), FIELD_TEXT_COLOR_NORMAL,
                                        SCREEN_WIDTH / 2, 104, FIELD_TEXT_ALIGN_CENTER);
    }
    render->primitive_cursor = (u8*)packet_cursor;
}

/**
 * @brief Start the duel introduction panels sliding in from off-screen.
 */
void field_begin_duel_intro(void)
{
    g_field_modal_state = FIELD_MODAL_DUEL_INTRO;
    field_set_fade_target_only(0xC0, 0xC0, 0xC0, 8);
    field_upload_player_icons();
    field_play_sound(FIELD_SOUND_DUEL_INTRO, FIELD_SOUND_PAN_CENTRE);
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
    field_upload_player_icons();
    field_play_sound(FIELD_SOUND_DUEL_RESULT, FIELD_SOUND_PAN_CENTRE);
    g_field_duel_panel_phase = FIELD_DUEL_SLIDE_IN;
    g_field_duel_panel_hold_frames = 0;
    g_field_duel_panel_offset = FIELD_DUEL_PANEL_START_OFFSET;

    if ((g_field_actors[0].animation & FIELD_ANIMATION_INDEX_MASK) == FIELD_ANIMATION_DUEL_LOST)
    {
        g_field_duel_winner = 1;
        g_pad_ctx->characters[0].duel_losses = g_pad_ctx->characters[0].duel_losses + 1;
        if ((u32)(g_pad_ctx->characters[1].info.bytes[0] & FIELD_CHARACTER_TYPE_MASK) < FIELD_CHARACTER_GUEST)
        {
            g_pad_ctx->characters[1].duel_wins = g_pad_ctx->characters[1].duel_wins + 1;
        }
    }
    else
    {
        g_field_duel_winner = 0;
        g_pad_ctx->characters[0].duel_wins = g_pad_ctx->characters[0].duel_wins + 1;
        if ((u32)(g_pad_ctx->characters[1].info.bytes[0] & FIELD_CHARACTER_TYPE_MASK) < FIELD_CHARACTER_GUEST)
        {
            g_pad_ctx->characters[1].duel_losses = g_pad_ctx->characters[1].duel_losses + 1;
        }
    }
}

/**
 * @brief Animate and draw the opposing players and their duel records.
 * @param render Render half receiving the panels.
 * @return Nonzero after the panels have slid out.
 */
static s32 field_draw_duel_intro(FieldRenderHalf* render)
{
    u8 record_text[56];
    u8 loss_text[56];
    s32 packet_cursor;
    u_long* ot;

    packet_cursor = (s32)render->primitive_cursor;
    ot = render->ordering_table;
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
            field_play_sound(FIELD_SOUND_DUEL_PANEL_OUT, FIELD_SOUND_PAN_CENTRE);
            g_field_duel_panel_phase = FIELD_DUEL_SLIDE_OUT;
            g_field_duel_panel_offset = -1;
        }
        break;
    case FIELD_DUEL_SLIDE_OUT:
        g_field_duel_panel_offset *= 2;
        if (g_field_duel_panel_offset < FIELD_DUEL_PANEL_END_OFFSET)
        {
            g_field_duel_panel_phase = FIELD_DUEL_FINISHED;
        }
        break;
    case FIELD_DUEL_FINISHED:
        return 1;
    }

    packet_cursor = field_draw_player_icon(packet_cursor, ot, 0, g_field_duel_panel_offset + 50, 34, 1);
    packet_cursor = func_800AF950(packet_cursor, ot, g_pad_ctx->characters[0].name, FIELD_TEXT_COLOR_NORMAL, g_field_duel_panel_offset + 108, 50, 0, 5, 384,
                                  384, -4, FIELD_DUEL_PANEL_MOVING);

    field_format_number(record_text, g_pad_ctx->characters[0].duel_wins, 0);
    field_append_dialog_text(record_text, &D_800EC408, 34);
    field_format_number(loss_text, g_pad_ctx->characters[0].duel_losses, 0);
    field_append_name(record_text, loss_text);
    field_append_dialog_text(record_text, &D_800EC40A, 35);

    packet_cursor = func_800AF950(packet_cursor, ot, record_text, FIELD_TEXT_COLOR_NORMAL, g_field_duel_panel_offset + 108, 66, 0, 6, FIELD_DUEL_TEXT_SCALE,
                                  FIELD_DUEL_TEXT_SCALE, FIELD_DUEL_TEXT_SLANT, FIELD_DUEL_PANEL_MOVING);
    packet_cursor = func_800AF950(packet_cursor, ot, field_dialog_text(&D_800EC406, 33), FIELD_TEXT_COLOR_NORMAL, 160, 100, 2, 7, FIELD_DUEL_TEXT_SCALE,
                                  FIELD_DUEL_TEXT_SCALE, FIELD_DUEL_TEXT_SLANT, FIELD_DUEL_PANEL_MOVING);
    packet_cursor = field_draw_player_icon(packet_cursor, ot, 1, 222 - g_field_duel_panel_offset, 134, 0);
    packet_cursor = func_800AF950(packet_cursor, ot, g_pad_ctx->characters[1].name, FIELD_TEXT_COLOR_NORMAL, 212 - g_field_duel_panel_offset, 150, 1, 8,
                                  FIELD_DUEL_TEXT_SCALE, FIELD_DUEL_TEXT_SCALE, FIELD_DUEL_TEXT_SLANT, FIELD_DUEL_PANEL_MOVING);

    if ((u32)(g_pad_ctx->characters[1].info.bytes[0] & FIELD_CHARACTER_TYPE_MASK) < FIELD_CHARACTER_GUEST)
    {
        field_format_number(record_text, g_pad_ctx->characters[1].duel_wins, 0);
        field_append_dialog_text(record_text, &D_800EC408, 34);
        field_format_number(loss_text, g_pad_ctx->characters[1].duel_losses, 0);
        field_append_name(record_text, loss_text);
        field_append_dialog_text(record_text, &D_800EC40A, 35);
        packet_cursor = func_800AF950(packet_cursor, ot, record_text, FIELD_TEXT_COLOR_NORMAL, 212 - g_field_duel_panel_offset, 166, 1, 9,
                                      FIELD_DUEL_TEXT_SCALE, FIELD_DUEL_TEXT_SCALE, FIELD_DUEL_TEXT_SLANT, FIELD_DUEL_PANEL_MOVING);
    }

    render->primitive_cursor = (u8*)packet_cursor;
    return 0;
}

/**
 * @brief Animate and draw the winner and their duel record.
 * @param render Render half receiving the panels.
 * @return Nonzero after the panel has slid out.
 */
static s32 field_draw_duel_result(FieldRenderHalf* render)
{
    u8 record_text[56];
    u8 loss_text[56];
    s32 packet_cursor;
    u_long* ot;

    packet_cursor = (s32)render->primitive_cursor;
    ot = render->ordering_table;
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
            field_play_sound(FIELD_SOUND_DUEL_PANEL_OUT, FIELD_SOUND_PAN_CENTRE);
            g_field_duel_panel_phase = FIELD_DUEL_SLIDE_OUT;
            g_field_duel_panel_offset = -1;
        }
        break;
    case FIELD_DUEL_SLIDE_OUT:
        g_field_duel_panel_offset *= 2;
        if (g_field_duel_panel_offset < FIELD_DUEL_PANEL_END_OFFSET)
        {
            g_field_duel_panel_phase = FIELD_DUEL_FINISHED;
        }
        break;
    case FIELD_DUEL_FINISHED:
        return 1;
    }

    packet_cursor = func_800AF950(packet_cursor, ot, field_dialog_text(&D_800EC40C, 36), FIELD_TEXT_COLOR_NORMAL, 160, 52, 2, 5, FIELD_DUEL_TITLE_SCALE,
                                  FIELD_DUEL_TITLE_SCALE, FIELD_DUEL_TEXT_SLANT, FIELD_DUEL_PANEL_MOVING);
    packet_cursor = field_draw_player_icon(packet_cursor, ot, g_field_duel_winner, g_field_duel_panel_offset + 50, 84, 1);
    packet_cursor = func_800AF950(packet_cursor, ot, g_pad_ctx->characters[g_field_duel_winner].name, FIELD_TEXT_COLOR_NORMAL, g_field_duel_panel_offset + 108,
                                  100, 0, 6, FIELD_DUEL_WINNER_SCALE, FIELD_DUEL_WINNER_SCALE, FIELD_DUEL_TEXT_SLANT, FIELD_DUEL_PANEL_MOVING);

    if ((u32)(g_pad_ctx->characters[g_field_duel_winner].info.bytes[0] & FIELD_CHARACTER_TYPE_MASK) < FIELD_CHARACTER_GUEST)
    {
        field_format_number(record_text, g_pad_ctx->characters[g_field_duel_winner].duel_wins, 0);
        field_append_dialog_text(record_text, &D_800EC408, 34);
        field_format_number(loss_text, g_pad_ctx->characters[g_field_duel_winner].duel_losses, 0);
        field_append_name(record_text, loss_text);
        field_append_dialog_text(record_text, &D_800EC40A, 35);
        packet_cursor = func_800AF950(packet_cursor, ot, record_text, FIELD_TEXT_COLOR_NORMAL, g_field_duel_panel_offset + 140, 132, 0, 7,
                                      FIELD_DUEL_TEXT_SCALE, FIELD_DUEL_TEXT_SCALE, FIELD_DUEL_TEXT_SLANT, FIELD_DUEL_PANEL_MOVING);
    }

    render->primitive_cursor = (u8*)packet_cursor;
    return 0;
}
