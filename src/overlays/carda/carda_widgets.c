#include "internal/carda_widgets.h"
#include "overlays/field/field_text.h"
#include "internal/carda_internal.h"
#include "common/saved_game.h"
#include "common/gpu_packet.h"
#include "main/audio/akao.h"

/** @brief Bytes cleared when preparing the card header and icon area. */
#define CARDA_CARD_HEADER_BYTES 512

/** @brief Memory-card header icon flag: 16-color icon with two animation frames. */
#define CARDA_CARD_ICON_TWO_FRAMES 0x12

/** @brief Number of memory-card blocks a CARDA save file occupies. */
#define CARDA_SAVE_BLOCK_COUNT 2

/** @brief Number of save icons in the built-in save-icon table. */
#define CARDA_SAVE_ICON_COUNT 10

/** @brief One save icon: a 16-color CLUT followed by two 16x16 4-bit frames. */
typedef struct
{
    u16 clut[16];
    u8 frames[2][128];
} CardaSaveIcon;

/**
 * @brief Save icon @p icon in the built-in save-icon table, which holds an icon
 *        count followed by per-icon byte offsets from the table start.
 * @note g_carda_save_icon_offsets is the first offset, so the table starts one word before it.
 */
#define CARDA_SAVE_ICON(icon) ((CardaSaveIcon*)((u8*)&g_carda_save_icon_offsets - 4 + g_carda_save_icon_offsets[icon]))

/** @brief One icon in the built-in icon table: a 16-color CLUT followed by 48x48 4-bit pixels. */
typedef struct
{
    u_long clut[8];
    u_long pixels[48 * 48 / 8];
} CardaIconImage;

/**
 * @brief Icon @p icon in the built-in icon table, which holds an icon count
 *        followed by per-icon byte offsets from the table start.
 * @note g_carda_icon_image_offsets is the first offset, so the table starts one word before it.
 */
#define CARDA_ICON_IMAGE(icon) ((CardaIconImage*)((u8*)&g_carda_icon_image_offsets - 4 + g_carda_icon_image_offsets[icon]))

/** @brief One Shift-JIS character plus its terminator, copied whole into the save title. */
typedef struct
{
    s8 text[3];
} CardaSjisChar;

extern CardaSjisChar g_carda_title_dash;

extern CardaSjisChar g_carda_title_colon;

