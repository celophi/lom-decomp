#include "addhero_internal.h"

/* UI states and memory-card limits. */
#define ADDHERO_ELEMENT_COUNT 8
#define ADDHERO_ELEMENT_STATE_MASK 7
#define ADDHERO_ELEMENT_PHASE_MASK 0x78
#define ADDHERO_ELEMENT_STATE_INACTIVE 0
#define ADDHERO_ELEMENT_STATE_OPENING 1
#define ADDHERO_ELEMENT_STATE_ACTIVE 2
#define ADDHERO_ELEMENT_STATE_CLOSING 3
#define ADDHERO_ELEMENT_STATE_FINISHING 4
#define ADDHERO_ENTRY_ROW_HEIGHT 14
#define ADDHERO_ENTRY_COUNT_LIMIT 0x10
#define ADDHERO_NO_ICON 0x7F

/* Return codes produced by addhero_advance_load_sequence. */
#define ADDHERO_LOAD_RESULT_NONE 0
#define ADDHERO_ENTRY_STATE_LOAD_PROGRESS 0xF6  /* importing a matched entry; progress bar */
#define ADDHERO_ENTRY_STATE_SAVE_PROGRESS 0xF5  /* write in progress; progress bar */
#define ADDHERO_ENTRY_STATE_SAVE_CONFIRM 0xF4   /* confirm dialog; accepting writes the save */
#define ADDHERO_ENTRY_STATE_CONFIRM_PROMPT 0xF3 /* companion choice prompt for save/exit flow */
#define ADDHERO_CONFIRM_BUTTON_MASK (PAD_BTN_CROSS | PAD_BTN_L3)
#define ADDHERO_CARD_SWITCH_BUTTON_MASK (PAD_BTN_SELECT | PAD_BTN_RIGHT | PAD_BTN_LEFT)
#define ADDHERO_INPUT_INJECTION_ENABLED 0x80
#define ADDHERO_SAVE_MAGIC 0x414E41
#define ADDHERO_SAVE_CHECKSUM_BYTES 0x33E0
#define ADDHERO_SAVE_CHECKSUM_BIAS 0x0414E410
#define SET_ELEM_WIDTH_LOW(element, width) ((element)->attr.word = ((element)->attr.word & 0x00FFFFFF) | ((u32)(width) << 24))
#define GLYPH_SYM(sym, off) ((void*)(((u8*)&(sym) - (off)) + (sym)))
#define GLYPH_OFF(base, off) ((void*)((base) + *(u16*)((base) + (off))))

/** Resolve a glyph string from a preloaded table @p base plus the u16 offset
 *  held in @p sym (@p sym is a table entry naming its own offset value). */
#define GLYPH_ENTRY(base, sym) ((void*)((s32)(sym) + (s32)(base)))

/** @brief Animated panel or list element used by the ADDHERO interface. */
typedef struct AddheroElement
{
    union
    {
        u32 word;
        struct
        {
            u32 state : 3;
            u32 transition_step : 4;
            u32 x : 9;
            u32 y : 8;
            u32 width_low : 8;
        } bits;
        struct
        {
            u16 control_x;
            u8 y;
            u8 width_low;
        } bytes;
    } attr;
    union
    {
        u32 word;
        struct
        {
            u32 width_high : 1;
            u32 height : 8;
            u32 scrollable : 1;
            u32 reserved : 22;
        } bits;
    } size;
    void* draw_handler;
} AddheroElement;

/** @brief Saved character metadata displayed in the card browser. */
typedef struct AddheroRecord
{
    u8 name[23];
    u8 marker_17;
    u32 first_icon_and_flags;
    u8 _pad1c[3];
    u8 icon_palette;
    u32 label_and_icons;
    u8 _pad24[12];
    s32 play_time_frames;
    u8 _pad34[0xCF - 0x34];
    u8 owner_id;
    u8 _padd0[4];
    u16 hero_id;
    u16 reserved_d6;
    s32 identity;
} AddheroRecord;

/** @brief Alternate title text in a loaded card-entry record. */
typedef struct AddheroFallbackText
{
    u8 pad[0x24];
    u8 text[0x20];
} AddheroFallbackText;

/** @brief Saved context block copied between the live game state and the save image. */
typedef struct
{
    u8 inject_enable;
    u8 _pad01[0x17];
    u32 inject_flags;
    u8 _pad1c[0x234];
} AddheroSaveContextBlock;

/** @brief Portion of the ADDHERO save image whose layout is used by this overlay. */
typedef struct
{
    u8 _pad0000[0x770];
    AddheroSaveContextBlock context;
    u8 _pad09c0[0x2A20];
    s32 checksum;
    s32 magic;
} AddheroSaveBlob;

/** @brief Common packet storage used by the element drawing callbacks. */
typedef struct
{
    s32 tag;
    s32 word4;
    s16 x0;
    s16 y0;
    s16 unknown_0x0c;
    u16 unknown_0x0e;
} AddheroGpuPacket;

/** @brief Frame drawing context with its ordering-table head and primitive cursor. */
typedef struct
{
    s32 tag;
    u8 pad4[0x40AE];
    s16 frame_flag;
    u8 _pad40b4[4];
    AddheroGpuPacket* prim_cursor;
} AddheroDrawState;

typedef AddheroGpuPacket* (*AddheroElementDrawFunc)();

/** @brief Pool of animated UI elements used by the ADDHERO screen. */
/** @brief Three double-byte overflow glyphs and their string terminator. */
typedef struct
{
    s8 data[7];
} AddheroOverflowGlyphString;

extern s8 g_addhero_decimal_overflow_glyphs[];
extern AddheroElement g_addhero_element_pool[ADDHERO_ELEMENT_COUNT];
extern AddheroElement g_addhero_element1;
extern AddheroRecord g_addhero_entry_metadata;

/* Shared controller/game context (main.h PadContext); addressed here as a byte
   buffer for the save-blob copies and metadata reads. */
extern u8* g_pad_ctx;
extern s32 g_save_slot_index;
extern s32 D_80122718;
extern s32 g_pad_input;
extern s32 g_menu_element_counter;
extern s32 g_addhero_loadseq_done;
extern s32 g_addhero_icon_phase;
extern s32 g_addhero_pad_work_ptr;
extern s32 g_addhero_result;
extern s32 g_addhero_work_ram_base;
extern s32 g_addhero_exit_requested;
extern s32 g_addhero_choice_toggle;
extern s32 g_addhero_load_flow_active;
extern s32 g_addhero_icon_palette;
extern s32 g_addhero_frame_parity;
extern s32 g_addhero_dialog_state;
extern s32 g_addhero_entry_identity;
extern s32 g_addhero_icon_image_table[];
extern s32 g_addhero_entry_fields[];
extern u8 g_addhero_loadseq_abort[];
extern u8 g_addhero_loadseq_load_begin[];
extern u8 g_addhero_loadseq_load_progress;
extern u8 g_addhero_loadseq_save_begin;
extern u8 g_addhero_icon_context[];
extern u8 g_addhero_entry_record;
extern u8 g_addhero_entry_owner_id;
extern u8 g_text_time_separator_offset_bytes[2];
extern u8 g_text_choice_glyph_offsets[];
extern u16 g_addhero_glyph_table;
extern u16 g_addhero_glyph_status_fa;
extern u16 g_addhero_glyph_status_fd;
extern u16 g_addhero_glyph_save_entry_label;
extern u16 g_addhero_glyph_default_entry_label;
extern u16 g_addhero_glyph_card_slot0_label;
extern u16 g_addhero_glyph_card_slot1_label;
extern u16 g_addhero_glyph_status_fb;
extern u16 g_addhero_glyph_status_fc;
extern u16 g_addhero_glyph_new_entry_label;
extern u16 g_addhero_glyph_save_progress;
extern u16 g_addhero_glyph_details_status2_msg;
extern u16 g_addhero_glyph_entry_value_label;
extern u16 g_addhero_glyph_load_prompt;
extern u16 g_addhero_glyph_load_progress;
extern u16 g_addhero_glyph_status_f8;
extern u16 g_addhero_glyph_alt_save_entry_label;
extern u16 g_addhero_glyph_dialog_msg0;
extern u16 g_addhero_glyph_dialog_msg1;
extern u16 g_addhero_glyph_dialog_msg2;
extern u16 g_addhero_glyph_dialog_msg3;
extern u16 g_addhero_glyph_mode0;
extern u16 g_addhero_glyph_mode1;
extern u16 g_addhero_glyph_current_hero_marker;
extern u16 g_addhero_glyph_owner_mismatch_msg;
extern u16 g_addhero_glyph_status_f3;
extern u16 g_addhero_glyph_status_f7;
extern u16 g_addhero_glyph_save_confirm_msg;
extern u16 g_addhero_glyph_plus_marker;
extern u16 g_addhero_entry_glyph_table[];

/* Overlay function declarations. */
void addhero_init(s32 work_base, s32 mode);
s32 addhero_state_step(AddheroDrawState* draw_state);
void addhero_build_ui_elements(void);
void addhero_update_state(AddheroDrawState* draw_state);
s32 addhero_update_load_sequence(void);
s32 addhero_handle_input(void);
void addhero_reset_state(void);
void addhero_close_all_elements(void);
void addhero_update_elements(AddheroDrawState* draw_state);
s32 addhero_draw_entry_list(s32* ot, s32 prim, s32 x_offset, s32 y_offset);
s32 addhero_draw_mode_glyph(s32* ot, s32 prim, s32 x_offset, s32 y_offset);
s32 addhero_draw_card_slot0_label(s32* ot, s32 prim, s32 x_offset, s32 y_offset);
s32 addhero_draw_card_slot1_label(s32* ot, s32 prim, s32 x_offset, s32 y_offset);
s32 addhero_draw_selected_entry_details(s32* ot, s32 prim, s32 x_offset, s32 y_offset);
u8* addhero_skip_hex_digits(void* text);
void addhero_terminate_multibyte_text(void* buffer);
void addhero_clear_elements();
AddheroElement* addhero_alloc_element(void);
void addhero_update_and_draw_elements(AddheroDrawState* draw_state);
void addhero_deactivate_primary_element(void);
void addhero_text_append(u8* dst, u8* src);
s32 addhero_text_byte_length(u8* str);
void addhero_text_copy(u8* dst, u8* src);
s32 addhero_draw_load_prompt(s32* ot, s32 prim, s32 x_offset, s32 y_offset);
s32 addhero_draw_load_progress(s32* ot, s32 prim, s32 x_offset, s32 y_offset);
s32 addhero_draw_progress_bar(s32 prim, s32* ot);
s32 addhero_draw_status_dialog(s32* ot, s32 prim, s32 x_offset, s32 y_offset);
s32 addhero_draw_exit_dialog(s32* ot, s32 prim, s32 x_offset, s32 y_offset);
s32 addhero_draw_transfer_status(s32* ot, s32 prim, s32 x_offset, s32 y_offset);
s32 addhero_draw_icon_highlight(s32 result, s32* ot, s32 x, s32 y, s32 adjust, s32 slot, s32 visible_index, s32 party_index);
void addhero_enable_choice_toggle(void);
s32 addhero_draw_choice_prompt(s32 prim, s32* ot, s32 x, s32 y);
s32 addhero_validate_save_blob(u8* base);
s32 addhero_compute_save_checksum(u8* data);
s8* addhero_format_decimal(s8* out, s32 value);
void addhero_format_hex(s8* out, s32 value, s32 max_chars);
void addhero_hex_nibble_to_ascii(s8* out, s32 value);
u32 addhero_parse_hex(u8* s, s32 len);
s32 addhero_parse_hex_suffix_byte(u8* text);
s32 addhero_entry_blocks_reach_limit(void);
void addhero_erase_placeholder_files(void);

/* Game and SDK function declarations. */
s32 func_800A88A0(s32 prim, s32* ot, void* glyph, s32 a3, s32 x, s32 y, s32 mode);
s32 func_800A8A78(s32* ot, s32 prim, s32 ch, s32 a3, Vec2s* pos, s32 mode);
void play_menu_sfx();
void field_restore_fade_target(void);
void field_set_default_fade_target(void);
void field_restore_fade_target_with_duration(s32 arg0);
void func_80063194(void);
void func_8001990C(RECT* rect, s32 a1, s32 a2, s32 a3);
void func_800A55E4(void* buf, s32 arg1);
void func_800A5638(void* buf, s32 arg1);
void func_8001A5D4(s32 arg0, s32* arg1);
void func_8001C56C(s32* arg0, s32 a1, s32 a2, s32 a3, s32 a4);
s32 func_800AD850();
s32 func_800AE76C();
void field_text_reset_scratch(void);
void field_text_reset_windows(void);
void addhero_reset_entry_ranks(void);
void addhero_enable_choice_toggle(void);

/**
 * @brief Reset overlay state and build the initial UI elements.
 * @param work_base Work-RAM base (always 0x80170000); stored in g_addhero_work_ram_base, unused so far.
 * @param mode Mode selector, stored in g_addhero_mode.
 * @see decomp.me (100%)
 */
