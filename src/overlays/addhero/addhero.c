#include "common.h"
#include "vector.h"
#include "display.h"
#include "gpu_packet.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

/** @brief One 0xC-byte animated ADDHERO UI element. */
typedef struct AddheroElement
{
    union
    {
        u32 word;
        struct
        {
            u32 state : 3;
            u32 phase : 4;
            u32 x : 9;
            u32 code : 8;
        } f;
    } attr;
    u32 active : 1;
    u32 y : 8;
    u32 rest : 23;
    void *draw;
} AddheroElement;

/**
 * @brief Prefix view of the eight-element pool.
 *
 * The historical code addresses the first element as the pool base and also
 * reads the second element's state word at +0xC. Keeping that overlap explicit
 * preserves GCC 2.7.2's original address/signedness behavior without pretending
 * each element is 0x10 bytes.
 */
typedef struct
{
    AddheroElement first;
    s32 second_state;
} AddheroElementPoolHead;

/** @brief Plain 3-word (0xC) view of a menu element. */
typedef struct
{
    s32 state_word;
    s32 size_flags;
    s32 draw;
} AddheroPacket;

typedef struct
{
    u32 word;
} AddheroAttrWord;

/** @brief attr-word view of a menu element used by the claim scan. */
typedef struct
{
    AddheroAttrWord attr;
    u32 unk4;
    u32 unk8;
} AddheroWordPacket;

/** @brief Two-element window over g_addhero_element_pool.first used by addhero_update_and_draw_elements. */
typedef struct
{
    AddheroWordPacket first;
    AddheroWordPacket second;
} AddheroPacketBlock;

typedef struct AddheroRecord
{
    u8 pad0[0x17];
    u8 marker_17;
    u8 pad18[0xCF - 0x18];
    u8 owner_id;
    u8 padD0[4];
    u16 hero_id;
    u16 reserved_d6;
} AddheroRecord;

/** @brief Memory-card directory entry; layout matches Psy-Q DIRENTRY (0x28 bytes). */
typedef struct
{
    char name[20];
    s32 attr;
    s32 size;
    void *next;
    s32 head;
    char system[4];
} AddheroDirEntry;

/* ADDHERO layout/state constants recovered from the element and card-directory loops. */
#define ADDHERO_ELEMENT_COUNT 8
#define ADDHERO_ELEMENT_WORD_STRIDE 3
#define ADDHERO_ELEMENT_STATE_MASK 7
#define ADDHERO_ELEMENT_PHASE_MASK 0x78
#define ADDHERO_CARD_DIRECTORY_BYTES 0x320
#define ADDHERO_DIRECTORY_ENTRY_BYTES 0x28
#define ADDHERO_ENTRY_ROW_HEIGHT 14
#define ADDHERO_NO_ICON 0x7F

/*
 * g_addhero_entry_state is dual-purpose:
 *
 *   0x00-0x0F  Number of save entries loaded from the current card. A PSX
 *              memory card holds 15 directory blocks, so the count never
 *              reaches 0x10; the value is used directly as a count/index
 *              while scanning and browsing the entry list.
 *   0xF3-0xFF  Modal state-machine sentinel. The `state >= 0x10` test tells a
 *              sentinel apart from a live entry count.
 *
 * Only the sentinels whose role is unambiguous from the control flow are named
 * below. The remaining sentinels are message/status screens whose exact meaning
 * depends on the (binary-resident) message resource each one draws, so they are
 * left as raw values until identified:
 *   0xF3  companion prompt page to SAVE_CONFIRM (navigates to it or cancels)
 *   0xF7  drawn when the selected entry is not found while advancing the list
 *   0xF8/0xF9  mode-dependent card read/scan failure notice
 *   0xFA/0xFB/0xFC/0xFD  status/result notices
 *   0xFE  no-op (draws nothing)
 */
#define ADDHERO_ENTRY_STATE_IDLE          0xFF  /* browsing / reset; no operation active */
#define ADDHERO_ENTRY_STATE_LOAD_PROGRESS 0xF6  /* importing a matched entry; progress bar */
#define ADDHERO_ENTRY_STATE_SAVE_PROGRESS 0xF5  /* write in progress; progress bar */
#define ADDHERO_ENTRY_STATE_SAVE_CONFIRM  0xF4  /* confirm dialog; accepting writes the save */

typedef struct AddheroFallbackText
{
    u8 pad[0x24];
    u8 text[0x20];
} AddheroFallbackText;

/* addhero_update_and_draw_elements element-draw pipeline types */
typedef struct
{
    s32 tag;
    s32 word4;
    s16 x0;
    s16 y0;
    s16 unkC;
    u16 unkE;
} AddheroGpuPacket;

typedef struct
{
    s32 tag;
    u8 pad4[0x40AE];
    s16 frame_flag;
    u8 pad40B4[4];
    AddheroGpuPacket *prim_cursor;
} AddheroDrawState;

typedef AddheroGpuPacket *(*AddheroElemDrawFunc)();

extern AddheroElementPoolHead g_addhero_element_pool;
extern AddheroElement g_addhero_element1;
extern AddheroDirEntry g_addhero_entries[][20];
extern AddheroRecord g_addhero_entry_metadata;
/* Shared controller/game context (main.h PadContext); addressed here as a byte
   buffer for the save-blob copies and metadata reads. */
extern u8 *g_pad_ctx;
extern u8 *g_addhero_load_step;
extern void *jtbl_80140098[];

extern s32 D_8003EC9C;
extern s32 D_80122718;
extern s32 g_pad_input;
extern s32 D_8012298C;
extern s32 D_80160580;
extern s32 g_addhero_icon_phase;
extern s32 D_80160924;
extern s32 g_addhero_scroll_y;
extern s32 g_addhero_result;
extern s32 D_80160930;
extern s32 g_addhero_progress_active;
extern s32 g_addhero_scroll_target_y;
extern s32 g_addhero_mode;
extern s32 g_addhero_exit_requested;
extern s32 g_addhero_entry_state;
extern s32 g_addhero_card_slot;
extern s32 g_addhero_selected_row;
extern s32 g_addhero_choice_toggle;
extern s32 D_801609B4;
extern s32 g_addhero_selection_status;
extern s32 g_addhero_scroll_frames;
extern s32 g_addhero_icon_palette;
extern s32 g_addhero_frame_parity;
extern s32 g_addhero_dialog_state;
extern s32 g_addhero_io_busy;
extern s32 g_addhero_progress_bar_active;
extern s32 g_addhero_progress_start_tick;
extern s32 g_addhero_entry_scan_active;
extern s32 g_addhero_entry_identity;
extern s32 g_addhero_write_in_progress;
extern s32 g_addhero_rank_count;
extern s32 D_80147658[];
extern s32 g_addhero_entry_suffix_values[];
extern s32 g_addhero_entry_ranks[];

extern u8 D_80160574;
extern u8 D_80160588[];
extern u8 D_80160590[];
extern u8 D_80160598;
extern u8 D_801605A1;
extern u8 g_addhero_icon_context[];
extern u8 g_addhero_save_blob[];
extern u8 D_80165208;
extern u8 D_8016520C;
extern u8 g_addhero_entry_owner_id;
extern u8 D_800EC3F6[2];
extern u8 D_800EC3FA[];

extern char D_800ECF7C[];
extern char D_800ECF8C[];
extern char D_800ECFC4[];

extern u16 D_80146FA4;
extern u16 D_80146FA6;
extern u16 D_80146FA8;
extern u16 D_80146FAA;
extern u16 D_80146FAC;
extern u16 D_80146FB0;
extern u16 D_80146FB2;
extern u16 D_80146FB4;
extern u16 D_80146FB6;
extern u16 D_80146FB8;
extern u16 D_80146FC0;
extern u16 D_80146FCC;
extern u16 D_80146FD2;
extern u16 D_80146FD4;
extern u16 D_80146FD6;
extern u16 D_80146FD8;
extern u16 D_80146FDE;
extern u16 D_80146FE0;
extern u16 D_80146FE2;
extern u16 D_80146FE4;
extern u16 D_80146FE6;
extern u16 D_80146FE8;
extern u16 D_80146FEA;
extern u16 D_80146FF4;
extern u16 D_80146FF8;
extern u16 D_80147012;
extern u16 D_8014700C;
extern u16 D_8014700E;
extern u16 D_80147054;
extern u16 D_80147470[];

/* In-file functions */
void addhero_init(s32 work_base, s32 mode);
s32 addhero_state_step(s32 render_half);
void addhero_build_ui_elements(void);
void addhero_update_state();
s32 addhero_update_load_sequence(void);
s32 addhero_handle_input(void);
void addhero_reset_state(void);
void addhero_close_all_elements(void);
void addhero_scroll_to_selection(void);
void addhero_update_elements(void);
s32 addhero_draw_entry_list(s32 *ot, s32 prim, s32 x_offset, s32 y_offset);
s32 addhero_draw_mode_glyph(s32 *ot, s32 prim, s32 x_offset, s32 y_offset);
s32 addhero_draw_card_slot0_label(s32 *ot, s32 prim, s32 x_offset, s32 y_offset);
s32 addhero_draw_card_slot1_label(s32 *ot, s32 prim, s32 x_offset, s32 y_offset);
s32 addhero_draw_selected_entry_details(s32 *ot, s32 prim, s32 x_offset, s32 y_offset);
u8 *addhero_skip_hex_digits(void *arg0);
void addhero_terminate_multibyte_text(void *arg0);
void addhero_clear_elements();
AddheroElement *addhero_alloc_element(void);
void addhero_update_and_draw_elements();
void addhero_deactivate_primary_element(void);
void addhero_text_append(u8 *arg0, u8 *arg1);
s32 addhero_text_byte_length(u8 *arg0);
void addhero_text_copy(u8 *arg0, u8 *arg1);
s32 addhero_draw_load_prompt(s32 *ot, s32 prim, s32 x_offset, s32 y_offset);
s32 addhero_draw_load_progress(s32 *ot, s32 prim, s32 x_offset, s32 y_offset);
s32 addhero_draw_progress_bar(s32 arg0, s32 *arg1);
void addhero_open_status_dialog(s32 arg0);
void addhero_open_exit_dialog(s32 arg0);
s32 addhero_draw_status_dialog(s32 *ot, s32 prim, s32 x_offset, s32 y_offset);
s32 addhero_draw_exit_dialog(s32 *ot, s32 prim, s32 x_offset, s32 y_offset);
s32 addhero_draw_transfer_status(s32 *ot, s32 prim, s32 x_offset, s32 y_offset);
s32 addhero_draw_icon_highlight(s32 result, s32 *ot, s32 x, s32 y, s32 adjust, s32 slot, s32 i, s32 j);
void addhero_enable_choice_toggle(void);
s32 addhero_draw_choice_prompt(s32 prim, s32 *ot, s32 x, s32 y);
s32 addhero_validate_save_blob(u8 *base);
s32 addhero_compute_save_checksum(u8 *data);
s8 *addhero_format_decimal(s8 *out, s32 value);
void addhero_format_hex(s8 *out, s32 value, s32 max_chars);
void addhero_hex_nibble_to_ascii(s8 *out, s32 value);
u32 addhero_parse_hex(u8 *s, s32 len);
s32 addhero_parse_hex_suffix_byte(u8 *text);

/* External functions */
s32 func_800A88A0(s32 prim, s32 *ot, void *glyph, s32 a3, s32 x, s32 y, s32 mode);
s32 func_800A8A78(s32 *ot, s32 prim, s32 ch, s32 a3, Vec2s *pos, s32 mode);
s32 func_8001714C();
void func_800A3938();
void func_800AA02C();
void func_80067F28(void);
void func_80067F8C(void);
void func_80067F5C(s32 arg0);
void func_80063194(void);
void func_80019788(s32 arg0);
void func_8001990C(RECT *rect, s32 a1, s32 a2, s32 a3);
void func_80019A34(RECT *rect, void *str);
void func_800A55E4(void *buf, s32 arg1);
void func_800A5638(void *buf, s32 arg1);
void func_8001A5D4(s32 arg0, s32 *arg1);
void func_8001C56C(s32 *arg0, s32 a1, s32 a2, s32 a3, s32 a4);
s32 func_800AD850();
s32 func_800AE76C();
s32 func_8002054C(s32 arg0);
void func_80016E7C(void *dst, void *src, s32 len);
void field_text_reset_scratch(void);
void field_text_reset_windows(void);
void addhero_shutdown_stream_handles(void);
void addhero_begin_glyph_cache_frame(void);
void addhero_evict_unused_glyphs(void);
void addhero_reset_glyph_cache(void);
void addhero_reset_entry_ranks(void);
void addhero_init_stream_handles(void);
void addhero_enable_choice_toggle(void);
void addhero_restart_load_sequence(void);
s32 addhero_poll_and_rewind_primary_handles(void);
void addhero_commit_selected_entry(void);
s32 addhero_draw_cached_text(s32 result, s32 *ot, u8 *name, s32 x, s32 y, s32 a5, s32 a6);
s32 addhero_advance_load_sequence();

#define SET_ELEM_CODE(e, c) ((e)->attr.word = ((e)->attr.word & 0x00FFFFFF) | ((u32)(c) << 24))
#define GLYPH_SYM(sym, off) ((void *)(((u8 *)&(sym) - (off)) + (sym)))
#define GLYPH_OFF(base, off) ((void *)((base) + *(u16 *)((base) + (off))))

/**
 * @brief Reset overlay state and build the initial UI elements.
 * @param work_base Work-RAM base (always 0x80170000); stored in D_80160930, unused so far.
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
    addhero_init_stream_handles();
    g_addhero_icon_phase = 0;
    func_80067F8C();

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

    D_80160930 = work_base;
}

/**
 * @brief Run one frame: tear down and exit if requested, else update and render.
 * @param render_half Render buffer half being drawn; forwarded to addhero_update_state, which ignores it.
 * @return Non-zero exit code when exiting, 0 while running.
 * @see decomp.me (100%)
 */
s32 addhero_state_step(s32 render_half)
{
    if (g_addhero_exit_requested != 0)
    {
        addhero_shutdown_stream_handles();
        field_text_reset_windows();
        func_80019788(0);
        return g_addhero_exit_requested;
    }
    
    field_text_reset_scratch();
    addhero_begin_glyph_cache_frame();
    addhero_update_state(render_half);
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
    AddheroElement *p;
    g_addhero_scroll_frames = 0;
    g_addhero_scroll_target_y = 0;
    g_addhero_scroll_y = 0;
    g_addhero_selected_row = 0;
    g_addhero_selection_status = 0;
    D_80160924 = (s32)g_pad_ctx + 0xCE0;
    if (0) addhero_clear_elements(0,0,0,0,0);
    addhero_clear_elements();
    D_801609B4 = 0;
    if (g_addhero_mode != 0)
    {
        g_addhero_element_pool.first.attr.f.state = 1;
        p = addhero_alloc_element();
        p->draw = (void *)addhero_draw_transfer_status;
        p->attr.f.phase = 1;
        p->attr.f.x = 0x10;
        p->attr.f.code = 0x61;
        p->active = 1;
        p->y = 0x2C;
        SET_ELEM_CODE(p, 0x20);

        p = addhero_alloc_element();
        p->draw = (void *)addhero_draw_card_slot0_label;
        p->attr.f.phase = 1;
        p->attr.f.x = 0x18;
        p->attr.f.code = 0x4D;
        p->active = 0;
        p->y = 0x10;
        SET_ELEM_CODE(p, 0x80);

        p = addhero_alloc_element();
        p->draw = (void *)addhero_draw_card_slot1_label;
        p->attr.f.phase = 1;
        p->attr.f.x = 0xA0;
        p->attr.f.code = 0x4D;
        p->active = 0;
        p->y = 0x10;
        SET_ELEM_CODE(p, 0x80);
        g_addhero_element_pool.first.attr.f.state = 0;
        return;
    }

    g_addhero_element_pool.first.attr.f.state = 1;
    p = addhero_alloc_element();
    p->draw = (void *)addhero_draw_entry_list;
    p->attr.f.phase = 1;
    p->attr.f.x = 0x1C;
    p->attr.f.code = 0x32;
    p->active = 1;
    p->y = 0x58;
    SET_ELEM_CODE(p, 8);
    /* ADDHERO-specific flag */
    *(u32 *)((u8 *)p + 4) |= 0x200;

    p = addhero_alloc_element();
    p->draw = (void *)addhero_draw_mode_glyph;
    p->attr.f.phase = 1;
    p->attr.f.x = 0x24;
    p->attr.f.code = 0x0A;
    p->active = 0;
    p->y = 0x10;
    SET_ELEM_CODE(p, 0xF0);

    p = addhero_alloc_element();
    p->draw = (void *)addhero_draw_card_slot0_label;
    p->attr.f.phase = 1;
    p->attr.f.x = 0x18;
    p->attr.f.code = 0x1E;
    p->active = 0;
    p->y = 0x10;
    SET_ELEM_CODE(p, 0x80);

    p = addhero_alloc_element();
    p->draw = (void *)addhero_draw_card_slot1_label;
    p->attr.f.phase = 1;
    p->attr.f.x = 0xA0;
    p->attr.f.code = 0x1E;
    p->active = 0;
    p->y = 0x10;
    SET_ELEM_CODE(p, 0x80);

    p = addhero_alloc_element();
    p->draw = (void *)addhero_draw_selected_entry_details;
    p->attr.f.phase = 1;
    p->attr.f.x = 0x1E;
    p->attr.f.code = 0x8E;
    p->active = 1;
    p->y = 0x34;
    SET_ELEM_CODE(p, 4);
    g_addhero_element_pool.first.attr.f.state = 0;
}