static void* carda_draw_load_progress(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
static void* carda_draw_save_progress(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
static void* carda_draw_save_complete(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
static void* carda_draw_format_progress(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
static void* carda_draw_status_dialog(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
static void* carda_draw_choice_prompt(void* prim, u_long* ot, s32 x, s32 y);

/**
 * @brief Build the save file for the live saved game in g_carda_save_blob: card
 *        header with title and icon, updated file-select summary, a copy of
 *        the saved game, and its checksum.
 * @note In overlay modes 2 and 3 (g_carda_mode) it only calls carda_store_active_record.
 */
void carda_build_save_file(void)
{
    SaveFile* blob;
    u8* cursor;
    u8* src;
    u8* title_text;
    s8* text;
    s32 i;
    s32 count;
    s32 icon;
    s32 time;
    s32 hours;
    u32 party_icon;
    s32 seed;

    if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
    {
        carda_store_active_record();
        return;
    }

    /* Header: magic, icon format and size, then clear the rest of the standard card header. */
    blob = (SaveFile*)g_carda_save_blob;
    cursor = (u8*)blob + CARDA_CARD_HEADER_BYTES - 5;
    blob->header.magic[0] = 'S';
    blob->header.magic[1] = 'C';
    blob->header.icon_flags = CARDA_CARD_ICON_TWO_FRAMES;
    blob->header.block_count = CARDA_SAVE_BLOCK_COUNT;
    for (i = CARDA_CARD_HEADER_BYTES - 5; i >= 0; i--)
    {
        cursor[4] = 0;
        cursor--;
    }

    icon = g_saved_game_ctx->control.fields.placed_land_count * 9 / 26;
    i = 0;
    if (icon >= CARDA_SAVE_ICON_COUNT)
    {
        icon = CARDA_SAVE_ICON_COUNT - 1;
    }
    src = (u8*)CARDA_SAVE_ICON(icon);
    while (i < 32)
    {
        blob->header.clut[i] = *src;
        i++;
        src++;
    }
    count = 0;
    while (count < 2)
    {
        i = 0;
        while (i < 128)
        {
            blob->header.icon_frames[count][i] = *src;
            i++;
            src++;
        }
        count++;
    }

    /* Title: template text, optional marker, save number, play time and the player's name. */
    strcpy(blob->header.title[0], g_carda_save_title_template);
    if (g_saved_game_ctx->options.bits.flag_2)
    {
        blob->header.title[0][8] = 0x81;
        blob->header.title[0][9] = 0xF4;
    }
    time = g_saved_game_ctx->play_time + VSync(-1) - g_playtime_vsync_origin;
    g_saved_game_ctx->play_time = time;
    g_playtime_vsync_origin = VSync(-1);
    text = format_decimal((s8*)&blob->header.title[0][0x12], g_card_entry_suffix_values[g_carda_selected_row]);
    *(CardaSjisChar*)text = g_carda_title_dash;
    hours = time / SAVED_PLAY_TIME_TICKS_PER_HOUR;
    text = format_decimal(text + 2, hours);
    *(CardaSjisChar*)text = g_carda_title_colon;
    time = time / SAVED_PLAY_TIME_TICKS_PER_MINUTE - hours * 60;
    text += 2;
    if (time < 10)
    {
        text = format_decimal(text, 0);
    }
    expand_text_glyph_codes((u8*)format_decimal(text, time), g_saved_game_ctx->characters[FIELD_PARTY_HERO].name);

    /* File-select summary in the live saved game. */
    g_saved_game_ctx->spawn.bits.party_icon_0 = g_saved_game_ctx->characters[FIELD_PARTY_HERO].info.bits.type;
    if (g_saved_game_ctx->characters[FIELD_PARTY_GUEST].name[0] != 0)
    {
        party_icon = g_saved_game_ctx->characters[FIELD_PARTY_GUEST].info.bits.type;
        if (party_icon < 2)
        {
            g_saved_game_ctx->track.bits.party_icon_1 = party_icon;
        }
        else
        {
            g_saved_game_ctx->track.bits.party_icon_1 = g_saved_game_ctx->characters[FIELD_PARTY_GUEST].info.bytes[1] + 2;
        }
    }
    else
    {
        g_saved_game_ctx->track.bits.party_icon_1 = SAVE_NO_ICON;
    }
    if (g_saved_game_ctx->characters[FIELD_PARTY_COMPANION].name[0] != 0)
    {
        if (g_saved_game_ctx->characters[FIELD_PARTY_COMPANION].info.bits.type == FIELD_CHARACTER_GOLEM)
        {
            g_saved_game_ctx->track.bits.party_icon_2 = g_saved_game_ctx->characters[FIELD_PARTY_COMPANION].info.bytes[1] + SAVE_ICON_GOLEM_BASE;
            g_saved_game_ctx->icon_palette = g_saved_game_ctx->golem_records[g_saved_game_ctx->joined_golem].palette;
        }
        else
        {
            g_saved_game_ctx->track.bits.party_icon_2 = g_saved_game_ctx->characters[FIELD_PARTY_COMPANION].info.bytes[1] + SAVE_ICON_PET_BASE;
        }
    }
    else
    {
        g_saved_game_ctx->track.bits.party_icon_2 = SAVE_NO_ICON;
    }
    for (i = 0; i < 21; i++)
    {
        g_saved_game_ctx->summary_name[i] = g_saved_game_ctx->characters[FIELD_PARTY_HERO].name[i];
    }
    g_saved_game_ctx->unk15 = g_saved_game_ctx->characters[FIELD_PARTY_HERO].progress.level;
    g_saved_game_ctx->unk16 = g_saved_game_ctx->characters[FIELD_PARTY_HERO].equipment[FIELD_WEAPON_SLOT].derived.bytes[0];
    count = 0;
    for (i = 0; i < 4; i++)
    {
        if (g_saved_game_ctx->summary_records[i].in_use != 0)
        {
            count++;
        }
    }
    g_saved_game_ctx->summary_slot_count = count;

    /* The copy runs 8 bytes past saved_game; the checksum and marker overwrite them. */
    bcopy((u8*)g_saved_game_ctx, (u8*)&blob->saved_game, SAVED_GAME_DATA_SIZE);
    blob->saved_game.spawn.bits.id = FIELD_SPAWN_LOAD_GAME;
    seed = rand();
    blob->saved_game.identity.ids.save_id = seed | (rand() << 15);
    blob->checksum = compute_save_checksum(blob);
    blob->magic = SAVE_FILE_MAGIC;

    /* Replace the start of the title with its final text. */
    src = blob->header.title[0];
    title_text = g_carda_bad_title_template;
    count = 0;
    while (count < 18)
    {
        count++;
        *src++ = *title_text++;
    }
}

#include "../../common/save_file/skip_hex_digits.inc.c"

/**
 * @brief Test SAVED_OPTION_FLAG_2 of the live saved game in overlay mode 0.
 * @return 1 when g_carda_mode is 0 and the flag is set, otherwise 0.
 */
s32 carda_test_option_flag_2(void)
{
    if (g_carda_mode == CARDA_MODE_SAVE && g_saved_game_ctx->options.bits.flag_2)
    {
        return 1;
    }
    return 0;
}

#include "../../common/save_file/validate_save_file.inc.c"
#include "../../common/save_file/compute_save_checksum.inc.c"

/**
 * @brief Draw the load confirmation prompt and handle its input: a card
 *        change closes the prompt, cancel or "no" returns to the card reset
 *        steps, and "yes" starts loading the selected save.
 * @param ot Ordering-table entry the text is linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 */
void* carda_draw_load_prompt(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2];
    void* result;
    s32 x;
    s32 status;
    CardaElement* prompt;

    x = -x_offset + CARDA_MESSAGE_WIDTH / 2;
    result = carda_draw_choice_prompt(field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_load_prompt, CARDA_TEXT_LOAD_PROMPT), FIELD_TEXT_COLOR_NORMAL, x,
                                                      -y_offset, FIELD_TEXT_ALIGN_CENTER),
                                      ot, x, CARDA_TEXT_LINE_HEIGHT - y_offset);

    status = poll_and_retry_card_info();
    if (status == 1 || status == 2)
    {
        g_carda_element_pool[CARDA_ELEMENT_MODAL].attr.bits.state = CARDA_ELEMENT_FREE;
        field_reset_input_repeat();
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
        g_card_entry_state = CARDA_ENTRY_STATE_CHECKING_CARD;
        carda_reset_entry_ranks();
        if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
        {
            g_card_step = g_carda_steps_initial_scan;
        }
        else
        {
            g_card_step = NULL;
        }
    }
    else
    {
        if (g_pad_input & PAD_BTN_CIRCLE)
        {
            g_carda_element_pool[CARDA_ELEMENT_MODAL].attr.bits.state = CARDA_ELEMENT_FREE;
            field_reset_input_repeat();
            field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
            g_card_step = g_carda_steps_card_reset;
        }
        else if (g_pad_input & CARDA_CONFIRM_BUTTON_MASK)
        {
            if (g_carda_choice_toggle != 0)
            {
                g_carda_element_pool[CARDA_ELEMENT_MODAL].attr.bits.state = CARDA_ELEMENT_FREE;
                field_reset_input_repeat();
                field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
                g_card_step = g_carda_steps_card_reset;
            }
            else
            {
                field_play_sound(FIELD_SOUND_SELECT, AKAO_PAN_CENTER);
                g_carda_progress_active = 1;
                g_card_step = g_carda_steps_load_selected_save;
                prompt = g_carda_element_pool;
                prompt->draw = carda_draw_load_progress;
                prompt->attr.bits.transition_step = 1;
                prompt->attr.bits.state = CARDA_ELEMENT_OPENING;
                prompt->attr.bits.x = CARDA_MESSAGE_X;
                prompt->attr.bits.y = CARDA_MESSAGE_Y;
                prompt->size.bits.width_high = 1;
                prompt->size.bits.height = CARDA_MESSAGE_HEIGHT;
                CARDA_SET_ELEMENT_WIDTH_LOW(prompt, 0x20);
            }
        }
    }
    return result;
}

/**
 * @brief Draw the "loading" message and progress bar, then commit the save
 *        file to the live saved game once the read has finished and validates.
 * @param ot Ordering-table entry the text is linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 */
static void* carda_draw_load_progress(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2];
    SaveFile* blob;
    CardaElement* element;
    CardaElement* cursor;
    void* result;
    u16* text_table;
    s32 x;
    s32 i;
    s32 valid;
    s32 checksum;

    x = -x_offset + CARDA_MESSAGE_WIDTH / 2;
    result = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_loading, CARDA_TEXT_LOADING), FIELD_TEXT_COLOR_NORMAL, x, -y_offset, FIELD_TEXT_ALIGN_CENTER);
    text_table = CARDA_TEXT_TABLE(g_carda_text_loading, CARDA_TEXT_LOADING);
    result = field_draw_text(result, ot, CARDA_TEXT(text_table, CARDA_TEXT_DO_NOT_REMOVE_CARD), FIELD_TEXT_COLOR_NORMAL, x, CARDA_TEXT_LINE_HEIGHT - y_offset,
                             FIELD_TEXT_ALIGN_CENTER);
    result = field_draw_text(result, ot, CARDA_TEXT(text_table, CARDA_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                             CARDA_TEXT_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);
    result = carda_draw_progress_bar(result, ot);

    if (g_carda_progress_active == 0)
    {
        blob = (SaveFile*)g_carda_save_blob;
        element = g_carda_element_pool;
        element->attr.bits.state = CARDA_ELEMENT_FREE;
        checksum = compute_save_checksum(blob);
        valid = 0;
        if (blob->checksum == checksum)
        {
            valid = blob->magic == SAVE_FILE_MAGIC;
        }
        if (valid == 0)
        {
            carda_open_status_dialog(CARDA_DIALOG_SAVE_CORRUPT);
            return result;
        }

        field_play_sound(FIELD_SOUND_LOAD_DONE, AKAO_PAN_CENTER);
        bcopy((u8*)&blob->saved_game, (u8*)g_saved_game_ctx, SAVED_GAME_DATA_SIZE);
        g_playtime_vsync_origin = VSync(-1);
        field_restore_fade_target();

        cursor = element;
        for (i = 0; i < CARDA_ELEMENT_COUNT; i++, cursor++)
        {
            if (cursor->attr.bits.state != CARDA_ELEMENT_FREE)
            {
                cursor->attr.bits.state = CARDA_ELEMENT_CLOSING;
                cursor->attr.bits.transition_step = CARDA_ELEMENT_TRANSITION_STEPS;
            }
        }
        field_set_fade_target(0, 0, 0, 8);
    }

    return result;
}