void addhero_init(s32 work_base, s32 mode)
{
    RECT rect;

    g_addhero_mode = mode;
    g_addhero_entry_state = ADDHERO_ENTRY_STATE_IDLE;
    g_addhero_card_slot = 0;

    addhero_reset_entry_ranks();
    g_addhero_result = 3;
    addhero_init_card_events();
    g_addhero_icon_phase = 0;
    field_set_default_fade_target();

    rect.x = OVERLAY_INIT_CLEAR_VRAM_X;
    rect.y = OVERLAY_INIT_CLEAR_VRAM_Y;
    rect.w = OVERLAY_INIT_CLEAR_VRAM_W;
    rect.h = OVERLAY_INIT_CLEAR_VRAM_H;

    func_8001990C(&rect, 0, 0, 0);
    addhero_reset_glyph_cache();

    g_addhero_write_in_progress = 0;
    g_addhero_progress_active = 0;
    g_addhero_selection_status = 0;
    g_addhero_io_busy = 0;
    g_addhero_frame_parity = 0;
    g_addhero_exit_requested = 0;

    func_800AA02C();
    addhero_build_ui_elements();

    g_addhero_work_ram_base = work_base;
}

/**
 * @brief Run one frame: tear down and exit if requested, else update and render.
 * @param draw_state Frame drawing context passed through to the element renderer.
 * @return Non-zero exit code when exiting, 0 while running.
 * @see decomp.me (100%)
 */
s32 addhero_state_step(AddheroDrawState* draw_state)
{
    if (g_addhero_exit_requested != 0)
    {
        addhero_shutdown_card_events();
        field_text_reset_windows();
        func_80019788(0);
        return g_addhero_exit_requested;
    }

    field_text_reset_scratch();
    addhero_begin_glyph_cache_frame();
    addhero_update_state(draw_state);
    addhero_evict_unused_glyphs();
    func_80063194();
    g_addhero_frame_parity ^= 1;
    return 0;
}

/**
 * @brief Reset scroll/selection state and populate the UI element pool for the current mode.
 * @note mode != 0: transfer layout (status + two card-slot labels). mode == 0: full browser
 *       (entry list, mode glyph, two slot labels, entry details).
 * @see decomp.me (100%)
 */
void addhero_build_ui_elements(void)
{
    AddheroElement* element;
    g_addhero_scroll_frames = 0;
    g_addhero_scroll_target_y = 0;
    g_addhero_scroll_y = 0;
    g_addhero_selected_row = 0;
    g_addhero_selection_status = 0;
    g_addhero_pad_work_ptr = (s32)g_pad_ctx + 0xCE0;
    if (0)
    {
        addhero_clear_elements(0, 0, 0, 0, 0);
    }
    addhero_clear_elements();
    g_addhero_load_flow_active = 0;
    if (g_addhero_mode != 0)
    {
        g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_OPENING;
        element = addhero_alloc_element();
        element->draw_handler = (void*)addhero_draw_transfer_status;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = 0x10;
        element->attr.bits.y = 0x61;
        element->size.bits.width_high = 1;
        element->size.bits.height = 0x2C;
        SET_ELEM_WIDTH_LOW(element, 0x20);

        element = addhero_alloc_element();
        element->draw_handler = (void*)addhero_draw_card_slot0_label;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = 0x18;
        element->attr.bits.y = 0x4D;
        element->size.bits.width_high = 0;
        element->size.bits.height = 0x10;
        SET_ELEM_WIDTH_LOW(element, 0x80);

        element = addhero_alloc_element();
        element->draw_handler = (void*)addhero_draw_card_slot1_label;
        element->attr.bits.transition_step = 1;
        element->attr.bits.x = 0xA0;
        element->attr.bits.y = 0x4D;
        element->size.bits.width_high = 0;
        element->size.bits.height = 0x10;
        SET_ELEM_WIDTH_LOW(element, 0x80);
        g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
        return;
    }

    g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_OPENING;
    element = addhero_alloc_element();
    element->draw_handler = (void*)addhero_draw_entry_list;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = 0x1C;
    element->attr.bits.y = 0x32;
    element->size.bits.width_high = 1;
    element->size.bits.height = 0x58;
    SET_ELEM_WIDTH_LOW(element, 8);
    element->size.bits.scrollable = 1;

    element = addhero_alloc_element();
    element->draw_handler = (void*)addhero_draw_mode_glyph;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = 0x24;
    element->attr.bits.y = 0x0A;
    element->size.bits.width_high = 0;
    element->size.bits.height = 0x10;
    SET_ELEM_WIDTH_LOW(element, 0xF0);

    element = addhero_alloc_element();
    element->draw_handler = (void*)addhero_draw_card_slot0_label;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = 0x18;
    element->attr.bits.y = 0x1E;
    element->size.bits.width_high = 0;
    element->size.bits.height = 0x10;
    SET_ELEM_WIDTH_LOW(element, 0x80);

    element = addhero_alloc_element();
    element->draw_handler = (void*)addhero_draw_card_slot1_label;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = 0xA0;
    element->attr.bits.y = 0x1E;
    element->size.bits.width_high = 0;
    element->size.bits.height = 0x10;
    SET_ELEM_WIDTH_LOW(element, 0x80);

    element = addhero_alloc_element();
    element->draw_handler = (void*)addhero_draw_selected_entry_details;
    element->attr.bits.transition_step = 1;
    element->attr.bits.x = 0x1E;
    element->attr.bits.y = 0x8E;
    element->size.bits.width_high = 1;
    element->size.bits.height = 0x34;
    SET_ELEM_WIDTH_LOW(element, 4);
    g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
}

/**
 * @brief Run one frame of overlay logic: update elements, advance the load
 *        sequence when armed, sample pad input, and step the scroll animation.
 * @param draw_state Frame drawing context passed to the element renderer.
 * @see decomp.me (100%)
 */
void addhero_update_state(AddheroDrawState* draw_state)
{
    s32 delta;

    addhero_update_elements(draw_state);
    g_addhero_icon_phase += 2;
    if ((g_addhero_element1.attr.word & 0x7F) == 2)
    {
        addhero_update_load_sequence();
    }
    if ((u16)g_pad_input == 0xFFFF)
    {
        g_pad_input = 0;
    }
    addhero_handle_input();
    if (g_addhero_scroll_frames != 0)
    {
        s32* scroll_y = &g_addhero_scroll_y;
        delta = (g_addhero_scroll_target_y - *scroll_y) / g_addhero_scroll_frames;
        g_addhero_scroll_frames -= 1;
        *scroll_y += delta;
    }
    else
    {
        g_addhero_scroll_y = g_addhero_scroll_target_y;
    }
}

/**
 * @brief Drive the card load/scan state machine one frame, mapping its result
 *        code onto the next load step and any error entry-state sentinel.
 * @return Unspecified; callers ignore the return value.
 * @see decomp.me (100%)
 */
s32 addhero_update_load_sequence(void)
{
    s32 result;

    if (g_addhero_entry_state >= 0x10)
    {
        if (g_addhero_load_step == 0)
        {
            g_addhero_load_step = (u8*)&g_addhero_loadseq_start;
        }
    }

    do
    {
        result = addhero_advance_load_sequence();
    } while (result == ADDHERO_LOAD_RESULT_CONTINUE);

    if ((g_addhero_load_flow_active != 0) && (g_pad_input & 0x220))
    {
        if (g_addhero_mode == 0)
        {
            g_addhero_entry_state = 0xF9;
        }
        else
        {
            g_addhero_entry_state = 0xF8;
        }
        g_addhero_load_step = (u8*)&g_addhero_loadseq_abort;
    }
    else
    {
        switch (result)
        {
        case ADDHERO_LOAD_RESULT_NONE:
            break;
        case ADDHERO_LOAD_RESULT_COMPLETE:
            g_addhero_load_step = (u8*)&g_addhero_loadseq_done;
            g_addhero_load_flow_active = 0;
            break;
        case ADDHERO_LOAD_RESULT_CARD_ERROR:
            if (g_addhero_mode == 0)
            {
                g_addhero_entry_state = 0xF9;
            }
            else
            {
                g_addhero_entry_state = 0xF8;
            }
            /* fallthrough */
        case ADDHERO_LOAD_RESULT_ABORT:
            g_addhero_load_step = (u8*)&g_addhero_loadseq_abort;
            break;
        }
    }
}

/**
 * @brief Handle browser input, entry navigation, and load confirmation.
 * @return Unspecified; callers ignore the return value.
 */
s32 addhero_handle_input(void)
{
    s32 entry_count;
    s32 input;
    s32 move_count;
    AddheroElement* prompt;
    struct DIRENTRY* selected_entry;

    if (g_addhero_element_pool[1].attr.bits.state == ADDHERO_ELEMENT_STATE_INACTIVE)
    {
        g_addhero_exit_requested = g_addhero_result;
        return;
    }
    if (g_addhero_exit_requested != 0)
    {
        return;
    }
    if (g_addhero_element_pool[1].attr.bits.state >= ADDHERO_ELEMENT_STATE_CLOSING)
    {
        return;
    }
    if (g_addhero_element_pool[0].attr.bits.state != ADDHERO_ELEMENT_STATE_INACTIVE)
    {
        return;
    }

    entry_count = g_addhero_entry_state;
    if (entry_count == ADDHERO_ENTRY_STATE_IDLE)
    {
        return;
    }
    if (g_addhero_entry_scan_active != 0)
    {
        return;
    }
    if (g_addhero_io_busy != 0)
    {
        return;
    }
    if ((u32)(*g_addhero_load_step - 6) < 2U)
    {
        return;
    }
    if (g_addhero_mode != 0)
    {
        return;
    }

    input = g_pad_input;
    if (input & PAD_BTN_CIRCLE)
    {
        D_80122718 = 3;
        play_menu_sfx(0x78, 0x80);
        addhero_close_all_elements();
        return;
    }
    if (input & ADDHERO_CARD_SWITCH_BUTTON_MASK)
    {
        play_menu_sfx(0x7D, 0x80);
        addhero_reset_state();
        return;
    }
    if (entry_count >= ADDHERO_ENTRY_COUNT_LIMIT)
    {
        return;
    }

    move_count = 1;
    if (input & PAD_BTN_R1)
    {
        g_pad_input = PAD_BTN_DOWN;
        move_count = 1;
    }
    if (g_pad_input & PAD_BTN_L1)
    {
        g_pad_input = PAD_BTN_UP;
        move_count = 1;
    }

    while (move_count != 0)
    {
        if (g_pad_input & PAD_BTN_UP)
        {
            g_addhero_selected_row--;
            if (g_addhero_selected_row < 0)
            {
                g_addhero_selected_row = g_addhero_entry_state - 1;
            }
        }
        if (g_pad_input & PAD_BTN_DOWN)
        {
            g_addhero_selected_row++;
            if (g_addhero_selected_row >= g_addhero_entry_state)
            {
                g_addhero_selected_row = 0;
            }
        }
        move_count--;
    }

    if (g_pad_input & (PAD_BTN_UP | PAD_BTN_DOWN))
    {
        addhero_commit_selected_entry();
        play_menu_sfx(0x7D, 0x80);
        addhero_scroll_to_selection();
        return;
    }

    if (g_pad_input & ADDHERO_CONFIRM_BUTTON_MASK)
    {
        selected_entry = &g_addhero_entries[g_addhero_card_slot][g_addhero_selected_row];
        if (strncmp(g_lom_save_filename_prefix, selected_entry->name, 0xC) == 0)
        {
            if ((g_addhero_entry_metadata.hero_id != ((AddheroRecord*)g_pad_ctx)->hero_id) &&
                ((g_save_slot_index == 0xFF) || (g_addhero_entry_metadata.owner_id == g_save_slot_index)))
            {
                prompt = addhero_alloc_element();
                prompt->attr.bits.transition_step = 1;
                prompt->attr.bits.x = 0x10;
                prompt->attr.bits.y = 0x61;
                prompt->size.bits.width_high = 1;
                prompt->size.bits.height = 0x1E;
                SET_ELEM_WIDTH_LOW(prompt, 0x20);
                addhero_enable_choice_toggle();
                prompt->draw_handler = addhero_draw_load_prompt;
                addhero_restart_load_sequence();
                play_menu_sfx(0x7E, 0x80);
                return;
            }
        }
        play_menu_sfx(0x78, 0x80);
    }
}

/**
 * @brief Reset scroll/selection state and flip to the other card slot, then
 *        clear ranks and pad input to restart browsing.
 * @see decomp.me (100%)
 */
void addhero_reset_state(void)
{
    g_addhero_load_flow_active = 0;
    g_addhero_load_step = 0;
    g_addhero_entry_state = ADDHERO_ENTRY_STATE_IDLE;
    g_addhero_scroll_frames = 0;
    g_addhero_scroll_target_y = 0;
    g_addhero_scroll_y = 0;
    g_addhero_selected_row = 0;
    g_addhero_selection_status = 0;
    g_addhero_card_slot ^= 1;
    addhero_reset_entry_ranks();
    func_800AA02C();
    g_pad_input = 0;
}

/**
 * @brief Put every active pool element into the closing transition (state 3,
 *        phase 0x40) so they animate out.
 * @see decomp.me (100%)
 */
