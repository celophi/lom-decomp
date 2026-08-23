#include "common.h"

typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} RECT;

/** @brief One 0xC-byte animated CLOAD UI element. */
typedef struct CloadElement CloadElement;
struct CloadElement
{
    /* 0x0 */ s32 state;
    /* 0x4 */ s32 size_flags;
    /* 0x8 */ void (*draw)();
};

/** @brief Prefix view used for direct access to the first two element states. */
typedef struct
{
    /* 0x0 */ s32 first_state;
    /* 0x4 */ u8 first_payload[8];
    /* 0xC */ s32 second_state;
} CloadElementPoolHead;

typedef struct
{
    s16 x;
    s16 y;
} Vec2s;

typedef struct
{
    /* 0x0 */ u32 tag;
    /* 0x4 */ u8 r0, g0, b0, code;
    /* 0x8 */ s16 x0, y0;
    /* 0xC */ s16 w, h;
} TILE;

/** @brief Memory-card directory entry; layout matches Psy-Q struct DIRENTRY. */
typedef struct
{
    /* 0x00 */ char name[20];
    /* 0x14 */ s32 attr;
    /* 0x18 */ s32 size;
    /* 0x1C */ void *next;
    /* 0x20 */ s32 head;
    /* 0x24 */ char system[4];
} CloadDirEntry;

/** @brief Scratch draw/header record built on the stack before an emit. */
typedef struct
{
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s16 unk4;
    /* 0x06 */ s16 unk6;
    /* 0x08 */ u8 unk8[0x18];
} CloadFileHeaderScratch;

/** @brief 0x68-byte top-of-frame scratch buffer used by cload_advance_load_sequence. */
typedef struct
{
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s16 unk4;
    /* 0x06 */ u8 pad[0x62];
} CloadLoadScratch;

/** @brief 0x100-byte CD file/header record template (g_cload_file_template). */
typedef struct
{
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s16 unk4;
    /* 0x06 */ u8 pad[0x100 - 6];
} CloadFileHeader;

/** @brief CloadEntryHeader record with a byte flag at 0x6 (g_cload_entry_header_template). */
typedef struct
{
    /* 0x0 */ s32 unk0;
    /* 0x4 */ s16 unk4;
    /* 0x6 */ s8 unk6;
    /* 0x7 */ u8 pad[9];
} CloadEntryHeader;

extern s32 g_pad_input;
extern s32 g_cload_exit_requested;
extern CloadElementPoolHead g_cload_element_pool;
extern s32 g_cload_card_slot;
extern u8 D_8014A988[];
extern s16 D_8014EA38;
extern s32 g_cload_io_busy;
extern s32 g_cload_icon_resource;
extern s32 g_cload_scroll_y;
extern s32 g_cload_icon_palette;
extern s32 g_cload_progress_active;
extern s32 g_cload_scroll_target_y;
extern s32 g_cload_icon_phase;
extern s32 g_cload_icon_context;
extern u8 D_8015A350[];
extern s32 g_cload_entry_state;
extern s32 g_cload_selected_row;
extern s32 g_cload_result;
extern s32 g_cload_scroll_frames;
extern s32 g_cload_selection_status;
extern s32 g_cload_frame_parity;
extern s32 D_80162370;
extern s32 D_80162A10;
extern s32 D_80162A14;
extern u8 g_cload_entry_metadata[];
extern s32 g_cload_element1_state;
extern u8 D_8014651C[];
extern u8 D_8014652C[];
extern u8 D_80146534[];
extern u8 D_8014653C[];
extern s32 g_cload_choice_toggle;
extern u8 *g_cload_load_step;
extern s32 D_8003EC9C;
extern char D_800ECF7C[];
extern char g_cload_entries[];
extern u8 D_80162C5F;
extern s32 g_cload_entry_scan_active;
extern char D_800ECF8C[];
extern char D_800ECFC4[];
extern u16 D_80145E9C;
extern u16 D_80145E9E;
extern u16 D_80145EA0;
extern u16 D_80145EA2;
extern u16 D_80145EA4;
extern u16 D_80145EA8;
extern u16 D_80145EAA;
extern u16 D_80145EAC;
extern u16 D_80145EAE;
extern u16 D_80145EB0;
extern u16 D_80145EC4;
extern u16 D_80145EC8;
extern u16 D_80145ECA;
extern u16 D_80145ECC;
extern u16 D_80145ECE;
extern u8 g_cload_save_blob[];
extern u8 D_80042FD8[];
extern s32 D_80042FB4;
extern s32 g_cload_progress_bar_active;
extern s32 g_cload_progress_start_tick;
extern s32 g_cload_dialog_state;
extern u16 D_80145ED0;
extern u16 D_80145ED6;
extern u16 D_80145ED8;
extern u16 D_80145EDA;
extern u16 D_80145EDC;
extern u16 D_80145EDE;
extern u16 D_80145EF0;
extern u16 D_80145F4C;
extern s32 g_cload_rank_count;
extern s32 g_cload_entry_ranks[];
extern s32 g_cload_entry_suffix_values[];
extern u8 D_800EC3F6[2];
extern u16 D_80146338[];

/* Globals introduced with the unk1 (0x3520-0x56E0) function block. */
extern u8 D_800EC3FA[];
extern u8 D_800ECF9C;
extern u8 D_800ECFB0;
extern u8 D_80146528;
extern u8 D_80146538[];
extern u8 D_80162C90[];
extern CloadFileHeader g_cload_file_template;
extern CloadEntryHeader g_cload_entry_header_template;
extern s32 g_cload_entry_fields[];
extern s32 g_cload_retry_count;
extern s32 g_cload_primary_poll_countdown;
extern s32 g_cload_entry_value_limit;
extern s32 g_cload_selected_entry_extended;
extern s32 g_cload_secondary_poll_countdown;
extern s32 g_cload_primary_handle0;
extern s32 g_cload_primary_handle1;
extern s32 g_cload_primary_handle2;
extern s32 g_cload_primary_handle3;
extern s32 g_cload_secondary_handle0;
extern s32 g_cload_secondary_handle1;
extern s32 g_cload_secondary_handle2;
extern s32 g_cload_secondary_handle3;
extern s32 g_cload_file_handle;
extern s32 g_cload_text_line_start_x;
extern s32 g_cload_glyph_upload_x;
extern s32 g_cload_glyph_upload_y;
extern u8 *g_cload_glyph_raster_cursor;
extern u8 g_cload_glyph_raster_buffer[];
extern u8 g_cload_double_byte_char_table[];
extern u8 g_cload_single_byte_char_table[];
extern s32 g_cload_glyph_cursor_x;
extern s32 g_cload_glyph_cursor_y;
extern u16 g_cload_decimal_glyphs[];
extern u16 g_cload_hex_glyphs[];

extern int strncmp(char *, char *, int);
s32 cload_draw_load_prompt();
s32 cload_draw_load_progress(s32 ot, s32 prim, s32 x_offset, s32 y_offset);
s32 cload_draw_status_dialog(s32 *ot, s32 prim, s32 x_offset, s32 y_offset);
s32 func_800A88A0(s32 prim, s32 *ot, void *glyph, s32 a3, s32 x, s32 y, s32 mode);
s32 func_800A8A78(s32 *ot, s32 prim, s32 ch, s32 a3, Vec2s *pos, s32 mode);
u8 *cload_skip_hex_digits(void *);
void cload_terminate_multibyte_text(void *text);
s32 cload_draw_icon_highlight(s32 prim, s32 *ot, s32 x, s32 y, s32 highlight, s32 icon, s32 index, s32 row);
s32 cload_draw_cached_text(s32 prim, s32 *ot, u8 *text, s32 x, s32 y, s32 palette, s32 alignment);

/* Callees used by the unk1 (0x3520-0x56E0) function block. */
s32 func_8001714C(void *, void *, s32);
s32 func_8001680C(void *, s32);
s32 func_8001681C(s32, void *, s32);
s32 func_8001683C(s32);
void func_8001686C(void *);
s32 func_8001724C(s32);
s32 func_8001725C(s32);
s32 func_8001729C(s32);
s32 func_800172AC(s32);
s32 func_80032174(s32, void *, s32 *);
s32 func_800342CC(s32);
void func_800167EC(void);
void func_800167BC(s32);
void func_800167FC(void);
s32 func_800167AC(u32, s32, s32, s32);
void func_800167DC(s32);
s32 func_800167CC(s32);
s32 func_80016BCC(void *, void *, s8, void *);
s32 func_8001684C(void *);
s32 func_800170BC(void *, void *, u8);

/**
 * @brief Initialize and run the CLOAD save/continue menu.
 * @return CLOAD result code set by the menu loop.
 * @see decomp.me (100.00%)
 */
s32 cload_main(void)
{
    RECT rect;

    g_cload_entry_state = 0xFF;
    g_cload_card_slot = 0;
    cload_reset_entry_ranks();
    cload_load_icon_resources();
    cload_init_display();
    g_cload_result = 0;
    cload_init_stream_handles();
    g_cload_icon_phase = 0;
    rect.x = 0x140;
    rect.y = 0;
    rect.w = 0x40;
    rect.h = 0x100;
    func_8001990C(&rect, 0, 0, 0);
    cload_reset_glyph_cache();
    D_80162370 = 0;
    g_cload_progress_active = 0;
    g_cload_selection_status = 0;
    g_cload_io_busy = 0;
    g_cload_frame_parity = 0;
    g_cload_exit_requested = 0;
    func_800AA02C();
    cload_build_ui_elements();
    cload_run_menu_loop();
    return g_cload_result;
}

/**
 * @brief Run the double-buffered CLOAD menu loop until it exits.
 * @see decomp.me (100.00%)
 */
void cload_run_menu_loop(void)
{
    RECT rect;
    u8 *p;
    u8 *q;
    s32 flag;
    s32 temp;

    func_80019788(0);
    func_8002054C(0);
    rect.x = 0;
    rect.y = 0;
    rect.w = 0x140;
    rect.h = 0x1D8;
    func_8001990C(&rect, 0, 0, 0);
    p = D_8014A988;
    flag = 0;
    func_80019C74(p + 0x40, 0x1000);
    func_80019C74(p + 0x7D04, 0x1000);
    func_80019FB8(p + 0x4040);
    func_800157DC();
    func_800196F0(1);
    do
    {
        q = p + 0x40;
        func_80019C74(q, 0x1000);
        *(u8 **)(p + 0x40B8) = D_8015A350 + (flag << 14);
        func_800A9E78();
        temp = g_pad_input & 0xF000;
        if (temp != 0)
        {
            g_pad_input = temp;
        }
        func_80067BBC(p);
        if (cload_update_frame(p) != 0)
        {
            break;
        }
        func_80019788(0);
        func_800157B0(2);
        func_8002054C(2);
        func_8001990C(p + 0x40B0, 0, 0, 0);
        flag = 0;
        if (p == D_8014A988)
        {
            p += 0x7CC4;
            flag = 1;
        }
        else
        {
            p = D_8014A988;
        }
        func_80019FB8(p + 0x4040);
        func_80019DEC(p + 0x4054);
        func_80019D7C(q + 0x3FFC);
        func_800157DC();
        func_800122C0();
    } while (1);
    func_800158E0();
    func_8002054C(0);
}

/**
 * @brief Initialize the CLOAD display and draw buffers.
 * @see decomp.me (100.00%)
 */
void cload_init_display(void)
{
    u8 *base;
    s16 *bank2;

    /* Reserve the outgoing-argument area: a wider (7-arg) call was compiled
       out here, so the frame keeps its space. */
    if (0)
    {
        func_8002054C(0, 0, 0, 0, 0, 0, 0);
    }
    func_8001D5AC(0x5DC);
    func_8001D58C(0xA0, 0x78);
    D_8014EA38 = 0;
    base = (u8 *)&D_8014EA38;
    bank2 = (s16 *)(base + 0x7CC4);
    *(s16 *)(base + 0x2) = 0;
    *(s16 *)(base + 0x4) = 0x140;
    *(s16 *)(base + 0x6) = 0xF0;
    *(s16 *)(base + 0x7CC4) = 0;
    bank2[1] = 0xE8;
    bank2[2] = 0x140;
    bank2[3] = 0xF0;
    func_80019788(0);
    func_8002054C(0);
    func_8001C62C(base - 0x70, 0, 0, 0x140, 0xF0);
    func_8001C62C(base + 0x7C54, 0, 0xE8, 0x140, 0xF0);
    func_8001C56C(base - 0x5C, 0, 0xF0, 0x140, 0xE0);
    func_8001C56C(base + 0x7C68, 0, 0x8, 0x140, 0xE0);
    base[0x7C7E] = 0;
    base[-0x46] = 0;
    func_80067B8C();
    func_80067EB4(0x100, 0x100, 0x100, 0x14);
}

/**
 * @brief Advance one CLOAD menu frame and report whether it should exit.
 * @param frame Function argument.
 * @return 1 when the overlay should exit, otherwise 0.
 * @see decomp.me (100.00%)
 */
s32 cload_update_frame(s32 frame)
{
    if (g_cload_exit_requested != 0)
    {
        cload_shutdown_stream_handles();
        field_text_reset_windows();
        func_80019788(0);
        return 1;
    }
    field_text_reset_scratch();
    cload_begin_glyph_cache_frame();
    cload_update_menu(frame);
    cload_evict_unused_glyphs();
    func_80063194();
    g_cload_frame_parity ^= 1;
    return 0;
}

s32 cload_draw_entry_list(s32 *, s32, s32, s32);
s32 cload_draw_header_label(s32 *, s32, s32, s32);
s32 cload_draw_card_slot0_label(s32 *, s32, s32, s32);
s32 cload_draw_card_slot1_label(s32 *, s32, s32, s32);
s32 cload_draw_selected_entry_details(s32 *, s32, s32, s32);
void cload_clear_elements();
CloadElement *cload_alloc_element();

/**
 * @brief Build the fixed 5-entry GPU primitive/callback chain for the loader.
 * @note WIP. Structure, control flow, and the FRAME-04 dead-call frame padding
 *       are correct; the residual is a register-allocation permutation - the
 *       value temp lands in v1 (target v0) and the five hoisted mask constants
 *       occupy a permuted set of saved registers.
 * @see decomp.me (83.09%)
 */
void cload_build_ui_elements(void)
{
    CloadElement *p;
    s32 v;
    s32 v1;

    g_cload_scroll_frames = 0;
    g_cload_scroll_target_y = 0;
    g_cload_scroll_y = 0;
    g_cload_selected_row = 0;
    g_cload_selection_status = 0;
    if (0)
    {
        cload_clear_elements(0, 0, 0, 0, 0);
    }
    cload_clear_elements();
    v = g_cload_element_pool.first_state;
    v = v & ~7;
    v = v | 1;
    g_cload_element_pool.first_state = v;

    p = cload_alloc_element();
    p->draw = cload_draw_entry_list;
    v = p->state;
    v = v & ~0x78;
    v = v | 8;
    v = v & 0xFFFF007F;
    v = v | 0xE00;
    p->state = v;
    *((u8 *)p + 2) = 0x4A;
    v = p->size_flags;
    v = v & ~0x200;
    p->size_flags = v;
    v1 = p->state;
    v1 = v1 & 0xFFFFFF;
    v1 = v1 | 0x08000000;
    p->state = v1;
    v = v | 1;
    v = v & ~0x1FE;
    v = v | 0x92;
    p->size_flags = v;

    p = cload_alloc_element();
    p->draw = cload_draw_header_label;
    v = p->state;
    v = v & ~7;
    v = v | 2;
    v = v & ~0x78;
    v = v | 8;
    v = v & 0xFFFF007F;
    v = v | 0x2800;
    p->state = v;
    *((u8 *)p + 2) = 0xC;
    v = p->size_flags;
    v = v & ~0x200;
    p->size_flags = v;
    v1 = p->state;
    v1 = v1 & 0xFFFFFF;
    v1 = v1 | 0xA0000000;
    p->state = v1;
    v = p->size_flags;
    v = v & ~1;
    v = v & ~0x1FE;
    v = v | 0x1E;
    p->size_flags = v;

    p = cload_alloc_element();
    v = p->state;
    v = v & ~7;
    v = v | 2;
    v = v & ~0x78;
    v = v | 8;
    v = v & 0xFFFF007F;
    v = v | 0xC00;
    p->state = v;
    p->draw = cload_draw_card_slot0_label;
    *((u8 *)p + 2) = 0x2C;
    v = p->size_flags;
    v = v & ~0x200;
    p->size_flags = v;
    v1 = p->state;
    v1 = v1 & 0xFFFFFF;
    v1 = v1 | 0x80000000;
    p->state = v1;
    v = v & ~1;
    v = v & ~0x1FE;
    v = v | 0x1E;
    p->size_flags = v;

    p = cload_alloc_element();
    v = p->state;
    v = v & ~7;
    v = v | 2;
    v = v & ~0x78;
    v = v | 8;
    v = v & 0xFFFF007F;
    v = v | 0x5400;
    p->state = v;
    p->draw = cload_draw_card_slot1_label;
    *((u8 *)p + 2) = 0x2C;
    v = p->size_flags;
    v = v & ~0x200;
    p->size_flags = v;
    v1 = p->state;
    v1 = v1 & 0xFFFFFF;
    v1 = v1 | 0x80000000;
    p->state = v1;
    v = v & ~1;
    v = v & ~0x1FE;
    v = v | 0x1E;
    p->size_flags = v;

    p = cload_alloc_element();
    p->draw = cload_draw_selected_entry_details;
    v = p->state;
    v = v & ~7;
    v = v | 2;
    v = v & ~0x78;
    v = v | 8;
    v = v & 0xFFFF007F;
    v = v | 0xF00;
    p->state = v;
    *((u8 *)p + 2) = 0xA0;
    v = p->size_flags;
    v = v & ~0x200;
    p->size_flags = v;
    v1 = p->state;
    v1 = v1 & 0xFFFFFF;
    v1 = v1 | 0x04000000;
    p->state = v1;
    v = v | 1;
    v = v & ~0x1FE;
    v = v | 0x66;
    p->size_flags = v;

    v = g_cload_element_pool.first_state;
    v = v & ~7;
    g_cload_element_pool.first_state = v;
}

