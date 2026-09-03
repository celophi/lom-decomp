#include "common.h"

/* ------------------------------------------------------------------ */
/* Shared types                                                       */
/* ------------------------------------------------------------------ */

typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} RECT;

typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} AddheroRect;

typedef struct
{
    s16 x;
    s16 y;
} Vec2s;

typedef struct
{
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 w, h;
} TILE;

typedef struct
{
    u32 tag;
    u8 r0;
    u8 g0;
    u8 b0;
    u8 code;
    s16 x0;
    s16 y0;
    s16 w;
    s16 h;
} AddheroTile;

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

/** @brief Cursor view (0xC) with a bitfield attr and a draw handler pointer. */
typedef struct
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
    void (*draw)();
} AddheroCursor;

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
    /* 0x00 */ char name[20];
    /* 0x14 */ s32 attr;
    /* 0x18 */ s32 size;
    /* 0x1C */ void *next;
    /* 0x20 */ s32 head;
    /* 0x24 */ char system[4];
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

typedef struct AddheroFallbackText
{
    u8 pad[0x24];
    u8 text[0x20];
} AddheroFallbackText;

/* addhero_update_and_draw_elements element-draw pipeline types */
typedef struct
{
    /* 0x00 */ s32 tag;
    /* 0x04 */ s32 word4;
    /* 0x08 */ s16 x0;
    /* 0x0A */ s16 y0;
    /* 0x0C */ s16 unkC;
    /* 0x0E */ u16 unkE;
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

/** @brief POLY_G4 words used to draw the timer bar. */
typedef struct
{
    /* 0x00 */ s32 tag;
    /* 0x04 */ s32 color0;
    /* 0x08 */ s16 x0;
    /* 0x0A */ s16 y0;
    /* 0x0C */ s32 color1;
    /* 0x10 */ s16 x1;
    /* 0x12 */ s16 y1;
    /* 0x14 */ s32 color2;
    /* 0x18 */ s16 x2;
    /* 0x1A */ s16 y2;
    /* 0x1C */ s32 color3;
    /* 0x20 */ s16 x3;
    /* 0x22 */ s16 y3;
} AddheroPolyG4Packet;

/** @brief 0x28-byte textured-quad primitive built for a save-slot glyph. */
typedef struct
{
    /* 0x00 */ s32 tag;
    /* 0x04 */ s32 color0;
    /* 0x08 */ s16 x0;
    /* 0x0A */ s16 y0;
    /* 0x0C */ u8 u0;
    /* 0x0D */ u8 v0;
    /* 0x0E */ s16 clut;
    /* 0x10 */ s16 x1;
    /* 0x12 */ s16 y1;
    /* 0x14 */ u8 u1;
    /* 0x15 */ u8 v1;
    /* 0x16 */ s16 tpage;
    /* 0x18 */ s16 x2;
    /* 0x1A */ s16 y2;
    /* 0x1C */ u8 u2;
    /* 0x1D */ u8 v2;
    /* 0x1E */ u8 pad1E[2];
    /* 0x20 */ s16 x3;
    /* 0x22 */ s16 y3;
    /* 0x24 */ u8 u3;
    /* 0x25 */ u8 v3;
    /* 0x26 */ u8 pad26[2];
} AddheroPolyFT4Packet;

/* ------------------------------------------------------------------ */
/* Globals                                                            */
/* ------------------------------------------------------------------ */

extern AddheroElementPoolHead g_addhero_element_pool;
extern AddheroElement g_addhero_element1;
extern AddheroDirEntry g_addhero_entries[][20];
extern AddheroRecord g_addhero_entry_metadata;
extern u8 *D_8012271C;
extern u8 *g_addhero_load_step;

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

/* ------------------------------------------------------------------ */
/* Forward declarations                                               */
/* ------------------------------------------------------------------ */

/* In-file functions */
void addhero_init(s32 arg0, s32 arg1);
s32 addhero_state_step(s32 arg0);
void addhero_build_ui_elements(void);
void addhero_update_state();
s32 addhero_update_load_sequence(void);
s32 addhero_handle_input(void);
void addhero_reset_state(void);
void addhero_close_all_elements(void);
void addhero_scroll_to_selection(void);
void addhero_update_elements(void);
s32 addhero_draw_entry_list(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 addhero_draw_mode_glyph(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 addhero_draw_card_slot0_label(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 addhero_draw_card_slot1_label(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 addhero_draw_selected_entry_details(s32 *ot, s32 prim, s32 arg2, s32 arg3);
u8 *addhero_skip_hex_digits(void *arg0);
void addhero_terminate_multibyte_text(void *arg0);
void addhero_clear_elements();
AddheroElement *addhero_alloc_element(void);
void addhero_update_and_draw_elements();
void addhero_deactivate_primary_element(void);
void addhero_text_append(u8 *arg0, u8 *arg1);
s32 addhero_text_byte_length(u8 *arg0);
void addhero_text_copy(u8 *arg0, u8 *arg1);
s32 addhero_draw_load_prompt(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 addhero_draw_load_progress(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 addhero_draw_progress_bar(s32 arg0, s32 *arg1);
void addhero_open_status_dialog(s32 arg0);
void addhero_open_exit_dialog(s32 arg0);
s32 addhero_draw_status_dialog(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 addhero_draw_exit_dialog(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 addhero_draw_transfer_status(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 addhero_draw_icon_highlight(s32 result, s32 *ot, s32 x, s32 y, s32 adjust, s32 slot, s32 i, s32 j);
void addhero_enable_choice_toggle(void);
s32 addhero_draw_choice_prompt(s32 prim, s32 *ot, s32 x, s32 y);
s32 addhero_validate_save_blob(u8 *base);
s32 addhero_compute_save_checksum(u8 *data);
s8 *addhero_format_decimal(s8 *out, s32 value);
void addhero_format_hex(s8 *out, s32 value, s32 max_chars);
void addhero_hex_nibble_to_ascii(s8 *out, s32 value);
u32 addhero_parse_hex(u8 *s, s32 len);
s32 addhero_parse_hex_suffix_byte(u8 *text, s32 unused1, s32 unused2);

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

/* ------------------------------------------------------------------ */
/* Functions                                                          */
/* ------------------------------------------------------------------ */

/** @see decomp.me (100%) */
void addhero_init(s32 arg0, s32 arg1)
{
    RECT rect;

    g_addhero_mode = arg1;
    g_addhero_entry_state = 0xFF;
    g_addhero_card_slot = 0;
    addhero_reset_entry_ranks();
    g_addhero_result = 3;
    addhero_init_stream_handles();
    g_addhero_icon_phase = 0;
    func_80067F8C();
    rect.x = 0x140;
    rect.y = 0;
    rect.w = 0x40;
    rect.h = 0x100;
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
    D_80160930 = arg0;
}

/** @see decomp.me (100%) */
s32 addhero_state_step(s32 arg0)
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
    addhero_update_state(arg0);
    addhero_evict_unused_glyphs();
    func_80063194();
    g_addhero_frame_parity ^= 1;
    return 0;
}

void addhero_build_ui_elements(void)
{
    AddheroElement *p;
    g_addhero_scroll_frames = 0;
    g_addhero_scroll_target_y = 0;
    g_addhero_scroll_y = 0;
    g_addhero_selected_row = 0;
    g_addhero_selection_status = 0;
    D_80160924 = (s32)D_8012271C + 0xCE0;
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

/** @see decomp.me (100%) */
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
            if ((g_addhero_entry_metadata.hero_id != ((AddheroRecord *)D_8012271C)->hero_id) &&
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

/** @see decomp.me (100%) */
void addhero_reset_state(void)
{
    D_801609B4 = 0;
    g_addhero_load_step = 0;
    g_addhero_entry_state = 0xFF;
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

void addhero_close_all_elements(void)
{
    s32 temp_v1;
    s32 var_a1;
    s32 *var_a0;
    s32 temp;

    func_80067F28();
    var_a0 = (s32 *)&g_addhero_element_pool.first;
    var_a1 = 0;
    do
    {
        temp_v1 = *var_a0;
        if (temp_v1 & 7)
        {
            temp = (temp_v1 & ~7) | 3;
            *var_a0 = (temp & ~0x78) | 0x40;
        }
        var_a1 += 1;
        var_a0 += 3;
    } while (var_a1 < 8);
}

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

void addhero_update_elements(void)
{
    addhero_update_and_draw_elements();
}

s32 addhero_draw_entry_list(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    s32 state = g_addhero_entry_state;

    switch (state)
    {
    case 0xF8:
        do { prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FD8, 0x34), 4, -arg2 + 0x84, -arg3, 2); } while (0);
        break;
    case 0xF9:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FD8, 0x34), 4, -arg2 + 0x84, -arg3, 2);
        break;
    case 0xFA:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FA6, 2), 4, -arg2 + 0x84, -arg3, 2);
        break;
    case 0xFD:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FA8, 4), 4, -arg2 + 0x84, -arg3, 2);
        break;
    case 0xFB:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FB4, 0x10), 4, -arg2 + 0x84, -arg3, 2);
        break;
    case 0xFC:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FB6, 0x12), 4, -arg2 + 0x84, -arg3, 2);
        break;
    default:
        {
            s32 row_y;
            s32 i;

        if (g_addhero_entry_scan_active != 0)
        {
            s32 x;
            u8 *base;
        case 0xFF:
            x = -arg2 + 0x84;
            base = (u8 *)&D_80146FA4;
            prim = func_800A88A0(prim, ot, base + D_80146FA4, 4, x, -arg3, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x1E), 4, x, 0xE - arg3, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - arg3, 2);
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
            base_x = -arg2;
            do
            {
                row = ((i * 14) - arg3) - g_addhero_scroll_y;
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
                            prim = func_800A88A0(prim, ot, (void *)((s32)D_80147054 + (s32)base), 4, 0xF2 - arg2, row_y, 1);
                        }
                    }
                    if (func_8001714C(D_800ECF7C, (char *)((s32)&g_addhero_entries[g_addhero_card_slot][i]), 0xC) == 0)
                    {
                        prim = func_800A88A0(prim, ot, (void *)((s32)D_80146FAA + (s32)base), 4, 1 - arg2, row_y, 0);
                    }
                    else if (func_8001714C(D_800ECF8C, (char *)((s32)&g_addhero_entries[g_addhero_card_slot][i]), 0xC) == 0)
                    {
                        prim = func_800A88A0(prim, ot, (void *)((s32)D_80146FDE + (s32)base), 4, 1 - arg2, row_y, 0);
                    }
                    else if (func_8001714C(D_800ECFC4, (char *)((s32)&g_addhero_entries[g_addhero_card_slot][i]), 8) == 0)
                    {
                        prim = func_800A88A0(prim, ot, (void *)((s32)D_80146FB8 + (s32)base), 4, 1 - arg2, row_y, 0);
                    }
                    else
                    {
                        prim = func_800A88A0(prim, ot, (void *)((s32)D_80146FAC + (s32)base), 4, 1 - arg2, row_y, 0);
                    }
                }
                i++;
            } while (i < g_addhero_entry_state);
        }
            row_y = ((g_addhero_selected_row * 14) - arg3) - g_addhero_scroll_y;

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
                tile->tag = (tile->tag & 0xFF000000) | (*ot & 0xFFFFFF);
                *ot = (*ot & 0xFF000000) | ((s32)tile & 0xFFFFFF);
                prim += sizeof(TILE);
            }
        }
        break;
    }
    return prim;
}