void addhero_close_all_elements(void)
{
    AddheroElement* element;
    s32 slot;

    field_restore_fade_target();
    element = g_addhero_element_pool;
    for (slot = 0; slot < ADDHERO_ELEMENT_COUNT; slot++, element++)
    {
        if (element->attr.bits.state != ADDHERO_ELEMENT_STATE_INACTIVE)
        {
            element->attr.bits.state = ADDHERO_ELEMENT_STATE_CLOSING;
            element->attr.bits.transition_step = 8;
        }
    }
}

/**
 * @brief Retarget the list scroll so the selected row stays on screen,
 *        animating over four frames when it falls above or below the window.
 * @see decomp.me (100%)
 */
void addhero_scroll_to_selection(void)
{
    s32 selected_row;
    s32 half_row_y;
    s32 scroll_y;
    s32 row_y;
    s32 relative_y;

    selected_row = g_addhero_selected_row;
    half_row_y = (selected_row << 3) - selected_row;
    scroll_y = g_addhero_scroll_y;
    row_y = half_row_y << 1;
    relative_y = row_y - scroll_y;

    if (relative_y >= 0x4B)
    {
        g_addhero_scroll_target_y = row_y - 0x46;
        g_addhero_scroll_frames = 4;
    }
    if (relative_y < 0)
    {
        g_addhero_scroll_target_y = row_y;
        g_addhero_scroll_frames = 4;
    }
}

/**
 * @brief Thin wrapper that runs the element update/draw pass on the active
 *        draw state.
 * @param draw_state Frame drawing context to update.
 * @see decomp.me (100%)
 */
void addhero_update_elements(AddheroDrawState* draw_state)
{
    addhero_update_and_draw_elements(draw_state);
}

/**
 * @brief Draw the save-entry browser: status/error screens by entry-state
 *        sentinel, the per-row entry list with rank glyphs, and the selection
 *        highlight tile.
 * @param ot   Ordering table the primitives are linked into.
 * @param prim Current primitive pointer/index within the ordering table.
 * @param x_offset Horizontal offset; screen X is derived from it.
 * @param y_offset Vertical offset applied to each row.
 * @return The updated primitive pointer after linking this frame's glyphs.
 */
s32 addhero_draw_entry_list(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    s32 entry_state = g_addhero_entry_state;

    switch (entry_state)
    {
    case 0xF8:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_status_f8, 0x34), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xF9:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_status_f8, 0x34), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFA:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_status_fa, 2), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFD:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_status_fd, 4), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFB:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_status_fb, 0x10), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFC:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_status_fc, 0x12), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFE:
        break;
    case ADDHERO_ENTRY_STATE_IDLE:
    {
        s32 message_x;
        u8* glyph_base;

        message_x = -x_offset + 0x84;
        glyph_base = (u8*)&g_addhero_glyph_table;
        prim = func_800A88A0(prim, ot, glyph_base + g_addhero_glyph_table, 4, message_x, -y_offset, 2);
        prim = func_800A88A0(prim, ot, GLYPH_OFF(glyph_base, 0x1E), 4, message_x, ADDHERO_ENTRY_ROW_HEIGHT - y_offset, 2);
        prim = func_800A88A0(prim, ot, GLYPH_OFF(glyph_base, 0xB2), 4, message_x, (ADDHERO_ENTRY_ROW_HEIGHT * 2) - y_offset, 2);
    }
    break;
    default:
    {
        s32 row_y;
        s32 entry_index;

        if (g_addhero_entry_scan_active != 0)
        {
            s32 message_x;
            u8* glyph_base;

            message_x = -x_offset + 0x84;
            glyph_base = (u8*)&g_addhero_glyph_table;
            prim = func_800A88A0(prim, ot, glyph_base + g_addhero_glyph_table, 4, message_x, -y_offset, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(glyph_base, 0x1E), 4, message_x, ADDHERO_ENTRY_ROW_HEIGHT - y_offset, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(glyph_base, 0xB2), 4, message_x, (ADDHERO_ENTRY_ROW_HEIGHT * 2) - y_offset, 2);
            break;
        }
        entry_index = 0;
        if (entry_state > 0)
        {
            s32 list_x;
            u16 rank_glyph;
            Vec2s value_pos;
            u8* glyph_base;

            glyph_base = (u8*)&g_addhero_glyph_table;
            list_x = -x_offset;
            do
            {
                row_y = ((entry_index * ADDHERO_ENTRY_ROW_HEIGHT) - y_offset) - g_addhero_scroll_y + 1;
                if ((u32)(row_y + ADDHERO_ENTRY_ROW_HEIGHT - 1) < 0x65U)
                {
                    if (g_addhero_entry_ranks[entry_index] >= 0)
                    {
                        value_pos.x = list_x + 0x86;
                        value_pos.y = row_y;
                        prim = func_800A88A0(func_800A8A78(ot, prim, g_addhero_entry_suffix_values[entry_index], 4, &value_pos, 0), ot,
                                             GLYPH_ENTRY(glyph_base, g_addhero_glyph_entry_value_label), 4, list_x + 0x70, row_y, 0);
                        if ((g_addhero_rank_count - 1) == g_addhero_entry_ranks[entry_index])
                        {
                            rank_glyph = *(u16*)(glyph_base + 0x36);
                            prim = func_800A88A0(prim, ot, GLYPH_ENTRY(glyph_base, rank_glyph), 4, list_x + 0xC0, row_y, 0);
                        }
                        else if (g_addhero_entry_ranks[entry_index] < 2)
                        {
                            rank_glyph = *(u16*)(glyph_base + 0x38);
                            prim = func_800A88A0(prim, ot, GLYPH_ENTRY(glyph_base, rank_glyph), 4, list_x + 0xC0, row_y, 0);
                        }
                        if (*addhero_skip_hex_digits(&g_addhero_entries[g_addhero_card_slot][entry_index].name[ADDHERO_SAVE_FILENAME_PREFIX_LENGTH]) == '+')
                        {
                            prim = func_800A88A0(prim, ot, GLYPH_ENTRY(glyph_base, g_addhero_glyph_plus_marker), 4, 0xF2 - x_offset, row_y, 1);
                        }
                    }
                    if (strncmp(g_lom_save_filename_prefix, g_addhero_entries[g_addhero_card_slot][entry_index].name, ADDHERO_SAVE_FILENAME_PREFIX_LENGTH) == 0)
                    {
                        prim = func_800A88A0(prim, ot, GLYPH_ENTRY(glyph_base, g_addhero_glyph_save_entry_label), 4, 1 - x_offset, row_y, 0);
                    }
                    else if (strncmp(g_lom_alt_save_filename_prefix, g_addhero_entries[g_addhero_card_slot][entry_index].name,
                                     ADDHERO_SAVE_FILENAME_PREFIX_LENGTH) == 0)
                    {
                        prim = func_800A88A0(prim, ot, GLYPH_ENTRY(glyph_base, g_addhero_glyph_alt_save_entry_label), 4, 1 - x_offset, row_y, 0);
                    }
                    else if (strncmp(g_new_save_entry_prefix, g_addhero_entries[g_addhero_card_slot][entry_index].name,
                                     ADDHERO_NEW_SAVE_FILENAME_PREFIX_LENGTH) == 0)
                    {
                        prim = func_800A88A0(prim, ot, GLYPH_ENTRY(glyph_base, g_addhero_glyph_new_entry_label), 4, 1 - x_offset, row_y, 0);
                    }
                    else
                    {
                        prim = func_800A88A0(prim, ot, GLYPH_ENTRY(glyph_base, g_addhero_glyph_default_entry_label), 4, 1 - x_offset, row_y, 0);
                    }
                }
                entry_index++;
            } while (entry_index < g_addhero_entry_state);
        }
        row_y = ((g_addhero_selected_row * ADDHERO_ENTRY_ROW_HEIGHT) - y_offset) - g_addhero_scroll_y;

        if (g_addhero_entry_scan_active == 0)
        {
            TILE* tile = (TILE*)prim;

            *(u32*)&tile->r0 = 0xF080F0;
            setlen(tile, 3);
            setcode(tile, 0x62);
            tile->w = 0x108;
            tile->x0 = 0;
            tile->y0 = row_y;
            tile->h = ADDHERO_ENTRY_ROW_HEIGHT;
            addPrim(ot, tile);
            prim += sizeof(TILE);
        }
    }
    break;
    }
    return prim;
}

/**
 * @brief Draw the header glyph that reflects the current mode (load vs save).
 * @param ot   Ordering table the glyph is linked into.
 * @param prim Current primitive pointer/index.
 * @param x_offset Horizontal offset; screen X is derived from it.
 * @param y_offset Vertical offset.
 * @return The updated primitive pointer.
 * @see decomp.me (100%)
 */
s32 addhero_draw_mode_glyph(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;

    if (g_addhero_mode == 1)
    {
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_mode1, 0x46), 4, -x_offset + 0x78, -y_offset, 2);
    }
    else
    {
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_mode0, 0x44), 4, -x_offset + 0x78, -y_offset, 2);
    }
    return prim;
}

/**
 * @brief Draw the slot-0 card label, dimming it with a backing tile when that
 *        slot is not the active one.
 * @param ot   Ordering table the primitives are linked into.
 * @param prim Current primitive pointer/index.
 * @param x_offset Horizontal offset; screen X is derived from it.
 * @param y_offset Vertical offset.
 * @return The updated primitive pointer.
 * @see decomp.me (100%)
 */
s32 addhero_draw_card_slot0_label(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    TILE* tile;

    if (g_addhero_card_slot != 0)
    {
        tile = (TILE*)prim;
        *(u32*)&tile->r0 = 0x101010;
        setlen(tile, 3);
        tile->code = 0x62;
        tile->x0 = 0;
        tile->y0 = 0;
        tile->w = 0x80;
        tile->h = 0x10;
        addPrim(ot, tile);
        prim += sizeof(TILE);
    }
    return func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_card_slot0_label, 0xC), 4, -x_offset + 0x40, -y_offset, 2);
}

/**
 * @brief Draw the slot-1 card label, dimming it with a backing tile when that
 *        slot is not the active one.
 * @param ot   Ordering table the primitives are linked into.
 * @param prim Current primitive pointer/index.
 * @param x_offset Horizontal offset; screen X is derived from it.
 * @param y_offset Vertical offset.
 * @return The updated primitive pointer.
 * @see decomp.me (100%)
 */
s32 addhero_draw_card_slot1_label(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    TILE* tile;

    if (g_addhero_card_slot == 0)
    {
        tile = (TILE*)prim;
        *(u32*)&tile->r0 = 0x101010;
        setlen(tile, 3);
        tile->code = 0x62;
        tile->x0 = 0;
        tile->y0 = 0;
        tile->w = 0x80;
        tile->h = 0x10;
        addPrim(ot, tile);
        prim += sizeof(TILE);
    }
    return func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_card_slot1_label, 0xE), 4, -x_offset + 0x40, -y_offset, 2);
}

/**
 * @brief Draw the detail panel for the selected entry: animated character
 *        icons, play-time, hero name, and either the cached name text or a
 *        fallback message depending on entry type.
 * @param ot   Ordering table the primitives are linked into.
 * @param prim Current primitive pointer/index.
 * @param x_offset Horizontal offset; screen X is derived from it.
 * @param y_offset Vertical offset.
 * @return The updated primitive pointer.
 * @see decomp.me (100%)
 */