/**
 * @brief Update input, loading state, scrolling, and UI elements for one frame.
 * @see decomp.me (100.00%)
 */
void cload_update_menu(void)
{
    s32 delta;

    cload_update_elements();
    g_cload_icon_phase += 2;
    if ((g_cload_element1_state & 0x7F) == 2)
    {
        cload_update_load_sequence();
    }
    if ((u16)g_pad_input == 0xFFFF)
    {
        g_pad_input = 0;
    }
    cload_handle_input();
    if (g_cload_scroll_frames != 0)
    {
        s32 base = g_cload_scroll_y;
        delta = (g_cload_scroll_target_y - g_cload_scroll_y) / g_cload_scroll_frames;
        g_cload_scroll_frames -= 1;
        g_cload_scroll_y += delta;
    }
    else
    {
        g_cload_scroll_y = g_cload_scroll_target_y;
    }
}

/**
 * @brief Advance the active load sequence and react to its phase result.
 * @param phase Load-sequence phase result.
 * @see decomp.me (100.00%)
 */
void cload_update_load_sequence(s32 phase)
{
    if (g_cload_entry_state >= 0x10)
    {
        if (g_cload_load_step == NULL)
        {
            g_cload_load_step = D_8014651C;
        }
    }
    do
    {
        phase = cload_advance_load_sequence(phase);
    } while (phase == 3);
    if (phase == 2)
    {
        g_cload_load_step = D_80146534;
    }
    if (phase == 4)
    {
        g_cload_load_step = D_8014652C;
    }
    if (phase == 5)
    {
        g_cload_entry_state = 0xF9;
        g_cload_load_step = D_80146534;
    }
}

/**
 * @brief Handle CLOAD menu navigation, confirm, and cancel input.
 * @return Input-handler status used by the caller.
 * @note WIP. Structurally and semantically correct (verified via probe:
 *       the packet unk0/unk4 register roles and the loop-preheader a1/a2
 *       roles are a coupled register-coloring pair - fixing one region's
 *       roles regresses the other by the same amount). Every attempt to
 *       force the target's exact roles, including a permuter candidate
 *       that scored higher, was verified to silently drop the `| 0x56`
 *       term from the packet unk4 store and rejected.
 * @see decomp.me (94.25%)
 */
s32 cload_handle_input(void)
{
    s32 pending;
    s32 status;
    s32 flag_a3;
    s32 flag_a2;
    s32 count;
    s32 last;
    s32 arg0;
    CloadElement *p;

    if ((g_cload_element_pool.second_state & 7) == 0)
    {
        g_cload_exit_requested = 1;
        return;
    }
    if (g_cload_exit_requested != 0)
    {
        return;
    }
    if ((g_cload_element_pool.second_state & 7) >= 3)
    {
        return;
    }
    if ((g_cload_element_pool.first_state & 7) != 0)
    {
        return;
    }
    pending = g_cload_entry_state;
    if (pending == 0xFF)
    {
        return;
    }
    if (g_cload_entry_scan_active != 0)
    {
        return;
    }
    if (g_cload_io_busy != 0)
    {
        return;
    }
    if ((u32)(*g_cload_load_step - 6) < 2U)
    {
        return;
    }
    status = g_pad_input;
    if (status & 0x40)
    {
        g_cload_exit_requested = 1;
        g_cload_result = 1;
        func_800A3938(0x78, 0x80);
        return;
    }
    if (status & 0xA100)
    {
        func_800A3938(0x7D, 0x80);
        g_cload_scroll_frames = 0;
        g_cload_scroll_target_y = 0;
        g_cload_scroll_y = 0;
        g_cload_selected_row = 0;
        g_cload_load_step = NULL;
        g_cload_entry_state = 0xFF;
        g_cload_selection_status = 0;
        g_cload_card_slot ^= 1;
        cload_reset_entry_ranks();
        return;
    }
    if (pending >= 0x10)
    {
        return;
    }
    count = 1;
    if (status & 8)
    {
        g_pad_input = 0x4000;
        count = 1;
    }
    if (g_pad_input & 4)
    {
        g_pad_input = 0x1000;
        count = 1;
    }
    last = pending - 1;
    flag_a3 = g_pad_input & 0x1000;
    flag_a2 = g_pad_input & 0x4000;
    while (count != 0)
    {
        if (flag_a3 != 0)
        {
            g_cload_selected_row -= 1;
            if (g_cload_selected_row < 0)
            {
                g_cload_selected_row = last;
            }
        }
        if (flag_a2 != 0)
        {
            g_cload_selected_row += 1;
            if (g_cload_selected_row >= pending)
            {
                g_cload_selected_row = 0;
            }
        }
        count -= 1;
    }
    if (g_pad_input & 0x5000)
    {
        cload_commit_selected_entry();
        func_800A3938(0x7D, 0x80);
        cload_scroll_to_selection();
        return;
    }
    if (g_pad_input & 0x220)
    {
        s32 term1 = g_cload_card_slot * 0x320;
        s32 term2 = (g_cload_selected_row * 0x28) + (s32)g_cload_entries;

        if (strncmp(D_800ECF7C, (char *)(term1 + term2), 0xC) != 0)
        {
            arg0 = 0x78;
        }
        else
        {
            if ((D_80162C5F == D_8003EC9C) || (D_80162C5F == 0xFF))
            {
                p = cload_alloc_element(D_80162C5F);
                p->draw = cload_draw_load_prompt;
                p->state = (p->state & ~0x78) | 8;
                p->state = (p->state & 0xFFFF007F) | 0x800;
                *((u8 *)p + 2) = 0x5B;
                p->size_flags = (p->size_flags | 1) & ~0x1FE;
                p->size_flags = p->size_flags | 0x56;
                p->state = (p->state & 0xFFFFFF) | 0x20000000;
                cload_enable_choice_toggle();
                cload_restart_load_sequence();
                arg0 = 0x7E;
            }
            else
            {
                arg0 = 0x78;
            }
        }
        func_800A3938(arg0, 0x80);
    }
}

/**
 * @brief Put every live UI element into its closing state.
 * @see decomp.me (100.00%)
 */
void cload_close_all_elements(void)
{
    s32 temp_v1;
    s32 var_a1;
    s32 *var_a0;
    s32 temp;

    var_a0 = (s32 *)&g_cload_element_pool;
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

/**
 * @brief Move the list scroll target to keep the selected row visible.
 * @see decomp.me (100.00%)
 */
void cload_scroll_to_selection(void)
{
    s32 base;
    s32 delta;

    base = g_cload_selected_row * 14;
    delta = base - g_cload_scroll_y;
    if (delta >= 0x3C)
    {
        g_cload_scroll_target_y = base - 0x38;
        g_cload_scroll_frames = 4;
    }
    if (delta < 0)
    {
        g_cload_scroll_target_y = g_cload_selected_row * 14;
        g_cload_scroll_frames = 4;
    }
}

/**
 * @brief Run the UI element update/draw pass.
 * @see decomp.me (100.00%)
 */
void cload_update_elements(void)
{
    cload_update_and_draw_elements();
}

#define GLYPH_SYM(sym, off) ((void *)(((u8 *)&(sym) - (off)) + (sym)))
#define GLYPH_OFF(base, off) ((void *)((base) + *(u16 *)((base) + (off))))

/**
 * @brief Draw the visible save-entry list and selection cursor.
 * @param ot Ordering-table head.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 * @note WIP. Menu string/glyph-row drawing callback (state-dispatched TILE +
 *       text renderer). Structurally correct; residue is an `ot` parameter
 *       register-letter offset that recurs through most of the function body
 *       (content matches, register name differs - counted as argdiff, not a
 *       real mismatch) plus two small leftover items: a `g_cload_entry_state` reload
 *       at the loop's zero-trip guard that the target avoids by reusing the
 *       switch dispatch value, and one duplicated `D_80145EA4` address
 *       computation right before the final row's string-glyph call.
 * @see decomp.me (93.53%)
 */
s32 cload_draw_entry_list(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    s32 state = g_cload_entry_state;

    switch (state)
    {
    case 0xF8:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80145ED0, 0x34), 1, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xF9:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80145ED0, 0x34), 1, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFA:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80145E9E, 2), 1, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFD:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80145EA0, 4), 1, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFB:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80145EAC, 0x10), 1, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFC:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80145EAE, 0x12), 1, -x_offset + 0x84, -y_offset, 2);
        break;
    default:
        if (g_cload_entry_scan_active != 0)
        {
            s32 x;
            u8 *base;
        case 0xFF:
            x = -x_offset + 0x84;
            base = (u8 *)&D_80145E9C;
            prim = func_800A88A0(prim, ot, base + D_80145E9C, 1, x, -y_offset, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x1E), 1, x, 0xE - y_offset, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 1, x, 0x1C - y_offset, 2);
            break;
        }
        if (state > 0)
        {
            s32 i;
            s32 off;
            s32 base_x;
            s32 *flag_ptr;
            void *str_glyph;
            void *misc_glyph;
            char *entry;
            Vec2s pos;
            s32 row_y;
            u8 *base;

            base_x = -x_offset;
            entry = g_cload_entries;
            base = (u8 *)&D_80145E9C;
            for (i = 0, off = 0; i < g_cload_entry_state; entry += 0x28, i++, off += 4)
            {
                row_y = ((i * 14) - y_offset) - g_cload_scroll_y;
                if ((u32)(row_y + 0xD) < 0x56U)
                {
                    flag_ptr = (s32 *)((u8 *)g_cload_entry_ranks + off);
                    if (*flag_ptr >= 0)
                    {
                        pos.x = base_x + 0x86;
                        pos.y = row_y;
                        prim = func_800A88A0(func_800A8A78(ot, prim, *(s32 *)((u8 *)g_cload_entry_suffix_values + off), 1, &pos, 0), ot, base + D_80145ECA, 1, base_x + 0x70, row_y, 0);
                        if ((g_cload_rank_count - 1) == *flag_ptr)
                        {
                            misc_glyph = GLYPH_OFF(base, 0x36);
                            prim = func_800A88A0(prim, ot, misc_glyph, 1, base_x + 0xC2, row_y, 0);
                        }
                        else if (*flag_ptr < 2)
                        {
                            misc_glyph = GLYPH_OFF(base, 0x38);
                            prim = func_800A88A0(prim, ot, misc_glyph, 1, base_x + 0xC2, row_y, 0);
                        }
                        if (*cload_skip_hex_digits((void *)((g_cload_card_slot * 0x320) + (s32)entry + 0xC)) == 0x2B)
                        {
                            prim = func_800A88A0(prim, ot, base + D_80145F4C, 1, 0xF8 - x_offset, row_y, 1);
                        }
                    }
                    if (strncmp(D_800ECF7C, (char *)((g_cload_card_slot * 0x320) + (s32)entry), 0xC) == 0)
                    {
                        str_glyph = base + D_80145EA2;
                    }
                    else if (strncmp(D_800ECF8C, (char *)((g_cload_card_slot * 0x320) + (s32)entry), 0xC) == 0)
                    {
                        str_glyph = base + D_80145ED6;
                    }
                    else if (strncmp(D_800ECFC4, (char *)((g_cload_card_slot * 0x320) + (s32)entry), 8) == 0)
                    {
                        str_glyph = base + D_80145EB0;
                    }
                    else
                    {
                        str_glyph = base + D_80145EA4;
                    }
                    prim = func_800A88A0(prim, ot, str_glyph, 1, base_x, row_y, 0);
                }
            }
        }
        {
            s32 y0 = ((g_cload_selected_row * 14) - y_offset) - g_cload_scroll_y;

            if (g_cload_entry_scan_active == 0)
            {
                TILE *tile = (TILE *)prim;

                *(u32 *)&tile->r0 = 0xF080F0;
                *((u8 *)tile + 3) = 3;
                tile->code = 0x62;
                tile->y0 = (s16)(y0 - 1);
                tile->w = 0x108;
                tile->x0 = 0;
                tile->h = 0xE;
                tile->tag = (tile->tag & 0xFF000000) | (*ot & 0xFFFFFF);
                *ot = (*ot & 0xFF000000) | ((s32)tile & 0xFFFFFF);
                prim += 0x10;
            }
        }
        break;
    }
    return prim;
}

/**
 * @brief Advance a pointer past a run of hex-digit characters ('0'-'9',
 *        'a'-'f', 'A'-'F').
 * @param text Pointer to the first character to test.
 * @return Pointer to the first character that is not a hex digit.
 * @note WIP. Semantics, control flow, and the target bytes are fully
 *       understood (three independent range checks, each looping back to
 *       the function's own entry on a match). The residual is that GCC's
 *       jump2 cross-jump pass (jump.c, cross_jump=1) merges all three
 *       "advance and loop back" arms into one shared tail here, while the
 *       target keeps them independent - each of its three branches carries
 *       its own delay-slot increment plus a compensating decrement on the
 *       fallthrough (reorg target-steal-with-compensation). No source
 *       rephrasing tried so far (if/else-if, sequential if-continue,
 *       self-tail-recursion, do-while, for-loop-continue, precomputed
 *       conditions) avoids the merge; see the crossjump_oracle OVER-MERGED
 *       finding and idiom candidates JUMP-16/JUMP-19 in idioms.md, neither
 *       of which closed it. A 300k-iteration permuter run found no valid
 *       (semantics-preserving) improvement past this score.
 * @see decomp.me (65.63%)
 */
u8 *cload_skip_hex_digits(void *text)
{
    u8 *p;
    u8 c;

    p = (u8 *)text;
    while (1)
    {
        c = *p;
        if ((u32)(c - '0') < 10)
        {
            p++;
            continue;
        }
        if ((u32)(c - 'a') < 6)
        {
            p++;
            continue;
        }
        if ((u32)(c - 'A') < 6)
        {
            p++;
            continue;
        }
        break;
    }
    return p;
}

/**
 * @brief Draw the fixed CLOAD header label.
 * @param ot Ordering-table head.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 * @see decomp.me (100.00%)
 */
s32 cload_draw_header_label(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;

    return func_800A88A0(prim, ot, GLYPH_SYM(D_80145EC8, 0x2C), 1, -x_offset + 0x50, -y_offset, 2);
}

/**
 * @brief Draw the first memory-card slot label.
 * @param ot Ordering-table head.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 * @see decomp.me (100.00%)
 */
s32 cload_draw_card_slot0_label(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    s32 a3;
    void *glyph;
    RECT pos;

    a3 = 1;
    glyph = GLYPH_SYM(D_80145EA8, 0xC);
    if (g_cload_card_slot != 0)
    {
        a3 = 3;
    }
    return func_800A88A0(prim, ot, glyph, a3, -x_offset + 0x40, -y_offset, 2);
}

/**
 * @brief Draw the second memory-card slot label.
 * @param ot Ordering-table head.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 * @see decomp.me (100.00%)
 */
s32 cload_draw_card_slot1_label(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    s32 a3;
    void *glyph;
    RECT pos;

    a3 = 1;
    glyph = GLYPH_SYM(D_80145EAA, 0xE);
    if (g_cload_card_slot == 0)
    {
        a3 = 3;
    }
    return func_800A88A0(prim, ot, glyph, a3, -x_offset + 0x40, -y_offset, 2);
}

/**
 * @brief Draw metadata for the selected save entry.
 * @param ot Ordering-table head.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 * @note WIP. Save-slot HUD callback: draws either the elapsed-play-time
 *       display (hours:minutes plus a 3-memcard-icon highlight strip) when
 *       the slot name matches the empty-slot marker, or the slot's save-file
 *       name otherwise. Structurally and semantically matched (frame, control
 *       flow, and every field offset into the g_cload_entry_metadata status block agree
 *       with the target); the residue is a coupled register-allocation
 *       problem in the icon-highlight loop's cload_draw_icon_highlight call: the target
 *       tracks the "entries seen so far" count and the raw loop index as two
 *       separate spilled locals (sp+0x18/0x1c, extra sp+0x138/0x140 spills
 *       around the call), while reintroducing that second counter here
 *       consistently regresses the whole function's register allocation
 *       instead of only affecting this call. Permuter (gcc272_cdk, ~60k
 *       iterations across two seeds) found no valid improvement past this
 *       point.
 * @see decomp.me (89.53%)
 */
