#include "common.h"
#include "vector.h"
#include "display.h"

#define NIKI_SJIS_FULLWIDTH_ZERO 0x4F82
#define NIKI_SJIS_MINUS 0x5B81
#define NIKI_PROGRESS_DURATION 256
#define NIKI_PROGRESS_WIDTH 288
#define NIKI_PROGRESS_HEIGHT 44
#define NIKI_SAVE_PAYLOAD_BYTES 0x33E0
#define NIKI_SAVE_MAGIC 0x00414E41
#define NIKI_SAVE_CHECKSUM_BIAS 0x0414E410
#define NIKI_ELEMENT_COUNT 8
#define NIKI_ELEMENT_WORD_STRIDE 3
#define NIKI_ELEMENT_STATE_MASK 7
#define NIKI_ELEMENT_PHASE_MASK 0x78
#define NIKI_CARD_DIRECTORY_BYTES 0x320
#define NIKI_DIRECTORY_ENTRY_BYTES 0x28
#define NIKI_MEMORY_CARD_BLOCK_BYTES 8192
#define SET_ELEM_CODE(e, c) ((e)->attr.word = ((e)->attr.word & 0x00FFFFFF) | ((u32)(c) << 24))
#define GLYPH_SYM(sym, off) ((void*)(((u8*)&(sym) - (off)) + (sym)))
#define GLYPH_OFF(base, off) ((void*)((base) + *(u16*)((base) + (off))))
#define NIKI_DIRECTORY_ENTRY_COUNT 20
#define GLYPH_CACHE_SLOTS 0x100
#define GLYPH_CACHE_COLUMNS 16
#define GLYPH_CACHE_ROW_MASK 0xF0
#define GLYPH_RASTER_BYTES 0x80
#define GPU_ADDR_MASK 0xFFFFFF
#define GPU_TAG_HIGH_MASK 0xFF000000
#define NIKI_SET_PACKET_LENGTH(prim, length) (((NikiPrimTag*)(prim))->len = (u8)(length))
#define NIKI_SET_PACKET_ADDRESS(prim, address) (((NikiPrimTag*)(prim))->addr = (u32)(address))
#define NIKI_SET_PACKET_CODE(prim, command) (((NikiPrimTag*)(prim))->code = (u8)(command))
#define NIKI_GET_PACKET_ADDRESS(prim) ((u32)(((NikiPrimTag*)(prim))->addr))
#define NIKI_ADD_PRIMITIVE(ordering_table, prim)                                                                                                               \
    (NIKI_SET_PACKET_ADDRESS((prim), NIKI_GET_PACKET_ADDRESS(ordering_table)), NIKI_SET_PACKET_ADDRESS((ordering_table), (prim)))
#define NIKI_TEXT_EXTENDED_LEAD_FIRST 0x19
#define NIKI_TEXT_EXTENDED_PAGE_COUNT 7
#define NIKI_TEXT_SINGLE_BYTE_BASE 0x20
#define NIKI_TEXT_PRINTABLE_FIRST 0x21
#define NIKI_SJIS_CODES_PER_ROW 16
#define NIKI_SJIS_ROWS_PER_PAGE 16
#define NIKI_SJIS_ROW_SHIFT 4

/** @brief Serialized save payload followed by its checksum and format marker. */
typedef struct
{
    u8 payload[NIKI_SAVE_PAYLOAD_BYTES];
    s32 checksum;
    s32 magic;
} NikiSaveBlob;

/** @brief Three two-byte overflow glyphs and their string terminator. */
typedef struct
{
    s8 data[7];
} NikiDecimalOverflow;

typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} RECT;

typedef struct NikiElement
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
    void* draw;
    s32 second_state;
} NikiElement;

typedef struct NikiEntryMetadata
{
    u8 pad0[0x17];
    u8 unk17;
    u8 pad18[0xCF - 0x18];
    u8 unkCF;
    u8 padD0[4];
    u16 unkD4;
    u16 unkD6;
} NikiEntryMetadata;

/** @brief Memory-card directory entry; layout matches Psy-Q DIRENTRY. */
typedef struct NikiDirEntry
{
    char name[20];
    s32 attr;
    s32 size;
    void* next;
    s32 head;
    char system[4];
} NikiDirEntry;

/**
 * @brief 0xC-stride element of the g_niki_element_pool array (attr word + handler).
 * @note Same layout as NikiElement minus its trailing unkC, so a NikiPacket*
 *       walks the array at its real 0xC stride while still exposing attr.f.
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
    void (*draw)();
} NikiPacket;

/**
 * @brief One GPU primitive packet the element draw functions emit into.
 */
typedef struct
{
    s32 tag;
    s32 word4;
    s16 x0;
    s16 y0;
    s16 unkC;
    u16 unkE;
} NikiGpuPacket;

/**
 * @brief Per-frame draw context passed to niki_update_and_draw_elements.
 * @note prim_cursor is the running GPU-packet write cursor; frame_flag selects
 *       the clip/window variant.
 */
typedef struct
{
    s32 head_tag;
    u8 pad4[0x40AE];
    s16 frame_flag;
    u8 pad40B4[4];
    NikiGpuPacket* prim_cursor;
} NikiFrameState;

/** @brief Element draw callback: returns the advanced GPU-packet cursor. */
typedef NikiGpuPacket* (*NikiElementDrawFunc)();

/** @brief GPU linked-list address and packet length, with a packed word view. */
typedef union
{
    s32 word;
    struct
    {
        u8 address[3];
        u8 length;
    } bytes;
} NikiGpuTag;

/** @brief GPU vertex color and command byte, with a packed word view. */
typedef union
{
    s32 word;
    struct
    {
        u8 r, g, b, code;
    } bytes;
} NikiGpuColor;

/** @brief Field layout of the POLY_G4 timer-bar packet built by niki_draw_progress_bar. */
typedef struct
{
    NikiGpuTag tag;
    NikiGpuColor color0;
    s16 x0;
    s16 y0;
    NikiGpuColor color1;
    s16 x1;
    s16 y1;
    NikiGpuColor color2;
    s16 x2;
    s16 y2;
    NikiGpuColor color3;
    s16 x3;
    s16 y3;
} NikiPolyG4Packet;

/**
 * @brief Fallback name/second-line text carried alongside the save-slot record.
 * @note Consumed only when the primary slot compare (func_8001714C) fails.
 */
typedef struct NikiFallbackText
{
    u8 pad[0x24];
    u8 text[0x20];
} NikiFallbackText;

typedef struct
{
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 w, h;
} TILE;

/** @brief Textured quadrilateral used to display a save-file icon. */
typedef struct
{
    NikiGpuTag tag;
    NikiGpuColor color;
    s16 x0, y0;
    u8 u0, v0;
    s16 clut;
    s16 x1, y1;
    u8 u1, v1;
    s16 tpage;
    s16 x2, y2;
    u8 u2, v2;
    u8 padding2[2];
    s16 x3, y3;
    u8 u3, v3;
    u8 padding3[2];
} NikiTexturedQuad;

/** @brief Word-aligned memory-card device prefix, such as "bu00". */
typedef union
{
    u32 word;
    struct
    {
        u8 name[2];
        u8 slot;
        u8 port;
    } characters;
} NikiCardDevice;

/** @brief Card path used to remove a placeholder save file. */
typedef struct
{
    NikiCardDevice device;
    u8 suffix[28];
} NikiPlaceholderPath;

/** @brief Memory-card path workspace for the load/save sequence. */
typedef struct
{
    NikiCardDevice device;
    u8 suffix[100];
} NikiSequencePath;

/** @brief Memory-card directory search path, including the device and wildcard. */
typedef struct
{
    NikiCardDevice device;
    u8 suffix[12];
} NikiDirectoryPattern;

/** @brief Complete memory-card path for the selected save file. */
typedef struct
{
    NikiCardDevice device;
    u8 suffix[252];
} NikiSelectedFilePath;

typedef struct
{
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
} NikiSprt16;

typedef struct
{
    unsigned addr : 24;
    unsigned len : 8;
    u8 r0, g0, b0, code;
} NikiPrimTag;

typedef struct
{
    s32 tag;
    s32 word4;
    s16 x0;
    s16 y0;
    s16 unkC;
    u16 unkE;
} NikiGpuPacketPrefix;

typedef union
{
    u32 raw;
    struct
    {
        u16 code;
        u16 flags;
    } data;
} NikiGlyphCacheEntry;

typedef struct
{
    NikiSprt16 packet;
    u32 padding;
} NikiGlyphSprite;

/** @brief Two bytes of a Shift-JIS character in the glyph lookup tables. */
typedef struct NikiSjisCode
{
    u8 lead;
    u8 trail;
} NikiSjisCode;

/** @brief Sixteen Shift-JIS characters followed by the table row delimiter. */
typedef struct NikiSjisRow
{
    NikiSjisCode codes[NIKI_SJIS_CODES_PER_ROW];
    u8 newline;
} NikiSjisRow;

/** @brief Lookup page selected by an extended character lead byte. */
typedef struct NikiSjisPage
{
    NikiSjisRow rows[NIKI_SJIS_ROWS_PER_PAGE];
} NikiSjisPage;

extern s8 D_80140088[];
extern s32 g_niki_io_busy;
extern s32 g_niki_icon_phase;
extern s32 g_niki_confirm_latch;
extern s32 D_80164AE4;
extern s32 g_niki_mode;
extern s32 g_niki_card_slot;
extern s32 g_niki_exit_requested;
extern s32 g_niki_entry_state;
extern s32 g_niki_selection_status;
extern s32 g_niki_frame_parity;
extern s32 g_niki_progress_active;
extern s32 D_80164ADC;
extern s32 g_niki_selected_row;
extern NikiElement g_niki_element1;
extern s32 g_pad_input;
extern s32 g_niki_scroll_frames;
extern s32 g_niki_scroll_y;
extern s32 g_niki_scroll_target_y;
extern s32 D_80164B80;
extern u8 D_801606C8[];
extern u8 D_801606D4[];
extern u8 D_801606DC[];
extern NikiElement g_niki_element_pool;
extern s32 g_niki_entry_scan_active;
extern u8* g_niki_load_step;
extern s32 D_80122994;
extern char D_800ECF7C[];
extern NikiDirEntry g_niki_entries[][NIKI_DIRECTORY_ENTRY_COUNT];
extern NikiEntryMetadata g_niki_entry_metadata;
extern NikiEntryMetadata* D_8012271C;
extern s32 D_8003EC9C;
extern s32 g_niki_icon_palette;
extern s32 g_niki_dialog_state;
extern u8 D_80164DE7;
extern u8 D_80164B98;
extern u8 D_80164B9C;
extern u16 D_80147120;
extern u16 D_80147146;
extern u16 D_80147148;
extern u16 D_8014714C;
extern u16 D_801475C4[];
extern u8 D_800EC3F6[2];
extern u8 D_800EC3FA[];
extern u8 D_800EC3D0[];
extern s32 g_menu_element_counter;
extern u16 D_80147128;
extern s32 g_niki_choice_toggle;
extern u8 D_801606E4[];
extern u16 D_8014712A;
extern u8 g_niki_save_blob[];
extern u8 D_8011F3D8[];
extern u8 D_80164E70[];
extern s32 D_8011F428;
extern s32 D_801227CC;
extern s32 D_801227F4;
extern s32 D_8011F418;
extern u8 D_80122A08[];
extern s32 g_niki_progress_bar_active;
extern s32 g_niki_progress_start_tick;
extern char D_800ECF8C[];
extern char D_800ECFC4[];
extern u16 D_801470F8;
extern u16 D_801470FA;
extern u16 D_801470FC;
extern u16 D_801470FE;
extern u16 D_80147100;
extern u16 D_80147108;
extern u16 D_8014710A;
extern u16 D_8014710C;
extern u16 D_80147126;
extern u16 D_8014712C;
extern u16 D_80147132;
extern u16 D_80147134;
extern u16 D_80147136;
extern u16 D_80147138;
extern u16 D_8014713A;
extern u16 D_801471A8;
extern s32 g_niki_entry_ranks[];
extern s32 g_niki_rank_count;
extern s32 g_niki_entry_suffix_values[];
extern s32 D_801477AC[];
extern u8 g_niki_icon_context[];
extern u16 D_8014713C;
extern u16 D_8014713E;
extern u16 D_80147104;
extern u16 D_80147106;
extern u16 D_80147114;
extern u16 D_80147160;
extern u16 D_80147162;
extern u16 D_8014716A;
extern u8 D_801606EC;
extern u8 D_801606F5;
extern void* jtbl_80140054[];
extern s32 g_niki_entry_fields[];
extern s32 g_niki_entry_value_limit;
extern NikiPlaceholderPath g_niki_file_template;
extern char D_800ECF9C[];
extern char D_800ECFB0[];
extern void* jtbl_80140098[];
extern s32 g_niki_file_handle;
extern s32 g_niki_retry_count;
extern s32 g_niki_selected_entry_extended;
extern s32 g_niki_primary_poll_countdown;
extern s32 g_niki_secondary_poll_countdown;
extern s32 D_80164FD4;
extern u8 D_80164FD8[];
extern u8 D_801606D0[];
extern s32 g_niki_primary_handle0;
extern s32 g_niki_primary_handle1;
extern s32 g_niki_primary_handle2;
extern s32 g_niki_primary_handle3;
extern s32 g_niki_secondary_handle0;
extern s32 g_niki_secondary_handle1;
extern s32 g_niki_secondary_handle2;
extern s32 g_niki_secondary_handle3;
extern NikiDirectoryPattern g_niki_entry_header_template;
extern u8 D_801606E0[];
extern u16 g_niki_decimal_glyphs[];
extern u16 g_niki_hex_glyphs[];
extern s32 g_niki_glyph_cursor_x;
extern s32 g_niki_text_line_start_x;
extern s32 g_niki_glyph_cursor_y;
extern NikiGlyphCacheEntry g_niki_glyph_cache[];
extern u8* g_niki_glyph_raster_cursor;
extern s32 g_niki_glyph_upload_x;
extern s32 g_niki_glyph_upload_y;
extern u8 g_niki_glyph_raster_buffer[];
extern NikiSjisPage g_niki_double_byte_char_table[];
extern NikiSjisRow g_niki_single_byte_char_table[];

s32 niki_validate_save_blob(NikiSaveBlob* blob);
s32 niki_compute_save_checksum(u8* data);

void niki_update_elements(NikiFrameState* frame);
void niki_update_and_draw_elements(NikiFrameState* frame);
s32 niki_update_load_sequence(void);
s32 niki_handle_input(void);
s32 niki_draw_entry_list(s32* ot, s32 prim, s32 arg2, s32 arg3);
s32 niki_draw_header_label(s32* ot, s32 prim, s32 arg2, s32 arg3);
s32 niki_draw_card_slot0_label(s32* ot, s32 prim, s32 arg2, s32 arg3);
s32 niki_draw_card_slot1_label(s32* ot, s32 prim, s32 arg2, s32 arg3);
s32 niki_draw_selected_entry_details(s32* ot, s32 prim, s32 arg2, s32 arg3);
s32 niki_draw_icon_highlight(s32 prim, s32* ot, s32 x, s32 y, s32 width, s32 icon_index, s32 texture_slot, s32 palette_mode);
s32 niki_draw_cached_text(s32 prim, s32* ot, u8* text, s32 x, s32 y, s32 palette, s32 alignment);
void niki_terminate_multibyte_text(void* arg0);
s32 niki_draw_footer_label(s32* ot, s32 prim, s32 arg2, s32 arg3);
s32 niki_draw_state_page(s32* ot, s32 prim, s32 arg2, s32 arg3);
void niki_clear_elements();
s32 niki_advance_load_sequence(void);
void func_800A3938();
s32 niki_draw_status_dialog(s32* ot, s32 prim, s32 arg2, s32 arg3);
s32 niki_draw_secondary_status_dialog(s32* ot, s32 prim, s32 arg2, s32 arg3);
void niki_close_all_elements();
void niki_switch_card_slot();
void niki_commit_selected_entry(void);
void niki_scroll_to_selection();
s32 func_8001714C(void*, void*, s32);
NikiElement* niki_alloc_element();
void niki_enable_choice_toggle();
void niki_restart_load_sequence();
s32 niki_draw_save_confirm_dialog(s32* ot, s32 prim, s32 arg2, s32 arg3);
s32 niki_draw_confirm_prompt(s32* ot, s32 prim, s32 arg2, s32 arg3);
s32 func_800A88A0(s32 prim, s32* ot, void* glyph, s32 a3, s32 x, s32 y, s32 mode);
s32 func_800A8A78(s32* ot, s32 prim, s32 ch, s32 a3, Vec2s* pos, s32 mode);
u8* niki_skip_hex_digits(void* text);
void func_80019A34(RECT* rect, void* str);
void func_800A55E4(void* buf, s32 arg1);
void func_800A5638(void* buf, s32 arg1);
s32 niki_parse_hex_suffix_byte();
s32 niki_parse_entry_fields();
void niki_sort_entries_by_type();
void niki_reset_entry_ranks(void);
s32 func_80016F9C(void*, void*);
s32 func_8001686C(void*);
s32 func_8001680C(void*, s32);
s32 func_8001681C(s32, void*, s32);
s32 func_8001682C(s32, void*, s32);
s32 func_8001683C(s32);
s32 func_8001685C(void*, void*);
s32 func_800170BC(void*, void*, ...);
s32 func_8001724C(s32);
s32 func_8001725C(s32);
s32 func_8001729C(s32);
s32 func_800172AC(s32);
s32 func_8002054C(s32);
s32 func_80032174(s32, void*, s32*);
s32 func_800342CC(s32);
s32 niki_begin_entry_scan(s32);
s32 niki_scan_next_entry(s32);
void niki_release_primary_handles(void);
void niki_release_secondary_handles(void);
s32 niki_poll_primary_handle_group(void);
s32 niki_poll_secondary_handle_group(void);
void niki_open_status_dialog(s32);
void niki_open_secondary_status_dialog(s32);
void func_800158E0(void);
s32 func_800167AC(s32, s32, s32, s32);
void func_800167DC(s32);
void func_800167EC(void);
void func_800167FC(void);
void func_800167BC(s32);
s32 func_800167CC(s32);
void func_80016E7C();
s32 func_8001687C(s32);
void func_80019788(s32);
s32 niki_render_cached_glyph(s32 prim, s32* ot, s32 character_code, s32 palette);
s32 niki_emit_glyph_sprite(NikiGlyphSprite* sprite, s32* ot, s32 cache_slot, s32 palette);
void niki_build_ui_elements(void);
void niki_update_menu(NikiFrameState* frame);
void niki_hex_nibble_to_ascii(s8* out, s32 value);
void niki_init_stream_handles(void);
void niki_shutdown_stream_handles(void);
void niki_begin_glyph_cache_frame(void);
void niki_evict_unused_glyphs(void);
void niki_reset_glyph_cache(void);