/**
 * @brief Run one frame of overlay logic: update elements, advance the load
 *        sequence when armed, sample pad input, and step the scroll animation.
 * @see decomp.me (100%)
 */
void addhero_update_state(void)
{
    s32 delta;

    addhero_update_elements();
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
        s32 base = g_addhero_scroll_y;
        delta = (g_addhero_scroll_target_y - g_addhero_scroll_y) / g_addhero_scroll_frames;
        g_addhero_scroll_frames -= 1;
        g_addhero_scroll_y += delta;
    }
    else
    {
        g_addhero_scroll_y = g_addhero_scroll_target_y;
    }
}

/**
 * @brief Drive the card load/scan state machine one frame, mapping its result
 *        code onto the next load step and any error entry-state sentinel.
 * @return Unused; declared s32 for the original signature but falls through
 *         without an explicit return value.
 * @see decomp.me (100%)
 */
s32 addhero_update_load_sequence(void)
{
    s32 result;

    if (g_addhero_entry_state >= 0x10)
    {
        if (g_addhero_load_step == 0)
        {
            g_addhero_load_step = (u8 *)&D_80160574;
        }
    }

    do
    {
        result = addhero_advance_load_sequence();
    } while (result == 3);

    if ((D_801609B4 != 0) && (g_pad_input & 0x220))
    {
        if (g_addhero_mode == 0)
        {
            g_addhero_entry_state = 0xF9;
        }
        else
        {
            g_addhero_entry_state = 0xF8;
        }
        g_addhero_load_step = (u8 *)&D_80160588;
    }
    else
    {
        switch (result)
        {
        case 0:
            break;
        case 4:
            g_addhero_load_step = (u8 *)&D_80160580;
            D_801609B4 = 0;
            break;
        case 5:
            if (g_addhero_mode == 0)
            {
                g_addhero_entry_state = 0xF9;
            }
            else
            {
                g_addhero_entry_state = 0xF8;
            }
            /* fallthrough */
        case 2:
            g_addhero_load_step = (u8 *)&D_80160588;
            break;
        }
    }
}

/**
 * @brief Handle browser pad input: exit/back, list navigation, entry
 *        selection, and launching the load prompt for a compatible save.
 * @return Unused; declared s32 for the original signature but every path
 *         returns via a bare return with no value.
 * @see decomp.me (100%)
 */
s32 addhero_handle_input(void)
{
    s32 pending;
    s32 status;
    s32 count;
    s32 term1;
    s32 term2;
    AddheroElement *p;

    if ((g_addhero_element_pool.second_state & 7) == 0) {
        g_addhero_exit_requested = g_addhero_result;
        return;
    }
    if (g_addhero_exit_requested != 0) {
        return;
    }
    if ((g_addhero_element_pool.second_state & 7) >= 3) {
        return;
    }
    if ((g_addhero_element_pool.first.attr.word & 7) != 0) {
        return;
    }
    pending = g_addhero_entry_state;
    if (pending == 0xFF) {
        return;
    }
    if (g_addhero_entry_scan_active != 0) {
        return;
    }
    if (g_addhero_io_busy != 0) {
        return;
    }
    if ((u32)(*g_addhero_load_step - 6) < 2U) {
        return;
    }
    if (g_addhero_mode != 0) {
        return;
    }

    status = g_pad_input;
    if (status & 0x40) {
        D_80122718 = 3;
        func_800A3938(0x78, 0x80);
        addhero_close_all_elements();
        return;
    }
    if (status & 0xA100) {
        func_800A3938(0x7D, 0x80);
        addhero_reset_state();
        return;
    }
    if (pending >= 0x10) {
        return;
    }

    count = 1;
    if (status & 8) {
        g_pad_input = 0x4000;
        count = 1;
    }
    if (g_pad_input & 4) {
        g_pad_input = 0x1000;
        count = 1;
    }

    while (count != 0) {
        if (g_pad_input & 0x1000) {
            g_addhero_selected_row -= 1;
            if (g_addhero_selected_row < 0) {
                g_addhero_selected_row = g_addhero_entry_state - 1;
            }
        }
        if (g_pad_input & 0x4000) {
            g_addhero_selected_row += 1;
            if (g_addhero_selected_row >= g_addhero_entry_state) {
                g_addhero_selected_row = 0;
            }
        }
        count -= 1;
    }

    if (g_pad_input & 0x5000) {
        addhero_commit_selected_entry();
        func_800A3938(0x7D, 0x80);
        addhero_scroll_to_selection();
        return;
    }

    if (g_pad_input & 0x220) {
        term1 = g_addhero_card_slot * ADDHERO_CARD_DIRECTORY_BYTES;
        term2 = (g_addhero_selected_row * ADDHERO_DIRECTORY_ENTRY_BYTES) + (s32)g_addhero_entries;
        if (func_8001714C(D_800ECF7C, (char *)(term1 + term2), 0xC) == 0) {
            if ((g_addhero_entry_metadata.hero_id != ((AddheroRecord *)g_pad_ctx)->hero_id) &&
                ((D_8003EC9C == 0xFF) || (g_addhero_entry_metadata.owner_id == D_8003EC9C))) {
                p = addhero_alloc_element();
                p->attr.f.phase = 1;
                p->attr.f.x = 0x10;
                p->attr.f.code = 0x61;
                p->active = 1;
                p->y = 0x1E;
                SET_ELEM_CODE(p, 0x20);
                addhero_enable_choice_toggle();
                p->draw = addhero_draw_load_prompt;
                addhero_restart_load_sequence();
                func_800A3938(0x7E, 0x80);
                return;
            }
        }
        func_800A3938(0x78, 0x80);
    }
}

/**
 * @brief Reset scroll/selection state and flip to the other card slot, then
 *        clear ranks and pad input to restart browsing.
 * @see decomp.me (100%)
 */