s32 addhero_draw_mode_glyph(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    AddheroRect pos;

    if (g_addhero_mode == 1)
    {
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FEA, 0x46), 4, -arg2 + 0x78, -arg3, 2);
    }
    else
    {
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FE8, 0x44), 4, -arg2 + 0x78, -arg3, 2);
    }
    return prim;
}

s32 addhero_draw_card_slot0_label(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    AddheroRect pos;
    AddheroTile *tile;

    if (g_addhero_card_slot != 0)
    {
        tile = (AddheroTile *)prim;
        *(u32 *)&tile->r0 = 0x101010;
        *((u8 *)tile + 3) = 3;
        tile->code = 0x62;
        tile->x0 = 0;
        tile->y0 = 0;
        tile->w = 0x80;
        tile->h = 0x10;
        tile->tag = (tile->tag & 0xFF000000) | (*ot & 0xFFFFFF);
        *ot = (*ot & 0xFF000000) | ((s32)tile & 0xFFFFFF);
        prim += 0x10;
    }
    return func_800A88A0(prim, ot, GLYPH_SYM(D_80146FB0, 0xC), 4, -arg2 + 0x40, -arg3, 2);
}

s32 addhero_draw_card_slot1_label(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    AddheroRect pos;
    AddheroTile *tile;

    if (g_addhero_card_slot == 0)
    {
        tile = (AddheroTile *)prim;
        *(u32 *)&tile->r0 = 0x101010;
        *((u8 *)tile + 3) = 3;
        tile->code = 0x62;
        tile->x0 = 0;
        tile->y0 = 0;
        tile->w = 0x80;
        tile->h = 0x10;
        tile->tag = (tile->tag & 0xFF000000) | (*ot & 0xFFFFFF);
        *ot = (*ot & 0xFF000000) | ((s32)tile & 0xFFFFFF);
        prim += 0x10;
    }
    return func_800A88A0(prim, ot, GLYPH_SYM(D_80146FB2, 0xE), 4, -arg2 + 0x40, -arg3, 2);
}