s32 addhero_draw_selected_entry_details(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    s32 result;
    Vec2s pos;
    u8 name[0x21];
    char unused_pad[212];
    s32 slot[3];

    result = prim;
    if (g_addhero_selection_status == 0)
    {
        return result;
    }
    if (g_addhero_entry_scan_active != 0)
    {
        return result;
    }
    if (g_addhero_selection_status != 3 && g_addhero_entry_state < 0x10)
    {
        if (g_addhero_selection_status == 2)
        {
            s32 x = -x_offset;
            u8* base;

            result = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_details_status2_msg, 0x28), 4, x, -y_offset, 0);
            base = (u8*)&g_addhero_glyph_details_status2_msg - 0x28;
            return func_800A88A0(result, ot, GLYPH_OFF(base, 0x2A), 4, x, 0x10 - y_offset, 0);
        }
        else
        {
            if (strncmp(g_lom_save_filename_prefix, g_addhero_entries[g_addhero_card_slot][g_addhero_selected_row].name, ADDHERO_SAVE_FILENAME_PREFIX_LENGTH) ==
                0)
            {
                if (g_save_slot_index == 0xFF || g_addhero_entry_owner_id == g_save_slot_index)
                {
                    s32 present_count;
                    s32 i;
                    s32 j;
                    s32 step;
                    s32 half_step;
                    s32 base_x;
                    s32 base_y;
                    s32 total;
                    s32 hours;
                    s32 time_val;

                    {
                        AddheroRecord* record = &g_addhero_entry_metadata;
                        slot[0] = record->first_icon_and_flags >> 0x19;
                        slot[1] = (record->label_and_icons >> 0x12) & 0x7F;
                        slot[2] = record->label_and_icons >> 0x19;
                        g_addhero_icon_palette = (s32)record->icon_palette;
                    }

                    total = 0;
                    present_count = 0;
                    for (i = 0; i < 3; i++)
                    {
                        if (slot[i] != 0x7F)
                        {
                            present_count += 1;
                        }
                    }

                    switch (present_count)
                    {
                    case 2:
                        step = 0x20;
                        half_step = 0x10;
                        time_val = g_addhero_icon_phase;
                        if (g_addhero_icon_phase < 0)
                        {
                            time_val = g_addhero_icon_phase + 0x1F;
                        }
                        g_addhero_icon_phase -= (time_val >> 5) << 5;
                        break;
                    case 3:
                        step = 0x10;
                        half_step = 0x20;
                        g_addhero_icon_phase %= 0x60;
                        break;
                    default:
                        step = 0x10;
                        half_step = 0x20;
                        g_addhero_icon_phase = 0x1F;
                        break;
                    }

                    i = 0;
                    j = i;
                    for (; j < 3; j++)
                    {
                        base_y = i * half_step;
                        base_x = base_y + half_step;
                        if (slot[j] != 0x7F)
                        {
                            s32 adjust = step;
                            s32 rem;
                            s32 hi;
                            s32 delta;

                            if ((g_addhero_icon_phase >= base_y && g_addhero_icon_phase < base_x && (delta = g_addhero_icon_phase - base_y, 1)) ||
                                (rem = base_x % (half_step * present_count),
                                 g_addhero_icon_phase >= rem && g_addhero_icon_phase < (hi = rem + half_step) && (delta = hi - g_addhero_icon_phase, 1)))
                            {
                                adjust += delta;
                            }
                            result = addhero_draw_icon_highlight(result, ot, total - x_offset, -y_offset, adjust, slot[j], i, j);
                            i += 1;
                            total += adjust;
                        }
                    }

                    {
                        AddheroRecord* record = &g_addhero_entry_metadata;
                        s32 x = -x_offset;
                        s32 y = -y_offset;

                        base_y = record->play_time_frames;
                        pos.x = (s16)(x + 0x70);
                        pos.y = (s16)y;
                        hours = base_y / 216000;
                        result = func_800A8A78(ot, result, hours, 4, &pos, 1);
                        result = func_800A88A0(result, ot,
                                               (void*)(g_text_time_separator_offset_bytes[0] + ((s32)&g_text_time_separator_offset_bytes - 0x32) +
                                                       (g_text_time_separator_offset_bytes[1] << 8)),
                                               4, x + 0x6F, y, 0);
                        base_y = (base_y / 3600) - (hours * 0x3C);
                        if (base_y < 0xA)
                        {
                            pos.x = (s16)(x + 0x7D);
                            pos.y = (s16)y;
                            result = func_800A8A78(ot, result, 0, 4, &pos, 1);
                        }
                        pos.x = (s16)(x + 0x85);
                        pos.y = (s16)y;
                        result = func_800A8A78(ot, result, base_y, 4, &pos, 1);
                        result = func_800A88A0(result, ot, record->name, 4, x + 0x54, y + 0x10, 0);

                        if (record->hero_id == ((AddheroRecord*)g_pad_ctx)->hero_id)
                        {
                            do
                            {
                                result = func_800A88A0(result, ot, GLYPH_SYM(g_addhero_glyph_current_hero_marker, 0x50), 4, x + 0x54, y + 0x20, 0);
                            } while (0);
                        }
                        else
                        {
                            result = func_800A88A0(result, ot, GLYPH_OFF((u8*)g_addhero_entry_glyph_table, ((s32)record->label_and_icons & 0x3FFFF) * 2), 4,
                                                   x + 0x54, y + 0x20, 0);
                        }
                    }
                }
                else
                {
                    result = func_800A88A0(result, ot, GLYPH_SYM(g_addhero_glyph_owner_mismatch_msg, 0x54), 4, -x_offset, -y_offset, 0);
                }
            }
            else
            {
                s32 j;
                u8* record;

                addhero_terminate_multibyte_text(&g_addhero_entry_record);
                record = &g_addhero_entry_record;
                record -= 4;
                if ((u32)(record[0x24] - 1) >= 0x7FU)
                {
                    for (j = 0; j < 0x20; j++)
                    {
                        name[j] = record[4 + j];
                    }
                    name[j] = 0;
                    result = addhero_draw_cached_text(result, ot, name, -x_offset, -y_offset, 4, 0);

                    for (j = 0; j < 0x20; j++)
                    {
                        name[j] = ((AddheroFallbackText*)&g_addhero_entry_read_buffer)->text[j];
                    }
                    name[j] = 0;
                    result = addhero_draw_cached_text(result, ot, name, -x_offset, -y_offset + 0x10, 4, 0);
                }
            }
        }
    }
    return result;
}

/**
 * @brief Advance past a run of hex digit characters (0-9, a-f, A-F) and return
 *        the pointer to the first non-hex byte.
 * @param text Start of the text to scan.
 * @return Pointer to the first byte that is not a hex digit.
 * @see decomp.me (100%)
 */
u8* addhero_skip_hex_digits(void* text)
{
    u8* cursor = text;

    while ((u32)(*cursor - '0') < 10 || (u32)(*cursor - 'a') < 6 || (u32)(*cursor - 'A') < 6)
    {
        cursor++;
    }
    return cursor;
}

/**
 * @brief Zero-fill a 0x40-byte text field from the first null byte onward,
 *        walking multibyte (>= 0x80 lead) characters two bytes at a time.
 * @param buffer Start of the 0x40-byte text buffer to terminate/clear.
 * @see decomp.me (100%)
 */
void addhero_terminate_multibyte_text(void* buffer)
{
    u8* p;
    s32 i;

    p = (u8*)buffer;
    i = 0;
    for (;;)
    {
        if (i >= 0x40)
        {
            return;
        }
        if (*p == 0)
        {
            while (i < 0x40)
            {
                *p = 0;
                i++;
                p++;
            }
            return;
        }
        if (*p >= 0x80)
        {
            p += 2;
            i += 2;
        }
        else
        {
            p += 1;
            i += 1;
        }
    }
}

/**
 * @brief Clear the eight-element pool: drop the ADDHERO flag and free (state 0)
 *        every element, and reset the shared draw scale to 0x20.
 * @see decomp.me (100%)
 */
void addhero_clear_elements(void)
{
    AddheroElement* p;
    s32 i;

    g_menu_element_counter = 0x20;
    p = g_addhero_element_pool;
    for (i = 0; i < ADDHERO_ELEMENT_COUNT; i++)
    {
        p->size.bits.scrollable = 0;
        p->attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
        p++;
    }
}

/**
 * @brief Claim the first free pool element, marking it state 1 (opening).
 * @return The claimed element, or the pool base element when none are free.
 * @see decomp.me (100%)
 */
AddheroElement* addhero_alloc_element(void)
{
    AddheroElement* p;
    s32 i;

    p = g_addhero_element_pool;
    for (i = 0; i < ADDHERO_ELEMENT_COUNT; i++, p++)
    {
        if (p->attr.bits.state == ADDHERO_ELEMENT_STATE_INACTIVE)
        {
            p->attr.bits.state = ADDHERO_ELEMENT_STATE_OPENING;
            return p;
        }
    }
    return g_addhero_element_pool;
}

/**
 * @brief Update and render the active ADDHERO UI elements.
 * @param draw_state Draw state holding the primitive cursor and frame flag.
 * @see decomp.me (100%)
 */
void addhero_update_and_draw_elements(AddheroDrawState* draw_state)
{
    AddheroGpuPacket* packet_cursor;
    AddheroDrawState* ordering_table;
    AddheroElement* element;
    s32 animated_width;
    s32 animated_height;
    s32 element_index;
    s32 draw_env[24];
    u32 state_word;
    s32 state;
    u32 size_word;
    u32 width;
    s32 transition_step;
    s32 scaled_width;
    s32 height;
    s32 scaled_height;
    s32 remaining_height;
    u32 opening_word;
    u32 width_low;
    s32 closing_word;
    s32 closing_scaled_width;
    s32 closing_height;
    s32 closing_scaled_height;
    s32 closing_remaining_height;
    u32 finishing_word;
    u32 updated_word;
    s32 entry_count;

    packet_cursor = draw_state->prim_cursor;
    ordering_table = draw_state;

    entry_count = g_addhero_entry_state;
    if ((entry_count < 0x10) && (g_addhero_element_pool[1].attr.bits.state == ADDHERO_ELEMENT_STATE_ACTIVE) &&
        (g_addhero_element_pool[1].size.bits.scrollable != 0))
    {
        entry_count *= ADDHERO_ENTRY_ROW_HEIGHT;
        if ((g_addhero_scroll_y + 0x58) < entry_count)
        {
            packet_cursor = (AddheroGpuPacket*)func_800AE76C(packet_cursor, ordering_table, 0x114, 0x82, 0);
        }
        if (g_addhero_scroll_y != 0)
        {
            packet_cursor = (AddheroGpuPacket*)func_800AE76C(packet_cursor, ordering_table, 0x114, 0x3A, 1);
        }
    }

    if (draw_state->frame_flag != 0)
    {
        func_8001C56C(draw_env, 0, 0xF0, 0x140, 0xE0);
    }
    else
    {
        func_8001C56C(draw_env, 0, 8, 0x140, 0xE0);
    }

    element = g_addhero_element_pool;
    element_index = 0;

    for (; element_index < ADDHERO_ELEMENT_COUNT; element_index++, element++)
    {
        if (element->attr.word & ADDHERO_ELEMENT_STATE_MASK)
        {
            func_8001A5D4((s32)packet_cursor, draw_env);

            addPrim(ordering_table, packet_cursor);

            state_word = ((volatile AddheroElement*)element)->attr.word;
            state = state_word & ADDHERO_ELEMENT_STATE_MASK;

            packet_cursor = (AddheroGpuPacket*)((u8*)packet_cursor + 0x40);

            switch (state)
            {
            case ADDHERO_ELEMENT_STATE_OPENING:
                opening_word = element->attr.word;
                size_word = element->size.word;
                width_low = opening_word >> 24;
                width = ((size_word & 1) << 8) | width_low;
                transition_step = (opening_word >> 3) & 0xF;
                scaled_width = width * transition_step;
                g_pad_input = 0;
                animated_width = scaled_width / 8;
                height = (size_word >> 1) & 0xFF;
                scaled_height = height * transition_step;
                animated_height = scaled_height / 8;
                remaining_height = (s32)(height - animated_height);

                packet_cursor =
                    ((AddheroElementDrawFunc)element->draw_handler)(ordering_table, packet_cursor, (s32)(width - animated_width) / 2, remaining_height / 2);
                {
                    u32 post_word;
                    u32 field;
                    u32 high;
                    post_word = element->attr.word;
                    field = (post_word >> 7) & 0x1FF;
                    high = post_word >> 24;
                    packet_cursor = (AddheroGpuPacket*)func_800AD850(packet_cursor, ordering_table,
                                                                     field + (s32)((((element->size.word & 1) << 8) | high) - animated_width) / 2,
                                                                     (element->attr.bytes.y) + ((s32)((element->size.word >> 1) & 0xFF) - animated_height) / 2,
                                                                     animated_width, animated_height, draw_state->frame_flag, element_index == 0);
                }
                {
                    u32 old_word;
                    u32 new_word;
                    old_word = element->attr.word;
                    new_word = (old_word & ~ADDHERO_ELEMENT_PHASE_MASK) | (((((old_word >> 3) & 0xF) + 1) & 0xF) * 8);
                    element->attr.word = new_word;
                    if (((new_word >> 3) & 0xF) == 8)
                    {
                        func_800AA02C();
                        element->attr.word = (element->attr.word & ~ADDHERO_ELEMENT_STATE_MASK) | ADDHERO_ELEMENT_STATE_ACTIVE;
                    }
                }
                break;

            case ADDHERO_ELEMENT_STATE_ACTIVE:
                packet_cursor = ((AddheroElementDrawFunc)element->draw_handler)(ordering_table, packet_cursor, 0, 0);
                {
                    u32 case_word;
                    u32 high;
                    case_word = element->attr.word;
                    high = case_word >> 24;
                    packet_cursor = (AddheroGpuPacket*)func_800AD850(packet_cursor, ordering_table, (case_word >> 7) & 0x1FF, element->attr.bytes.y,
                                                                     ((element->size.word & 1) << 8) | high, (element->size.word >> 1) & 0xFF,
                                                                     draw_state->frame_flag, element_index == 0);
                }
                updated_word = element->attr.word;
                if (((updated_word >> 3) & 0xF) != 0)
                {
                    element->attr.word = (updated_word & ~ADDHERO_ELEMENT_PHASE_MASK) | (((((updated_word >> 3) & 0xF) - 1) & 0xF) * 8);
                }
                break;

            case ADDHERO_ELEMENT_STATE_CLOSING:
                closing_word = element->attr.word;
                size_word = element->size.word;
                closing_scaled_width = (u32)closing_word >> 24;
                width = ((size_word & 1) << 8) | closing_scaled_width;
                closing_word = (u32)closing_word >> 3;
                closing_word &= 0xF;
                closing_scaled_width = width * closing_word;
                g_pad_input = 0;
                if (closing_scaled_width < 0)
                {
                    closing_scaled_width += 7;
                }
                closing_height = (size_word >> 1) & 0xFF;
                closing_scaled_height = closing_height * closing_word;
                animated_width = closing_scaled_width >> 3;
                animated_height = closing_scaled_height / 8;
                closing_remaining_height = (s32)(closing_height - animated_height);

                packet_cursor = ((AddheroElementDrawFunc)element->draw_handler)(ordering_table, packet_cursor, (s32)(width - animated_width) / 2,
                                                                                closing_remaining_height / 2);
                {
                    u32 post_word;
                    u32 field;
                    u32 high;
                    post_word = element->attr.word;
                    field = (post_word >> 7) & 0x1FF;
                    high = post_word >> 24;
                    packet_cursor = (AddheroGpuPacket*)func_800AD850(packet_cursor, ordering_table,
                                                                     field + (s32)((((element->size.word & 1) << 8) | high) - animated_width) / 2,
                                                                     (element->attr.bytes.y) + ((s32)((element->size.word >> 1) & 0xFF) - animated_height) / 2,
                                                                     animated_width, animated_height, draw_state->frame_flag, element_index == 0);
                }
                {
                    u32 old_word;
                    old_word = element->attr.word;
                    closing_scaled_width = old_word & ~ADDHERO_ELEMENT_PHASE_MASK;
                    old_word >>= 3;
                    old_word &= 0xF;
                    old_word--;
                    old_word &= 0xF;
                    old_word <<= 3;
                    closing_scaled_width |= old_word;
                    element->attr.word = closing_scaled_width;
                    if (!(((u32)closing_scaled_width >> 3) & 0xF))
                    {
                        element->attr.word = ((((u32)closing_scaled_width & ~ADDHERO_ELEMENT_PHASE_MASK) | 0x18) & ~ADDHERO_ELEMENT_STATE_MASK) |
                                             ADDHERO_ELEMENT_STATE_FINISHING;
                    }
                }
                break;

            case ADDHERO_ELEMENT_STATE_FINISHING:
                finishing_word = element->attr.word;
                g_pad_input = 0;
                updated_word = (finishing_word & ~ADDHERO_ELEMENT_PHASE_MASK) | (((((finishing_word >> 3) & 0xF) - 1) & 0xF) * 8);
                element->attr.word = updated_word;
                if (!((updated_word >> 3) & 0xF))
                {
                    element->attr.word = updated_word & ~ADDHERO_ELEMENT_STATE_MASK;
                }
                break;
            }
        }
    }

    draw_state->prim_cursor = packet_cursor;
}

