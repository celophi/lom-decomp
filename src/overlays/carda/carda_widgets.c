#include "carda_internal.h"
#include "saved_game.h"

/** @brief Byte size of a standard memory-card file header (title frame plus three icon frames). */
#define CARDA_CARD_HEADER_BYTES 0x200

/** @brief Memory-card header icon flag: 16-color icon with two animation frames. */
#define CARDA_CARD_ICON_TWO_FRAMES 0x12

/** @brief Number of memory-card blocks a CARDA save file occupies. */
#define CARDA_SAVE_BLOCK_COUNT 2

/** @brief Number of save icons in the built-in save-icon table. */
#define CARDA_SAVE_ICON_COUNT 10

/** @brief First icon id of the golem icons, drawn with a generated CLUT (see carda_draw_icon_highlight). */
#define CARDA_GOLEM_ICON_BASE 0x4F

/** @brief First save-icon id of the pet icons. */
#define CARDA_PET_ICON_BASE 0x0E

/** @brief Play-time ticks (1/60 s) per minute and per hour. */
#define CARDA_TICKS_PER_MINUTE 3600
#define CARDA_TICKS_PER_HOUR 216000

/** @brief One save icon: a 16-color CLUT followed by two 16x16 4-bit frames. */
typedef struct
{
    u16 clut[16];
    u8 frames[2][0x80];
} CardaSaveIcon;

/**
 * @brief Save icon @p icon in the built-in save-icon table, which holds an icon
 *        count followed by per-icon byte offsets from the table start.
 * @note D_8014BF00 is the first offset, so the table starts one word before it.
 */
#define CARDA_SAVE_ICON(icon) ((CardaSaveIcon*)((u8*)&D_8014BF00 - 4 + D_8014BF00[icon]))

/** @brief One icon in the built-in icon table: a 16-color CLUT followed by 48x48 4-bit pixels. */
typedef struct
{
    u16 clut[16];
    u8 pixels[48 * 48 / 2];
} CardaIconImage;

/**
 * @brief Icon @p icon in the built-in icon table, which holds an icon count
 *        followed by per-icon byte offsets from the table start.
 * @note D_8014CC54 is the first offset, so the table starts one word before it.
 */
#define CARDA_ICON_IMAGE(icon) ((CardaIconImage*)((u8*)&D_8014CC54 - 4 + D_8014CC54[icon]))

/* Only referenced here; its blob type is private to this file. */
s32 carda_validate_save_file(SaveFile* file);

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

    if (g_carda_mode == 2 || g_carda_mode == 3)
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
    do
    {
        blob->header.clut[i] = *src;
        i++;
        src++;
    } while (i < 0x20);
    count = 0;
    do
    {
        i = 0;
        do
        {
            blob->header.icon_frames[count][i] = *src;
            i++;
            src++;
        } while (i < 0x80);
        count++;
    } while (count < 2);

    /* Title: template text, optional marker, save number, play time and the player's name. */
    strcpy(blob->header.title[0], &g_carda_save_title_template);
    if (g_saved_game_ctx->options.bits.flag_2)
    {
        blob->header.title[0][8] = 0x81;
        blob->header.title[0][9] = 0xF4;
    }
    time = g_saved_game_ctx->play_time + VSync(-1) - g_playtime_vsync_origin;
    g_saved_game_ctx->play_time = time;
    g_playtime_vsync_origin = VSync(-1);
    text = carda_format_decimal((s8*)&blob->header.title[0][0x12], g_carda_entry_suffix_values[g_carda_selected_row]);
    *(CardaSjisChar*)text = D_8014008C;
    hours = time / CARDA_TICKS_PER_HOUR;
    text = carda_format_decimal(text + 2, hours);
    *(CardaSjisChar*)text = g_niki_file_template;
    time = time / CARDA_TICKS_PER_MINUTE - hours * 60;
    text += 2;
    if (time < 10)
    {
        text = carda_format_decimal(text, 0);
    }
    carda_expand_text_glyph_codes((u8*)carda_format_decimal(text, time), g_saved_game_ctx->characters[FIELD_PARTY_HERO].name);

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
            g_saved_game_ctx->track.bits.party_icon_2 = g_saved_game_ctx->characters[FIELD_PARTY_COMPANION].info.bytes[1] + CARDA_GOLEM_ICON_BASE;
            g_saved_game_ctx->icon_palette = g_saved_game_ctx->golem_records[g_saved_game_ctx->joined_golem].palette;
        }
        else
        {
            g_saved_game_ctx->track.bits.party_icon_2 = g_saved_game_ctx->characters[FIELD_PARTY_COMPANION].info.bytes[1] + CARDA_PET_ICON_BASE;
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
    blob->checksum = carda_compute_save_checksum(blob);
    blob->magic = SAVE_FILE_MAGIC;

    /* Replace the start of the title with its final text. */
    src = blob->header.title[0];
    title_text = D_8014BEE4;
    count = 0;
    do
    {
        count++;
        *src = *title_text;
        title_text++;
        src++;
    } while (count < 18);
}