s32 addhero_draw_selected_entry_details(s32 *ot, s32 prim, s32 arg2, s32 arg3)
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
            s32 x = -arg2;
            u8 *base;

            result = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FCC, 0x28), 4, x, -arg3, 0);
            base = (u8 *)&D_80146FCC - 0x28;
            return func_800A88A0(result, ot, GLYPH_OFF(base, 0x2A), 4, x, 0x10 - arg3, 0);
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
                            result = addhero_draw_icon_highlight(result, ot, total - arg2, -arg3, adjust, slot[j], i, j);
                            i += 1;
                            total += adjust;
                        }
                    }

                    {
                        u8 *base90 = (u8 *)&g_addhero_entry_metadata;
                        s32 x = -arg2;
                        s32 y = -arg3;

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

                        if (*(u16 *)(base90 + 0xD4) == *(u16 *)((u8 *)D_8012271C + 0xD4))
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
                    result = func_800A88A0(result, ot, GLYPH_SYM(D_80146FF8, 0x54), 4, -arg2, -arg3, 0);
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
                    result = addhero_draw_cached_text(result, ot, name, -arg2, -arg3, 4, 0);

                    for (j = 0; j < 0x20; j++)
                    {
                        name[j] = ((AddheroFallbackText *)&D_80165208)->text[j];
                    }
                    name[j] = 0;
                    result = addhero_draw_cached_text(result, ot, name, -arg2, -arg3 + 0x10, 4, 0);
                }
            }
        }
    }
    return result;
}

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

/** @see decomp.me (100%) */
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