/**
 * @brief Free the primary pool element by clearing its state bits.
 * @see decomp.me (100%)
 */
void addhero_deactivate_primary_element(void)
{
    g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
}

/**
 * @brief Append the multibyte string @p src onto the end of @p dst and
 *        null-terminate the result.
 * @param dst Destination string; appended to in place.
 * @param src Source string copied onto the end of @p dst.
 * @see decomp.me (100%)
 */
void addhero_text_append(u8* dst, u8* src)
{
    s32 dst_len;
    s32 src_len;
    s32 i;

    dst_len = addhero_text_byte_length(dst);
    src_len = addhero_text_byte_length(src);
    for (i = 0; i < src_len; i++)
    {
        dst[dst_len + i] = src[i];
    }
    dst[dst_len + i] = 0;
}

/**
 * @brief Measure the byte length of a string, counting characters in the
 *        0x19-0x1F lead range as two bytes.
 * @param str Null-terminated string to measure.
 * @return Length in bytes, excluding the terminator.
 * @see decomp.me (100%)
 */
s32 addhero_text_byte_length(u8* str)
{
    u8* p;
    u8 c;
    s32 len;

    p = str;
    c = *p;
    len = 0;
    while (c != 0)
    {
        if ((u32)(c - 0x19) < 7)
        {
            p += 2;
            len += 2;
        }
        else
        {
            p += 1;
            len += 1;
        }
        c = *p;
    }
    return len;
}

/**
 * @brief Copy a multibyte string, counting 0x19-0x1F lead bytes as two-byte
 *        characters when computing its length, and null-terminate the result.
 * @param dst Destination buffer.
 * @param src Source string to copy.
 * @see decomp.me (100%)
 */
void addhero_text_copy(u8* dst, u8* src)
{
    u8* p;
    u8 c;
    s32 len;
    s32 i;

    p = src;
    len = 0;
    while (*p != 0)
    {
        c = *(volatile u8*)p;
        if ((u32)(c - 0x19) < 7)
        {
            p += 2;
            len += 2;
        }
        else
        {
            p += 1;
            len += 1;
        }
    }
    for (i = 0; i < len; i++)
    {
        dst[i] = src[i];
    }
    dst[i] = 0;
}

/**
 * @brief Draw the load confirmation prompt and its yes/no choice, then act on
 *        input: cancel/back to the browser, or accept and swap this element to
 *        the load-progress bar.
 * @param ot   Ordering table the primitives are linked into.
 * @param prim Current primitive pointer/index.
 * @param x_offset Horizontal offset; screen X = 0x90 - x_offset.
 * @param y_offset Vertical offset applied to the prompt rows.
 * @return The updated primitive pointer after linking the prompt.
 * @see decomp.me (100%)
 */
s32 addhero_draw_load_prompt(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    s32 result;
    s32 x;
    s32 status;
    AddheroElement* p;

    x = -x_offset + 0x90;
    result = addhero_draw_choice_prompt(func_800A88A0(prim, ot, (u8*)&g_addhero_glyph_load_prompt + g_addhero_glyph_load_prompt - 0x30, 4, x, -y_offset, 2), ot,
                                        x, 0xE - y_offset);

    if ((u32)(addhero_poll_and_retry_card_info() - 1) < 2U)
    {
        g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
        func_800AA02C();
        play_menu_sfx(0x78, 0x80);
        g_addhero_entry_state = ADDHERO_ENTRY_STATE_IDLE;
        addhero_reset_entry_ranks();
        g_addhero_load_step = 0;
    }
    else
    {
        status = g_pad_input;
        if (status & 0x40)
        {
            g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
            func_800AA02C();
            play_menu_sfx(0x78, 0x80);
            g_addhero_load_step = g_addhero_loadseq_abort;
        }
        else if (status & 0x220)
        {
            if (g_addhero_choice_toggle != 0)
            {
                g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
                func_800AA02C();
                play_menu_sfx(0x78, 0x80);
                g_addhero_load_step = g_addhero_loadseq_abort;
            }
            else
            {
                play_menu_sfx(0x7E, 0x80);
                g_addhero_progress_active = 1;
                g_addhero_load_step = g_addhero_loadseq_load_begin;
                p = &g_addhero_element_pool[0];
                p->draw_handler = addhero_draw_load_progress;
                p->attr.bits.transition_step = 1;
                p->attr.bits.state = ADDHERO_ELEMENT_STATE_OPENING;
                p->attr.bits.x = 0x10;
                p->attr.bits.y = 0x61;
                p->size.bits.width_high = 1;
                p->size.bits.height = 0x2C;
                SET_ELEM_WIDTH_LOW(p, 0x20);
            }
        }
    }
    return result;
}

/**
 * @brief Draw the "loading new hero" prompt and, once the load has finished,
 *        validate and commit the freshly loaded save data.
 * @param ot   Ordering table the prompt primitives are linked into.
 * @param prim Current primitive pointer / index within the ordering table.
 * @param x_offset Horizontal offset used to place the prompt (screen X = 0x90 - x_offset).
 * @param y_offset Vertical offset used to place the prompt rows.
 * @return The updated primitive pointer / index after linking the prompt.
 * @see decomp.me (100%)
 */
s32 addhero_draw_load_progress(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    u8* base;
    u8* resource;
    AddheroElement* p;
    AddheroElement* cursor;
    s32 result;
    s32 x;
    s32 i;
    u32 saved;

    x = -x_offset + 0x90;
    result = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_load_progress, 0x32), 4, x, -y_offset, 2);
    base = (u8*)&g_addhero_glyph_load_progress - 0x32;
    result = func_800A88A0(result, ot, base + *(u16*)(base + 0x1E), 4, x, 0xE - y_offset, 2);
    result = func_800A88A0(result, ot, base + *(u16*)(base + 0xB2), 4, x, 0x1C - y_offset, 2);
    result = addhero_draw_progress_bar(result, ot);

    if (g_addhero_progress_active == 0)
    {
        resource = g_addhero_save_blob;
        p = (AddheroElement*)&g_addhero_element_pool[0];
        p->attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
        if (addhero_validate_save_blob(resource) == 0)
        {
            addhero_open_status_dialog(4);
            return result;
        }

        play_menu_sfx(0x7B, 0x80);
        saved = g_pad_ctx[0x858] >> 7;
        bcopy(resource + 0x770, g_pad_ctx + 0x840, 0x250);
        *(u32*)(g_pad_ctx + 0x858) = (*(u32*)(g_pad_ctx + 0x858) & ~0x80) | (saved << 7);
        *(u16*)(g_pad_ctx + 0xD8) = *(u16*)(resource + 0x254);
        *(u16*)(g_pad_ctx + 0xDA) = *(u16*)(resource + 0x256);
        *(u16*)(g_pad_ctx + 0xDE) = 1;
        field_restore_fade_target();

        cursor = p;
        for (i = 0; i < ADDHERO_ELEMENT_COUNT; i++, cursor++)
        {
            if (cursor->attr.bits.state != ADDHERO_ELEMENT_STATE_INACTIVE)
            {
                cursor->attr.bits.state = ADDHERO_ELEMENT_STATE_CLOSING;
                cursor->attr.bits.transition_step = 8;
            }
        }
        field_restore_fade_target_with_duration(8);
        g_addhero_result = 1;
    }

    return result;
}

/**
 * @brief Draw the gradient progress bar whose width tracks elapsed ticks, when
 *        the bar is active.
 * @param prim Current primitive pointer/index the POLY_G4 is written to.
 * @param ot   Ordering table the primitive is linked into.
 * @return The advanced primitive pointer, unchanged when the bar is inactive.
 * @see decomp.me (100%)
 */
s32 addhero_draw_progress_bar(s32 prim, s32* ot)
{
    POLY_G4* g;
    s32 elapsed;
    s32 extent;
    s32 color;

    g = (POLY_G4*)prim;
    if (g_addhero_progress_bar_active != 0)
    {
        elapsed = VSync(-1) - g_addhero_progress_start_tick;
        if (elapsed >= 0x101)
        {
            elapsed = 0x100;
        }
        color = 0xFFFF00;
        extent = elapsed * 0x120;
        SET_BGR0_PACKED(g, 0xFF);
        SET_POLY_G4_BGR1_PACKED(g, 0xFFFF);
        SET_POLY_G4_BGR3_PACKED(g, 0xFF0000);
        ((u8*)g)[3] = 8;
        SET_POLY_G4_BGR2_PACKED(g, color);
        g->code = 0x38;
        g->x2 = 0;
        g->x0 = 0;
        if (extent < 0)
        {
            extent += 0xFF;
        }
        g->x3 = extent >> 8;
        g->x1 = extent >> 8;
        g->y1 = 0;
        g->y0 = 0;
        g->y3 = 0x2C;
        g->y2 = 0x2C;
        addPrim(ot, g);
        prim += 0x24;
    }
    return prim;
}

/**
 * @brief Reconfigure the primary element as a modal status dialog and reset the
 *        browser/IO state, storing the dialog message id.
 * @param message_id Dialog message id stored in g_addhero_dialog_state.
 * @see decomp.me (100%)
 */
void addhero_open_status_dialog(s32 message_id)
{
    play_menu_sfx(0x78, 0x80);
    g_addhero_element_pool[0].draw_handler = (void*)addhero_draw_status_dialog;
    g_addhero_element_pool[0].attr.bits.transition_step = 1;
    g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_OPENING;
    g_addhero_element_pool[0].attr.bits.x = 0x20;
    g_addhero_element_pool[0].attr.bits.y = 0x70;
    g_addhero_element_pool[0].size.bits.width_high = 1;
    g_addhero_element_pool[0].size.bits.height = 0x14;
    SET_ELEM_WIDTH_LOW(&g_addhero_element_pool[0], 0);
    func_800AA02C();
    g_addhero_write_in_progress = 0;
    g_addhero_progress_active = 0;
    g_addhero_selection_status = 0;
    g_addhero_io_busy = 0;
    g_addhero_entry_state = ADDHERO_ENTRY_STATE_IDLE;
    addhero_reset_entry_ranks();
    g_addhero_load_step = 0;
    g_addhero_dialog_state = message_id;
}

/**
 * @brief Reconfigure the primary element as a modal exit dialog and reset the
 *        browser/IO state, storing the dialog message id.
 * @param message_id Dialog message id stored in g_addhero_dialog_state.
 * @see decomp.me (100%)
 */