void niki_init(s32 arg0, s32 mode)
{
    RECT rect;

    g_niki_mode = mode;
    g_niki_entry_state = 0xFF;
    g_niki_card_slot = 0;
    niki_reset_entry_ranks();
    niki_init_stream_handles();
    g_niki_icon_phase = 0;
    func_80067F8C();
    rect.x = OVERLAY_INIT_CLEAR_VRAM_X;
    rect.y = OVERLAY_INIT_CLEAR_VRAM_Y;
    rect.w = OVERLAY_INIT_CLEAR_VRAM_W;
    rect.h = OVERLAY_INIT_CLEAR_VRAM_H;
    func_8001990C(&rect, 0, 0, 0);
    niki_reset_glyph_cache();
    g_niki_progress_active = 0;
    g_niki_confirm_latch = 0;
    g_niki_selection_status = 0;
    g_niki_io_busy = 0;
    g_niki_frame_parity = 0;
    g_niki_exit_requested = 0;
    func_800AA02C();
    niki_build_ui_elements();
    D_80164AE4 = arg0;
}

/**
 * @brief Draw and update one menu frame, or finish a requested exit.
 * @param frame Draw context receiving this frame's GPU packets.
 * @return One when the menu has exited, otherwise zero.
 */
s32 niki_update_frame(NikiFrameState* frame)
{
    if (g_niki_exit_requested != 0)
    {
        niki_shutdown_stream_handles();
        field_text_reset_windows();
        func_80019788(0);
        return 1;
    }
    field_text_reset_scratch();
    niki_begin_glyph_cache_frame();
    niki_update_menu(frame);
    niki_evict_unused_glyphs();
    func_80063194();
    g_niki_frame_parity ^= 1;
    return 0;
}

void niki_build_ui_elements(void)
{
    NikiElement* p;
    g_niki_scroll_frames = 0;
    g_niki_scroll_target_y = 0;
    g_niki_scroll_y = 0;
    g_niki_selected_row = 0;
    g_niki_selection_status = 0;
    D_80164ADC = (s32)D_8012271C + 0xCE0;
    if (0)
    {
        niki_clear_elements(0, 0, 0, 0, 0);
    }
    niki_clear_elements();
    D_80164B80 = 0;
    if (g_niki_mode != 0)
    {
        g_niki_element_pool.attr.f.state = 1;
        p = niki_alloc_element();
        p->draw = (void*)niki_draw_state_page;
        p->attr.f.phase = 1;
        p->attr.f.x = 0x10;
        p->attr.f.code = 0x61;
        p->active = 1;
        p->y = 0x2C;
        SET_ELEM_CODE(p, 0x20);

        p = niki_alloc_element();
        p->draw = (void*)niki_draw_card_slot0_label;
        p->attr.f.phase = 1;
        p->attr.f.x = 0x18;
        p->attr.f.code = 0x4D;
        p->active = 0;
        p->y = 0x10;
        SET_ELEM_CODE(p, 0x80);

        p = niki_alloc_element();
        p->draw = (void*)niki_draw_card_slot1_label;
        p->attr.f.phase = 1;
        p->attr.f.x = 0xA0;
        p->attr.f.code = 0x4D;
        p->active = 0;
        p->y = 0x10;
        SET_ELEM_CODE(p, 0x80);
        g_niki_element_pool.attr.f.state = 0;
        return;
    }

    g_niki_element_pool.attr.f.state = 1;
    p = niki_alloc_element();
    p->draw = (void*)niki_draw_entry_list;
    p->attr.f.phase = 1;
    p->attr.f.x = 0x1C;
    p->attr.f.code = 0x32;
    p->active = 1;
    p->y = 0x58;
    SET_ELEM_CODE(p, 8);

    p = niki_alloc_element();
    p->draw = (void*)niki_draw_header_label;
    p->attr.f.phase = 1;
    p->attr.f.x = 0x24;
    p->attr.f.code = 0x0A;
    p->active = 0;
    p->y = 0x10;
    SET_ELEM_CODE(p, 0xF0);

    p = niki_alloc_element();
    p->draw = (void*)niki_draw_card_slot0_label;
    p->attr.f.phase = 1;
    p->attr.f.x = 0x18;
    p->attr.f.code = 0x1E;
    p->active = 0;
    p->y = 0x10;
    SET_ELEM_CODE(p, 0x80);

    p = niki_alloc_element();
    p->draw = (void*)niki_draw_card_slot1_label;
    p->attr.f.phase = 1;
    p->attr.f.x = 0xA0;
    p->attr.f.code = 0x1E;
    p->active = 0;
    p->y = 0x10;
    SET_ELEM_CODE(p, 0x80);

    p = niki_alloc_element();
    p->draw = (void*)niki_draw_selected_entry_details;
    p->attr.f.phase = 1;
    p->attr.f.x = 0x1E;
    p->attr.f.code = 0x8E;
    p->active = 1;
    p->y = 0x34;
    SET_ELEM_CODE(p, 4);
    g_niki_element_pool.attr.f.state = 0;
}

/**
 * @brief Draw menu elements, process input and advance scrolling.
 * @param frame Draw context receiving the menu packets.
 */
void niki_update_menu(NikiFrameState* frame)
{
    s32 delta;

    niki_update_elements(frame);
    g_niki_icon_phase += 2;
    if ((g_niki_element1.attr.word & 0x7F) == 2)
    {
        niki_update_load_sequence();
    }
    if ((u16)g_pad_input == 0xFFFF)
    {
        g_pad_input = 0;
    }
    niki_handle_input();
    if (g_niki_scroll_frames != 0)
    {
        s32 base = g_niki_scroll_y;
        delta = (g_niki_scroll_target_y - g_niki_scroll_y) / g_niki_scroll_frames;
        g_niki_scroll_frames -= 1;
        g_niki_scroll_y += delta;
    }
    else
    {
        g_niki_scroll_y = g_niki_scroll_target_y;
    }
}

/**
 * @brief Run immediate sequence steps, then select the next wait or recovery sequence.
 * @return Unspecified; callers ignore the return value.
 */
s32 niki_update_load_sequence(void)
{
    s32 result;

    if (g_niki_entry_state >= 0x10)
    {
        if (g_niki_load_step == NULL)
        {
            g_niki_load_step = D_801606C8;
        }
    }

    do
    {
        result = niki_advance_load_sequence();
    } while (result == 3);

    if ((D_80164B80 != 0) && (g_pad_input & 0x220))
    {
        g_niki_entry_state = 0xF8;
        g_niki_load_step = D_801606DC;
    }
    else
    {
        switch (result)
        {
        case 0:
            break;
        case 4:
            g_niki_load_step = D_801606D4;
            D_80164B80 = 0;
            break;
        case 5:
            g_niki_entry_state = 0xF8;
            /* fallthrough */
        case 2:
            g_niki_load_step = D_801606DC;
            break;
        }
    }
}

s32 niki_handle_input(void)
{
    s32 pending;
    s32 status;
    s32 count;
    s32 term1;
    s32 term2;
    NikiElement* p;

    if ((g_niki_element_pool.second_state & 7) == 0)
    {
        g_niki_exit_requested = 1;
        return;
    }
    if (g_niki_exit_requested != 0)
    {
        return;
    }
    if ((g_niki_element_pool.second_state & 7) >= 3)
    {
        return;
    }
    if ((g_niki_element_pool.attr.word & 7) != 0)
    {
        return;
    }
    pending = g_niki_entry_state;
    if (pending == 0xFF)
    {
        return;
    }
    if (g_niki_entry_scan_active != 0)
    {
        return;
    }
    if (g_niki_io_busy != 0)
    {
        return;
    }
    if ((u32)(*g_niki_load_step - 6) < 2U)
    {
        return;
    }
    if (g_niki_mode != 0)
    {
        return;
    }

    status = g_pad_input;
    if (status & 0x40)
    {
        D_80122994 = 3;
        func_800A3938(0x78, 0x80);
        niki_close_all_elements();
        return;
    }
    if (status & 0xA100)
    {
        func_800A3938(0x7D, 0x80);
        niki_switch_card_slot();
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
            g_niki_selected_row -= 1;
            if (g_niki_selected_row < 0)
            {
                g_niki_selected_row = g_niki_entry_state - 1;
            }
        }
        if (g_pad_input & 0x4000)
        {
            g_niki_selected_row += 1;
            if (g_niki_selected_row >= g_niki_entry_state)
            {
                g_niki_selected_row = 0;
            }
        }
        count -= 1;
    }

    if (g_pad_input & 0x5000)
    {
        niki_commit_selected_entry();
        func_800A3938(0x7D, 0x80);
        niki_scroll_to_selection();
        return;
    }

    if (g_pad_input & 0x220)
    {
        if (g_niki_mode != 0)
        {
            return;
        }
        term1 = g_niki_card_slot * NIKI_CARD_DIRECTORY_BYTES;
        term2 = (g_niki_selected_row * NIKI_DIRECTORY_ENTRY_BYTES) + (s32)g_niki_entries;
        if (func_8001714C(D_800ECF7C, (char*)(term1 + term2), 0xC) == 0)
        {
            if ((g_niki_entry_metadata.unkD4 != D_8012271C->unkD4) && (g_niki_entry_metadata.unk17 != 0) &&
                ((D_8003EC9C == 0xFF) || (g_niki_entry_metadata.unkCF == D_8003EC9C)))
            {
                p = niki_alloc_element();
                p->attr.f.phase = 1;
                p->attr.f.x = 0x10;
                p->attr.f.code = 0x61;
                p->active = 1;
                p->y = 0x1E;
                SET_ELEM_CODE(p, 0x20);
                niki_enable_choice_toggle();
                p->draw = niki_draw_confirm_prompt;
                niki_restart_load_sequence();
                func_800A3938(0x7E, 0x80);
                return;
            }
        }
        func_800A3938(0x78, 0x80);
    }
}

void niki_switch_card_slot(void)
{
    D_80164B80 = 0;
    g_niki_load_step = 0;
    g_niki_entry_state = 0xFF;
    g_niki_scroll_frames = 0;
    g_niki_scroll_target_y = 0;
    g_niki_scroll_y = 0;
    g_niki_selected_row = 0;
    g_niki_selection_status = 0;
    g_niki_card_slot ^= 1;
    niki_reset_entry_ranks();
}

/** @brief Start the closing animation for every active menu element. */
void niki_close_all_elements(void)
{
    s32 attributes;
    s32 element_index;
    NikiPacket* element;
    s32 closing_attributes;

    func_80067F28();
    element = (NikiPacket*)&g_niki_element_pool;
    element_index = 0;
    for (; element_index < NIKI_ELEMENT_COUNT; element_index++, element++)
    {
        attributes = element->attr.word;
        if (attributes & NIKI_ELEMENT_STATE_MASK)
        {
            closing_attributes = (attributes & ~NIKI_ELEMENT_STATE_MASK) | 3;
            element->attr.word = (closing_attributes & ~NIKI_ELEMENT_PHASE_MASK) | 0x40;
        }
    }
}

/** @brief Scroll the browser over four frames to keep the selected row visible. */
void niki_scroll_to_selection(void)
{
    s32 selected_row;
    s32 half_row_y;
    s32 scroll_y;
    s32 selected_y;
    s32 relative_y;

    selected_row = g_niki_selected_row;
    half_row_y = (selected_row << 3) - selected_row;
    scroll_y = g_niki_scroll_y;
    selected_y = half_row_y << 1;
    relative_y = selected_y - scroll_y;

    if (relative_y >= 75)
    {
        g_niki_scroll_target_y = selected_y - 70;
        g_niki_scroll_frames = 4;
    }
    if (relative_y < 0)
    {
        g_niki_scroll_target_y = selected_y;
        g_niki_scroll_frames = 4;
    }
}

/**
 * @brief Update and draw the active menu elements.
 * @param frame Draw context receiving the element packets.
 */
void niki_update_elements(NikiFrameState* frame)
{
    niki_update_and_draw_elements(frame);
}

/**
 * @brief Render the niki row/status list: per-entry glyphs, markers and the
 *        highlight tile, dispatched by the g_niki_entry_state list-state selector.
 * @param ot Ordering-table pointer.
 * @param prim Primitive-buffer write cursor.
 * @param x_offset Horizontal scroll offset (subtracted from every x).
 * @param y_offset Vertical scroll offset (subtracted from every row y).
 * @return Advanced primitive-buffer write cursor.
 * @see decomp.me (100%)
 */
s32 niki_draw_entry_list(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    s32 state = g_niki_entry_state;

    switch (state)
    {
    case 0xF8:
        do
        {
            prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014712C, 0x34), 4, -x_offset + 0x84, -y_offset, 2);
        } while (0);
        break;
    case 0xF9:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014712C, 0x34), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFA:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_801470FA, 2), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFD:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_801470FC, 4), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFB:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80147108, 0x10), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFC:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014710A, 0x12), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFE:
        break;
    default:
    {
        s32 row_y;
        s32 i;

        if (g_niki_entry_scan_active != 0)
        {
            s32 x;
            u8* base;
        case 0xFF:
            x = -x_offset + 0x84;
            base = (u8*)&D_801470F8;
            prim = func_800A88A0(prim, ot, base + D_801470F8, 4, x, -y_offset, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x1E), 4, x, 0xE - y_offset, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - y_offset, 2);
            break;
        }
        i = 0;
        if (state > 0)
        {
            s32 base_x;
            s32* flag_ptr;
            u16 misc_glyph;
            Vec2s pos;
            s32 row;
            u8* base;

            base = (u8*)&D_801470F8;
            base_x = -x_offset;
            do
            {
                row = ((i * 14) - y_offset) - g_niki_scroll_y;
                row_y = row + 1;
                if ((u32)(row + 0xE) < 0x65U)
                {
                    flag_ptr = &g_niki_entry_ranks[i];
                    if (*flag_ptr >= 0)
                    {
                        pos.x = base_x + 0x86;
                        pos.y = row_y;
                        prim = func_800A88A0(func_800A8A78(ot, prim, *(s32*)((u8*)g_niki_entry_suffix_values + (i * 4)), 4, &pos, 0), ot,
                                             (void*)((s32)D_80147126 + (s32)base), 4, base_x + 0x70, row_y, 0);
                        if ((g_niki_rank_count - 1) == *flag_ptr)
                        {
                            misc_glyph = *(u16*)(base + 0x36);
                            prim = func_800A88A0(prim, ot, (void*)((s32)misc_glyph + (s32)base), 4, base_x + 0xC0, row_y, 0);
                        }
                        else if (*flag_ptr < 2)
                        {
                            misc_glyph = *(u16*)(base + 0x38);
                            prim = func_800A88A0(prim, ot, (void*)((s32)misc_glyph + (s32)base), 4, base_x + 0xC0, row_y, 0);
                        }
                        if (*niki_skip_hex_digits((void*)((s32)&g_niki_entries[g_niki_card_slot][i] + 0xC)) == 0x2B)
                        {
                            prim = func_800A88A0(prim, ot, (void*)((s32)D_801471A8 + (s32)base), 4, 0xF2 - x_offset, row_y, 1);
                        }
                    }
                    if (func_8001714C(D_800ECF7C, (char*)((s32)&g_niki_entries[g_niki_card_slot][i]), 0xC) == 0)
                    {
                        prim = func_800A88A0(prim, ot, (void*)((s32)D_801470FE + (s32)base), 4, 1 - x_offset, row_y, 0);
                    }
                    else if (func_8001714C(D_800ECF8C, (char*)((s32)&g_niki_entries[g_niki_card_slot][i]), 0xC) == 0)
                    {
                        prim = func_800A88A0(prim, ot, (void*)((s32)D_80147132 + (s32)base), 4, 1 - x_offset, row_y, 0);
                    }
                    else if (func_8001714C(D_800ECFC4, (char*)((s32)&g_niki_entries[g_niki_card_slot][i]), 8) == 0)
                    {
                        prim = func_800A88A0(prim, ot, (void*)((s32)D_8014710C + (s32)base), 4, 1 - x_offset, row_y, 0);
                    }
                    else
                    {
                        prim = func_800A88A0(prim, ot, (void*)((s32)D_80147100 + (s32)base), 4, 1 - x_offset, row_y, 0);
                    }
                }
                i++;
            } while (i < g_niki_entry_state);
        }
        row_y = ((g_niki_selected_row * 14) - y_offset) - g_niki_scroll_y;

        if (g_niki_entry_scan_active == 0)
        {
            TILE* tile = (TILE*)prim;

            *(u32*)&tile->r0 = 0xF080F0;
            *((u8*)tile + 3) = 3;
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

/**
 * @brief Draw the niki header banner glyph, picking one of two captions
 *        according to the g_niki_mode mode selector.
 * @param ot Ordering-table pointer.
 * @param prim Primitive-buffer write cursor.
 * @param x_offset Horizontal scroll offset (subtracted from the banner x).
 * @param y_offset Vertical scroll offset (subtracted from the banner y).
 * @return Advanced primitive-buffer write cursor.
 * @see decomp.me (100%)
 */
s32 niki_draw_header_label(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;

    if (g_niki_mode == 1)
    {
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014713E, 0x46), 4, -x_offset + 0x78, -y_offset, 2);
    }
    else
    {
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014713C, 0x44), 4, -x_offset + 0x78, -y_offset, 2);
    }
    return prim;
}