s32 cload_draw_selected_entry_details(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    s32 result;
    Vec2s pos;
    u8 name[0x21];
    char unused_pad[228];
    s32 slot[3];

    result = prim;
    if (g_cload_selection_status == 0)
    {
        return result;
    }
    if (g_cload_entry_scan_active != 0)
    {
        return result;
    }
    if (g_cload_selection_status != 3 && g_cload_entry_state < 0x10)
    {
        if (g_cload_selection_status == 2)
        {
            s32 x = -x_offset;
            u8 *base;

            result = func_800A88A0(prim, ot, GLYPH_SYM(D_80145EC4, 0x28), 1, x, -y_offset, 0);
            base = (u8 *)&D_80145EC4 - 0x28;
            return func_800A88A0(result, ot, GLYPH_OFF(base, 0x2A), 1, x, 0x10 - y_offset, 0);
        }
        else
        {
            s32 term1 = g_cload_card_slot * 0x320;
            s32 term2 = (g_cload_selected_row * 0x28) + (s32)g_cload_entries;

            if (strncmp(D_800ECF7C, (char *)(term1 + term2), 0xC) == 0)
            {
                u8 *base90 = g_cload_entry_metadata;

                if (base90[0xCF] == 0xFF || base90[0xCF] == D_8003EC9C)
                {
                    s32 present_count;
                    s32 i;
                    s32 step;
                    s32 half_step;
                    s32 base_x;
                    s32 base_y;
                    s32 total;
                    s32 hours;
                    s32 minutes;
                    s32 sign_adj;
                    s32 time_val;

                    slot[0] = (u32)(*(s32 *)(base90 + 0x18)) >> 0x19;
                    slot[1] = ((u32)(*(s32 *)(base90 + 0x20)) >> 0x12) & 0x7F;
                    slot[2] = (u32)(*(s32 *)(base90 + 0x20)) >> 0x19;
                    g_cload_icon_palette = (s32)base90[0x1F];

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
                        time_val = g_cload_icon_phase;
                        if (g_cload_icon_phase < 0)
                        {
                            time_val = g_cload_icon_phase + 0x1F;
                        }
                        g_cload_icon_phase -= (time_val >> 5) << 5;
                        break;
                    case 3:
                        step = 0x10;
                        half_step = 0x20;
                        g_cload_icon_phase %= 0x60;
                        break;
                    default:
                        step = 0x10;
                        half_step = 0x20;
                        g_cload_icon_phase = 0x1F;
                        break;
                    }

                    total = 0;
                    base_x = half_step;
                    base_y = 0;
                    for (i = 0; i < 3; i++)
                    {
                        if (slot[i] != 0x7F)
                        {
                            s32 adjust = step;
                            s32 rem;
                            s32 hi;
                            s32 delta;

                            if ((g_cload_icon_phase >= base_y && g_cload_icon_phase < base_x && (delta = g_cload_icon_phase - base_y, 1))
                                || (rem = base_x % (step * present_count), g_cload_icon_phase >= rem && g_cload_icon_phase < (hi = rem + step) && (delta = hi - g_cload_icon_phase, 1)))
                            {
                                adjust += delta;
                            }
                            total += adjust;
                            result = cload_draw_icon_highlight(result, ot, total - x_offset, -y_offset, adjust, slot[i], i, i);
                        }
                        base_x += step;
                        base_y += step;
                    }

                    {
                        s32 unk30 = *(s32 *)(base90 + 0x30);
                        s32 x = -x_offset;
                        s32 y = -y_offset;

                        sign_adj = unk30 >> 0x1F;
                        pos.x = (s16)(x + 0x70);
                        pos.y = (s16)y;
                        hours = unk30 / 216000;
                        result = func_800A88A0(func_800A8A78(ot, result, hours, 1, &pos, 1), ot, D_800EC3F6[0] + (D_800EC3F6[1] << 8) + ((s32)&D_800EC3F6 - 0x32), 1, x + 0x6F, y, 0);
                        minutes = ((unk30 / 3600) - sign_adj) - (hours * 0x3C);
                        if (minutes < 0xA)
                        {
                            pos.x = (s16)(x + 0x7D);
                            pos.y = (s16)y;
                            result = func_800A8A78(ot, result, 0, 1, &pos, 1);
                        }
                        pos.x = (s16)(x + 0x85);
                        pos.y = (s16)y;
                        result = func_800A88A0(func_800A88A0(func_800A8A78(ot, result, minutes, 1, &pos, 1), ot, base90, 1, x + 0x54, y + 0x10, 0), ot, GLYPH_OFF((u8 *)D_80146338, (*(s32 *)(base90 + 0x20) & 0x3FFFF) * 2), 1, x + 0x54, y + 0x20, 0);
                    }
                }
                else
                {
                    result = func_800A88A0(result, ot, GLYPH_SYM(D_80145EF0, 0x54), 1, -x_offset, -y_offset, 0);
                }
            }
            else
            {
                s32 j;

                cload_terminate_multibyte_text(&D_80162A14);
                if ((u32)(*((u8 *)&D_80162A10 + 0x24) - 1) >= 0x7FU)
                {
                    for (j = 0; j < 0x20; j++)
                    {
                        name[j] = *((u8 *)&D_80162A14 + j);
                    }
                    name[j] = 0;
                    result = cload_draw_cached_text(result, ot, name, -x_offset, -y_offset, 1, 0);

                    for (j = 0; j < 0x20; j++)
                    {
                        name[j] = *((u8 *)&D_80162A10 + 0x24 + j);
                    }
                    name[j] = 0;
                    result = cload_draw_cached_text(result, ot, name, -x_offset, 0x10 - y_offset, 1, 0);
                }
            }
        }
    }
    return result;
}

/**
 * @brief Zero-fill a 64-byte text buffer after its encoded terminator.
 * @param text Encoded text buffer.
 * @see decomp.me (100.00%)
 */