void addhero_update_and_draw_elements(AddheroDrawState *arg0)
{
    AddheroGpuPacket *var_s0;
    AddheroDrawState *var_s5;
    volatile u32 *var_s3;
    s32 temp_s1;
    s32 temp_s2;
    s32 var_s6;
    s32 sp20[24];
    u32 temp_a0_2;
    s32 temp_v1_2;
    u32 temp_a1;
    u32 temp_a2;
    s32 temp_a0_3;
    s32 var_v1;
    s32 temp_a3_2;
    s32 var_v0;
    s32 temp_a3_3;
    u32 temp_v0_3;
    u32 temp_a0_4;
    s32 temp_a0_5;
    s32 var_v1_2;
    s32 temp_a3_5;
    s32 var_v0_2;
    s32 temp_a3_6;
    u32 temp_v0_5;
    u32 temp_v1_3;
    s32 count;

    var_s0 = arg0->prim_cursor;
    var_s5 = arg0;

    count = g_addhero_entry_state;
    if ((count < 0x10) &&
        ((((AddheroPacketBlock *)&g_addhero_element_pool.first)->second.attr.word & 7) == 2) &&
        (((((AddheroPacketBlock *)&g_addhero_element_pool.first)->second.unk4 >> 9) & 1) != 0))
    {
        count *= 0xE;
        if ((g_addhero_scroll_y + 0x58) < count)
        {
            var_s0 = (AddheroGpuPacket *)func_800AE76C(var_s0, var_s5, 0x114, 0x82, 0);
        }
        if (g_addhero_scroll_y != 0)
        {
            var_s0 = (AddheroGpuPacket *)func_800AE76C(var_s0, var_s5, 0x114, 0x3A, 1);
        }
    }

    if (arg0->frame_flag != 0)
    {
        func_8001C56C(sp20, 0, 0xF0, 0x140, 0xE0);
    }
    else
    {
        func_8001C56C(sp20, 0, 8, 0x140, 0xE0);
    }

    var_s3 = (volatile u32 *)&g_addhero_element_pool.first;
    var_s6 = 0;

    for (; var_s6 < 8; var_s6++, var_s3 += 3)
    {
        if (*var_s3 & 7)
        {
            func_8001A5D4((s32)var_s0, sp20);

            var_s0->tag = (var_s0->tag & 0xFF000000) | (var_s5->tag & 0x00FFFFFF);
            var_s5->tag = (s32)((var_s5->tag & 0xFF000000) | ((s32)var_s0 & 0x00FFFFFF));

            temp_a0_2 = *var_s3;
            temp_v1_2 = temp_a0_2 & 7;

            var_s0 = (AddheroGpuPacket *)((u8 *)var_s0 + 0x40);

            switch (temp_v1_2)
            {
            case 1:
                temp_v0_3 = *var_s3;
                temp_a1 = *(u32 *)((u8 *)var_s3 + 4);
                temp_a0_4 = temp_v0_3 >> 24;
                temp_a2 = ((temp_a1 & 1) << 8) | temp_a0_4;
                temp_a0_3 = (temp_v0_3 >> 3) & 0xF;
                var_v1 = temp_a2 * temp_a0_3;
                g_pad_input = 0;
                if (var_v1 < 0)
                {
                    var_v1 += 7;
                }
                temp_a3_2 = (temp_a1 >> 1) & 0xFF;
                var_v0 = temp_a3_2 * temp_a0_3;
                temp_s1 = var_v1 >> 3;
                if (var_v0 < 0)
                {
                    var_v0 += 7;
                }
                temp_s2 = var_v0 >> 3;
                temp_a3_3 = (s32)(temp_a3_2 - temp_s2);

                var_s0 = (*(AddheroElemDrawFunc *)((u8 *)var_s3 + 8))(var_s5, var_s0, (s32)(temp_a2 - temp_s1) / 2, temp_a3_3 / 2);
                {
                    u32 post_word;
                    u32 field;
                    u32 high;
                    post_word = *var_s3;
                    field = (post_word >> 7) & 0x1FF;
                    high = post_word >> 24;
                    var_s0 = (AddheroGpuPacket *)func_800AD850(var_s0, var_s5,
                                           field + (s32)((((*(u32 *)((u8 *)var_s3 + 4) & 1) << 8) | high) - temp_s1) / 2,
                                           (*((u8 *)var_s3 + 2)) + ((s32)((*(u32 *)((u8 *)var_s3 + 4) >> 1) & 0xFF) - temp_s2) / 2,
                                           temp_s1, temp_s2, arg0->frame_flag, var_s6 == 0);
                }
                {
                    u32 old_word;
                    u32 new_word;
                    old_word = *var_s3;
                    new_word = (old_word & ~0x78) | (((((old_word >> 3) & 0xF) + 1) & 0xF) * 8);
                    *(u32 *)var_s3 = new_word;
                    if (((new_word >> 3) & 0xF) == 8)
                    {
                        func_800AA02C();
                        *(u32 *)var_s3 = (*var_s3 & ~7) | 2;
                    }
                }
                break;

            case 2:
                var_s0 = (*(AddheroElemDrawFunc *)((u8 *)var_s3 + 8))(var_s5, var_s0, 0, 0);
                {
                    u32 case_word;
                    u32 high;
                    case_word = *var_s3;
                    high = case_word >> 24;
                    var_s0 = (AddheroGpuPacket *)func_800AD850(var_s0, var_s5,
                                           (case_word >> 7) & 0x1FF, *((u8 *)var_s3 + 2),
                                           ((*(u32 *)((u8 *)var_s3 + 4) & 1) << 8) | high,
                                           (*(u32 *)((u8 *)var_s3 + 4) >> 1) & 0xFF, arg0->frame_flag, var_s6 == 0);
                }
                temp_v1_3 = *var_s3;
                if (((temp_v1_3 >> 3) & 0xF) != 0)
                {
                    *(u32 *)var_s3 = (temp_v1_3 & ~0x78) | (((((temp_v1_3 >> 3) & 0xF) - 1) & 0xF) * 8);
                }
                break;

            case 3:
                temp_a0_5 = *var_s3;
                temp_a1 = *(u32 *)((u8 *)var_s3 + 4);
                var_v1_2 = (u32)temp_a0_5 >> 24;
                temp_a2 = ((temp_a1 & 1) << 8) | var_v1_2;
                temp_a0_5 = (u32)temp_a0_5 >> 3;
                temp_a0_5 &= 0xF;
                var_v1_2 = temp_a2 * temp_a0_5;
                g_pad_input = 0;
                if (var_v1_2 < 0)
                {
                    var_v1_2 += 7;
                }
                temp_a3_5 = (temp_a1 >> 1) & 0xFF;
                var_v0_2 = temp_a3_5 * temp_a0_5;
                temp_s1 = var_v1_2 >> 3;
                if (var_v0_2 < 0)
                {
                    var_v0_2 += 7;
                }
                temp_s2 = var_v0_2 >> 3;
                temp_a3_6 = (s32)(temp_a3_5 - temp_s2);

                var_s0 = (*(AddheroElemDrawFunc *)((u8 *)var_s3 + 8))(var_s5, var_s0, (s32)(temp_a2 - temp_s1) / 2, temp_a3_6 / 2);
                {
                    u32 post_word;
                    u32 field;
                    u32 high;
                    post_word = *var_s3;
                    field = (post_word >> 7) & 0x1FF;
                    high = post_word >> 24;
                    var_s0 = (AddheroGpuPacket *)func_800AD850(var_s0, var_s5,
                                           field + (s32)((((*(u32 *)((u8 *)var_s3 + 4) & 1) << 8) | high) - temp_s1) / 2,
                                           (*((u8 *)var_s3 + 2)) + ((s32)((*(u32 *)((u8 *)var_s3 + 4) >> 1) & 0xFF) - temp_s2) / 2,
                                           temp_s1, temp_s2, arg0->frame_flag, var_s6 == 0);
                }
                {
                    u32 old_word;
                    old_word = *var_s3;
                    var_v1_2 = old_word & ~0x78;
                    old_word >>= 3;
                    old_word &= 0xF;
                    old_word--;
                    old_word &= 0xF;
                    old_word <<= 3;
                    var_v1_2 |= old_word;
                    *(u32 *)var_s3 = var_v1_2;
                    if (!(((u32)var_v1_2 >> 3) & 0xF))
                    {
                        *(u32 *)var_s3 = ((((u32)var_v1_2 & ~0x78) | 0x18) & ~7) | 4;
                    }
                }
                break;

            case 4:
                temp_v0_5 = *(u32 *)var_s3;
                g_pad_input = 0;
                temp_v1_3 = (temp_v0_5 & ~0x78) | (((((temp_v0_5 >> 3) & 0xF) - 1) & 0xF) * 8);
                *(u32 *)var_s3 = temp_v1_3;
                if (!((temp_v1_3 >> 3) & 0xF))
                {
                    *(u32 *)var_s3 = temp_v1_3 & ~7;
                }
                break;
            }
        }
    }

    arg0->prim_cursor = var_s0;
}

void addhero_deactivate_primary_element(void)
{
    g_addhero_element_pool.first.attr.word &= ~7;
}

void addhero_text_append(u8 *arg0, u8 *arg1)
{
    s32 temp_s0;
    s32 temp_v0;
    s32 i;

    temp_s0 = addhero_text_byte_length(arg0);
    temp_v0 = addhero_text_byte_length(arg1);
    for (i = 0; i < temp_v0; i++)
    {
        arg0[temp_s0 + i] = arg1[i];
    }
    arg0[temp_s0 + i] = 0;
}

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