/**
 * @brief Draw the niki page header: an optional dark backdrop tile for pages
 *        past the first, then the header caption glyph.
 *
 * The backdrop is a 0x80 x 0x10 flat tile (code 0x62, color 0x101010) linked
 * into @p ot ahead of the caption, and is emitted only when g_niki_card_slot selects
 * a non-zero page.
 *
 * @param ot Ordering-table pointer.
 * @param prim Primitive-buffer write cursor.
 * @param x_offset Horizontal scroll offset (subtracted from the caption x).
 * @param y_offset Vertical scroll offset (subtracted from the caption y).
 * @return Advanced primitive-buffer write cursor.
 * @see decomp.me (100%)
 */
s32 niki_draw_card_slot0_label(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    TILE* tile;

    if (g_niki_card_slot != 0)
    {
        tile = (TILE*)prim;
        *(u32*)&tile->r0 = 0x101010;
        *((u8*)tile + 3) = 3;
        tile->code = 0x62;
        tile->x0 = 0;
        tile->y0 = 0;
        tile->w = 0x80;
        tile->h = 0x10;
        tile->tag = (tile->tag & 0xFF000000) | (*ot & 0xFFFFFF);
        *ot = (*ot & 0xFF000000) | ((s32)tile & 0xFFFFFF);
        prim += 0x10;
    }
    return func_800A88A0(prim, ot, GLYPH_SYM(D_80147104, 0xC), 4, -x_offset + 0x40, -y_offset, 2);
}

/**
 * @brief Draw the niki first-page header: an optional dark backdrop tile,
 *        then the header caption glyph.
 *
 * Mirror of niki_draw_card_slot0_label: identical 0x80 x 0x10 flat backdrop tile (code
 * 0x62, color 0x101010), but emitted only when g_niki_card_slot selects page zero,
 * and paired with a different caption glyph.
 *
 * @param ot Ordering-table pointer.
 * @param prim Primitive-buffer write cursor.
 * @param x_offset Horizontal scroll offset (subtracted from the caption x).
 * @param y_offset Vertical scroll offset (subtracted from the caption y).
 * @return Advanced primitive-buffer write cursor.
 * @see decomp.me (100%)
 */
s32 niki_draw_card_slot1_label(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    TILE* tile;

    if (g_niki_card_slot == 0)
    {
        tile = (TILE*)prim;
        *(u32*)&tile->r0 = 0x101010;
        *((u8*)tile + 3) = 3;
        tile->code = 0x62;
        tile->x0 = 0;
        tile->y0 = 0;
        tile->w = 0x80;
        tile->h = 0x10;
        tile->tag = (tile->tag & 0xFF000000) | (*ot & 0xFFFFFF);
        *ot = (*ot & 0xFF000000) | ((s32)tile & 0xFFFFFF);
        prim += 0x10;
    }

    return func_800A88A0(prim, ot, GLYPH_SYM(D_80147106, 0xE), 4, -x_offset + 0x40, -y_offset, 2);
}

/**
 * @brief Draw the niki save-slot detail panel: element glyphs, the playtime
 *        clock, the slot marker row, and a fallback name/second-line block.
 *
 * Runs only while the panel is active (g_niki_selection_status non-zero) and not suppressed
 * (g_niki_entry_scan_active zero). Depending on g_niki_selection_status it either emits a two-line caption
 * (state 2), or renders the full slot detail: up to three party markers laid out
 * by niki_draw_icon_highlight with an animated highlight (g_niki_icon_phase), the playtime split
 * into hours/minutes via func_800A8A78, and one of three status glyphs. If the
 * slot compare fails it falls back to drawing the stored name and second line.
 *
 * @param ot Ordering-table pointer.
 * @param prim Primitive-buffer write cursor.
 * @param x_offset Horizontal scroll offset (subtracted from every x).
 * @param y_offset Vertical scroll offset (subtracted from every row y).
 * @return Advanced primitive-buffer write cursor.
 * @see decomp.me (100%)
 */
s32 niki_draw_selected_entry_details(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    s32 result;
    Vec2s pos;
    u8 name[0x21];
    char unused_pad[212];
    s32 slot[3];

    result = prim;
    if (g_niki_selection_status == 0)
    {
        return result;
    }
    if (g_niki_entry_scan_active != 0)
    {
        return result;
    }
    if (g_niki_selection_status != 3 && g_niki_entry_state < 0x10)
    {
        if (g_niki_selection_status == 2)
        {
            s32 x = -x_offset;
            u8* base;

            result = func_800A88A0(prim, ot, GLYPH_SYM(D_80147120, 0x28), 4, x, -y_offset, 0);
            base = (u8*)&D_80147120 - 0x28;
            return func_800A88A0(result, ot, GLYPH_OFF(base, 0x2A), 4, x, 0x10 - y_offset, 0);
        }
        else
        {
            s32 term1 = g_niki_card_slot * NIKI_CARD_DIRECTORY_BYTES;
            s32 term2 = (g_niki_selected_row * NIKI_DIRECTORY_ENTRY_BYTES) + (s32)g_niki_entries;

            if (func_8001714C(D_800ECF7C, (char*)(term1 + term2), 0xC) == 0)
            {
                if (D_8003EC9C == 0xFF || D_80164DE7 == D_8003EC9C)
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
                    s32 minutes;
                    s32 time_val;

                    {
                        u8* record = (u8*)&g_niki_entry_metadata;
                        slot[0] = (u32)(*(s32*)(record + 0x18)) >> 0x19;
                        slot[1] = ((u32)(*(s32*)(record + 0x20)) >> 0x12) & 0x7F;
                        slot[2] = (u32)(*(s32*)(record + 0x20)) >> 0x19;
                        g_niki_icon_palette = (s32)record[0x1F];
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
                        time_val = g_niki_icon_phase;
                        if (g_niki_icon_phase < 0)
                        {
                            time_val = g_niki_icon_phase + 0x1F;
                        }
                        g_niki_icon_phase -= (time_val >> 5) << 5;
                        break;
                    case 3:
                        step = 0x10;
                        half_step = 0x20;
                        g_niki_icon_phase %= 0x60;
                        break;
                    default:
                        step = 0x10;
                        half_step = 0x20;
                        g_niki_icon_phase = 0x1F;
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

                            if ((g_niki_icon_phase >= base_y && g_niki_icon_phase < base_x && (delta = g_niki_icon_phase - base_y, 1)) ||
                                (rem = base_x % (half_step * present_count),
                                 g_niki_icon_phase >= rem && g_niki_icon_phase < (hi = rem + half_step) && (delta = hi - g_niki_icon_phase, 1)))
                            {
                                adjust += delta;
                            }
                            result = niki_draw_icon_highlight(result, ot, total - x_offset, -y_offset, adjust, slot[j], i, j);
                            i += 1;
                            total += adjust;
                        }
                    }

                    {
                        u8* base90 = (u8*)&g_niki_entry_metadata;
                        s32 x = -x_offset;
                        s32 y = -y_offset;

                        base_y = *(s32*)(base90 + 0x30);
                        pos.x = (s16)(x + 0x70);
                        pos.y = (s16)y;
                        hours = base_y / 216000;
                        result = func_800A8A78(ot, result, hours, 4, &pos, 1);
                        result = func_800A88A0(result, ot, D_800EC3F6[0] + ((s32)&D_800EC3F6 - 0x32) + (D_800EC3F6[1] << 8), 4, x + 0x6F, y, 0);
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

                        if (*(u16*)(base90 + 0xD4) == *(u16*)((u8*)D_8012271C + 0xD4))
                        {
                            result = func_800A88A0(result, ot, GLYPH_SYM(D_80147148, 0x50), 4, x + 0x54, y + 0x20, 0);
                        }
                        else if (base90[0x17] == 0)
                        {
                            result = func_800A88A0(result, ot, GLYPH_SYM(D_80147146, 0x4E), 4, x + 0x54, y + 0x20, 0);
                        }
                        else
                        {
                            result = func_800A88A0(result, ot, GLYPH_OFF((u8*)D_801475C4, (*(s32*)(base90 + 0x20) & 0x3FFFF) * 2), 4, x + 0x54, y + 0x20, 0);
                        }
                    }
                }
                else
                {
                    result = func_800A88A0(result, ot, GLYPH_SYM(D_8014714C, 0x54), 4, -x_offset, -y_offset, 0);
                }
            }
            else
            {
                s32 j;
                u8* record;

                niki_terminate_multibyte_text(&D_80164B9C);
                record = &D_80164B9C;
                record -= 4;
                if ((u32)(record[0x24] - 1) >= 0x7FU)
                {
                    for (j = 0; j < 0x20; j++)
                    {
                        name[j] = record[4 + j];
                    }
                    name[j] = 0;
                    result = niki_draw_cached_text(result, ot, name, -x_offset, -y_offset, 4, 0);

                    for (j = 0; j < 0x20; j++)
                    {
                        name[j] = ((NikiFallbackText*)&D_80164B98)->text[j];
                    }
                    name[j] = 0;
                    result = niki_draw_cached_text(result, ot, name, -x_offset, -y_offset + 0x10, 4, 0);
                }
            }
        }
    }
    return result;
}

/**
 * @brief Zero-fill the tail of a 0x40-byte record once a terminator is seen.
 *
 * Walks a variable-width byte stream up to logical index 0x40: a lead byte
 * >= 0x80 consumes two positions, otherwise one. On the first zero byte the
 * remaining bytes through index 0x40 are cleared.
 *
 * @param arg0 Pointer to the record buffer to scan and pad.
 * @see decomp.me (100%)
 */