void cload_terminate_multibyte_text(void *text)
{
    u8 *p;
    s32 i;

    p = (u8 *)text;
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
 * @brief Mark all eight UI elements as inactive.
 * @see decomp.me (100.00%)
 */
void cload_clear_elements(void)
{
    CloadElement *p;
    s32 i;

    p = (CloadElement *)&g_cload_element_pool;
    for (i = 0; i < 8; i++)
    {
        p->state &= ~7;
        p++;
    }
}

/**
 * @brief Activate and return the first free UI element.
 * @return First free element, or the pool head if all slots are busy.
 * @see decomp.me (100.00%)
 */
CloadElement *cload_alloc_element(void)
{
    CloadElement *p;
    s32 i;

    p = (CloadElement *)&g_cload_element_pool;
    for (i = 0; i < 8; i++, p++)
    {
        if ((p->state & 7) == 0)
        {
            p->size_flags |= 0x200;
            p->state = (p->state & ~7) | 1;
            return p;
        }
    }
    return (CloadElement *)&g_cload_element_pool;
}

/** @brief GPU packet emitted into the ordering table; unk0 is the OT link word. */
typedef struct
{
    /* 0x0 */ s32 unk0;
    /* 0x4 */ s32 unk4;
    /* 0x8 */ s16 unk8;
    /* 0xA */ s16 unkA;
    /* 0xC */ s16 unkC;
    /* 0xE */ u16 unkE;
} CloadGpuPacket;

/** @brief Draw context at arg0 + 0x40; unk0 is the head of the ordering table. */
typedef struct
{
    /* 0x0 */ s32 unk0;
} CloadOrderingTable;

/** @brief Owning screen state passed to the element tick/draw pass. */
typedef struct
{
    u8 pad0[0x40B2];
    /* 0x40B2 */ s16 unk40B2;
    u8 pad40B4[4];
    /* 0x40B8 */ CloadGpuPacket *unk40B8;
} CloadFrameState;

/** @brief Per-element draw callback stored at element + 8. */
typedef CloadGpuPacket *(*CloadElementDrawFunc)();

CloadGpuPacket *cload_emit_scroll_arrow();
CloadGpuPacket *cload_emit_window_frame();
CloadGpuPacket *cload_emit_icon_highlight_strip();

/**
 * @brief Advance and draw the eight pool elements for one frame.
 *
 * Walks the 0xC-stride element pool at g_cload_element_pool, links each live element's
 * packets into the ordering table, and dispatches on the low 3 bits of its
 * state word: 1 grows, 2 holds, 3 shrinks, 4 counts down to idle.
 *
 * @param frame Owning screen state; unk40B8 is the packet cursor, read on entry
 *             and written back on exit.
 *
 * @note NOT MATCHED (328/365 exact rows). Residue is the case 1/3/4 tails,
 *       where the target stores the updated state word before testing it and
 *       materializes the comparison constant 8 early; sched_oracle reports
 *       emit[li 8] < emit[andi 15] violated. Two shapes are required to match:
 *       `volatile u32 *var_s3` (the target emits 12 loads of the element word
 *       and gcc CSEs 3 away without it), and the `do { ... } while (0)` around
 *       the var_s0 advance, which is an [ALLOC-23] loop-note ref bump worth 28
 *       exact rows - it wins s0 for var_s0 against temp_s1. A second such
 *       wrapper around the case-1 state-word update is worth 2 more. gosub's
 *       func_80143C58 is the matched 100% twin and the source model to work
 *       from. See working/cload_update_and_draw_elements/STATUS.md.
 * @see decomp.me (96.49%)
 */
void cload_update_and_draw_elements(CloadFrameState *frame)
{
    CloadGpuPacket *var_s0;
    CloadOrderingTable *var_s5;
    volatile u32 *var_s3;
    s32 temp_s1;
    s32 temp_s2;
    s32 sp80;
    s32 sp20[24];
    u32 temp_a3;
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

    var_s0 = frame->unk40B8;
    var_s5 = (CloadOrderingTable *)((u8 *)frame + 0x40);

    if ((g_cload_entry_state < 0x10) && ((g_cload_element1_state & 7) == 2))
    {
        if ((g_cload_entry_state * 0xE) > (g_cload_scroll_y + 0x49))
        {
            var_s0 = cload_emit_scroll_arrow(var_s0, var_s5, 0x114, 0x87, 0);
        }
        if (g_cload_scroll_y != 0)
        {
            var_s0 = cload_emit_scroll_arrow(var_s0, var_s5, 0x114, 0x4A, 1);
        }
    }

    if (frame->unk40B2 != 0)
    {
        func_8001C56C(sp20, 0, 0xF0, 0x140, 0xE0);
    }
    else
    {
        func_8001C56C(sp20, 0, 8, 0x140, 0xE0);
    }

    var_s3 = (volatile u32 *)&g_cload_element_pool;
    sp80 = 0;

    for (; sp80 < 8; sp80++, var_s3 += 3)
    {
        temp_a3 = *var_s3;
        if (temp_a3 & 7)
        {
            func_8001A5D4((s32)var_s0, sp20);

            var_s0->unk0 = (var_s0->unk0 & 0xFF000000) | (var_s5->unk0 & 0x00FFFFFF);
            var_s5->unk0 = (s32)((var_s5->unk0 & 0xFF000000) | ((s32)var_s0 & 0x00FFFFFF));

            temp_a0_2 = *var_s3;
            temp_v1_2 = temp_a0_2 & 7;

            do
            {
                var_s0 = (CloadGpuPacket *)((u8 *)var_s0 + 0x40);
            } while (0);

            switch (temp_v1_2)
            {
            case 1:
                temp_v0_3 = *var_s3;
                temp_a1 = *(u32 *)((u8 *)var_s3 + 4);
                temp_a2 = (temp_v0_3 >> 24) | ((temp_a1 & 1) << 8);
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

                var_s0 = (*(CloadElementDrawFunc *)((u8 *)var_s3 + 8))(var_s5, var_s0, (s32)(temp_a2 - temp_s1) / 2, temp_a3_3 / 2);
                {
                    u32 post_word;
                    u32 field;
                    u32 high;
                    post_word = *var_s3;
                    field = (post_word >> 7) & 0x1FF;
                    high = post_word >> 24;
                    var_s0 = cload_emit_window_frame(var_s0, var_s5,
                                           field + (s32)((((*(u32 *)((u8 *)var_s3 + 4) & 1) << 8) | high) - temp_s1) / 2,
                                           (*((u8 *)var_s3 + 2)) + ((s32)((*(u32 *)((u8 *)var_s3 + 4) >> 1) & 0xFF) - temp_s2) / 2,
                                           temp_s1, temp_s2, frame->unk40B2, (*(u32 *)((u8 *)var_s3 + 4) >> 9) & 1);
                }
                do
                {
                    temp_a0_4 = *var_s3;
                    temp_a0_4 = (temp_a0_4 & ~0x78) | (((((temp_a0_4 >> 3) & 0xF) + 1) & 0xF) * 8);
                    *var_s3 = temp_a0_4;
                } while (0);
                if (((temp_a0_4 >> 3) & 0xF) == 8)
                {
                    func_800AA02C();
                    *var_s3 = (*var_s3 & ~7) | 2;
                }
                break;

            case 2:
                var_s0 = (*(CloadElementDrawFunc *)((u8 *)var_s3 + 8))(var_s5, var_s0, 0, 0);
                {
                    u32 case_word;
                    u32 high;
                    case_word = *var_s3;
                    high = case_word >> 24;
                    var_s0 = cload_emit_window_frame(var_s0, var_s5, (case_word >> 7) & 0x1FF, (*((u8 *)var_s3 + 2)),
                                           ((*(u32 *)((u8 *)var_s3 + 4) & 1) << 8) | high,
                                           (*(u32 *)((u8 *)var_s3 + 4) >> 1) & 0xFF, frame->unk40B2,
                                           (*(u32 *)((u8 *)var_s3 + 4) >> 9) & 1);
                }
                temp_v1_3 = *var_s3;
                if (((temp_v1_3 >> 3) & 0xF) != 0)
                {
                    *var_s3 = (temp_v1_3 & ~0x78) | (((((temp_v1_3 >> 3) & 0xF) - 1) & 0xF) * 8);
                }
                break;

            case 3:
                temp_v0_5 = *var_s3;
                temp_a1 = *(u32 *)((u8 *)var_s3 + 4);
                temp_a2 = (temp_v0_5 >> 24) | ((temp_a1 & 1) << 8);
                temp_a0_5 = temp_v0_5 >> 3;
                temp_a0_5 = temp_a0_5 & 0xF;
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

                var_s0 = (*(CloadElementDrawFunc *)((u8 *)var_s3 + 8))(var_s5, var_s0, (s32)(temp_a2 - temp_s1) / 2, temp_a3_6 / 2);
                {
                    u32 post_word;
                    u32 field;
                    u32 high;
                    post_word = *var_s3;
                    field = (post_word >> 7) & 0x1FF;
                    high = post_word >> 24;
                    var_s0 = cload_emit_window_frame(var_s0, var_s5,
                                           field + (s32)((((*(u32 *)((u8 *)var_s3 + 4) & 1) << 8) | high) - temp_s1) / 2,
                                           (*((u8 *)var_s3 + 2)) + ((s32)((*(u32 *)((u8 *)var_s3 + 4) >> 1) & 0xFF) - temp_s2) / 2,
                                           temp_s1, temp_s2, frame->unk40B2, (*(u32 *)((u8 *)var_s3 + 4) >> 9) & 1);
                }
                temp_a0_4 = *var_s3;
                temp_a0_4 = (temp_a0_4 & ~0x78) | (((((temp_a0_4 >> 3) & 0xF) - 1) & 0xF) * 8);
                *var_s3 = temp_a0_4;
                if (!((temp_a0_4 >> 3) & 0xF))
                {
                    *var_s3 = (((temp_a0_4 & ~0x78) | 0x18) & ~7) | 4;
                }
                break;

            case 4:
                temp_v0_5 = *var_s3;
                g_pad_input = 0;
                temp_v1_3 = (temp_v0_5 & ~0x78) | (((((temp_v0_5 >> 3) & 0xF) - 1) & 0xF) * 8);
                *var_s3 = temp_v1_3;
                if (!((temp_v1_3 >> 3) & 0xF))
                {
                    *var_s3 = temp_v1_3 & ~7;
                }
                break;
            }
        }
    }

    frame->unk40B8 = cload_emit_icon_highlight_strip(var_s0, var_s5);
}

/**
 * @brief Append one encoded CLOAD string to another.
 * @param dest Destination text buffer.
 * @param src Source text buffer.
 * @see decomp.me (100.00%)
 */
void cload_text_append(u8 *dest, u8 *src)
{
    s32 dst_len;
    s32 src_len;
    s32 i;

    dst_len = cload_text_byte_length(dest);
    src_len = cload_text_byte_length(src);
    for (i = 0; i < src_len; i++)
    {
        dest[dst_len + i] = src[i];
    }
    dest[dst_len + i] = 0;
}

/**
 * @brief Measure an encoded CLOAD string in bytes.
 * @param text Encoded text buffer.
 * @return Encoded byte length excluding the terminator.
 * @see decomp.me (100.00%)
 */
s32 cload_text_byte_length(u8 *text)
{
    u8 *p;
    u8 c;
    s32 len;

    p = text;
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
 * @brief Copy one encoded CLOAD string including its terminator.
 * @param dest Destination text buffer.
 * @param src Source text buffer.
 * @see decomp.me (100.00%)
 */
void cload_text_copy(u8 *dest, u8 *src)
{
    volatile u8 *p;
    s32 len;
    s32 i;

    p = (volatile u8 *)src;
    len = 0;
    while (*p != 0)
    {
        if ((u32)(*p - 0x19) < 7)
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
        dest[i] = src[i];
    }
    dest[i] = 0;
}

CloadGpuPacket *cload_emit_rect_outline();

/**
 * @brief Emit the GPU packets for a CLOAD window frame.
 * @param prim Primitive-buffer cursor.
 * @param ot Ordering-table head.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param w Width.
 * @param h Height.
 * @param flag Mode flag.
 * @param draw_fill Nonzero to emit the filled frame packets.
 * @return Advanced GPU-packet cursor.
 * @note The `do { ... } while (0)` around the three cload_emit_rect_outline calls is
 *       required to match; plain braces do not substitute.
 * @see decomp.me (100.00%)
 */
CloadGpuPacket *cload_emit_window_frame(CloadGpuPacket *prim, s32 *ot, s32 x, s32 y, s32 w, s32 h, s32 flag, s32 draw_fill)
{
    CloadGpuPacket *var_s0;
    CloadGpuPacket *var_a0;
    CloadGpuPacket *buf;
    CloadGpuPacket *result;
    s32 sp20[24];
    s32 temp_a2;

    buf = prim;
    if (flag != 0)
    {
        temp_a2 = y + 0xF2;
        func_8001C56C(sp20, x + 2, temp_a2, w - 4, h - 3);
    }
    else
    {
        temp_a2 = y + 0xA;
        func_8001C56C(sp20, x + 2, temp_a2, w - 4, h - 3);
    }
    func_8001A5D4((s32)buf, sp20);

    buf->unk0 = (buf->unk0 & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & 0xFF000000) | ((s32)buf & 0xFFFFFF);

    result = (CloadGpuPacket *)((u8 *)buf + 0x40);
    if (draw_fill != 0)
    {
        do
        {
            var_s0 = cload_emit_rect_outline(result, ot, x, y, w, h, 0xFFFFFF);
            var_s0 = cload_emit_rect_outline(var_s0, ot, x + 1, y + 1, w - 2, h - 2, 0);
            var_s0 = cload_emit_rect_outline(var_s0, ot, x - 1, y - 1, w + 2, h + 2, 0);
        } while (0);

        result = var_s0;
        result->unk4 = 0x808080;
        ((u8 *)result)[3] = 3;
        ((u8 *)result)[7] = 0x62;
        result->unk8 = x;
        result->unkA = y;
        result->unkC = w;
        result->unkE = h;
        result->unk0 = (result->unk0 & 0xFF000000) | (*ot & 0xFFFFFF);
        *ot = (*ot & 0xFF000000) | ((s32)result & 0xFFFFFF);

        var_a0 = (CloadGpuPacket *)((u8 *)result + 0x10);
        ((u8 *)var_a0)[3] = 1;
        var_a0->unk4 = 0xE1000045;
        var_a0->unk0 = (var_a0->unk0 & 0xFF000000) | (*ot & 0xFFFFFF);
        *ot = (*ot & 0xFF000000) | ((s32)var_a0 & 0xFFFFFF);
        result = (CloadGpuPacket *)((u8 *)var_a0 + 8);
    }
    return result;
}

/**
 * @brief Emit four line packets forming a rectangle outline.
 * @param p Packet or text pointer.
 * @param ot Ordering-table head.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param w Width.
 * @param h Height.
 * @param color Packed RGB color.
 * @return Advanced GPU-packet cursor.
 * @note `tmp` is deliberately reused for the packet-1 OT-link mask and then for
 *       the bottom edge's y coordinate. Both the mask binding and the reuse are
 *       required to match: with one basic block this function is allocated by
 *       local-alloc, and the reassignment truncates the constant's live range
 *       so it wins the lower temporary register ([ALLOC-23] style ref bumps do
 *       nothing here). Separate variables score 98.74%.
 * @see decomp.me (100.00%)
 */
CloadGpuPacket *cload_emit_rect_outline(CloadGpuPacket *p, s32 *ot, s32 x, s32 y, s32 w, s32 h, s32 color)
{
    s32 tmp;

    p->unk4 = color;
    ((u8 *)p)[3] = 3;
    ((u8 *)p)[7] = 0x40;
    p->unk8 = x;
    p->unkA = y;
    p->unkC = x + w;
    p->unkE = y;
    tmp = 0xFF000000;
    p->unk0 = (p->unk0 & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & tmp) | ((s32)p & 0xFFFFFF);
    p++;

    p->unk4 = color;
    ((u8 *)p)[3] = 3;
    ((u8 *)p)[7] = 0x40;
    p->unk8 = x + w;
    p->unkA = y;
    p->unkC = x + w;
    p->unkE = y + h;
    p->unk0 = (p->unk0 & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & 0xFF000000) | ((s32)p & 0xFFFFFF);
    p++;

    p->unk4 = color;
    ((u8 *)p)[3] = 3;
    ((u8 *)p)[7] = 0x40;
    p->unk8 = x + w;
    tmp = y + h;
    p->unkA = tmp;
    p->unkC = x;
    p->unkE = y + h;
    p->unk0 = (p->unk0 & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & 0xFF000000) | ((s32)p & 0xFFFFFF);
    p++;

    p->unk4 = color;
    ((u8 *)p)[3] = 3;
    ((u8 *)p)[7] = 0x40;
    p->unk8 = x;
    p->unkA = y;
    p->unkC = x;
    p->unkE = y + h;
    p->unk0 = (p->unk0 & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & 0xFF000000) | ((s32)p & 0xFFFFFF);
    return p + 1;
}

/**
 * @brief Ordering-table link word at the head of a GPU packet; mirrors the
 *        P_TAG layout in include/psyq/libgpu.h.
 * @note Writing the link through the 24-bit @c addr bitfield (setaddr) makes gcc
 *       build the two mask constants in the order the original used; the
 *       hand-written @c (x & 0xFF000000) | (y & 0xFFFFFF) form colours them the
 *       other way round.
 */
typedef struct
{
    /* 0x0 */ u32 addr : 24; /* next-primitive address (24-bit) */
    /* 0x3 */ u32 len : 8;   /* packet word count */
} CloadOtTag;

/**
 * @brief Emit a textured scroll-arrow sprite and draw-mode packet.
 * @param p Packet or text pointer.
 * @param ot Ordering-table head.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param flag Mode flag.
 * @return Advanced GPU-packet cursor.
 * @see decomp.me (100.00%)
 */
CloadGpuPacket *cload_emit_scroll_arrow(CloadGpuPacket *p, s32 *ot, s32 x, s32 y, s32 flag)
{
    p->unk4 = 0x808080;
    do
    {
        ((u8 *)p)[3] = 4;
        ((u8 *)p)[7] = 0x64;
        p->unk8 = x;
        p->unkA = y;
    } while (0);
    if (flag != 0)
    {
        ((u8 *)p)[0xD] = 0;
    }
    else
    {
        ((u8 *)p)[0xD] = 0x10;
    }
    ((u8 *)p)[0xC] = 0x30;
    *(s16 *)((u8 *)p + 0x10) = 0x10;
    *(s16 *)((u8 *)p + 0x12) = 0x10;
    p->unkE = 0x7D80;
    ((CloadOtTag *)p)->addr = ((CloadOtTag *)ot)->addr;
    ((CloadOtTag *)ot)->addr = (u32)p;

    p = (CloadGpuPacket *)((u8 *)p + 0x14);
    ((u8 *)p)[3] = 1;
    p->unk4 = 0xE100008A;
    ((CloadOtTag *)p)->addr = ((CloadOtTag *)ot)->addr;
    ((CloadOtTag *)ot)->addr = (u32)p;
    return (CloadGpuPacket *)((u8 *)p + 8);
}

/**
 * @brief Draw the CD-load prompt glyph then set up the driver/GPU-packet state,
 *        branching on the CD status (cload_poll_and_rewind_primary_handles) and the g_pad_input flags.
 * @param ot ordering-table head threaded through the glyph/line draws.
 * @param prim primitive buffer for the glyph draw (func_800A88A0).
 * @param x_offset base for the row y-coordinate (-x_offset + 0x90).
 * @param y_offset row delta applied to the draw extents.
 * @return the CloadGpuPacket* chain pointer returned by cload_draw_choice_prompt.
 * @note WIP 96.72% (121/125 exact). Required-to-match shapes in place:
 *       FRAME-04 dead 9-arg call for the -0x38 outgoing-arg frame; ONE-EXIT
 *       `goto ret` single return so the `v0 = result` copy is shared; block-C
 *       laid out before block-B (target reaches C by branch, falls into B);
 *       g_cload_element_pool accessed dually (CloadElementPoolHead `.unk0` direct via %hi/%lo +
 *       CloadElement* base for unk4/unk8/byte2); `tmp = 0xFFFF007F` bound before
 *       `p->unk8` (born before the base pointer, wins its register); the unk4
 *       update split as `tmp = (unk4 | 1) & ~0x1FE; unk4 = tmp | 0x56`.
 *       Residual (4 rows): the split unk4 accumulator lands in a0 where the
 *       target keeps it in v1, plus a 1-slot schedule shift on the
 *       -0x1FF/unk4-load pair. Coupled block-B coloring; permuter best 130.
 * @see decomp.me (96.72%)
 */
s32 cload_draw_load_prompt(s32 ot, s32 prim, s32 x_offset, s32 y_offset)
{
    s32 result;
    s32 y;

    if (0)
    {
        cload_draw_choice_prompt(0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
    y = -x_offset + 0x90;
    result = cload_draw_choice_prompt(
        func_800A88A0(prim, (s32 *)ot, (void *)((s32)&D_80145ECC - 0x30 + D_80145ECC), 4, y, -y_offset, 2),
        ot, y, 0xE - y_offset);
    if ((u32)(cload_poll_and_rewind_primary_handles() - 1) < 2)
    {
        g_cload_element_pool.first_state = g_cload_element_pool.first_state & ~7;
        func_800AA02C();
        func_800A3938(0x78, 0x80);
        g_cload_entry_state = 0xFF;
        cload_reset_entry_ranks();
        g_cload_load_step = NULL;
        goto ret;
    }
    if (g_pad_input & 0x40)
    {
        goto block_c;
    }
    if (!(g_pad_input & 0x220))
    {
        goto ret;
    }
    if (g_cload_choice_toggle == 0)
    {
        goto block_b;
    }
block_c:
    g_cload_element_pool.first_state = g_cload_element_pool.first_state & ~7;
    func_800AA02C();
    func_800A3938(0x78, 0x80);
    g_cload_load_step = D_80146534;
    goto ret;
block_b:
    {
        CloadElement *p = (CloadElement *)&g_cload_element_pool;
        s32 tmp;

        func_800A3938(0x7E, 0x80);
        g_cload_progress_active = 1;
        g_cload_load_step = D_8014653C;
        tmp = 0xFFFF007F;
        p->draw = cload_draw_load_progress;
        g_cload_element_pool.first_state = (((((g_cload_element_pool.first_state & ~0x78) | 8) & ~7) | 1) & tmp) | 0x800;
        ((u8 *)p)[2] = 0x5B;
        tmp = (p->size_flags | 1) & ~0x1FE;
        p->size_flags = tmp | 0x56;
        g_cload_element_pool.first_state = (g_cload_element_pool.first_state & 0xFFFFFF) | 0x20000000;
    }
ret:
    return result;
}

/**
 * @brief Draw load progress and commit a validated save payload when ready.
 * @param ot Ordering-table head.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 * @see decomp.me (100.00%)
 */
s32 cload_draw_load_progress(s32 ot, s32 prim, s32 x_offset, s32 y_offset)
{
    s32 y;
    s32 result;
    u8 *base;
    u8 *p;
    s32 vol;

    if (0)
    {
        cload_draw_progress_bar(0, 0, 0, 0, 0, 0, 0, 0, 0);
    }

    y = -x_offset + 0x90;
    result = func_800A88A0(prim, (s32 *)ot, (void *)((s32)&D_80145ECE - 0x32 + D_80145ECE), 4, y, -y_offset, 2);
    base = (u8 *)&D_80145ECE - 0x32;
    result = func_800A88A0(result, (s32 *)ot, GLYPH_OFF(base, 0x1E), 4, y, 0xE - y_offset, 2);
    result = func_800A88A0(result, (s32 *)ot, GLYPH_OFF(base, 0xB2), 4, y, 0x1C - y_offset, 2);
    result = cload_draw_progress_bar(result, ot);

    if (g_cload_progress_active != 0)
    {
        goto ret;
    }

    p = g_cload_save_blob;
    vol = 0x80;
    base = p;
    if (cload_validate_save_blob(base) == 0)
    {
        cload_open_status_dialog(4);
        goto ret;
    }

    func_800A3938(0x7B, vol);
    g_cload_element_pool.first_state = g_cload_element_pool.first_state & ~7;
    func_80016E7C(base + 0x180, D_80042FD8, 0x3268);
    D_8003EC9C = D_80042FD8[0xCF];
    D_80042FB4 = func_8002054C(-1);
    g_cload_exit_requested = 1;
ret:
    return result;
}

/**
 * @brief Same word/half raw-store granularity as POLY_G4 in
 *        include/psyq/libgpu.h; the r/g/b/code quad and each x/y pair are
 *        written as single word/half stores (matching the target's
 *        codegen), not per-channel byte assignments.
 */
typedef struct
{
    /* 0x0 */ s32 unk0;
    /* 0x4 */ s32 unk4;
    /* 0x8 */ s16 unk8;
    /* 0xA */ s16 unkA;
    /* 0xC */ s32 unkC;
    /* 0x10 */ s16 unk10;
    /* 0x12 */ s16 unk12;
    /* 0x14 */ s32 unk14;
    /* 0x18 */ s16 unk18;
    /* 0x1A */ s16 unk1A;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s16 unk20;
    /* 0x22 */ s16 unk22;
} CloadPolyG4Words;

/**
 * @brief Build the CD-load progress-bar gouraud quad (a POLY_G4-shaped
 *        packet) and link it into the ordering table.
 * @param p packet cursor to write the quad into.
 * @param ot ordering-table head threaded through the OT-link update.
 * @return the advanced packet cursor (p + 0x24), or p unchanged if
 *         g_cload_progress_bar_active is 0.
 * @note WIP 94.26% (60/68 exact). g_cload_progress_bar_active gates the whole body; when set,
 *       `elapsed` (VSync(-1) - g_cload_progress_start_tick, clamped to 0x100) scales into the
 *       quad's right-edge x extent (elapsed * 0x120, rounded via +0xFF for
 *       negative values, then >>8) - a time-based progress bar. Required-to-
 *       match shape: the two `>>8` stores are written inline
 *       (`g->unk20 = extent >> 8; g->unk10 = extent >> 8;`), NOT through a
 *       reassigned `extent = extent >> 8;` local - the reassignment costs
 *       -7 exact rows (this single change was the jump from 82.28% to
 *       94.26%, and incidentally fixed a second, separate-looking mismatch
 *       around the OT-link mask too).
 *       Residual (8 rows, SCHED-LUID): the target fills the elapsed-clamp
 *       branch's delay slot with the start of the `g->unk14 = 0xFFFF00`
 *       constant build (`lui`); this compile fills it by speculatively
 *       re-running the `x * 9` shift with the pre-clamp value and redoing it
 *       after the join (+1 insn). Named-local hoists of the 0xFFFF00/0xFFFFFF
 *       constants to various earlier points all measured neutral or worse;
 *       permuter (v2, ~16k iters) got to score 245 (from 575) via the same
 *       inline->8 fix already applied here and did not find the remaining
 *       delay-slot swap.
 * @see decomp.me (94.26%)
 */
CloadGpuPacket *cload_draw_progress_bar(CloadGpuPacket *p, s32 *ot)
{
    CloadPolyG4Words *g;
    s32 elapsed;
    s32 extent;

    g = (CloadPolyG4Words *)p;
    if (g_cload_progress_bar_active != 0)
    {
        elapsed = func_8002054C(-1) - g_cload_progress_start_tick;
        if (elapsed >= 0x101)
        {
            elapsed = 0x100;
        }
        extent = elapsed * 0x120;
        g->unk4 = 0xFF;
        g->unkC = 0xFFFF;
        g->unk1C = 0xFF0000;
        ((u8 *)g)[3] = 8;
        g->unk14 = 0xFFFF00;
        ((u8 *)g)[7] = 0x38;
        g->unk18 = 0;
        g->unk8 = 0;
        if (extent < 0)
        {
            extent += 0xFF;
        }
        g->unk20 = extent >> 8;
        g->unk10 = extent >> 8;
        g->unk12 = 0;
        g->unkA = 0;
        g->unk22 = 0x35;
        g->unk1A = 0x35;
        p->unk0 = (p->unk0 & 0xFF000000) | (*ot & 0xFFFFFF);
        *ot = (*ot & 0xFF000000) | ((s32)p & 0xFFFFFF);
        p = (CloadGpuPacket *)((u8 *)p + 0x24);
    }
    return p;
}

/**
 * @brief Open a status dialog for the requested dialog state.
 * @param dialog_state Dialog state to open.
 * @see decomp.me (100.00%)
 */
void cload_open_status_dialog(s32 dialog_state)
{
    CloadElement *p;
    s32 state;

    func_800A3938(0x78, 0x80);
    g_cload_element_pool.first_state = ((((g_cload_element_pool.first_state & ~0x78) | 8) & ~7 | 1) & 0xFFFF007F | 0x1000) & 0xFFFFFF;
    p = (CloadElement *)&g_cload_element_pool;
    p->size_flags |= 1;
    if (dialog_state < 2 || dialog_state == 4)
    {
        p->size_flags = (p->size_flags & ~0x1FE) | 0x3E;
        state = 0x60;
    }
    else
    {
        p->size_flags = (p->size_flags & ~0x1FE) | 0x1E;
        state = 0x70;
    }
    ((u8 *)p)[2] = state;
    p->draw = cload_draw_status_dialog;
    func_800AA02C();
    D_80162370 = 0;
    g_cload_progress_active = 0;
    g_cload_selection_status = 0;
    g_cload_io_busy = 0;
    g_cload_entry_state = 0xFF;
    cload_reset_entry_ranks();
    g_cload_load_step = 0;
    g_cload_dialog_state = dialog_state;
}

/**
 * @brief Draw the active CLOAD status dialog.
 * @param ot Ordering-table head.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 * @see decomp.me (99.93%)
 */
s32 cload_draw_status_dialog(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    s32 state = g_cload_dialog_state;
    u8 *base;

    if (0)
    {
        cload_draw_choice_prompt(0, 0, 0, 0, 0, 0, 0, 0, 0);
    }

    switch (state)
    {
    case 0:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80145ED8, 0x3C), 4, -x_offset + 0x80, -y_offset, 2);
        base = (u8 *)&D_80145ED8 - 0x3C;
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x56), 4, -x_offset + 0x80, -y_offset + 0x10, 2);
        break;
    case 1:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80145EDA, 0x3E), 4, -x_offset + 0x80, -y_offset, 2);
        base = (u8 *)&D_80145EDA - 0x3E;
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x56), 4, -x_offset + 0x80, -y_offset + 0x10, 2);
        break;
    case 2:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80145EDC, 0x40), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 3:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80145EDE, 0x42), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 4:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80145EDA, 0x3E), 4, -x_offset + 0x80, -y_offset, 2);
        base = (u8 *)&D_80145EDA - 0x3E;
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x5C), 4, -x_offset + 0x80, -y_offset + 0x10, 2);
        break;
    }

    if (g_pad_input & 0x220)
    {
        g_cload_element_pool.first_state &= ~7;
        func_800AA02C();
    }

    return prim;
}

typedef struct
{
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ s16 unk8;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ u8 unkC;
    /* 0x0D */ u8 unkD;
    /* 0x0E */ s16 unkE;
    /* 0x10 */ s16 unk10;
    /* 0x12 */ s16 unk12;
    /* 0x14 */ u8 unk14;
    /* 0x15 */ u8 unk15;
    /* 0x16 */ s16 unk16;
    /* 0x18 */ s16 unk18;
    /* 0x1A */ s16 unk1A;
    /* 0x1C */ u8 unk1C;
    /* 0x1D */ u8 unk1D;
    /* 0x1E */ s16 unk1E;
    /* 0x20 */ s16 unk20;
    /* 0x22 */ s16 unk22;
    /* 0x24 */ u8 unk24;
    /* 0x25 */ u8 unk25;
} StructFT4;