s32 addhero_draw_load_prompt(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;
    s32 result;
    s32 x;
    s32 status;
    AddheroElement *p;

    x = -arg2 + 0x90;
    result = addhero_draw_choice_prompt(
        func_800A88A0(prim, ot,
                      (u8 *)&D_80146FD4 + D_80146FD4 - 0x30,
                      4, x, -arg3, 2),
        ot, x, 0xE - arg3);

    if ((u32)(addhero_poll_and_rewind_primary_handles() - 1) < 2U)
    {
        g_addhero_element_pool.first.attr.f.state = 0;
        func_800AA02C();
        func_800A3938(0x78, 0x80);
        g_addhero_entry_state = 0xFF;
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
 * @param arg2 Horizontal offset used to place the prompt (screen X = 0x90 - arg2).
 * @param arg3 Vertical offset used to place the prompt rows.
 * @return The updated primitive pointer / index after linking the prompt.
 */
s32 addhero_draw_load_progress(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;
    u8 *base;
    u8 *resource;
    AddheroCursor *p;
    AddheroCursor *cursor;
    s32 result;
    s32 x;
    s32 i;
    u32 saved;

    x = -arg2 + 0x90;
    result = func_800A88A0(prim, ot, (void *)((s32)&D_80146FD6 - 0x32 + D_80146FD6), 4, x, -arg3, 2);
    base = (u8 *)&D_80146FD6 - 0x32;
    result = func_800A88A0(result, ot, base + *(u16 *)(base + 0x1E), 4, x, 0xE - arg3, 2);
    result = func_800A88A0(result, ot, base + *(u16 *)(base + 0xB2), 4, x, 0x1C - arg3, 2);
    result = addhero_draw_progress_bar(result, ot);

    if (g_addhero_progress_active == 0)
    {
        resource = g_addhero_save_blob;
        p = (AddheroCursor *)&g_addhero_element_pool.first;
        p->attr.f.state = 0;
        if (addhero_validate_save_blob(resource) == 0)
        {
            addhero_open_status_dialog(4);
            return result;
        }

        func_800A3938(0x7B, 0x80);
        saved = D_8012271C[0x858] >> 7;
        func_80016E7C(resource + 0x770, D_8012271C + 0x840, 0x250);
        *(u32 *)(D_8012271C + 0x858) = (*(u32 *)(D_8012271C + 0x858) & ~0x80) | (saved << 7);
        *(u16 *)(D_8012271C + 0xD8) = *(u16 *)(resource + 0x254);
        *(u16 *)(D_8012271C + 0xDA) = *(u16 *)(resource + 0x256);
        *(u16 *)(D_8012271C + 0xDE) = 1;
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

s32 addhero_draw_progress_bar(s32 arg0, s32 *arg1)
{
    AddheroPolyG4Packet *g;
    s32 elapsed;
    s32 extent;
    s32 color;

    g = (AddheroPolyG4Packet *)arg0;
    if (g_addhero_progress_bar_active != 0)
    {
        elapsed = func_8002054C(-1) - g_addhero_progress_start_tick;
        if (elapsed >= 0x101)
        {
            elapsed = 0x100;
        }
        color = 0xFFFF00;
        extent = elapsed * 0x120;
        g->color0 = 0xFF;
        g->color1 = 0xFFFF;
        g->color3 = 0xFF0000;
        ((u8 *)g)[3] = 8;
        g->color2 = color;
        ((u8 *)g)[7] = 0x38;
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
        g->tag = (g->tag & 0xFF000000) | (*arg1 & 0xFFFFFF);
        *arg1 = (*arg1 & 0xFF000000) | (arg0 & 0xFFFFFF);
        arg0 += 0x24;
    }
    return arg0;
}

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
    g_addhero_entry_state = 0xFF;
    addhero_reset_entry_ranks();
    g_addhero_load_step = 0;
    g_addhero_dialog_state = arg0;
}

/**
 * @brief Initialize the secondary choice element and reset its transition state.
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

s32 addhero_draw_status_dialog(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;

    switch (g_addhero_dialog_state)
    {
    case 0:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FE0, 0x3C), 4, -arg2 + 0x80, -arg3, 2);
        break;
    case 2:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FE4, 0x40), 4, -arg2 + 0x80, -arg3, 2);
        break;
    case 3:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FE6, 0x42), 4, -arg2 + 0x80, -arg3, 2);
        break;
    case 1:
    case 4:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FE2, 0x3E), 4, -arg2 + 0x80, -arg3, 2);
        break;
    }
    if (g_pad_input & 0x220)
    {
        g_addhero_element_pool.first.attr.f.state = 0;
        func_800AA02C();
    }
    return prim;
}

/** @see decomp.me (100%) */
s32 addhero_draw_exit_dialog(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;
    AddheroPacket *p;
    s32 i;

    switch (g_addhero_dialog_state)
    {
    case 0:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FE0, 0x3C), 4, -arg2 + 0x80, -arg3, 2);
        break;
    case 2:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FE4, 0x40), 4, -arg2 + 0x80, -arg3, 2);
        break;
    case 3:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FE6, 0x42), 4, -arg2 + 0x80, -arg3, 2);
        break;
    case 1:
    case 4:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FE2, 0x3E), 4, -arg2 + 0x80, -arg3, 2);
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