/**
 * @brief Emit the time-based memory-card progress bar as a gouraud-shaded quad.
 * @param quad Quad packet to fill.
 * @param ot Ordering-table entry the quad is linked into.
 * @return Primitive-buffer cursor after the quad, or @p quad unchanged while
 *         g_carda_progress_bar_active is 0.
 * @note The bar is 288 pixels wide after 256 frames since g_carda_progress_start_tick; in
 *       overlay modes 2 and 3 it runs 16 times faster for entry state 0xF4
 *       and three times slower otherwise.
 */
void* carda_draw_progress_bar(POLY_G4* quad, u_long* ot)
{
    POLY_G4* bar;
    s32 elapsed;
    s32 width;

    if (g_carda_progress_bar_active != 0)
    {
        elapsed = VSync(-1) - g_carda_progress_start_tick;
        if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
        {
            if (g_card_entry_state == CARDA_ENTRY_STATE_UPLOADING)
            {
                elapsed *= 16;
            }
            else
            {
                elapsed /= 3;
            }
        }
        if (elapsed > 256)
        {
            elapsed = 256;
        }
        SET_BGR0_PACKED(quad, GPU_COLOR_WORD(0xFF, 0, 0));
        SET_POLY_G4_BGR1_PACKED(quad, GPU_COLOR_WORD(0xFF, 0xFF, 0));
        SET_POLY_G4_BGR3_PACKED(quad, GPU_COLOR_WORD(0, 0, 0xFF));
        setlen(quad, 8);
        setcode(quad, GPU_CODE_POLY_G4);
        SET_POLY_G4_BGR2_PACKED(quad, GPU_COLOR_WORD(0, 0xFF, 0xFF));
        /* TODO: recover a structured form for the horizontal bounds. */
        do
        {
            do
            {
                bar = quad;
                quad->x2 = 0;
            } while (0);
            width = elapsed * CARDA_MESSAGE_WIDTH;
            quad->x0 = 0;
            if (width < 0)
            {
                bar = quad;
                width += 255;
            }
            quad->x3 = width >> 8;
        } while (0);
        quad->x1 = width >> 8;
        quad = bar + 1;
        bar->y1 = 0;
        bar->y0 = 0;
        bar->y3 = CARDA_TRANSFER_HEIGHT;
        bar->y2 = CARDA_TRANSFER_HEIGHT;
        addPrim(ot, bar);
    }
    return quad;
}

/**
 * @brief Draw the "Save?" prompt for a new save file and handle its input;
 *        "yes" starts writing the save file.
 * @param ot Ordering-table entry the text is linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 */