/**
 * @brief Draw one icon-highlight row: set the text scissor window and build a
 *        textured (POLY_FT4) quad for the icon at slot @p index, linked into
 *        the ordering table.
 * @param prim primitive cursor the FT4 quad is written into.
 * @param ot ordering-table head threaded through the OT-link update.
 * @param x quad left x (unk8/unk18).
 * @param y quad top y (unkA/unk12).
 * @param highlight x offset added for the quad's right edge (unk10/unk20).
 * @param icon icon id; 0x7F skips the whole draw, <2 / <0x4F / >=0x4F pick the
 *             scissor source (func_800A5638 vs the g_cload_icon_resource table vs
 *             func_800A55E4 with g_cload_icon_palette).
 * @param index slot index; drives the scissor x and the quad UV base (index*3).
 * @param row when 1 and icon<2 uses the func_800A5638 scissor path.
 * @return the advanced primitive cursor (prim + 0x28), or prim unchanged when
 *         icon == 0x7F.
 * @note WIP 85.14% (122/145 exact). Structure, control flow, the -0x40 frame,
 *       prim in s2 (via `base = index * 3` kept as a single post-branch LOCAL so
 *       local-alloc gives it s0 and the long-lived global prim keeps s2), and
 *       the CloadOtTag setaddr link are all in place. Residue is packet-block
 *       scheduling in the final basic block: the target hoists the `highlight`
 *       (sp+0x50) load and the `prim->unk0` OT-read early to hide latency, which
 *       pushes the `uv = base * 0x10` shift later, whereas this compile emits the
 *       uv block right after the code bytes. sched_oracle reports the block as
 *       NON_LUID / regalloc-decided (not a clean sched1 emit-order fix), and the
 *       permuter (v2) only found scaffolding (a stray pointer split plus a junk
 *       `(double)` cast). Also 2 minor argdiff rows: the `row` compare temp
 *       lands a0 vs target v1, and the func_800A55E4 path loads g_cload_icon_palette after
 *       &g_cload_icon_context (target loads it first; a statement-precompute was inert via
 *       copy-propagation). working/cload_draw_icon_highlight kept for a future 100% pass.
 * @see decomp.me (85.14%)
 */
s32 cload_draw_icon_highlight(s32 prim, s32 *ot, s32 x, s32 y, s32 highlight, s32 icon, s32 index, s32 row)
{
    RECT rect;
    s32 base;
    u8 uv;
    u8 uv2;
    s16 tmp;
    s32 v;
    s32 *ctx;

    if (icon == 0x7F)
    {
        return prim;
    }

    rect.x = index * 0x10;
    rect.y = 0x1F2;
    rect.w = 0x10;
    rect.h = 1;
    v = icon < 0x4F;
    if (row == 1)
    {
        if (icon < 2)
        {
            ctx = &g_cload_icon_context;
            func_800A5638(ctx, icon);
            goto block_8;
        }
        v = icon < 0x4F;
    }
    if (v == 0)
    {
        ctx = &g_cload_icon_context;
        func_800A55E4(ctx, g_cload_icon_palette);
    block_8:
        func_80019A34(&rect, ctx);
        func_80019788(0);
    }
    else
    {
        func_80019A34(&rect, g_cload_icon_resource + *(s32 *)(g_cload_icon_resource + icon * 4 + 4));
    }

    base = index * 3;
    rect.x = (base * 4) + 0x140;
    rect.w = 0xC;
    rect.h = 0x30;
    rect.y = 0xD0;
    func_80019A34(&rect, g_cload_icon_resource + *(s32 *)(g_cload_icon_resource + icon * 4 + 4) + 0x20);

    ((StructFT4 *)prim)->unk4 = 0x808080;
    ((u8 *)prim)[3] = 9;
    ((u8 *)prim)[7] = 0x2C;
    ((StructFT4 *)prim)->unk18 = x;
    ((StructFT4 *)prim)->unk8 = x;
    ((StructFT4 *)prim)->unk12 = y;
    ((StructFT4 *)prim)->unkA = y;
    uv = base * 0x10;
    ((StructFT4 *)prim)->unk1C = uv;
    ((StructFT4 *)prim)->unkC = uv;
    uv2 = uv + 0x2F;
    ((StructFT4 *)prim)->unk24 = uv2;
    ((StructFT4 *)prim)->unk14 = uv2;
    ((StructFT4 *)prim)->unk15 = 0xD0;
    ((StructFT4 *)prim)->unkD = 0xD0;
    tmp = x + highlight;
    ((StructFT4 *)prim)->unk20 = tmp;
    ((StructFT4 *)prim)->unk10 = tmp;
    tmp = y + 0x2F;
    ((StructFT4 *)prim)->unk22 = tmp;
    ((StructFT4 *)prim)->unk1A = tmp;
    ((StructFT4 *)prim)->unk25 = 0xFF;
    ((StructFT4 *)prim)->unk1D = 0xFF;
    ((StructFT4 *)prim)->unkE = (index & 0x3F) | 0x7C80;
    ((StructFT4 *)prim)->unk16 = 5;
    ((CloadOtTag *)prim)->addr = ((CloadOtTag *)ot)->addr;
    ((CloadOtTag *)ot)->addr = (u32)prim;

    return prim + 0x28;
}

/**
 * @brief Deactivate the first UI element.
 * @see decomp.me (100.00%)
 */
void cload_deactivate_primary_element(void)
{
    g_cload_element_pool.first_state = g_cload_element_pool.first_state & ~7;
}

/**
 * @brief Load the CD icon-set resource into the 0x80180000 scratch buffer and
 *        register its three icon glyphs (codes 0x200/0x240/0x280) with ids
 *        0x1F4/0x1F5/0x1F6 via func_80086374, also pointing g_cload_icon_resource (the
 *        icon UV table used by cload_draw_icon_highlight) at the resource's first offset
 *        field.
 * @note WIP 99.50% (45/50 exact, correct frame/insn count, no structural
 *       rows). The resource header at 0x80180000 stores four s32 offset
 *       fields at +4/+8/+0xC/+0x10; the target reads them via a SINGLE
 *       `lui $s0,%hi(D_80180008)` reused (as a raw 0x80180000 constant) for
 *       all four %lo(D_80180004/8/C/10) loads AND the "+ base" pointer
 *       conversions, with zero extra instructions. Every measured source
 *       shape that gives the four fields their own extern symbol (plain
 *       scalars: 72.76%, 4 separate luis; one struct/array anchored at the
 *       base: 82.68%, gcc instead MATERIALIZES a full address via an extra
 *       `addiu` + saved register since the fields are read across 3
 *       intervening calls; `&sym +/- k` anchor tricks: 75.82-88.64%, same
 *       materialization cost) costs extra insns and regresses the match.
 *       Raw pointer arithmetic on the literal `(u8*)0x80180000` (this file)
 *       reproduces the target's exact instruction count and frame with the
 *       four loads showing as plain register+offset instead of named
 *       relocations - that is the whole gap. Permuter (v2, ~29.5k
 *       iterations) converged on this same state and found nothing further.
 *       Same residual class as func_800A3728 in field_audio.c (98.97%, also
 *       left unmatched after 22 measured-inert variants) - gcc sharing one
 *       %hi() across several extern data symbols in a page is not reachable
 *       from any C shape tried so far. working/cload_load_icon_resources kept for a
 *       future pass.
 * @see decomp.me (99.50%)
 */
void cload_load_icon_resources(void)
{
    u8 *base;
    s16 buf[4];

    func_800141EC(0x5E4, (void *)0x80180000);
    func_80013F2C();

    base = (u8 *)0x80180000;

    g_cload_icon_resource = *(s32 *)(base + 4) + (s32)base;

    buf[0] = 0x200;
    buf[1] = 0;
    buf[2] = 0;
    buf[3] = 0x1F4;
    func_80086374(buf, *(s32 *)(base + 8) + (s32)base, 1);

    buf[0] = 0x240;
    buf[1] = 0;
    buf[2] = 0;
    buf[3] = 0x1F5;
    func_80086374(buf, *(s32 *)(base + 0xC) + (s32)base, 1);

    buf[0] = 0x280;
    buf[1] = 0;
    buf[2] = 0;
    buf[3] = 0x1F6;
    func_80086374(buf, *(s32 *)(base + 0x10) + (s32)base, 1);
}

/**
 * @brief Build three OT-linked icon-highlight primitives (one 20-byte
 *        SPRT-shaped color record plus one 8-byte DR_MODE-style texture-page
 *        record per icon), stepping the color/row coordinates and
 *        texture-page code for each of the 3 slots.
 * @param p primitive buffer to write the 3 record pairs into.
 * @param ot ordering-table head threaded through each record's OT link.
 * @return the advanced primitive cursor after all 3 pairs (p + 0x54).
 * @note WIP 91.23% (63/73 exact, correct frame/insn count, no structural or
 *       scheduling rows left). Residual is a single swapped register pair:
 *       the CloadOtTag bitfield's 0xFF000000 len-mask constant and the `code`
 *       (unk8) loop accumulator land in t3/t6 opposite of the target (mask
 *       should be t6, code should be t3). alloc_report confirms this is a
 *       genuine allocator priority tie broken the wrong way: mask
 *       pri=2142 (9 refs/126 live) vs code pri=2028 (7 refs/69 live), and
 *       explain_conflict confirms the two pseudos DO conflict, so it is not
 *       hand-swappable - gcc's own priority formula decides it. Measured
 *       inert: every declaration/init/increment-clause reordering of
 *       code/col/row (byte-identical output), an explicit s16 cast on the
 *       unk8 store, inlining vs naming `tpage`, and merging row/col into one
 *       accumulator via their constant 0x7B00 offset (both merge directions
 *       cost 12 exact rows - they are genuinely separate variables).
 *       Permuter (v2, 100k+ iterations across 3 re-seeds) never beat the
 *       seeded state. Shapes that ARE measured required: `col += 0x40,
 *       row += 0x40, code += 0x80, i++` in that exact order as the for-loop's
 *       increment clause (i last); `tpage` computed AFTER the first CloadOtTag
 *       link and p-advance, not before. working/cload_emit_icon_highlight_strip kept for a
 *       future pass.
 * @see decomp.me (91.23%)
 */
CloadGpuPacket *cload_emit_icon_highlight_strip(CloadGpuPacket *p, CloadOrderingTable *ot)
{
    s32 i;
    s32 code;
    s32 col;
    s32 row;
    s32 tpage;

    code = 8;
    col = 0x200;
    row = 0x7D00;
    for (i = 0; i < 3; col += 0x40, row += 0x40, code += 0x80, i++)
    {
        p->unk4 = 0x808080;
        ((u8 *)p)[3] = 4;
        ((u8 *)p)[7] = 0x64;
        p->unk8 = code;
        p->unkA = 0;
        ((u8 *)p)[0xC] = 0;
        ((u8 *)p)[0xD] = 0;
        if (i == 2)
        {
            *(s16 *)((u8 *)p + 0x10) = 0x30;
        }
        else
        {
            *(s16 *)((u8 *)p + 0x10) = 0x80;
        }
        *(s16 *)((u8 *)p + 0x12) = 0xE0;
        p->unkE = row;
        ((CloadOtTag *)p)->addr = ((CloadOtTag *)ot)->addr;
        ((CloadOtTag *)ot)->addr = (u32)p;
        p = (CloadGpuPacket *)((u8 *)p + 0x14);
        ((u8 *)p)[3] = 1;
        tpage = ((col & 0x3FF) >> 6) | 0xE1000080;
        p->unk4 = tpage;
        ((CloadOtTag *)p)->addr = ((CloadOtTag *)ot)->addr;
        ((CloadOtTag *)ot)->addr = (u32)p;
        p = (CloadGpuPacket *)((u8 *)p + 8);
    }
    return p;
}

/**
 * @brief Initialize the two-choice prompt selection state.
 * @return Always 1.
 * @see decomp.me (100.00%)
 */
s32 cload_enable_choice_toggle(void)
{
    g_cload_choice_toggle = 1;
    return 1;
}

/**
 * @brief Emit two glyph prims from the shared text table (base 0x800EC3C4),
 *        toggling the copy/render mode (4 vs 5) on the g_cload_choice_toggle flag.
 * @param prim Current prim pointer; chained through both func_800A88A0 calls and returned.
 * @param ot   Ordering table pointer.
 * @param x Base X used as (x - 0x10) for the first glyph and (x + 8) for the second.
 * @param y Y coordinate passed through to both glyphs.
 * @return Prim pointer after both emits.
 * @note WIP 97.15% (gcc272_cdk). Instruction-exact (74/74), frame -0x40, all sp
 *       slots match. Sole residue: a sched2 floater transposition -- the
 *       `mode = 4` (`addiu a3, 4`) is hoisted one slot above p's `addiu v1`.
 *       sched_oracle reports every emit-order constraint satisfied; whatif shows
 *       the floater order is fixed in the sched1 model, so the diff is a pure
 *       post-sched1 (sched2) reorder, not reachable by source-level emit control.
 * @see decomp.me (97.15%)
 */
s32 cload_draw_choice_prompt(s32 prim, s32 *ot, s32 x, s32 y)
{
    u8 *p;
    u8 *base;
    u8 *glyph;
    s32 mode;

    p = D_800EC3FA;
    base = p - 0x36;
    glyph = (u8 *)(p[0] + ((p[1] << 8) + (s32)base));
    mode = 4;
    if (g_cload_choice_toggle != 0)
    {
        mode = 5;
    }
    prim = func_800A88A0(prim, ot, glyph, mode, x - 0x10, y, 1);

    glyph = (u8 *)(base[0x38] + ((base[0x39] << 8) + (s32)base));
    mode = 4;
    if (g_cload_choice_toggle == 0)
    {
        mode = 5;
    }
    prim = func_800A88A0(prim, ot, glyph, mode, x + 8, y, 0);

    if (g_pad_input & 0xA000)
    {
        g_cload_choice_toggle ^= 1;
        func_800A3938(0x7D, 0x80);
        g_pad_input = 0;
    }
    return prim;
}


/**
 * @brief Validate a loaded 0x33E0-byte blob against its trailing checksum/magic.
 * @param base Pointer to the blob; cload_compute_save_checksum sums bytes 0..0x33DF.
 * @return 1 if the stored checksum at 0x33E0 matches and the magic at 0x33E4
 *         equals 0x414E41, otherwise 0.
 * @note The single shared `return 0` join (reached by both failure paths) keeps
 *       gcc's jump.c from folding the magic compare into a store-flag; the
 *       nested if with literal returns keeps the result in v0 (idiom JUMP-23).
 * @see decomp.me (100.00%)
 */
s32 cload_validate_save_blob(u8 *base)
{
    if (*(s32 *)(base + 0x33E0) == cload_compute_save_checksum(base))
    {
        if (*(s32 *)(base + 0x33E4) == 0x414E41)
        {
            return 1;
        }
    }
    return 0;
}


/**
 * @brief Compute the additive checksum used to validate a loaded 0x33E0-byte blob.
 * @param data Pointer to the blob; bytes 0..0x33DF are summed.
 * @return (sum * 2) + 0x0414E410, the value compared against the stored trailer.
 * @see decomp.me (100.00%)
 */
s32 cload_compute_save_checksum(u8 *data)
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
 * @brief Render the hex nibbles of @p value to ASCII, suppressing leading zeros.
 * @param out Destination buffer; receives the ASCII digits and a terminating 0.
 * @param value Value whose nibbles (most-significant first) are emitted.
 * @param max_chars Maximum number of characters to emit.
 * @note Each nibble is converted by cload_hex_nibble_to_ascii; a leading run of zero nibbles
 *       is skipped until the first non-zero digit is seen.
 * @see decomp.me (98.96%)
 */
void cload_format_hex(s8 *out, s32 value, s32 max_chars)
{
    s32 temp_s0;
    s32 var_s1;
    s32 var_s2;
    s32 var_s5;
    s32 var_v1;
    s8 *var_s3;
    s32 var_s6;

    var_s3 = out;
    var_s5 = value;
    var_s2 = max_chars;
    var_s1 = 7;
    var_v1 = 0;
    if (var_s2 != 0)
    {
        var_s6 = -1;
loop_2:
        temp_s0 = (var_s5 >> (var_s1 * 4)) & 0xF;
        if ((temp_s0 != 0) || (var_v1 != 0))
        {
            cload_hex_nibble_to_ascii(var_s3, temp_s0);
            var_s3 += 1;
            var_s2 -= 1;
            var_v1 = 1;
            var_s5 -= temp_s0 << (var_s1 * 4);
        }
        var_s1 -= 1;
        if (var_s1 != var_s6)
        {
            if (var_s1 == 0)
            {
                var_v1 = 1;
            }
            if (var_s2 != 0)
            {
                goto loop_2;
            }
        }
    }
    *var_s3 = 0;
}


/**
 * @brief Convert a 4-bit value to its ASCII hex digit and store it.
 *
 * Writes '0'-'9' for @p nibble 0-9, 'A'-'F' for 10-15, and '_' (0x5F) for any
 * value >= 16, storing the single character byte at @p out.
 *
 * @param out Destination byte written with the ASCII character.
 * @param nibble Value to convert; expected range 0-15.
 * @see decomp.me (100.00%)
 */
void cload_hex_nibble_to_ascii(s8 *out, s32 nibble)
{
    if (nibble < 0xA)
    {
        *out = nibble + 0x30;
        return;
    }
    if (nibble < 0x10)
    {
        *out = nibble + 0x37;
        return;
    }
    *out = 0x5F;
}


/**
 * @brief Parse an ASCII hex string into a 32-bit value.
 * @param s Pointer to the hex text (0-9, A-F, a-f).
 * @param len Maximum number of characters to consume.
 * @return The accumulated big-endian value of the hex digits read.
 * @see decomp.me (100.00%)
 */