void addhero_open_exit_dialog(s32 message_id)
{
    play_menu_sfx(0x78, 0x80);
    g_addhero_element1.draw_handler = (void*)addhero_draw_exit_dialog;
    g_addhero_element1.attr.bits.transition_step = 1;
    g_addhero_element1.attr.bits.state = ADDHERO_ELEMENT_STATE_OPENING;
    g_addhero_element1.attr.bits.x = 0x20;
    g_addhero_element1.attr.bits.y = 0x70;
    g_addhero_element1.size.bits.width_high = 1;
    g_addhero_element1.size.bits.height = 0x14;
    SET_ELEM_WIDTH_LOW(&g_addhero_element1, 0);
    func_800AA02C();
    g_addhero_write_in_progress = 0;
    g_addhero_progress_active = 0;
    g_addhero_selection_status = 0;
    g_addhero_io_busy = 0;
    addhero_reset_entry_ranks();
    g_addhero_load_step = 0;
    g_addhero_dialog_state = message_id;
}

/**
 * @brief Draw the status dialog message for the current dialog state and close
 *        the element once the player acknowledges it.
 * @param ot   Ordering table the message glyph is linked into.
 * @param prim Current primitive pointer/index.
 * @param x_offset Horizontal offset; screen X is derived from it.
 * @param y_offset Vertical offset.
 * @return The updated primitive pointer.
 * @see decomp.me (100%)
 */
s32 addhero_draw_status_dialog(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;

    switch (g_addhero_dialog_state)
    {
    case 0:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_dialog_msg0, 0x3C), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 2:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_dialog_msg2, 0x40), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 3:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_dialog_msg3, 0x42), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 1:
    case 4:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_dialog_msg1, 0x3E), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    }
    if (g_pad_input & 0x220)
    {
        g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
        func_800AA02C();
    }
    return prim;
}

/**
 * @brief Draw the exit dialog message and, on acknowledge, tear down all
 *        elements and request the overlay to exit with result 3.
 * @param ot   Ordering table the message glyph is linked into.
 * @param prim Current primitive pointer/index.
 * @param x_offset Horizontal offset; screen X is derived from it.
 * @param y_offset Vertical offset.
 * @return The updated primitive pointer.
 * @see decomp.me (100%)
 */
s32 addhero_draw_exit_dialog(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    AddheroElement* p;
    s32 i;

    switch (g_addhero_dialog_state)
    {
    case 0:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_dialog_msg0, 0x3C), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 2:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_dialog_msg2, 0x40), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 3:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_dialog_msg3, 0x42), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 1:
    case 4:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_dialog_msg1, 0x3E), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    }
    if (g_pad_input & 0x220)
    {
        g_addhero_result = 3;
        g_menu_element_counter = 0x20;
        p = &g_addhero_element_pool[0];
        for (i = 0; i < ADDHERO_ELEMENT_COUNT; i++)
        {
            p->size.word &= ~0x200;
            p->attr.word &= ~7;
            p++;
        }
        field_restore_fade_target_with_duration(8);
        func_800AA02C();
    }
    return prim;
}

/**
 * @brief Transfer-mode driver/renderer: draws the message for the current
 *        entry-state sentinel, runs the load/save confirm and progress steps,
 *        and handles cancel/back input.
 * @param ot   Ordering table the primitives are linked into.
 * @param prim Current primitive pointer/index.
 * @param x_offset Horizontal offset; screen X = 0x90 - x_offset.
 * @param y_offset Vertical offset applied to the message rows.
 * @return The updated primitive pointer.
 */
s32 addhero_draw_transfer_status(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;

    switch (g_addhero_entry_state)
    {
    case 0xF8:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_status_f8, 0x34), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case 0xF9:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_status_f8, 0x34), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case ADDHERO_ENTRY_STATE_IDLE:
    {
        s32 message_x;
        u8* glyph_base;

        message_x = -x_offset + 0x90;
        glyph_base = (u8*)&g_addhero_glyph_table;
        prim = func_800A88A0(prim, ot, glyph_base + g_addhero_glyph_table, 4, message_x, -y_offset, 2);
        prim = func_800A88A0(prim, ot, GLYPH_OFF(glyph_base, 0x1E), 4, message_x, 0xE - y_offset, 2);
        prim = func_800A88A0(prim, ot, GLYPH_OFF(glyph_base, 0xB2), 4, message_x, 0x1C - y_offset, 2);
    }
    break;
    case 0xFA:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_status_f8, 0x34), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case 0xFD:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_status_fd, 4), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case 0xFB:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_status_fb, 0x10), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case 0xFC:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_status_fc, 0x12), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case 0xF7:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_status_f7, 0x68), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case ADDHERO_ENTRY_STATE_LOAD_PROGRESS:
    {
        s32 message_x;
        u8* glyph_base;
        POLY_G4* bar;
        s32 next_prim;
        s32 elapsed_frames;
        s32 bar_extent;
        s32 bar_color;
        s32 dialog_state;

        message_x = -x_offset + 0x90;
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_load_progress, 0x32), 4, message_x, -y_offset, 2);
        glyph_base = (u8*)&g_addhero_glyph_load_progress - 0x32;
        prim = func_800A88A0(prim, ot, GLYPH_OFF(glyph_base, 0x1E), 4, message_x, 0xE - y_offset, 2);
        prim = func_800A88A0(prim, ot, GLYPH_OFF(glyph_base, 0xB2), 4, message_x, 0x1C - y_offset, 2);
        next_prim = prim;
        bar = (POLY_G4*)prim;
        if (g_addhero_progress_bar_active != 0)
        {
            elapsed_frames = VSync(-1) - g_addhero_progress_start_tick;
            if (elapsed_frames >= 0x101)
            {
                elapsed_frames = 0x100;
            }
            bar_color = 0xFFFF00;
            bar_extent = elapsed_frames * 0x120;
            SET_BGR0_PACKED(bar, 0xFF);
            SET_POLY_G4_BGR1_PACKED(bar, 0xFFFF);
            SET_POLY_G4_BGR3_PACKED(bar, 0xFF0000);
            setlen(bar, 8);
            SET_POLY_G4_BGR2_PACKED(bar, bar_color);
            setcode(bar, 0x38);
            bar->x2 = 0;
            bar->x0 = 0;
            if (bar_extent < 0)
            {
                bar_extent += 0xFF;
            }
            bar->x3 = bar_extent >> 8;
            bar->x1 = bar_extent >> 8;
            bar->y1 = 0;
            bar->y0 = 0;
            bar->y3 = 0x2C;
            bar->y2 = 0x2C;
            addPrim(ot, bar);
            next_prim = prim + 0x24;
        }
        prim = next_prim;
        if (g_addhero_progress_active == 0)
        {
            if (addhero_validate_save_blob(g_addhero_save_blob) == 0)
            {
                play_menu_sfx(0x78, 0x80);
                g_addhero_element_pool[0].draw_handler = (void*)addhero_draw_status_dialog;
                g_addhero_element_pool[0].attr.bits.transition_step = 1;
                g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_OPENING;
                g_addhero_element_pool[0].attr.bits.x = 0x20;
                g_addhero_element_pool[0].attr.bits.y = 0x70;
                g_addhero_element_pool[0].size.bits.width_high = 1;
                g_addhero_element_pool[0].size.bits.height = 0x14;
                SET_ELEM_WIDTH_LOW(&g_addhero_element_pool[0], 0);
                func_800AA02C();
                g_addhero_write_in_progress = 0;
                g_addhero_selection_status = 0;
                g_addhero_io_busy = 0;
                g_addhero_progress_active = 0;
                g_addhero_entry_state = ADDHERO_ENTRY_STATE_IDLE;
                addhero_reset_entry_ranks();
                dialog_state = 4;
                g_addhero_load_step = 0;
                g_addhero_dialog_state = dialog_state;
                return prim;
            }
            play_menu_sfx(0x7B, 0x80);
            g_addhero_entry_state = ADDHERO_ENTRY_STATE_SAVE_CONFIRM;
            addhero_enable_choice_toggle();
            func_800AA02C();
        }
    }
    break;
    case ADDHERO_ENTRY_STATE_CONFIRM_PROMPT:
    {
        s32 message_x;
        AddheroElement* element;
        s32 i;

        message_x = -x_offset + 0x90;
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_status_f3, 0x6E), 4, message_x, -y_offset, 2);
        prim = addhero_draw_choice_prompt(prim, ot, message_x, 0xE - y_offset);
        if (g_pad_input & PAD_BTN_CIRCLE)
        {
            play_menu_sfx(0x78, 0x80);
            addhero_enable_choice_toggle();
            g_addhero_entry_state = ADDHERO_ENTRY_STATE_SAVE_CONFIRM;
            func_800AA02C();
        }
        else if (g_pad_input & ADDHERO_CONFIRM_BUTTON_MASK)
        {
            if (g_addhero_choice_toggle != 0)
            {
                play_menu_sfx(0x78, 0x80);
                addhero_enable_choice_toggle();
                g_addhero_entry_state = ADDHERO_ENTRY_STATE_SAVE_CONFIRM;
                func_800AA02C();
            }
            else
            {
                play_menu_sfx(0x7D, 0x80);
                g_addhero_result = 3;
                g_menu_element_counter = 0x20;
                element = g_addhero_element_pool;
                for (i = 0; i < ADDHERO_ELEMENT_COUNT; i++, element++)
                {
                    element->size.bits.scrollable = 0;
                    element->attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
                }
                field_restore_fade_target_with_duration(8);
                func_800AA02C();
            }
        }
    }
    break;
    case ADDHERO_ENTRY_STATE_SAVE_CONFIRM:
    {
        s32 message_x;
        u8* base;
        s32 checksum;

        message_x = -x_offset + 0x90;
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_save_confirm_msg, 0x6A), 4, message_x, -y_offset, 2);
        base = (u8*)&g_addhero_glyph_save_confirm_msg - 0x6A;
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x6C), 4, message_x, 0xE - y_offset, 2);
        prim = addhero_draw_choice_prompt(prim, ot, message_x, 0x1C - y_offset);
        if (g_pad_input & PAD_BTN_CIRCLE)
        {
            addhero_enable_choice_toggle();
            g_addhero_entry_state = ADDHERO_ENTRY_STATE_CONFIRM_PROMPT;
            play_menu_sfx(0x78, 0x80);
            func_800AA02C();
        }
        else if (g_pad_input & ADDHERO_CONFIRM_BUTTON_MASK)
        {
            if (g_addhero_choice_toggle != 0)
            {
                addhero_enable_choice_toggle();
                g_addhero_entry_state = ADDHERO_ENTRY_STATE_CONFIRM_PROMPT;
                play_menu_sfx(0x78, 0x80);
                func_800AA02C();
            }
            else
            {
                play_menu_sfx(0x7E, 0x80);
                base = g_addhero_save_blob;
                bcopy(g_pad_ctx + 0x840, &((AddheroSaveBlob*)base)->context, sizeof(AddheroSaveContextBlock));
                ((AddheroSaveBlob*)base)->context.inject_flags |= ADDHERO_INPUT_INJECTION_ENABLED;
                checksum = addhero_compute_save_checksum(base);
                ((AddheroSaveBlob*)base)->magic = ADDHERO_SAVE_MAGIC;
                ((AddheroSaveBlob*)base)->checksum = checksum;
                g_addhero_write_in_progress = 1;
                g_addhero_load_step = &g_addhero_loadseq_save_begin;
                g_addhero_entry_state = ADDHERO_ENTRY_STATE_SAVE_PROGRESS;
            }
        }
    }
    break;
    case ADDHERO_ENTRY_STATE_SAVE_PROGRESS:
    {
        s32 message_x;
        u8* glyph_base;
        POLY_G4* bar;
        s32 next_prim;
        s32 elapsed_frames;
        s32 bar_extent;
        s32 bar_color;
        AddheroElement* element;
        s32 i;

        message_x = -x_offset + 0x90;
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_save_progress, 0x1C), 4, message_x, -y_offset, 2);
        glyph_base = (u8*)&g_addhero_glyph_save_progress - 0x1C;
        prim = func_800A88A0(prim, ot, GLYPH_OFF(glyph_base, 0x1E), 4, message_x, 0xE - y_offset, 2);
        prim = func_800A88A0(prim, ot, GLYPH_OFF(glyph_base, 0xB2), 4, message_x, 0x1C - y_offset, 2);
        next_prim = prim;
        bar = (POLY_G4*)prim;
        if (g_addhero_progress_bar_active != 0)
        {
            elapsed_frames = VSync(-1) - g_addhero_progress_start_tick;
            if (elapsed_frames >= 0x101)
            {
                elapsed_frames = 0x100;
            }
            bar_color = 0xFFFF00;
            bar_extent = elapsed_frames * 0x120;
            SET_BGR0_PACKED(bar, 0xFF);
            SET_POLY_G4_BGR1_PACKED(bar, 0xFFFF);
            SET_POLY_G4_BGR3_PACKED(bar, 0xFF0000);
            setlen(bar, 8);
            SET_POLY_G4_BGR2_PACKED(bar, bar_color);
            setcode(bar, 0x38);
            bar->x2 = 0;
            bar->x0 = 0;
            if (bar_extent < 0)
            {
                bar_extent += 0xFF;
            }
            bar->x3 = bar_extent >> 8;
            bar->x1 = bar_extent >> 8;
            bar->y1 = 0;
            bar->y0 = 0;
            bar->y3 = 0x2C;
            bar->y2 = 0x2C;
            addPrim(ot, bar);
            next_prim = prim + 0x24;
        }
        prim = next_prim;
        if (g_addhero_write_in_progress == 0)
        {
            g_pad_ctx[0x840] = 0;
            play_menu_sfx(0x7A, 0x80);
            g_menu_element_counter = 0x20;
            element = g_addhero_element_pool;
            for (i = 0; i < ADDHERO_ELEMENT_COUNT; i++, element++)
            {
                element->size.bits.scrollable = 0;
                element->attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
            }
            field_restore_fade_target_with_duration(8);
            g_addhero_result = 2;
        }
    }
    break;
    default:
    {
        s32 message_x;
        s32 selected_y;
        s32 scroll_delta;
        u8* glyph_base;

        message_x = -x_offset + 0x90;
        glyph_base = (u8*)&g_addhero_glyph_table;
        prim = func_800A88A0(prim, ot, glyph_base + g_addhero_glyph_table, 4, message_x, -y_offset, 2);
        prim = func_800A88A0(prim, ot, GLYPH_OFF(glyph_base, 0x1E), 4, message_x, 0xE - y_offset, 2);
        prim = func_800A88A0(prim, ot, GLYPH_OFF(glyph_base, 0xB2), 4, message_x, 0x1C - y_offset, 2);
        if (g_addhero_entry_scan_active == 0)
        {
            if (g_addhero_io_busy != 0)
            {
                return prim;
            }
            if ((u32)(*g_addhero_load_step - 6) < 2U)
            {
                return prim;
            }
            if ((strncmp(g_lom_save_filename_prefix, &g_addhero_entries[g_addhero_card_slot][g_addhero_selected_row], 0xC) != 0) ||
                (g_addhero_entry_identity != ((AddheroRecord*)g_pad_ctx)->identity))
            {
                g_addhero_selected_row++;
                if (g_addhero_selected_row >= g_addhero_entry_state)
                {
                    if (g_addhero_entry_state != 0)
                    {
                        g_addhero_entry_state = 0xF7;
                    }
                    else
                    {
                        g_addhero_entry_state = 0xF8;
                    }
                }
                else
                {
                    addhero_commit_selected_entry();
                    selected_y = g_addhero_selected_row * 0xE;
                    scroll_delta = selected_y - g_addhero_scroll_y;
                    if (scroll_delta >= 0x4B)
                    {
                        g_addhero_scroll_target_y = selected_y - 0x46;
                        g_addhero_scroll_frames = 4;
                    }
                    if (scroll_delta < 0)
                    {
                        g_addhero_scroll_target_y = selected_y;
                        g_addhero_scroll_frames = 4;
                    }
                }
            }
            else
            {
                g_addhero_progress_start_tick = VSync(-1);
                g_addhero_progress_active = 1;
                g_addhero_load_step = &g_addhero_loadseq_load_progress;
                g_addhero_entry_state = ADDHERO_ENTRY_STATE_LOAD_PROGRESS;
            }
        }
    }
    break;
    case 0xFE:
        break;
    }

    if (g_addhero_io_busy != 0)
    {
        return prim;
    }
    if (g_addhero_entry_state == ADDHERO_ENTRY_STATE_LOAD_PROGRESS)
    {
        return prim;
    }
    if (g_addhero_entry_state == ADDHERO_ENTRY_STATE_SAVE_PROGRESS)
    {
        return prim;
    }
    if (g_addhero_entry_state == ADDHERO_ENTRY_STATE_SAVE_CONFIRM)
    {
        return prim;
    }
    if (g_addhero_entry_state == ADDHERO_ENTRY_STATE_CONFIRM_PROMPT)
    {
        return prim;
    }

    if (g_pad_input & PAD_BTN_CIRCLE)
    {
        AddheroElement* element;
        s32 i;
        s32 attr;

        D_80122718 = 3;
        play_menu_sfx(0x78, 0x80);
        field_restore_fade_target();
        element = g_addhero_element_pool;
        i = 0;
        do
        {
            attr = element->attr.word;
            if (attr & ADDHERO_ELEMENT_STATE_MASK)
            {
                element->attr.word = (((attr & ~ADDHERO_ELEMENT_STATE_MASK) | ADDHERO_ELEMENT_STATE_CLOSING) & ~ADDHERO_ELEMENT_PHASE_MASK) | 0x40;
            }
            i++;
            element++;
        } while (i < ADDHERO_ELEMENT_COUNT);
        return prim;
    }

    if ((g_pad_input & ADDHERO_CARD_SWITCH_BUTTON_MASK) && (g_addhero_entry_state != ADDHERO_ENTRY_STATE_IDLE))
    {
        play_menu_sfx(0x7D, 0x80);
        g_addhero_load_flow_active = 0;
        g_addhero_load_step = 0;
        g_addhero_scroll_frames = 0;
        g_addhero_scroll_target_y = 0;
        g_addhero_scroll_y = 0;
        g_addhero_selected_row = 0;
        g_addhero_entry_state = ADDHERO_ENTRY_STATE_IDLE;
        g_addhero_selection_status = 0;
        g_addhero_card_slot ^= 1;
        addhero_reset_entry_ranks();
        func_800AA02C();
        g_addhero_progress_bar_active = 0;
        g_pad_input = 0;
        g_addhero_load_step = &g_addhero_loadseq_start;
    }
    return prim;
}