void* carda_draw_save_prompt(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2];
    void* result;
    s32 x;
    s32 status;
    CardaElement* prompt;

    x = -x_offset + CARDA_MESSAGE_WIDTH / 2;
    result = carda_draw_choice_prompt(field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_save_prompt, CARDA_TEXT_SAVE_PROMPT), FIELD_TEXT_COLOR_NORMAL, x,
                                                      -y_offset, FIELD_TEXT_ALIGN_CENTER),
                                      ot, x, CARDA_TEXT_LINE_HEIGHT - y_offset);

    status = poll_and_retry_card_info();
    if (status == 1 || status == 2)
    {
        g_carda_element_pool[CARDA_ELEMENT_MODAL].attr.bits.state = CARDA_ELEMENT_FREE;
        field_reset_input_repeat();
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
        g_card_entry_state = CARDA_ENTRY_STATE_CHECKING_CARD;
        carda_reset_entry_ranks();
        g_card_step = NULL;
    }
    else
    {
        if (g_pad_input & PAD_BTN_CIRCLE)
        {
            g_carda_element_pool[CARDA_ELEMENT_MODAL].attr.bits.state = CARDA_ELEMENT_FREE;
            field_reset_input_repeat();
            field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
            g_card_step = g_carda_steps_card_reset;
        }
        else if (g_pad_input & CARDA_CONFIRM_BUTTON_MASK)
        {
            if (g_carda_choice_toggle != 0)
            {
                g_carda_element_pool[CARDA_ELEMENT_MODAL].attr.bits.state = CARDA_ELEMENT_FREE;
                field_reset_input_repeat();
                field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
                g_card_step = g_carda_steps_card_reset;
            }
            else
            {
                field_play_sound(FIELD_SOUND_SELECT, AKAO_PAN_CENTER);
                g_carda_progress_bar_active = 0;
                g_carda_save_in_progress = 1;
                if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
                {
                    g_card_step = g_carda_steps_write_alt_save;
                }
                else
                {
                    g_card_step = g_carda_steps_write_save_keep_handles;
                }

                prompt = g_carda_element_pool;
                prompt->draw = carda_draw_save_progress;
                prompt->attr.bits.transition_step = 1;
                prompt->attr.bits.state = CARDA_ELEMENT_OPENING;
                prompt->attr.bits.x = CARDA_MESSAGE_X;
                prompt->attr.bits.y = CARDA_MESSAGE_Y;
                prompt->size.bits.width_high = 1;
                prompt->size.bits.height = CARDA_MESSAGE_HEIGHT;
                CARDA_SET_ELEMENT_WIDTH_LOW(prompt, 0x20);
            }
        }
    }
    return result;
}

/**
 * @brief Draw the "Overwrite data?" prompt for an existing save file and
 *        handle its input; "yes" starts writing the save file.
 * @param ot Ordering-table entry the text is linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 */
void* carda_draw_overwrite_prompt(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2];
    void* result;
    s32 x;
    s32 status;
    CardaElement* prompt;

    x = -x_offset + CARDA_MESSAGE_WIDTH / 2;
    result = carda_draw_choice_prompt(field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_overwrite_prompt, CARDA_TEXT_OVERWRITE_PROMPT),
                                                      FIELD_TEXT_COLOR_NORMAL, x, -y_offset, FIELD_TEXT_ALIGN_CENTER),
                                      ot, x, CARDA_TEXT_LINE_HEIGHT - y_offset);

    status = poll_and_retry_card_info();
    if (status == 1 || status == 2)
    {
        g_carda_element_pool[CARDA_ELEMENT_MODAL].attr.bits.state = CARDA_ELEMENT_FREE;
        field_reset_input_repeat();
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
        g_card_entry_state = CARDA_ENTRY_STATE_CHECKING_CARD;
        carda_reset_entry_ranks();
        g_card_step = NULL;
    }
    else
    {
        if (g_pad_input & PAD_BTN_CIRCLE)
        {
            g_carda_element_pool[CARDA_ELEMENT_MODAL].attr.bits.state = CARDA_ELEMENT_FREE;
            field_reset_input_repeat();
            field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
            g_card_step = g_carda_steps_card_reset;
        }
        else if (g_pad_input & CARDA_CONFIRM_BUTTON_MASK)
        {
            if (g_carda_choice_toggle != 0)
            {
                g_carda_element_pool[CARDA_ELEMENT_MODAL].attr.bits.state = CARDA_ELEMENT_FREE;
                field_reset_input_repeat();
                field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
                g_card_step = g_carda_steps_card_reset;
            }
            else
            {
                field_play_sound(FIELD_SOUND_SELECT, AKAO_PAN_CENTER);
                g_carda_progress_bar_active = 0;
                g_carda_save_in_progress = 1;
                if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
                {
                    g_card_step = g_carda_steps_scan_and_write_alt_save;
                }
                else
                {
                    g_card_step = g_carda_steps_write_save;
                }

                prompt = g_carda_element_pool;
                prompt->draw = carda_draw_save_progress;
                prompt->attr.bits.transition_step = 1;
                prompt->attr.bits.state = CARDA_ELEMENT_OPENING;
                prompt->attr.bits.x = CARDA_MESSAGE_X;
                prompt->attr.bits.y = CARDA_MESSAGE_Y;
                prompt->size.bits.width_high = 1;
                prompt->size.bits.height = CARDA_MESSAGE_HEIGHT;
                CARDA_SET_ELEMENT_WIDTH_LOW(prompt, 0x20);
            }
        }
    }
    return result;
}

/**
 * @brief Draw the "Now Saving..." message and progress bar; once the write
 *        has finished, replace the window with the "Saved." message.
 * @param ot Ordering-table entry the text is linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 */