void addhero_reset_state(void)
{
    D_801609B4 = 0;
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
    s32 attr;
    s32 slot;
    s32 *elem;
    s32 closing_attr;

    func_80067F28();
    elem = (s32 *)&g_addhero_element_pool.first;
    slot = 0;
    for (; slot < 8; slot++, elem += 3)
    {
        attr = *elem;
        if (attr & 7)
        {
            closing_attr = (attr & ~7) | 3;
            *elem = (closing_attr & ~0x78) | 0x40;
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
    s32 index;
    s32 temp;
    s32 base;
    s32 pos;
    s32 diff;

    index = g_addhero_selected_row;
    temp = (index << 3) - index;
    base = g_addhero_scroll_y;
    pos = temp << 1;
    diff = pos - base;

    if (diff >= 0x4B)
    {
        g_addhero_scroll_target_y = pos - 0x46;
        g_addhero_scroll_frames = 4;
    }
    if (diff < 0)
    {
        g_addhero_scroll_target_y = pos;
        g_addhero_scroll_frames = 4;
    }
}

/**
 * @brief Thin wrapper that runs the element update/draw pass on the active
 *        draw state.
 * @see decomp.me (100%)
 */
void addhero_update_elements(void)
{
    addhero_update_and_draw_elements();
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
 * @see decomp.me (100%)
 */
s32 addhero_draw_entry_list(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    s32 state = g_addhero_entry_state;

    switch (state)
    {
    case 0xF8:
        do { prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FD8, 0x34), 4, -x_offset + 0x84, -y_offset, 2); } while (0);
        break;
    case 0xF9:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FD8, 0x34), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFA:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FA6, 2), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFD:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FA8, 4), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFB:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FB4, 0x10), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFC:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FB6, 0x12), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFE:
        break;
    default:
        {
            s32 row_y;
            s32 i;

        if (g_addhero_entry_scan_active != 0)
        {
            s32 x;
            u8 *base;
        case ADDHERO_ENTRY_STATE_IDLE:
            x = -x_offset + 0x84;
            base = (u8 *)&D_80146FA4;
            prim = func_800A88A0(prim, ot, base + D_80146FA4, 4, x, -y_offset, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x1E), 4, x, 0xE - y_offset, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - y_offset, 2);
            break;
        }
        i = 0;
        if (state > 0)
        {
            s32 base_x;
            s32 *flag_ptr;
            u16 misc_glyph;
            Vec2s pos;
            s32 row;
            u8 *base;

            base = (u8 *)&D_80146FA4;
            base_x = -x_offset;
            do
            {
                row = ((i * 14) - y_offset) - g_addhero_scroll_y;
                row_y = row + 1;
                if ((u32)(row + 0xE) < 0x65U)
                {
                    flag_ptr = (s32 *)((u8 *)g_addhero_entry_ranks + (i * 4));
                    if (*flag_ptr >= 0)
                    {
                        pos.x = base_x + 0x86;
                        pos.y = row_y;
                        prim = func_800A88A0(func_800A8A78(ot, prim, *(s32 *)((u8 *)g_addhero_entry_suffix_values + (i * 4)), 4, &pos, 0), ot, (void *)((s32)D_80146FD2 + (s32)base), 4, base_x + 0x70, row_y, 0);
                        if ((g_addhero_rank_count - 1) == *flag_ptr)
                        {
                            misc_glyph = *(u16 *)(base + 0x36);
                            prim = func_800A88A0(prim, ot, (void *)((s32)misc_glyph + (s32)base), 4, base_x + 0xC0, row_y, 0);
                        }
                        else if (*flag_ptr < 2)
                        {
                            misc_glyph = *(u16 *)(base + 0x38);
                            prim = func_800A88A0(prim, ot, (void *)((s32)misc_glyph + (s32)base), 4, base_x + 0xC0, row_y, 0);
                        }
                        if (*addhero_skip_hex_digits((void *)((s32)&g_addhero_entries[g_addhero_card_slot][i] + 0xC)) == 0x2B)
                        {
                            prim = func_800A88A0(prim, ot, (void *)((s32)D_80147054 + (s32)base), 4, 0xF2 - x_offset, row_y, 1);
                        }
                    }
                    if (func_8001714C(D_800ECF7C, (char *)((s32)&g_addhero_entries[g_addhero_card_slot][i]), 0xC) == 0)
                    {
                        prim = func_800A88A0(prim, ot, (void *)((s32)D_80146FAA + (s32)base), 4, 1 - x_offset, row_y, 0);
                    }
                    else if (func_8001714C(D_800ECF8C, (char *)((s32)&g_addhero_entries[g_addhero_card_slot][i]), 0xC) == 0)
                    {
                        prim = func_800A88A0(prim, ot, (void *)((s32)D_80146FDE + (s32)base), 4, 1 - x_offset, row_y, 0);
                    }
                    else if (func_8001714C(D_800ECFC4, (char *)((s32)&g_addhero_entries[g_addhero_card_slot][i]), 8) == 0)
                    {
                        prim = func_800A88A0(prim, ot, (void *)((s32)D_80146FB8 + (s32)base), 4, 1 - x_offset, row_y, 0);
                    }
                    else
                    {
                        prim = func_800A88A0(prim, ot, (void *)((s32)D_80146FAC + (s32)base), 4, 1 - x_offset, row_y, 0);
                    }
                }
                i++;
            } while (i < g_addhero_entry_state);
        }
            row_y = ((g_addhero_selected_row * 14) - y_offset) - g_addhero_scroll_y;

            if (g_addhero_entry_scan_active == 0)
            {
                TILE *tile = (TILE *)prim;

                *(u32 *)&tile->r0 = 0xF080F0;
                *((u8 *)tile + 3) = 3;
                tile->code = 0x62;
                tile->w = 0x108;
                tile->x0 = 0;
                tile->y0 = row_y;
                tile->h = 0xE;
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
s32 addhero_draw_mode_glyph(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;

    if (g_addhero_mode == 1)
    {
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FEA, 0x46), 4, -x_offset + 0x78, -y_offset, 2);
    }
    else
    {
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FE8, 0x44), 4, -x_offset + 0x78, -y_offset, 2);
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
s32 addhero_draw_card_slot0_label(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    TILE *tile;

    if (g_addhero_card_slot != 0)
    {
        tile = (TILE *)prim;
        *(u32 *)&tile->r0 = 0x101010;
        *((u8 *)tile + 3) = 3;
        tile->code = 0x62;
        tile->x0 = 0;
        tile->y0 = 0;
        tile->w = 0x80;
        tile->h = 0x10;
        addPrim(ot, tile);
        prim += 0x10;
    }
    return func_800A88A0(prim, ot, GLYPH_SYM(D_80146FB0, 0xC), 4, -x_offset + 0x40, -y_offset, 2);
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
s32 addhero_draw_card_slot1_label(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    TILE *tile;

    if (g_addhero_card_slot == 0)
    {
        tile = (TILE *)prim;
        *(u32 *)&tile->r0 = 0x101010;
        *((u8 *)tile + 3) = 3;
        tile->code = 0x62;
        tile->x0 = 0;
        tile->y0 = 0;
        tile->w = 0x80;
        tile->h = 0x10;
        addPrim(ot, tile);
        prim += 0x10;
    }
    return func_800A88A0(prim, ot, GLYPH_SYM(D_80146FB2, 0xE), 4, -x_offset + 0x40, -y_offset, 2);
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
s32 addhero_draw_selected_entry_details(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
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
            u8 *base;

            result = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FCC, 0x28), 4, x, -y_offset, 0);
            base = (u8 *)&D_80146FCC - 0x28;
            return func_800A88A0(result, ot, GLYPH_OFF(base, 0x2A), 4, x, 0x10 - y_offset, 0);
        }
        else
        {
            s32 term1 = g_addhero_card_slot * ADDHERO_CARD_DIRECTORY_BYTES;
            s32 term2 = (g_addhero_selected_row * ADDHERO_DIRECTORY_ENTRY_BYTES) + (s32)g_addhero_entries;

            if (func_8001714C(D_800ECF7C, (char *)(term1 + term2), 0xC) == 0)
            {
                if (D_8003EC9C == 0xFF || g_addhero_entry_owner_id == D_8003EC9C)
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
                        u8 *record = (u8 *)&g_addhero_entry_metadata;
                        slot[0] = (u32)(*(s32 *)(record + 0x18)) >> 0x19;
                        slot[1] = ((u32)(*(s32 *)(record + 0x20)) >> 0x12) & 0x7F;
                        slot[2] = (u32)(*(s32 *)(record + 0x20)) >> 0x19;
                        g_addhero_icon_palette = (s32)record[0x1F];
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

                            if ((g_addhero_icon_phase >= base_y && g_addhero_icon_phase < base_x && (delta = g_addhero_icon_phase - base_y, 1))
                                || (rem = base_x % (half_step * present_count), g_addhero_icon_phase >= rem && g_addhero_icon_phase < (hi = rem + half_step) && (delta = hi - g_addhero_icon_phase, 1)))
                            {
                                adjust += delta;
                            }
                            result = addhero_draw_icon_highlight(result, ot, total - x_offset, -y_offset, adjust, slot[j], i, j);
                            i += 1;
                            total += adjust;
                        }
                    }

                    {
                        u8 *base90 = (u8 *)&g_addhero_entry_metadata;
                        s32 x = -x_offset;
                        s32 y = -y_offset;

                        base_y = *(s32 *)(base90 + 0x30);
                        pos.x = (s16)(x + 0x70);
                        pos.y = (s16)y;
                        hours = base_y / 216000;
                        result = func_800A8A78(ot, result, hours, 4, &pos, 1);
                        result = func_800A88A0(result, ot,
                            D_800EC3F6[0] + ((s32)&D_800EC3F6 - 0x32) + (D_800EC3F6[1] << 8), 4, x + 0x6F, y, 0);
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
                        result = func_800A88A0(result, ot, base90, 4, x + 0x54, y + 0x10, 0);

                        if (*(u16 *)(base90 + 0xD4) == ((AddheroRecord *)g_pad_ctx)->hero_id)
                        {
                            do { result = func_800A88A0(result, ot, GLYPH_SYM(D_80146FF4, 0x50), 4, x + 0x54, y + 0x20, 0); } while (0);
                        }
                        else
                        {
                            result = func_800A88A0(result, ot, GLYPH_OFF((u8 *)D_80147470, (*(s32 *)(base90 + 0x20) & 0x3FFFF) * 2), 4,
                                x + 0x54, y + 0x20, 0);
                        }
                    }
                }
                else
                {
                    result = func_800A88A0(result, ot, GLYPH_SYM(D_80146FF8, 0x54), 4, -x_offset, -y_offset, 0);
                }
            }
            else
            {
                s32 j;
                u8 *record;

                addhero_terminate_multibyte_text(&D_8016520C);
                record = &D_8016520C;
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
                        name[j] = ((AddheroFallbackText *)&D_80165208)->text[j];
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
 * @param arg0 Start of the text to scan.
 * @return Pointer to the first byte that is not a hex digit.
 * @see decomp.me (100%)
 */
u8 *addhero_skip_hex_digits(void *arg0)
{
    u8 *p;
    u32 c;

    p = arg0;
    while (1)
    {
        c = *p;
        p++;
        if ((u32)(c - '0') < 10)
        {
            continue;
        }
        p--;
        if (p)
        {
            p++;
            p--;
        }

        p++;
        if ((u32)(c - 'a') < 6)
        {
            continue;
        }
        p--;
        if (p)
        {
            p++;
            p--;
        }

        p++;
        if ((u32)(c - 'A') < 6)
        {
            continue;
        }
        p--;
        if (p)
        {
            p++;
            p--;
        }
        break;
    }
    return p;
}

/**
 * @brief Zero-fill a 0x40-byte text field from the first null byte onward,
 *        walking multibyte (>= 0x80 lead) characters two bytes at a time.
 * @param arg0 Start of the 0x40-byte text buffer to terminate/clear.
 * @see decomp.me (100%)
 */
void addhero_terminate_multibyte_text(void *arg0)
{
    u8 *p;
    s32 i;

    p = (u8 *)arg0;
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
    AddheroPacket *p;
    s32 i;

    D_8012298C = 0x20;
    p = (AddheroPacket *)&g_addhero_element_pool.first;
    for (i = 0; i < ADDHERO_ELEMENT_COUNT; i++)
    {
        p->size_flags &= ~0x200;
        p->state_word &= ~7;
        p++;
    }
}

/**
 * @brief Claim the first free pool element, marking it state 1 (opening).
 * @return The claimed element, or the pool base element when none are free.
 * @see decomp.me (100%)
 */
AddheroElement *addhero_alloc_element(void)
{
    AddheroWordPacket *p;
    s32 i;

    p = (AddheroWordPacket *)&g_addhero_element_pool.first;
    for (i = 0; i < ADDHERO_ELEMENT_COUNT; i++, p++)
    {
        if ((p->attr.word & 7) == 0)
        {
            p->attr.word = (p->attr.word & ~7) | 1;
            return (AddheroElement *)p;
        }
    }
    return (AddheroElement *)&g_addhero_element_pool.first;
}

/**
 * @brief Update and render every active pool element: draw scroll arrows, run
 *        each element's per-state transition (open/hold/close), invoke its draw
 *        callback, and advance the shared primitive cursor.
 * @details Walks the 8 element descriptors in g_addhero_element_pool (12 bytes
 *          each: attr/state word, flags word, draw-func pointer). The low 3 bits
 *          of the attr word select the animation state: 1 = opening (grow, then
 *          switch to hold), 2 = open/hold, 3 = closing (shrink, then finish),
 *          4 = finishing (clear the slot). The (attr >> 3) & 0xF nibble is the
 *          0..8 scale step driving the open/close interpolation.
 * @param draw_state Draw state holding the primitive cursor and frame flag.
 * @see decomp.me (100%)
 */
void addhero_update_and_draw_elements(AddheroDrawState *draw_state)
{
    AddheroGpuPacket *prim;
    AddheroDrawState *ot;
    volatile u32 *elem;
    s32 shrink_x;
    s32 shrink_y;
    s32 slot;
    s32 sp20[24];
    u32 attr_word;
    s32 elem_state;
    u32 flags_word;
    u32 span;
    s32 anim_step;
    s32 span_scaled;
    s32 height;
    s32 height_scaled;
    s32 inner_h;
    u32 attr_bits;
    u32 hi_byte;
    s32 word3;
    s32 acc3;
    s32 height3;
    s32 height_scaled3;
    s32 inner_h3;
    u32 elem_word4;
    u32 anim_word;
    s32 count;

    prim = draw_state->prim_cursor;
    ot = draw_state;

    count = g_addhero_entry_state;
    if ((count < 0x10) &&
        ((((AddheroPacketBlock *)&g_addhero_element_pool.first)->second.attr.word & 7) == 2) &&
        (((((AddheroPacketBlock *)&g_addhero_element_pool.first)->second.unk4 >> 9) & 1) != 0))
    {
        count *= 0xE;
        if ((g_addhero_scroll_y + 0x58) < count)
        {
            prim = (AddheroGpuPacket *)func_800AE76C(prim, ot, 0x114, 0x82, 0);
        }
        if (g_addhero_scroll_y != 0)
        {
            prim = (AddheroGpuPacket *)func_800AE76C(prim, ot, 0x114, 0x3A, 1);
        }
    }

    if (draw_state->frame_flag != 0)
    {
        func_8001C56C(sp20, 0, 0xF0, 0x140, 0xE0);
    }
    else
    {
        func_8001C56C(sp20, 0, 8, 0x140, 0xE0);
    }

    elem = (volatile u32 *)&g_addhero_element_pool.first;
    slot = 0;

    for (; slot < 8; slot++, elem += 3)
    {
        if (*elem & 7)
        {
            func_8001A5D4((s32)prim, sp20);

            addPrim(ot, prim);

            attr_word = *elem;
            elem_state = attr_word & 7;

            prim = (AddheroGpuPacket *)((u8 *)prim + 0x40);

            switch (elem_state)
            {
            case 1:
                attr_bits = *elem;
                flags_word = *(u32 *)((u8 *)elem + 4);
                hi_byte = attr_bits >> 24;
                span = ((flags_word & 1) << 8) | hi_byte;
                anim_step = (attr_bits >> 3) & 0xF;
                span_scaled = span * anim_step;
                g_pad_input = 0;
                if (span_scaled < 0)
                {
                    span_scaled += 7;
                }
                height = (flags_word >> 1) & 0xFF;
                height_scaled = height * anim_step;
                shrink_x = span_scaled >> 3;
                if (height_scaled < 0)
                {
                    height_scaled += 7;
                }
                shrink_y = height_scaled >> 3;
                inner_h = (s32)(height - shrink_y);

                prim = (*(AddheroElemDrawFunc *)((u8 *)elem + 8))(ot, prim, (s32)(span - shrink_x) / 2, inner_h / 2);
                {
                    u32 post_word;
                    u32 field;
                    u32 high;
                    post_word = *elem;
                    field = (post_word >> 7) & 0x1FF;
                    high = post_word >> 24;
                    prim = (AddheroGpuPacket *)func_800AD850(prim, ot,
                                           field + (s32)((((*(u32 *)((u8 *)elem + 4) & 1) << 8) | high) - shrink_x) / 2,
                                           (*((u8 *)elem + 2)) + ((s32)((*(u32 *)((u8 *)elem + 4) >> 1) & 0xFF) - shrink_y) / 2,
                                           shrink_x, shrink_y, draw_state->frame_flag, slot == 0);
                }
                {
                    u32 old_word;
                    u32 new_word;
                    old_word = *elem;
                    new_word = (old_word & ~0x78) | (((((old_word >> 3) & 0xF) + 1) & 0xF) * 8);
                    *(u32 *)elem = new_word;
                    if (((new_word >> 3) & 0xF) == 8)
                    {
                        func_800AA02C();
                        *(u32 *)elem = (*elem & ~7) | 2;
                    }
                }
                break;

            case 2:
                prim = (*(AddheroElemDrawFunc *)((u8 *)elem + 8))(ot, prim, 0, 0);
                {
                    u32 case_word;
                    u32 high;
                    case_word = *elem;
                    high = case_word >> 24;
                    prim = (AddheroGpuPacket *)func_800AD850(prim, ot,
                                           (case_word >> 7) & 0x1FF, *((u8 *)elem + 2),
                                           ((*(u32 *)((u8 *)elem + 4) & 1) << 8) | high,
                                           (*(u32 *)((u8 *)elem + 4) >> 1) & 0xFF, draw_state->frame_flag, slot == 0);
                }
                anim_word = *elem;
                if (((anim_word >> 3) & 0xF) != 0)
                {
                    *(u32 *)elem = (anim_word & ~0x78) | (((((anim_word >> 3) & 0xF) - 1) & 0xF) * 8);
                }
                break;

            case 3:
                word3 = *elem;
                flags_word = *(u32 *)((u8 *)elem + 4);
                acc3 = (u32)word3 >> 24;
                span = ((flags_word & 1) << 8) | acc3;
                word3 = (u32)word3 >> 3;
                word3 &= 0xF;
                acc3 = span * word3;
                g_pad_input = 0;
                if (acc3 < 0)
                {
                    acc3 += 7;
                }
                height3 = (flags_word >> 1) & 0xFF;
                height_scaled3 = height3 * word3;
                shrink_x = acc3 >> 3;
                if (height_scaled3 < 0)
                {
                    height_scaled3 += 7;
                }
                shrink_y = height_scaled3 >> 3;
                inner_h3 = (s32)(height3 - shrink_y);

                prim = (*(AddheroElemDrawFunc *)((u8 *)elem + 8))(ot, prim, (s32)(span - shrink_x) / 2, inner_h3 / 2);
                {
                    u32 post_word;
                    u32 field;
                    u32 high;
                    post_word = *elem;
                    field = (post_word >> 7) & 0x1FF;
                    high = post_word >> 24;
                    prim = (AddheroGpuPacket *)func_800AD850(prim, ot,
                                           field + (s32)((((*(u32 *)((u8 *)elem + 4) & 1) << 8) | high) - shrink_x) / 2,
                                           (*((u8 *)elem + 2)) + ((s32)((*(u32 *)((u8 *)elem + 4) >> 1) & 0xFF) - shrink_y) / 2,
                                           shrink_x, shrink_y, draw_state->frame_flag, slot == 0);
                }
                {
                    u32 old_word;
                    old_word = *elem;
                    acc3 = old_word & ~0x78;
                    old_word >>= 3;
                    old_word &= 0xF;
                    old_word--;
                    old_word &= 0xF;
                    old_word <<= 3;
                    acc3 |= old_word;
                    *(u32 *)elem = acc3;
                    if (!(((u32)acc3 >> 3) & 0xF))
                    {
                        *(u32 *)elem = ((((u32)acc3 & ~0x78) | 0x18) & ~7) | 4;
                    }
                }
                break;

            case 4:
                elem_word4 = *(u32 *)elem;
                g_pad_input = 0;
                anim_word = (elem_word4 & ~0x78) | (((((elem_word4 >> 3) & 0xF) - 1) & 0xF) * 8);
                *(u32 *)elem = anim_word;
                if (!((anim_word >> 3) & 0xF))
                {
                    *(u32 *)elem = anim_word & ~7;
                }
                break;
            }
        }
    }

    draw_state->prim_cursor = prim;
}

/**
 * @brief Free the primary pool element by clearing its state bits.
 * @see decomp.me (100%)
 */
void addhero_deactivate_primary_element(void)
{
    g_addhero_element_pool.first.attr.word &= ~7;
}

/**
 * @brief Append the multibyte string @p src onto the end of @p dst and
 *        null-terminate the result.
 * @param dst Destination string; appended to in place.
 * @param src Source string copied onto the end of @p dst.
 * @see decomp.me (100%)
 */
void addhero_text_append(u8 *dst, u8 *src)
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
 * @param arg0 Null-terminated string to measure.
 * @return Length in bytes, excluding the terminator.
 * @see decomp.me (100%)
 */
s32 addhero_text_byte_length(u8 *arg0)
{
    u8 *p;
    u8 c;
    s32 len;

    p = arg0;
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
 * @param arg0 Destination buffer.
 * @param arg1 Source string to copy.
 * @see decomp.me (100%)
 */
void addhero_text_copy(u8 *arg0, u8 *arg1)
{
    u8 *p;
    u8 c;
    s32 len;
    s32 i;

    p = arg1;
    len = 0;
    while (*p != 0)
    {
        c = *(volatile u8 *)p;
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
        arg0[i] = arg1[i];
    }
    arg0[i] = 0;
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
s32 addhero_draw_load_prompt(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    s32 result;
    s32 x;
    s32 status;
    AddheroElement *p;

    x = -x_offset + 0x90;
    result = addhero_draw_choice_prompt(
        func_800A88A0(prim, ot,
                      (u8 *)&D_80146FD4 + D_80146FD4 - 0x30,
                      4, x, -y_offset, 2),
        ot, x, 0xE - y_offset);

    if ((u32)(addhero_poll_and_rewind_primary_handles() - 1) < 2U)
    {
        g_addhero_element_pool.first.attr.f.state = 0;
        func_800AA02C();
        func_800A3938(0x78, 0x80);
        g_addhero_entry_state = ADDHERO_ENTRY_STATE_IDLE;
        addhero_reset_entry_ranks();
        g_addhero_load_step = 0;
    }
    else
    {
        status = g_pad_input;
        if (status & 0x40)
        {
            g_addhero_element_pool.first.attr.f.state = 0;
            func_800AA02C();
            func_800A3938(0x78, 0x80);
            g_addhero_load_step = D_80160588;
        }
        else if (status & 0x220)
        {
            if (g_addhero_choice_toggle != 0)
            {
                g_addhero_element_pool.first.attr.f.state = 0;
                func_800AA02C();
                func_800A3938(0x78, 0x80);
                g_addhero_load_step = D_80160588;
            }
            else
            {
                func_800A3938(0x7E, 0x80);
                g_addhero_progress_active = 1;
                g_addhero_load_step = D_80160590;
                p = &g_addhero_element_pool.first;
                p->draw = addhero_draw_load_progress;
                p->attr.f.phase = 1;
                p->attr.f.state = 1;
                p->attr.f.x = 0x10;
                p->attr.f.code = 0x61;
                p->active = 1;
                p->y = 0x2C;
                SET_ELEM_CODE(p, 0x20);
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
s32 addhero_draw_load_progress(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    u8 *base;
    u8 *resource;
    AddheroElement *p;
    AddheroElement *cursor;
    s32 result;
    s32 x;
    s32 i;
    u32 saved;

    x = -x_offset + 0x90;
    result = func_800A88A0(prim, ot, (void *)((s32)&D_80146FD6 - 0x32 + D_80146FD6), 4, x, -y_offset, 2);
    base = (u8 *)&D_80146FD6 - 0x32;
    result = func_800A88A0(result, ot, base + *(u16 *)(base + 0x1E), 4, x, 0xE - y_offset, 2);
    result = func_800A88A0(result, ot, base + *(u16 *)(base + 0xB2), 4, x, 0x1C - y_offset, 2);
    result = addhero_draw_progress_bar(result, ot);

    if (g_addhero_progress_active == 0)
    {
        resource = g_addhero_save_blob;
        p = (AddheroElement *)&g_addhero_element_pool.first;
        p->attr.f.state = 0;
        if (addhero_validate_save_blob(resource) == 0)
        {
            addhero_open_status_dialog(4);
            return result;
        }

        func_800A3938(0x7B, 0x80);
        saved = g_pad_ctx[0x858] >> 7;
        func_80016E7C(resource + 0x770, g_pad_ctx + 0x840, 0x250);
        *(u32 *)(g_pad_ctx + 0x858) = (*(u32 *)(g_pad_ctx + 0x858) & ~0x80) | (saved << 7);
        *(u16 *)(g_pad_ctx + 0xD8) = *(u16 *)(resource + 0x254);
        *(u16 *)(g_pad_ctx + 0xDA) = *(u16 *)(resource + 0x256);
        *(u16 *)(g_pad_ctx + 0xDE) = 1;
        func_80067F28();

        cursor = p;
        for (i = 0; i < ADDHERO_ELEMENT_COUNT; i++, cursor++)
        {
            if (cursor->attr.f.state != 0)
            {
                cursor->attr.f.state = 3;
                cursor->attr.f.phase = 8;
            }
        }
        func_80067F5C(8);
        g_addhero_result = 1;
    }

    return result;
}

/**
 * @brief Draw the gradient progress bar whose width tracks elapsed ticks, when
 *        the bar is active.
 * @param arg0 Current primitive pointer/index the POLY_G4 is written to.
 * @param arg1 Ordering table the primitive is linked into.
 * @return The advanced primitive pointer, unchanged when the bar is inactive.
 * @see decomp.me (100%)
 */
s32 addhero_draw_progress_bar(s32 arg0, s32 *arg1)
{
    POLY_G4 *g;
    s32 elapsed;
    s32 extent;
    s32 color;

    g = (POLY_G4 *)arg0;
    if (g_addhero_progress_bar_active != 0)
    {
        elapsed = func_8002054C(-1) - g_addhero_progress_start_tick;
        if (elapsed >= 0x101)
        {
            elapsed = 0x100;
        }
        color = 0xFFFF00;
        extent = elapsed * 0x120;
        SET_BGR0_PACKED(g, 0xFF);
        SET_POLY_G4_BGR1_PACKED(g, 0xFFFF);
        SET_POLY_G4_BGR3_PACKED(g, 0xFF0000);
        ((u8 *)g)[3] = 8;
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
        addPrim(arg1, g);
        arg0 += 0x24;
    }
    return arg0;
}

/**
 * @brief Reconfigure the primary element as a modal status dialog and reset the
 *        browser/IO state, storing the dialog message id.
 * @param arg0 Dialog message id stored in g_addhero_dialog_state.
 * @see decomp.me (100%)
 */
void addhero_open_status_dialog(s32 arg0)
{
    func_800A3938(0x78, 0x80);
    g_addhero_element_pool.first.draw = (void *)addhero_draw_status_dialog;
    g_addhero_element_pool.first.attr.f.phase = 1;
    g_addhero_element_pool.first.attr.f.state = 1;
    g_addhero_element_pool.first.attr.f.x = 0x20;
    g_addhero_element_pool.first.attr.f.code = 0x70;
    g_addhero_element_pool.first.active = 1;
    g_addhero_element_pool.first.y = 0x14;
    SET_ELEM_CODE(&g_addhero_element_pool.first, 0);
    func_800AA02C();
    g_addhero_write_in_progress = 0;
    g_addhero_progress_active = 0;
    g_addhero_selection_status = 0;
    g_addhero_io_busy = 0;
    g_addhero_entry_state = ADDHERO_ENTRY_STATE_IDLE;
    addhero_reset_entry_ranks();
    g_addhero_load_step = 0;
    g_addhero_dialog_state = arg0;
}

/**
 * @brief Reconfigure the primary element as a modal exit dialog and reset the
 *        browser/IO state, storing the dialog message id.
 * @param arg0 Dialog message id stored in g_addhero_dialog_state.
 * @see decomp.me (100%)
 */
void addhero_open_exit_dialog(s32 arg0)
{
    func_800A3938(0x78, 0x80);
    g_addhero_element1.draw = (void *)addhero_draw_exit_dialog;
    g_addhero_element1.attr.f.phase = 1;
    g_addhero_element1.attr.f.state = 1;
    g_addhero_element1.attr.f.x = 0x20;
    g_addhero_element1.attr.f.code = 0x70;
    g_addhero_element1.active = 1;
    g_addhero_element1.y = 0x14;
    SET_ELEM_CODE(&g_addhero_element1, 0);
    func_800AA02C();
    g_addhero_write_in_progress = 0;
    g_addhero_progress_active = 0;
    g_addhero_selection_status = 0;
    g_addhero_io_busy = 0;
    addhero_reset_entry_ranks();
    g_addhero_load_step = 0;
    g_addhero_dialog_state = arg0;
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
s32 addhero_draw_status_dialog(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;

    switch (g_addhero_dialog_state)
    {
    case 0:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FE0, 0x3C), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 2:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FE4, 0x40), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 3:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FE6, 0x42), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 1:
    case 4:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FE2, 0x3E), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    }
    if (g_pad_input & 0x220)
    {
        g_addhero_element_pool.first.attr.f.state = 0;
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
s32 addhero_draw_exit_dialog(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    AddheroPacket *p;
    s32 i;

    switch (g_addhero_dialog_state)
    {
    case 0:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FE0, 0x3C), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 2:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FE4, 0x40), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 3:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FE6, 0x42), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 1:
    case 4:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FE2, 0x3E), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    }
    if (g_pad_input & 0x220)
    {
        g_addhero_result = 3;
        D_8012298C = 0x20;
        p = &g_addhero_element_pool.first;
        for (i = 0; i < ADDHERO_ELEMENT_COUNT; i++)
        {
            p->size_flags &= ~0x200;
            p->state_word &= ~7;
            p++;
        }
        func_80067F5C(8);
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
 * @see decomp.me (100%)
 */
s32 addhero_draw_transfer_status(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    switch (g_addhero_entry_state)
    {
    case 0xF8:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FD8, 0x34), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case 0xF9:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FD8, 0x34), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case ADDHERO_ENTRY_STATE_IDLE:
        {
            s32 x; u8 *base;
            x = -x_offset + 0x90;
            base = (u8 *)&D_80146FA4;
            prim = func_800A88A0(prim, ot, base + D_80146FA4, 4, x, -y_offset, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x1E), 4, x, 0xE - y_offset, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - y_offset, 2);
        }
        break;
    case 0xFA:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FD8, 0x34), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case 0xFD:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FA8, 4), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case 0xFB:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FB4, 0x10), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case 0xFC:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FB6, 0x12), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case 0xF7:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014700C, 0x68), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case ADDHERO_ENTRY_STATE_LOAD_PROGRESS:
        {
            s32 x; u8 *base; POLY_G4 *g; s32 next, elapsed, extent, color, finalmode;
            x = -x_offset + 0x90;
            prim = func_800A88A0(prim, ot, (void *)((s32)&D_80146FD6 - 0x32 + D_80146FD6), 4, x, -y_offset, 2);
            base = (u8 *)&D_80146FD6 - 0x32;
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x1E), 4, x, 0xE - y_offset, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - y_offset, 2);
            next = prim; g = (POLY_G4 *)prim;
            if (g_addhero_progress_bar_active != 0)
            {
                elapsed = func_8002054C(-1) - g_addhero_progress_start_tick;
                if (elapsed >= 0x101) elapsed = 0x100;
                color = 0xFFFF00; extent = elapsed * 0x120;
                SET_BGR0_PACKED(g, 0xFF); SET_POLY_G4_BGR1_PACKED(g, 0xFFFF); SET_POLY_G4_BGR3_PACKED(g, 0xFF0000); ((u8 *)g)[3]=8;
                SET_POLY_G4_BGR2_PACKED(g, color); g->code=0x38; g->x2=0; g->x0=0;
                if (extent < 0) extent += 0xFF;
                g->x3=extent>>8; g->x1=extent>>8; g->y1=0; g->y0=0; g->y3=0x2C; g->y2=0x2C;
                g->tag=(g->tag & 0xFF000000)|(*ot & 0xFFFFFF);
                *ot=(*ot & 0xFF000000)|(prim & 0xFFFFFF); next=prim+0x24;
            }
            prim = next;
            if (g_addhero_progress_active == 0)
            {
                if (addhero_validate_save_blob(g_addhero_save_blob) == 0)
                {
                    func_800A3938(0x78, 0x80);
                    g_addhero_element_pool.first.draw=(void *)addhero_draw_status_dialog;
                    g_addhero_element_pool.first.attr.f.phase=1; g_addhero_element_pool.first.attr.f.state=1; g_addhero_element_pool.first.attr.f.x=0x20; g_addhero_element_pool.first.attr.f.code=0x70;
                    g_addhero_element_pool.first.active=1; g_addhero_element_pool.first.y=0x14; SET_ELEM_CODE(&g_addhero_element_pool.first,0);
                    func_800AA02C();
                    g_addhero_write_in_progress=0; g_addhero_selection_status=0; g_addhero_io_busy=0; g_addhero_progress_active=0; g_addhero_entry_state=ADDHERO_ENTRY_STATE_IDLE;
                    addhero_reset_entry_ranks(); finalmode=4; g_addhero_load_step=0; g_addhero_dialog_state=finalmode; return prim;
                }
                func_800A3938(0x7B,0x80); g_addhero_entry_state=ADDHERO_ENTRY_STATE_SAVE_CONFIRM; addhero_enable_choice_toggle(); func_800AA02C();
            }
        }
        break;
    case 0xF3:
        {
            s32 x; AddheroPacket *packet; s32 i;
            x = -x_offset + 0x90;
            prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80147012,0x6E), 4, x, -y_offset, 2);
            prim = addhero_draw_choice_prompt(prim, ot, x, 0xE -y_offset);
            if (g_pad_input & 0x40)
            {
                func_800A3938(0x78, 0x80);
                addhero_enable_choice_toggle();
                g_addhero_entry_state = ADDHERO_ENTRY_STATE_SAVE_CONFIRM;
                func_800AA02C();
            }
            else if (g_pad_input & 0x220)
            {
                if (g_addhero_choice_toggle != 0)
                {
                    func_800A3938(0x78, 0x80);
                    addhero_enable_choice_toggle();
                    g_addhero_entry_state = ADDHERO_ENTRY_STATE_SAVE_CONFIRM;
                    func_800AA02C();
                }
                else
                {
                    func_800A3938(0x7D, 0x80);
                    g_addhero_result = 3;
                    D_8012298C = 0x20;
                    packet = (AddheroPacket *)&g_addhero_element_pool.first;
                    for (i = 0; i < ADDHERO_ELEMENT_COUNT; i++, packet++)
                    {
                        packet->size_flags &= ~0x200;
                        packet->state_word &= ~7;
                    }
                    func_80067F5C(8);
                    func_800AA02C();
                }
            }
        }
        break;
    case ADDHERO_ENTRY_STATE_SAVE_CONFIRM:
        {
            s32 x; u8 *base; s32 temp;
            x=-x_offset+0x90;
            prim=func_800A88A0(prim,ot,(void *)((s32)&D_8014700E-0x6A+D_8014700E),4,x,-y_offset,2);
            base=(u8 *)&D_8014700E-0x6A;
            prim=func_800A88A0(prim,ot,GLYPH_OFF(base,0x6C),4,x,0xE -y_offset,2);
            prim=addhero_draw_choice_prompt(prim,ot,x,0x1C-y_offset);
            if (g_pad_input & 0x40)
            {
                addhero_enable_choice_toggle();
                g_addhero_entry_state = 0xF3;
                func_800A3938(0x78, 0x80);
                func_800AA02C();
            }
            else if (g_pad_input & 0x220)
            {
                if (g_addhero_choice_toggle != 0)
                {
                    addhero_enable_choice_toggle();
                    g_addhero_entry_state = 0xF3;
                    func_800A3938(0x78, 0x80);
                    func_800AA02C();
                }
                else
                {
                    func_800A3938(0x7E,0x80);
                    base = g_addhero_save_blob;
                    func_80016E7C(g_pad_ctx + 0x840, base + 0x770, 0x250);
                    *(s32 *)(base + 0x788) |= 0x80;
                    temp = addhero_compute_save_checksum(base);
                    *(s32 *)(base + 0x33E4) = 0x414E41;
                    *(s32 *)(base + 0x33E0) = temp;
                    g_addhero_write_in_progress = 1;
                    g_addhero_load_step = &D_801605A1;
                    g_addhero_entry_state = ADDHERO_ENTRY_STATE_SAVE_PROGRESS;
                }
            }
        }
        break;
    case ADDHERO_ENTRY_STATE_SAVE_PROGRESS:
        {
            s32 x; u8 *base; POLY_G4 *g; s32 next,elapsed,extent,color; AddheroPacket *packet; s32 i;
            x=-x_offset+0x90;
            prim=func_800A88A0(prim,ot,(void *)((s32)&D_80146FC0-0x1C+D_80146FC0),4,x,-y_offset,2);
            base=(u8 *)&D_80146FC0-0x1C;
            prim=func_800A88A0(prim,ot,GLYPH_OFF(base,0x1E),4,x,0xE -y_offset,2);
            prim=func_800A88A0(prim,ot,GLYPH_OFF(base,0xB2),4,x,0x1C-y_offset,2);
            next=prim; g=(POLY_G4 *)prim;
            if(g_addhero_progress_bar_active!=0){
                elapsed=func_8002054C(-1)-g_addhero_progress_start_tick; if(elapsed>=0x101)elapsed=0x100; color=0xFFFF00; extent=elapsed*0x120;
                SET_BGR0_PACKED(g,0xFF);SET_POLY_G4_BGR1_PACKED(g,0xFFFF);SET_POLY_G4_BGR3_PACKED(g,0xFF0000);((u8*)g)[3]=8;SET_POLY_G4_BGR2_PACKED(g,color);g->code=0x38;g->x2=0;g->x0=0;
                if(extent<0)extent+=0xFF;g->x3=extent>>8;g->x1=extent>>8;g->y1=0;g->y0=0;g->y3=0x2C;g->y2=0x2C;
                g->tag=(g->tag&0xFF000000)|(*ot&0xFFFFFF);*ot=(*ot&0xFF000000)|(prim&0xFFFFFF);next=prim+0x24;
            }
            prim=next;
            if(g_addhero_write_in_progress==0){
                g_pad_ctx[0x840]=0; func_800A3938(0x7A,0x80); D_8012298C=0x20;
                packet=(AddheroPacket *)&g_addhero_element_pool.first;
                for(i=0;i<ADDHERO_ELEMENT_COUNT;i++,packet++){ packet->size_flags &= ~0x200; packet->state_word &= ~7; }
                func_80067F5C(8); g_addhero_result=2;
            }
        }
        break;
    default:
        {
            s32 x,posv,diff; u8 *base;
            x=-x_offset+0x90; base=(u8 *)&D_80146FA4;
            prim=func_800A88A0(prim,ot,base+D_80146FA4,4,x,-y_offset,2);
            prim=func_800A88A0(prim,ot,GLYPH_OFF(base,0x1E),4,x,0xE -y_offset,2);
            prim=func_800A88A0(prim,ot,GLYPH_OFF(base,0xB2),4,x,0x1C-y_offset,2);
            if(g_addhero_entry_scan_active==0){
                if(g_addhero_io_busy!=0)return prim;
                if((u32)(*g_addhero_load_step-6)<2U)return prim;
                if((func_8001714C(D_800ECF7C,&g_addhero_entries[g_addhero_card_slot][g_addhero_selected_row],0xC)!=0) ||
                   (g_addhero_entry_identity != *(s32 *)(g_pad_ctx+0xD8))) {
                    g_addhero_selected_row++;
                    if(g_addhero_selected_row>=g_addhero_entry_state){ if(g_addhero_entry_state!=0)g_addhero_entry_state=0xF7; else g_addhero_entry_state=0xF8; }
                    else { addhero_commit_selected_entry(); posv=g_addhero_selected_row*0xE; diff=posv-g_addhero_scroll_y;
                        if(diff>=0x4B){g_addhero_scroll_target_y=posv-0x46;g_addhero_scroll_frames=4;} if(diff<0){g_addhero_scroll_target_y=posv;g_addhero_scroll_frames=4;}
                    }
                } else { g_addhero_progress_start_tick=func_8002054C(-1);g_addhero_progress_active=1;g_addhero_load_step=&D_80160598;g_addhero_entry_state=ADDHERO_ENTRY_STATE_LOAD_PROGRESS; }
            }
        }
        break;
    case 0xFE:
        break;
    }
    if(g_addhero_io_busy!=0)return prim;
    if(g_addhero_entry_state==ADDHERO_ENTRY_STATE_LOAD_PROGRESS)return prim; if(g_addhero_entry_state==ADDHERO_ENTRY_STATE_SAVE_PROGRESS)return prim; if(g_addhero_entry_state==ADDHERO_ENTRY_STATE_SAVE_CONFIRM)return prim; if(g_addhero_entry_state==0xF3)return prim;
    if(g_pad_input&0x40){
        s32 *p; s32 i,word; D_80122718=3;func_800A3938(0x78,0x80);func_80067F28();p=(s32 *)&g_addhero_element_pool.first;i=0;
        do{word=*p;if(word&7)*p=(((word&~7)|3)&~0x78)|0x40;i++;p+=3;}while(i<8);return prim;
    }
    if((g_pad_input&0xA100)&&(g_addhero_entry_state!=ADDHERO_ENTRY_STATE_IDLE)){
        func_800A3938(0x7D,0x80);D_801609B4=0;g_addhero_load_step=0;g_addhero_scroll_frames=0;g_addhero_scroll_target_y=0;g_addhero_scroll_y=0;g_addhero_selected_row=0;g_addhero_entry_state=ADDHERO_ENTRY_STATE_IDLE;g_addhero_selection_status=0;
        g_addhero_card_slot^=1;addhero_reset_entry_ranks();func_800AA02C();g_addhero_progress_bar_active=0;g_pad_input=0;g_addhero_load_step=&D_80160574;
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
 * @param i      Index among the present (non-empty) slots.
 * @param j      Index among all three slots.
 * @return The advanced primitive pointer (result + 0x28), or @p result when the
 *         slot is empty.
 * @see decomp.me (100%)
 */
s32 addhero_draw_icon_highlight(s32 result, s32 *ot, s32 x, s32 y, s32 adjust, s32 slot, s32 i, s32 j)
{
    RECT rect;
    s32 temp;
    s8 shade;

    if (slot == 0x7F) return result;
    rect.x = i * 0x10;
    rect.y = 0x1F2;
    rect.w = 0x10;
    rect.h = 1;
    if ((j == 1) && (slot < 2)) {
        func_800A5638(g_addhero_icon_context, slot);
        func_80019A34(&rect, g_addhero_icon_context);
        func_80019788(0);
    } else if (slot >= 0x4F) {
        func_800A55E4(g_addhero_icon_context, g_addhero_icon_palette);
        func_80019A34(&rect, g_addhero_icon_context);
        func_80019788(0);
    } else {
        func_80019A34(&rect, (void *)((u8 *)&D_80147658 - 4 + D_80147658[slot]));
    }
    temp = i * 3;
    rect.x = temp * 4 + 0x140;
    rect.y = 0xD0;
    rect.w = 0xC;
    rect.h = 0x30;
    func_80019A34(&rect, (void *)((u8 *)&D_80147658 + 0x1C + D_80147658[slot]));
    SET_BGR0_PACKED((POLY_FT4 *)result, 0x808080);
    ((u8 *)result)[3] = 9;
    ((POLY_FT4 *)result)->code = 0x2C;
    ((POLY_FT4 *)result)->x2 = x;
    ((POLY_FT4 *)result)->x0 = x;
    ((POLY_FT4 *)result)->y1 = y;
    ((POLY_FT4 *)result)->y0 = y;
    ((POLY_FT4 *)result)->x3 = x + adjust;
    shade = temp * 0x10;
    ((POLY_FT4 *)result)->u2 = shade;
    ((POLY_FT4 *)result)->u0 = shade;
    shade += 0x2F;
    ((POLY_FT4 *)result)->u3 = shade;
    ((POLY_FT4 *)result)->u1 = shade;
    ((POLY_FT4 *)result)->v1 = 0xD0;
    ((POLY_FT4 *)result)->v0 = 0xD0;
    ((POLY_FT4 *)result)->x1 = x + adjust;
    ((POLY_FT4 *)result)->y3 = y + 0x2F;
    ((POLY_FT4 *)result)->y2 = y + 0x2F;
    ((POLY_FT4 *)result)->v3 = 0xFF;
    ((POLY_FT4 *)result)->v2 = 0xFF;
    ((POLY_FT4 *)result)->clut = (i & 0x3F) | 0x7C80;
    ((POLY_FT4 *)result)->tpage = 5;
    addPrim(ot, result);
    return result + 0x28;
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
s32 addhero_draw_choice_prompt(s32 prim, s32 *ot, s32 x, s32 y)
{
    u8 *p;
    u8 *base;
    s32 g1;
    s32 g2;
    s32 hi;
    s32 a3;

    p = (u8 *)&D_800EC3FA;
    hi = p[1] << 8;
    base = p - 0x36;
    a3 = 4;
    g1 = p[0] + (hi + (s32)base);
    if (g_addhero_choice_toggle != 0)
    {
        a3 = 5;
    }
    prim = func_800A88A0(prim, ot, (void *)g1, a3, x - 0x10, y, 1);
    a3 = 4;
    g2 = base[0x38] + ((base[0x39] << 8) + (s32)base);
    if (g_addhero_choice_toggle == 0)
    {
        a3 = 5;
    }
    prim = func_800A88A0(prim, ot, (void *)g2, a3, x + 8, y, 0);
    if (g_pad_input & 0xA000)
    {
        g_addhero_choice_toggle ^= 1;
        func_800A3938(0x7D, 0x80);
        g_pad_input = 0;
    }
    return prim;
}

/**
 * @brief Validate a loaded save blob by checking its stored checksum and the
 *        "ANA" magic tag.
 * @param base Base of the 0x4000-byte save blob.
 * @return 1 when the checksum and magic both match, 0 otherwise.
 * @see decomp.me (100%)
 */
s32 addhero_validate_save_blob(u8 *base)
{
    if (*(s32 *)(base + 0x33E0) == addhero_compute_save_checksum(base))
    {
        if (*(s32 *)(base + 0x33E4) == 0x414E41)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Compute the save-blob checksum over the first 0x33E0 bytes.
 * @param data Base of the save blob.
 * @return The checksum: (byte sum * 2) + 0x0414E410.
 * @see decomp.me (100%)
 */
s32 addhero_compute_save_checksum(u8 *data)
{
    s32 sum;
    u32 i;
    u8 *p;

    p = data;
    sum = 0;
    i = 0;
    do
    {
        i += 1;
        sum += *p;
        p += 1;
    } while (i < 0x33E0U);
    return (sum * 2) + 0x0414E410;
}

/**
 * @brief Format @p value as a big-endian double-byte decimal glyph string,
 *        suppressing leading zeros; emits a fixed overflow string past 999999.
 * @param out   Destination glyph buffer.
 * @param value Value to format.
 * @return Pointer to the terminator written after the last glyph.
 * @see decomp.me (100%)
 */
s8 *addhero_format_decimal(s8 *out, s32 value)
{
    struct Copy7 { s8 data[7]; };
    extern s8 D_80140088[];
    s32 digit;
    s32 divisor;
    s32 started;
    s8 *p;

    p = out;
    divisor = 100000;
    if (value < divisor * 10)
    {
        goto format;
    }
    *(struct Copy7 *)p = *(struct Copy7 *)D_80140088;
    return p + 6;

format:
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
 * @see decomp.me (100%)
 */
void addhero_format_hex(s8 *out, s32 value, s32 max_chars)
{
    s32 nibble;
    s32 shift_index;
    s32 remaining_chars;
    s32 remaining_value;
    s32 started;
    s8 *cursor;
    s32 end_index;

    cursor = out;
    remaining_value = value;
    remaining_chars = max_chars;
    shift_index = 7;
    started = 0;
    if (remaining_chars != 0)
    {
        end_index = -1;
loop_2:
        nibble = (remaining_value >> (shift_index * 4)) & 0xF;
        do
        {
            if ((nibble != 0) || (started != 0))
            {
                addhero_hex_nibble_to_ascii(cursor, nibble);
                cursor += 1;
                remaining_chars -= 1;
                started = 1;
                remaining_value -= nibble << (shift_index * 4);
            }
        } while (0);
        do
        {
            shift_index -= 1;
        } while (0);
        if (shift_index != end_index)
        {
            if (shift_index == 0)
            {
                started = 1;
            }
            do
            {
                if (remaining_chars != 0)
                {
                    goto loop_2;
                }
            } while (0);
        }
    }
    *cursor = 0;
}

/**
 * @brief Write one nibble as its ASCII hex digit ('0'-'9', 'A'-'F'), or '_' for
 *        out-of-range values.
 * @param out   Destination byte.
 * @param value Nibble value to convert.
 * @see decomp.me (100%)
 */
void addhero_hex_nibble_to_ascii(s8 *out, s32 value)
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
u32 addhero_parse_hex(u8 *s, s32 len)
{
    u32 result;
    u32 tmp0;
    u32 tmp1;
    u32 tmp2;

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
            tmp0 = result - 0x30;
            result = tmp0 + *s;
        }
        else if ((u8)(*s - 'A') < 6)
        {
            tmp1 = result - 0x37;
            result = tmp1 + *s;
        }
        else if ((u8)(*s - 'a') < 6)
        {
            tmp2 = result - 0x57;
            result = tmp2 + *s;
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
s32 addhero_parse_hex_suffix_byte(u8 *text)
{
    u32 c;
    s32 count;
    u32 result;
    u32 tmp0;
    u32 tmp1;
    u32 tmp2;

    while (1)
    {
        c = *text;
        text++;
        if ((u32)(c - '0') < 10)
        {
            continue;
        }
        text--;
        if (text)
        {
            text++;
            text--;
        }

        text++;
        if ((u32)(c - 'a') < 6)
        {
            continue;
        }
        text--;
        if (text)
        {
            text++;
            text--;
        }

        text++;
        if ((u32)(c - 'A') < 6)
        {
            continue;
        }
        text--;
        if (text)
        {
            text++;
            text--;
        }
        break;
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
            tmp0 = result - 0x30;
            result = tmp0 + *text;
        }
        else if ((u8)(*text - 'A') < 6)
        {
            tmp1 = result - 0x37;
            result = tmp1 + *text;
        }
        else if ((u8)(*text - 'a') < 6)
        {
            tmp2 = result - 0x57;
            result = tmp2 + *text;
        }
        text++;
        count--;
    }
    return result;
}

extern s32 g_addhero_entry_fields[];

/**
 * @brief Parse the hex value suffix of every "SD"-tagged directory entry on the
 *        active card, recording per-entry field values and their ranked bytes.
 * @return The maximum suffix byte value seen across all matching entries.
 * @see decomp.me (100%) https://decomp.me/scratch/7hY8R
 */
s32 addhero_parse_entry_fields(void)
{
    s32 i;
    s32 max;
    u8 *p;
    u8 *field;
    s32 count;
    s32 acc;
    u32 tmp0, tmp1, tmp2;
    s32 r;
    s32 *fields;

    max = 0;

    for (i = 0; i < g_addhero_entry_state; i++)
    {
        char *ref;
        u8 *tmp;
        ref = D_800ECF7C;
        tmp = (u8 *)&((AddheroDirEntry (*)[20])g_addhero_entries)[g_addhero_card_slot][i];

        if (func_8001714C(ref, tmp, 0xC) == 0)
        {
            p = (u8 *)(
                g_addhero_card_slot * ADDHERO_CARD_DIRECTORY_BYTES +
                ((i << 4) + (i << 4) + (i << 3)) +
                (s32)g_addhero_entries +
                0xC);

            for (count = 5, acc = 0;
                 (((u8)(*p - '0') < 10) ||
                  ((u8)(*p - 'a') < 6) ||
                  ((u8)(*p - 'A') < 6)) &&
                     count != 0;
                 p++, count--)
            {
                acc <<= 4;

                if ((u8)(*p - '0') < 10)
                {
                    tmp0 = acc - 0x30;
                    acc = tmp0 + *p;
                }
                else if ((u8)(*p - 'A') < 6)
                {
                    tmp1 = acc - 0x37;
                    acc = tmp1 + *p;
                }
                else if ((u8)(*p - 'a') < 6)
                {
                    tmp2 = acc - 0x57;
                    acc = tmp2 + *p;
                }
            }

            field = (u8 *)&((AddheroDirEntry (*)[20])g_addhero_entries)[g_addhero_card_slot][i].name[0xC];
            fields = &g_addhero_entry_fields[g_addhero_card_slot * 20];
            fields[i] = acc;

            r = addhero_parse_hex_suffix_byte(field);
            g_addhero_entry_suffix_values[i] = r;

            if (max < r)
            {
                max = r;
            }
        }
        else
        {
            s32 *fields = &g_addhero_entry_fields[g_addhero_card_slot * 20];
            fields[i] = -1;
            g_addhero_entry_suffix_values[i] = 0;
        }
    }

    return max;
}

typedef struct AddheroEntryHeader
{
    s32 unk0;
    s16 unk4;
    s8 unk6;
    u8 pad[9];
} AddheroEntryHeader;

/** @brief Save-file header block at g_addhero_file_template (only the first 6 bytes used). */
typedef struct
{
    s32 unk0;
    s16 unk4;
    u8 pad[0xFA];
} AddheroFileHeader;

typedef struct
{
    s32 unk0;
    s16 unk4;
    s16 unk6;
    u8 unk8[0x18];
} AddheroFileHeaderScratch;

typedef struct
{
    s32 unk0;
    s16 unk4;
    u8 pad[0x62];
} AddheroLoadScratch;

/* --- text renderer (addhero_draw_cached_text family) --- */

typedef union
{
    u32 raw;
    struct
    {
        u16 code;
        u16 flags;
    } data;
} AddheroGlyphCacheEntry;

typedef struct
{
    SPRT_16 packet;
    u32 padding;
} AddheroGlyphSprite;

#define GLYPH_CACHE_SLOTS 0x100
#define GLYPH_CACHE_COLUMNS 16
#define GLYPH_CACHE_ROW_MASK 0xF0
#define GLYPH_RASTER_BYTES 0x80
#define GPU_ADDR_MASK 0xFFFFFF
#define GPU_TAG_HIGH_MASK 0xFF000000

extern AddheroDirEntry g_addhero_entries[][20];
extern AddheroFileHeader g_addhero_file_template;
extern AddheroEntryHeader g_addhero_entry_header_template;
extern AddheroGlyphCacheEntry g_addhero_glyph_cache[];

extern s32 g_addhero_scroll_y;
extern s32 g_addhero_progress_active;
extern s32 g_addhero_scroll_target_y;
extern s32 g_addhero_mode;
extern s32 g_addhero_entry_state;
extern s32 g_addhero_card_slot;
extern s32 g_addhero_selected_row;
extern s32 g_addhero_selection_status;
extern s32 g_addhero_scroll_frames;
extern s32 g_addhero_io_busy;
extern s32 g_addhero_retry_count;
extern s32 g_addhero_progress_bar_active;
extern s32 g_addhero_selected_entry_extended;
extern s32 g_addhero_progress_start_tick;
extern s32 g_addhero_primary_poll_countdown;
extern s32 g_addhero_entry_value_limit;
extern s32 g_addhero_entry_scan_active;
extern s32 g_addhero_entry_fields[];
extern s32 g_addhero_secondary_poll_countdown;
extern s32 g_addhero_primary_handle0;
extern s32 g_addhero_primary_handle1;
extern s32 g_addhero_primary_handle2;
extern s32 g_addhero_primary_handle3;
extern s32 g_addhero_has_free_entry_space;
extern s32 g_addhero_write_in_progress;
extern s32 g_addhero_file_handle;
extern s32 g_addhero_entry_ranks[];
extern s32 g_addhero_secondary_handle0;
extern s32 g_addhero_secondary_handle1;
extern s32 g_addhero_secondary_handle2;
extern s32 g_addhero_secondary_handle3;
extern s32 g_addhero_entry_suffix_values[];
extern s32 g_addhero_rank_count;

extern s32 g_addhero_glyph_cursor_x;
extern s32 g_addhero_glyph_cursor_y;
extern s32 g_addhero_text_line_start_x;
extern s32 g_addhero_glyph_upload_x;
extern s32 g_addhero_glyph_upload_y;

extern u8 *g_addhero_load_step;
extern u8 *g_addhero_glyph_raster_cursor;

extern u8 D_8016057C[];
extern u8 D_8016058C[];
extern u8 g_addhero_single_byte_char_table[];
extern u8 g_addhero_double_byte_char_table[];
extern u8 g_addhero_save_blob[];
extern u8 D_80164B20[];
extern u8 g_addhero_glyph_raster_buffer[];
extern u8 D_801654E0[];

extern u16 g_addhero_decimal_glyphs[];
extern u16 g_addhero_hex_glyphs[];

extern char D_800ECF7C[];
extern char D_800ECF8C[];
extern char D_800ECF9C[];
extern char D_800ECFB0[];
extern char D_800ECFC4[];

/* In-file functions */
s32 addhero_rank_entries(s32 unused0, s32 unused1, s32 unused2);
void addhero_reset_entry_ranks(void);
s32 addhero_has_known_entry_type(void);
s32 addhero_entry_blocks_reach_limit(void);
void addhero_render_fixed_prompts(void);
s32 addhero_advance_load_sequence(void);
void addhero_restart_load_sequence(void);
s32 addhero_poll_and_rewind_primary_handles(void);
void addhero_init_stream_handles(void);
void addhero_shutdown_stream_handles(void);
s32 addhero_begin_entry_scan(s32 page);
s32 addhero_scan_next_entry(s32 page);
void addhero_commit_selected_entry(void);
void addhero_release_primary_handles(void);
void addhero_release_secondary_handles(void);
s32 addhero_poll_primary_handle_group(void);
s32 addhero_poll_secondary_handle_group(void);
void addhero_sort_entries_by_type(void);
s32 addhero_draw_signed_decimal(s32 prim, s32 *ot, s32 value, s32 x, s32 y, s32 palette, s32 alignment);
void addhero_draw_hex_byte(s32 prim, s32 ot, s32 byte_value, s32 x, s32 y, s32 alignment);
s32 addhero_draw_cached_text(s32 prim, s32 *ot, u8 *text, s32 x, s32 y, s32 palette, s32 alignment);
s32 addhero_render_cached_glyph(s32 prim, s32 *ot, s32 character_code, s32 palette);
s32 addhero_emit_glyph_sprite(AddheroGlyphSprite *sprite, s32 *ot, s32 cache_slot, s32 palette);
void addhero_begin_glyph_cache_frame(void);
void addhero_evict_unused_glyphs(void);
void addhero_reset_glyph_cache(void);
void addhero_expand_text_glyph_codes(u8 *out, u8 *in);

/* External functions (defined in addhero.c or elsewhere) */
s32 addhero_parse_entry_fields();
void addhero_scroll_to_selection(void);
void addhero_open_status_dialog(s32 arg0);
void addhero_open_exit_dialog(s32 arg0);
void func_800AA02C(void);
s32 func_8001714C(void *a, void *b, s32 n);
s32 func_80016F9C(void *a, void *b);
s32 func_8001680C(void *a, s32 b);
s32 func_8001681C(s32 a, void *b, s32 c);
s32 func_8001682C(s32 a, void *b, s32 c);
s32 func_8001683C(s32 a);
s32 func_8001685C(void *a, void *b);
s32 func_8001686C(void *a);
s32 func_800170BC(void *a, void *b, ...);
s32 func_8001724C(s32 a);
s32 func_8001725C(s32 a);
s32 func_8001729C(s32 a);
s32 func_800172AC(s32 a);
s32 func_8002054C(s32 a);
s32 func_80032174(s32 a, void *b, s32 *c);
s32 func_800342CC(s32 a);
s32 func_80016BCC(void *a, void *b);
void func_800B0170(void *a);
s32 func_8001684C(void *a);
void func_80016E7C(void *a, void *b, s32 c);
s32 func_8001687C(s32 a);
void func_80019A34(RECT *rect, void *str);
void func_80019788(s32 arg0);
void func_800158E0(void);
s32 func_800167AC(s32 a, s32 b, s32 c, s32 d);
void func_800167BC(s32 a);
s32 func_800167CC(s32 a);
void func_800167DC(s32 a);
void func_800167EC(void);
void func_800167FC(void);

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
    s32 *row;
    s32 *elem;
    s32 *rank_ptr;
    s32 *cmp_ptr;
    s32 *inc_ptr;
    s32 *base_rank;
    s32 *ecopy;
    s32 *max_ptr;
    s32 *field_base;
    s32 *field1;
    s32 slot;
    s32 *out_ptr;
    char *ent_ptr;
    s32 t0v;
    s32 i;
    s32 s3v;
    s32 count;
    s32 handle;
    s32 less_count;
    s32 j;

    addhero_parse_entry_fields();
    s3v = -1;
    addhero_sort_entries_by_type();
    i = 0;
    handle = addhero_parse_entry_fields();
    addhero_reset_entry_ranks();
    t0v = 1;
    if (g_addhero_entry_state > 0)
    {
        count = g_addhero_entry_state;
        base_rank = &g_addhero_entry_ranks[0];
        rank_ptr = base_rank;
        slot = g_addhero_card_slot;
        field1 = g_addhero_entry_fields;
        row = field1 + slot * 20;
        elem = row;
        do
        {
            if (*elem >= 0)
            {
                j = 0;
                if (i > 0)
                {
                    j += 1; j -= 1;
                }
                if (*elem >= s3v)
                {
                    *rank_ptr = t0v;
                    s3v = *elem;
                    t0v += 1;
                }
                else
                {
                    less_count = j;
                    if (i > 0)
                    {
                        ecopy = elem;
                        inc_ptr = base_rank;
                        cmp_ptr = row;
                        do
                        {
                            if (*ecopy < *cmp_ptr)
                            {
                                less_count += 1;
                                *inc_ptr += 1;
                            }
                            inc_ptr += 1;
                            j += 1;
                            cmp_ptr += 1;
                        } while (j < i);
                    }
                    {
                        s32 rank_value;
                        do { do { do { rank_value = t0v - less_count; } while (0); } while (0); } while (0);
                        *rank_ptr = rank_value;
                    }
                    t0v += 1;
                }
            }
            rank_ptr += 1;
            i += 1;
            elem += 1;
        } while (i < count);
    }
    cmp_ptr = base_rank;
    inc_ptr = row;
    g_addhero_rank_count = t0v;
    t0v = -1;
    i = 0;
    s3v = 0;
    if (g_addhero_entry_state > 0)
    {
        s32 max_count;
        max_count = g_addhero_entry_state;
        slot = g_addhero_card_slot;
        field_base = g_addhero_entry_fields;
        max_ptr = (s32 *)((slot * 0x50) + (s32)field_base);
        do
        {
            if (t0v < *max_ptr)
            {
                t0v = *max_ptr;
                s3v = i;
            }
            i += 1;
            max_ptr += 1;
        } while (i < max_count);
        i = 0;
    }
    g_addhero_entry_value_limit = t0v + 1;
    if (g_addhero_entry_state > 0)
    {
        out_ptr = &g_addhero_entry_suffix_values[0];
        ent_ptr = (char *)&g_addhero_entries[0];
    loop_20:
        if (func_8001714C(&D_800ECFC4[0], (void *)((g_addhero_card_slot * ADDHERO_CARD_DIRECTORY_BYTES) + (s32)ent_ptr), 8) == 0)
        {
            *out_ptr = handle + 1;
        }
        else
        {
            out_ptr += 1;
            ent_ptr += 0x28;
            i += 1;
            if (i < g_addhero_entry_state)
            {
                goto loop_20;
            }
        }
    }
    return s3v;
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
    s32 i;
    u8 *entry;

    i = 0;
    if (g_addhero_entry_state > 0)
    {
        do
        {
            entry = (u8 *)g_addhero_entries + i * ADDHERO_DIRECTORY_ENTRY_BYTES;
            if (func_8001714C(&D_800ECF7C, (void *)(g_addhero_card_slot * ADDHERO_CARD_DIRECTORY_BYTES + (s32)entry), 0xC) == 0 ||
                func_8001714C(&D_800ECF8C, (void *)(g_addhero_card_slot * ADDHERO_CARD_DIRECTORY_BYTES + (s32)entry), 0xC) == 0)
            {
                return 1;
            }
            i++;
        } while (i < g_addhero_entry_state);
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
    s32 i;
    s32 sum;
    s32 offset;

    i = 0;
    sum = 0;
    if (g_addhero_entry_state > 0)
    {
        offset = g_addhero_card_slot * ADDHERO_CARD_DIRECTORY_BYTES;
        do
        {
            do {
                sum += ((AddheroDirEntry *)((u8 *)g_addhero_entries + offset))->size / 8192;
            } while (0);
            i++;
            offset += ADDHERO_DIRECTORY_ENTRY_BYTES;
        } while (i < g_addhero_entry_state);
    }
    return sum >= 0xE;
}

/**
 * @brief Issue the two fixed card-directory probe requests for the active slot.
 * @see decomp.me (100%)
 */
void addhero_render_fixed_prompts(void)
{
    AddheroFileHeaderScratch buf;

    memcpy(&buf, &g_addhero_file_template, 6);
    ((u8 *)&buf)[2] += *(u8 *)&g_addhero_card_slot;
    func_80016F9C(&buf, &D_800ECF9C);
    func_8001686C(&buf);

    memcpy(&buf, &g_addhero_file_template, 6);
    ((u8 *)&buf)[2] += *(u8 *)&g_addhero_card_slot;
    func_80016F9C(&buf, &D_800ECFB0);
    func_8001686C(&buf);
}

/**
 * @brief Inlined helper mirroring addhero_render_fixed_prompts: issue the two
 *        fixed card-directory probe requests for the active slot.
 * @see decomp.me (100%)
 */
static inline void addhero_probe_render_two(void)
{
    AddheroFileHeaderScratch p;

    memcpy(&p, &g_addhero_file_template, 6);
    ((u8 *)&p)[2] += *(u8 *)&g_addhero_card_slot;
    func_80016F9C(&p, &D_800ECF9C);
    func_8001686C(&p);

    memcpy(&p, &g_addhero_file_template, 6);
    ((u8 *)&p)[2] += *(u8 *)&g_addhero_card_slot;
    func_80016F9C(&p, &D_800ECFB0);
    func_8001686C(&p);
}

/**
 * @brief Execute one step of the card load/save/scan sequencer, dispatched by
 *        the current *g_addhero_load_step opcode through jtbl_80140098.
 * @return A phase code: 0/1 idle-ish, 2 done, 3 keep running, 4/5 error paths.
 * @note The computed-goto label table @c keep is discarded at link time; it only
 *       forces GCC to keep the label addresses that the jump table references.
 * @see decomp.me (100%)
 */
s32 addhero_advance_load_sequence(void)
{
    AddheroLoadScratch buf;
    s32 status0;
    s32 status1;
    s32 phase_result;
    s32 wait_attempts;
    s32 poll_result;
    s32 poll_result20;
    s32 rank_index;
    s32 rank_value;
    s32 dispatch;
    static void *const keep[] __attribute__((section(".discard"))) = {
        &&cl_case_0, &&cl_case_1, &&cl_case_2, &&cl_case_3,
        &&cl_case_4, &&cl_case_5, &&cl_case_6, &&block_return,
        &&cl_case_8, &&cl_case_9, &&cl_case_10, &&block_return,
        &&block_return, &&block_return, &&block_return, &&cl_case_15,
        &&cl_case_16, &&cl_case_17, &&cl_case_18, &&cl_case_19,
        &&cl_case_20, &&block_return, &&block_return, &&block_return,
        &&cl_case_24, &&cl_case_25, &&cl_case_26, &&cl_case_27,
        &&cl_case_28, &&block_return, &&cl_case_30
    };

    memcpy(&buf, &g_addhero_file_template, 6);
    phase_result = 1;
    ((u8 *)&buf)[2] += *(u8 *)&g_addhero_card_slot;

    if (g_addhero_load_step == NULL)
    {
        goto block_return;
    }

    switch (0)
    {
    case 0:
        dispatch = *g_addhero_load_step;
        if ((u32)dispatch >= 0x1F)
        {
            goto block_return;
        }
        goto *jtbl_80140098[dispatch];

    cl_case_1:
        phase_result = 3;
        func_8001729C(g_addhero_card_slot);
        func_8001724C(g_addhero_card_slot * 0x10);
        g_addhero_load_step = g_addhero_load_step + 1;
        break;

    cl_case_2:
        poll_result = addhero_poll_primary_handle_group();
        if (poll_result >= 3)
        {
            goto c2_ge3;
        }
        if (poll_result > 0)
        {
            goto c2_pos;
        }
        if (poll_result == 0)
        {
            goto c2_increment;
        }
        break;
    c2_ge3:
        if (poll_result == 3)
        {
            goto c2_eq3;
        }
        break;
    c2_increment:
        g_addhero_load_step = g_addhero_load_step + 1;
        break;
    c2_pos:
        phase_result = 4;
        g_addhero_selection_status = 0;
        g_addhero_entry_state = 0xFD;
        g_addhero_load_step = g_addhero_load_step + 1;
        break;
    c2_eq3:
        g_addhero_rank_count = 0x28;
        rank_value = -1;
        for (rank_index = 14; rank_index >= 0; rank_index--)
        {
            g_addhero_entry_ranks[rank_index] = rank_value;
        }
        g_addhero_entry_state = ADDHERO_ENTRY_STATE_IDLE;
        g_addhero_load_step = &D_80160574;
        break;

    cl_case_3:
        addhero_release_primary_handles();
        g_addhero_load_step = g_addhero_load_step + 1;
        break;

    cl_case_4:
        do
        {
            poll_result = addhero_poll_secondary_handle_group();
        } while (poll_result == -1);
        if (poll_result == 0)
        {
            g_addhero_load_step = g_addhero_load_step + 1;
            break;
        }
        if (poll_result < 0)
        {
            break;
        }
        if (poll_result >= 4)
        {
            break;
        }
        phase_result = 4;
        g_addhero_selection_status = 0;
        g_addhero_entry_state = 0xFD;
        break;

    cl_case_5:
        addhero_release_secondary_handles();
        g_addhero_load_step = g_addhero_load_step + 1;
        break;

    cl_case_6:
        addhero_probe_render_two();
        g_addhero_entry_scan_active = 1;
        if (addhero_begin_entry_scan(g_addhero_card_slot) == 0)
        {
            phase_result = 2;
            g_addhero_load_step = NULL;
            g_addhero_entry_state = 0xF8;
            g_addhero_entry_scan_active = 0;
            break;
        }
        wait_attempts = 0;
        g_addhero_load_step = g_addhero_load_step + 1;
        do
        {
            if (addhero_scan_next_entry(g_addhero_card_slot) == 0)
            {
                if (g_addhero_mode != 0)
                {
                    g_addhero_selected_row = 0;
                }
                g_addhero_entry_scan_active = 0;
                if (g_addhero_entry_state == 0xF8)
                {
                    break;
                }
                if (g_addhero_entry_state == 0xFA)
                {
                    break;
                }
                addhero_commit_selected_entry();
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        break;

    cl_case_8:
        phase_result = 3;
        func_8001729C(g_addhero_card_slot);
        func_800172AC(g_addhero_card_slot * 0x10);
        g_addhero_load_step = g_addhero_load_step + 1;
        break;

    cl_case_9:
        phase_result = 3;
        func_8001729C(g_addhero_card_slot);
        func_8001725C(g_addhero_card_slot * 0x10);
        g_addhero_primary_poll_countdown = 0x10;
        g_addhero_secondary_poll_countdown = 0x10;
        g_addhero_load_step = g_addhero_load_step + 1;
        break;

    cl_case_0:
        phase_result = 2;
        g_addhero_write_in_progress = 0;
        break;

    cl_case_10:
        func_80016F9C(&buf, (u8 *)g_addhero_entries + (g_addhero_card_slot * ADDHERO_CARD_DIRECTORY_BYTES) + (g_addhero_selected_row * ADDHERO_DIRECTORY_ENTRY_BYTES));
        wait_attempts = 0;
        do
        {
            if (func_8001686C(&buf) != 0)
            {
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        g_addhero_load_step = g_addhero_load_step + 1;
        break;

    cl_case_15:
        poll_result = addhero_poll_primary_handle_group();
        if (poll_result >= 3)
        {
            goto c15_ge3;
        }
        if (poll_result > 0)
        {
            goto c15_pos;
        }
        if (poll_result == 0)
        {
            goto c15_increment;
        }
        break;
    c15_ge3:
        if (poll_result == 3)
        {
            goto c15_eq3;
        }
        break;
    c15_increment:
        g_addhero_load_step = g_addhero_load_step + 1;
        break;
    c15_pos:
        g_addhero_secondary_poll_countdown = g_addhero_secondary_poll_countdown - 1;
        if (g_addhero_secondary_poll_countdown != 0)
        {
            goto block_reissue;
        }
        phase_result = 4;
        g_addhero_selection_status = 0;
        g_addhero_entry_state = 0xFD;
        break;
    c15_eq3:
        g_addhero_primary_poll_countdown = g_addhero_primary_poll_countdown - 1;
        if (g_addhero_primary_poll_countdown == 0)
        {
            goto c15_d70zero;
        }
    block_reissue:
        func_8001729C(g_addhero_card_slot);
        func_800172AC(g_addhero_card_slot * 0x10);
        func_8001729C(g_addhero_card_slot);
        func_8001725C(g_addhero_card_slot * 0x10);
        break;
    c15_d70zero:
        phase_result = 5;
        g_addhero_entry_state = 0xFC;
        g_addhero_load_step = D_8016057C;
        break;

    cl_case_16:
        do
        {
            poll_result = addhero_poll_secondary_handle_group();
        } while (poll_result == -1);
        g_addhero_load_step = g_addhero_load_step + 1;
        break;

    cl_case_17:
        g_addhero_io_busy = 1;
        g_addhero_selection_status = 0;
        func_8001729C(g_addhero_card_slot);
        g_addhero_file_handle = func_8001680C(D_801654E0, 0x8001);
        if (g_addhero_file_handle == -1)
        {
            break;
        }
        addhero_release_primary_handles();
        func_8001729C(g_addhero_card_slot);
        if (func_8001681C(g_addhero_file_handle, &D_80165208,
                           g_addhero_selected_entry_extended != 0 ? 0x280 : 0x80) == -1)
        {
            func_8001683C(g_addhero_file_handle);
            break;
        }
        g_addhero_load_step = g_addhero_load_step + 1;
        break;

    cl_case_18:
        if (g_addhero_io_busy != 0)
        {
            poll_result = addhero_poll_primary_handle_group();
            if (poll_result == 0)
            {
                g_addhero_io_busy = 0;
                g_addhero_selection_status = 1;
                func_8001683C(g_addhero_file_handle);
                break;
            }
            if (poll_result == -1)
            {
                break;
            }
            func_8001683C(g_addhero_file_handle);
            g_addhero_entry_state = ADDHERO_ENTRY_STATE_IDLE;
            g_addhero_load_step = &D_80160574;
        }
        else
        {
            g_addhero_load_step = g_addhero_load_step + 1;
        }
        break;

    cl_case_19:
        g_addhero_progress_active = 1;
        g_addhero_progress_start_tick = func_8002054C(-1);
        g_addhero_progress_bar_active = 1;
        func_8001729C(g_addhero_card_slot);
        g_addhero_file_handle = func_8001680C(D_801654E0, 0x8001);
        addhero_release_primary_handles();
        func_8001729C(g_addhero_card_slot);
        if (func_8001681C(g_addhero_file_handle, g_addhero_save_blob, 0x4000) == -1)
        {
            func_8001683C(g_addhero_file_handle);
            g_addhero_retry_count = g_addhero_retry_count - 1;
            if (g_addhero_retry_count == 0)
            {
            block_dialog_read:
                addhero_open_status_dialog(1);
                break;
            }
            break;
        }
        g_addhero_load_step = g_addhero_load_step + 1;
        break;

    cl_case_20:
        poll_result20 = addhero_poll_primary_handle_group();
        if (poll_result20 == 0)
        {
            g_addhero_progress_active = 0;
            g_addhero_load_step = g_addhero_load_step + 1;
            func_8001683C(g_addhero_file_handle);
            break;
        }
        if (poll_result20 < 0)
        {
            break;
        }
        if (poll_result20 >= 4)
        {
            break;
        }
        func_8001683C(g_addhero_file_handle);
        g_addhero_retry_count = g_addhero_retry_count - 1;
        if (g_addhero_retry_count == 0)
        {
            g_addhero_progress_bar_active = 0;
            goto block_dialog_read;
        }
        g_addhero_load_step = g_addhero_load_step - 1;
        break;

    cl_case_24:
        wait_attempts = 0;
        do
        {
            if (func_800342CC(g_addhero_card_slot * 0x10) == 1)
            {
                break;
            }
            func_8002054C(0);
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        if (wait_attempts != 0x14)
        {
            func_80032174(0, &status0, &status1);
            if (status1 == 0)
            {
                g_addhero_load_step = g_addhero_load_step + 1;
                break;
            }
        }
        addhero_open_status_dialog(3);
        break;

    cl_case_30:
        g_addhero_retry_count = 5;
        g_addhero_load_step = g_addhero_load_step + 1;
        break;

    cl_case_27:
        g_addhero_progress_active = 1;
        g_addhero_progress_start_tick = func_8002054C(-1);
        g_addhero_progress_bar_active = 1;
        func_8001729C(g_addhero_card_slot);
        g_addhero_file_handle = func_8001680C(D_801654E0, 0x8001);
        addhero_release_primary_handles();
        func_8001729C(g_addhero_card_slot);
        if (func_8001681C(g_addhero_file_handle, g_addhero_save_blob, 0x4000) == -1)
        {
            func_8001683C(g_addhero_file_handle);
            g_addhero_retry_count = g_addhero_retry_count - 1;
            if (g_addhero_retry_count == 0)
            {
            block_dialog_write_read:
                addhero_open_exit_dialog(1);
                break;
            }
            break;
        }
        g_addhero_load_step = g_addhero_load_step + 1;
        break;

    cl_case_28:
        poll_result20 = addhero_poll_primary_handle_group();
        if (poll_result20 == 0)
        {
            g_addhero_progress_active = 0;
            g_addhero_load_step = g_addhero_load_step + 1;
            func_8001683C(g_addhero_file_handle);
            break;
        }
        if (poll_result20 < 0)
        {
            break;
        }
        if (poll_result20 >= 4)
        {
            break;
        }
        g_addhero_retry_count = g_addhero_retry_count - 1;
        if (g_addhero_retry_count == 0)
        {
            g_addhero_progress_bar_active = 0;
            goto block_dialog_write_read;
        }
        goto block_close_decrement;

    cl_case_25:
        if (g_addhero_has_free_entry_space == 0)
        {
            func_8001729C(g_addhero_card_slot);
            wait_attempts = 0;
            do
            {
                if (func_8001686C(D_801654E0) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 0x14);
        }
        func_80016F9C(&buf, D_800ECF9C);
        func_8001729C(g_addhero_card_slot);
        g_addhero_file_handle = func_8001680C(&buf, 0x20200);
        if (g_addhero_file_handle != -1)
        {
            goto block_write_opened;
        }
        func_8001683C(-1);
        wait_attempts = 0;
        do
        {
            if (func_8001686C(&buf) != 0)
            {
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
    block_write_retry:
        g_addhero_retry_count = g_addhero_retry_count - 1;
        if (g_addhero_retry_count == 0)
        {
        block_dialog_write:
            addhero_open_exit_dialog(0);
            break;
        }
        break;

    block_write_opened:
        func_8001683C(g_addhero_file_handle);
        func_800170BC(D_80164B20, &buf);
        func_8001729C(g_addhero_card_slot);
        g_addhero_file_handle = func_8001680C(D_80164B20, 0x8002);
        addhero_release_primary_handles();
        g_addhero_progress_start_tick = func_8002054C(-1);
        g_addhero_progress_bar_active = 1;
        func_8001729C(g_addhero_card_slot);
        if (func_8001682C(g_addhero_file_handle, g_addhero_save_blob, 0x4000) == -1)
        {
            func_8001683C(g_addhero_file_handle);
            wait_attempts = 0;
            do
            {
                if (func_8001686C(D_80164B20) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 0x14);
            goto block_write_retry;
        }
        g_addhero_load_step = g_addhero_load_step + 1;
        break;

    cl_case_26:
        poll_result20 = addhero_poll_primary_handle_group();
        if (poll_result20 != 0)
        {
            if (poll_result20 < 0)
            {
                break;
            }
            if (poll_result20 >= 4)
            {
                break;
            }
            goto block_case26_retry;
        }
        if (g_addhero_has_free_entry_space != 0)
        {
            func_8001729C(g_addhero_card_slot);
            wait_attempts = 0;
            do
            {
                if (func_8001686C(D_801654E0) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 0x14);
        }
        func_8001729C(g_addhero_card_slot);
        wait_attempts = 0;
        do
        {
            if (func_8001685C(D_80164B20, D_801654E0) != 0)
            {
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        g_addhero_write_in_progress = 0;
        g_addhero_load_step = g_addhero_load_step + 1;
        func_8001683C(g_addhero_file_handle);
        break;

    block_case26_retry:
        g_addhero_retry_count = g_addhero_retry_count - 1;
        if (g_addhero_retry_count == 0)
        {
            g_addhero_progress_bar_active = 0;
            goto block_dialog_write;
        }
        goto block_close_decrement;

    }

    goto block_return;

block_close_decrement:
    func_8001683C(g_addhero_file_handle);
    g_addhero_load_step = g_addhero_load_step - 1;

block_return:
    return phase_result;
}

/**
 * @brief Rewind the active card and restart the load sequence from its first
 *        step.
 * @see decomp.me (100.00%)
 */
void addhero_restart_load_sequence(void)
{
    func_8001729C(g_addhero_card_slot);
    addhero_release_primary_handles();
    func_8001724C(g_addhero_card_slot * 0x10);
    g_addhero_load_step = D_8016057C;
}

/**
 * @brief Poll the primary handle group and, if any handle is still busy, rewind
 *        the active card so it can be retried.
 * @return The busy handle index, or -1 when all primary handles are idle.
 * @see decomp.me (100.00%)
 */
s32 addhero_poll_and_rewind_primary_handles(void)
{
    s32 busy_slot;

    busy_slot = addhero_poll_primary_handle_group();
    if (busy_slot != -1)
    {
        func_8001729C(g_addhero_card_slot);
        func_8001724C(g_addhero_card_slot * 0x10);
    }
    return busy_slot;
}

/**
 * @brief Allocate and register the four primary and four secondary card stream
 *        handles and clear the progress/scan flags.
 * @see decomp.me (100.00%)
 */
void addhero_init_stream_handles(void)
{
    func_800158E0();
    func_800167EC();
    g_addhero_primary_handle0 = func_800167AC(0xF4000001, 4, 0x2000, 0);
    g_addhero_primary_handle1 = func_800167AC(0xF4000001, 0x8000, 0x2000, 0);
    g_addhero_primary_handle2 = func_800167AC(0xF4000001, 0x100, 0x2000, 0);
    g_addhero_primary_handle3 = func_800167AC(0xF4000001, 0x2000, 0x2000, 0);
    g_addhero_secondary_handle0 = func_800167AC(0xF0000011, 4, 0x2000, 0);
    g_addhero_secondary_handle1 = func_800167AC(0xF0000011, 0x8000, 0x2000, 0);
    g_addhero_secondary_handle2 = func_800167AC(0xF0000011, 0x100, 0x2000, 0);
    g_addhero_secondary_handle3 = func_800167AC(0xF0000011, 0x2000, 0x2000, 0);
    func_800167DC(g_addhero_primary_handle0);
    func_800167DC(g_addhero_primary_handle1);
    func_800167DC(g_addhero_primary_handle2);
    func_800167DC(g_addhero_primary_handle3);
    func_800167DC(g_addhero_secondary_handle0);
    func_800167DC(g_addhero_secondary_handle1);
    func_800167DC(g_addhero_secondary_handle2);
    func_800167DC(g_addhero_secondary_handle3);
    func_800167FC();
    g_addhero_progress_bar_active = 0;
    g_addhero_entry_scan_active = 0;
}

/**
 * @brief Unregister and free the four primary and four secondary card stream
 *        handles.
 * @see decomp.me (100.00%)
 */
void addhero_shutdown_stream_handles(void)
{
    func_800158E0();
    func_800167EC();
    func_800167BC(g_addhero_primary_handle0);
    func_800167BC(g_addhero_primary_handle1);
    func_800167BC(g_addhero_primary_handle2);
    func_800167BC(g_addhero_primary_handle3);
    func_800167BC(g_addhero_secondary_handle0);
    func_800167BC(g_addhero_secondary_handle1);
    func_800167BC(g_addhero_secondary_handle2);
    func_800167BC(g_addhero_secondary_handle3);
    func_800167FC();
}

/**
 * @brief Reset browser state and read the first directory entry of the given
 *        card page, priming the scan.
 * @param page Card page index to begin scanning.
 * @return 1 if a first entry was read, 0 if the page is empty.
 * @see decomp.me (100.00%)
 */
s32 addhero_begin_entry_scan(s32 page)
{
    AddheroEntryHeader buf;

    memcpy(&buf, &g_addhero_entry_header_template, 7);
    g_addhero_selected_row = 0;
    g_addhero_scroll_frames = 0;
    g_addhero_scroll_target_y = 0;
    g_addhero_scroll_y = 0;
    g_addhero_entry_state = 0;
    ((u8 *)&buf)[2] += page;
    if (func_80016BCC(&buf, (u8 *)g_addhero_entries + page * ADDHERO_CARD_DIRECTORY_BYTES) != 0)
    {
        func_800B0170((u8 *)g_addhero_entries + page * ADDHERO_CARD_DIRECTORY_BYTES + g_addhero_entry_state * ADDHERO_DIRECTORY_ENTRY_BYTES);
        g_addhero_entry_state += 1;
        return 1;
    }
    return 0;
}

/**
 * @brief Advance one step of the add-hero entry load scan for the given page.
 * @param page Page index whose entry block is being scanned.
 * @return 1 if an entry was consumed this step, 0 otherwise.
 * @see decomp.me (100.00%)
 */
s32 addhero_scan_next_entry(s32 page)
{
    s32 i;
    s32 sum;
    s32 offset;
    s32 selected;
    s32 page_offset;
    s32 count;
    s32 cond;

    page_offset = page * ADDHERO_CARD_DIRECTORY_BYTES;
    if (func_8001684C((void *)((u8 *)g_addhero_entries + page_offset + g_addhero_entry_state * ADDHERO_DIRECTORY_ENTRY_BYTES)) != 0)
    {
        func_800B0170((void *)((u8 *)g_addhero_entries + page_offset + g_addhero_entry_state * ADDHERO_DIRECTORY_ENTRY_BYTES));
        g_addhero_entry_state += 1;
        return 1;
    }

    func_800AA02C();
    if ((g_addhero_mode == 0) && (addhero_has_known_entry_type() == 0))
    {
        g_addhero_entry_state = 0xF8;
    }
    else
    {
        i = 0;
        sum = 0;
        g_addhero_has_free_entry_space = 0;
        count = g_addhero_entry_state;
        if (count > 0)
        {
            u8 *entries;
            do { entries = (u8 *)g_addhero_entries; } while (0);
            offset = g_addhero_card_slot * ADDHERO_CARD_DIRECTORY_BYTES;
            do
            {
                sum += ((AddheroDirEntry *)(offset + (s32)entries))->size / 8192;
                i++;
                offset += ADDHERO_DIRECTORY_ENTRY_BYTES;
            } while (i < count);
        }
        cond = sum >= 0xE;
        if (cond != 0)
        {
            selected = addhero_rank_entries(sum, i, count);
            if (addhero_has_known_entry_type() == 0)
            {
                g_addhero_entry_state = 0xFA;
                g_addhero_entry_value_limit = 0;
            }
            else
            {
                if (g_addhero_mode != 0)
                {
                    g_addhero_selected_row = 0;
                }
                g_addhero_selected_row = selected;
                addhero_scroll_to_selection();
            }
        }
        else
        {
            g_addhero_has_free_entry_space = 1;
            selected = addhero_rank_entries(sum, i, count);
            if (addhero_has_known_entry_type() == 0)
            {
                g_addhero_selected_row = 0;
                addhero_scroll_to_selection();
                g_addhero_entry_value_limit = 0;
            }
            else
            {
                if (g_addhero_mode != 0)
                {
                    g_addhero_selected_row = 0;
                }
                g_addhero_selected_row = selected;
                addhero_scroll_to_selection();
            }
        }
    }
    return 0;
}

/**
 * @brief Prepare the currently selected directory entry for loading: set the
 *        selection status, build its file spec, and arm the read step.
 * @see decomp.me (100.00%)
 */
void addhero_commit_selected_entry(void)
{
    AddheroFileHeader local;
    u8 *p;

    if (g_addhero_entry_state == 0)
    {
        g_addhero_selection_status = 3;
        return;
    }
    {
        s32 term1;
        s32 term2;
        term1 = g_addhero_card_slot * ADDHERO_CARD_DIRECTORY_BYTES;
        term2 = (g_addhero_selected_row * ADDHERO_DIRECTORY_ENTRY_BYTES) + (s32)g_addhero_entries;
        if (func_8001714C(&D_800ECFC4[0], (void *)(term1 + term2), 8) == 0)
        {
            g_addhero_selection_status = 2;
            return;
        }
    }
    memcpy(&local, &g_addhero_file_template, 6);
    p = (u8 *)&local;
    {
        s32 term1;
        s32 term2;
        term1 = g_addhero_card_slot * ADDHERO_CARD_DIRECTORY_BYTES;
        term2 = (g_addhero_selected_row * ADDHERO_DIRECTORY_ENTRY_BYTES) + (s32)g_addhero_entries;
        func_80016F9C(p, (void *)(term1 + term2));
    }
    {
        s32 slot;
        s32 value;
        value = *((u8 *)&local + 2);
        slot = (u8)g_addhero_card_slot;
        g_addhero_selection_status = 0;
        value += slot;
        *((u8 *)&local + 2) = value;
        func_800170BC(&D_801654E0[0], p, slot);
    }
    g_addhero_load_step = &D_8016058C[0];
    {
        s32 term1;
        s32 term2;
        term1 = g_addhero_card_slot * ADDHERO_CARD_DIRECTORY_BYTES;
        term2 = (g_addhero_selected_row * ADDHERO_DIRECTORY_ENTRY_BYTES) + (s32)g_addhero_entries;
        if (func_8001714C(&D_800ECF7C[0], (void *)(term1 + term2), 0xC) == 0)
            g_addhero_selected_entry_extended = 1;
        else
            g_addhero_selected_entry_extended = 0;
    }
    g_addhero_io_busy = 1;
}

/**
 * @brief Release (poll to idle) all four primary card stream handles.
 * @see decomp.me (100.00%)
 */
void addhero_release_primary_handles(void)
{
    func_800167CC(g_addhero_primary_handle0);
    func_800167CC(g_addhero_primary_handle1);
    func_800167CC(g_addhero_primary_handle2);
    func_800167CC(g_addhero_primary_handle3);
}

/**
 * @brief Release (poll to idle) all four secondary card stream handles.
 * @see decomp.me (100.00%)
 */
void addhero_release_secondary_handles(void)
{
    func_800167CC(g_addhero_secondary_handle0);
    func_800167CC(g_addhero_secondary_handle1);
    func_800167CC(g_addhero_secondary_handle2);
    func_800167CC(g_addhero_secondary_handle3);
}

/**
 * @brief Poll the four primary card stream handles for one that is busy.
 * @return Index (0-3) of the first busy handle, or -1 when all are idle.
 * @see decomp.me (100.00%)
 */
s32 addhero_poll_primary_handle_group(void)
{
    if (func_800167CC(g_addhero_primary_handle0) == 1)
    {
        return 0;
    }
    if (func_800167CC(g_addhero_primary_handle1) == 1)
    {
        return 1;
    }
    if (func_800167CC(g_addhero_primary_handle2) == 1)
    {
        return 2;
    }
    if (func_800167CC(g_addhero_primary_handle3) == 1)
    {
        return 3;
    }
    return -1;
}

/**
 * @brief Poll the four secondary card stream handles for one that is busy.
 * @return Index (0-3) of the first busy handle, or -1 when all are idle.
 * @see decomp.me (100.00%)
 */
s32 addhero_poll_secondary_handle_group(void)
{
    if (func_800167CC(g_addhero_secondary_handle0) == 1)
    {
        return 0;
    }
    if (func_800167CC(g_addhero_secondary_handle1) == 1)
    {
        return 1;
    }
    if (func_800167CC(g_addhero_secondary_handle2) == 1)
    {
        return 2;
    }
    if (func_800167CC(g_addhero_secondary_handle3) == 1)
    {
        return 3;
    }
    return -1;
}

/**
 * @brief Reorder the active card's directory entries into a stable grouping:
 *        by suffix value within each known name prefix, then a third prefix,
 *        then any remaining entries, writing the result back in place.
 * @see decomp.me (100%)
 */
void addhero_sort_entries_by_type(void)
{
    AddheroDirEntry sorted[20];
    s32 out = 0;
    s32 group = 0;
    s32 i;
    do {
        i = 0;
        if (i < g_addhero_entry_state) {
            do {
                if (g_addhero_entry_suffix_values[i] == group &&
                    func_8001714C(D_800ECF7C, &g_addhero_entries[g_addhero_card_slot][i], 0xC) == 0) {
                    func_80016E7C(&g_addhero_entries[g_addhero_card_slot][i], &sorted[out], 0x28);
                    out++;
                }
                i++;
            } while (i < g_addhero_entry_state);
        }
        group++;
    } while (group < 8);

    group = 0;
    do {
        i = 0;
        if (i < g_addhero_entry_state) {
            do {
                if (g_addhero_entry_suffix_values[i] == group &&
                    func_8001714C(D_800ECF8C, &g_addhero_entries[g_addhero_card_slot][i], 0xC) == 0) {
                    func_80016E7C(&g_addhero_entries[g_addhero_card_slot][i], &sorted[out], 0x28);
                    out++;
                }
                i++;
            } while (i < g_addhero_entry_state);
        }
        group++;
    } while (group < 8);

    i = 0;
    if (g_addhero_entry_state > 0) {
        do {
            if (func_8001714C(D_800ECFC4, &g_addhero_entries[g_addhero_card_slot][i], 8) == 0) {
                func_80016E7C(&g_addhero_entries[g_addhero_card_slot][i], &sorted[out], 0x28);
                out++;
            }
            i++;
        } while (i < g_addhero_entry_state);
    }

    if (*(volatile s32 *)&g_addhero_entry_state > 0) {
        i = 0;
        do {
            if (func_8001714C(D_800ECF7C, &g_addhero_entries[g_addhero_card_slot][i], 0xC) != 0 &&
                func_8001714C(D_800ECF8C, &g_addhero_entries[g_addhero_card_slot][i], 0xC) != 0 &&
                func_8001714C(D_800ECFC4, &g_addhero_entries[g_addhero_card_slot][i], 8) != 0) {
                func_80016E7C(&g_addhero_entries[g_addhero_card_slot][i], &sorted[out], 0x28);
                out++;
            }
            i++;
        } while (i < g_addhero_entry_state);
    }

    i = 0;
    if (g_addhero_entry_state > 0) {
        do {
            func_80016E7C(&sorted[i], &g_addhero_entries[g_addhero_card_slot][i], 0x28);
            i++;
        } while (i < g_addhero_entry_state);
    }
}

/**
 * @brief Render a signed decimal value as cached-glyph text, suppressing
 *        leading zeros and prefixing a minus glyph when negative.
 * @param prim      Current primitive pointer/index.
 * @param ot        Ordering table the glyphs are linked into.
 * @param value     Signed value to render.
 * @param x         X position (interpreted per @p alignment).
 * @param y         Y baseline.
 * @param palette   Glyph palette index.
 * @param alignment Text alignment mode passed to addhero_draw_cached_text.
 * @return The updated primitive pointer.
 * @see decomp.me (100%)
 */
s32 addhero_draw_signed_decimal(s32 prim, s32 *ot, s32 value, s32 x, s32 y, s32 palette, s32 alignment)
{
    u16 buf[7];
    s32 first_digit;
    s32 magnitude;
    s32 negative;

    magnitude = value;
    if (magnitude < 0)
    {
        magnitude = -magnitude;
        negative = 1;
    }
    else
    {
        negative = 0;
    }
    buf[1] = g_addhero_decimal_glyphs[magnitude / 10000];
    buf[2] = g_addhero_decimal_glyphs[(magnitude % 10000) / 1000];
    buf[3] = g_addhero_decimal_glyphs[(magnitude % 1000) / 100];
    buf[4] = g_addhero_decimal_glyphs[(magnitude % 100) / 10];
    buf[5] = g_addhero_decimal_glyphs[magnitude % 10];

    first_digit = 1;
    buf[6] = 0;

    while (first_digit < 5 && buf[first_digit] == 0x4F82)
    {
        first_digit++;
    }

    if (negative != 0)
    {
        first_digit--;
        buf[first_digit] = 0x5B81;
    }
    prim = addhero_draw_cached_text(prim, ot, (u8 *)&buf[first_digit], x, y, palette, alignment);
    return prim;
}

/**
 * @brief Render a byte as two hex-digit glyphs via the cached-text renderer.
 * @param prim       Current primitive pointer/index.
 * @param ot         Ordering table the glyphs are linked into.
 * @param byte_value Byte value to render.
 * @param x          X position.
 * @param y          Y baseline.
 * @param alignment  Text alignment mode.
 * @see decomp.me (100%)
 */
void addhero_draw_hex_byte(s32 prim, s32 ot, s32 byte_value, s32 x, s32 y, s32 alignment)
{
    u16 pair[3];
    s32 row;
    s32 adjusted;
    s32 off;
    u16 *base;

    adjusted = byte_value;
    if (byte_value < 0)
    {
        adjusted = byte_value + 15;
    }
    row = adjusted >> 4;
    off = row * 2;
    base = g_addhero_hex_glyphs;
    pair[0] = *(u16 *)((u8 *)base + off);
    off = (byte_value - row * 16) * 2;
    pair[1] = *(u16 *)((u8 *)base + off);
    pair[2] = 0;
    addhero_draw_cached_text(prim, ot, pair, x, y, 0, alignment);
}

/**
 * @brief Render a multibyte string through the glyph cache: measure it, apply
 *        left/center/right alignment, then emit one cached glyph per character
 *        and terminate the primitive list.
 * @param prim      Current primitive pointer/index.
 * @param ot        Ordering table the glyphs are linked into.
 * @param text      Null-terminated multibyte string to render.
 * @param x         X position (interpreted per @p alignment).
 * @param y         Y baseline.
 * @param palette   Glyph palette index.
 * @param alignment 0 left, 1 right (16px/char), 2 right (8px/char).
 * @return The updated primitive pointer past the terminator.
 * @see decomp.me (100%)
 */
s32 addhero_draw_cached_text(s32 prim, s32 *ot, u8 *text, s32 x, s32 y, s32 palette, s32 alignment)
{
    u8 *cursor;
    s32 count;
    u16 code;
    u8 *scan;

    cursor = text;
    count = 0;
    if (*cursor >= 0x20)
    {
        scan = cursor;
        do
        {
            code = *scan;
            if (code >= 0x80)
            {
                scan++;
            }
            scan++;
            count++;
        } while (*scan >= 0x20);
    }

    switch (alignment)
    {
    case 1:
        x -= count * 0x10;
        break;
    case 2:
        x -= count * 8;
        break;
    case 0:
    default:
        break;
    }
    g_addhero_text_line_start_x = x;
    g_addhero_glyph_cursor_x = x;
    g_addhero_glyph_cursor_y = y;

    while (1)
    {
        u32 lead = *cursor;

        if ((u8)lead == 0x20)
        {
            cursor++;
            g_addhero_glyph_cursor_x += 0x10;
            continue;
        }
        if ((u8)lead >= 0x80)
        {
            code = cursor[0];
            code = (code << 8) | cursor[1];
            cursor += 2;
        }
        else
        {
            if ((u8)lead < 0x20)
            {
                break;
            }
            if ((u32)(lead - 0x30) < 0x50)
            {
                code = *cursor - 0x7DE1;
                cursor++;
            }
            else
            {
                code = *cursor - 0x7AE1;
                cursor++;
            }
        }
        prim = addhero_render_cached_glyph(prim, ot, code, palette);
    }

    setDrawTPage(prim, 0, 0, 5);
    addPrim(ot, prim);
    return prim + 8;
}

/**
 * @brief Emit one glyph sprite, rasterizing and uploading the glyph to the VRAM
 *        cache first when it is not already cached.
 * @param prim           Current primitive pointer/index.
 * @param ot             Ordering table the sprite is linked into.
 * @param character_code Glyph code to render.
 * @param palette        Glyph palette index used when rasterizing.
 * @return The updated primitive pointer, unchanged when the glyph is missing or
 *         the cache is full.
 * @see decomp.me (100%)
 */
s32 addhero_render_cached_glyph(s32 prim, s32 *ot, s32 character_code, s32 palette)
{
    AddheroGlyphCacheEntry *entry;
    u8 *font_data;
    s32 font_address;
    u32 requested_code;
    s32 slot;
    s32 high_pixel_set;
    s32 code;
    RECT rect;

    u8 *raster;
    s32 color_index;
    s32 high_nibble_color;
    s32 row;
    s32 source_byte;

    u16 mask;
    volatile u8 *raster_byte;
    u8 packed_pixels;

    code = character_code;
    slot = 0;
    requested_code = code & 0xFFFF;
    entry = g_addhero_glyph_cache;

    while (slot < GLYPH_CACHE_SLOTS)
    {
        if (requested_code == entry->data.code)
        {
            return addhero_emit_glyph_sprite((AddheroGlyphSprite *)prim, ot, slot, palette);
        }
        slot++;
        entry++;
    }

    font_address = func_8001687C(code & 0xFFFF);
    font_data = (u8 *)font_address;
    if (font_address == -1)
    {
        return prim;
    }

    raster = g_addhero_glyph_raster_cursor;
    row = 0;
    color_index = (palette + 1) * 2;
    high_nibble_color = color_index * 16;
    for (; row < 15; row++)
    {
        for (source_byte = 0; source_byte < 2; source_byte++)
        {
            mask = 0x80;

            for (slot = 0; slot < 4; slot++)
            {
                *raster = ((*font_data) & mask) ? color_index : 0;

                mask >>= 1;
                high_pixel_set = (*font_data) & mask;

                raster_byte = raster;
                packed_pixels = *raster_byte;
                if (high_pixel_set)
                {
                    packed_pixels += high_nibble_color;
                }

                *raster_byte = packed_pixels;

                mask >>= 1;
                raster++;
            }

            font_data++;
        }
    }

    slot = 0;
    while ((slot < GLYPH_CACHE_SLOTS) && (g_addhero_glyph_cache[slot].raw != 0))
    {
        slot++;
    }

    if (slot == GLYPH_CACHE_SLOTS)
    {
        return prim;
    }
    g_addhero_glyph_cache[slot].raw = code & 0xFFFF;
    prim = addhero_emit_glyph_sprite((AddheroGlyphSprite *)prim, ot, slot, palette);

    g_addhero_glyph_upload_x = (slot % GLYPH_CACHE_COLUMNS) * 4;
    g_addhero_glyph_upload_y = slot & GLYPH_CACHE_ROW_MASK;

    rect.w = 4;
    rect.h = 15;
    rect.x = g_addhero_glyph_upload_x + 0x140;
    rect.y = g_addhero_glyph_upload_y;

    func_80019A34(&rect, g_addhero_glyph_raster_cursor);
    func_80019788(0);

    g_addhero_glyph_raster_cursor += GLYPH_RASTER_BYTES;
    return prim;
}

/**
 * @brief Write a 16x16 sprite for a cached glyph at the current text cursor,
 *        mark the slot used, and advance the cursor (wrapping to the next line).
 * @param sprite     Destination sprite primitive.
 * @param ot         Ordering table the sprite is linked into.
 * @param cache_slot Glyph cache slot whose VRAM tile to sample.
 * @param palette    Unused here; the CLUT is fixed.
 * @return The primitive pointer advanced past the emitted sprite.
 * @see decomp.me (100%)
 */
s32 addhero_emit_glyph_sprite(AddheroGlyphSprite *sprite, s32 *ot, s32 cache_slot, s32 palette)
{
    u32 ot_tag_high_byte;
    s32 normalized_slot;
    u32 packet_address;
    s32 old_x;
    s32 new_x;
    s32 fits_line;

    g_addhero_glyph_cache[cache_slot].raw |= 0x10000;

    setSprt16(sprite);
    sprite->packet.g0 = 0x80;
    sprite->packet.b0 = 0x80;
    sprite->packet.r0 = 0x80;
    normalized_slot = cache_slot;
    setXY0(&sprite->packet, g_addhero_glyph_cursor_x, g_addhero_glyph_cursor_y);

    if (cache_slot < 0)
    {
        normalized_slot = cache_slot + 15;
    }

    setUV0(&sprite->packet, (cache_slot - ((normalized_slot >> 4) * 16)) * 16,
           cache_slot & GLYPH_CACHE_ROW_MASK);
    sprite->packet.clut = 0x7FD3;
    sprite->packet.tag = (sprite->packet.tag & GPU_TAG_HIGH_MASK) | (*ot & GPU_ADDR_MASK);

    packet_address = ((u32)sprite) & GPU_ADDR_MASK;
    ot_tag_high_byte = *ot & GPU_TAG_HIGH_MASK;

    sprite++;
    old_x = g_addhero_glyph_cursor_x;
    new_x = old_x + 16;
    fits_line = (old_x + 32) < 0x280;
    g_addhero_glyph_cursor_x = new_x;

    *ot = ot_tag_high_byte | packet_address;

    if (!fits_line)
    {
        g_addhero_glyph_cursor_x = g_addhero_text_line_start_x;
        g_addhero_glyph_cursor_y += 16;
    }

    return (s32)sprite;
}

/**
 * @brief Start a new glyph cache frame: rewind the raster cursor and clear each
 *        cache entry's per-frame "used" flag (the high half-word).
 * @see decomp.me (100.00%)
 */
void addhero_begin_glyph_cache_frame(void)
{
    s32 i;
    s32 *p;

    g_addhero_glyph_raster_cursor = g_addhero_glyph_raster_buffer;
    for (i = 0, p = (s32 *)g_addhero_glyph_cache; i < 0x100; i++, p++)
    {
        *p = (u16)*p;
    }
}

/**
 * @brief Evict cache entries not touched this frame by zeroing any slot whose
 *        "used" flag (bit 0x10000) is clear.
 * @see decomp.me (100.00%)
 */
void addhero_evict_unused_glyphs(void)
{
    s32 i;
    s32 *p;
    s32 flag;

    i = 0;
    flag = 0x10000;
    p = (s32 *)g_addhero_glyph_cache;
    for (; i < 0x100; i++, p++)
    {
        if (!(*p & flag))
        {
            *p = 0;
        }
    }
}

/**
 * @brief Fully reset the glyph cache: zero all 0x100 cache entries and clear the
 *        entire 0x8000-byte glyph raster buffer.
 * @see decomp.me (100.00%)
 */
void addhero_reset_glyph_cache(void)
{
    s32 i;
    s32 *p;
    u8 *q;

    i = 0xFF;
    p = (s32 *)g_addhero_glyph_cache;
    p += 0xFF;
    for (; i >= 0; i--, p--)
    {
        *p = 0;
    }

    i = 0;
    q = g_addhero_glyph_raster_buffer;
    for (; i <= 0x7FFF; i++)
    {
        *(u8 *)(i + (s32)q) = 0;
    }
}

/**
 * @brief Translate a source string into internal glyph codes via the single-
 *        and double-byte character tables, writing two output bytes per input
 *        character and null-terminating the result.
 * @param out Destination glyph-code buffer.
 * @param in  Null-terminated source string.
 * @see decomp.me (100%)
 */
void addhero_expand_text_glyph_codes(u8 *out, u8 *in)
{
    u32 c;
    s32 index;
    s16 lead;

    for (;;)
    {
        c = *in;
        if ((u8)c == 0)
        {
            goto done;
        }
        if ((u32)(c - 0x19) < 7)
        {
            u32 b1;
            s32 off;
            u8 *pa;
            u8 *pb;

            b1 = in[1];
            off = b1 >> 4;
            b1 &= 0xF;
            pa = g_addhero_double_byte_char_table + b1 * 2;
            pa += off * 33;
            lead = *in;
            pa += lead * 528;
            *out = *pa;
            out++;
            b1 = in[1];
            off = b1 >> 4;
            b1 &= 0xF;
            pb = g_addhero_double_byte_char_table + 1 + b1 * 2;
            pb += off * 33;
            lead = *in;
            pb += lead * 528;
            *out = *pb;
            out++;
            in += 2;
        }
        else if ((u8)c >= 0x21)
        {
            lead = *in;
            index = lead - 0x20;
            *out = g_addhero_single_byte_char_table[(index / 16) * 33 + (index & 0xF) * 2];
            out++;
            lead = *in;
            index = lead - 0x20;
            *out = g_addhero_single_byte_char_table[(index / 16) * 33 + (index & 0xF) * 2 + 1];
            out++;
            in += 1;
        }
        else
        {
            *out = g_addhero_single_byte_char_table[0];
            out++;
            *out = g_addhero_single_byte_char_table[1];
            out++;
            in += 1;
        }
    }
done:
    *out = 0;
}