s32 addhero_draw_transfer_status(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;
    switch (g_addhero_entry_state)
    {
    case 0xF8:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FD8, 0x34), 4, -arg2 + 0x90, -arg3, 2);
        break;
    case 0xF9:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FD8, 0x34), 4, -arg2 + 0x90, -arg3, 2);
        break;
    case 0xFF:
        {
            s32 x; u8 *base;
            x = -arg2 + 0x90;
            base = (u8 *)&D_80146FA4;
            prim = func_800A88A0(prim, ot, base + D_80146FA4, 4, x, -arg3, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x1E), 4, x, 0xE - arg3, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - arg3, 2);
        }
        break;
    case 0xFA:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FD8, 0x34), 4, -arg2 + 0x90, -arg3, 2);
        break;
    case 0xFD:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FA8, 4), 4, -arg2 + 0x90, -arg3, 2);
        break;
    case 0xFB:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FB4, 0x10), 4, -arg2 + 0x90, -arg3, 2);
        break;
    case 0xFC:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FB6, 0x12), 4, -arg2 + 0x90, -arg3, 2);
        break;
    case 0xF7:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014700C, 0x68), 4, -arg2 + 0x90, -arg3, 2);
        break;
    case 0xF6:
        {
            s32 x; u8 *base; AddheroPolyG4Packet *g; s32 next, elapsed, extent, color, finalmode;
            x = -arg2 + 0x90;
            prim = func_800A88A0(prim, ot, (void *)((s32)&D_80146FD6 - 0x32 + D_80146FD6), 4, x, -arg3, 2);
            base = (u8 *)&D_80146FD6 - 0x32;
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x1E), 4, x, 0xE - arg3, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - arg3, 2);
            next = prim; g = (AddheroPolyG4Packet *)prim;
            if (g_addhero_progress_bar_active != 0)
            {
                elapsed = func_8002054C(-1) - g_addhero_progress_start_tick;
                if (elapsed >= 0x101) elapsed = 0x100;
                color = 0xFFFF00; extent = elapsed * 0x120;
                g->color0=0xFF; g->color1=0xFFFF; g->color3=0xFF0000; ((u8 *)g)[3]=8;
                g->color2=color; ((u8 *)g)[7]=0x38; g->x2=0; g->x0=0;
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
                    g_addhero_write_in_progress=0; g_addhero_selection_status=0; g_addhero_io_busy=0; g_addhero_progress_active=0; g_addhero_entry_state=0xFF;
                    addhero_reset_entry_ranks(); finalmode=4; g_addhero_load_step=0; g_addhero_dialog_state=finalmode; return prim;
                }
                func_800A3938(0x7B,0x80); g_addhero_entry_state=0xF4; addhero_enable_choice_toggle(); func_800AA02C();
            }
        }
        break;
    case 0xF3:
        {
            s32 x; AddheroPacket *packet; s32 i;
            x = -arg2 + 0x90;
            prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80147012,0x6E), 4, x, -arg3, 2);
            prim = addhero_draw_choice_prompt(prim, ot, x, 0xE -arg3);
            if (g_pad_input & 0x40)
            {
                func_800A3938(0x78, 0x80);
                addhero_enable_choice_toggle();
                g_addhero_entry_state = 0xF4;
                func_800AA02C();
            }
            else if (g_pad_input & 0x220)
            {
                if (g_addhero_choice_toggle != 0)
                {
                    func_800A3938(0x78, 0x80);
                    addhero_enable_choice_toggle();
                    g_addhero_entry_state = 0xF4;
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
    case 0xF4:
        {
            s32 x; u8 *base; s32 temp;
            x=-arg2+0x90;
            prim=func_800A88A0(prim,ot,(void *)((s32)&D_8014700E-0x6A+D_8014700E),4,x,-arg3,2);
            base=(u8 *)&D_8014700E-0x6A;
            prim=func_800A88A0(prim,ot,GLYPH_OFF(base,0x6C),4,x,0xE -arg3,2);
            prim=addhero_draw_choice_prompt(prim,ot,x,0x1C-arg3);
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
                    func_80016E7C(D_8012271C + 0x840, base + 0x770, 0x250);
                    *(s32 *)(base + 0x788) |= 0x80;
                    temp = addhero_compute_save_checksum(base);
                    *(s32 *)(base + 0x33E4) = 0x414E41;
                    *(s32 *)(base + 0x33E0) = temp;
                    g_addhero_write_in_progress = 1;
                    g_addhero_load_step = &D_801605A1;
                    g_addhero_entry_state = 0xF5;
                }
            }
        }
        break;
    case 0xF5:
        {
            s32 x; u8 *base; AddheroPolyG4Packet *g; s32 next,elapsed,extent,color; AddheroPacket *packet; s32 i;
            x=-arg2+0x90;
            prim=func_800A88A0(prim,ot,(void *)((s32)&D_80146FC0-0x1C+D_80146FC0),4,x,-arg3,2);
            base=(u8 *)&D_80146FC0-0x1C;
            prim=func_800A88A0(prim,ot,GLYPH_OFF(base,0x1E),4,x,0xE -arg3,2);
            prim=func_800A88A0(prim,ot,GLYPH_OFF(base,0xB2),4,x,0x1C-arg3,2);
            next=prim; g=(AddheroPolyG4Packet *)prim;
            if(g_addhero_progress_bar_active!=0){
                elapsed=func_8002054C(-1)-g_addhero_progress_start_tick; if(elapsed>=0x101)elapsed=0x100; color=0xFFFF00; extent=elapsed*0x120;
                g->color0=0xFF;g->color1=0xFFFF;g->color3=0xFF0000;((u8*)g)[3]=8;g->color2=color;((u8*)g)[7]=0x38;g->x2=0;g->x0=0;
                if(extent<0)extent+=0xFF;g->x3=extent>>8;g->x1=extent>>8;g->y1=0;g->y0=0;g->y3=0x2C;g->y2=0x2C;
                g->tag=(g->tag&0xFF000000)|(*ot&0xFFFFFF);*ot=(*ot&0xFF000000)|(prim&0xFFFFFF);next=prim+0x24;
            }
            prim=next;
            if(g_addhero_write_in_progress==0){
                D_8012271C[0x840]=0; func_800A3938(0x7A,0x80); D_8012298C=0x20;
                packet=(AddheroPacket *)&g_addhero_element_pool.first;
                for(i=0;i<ADDHERO_ELEMENT_COUNT;i++,packet++){ packet->size_flags &= ~0x200; packet->state_word &= ~7; }
                func_80067F5C(8); g_addhero_result=2;
            }
        }
        break;
    default:
        {
            s32 x,posv,diff; u8 *base;
            x=-arg2+0x90; base=(u8 *)&D_80146FA4;
            prim=func_800A88A0(prim,ot,base+D_80146FA4,4,x,-arg3,2);
            prim=func_800A88A0(prim,ot,GLYPH_OFF(base,0x1E),4,x,0xE -arg3,2);
            prim=func_800A88A0(prim,ot,GLYPH_OFF(base,0xB2),4,x,0x1C-arg3,2);
            if(g_addhero_entry_scan_active==0){
                if(g_addhero_io_busy!=0)return prim;
                if((u32)(*g_addhero_load_step-6)<2U)return prim;
                if((func_8001714C(D_800ECF7C,&g_addhero_entries[g_addhero_card_slot][g_addhero_selected_row],0xC)!=0) ||
                   (g_addhero_entry_identity != *(s32 *)(D_8012271C+0xD8))) {
                    g_addhero_selected_row++;
                    if(g_addhero_selected_row>=g_addhero_entry_state){ if(g_addhero_entry_state!=0)g_addhero_entry_state=0xF7; else g_addhero_entry_state=0xF8; }
                    else { addhero_commit_selected_entry(); posv=g_addhero_selected_row*0xE; diff=posv-g_addhero_scroll_y;
                        if(diff>=0x4B){g_addhero_scroll_target_y=posv-0x46;g_addhero_scroll_frames=4;} if(diff<0){g_addhero_scroll_target_y=posv;g_addhero_scroll_frames=4;}
                    }
                } else { g_addhero_progress_start_tick=func_8002054C(-1);g_addhero_progress_active=1;g_addhero_load_step=&D_80160598;g_addhero_entry_state=0xF6; }
            }
        }
        break;
    case 0xFE:
        break;
    }
    if(g_addhero_io_busy!=0)return prim;
    if(g_addhero_entry_state==0xF6)return prim; if(g_addhero_entry_state==0xF5)return prim; if(g_addhero_entry_state==0xF4)return prim; if(g_addhero_entry_state==0xF3)return prim;
    if(g_pad_input&0x40){
        s32 *p; s32 i,word; D_80122718=3;func_800A3938(0x78,0x80);func_80067F28();p=(s32 *)&g_addhero_element_pool.first;i=0;
        do{word=*p;if(word&7)*p=(((word&~7)|3)&~0x78)|0x40;i++;p+=3;}while(i<8);return prim;
    }
    if((g_pad_input&0xA100)&&(g_addhero_entry_state!=0xFF)){
        func_800A3938(0x7D,0x80);D_801609B4=0;g_addhero_load_step=0;g_addhero_scroll_frames=0;g_addhero_scroll_target_y=0;g_addhero_scroll_y=0;g_addhero_selected_row=0;g_addhero_entry_state=0xFF;g_addhero_selection_status=0;
        g_addhero_card_slot^=1;addhero_reset_entry_ranks();func_800AA02C();g_addhero_progress_bar_active=0;g_pad_input=0;g_addhero_load_step=&D_80160574;
    }
    return prim;
}

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
    ((AddheroPolyFT4Packet *)result)->color0 = 0x808080;
    ((u8 *)result)[3] = 9;
    ((u8 *)result)[7] = 0x2C;
    ((AddheroPolyFT4Packet *)result)->x2 = x;
    ((AddheroPolyFT4Packet *)result)->x0 = x;
    ((AddheroPolyFT4Packet *)result)->y1 = y;
    ((AddheroPolyFT4Packet *)result)->y0 = y;
    ((AddheroPolyFT4Packet *)result)->x3 = x + adjust;
    shade = temp * 0x10;
    ((AddheroPolyFT4Packet *)result)->u2 = shade;
    ((AddheroPolyFT4Packet *)result)->u0 = shade;
    shade += 0x2F;
    ((AddheroPolyFT4Packet *)result)->u3 = shade;
    ((AddheroPolyFT4Packet *)result)->u1 = shade;
    ((AddheroPolyFT4Packet *)result)->v1 = 0xD0;
    ((AddheroPolyFT4Packet *)result)->v0 = 0xD0;
    ((AddheroPolyFT4Packet *)result)->x1 = x + adjust;
    ((AddheroPolyFT4Packet *)result)->y3 = y + 0x2F;
    ((AddheroPolyFT4Packet *)result)->y2 = y + 0x2F;
    ((AddheroPolyFT4Packet *)result)->v3 = 0xFF;
    ((AddheroPolyFT4Packet *)result)->v2 = 0xFF;
    ((AddheroPolyFT4Packet *)result)->clut = (i & 0x3F) | 0x7C80;
    ((AddheroPolyFT4Packet *)result)->tpage = 5;
    ((AddheroPolyFT4Packet *)result)->tag = (((AddheroPolyFT4Packet *)result)->tag & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & 0xFF000000) | (result & 0xFFFFFF);
    return result + 0x28;
}