static void* carda_draw_save_progress(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 x;
    void* result;
    u16* text_table;
    CardaElement* message;
    s32 unused[2];

    x = -x_offset + CARDA_MESSAGE_WIDTH / 2;
    result = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_saving, CARDA_TEXT_SAVING), FIELD_TEXT_COLOR_NORMAL, x, -y_offset, FIELD_TEXT_ALIGN_CENTER);
    text_table = CARDA_TEXT_TABLE(g_carda_text_saving, CARDA_TEXT_SAVING);
    result = field_draw_text(result, ot, CARDA_TEXT(text_table, CARDA_TEXT_DO_NOT_REMOVE_CARD), FIELD_TEXT_COLOR_NORMAL, x, CARDA_TEXT_LINE_HEIGHT - y_offset,
                             FIELD_TEXT_ALIGN_CENTER);
    result = field_draw_text(result, ot, CARDA_TEXT(text_table, CARDA_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                             CARDA_TEXT_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);
    result = carda_draw_progress_bar(result, ot);

    if (g_carda_save_in_progress == 0)
    {
        field_play_sound(FIELD_SOUND_SAVE_DONE, AKAO_PAN_CENTER);
        g_card_entry_state = CARDA_ENTRY_STATE_CHECKING_CARD;
        message = g_carda_element_pool;
        message->draw = carda_draw_save_complete;
        message->attr.bits.transition_step = 1;
        message->attr.bits.state = CARDA_ELEMENT_OPENING;
        message->attr.bits.x = CARDA_MESSAGE_X;
        message->attr.bits.y = 0x68;
        message->size.bits.width_high = 1;
        message->size.bits.height = 0x10;
        CARDA_SET_ELEMENT_WIDTH_LOW(message, 0x20);
    }
    return result;
}

/**
 * @brief Draw the "Saved." message and close it on confirm, cancel or entry
 *        a missing card.
 * @param ot Ordering-table entry the text is linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 */
static void* carda_draw_save_complete(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2];
    void* result;

    result = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_saved, CARDA_TEXT_SAVED), FIELD_TEXT_COLOR_NORMAL, -x_offset + CARDA_MESSAGE_WIDTH / 2,
                             -y_offset, FIELD_TEXT_ALIGN_CENTER);
    if (g_pad_input & (CARDA_CONFIRM_BUTTON_MASK | PAD_BTN_CIRCLE))
    {
        field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
        g_carda_element_pool[CARDA_ELEMENT_MODAL].attr.bits.state = CARDA_ELEMENT_FREE;
        field_reset_input_repeat();
    }
    else if (g_card_entry_state == CARDA_ENTRY_STATE_NO_CARD)
    {
        g_carda_element_pool[CARDA_ELEMENT_MODAL].attr.bits.state = CARDA_ELEMENT_FREE;
        field_reset_input_repeat();
    }
    return result;
}

/**
 * @brief Draw the "Format?" prompt for an unformatted memory card and handle
 *        its input; "yes" replaces the window with the formatting message.
 * @param ot Ordering-table entry the text is linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 * @note JP names the card type on the first line and puts "not formatted" on the
 *       second line for both card types.
 */
void* carda_draw_format_prompt(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2];
    s32 x;
    s32 status;
    u32 attr;
    u16* text_table;
    CardaElement* message;
    s32 y;

    y = y_offset;
    if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
    {
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_pocketstation_is, CARDA_TEXT_POCKETSTATION_IS), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_MESSAGE_WIDTH / 2, -y, FIELD_TEXT_ALIGN_CENTER);
#if !defined(VERSION_JP)
        text_table = CARDA_TEXT_TABLE(g_carda_text_pocketstation_is, CARDA_TEXT_POCKETSTATION_IS);
        prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_NOT_FORMATTED), FIELD_TEXT_COLOR_NORMAL, -x_offset + CARDA_MESSAGE_WIDTH / 2,
                               CARDA_TEXT_LINE_HEIGHT - y, FIELD_TEXT_ALIGN_CENTER);
#endif
    }
    else
    {
#if defined(VERSION_JP)
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_memory_card_is, CARDA_TEXT_MEMORY_CARD_IS), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_MESSAGE_WIDTH / 2, -y, FIELD_TEXT_ALIGN_CENTER);
#else
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_card_unformatted, CARDA_TEXT_CARD_UNFORMATTED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_MESSAGE_WIDTH / 2, -y, FIELD_TEXT_ALIGN_CENTER);
#endif
    }
    x = -x_offset + CARDA_MESSAGE_WIDTH / 2;
#if defined(VERSION_JP)
    prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_card_unformatted, CARDA_TEXT_CARD_UNFORMATTED), FIELD_TEXT_COLOR_NORMAL, x,
                           CARDA_TEXT_LINE_HEIGHT - y, FIELD_TEXT_ALIGN_CENTER);
    text_table = CARDA_TEXT_TABLE(g_carda_text_card_unformatted, CARDA_TEXT_CARD_UNFORMATTED);
    prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_FORMAT_PROMPT), FIELD_TEXT_COLOR_NORMAL, x,
                           CARDA_TEXT_LINE_HEIGHT * 2 - y, FIELD_TEXT_ALIGN_CENTER);
#else
    prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_format_prompt, CARDA_TEXT_FORMAT_PROMPT), FIELD_TEXT_COLOR_NORMAL, x,
                           CARDA_TEXT_LINE_HEIGHT * 2 - y, FIELD_TEXT_ALIGN_CENTER);
