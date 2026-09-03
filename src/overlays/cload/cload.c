#include "common.h"
#include "gpu_packet.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

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

/**
 * @brief Element-pool head viewed as the CD-load prompt element: the 0x0 state
 *        word split into its state/phase/x/code bitfields, plus the 0x4
 *        active/y sub-fields and the 0x8 draw callback.
 */
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
    void *draw;
    s32 unused;
} CloadPromptElement;

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

/**
 * @brief 0x20-byte, word-aligned memory-card path scratch buffer.
 * The first six bytes are initialized from the "bu00:" device prefix before a
 * filename suffix is appended.
 */
typedef union
{
    u8 bytes[0x20];
    u32 align;
} CloadCardPathScratch;

/** @brief 0x68-byte, word-aligned scratch buffer used by cload_advance_load_sequence. */
typedef union
{
    u8 bytes[0x68];
    u32 align;
} CloadLoadScratch;

/** @brief 0x100-byte, word-aligned memory-card path buffer. */
typedef union
{
    u8 bytes[0x100];
    u32 align;
} CloadCardPathBuffer;

/** @brief 0x10-byte, word-aligned buffer initialized from the "bu00:*" search path. */
typedef union
{
    u8 bytes[0x10];
    u32 align;
} CloadCardSearchPathBuffer;

/* CLOAD layout/state constants. */
#define CLOAD_ELEMENT_COUNT 8
#define CLOAD_ELEMENT_WORD_STRIDE 3
#define CLOAD_ELEMENT_STATE_MASK 7
#define CLOAD_ELEMENT_PHASE_MASK 0x78
#define CLOAD_ENTRY_GROUP_COUNT 8
#define CLOAD_CARD_DIRECTORY_BYTES 0x320
#define CLOAD_DIRECTORY_ENTRY_BYTES 0x28
#define CLOAD_ENTRY_ROW_HEIGHT 14
#define CLOAD_NO_ICON 0x7F
#define CLOAD_GLYPH_CACHE_SLOTS 0x100
#define CLOAD_GLYPH_CACHE_COLUMNS 16
#define CLOAD_GLYPH_CACHE_ROW_MASK 0xF0
#define CLOAD_GLYPH_RASTER_BYTES 0x80
#define CLOAD_GPU_ADDR_MASK 0xFFFFFF
#define CLOAD_COLOR_WHITE 0xFFFFFF
#define CLOAD_GPU_TAG_HIGH_MASK 0xFF000000

#define CLOAD_GLYPH_SYM(sym, off) ((void *)(((u8 *)&(sym) - (off)) + (sym)))
#define CLOAD_GLYPH_OFF(base, off) ((void *)((base) + *(u16 *)((base) + (off))))

/** @brief Generic GPU packet prefix used while advancing the primitive buffer. */
typedef struct
{
    /* 0x0 */ s32 tag;
    /* 0x4 */ s32 word4;
    /* 0x8 */ s16 x0;
    /* 0xA */ s16 y0;
    /* 0xC */ s16 unkC;
    /* 0xE */ u16 unkE;
} CloadGpuPacket;

/** @brief Draw context at arg0 + 0x40; head_tag is the ordering-table head. */
typedef struct
{
    /* 0x0 */ s32 head_tag;
} CloadOrderingTable;

/** @brief Owning screen state passed to the element tick/draw pass. */
typedef struct
{
    u8 pad0[0x40B2];
    /* 0x40B2 */ s16 frame_flag;
    u8 pad40B4[4];
    /* 0x40B8 */ CloadGpuPacket *prim_cursor;
} CloadFrameState;

/** @brief Per-element draw callback stored at element + 8. */
typedef CloadGpuPacket *(*CloadElementDrawFunc)();

/**
 * @brief Same word/half raw-store granularity as POLY_G4 in
 *        include/psyq/libgpu.h; the r/g/b/code quad and each x/y pair are
 *        written as single word/half stores (matching the target's
 *        codegen), not per-channel byte assignments.
 */
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
} CloadPolyG4Packet;

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
    /* 0x1E */ s16 pad1;
    /* 0x20 */ s16 x3;
    /* 0x22 */ s16 y3;
    /* 0x24 */ u8 u3;
    /* 0x25 */ u8 v3;
} CloadPolyFT4Packet;

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
 * @brief 0x14-byte glyph packet: a Psy-Q SPRT_16 plus the trailing word that
 *        keeps consecutive cached-glyph packets 20 bytes apart.
 */
typedef struct
{
    /* 0x00 */ SPRT_16 packet;
    /* 0x10 */ u32 padding;
} CloadGlyphSprite;

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
extern s32 g_save_slot_index;
extern char D_800ECF7C[];
extern char g_cload_entries[];
extern u8 D_80162C5F;
extern s32 g_cload_entry_scan_active;
extern char D_800ECF8C[];
extern char D_800ECFC4[];
extern u16 g_cload_text_check_memory_card;
extern u16 g_cload_text_not_enough_blocks;
extern u16 g_cload_text_no_memory_card;
extern u16 g_cload_text_mana;
extern u16 g_cload_text_other_game;
extern u16 g_cload_text_card_slot_1;
extern u16 g_cload_text_card_slot_2;
extern u16 g_cload_text_card_access_failed;
extern u16 g_cload_text_no_save_data;
extern u16 g_cload_text_new_save;
extern u16 g_cload_text_new_save_prompt;
extern u16 g_cload_text_load;
extern u16 g_cload_text_number_prefix;
extern u16 g_cload_text_load_prompt;
extern u16 g_cload_text_loading;
extern u8 g_cload_save_blob[];
extern u8 g_menuLayoutBuffer[];
extern s32 D_80042FB4;
extern s32 g_cload_progress_bar_active;
extern s32 g_cload_progress_start_tick;
extern s32 g_cload_dialog_state;
extern u16 g_cload_text_no_lom_save_data;
extern u16 D_80145ED6;
extern u16 g_cload_text_save_failed;
extern u16 g_cload_text_load_failed;
extern u16 g_cload_text_card_insert_error;
extern u16 D_80145EDE;
extern u16 g_cload_text_version_error;
extern u16 D_80145F4C;
extern s32 g_cload_rank_count;
extern s32 g_cload_entry_ranks[];
extern s32 g_cload_entry_suffix_values[];
extern u8 D_800EC3F6[2];
extern u16 D_80146338[];

/* Globals used by the memory-card I/O, load-state, and glyph-cache block. */
extern u8 D_800EC3FA[];
extern u8 D_800ECF9C;
extern u8 D_800ECFB0;
extern u8 D_80146528;
extern u8 D_80146538[];
extern u8 D_80162C90[];
extern CloadCardPathBuffer g_cload_card_path_prefix;
extern CloadCardSearchPathBuffer g_cload_card_search_path;
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
s32 func_800A8A78(s32 *ot, s32 prim, s32 ch, s32 a3, DVECTOR *pos, s32 mode);
u8 *cload_skip_hex_digits(void *);
void cload_terminate_multibyte_text(void *text);
s32 cload_draw_icon_highlight(s32 prim, s32 *ot, s32 x, s32 y, s32 highlight, s32 icon, s32 index, s32 row);
s32 cload_draw_cached_text(s32 prim, s32 *ot, u8 *text, s32 x, s32 y, s32 palette, s32 alignment);
void cload_reset_glyph_cache(void);
void cload_begin_glyph_cache_frame(void);
void cload_evict_unused_glyphs(void);

/* External callees used by the memory-card I/O/load-state block. */
/* strncmp is declared above with the original visible signature. */
s32 open(void *, s32);
s32 read(s32, void *, s32);
s32 close(s32);
void erase(void *);
s32 _card_info(s32);
s32 _card_load(s32);
s32 _card_wait(s32);
s32 _card_clear(s32);
s32 func_80032174(s32, void *, s32 *);
s32 McxCardType(s32);
void EnterCriticalSection(void);
void CloseEvent(s32);
void ExitCriticalSection(void);
s32 OpenEvent(u32, s32, s32, s32);
void EnableEvent(s32);
s32 TestEvent(s32);
s32 firstfile(void *, void *);
s32 nextfile(void *);
s32 strcpy(void *, void *, u8);

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
    ClearImage(&rect, 0, 0, 0);
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
    u8 *frame;
    u8 *ordering_table;
    s32 buffer_index;
    s32 dpad_input;

    DrawSync(0);
    VSync(0);
    rect.x = 0;
    rect.y = 0;
    rect.w = 0x140;
    rect.h = 0x1D8;
    ClearImage(&rect, 0, 0, 0);
    frame = D_8014A988;
    buffer_index = 0;
    ClearOTagR(frame + 0x40, 0x1000);
    ClearOTagR(frame + 0x7D04, 0x1000);
    PutDispEnv(frame + 0x4040);
    update_controllers();
    SetDispMask(1);
    do
    {
        ordering_table = frame + 0x40;
        ClearOTagR(ordering_table, 0x1000);
        *(u8 **)(frame + 0x40B8) = D_8015A350 + (buffer_index << 14);
        func_800A9E78();
        dpad_input = g_pad_input & 0xF000;
        if (dpad_input != 0)
        {
            g_pad_input = dpad_input;
        }
        func_80067BBC(frame);
        if (cload_update_frame(frame) != 0)
        {
            break;
        }
        DrawSync(0);
        set_controller_vsync_interval(2);
        VSync(2);
        ClearImage(frame + 0x40B0, 0, 0, 0);
        buffer_index = 0;
        if (frame == D_8014A988)
        {
            frame += 0x7CC4;
            buffer_index = 1;
        }
        else
        {
            frame = D_8014A988;
        }
        PutDispEnv(frame + 0x4040);
        PutDrawEnv(frame + 0x4054);
        DrawOTag(ordering_table + 0x3FFC);
        update_controllers();
        cdrom_process_state();
    } while (1);
    reset_controller_vsync_state();
    VSync(0);
}

/**
 * @brief Initialize the CLOAD display and draw buffers.
 * @see decomp.me (100.00%)
 */
void cload_init_display(void)
{
    u8 *display_rect;
    s16 *second_display_rect;

    /* Preserve GCC 2.7.2's original stack-frame bucket without a dead call. */
    s32 stack_frame_pad[2];
    SetGeomScreen(0x5DC);
    SetGeomOffset(0xA0, 0x78);
    D_8014EA38 = 0;
    display_rect = (u8 *)&D_8014EA38;
    second_display_rect = (s16 *)(display_rect + 0x7CC4);
    *(s16 *)(display_rect + 0x2) = 0;
    *(s16 *)(display_rect + 0x4) = 0x140;
    *(s16 *)(display_rect + 0x6) = 0xF0;
    *(s16 *)(display_rect + 0x7CC4) = 0;
    second_display_rect[1] = 0xE8;
    second_display_rect[2] = 0x140;
    second_display_rect[3] = 0xF0;
    DrawSync(0);
    VSync(0);
    SetDefDispEnv(display_rect - 0x70, 0, 0, 0x140, 0xF0);
    SetDefDispEnv(display_rect + 0x7C54, 0, 0xE8, 0x140, 0xF0);
    SetDefDrawEnv(display_rect - 0x5C, 0, 0xF0, 0x140, 0xE0);
    SetDefDrawEnv(display_rect + 0x7C68, 0, 0x8, 0x140, 0xE0);
    display_rect[0x7C7E] = 0;
    display_rect[-0x46] = 0;
    func_80067B8C();
    func_80067EB4(0x100, 0x100, 0x100, 0x14);
}

/**
 * @brief Advance one CLOAD menu frame and report whether it should exit.
 * @param frame_state Current double-buffered CLOAD frame state.
 * @return 1 when the overlay should exit, otherwise 0.
 * @see decomp.me (100.00%)
 */