void niki_terminate_multibyte_text(void* arg0)
{
    u8* p;
    s32 i;

    p = (u8*)arg0;
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
 * @brief Draw the niki footer glyph, anchored to the right edge of the panel.
 *
 * Resolves the glyph pointer from the D_800EC3D0 header (a 16-bit offset stored
 * across bytes [0] and [1], added to the header base less 0xC), then submits it
 * at x = 0x80 - arg2, y = -arg3.
 *
 * @param ot Ordering-table pointer.
 * @param prim Primitive-buffer write cursor.
 * @param x_offset Horizontal scroll offset (subtracted from the anchor x).
 * @param y_offset Vertical scroll offset (subtracted from the anchor y).
 * @return Advanced primitive-buffer write cursor.
 * @see decomp.me (100%)
 */
s32 niki_draw_footer_label(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;

    return func_800A88A0(prim, ot, (void*)((u8*)D_800EC3D0 - 0xC + D_800EC3D0[0] + (D_800EC3D0[1] << 8)), 5, 0x80 - x_offset, -y_offset, 2);
}

/**
 * @brief Reset the niki element array: clear the low 3 state bits of each of
 *        the eight g_niki_element_pool entries and reload the g_menu_element_counter counter.
 *
 * @see decomp.me (100%)
 */
void niki_clear_elements(void)
{
    NikiPacket* element;
    s32 element_index;

    g_menu_element_counter = 0x20;
    element = (NikiPacket*)&g_niki_element_pool;
    for (element_index = 0; element_index < NIKI_ELEMENT_COUNT; element_index++)
    {
        element->attr.word &= ~NIKI_ELEMENT_STATE_MASK;
        element++;
    }
}

/**
 * @brief Claim the first free niki element slot, marking its state bits to 1.
 *
 * Scans the eight g_niki_element_pool entries for one whose low 3 state bits are clear,
 * sets them to 1, and returns it. Falls back to the first entry if none free.
 *
 * @return Pointer to the claimed (or fallback) element.
 * @see decomp.me (100%)
 */
NikiElement* niki_alloc_element(void)
{
    NikiPacket* element;
    s32 element_index;

    element = (NikiPacket*)&g_niki_element_pool;
    for (element_index = 0; element_index < NIKI_ELEMENT_COUNT; element_index++, element++)
    {
        if ((element->attr.word & NIKI_ELEMENT_STATE_MASK) == 0)
        {
            element->attr.word = (element->attr.word & ~NIKI_ELEMENT_STATE_MASK) | 1;
            return (NikiElement*)element;
        }
    }
    return &g_niki_element_pool;
}

/**
 * @brief Advance and draw the eight niki element packets for one frame.
 *
 * Reinitialises the GPU primitive window (clip variant selected by
 * frame_arg->frame_flag), then walks the eight g_niki_element_pool element slots. Active slots
 * (low 3 state bits set) are stepped through their animation phase (switch on
 * the state code) and rendered via each slot's draw callback plus func_800AD850,
 * advancing the shared GPU-packet cursor which is written back to frame_arg->prim_cursor.
 *
 * @param frame_arg Per-frame draw state (see NikiFrameState).
 * @see decomp.me (100%)
 */
void niki_update_and_draw_elements(NikiFrameState* frame_arg)
{
    NikiGpuPacket* prim;
    NikiFrameState* frame;
    volatile u32* element_state;
    s32 scaled_width;
    s32 scaled_height;
    s32 element_index;
    s32 draw_area[24];
    u32 dispatch_word;
    s32 state;
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
    u32 hold_word;
    s32 entry_count;

    prim = frame_arg->prim_cursor;
    frame = frame_arg;

    if (frame_arg->frame_flag != 0)
    {
        func_8001C56C(draw_area, 0, 0xF0, 0x140, 0xE0);
    }
    else
    {
        func_8001C56C(draw_area, 0, 8, 0x140, 0xE0);
    }

    element_state = (volatile u32*)&g_niki_element_pool;
    element_index = 0;

    for (; element_index < NIKI_ELEMENT_COUNT; element_index++, element_state += NIKI_ELEMENT_WORD_STRIDE)
    {
        if (*element_state & NIKI_ELEMENT_STATE_MASK)
        {
            entry_count = g_niki_entry_state;
            if ((entry_count < 0x10) && (*(NikiElementDrawFunc*)((u8*)element_state + 8) == (NikiElementDrawFunc)niki_draw_entry_list) &&
                ((g_niki_element1.attr.word & 7) == 2))
            {
                entry_count *= 0xE;
                if ((g_niki_scroll_y + 0x58) < entry_count)
                {
                    prim = (NikiGpuPacket*)func_800AE76C(prim, frame, 0x114, 0x82, 0);
                }
                if (g_niki_scroll_y != 0)
                {
                    prim = (NikiGpuPacket*)func_800AE76C(prim, frame, 0x114, 0x3A, 1);
                }
            }

            func_8001A5D4((s32)prim, draw_area);

            prim->tag = (prim->tag & 0xFF000000) | (frame->head_tag & 0x00FFFFFF);
            frame->head_tag = (s32)((frame->head_tag & 0xFF000000) | ((s32)prim & 0x00FFFFFF));

            dispatch_word = *element_state;
            state = dispatch_word & NIKI_ELEMENT_STATE_MASK;

            prim = (NikiGpuPacket*)((u8*)prim + 0x40);

            switch (state)
            {
            case 1:
                temp_v0_3 = *element_state;
                temp_a1 = *(u32*)((u8*)element_state + 4);
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
                scaled_width = var_v1 >> 3;
                if (var_v0 < 0)
                {
                    var_v0 += 7;
                }
                scaled_height = var_v0 >> 3;
                temp_a3_3 = (s32)(temp_a3_2 - scaled_height);

                prim = (*(NikiElementDrawFunc*)((u8*)element_state + 8))(frame, prim, (s32)(temp_a2 - scaled_width) / 2, temp_a3_3 / 2);
                {
                    u32 post_word;
                    u32 field;
                    u32 high;
                    post_word = *element_state;
                    field = (post_word >> 7) & 0x1FF;
                    high = post_word >> 24;
                    prim =
                        (NikiGpuPacket*)func_800AD850(prim, frame, field + (s32)((((*(u32*)((u8*)element_state + 4) & 1) << 8) | high) - scaled_width) / 2,
                                                      (*((u8*)element_state + 2)) + ((s32)((*(u32*)((u8*)element_state + 4) >> 1) & 0xFF) - scaled_height) / 2,
                                                      scaled_width, scaled_height, frame_arg->frame_flag, element_index == 0);
                }
                {
                    u32 old_word;
                    u32 new_word;
                    old_word = *element_state;
                    new_word = (old_word & ~NIKI_ELEMENT_PHASE_MASK) | (((((old_word >> 3) & 0xF) + 1) & 0xF) * 8);
                    *(u32*)element_state = new_word;
                    if (((new_word >> 3) & 0xF) == 8)
                    {
                        func_800AA02C();
                        *(u32*)element_state = (*element_state & ~7) | 2;
                    }
                }
                break;

            case 2:
                prim = (*(NikiElementDrawFunc*)((u8*)element_state + 8))(frame, prim, 0, 0);
                {
                    u32 case_word;
                    u32 high;
                    case_word = *element_state;
                    high = case_word >> 24;
                    prim = (NikiGpuPacket*)func_800AD850(prim, frame, (case_word >> 7) & 0x1FF, *((u8*)element_state + 2),
                                                         ((*(u32*)((u8*)element_state + 4) & 1) << 8) | high, (*(u32*)((u8*)element_state + 4) >> 1) & 0xFF,
                                                         frame_arg->frame_flag, element_index == 0);
                }
                hold_word = *element_state;
                if (((hold_word >> 3) & 0xF) != 0)
                {
                    *(u32*)element_state = (hold_word & ~NIKI_ELEMENT_PHASE_MASK) | (((((hold_word >> 3) & 0xF) - 1) & 0xF) * 8);
                }
                break;

            case 3:
                temp_a0_5 = *element_state;
                temp_a1 = *(u32*)((u8*)element_state + 4);
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
                scaled_width = var_v1_2 >> 3;
                if (var_v0_2 < 0)
                {
                    var_v0_2 += 7;
                }
                scaled_height = var_v0_2 >> 3;
                temp_a3_6 = (s32)(temp_a3_5 - scaled_height);

                prim = (*(NikiElementDrawFunc*)((u8*)element_state + 8))(frame, prim, (s32)(temp_a2 - scaled_width) / 2, temp_a3_6 / 2);
                {
                    u32 post_word;
                    u32 field;
                    u32 high;
                    post_word = *element_state;
                    field = (post_word >> 7) & 0x1FF;
                    high = post_word >> 24;
                    prim =
                        (NikiGpuPacket*)func_800AD850(prim, frame, field + (s32)((((*(u32*)((u8*)element_state + 4) & 1) << 8) | high) - scaled_width) / 2,
                                                      (*((u8*)element_state + 2)) + ((s32)((*(u32*)((u8*)element_state + 4) >> 1) & 0xFF) - scaled_height) / 2,
                                                      scaled_width, scaled_height, frame_arg->frame_flag, element_index == 0);
                }
                {
                    u32 old_word;
                    old_word = *element_state;
                    var_v1_2 = old_word & ~NIKI_ELEMENT_PHASE_MASK;
                    old_word >>= 3;
                    old_word &= 0xF;
                    old_word--;
                    old_word &= 0xF;
                    old_word <<= 3;
                    var_v1_2 |= old_word;
                    *(u32*)element_state = var_v1_2;
                    if (!(((u32)var_v1_2 >> 3) & 0xF))
                    {
                        *(u32*)element_state = ((((u32)var_v1_2 & ~NIKI_ELEMENT_PHASE_MASK) | 0x18) & ~7) | 4;
                    }
                }
                break;

            case 4:
                temp_v0_5 = *(u32*)element_state;
                g_pad_input = 0;
                hold_word = (temp_v0_5 & ~NIKI_ELEMENT_PHASE_MASK) | (((((temp_v0_5 >> 3) & 0xF) - 1) & 0xF) * 8);
                *(u32*)element_state = hold_word;
                if (!((hold_word >> 3) & 0xF))
                {
                    *(u32*)element_state = hold_word & ~7;
                }
                break;
            }
        }
    }

    frame_arg->prim_cursor = prim;
}

/**
 * @brief Clear the low 3 state bits of the first niki element slot.
 * @see decomp.me (100%)
 */
void niki_deactivate_primary_element(void)
{
    ((NikiPacket*)&g_niki_element_pool)->attr.word &= ~7;
}

/**
 * @brief Append an encoded NIKI string and terminate the result.
 * @param dst Destination string with room for the appended bytes and terminator.
 * @param src Encoded string to append.
 * @see decomp.me (100%)
 */
void niki_text_append(u8* dst, u8* src)
{
    s32 dst_length;
    s32 src_length;
    s32 byte_index;

    dst_length = niki_text_byte_length(dst);
    src_length = niki_text_byte_length(src);
    for (byte_index = 0; byte_index < src_length; byte_index++)
    {
        dst[dst_length + byte_index] = src[byte_index];
    }
    dst[dst_length + byte_index] = 0;
}

/**
 * @brief Measure an encoded NIKI string, skipping trail bytes of extended characters.
 * @param text Encoded string to measure.
 * @return Length in bytes, excluding the terminator.
 * @see decomp.me (100%)
 */
s32 niki_text_byte_length(u8* text)
{
    u8* cursor;
    u8 lead_byte;
    s32 byte_length;

    cursor = text;
    lead_byte = *cursor;
    byte_length = 0;
    while (lead_byte != 0)
    {
        if ((u32)(lead_byte - NIKI_TEXT_EXTENDED_LEAD_FIRST) < NIKI_TEXT_EXTENDED_PAGE_COUNT)
        {
            cursor += 2;
            byte_length += 2;
        }
        else
        {
            cursor += 1;
            byte_length += 1;
        }
        lead_byte = *cursor;
    }
    return byte_length;
}

/**
 * @brief Copy an encoded NIKI string and append its terminator.
 * @param dst Destination buffer with room for the string and terminator.
 * @param src Encoded string to copy.
 * @see decomp.me (100%)
 */
void niki_text_copy(u8* dst, u8* src)
{
    u8* cursor;
    u8 lead_byte;
    s32 byte_length;
    s32 byte_index;

    cursor = src;
    byte_length = 0;

    while (*cursor != 0)
    {
        lead_byte = *(volatile u8*)cursor;

        if ((u32)(lead_byte - NIKI_TEXT_EXTENDED_LEAD_FIRST) < NIKI_TEXT_EXTENDED_PAGE_COUNT)
        {
            cursor += 2;
            byte_length += 2;
        }
        else
        {
            cursor++;
            byte_length++;
        }
    }

    for (byte_index = 0; byte_index < byte_length; byte_index++)
    {
        dst[byte_index] = src[byte_index];
    }

    dst[byte_index] = 0;
}

/**
 * @brief Draw and advance the niki confirmation element.
 *
 * Emits the confirmation caption glyph, then reads input: L1/R1-class cancels
 * (niki_poll_and_rewind_primary_handles result 1 or 2) and the cancel button tear down the element and
 * restore the prior menu; the confirm button spawns the confirmation element at
 * a fixed position with its draw handler set to niki_draw_save_confirm_dialog.
 *
 * @param ot Ordering-table pointer.
 * @param prim Primitive-buffer write cursor.
 * @param x_offset Horizontal scroll offset (subtracted from the caption x).
 * @param y_offset Vertical scroll offset (subtracted from the caption y).
 * @return Advanced primitive-buffer write cursor.
 * @see decomp.me (100%)
 */
s32 niki_draw_confirm_prompt(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    s32 result;
    s32 x;
    s32 status;
    NikiElement* p;

    x = -x_offset + 0x90;
    result = niki_draw_choice_prompt(func_800A88A0(prim, ot, (u8*)&D_80147128 + D_80147128 - 0x30, 4, x, -y_offset, 2), ot, x, 0xE - y_offset);

    if ((u32)(niki_poll_and_rewind_primary_handles() - 1) < 2U)
    {
        g_niki_element_pool.attr.f.state = 0;
        func_800AA02C();
        func_800A3938(0x78, 0x80);
        g_niki_entry_state = 0xFF;
        niki_reset_entry_ranks();
        g_niki_load_step = 0;
    }
    else
    {
        status = g_pad_input;
        if (status & 0x40)
        {
            g_niki_element_pool.attr.f.state = 0;
            func_800AA02C();
            func_800A3938(0x78, 0x80);
            g_niki_load_step = D_801606DC;
        }
        else if (status & 0x220)
        {
            if (g_niki_choice_toggle != 0)
            {
                g_niki_element_pool.attr.f.state = 0;
                func_800AA02C();
                func_800A3938(0x78, 0x80);
                g_niki_load_step = D_801606DC;
            }
            else
            {
                func_800A3938(0x7E, 0x80);
                g_niki_confirm_latch = 1;
                g_niki_load_step = D_801606E4;
                p = &g_niki_element_pool;
                p->draw = niki_draw_save_confirm_dialog;
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
 * @brief Draw the niki save-confirm dialog and, on accept, commit the save.
 *
 * Emits the three caption glyphs and the sub-panel (niki_draw_progress_bar). When the
 * confirm latch g_niki_confirm_latch is clear, reads the selected save resource: if the
 * slot is empty (niki_validate_save_blob == 0) it just re-arms the chooser, otherwise it
 * copies the save payload out, kicks the write, and flips every active element
 * to its closing animation state.
 *
 * @param ot Ordering-table pointer.
 * @param prim Primitive-buffer write cursor.
 * @param x_offset Horizontal scroll offset (subtracted from every caption x).
 * @param y_offset Vertical scroll offset (subtracted from every caption y).
 * @return Advanced primitive-buffer write cursor.
 * @see decomp.me (100%)
 */
s32 niki_draw_save_confirm_dialog(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    u8* base;
    u8* resource;
    NikiPacket* p;
    NikiPacket* cursor;
    s32 result;
    s32 x;
    s32 i;

    x = -x_offset + 0x90;
    result = func_800A88A0(prim, ot, (void*)((s32)&D_8014712A - 0x32 + D_8014712A), 4, x, -y_offset, 2);
    base = (u8*)&D_8014712A - 0x32;
    result = func_800A88A0(result, ot, base + *(u16*)(base + 0x1E), 4, x, 0xE - y_offset, 2);
    result = func_800A88A0(result, ot, base + *(u16*)(base + 0xB2), 4, x, 0x1C - y_offset, 2);
    result = niki_draw_progress_bar(result, ot);

    if (g_niki_confirm_latch == 0)
    {
        resource = g_niki_save_blob;
        p = (NikiPacket*)&g_niki_element_pool;
        p->attr.f.state = 0;
        if (niki_validate_save_blob((NikiSaveBlob*)resource) == 0)
        {
            niki_open_status_dialog(4);
            return result;
        }

        func_800A3938(0x7B, 0x80);
        D_8011F428 = 1;
        D_801227CC = *(u16*)&resource[0x254];
        D_801227F4 = *(u16*)&resource[0x256];
        D_8011F418 = g_niki_card_slot;
        func_800170BC(D_8011F3D8, D_80164E70);
        func_80016E7C(&resource[0x32E0], D_80122A08, 0x100);
        func_80067F28();

        cursor = p;
        for (i = 0; i < 8; i++, cursor++)
        {
            if (cursor->attr.f.state != 0)
            {
                cursor->attr.f.state = 3;
                cursor->attr.f.phase = 8;
            }
        }
        func_80067F5C(8);
    }

    return result;
}

/**
 * @brief Draw the active save-progress bar with width determined by elapsed ticks.
 * @param prim GPU packet write cursor.
 * @param ot Ordering-table entry receiving the bar.
 * @return Advanced packet cursor, or the original cursor when the timer is inactive.
 * @see decomp.me (100%)
 */
s32 niki_draw_progress_bar(s32 prim, s32* ot)
{
    NikiPolyG4Packet* bar;
    s32 elapsed;
    s32 extent;
    s32 color;

    bar = (NikiPolyG4Packet*)prim;
    if (g_niki_progress_bar_active != 0)
    {
        elapsed = func_8002054C(-1) - g_niki_progress_start_tick;
        if (elapsed >= NIKI_PROGRESS_DURATION + 1)
        {
            elapsed = NIKI_PROGRESS_DURATION;
        }
        color = 0xFFFF00;
        extent = elapsed * NIKI_PROGRESS_WIDTH;
        bar->color0.word = 0xFF;
        bar->color1.word = 0xFFFF;
        bar->color3.word = 0xFF0000;
        bar->tag.bytes.length = 8;
        bar->color2.word = color;
        bar->color0.bytes.code = 0x38;
        bar->x2 = 0;
        bar->x0 = 0;
        if (extent < 0)
        {
            extent += 0xFF;
        }
        bar->x3 = extent >> 8;
        bar->x1 = extent >> 8;
        bar->y1 = 0;
        bar->y0 = 0;
        bar->y3 = NIKI_PROGRESS_HEIGHT;
        bar->y2 = NIKI_PROGRESS_HEIGHT;
        bar->tag.word = (bar->tag.word & 0xFF000000) | (*ot & 0xFFFFFF);
        *ot = (*ot & 0xFF000000) | (prim & 0xFFFFFF);
        prim += sizeof(NikiPolyG4Packet);
    }
    return prim;
}

/**
 * @see decomp.me (100%)
 */
void niki_open_status_dialog(s32 dialog_state)
{
    func_800A3938(0x78, 0x80);
    g_niki_element_pool.draw = (void*)niki_draw_status_dialog;
    g_niki_element_pool.attr.f.phase = 1;
    g_niki_element_pool.attr.f.state = 1;
    g_niki_element_pool.attr.f.x = 0x20;
    g_niki_element_pool.attr.f.code = 0x70;
    g_niki_element_pool.active = 1;
    g_niki_element_pool.y = 0x14;
    SET_ELEM_CODE(&g_niki_element_pool, 0);
    func_800AA02C();
    g_niki_progress_active = 0;
    g_niki_confirm_latch = 0;
    g_niki_selection_status = 0;
    g_niki_io_busy = 0;
    g_niki_entry_state = 0xFF;
    niki_reset_entry_ranks();
    g_niki_load_step = 0;
    g_niki_dialog_state = dialog_state;
}

/**
 * @see decomp.me (100%)
 */
void niki_open_secondary_status_dialog(s32 dialog_state)
{
    func_800A3938(0x78, 0x80);
    g_niki_element1.draw = (void*)niki_draw_secondary_status_dialog;
    g_niki_element1.attr.f.phase = 1;
    g_niki_element1.attr.f.state = 1;
    g_niki_element1.attr.f.x = 0x20;
    g_niki_element1.attr.f.code = 0x70;
    g_niki_element1.active = 1;
    g_niki_element1.y = 0x14;
    SET_ELEM_CODE(&g_niki_element1, 0);
    func_800AA02C();
    D_8011F428 = 2;
    g_niki_progress_active = 0;
    g_niki_confirm_latch = 0;
    g_niki_selection_status = 0;
    g_niki_io_busy = 0;
    niki_reset_entry_ranks();
    g_niki_load_step = 0;
    g_niki_dialog_state = dialog_state;
}

/**
 * @see decomp.me (100%)
 */
s32 niki_draw_status_dialog(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;

    switch (g_niki_dialog_state)
    {
    case 0:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80147134, 0x3C), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 2:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80147138, 0x40), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 3:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014713A, 0x42), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 1:
    case 4:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80147136, 0x3E), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    }
    if (g_pad_input & 0x220)
    {
        g_niki_element_pool.attr.f.state = 0;
        func_800AA02C();
    }
    return prim;
}

/**
 * @see decomp.me (100%)
 */
s32 niki_draw_secondary_status_dialog(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    NikiPacket* p;
    s32 i;

    switch (g_niki_dialog_state)
    {
    case 0:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80147134, 0x3C), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 2:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80147138, 0x40), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 3:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014713A, 0x42), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 1:
    case 4:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80147136, 0x3E), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    }
    if (g_pad_input & 0x220)
    {
        g_menu_element_counter = 0x20;
        p = (NikiPacket*)&g_niki_element_pool;
        for (i = 0; i < 8; i++)
        {
            p->attr.word &= ~7;
            p++;
        }
        func_80067F5C(8);
        func_800AA02C();
    }
    return prim;
}

/**
 * @brief Upload a save-file icon and append its textured quad to the ordering table.
 * @param prim GPU packet write cursor.
 * @param ot Ordering-table entry receiving the quad.
 * @param x Left edge of the icon.
 * @param y Top edge of the icon.
 * @param width Displayed icon width.
 * @param icon_index Icon resource index; 0x7F skips drawing.
 * @param texture_slot VRAM slot for the icon texture and palette.
 * @param palette_mode Selects the special palette path when one and the icon index is below two.
 * @return Advanced packet cursor, or the original cursor when drawing is skipped.
 * @see decomp.me (100%)
 */