/**
 * @brief Blit one character-slot icon into VRAM and emit the highlighted
 *        textured quad for it in the detail panel.
 * @param result Current primitive pointer/index the POLY_FT4 is written to.
 * @param ot     Ordering table the quad is linked into.
 * @param x      Left edge of the quad.
 * @param y      Top edge of the quad.
 * @param adjust Width added to the base quad for the highlight animation.
 * @param slot   Character/icon id for this position (0x7F means empty).
 * @param visible_index Index among the present (non-empty) slots.
 * @param party_index Index among all three slots.
 * @return The advanced primitive pointer (result + 0x28), or @p result when the
 *         slot is empty.
 * @see decomp.me (100%)
 */
s32 addhero_draw_icon_highlight(s32 result, s32* ot, s32 x, s32 y, s32 adjust, s32 slot, s32 visible_index, s32 party_index)
{
    RECT rect;
    POLY_FT4* icon;
    s32 icon_column;
    s8 u;

    if (slot == ADDHERO_NO_ICON)
    {
        return result;
    }
    rect.x = visible_index * 0x10;
    rect.y = VRAM_CLUT_Y;
    rect.w = 0x10;
    rect.h = 1;
    if ((party_index == 1) && (slot < 2))
    {
        func_800A5638(g_addhero_icon_context, slot);
        func_80019A34(&rect, g_addhero_icon_context);
        func_80019788(0);
    }
    else if (slot >= 0x4F)
    {
        func_800A55E4(g_addhero_icon_context, g_addhero_icon_palette);
        func_80019A34(&rect, g_addhero_icon_context);
        func_80019788(0);
    }
    else
    {
        func_80019A34(&rect, (void*)((u8*)&g_addhero_icon_image_table - 4 + g_addhero_icon_image_table[slot]));
    }
    icon_column = visible_index * 3;
    rect.x = icon_column * 4 + SCREEN_WIDTH;
    rect.y = 0xD0;
    rect.w = 0xC;
    rect.h = 0x30;
    func_80019A34(&rect, (void*)((u8*)&g_addhero_icon_image_table + 0x1C + g_addhero_icon_image_table[slot]));
    icon = (POLY_FT4*)result;
    SET_BGR0_PACKED(icon, GPU_TINT_NEUTRAL);
    setlen(icon, 9);
    icon->code = 0x2C;
    icon->x2 = x;
    icon->x0 = x;
    icon->y1 = y;
    icon->y0 = y;
    icon->x3 = x + adjust;
    u = icon_column * 0x10;
    icon->u2 = u;
    icon->u0 = u;
    u += 0x2F;
    icon->u3 = u;
    icon->u1 = u;
    icon->v1 = 0xD0;
    icon->v0 = 0xD0;
    icon->x1 = x + adjust;
    icon->y3 = y + 0x2F;
    icon->y2 = y + 0x2F;
    icon->v3 = 0xFF;
    icon->v2 = 0xFF;
    icon->clut = (visible_index & 0x3F) | 0x7C80;
    icon->tpage = 5;
    addPrim(ot, result);
    return result + sizeof(POLY_FT4);
}

/**
 * @brief Select the second (cancel) option as the default choice.
 * @see decomp.me (100%)
 */
void addhero_enable_choice_toggle(void)
{
    g_addhero_choice_toggle = 1;
}

/**
 * @brief Draw the two-option (yes/no) choice glyphs, highlighting the current
 *        selection, and flip the selection on left/right pad input.
 * @param prim Current primitive pointer/index.
 * @param ot   Ordering table the glyphs are linked into.
 * @param x    Center X the two options are placed around.
 * @param y    Baseline Y for both options.
 * @return The updated primitive pointer.
 * @see decomp.me (100%)
 */
s32 addhero_draw_choice_prompt(s32 prim, s32* ot, s32 x, s32 y)
{
    u8* p;
    u8* base;
    s32 g1;
    s32 g2;
    s32 hi;
    s32 a3;

    p = (u8*)&g_text_choice_glyph_offsets;
    hi = p[1] << 8;
    base = p - 0x36;
    a3 = 4;
    g1 = p[0] + (hi + (s32)base);
    if (g_addhero_choice_toggle != 0)
    {
        a3 = 5;
    }
    prim = func_800A88A0(prim, ot, (void*)g1, a3, x - 0x10, y, 1);
    a3 = 4;
    g2 = base[0x38] + ((base[0x39] << 8) + (s32)base);
    if (g_addhero_choice_toggle == 0)
    {
        a3 = 5;
    }
    prim = func_800A88A0(prim, ot, (void*)g2, a3, x + 8, y, 0);
    if (g_pad_input & 0xA000)
    {
        g_addhero_choice_toggle ^= 1;
        play_menu_sfx(0x7D, 0x80);
        g_pad_input = 0;
    }
    return prim;
}

/**
 * @brief Validate a loaded save blob by checking its stored checksum and the
 *        "ANA" magic tag.
 * @param base Base of the 0x4000-byte save blob.
 * @return 1 when the checksum and magic both match, 0 otherwise.
 */