s32 cload_update_frame(s32 frame_state)
{
    if (g_cload_exit_requested != 0)
    {
        cload_shutdown_stream_handles();
        field_text_reset_windows();
        DrawSync(0);
        return 1;
    }
    field_text_reset_scratch();
    cload_begin_glyph_cache_frame();
    cload_update_menu(frame_state);
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
 * @note The unreachable cload_clear_elements(0, 0, 0, 0, 0) call preserves
 *       GCC 2.7.2's original stack-frame allocation (FRAME-04) without emitting
 *       a live dead call.
 * @see decomp.me (100.00%)
 */
void cload_build_ui_elements(void)
{
    CloadElement *element;

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
    g_cload_element_pool.first_state = (g_cload_element_pool.first_state & ~CLOAD_ELEMENT_STATE_MASK) | 1;

    element = cload_alloc_element();
    element->draw = cload_draw_entry_list;
    element->state = (((element->state & ~CLOAD_ELEMENT_PHASE_MASK) | 8) & 0xFFFF007F) | 0xE00;
    *((u8 *)element + 2) = 0x4A;
    element->size_flags = element->size_flags & ~0x200;
    element->state = (element->state & 0xFFFFFF) | 0x08000000;
    element->size_flags = (((element->size_flags | 1) & ~0x1FE) | 0x92);

    element = cload_alloc_element();
    element->draw = cload_draw_header_label;
    element->state = (((((element->state & ~CLOAD_ELEMENT_STATE_MASK) | 2) & ~CLOAD_ELEMENT_PHASE_MASK) | 8) & 0xFFFF007F) | 0x2800;
    *((u8 *)element + 2) = 0xC;
    element->size_flags = element->size_flags & ~0x200;
    element->state = (element->state & 0xFFFFFF) | 0xA0000000;
    element->size_flags = element->size_flags & ~1;
    element->size_flags = element->size_flags & ~0x1FE;
    element->size_flags = element->size_flags | 0x1E;

    element = cload_alloc_element();
    element->state = (((((element->state & ~CLOAD_ELEMENT_STATE_MASK) | 2) & ~CLOAD_ELEMENT_PHASE_MASK) | 8) & 0xFFFF007F) | 0xC00;
    element->draw = cload_draw_card_slot0_label;
    *((u8 *)element + 2) = 0x2C;
    element->size_flags = element->size_flags & ~0x200;
    element->state = (element->state & 0xFFFFFF) | 0x80000000;
    element->size_flags = element->size_flags & ~1;
    element->size_flags = element->size_flags & ~0x1FE;
    element->size_flags = element->size_flags | 0x1E;

    element = cload_alloc_element();
    element->state = (((((element->state & ~CLOAD_ELEMENT_STATE_MASK) | 2) & ~CLOAD_ELEMENT_PHASE_MASK) | 8) & 0xFFFF007F) | 0x5400;
    element->draw = cload_draw_card_slot1_label;
    *((u8 *)element + 2) = 0x2C;
    element->size_flags = element->size_flags & ~0x200;
    element->state = (element->state & 0xFFFFFF) | 0x80000000;
    element->size_flags = element->size_flags & ~1;
    element->size_flags = element->size_flags & ~0x1FE;
    element->size_flags = element->size_flags | 0x1E;

    element = cload_alloc_element();
    element->draw = cload_draw_selected_entry_details;
    element->state = (((((element->state & ~CLOAD_ELEMENT_STATE_MASK) | 2) & ~CLOAD_ELEMENT_PHASE_MASK) | 8) & 0xFFFF007F) | 0xF00;
    *((u8 *)element + 2) = 0xA0;
    element->size_flags = element->size_flags & ~0x200;
    element->state = (element->state & 0xFFFFFF) | 0x04000000;
    element->size_flags = (((element->size_flags | 1) & ~0x1FE) | 0x66);

    g_cload_element_pool.first_state &= ~CLOAD_ELEMENT_STATE_MASK;
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
 * @note The load-prompt element is set up through the CloadPromptElement
 *       bitfield view (phase/x/code plus the 0x4 active/y sub-fields), and the
 *       up/down navigation reads g_pad_input directly inside the count loop, so
 *       the selected-row arithmetic materializes in the target's registers.
 * @see decomp.me (100.00%)
 */
s32 cload_handle_input(void)
{
    s32 pending;
    s32 status;
    s32 count;
    s32 sfx_id;
    CloadPromptElement *prompt;

    if ((g_cload_element_pool.second_state & CLOAD_ELEMENT_STATE_MASK) == 0)
    {
        g_cload_exit_requested = 1;
        return;
    }
    if (g_cload_exit_requested != 0)
    {
        return;
    }
    if ((g_cload_element_pool.second_state & CLOAD_ELEMENT_STATE_MASK) >= 3)
    {
        return;
    }
    if ((g_cload_element_pool.first_state & CLOAD_ELEMENT_STATE_MASK) != 0)
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
    while (count != 0)
    {
        if (g_pad_input & 0x1000)
        {
            g_cload_selected_row -= 1;
            if (g_cload_selected_row < 0)
            {
                g_cload_selected_row = g_cload_entry_state - 1;
            }
        }
        if (g_pad_input & 0x4000)
        {
            g_cload_selected_row += 1;
            if (g_cload_selected_row >= g_cload_entry_state)
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
        if (strncmp(D_800ECF7C,
                &((CloadDirEntry (*)[20])g_cload_entries)[g_cload_card_slot][g_cload_selected_row], 0xC) != 0)
        {
            sfx_id = 0x78;
        }
        else
        {
            if ((D_80162C5F == g_save_slot_index) || (D_80162C5F == 0xFF))
            {
                prompt = (CloadPromptElement *)cload_alloc_element(D_80162C5F);
                prompt->draw = cload_draw_load_prompt;
                prompt->attr.f.phase = 1;
                prompt->attr.f.x = 0x10;
                prompt->attr.f.code = 0x5B;
                prompt->active = 1;
                prompt->y = 0x2B;
                prompt->attr.word = (prompt->attr.word & 0x00FFFFFF) | 0x20000000;
                cload_enable_choice_toggle();
                cload_restart_load_sequence();
                sfx_id = 0x7E;
            }
            else
            {
                sfx_id = 0x78;
            }
        }
        func_800A3938(sfx_id, 0x80);
    }
}

/**
 * @brief Put every live UI element into its closing state.
 * @see decomp.me (100.00%)
 */
void cload_close_all_elements(void)
{
    s32 state_word;
    s32 element_index;
    s32 *element_word;
    s32 closing_state;

    element_word = (s32 *)&g_cload_element_pool;
    element_index = 0;
    do
    {
        state_word = *element_word;
        if (state_word & CLOAD_ELEMENT_STATE_MASK)
        {
            closing_state = (state_word & ~CLOAD_ELEMENT_STATE_MASK) | 3;
            *element_word = (closing_state & ~CLOAD_ELEMENT_PHASE_MASK) | 0x40;
        }
        element_index += 1;
        element_word += CLOAD_ELEMENT_WORD_STRIDE;
    } while (element_index < CLOAD_ELEMENT_COUNT);
}

/**
 * @brief Move the list scroll target to keep the selected row visible.
 * @see decomp.me (100.00%)
 */
void cload_scroll_to_selection(void)
{
    s32 base;
    s32 delta;

    base = g_cload_selected_row * CLOAD_ENTRY_ROW_HEIGHT;
    delta = base - g_cload_scroll_y;
    if (delta >= 0x3C)
    {
        g_cload_scroll_target_y = base - 0x38;
        g_cload_scroll_frames = 4;
    }
    if (delta < 0)
    {
        g_cload_scroll_target_y = g_cload_selected_row * CLOAD_ENTRY_ROW_HEIGHT;
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

/**
 * @brief Draw the visible save-entry list and selection cursor.
 * @param ot Ordering-table head.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 * @note Menu string/glyph-row drawing callback (state-dispatched TILE + text
 *       renderer). The row loop is a `do { } while (i < g_cload_entry_state)`
 *       guarded by `if (state > 0)` with `row_y`/`i` hoisted to the default
 *       block, the rank-marker glyph offsets are materialized through a `u16
 *       misc_glyph` intermediate, and entry comparisons use strncmp - the
 *       shapes the target's register assignment requires.
 * @see decomp.me (100.00%)
 */
s32 cload_draw_entry_list(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    s32 state = g_cload_entry_state;

    switch (state)
    {
    case 0xF8:
        do { prim = func_800A88A0(prim, ot, CLOAD_GLYPH_SYM(g_cload_text_no_lom_save_data, 0x34), 1, -x_offset + 0x84, -y_offset, 2); } while (0);
        break;
    case 0xF9:
        prim = func_800A88A0(prim, ot, CLOAD_GLYPH_SYM(g_cload_text_no_lom_save_data, 0x34), 1, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFA:
        prim = func_800A88A0(prim, ot, CLOAD_GLYPH_SYM(g_cload_text_not_enough_blocks, 2), 1, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFD:
        prim = func_800A88A0(prim, ot, CLOAD_GLYPH_SYM(g_cload_text_no_memory_card, 4), 1, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFB:
        prim = func_800A88A0(prim, ot, CLOAD_GLYPH_SYM(g_cload_text_card_access_failed, 0x10), 1, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFC:
        prim = func_800A88A0(prim, ot, CLOAD_GLYPH_SYM(g_cload_text_no_save_data, 0x12), 1, -x_offset + 0x84, -y_offset, 2);
        break;
    default:
        {
            s32 row_y;
            s32 i;

        if (g_cload_entry_scan_active != 0)
        {
            s32 x;
            u8 *base;
        case 0xFF:
            x = -x_offset + 0x84;
            base = (u8 *)&g_cload_text_check_memory_card;
            prim = func_800A88A0(prim, ot, base + g_cload_text_check_memory_card, 1, x, -y_offset, 2);
            prim = func_800A88A0(prim, ot, CLOAD_GLYPH_OFF(base, 0x1E), 1, x, 0xE - y_offset, 2);
            prim = func_800A88A0(prim, ot, CLOAD_GLYPH_OFF(base, 0xB2), 1, x, 0x1C - y_offset, 2);
            break;
        }
        i = 0;
        if (state > 0)
        {
            s32 off;
            s32 base_x;
            s32 *flag_ptr;
            u16 misc_glyph;
            char *entry;
            DVECTOR pos;
            u8 *base;

            off = i;
            base_x = -x_offset;
            base = (u8 *)&g_cload_text_check_memory_card;
            entry = g_cload_entries;
            off = i;
            do
            {
                row_y = ((i * CLOAD_ENTRY_ROW_HEIGHT) - y_offset) - g_cload_scroll_y;
                if ((u32)(row_y + 0xD) < 0x56U)
                {
                    flag_ptr = (s32 *)((u8 *)g_cload_entry_ranks + off);
                    if (*flag_ptr >= 0)
                    {
                        pos.vx = base_x + 0x86;
                        pos.vy = row_y;
                        prim = func_800A88A0(func_800A8A78(ot, prim, *(s32 *)((u8 *)g_cload_entry_suffix_values + off), 1, &pos, 0), ot, (void *)((s32)g_cload_text_number_prefix + (s32)base), 1, base_x + 0x70, row_y, 0);
                        if ((g_cload_rank_count - 1) == *flag_ptr)
                        {
                            misc_glyph = *(u16 *)(base + 0x36);
                            prim = func_800A88A0(prim, ot, (void *)((s32)misc_glyph + (s32)base), 1, base_x + 0xC2, row_y, 0);
                        }
                        else if (*flag_ptr < 2)
                        {
                            misc_glyph = *(u16 *)(base + 0x38);
                            prim = func_800A88A0(prim, ot, (void *)((s32)misc_glyph + (s32)base), 1, base_x + 0xC2, row_y, 0);
                        }
                        if (*cload_skip_hex_digits((void *)((g_cload_card_slot * CLOAD_CARD_DIRECTORY_BYTES) + (s32)entry + 0xC)) == 0x2B)
                        {
                            prim = func_800A88A0(prim, ot, (void *)((s32)D_80145F4C + (s32)base), 1, 0xF8 - x_offset, row_y, 1);
                        }
                    }
                    if (strncmp(D_800ECF7C, (char *)((g_cload_card_slot * CLOAD_CARD_DIRECTORY_BYTES) + (s32)entry), 0xC) == 0)
                    {
                        prim = func_800A88A0(prim, ot, (void *)((s32)g_cload_text_mana + (s32)base), 1, base_x, row_y, 0);
                    }
                    else if (strncmp(D_800ECF8C, (char *)((g_cload_card_slot * CLOAD_CARD_DIRECTORY_BYTES) + (s32)entry), 0xC) == 0)
                    {
                        prim = func_800A88A0(prim, ot, (void *)((s32)D_80145ED6 + (s32)base), 1, base_x, row_y, 0);
                    }
                    else if (strncmp(D_800ECFC4, (char *)((g_cload_card_slot * CLOAD_CARD_DIRECTORY_BYTES) + (s32)entry), 8) == 0)
                    {
                        prim = func_800A88A0(prim, ot, (void *)((s32)g_cload_text_new_save + (s32)base), 1, base_x, row_y, 0);
                    }
                    else
                    {
                        prim = func_800A88A0(prim, ot, (void *)((s32)g_cload_text_other_game + (s32)base), 1, base_x, row_y, 0);
                    }
                }
                entry += CLOAD_DIRECTORY_ENTRY_BYTES;
                off += 4;
                i++;
            } while (i < g_cload_entry_state);
        }
            row_y = ((g_cload_selected_row * CLOAD_ENTRY_ROW_HEIGHT) - y_offset) - g_cload_scroll_y;

            if (g_cload_entry_scan_active == 0)
            {
                TILE *tile = (TILE *)prim;

                *(u32 *)&tile->r0 = 0xF080F0;
                *((u8 *)tile + 3) = 3;
                setcode(tile, 0x62);
                tile->y0 = (s16)(row_y - 1);
                tile->w = 0x108;
                tile->x0 = 0;
                tile->h = 0xE;
                addPrim(ot, tile);
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
 * @see decomp.me (100%)
 */
u8 *cload_skip_hex_digits(void *text)
{
    u8 *p = (u8 *)text;
    u32 c;

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

    return func_800A88A0(prim, ot, CLOAD_GLYPH_SYM(g_cload_text_load, 0x2C), 1, -x_offset + 0x50, -y_offset, 2);
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
    glyph = CLOAD_GLYPH_SYM(g_cload_text_card_slot_1, 0xC);
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
    glyph = CLOAD_GLYPH_SYM(g_cload_text_card_slot_2, 0xE);
    if (g_cload_card_slot == 0)
    {
        a3 = 3;
    }
    return func_800A88A0(prim, ot, glyph, a3, -x_offset + 0x40, -y_offset, 2);
}

/** @brief Fallback save-name view: 0x24-byte header then a 0x20-byte text field. */
typedef struct CloadFallbackTextMatch
{
    u8 pad[0x24];
    u8 text[0x20];
} CloadFallbackTextMatch;

/**
 * @brief Draw metadata for the selected save entry.
 * @param ot Ordering-table head.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 * @note Save-slot HUD callback: draws either the elapsed-play-time display
 *       (hours:minutes plus a 3-memcard-icon highlight strip) when the slot
 *       name matches the empty-slot marker, or the slot's save-file name
 *       otherwise. The icon-highlight loop keeps the "entries seen so far"
 *       count (i) and the raw slot index (j) as two separate locals, and the
 *       fallback-text branch wraps its two copy loops in the target's nested
 *       do/while(0) cross-jump shells.
 * @see decomp.me (100.00%)
 */
s32 cload_draw_selected_entry_details(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    s32 result;
    DVECTOR pos;
    u8 name[0x21];
    char unused_pad[212];
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

            result = func_800A88A0(prim, ot, CLOAD_GLYPH_SYM(g_cload_text_new_save_prompt, 0x28), 1, x, -y_offset, 0);
            base = (u8 *)&g_cload_text_new_save_prompt - 0x28;
            return func_800A88A0(result, ot, CLOAD_GLYPH_OFF(base, 0x2A), 1, x, 0x10 - y_offset, 0);
        }
        else
        {
            s32 term1 = g_cload_card_slot * CLOAD_CARD_DIRECTORY_BYTES;
            s32 term2 = (g_cload_selected_row * CLOAD_DIRECTORY_ENTRY_BYTES) + (s32)g_cload_entries;

            if (strncmp(D_800ECF7C, (void *)(term1 + term2), 0xC) == 0)
            {
                u8 *base90 = g_cload_entry_metadata;

                if (base90[0xCF] == 0xFF || base90[0xCF] == g_save_slot_index)
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

                    total = 0;
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

                            if ((g_cload_icon_phase >= base_y && g_cload_icon_phase < base_x && (delta = g_cload_icon_phase - base_y, 1))
                                || (rem = base_x % (half_step * present_count), g_cload_icon_phase >= rem && g_cload_icon_phase < (hi = rem + half_step) && (delta = hi - g_cload_icon_phase, 1)))
                            {
                                adjust += delta;
                            }
                            result = cload_draw_icon_highlight(result, ot, total - x_offset, -y_offset, adjust, slot[j], i, j);
                            total += adjust;
                            i += 1;
                        }
                    }

                    {
                        u8 *base90_2 = g_cload_entry_metadata;
                        s32 x = -x_offset;
                        s32 y = -y_offset;

                        base_y = *(s32 *)(base90_2 + 0x30);

                        pos.vx = (s16)(x + 0x70);
                        pos.vy = (s16)y;
                        hours = base_y / 216000;
                        result = func_800A8A78(ot, result, hours, 1, &pos, 1);
                        result = func_800A88A0(result, ot, D_800EC3F6[0] + ((s32)&D_800EC3F6 - 0x32) + (D_800EC3F6[1] << 8), 1, x + 0x6F, y, 0);
                        base_y = (base_y / 3600) - (hours * 0x3C);
                        if (base_y < 0xA)
                        {
                            pos.vx = (s16)(x + 0x7D);
                            pos.vy = (s16)y;
                            result = func_800A8A78(ot, result, 0, 1, &pos, 1);
                        }
                        pos.vx = (s16)(x + 0x85);
                        pos.vy = (s16)y;
                        result = func_800A88A0(func_800A88A0(func_800A8A78(ot, result, base_y, 1, &pos, 1), ot, base90_2, 1, x + 0x54, y + 0x10, 0), ot, CLOAD_GLYPH_OFF((u8 *)D_80146338, (*(s32 *)(base90_2 + 0x20) & 0x3FFFF) * 2), 1, x + 0x54, y + 0x20, 0);
                    }
                }
                else
                {
                    result = func_800A88A0(result, ot, CLOAD_GLYPH_SYM(g_cload_text_version_error, 0x54), 1, -x_offset, -y_offset, 0);
                }
            }
            else
            {
                s32 j;

                {
                    u8 *text_base;
                    cload_terminate_multibyte_text(&D_80162A14);
                    text_base = (u8 *)&D_80162A14;
                    text_base -= 4;
                    if ((u32)(text_base[0x24] - 1) >= 0x7FU)
                    {
                        do {
                        do {
                        do {

                        for (j = 0; j < 0x20; j++)
                        {
                            name[j] = text_base[j + 4];
                        }
                    name[j] = 0;
                    result = cload_draw_cached_text(result, ot, name, -x_offset, -y_offset, 1, 0);

                    for (j = 0; j < 0x20; j++)
                    {
                        name[j] = ((CloadFallbackTextMatch *)&D_80162A10)->text[j];
                    }
                    name[j] = 0;
                        result = cload_draw_cached_text(result, ot, name, -x_offset, -y_offset + 0x10, 1, 0);
                        } while (0);
                        } while (0);
                        } while (0);
                    }
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
    for (i = 0; i < CLOAD_ELEMENT_COUNT; i++)
    {
        p->state &= ~CLOAD_ELEMENT_STATE_MASK;
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
    for (i = 0; i < CLOAD_ELEMENT_COUNT; i++, p++)
    {
        if ((p->state & CLOAD_ELEMENT_STATE_MASK) == 0)
        {
            p->size_flags |= 0x200;
            p->state = (p->state & ~CLOAD_ELEMENT_STATE_MASK) | 1;
            return p;
        }
    }
    return (CloadElement *)&g_cload_element_pool;
}

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
 * @param frame Owning screen state; prim_cursor is read on entry and written back on exit.
 *
 * @note The `volatile u32 *element_state` view forces the target's repeated
 *       loads of the element word (gcc CSEs some away without it), and the
 *       `do { ... } while (0)` wrappers around the primitive-cursor advance and
 *       the case-4 state-word load are [ALLOC-23] loop-note ref bumps that pin
 *       the s-register assignment. The case 1/3 tails materialize the growth
 *       and shrink arithmetic through the target's exact per-register temps.
 * @see decomp.me (100.00%)
 */
void cload_update_and_draw_elements(CloadFrameState *frame)
{
    CloadGpuPacket *prim;
    CloadOrderingTable *ot;
    volatile u32 *element_state;
    s32 scaled_width;
    s32 scaled_height;
    s32 element_index;
    s32 draw_area[24];
    u32 state_word;
    u32 dispatch_word;
    s32 state;
    u32 size_flags;
    u32 full_width;
    s32 phase;
    s32 width_product;
    s32 full_height;
    s32 height_product;
    s32 height_delta;
    u32 opening_word;
    u32 updated_state;
    s32 closing_phase;
    s32 closing_width_product;
    s32 closing_height;
    s32 closing_height_product;
    s32 closing_height_delta;
    u32 closing_word;
    u32 hold_word;
    u32 n_temp_a0_2;
    s32 n_temp_v1_2;
    u32 n_temp_a1;
    u32 n_temp_a2;
    s32 n_temp_a0_3;
    s32 n_var_v1;
    s32 n_temp_a3_2;
    s32 n_var_v0;
    s32 n_temp_a3_3;
    u32 n_temp_v0_3;
    u32 n_temp_a0_4;
    s32 n_temp_a0_5;
    s32 n_var_v1_2;
    s32 n_temp_a3_5;
    s32 n_var_v0_2;
    s32 n_temp_a3_6;
    u32 n_temp_v0_5;
    u32 n_temp_v1_3;

    prim = frame->prim_cursor;
    ot = (CloadOrderingTable *)((u8 *)frame + 0x40);

    if ((g_cload_entry_state < 0x10) && ((g_cload_element1_state & CLOAD_ELEMENT_STATE_MASK) == 2))
    {
        if ((g_cload_entry_state * CLOAD_ENTRY_ROW_HEIGHT) > (g_cload_scroll_y + 0x49))
        {
            prim = cload_emit_scroll_arrow(prim, ot, 0x114, 0x87, 0);
        }
        if (g_cload_scroll_y != 0)
        {
            prim = cload_emit_scroll_arrow(prim, ot, 0x114, 0x4A, 1);
        }
    }

    if (frame->frame_flag != 0)
    {
        SetDefDrawEnv(draw_area, 0, 0xF0, 0x140, 0xE0);
    }
    else
    {
        SetDefDrawEnv(draw_area, 0, 8, 0x140, 0xE0);
    }

    element_state = (volatile u32 *)&g_cload_element_pool;
    element_index = 0;

    for (; element_index < CLOAD_ELEMENT_COUNT; element_index++, element_state += CLOAD_ELEMENT_WORD_STRIDE)
    {
        state_word = *element_state;
        if (state_word & CLOAD_ELEMENT_STATE_MASK)
        {
            SetDrawEnv((s32)prim, draw_area);

            addPrim(&ot->head_tag, prim);

            dispatch_word = *element_state;
            state = dispatch_word & CLOAD_ELEMENT_STATE_MASK;

            /* GCC 2.7.2 allocation boundary; removing this wrapper changes s-register assignment. */
            do
            {
                prim = (CloadGpuPacket *)((u8 *)prim + 0x40);
            } while (0);

            switch (state)
            {
            case 1:
                n_temp_v0_3 = *element_state;
                n_temp_a1 = *(u32 *)((u8 *)element_state + 4);
                n_temp_a0_4 = n_temp_v0_3 >> 24;
                n_temp_a2 = ((n_temp_a1 & 1) << 8) | n_temp_a0_4;
                n_temp_a0_3 = (n_temp_v0_3 >> 3) & 0xF;
                n_var_v1 = n_temp_a2 * n_temp_a0_3;
                g_pad_input = 0;
                if (n_var_v1 < 0)
                {
                    n_var_v1 += 7;
                }
                n_temp_a3_2 = (n_temp_a1 >> 1) & 0xFF;
                n_var_v0 = n_temp_a3_2 * n_temp_a0_3;
                scaled_width = n_var_v1 >> 3;
                if (n_var_v0 < 0)
                {
                    n_var_v0 += 7;
                }
                scaled_height = n_var_v0 >> 3;
                n_temp_a3_3 = (s32)(n_temp_a3_2 - scaled_height);

                prim = (*(CloadElementDrawFunc *)((u8 *)element_state + 8))(ot, prim, (s32)(n_temp_a2 - scaled_width) / 2, n_temp_a3_3 / 2);
                {
                    u32 post_word;
                    u32 field;
                    u32 high;
                    post_word = *element_state;
                    field = (post_word >> 7) & 0x1FF;
                    high = post_word >> 24;
                    prim = cload_emit_window_frame(prim, ot,
                                           field + (s32)((((*(u32 *)((u8 *)element_state + 4) & 1) << 8) | high) - scaled_width) / 2,
                                           (*((u8 *)element_state + 2)) + ((s32)((*(u32 *)((u8 *)element_state + 4) >> 1) & 0xFF) - scaled_height) / 2,
                                           scaled_width, scaled_height, frame->frame_flag, (*(u32 *)((u8 *)element_state + 4) >> 9) & 1);
                }
                {
                    u32 old_word;
                    u32 new_word;
                    old_word = *element_state;
                    new_word = (old_word & ~CLOAD_ELEMENT_PHASE_MASK) | (((((old_word >> 3) & 0xF) + 1) & 0xF) * 8);
                    *element_state = new_word;
                    if (((new_word >> 3) & 0xF) == 8)
                    {
                        func_800AA02C();
                        *element_state = (*element_state & ~CLOAD_ELEMENT_STATE_MASK) | 2;
                    }
                }
                break;

            case 2:
                prim = (*(CloadElementDrawFunc *)((u8 *)element_state + 8))(ot, prim, 0, 0);
                {
                    u32 case_word;
                    u32 high;
                    case_word = *element_state;
                    high = case_word >> 24;
                    prim = cload_emit_window_frame(prim, ot, (case_word >> 7) & 0x1FF, (*((u8 *)element_state + 2)),
                                           ((*(u32 *)((u8 *)element_state + 4) & 1) << 8) | high,
                                           (*(u32 *)((u8 *)element_state + 4) >> 1) & 0xFF, frame->frame_flag,
                                           (*(u32 *)((u8 *)element_state + 4) >> 9) & 1);
                }
                hold_word = *element_state;
                if (((hold_word >> 3) & 0xF) != 0)
                {
                    *(u32 *)element_state = (hold_word & ~CLOAD_ELEMENT_PHASE_MASK) | (((((hold_word >> 3) & 0xF) - 1) & 0xF) * 8);
                }
                break;

            case 3:
                n_temp_a0_5 = (s32)*element_state;
                n_temp_a1 = *(u32 *)((u8 *)element_state + 4);
                n_var_v1_2 = (u32)n_temp_a0_5 >> 24;
                n_temp_a2 = ((n_temp_a1 & 1) << 8) | n_var_v1_2;
                n_temp_a0_5 = (u32)n_temp_a0_5 >> 3;
                n_temp_a0_5 &= 0xF;
                n_var_v1_2 = n_temp_a2 * n_temp_a0_5;
                g_pad_input = 0;
                if (n_var_v1_2 < 0)
                {
                    n_var_v1_2 += 7;
                }
                n_temp_a3_5 = (n_temp_a1 >> 1) & 0xFF;
                n_var_v0_2 = n_temp_a3_5 * n_temp_a0_5;
                scaled_width = n_var_v1_2 >> 3;
                if (n_var_v0_2 < 0)
                {
                    n_var_v0_2 += 7;
                }
                scaled_height = n_var_v0_2 >> 3;
                n_temp_a3_6 = (s32)(n_temp_a3_5 - scaled_height);

                prim = (*(CloadElementDrawFunc *)((u8 *)element_state + 8))(ot, prim, (s32)(n_temp_a2 - scaled_width) / 2, n_temp_a3_6 / 2);
                {
                    u32 post_word;
                    u32 field;
                    u32 high;
                    post_word = *element_state;
                    field = (post_word >> 7) & 0x1FF;
                    high = post_word >> 24;
                    prim = cload_emit_window_frame(prim, ot,
                                           field + (s32)((((*(u32 *)((u8 *)element_state + 4) & 1) << 8) | high) - scaled_width) / 2,
                                           (*((u8 *)element_state + 2)) + ((s32)((*(u32 *)((u8 *)element_state + 4) >> 1) & 0xFF) - scaled_height) / 2,
                                           scaled_width, scaled_height, frame->frame_flag, (*(u32 *)((u8 *)element_state + 4) >> 9) & 1);
                }
                {
                    u32 exiting_word;
                    exiting_word = *element_state;
                    closing_width_product = exiting_word & ~CLOAD_ELEMENT_PHASE_MASK;
                    closing_width_product |= (((((exiting_word >> 3) & 0xF) - 1) & 0xF) * 8);
                    *(u32 *)element_state = (u32)closing_width_product;
                    if (!(((u32)closing_width_product >> 3) & 0xF))
                    {
                        *element_state = ((((u32)closing_width_product & ~CLOAD_ELEMENT_PHASE_MASK) | 0x18) & ~CLOAD_ELEMENT_STATE_MASK) | 4;
                    }
                }
                break;

            case 4:
                {
                    s32 *pad_ptr;
                    pad_ptr = &g_pad_input;
                    do
                    {
                        closing_word = *element_state;
                    } while (0);
                    *pad_ptr = 0;
                }
                hold_word = (closing_word & ~CLOAD_ELEMENT_PHASE_MASK) | (((((closing_word >> 3) & 0xF) - 1) & 0xF) * 8);
                *(u32 *)element_state = hold_word;
                if (!((hold_word >> 3) & 0xF))
                {
                    *element_state = hold_word & ~CLOAD_ELEMENT_STATE_MASK;
                }
                break;
            }
        }
    }

    frame->prim_cursor = cload_emit_icon_highlight_strip(prim, ot);
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
    CloadGpuPacket *outline_prim;
    DR_TPAGE *draw_mode_prim;
    CloadGpuPacket *frame_prim;
    CloadGpuPacket *next_prim;
    s32 draw_area[24];
    s32 draw_y;

    frame_prim = prim;
    if (flag != 0)
    {
        draw_y = y + 0xF2;
        SetDefDrawEnv(draw_area, x + 2, draw_y, w - 4, h - 3);
    }
    else
    {
        draw_y = y + 0xA;
        SetDefDrawEnv(draw_area, x + 2, draw_y, w - 4, h - 3);
    }
    SetDrawEnv((s32)frame_prim, draw_area);

    addPrim(ot, frame_prim);

    next_prim = (CloadGpuPacket *)((u8 *)frame_prim + 0x40);
    if (draw_fill != 0)
    {
        outline_prim = cload_emit_rect_outline(next_prim, ot, x, y, w, h, CLOAD_COLOR_WHITE);
        outline_prim = cload_emit_rect_outline(outline_prim, ot, x + 1, y + 1, w - 2, h - 2, 0);
        outline_prim = cload_emit_rect_outline(outline_prim, ot, x - 1, y - 1, w + 2, h + 2, 0);

        next_prim = outline_prim;
        SET_BGR0_PACKED((TILE *)next_prim, GPU_TINT_NEUTRAL);
        setTile((TILE *)next_prim);
        setSemiTrans((TILE *)next_prim, 1);
        setXY0((TILE *)next_prim, x, y);
        setWH((TILE *)next_prim, w, h);
        addPrim(ot, (TILE *)next_prim);

        draw_mode_prim = (DR_TPAGE *)((u8 *)next_prim + sizeof(TILE));
        setDrawTPage(draw_mode_prim, 0, 0, 0x45);
        addPrim(ot, draw_mode_prim);
        next_prim = (CloadGpuPacket *)((u8 *)draw_mode_prim + sizeof(DR_TPAGE));
    }
    return next_prim;
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

    p->word4 = color;
    setlen(p, 3);
    setcode(p, 0x40);
    p->x0 = x;
    p->y0 = y;
    p->unkC = x + w;
    p->unkE = y;
    tmp = CLOAD_GPU_TAG_HIGH_MASK;
    p->tag = (p->tag & CLOAD_GPU_TAG_HIGH_MASK) | (*ot & CLOAD_GPU_ADDR_MASK);
    *ot = (*ot & tmp) | ((s32)p & CLOAD_GPU_ADDR_MASK);
    p++;

    p->word4 = color;
    setlen(p, 3);
    setcode(p, 0x40);
    p->x0 = x + w;
    p->y0 = y;
    p->unkC = x + w;
    p->unkE = y + h;
    addPrim(ot, p);
    p++;

    p->word4 = color;
    setlen(p, 3);
    setcode(p, 0x40);
    p->x0 = x + w;
    tmp = y + h;
    p->y0 = tmp;
    p->unkC = x;
    p->unkE = y + h;
    addPrim(ot, p);
    p++;

    p->word4 = color;
    setlen(p, 3);
    setcode(p, 0x40);
    p->x0 = x;
    p->y0 = y;
    p->unkC = x;
    p->unkE = y + h;
    addPrim(ot, p);
    return p + 1;
}

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
    DR_TPAGE *draw_mode;

    p->word4 = 0x808080;
    /* GCC 2.7.2 scheduling boundary; removing this wrapper changes packet-store order. */
    do
    {
        setlen(p, 4);
        setcode(p, 0x64);
        p->x0 = x;
        p->y0 = y;
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
    addPrim(ot, p);

    p = (CloadGpuPacket *)((u8 *)p + 0x14);
    draw_mode = (DR_TPAGE *)p;
    setDrawTPage(draw_mode, 0, 0, 0x8A);
    addPrim(ot, draw_mode);
    draw_mode++;
    return (CloadGpuPacket *)draw_mode;
}

/**
 * @brief Draw the CD-load prompt glyph then set up the driver/GPU-packet state,
 *        branching on the CD status (cload_poll_and_rewind_primary_handles) and
 *        the g_pad_input flags.
 * @param ot Ordering-table head threaded through the glyph/line draws.
 * @param prim Primitive buffer for the glyph draw (func_800A88A0).
 * @param x_offset Base for the row x-coordinate (-x_offset + 0x90).
 * @param y_offset Row delta applied to the draw extents.
 * @return The CloadGpuPacket* chain pointer returned by cload_draw_choice_prompt.
 * @see decomp.me (100.00%)
 */
s32 cload_draw_load_prompt(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    s32 result;
    s32 x;
    s32 status;
    CloadPromptElement *p;

    x = -x_offset + 0x90;
    result = cload_draw_choice_prompt(
        func_800A88A0(prim, ot,
                      (u8 *)&g_cload_text_load_prompt + g_cload_text_load_prompt - 0x30,
                      4, x, -y_offset, 2),
        ot, x, 0xE - y_offset);

    if ((u32)(cload_poll_and_rewind_primary_handles() - 1) < 2U)
    {
        ((CloadPromptElement *)&g_cload_element_pool)->attr.f.state = 0;
        func_800AA02C();
        func_800A3938(0x78, 0x80);
        g_cload_entry_state = 0xFF;
        cload_reset_entry_ranks();
        g_cload_load_step = 0;
    }
    else
    {
        status = g_pad_input;
        if (status & 0x40)
        {
            ((CloadPromptElement *)&g_cload_element_pool)->attr.f.state = 0;
            func_800AA02C();
            func_800A3938(0x78, 0x80);
            g_cload_load_step = D_80146534;
        }
        else if (status & 0x220)
        {
            if (g_cload_choice_toggle != 0)
            {
                ((CloadPromptElement *)&g_cload_element_pool)->attr.f.state = 0;
                func_800AA02C();
                func_800A3938(0x78, 0x80);
                g_cload_load_step = D_80146534;
            }
            else
            {
                func_800A3938(0x7E, 0x80);
                g_cload_progress_active = 1;
                g_cload_load_step = D_8014653C;
                p = (CloadPromptElement *)&g_cload_element_pool;
                p->draw = cload_draw_load_progress;
                p->attr.f.phase = 1;
                p->attr.f.state = 1;
                p->attr.f.x = 0x10;
                p->attr.f.code = 0x5B;
                p->active = 1;
                p->y = 0x2B;
                p->attr.word = (p->attr.word & 0x00FFFFFF) | ((u32)0x20 << 24);
            }
        }
    }
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

    /* Preserve GCC 2.7.2's original stack-frame bucket without a dead call. */
    s32 frame_scratch[2];

    y = -x_offset + 0x90;
    result = func_800A88A0(prim, (s32 *)ot, (void *)((s32)&g_cload_text_loading - 0x32 + g_cload_text_loading), 4, y, -y_offset, 2);
    base = (u8 *)&g_cload_text_loading - 0x32;
    result = func_800A88A0(result, (s32 *)ot, CLOAD_GLYPH_OFF(base, 0x1E), 4, y, 0xE - y_offset, 2);
    result = func_800A88A0(result, (s32 *)ot, CLOAD_GLYPH_OFF(base, 0xB2), 4, y, 0x1C - y_offset, 2);
    result = cload_draw_progress_bar(result, ot);

    if (g_cload_progress_active == 0)
    {
        p = g_cload_save_blob;
        vol = 0x80;
        base = p;
        if (cload_validate_save_blob(base) == 0)
        {
            cload_open_status_dialog(4);
        }
        else
        {
            func_800A3938(0x7B, vol);
            g_cload_element_pool.first_state = g_cload_element_pool.first_state & ~CLOAD_ELEMENT_STATE_MASK;
            bcopy(base + 0x180, g_menuLayoutBuffer, 0x3268);
            g_save_slot_index = g_menuLayoutBuffer[0xCF];
            D_80042FB4 = VSync(-1);
            g_cload_exit_requested = 1;
        }
    }
    return result;
}

/**
 * @brief Build the CD-load progress-bar gouraud quad (a POLY_G4-shaped
 *        packet) and link it into the ordering table.
 * @param p packet cursor to write the quad into.
 * @param ot ordering-table head threaded through the OT-link update.
 * @return the advanced packet cursor (p + 0x24), or p unchanged if
 *         g_cload_progress_bar_active is 0.
 * @note g_cload_progress_bar_active gates the whole body; when set, `elapsed`
 *       (VSync(-1) - g_cload_progress_start_tick, clamped to 0x100) scales into
 *       the quad's right-edge x extent (elapsed * 0x120, rounded via +0xFF for
 *       negative values, then >>8) - a time-based progress bar.
 * @see decomp.me (100.00%)
 */
CloadGpuPacket *cload_draw_progress_bar(CloadGpuPacket *p, s32 *ot)
{
    CloadPolyG4Packet *g;
    s32 elapsed;
    s32 extent;
    s32 color;

    g = (CloadPolyG4Packet *)p;
    if (g_cload_progress_bar_active != 0)
    {
        elapsed = VSync(-1) - g_cload_progress_start_tick;
        if (elapsed >= 0x101)
        {
            elapsed = 0x100;
        }
        color = 0xFFFF00;
        extent = elapsed * 0x120;
        g->color0 = 0xFF;
        g->color1 = 0xFFFF;
        g->color3 = 0xFF0000;
        setlen(g, 8);
        g->color2 = color;
        setcode(g, 0x38);
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
        g->y3 = 0x35;
        g->y2 = 0x35;
        addPrim(ot, p);
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
    g_cload_element_pool.first_state = ((((g_cload_element_pool.first_state & ~CLOAD_ELEMENT_PHASE_MASK) | 8) & ~CLOAD_ELEMENT_STATE_MASK | 1) & 0xFFFF007F | 0x1000) & 0xFFFFFF;
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
 * @see decomp.me (100%)
 */
s32 cload_draw_status_dialog(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    s32 state = g_cload_dialog_state;
    u8 *base;

    /* Preserve GCC 2.7.2's original stack-frame bucket without a dead call. */
    s32 frame_scratch[2];

    switch (state)
    {
    case 0:
        prim = func_800A88A0(prim, ot, CLOAD_GLYPH_SYM(g_cload_text_save_failed, 0x3C), 4, -x_offset + 0x80, -y_offset, 2);
        base = (u8 *)&g_cload_text_save_failed - 0x3C;
        prim = func_800A88A0(prim, ot, CLOAD_GLYPH_OFF(base, 0x56), 4, -x_offset + 0x80, -y_offset + 0x10, 2);
        break;
    case 1:
        prim = func_800A88A0(prim, ot, CLOAD_GLYPH_SYM(g_cload_text_load_failed, 0x3E), 4, -x_offset + 0x80, -y_offset, 2);
        base = (u8 *)&g_cload_text_load_failed - 0x3E;
        prim = func_800A88A0(prim, ot, CLOAD_GLYPH_OFF(base, 0x56), 4, -x_offset + 0x80, -y_offset + 0x10, 2);
        break;
    case 2:
        prim = func_800A88A0(prim, ot, CLOAD_GLYPH_SYM(g_cload_text_card_insert_error, 0x40), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 3:
        prim = func_800A88A0(prim, ot, CLOAD_GLYPH_SYM(D_80145EDE, 0x42), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 4:
        prim = func_800A88A0(prim, ot, CLOAD_GLYPH_SYM(g_cload_text_load_failed, 0x3E), 4, -x_offset + 0x80, -y_offset, 2);
        base = (u8 *)&g_cload_text_load_failed - 0x3E;
        prim = func_800A88A0(prim, ot, CLOAD_GLYPH_OFF(base, 0x5C), 4, -x_offset + 0x80, -y_offset + 0x10, 2);
        break;
    }

    if (g_pad_input & 0x220)
    {
        g_cload_element_pool.first_state &= ~CLOAD_ELEMENT_STATE_MASK;
        func_800AA02C();
    }

    return prim;
}

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
 *         icon == CLOAD_NO_ICON.
 * @see decomp.me (100.00%)
 */
s32 cload_draw_icon_highlight(s32 prim, s32 *ot, s32 x, s32 y, s32 highlight, s32 icon, s32 index, s32 row)
{
    RECT rect;
    s32 base;
    s8 uv;

    if (icon == CLOAD_NO_ICON)
    {
        return prim;
    }

    rect.x = index * 0x10;
    rect.y = 0x1F2;
    rect.w = 0x10;
    rect.h = 1;
    if ((row == 1) && (icon < 2))
    {
        func_800A5638(&g_cload_icon_context, icon);
        LoadImage(&rect, &g_cload_icon_context);
        DrawSync(0);
    }
    else if (icon >= 0x4F)
    {
        func_800A55E4(&g_cload_icon_context, g_cload_icon_palette);
        LoadImage(&rect, &g_cload_icon_context);
        DrawSync(0);
    }
    else
    {
        LoadImage(&rect, g_cload_icon_resource + *(s32 *)(g_cload_icon_resource + icon * 4 + 4));
    }

    base = index * 3;
    rect.x = base * 4 + 0x140;
    rect.y = 0xD0;
    rect.w = 0xC;
    rect.h = 0x30;
    LoadImage(&rect, g_cload_icon_resource + *(s32 *)(g_cload_icon_resource + icon * 4 + 4) + 0x20);

    ((CloadPolyFT4Packet *)prim)->color0 = 0x808080;
    setlen(prim, 9);
    setcode(prim, 0x2C);
    ((CloadPolyFT4Packet *)prim)->x2 = x;
    ((CloadPolyFT4Packet *)prim)->x0 = x;
    ((CloadPolyFT4Packet *)prim)->y1 = y;
    ((CloadPolyFT4Packet *)prim)->y0 = y;
    ((CloadPolyFT4Packet *)prim)->x3 = x + highlight;
    uv = base * 0x10;
    ((CloadPolyFT4Packet *)prim)->u2 = uv;
    ((CloadPolyFT4Packet *)prim)->u0 = uv;
    uv += 0x2F;
    ((CloadPolyFT4Packet *)prim)->u3 = uv;
    ((CloadPolyFT4Packet *)prim)->u1 = uv;
    ((CloadPolyFT4Packet *)prim)->v1 = 0xD0;
    ((CloadPolyFT4Packet *)prim)->v0 = 0xD0;
    ((CloadPolyFT4Packet *)prim)->x1 = x + highlight;
    ((CloadPolyFT4Packet *)prim)->y3 = y + 0x2F;
    ((CloadPolyFT4Packet *)prim)->y2 = y + 0x2F;
    ((CloadPolyFT4Packet *)prim)->v3 = 0xFF;
    ((CloadPolyFT4Packet *)prim)->v2 = 0xFF;
    ((CloadPolyFT4Packet *)prim)->clut = (index & 0x3F) | 0x7C80;
    ((CloadPolyFT4Packet *)prim)->tpage = 5;
    addPrim(ot, prim);

    return prim + 0x28;
}

/**
 * @brief Deactivate the first UI element.
 * @see decomp.me (100.00%)
 */
void cload_deactivate_primary_element(void)
{
    g_cload_element_pool.first_state = g_cload_element_pool.first_state & ~CLOAD_ELEMENT_STATE_MASK;
}

/**
 * @brief Load the CD icon-set resource into the 0x80180000 scratch buffer and
 *        register its three icon glyphs (codes 0x200/0x240/0x280) with ids
 *        0x1F4/0x1F5/0x1F6 via func_80086374, also pointing g_cload_icon_resource (the
 *        icon UV table used by cload_draw_icon_highlight) at the resource's first offset
 *        field.
 * @note The four header reads use raw offsets off (u8 *)0x80180000 so gcc shares a
 *       single `lui 0x8018`. Do not give the fields extern symbols: every such shape
 *       costs an extra addiu plus a saved register (72.76-88.64%). D_80180004/8/C/10
 *       are marked `ignore:true` in config/symbols/cload_symbol_addrs.txt to stop
 *       splat symbolizing them in the target asm.
 * @see decomp.me (100.00%)
 */
void cload_load_icon_resources(void)
{
    u8 *base;
    s16 buf[4];

    cdrom_queue_read(0x5E4, (void *)0x80180000);
    cdrom_wait_queue_empty();

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
 * @note Every per-slot value is derived from the loop index (i * 0x80 + 8 for x,
 *       i * 0x40 + 0x7D00 for the row code, i * 0x40 + 0x200 for the texture page)
 *       rather than carried in separate accumulators; the accumulator form ties up
 *       an extra register against the P_TAG len mask and costs the match.
 * @see decomp.me (100.00%)
 */
CloadGpuPacket *cload_emit_icon_highlight_strip(CloadGpuPacket *p, CloadOrderingTable *ot)
{
    s32 i;
    s32 tpage;

    for (i = 0; i < 3; i++)
    {
        p->word4 = 0x808080;
        setlen(p, 4);
        setcode(p, 0x64);
        p->x0 = (i * 0x80) + 8;
        p->y0 = 0;
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
        p->unkE = (i * 0x40) + 0x7D00;
        addPrim(ot, p);
        p = (CloadGpuPacket *)((u8 *)p + 0x14);
        setlen(p, 1);
        tpage = ((((i * 0x40) + 0x200) & 0x3FF) >> 6) | 0xE1000080;
        p->word4 = tpage;
        addPrim(ot, p);
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
 * @see decomp.me (100.00%)
 */
s32 cload_draw_choice_prompt(s32 prim, s32 *ot, s32 x, s32 y)
{
    u8 *p;
    u8 *base;
    s32 glyph0;
    s32 glyph1;
    s32 hi_byte;
    s32 mode;

    p = (u8 *)&D_800EC3FA;
    hi_byte = p[1] << 8;
    base = p - 0x36;
    mode = 4;
    glyph0 = p[0] + (hi_byte + (s32)base);
    if (g_cload_choice_toggle != 0)
    {
        mode = 5;
    }
    prim = func_800A88A0(prim, ot, (void *)glyph0, mode, x - 0x10, y, 1);

    mode = 4;
    glyph1 = base[0x38] + ((base[0x39] << 8) + (s32)base);
    if (g_cload_choice_toggle == 0)
    {
        mode = 5;
    }
    prim = func_800A88A0(prim, ot, (void *)glyph1, mode, x + 8, y, 0);

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
 * @see decomp.me (100.00%)
 */
void cload_format_hex(s8 *out, s32 value, s32 max_chars)
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
                cload_hex_nibble_to_ascii(cursor, nibble);
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
 * @param unused1 Unused ABI argument.
 * @param unused2 Unused ABI argument.
 * @return The value of the (at most two) hex digits found after the skipped run.
 * @note The `text--; if (text) { text++; text--; }` sequences in the skip loop are
 *       required to match: without them gcc's jump.c cross-jumps the three continue
 *       arms into a single back-edge and the function loses 6 instructions.
 * @note The accumulation uses an explicit temp so K is folded with the running
 *       value (result - K) rather than with the freshly read digit.
 * @see decomp.me (100.00%)
 */
s32 cload_parse_hex_suffix_byte(u8 *text, s32 unused1, s32 unused2)
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


/**
 * @brief Parse the hex-string field of each table entry and record the results.
 *
 * Iterates over the g_cload_entry_state active entries. For each entry the 0x28-byte
 * record is validated with @ref strncmp against pattern D_800ECF7C. On a
 * match (return 0) the ASCII hex string at record offset 0xC is scanned for up
 * to five hex digits (0-9, A-F, a-f), accumulated big-endian into a value that
 * is written to the g_cload_entry_fields result array; @ref cload_parse_hex_suffix_byte is then invoked
 * with the field, the parsed value, and the remaining digit budget, its result
 * stored in the g_cload_entry_suffix_values array and folded into a running maximum. A failed
 * validation stores -1 / 0 into the two arrays instead.
 *
 * @return The largest value returned by any @ref cload_parse_hex_suffix_byte call (0 if none).
 * @note WIP match, 98.62% (gcc272_cdk).
 * @see decomp.me (98.62%)
 */
s32 cload_parse_entry_fields(void)
{
 typedef struct { u8 data[0x28]; } CloadEntry28;
 s32 i; s32 max; u8 *entry; u8 *p; u8 *field; s32 count; s32 acc; u32 tmp0; u32 tmp1; u32 tmp2; s32 r;
 i=0; max=i;
 while (i < g_cload_entry_state) {
    if (strncmp(&D_800ECF7C, (u8 *)&((CloadEntry28 (*)[20])g_cload_entries)[g_cload_card_slot][i], 0xC)==0) {
     count=5; p=(u8 *)(g_cload_card_slot*CLOAD_CARD_DIRECTORY_BYTES + ((i << 4) + (i << 4) + (i << 3)) + (s32)g_cload_entries + 0xC); acc=0;
     while (((u8)(*p-'0')<10)||((u8)(*p-'a')<6)||((u8)(*p-'A')<6)) { if(count==0)break; acc<<=4; if((u8)(*p-'0')<10){tmp0=acc-0x30;acc=tmp0+*p;} else if((u8)(*p-'A')<6){tmp1=acc-0x37;acc=tmp1+*p;} else if((u8)(*p-'a')<6){tmp2=acc-0x57;acc=tmp2+*p;} p++;count--; }
     field=(u8 *)&((CloadEntry28 (*)[20])g_cload_entries)[g_cload_card_slot][i].data[0xC];
     { s32 addr; addr=g_cload_card_slot*0x50+(s32)g_cload_entry_fields; *(s32 *)(addr+i*4)=acc; }
     r=cload_parse_hex_suffix_byte(field,acc,count); g_cload_entry_suffix_values[i]=r; if(max<r)max=r;
    } else { { s32 addr; addr=g_cload_card_slot*0x50+(s32)g_cload_entry_fields; *(s32 *)(addr+i*4)=-1; } g_cload_entry_suffix_values[i]=0; }
   i++;
 }
 return max;
}


/**
 * @brief Rank the current page's entries and select the highest-scoring slot.
 * @param unused0 Unused; present only to match the caller's 3-argument ABI.
 * @param unused1 Unused; present only to match the caller's 3-argument ABI.
 * @param unused2 Unused; present only to match the caller's 3-argument ABI.
 * @return Index of the entry holding the maximum value.
 * @see decomp.me (100.00%)
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
        slot = g_cload_card_slot;
        field1 = g_cload_entry_fields;
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
    g_cload_rank_count = t0v;
    t0v = -1;
    i = 0;
    s3v = 0;
    if (g_cload_entry_state > 0)
    {
        s32 max_count;
        max_count = g_cload_entry_state;
        slot = g_cload_card_slot;
        field_base = g_cload_entry_fields;
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
    g_cload_entry_value_limit = t0v + 1;
    if (g_cload_entry_state > 0)
    {
        out_ptr = &g_cload_entry_suffix_values[0];
        ent_ptr = &g_cload_entries[0];
    loop_20:
        if (strncmp(&D_800ECFC4[0], (void *)((g_cload_card_slot * CLOAD_CARD_DIRECTORY_BYTES) + (s32)ent_ptr), 8) == 0)
        {
            *out_ptr = handle + 1;
        }
        else
        {
            out_ptr += 1;
            ent_ptr += CLOAD_DIRECTORY_ENTRY_BYTES;
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
 * @brief Scan up to g_cload_entry_state entries of the g_cload_entries table (row
 *        selected by g_cload_card_slot, stride 0x28) and report whether any entry
 *        matches one of the two known-type patterns D_800ECF7C / D_800ECF8C.
 * @return 1 on the first entry that matches either pattern (strncmp returns 0
 *         on a match), 0 if no entry matches.
 * @note The entry address is recomputed from the index each iteration rather than
 *       carried as an incremented pointer; that is what puts the g_cload_card_slot
 *       %hi ahead of the g_cload_entries lui/addiu in the loop preheader.
 * @see decomp.me (100.00%)
 */
s32 cload_has_known_entry_type(void)
{
    s32 i;

    i = 0;
    if (g_cload_entry_state > 0)
    {
        do
        {
            if (strncmp(&D_800ECF7C, &((CloadDirEntry (*)[20])g_cload_entries)[g_cload_card_slot][i], 0xC) == 0 ||
                strncmp(&D_800ECF8C, &((CloadDirEntry (*)[20])g_cload_entries)[g_cload_card_slot][i], 0xC) == 0)
            {
                return 1;
            }
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
 * @note The `i`/`sum` init order and the do/while(0) wrapper around the
 *       accumulate step reproduce the target's loop-body counter/accumulator
 *       register assignment.
 * @see decomp.me (100.00%)
 */
s32 cload_entry_blocks_reach_limit(void)
{
    s32 i;
    s32 sum;
    s32 offset;

    i = 0;
    sum = 0;
    if (g_cload_entry_state > 0)
    {
        offset = g_cload_card_slot * CLOAD_CARD_DIRECTORY_BYTES;
        do
        {
            do {
                sum += ((CloadDirEntry *)((u8 *)g_cload_entries + offset))->size / 8192;
            } while (0);
            i++;
            offset += CLOAD_DIRECTORY_ENTRY_BYTES;
        } while (i < g_cload_entry_state);
    }
    return sum >= 0xE;
}


/**
 * @brief Erase the two fixed per-slot memory-card files.
 * @note Each erase starts from the six-byte "bu00:" device path, adjusts the
 *       slot digit, appends one fixed filename suffix, and calls Psy-Q erase().
 * @see decomp.me (100.00%)
 */
void cload_erase_fixed_card_files(void)
{
    CloadCardPathScratch card_path;

    memcpy(&card_path, &g_cload_card_path_prefix, 6);
    ((u8 *)&card_path)[2] += *(u8 *)&g_cload_card_slot;
    strcat(&card_path, &D_800ECF9C);
    erase(&card_path);

    memcpy(&card_path, &g_cload_card_path_prefix, 6);
    ((u8 *)&card_path)[2] += *(u8 *)&g_cload_card_slot;
    strcat(&card_path, &D_800ECFB0);
    erase(&card_path);
}


/**
 * @brief Inlined copy of cload_erase_fixed_card_files used by load-sequence case 6.
 */
extern void *jtbl_80140040[];

static inline void cload_erase_fixed_card_files_inline(void)
{
    CloadCardPathScratch card_path;

    memcpy(&card_path, &g_cload_card_path_prefix, 6);
    ((u8 *)&card_path)[2] += *(u8 *)&g_cload_card_slot;
    strcat(&card_path, &D_800ECF9C);
    erase(&card_path);

    memcpy(&card_path, &g_cload_card_path_prefix, 6);
    ((u8 *)&card_path)[2] += *(u8 *)&g_cload_card_slot;
    strcat(&card_path, &D_800ECFB0);
    erase(&card_path);
}


/**
 * @brief Advance the cload overlay load/decompress state machine one step.
 * @return The next phase code (1-5) for the caller to act on.
 * @note Dispatches on *g_cload_load_step (the current step opcode); each case drives
 *       CD reads, buffer setup, decode, and teardown, updating g_cload_load_step and
 *       the g_cload_entry_state / g_cload_selection_status status fields.
 * @see decomp.me (100%)
 */
s32 cload_advance_load_sequence(void)
{
    CloadLoadScratch card_path;
    s32 status0;
    s32 status1;
    s32 phase_result;
    s32 scan_attempts;
    s32 wait_attempts;
    s32 poll_result;
    s32 poll_result20;
    s32 dialog_state;
    s32 rank_index;
    s32 rank_value;
    s32 dispatch;
    static void *const keep[] = {
        &&cl_case_0, &&cl_case_1, &&cl_case_2, &&cl_case_3,
        &&cl_case_4, &&cl_case_5, &&cl_case_6, &&block_81,
        &&cl_case_8, &&cl_case_9, &&block_81, &&block_81,
        &&block_81, &&block_81, &&block_81, &&cl_case_15,
        &&cl_case_16, &&cl_case_17, &&cl_case_18, &&cl_case_19,
        &&cl_case_20, &&block_81, &&block_81, &&block_81,
        &&cl_case_24, &&block_81, &&block_81, &&block_81,
        &&block_81, &&block_81, &&cl_case_30
    };

    memcpy(&card_path, &g_cload_card_path_prefix, 6);
    do
    {
        phase_result = 1;
    } while (0);
    ((u8 *)&card_path)[2] += *(u8 *)&g_cload_card_slot;

    if (g_cload_load_step != NULL)
    {
        switch (0)
        {
        case 0:
            dispatch = *g_cload_load_step;
            if ((u32)dispatch >= 0x1F)
            {
                goto block_81;
            }
            goto *jtbl_80140040[dispatch];

        cl_case_1:
            phase_result = 3;
            _card_wait(g_cload_card_slot);
            _card_info(g_cload_card_slot * 0x10);
            g_cload_load_step = g_cload_load_step + 1;
            goto block_81;

        cl_case_2:
            poll_result = cload_poll_primary_handle_group();
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
                goto block_increment;
            }
            goto block_81;
        c2_ge3:
            if (poll_result == 3)
            {
                goto c2_eq3;
            }
            goto block_81;
        c2_pos:
            do
            {
                phase_result = 4;
            } while (0);
            g_cload_selection_status = 0;
            g_cload_entry_state = 0xFD;
            g_cload_load_step = g_cload_load_step + 1;
            cload_deactivate_primary_element();
            goto block_81;
        c2_eq3:
            g_cload_rank_count = 0x28;
            rank_value = -1;
            for (rank_index = 14; rank_index >= 0; rank_index--)
            {
                g_cload_entry_ranks[rank_index] = rank_value;
            }
            goto block_58;

        cl_case_3:
            cload_release_primary_handles();
            g_cload_load_step = g_cload_load_step + 1;
            goto block_81;

        cl_case_4:
            do
            {
                poll_result = cload_poll_secondary_handle_group();
            } while (poll_result == -1);
            if (poll_result == 0)
            {
                g_cload_load_step = g_cload_load_step + 1;
                goto block_81;
            }
            if (poll_result < 0)
            {
                goto block_81;
            }
            if (poll_result >= 4)
            {
                goto block_81;
            }
            do
            {
                phase_result = 4;
            } while (0);
            goto block_42;

        cl_case_5:
            cload_release_secondary_handles();
            g_cload_load_step = g_cload_load_step + 1;
            goto block_81;

        cl_case_6:
            cload_erase_fixed_card_files_inline();
            g_cload_entry_scan_active = 1;
            if (cload_begin_entry_scan(g_cload_card_slot) == 0)
            {
                phase_result = 2;
                g_cload_entry_state = 0xF8;
                g_cload_load_step = NULL;
                g_cload_entry_scan_active = 0;
                goto block_81;
            }
            scan_attempts = 0;
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
                scan_attempts = scan_attempts + 1;
            } while (scan_attempts < 0x14);
            goto block_81;

        cl_case_8:
            phase_result = 3;
            _card_wait(g_cload_card_slot);
            _card_clear(g_cload_card_slot * 0x10);
            g_cload_load_step = g_cload_load_step + 1;
            goto block_81;

        cl_case_9:
            phase_result = 3;
            _card_wait(g_cload_card_slot);
            _card_load(g_cload_card_slot * 0x10);
            g_cload_primary_poll_countdown = 0x10;
            g_cload_secondary_poll_countdown = 0x10;
            g_cload_load_step = g_cload_load_step + 1;
            goto block_81;

        cl_case_0:
            phase_result = 2;
            D_80162370 = 0;
            goto block_81;

        cl_case_15:
            poll_result = cload_poll_primary_handle_group();
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
                goto block_increment;
            }
            goto block_81;
        c15_ge3:
            if (poll_result == 3)
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
            phase_result = 4;
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
            _card_wait(g_cload_card_slot);
            _card_clear(g_cload_card_slot * 0x10);
            _card_wait(g_cload_card_slot);
            _card_load(g_cload_card_slot * 0x10);
            goto block_81;
        c15_d70zero:
            phase_result = 5;
            g_cload_entry_state = 0xFC;
            g_cload_load_step = &D_80146528;
            goto block_81;

        cl_case_16:
            do
            {
                poll_result = cload_poll_secondary_handle_group();
            } while (poll_result == -1);
            g_cload_load_step = g_cload_load_step + 1;
            goto block_81;

        cl_case_17:
            g_cload_io_busy = 1;
            g_cload_selection_status = 0;
            _card_wait(g_cload_card_slot);
            g_cload_file_handle = open(&D_80162C90, 0x8001);
            if (g_cload_file_handle == -1)
            {
                goto block_81;
            }
            cload_release_primary_handles();
            _card_wait(g_cload_card_slot);
            if (read(g_cload_file_handle, &D_80162A10,
                               g_cload_selected_entry_extended != 0 ? 0x280 : 0x80) != -1)
            {
                g_cload_load_step = g_cload_load_step + 1;
                goto block_81;
            }
            close(g_cload_file_handle);
            goto block_81;

        cl_case_18:
            poll_result = cload_poll_primary_handle_group();
            if (poll_result == 0)
            {
                g_cload_io_busy = 0;
                g_cload_selection_status = 1;
                g_cload_load_step = g_cload_load_step + 1;
                close(g_cload_file_handle);
                goto block_81;
            }
            if (poll_result == -1)
            {
                goto block_81;
            }
            g_cload_io_busy = 0;
            close(g_cload_file_handle);
        block_58:
            g_cload_entry_state = 0xFF;
            g_cload_load_step = D_8014651C;
            goto block_81;

        cl_case_30:
            g_cload_retry_count = 5;
            g_cload_load_step = g_cload_load_step + 1;
            goto block_81;

        cl_case_19:
            g_cload_progress_active = 1;
            g_cload_progress_bar_active = 1;
            g_cload_progress_start_tick = VSync(-1);
            _card_wait(g_cload_card_slot);
            g_cload_file_handle = open(&D_80162C90, 0x8001);
            cload_release_primary_handles();
            _card_wait(g_cload_card_slot);
            if (read(g_cload_file_handle, &g_cload_save_blob, 0x4000) != -1)
            {
                g_cload_load_step = g_cload_load_step + 1;
                goto block_81;
            }
            close(g_cload_file_handle);
            g_cload_retry_count = g_cload_retry_count - 1;
            if (g_cload_retry_count == 0)
            {
                dialog_state = 1;
                goto block_80;
            }
            goto block_81;

        cl_case_20:
            poll_result20 = cload_poll_primary_handle_group();
            if (poll_result20 == 0)
            {
                g_cload_progress_active = 0;
                g_cload_load_step = g_cload_load_step + 1;
                close(g_cload_file_handle);
                goto block_81;
            }
            if (poll_result20 < 0)
            {
                goto block_81;
            }
            if (poll_result20 >= 4)
            {
                goto block_81;
            }
            g_cload_retry_count = g_cload_retry_count - 1;
            if (g_cload_retry_count == 0)
            {
                goto c20_378zero;
            }
            close(g_cload_file_handle);
            g_cload_load_step = g_cload_load_step - 1;
            goto block_81;
        c20_378zero:
            close(g_cload_file_handle);
            dialog_state = 1;
            g_cload_progress_bar_active = 0;
            goto block_80;

        cl_case_24:
            wait_attempts = 0;
            do
            {
                if (McxCardType(g_cload_card_slot * 0x10) == 1)
                {
                    break;
                }
                VSync(0);
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 0x14);
            if (wait_attempts != 0x14)
            {
                func_80032174(0, &status0, &status1);
                dialog_state = 3;
                if (status1 == 0)
                {
                block_increment:
                    g_cload_load_step = g_cload_load_step + 1;
                    goto block_81;
                }
                goto block_80;
            }
            dialog_state = 3;
            goto block_80;
        }
    }
    goto block_81;

block_80:
    cload_open_status_dialog(dialog_state);

block_81:
    return phase_result;
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
    _card_wait(g_cload_card_slot);
    _card_info(g_cload_card_slot * 0x10);
    g_cload_load_step = &D_80146528;
}


/**
 * @brief Poll the four cached handles; on completion, rewind the CD channel.
 * @return The busy-slot index from cload_poll_primary_handle_group (-1 when none are busy).
 * @see decomp.me (100.00%)
 */
s32 cload_poll_and_rewind_primary_handles(void)
{
    s32 busy_slot;

    busy_slot = cload_poll_primary_handle_group();
    if (busy_slot != -1)
    {
        _card_wait(g_cload_card_slot);
        _card_info(g_cload_card_slot * 0x10);
    }
    return busy_slot;
}


/**
 * @brief Allocate and register the eight streaming buffers for this overlay.
 * @note Brackets the eight OpenEvent allocations (handles stored in
 *       g_cload_primary_handle0..g_cload_secondary_handle3) with EnterCriticalSection / ExitCriticalSection and resets the
 *       stream bookkeeping (g_cload_entry_scan_active, g_cload_progress_start_tick, g_cload_progress_bar_active).
 * @see decomp.me (100.00%)
 */
void cload_init_stream_handles(void)
{
    reset_controller_vsync_state();
    EnterCriticalSection();
    g_cload_primary_handle0 = OpenEvent(0xF4000001, 4, 0x2000, 0);
    g_cload_primary_handle1 = OpenEvent(0xF4000001, 0x8000, 0x2000, 0);
    g_cload_primary_handle2 = OpenEvent(0xF4000001, 0x100, 0x2000, 0);
    g_cload_primary_handle3 = OpenEvent(0xF4000001, 0x2000, 0x2000, 0);
    g_cload_secondary_handle0 = OpenEvent(0xF0000011, 4, 0x2000, 0);
    g_cload_secondary_handle1 = OpenEvent(0xF0000011, 0x8000, 0x2000, 0);
    g_cload_secondary_handle2 = OpenEvent(0xF0000011, 0x100, 0x2000, 0);
    g_cload_secondary_handle3 = OpenEvent(0xF0000011, 0x2000, 0x2000, 0);
    EnableEvent(g_cload_primary_handle0);
    EnableEvent(g_cload_primary_handle1);
    EnableEvent(g_cload_primary_handle2);
    EnableEvent(g_cload_primary_handle3);
    EnableEvent(g_cload_secondary_handle0);
    EnableEvent(g_cload_secondary_handle1);
    EnableEvent(g_cload_secondary_handle2);
    EnableEvent(g_cload_secondary_handle3);
    ExitCriticalSection();
    g_cload_entry_scan_active = 0;
    g_cload_progress_start_tick = VSync(-1);
    g_cload_progress_bar_active = 0;
}


/**
 * @brief Tear down / release the eight g_cload_primary_handle0..g_cload_secondary_handle3 handles.
 * @note Wrapped by reset_controller_vsync_state and EnterCriticalSection/ExitCriticalSection bracket calls;
 *       each handle is passed to CloseEvent in turn (g_cload_entry_scan_active is skipped).
 * @see decomp.me (100.00%)
 */
void cload_shutdown_stream_handles(void)
{
    reset_controller_vsync_state();
    EnterCriticalSection();
    CloseEvent(g_cload_primary_handle0);
    CloseEvent(g_cload_primary_handle1);
    CloseEvent(g_cload_primary_handle2);
    CloseEvent(g_cload_primary_handle3);
    CloseEvent(g_cload_secondary_handle0);
    CloseEvent(g_cload_secondary_handle1);
    CloseEvent(g_cload_secondary_handle2);
    CloseEvent(g_cload_secondary_handle3);
    ExitCriticalSection();
}


/**
 * @brief Begin streaming the page's first g_cload_entries record.
 * @param page Page index (each page is 0x320 bytes in g_cload_entries).
 * @return 1 if firstfile accepted the record (count bumped), else 0.
 * @see decomp.me (100%)
 */
s32 cload_begin_entry_scan(s32 page)
{
    CloadCardSearchPathBuffer search_path;
    CloadCardSearchPathBuffer *search_template = &g_cload_card_search_path;

    memcpy(&search_path, search_template, 7);
    g_cload_scroll_frames = 0;
    g_cload_scroll_target_y = 0;
    g_cload_scroll_y = 0;
    g_cload_selected_row = 0;
    g_cload_entry_state = 0;
    ((u8 *)&search_path)[2] += page;
    if (firstfile(&search_path, g_cload_entries + page * CLOAD_CARD_DIRECTORY_BYTES) != 0)
    {
        g_cload_entry_state += 1;
        return 1;
    }
    return 0;
}


/**
 * @brief Try to append the page's next g_cload_entries record; if it cannot,
 *        recompute the page's fixed-point total and update the selection state.
 * @param page Page index (each page is 0x320 bytes / 20 records in g_cload_entries).
 * @return 1 if nextfile accepted the new record (count bumped), else 0.
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

    term1 = page * CLOAD_CARD_DIRECTORY_BYTES;
    term2 = g_cload_entry_state * CLOAD_DIRECTORY_ENTRY_BYTES + (s32)g_cload_entries;
    if (nextfile((void *)(term1 + term2)) != 0)
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
            offset = g_cload_card_slot * CLOAD_CARD_DIRECTORY_BYTES;
            do
            {
                sum += ((CloadDirEntry *)((u8 *)g_cload_entries + offset))->size / 8192;
                i++;
                offset += CLOAD_DIRECTORY_ENTRY_BYTES;
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
 *       its header into a local CloadCardPathBuffer, biases byte 2 by the page index, and
 *       registers it via strcpy; sets the g_cload_selection_status / g_cload_selected_entry_extended status.
 * @see decomp.me (100%)
 */
void cload_commit_selected_entry(void)
{
    CloadCardPathBuffer card_path;
    u8 *path;

    if (g_cload_entry_state == 0)
    {
        g_cload_selection_status = 3;
        return;
    }
    if (strncmp(&D_800ECFC4[0],
            &((CloadDirEntry (*)[20])g_cload_entries)[g_cload_card_slot][g_cload_selected_row], 8) == 0)
    {
        g_cload_selection_status = 2;
        return;
    }
    memcpy(&card_path, &g_cload_card_path_prefix, 6);
    path = (u8 *)&card_path;
    strcat(path, &((CloadDirEntry (*)[20])g_cload_entries)[g_cload_card_slot][g_cload_selected_row]);
    {
        s32 slot_index;
        s32 path_slot_digit;
        path_slot_digit = *((u8 *)&card_path + 2);
        slot_index = (u8)g_cload_card_slot;
        g_cload_selection_status = 0;
        path_slot_digit += slot_index;
        *((u8 *)&card_path + 2) = path_slot_digit;
        strcpy(&D_80162C90[0], path, slot_index);
    }
    g_cload_load_step = &D_80146538[0];
    if (strncmp(&D_800ECF7C[0],
            &((CloadDirEntry (*)[20])g_cload_entries)[g_cload_card_slot][g_cload_selected_row], 0xC) == 0)
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
 * (in that order) to @ref TestEvent.
 * @see decomp.me (100.00%)
 */
void cload_release_primary_handles(void)
{
    TestEvent(g_cload_primary_handle0);
    TestEvent(g_cload_primary_handle1);
    TestEvent(g_cload_primary_handle2);
    TestEvent(g_cload_primary_handle3);
}


/**
 * @brief Release the next four cached resource handles for this overlay.
 *
 * Passes the values held in g_cload_secondary_handle0, g_cload_secondary_handle1, g_cload_secondary_handle2, and g_cload_secondary_handle3
 * (in that order) to @ref TestEvent.
 * @see decomp.me (100.00%)
 */
void cload_release_secondary_handles(void)
{
    TestEvent(g_cload_secondary_handle0);
    TestEvent(g_cload_secondary_handle1);
    TestEvent(g_cload_secondary_handle2);
    TestEvent(g_cload_secondary_handle3);
}


/**
 * @brief Release four cached handles, returning the index of the first busy one.
 *
 * Passes each of g_cload_primary_handle0, g_cload_primary_handle1, g_cload_primary_handle2, g_cload_primary_handle3 to
 * @ref TestEvent in order; the first call that returns 1 stops the sequence
 * and yields that slot's index (0-3). Returns -1 if none report busy.
 *
 * @return Index 0-3 of the first handle whose release returned 1, else -1.
 * @see decomp.me (100.00%)
 */
s32 cload_poll_primary_handle_group(void)
{
    if (TestEvent(g_cload_primary_handle0) == 1)
    {
        return 0;
    }
    if (TestEvent(g_cload_primary_handle1) == 1)
    {
        return 1;
    }
    if (TestEvent(g_cload_primary_handle2) == 1)
    {
        return 2;
    }
    if (TestEvent(g_cload_primary_handle3) == 1)
    {
        return 3;
    }
    return -1;
}


/**
 * @brief Release four cached handles, returning the index of the first busy one.
 *
 * Passes each of g_cload_secondary_handle0, g_cload_secondary_handle1, g_cload_secondary_handle2, g_cload_secondary_handle3 to
 * @ref TestEvent in order; the first call that returns 1 stops the sequence
 * and yields that slot's index (0-3). Returns -1 if none report busy.
 *
 * @return Index 0-3 of the first handle whose release returned 1, else -1.
 * @see decomp.me (100.00%)
 */
s32 cload_poll_secondary_handle_group(void)
{
    if (TestEvent(g_cload_secondary_handle0) == 1)
    {
        return 0;
    }
    if (TestEvent(g_cload_secondary_handle1) == 1)
    {
        return 1;
    }
    if (TestEvent(g_cload_secondary_handle2) == 1)
    {
        return 2;
    }
    if (TestEvent(g_cload_secondary_handle3) == 1)
    {
        return 3;
    }
    return -1;
}


/**
 * @brief Collate the g_cload_entries page records, ordering them by pattern class.
 * @note Five passes bucket records matching D_800ECF7C, then D_800ECF8C, then
 *       D_800ECFC4, then the remainder, copying each 0x28-byte record with
 *       bcopy before writing the ordered set back to the page.
 * @see decomp.me (95.37%)
 */
void cload_sort_entries_by_type(void)
{
    u8 sorted_entries[CLOAD_CARD_DIRECTORY_BYTES];
    s32 output_count;
    s32 offset_tmp_0;
    s32 offset_tmp_1;
    s32 offset_tmp_2;
    s32 offset_tmp_3;

    s32 card_offset_0;
    s32 card_offset_1;
    s32 card_offset_2;
    s32 card_offset_3;
    s32 card_offset_4;
    s32 card_offset_5;
    s32 card_offset_6;
    s32 card_offset_7;
    s32 card_offset_8;
    s32 card_offset_9;
    s32 card_offset_10;

    s32 group;
    s32 first_index;
    s32 first_limit;
    u8 *first_entry;
    s32 *first_suffix;
    s32 *first_suffix_base;
    s32 output_offset;
    s32 second_index;
    s32 second_limit;
    u8 *second_entry;
    s32 *second_suffix;
    s32 *second_suffix_base;
    s32 third_index;
    s32 third_limit;
    u8 *third_entry;
    s32 other_index;
    s32 other_limit;
    u8 *other_entry;
    s32 *third_card_base;
    s32 *other_card_base;
    s32 *write_card_base;
    s32 write_index;
    u8 *page_entry;
    u8 *sorted_entry;

    output_count = 0;
    group = 0;
    first_suffix_base = &g_cload_entry_suffix_values;
    do
    {
        do { do { do { do { do { do { do { do { do { first_index = 0; } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0);
        if (g_cload_entry_state > 0)
        {
            first_entry = &g_cload_entries;
            first_suffix = first_suffix_base;
            offset_tmp_0 = output_count * 5;
            output_offset = offset_tmp_0 * 8;
            do
            {
                if (*first_suffix == group && strncmp(&D_800ECF7C, (card_offset_0 = g_cload_card_slot * CLOAD_CARD_DIRECTORY_BYTES, card_offset_0 += (s32)first_entry, (void *)card_offset_0), 0xC) == 0)
                {
                    bcopy((card_offset_1 = g_cload_card_slot * CLOAD_CARD_DIRECTORY_BYTES, card_offset_1 += (s32)first_entry, (void *)card_offset_1), &sorted_entries[output_offset], CLOAD_DIRECTORY_ENTRY_BYTES);
                    output_offset += CLOAD_DIRECTORY_ENTRY_BYTES;
                    output_count += 1;
                }
                do { first_entry += CLOAD_DIRECTORY_ENTRY_BYTES; } while (0);
                first_limit = g_cload_entry_state;
                first_index += 1;
                first_suffix += 1;
            } while (first_index < first_limit);
        }
        group += 1;
    } while (group < CLOAD_ENTRY_GROUP_COUNT);

    second_suffix_base = &g_cload_entry_suffix_values;
    group = 0;
    do
    {
        do { do { do { do { do { do { do { do { do { second_index = 0; } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0);
        if (g_cload_entry_state > 0)
        {
            second_entry = &g_cload_entries;
            second_suffix = second_suffix_base;
            offset_tmp_1 = output_count * 5;
            output_offset = offset_tmp_1 * 8;
            do
            {
                if (*second_suffix == group && strncmp(&D_800ECF8C, (card_offset_2 = g_cload_card_slot * CLOAD_CARD_DIRECTORY_BYTES, card_offset_2 += (s32)second_entry, (void *)card_offset_2), 0xC) == 0)
                {
                    bcopy((card_offset_3 = g_cload_card_slot * CLOAD_CARD_DIRECTORY_BYTES, card_offset_3 += (s32)second_entry, (void *)card_offset_3), &sorted_entries[output_offset], CLOAD_DIRECTORY_ENTRY_BYTES);
                    output_offset += CLOAD_DIRECTORY_ENTRY_BYTES;
                    output_count += 1;
                }
                do { second_entry += CLOAD_DIRECTORY_ENTRY_BYTES; } while (0);
                second_limit = g_cload_entry_state;
                second_index += 1;
                second_suffix += 1;
            } while (second_index < second_limit);
        }
        group += 1;
    } while (group < CLOAD_ENTRY_GROUP_COUNT);

    do { do { do { do { do { do { do { do { do { third_index = 0; } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0);
    if (g_cload_entry_state > 0)
    {
        third_card_base = &g_cload_card_slot;
        third_entry = &g_cload_entries;
        offset_tmp_2 = output_count * 5;
        output_offset = offset_tmp_2 * 8;
        do
        {
            if (strncmp(&D_800ECFC4, (card_offset_4 = (*third_card_base) * CLOAD_CARD_DIRECTORY_BYTES, card_offset_4 += (s32)third_entry, (void *)card_offset_4), 8) == 0)
            {
                bcopy((card_offset_5 = (*third_card_base) * CLOAD_CARD_DIRECTORY_BYTES, card_offset_5 += (s32)third_entry, (void *)card_offset_5), &sorted_entries[output_offset], CLOAD_DIRECTORY_ENTRY_BYTES);
                output_offset += CLOAD_DIRECTORY_ENTRY_BYTES;
                output_count += 1;
            }
            third_limit = g_cload_entry_state;
            third_index += 1;
            do { third_entry += CLOAD_DIRECTORY_ENTRY_BYTES; } while (0);
        } while (third_index < third_limit);
    }

    do { do { do { do { do { do { do { do { do { other_index = 0; } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0);
    if (g_cload_entry_state > 0)
    {
        other_card_base = &g_cload_card_slot;
        other_entry = &g_cload_entries;
        offset_tmp_3 = output_count * 5;
        output_offset = offset_tmp_3 * 8;
        do
        {
            if (strncmp(&D_800ECF7C, (card_offset_6 = (*other_card_base) * CLOAD_CARD_DIRECTORY_BYTES, card_offset_6 += (s32)other_entry, (void *)card_offset_6), 0xC) != 0 &&
                strncmp(&D_800ECF8C, (card_offset_7 = (*other_card_base) * CLOAD_CARD_DIRECTORY_BYTES, card_offset_7 += (s32)other_entry, (void *)card_offset_7), 0xC) != 0 &&
                strncmp(&D_800ECFC4, (card_offset_8 = (*other_card_base) * CLOAD_CARD_DIRECTORY_BYTES, card_offset_8 += (s32)other_entry, (void *)card_offset_8), 8) != 0)
            {
                bcopy((card_offset_9 = (*other_card_base) * CLOAD_CARD_DIRECTORY_BYTES, card_offset_9 += (s32)other_entry, (void *)card_offset_9), &sorted_entries[output_offset], CLOAD_DIRECTORY_ENTRY_BYTES);
                output_offset += CLOAD_DIRECTORY_ENTRY_BYTES;
            }
            other_limit = g_cload_entry_state;
            other_index += 1;
            do { other_entry += CLOAD_DIRECTORY_ENTRY_BYTES; } while (0);
        } while (other_index < other_limit);
    }

    write_index = 0;
    if (g_cload_entry_state > 0)
    {
        do { page_entry = &g_cload_entries; } while (0);
        write_card_base = &g_cload_card_slot;
        sorted_entry = &sorted_entries[0];
        do
        {
            bcopy(sorted_entry, (card_offset_10 = (*write_card_base) * CLOAD_CARD_DIRECTORY_BYTES, card_offset_10 += (s32)page_entry, (void *)card_offset_10), CLOAD_DIRECTORY_ENTRY_BYTES);
            sorted_entry += CLOAD_DIRECTORY_ENTRY_BYTES;
            write_index += 1;
            page_entry += CLOAD_DIRECTORY_ENTRY_BYTES;
        } while (write_index < g_cload_entry_state);
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
 * @return The prim/handle returned by cload_draw_cached_text.
 * @note Each decimal digit indexes the g_cload_decimal_glyphs glyph table; 0x4F82 is the
 *       '0' glyph (skipped while leading) and 0x5B81 the minus glyph.
 * @see decomp.me (100.00%)
 */
s32 cload_draw_signed_decimal(s32 prim, s32 *ot, s32 value, s32 x, s32 y, s32 palette, s32 alignment)
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
    buf[1] = g_cload_decimal_glyphs[magnitude / 10000];
    buf[2] = g_cload_decimal_glyphs[(magnitude % 10000) / 1000];
    buf[3] = g_cload_decimal_glyphs[(magnitude % 1000) / 100];
    buf[4] = g_cload_decimal_glyphs[(magnitude % 100) / 10];
    buf[5] = g_cload_decimal_glyphs[magnitude % 10];

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
    prim = cload_draw_cached_text(prim, ot, &buf[first_digit], x, y, palette, alignment);
    return prim;
}


/**
 * @brief Render @p value as a two-digit hex glyph string via cload_draw_cached_text.
 * @param prim Passed through to cload_draw_cached_text (prim/handle).
 * @param ot Passed through to cload_draw_cached_text (ordering table).
 * @param value Byte value; its high and low nibbles index the g_cload_hex_glyphs glyph table.
 * @param x X coordinate passed through.
 * @param y Y coordinate passed through.
 * @param palette Passed through to cload_draw_cached_text.
 * @see decomp.me (100%)
 */
void cload_draw_hex_byte(s32 prim, s32 ot, s32 value, s32 x, s32 y, s32 palette)
{
    u16 buf[3];
    u16 *high_glyph;

    high_glyph = &g_cload_hex_glyphs[value / 16];
    buf[0] = *high_glyph;
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

    setlen(prim, 1);
    ((CloadGpuPacket *)prim)->word4 = 0xE1000005;
    addPrim(ot, prim);
    return prim + 8;
}


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

    while (slot < CLOAD_GLYPH_CACHE_SLOTS)
    {
        if (requested_code == entry->data.code)
        {
            return cload_emit_glyph_sprite(prim, ot, slot, palette);
        }
        slot++;
        entry++;
    }

    font_address = Krom2RawAdd(code & 0xFFFF);
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
    while ((slot < CLOAD_GLYPH_CACHE_SLOTS) && (g_cload_glyph_cache[slot].raw != 0))
    {
        slot++;
    }

    if (slot == CLOAD_GLYPH_CACHE_SLOTS)
    {
        return prim;
    }
    g_cload_glyph_cache[slot].raw = code & 0xFFFF;
    prim = cload_emit_glyph_sprite(prim, ot, slot, palette);

    g_cload_glyph_upload_x = (slot % CLOAD_GLYPH_CACHE_COLUMNS) * 4;
    g_cload_glyph_upload_y = slot & CLOAD_GLYPH_CACHE_ROW_MASK;

    rect.w = 4;
    rect.h = 15;
    rect.x = g_cload_glyph_upload_x + 0x140;
    rect.y = g_cload_glyph_upload_y;

    LoadImage(&rect, g_cload_glyph_raster_cursor);
    DrawSync(0);

    g_cload_glyph_raster_cursor += CLOAD_GLYPH_RASTER_BYTES;
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

    setlen(sprite, 3);
    setcode(sprite, 0x7C);
    sprite->packet.g0 = 0x80;
    sprite->packet.b0 = 0x80;
    sprite->packet.r0 = 0x80;
    normalized_slot = cache_slot;
    sprite->packet.x0 = g_cload_glyph_cursor_x;
    sprite->packet.y0 = g_cload_glyph_cursor_y;

    if (cache_slot < 0)
    {
        normalized_slot = cache_slot + 15;
    }

    sprite->packet.u0 = (cache_slot - ((normalized_slot >> 4) * 16)) * 16;
    sprite->packet.v0 = cache_slot & CLOAD_GLYPH_CACHE_ROW_MASK;
    sprite->packet.clut = 0x7FD3;
    sprite->packet.tag = (sprite->packet.tag & CLOAD_GPU_TAG_HIGH_MASK) | (*ot & CLOAD_GPU_ADDR_MASK);

    packet_address = ((u32)sprite) & CLOAD_GPU_ADDR_MASK;
    ot_tag_high_byte = *ot & CLOAD_GPU_TAG_HIGH_MASK;

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

    while (cache_slot < CLOAD_GLYPH_CACHE_SLOTS)
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

    while (cache_slot < CLOAD_GLYPH_CACHE_SLOTS)
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
 *       Lead bytes 0x19..0x1F select a [16][33] block of the double-byte glyph
 *       table indexed by the next byte's nibbles; 0x21 and above index
 *       g_cload_single_byte_char_table by (c - 0x20); everything else emits
 *       g_cload_single_byte_char_table's first entry (the blank glyph) and
 *       consumes one byte. Both tables are arrays of 33-byte rows (16 two-byte
 *       glyphs plus a 0x0A row terminator).
 * @note Measured-required shapes for the byte-exact match:
 *       (1) `for (;;)` with `goto done` past the loop, NOT `while (*in != 0)`;
 *       jumping to a label beyond the loop avoids gcc 2.7.2's expand_end_loop
 *       test rotation while keeping the loop notes that let LICM hoist the
 *       table base pointers.
 *       (2) arm 1's double-byte table base is spelled
 *       `(u8 *)cload_load_icon_resources + 0x2C` / `+ 0x2D` - the target's
 *       relocation is against that text symbol (both resolve to 0x80143350),
 *       and using g_cload_double_byte_char_table there mismatched the symbol.
 *       (3) the per-arm `lead = *(volatile u8 *)in` reads force the target's
 *       re-load of in[0] inside each index expression instead of a CSE reuse.
 *       (4) arm 2's index reads `(index / 16) * 33` before `(index & 0xF) * 2`.
 * @see decomp.me (100.00%)
 */
void cload_expand_text_glyph_codes(u8 *out, u8 *in)
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
            pa = ((u8 *)cload_load_icon_resources + 0x2C) + b1 * 2;
            pa += off * 33;
            lead = *(volatile u8 *)in;
            pa += lead * 528;
            *out = *pa;
            out++;
            b1 = in[1];
            off = b1 >> 4;
            b1 &= 0xF;
            pb = ((u8 *)cload_load_icon_resources + 0x2D) + b1 * 2;
            pb += off * 33;
            lead = *(volatile u8 *)in;
            pb += lead * 528;
            *out = *pb;
            out++;
            in += 2;
        }
        else if ((u8)c >= 0x21)
        {
            lead = *(volatile u8 *)in;
            index = lead - 0x20;
            *out = g_cload_single_byte_char_table[(index / 16) * 33 + (index & 0xF) * 2];
            out++;
            lead = *(volatile u8 *)in;
            index = lead - 0x20;
            *out = g_cload_single_byte_char_table[(index / 16) * 33 + (index & 0xF) * 2 + 1];
            out++;
            in += 1;
        }
        else
        {
            *out = g_cload_single_byte_char_table[0];
            out++;
            *out = g_cload_single_byte_char_table[1];
            out++;
            in += 1;
        }
    }
done:
    *out = 0;
}