#endif
    prim = carda_draw_choice_prompt(prim, ot, x, CARDA_TEXT_LINE_HEIGHT * 3 - y);

    status = poll_and_retry_card_info();
    if (status == 1 || status == 2)
    {
        field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
        g_carda_element_pool[CARDA_ELEMENT_MODAL].attr.bits.state = CARDA_ELEMENT_FREE;
        field_reset_input_repeat();
        g_card_entry_state = CARDA_ENTRY_STATE_CHECKING_CARD;
        carda_reset_entry_ranks();
        if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
        {
            g_card_step = g_carda_steps_card_reset;
        }
        else
        {
            g_card_step = NULL;
        }
    }
    else
    {
        status = g_pad_input;
        if ((status & PAD_BTN_CIRCLE) || ((status & CARDA_CONFIRM_BUTTON_MASK) && g_carda_choice_toggle != 0))
        {
            g_carda_format_declined = 1;
            field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
            g_carda_element_pool[CARDA_ELEMENT_MODAL].attr.bits.state = CARDA_ELEMENT_FREE;
            field_reset_input_repeat();
            g_card_entry_state = CARDA_ENTRY_STATE_CHECKING_CARD;
            carda_reset_entry_ranks();
            g_card_entry_state = CARDA_ENTRY_STATE_UNFORMATTED;
            if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
            {
                g_card_step = NULL;
            }
            else
            {
                g_card_step = g_carda_steps_card_reset;
            }
        }
        else if (status & CARDA_CONFIRM_BUTTON_MASK)
        {
            field_play_sound(FIELD_SOUND_SELECT, AKAO_PAN_CENTER);
            g_carda_element_pool[CARDA_ELEMENT_MODAL].attr.bits.state = CARDA_ELEMENT_FREE;
            carda_reset_entry_ranks();
            g_carda_format_frames = 0;
            message = g_carda_element_pool;
            message->attr.bits.transition_step = 1;
            message->attr.bits.state = CARDA_ELEMENT_OPENING;
            if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
            {
                message->attr.bits.x = CARDA_MESSAGE_X;
                message->attr.bits.y = CARDA_TRANSFER_Y;
                attr = message->attr.word;
                attr &= (1 << CARDA_ELEMENT_WIDTH_SHIFT) - 1;
                attr |= 0x20 << CARDA_ELEMENT_WIDTH_SHIFT;
                message->attr.word = attr;
                message->size.bits.width_high = 1;
                message->size.bits.height = CARDA_TRANSFER_HEIGHT;
            }
            else
            {
                message->attr.bits.x = CARDA_MESSAGE_X;
                message->attr.bits.y = CARDA_MESSAGE_Y;
                attr = message->attr.word;
                attr &= (1 << CARDA_ELEMENT_WIDTH_SHIFT) - 1;
                attr |= 0x20 << CARDA_ELEMENT_WIDTH_SHIFT;
                message->attr.word = attr;
                message->size.bits.width_high = 1;
                message->size.bits.height = CARDA_MESSAGE_HEIGHT;
            }
            message->draw = carda_draw_format_progress;
        }
    }
    return prim;
}

/**
 * @brief Draw the formatting message (or, once done, the saving message);
 *        on frame 12 format the card, on frame 13 build the save file and
 *        switch to the saving window.
 * @param ot Ordering table used for the text primitives.
 * @param prim Current primitive packet cursor.
 * @param x_offset Horizontal placement offset.
 * @param y_offset Vertical placement offset.
 * @return Updated primitive packet cursor.
 */
static void* carda_draw_format_progress(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2];

    if (g_carda_format_frames >= 13)
    {
        if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
        {
            s32 x;
            u16* text_table;

            x = -x_offset + CARDA_MESSAGE_WIDTH / 2;
            prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_downloading, CARDA_TEXT_DOWNLOADING), FIELD_TEXT_COLOR_NORMAL, x, -y_offset,
                                   FIELD_TEXT_ALIGN_CENTER);
            text_table = CARDA_TEXT_TABLE(g_carda_text_downloading, CARDA_TEXT_DOWNLOADING);
            prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_POCKETSTATION_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                                   CARDA_TEXT_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
            prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                                   CARDA_TEXT_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);
        }
        else
        {
            s32 x;
            u16* text_table;

            x = -x_offset + CARDA_MESSAGE_WIDTH / 2;
            prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_saving, CARDA_TEXT_SAVING), FIELD_TEXT_COLOR_NORMAL, x, -y_offset,
                                   FIELD_TEXT_ALIGN_CENTER);
            text_table = CARDA_TEXT_TABLE(g_carda_text_saving, CARDA_TEXT_SAVING);
            prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_DO_NOT_REMOVE_CARD), FIELD_TEXT_COLOR_NORMAL, x,
                                   CARDA_TEXT_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
            prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                                   CARDA_TEXT_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);
        }
    }
    else
    {
        s32 x;
        u16* text_table;

        x = -x_offset + CARDA_MESSAGE_WIDTH / 2;
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_formatting, CARDA_TEXT_FORMATTING), FIELD_TEXT_COLOR_NORMAL, x, -y_offset,
                               FIELD_TEXT_ALIGN_CENTER);
        text_table = CARDA_TEXT_TABLE(g_carda_text_formatting, CARDA_TEXT_FORMATTING);
        if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
        {
            prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_POCKETSTATION_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                                   CARDA_TEXT_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
            prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                                   CARDA_TEXT_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);
        }
        else
        {
            prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_DO_NOT_REMOVE_CARD), FIELD_TEXT_COLOR_NORMAL, x,
                                   CARDA_TEXT_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
            prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                                   CARDA_TEXT_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);
        }
    }

    if (g_carda_format_frames == 12)
    {
        carda_reset_to_new_save_entry();
        g_card_step = g_carda_steps_card_check;
    }
    else if (g_carda_format_frames >= 13)
    {
        g_carda_selected_row = 0;
        g_card_entry_suffix_values[0] = 1;
        carda_build_save_file();
        g_carda_progress_bar_active = 0;
        g_carda_save_in_progress = 1;
        if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
        {
            g_carda_received_item_count = 0;
            g_gosub_result_values = 5;
            g_carda_new_save_file = 1;
            carda_store_active_record();
            g_card_step = g_carda_steps_write_alt_save;
            g_card_entry_state = CARDA_ENTRY_STATE_DOWNLOADING;
            g_carda_element_pool[CARDA_ELEMENT_MODAL].attr.bits.state = CARDA_ELEMENT_FREE;
        }
        else
        {
            CardaElement* message;

            g_card_step = g_carda_steps_write_save_keep_handles;
            message = g_carda_element_pool;
            message->draw = carda_draw_save_progress;
            message->attr.bits.transition_step = 1;
            message->attr.bits.state = CARDA_ELEMENT_OPEN;
            message->attr.bits.x = CARDA_MESSAGE_X;
            message->attr.bits.y = CARDA_MESSAGE_Y;
            message->size.bits.width_high = 1;
            message->size.bits.height = CARDA_MESSAGE_HEIGHT;
            CARDA_SET_ELEMENT_WIDTH_LOW(message, 0x20);
        }
    }
    g_carda_format_frames += 1;
    return prim;
}