s32 niki_draw_icon_highlight(s32 prim, s32* ot, s32 x, s32 y, s32 width, s32 icon_index, s32 texture_slot, s32 palette_mode)
{
    RECT rect;
    NikiTexturedQuad* quad;
    s32 texture_column;
    s8 texture_u;

    if (icon_index == 0x7F)
    {
        return prim;
    }
    rect.x = texture_slot * 0x10;
    rect.y = 0x1F2;
    rect.w = 0x10;
    rect.h = 1;
    if ((palette_mode == 1) && (icon_index < 2))
    {
        func_800A5638(g_niki_icon_context, icon_index);
        func_80019A34(&rect, g_niki_icon_context);
        func_80019788(0);
    }
    else if (icon_index >= 0x4F)
    {
        func_800A55E4(g_niki_icon_context, g_niki_icon_palette);
        func_80019A34(&rect, g_niki_icon_context);
        func_80019788(0);
    }
    else
    {
        func_80019A34(&rect, (void*)((u8*)&D_801477AC - 4 + D_801477AC[icon_index]));
    }

    texture_column = texture_slot * 3;
    rect.x = texture_column * 4 + 0x140;
    rect.y = 0xD0;
    rect.w = 0xC;
    rect.h = 0x30;
    func_80019A34(&rect, (void*)((u8*)&D_801477AC + 0x1C + D_801477AC[icon_index]));
    quad = (NikiTexturedQuad*)prim;
    quad->color.word = 0x808080;
    quad->tag.bytes.length = 9;
    quad->color.bytes.code = 0x2C;
    quad->x2 = x;
    quad->x0 = x;
    quad->y1 = y;
    quad->y0 = y;
    quad->x3 = x + width;
    texture_u = texture_column * 0x10;
    quad->u2 = texture_u;
    quad->u0 = texture_u;
    texture_u += 0x2F;
    quad->u3 = texture_u;
    quad->u1 = texture_u;
    quad->v1 = 0xD0;
    quad->v0 = 0xD0;
    quad->x1 = x + width;
    quad->y3 = y + 0x2F;
    quad->y2 = y + 0x2F;
    quad->v3 = 0xFF;
    quad->v2 = 0xFF;
    quad->clut = (texture_slot & 0x3F) | 0x7C80;
    quad->tpage = 5;
    quad->tag.word = (quad->tag.word & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & 0xFF000000) | (prim & 0xFFFFFF);
    return prim + 0x28;
}

/**
 * @see decomp.me (100%)
 */
void niki_enable_choice_toggle(void)
{
    g_niki_choice_toggle = 1;
}

/**
 * @see decomp.me (100%)
 */
s32 niki_draw_choice_prompt(s32 prim, s32* ot, s32 x, s32 y)
{
    u8* p;
    u8* base;
    s32 g1;
    s32 g2;
    s32 hi;
    s32 a3;

    p = (u8*)&D_800EC3FA;
    hi = p[1] << 8;
    base = p - 0x36;
    a3 = 4;
    g1 = p[0] + (hi + (s32)base);
    if (g_niki_choice_toggle != 0)
    {
        a3 = 5;
    }
    prim = func_800A88A0(prim, ot, (void*)g1, a3, x - 0x10, y, 1);
    a3 = 4;
    g2 = base[0x38] + ((base[0x39] << 8) + (s32)base);
    if (g_niki_choice_toggle == 0)
    {
        a3 = 5;
    }
    prim = func_800A88A0(prim, ot, (void*)g2, a3, x + 8, y, 0);
    if (g_pad_input & 0xA000)
    {
        g_niki_choice_toggle ^= 1;
        func_800A3938(0x7D, 0x80);
        g_pad_input = 0;
    }
    return prim;
}

/**
 * @brief Draw/update dispatcher for the niki save-menu state machine.
 *
 * Dispatches on the state code in g_niki_entry_state (0xF3..0xFF) through the rodata
 * jump table jtbl_80140054, emitting the glyph primitives for the active
 * dialog page and advancing the state on pad input.
 *
 * @param ot Ordering-table entry the glyph primitives are linked into.
 * @param prim GPU packet write cursor.
 * @param arg2 X scroll offset subtracted from all glyph positions.
 * @param arg3 Y scroll offset subtracted from all glyph positions.
 * @return Advanced GPU packet cursor.
 *
 * @note The `switch (0)` wrapper, the static `keep[]` label-address array and
 *       `goto *jtbl_80140054[dispatch]` reproduce the original rodata jump
 *       table dispatch; same computed-goto pattern as the menu.c matches.
 * @note Verified 100.000000% (962/962 exact, gcc272_cdk) in-tree 2026-08-25;
 *       scratch history in working/niki_draw_state_page/.
 */
s32 niki_draw_state_page(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    s32 dispatch;
    static void* const keep[] __attribute__((section(".discard"))) = {&&niki_f3, &&niki_f4, &&niki_f5, &&niki_f6, &&niki_f7, &&niki_f8, &&niki_f9,
                                                                      &&niki_fa, &&niki_fb, &&niki_fc, &&niki_fd, &&niki_fe, &&niki_ff};
    switch (0)
    {
    case 0:
        dispatch = g_niki_entry_state - 0xF3;
        if ((u32)dispatch >= 0xD)
        {
            goto niki_default;
        }
        goto* jtbl_80140054[dispatch];
    niki_f8:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014712C, 0x34), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    niki_f9:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014712C, 0x34), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    niki_ff:
    {
        s32 x;
        u8* base;
        x = -x_offset + 0x90;
        base = (u8*)&D_801470F8;
        prim = func_800A88A0(prim, ot, base + D_801470F8, 4, x, -y_offset, 2);
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x1E), 4, x, 0xE - y_offset, 2);
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - y_offset, 2);
    }
    break;
    niki_fa:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014712C, 0x34), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    niki_fd:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_801470FC, 4), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    niki_fb:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80147108, 0x10), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    niki_fc:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014710A, 0x12), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    niki_f7:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80147160, 0x68), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    niki_f6:
    {
        s32 x;
        u8* base;
        NikiPolyG4Packet* g;
        s32 next;
        s32 elapsed;
        s32 extent;
        s32 color;
        s32 finalmode;

        x = -x_offset + 0x90;
        prim = func_800A88A0(prim, ot, (void*)((s32)&D_8014712A - 0x32 + D_8014712A), 4, x, -y_offset, 2);
        base = (u8*)&D_8014712A - 0x32;
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x1E), 4, x, 0xE - y_offset, 2);
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - y_offset, 2);

        next = prim;
        g = (NikiPolyG4Packet*)prim;
        if (g_niki_progress_bar_active != 0)
        {
            elapsed = func_8002054C(-1) - g_niki_progress_start_tick;
            if (elapsed >= NIKI_PROGRESS_DURATION + 1)
            {
                elapsed = NIKI_PROGRESS_DURATION;
            }
            color = 0xFFFF00;
            extent = elapsed * NIKI_PROGRESS_WIDTH;
            g->color0.word = 0xFF;
            g->color1.word = 0xFFFF;
            g->color3.word = 0xFF0000;
            g->tag.bytes.length = 8;
            g->color2.word = color;
            g->color0.bytes.code = 0x38;
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
            g->y3 = NIKI_PROGRESS_HEIGHT;
            g->y2 = NIKI_PROGRESS_HEIGHT;
            g->tag.word = (g->tag.word & 0xFF000000) | (*ot & 0xFFFFFF);
            *ot = (*ot & 0xFF000000) | (prim & 0xFFFFFF);
            next = prim + 0x24;
        }
        prim = next;

        if (g_niki_confirm_latch == 0)
        {
            if (niki_validate_save_blob((NikiSaveBlob*)g_niki_save_blob) == 0)
            {
                func_800A3938(0x78, 0x80);
                g_niki_element_pool.draw = (void*)niki_draw_status_dialog;
                g_niki_element_pool.attr.f.phase = 1;
                g_niki_element_pool.attr.f.state = 1;
                g_niki_element_pool.attr.f.x = 0x20;
                g_niki_element_pool.attr.f.code = 0x70;
                g_niki_element_pool.active = 1;
                g_niki_element_pool.y = 0x14;
                SET_ELEM_CODE(&g_niki_element_pool, 0);
                func_800AA02C();
                g_niki_progress_active = 0;
                g_niki_selection_status = 0;
                g_niki_io_busy = 0;
                g_niki_confirm_latch = 0;
                g_niki_entry_state = 0xFF;
                niki_reset_entry_ranks();
                finalmode = 4;
                g_niki_load_step = 0;
                g_niki_dialog_state = finalmode;
                return prim;
            }
            func_800A3938(0x7B, 0x80);
            g_niki_entry_state = 0xF4;
            g_niki_choice_toggle = 1;
            func_800AA02C();
        }
    }
    break;
    niki_f3:
    {
        s32 x;
        s32 result;
        s32 y;
        u8* p;
        u8* base;
        s32 g1;
        s32 g2;
        s32 hi;
        s32 a3;
        NikiPacket* packet;
        s32 i;

        x = -x_offset;
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014716A, 0x72), 4, x + 0x90, -y_offset, 2);
        y = 0xE - y_offset;
        p = (u8*)&D_800EC3FA;
        hi = p[1] << 8;
        base = p - 0x36;
        a3 = 4;
        g1 = p[0] + (hi + (s32)base);
        if (g_niki_choice_toggle != 0)
        {
            a3 = 5;
        }
        result = func_800A88A0(prim, ot, (void*)g1, a3, x + 0x80, y, 1);
        a3 = 4;
        g2 = base[0x38] + ((base[0x39] << 8) + (s32)base);
        if (g_niki_choice_toggle == 0)
        {
            a3 = 5;
        }
        result = func_800A88A0(result, ot, (void*)g2, a3, x + 0x98, y, 0);
        if (g_pad_input & 0xA000)
        {
            g_niki_choice_toggle ^= 1;
            func_800A3938(0x7D, 0x80);
            g_pad_input = 0;
        }

        prim = result;

        if (g_pad_input & 0x40)
        {
            func_800A3938(0x78, 0x80);
            g_niki_choice_toggle = 1;
            g_niki_entry_state = 0xF4;
            func_800AA02C();
        }
        else if (g_pad_input & 0x220)
        {
            if (g_niki_choice_toggle != 0)
            {
                func_800A3938(0x78, 0x80);
                g_niki_choice_toggle = 1;
                g_niki_entry_state = 0xF4;
                func_800AA02C();
            }
            else
            {
                func_800A3938(0x7D, 0x80);
                packet = (NikiPacket*)&g_niki_element_pool;
                D_8011F428 = 2;
                g_menu_element_counter = 0x20;
                for (i = 0; i < 8; i++, packet++)
                {
                    packet->attr.f.state = 0;
                }
                func_80067F5C(8);
                func_800AA02C();
            }
        }
    }
    break;
    niki_f4:
    {
        s32 x;
        s32 result;
        s32 one;
        s32 y;
        u8* glyphbase;
        u8* p;
        u8* base;
        s32 g1;
        s32 g2;
        s32 hi;
        s32 a3;
        s32 count;
        s32 i;
        u8* cursor;
        u8* resource;
        s32 temp;

        x = -x_offset;
        prim = func_800A88A0(prim, ot, (void*)((s32)&D_80147162 - 0x6A + D_80147162), 4, x + 0x90, -y_offset, 2);
        glyphbase = (u8*)&D_80147162 - 0x6A;
        prim = func_800A88A0(prim, ot, GLYPH_OFF(glyphbase, 0x70), 4, x + 0x90, 0xE - y_offset, 2);

        y = 0x1C - y_offset;
        p = (u8*)&D_800EC3FA;
        hi = p[1] << 8;
        base = p - 0x36;
        a3 = 4;
        g1 = p[0] + (hi + (s32)base);
        if (g_niki_choice_toggle != 0)
        {
            a3 = 5;
        }
        one = 1;
        result = func_800A88A0(prim, ot, (void*)g1, a3, x + 0x80, y, one);
        a3 = 4;
        g2 = base[0x38] + ((base[0x39] << 8) + (s32)base);
        if (g_niki_choice_toggle == 0)
        {
            a3 = 5;
        }
        result = func_800A88A0(result, ot, (void*)g2, a3, x + 0x98, y, 0);
        if (g_pad_input & 0xA000)
        {
            g_niki_choice_toggle ^= 1;
            func_800A3938(0x7D, 0x80);
            g_pad_input = 0;
        }

        prim = result;

        if (g_pad_input & 0x40)
        {
            goto f4_accept;
        }
        if (g_pad_input & 0x220)
        {
            if (g_niki_choice_toggle != 0)
            {
            f4_accept:
                g_niki_choice_toggle = one;
                g_niki_entry_state = 0xF3;
                func_800A3938(0x78, 0x80);
                func_800AA02C();
            }
            else
            {
                func_800A3938(0x7E, 0x80);
                cursor = D_80122A08;
                resource = g_niki_save_blob;
                func_80016E7C(D_80122A08, resource + 0x32E0, 0x100);
                count = 0;
                for (i = 0; i < 4; i++)
                {
                    if (cursor[i * 0x40] != 0)
                    {
                        count++;
                    }
                }
                resource[0x197] = count;
                temp = niki_compute_save_checksum(resource);
                ((NikiSaveBlob*)resource)->magic = NIKI_SAVE_MAGIC;
                ((NikiSaveBlob*)resource)->checksum = temp;
                g_niki_progress_active = 1;
                g_niki_load_step = &D_801606F5;
                g_niki_entry_state = 0xF5;
            }
        }
    }
    break;
    niki_f5:
    {
        s32 x;
        u8* base;
        NikiPolyG4Packet* g;
        s32 next;
        s32 elapsed;
        s32 extent;
        s32 color;
        NikiPacket* packet;
        s32 i;

        x = -x_offset + 0x90;
        prim = func_800A88A0(prim, ot, (void*)((s32)&D_80147114 - 0x1C + D_80147114), 4, x, -y_offset, 2);
        base = (u8*)&D_80147114 - 0x1C;
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x1E), 4, x, 0xE - y_offset, 2);
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - y_offset, 2);

        next = prim;
        g = (NikiPolyG4Packet*)prim;
        if (g_niki_progress_bar_active != 0)
        {
            elapsed = func_8002054C(-1) - g_niki_progress_start_tick;
            if (elapsed >= NIKI_PROGRESS_DURATION + 1)
            {
                elapsed = NIKI_PROGRESS_DURATION;
            }
            color = 0xFFFF00;
            extent = elapsed * NIKI_PROGRESS_WIDTH;
            g->color0.word = 0xFF;
            g->color1.word = 0xFFFF;
            g->color3.word = 0xFF0000;
            g->tag.bytes.length = 8;
            g->color2.word = color;
            g->color0.bytes.code = 0x38;
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
            g->y3 = NIKI_PROGRESS_HEIGHT;
            g->y2 = NIKI_PROGRESS_HEIGHT;
            g->tag.word = (g->tag.word & 0xFF000000) | (*ot & 0xFFFFFF);
            *ot = (*ot & 0xFF000000) | (prim & 0xFFFFFF);
            next = prim + 0x24;
        }
        prim = next;

        if (g_niki_progress_active == 0)
        {
            func_800A3938(0x7A, 0x80);
            g_menu_element_counter = 0x20;
            packet = (NikiPacket*)&g_niki_element_pool;
            for (i = 0; i < 8; i++, packet++)
            {
                packet->attr.f.state = 0;
            }
            func_80067F5C(8);
            D_8011F428 = 0;
        }
    }
    break;
    niki_default:
    {
        s32 x;
        u8* base;
        s32 pos;
        s32 diff;

        x = -x_offset + 0x90;
        base = (u8*)&D_801470F8;
        prim = func_800A88A0(prim, ot, base + D_801470F8, 4, x, -y_offset, 2);
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x1E), 4, x, 0xE - y_offset, 2);
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - y_offset, 2);

        if (g_niki_entry_scan_active == 0)
        {
            if (g_niki_io_busy != 0)
            {
                return prim;
            }
            if ((u32)(*g_niki_load_step - 6) < 2U)
            {
                return prim;
            }
            if ((func_8001714C(D_800ECF7C, &g_niki_entries[g_niki_card_slot][g_niki_selected_row], 0xC) != 0) || (g_niki_entry_metadata.unkD4 != D_801227CC) ||
                (g_niki_entry_metadata.unkD6 != D_801227F4))
            {
                g_niki_selected_row++;
                if (g_niki_selected_row >= g_niki_entry_state)
                {
                    if (g_niki_entry_state != 0)
                    {
                        g_niki_entry_state = 0xF7;
                    }
                    else
                    {
                        g_niki_entry_state = 0xF8;
                    }
                }
                else
                {
                    niki_commit_selected_entry();
                    pos = g_niki_selected_row * 0xE;
                    diff = pos - g_niki_scroll_y;
                    if (diff >= 0x4B)
                    {
                        g_niki_scroll_target_y = pos - 0x46;
                        g_niki_scroll_frames = 4;
                    }
                    if (diff < 0)
                    {
                        g_niki_scroll_target_y = pos;
                        g_niki_scroll_frames = 4;
                    }
                }
            }
            else
            {
                g_niki_progress_start_tick = func_8002054C(-1);
                g_niki_confirm_latch = 1;
                g_niki_load_step = &D_801606EC;
                g_niki_entry_state = 0xF6;
            }
        }
    }
    break;
    }

niki_fe:
    if (g_niki_io_busy != 0)
    {
        return prim;
    }
    if (g_niki_entry_state == 0xF6)
    {
        return prim;
    }
    if (g_niki_entry_state == 0xF5)
    {
        return prim;
    }
    if (g_niki_entry_state == 0xF4)
    {
        return prim;
    }
    if (g_niki_entry_state == 0xF3)
    {
        return prim;
    }

    if (g_pad_input & 0x40)
    {
        s32* p;
        s32 i;
        s32 word;
        D_80122994 = 3;
        func_800A3938(0x78, 0x80);
        func_80067F28();
        p = (s32*)&g_niki_element_pool;
        i = 0;
        do
        {
            word = *p;
            if (word & 7)
            {
                *p = (((word & ~7) | 3) & ~NIKI_ELEMENT_PHASE_MASK) | 0x40;
            }
            i++;
            p += 3;
        } while (i < 8);
        return prim;
    }

    if ((g_pad_input & 0xA100) && (g_niki_entry_state != 0xFF))
    {
        func_800A3938(0x7D, 0x80);
        D_80164B80 = 0;
        g_niki_load_step = 0;
        g_niki_scroll_frames = 0;
        g_niki_scroll_target_y = 0;
        g_niki_scroll_y = 0;
        g_niki_selected_row = 0;
        g_niki_entry_state = 0xFF;
        g_niki_selection_status = 0;
        g_niki_card_slot ^= 1;
        niki_reset_entry_ranks();
        g_niki_progress_bar_active = 0;
        g_niki_load_step = D_801606C8;
    }

    return prim;
}