void addhero_enable_choice_toggle(void)
{
    g_addhero_choice_toggle = 1;
}

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

s32 addhero_parse_hex_suffix_byte(u8 *text, s32 unused1, s32 unused2)
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

/* ------------------------------------------------------------------ */
/* Former addhero_parse_entry_fields.c */
/* ------------------------------------------------------------------ */

extern s32 g_addhero_entry_fields[];

s32 addhero_parse_entry_fields(void)
{
    s32 i; s32 max; u8 *p; u8 *field; s32 count; s32 acc;
    u32 tmp0, tmp1, tmp2; s32 r; u8 *pattern;
    i = 0; max = i;
    while (i < g_addhero_entry_state) {
        do { pattern = (u8 *)&D_800ECF7C; } while (0);
        if (func_8001714C(pattern, (u8 *)&((AddheroDirEntry (*)[20])g_addhero_entries)[g_addhero_card_slot][i], 0xC) == 0) {
            count = 5;
            p = (u8 *)(g_addhero_card_slot * ADDHERO_CARD_DIRECTORY_BYTES + ((i << 4) + (i << 4) + (i << 3)) + (s32)g_addhero_entries + 0xC);
            acc = 0;
            while (((u8)(*p-'0') < 10) || ((u8)(*p-'a') < 6) || ((u8)(*p-'A') < 6)) {
                if (count == 0) break;
                acc <<= 4;
                if ((u8)(*p-'0') < 10) { tmp0=acc-0x30; acc=tmp0+*p; }
                else if ((u8)(*p-'A') < 6) { tmp1=acc-0x37; acc=tmp1+*p; }
                else if ((u8)(*p-'a') < 6) { tmp2=acc-0x57; acc=tmp2+*p; }
                p++; count--;
            }
            field = (u8 *)&((AddheroDirEntry (*)[20])g_addhero_entries)[g_addhero_card_slot][i].name[0xC];
            { s32 addr; addr = g_addhero_card_slot * 0x50 + (s32)g_addhero_entry_fields; *(s32 *)(addr + i*4) = acc; }
            r = addhero_parse_hex_suffix_byte(field, acc, count);
            g_addhero_entry_suffix_values[i] = r;
            if (max < r) max = r;
        } else {
            { s32 addr; addr = g_addhero_card_slot * 0x50 + (s32)g_addhero_entry_fields; *(s32 *)(addr + i*4) = -1; }
            g_addhero_entry_suffix_values[i] = 0;
        }
        i++;
    }
    return max;
}

/* ------------------------------------------------------------------ */
/* ADDHERO continuation declarations                                  */
/* ------------------------------------------------------------------ */

#include "gpu_packet.h"
#include "sdk/libgte.h"

/* addhero.c already has local RECT/TILE ABI views used by the earlier code. */
#define RECT AddheroSdkRect
#define TILE AddheroSdkTile
#include "sdk/libgpu.h"
#undef TILE
#undef RECT

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

/* ------------------------------------------------------------------ */
/* Globals                                                            */
/* ------------------------------------------------------------------ */

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

/* ------------------------------------------------------------------ */
/* Forward declarations                                               */
/* ------------------------------------------------------------------ */

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
void addhero_draw_hex_byte(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);
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