/**
 * @brief Open the status dialog in the first element slot and abandon any
 *        card operation in progress; in overlay modes 2 and 3 only the entry
 *        state is set.
 * @param dialog_state Message to show, from the CARDA_DIALOG values.
 */
void carda_open_status_dialog(s32 dialog_state)
{
    s32 state;
    CardaElement* dialog;

    if (g_carda_dialog_state != dialog_state || g_carda_element_pool[CARDA_ELEMENT_MODAL].attr.bits.state == CARDA_ELEMENT_FREE ||
        g_carda_element_pool[CARDA_ELEMENT_MODAL].draw != carda_draw_status_dialog)
    {
        g_carda_save_in_progress = 0;
        g_carda_progress_active = 0;
        g_carda_selection_status = CARDA_SELECTION_NONE;
        g_carda_io_busy = 0;
        carda_reset_entry_ranks();
        g_carda_dialog_state = dialog_state;
        _card_wait(g_card_slot);
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);

        if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
        {
            state = g_carda_dialog_state;
            switch (state)
            {
            case CARDA_DIALOG_SAVE_FAILED:
                g_card_entry_state = CARDA_ENTRY_STATE_DOWNLOAD_FAILED;
                break;
            case CARDA_DIALOG_LOAD_FAILED:
                g_card_entry_state = CARDA_ENTRY_STATE_UPLOAD_FAILED;
                break;
            case CARDA_DIALOG_CARD_NOT_INSERTED:
                g_card_entry_state = CARDA_ENTRY_STATE_POCKETSTATION_NOT_INSERTED;
                break;
            case CARDA_DIALOG_NOT_POCKETSTATION:
                g_card_entry_state = CARDA_ENTRY_STATE_NO_POCKETSTATION;
                break;
            case CARDA_DIALOG_SAVE_CORRUPT:
                g_card_entry_state = CARDA_ENTRY_STATE_SAVE_CORRUPT;
                break;
            case CARDA_DIALOG_FORMAT_FAILED:
                g_card_entry_state = CARDA_ENTRY_STATE_FORMAT_FAILED;
                break;
            }
            g_card_step = NULL;
            return;
        }

        dialog = g_carda_element_pool;
        dialog->attr.bits.transition_step = 1;
        dialog->attr.bits.state = CARDA_ELEMENT_OPENING;
        dialog->attr.bits.x = 0x20;
        dialog->attr.word &= 0x00FFFFFF;
        dialog->size.bits.width_high = 1;
        if (dialog_state < 2 || dialog_state == 4 || dialog_state == 5)
        {
            dialog->attr.bits.y = 0x60;
            dialog->size.bits.height = 0x24;
        }
        else
        {
            dialog->size.bits.height = 0x14;
            dialog->attr.bits.y = 0x70;
        }
        dialog->draw = carda_draw_status_dialog;
        field_reset_input_repeat();
        g_carda_save_in_progress = 0;
        g_carda_progress_active = 0;
        g_carda_selection_status = CARDA_SELECTION_NONE;
        g_carda_io_busy = 0;
        g_card_entry_state = CARDA_ENTRY_STATE_CHECKING_CARD;
        carda_reset_entry_ranks();
        g_card_step = g_carda_steps_initial_scan;
        g_carda_dialog_state = dialog_state;
        _card_wait(g_card_slot);
    }
}

/**
 * @brief Draw the active CARDA status dialog and handle dismissal input.
 * @param ot Ordering table used by the text renderer.
 * @param prim Current primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 */
static void* carda_draw_status_dialog(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2];
    u16* text_table;

    switch (g_carda_dialog_state)
    {
    case CARDA_DIALOG_SAVE_FAILED:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_save_failed, CARDA_TEXT_SAVE_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_ITEM_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        text_table = CARDA_TEXT_TABLE(g_carda_text_save_failed, CARDA_TEXT_SAVE_FAILED);
        prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_CHECK_CARD_INSERTED), FIELD_TEXT_COLOR_NORMAL, -x_offset + CARDA_ITEM_LIST_WIDTH / 2,
                               -y_offset + 0x10, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARDA_DIALOG_LOAD_FAILED:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_load_failed, CARDA_TEXT_LOAD_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_ITEM_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        text_table = CARDA_TEXT_TABLE(g_carda_text_load_failed, CARDA_TEXT_LOAD_FAILED);
        prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_CHECK_CARD_INSERTED), FIELD_TEXT_COLOR_NORMAL, -x_offset + CARDA_ITEM_LIST_WIDTH / 2,
                               -y_offset + 0x10, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARDA_DIALOG_CARD_NOT_INSERTED:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_card_not_inserted, CARDA_TEXT_CARD_NOT_INSERTED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_ITEM_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARDA_DIALOG_NOT_POCKETSTATION:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_not_pocketstation, CARDA_TEXT_NOT_POCKETSTATION), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_ITEM_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        if (g_card_entry_state == CARDA_ENTRY_STATE_NO_CARD)
        {
            g_carda_element_pool[CARDA_ELEMENT_MODAL].attr.bits.state = CARDA_ELEMENT_FREE;
            field_reset_input_repeat();
            return prim;
        }
        break;
    case CARDA_DIALOG_SAVE_CORRUPT:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_load_failed, CARDA_TEXT_LOAD_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_ITEM_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        text_table = CARDA_TEXT_TABLE(g_carda_text_load_failed, CARDA_TEXT_LOAD_FAILED);
        prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_SAVE_CORRUPT), FIELD_TEXT_COLOR_NORMAL, -x_offset + CARDA_ITEM_LIST_WIDTH / 2,
                               -y_offset + 0x10, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARDA_DIALOG_FORMAT_FAILED:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_format_failed, CARDA_TEXT_FORMAT_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_ITEM_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        text_table = CARDA_TEXT_TABLE(g_carda_text_format_failed, CARDA_TEXT_FORMAT_FAILED);
        prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_CHECK_CARD_INSERTED), FIELD_TEXT_COLOR_NORMAL, -x_offset + CARDA_ITEM_LIST_WIDTH / 2,
                               -y_offset + 0x10, FIELD_TEXT_ALIGN_CENTER);
        break;
    }

    if (g_pad_input & CARDA_CONFIRM_BUTTON_MASK)
    {
        switch (g_carda_mode)
        {
        case 2:
            g_field_card_overlay_mode = CARDA_RESULT_SEND_PET_CANCELLED;
            break;
        case 3:
            g_field_card_overlay_mode = CARDA_RESULT_RETURN_PET_CANCELLED;
            break;
        default:
            g_field_card_overlay_mode = CARDA_RESULT_CANCELLED;
            break;
        }
        g_carda_element_pool[CARDA_ELEMENT_MODAL].attr.bits.state = CARDA_ELEMENT_FREE;
        field_reset_input_repeat();
    }
    return prim;
}