u32 cload_parse_hex(u8 *s, s32 len)
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
 * @brief Skip a leading run of hex characters, then parse up to two hex digits
 *        into an integer value.
 * @param text Pointer to the ASCII text to scan.
 * @return The value of the (at most two) hex digits found after the skipped run.
 * @param unused1 Unused ABI argument.
 * @param unused2 Unused ABI argument.
 * @note The leading skip loop is the same shape as cload_skip_hex_digits; gcc's jump.c
 *       cross-jumps its three continue arms into one back-edge here, so the
 *       per-arm text++/text-- form the target keeps is not reproducible without the
 *       permuter (cload_skip_hex_digits itself is committed nonmatching for the same
 *       reason). The accumulation uses an explicit temp so K is folded with the
 *       running value (result - K) rather than with the freshly read digit.
 * @see decomp.me (87.66%)
 */
s32 cload_parse_hex_suffix_byte(u8 *text, s32 unused1, s32 unused2)
{
    u8 c;
    s32 count;
    s32 result;
    s32 tmp;

    while (1)
    {
        c = *text;
        if ((u32)(c - '0') < 10)
        {
            text++;
            continue;
        }
        if ((u32)(c - 'a') < 6)
        {
            text++;
            continue;
        }
        if ((u32)(c - 'A') < 6)
        {
            text++;
            continue;
        }
        break;
    }
    text++;
    count = 2;
    c = *text;
    result = 0;
    while ((u32)(c - '0') < 10 || (u32)(c - 'a') < 6 || (u32)(c - 'A') < 6)
    {
        if (count == 0)
        {
            break;
        }
        c = *text;
        result <<= 4;
        if ((u32)(c - '0') < 10)
        {
            tmp = result - 0x30;
            result = *text + tmp;
        }
        else if ((u32)(c - 'A') < 6)
        {
            tmp = result - 0x37;
            result = *text + tmp;
        }
        else if ((u32)(c - 'a') < 6)
        {
            tmp = result - 0x57;
            result = *text + tmp;
        }
        text++;
        c = *text;
        count--;
    }
    return result;
}


/**
 * @brief Parse the hex-string field of each table entry and record the results.
 *
 * Iterates over the g_cload_entry_state active entries. For each entry the 0x28-byte
 * record is validated with @ref func_8001714C against pattern D_800ECF7C. On a
 * match (return 0) the ASCII hex string at record offset 0xC is scanned for up
 * to five hex digits (0-9, A-F, a-f), accumulated big-endian into a value that
 * is written to the g_cload_entry_fields result array; @ref cload_parse_hex_suffix_byte is then invoked
 * with the field, the parsed value, and the remaining digit budget, its result
 * stored in the g_cload_entry_suffix_values array and folded into a running maximum. A failed
 * validation stores -1 / 0 into the two arrays instead.
 *
 * @return The largest value returned by any @ref cload_parse_hex_suffix_byte call (0 if none).
 * @note WIP match, 87.74%; residue is a saved-register priority rotation
 *       (max/D_8014A920hi/i and entry/off4) plus parse-loop temp coloring.
 * @see decomp.me (87.74%)
 */
s32 cload_parse_entry_fields(void)
{
    s32 i;
    s32 max;
    u8 *entry;
    u8 *fld;
    u8 *field;
    s32 *out;
    s32 off28;
    s32 off4;
    u8 *p;
    s32 count;
    s32 acc;
    u8 c;
    u8 cls;
    s32 t;
    s32 r;

    max = 0;
    i = 0;
    if (g_cload_entry_state > 0)
    {
        entry = &g_cload_entries;
        fld = (u8 *)&g_cload_entries + 0xC;
        out = &g_cload_entry_suffix_values;
        off28 = max;
        off4 = off28;
        do
        {
            if (func_8001714C(&D_800ECF7C, entry + g_cload_card_slot * 0x320, 0xC) == 0)
            {
                count = 5;
                p = fld + (g_cload_card_slot * 0x320 + off28);
                c = *p;
                acc = 0;
                while (((u32)(c - 0x30) < 0xA) || ((u32)(c - 0x61) < 6) || ((u32)(c - 0x41) < 6))
                {
                    if (count == 0)
                    {
                        break;
                    }
                    cls = *p;
                    acc *= 0x10;
                    if ((u32)(cls - 0x30) < 0xA)
                    {
                        t = acc - 0x30;
                        goto add_char;
                    }
                    if ((u32)(cls - 0x41) < 6)
                    {
                        acc = acc - 0x37 + *p;
                    }
                    else
                    {
                        t = acc - 0x57;
                        if ((u32)(cls - 0x61) < 6)
                        {
                        add_char:
                            acc = t + *p;
                        }
                    }
                    p++;
                    c = *p;
                    count--;
                }
                field = entry + g_cload_card_slot * 0x320 + 0xC;
                *(s32 *)(g_cload_card_slot * 0x50 + off4 + (s32)&g_cload_entry_fields) = acc;
                r = cload_parse_hex_suffix_byte(field, acc, count);
                *out = r;
                if (max < r)
                {
                    max = r;
                }
            }
            else
            {
                *(s32 *)(g_cload_card_slot * 0x50 + off4 + (s32)&g_cload_entry_fields) = -1;
                *out = 0;
            }
            out++;
            entry += 0x28;
            off28 += 0x28;
            i++;
            off4 += 4;
        } while (i < g_cload_entry_state);
    }
    return max;
}


/**
 * @brief Rank the current page's entries and select the highest-scoring slot.
 * @param unused0 Unused; present only to match the caller's 3-argument ABI.
 * @param unused1 Unused; present only to match the caller's 3-argument ABI.
 * @param unused2 Unused; present only to match the caller's 3-argument ABI.
 * @return Index of the entry holding the maximum value.
 * @note WIP 92.06% (gcc272_cdk); residual is a row-base regalloc/sched cascade
 *       (CSE-FOLD on g_cload_entry_fields).
 * @see decomp.me (92.06%)
 */
s32 cload_rank_entries(s32 unused0, s32 unused1, s32 unused2)
{
    s32 *row;
    s32 *elem;
    s32 *rank_ptr;
    s32 *cmp_ptr;
    s32 *inc_ptr;
    s32 *base_rank;
    s32 *ecopy;
    s32 *max_ptr;
    s32 *out_ptr;
    char *ent_ptr;
    s32 t0v;
    s32 i;
    s32 s3v;
    s32 count;
    s32 handle;
    s32 less_count;
    s32 j;

    cload_parse_entry_fields();
    s3v = -1;
    cload_sort_entries_by_type();
    i = 0;
    handle = cload_parse_entry_fields();
    cload_reset_entry_ranks();
    t0v = 1;
    if (g_cload_entry_state > 0)
    {
        count = g_cload_entry_state;
        base_rank = &g_cload_entry_ranks[0];
        rank_ptr = base_rank;
        row = (s32 *)((g_cload_card_slot * 0x50) + (s32)g_cload_entry_fields);
        elem = row;
        do
        {
            if (*elem >= 0)
            {
                j = 0;
                if (*elem >= s3v)
                {
                    *rank_ptr = t0v;
                    s3v = *elem;
                    t0v += 1;
                }
                else
                {
                    less_count = 0;
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
                    *rank_ptr = t0v - less_count;
                    t0v += 1;
                }
            }
            rank_ptr += 1;
            i += 1;
            elem += 1;
        } while (i < count);
    }
    g_cload_rank_count = t0v;
    t0v = -1;
    i = 0;
    s3v = 0;
    if (g_cload_entry_state > 0)
    {
        max_ptr = (s32 *)((g_cload_card_slot * 0x50) + (s32)g_cload_entry_fields);
        do
        {
            if (t0v < *max_ptr)
            {
                t0v = *max_ptr;
                s3v = i;
            }
            i += 1;
            max_ptr += 1;
        } while (i < g_cload_entry_state);
        i = 0;
    }
    g_cload_entry_value_limit = t0v + 1;
    if (g_cload_entry_state > 0)
    {
        out_ptr = &g_cload_entry_suffix_values[0];
        ent_ptr = &g_cload_entries[0];
    loop_20:
        if (func_8001714C(&D_800ECFC4[0], (void *)((g_cload_card_slot * 0x320) + (s32)ent_ptr), 8) == 0)
        {
            *out_ptr = handle + 1;
        }
        else
        {
            out_ptr += 1;
            ent_ptr += 0x28;
            i += 1;
            if (i < g_cload_entry_state)
            {
                goto loop_20;
            }
        }
    }
    return s3v;
}


/**
 * @brief Reset the cload menu state: set the row-count/pitch field to 0x28 and
 *        clear all 15 slot entries of g_cload_entry_ranks to -1 (empty).
 * @see decomp.me (100.00%)
 */
void cload_reset_entry_ranks(void)
{
    s32 i;
    s32 val;

    g_cload_rank_count = 0x28;
    val = -1;
    for (i = 14; i >= 0; i--)
    {
        g_cload_entry_ranks[i] = val;
    }
}


/**
 * @brief Scan up to g_cload_entry_state entries of the g_cload_entries table (row selected by
 *        g_cload_card_slot, stride 0x28) and return 1 as soon as an entry fails to
 *        match either of the two patterns D_800ECF7C / D_800ECF8C; 0 if all pass.
 * @return 1 on the first non-matching entry, 0 if every entry matches both patterns.
 * @note WIP 95.77% (gcc272_cdk). Instruction-exact (52/52), frame -0x20, all sp
 *       slots match. Sole residue: a preheader emit-order rotation -- the
 *       `lui s2, %hi(g_cload_card_slot)` sits after entry's `lui/addiu(g_cload_entries)`
 *       instead of before it. Same registers (s0=entry, s2=A920 hi), so it is
 *       pure emit order. The only C constructs that reorder it (aliasing
 *       g_cload_card_slot through a pointer, ALLOC-06 style) force a full-pointer
 *       lui+addiu with 0x0(s2) addressing instead of the target's hi-only
 *       %lo(g_cload_card_slot)(s2) form, adding a spurious insn -- a worse match.
 * @see decomp.me (95.77%)
 */
s32 cload_has_known_entry_type(void)
{
    s32 i;
    u8 *entry;

    i = 0;
    if (g_cload_entry_state > 0)
    {
        entry = &g_cload_entries;
        do
        {
            if (func_8001714C(&D_800ECF7C, (void *)(g_cload_card_slot * 0x320 + (s32)entry), 0xC) == 0 ||
                func_8001714C(&D_800ECF8C, (void *)(g_cload_card_slot * 0x320 + (s32)entry), 0xC) == 0)
            {
                return 1;
            }
            entry += 0x28;
            i++;
        } while (i < g_cload_entry_state);
    }
    return 0;
}


/**
 * @brief Sum the (directory-entry size >> 13) fixed-point field over g_cload_entry_state records of the
 *        current g_cload_entries page and report whether the total reaches 14.
 * @return 1 if the accumulated total is >= 14, otherwise 0.
 * @note Records are walked with a running byte offset that starts at the page
 *       base (g_cload_card_slot * 0x320) and steps by 0x28; each directory-entry size is divided by
 *       8192 (signed, round toward zero).
 * @note Residual vs target is a 4-row loop-body a0/a1 permutation of the
 *       counter and accumulator (ALLOC-ORDER): the counter's extra loop-compare
 *       ref out-prioritizes the accumulator by a hair and no source rewrite
 *       flips it (permuter territory).
 * @see decomp.me (99.03%)
 */
s32 cload_entry_blocks_reach_limit(void)
{
    s32 i;
    s32 sum;
    s32 offset;

    sum = 0;
    i = 0;
    if (g_cload_entry_state > 0)
    {
        offset = g_cload_card_slot * 0x320;
        do
        {
            sum += ((CloadDirEntry *)((u8 *)g_cload_entries + offset))->size / 8192;
            i++;
            offset += 0x28;
        } while (i < g_cload_entry_state);
    }
    return sum >= 0xE;
}


/**
 * @brief Render the two fixed prompt strings (D_800ECF9C, D_800ECFB0).
 * @note Each draw copies a 6-byte header from g_cload_file_template, biases byte 2 by the
 *       current page index (g_cload_card_slot), then emits via func_80016F9C /
 *       func_8001686C.
 * @see decomp.me (100.00%)
 */
void cload_render_fixed_prompts(void)
{
    CloadFileHeaderScratch buf;

    memcpy(&buf, &g_cload_file_template, 6);
    ((u8 *)&buf)[2] += *(u8 *)&g_cload_card_slot;
    func_80016F9C(&buf, &D_800ECF9C);
    func_8001686C(&buf);

    memcpy(&buf, &g_cload_file_template, 6);
    ((u8 *)&buf)[2] += *(u8 *)&g_cload_card_slot;
    func_80016F9C(&buf, &D_800ECFB0);
    func_8001686C(&buf);
}


/**
 * @brief Advance the cload overlay load/decompress state machine one step.
 * @return The next phase code (1-5) for the caller to act on.
 * @note Dispatches on *g_cload_load_step (the current step opcode); each case drives
 *       CD reads, buffer setup, decode, and teardown, updating g_cload_load_step and
 *       the g_cload_entry_state / g_cload_selection_status status fields.
 * @see decomp.me (92.84%)
 */
s32 cload_advance_load_sequence(void)
{
    CloadLoadScratch buf;
    CloadFileHeaderScratch buf2;
    s32 sp98;
    s32 sp9C;
    s32 var_s4;
    s32 var_s0;
    s32 temp;
    s32 var_a0;
    s32 i;

    var_s4 = 1;
    memcpy(&buf, &g_cload_file_template, 6);
    ((u8 *)&buf)[2] += *(u8 *)&g_cload_card_slot;

    if (g_cload_load_step != NULL)
    {
        switch (*g_cload_load_step)
        {
        case 1:
            var_s4 = 3;
            func_8001729C(g_cload_card_slot);
            func_8001724C(g_cload_card_slot * 0x10);
            g_cload_load_step = g_cload_load_step + 1;
            goto block_81;

        case 2:
            temp = cload_poll_primary_handle_group();
            if (temp >= 3)
            {
                goto c2_ge3;
            }
            if (temp > 0)
            {
                goto c2_pos;
            }
            if (temp == 0)
            {
                g_cload_load_step = g_cload_load_step + 1;
                goto block_81;
            }
            goto block_81;
        c2_ge3:
            if (temp == 3)
            {
                goto c2_eq3;
            }
            goto block_81;
        c2_pos:
            var_s4 = 4;
            g_cload_selection_status = 0;
            g_cload_entry_state = 0xFD;
            g_cload_load_step = g_cload_load_step + 1;
            cload_deactivate_primary_element();
            goto block_81;
        c2_eq3:
            g_cload_rank_count = 0x28;
            for (i = 14; i >= 0; i--)
            {
                g_cload_entry_ranks[i] = -1;
            }
            goto block_58;

        case 3:
            cload_release_primary_handles();
            g_cload_load_step = g_cload_load_step + 1;
            goto block_81;

        case 4:
            do
            {
                temp = cload_poll_secondary_handle_group();
            } while (temp == -1);
            if (temp == 0)
            {
                g_cload_load_step = g_cload_load_step + 1;
                goto block_81;
            }
            if (temp < 0)
            {
                goto block_81;
            }
            if (temp >= 4)
            {
                goto block_81;
            }
            var_s4 = 4;
            goto block_42;

        case 5:
            cload_release_secondary_handles();
            g_cload_load_step = g_cload_load_step + 1;
            goto block_81;

        case 6:
            memcpy(&buf2, &g_cload_file_template, 6);
            ((u8 *)&buf2)[2] += *(u8 *)&g_cload_card_slot;
            func_80016F9C(&buf2, &D_800ECF9C);
            func_8001686C(&buf2);
            memcpy(&buf2, &g_cload_file_template, 6);
            ((u8 *)&buf2)[2] += *(u8 *)&g_cload_card_slot;
            func_80016F9C(&buf2, &D_800ECFB0);
            func_8001686C(&buf2);
            g_cload_entry_scan_active = 1;
            var_s0 = 0;
            if (cload_begin_entry_scan(g_cload_card_slot) == 0)
            {
                var_s4 = 2;
                g_cload_entry_state = 0xF8;
                g_cload_load_step = NULL;
                g_cload_entry_scan_active = 0;
                goto block_81;
            }
            g_cload_load_step = g_cload_load_step + 1;
            do
            {
                if (cload_scan_next_entry(g_cload_card_slot) == 0)
                {
                    g_cload_entry_scan_active = 0;
                    if (g_cload_entry_state == 0xF8)
                    {
                        goto block_81;
                    }
                    if (g_cload_entry_state == 0xFA)
                    {
                        goto block_81;
                    }
                    cload_commit_selected_entry();
                    goto block_81;
                }
                var_s0 = var_s0 + 1;
            } while (var_s0 < 0x14);
            goto block_81;

        case 8:
            var_s4 = 3;
            func_8001729C(g_cload_card_slot);
            func_800172AC(g_cload_card_slot * 0x10);
            g_cload_load_step = g_cload_load_step + 1;
            goto block_81;

        case 9:
            var_s4 = 3;
            func_8001729C(g_cload_card_slot);
            func_8001725C(g_cload_card_slot * 0x10);
            g_cload_primary_poll_countdown = 0x10;
            g_cload_secondary_poll_countdown = 0x10;
            g_cload_load_step = g_cload_load_step + 1;
            goto block_81;

        case 0:
            var_s4 = 2;
            D_80162370 = 0;
            goto block_81;

        case 15:
            temp = cload_poll_primary_handle_group();
            if (temp >= 3)
            {
                goto c15_ge3;
            }
            if (temp > 0)
            {
                goto c15_pos;
            }
            if (temp == 0)
            {
                g_cload_load_step = g_cload_load_step + 1;
                goto block_81;
            }
            goto block_81;
        c15_ge3:
            if (temp == 3)
            {
                goto c15_eq3;
            }
            goto block_81;
        c15_pos:
            g_cload_secondary_poll_countdown = g_cload_secondary_poll_countdown - 1;
            if (g_cload_secondary_poll_countdown != 0)
            {
                goto block_44;
            }
            var_s4 = 4;
        block_42:
            g_cload_selection_status = 0;
            g_cload_entry_state = 0xFD;
            goto block_81;
        c15_eq3:
            g_cload_primary_poll_countdown = g_cload_primary_poll_countdown - 1;
            if (g_cload_primary_poll_countdown == 0)
            {
                goto c15_d70zero;
            }
        block_44:
            func_8001729C(g_cload_card_slot);
            func_800172AC(g_cload_card_slot * 0x10);
            func_8001729C(g_cload_card_slot);
            func_8001725C(g_cload_card_slot * 0x10);
            goto block_81;
        c15_d70zero:
            var_s4 = 5;
            g_cload_entry_state = 0xFC;
            g_cload_load_step = &D_80146528;
            goto block_81;

        case 16:
            do
            {
                temp = cload_poll_secondary_handle_group();
            } while (temp == -1);
            g_cload_load_step = g_cload_load_step + 1;
            goto block_81;

        case 17:
            g_cload_io_busy = 1;
            g_cload_selection_status = 0;
            func_8001729C(g_cload_card_slot);
            g_cload_file_handle = func_8001680C(&D_80162C90, 0x8001);
            if (g_cload_file_handle == -1)
            {
                goto block_81;
            }
            cload_release_primary_handles();
            func_8001729C(g_cload_card_slot);
            var_a0 = 0x80;
            if (g_cload_selected_entry_extended != 0)
            {
                var_a0 = 0x280;
            }
            if (func_8001681C(g_cload_file_handle, &D_80162A10, var_a0) != -1)
            {
                g_cload_load_step = g_cload_load_step + 1;
                goto block_81;
            }
            func_8001683C(g_cload_file_handle);
            goto block_81;

        case 18:
            temp = cload_poll_primary_handle_group();
            if (temp == 0)
            {
                g_cload_io_busy = 0;
                g_cload_selection_status = 1;
                goto block_65;
            }
            if (temp == -1)
            {
                goto block_81;
            }
            g_cload_io_busy = 0;
            func_8001683C(g_cload_file_handle);
        block_58:
            g_cload_entry_state = 0xFF;
            g_cload_load_step = D_8014651C;
            goto block_81;

        case 30:
            g_cload_retry_count = 5;
            g_cload_load_step = g_cload_load_step + 1;
            goto block_81;

        case 19:
            g_cload_progress_active = 1;
            g_cload_progress_bar_active = 1;
            g_cload_progress_start_tick = func_8002054C(-1);
            func_8001729C(g_cload_card_slot);
            g_cload_file_handle = func_8001680C(&D_80162C90, 0x8001);
            cload_release_primary_handles();
            func_8001729C(g_cload_card_slot);
            if (func_8001681C(g_cload_file_handle, &g_cload_save_blob, 0x4000) != -1)
            {
                g_cload_load_step = g_cload_load_step + 1;
                goto block_81;
            }
            func_8001683C(g_cload_file_handle);
            g_cload_retry_count = g_cload_retry_count - 1;
            if (g_cload_retry_count == 0)
            {
                var_a0 = 1;
                goto block_80;
            }
            goto block_81;

        case 20:
            temp = cload_poll_primary_handle_group();
            if (temp == 0)
            {
                g_cload_progress_active = 0;
            block_65:
                g_cload_load_step = g_cload_load_step + 1;
                func_8001683C(g_cload_file_handle);
                goto block_81;
            }
            if (temp < 0)
            {
                goto block_81;
            }
            if (temp >= 4)
            {
                goto block_81;
            }
            g_cload_retry_count = g_cload_retry_count - 1;
            if (g_cload_retry_count == 0)
            {
                goto c20_378zero;
            }
            func_8001683C(g_cload_file_handle);
            g_cload_load_step = g_cload_load_step - 1;
            goto block_81;
        c20_378zero:
            func_8001683C(g_cload_file_handle);
            var_a0 = 1;
            g_cload_progress_bar_active = 0;
            goto block_80;

        case 24:
            var_s0 = 0;
            do
            {
                if (func_800342CC(g_cload_card_slot * 0x10) == 1)
                {
                    break;
                }
                func_8002054C(0);
                var_s0 = var_s0 + 1;
            } while (var_s0 < 0x14);
            if (var_s0 != 0x14)
            {
                func_80032174(0, &sp98, &sp9C);
                var_a0 = 3;
                if (sp9C == 0)
                {
                    g_cload_load_step = g_cload_load_step + 1;
                    goto block_81;
                }
                goto block_80;
            }
            var_a0 = 3;
            goto block_80;
        }
    }
    goto block_81;

block_80:
    cload_open_status_dialog(var_a0);

block_81:
    return var_s4;
}