/**
 * @brief Advance past a run of ASCII hexadecimal-digit characters.
 * @param text Pointer to the start of the scan.
 * @return Pointer to the first byte that is not a hex digit
 *         (@c '0'-'9', @c 'a'-'f' or @c 'A'-'F').
 * @see decomp.me (100.00%)
 */
u8* niki_skip_hex_digits(void* text)
{
    u8* cursor = text;

    while ((u32)(*cursor - '0') < 10 || (u32)(*cursor - 'a') < 6 || (u32)(*cursor - 'A') < 6)
    {
        cursor++;
    }
    return cursor;
}

/**
 * @brief Validate the save payload checksum and format marker.
 * @param blob Serialized save data to validate.
 * @return One when both checks pass, otherwise zero.
 * @see decomp.me (100.00%)
 */
s32 niki_validate_save_blob(NikiSaveBlob* blob)
{
    if (blob->checksum == niki_compute_save_checksum(blob->payload))
    {
        if (blob->magic == NIKI_SAVE_MAGIC)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Sum the save payload bytes and apply the checksum scale and bias.
 * @param data Start of the fixed-size save payload.
 * @return Twice the byte sum plus NIKI_SAVE_CHECKSUM_BIAS.
 * @see decomp.me (100.00%)
 */
s32 niki_compute_save_checksum(u8* data)
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
    } while (byte_index < NIKI_SAVE_PAYLOAD_BYTES);
    return sum * 2 + NIKI_SAVE_CHECKSUM_BIAS;
}

/**
 * @brief Format up to six decimal digits as full-width Shift-JIS characters.
 * @param out Destination byte buffer.
 * @param value Number to format.
 * @return Pointer to the terminator, or six bytes past the start for the overflow string.
 * @note Values at least 1000000 use the fixed overflow string; leading zeroes are suppressed.
 * @see decomp.me (100.00%)
 */