/**
 * @brief Upload one save icon's CLUT and pixels to VRAM slot @p index and draw
 *        it as a 48x48 textured quad.
 * @param quad Quad packet to fill.
 * @param ot Ordering-table entry the quad is linked into.
 * @param x Quad left edge.
 * @param y Quad top edge.
 * @param width Quad width.
 * @param icon Icon id; SAVE_NO_ICON draws nothing, ids below 2 in row 1 and ids from
 *             0x4F up get a generated CLUT (field_copy_portrait_palette / field_copy_golem_portrait_palette).
 * @param index VRAM icon slot; selects the CLUT row entry and the texture column.
 * @param row Entry row; row 1 uses the generated CLUT for icons 0 and 1.
 * @return Primitive-buffer cursor after the quad, or @p quad unchanged for SAVE_NO_ICON.
 */
void* carda_draw_icon_highlight(POLY_FT4* quad, u_long* ot, s32 x, s32 y, s32 width, s32 icon, s32 index, s32 row)
{
    RECT rect;
    s32 column;
    u8 u;

    if (icon == SAVE_NO_ICON)
    {
        return quad;
    }

    setRECT(&rect, index * 16, 498, 16, 1);
    if ((row == 1) && (icon < 2))
    {
        field_copy_portrait_palette(g_carda_icon_context, icon);
        LoadImage(&rect, g_carda_icon_context);
        DrawSync(0);
    }
    else if (icon >= SAVE_ICON_GOLEM_BASE)
    {
        field_copy_golem_portrait_palette(g_carda_icon_context, g_carda_icon_palette);
        LoadImage(&rect, g_carda_icon_context);
        DrawSync(0);
    }
    else
    {
        LoadImage(&rect, CARDA_ICON_IMAGE(icon)->clut);
    }

    column = index * 3;
    setRECT(&rect, column * 4 + 320, 208, 12, 48);
    LoadImage(&rect, CARDA_ICON_IMAGE(icon)->pixels);

    SET_BGR0_PACKED(quad, GPU_TINT_NEUTRAL);
    setPolyFT4(quad);
    quad->x2 = x;
    quad->x0 = x;
    quad->y1 = y;
    quad->y0 = y;
    quad->x3 = x + width;
    u = column * 16;
    quad->u2 = u;
    quad->u0 = u;
    u += 47;
    quad->u3 = u;
    quad->u1 = u;
    quad->v1 = 208;
    quad->v0 = 208;
    quad->x1 = x + width;
    quad->y3 = y + 47;
    quad->y2 = y + 47;
    quad->v3 = 255;
    quad->v2 = 255;
    quad->clut = getClut(index * 16, 498);
    quad->tpage = getTPage(GPU_TEXTURE_4BIT, GPU_BLEND_HALF, 320, 0);
    addPrim(ot, quad);

    return quad + 1;
}

/**
 * @brief Preselect the default choice (CARDA_CHOICE_DEFAULT) of the two-choice prompt.
 */
void carda_enable_choice_toggle(void)
{
    g_carda_choice_toggle = CARDA_CHOICE_DEFAULT;
}

/**
 * @brief Draw the two choices of a yes/no prompt from the FIELD UI string table,
 *        highlighting the selected one, and toggle the selection on left/right.
 * @param prim Primitive-buffer cursor.
 * @param ot Ordering-table entry the text is linked into.
 * @param x Prompt center; the choices are drawn at x - 16 and x + 8.
 * @param y Prompt baseline.
 * @return Advanced primitive-buffer cursor.
 */
static void* carda_draw_choice_prompt(void* prim, u_long* ot, s32 x, s32 y)
{
    u8* text_table;
    u8* text;
    s32 color;

    color = FIELD_TEXT_COLOR_NORMAL;
    text = FIELD_UI_TEXT_AT(&g_text_choice_glyph_offsets, FIELD_UI_TEXT_YES);
    text_table = &g_text_choice_glyph_offsets - FIELD_UI_TEXT_YES * 2;
    if (g_carda_choice_toggle != 0)
    {
        color = FIELD_TEXT_COLOR_DIM;
    }
    prim = field_draw_text(prim, ot, text, color, x - 16, y, FIELD_TEXT_ALIGN_RIGHT);

    color = FIELD_TEXT_COLOR_NORMAL;
    text = FIELD_UI_TEXT(text_table, FIELD_UI_TEXT_NO);
    if (g_carda_choice_toggle == 0)
    {
        color = FIELD_TEXT_COLOR_DIM;
    }
    prim = field_draw_text(prim, ot, text, color, x + 8, y, FIELD_TEXT_ALIGN_LEFT);

    if (g_pad_input & CARDA_CHOICE_BUTTON_MASK)
    {
        g_carda_choice_toggle ^= 1;
        field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
        g_pad_input = 0;
    }
    return prim;
}