/**
 * @brief Reset the cached resource handles and arm the first load step.
 * @note Releases the handles (cload_release_primary_handles), rewinds the CD channel, and points
 *       g_cload_load_step at the D_80146528 step table.
 * @see decomp.me (100.00%)
 */
void cload_restart_load_sequence(void)
{
    cload_release_primary_handles();
    func_8001729C(g_cload_card_slot);
    func_8001724C(g_cload_card_slot * 0x10);
    g_cload_load_step = &D_80146528;
}


/**
 * @brief Poll the four cached handles; on completion, rewind the CD channel.
 * @return The busy-slot index from cload_poll_primary_handle_group (-1 when none are busy).
 * @see decomp.me (100.00%)
 */
s32 cload_poll_and_rewind_primary_handles(void)
{
    s32 temp_v0;

    temp_v0 = cload_poll_primary_handle_group();
    if (temp_v0 != -1)
    {
        func_8001729C(g_cload_card_slot);
        func_8001724C(g_cload_card_slot * 0x10);
    }
    return temp_v0;
}


/**
 * @brief Allocate and register the eight streaming buffers for this overlay.
 * @note Brackets the eight func_800167AC allocations (handles stored in
 *       g_cload_primary_handle0..g_cload_secondary_handle3) with func_800167EC / func_800167FC and resets the
 *       stream bookkeeping (g_cload_entry_scan_active, g_cload_progress_start_tick, g_cload_progress_bar_active).
 * @see decomp.me (100.00%)
 */
void cload_init_stream_handles(void)
{
    func_800158E0();
    func_800167EC();
    g_cload_primary_handle0 = func_800167AC(0xF4000001, 4, 0x2000, 0);
    g_cload_primary_handle1 = func_800167AC(0xF4000001, 0x8000, 0x2000, 0);
    g_cload_primary_handle2 = func_800167AC(0xF4000001, 0x100, 0x2000, 0);
    g_cload_primary_handle3 = func_800167AC(0xF4000001, 0x2000, 0x2000, 0);
    g_cload_secondary_handle0 = func_800167AC(0xF0000011, 4, 0x2000, 0);
    g_cload_secondary_handle1 = func_800167AC(0xF0000011, 0x8000, 0x2000, 0);
    g_cload_secondary_handle2 = func_800167AC(0xF0000011, 0x100, 0x2000, 0);
    g_cload_secondary_handle3 = func_800167AC(0xF0000011, 0x2000, 0x2000, 0);
    func_800167DC(g_cload_primary_handle0);
    func_800167DC(g_cload_primary_handle1);
    func_800167DC(g_cload_primary_handle2);
    func_800167DC(g_cload_primary_handle3);
    func_800167DC(g_cload_secondary_handle0);
    func_800167DC(g_cload_secondary_handle1);
    func_800167DC(g_cload_secondary_handle2);
    func_800167DC(g_cload_secondary_handle3);
    func_800167FC();
    g_cload_entry_scan_active = 0;
    g_cload_progress_start_tick = func_8002054C(-1);
    g_cload_progress_bar_active = 0;
}


/**
 * @brief Tear down / release the eight g_cload_primary_handle0..g_cload_secondary_handle3 handles.
 * @note Wrapped by func_800158E0 and func_800167EC/func_800167FC bracket calls;
 *       each handle is passed to func_800167BC in turn (g_cload_entry_scan_active is skipped).
 * @see decomp.me (100.00%)
 */
void cload_shutdown_stream_handles(void)
{
    func_800158E0();
    func_800167EC();
    func_800167BC(g_cload_primary_handle0);
    func_800167BC(g_cload_primary_handle1);
    func_800167BC(g_cload_primary_handle2);
    func_800167BC(g_cload_primary_handle3);
    func_800167BC(g_cload_secondary_handle0);
    func_800167BC(g_cload_secondary_handle1);
    func_800167BC(g_cload_secondary_handle2);
    func_800167BC(g_cload_secondary_handle3);
    func_800167FC();
}


/**
 * @brief Begin streaming the page page's first g_cload_entries record.
 * @param page Page index (each page is 0x320 bytes in g_cload_entries).
 * @return 1 if func_80016BCC accepted the record (count bumped), else 0.
 * @note WIP 94.47% (gcc272_cdk); residual is a 2-insn memcpy base / CSE-share
 *       coupling.
 * @see decomp.me (94.47%)
 */
s32 cload_begin_entry_scan(s32 page)
{
    CloadEntryHeader buf;
    CloadEntryHeader *p = &g_cload_entry_header_template;

    memcpy(&buf, p, 7);
    g_cload_scroll_frames = 0;
    g_cload_scroll_target_y = 0;
    g_cload_scroll_y = 0;
    g_cload_selected_row = 0;
    g_cload_entry_state = 0;
    ((u8 *)&buf)[2] += page;
    if (func_80016BCC(&buf, g_cload_entries + page * 0x320, p->unk6, p) != 0)
    {
        g_cload_entry_state += 1;
        return 1;
    }
    return 0;
}


/**
 * @brief Try to append the page page's next g_cload_entries record; if it cannot,
 *        recompute the page's fixed-point total and update the selection state.
 * @param page Page index (each page is 0x320 bytes / 20 records in g_cload_entries).
 * @return 1 if func_8001684C accepted the new record (count bumped), else 0.
 * @note When the record is rejected, the (directory-entry size >> 13) total over the page's
 *       records decides whether the count is clamped (0xFA) or the selection
 *       (g_cload_selected_row) is set from cload_rank_entries's result; the >= 14 test is
 *       materialized as a boolean, matching the inlined cload_entry_blocks_reach_limit pattern.
 * @see decomp.me (100.00%)
 */
s32 cload_scan_next_entry(s32 page)
{
    s32 i;
    s32 sum;
    s32 offset;
    s32 s0_val;
    s32 term1;
    s32 term2;
    s32 count;
    s32 cond;

    term1 = page * 0x320;
    term2 = g_cload_entry_state * 0x28 + (s32)g_cload_entries;
    if (func_8001684C((void *)(term1 + term2)) != 0)
    {
        g_cload_entry_state += 1;
        return 1;
    }
    func_800AA02C();
    if (cload_has_known_entry_type() == 0)
    {
        g_cload_entry_state = 0xF8;
    }
    else
    {
        i = 0;
        sum = 0;
        count = g_cload_entry_state;
        if (count > 0)
        {
            offset = g_cload_card_slot * 0x320;
            do
            {
                sum += ((CloadDirEntry *)((u8 *)g_cload_entries + offset))->size / 8192;
                i++;
                offset += 0x28;
            } while (i < count);
        }
        cond = sum >= 0xE;
        if (cond != 0)
        {
            s0_val = cload_rank_entries(sum, i, count);
            if (cload_has_known_entry_type() == 0)
            {
                g_cload_entry_state = 0xFA;
                g_cload_entry_value_limit = 0;
            }
            else
            {
                g_cload_selected_row = s0_val;
                cload_scroll_to_selection();
            }
        }
        else
        {
            s0_val = cload_rank_entries(sum, i, count);
            if (cload_has_known_entry_type() == 0)
            {
                g_cload_selected_row = 0;
                cload_scroll_to_selection();
                g_cload_entry_value_limit = 0;
            }
            else
            {
                g_cload_selected_row = s0_val;
                cload_scroll_to_selection();
            }
        }
    }
    return 0;
}


/**
 * @brief Commit the selected g_cload_entries record and arm the next step.
 * @note Validates the record against the D_800ECFC4 / D_800ECF7C patterns, copies
 *       its header into a local CloadFileHeader, biases byte 2 by the page index, and
 *       registers it via func_800170BC; sets the g_cload_selection_status / g_cload_selected_entry_extended status.
 * @see decomp.me (91.94%)
 */
void cload_commit_selected_entry(void)
{
    CloadFileHeader local;
    CloadFileHeader *src;
    u8 *p;
    s32 term1;
    s32 term2;

    if (g_cload_entry_state == 0)
    {
        g_cload_selection_status = 3;
        return;
    }
    term1 = g_cload_card_slot * 0x320;
    term2 = (g_cload_selected_row * 0x28) + (s32)g_cload_entries;
    term2 += term1;
    if (func_8001714C(&D_800ECFC4[0], (void *)(term2 + term1), 8) == 0)
    {
        g_cload_selection_status = 2;
        return;
    }
    p = (u8 *)&local;
    src = &g_cload_file_template;
    *(s32 *)p = src->unk0;
    *(s16 *)(p + 4) = src->unk4;
    term1 = g_cload_card_slot * 0x320;
    term2 = (g_cload_selected_row * 0x28) + (s32)g_cload_entries;
    term2 += term1;
    func_80016F9C(p, (void *)(term2 + term1), src);
    g_cload_selection_status = 0;
    *((u8 *)&local + 2) += (u8)g_cload_card_slot;
    func_800170BC(&D_80162C90[0], p, (u8)g_cload_card_slot);
    g_cload_load_step = &D_80146538[0];
    term1 = g_cload_card_slot * 0x320;
    term2 = (g_cload_selected_row * 0x28) + (s32)g_cload_entries;
    term2 += term1;
    if (func_8001714C(&D_800ECF7C[0], (void *)(term2 + term1), 0xC) == 0)
    {
        g_cload_selected_entry_extended = 1;
        return;
    }
    g_cload_selected_entry_extended = 0;
}


/**
 * @brief Release the four cached resource handles for this overlay.
 *
 * Passes the values held in g_cload_primary_handle0, g_cload_primary_handle1, g_cload_primary_handle2, and g_cload_primary_handle3
 * (in that order) to @ref func_800167CC.
 * @see decomp.me (100.00%)
 */
void cload_release_primary_handles(void)
{
    func_800167CC(g_cload_primary_handle0);
    func_800167CC(g_cload_primary_handle1);
    func_800167CC(g_cload_primary_handle2);
    func_800167CC(g_cload_primary_handle3);
}


/**
 * @brief Release the next four cached resource handles for this overlay.
 *
 * Passes the values held in g_cload_secondary_handle0, g_cload_secondary_handle1, g_cload_secondary_handle2, and g_cload_secondary_handle3
 * (in that order) to @ref func_800167CC.
 * @see decomp.me (100.00%)
 */
void cload_release_secondary_handles(void)
{
    func_800167CC(g_cload_secondary_handle0);
    func_800167CC(g_cload_secondary_handle1);
    func_800167CC(g_cload_secondary_handle2);
    func_800167CC(g_cload_secondary_handle3);
}


/**
 * @brief Release four cached handles, returning the index of the first busy one.
 *
 * Passes each of g_cload_primary_handle0, g_cload_primary_handle1, g_cload_primary_handle2, g_cload_primary_handle3 to
 * @ref func_800167CC in order; the first call that returns 1 stops the sequence
 * and yields that slot's index (0-3). Returns -1 if none report busy.
 *
 * @return Index 0-3 of the first handle whose release returned 1, else -1.
 * @see decomp.me (100.00%)
 */
s32 cload_poll_primary_handle_group(void)
{
    if (func_800167CC(g_cload_primary_handle0) == 1)
    {
        return 0;
    }
    if (func_800167CC(g_cload_primary_handle1) == 1)
    {
        return 1;
    }
    if (func_800167CC(g_cload_primary_handle2) == 1)
    {
        return 2;
    }
    if (func_800167CC(g_cload_primary_handle3) == 1)
    {
        return 3;
    }
    return -1;
}


/**
 * @brief Release four cached handles, returning the index of the first busy one.
 *
 * Passes each of g_cload_secondary_handle0, g_cload_secondary_handle1, g_cload_secondary_handle2, g_cload_secondary_handle3 to
 * @ref func_800167CC in order; the first call that returns 1 stops the sequence
 * and yields that slot's index (0-3). Returns -1 if none report busy.
 *
 * @return Index 0-3 of the first handle whose release returned 1, else -1.
 * @see decomp.me (100.00%)
 */
s32 cload_poll_secondary_handle_group(void)
{
    if (func_800167CC(g_cload_secondary_handle0) == 1)
    {
        return 0;
    }
    if (func_800167CC(g_cload_secondary_handle1) == 1)
    {
        return 1;
    }
    if (func_800167CC(g_cload_secondary_handle2) == 1)
    {
        return 2;
    }
    if (func_800167CC(g_cload_secondary_handle3) == 1)
    {
        return 3;
    }
    return -1;
}


/**
 * @brief Collate the g_cload_entries page records, ordering them by pattern class.
 * @note Five passes bucket records matching D_800ECF7C, then D_800ECF8C, then
 *       D_800ECFC4, then the remainder, copying each 0x28-byte record with
 *       func_80016E7C before writing the ordered set back to the page.
 * @see decomp.me (86.61%)
 */