/**
 * @brief Advance a pointer past a run of hex-digit characters ('0'-'9',
 *        'a'-'f', 'A'-'F').
 * @param text Pointer to the first character to test.
 * @return Pointer to the first character that is not a hex digit.
 */
u8* carda_skip_hex_digits(u8* text)
{
    while ((*text >= '0' && *text <= '9') || (*text >= 'a' && *text <= 'f') || (*text >= 'A' && *text <= 'F'))
    {
        text++;
    }
    return text;
}

/**
 * @brief Test SAVED_OPTION_FLAG_2 of the live saved game in overlay mode 0.
 * @return 1 when g_carda_mode is 0 and the flag is set, otherwise 0.
 */
s32 carda_test_option_flag_2(void)
{
    if (g_carda_mode == 0 && g_saved_game_ctx->options.bits.flag_2)
    {
        return 1;
    }
    return 0;
}

/**
 * @brief Validate a save blob against its trailing checksum and magic.
 * @param file Save file; its first SAVE_FILE_CHECKSUM_BYTES bytes are summed by carda_compute_save_checksum.
 * @return 1 if the stored checksum matches and the magic equals SAVE_FILE_MAGIC, otherwise 0.
 */
s32 carda_validate_save_file(SaveFile* file)
{
    if (file->checksum == carda_compute_save_checksum(file))
    {
        if (file->magic == SAVE_FILE_MAGIC)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Compute the additive checksum of a save payload.
 * @param data Start of the save file; SAVE_FILE_CHECKSUM_BYTES bytes are summed.
 * @return Twice the byte sum plus SAVE_FILE_CHECKSUM_BIAS.
 */
s32 carda_compute_save_checksum(void* data)
{
    s32 sum;
    u32 byte_index;
    u8* cursor;

    cursor = data;
    sum = 0;
    byte_index = 0;
    do
    {
        byte_index++;
        sum += *cursor;
        cursor++;
    } while (byte_index < SAVE_FILE_CHECKSUM_BYTES);
    return sum * 2 + SAVE_FILE_CHECKSUM_BIAS;
}

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
    s32 unused[2]; /* never used, but the original stack frame reserves it */
    void* result;
    s32 x;
    s32 status;
    CardaElement* prompt;

    x = -x_offset + 0x90;
    result = carda_draw_choice_prompt(func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B068, 24), 4, x, -y_offset, 2), ot, x, 0xE - y_offset);

    status = carda_poll_and_rewind_primary_handles();
    if (status == 1 || status == 2)
    {
        g_carda_element_pool[0].attr.f.state = CARDA_ELEMENT_FREE;
        field_reset_input_repeat();
        play_menu_sfx(0x78, 0x80);
        g_carda_entry_state = 0xFF;
        carda_reset_entry_ranks();
        if (g_carda_mode == 2 || g_carda_mode == 3)
        {
            g_carda_save_step = g_carda_steps_initial_scan;
        }
        else
        {
            g_carda_save_step = 0;
        }
    }
    else
    {
        if (g_pad_input & 0x40)
        {
            g_carda_element_pool[0].attr.f.state = CARDA_ELEMENT_FREE;
            field_reset_input_repeat();
            play_menu_sfx(0x78, 0x80);
            g_carda_save_step = g_carda_steps_card_reset;
        }
        else if (g_pad_input & 0x220)
        {
            if (g_carda_choice_toggle != 0)
            {
                g_carda_element_pool[0].attr.f.state = CARDA_ELEMENT_FREE;
                field_reset_input_repeat();
                play_menu_sfx(0x78, 0x80);
                g_carda_save_step = g_carda_steps_card_reset;
            }
            else
            {
                play_menu_sfx(0x7E, 0x80);
                g_carda_progress_active = 1;
                g_carda_save_step = g_carda_steps_load_selected_save;
                prompt = g_carda_element_pool;
                prompt->draw = carda_draw_load_progress;
                prompt->attr.f.phase = 1;
                prompt->attr.f.state = CARDA_ELEMENT_OPENING;
                prompt->attr.f.x = 0x10;
                prompt->attr.f.y = 0x5A;
                prompt->size.f.width_high = 1;
                prompt->size.f.height = 0x2C;
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
void* carda_draw_load_progress(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2]; /* never used, but the original stack frame reserves it */
    SaveFile* blob;
    CardaElement* element;
    CardaElement* cursor;
    void* result;
    u16* text_table;
    s32 x;
    s32 i;
    s32 valid;
    s32 checksum;

    x = -x_offset + 0x90;
    result = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B06A, 25), 4, x, -y_offset, 2);
    text_table = CARDA_TEXT_TABLE(D_8014B06A, 25);
    result = func_800A88A0(result, ot, CARDA_TEXT(text_table, 15), 4, x, 0xE - y_offset, 2);
    result = func_800A88A0(result, ot, CARDA_TEXT(text_table, 89), 4, x, 0x1C - y_offset, 2);
    result = carda_draw_progress_bar(result, ot);

    if (g_carda_progress_active == 0)
    {
        blob = (SaveFile*)g_carda_save_blob;
        element = g_carda_element_pool;
        element->attr.f.state = CARDA_ELEMENT_FREE;
        checksum = carda_compute_save_checksum(blob);
        valid = 0;
        if (blob->checksum == checksum)
        {
            valid = blob->magic == SAVE_FILE_MAGIC;
        }
        if (valid == 0)
        {
            carda_open_status_dialog(4);
            return result;
        }

        play_menu_sfx(0x7B, 0x80);
        bcopy((u8*)&blob->saved_game, (u8*)g_saved_game_ctx, SAVED_GAME_DATA_SIZE);
        g_playtime_vsync_origin = VSync(-1);
        field_restore_fade_target();

        cursor = element;
        for (i = 0; i < CARDA_ELEMENT_COUNT; i++, cursor++)
        {
            if (cursor->attr.f.state != CARDA_ELEMENT_FREE)
            {
                cursor->attr.f.state = CARDA_ELEMENT_CLOSING;
                cursor->attr.f.phase = CARDA_ELEMENT_PHASE_STEPS;
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
 * @see matching: 100.00%
 */
void* carda_draw_progress_bar(POLY_G4* quad, u_long* ot)
{
    POLY_G4* bar;
    s32 elapsed;
    s32 width;

    if (g_carda_progress_bar_active != 0)
    {
        elapsed = VSync(-1) - g_carda_progress_start_tick;
        if (g_carda_mode == 2 || g_carda_mode == 3)
        {
            if (g_carda_entry_state == 0xF4)
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
        setcode(quad, 0x38);
        SET_POLY_G4_BGR2_PACKED(quad, GPU_COLOR_WORD(0, 0xFF, 0xFF));
        /*
         * TODO: these single-iteration scopes stand in for an unknown source
         * shape; their loop notes act as scheduling barriers around the x stores.
         */
        do
        {
            do
            {
                bar = quad;
                quad->x2 = 0;
            } while (0);
            width = elapsed * 288;
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
        bar->y3 = 72;
        bar->y2 = 72;
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
    s32 unused[2]; /* never used, but the original stack frame reserves it */
    void* result;
    s32 x;
    s32 status;
    CardaElement* prompt;

    x = -x_offset + 0x90;
    result = carda_draw_choice_prompt(func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B04E, 11), 4, x, -y_offset, 2), ot, x, 0xE - y_offset);

    status = carda_poll_and_rewind_primary_handles();
    if (status == 1 || status == 2)
    {
        g_carda_element_pool[0].attr.f.state = CARDA_ELEMENT_FREE;
        field_reset_input_repeat();
        play_menu_sfx(0x78, 0x80);
        g_carda_entry_state = 0xFF;
        carda_reset_entry_ranks();
        g_carda_save_step = 0;
    }
    else
    {
        if (g_pad_input & 0x40)
        {
            g_carda_element_pool[0].attr.f.state = CARDA_ELEMENT_FREE;
            field_reset_input_repeat();
            play_menu_sfx(0x78, 0x80);
            g_carda_save_step = g_carda_steps_card_reset;
        }
        else if (g_pad_input & 0x220)
        {
            if (g_carda_choice_toggle != 0)
            {
                g_carda_element_pool[0].attr.f.state = CARDA_ELEMENT_FREE;
                field_reset_input_repeat();
                play_menu_sfx(0x78, 0x80);
                g_carda_save_step = g_carda_steps_card_reset;
            }
            else
            {
                play_menu_sfx(0x7E, 0x80);
                g_carda_progress_bar_active = 0;
                g_carda_save_in_progress = 1;
                if (g_carda_mode == 2 || g_carda_mode == 3)
                {
                    g_carda_save_step = g_carda_steps_write_alt_save;
                }
                else
                {
                    g_carda_save_step = g_carda_steps_write_save_keep_handles;
                }

                prompt = g_carda_element_pool;
                prompt->draw = carda_draw_save_progress;
                prompt->attr.f.phase = 1;
                prompt->attr.f.state = CARDA_ELEMENT_OPENING;
                prompt->attr.f.x = 0x10;
                prompt->attr.f.y = 0x5A;
                prompt->size.f.width_high = 1;
                prompt->size.f.height = 0x2C;
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
    s32 unused[2]; /* never used, but the original stack frame reserves it */
    void* result;
    s32 x;
    s32 status;
    CardaElement* prompt;

    x = -x_offset + 0x90;
    result = carda_draw_choice_prompt(func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B050, 12), 4, x, -y_offset, 2), ot, x, 0xE - y_offset);

    status = carda_poll_and_rewind_primary_handles();
    if (status == 1 || status == 2)
    {
        g_carda_element_pool[0].attr.f.state = CARDA_ELEMENT_FREE;
        field_reset_input_repeat();
        play_menu_sfx(0x78, 0x80);
        g_carda_entry_state = 0xFF;
        carda_reset_entry_ranks();
        g_carda_save_step = 0;
    }
    else
    {
        if (g_pad_input & 0x40)
        {
            g_carda_element_pool[0].attr.f.state = CARDA_ELEMENT_FREE;
            field_reset_input_repeat();
            play_menu_sfx(0x78, 0x80);
            g_carda_save_step = g_carda_steps_card_reset;
        }
        else if (g_pad_input & 0x220)
        {
            if (g_carda_choice_toggle != 0)
            {
                g_carda_element_pool[0].attr.f.state = CARDA_ELEMENT_FREE;
                field_reset_input_repeat();
                play_menu_sfx(0x78, 0x80);
                g_carda_save_step = g_carda_steps_card_reset;
            }
            else
            {
                play_menu_sfx(0x7E, 0x80);
                g_carda_progress_bar_active = 0;
                g_carda_save_in_progress = 1;
                if (g_carda_mode == 2 || g_carda_mode == 3)
                {
                    g_carda_save_step = g_carda_steps_scan_and_write_alt_save;
                }
                else
                {
                    g_carda_save_step = g_carda_steps_write_save;
                }

                prompt = g_carda_element_pool;
                prompt->draw = carda_draw_save_progress;
                prompt->attr.f.phase = 1;
                prompt->attr.f.state = CARDA_ELEMENT_OPENING;
                prompt->attr.f.x = 0x10;
                prompt->attr.f.y = 0x5A;
                prompt->size.f.width_high = 1;
                prompt->size.f.height = 0x2C;
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
void* carda_draw_save_progress(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 x;
    void* result;
    u16* text_table;
    CardaElement* message;
    s32 unused[2]; /* never used, but the original stack frame reserves it */

    x = -x_offset + 0x90;
    result = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B054, 14), 4, x, -y_offset, 2);
    text_table = CARDA_TEXT_TABLE(D_8014B054, 14);
    result = func_800A88A0(result, ot, CARDA_TEXT(text_table, 15), 4, x, 0xE - y_offset, 2);
    result = func_800A88A0(result, ot, CARDA_TEXT(text_table, 89), 4, x, 0x1C - y_offset, 2);
    result = carda_draw_progress_bar(result, ot);

    if (g_carda_save_in_progress == 0)
    {
        play_menu_sfx(0x7A, 0x80);
        g_carda_entry_state = 0xFF;
        message = g_carda_element_pool;
        message->draw = carda_draw_save_complete;
        message->attr.f.phase = 1;
        message->attr.f.state = CARDA_ELEMENT_OPENING;
        message->attr.f.x = 0x10;
        message->attr.f.y = 0x68;
        message->size.f.width_high = 1;
        message->size.f.height = 0x10;
        CARDA_SET_ELEMENT_WIDTH_LOW(message, 0x20);
    }
    return result;
}

/**
 * @brief Draw the "Saved." message and close it on confirm, cancel or entry
 *        state 0xFD.
 * @param ot Ordering-table entry the text is linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 */
void* carda_draw_save_complete(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2]; /* never used, but the original stack frame reserves it */
    void* result;

    result = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B058, 16), 4, -x_offset + 0x90, -y_offset, 2);
    if (g_pad_input & 0x260)
    {
        play_menu_sfx(0x7D, 0x80);
        g_carda_element_pool[0].attr.f.state = CARDA_ELEMENT_FREE;
        field_reset_input_repeat();
    }
    else if (g_carda_entry_state == 0xFD)
    {
        g_carda_element_pool[0].attr.f.state = CARDA_ELEMENT_FREE;
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
 * @note JP changes this function; the JP build takes it from assembly.
 */
#if defined(VERSION_JP)
INCLUDE_ASM("overlays/carda/nonmatchings/carda_widgets", carda_draw_format_prompt);
#else
void* carda_draw_format_prompt(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2]; /* never used, but the original stack frame reserves it */
    s32 x;
    s32 status;
    u32 attr;
    u16* text_table;
    CardaElement* message;
    s32 y; /* TODO: a plain copy of y_offset; the original register allocation needs it */

    y = y_offset;
    if (g_carda_mode == 2 || g_carda_mode == 3)
    {
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B0B4, 62), 4, -x_offset + 0x90, -y, 2);
        text_table = CARDA_TEXT_TABLE(D_8014B0B4, 62);
        prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 18), 4, -x_offset + 0x90, 0xE - y, 2);
    }
    else
    {
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(g_carda_text_card_unformatted, CARDA_TEXT_CARD_UNFORMATTED), 4, -x_offset + 0x90, -y, 2);
    }
    x = -x_offset + 0x90;
    prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B05E, 19), 4, x, 0x1C - y, 2);
    prim = carda_draw_choice_prompt(prim, ot, x, 0x2A - y);

    status = carda_poll_and_rewind_primary_handles();
    if (status == 1 || status == 2)
    {
        play_menu_sfx(0x7D, 0x80);
        g_carda_element_pool[0].attr.f.state = CARDA_ELEMENT_FREE;
        field_reset_input_repeat();
        g_carda_entry_state = 0xFF;
        carda_reset_entry_ranks();
        if (g_carda_mode == 2 || g_carda_mode == 3)
        {
            g_carda_save_step = g_carda_steps_card_reset;
        }
        else
        {
            g_carda_save_step = 0;
        }
    }
    else
    {
        status = g_pad_input;
        if ((status & 0x40) || ((status & 0x220) && g_carda_choice_toggle != 0))
        {
            D_801660F8 = 1;
            play_menu_sfx(0x7D, 0x80);
            g_carda_element_pool[0].attr.f.state = CARDA_ELEMENT_FREE;
            field_reset_input_repeat();
            g_carda_entry_state = 0xFF;
            carda_reset_entry_ranks();
            g_carda_entry_state = 0xF9;
            if (g_carda_mode == 2 || g_carda_mode == 3)
            {
                g_carda_save_step = 0;
            }
            else
            {
                g_carda_save_step = g_carda_steps_card_reset;
            }
        }
        else if (status & 0x220)
        {
            play_menu_sfx(0x7E, 0x80);
            g_carda_element_pool[0].attr.f.state = CARDA_ELEMENT_FREE;
            carda_reset_entry_ranks();
            g_carda_format_frames = 0;
            message = g_carda_element_pool;
            message->attr.f.phase = 1;
            message->attr.f.state = CARDA_ELEMENT_OPENING;
            if (g_carda_mode == 2 || g_carda_mode == 3)
            {
                message->attr.f.x = 0x10;
                message->attr.f.y = 0x4C;
                attr = message->attr.word;
                attr &= (1 << CARDA_ELEMENT_WIDTH_SHIFT) - 1;
                attr |= 0x20 << CARDA_ELEMENT_WIDTH_SHIFT;
                message->attr.word = attr;
                message->size.f.width_high = 1;
                message->size.f.height = 0x48;
            }
            else
            {
                message->attr.f.x = 0x10;
                message->attr.f.y = 0x5A;
                attr = message->attr.word;
                attr &= (1 << CARDA_ELEMENT_WIDTH_SHIFT) - 1;
                attr |= 0x20 << CARDA_ELEMENT_WIDTH_SHIFT;
                message->attr.word = attr;
                message->size.f.width_high = 1;
                message->size.f.height = 0x2C;
            }
            message->draw = carda_draw_format_progress;
        }
    }
    return prim;
}
#endif

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
void* carda_draw_format_progress(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2]; /* never used, but the original stack frame reserves it */

    if (g_carda_format_frames >= 13)
    {
        if (g_carda_mode == 2 || g_carda_mode == 3)
        {
            s32 x;
            u16* text_table;

            x = -x_offset + 0x90;
            prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B0DA, 81), 4, x, -y_offset, 2);
            text_table = CARDA_TEXT_TABLE(D_8014B0DA, 81);
            prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 61), 4, x, 0xE - y_offset, 2);
            prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 89), 4, x, 0x1C - y_offset, 2);
        }
        else
        {
            s32 x;
            u16* text_table;

            x = -x_offset + 0x90;
            prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B054, 14), 4, x, -y_offset, 2);
            text_table = CARDA_TEXT_TABLE(D_8014B054, 14);
            prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 15), 4, x, 0xE - y_offset, 2);
            prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 89), 4, x, 0x1C - y_offset, 2);
        }
    }
    else
    {
        s32 x;
        u16* text_table;

        x = -x_offset + 0x90;
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B090, 44), 4, x, -y_offset, 2);
        text_table = CARDA_TEXT_TABLE(D_8014B090, 44);
        if (g_carda_mode == 2 || g_carda_mode == 3)
        {
            prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 61), 4, x, 0xE - y_offset, 2);
            prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 89), 4, x, 0x1C - y_offset, 2);
        }
        else
        {
            prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 15), 4, x, 0xE - y_offset, 2);
            prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 89), 4, x, 0x1C - y_offset, 2);
        }
    }

    if (g_carda_format_frames == 12)
    {
        carda_reset_to_new_save_entry();
        g_carda_save_step = g_carda_steps_card_check;
    }
    else if (g_carda_format_frames >= 13)
    {
        g_carda_selected_row = 0;
        g_carda_entry_suffix_values[0] = 1;
        carda_build_save_file();
        g_carda_progress_bar_active = 0;
        g_carda_save_in_progress = 1;
        if (g_carda_mode == 2 || g_carda_mode == 3)
        {
            g_carda_received_item_count = 0;
            g_gosub_result_values = 5;
            g_carda_new_save_file = 1;
            carda_store_active_record();
            g_carda_save_step = g_carda_steps_write_alt_save;
            g_carda_entry_state = 0xF3;
            g_carda_element_pool[0].attr.f.state = CARDA_ELEMENT_FREE;
        }
        else
        {
            CardaElement* message;

            g_carda_save_step = g_carda_steps_write_save_keep_handles;
            message = g_carda_element_pool;
            message->draw = carda_draw_save_progress;
            message->attr.f.phase = 1;
            message->attr.f.state = CARDA_ELEMENT_OPEN;
            message->attr.f.x = 0x10;
            message->attr.f.y = 0x5A;
            message->size.f.width_high = 1;
            message->size.f.height = 0x2C;
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
 * @param dialog_state Message to show (0 save failed, 1 load failed, 2 card not inserted,
 *                     3 TODO: unknown, 4 save data corrupt, 5 format failed).
 */
void carda_open_status_dialog(s32 dialog_state)
{
    s32 state;
    CardaElement* dialog;

    if (g_carda_dialog_state != dialog_state || g_carda_element_pool[0].attr.f.state == CARDA_ELEMENT_FREE || g_carda_element_pool[0].draw != carda_draw_status_dialog)
    {
        g_carda_save_in_progress = 0;
        g_carda_progress_active = 0;
        g_carda_selection_status = 0;
        g_carda_io_busy = 0;
        carda_reset_entry_ranks();
        g_carda_dialog_state = dialog_state;
        _card_wait(g_carda_card_slot);
        play_menu_sfx(0x78, 0x80);

        if (g_carda_mode == 2 || g_carda_mode == 3)
        {
            state = g_carda_dialog_state;
            switch (state)
            {
            case 0:
                g_carda_entry_state = 0xF0;
                break;
            case 1:
                g_carda_entry_state = 0xEF;
                break;
            case 2:
                g_carda_entry_state = 0xEE;
                break;
            case 3:
                g_carda_entry_state = 0xED;
                break;
            case 4:
                g_carda_entry_state = 0xEC;
                break;
            case 5:
                g_carda_entry_state = 0xEB;
                break;
            }
            g_carda_save_step = 0;
            return;
        }

        dialog = g_carda_element_pool;
        dialog->attr.f.phase = 1;
        dialog->attr.f.state = CARDA_ELEMENT_OPENING;
        dialog->attr.f.x = 0x20;
        dialog->attr.word &= 0x00FFFFFF;
        dialog->size.f.width_high = 1;
        if (dialog_state < 2 || dialog_state == 4 || dialog_state == 5)
        {
            dialog->attr.f.y = 0x60;
            dialog->size.f.height = 0x24;
        }
        else
        {
            dialog->size.f.height = 0x14;
            dialog->attr.f.y = 0x70;
        }
        dialog->draw = carda_draw_status_dialog;
        field_reset_input_repeat();
        g_carda_save_in_progress = 0;
        g_carda_progress_active = 0;
        g_carda_selection_status = 0;
        g_carda_io_busy = 0;
        g_carda_entry_state = 0xFF;
        carda_reset_entry_ranks();
        g_carda_save_step = g_carda_steps_initial_scan;
        g_carda_dialog_state = dialog_state;
        _card_wait(g_carda_card_slot);
    }
}

/**
 * @brief Draw the active CARDA status dialog and handle dismissal input.
 * @param ot Ordering table used by the text renderer.
 * @param prim Current primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 * @see matching: 100.00%
 */
void* carda_draw_status_dialog(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2]; /* never used, but the original stack frame reserves it */
    u16* text_table;

    switch (g_carda_dialog_state)
    {
    case 0:
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B074, 30), 4, -x_offset + 0x80, -y_offset, 2);
        text_table = CARDA_TEXT_TABLE(D_8014B074, 30);
        prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 43), 4, -x_offset + 0x80, -y_offset + 0x10, 2);
        break;
    case 1:
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B076, 31), 4, -x_offset + 0x80, -y_offset, 2);
        text_table = CARDA_TEXT_TABLE(D_8014B076, 31);
        prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 43), 4, -x_offset + 0x80, -y_offset + 0x10, 2);
        break;
    case 2:
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B078, 32), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 3:
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B07A, 33), 4, -x_offset + 0x80, -y_offset, 2);
        if (g_carda_entry_state == 0xFD)
        {
            g_carda_element_pool[0].attr.f.state = CARDA_ELEMENT_FREE;
            field_reset_input_repeat();
            return prim;
        }
        break;
    case 4:
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B076, 31), 4, -x_offset + 0x80, -y_offset, 2);
        text_table = CARDA_TEXT_TABLE(D_8014B076, 31);
        prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 46), 4, -x_offset + 0x80, -y_offset + 0x10, 2);
        break;
    case 5:
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B09C, 50), 4, -x_offset + 0x80, -y_offset, 2);
        text_table = CARDA_TEXT_TABLE(D_8014B09C, 50);
        prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 43), 4, -x_offset + 0x80, -y_offset + 0x10, 2);
        break;
    }

    if (g_pad_input & 0x220)
    {
        switch (g_carda_mode)
        {
        case 2:
            g_field_card_overlay_mode = 6;
            break;
        case 3:
            g_field_card_overlay_mode = 7;
            break;
        default:
            g_field_card_overlay_mode = 3;
            break;
        }
        g_carda_element_pool[0].attr.f.state = CARDA_ELEMENT_FREE;
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
 *             0x4F up get a generated CLUT (func_800A5638 / func_800A55E4).
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
        func_800A5638(g_carda_icon_context, icon);
        LoadImage(&rect, g_carda_icon_context);
        DrawSync(0);
    }
    else if (icon >= CARDA_GOLEM_ICON_BASE)
    {
        func_800A55E4(g_carda_icon_context, g_carda_icon_palette);
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
    quad->tpage = getTPage(0, 0, 320, 0);
    addPrim(ot, quad);

    return quad + 1;
}

/**
 * @brief Preselect the second choice of the two-choice prompt.
 * @note JP preselects the first choice instead.
 */
void carda_enable_choice_toggle(void)
{
#if defined(VERSION_JP)
    g_carda_choice_toggle = 0;
#else
    g_carda_choice_toggle = 1;
#endif
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
void* carda_draw_choice_prompt(void* prim, u_long* ot, s32 x, s32 y)
{
    u8* text_table;
    u8* text;
    s32 color;

    color = 4;
    text = FIELD_UI_TEXT_AT(&g_text_choice_glyph_offsets, 27);
    text_table = &g_text_choice_glyph_offsets - 27 * 2;
    if (g_carda_choice_toggle != 0)
    {
        color = 5;
    }
    prim = func_800A88A0(prim, ot, text, color, x - 0x10, y, 1);

    color = 4;
    text = FIELD_UI_TEXT(text_table, 28);
    if (g_carda_choice_toggle == 0)
    {
        color = 5;
    }
    prim = func_800A88A0(prim, ot, text, color, x + 8, y, 0);

    if (g_pad_input & 0xA000)
    {
        g_carda_choice_toggle ^= 1;
        play_menu_sfx(0x7D, 0x80);
        g_pad_input = 0;
    }
    return prim;
}