s8* niki_format_decimal(s8* out, s32 value)
{
    s32 digit;
    s32 divisor;
    s32 started;
    s8* cursor;

    cursor = out;
    divisor = 100000;
    if (value >= divisor * 10)
    {
        *(NikiDecimalOverflow*)cursor = *(NikiDecimalOverflow*)D_80140088;
        return cursor + 6;
    }

    started = 0;
    do
    {
        digit = value / divisor;
        if (digit != 0 || started != 0)
        {
            *cursor++ = (digit + 0x824F) >> 8;
            *cursor++ = digit + 0x4F;
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
    *cursor = 0;
    return cursor;
}

/**
 * @brief Format a hexadecimal string with leading zeroes suppressed.
 * @param out Destination character buffer.
 * @param value Number to format.
 * @param max_chars Maximum number of digits to emit, excluding the terminator.
 * @see decomp.me (100.00%)
 */
void niki_format_hex(s8* out, s32 value, s32 max_chars)
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
                niki_hex_nibble_to_ascii(out, nibble);
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
 * @brief Convert a 0-15 value to its ASCII hexadecimal digit.
 * @param out Destination byte.
 * @param value Nibble value; 0-9 -> '0'-'9', 10-15 -> 'A'-'F', else '_'.
 * @return None.
 * @see decomp.me (100.00%)
 */
void niki_hex_nibble_to_ascii(s8* out, s32 value)
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
 * @brief Parse a bounded run of ASCII hexadecimal digits.
 * @param text First digit to parse.
 * @param digits_left Maximum number of digits to consume.
 * @return Accumulated value; parsing stops at the digit limit or first non-hex byte.
 * @see decomp.me (100.00%)
 */
u32 niki_parse_hex(u8* text, s32 digits_left)
{
    u32 result;

    result = 0;
    while (((u8)(*text - '0') < 10) || ((u8)(*text - 'a') < 6) || ((u8)(*text - 'A') < 6))
    {
        if (digits_left == 0)
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
        digits_left--;
    }
    return result;
}

/**
 * @brief Skip a hexadecimal run and its separator, then parse up to two hex digits.
 * @param text Start of the leading hexadecimal run.
 * @return Parsed suffix byte.
 * @see decomp.me (100.00%)
 */
s32 niki_parse_hex_suffix_byte(u8* text)
{
    s32 digits_left;
    u32 result;

    while ((u32)(*text - '0') < 10 || (u32)(*text - 'a') < 6 || (u32)(*text - 'A') < 6)
    {
        text++;
    }

    text++;
    digits_left = 2;
    result = 0;
    while (((u8)(*text - '0') < 10) || ((u8)(*text - 'a') < 6) || ((u8)(*text - 'A') < 6))
    {
        if (digits_left == 0)
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
        digits_left--;
    }
    return result;
}

/**
 * @brief Parse field values and suffix bytes from recognized save-file names.
 * @return Largest suffix byte among the recognized entries.
 * @see decomp.me (100%)
 */
s32 niki_parse_entry_fields(void)
{
    s32 entry_index;
    s32 max_suffix;
    u8* cursor;
    u8* suffix;
    s32 digits_left;
    s32 value;
    u32 decimal_base;
    u32 uppercase_base;
    u32 lowercase_base;
    s32 suffix_value;

    entry_index = 0;
    max_suffix = entry_index;
    while (entry_index < g_niki_entry_state)
    {
        u8* pattern;
        pattern = (u8*)&D_800ECF7C;
        if (func_8001714C(pattern, (u8*)&g_niki_entries[g_niki_card_slot][entry_index], 0xC) == 0)
        {
            digits_left = 5;
            cursor = (u8*)(g_niki_card_slot * NIKI_CARD_DIRECTORY_BYTES + entry_index * NIKI_DIRECTORY_ENTRY_BYTES + (s32)g_niki_entries + 0xC);
            value = 0;
            while (((u8)(*cursor - '0') < 10) || ((u8)(*cursor - 'a') < 6) || ((u8)(*cursor - 'A') < 6))
            {
                if (digits_left == 0)
                {
                    break;
                }
                value <<= 4;
                if ((u8)(*cursor - '0') < 10)
                {
                    decimal_base = value - '0';
                    value = decimal_base + *cursor;
                }
                else if ((u8)(*cursor - 'A') < 6)
                {
                    uppercase_base = value - ('A' - 10);
                    value = uppercase_base + *cursor;
                }
                else if ((u8)(*cursor - 'a') < 6)
                {
                    lowercase_base = value - ('a' - 10);
                    value = lowercase_base + *cursor;
                }
                cursor++;
                digits_left--;
            }
            suffix = &g_niki_entries[g_niki_card_slot][entry_index].name[0xC];
            {
                s32* fields = &g_niki_entry_fields[g_niki_card_slot * NIKI_DIRECTORY_ENTRY_COUNT];
                fields[entry_index] = value;
            }
            suffix_value = niki_parse_hex_suffix_byte(suffix);
            g_niki_entry_suffix_values[entry_index] = suffix_value;
            if (max_suffix < suffix_value)
            {
                max_suffix = suffix_value;
            }
        }
        else
        {
            s32* fields = &g_niki_entry_fields[g_niki_card_slot * NIKI_DIRECTORY_ENTRY_COUNT];
            fields[entry_index] = -1;
            g_niki_entry_suffix_values[entry_index] = 0;
        }
        entry_index++;
    }
    return max_suffix;
}

/**
 * @brief Rank recognized entries and select the entry with the greatest field value.
 * @param unused0 Unused.
 * @param unused1 Unused.
 * @param unused2 Unused.
 * @return Index of the greatest field value, or zero when none is present.
 */
s32 niki_rank_entries(s32 unused0, s32 unused1, s32 unused2)
{
    s32* fields;
    s32* field;
    s32* rank;
    s32* previous_field;
    s32* previous_rank;
    s32* ranks;
    s32* current_field;
    s32* candidate;
    s32* field_table;
    s32* entry_field_table;
    s32 slot;
    s32* suffix_output;
    char* entry_cursor;
    s32 next_rank;
    s32 entry_index;
    s32 maximum;
    s32 entry_count;
    s32 max_suffix;
    s32 higher_count;
    s32 previous_index;

    niki_parse_entry_fields();
    maximum = -1;
    niki_sort_entries_by_type();
    entry_index = 0;
    max_suffix = niki_parse_entry_fields();
    niki_reset_entry_ranks();
    next_rank = 1;
    if (g_niki_entry_state > 0)
    {
        entry_count = g_niki_entry_state;
        ranks = &g_niki_entry_ranks[0];
        rank = ranks;
        slot = g_niki_card_slot;
        entry_field_table = g_niki_entry_fields;
        fields = entry_field_table + slot * NIKI_DIRECTORY_ENTRY_COUNT;
        field = fields;
        do
        {
            if (*field >= 0)
            {
                previous_index = 0;
                if (entry_index > 0)
                {
                    previous_index += 1;
                    previous_index -= 1;
                }
                if (*field >= maximum)
                {
                    *rank = next_rank;
                    maximum = *field;
                    next_rank += 1;
                }
                else
                {
                    higher_count = previous_index;
                    if (entry_index > 0)
                    {
                        current_field = field;
                        previous_rank = ranks;
                        previous_field = fields;
                        do
                        {
                            if (*current_field < *previous_field)
                            {
                                higher_count += 1;
                                *previous_rank += 1;
                            }
                            previous_rank += 1;
                            previous_index += 1;
                            previous_field += 1;
                        } while (previous_index < entry_index);
                    }
                    {
                        s32 rank_value;
                        do
                        {
                            do
                            {
                                do
                                {
                                    rank_value = next_rank - higher_count;
                                } while (0);
                            } while (0);
                        } while (0);
                        *rank = rank_value;
                    }
                    next_rank += 1;
                }
            }
            rank += 1;
            entry_index += 1;
            field += 1;
        } while (entry_index < entry_count);
    }
    previous_field = ranks;
    previous_rank = fields;
    g_niki_rank_count = next_rank;
    next_rank = -1;
    entry_index = 0;
    maximum = 0;
    if (g_niki_entry_state > 0)
    {
        s32 max_count;
        max_count = g_niki_entry_state;
        slot = g_niki_card_slot;
        field_table = g_niki_entry_fields;
        candidate = &field_table[slot * NIKI_DIRECTORY_ENTRY_COUNT];
        do
        {
            if (next_rank < *candidate)
            {
                next_rank = *candidate;
                maximum = entry_index;
            }
            entry_index += 1;
            candidate += 1;
        } while (entry_index < max_count);
        entry_index = 0;
    }
    g_niki_entry_value_limit = next_rank + 1;
    if (g_niki_entry_state > 0)
    {
        suffix_output = &g_niki_entry_suffix_values[0];
        entry_cursor = (char*)g_niki_entries;
    loop_20:
        if (func_8001714C(&D_800ECFC4[0], (void*)((g_niki_card_slot * NIKI_CARD_DIRECTORY_BYTES) + (s32)entry_cursor), 8) == 0)
        {
            *suffix_output = max_suffix + 1;
        }
        else
        {
            suffix_output += 1;
            entry_cursor += NIKI_DIRECTORY_ENTRY_BYTES;
            entry_index += 1;
            if (entry_index < g_niki_entry_state)
            {
                goto loop_20;
            }
        }
    }
    return maximum;
}

void niki_reset_entry_ranks(void)
{
    s32 i;
    s32 val;

    g_niki_rank_count = 0x28;
    val = -1;
    for (i = 14; i >= 0; i--)
    {
        g_niki_entry_ranks[i] = val;
    }
}

/**
 * @brief Check whether the selected card contains either recognized save-file prefix.
 * @return One if a recognized entry exists, otherwise zero.
 */
s32 niki_has_known_entry_type(void)
{
    s32 entry_index;

    for (entry_index = 0; entry_index < g_niki_entry_state; entry_index++)
    {
        if (func_8001714C(D_800ECF7C, g_niki_entries[g_niki_card_slot][entry_index].name, 12) == 0 ||
            func_8001714C(D_800ECF8C, g_niki_entries[g_niki_card_slot][entry_index].name, 12) == 0)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Check whether directory entries occupy at least fourteen memory-card blocks.
 * @return One when the block limit is reached, otherwise zero.
 */
s32 niki_entry_blocks_reach_limit(void)
{
    s32 entry_index;
    s32 total_blocks;

    total_blocks = 0;
    for (entry_index = 0; entry_index < g_niki_entry_state; entry_index++)
    {
        total_blocks += g_niki_entries[g_niki_card_slot][entry_index].size / NIKI_MEMORY_CARD_BLOCK_BYTES;
    }
    return total_blocks >= 14;
}

/** @brief Remove both placeholder save files from the selected memory card. */
void niki_render_fixed_prompts(void)
{
    NikiPlaceholderPath buf;

    memcpy(&buf, &g_niki_file_template, 6);
    buf.device.characters.slot += *(u8*)&g_niki_card_slot;
    func_80016F9C(&buf, &D_800ECF9C);
    func_8001686C(&buf);

    memcpy(&buf, &g_niki_file_template, 6);
    buf.device.characters.slot += *(u8*)&g_niki_card_slot;
    func_80016F9C(&buf, &D_800ECFB0);
    func_8001686C(&buf);
}

/** @brief Remove placeholder save files before starting the card operation. */
static inline void niki_erase_placeholder_paths(void)
{
    NikiPlaceholderPath p;

    memcpy(&p, &g_niki_file_template, 6);
    p.device.characters.slot += *(u8*)&g_niki_card_slot;
    func_80016F9C(&p, &D_800ECF9C);
    func_8001686C(&p);

    memcpy(&p, &g_niki_file_template, 6);
    p.device.characters.slot += *(u8*)&g_niki_card_slot;
    func_80016F9C(&p, &D_800ECFB0);
    func_8001686C(&p);
}

/**
 * @brief Execute the current memory-card load/save command and advance its sequence.
 * @return Command result reported by the current sequence step.
 */
s32 niki_advance_load_sequence(void)
{
    NikiSequencePath path;
    s32 status0;
    s32 status1;
    s32 phase_result;
    s32 wait_attempts;
    s32 poll_result;
    s32 poll_result20;
    s32 rank_index;
    s32 rank_value;
    s32 dispatch;
    static void* const keep[] __attribute__((section(".discard"))) = {
        &&cl_case_0,  &&cl_case_1,  &&cl_case_2,  &&cl_case_3,    &&cl_case_4,    &&cl_case_5,    &&cl_case_6,    &&block_return,
        &&cl_case_8,  &&cl_case_9,  &&cl_case_10, &&block_return, &&block_return, &&block_return, &&block_return, &&cl_case_15,
        &&cl_case_16, &&cl_case_17, &&cl_case_18, &&cl_case_19,   &&cl_case_20,   &&block_return, &&block_return, &&block_return,
        &&cl_case_24, &&cl_case_25, &&cl_case_26, &&cl_case_27,   &&cl_case_28,   &&block_return, &&cl_case_30};

    memcpy(&path, &g_niki_file_template, 6);
    phase_result = 1;
    path.device.characters.slot += *(u8*)&g_niki_card_slot;

    if (g_niki_load_step == NULL)
    {
        return phase_result;
    }

    switch (0)
    {
    case 0:
        dispatch = *g_niki_load_step;
        if ((u32)dispatch >= 0x1F)
        {
            goto block_return;
        }
        goto* jtbl_80140098[dispatch];

    cl_case_1:
        phase_result = 3;
        func_8001729C(g_niki_card_slot);
        func_8001724C(g_niki_card_slot * 0x10);
        g_niki_load_step = g_niki_load_step + 1;
        goto block_return;

    cl_case_2:
        poll_result = niki_poll_primary_handle_group();
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
        return phase_result;
    c2_ge3:
        if (poll_result == 3)
        {
            goto c2_eq3;
        }
        return phase_result;
    c2_pos:
        phase_result = 4;
        g_niki_selection_status = 0;
        g_niki_entry_state = 0xFD;
        g_niki_load_step = g_niki_load_step + 1;
        goto block_return;
    c2_eq3:
        g_niki_rank_count = 0x28;
        rank_value = -1;
        for (rank_index = 14; rank_index >= 0; rank_index--)
        {
            g_niki_entry_ranks[rank_index] = rank_value;
        }
        goto block_status_ff;

    cl_case_3:
        niki_release_primary_handles();
        goto block_increment;

    cl_case_4:
        do
        {
            poll_result = niki_poll_secondary_handle_group();
        } while (poll_result == -1);
        if (poll_result == 0)
        {
            goto block_increment;
        }
        if (poll_result < 0)
        {
            return phase_result;
        }
        if (poll_result >= 4)
        {
            return phase_result;
        }
        phase_result = 4;
        goto block_status_fd;

    cl_case_5:
        niki_release_secondary_handles();
        goto block_increment;

    cl_case_6:
        niki_erase_placeholder_paths();
        g_niki_entry_scan_active = 1;
        if (niki_begin_entry_scan(g_niki_card_slot) == 0)
        {
            phase_result = 2;
            g_niki_load_step = NULL;
            g_niki_entry_state = 0xF8;
            g_niki_entry_scan_active = 0;
            goto block_return;
        }
        wait_attempts = 0;
        g_niki_load_step = g_niki_load_step + 1;
        do
        {
            if (niki_scan_next_entry(g_niki_card_slot) == 0)
            {
                if (g_niki_mode != 0)
                {
                    g_niki_selected_row = 0;
                }
                g_niki_entry_scan_active = 0;
                if (g_niki_entry_state == 0xF8)
                {
                    return phase_result;
                }
                if (g_niki_entry_state == 0xFA)
                {
                    goto block_return;
                }
                niki_commit_selected_entry();
                goto block_return;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        goto block_return;

    cl_case_8:
        phase_result = 3;
        func_8001729C(g_niki_card_slot);
        func_800172AC(g_niki_card_slot * 0x10);
        g_niki_load_step = g_niki_load_step + 1;
        goto block_return;

    cl_case_9:
        phase_result = 3;
        func_8001729C(g_niki_card_slot);
        func_8001725C(g_niki_card_slot * 0x10);
        g_niki_primary_poll_countdown = 0x10;
        g_niki_secondary_poll_countdown = 0x10;
        g_niki_load_step = g_niki_load_step + 1;
        goto block_return;

    cl_case_0:
        phase_result = 2;
        g_niki_progress_active = 0;
        goto block_return;

    cl_case_10:
        func_80016F9C(&path, (u8*)g_niki_entries + (g_niki_card_slot * NIKI_CARD_DIRECTORY_BYTES) + (g_niki_selected_row * NIKI_DIRECTORY_ENTRY_BYTES));
        wait_attempts = 0;
        func_8001729C(g_niki_card_slot);
        do
        {
            poll_result = func_8001686C(&path);
            wait_attempts = wait_attempts + 1;
            if (poll_result != 0)
            {
                break;
            }
        } while (wait_attempts < 0x14);
        goto block_increment;

    cl_case_15:
        poll_result = niki_poll_primary_handle_group();
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
        goto block_return;
    c15_ge3:
        if (poll_result == 3)
        {
            goto c15_eq3;
        }
        goto block_return;
    c15_pos:
        g_niki_secondary_poll_countdown = g_niki_secondary_poll_countdown - 1;
        if (g_niki_secondary_poll_countdown != 0)
        {
            goto block_reissue;
        }
        phase_result = 4;
    block_status_fd:
        g_niki_selection_status = 0;
        g_niki_entry_state = 0xFD;
        goto block_return;
    c15_eq3:
        g_niki_primary_poll_countdown = g_niki_primary_poll_countdown - 1;
        if (g_niki_primary_poll_countdown == 0)
        {
            goto c15_d70zero;
        }
    block_reissue:
        func_8001729C(g_niki_card_slot);
        func_800172AC(g_niki_card_slot * 0x10);
        func_8001729C(g_niki_card_slot);
        func_8001725C(g_niki_card_slot * 0x10);
        goto block_return;
    c15_d70zero:
        phase_result = 5;
        g_niki_entry_state = 0xFC;
        g_niki_load_step = D_801606D0;
        goto block_return;

    cl_case_16:
        do
        {
            poll_result = niki_poll_secondary_handle_group();
        } while (poll_result == -1);
        goto block_increment;

    cl_case_17:
        g_niki_io_busy = 1;
        g_niki_selection_status = 0;
        func_8001729C(g_niki_card_slot);
        g_niki_file_handle = func_8001680C(D_80164E70, 0x8001);
        if (g_niki_file_handle == -1)
        {
            goto block_return;
        }
        niki_release_primary_handles();
        func_8001729C(g_niki_card_slot);
        if (func_8001681C(g_niki_file_handle, &D_80164B98, g_niki_selected_entry_extended != 0 ? 0x280 : 0x80) == -1)
        {
            func_8001683C(g_niki_file_handle);
            goto block_return;
        }
        goto block_increment;

    cl_case_18:
        poll_result = niki_poll_primary_handle_group();
        if (poll_result == 0)
        {
            g_niki_io_busy = 0;
            g_niki_selection_status = 1;
            g_niki_load_step = g_niki_load_step + 1;
            func_8001683C(g_niki_file_handle);
            goto block_return;
        }
        if (poll_result == -1)
        {
            goto block_return;
        }
        g_niki_io_busy = 0;
        func_8001683C(g_niki_file_handle);
    block_status_ff:
        g_niki_entry_state = 0xFF;
        g_niki_load_step = D_801606C8;
        goto block_return;

    cl_case_19:
        g_niki_confirm_latch = 1;
        g_niki_progress_bar_active = 1;
        g_niki_progress_start_tick = func_8002054C(-1);
        func_8001729C(g_niki_card_slot);
        g_niki_file_handle = func_8001680C(D_80164E70, 0x8001);
        niki_release_primary_handles();
        func_8001729C(g_niki_card_slot);
        if (func_8001681C(g_niki_file_handle, g_niki_save_blob, 0x4000) == -1)
        {
            g_niki_retry_count = g_niki_retry_count - 1;
            if (g_niki_retry_count == 0)
            {
            block_dialog_read:
                niki_open_status_dialog(1);
                goto block_return;
            }
            goto block_return;
        }
        goto block_increment;

    cl_case_20:
        poll_result20 = niki_poll_primary_handle_group();
        if (poll_result20 == 0)
        {
            g_niki_confirm_latch = 0;
            g_niki_load_step = g_niki_load_step + 1;
            func_8001683C(g_niki_file_handle);
            goto block_return;
        }
        if (poll_result20 < 0)
        {
            goto block_return;
        }
        if (poll_result20 >= 4)
        {
            goto block_return;
        }
        g_niki_retry_count = g_niki_retry_count - 1;
        if (g_niki_retry_count == 0)
        {
            g_niki_progress_bar_active = 0;
            goto block_dialog_read;
        }
        goto block_decrement_step;

    cl_case_24:
        wait_attempts = 0;
        do
        {
            if (func_800342CC(g_niki_card_slot * 0x10) == 1)
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
                goto block_increment;
            }
        }
        niki_open_status_dialog(3);
        goto block_return;

    cl_case_27:
        g_niki_confirm_latch = 1;
        g_niki_progress_bar_active = 1;
        g_niki_progress_start_tick = func_8002054C(-1);
        func_8001729C(g_niki_card_slot);
        g_niki_file_handle = func_8001680C(D_80164E70, 0x8001);
        niki_release_primary_handles();
        func_8001729C(g_niki_card_slot);
        if (func_8001681C(g_niki_file_handle, g_niki_save_blob, 0x4000) == -1)
        {
            func_8001683C(g_niki_file_handle);
            g_niki_retry_count = g_niki_retry_count - 1;
            if (g_niki_retry_count == 0)
            {
            block_dialog_write_read:
                niki_open_secondary_status_dialog(1);
                goto block_return;
            }
            goto block_return;
        }
        goto block_increment;

    cl_case_28:
        poll_result20 = niki_poll_primary_handle_group();
        if (poll_result20 == 0)
        {
            g_niki_confirm_latch = 0;
            g_niki_load_step = g_niki_load_step + 1;
            func_8001683C(g_niki_file_handle);
            goto block_return;
        }
        if (poll_result20 < 0)
        {
            goto block_return;
        }
        if (poll_result20 >= 4)
        {
            goto block_return;
        }
        g_niki_retry_count = g_niki_retry_count - 1;
        if (g_niki_retry_count == 0)
        {
            func_8001683C(g_niki_file_handle);
            g_niki_progress_bar_active = 0;
            niki_open_secondary_status_dialog(1);
            return phase_result;
        }
        goto block_close_decrement;

    cl_case_30:
        g_niki_retry_count = 5;
        g_niki_load_step = g_niki_load_step + 1;
        goto block_return;

    cl_case_25:
        if (D_80164FD4 == 0)
        {
            wait_attempts = 0;
            do
            {
                if (func_8001686C(D_80164E70) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 0x14);
        }
        func_80016F9C(&path, D_800ECF9C);
        func_8001729C(g_niki_card_slot);
        g_niki_file_handle = func_8001680C(&path, 0x20200);
        if (g_niki_file_handle != -1)
        {
            goto block_write_opened;
        }
        func_8001683C(-1);
        wait_attempts = 0;
        do
        {
            if (func_8001686C(&path) != 0)
            {
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
    block_write_retry:
        g_niki_retry_count = g_niki_retry_count - 1;
        if (g_niki_retry_count == 0)
        {
        block_dialog_write:
            niki_open_secondary_status_dialog(0);
            goto block_return;
        }
        goto block_return;

    block_write_opened:
        func_8001683C(g_niki_file_handle);
        func_800170BC(D_80164FD8, &path);
        func_8001729C(g_niki_card_slot);
        g_niki_file_handle = func_8001680C(D_80164FD8, 0x8002);
        niki_release_primary_handles();
        g_niki_progress_bar_active = 1;
        g_niki_progress_start_tick = func_8002054C(-1);
        func_8001729C(g_niki_card_slot);
        if (func_8001682C(g_niki_file_handle, g_niki_save_blob, 0x4000) == -1)
        {
            func_8001683C(g_niki_file_handle);
            wait_attempts = 0;
            do
            {
                if (func_8001686C(D_80164FD8) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 0x14);
            goto block_write_retry;
        }
        goto block_increment;

    block_increment:
        g_niki_load_step = g_niki_load_step + 1;
        goto block_return;

    cl_case_26:
        poll_result20 = niki_poll_primary_handle_group();
        if (poll_result20 != 0)
        {
            if (poll_result20 < 0)
            {
                goto block_return;
            }
            if (poll_result20 >= 4)
            {
                goto block_return;
            }
            goto block_case26_retry;
        }
        if (D_80164FD4 != 0)
        {
            func_8001729C(g_niki_card_slot);
            wait_attempts = 0;
            do
            {
                if (func_8001686C(D_80164E70) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 0x14);
        }
        func_8001729C(g_niki_card_slot);
        wait_attempts = 0;
        do
        {
            if (func_8001685C(D_80164FD8, D_80164E70) != 0)
            {
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        g_niki_progress_active = 0;
        g_niki_load_step = g_niki_load_step + 1;
        func_8001683C(g_niki_file_handle);
        goto block_return;
    }

block_case26_retry:
    g_niki_retry_count = g_niki_retry_count - 1;
    if (g_niki_retry_count == 0)
    {
        goto block_case26_exhausted;
    }

block_close_decrement:
    func_8001683C(g_niki_file_handle);
block_decrement_step:
    g_niki_load_step = g_niki_load_step - 1;
    goto block_return;

block_case26_exhausted:
    g_niki_progress_bar_active = 0;
    niki_open_secondary_status_dialog(0);
    wait_attempts = 0;
    do
    {
        if (func_8001686C(D_80164FD8) != 0)
        {
            break;
        }
        wait_attempts = wait_attempts + 1;
    } while (wait_attempts < 0x14);

block_return:
    return phase_result;
}

/** @see decomp.me (100.00%) */
void niki_restart_load_sequence(void)
{
    func_8001729C(g_niki_card_slot);
    niki_release_primary_handles();
    func_8001724C(g_niki_card_slot * 0x10);
    g_niki_load_step = D_801606D0;
}

/** @see decomp.me (100.00%) */
s32 niki_poll_and_rewind_primary_handles(void)
{
    s32 busy_slot;

    busy_slot = niki_poll_primary_handle_group();
    if (busy_slot != -1)
    {
        func_8001729C(g_niki_card_slot);
        func_8001724C(g_niki_card_slot * 0x10);
    }
    return busy_slot;
}

/** @see decomp.me (100.00%) */
void niki_init_stream_handles(void)
{
    func_800158E0();
    func_800167EC();
    g_niki_primary_handle0 = func_800167AC(0xF4000001, 4, 0x2000, 0);
    g_niki_primary_handle1 = func_800167AC(0xF4000001, 0x8000, 0x2000, 0);
    g_niki_primary_handle2 = func_800167AC(0xF4000001, 0x100, 0x2000, 0);
    g_niki_primary_handle3 = func_800167AC(0xF4000001, 0x2000, 0x2000, 0);
    g_niki_secondary_handle0 = func_800167AC(0xF0000011, 4, 0x2000, 0);
    g_niki_secondary_handle1 = func_800167AC(0xF0000011, 0x8000, 0x2000, 0);
    g_niki_secondary_handle2 = func_800167AC(0xF0000011, 0x100, 0x2000, 0);
    g_niki_secondary_handle3 = func_800167AC(0xF0000011, 0x2000, 0x2000, 0);
    func_800167DC(g_niki_primary_handle0);
    func_800167DC(g_niki_primary_handle1);
    func_800167DC(g_niki_primary_handle2);
    func_800167DC(g_niki_primary_handle3);
    func_800167DC(g_niki_secondary_handle0);
    func_800167DC(g_niki_secondary_handle1);
    func_800167DC(g_niki_secondary_handle2);
    func_800167DC(g_niki_secondary_handle3);
    func_800167FC();
    g_niki_progress_bar_active = 0;
    g_niki_entry_scan_active = 0;
}

/** @see decomp.me (100.00%) */
void niki_shutdown_stream_handles(void)
{
    func_800158E0();
    func_800167EC();
    func_800167BC(g_niki_primary_handle0);
    func_800167BC(g_niki_primary_handle1);
    func_800167BC(g_niki_primary_handle2);
    func_800167BC(g_niki_primary_handle3);
    func_800167BC(g_niki_secondary_handle0);
    func_800167BC(g_niki_secondary_handle1);
    func_800167BC(g_niki_secondary_handle2);
    func_800167BC(g_niki_secondary_handle3);
    func_800167FC();
}

/**
 * @brief Reset the browser and read the selected card's first directory entry.
 * @param card_slot Memory-card slot to scan.
 * @return One if the first entry was read, otherwise zero.
 * @see decomp.me (100.00%)
 */
s32 niki_begin_entry_scan(s32 card_slot)
{
    NikiDirectoryPattern pattern;

    memcpy(&pattern, &g_niki_entry_header_template, 7);
    g_niki_selected_row = 0;
    g_niki_scroll_frames = 0;
    g_niki_scroll_target_y = 0;
    g_niki_scroll_y = 0;
    g_niki_entry_state = 0;
    pattern.device.characters.slot += card_slot;
    if (func_80016BCC(&pattern, g_niki_entries[card_slot]) != 0)
    {
        func_800B0170(&g_niki_entries[card_slot][g_niki_entry_state]);
        g_niki_entry_state += 1;
        return 1;
    }
    return 0;
}

/**
 * @brief Read the next directory entry, or rank the completed directory.
 * @param page Memory-card slot being scanned.
 * @return One if another directory entry was read, otherwise zero.
 * @see decomp.me (100.00%)
 */
s32 niki_scan_next_entry(s32 page)
{
    s32 entry_index;
    s32 used_blocks;
    s32 selected;
    s32 entry_count;
    s32 card_full;

    if (func_8001684C(&g_niki_entries[page][g_niki_entry_state]) != 0)
    {
        func_800B0170(&g_niki_entries[page][g_niki_entry_state]);
        g_niki_entry_state += 1;
        return 1;
    }

    func_800AA02C();
    if ((g_niki_mode == 0) && (niki_has_known_entry_type() == 0))
    {
        g_niki_entry_state = 0xF8;
    }
    else
    {
        entry_index = 0;
        used_blocks = 0;
        D_80164FD4 = 0;
        entry_count = g_niki_entry_state;
        if (entry_count > 0)
        {
            u8* entries;
            s32 offset;
            do
            {
                entries = (u8*)g_niki_entries;
            } while (0);
            offset = g_niki_card_slot * NIKI_CARD_DIRECTORY_BYTES;
            do
            {
                used_blocks += ((NikiDirEntry*)(offset + (s32)entries))->size / NIKI_MEMORY_CARD_BLOCK_BYTES;
                entry_index++;
                offset += NIKI_DIRECTORY_ENTRY_BYTES;
            } while (entry_index < entry_count);
        }
        card_full = used_blocks >= 0xE;
        if (card_full != 0)
        {
            selected = niki_rank_entries(used_blocks, entry_index, entry_count);
            if (niki_has_known_entry_type() == 0)
            {
                g_niki_entry_state = 0xFA;
                g_niki_entry_value_limit = 0;
            }
            else
            {
                g_niki_selected_row = selected;
                niki_scroll_to_selection();
            }
        }
        else
        {
            D_80164FD4 = 1;
            selected = niki_rank_entries(used_blocks, entry_index, entry_count);
            if (niki_has_known_entry_type() == 0)
            {
                g_niki_selected_row = 0;
                niki_scroll_to_selection();
                g_niki_entry_value_limit = 0;
            }
            else
            {
                g_niki_selected_row = selected;
                niki_scroll_to_selection();
            }
        }
    }
    return 0;
}

/**
 * @brief Prepare the selected save-file path and begin its load sequence.
 * @see decomp.me (100.00%)
 */
void niki_commit_selected_entry(void)
{
    NikiSelectedFilePath path;
    u8* path_bytes;

    if (g_niki_entry_state == 0)
    {
        g_niki_selection_status = 3;
        return;
    }
    {
        if (func_8001714C(&D_800ECFC4[0], g_niki_entries[g_niki_card_slot][g_niki_selected_row].name, 8) == 0)
        {
            g_niki_selection_status = 2;
            return;
        }
    }
    memcpy(&path, &g_niki_file_template, 6);
    path_bytes = (u8*)&path;
    {
        func_80016F9C(path_bytes, g_niki_entries[g_niki_card_slot][g_niki_selected_row].name);
    }
    {
        s32 slot;
        s32 value;
        value = path.device.characters.slot;
        slot = (u8)g_niki_card_slot;
        g_niki_selection_status = 0;
        value += slot;
        path.device.characters.slot = value;
        func_800170BC(&D_80164E70[0], path_bytes, slot);
    }
    g_niki_load_step = &D_801606E0[0];
    {
        if (func_8001714C(&D_800ECF7C[0], g_niki_entries[g_niki_card_slot][g_niki_selected_row].name, 0xC) == 0)
        {
            g_niki_selected_entry_extended = 1;
        }
        else
        {
            g_niki_selected_entry_extended = 0;
        }
    }
    g_niki_io_busy = 1;
}

/** @see decomp.me (100.00%) */
void niki_release_primary_handles(void)
{
    func_800167CC(g_niki_primary_handle0);
    func_800167CC(g_niki_primary_handle1);
    func_800167CC(g_niki_primary_handle2);
    func_800167CC(g_niki_primary_handle3);
}

/** @see decomp.me (100.00%) */
void niki_release_secondary_handles(void)
{
    func_800167CC(g_niki_secondary_handle0);
    func_800167CC(g_niki_secondary_handle1);
    func_800167CC(g_niki_secondary_handle2);
    func_800167CC(g_niki_secondary_handle3);
}

/** @see decomp.me (100.00%) */
s32 niki_poll_primary_handle_group(void)
{
    if (func_800167CC(g_niki_primary_handle0) == 1)
    {
        return 0;
    }
    if (func_800167CC(g_niki_primary_handle1) == 1)
    {
        return 1;
    }
    if (func_800167CC(g_niki_primary_handle2) == 1)
    {
        return 2;
    }
    if (func_800167CC(g_niki_primary_handle3) == 1)
    {
        return 3;
    }
    return -1;
}

/** @see decomp.me (100.00%) */
s32 niki_poll_secondary_handle_group(void)
{
    if (func_800167CC(g_niki_secondary_handle0) == 1)
    {
        return 0;
    }
    if (func_800167CC(g_niki_secondary_handle1) == 1)
    {
        return 1;
    }
    if (func_800167CC(g_niki_secondary_handle2) == 1)
    {
        return 2;
    }
    if (func_800167CC(g_niki_secondary_handle3) == 1)
    {
        return 3;
    }
    return -1;
}

/**
 * @brief Group recognized save-file types by suffix, then append other entries.
 * @note Preserves directory order within each type and suffix group.
 */
void niki_sort_entries_by_type(void)
{
    NikiDirEntry sorted[NIKI_DIRECTORY_ENTRY_COUNT];
    s32 output_index = 0;
    s32 suffix;
    s32 entry_index;

    for (suffix = 0; suffix < 8; suffix++)
    {
        for (entry_index = 0; entry_index < g_niki_entry_state; entry_index++)
        {
            if (g_niki_entry_suffix_values[entry_index] == suffix && func_8001714C(D_800ECF7C, g_niki_entries[g_niki_card_slot][entry_index].name, 12) == 0)
            {
                func_80016E7C(&g_niki_entries[g_niki_card_slot][entry_index], &sorted[output_index], sizeof(NikiDirEntry));
                output_index++;
            }
        }
    }

    for (suffix = 0; suffix < 8; suffix++)
    {
        for (entry_index = 0; entry_index < g_niki_entry_state; entry_index++)
        {
            if (g_niki_entry_suffix_values[entry_index] == suffix && func_8001714C(D_800ECF8C, g_niki_entries[g_niki_card_slot][entry_index].name, 12) == 0)
            {
                func_80016E7C(&g_niki_entries[g_niki_card_slot][entry_index], &sorted[output_index], sizeof(NikiDirEntry));
                output_index++;
            }
        }
    }

    for (entry_index = 0; entry_index < g_niki_entry_state; entry_index++)
    {
        if (func_8001714C(D_800ECFC4, g_niki_entries[g_niki_card_slot][entry_index].name, 8) == 0)
        {
            func_80016E7C(&g_niki_entries[g_niki_card_slot][entry_index], &sorted[output_index], sizeof(NikiDirEntry));
            output_index++;
        }
    }

    for (entry_index = 0; entry_index < g_niki_entry_state; entry_index++)
    {
        if (func_8001714C(D_800ECF7C, g_niki_entries[g_niki_card_slot][entry_index].name, 12) != 0 &&
            func_8001714C(D_800ECF8C, g_niki_entries[g_niki_card_slot][entry_index].name, 12) != 0 &&
            func_8001714C(D_800ECFC4, g_niki_entries[g_niki_card_slot][entry_index].name, 8) != 0)
        {
            func_80016E7C(&g_niki_entries[g_niki_card_slot][entry_index], &sorted[output_index], sizeof(NikiDirEntry));
            output_index++;
        }
    }

    for (entry_index = 0; entry_index < g_niki_entry_state; entry_index++)
    {
        func_80016E7C(&sorted[entry_index], &g_niki_entries[g_niki_card_slot][entry_index], sizeof(NikiDirEntry));
    }
}

/**
 * @brief Draw a signed decimal value, suppressing leading zeroes.
 * @param prim GPU packet write cursor.
 * @param ot Ordering table receiving the text packets.
 * @param value Number to display, with magnitude at most 99999.
 * @param x Horizontal text anchor.
 * @param y Vertical text position.
 * @param palette Glyph palette index.
 * @param alignment Text alignment relative to the anchor.
 * @return GPU packet cursor after the text packets.
 */
s32 niki_draw_signed_decimal(s32 prim, s32* ot, s32 value, s32 x, s32 y, s32 palette, s32 alignment)
{
    u16 glyphs[7];
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
    glyphs[1] = g_niki_decimal_glyphs[magnitude / 10000];
    glyphs[2] = g_niki_decimal_glyphs[(magnitude % 10000) / 1000];
    glyphs[3] = g_niki_decimal_glyphs[(magnitude % 1000) / 100];
    glyphs[4] = g_niki_decimal_glyphs[(magnitude % 100) / 10];
    glyphs[5] = g_niki_decimal_glyphs[magnitude % 10];

    first_digit = 1;
    glyphs[6] = 0;

    while (first_digit < 5 && glyphs[first_digit] == NIKI_SJIS_FULLWIDTH_ZERO)
    {
        first_digit++;
    }

    if (negative != 0)
    {
        first_digit--;
        glyphs[first_digit] = NIKI_SJIS_MINUS;
    }
    prim = niki_draw_cached_text(prim, ot, (u8*)&glyphs[first_digit], x, y, palette, alignment);
    return prim;
}

/**
 * @brief Draw a byte as two hexadecimal glyphs.
 * @param prim GPU packet write cursor.
 * @param ot Ordering table receiving the text packets.
 * @param value Byte value to display, from 0 through 255.
 * @param x Horizontal text anchor.
 * @param y Vertical text position.
 * @param alignment Text alignment relative to the anchor.
 */
void niki_draw_hex_byte(s32 prim, s32* ot, s32 value, s32 x, s32 y, s32 alignment)
{
    u16 glyphs[3];
    s32 high_digit;
    s32 low_digit;
    u16* high_glyph;

    high_digit = value / 16;
    high_glyph = &g_niki_hex_glyphs[high_digit];
    low_digit = value % 16;
    glyphs[0] = *high_glyph;
    glyphs[1] = g_niki_hex_glyphs[low_digit];
    glyphs[2] = 0;
    niki_draw_cached_text(prim, ot, (u8*)glyphs, x, y, 0, alignment);
}

s32 niki_draw_cached_text(s32 prim, s32* ot, u8* text, s32 x, s32 y, s32 palette, s32 alignment)
{
    u8* cursor;
    s32 count;
    u16 code;
    u8* scan;

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
    g_niki_text_line_start_x = x;
    g_niki_glyph_cursor_x = x;
    g_niki_glyph_cursor_y = y;

    while (1)
    {
        u32 lead = *cursor;

        if ((u8)lead == 0x20)
        {
            cursor++;
            g_niki_glyph_cursor_x += 0x10;
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
        prim = niki_render_cached_glyph(prim, ot, code, palette);
    }

    NIKI_SET_PACKET_LENGTH(prim, 1);
    ((NikiGpuPacketPrefix*)prim)->word4 = 0xE1000005;
    NIKI_ADD_PRIMITIVE(ot, prim);
    return prim + 8;
}

s32 niki_render_cached_glyph(s32 prim, s32* ot, s32 character_code, s32 palette)
{
    NikiGlyphCacheEntry* entry;
    u8* font_data;
    s32 font_address;
    u32 requested_code;
    s32 slot;
    s32 high_pixel_set;
    s32 code;
    RECT rect;

    u8* raster;
    s32 color_index;
    s32 high_nibble_color;
    s32 row;
    s32 source_byte;

    u16 mask;
    volatile u8* raster_byte;
    u8 packed_pixels;

    code = character_code;
    slot = 0;
    requested_code = code & 0xFFFF;
    entry = g_niki_glyph_cache;

    while (slot < GLYPH_CACHE_SLOTS)
    {
        if (requested_code == entry->data.code)
        {
            return niki_emit_glyph_sprite((NikiGlyphSprite*)prim, ot, slot, palette);
        }
        slot++;
        entry++;
    }

    font_address = func_8001687C(code & 0xFFFF);
    font_data = (u8*)font_address;
    if (font_address == -1)
    {
        return prim;
    }

    raster = g_niki_glyph_raster_cursor;
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
    while ((slot < GLYPH_CACHE_SLOTS) && (g_niki_glyph_cache[slot].raw != 0))
    {
        slot++;
    }

    if (slot == GLYPH_CACHE_SLOTS)
    {
        return prim;
    }
    g_niki_glyph_cache[slot].raw = code & 0xFFFF;
    prim = niki_emit_glyph_sprite((NikiGlyphSprite*)prim, ot, slot, palette);

    g_niki_glyph_upload_x = (slot % GLYPH_CACHE_COLUMNS) * 4;
    g_niki_glyph_upload_y = slot & GLYPH_CACHE_ROW_MASK;

    rect.w = 4;
    rect.h = 15;
    rect.x = g_niki_glyph_upload_x + 0x140;
    rect.y = g_niki_glyph_upload_y;

    func_80019A34(&rect, g_niki_glyph_raster_cursor);
    func_80019788(0);

    g_niki_glyph_raster_cursor += GLYPH_RASTER_BYTES;
    return prim;
}

s32 niki_emit_glyph_sprite(NikiGlyphSprite* sprite, s32* ot, s32 cache_slot, s32 palette)
{
    u32 ot_tag_high_byte;
    s32 normalized_slot;
    u32 packet_address;
    s32 old_x;
    s32 new_x;
    s32 fits_line;

    g_niki_glyph_cache[cache_slot].raw |= 0x10000;

    NIKI_SET_PACKET_LENGTH(sprite, 3);
    NIKI_SET_PACKET_CODE(sprite, 0x7C);
    sprite->packet.g0 = 0x80;
    sprite->packet.b0 = 0x80;
    sprite->packet.r0 = 0x80;
    normalized_slot = cache_slot;
    sprite->packet.x0 = g_niki_glyph_cursor_x;
    sprite->packet.y0 = g_niki_glyph_cursor_y;

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
    old_x = g_niki_glyph_cursor_x;
    new_x = old_x + 16;
    fits_line = (old_x + 32) < 0x280;
    g_niki_glyph_cursor_x = new_x;

    *ot = ot_tag_high_byte | packet_address;

    if (!fits_line)
    {
        g_niki_glyph_cursor_x = g_niki_text_line_start_x;
        g_niki_glyph_cursor_y += 16;
    }

    return (s32)sprite;
}

/**
 * @brief Start a frame with all cached glyphs marked unused and reset raster allocation.
 * @see decomp.me (100.00%)
 */
void niki_begin_glyph_cache_frame(void)
{
    s32 slot;
    NikiGlyphCacheEntry* entry;

    g_niki_glyph_raster_cursor = g_niki_glyph_raster_buffer;
    slot = 0;
    entry = g_niki_glyph_cache;
    for (; slot < GLYPH_CACHE_SLOTS; slot++, entry++)
    {
        entry->raw = (u16)entry->raw;
    }
}

/**
 * @brief Free cache slots whose glyphs were not drawn this frame.
 * @see decomp.me (100.00%)
 */
void niki_evict_unused_glyphs(void)
{
    s32 slot;
    NikiGlyphCacheEntry* entry;
    s32 used_flag;

    slot = 0;
    used_flag = 0x10000;
    entry = g_niki_glyph_cache;
    for (; slot < GLYPH_CACHE_SLOTS; slot++, entry++)
    {
        if (!(entry->raw & used_flag))
        {
            entry->raw = 0;
        }
    }
}

/**
 * @brief Clear cached character codes and the glyph raster buffer.
 * @see decomp.me (100.00%)
 */
void niki_reset_glyph_cache(void)
{
    s32 slot;
    NikiGlyphCacheEntry* entry;
    u8* raster_buffer;

    slot = GLYPH_CACHE_SLOTS - 1;
    entry = g_niki_glyph_cache;
    entry += GLYPH_CACHE_SLOTS - 1;
    for (; slot >= 0; slot--, entry--)
    {
        entry->raw = 0;
    }

    slot = 0;
    raster_buffer = g_niki_glyph_raster_buffer;
    for (; slot < GLYPH_CACHE_SLOTS * GLYPH_RASTER_BYTES; slot++)
    {
        *(u8*)(slot + (s32)raster_buffer) = 0;
    }
}

/*
 * The extended table linker symbol is biased backwards by 0x19 pages.  This
 * lets the original code index it directly with the encoded lead byte
 * (0x19..0x1F) instead of subtracting NIKI_TEXT_EXTENDED_LEAD_FIRST first.
 */

/**
 * @brief Expand NIKI's internal text encoding into a NUL-terminated Shift-JIS
 *        byte string.
 * @param dst_sjis Destination buffer. Each decoded source character writes one
 *        two-byte Shift-JIS code; the function appends a single NUL byte.
 * @param src_text NUL-terminated NIKI text. Bytes 0x19..0x1F introduce a
 *        two-byte table code; printable one-byte codes use the compact table.
 * @note Each lookup row contains 16 two-byte Shift-JIS codes followed by a
 *       newline byte, so sizeof(NikiSjisRow) is 33 and sizeof(NikiSjisPage) is
 *       528.
 * @see decomp.me (100.00%)
 */
void niki_expand_text_glyph_codes(u8* dst_sjis, const u8* src_text)
{
    u32 source_byte;
    s32 glyph_index;
    s16 lead_byte;

    for (;;)
    {
        source_byte = *src_text;
        if ((u8)source_byte != 0)
        {
            if ((u32)(source_byte - NIKI_TEXT_EXTENDED_LEAD_FIRST) < NIKI_TEXT_EXTENDED_PAGE_COUNT)
            {
                u32 trail_byte;
                s32 row;
                u8* sjis_lead;
                u8* sjis_trail;

                trail_byte = src_text[1];
                row = trail_byte >> NIKI_SJIS_ROW_SHIFT;
                trail_byte &= NIKI_SJIS_CODES_PER_ROW - 1;
                sjis_lead = (u8*)g_niki_double_byte_char_table + trail_byte * sizeof(NikiSjisCode);
                sjis_lead += row * sizeof(NikiSjisRow);
                lead_byte = *src_text;
                sjis_lead += lead_byte * sizeof(NikiSjisPage);
                *dst_sjis = *sjis_lead;
                dst_sjis++;

                trail_byte = src_text[1];
                row = trail_byte >> NIKI_SJIS_ROW_SHIFT;
                trail_byte &= NIKI_SJIS_CODES_PER_ROW - 1;
                sjis_trail = (u8*)g_niki_double_byte_char_table + 1 + trail_byte * sizeof(NikiSjisCode);
                sjis_trail += row * sizeof(NikiSjisRow);
                lead_byte = *src_text;
                sjis_trail += lead_byte * sizeof(NikiSjisPage);
                *dst_sjis = *sjis_trail;
                dst_sjis++;
                src_text += 2;
            }
            else if ((u8)source_byte >= NIKI_TEXT_PRINTABLE_FIRST)
            {
                lead_byte = *src_text;
                glyph_index = lead_byte - NIKI_TEXT_SINGLE_BYTE_BASE;
                *dst_sjis = ((u8*)g_niki_single_byte_char_table)[(glyph_index / NIKI_SJIS_CODES_PER_ROW) * sizeof(NikiSjisRow) +
                                                                 (glyph_index & (NIKI_SJIS_CODES_PER_ROW - 1)) * sizeof(NikiSjisCode)];
                dst_sjis++;

                lead_byte = *src_text;
                glyph_index = lead_byte - NIKI_TEXT_SINGLE_BYTE_BASE;
                *dst_sjis = ((u8*)g_niki_single_byte_char_table)[(glyph_index / NIKI_SJIS_CODES_PER_ROW) * sizeof(NikiSjisRow) +
                                                                 (glyph_index & (NIKI_SJIS_CODES_PER_ROW - 1)) * sizeof(NikiSjisCode) + 1];
                dst_sjis++;
                src_text += 1;
            }
            else
            {
                *dst_sjis = g_niki_single_byte_char_table[0].codes[0].lead;
                dst_sjis++;
                *dst_sjis = g_niki_single_byte_char_table[0].codes[0].trail;
                dst_sjis++;
                src_text += 1;
            }
        }
        else
        {
            *dst_sjis = 0;
            return;
        }
    }
}