void cload_sort_entries_by_type(void)
{
    u8 buf[0x320];
    s32 out;
    s32 grp;
    s32 i1;
    u8 *entry1;
    s32 *gptr1;
    s32 off1;
    s32 grp2;
    s32 i2;
    u8 *entry2;
    s32 *gptr2;
    s32 off2;
    s32 i3;
    u8 *entry3;
    s32 off3;
    s32 i4;
    u8 *entry4;
    s32 off4;
    s32 i5;
    u8 *rec5;
    u8 *bp5;

    grp = 0;
    out = 0;
    do
    {
        i1 = 0;
        if (g_cload_entry_state > 0)
        {
            entry1 = &g_cload_entries;
            gptr1 = &g_cload_entry_suffix_values;
            off1 = out * 0x28;
            do
            {
                if (*gptr1 == grp && func_8001714C(&D_800ECF7C, entry1 + g_cload_card_slot * 0x320, 0xC) == 0)
                {
                    func_80016E7C(entry1 + g_cload_card_slot * 0x320, &buf[off1], 0x28);
                    off1 += 0x28;
                    out += 1;
                }
                entry1 += 0x28;
                i1 += 1;
                gptr1 += 1;
            } while (i1 < g_cload_entry_state);
        }
        grp += 1;
    } while (grp < 8);

    grp2 = 0;
    do
    {
        i2 = 0;
        if (g_cload_entry_state > 0)
        {
            entry2 = &g_cload_entries;
            gptr2 = &g_cload_entry_suffix_values;
            off2 = out * 0x28;
            do
            {
                if (*gptr2 == grp2 && func_8001714C(&D_800ECF8C, entry2 + g_cload_card_slot * 0x320, 0xC) == 0)
                {
                    func_80016E7C(entry2 + g_cload_card_slot * 0x320, &buf[off2], 0x28);
                    off2 += 0x28;
                    out += 1;
                }
                entry2 += 0x28;
                i2 += 1;
                gptr2 += 1;
            } while (i2 < g_cload_entry_state);
        }
        grp2 += 1;
    } while (grp2 < 8);

    i3 = 0;
    if (g_cload_entry_state > 0)
    {
        entry3 = &g_cload_entries;
        off3 = out * 0x28;
        do
        {
            if (func_8001714C(&D_800ECFC4, entry3 + g_cload_card_slot * 0x320, 8) == 0)
            {
                func_80016E7C(entry3 + g_cload_card_slot * 0x320, &buf[off3], 0x28);
                off3 += 0x28;
                out += 1;
            }
            i3 += 1;
            entry3 += 0x28;
        } while (i3 < g_cload_entry_state);
    }

    i4 = 0;
    if (g_cload_entry_state > 0)
    {
        entry4 = &g_cload_entries;
        off4 = out * 0x28;
        do
        {
            if (func_8001714C(&D_800ECF7C, entry4 + g_cload_card_slot * 0x320, 0xC) != 0 &&
                func_8001714C(&D_800ECF8C, entry4 + g_cload_card_slot * 0x320, 0xC) != 0 &&
                func_8001714C(&D_800ECFC4, entry4 + g_cload_card_slot * 0x320, 8) != 0)
            {
                func_80016E7C(entry4 + g_cload_card_slot * 0x320, &buf[off4], 0x28);
                off4 += 0x28;
            }
            i4 += 1;
            entry4 += 0x28;
        } while (i4 < g_cload_entry_state);
    }

    i5 = 0;
    if (g_cload_entry_state > 0)
    {
        rec5 = &g_cload_entries;
        bp5 = &buf[0];
        do
        {
            func_80016E7C(bp5, rec5 + g_cload_card_slot * 0x320, 0x28);
            bp5 += 0x28;
            i5 += 1;
            rec5 += 0x28;
        } while (i5 < g_cload_entry_state);
    }
}


/**
 * @brief Format value as a 5-digit glyph string (leading zeros suppressed, sign
 *        prepended) and hand it to cload_draw_cached_text for rendering.
 * @param prim Passed through to cload_draw_cached_text (prim/handle).
 * @param ot Passed through to cload_draw_cached_text (ordering table).
 * @param value Signed value to format.
 * @param x Passed through to cload_draw_cached_text (x).
 * @param y Passed through to cload_draw_cached_text (y).
 * @param palette Passed through to cload_draw_cached_text.
 * @param alignment Passed through to cload_draw_cached_text.
 * @note Each decimal digit indexes the g_cload_decimal_glyphs glyph table; 0x4F82 is the
 *       '0' glyph (skipped while leading) and 0x5B81 the minus glyph.
 * @note WIP ~70%: the seven magic-number divisions are emitted correctly but
 *       gcc's sched2 interleaving of the imuldiv ops and the resulting
 *       register-allocation cascade (the target's callee-saved s0 vs a temp,
 *       and the value register) are not reproducible from source shape alone
 *       (confirmed by sched_oracle) - permuter territory.
 * @see decomp.me (70.35%)
 */
void cload_draw_signed_decimal(s32 prim, s32 *ot, s32 value, s32 x, s32 y, s32 palette, s32 alignment)
{
    u16 buf[7];
    s32 count;
    s32 n;
    s32 neg;
    s32 minus;
    u16 *p;

    n = value;
    if (n < 0)
    {
        n = -n;
        neg = 1;
    }
    else
    {
        neg = 0;
    }
    buf[1] = g_cload_decimal_glyphs[n / 10000];
    count = 1;
    buf[2] = g_cload_decimal_glyphs[(n % 10000) / 1000];
    buf[3] = g_cload_decimal_glyphs[(n % 1000) / 100];
    buf[4] = g_cload_decimal_glyphs[(n % 100) / 10];
    p = &buf[1];
    buf[6] = 0;
    buf[5] = g_cload_decimal_glyphs[n % 10];
loop:
    if (*p == 0x4F82)
    {
        count++;
        p++;
        if (count < 5)
        {
            goto loop;
        }
    }
    if (neg != 0)
    {
        minus = 0x5B81;
        count--;
        buf[count] = minus;
    }
    cload_draw_cached_text(prim, ot, &buf[count], x, y, palette, alignment);
}


/**
 * @brief Render @p value as a two-digit hex glyph string via cload_draw_cached_text.
 * @param prim Passed through to cload_draw_cached_text (prim/handle).
 * @param ot Passed through to cload_draw_cached_text (ordering table).
 * @param value Byte value; its high and low nibbles index the g_cload_hex_glyphs glyph table.
 * @param x X coordinate passed through.
 * @param y Y coordinate passed through.
 * @param palette Passed through to cload_draw_cached_text.
 * @see decomp.me (91.21%)
 */
void cload_draw_hex_byte(s32 prim, s32 ot, s32 value, s32 x, s32 y, s32 palette)
{
    u16 buf[3];

    buf[0] = g_cload_hex_glyphs[value / 16];
    buf[1] = g_cload_hex_glyphs[value % 16];
    buf[2] = 0;
    cload_draw_cached_text(prim, ot, buf, x, y, 0, palette);
}


/**
 * @brief Draw encoded text through the CLOAD glyph cache.
 * @param prim Primitive-buffer cursor.
 * @param ot Ordering-table head.
 * @param text Encoded text buffer.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param palette Glyph palette index.
 * @param alignment Text alignment mode.
 * @return Advanced primitive-buffer cursor.
 * @see decomp.me (100.00%)
 */
s32 cload_draw_cached_text(s32 prim, s32 *ot, u8 *text, s32 x, s32 y, s32 palette, s32 alignment)
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
    g_cload_text_line_start_x = x;
    g_cload_glyph_cursor_x = x;
    g_cload_glyph_cursor_y = y;

    while (1)
    {
        u32 lead = *cursor;

        if ((u8)lead == 0x20)
        {
            cursor++;
            g_cload_glyph_cursor_x += 0x10;
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
        prim = cload_render_cached_glyph(prim, ot, code, palette);
    }

    ((u8 *)prim)[3] = 1;
    ((CloadGpuPacket *)prim)->unk4 = 0xE1000005;
    ((CloadOtTag *)prim)->addr = ((CloadOtTag *)ot)->addr;
    ((CloadOtTag *)ot)->addr = (u32)prim;
    return prim + 8;
}


/**
 * @brief One 4-byte glyph-cache slot: the cached character code plus per-frame
 *        usage flags, also read as a single word when scanning for a free slot.
 */
typedef union
{
    /* 0x0 */ u32 raw;
    struct
    {
        /* 0x0 */ u16 code;
        /* 0x2 */ u16 flags;
    } data;
} CloadGlyphCacheEntry;

extern CloadGlyphCacheEntry g_cload_glyph_cache[];

/**
 * @brief 0x14-byte packet-buffer slot holding one 16x16 textured sprite;
 *        mirrors SPRT_16 in include/psyq/libgpu.h plus the trailing word that
 *        keeps consecutive glyph packets 20 bytes apart.
 */
typedef struct
{
    /* 0x00 */ u32 tag;
    /* 0x04 */ u8 r0;
    /* 0x05 */ u8 g0;
    /* 0x06 */ u8 b0;
    /* 0x07 */ u8 code;
    /* 0x08 */ s16 x0;
    /* 0x0A */ s16 y0;
    /* 0x0C */ u8 u0;
    /* 0x0D */ u8 v0;
    /* 0x0E */ u16 clut;
    /* 0x10 */ u32 padding;
} CloadGlyphSprite;

/**
 * @brief Render one glyph, populating the cache on a miss.
 * @param prim Primitive-buffer cursor.
 * @param ot Ordering-table head.
 * @param character_code Encoded glyph code.
 * @param palette Glyph palette index.
 * @return Advanced primitive-buffer cursor, or the input cursor on failure.
 * @see decomp.me (100.00%)
 */
s32 cload_render_cached_glyph(s32 prim, s32 *ot, s32 character_code, s32 palette)
{
    CloadGlyphCacheEntry *entry;
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
    entry = g_cload_glyph_cache;

    while (slot < 0x100)
    {
        if (requested_code == entry->data.code)
        {
            return cload_emit_glyph_sprite(prim, ot, slot, palette);
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

    raster = g_cload_glyph_raster_cursor;
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
    while ((slot < 0x100) && (g_cload_glyph_cache[slot].raw != 0))
    {
        slot++;
    }

    if (slot == 0x100)
    {
        return prim;
    }
    g_cload_glyph_cache[slot].raw = code & 0xFFFF;
    prim = cload_emit_glyph_sprite(prim, ot, slot, palette);

    g_cload_glyph_upload_x = (slot % 16) * 4;
    g_cload_glyph_upload_y = slot & 0xF0;

    rect.w = 4;
    rect.h = 15;
    rect.x = g_cload_glyph_upload_x + 0x140;
    rect.y = g_cload_glyph_upload_y;

    func_80019A34(&rect, g_cload_glyph_raster_cursor);
    func_80019788(0);

    g_cload_glyph_raster_cursor += 0x80;
    return prim;
}


/**
 * @brief Emit one cached 16x16 glyph sprite.
 * @param sprite Glyph sprite packet.
 * @param ot Ordering-table head.
 * @param cache_slot Glyph-cache slot.
 * @param palette Glyph palette index.
 * @return Advanced glyph-packet cursor.
 * @see decomp.me (100.00%)
 */
s32 cload_emit_glyph_sprite(CloadGlyphSprite *sprite, s32 *ot, s32 cache_slot, s32 palette)
{
    u32 ot_tag_high_byte;
    s32 normalized_slot;
    u32 packet_address;
    s32 old_x;
    s32 new_x;
    s32 fits_line;

    g_cload_glyph_cache[cache_slot].raw |= 0x10000;

    ((u8 *)sprite)[3] = 3;
    sprite->code = 0x7C;
    sprite->g0 = 0x80;
    sprite->b0 = 0x80;
    sprite->r0 = 0x80;
    normalized_slot = cache_slot;
    sprite->x0 = g_cload_glyph_cursor_x;
    sprite->y0 = g_cload_glyph_cursor_y;

    if (cache_slot < 0)
    {
        normalized_slot = cache_slot + 15;
    }

    sprite->u0 = (cache_slot - ((normalized_slot >> 4) * 16)) * 16;
    sprite->v0 = cache_slot & 0xF0;
    sprite->clut = 0x7FD3;
    sprite->tag = (sprite->tag & 0xFF000000) | (*ot & 0xFFFFFF);

    packet_address = ((u32)sprite) & 0xFFFFFF;
    ot_tag_high_byte = *ot & 0xFF000000;

    sprite++;
    old_x = g_cload_glyph_cursor_x;
    new_x = old_x + 16;
    fits_line = (old_x + 32) < 0x280;
    g_cload_glyph_cursor_x = new_x;

    *ot = ot_tag_high_byte | packet_address;

    if (!fits_line)
    {
        g_cload_glyph_cursor_x = g_cload_text_line_start_x;
        g_cload_glyph_cursor_y += 16;
    }

    return (s32)sprite;
}


/**
 * @brief Clear per-frame glyph-use flags and reset raster allocation.
 * @see decomp.me (100.00%)
 */
void cload_begin_glyph_cache_frame(void)
{
    s32 cache_slot;
    CloadGlyphCacheEntry *cache_entry;

    g_cload_glyph_raster_cursor = g_cload_glyph_raster_buffer;

    cache_slot = 0;
    cache_entry = g_cload_glyph_cache;

    while (cache_slot < 0x100)
    {
        cache_entry->raw &= 0xFFFF;
        cache_entry++;
        cache_slot++;
    }
}


/**
 * @brief Evict glyph-cache entries not used this frame.
 * @see decomp.me (100.00%)
 */
void cload_evict_unused_glyphs(void)
{
    s32 used_flag;
    s32 cache_slot;
    CloadGlyphCacheEntry *cache_entry;

    cache_slot = 0;
    used_flag = 0x10000;
    cache_entry = g_cload_glyph_cache;

    while (cache_slot < 0x100)
    {
        if (!(cache_entry->raw & used_flag))
        {
            cache_entry->raw = 0;
        }

        cache_slot++;
        cache_entry++;
    }
}


/**
 * @brief Clear the glyph cache and raster scratch buffer.
 * @see decomp.me (100.00%)
 */
void cload_reset_glyph_cache(void)
{
    s32 cache_slot;
    CloadGlyphCacheEntry *cache_entry;

    cache_slot = 0x100 - 1;
    cache_entry = &g_cload_glyph_cache[cache_slot];
    while (cache_slot >= 0)
    {
        cache_entry->raw = 0;
        cache_entry--;
        cache_slot--;
    }

    for (cache_slot = 0; cache_slot <= 0x7FFF; cache_slot++)
    {
        g_cload_glyph_raster_buffer[cache_slot] = 0;
    }
}


/**
 * @brief Expand a mixed ASCII / two-byte input string into Shift-JIS glyph
 *        pairs, writing two bytes per source character and a NUL terminator.
 * @param out Destination buffer for the expanded 2-byte glyph codes.
 * @param in Source string, terminated by a 0 byte.
 * @note WIP 73.58% (45/93 exact, correct insn count, no structural runs).
 *       Lead bytes 0x19..0x1F select a [16][33] block of g_cload_double_byte_char_table indexed by
 *       the next byte's nibbles; 0x21 and above index g_cload_single_byte_char_table by
 *       (c - 0x20); everything else emits g_cload_single_byte_char_table's first entry (the blank
 *       glyph) and consumes one byte. Both tables are arrays of 33-byte rows
 *       (16 two-byte glyphs plus a 0x0A row terminator).
 * @note Measured-required shapes, each re-verified by reverting it:
 *       (1) `for (;;)` with `goto done` past the loop, NOT `while (*in != 0)`.
 *       A `while` puts a conditional jump to the loop's end_label at the top,
 *       which gcc 2.7.2's expand_end_loop rotates to the bottom (guard + copied
 *       test, +4 insns); jumping to a label BEYOND the loop is not an exit jump
 *       to end_label, so no rotation happens, and the loop notes still let LICM
 *       hoist the four table base pointers. A bare `goto` loop with no
 *       for/while at all defeats LICM instead and loses the hoists.
 *       (2) arm 2's index must read `(index / 16) * 33` BEFORE `(index & 0xF)
 *       * 2` (+17 exact; the other order was the single biggest gap).
 *       (3) arm 1's index must read `in[0] * 528 + (in[1] >> 4) * 33 +
 *       (in[1] & 0xF) * 2` in that order (every other permutation is -3 to -10).
 *       (4) `u32 c` holding the raw byte with `(u8)c` at the two comparisons -
 *       the `lbu` + `andi 0xff` pair the target shows, idiom [EXPAND-37].
 * @note Residue (10 target-only / 10 yours-only / 35 argdiff rows), two causes:
 *       (a) the target re-loads `in[0]` inside each of arm 1's two index
 *       expressions while this compile reuses the loop-head byte via CSE; no
 *       source spelling found that blocks that fold (using `c` there instead is
 *       -7, goto-separated arms are -7, pointer-vs-array and every term
 *       reordering are inert or worse). That also drags the `in` parameter into
 *       an extra `addu a3, a1, zero` entry copy and rotates several registers.
 *       (b) two rows show `%hi(g_cload_double_byte_char_table-0x3390)` against the target's
 *       `%hi(cload_load_icon_resources+0x2c)`; both resolve to 0x80143350, so those are a
 *       splat symbol-display artifact, not a real difference.
 *       Permuter v2 ran ~80k iterations over two seeds; it found (2) and
 *       nothing beyond it.
 * @see decomp.me (73.58%)
 */
void cload_expand_text_glyph_codes(u8 *out, u8 *in)
{
    u32 c;
    s32 index;

    for (;;)
    {
        c = *in;
        if ((u8)c == 0)
        {
            goto done;
        }
        if ((u32)(c - 0x19) < 7)
        {
            *out++ = g_cload_double_byte_char_table[in[0] * 528 + (in[1] >> 4) * 33 + (in[1] & 0xF) * 2];
            *out++ = g_cload_double_byte_char_table[in[0] * 528 + (in[1] >> 4) * 33 + (in[1] & 0xF) * 2 + 1];
            in += 2;
        }
        else if ((u8)c >= 0x21)
        {
            index = *in - 0x20;
            *out++ = g_cload_single_byte_char_table[(index / 16) * 33 + (index & 0xF) * 2];
            index = *in - 0x20;
            *out++ = g_cload_single_byte_char_table[(index / 16) * 33 + (index & 0xF) * 2 + 1];
            in += 1;
        }
        else
        {
            *out++ = g_cload_single_byte_char_table[0];
            *out++ = g_cload_single_byte_char_table[1];
            in += 1;
        }
    }
done:
    *out = 0;
}