/* ------------------------------------------------------------------ */
/* Former addhero2.c functions */
/* ------------------------------------------------------------------ */

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

    memcpy(&buf, &g_addhero_file_template, 6);
    phase_result = 1;
    ((u8 *)&buf)[2] += *(u8 *)&g_addhero_card_slot;

    if (g_addhero_load_step == NULL)
    {
        goto block_return;
    }

    switch (*g_addhero_load_step)
    {
    case 1:
        phase_result = 3;
        func_8001729C(g_addhero_card_slot);
        func_8001724C(g_addhero_card_slot * 0x10);
        g_addhero_load_step = g_addhero_load_step + 1;
        break;

    case 2:
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
        g_addhero_entry_state = 0xFF;
        g_addhero_load_step = &D_80160574;
        break;

    case 3:
        addhero_release_primary_handles();
        g_addhero_load_step = g_addhero_load_step + 1;
        break;

    case 4:
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

    case 5:
        addhero_release_secondary_handles();
        g_addhero_load_step = g_addhero_load_step + 1;
        break;

    case 6:
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

    case 8:
        phase_result = 3;
        func_8001729C(g_addhero_card_slot);
        func_800172AC(g_addhero_card_slot * 0x10);
        g_addhero_load_step = g_addhero_load_step + 1;
        break;

    case 9:
        phase_result = 3;
        func_8001729C(g_addhero_card_slot);
        func_8001725C(g_addhero_card_slot * 0x10);
        g_addhero_primary_poll_countdown = 0x10;
        g_addhero_secondary_poll_countdown = 0x10;
        g_addhero_load_step = g_addhero_load_step + 1;
        break;

    case 0:
        phase_result = 2;
        g_addhero_write_in_progress = 0;
        break;

    case 10:
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

    case 15:
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

    case 16:
        do
        {
            poll_result = addhero_poll_secondary_handle_group();
        } while (poll_result == -1);
        g_addhero_load_step = g_addhero_load_step + 1;
        break;

    case 17:
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

    case 18:
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
            g_addhero_entry_state = 0xFF;
            g_addhero_load_step = &D_80160574;
        }
        else
        {
            g_addhero_load_step = g_addhero_load_step + 1;
        }
        break;

    case 19:
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

    case 20:
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

    case 24:
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

    case 30:
        g_addhero_retry_count = 5;
        g_addhero_load_step = g_addhero_load_step + 1;
        break;

    case 27:
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

    case 28:
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

    case 25:
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

    case 26:
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

    default:
        break;
    }

    goto block_return;

block_close_decrement:
    func_8001683C(g_addhero_file_handle);
    g_addhero_load_step = g_addhero_load_step - 1;

block_return:
    return phase_result;
}

/** @see decomp.me (100.00%) */
void addhero_restart_load_sequence(void)
{
    func_8001729C(g_addhero_card_slot);
    addhero_release_primary_handles();
    func_8001724C(g_addhero_card_slot * 0x10);
    g_addhero_load_step = D_8016057C;
}

/** @see decomp.me (100.00%) */
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

/** @see decomp.me (100.00%) */
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

/** @see decomp.me (100.00%) */
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

/** @see decomp.me (100.00%) */
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

/** @see decomp.me (100.00%) */
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

/** @see decomp.me (100.00%) */
void addhero_release_primary_handles(void)
{
    func_800167CC(g_addhero_primary_handle0);
    func_800167CC(g_addhero_primary_handle1);
    func_800167CC(g_addhero_primary_handle2);
    func_800167CC(g_addhero_primary_handle3);
}

/** @see decomp.me (100.00%) */
void addhero_release_secondary_handles(void)
{
    func_800167CC(g_addhero_secondary_handle0);
    func_800167CC(g_addhero_secondary_handle1);
    func_800167CC(g_addhero_secondary_handle2);
    func_800167CC(g_addhero_secondary_handle3);
}

/** @see decomp.me (100.00%) */
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

/** @see decomp.me (100.00%) */
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

void addhero_draw_hex_byte(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5)
{
    u16 pair[3];
    s32 row;
    s32 adjusted;
    s32 off;
    u16 *base;

    adjusted = arg2;
    if (arg2 < 0)
        adjusted = arg2 + 15;
    row = adjusted >> 4;
    off = row * 2;
    base = g_addhero_hex_glyphs;
    pair[0] = *(u16 *)((u8 *)base + off);
    off = (arg2 - row * 16) * 2;
    pair[1] = *(u16 *)((u8 *)base + off);
    pair[2] = 0;
    addhero_draw_cached_text(arg0, arg1, pair, arg3, arg4, 0, arg5);
}

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

    setlen(prim, 1);
    ((AddheroGpuPacket *)prim)->word4 = 0xE1000005;
    addPrim(ot, prim);
    return prim + 8;
}

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

s32 addhero_emit_glyph_sprite(AddheroGlyphSprite *sprite, s32 *ot, s32 cache_slot, s32 palette)
{
    u32 ot_tag_high_byte;
    s32 normalized_slot;
    u32 packet_address;
    s32 old_x;
    s32 new_x;
    s32 fits_line;

    g_addhero_glyph_cache[cache_slot].raw |= 0x10000;

    setlen(sprite, 3);
    setcode(sprite, 0x7C);
    sprite->packet.g0 = 0x80;
    sprite->packet.b0 = 0x80;
    sprite->packet.r0 = 0x80;
    normalized_slot = cache_slot;
    sprite->packet.x0 = g_addhero_glyph_cursor_x;
    sprite->packet.y0 = g_addhero_glyph_cursor_y;

    if (cache_slot < 0)
    {
        normalized_slot = cache_slot + 15;
    }

    sprite->packet.u0 = (cache_slot - ((normalized_slot >> 4) * 16)) * 16;
    sprite->packet.v0 = cache_slot & GLYPH_CACHE_ROW_MASK;
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

/** @see decomp.me (100.00%) */
void addhero_begin_glyph_cache_frame(void)
{
    s32 i;
    s32 *p;

    g_addhero_glyph_raster_cursor = g_addhero_glyph_raster_buffer;
    i = 0;
    p = (s32 *)g_addhero_glyph_cache;
    do
    {
        *p = (u16)*p;
        i++;
        p++;
    } while (i < 0x100);
}

/** @see decomp.me (100.00%) */
void addhero_evict_unused_glyphs(void)
{
    s32 i;
    s32 *p;
    s32 flag;

    i = 0;
    flag = 0x10000;
    p = (s32 *)g_addhero_glyph_cache;
    do
    {
        if (!(*p & flag))
        {
            *p = 0;
        }
        i++;
        p++;
    } while (i < 0x100);
}

/** @see decomp.me (100.00%) */
void addhero_reset_glyph_cache(void)
{
    s32 i;
    s32 *p;
    u8 *q;

    i = 0xFF;
    p = (s32 *)g_addhero_glyph_cache;
    p += 0xFF;
    do
    {
        *p = 0;
        i--;
        p--;
    } while (i >= 0);

    i = 0;
    q = g_addhero_glyph_raster_buffer;
    do
    {
        *(u8 *)(i + (s32)q) = 0;
        i++;
    } while (i <= 0x7FFF);
}

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