s32 addhero_validate_save_blob(u8* base)
{
    AddheroSaveBlob* save;

    save = (AddheroSaveBlob*)base;
    if (save->checksum == addhero_compute_save_checksum(base))
    {
        if (save->magic == ADDHERO_SAVE_MAGIC)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Compute the save-blob checksum over the first 0x33E0 bytes.
 * @param data Base of the save blob.
 * @return Twice the byte sum plus ADDHERO_SAVE_CHECKSUM_BIAS.
 */
s32 addhero_compute_save_checksum(u8* data)
{
    s32 sum;
    u32 i;
    u8* p;

    p = data;
    sum = 0;
    i = 0;
    do
    {
        i++;
        sum += *p;
        p++;
    } while (i < ADDHERO_SAVE_CHECKSUM_BYTES);
    return (sum * 2) + ADDHERO_SAVE_CHECKSUM_BIAS;
}

/**
 * @brief Format @p value as a big-endian double-byte decimal glyph string,
 *        suppressing leading zeros; emits a fixed overflow string past 999999.
 * @param out Destination glyph buffer.
 * @param value Value to format.
 * @return Pointer to the terminator written after the last glyph.
 */
s8* addhero_format_decimal(s8* out, s32 value)
{
    s32 digit;
    s32 divisor;
    s32 started;
    s8* p;

    p = out;
    divisor = 100000;
    if (value >= divisor * 10)
    {
        *(AddheroOverflowGlyphString*)p = *(AddheroOverflowGlyphString*)g_addhero_decimal_overflow_glyphs;
        return p + 6;
    }

    started = 0;
    do
    {
        digit = value / divisor;
        if (digit != 0 || started != 0)
        {
            *p++ = (digit + 0x824F) >> 8;
            *p++ = digit + 0x4F;
            started = 1;
        }
        if (divisor == 1)
        {
            break;
        }
        if (divisor == 10)
        {
            started = 1;
        }
        value -= digit * divisor;
        divisor /= 10;
    } while (1);
    *p = 0;
    return p;
}

/**
 * @brief Format @p value as an ASCII hex string of up to @p max_chars digits,
 *        suppressing leading zeros, and null-terminate it.
 * @param out       Destination character buffer.
 * @param value     Value to format.
 * @param max_chars Maximum number of hex digits to emit.
 */
void addhero_format_hex(s8* out, s32 value, s32 max_chars)
{
    s32 nibble;
    s32 shift_index;
    s32 started;

    shift_index = 7;
    started = 0;
    if (max_chars != 0)
    {
        do
        {
            nibble = (value >> (shift_index * 4)) & 0xF;
            if (nibble != 0 || started != 0)
            {
                addhero_hex_nibble_to_ascii(out, nibble);
                out++;
                max_chars--;
                started = 1;
                value -= nibble << (shift_index * 4);
            }
            shift_index--;
            if (shift_index == -1)
            {
                break;
            }
            if (shift_index == 0)
            {
                started = 1;
            }
        } while (max_chars);
    }
    *out = 0;
}

/**
 * @brief Write one nibble as its ASCII hex digit ('0'-'9', 'A'-'F'), or '_' for
 *        out-of-range values.
 * @param out   Destination byte.
 * @param value Nibble value to convert.
 * @see decomp.me (100%)
 */
void addhero_hex_nibble_to_ascii(s8* out, s32 value)
{
    if (value < 10)
    {
        *out = value + 0x30;
    }
    else if (value < 16)
    {
        *out = value + 0x37;
    }
    else
    {
        *out = 0x5F;
    }
}

/**
 * @brief Parse up to @p len leading hex digits from @p s into an integer.
 * @param s   Text to parse.
 * @param len Maximum number of hex digits to consume.
 * @return The parsed value; 0 when no hex digits are present.
 * @see decomp.me (100%)
 */
u32 addhero_parse_hex(u8* s, s32 len)
{
    u32 result;

    result = 0;
    while (((u8)(*s - '0') < 10) || ((u8)(*s - 'a') < 6) || ((u8)(*s - 'A') < 6))
    {
        if (len == 0)
        {
            break;
        }
        result <<= 4;
        if ((u8)(*s - '0') < 10)
        {
            u32 decimal_base;

            decimal_base = result - '0';
            result = decimal_base + *s;
        }
        else if ((u8)(*s - 'A') < 6)
        {
            u32 uppercase_base;

            uppercase_base = result - ('A' - 10);
            result = uppercase_base + *s;
        }
        else if ((u8)(*s - 'a') < 6)
        {
            u32 lowercase_base;

            lowercase_base = result - ('a' - 10);
            result = lowercase_base + *s;
        }
        s++;
        len--;
    }
    return result;
}

/**
 * @brief Skip the leading hex-digit run of a field, then parse the next two hex
 *        digits (the suffix byte) that follow it.
 * @param text Field text to scan.
 * @return The parsed two-digit suffix byte value.
 * @see decomp.me (100%)
 */
s32 addhero_parse_hex_suffix_byte(u8* text)
{
    s32 count;
    u32 result;

    while ((u32)(*text - '0') < 10 || (u32)(*text - 'a') < 6 || (u32)(*text - 'A') < 6)
    {
        text++;
    }
    text++;
    count = 2;
    result = 0;
    while (((u8)(*text - '0') < 10) || ((u8)(*text - 'a') < 6) || ((u8)(*text - 'A') < 6))
    {
        if (count == 0)
        {
            break;
        }
        result <<= 4;
        if ((u8)(*text - '0') < 10)
        {
            u32 decimal_base;

            decimal_base = result - '0';
            result = decimal_base + *text;
        }
        else if ((u8)(*text - 'A') < 6)
        {
            u32 uppercase_base;

            uppercase_base = result - ('A' - 10);
            result = uppercase_base + *text;
        }
        else if ((u8)(*text - 'a') < 6)
        {
            u32 lowercase_base;

            lowercase_base = result - ('a' - 10);
            result = lowercase_base + *text;
        }
        text++;
        count--;
    }
    return result;
}

/**
 * @brief Parse the hex value suffix of every "SD"-tagged directory entry on the
 *        active card, recording per-entry field values and their ranked bytes.
 * @return The maximum suffix byte value seen across all matching entries.
 * @see decomp.me (100%) https://decomp.me/scratch/7hY8R
 */
s32 addhero_parse_entry_fields(void)
{
    s32 entry_index;
    s32 max_suffix;
    u8* cursor;
    u8* suffix;
    s32 digits_left;
    s32 value;
    s32 suffix_value;
    s32* fields;

    max_suffix = 0;

    for (entry_index = 0; entry_index < g_addhero_entry_state; entry_index++)
    {
        char* prefix;
        char* entry_name;
        prefix = g_lom_save_filename_prefix;
        entry_name = g_addhero_entries[g_addhero_card_slot][entry_index].name;

        if (strncmp(prefix, entry_name, ADDHERO_SAVE_FILENAME_PREFIX_LENGTH) == 0)
        {
            cursor = (u8*)(g_addhero_card_slot * ADDHERO_CARD_DIRECTORY_BYTES + ((entry_index << 4) + (entry_index << 4) + (entry_index << 3)) +
                           (s32)g_addhero_entries + ADDHERO_SAVE_FILENAME_PREFIX_LENGTH);

            for (digits_left = 5, value = 0; (((u8)(*cursor - '0') < 10) || ((u8)(*cursor - 'a') < 6) || ((u8)(*cursor - 'A') < 6)) && digits_left != 0;
                 cursor++, digits_left--)
            {
                value <<= 4;

                if ((u8)(*cursor - '0') < 10)
                {
                    u32 decimal_base;

                    decimal_base = value - '0';
                    value = decimal_base + *cursor;
                }
                else if ((u8)(*cursor - 'A') < 6)
                {
                    u32 uppercase_base;

                    uppercase_base = value - ('A' - 10);
                    value = uppercase_base + *cursor;
                }
                else if ((u8)(*cursor - 'a') < 6)
                {
                    u32 lowercase_base;

                    lowercase_base = value - ('a' - 10);
                    value = lowercase_base + *cursor;
                }
            }

            suffix = (u8*)&g_addhero_entries[g_addhero_card_slot][entry_index].name[ADDHERO_SAVE_FILENAME_PREFIX_LENGTH];
            fields = &g_addhero_entry_fields[g_addhero_card_slot * ADDHERO_DIRECTORY_ENTRY_COUNT];
            fields[entry_index] = value;

            suffix_value = addhero_parse_hex_suffix_byte(suffix);
            g_addhero_entry_suffix_values[entry_index] = suffix_value;

            if (max_suffix < suffix_value)
            {
                max_suffix = suffix_value;
            }
        }
        else
        {
            s32* fields = &g_addhero_entry_fields[g_addhero_card_slot * ADDHERO_DIRECTORY_ENTRY_COUNT];
            fields[entry_index] = -1;
            g_addhero_entry_suffix_values[entry_index] = 0;
        }
    }

    return max_suffix;
}

/**
 * @brief Rank the current card's entries by parsed field value, tag "full"
 *        entries, and pick the highest-valued entry to select.
 * @param unused0 Unused; kept for the original signature.
 * @param unused1 Unused; kept for the original signature.
 * @param unused2 Unused; kept for the original signature.
 * @return Index of the highest-valued entry.
 * @see decomp.me (100%)
 */
s32 addhero_rank_entries(s32 unused0, s32 unused1, s32 unused2)
{
    s32* card_values;
    s32* entry_value;
    s32* rank_ptr;
    s32* previous_value;
    s32* previous_rank;
    s32* base_rank;
    s32* current_value;
    s32* max_ptr;
    s32* field_base;
    s32* field_table;
    s32 slot;
    s32* out_ptr;
    char* entry_name;
    s32 rank_value;
    s32 entry_index;
    s32 selection;
    s32 count;
    s32 maximum_suffix;
    s32 greater_count;
    s32 previous_index;

    addhero_parse_entry_fields();
    selection = -1;
    addhero_sort_entries_by_type();
    entry_index = 0;
    maximum_suffix = addhero_parse_entry_fields();
    addhero_reset_entry_ranks();
    rank_value = 1;
    if (g_addhero_entry_state > 0)
    {
        count = g_addhero_entry_state;
        base_rank = &g_addhero_entry_ranks[0];
        rank_ptr = base_rank;
        slot = g_addhero_card_slot;
        field_table = g_addhero_entry_fields;
        card_values = field_table + slot * ADDHERO_DIRECTORY_ENTRY_COUNT;
        entry_value = card_values;
        do
        {
            if (*entry_value >= 0)
            {
                previous_index = 0;
                if (entry_index > 0)
                {
                    previous_index += 1;
                    previous_index -= 1;
                }
                if (*entry_value >= selection)
                {
                    *rank_ptr = rank_value;
                    selection = *entry_value;
                    rank_value += 1;
                }
                else
                {
                    greater_count = previous_index;
                    if (entry_index > 0)
                    {
                        current_value = entry_value;
                        previous_rank = base_rank;
                        previous_value = card_values;
                        do
                        {
                            if (*current_value < *previous_value)
                            {
                                greater_count += 1;
                                *previous_rank += 1;
                            }
                            previous_rank += 1;
                            previous_index += 1;
                            previous_value += 1;
                        } while (previous_index < entry_index);
                    }
                    {
                        s32 assigned_rank;
                        do
                        {
                            do
                            {
                                do
                                {
                                    assigned_rank = rank_value - greater_count;
                                } while (0);
                            } while (0);
                        } while (0);
                        *rank_ptr = assigned_rank;
                    }
                    rank_value += 1;
                }
            }
            rank_ptr += 1;
            entry_index += 1;
            entry_value += 1;
        } while (entry_index < count);
    }
    previous_value = base_rank;
    previous_rank = card_values;
    g_addhero_rank_count = rank_value;
    rank_value = -1;
    entry_index = 0;
    selection = 0;
    if (g_addhero_entry_state > 0)
    {
        s32 max_count;
        max_count = g_addhero_entry_state;
        slot = g_addhero_card_slot;
        field_base = g_addhero_entry_fields;
        max_ptr = (s32*)((slot * 0x50) + (s32)field_base);
        do
        {
            if (rank_value < *max_ptr)
            {
                rank_value = *max_ptr;
                selection = entry_index;
            }
            entry_index += 1;
            max_ptr += 1;
        } while (entry_index < max_count);
        entry_index = 0;
    }
    g_addhero_entry_value_limit = rank_value + 1;
    if (g_addhero_entry_state > 0)
    {
        out_ptr = &g_addhero_entry_suffix_values[0];
        entry_name = (char*)&g_addhero_entries[0];
    loop_20:
        if (strncmp(&g_new_save_entry_prefix[0], (void*)((g_addhero_card_slot * ADDHERO_CARD_DIRECTORY_BYTES) + (s32)entry_name), 8) == 0)
        {
            *out_ptr = maximum_suffix + 1;
        }
        else
        {
            out_ptr += 1;
            entry_name += 0x28;
            entry_index += 1;
            if (entry_index < g_addhero_entry_state)
            {
                goto loop_20;
            }
        }
    }
    return selection;
}

/**
 * @brief Reset the 15 per-entry rank slots to -1 and the rank count to 0x28.
 * @see decomp.me (100%)
 */
void addhero_reset_entry_ranks(void)
{
    s32 i;
    s32 val;

    g_addhero_rank_count = 0x28;
    val = -1;
    for (i = 14; i >= 0; i--)
    {
        g_addhero_entry_ranks[i] = val;
    }
}

/**
 * @brief Test whether the active card holds at least one entry matching a known
 *        save-name prefix.
 * @return 1 if a known-type entry exists, 0 otherwise.
 * @see decomp.me (100%)
 */
s32 addhero_has_known_entry_type(void)
{
    s32 entry_index;

    for (entry_index = 0; entry_index < g_addhero_entry_state; entry_index++)
    {
        if (strncmp(g_lom_save_filename_prefix, g_addhero_entries[g_addhero_card_slot][entry_index].name, ADDHERO_SAVE_FILENAME_PREFIX_LENGTH) == 0 ||
            strncmp(g_lom_alt_save_filename_prefix, g_addhero_entries[g_addhero_card_slot][entry_index].name, ADDHERO_SAVE_FILENAME_PREFIX_LENGTH) == 0)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Sum the block usage of the active card's entries and test whether it
 *        has reached the card's capacity.
 * @return 1 when total used blocks are >= 0xE, 0 otherwise.
 * @see decomp.me (100%)
 */
s32 addhero_entry_blocks_reach_limit(void)
{
    s32 entry_index;
    s32 used_blocks;

    used_blocks = 0;
    for (entry_index = 0; entry_index < g_addhero_entry_state; entry_index++)
    {
        used_blocks += g_addhero_entries[g_addhero_card_slot][entry_index].size / ADDHERO_CARD_BLOCK_BYTES;
    }
    return used_blocks >= ADDHERO_USED_BLOCK_LIMIT;
}

/**
 * @brief Remove both placeholder save filenames from the active card.
 * @see decomp.me (100%)
 */
void addhero_erase_placeholder_files(void)
{
    AddheroProbeFilePath buf;

    memcpy(&buf, &g_addhero_file_template, 6);
    buf.device.characters.slot += *(u8*)&g_addhero_card_slot;
    strcat(&buf, &g_lom_save_dummy_filename);
    erase(&buf);

    memcpy(&buf, &g_addhero_file_template, 6);
    buf.device.characters.slot += *(u8*)&g_addhero_card_slot;
    strcat(&buf, &g_lom_alt_save_dummy_filename);
    erase(&buf);
}
