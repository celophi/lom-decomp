#include "common.h"
#include "gpu_packet.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"
#include "tim.h"

typedef struct GosubTilePacket GosubTilePacket;

/* Overlay constants. */

/** @brief Number of entries in the gosub UI element pool. */
#define GOSUB_ELEMENT_COUNT 16

/** @brief Control entries embedded in a gosub screen sequence. */
#define GOSUB_SCREEN_SEQUENCE_END 0xFE
#define GOSUB_SCREEN_SEQUENCE_DIALOG 0xFF

/** @brief Fields in the encoded sort mode passed to gosub_sort_rows. */
#define GOSUB_SORT_KEY_MASK 0x0F
#define GOSUB_SORT_ASCENDING_MASK 0xF0
#define GOSUB_SORT_ASCENDING_SHIFT 7
#define GOSUB_SORT_ORDER_CAPACITY 0x100
#define GOSUB_SORT_ROW_CAPACITY 0x28

/** @brief Packed logic-block fields in g_pad_ctx. */
#define GOSUB_LOGIC_BLOCK_COUNT_OFFSET 0x29D6
#define GOSUB_LOGIC_BLOCK_RECORDS_OFFSET 0x29DC

/** @brief Texture page containing the gosub font and panel-corner sprites. */
#define GOSUB_FONT_TPAGE 5

/** @brief VRAM row containing the selectable glyph palettes. */
#define GOSUB_GLYPH_CLUT_Y 0x1F2
/** @brief Convert a glyph CLUT slot to its VRAM x coordinate. */
#define GOSUB_GLYPH_CLUT_X_SHIFT 4

/** @brief VRAM location of the gosub font CLUT. */
#define GOSUB_FONT_CLUT_X 0x150
#define GOSUB_FONT_CLUT_Y 0xFF
#define GOSUB_FONT_CLUT_WIDTH 0x10
#define GOSUB_FONT_CLUT_HEIGHT 1

/** @brief VRAM rectangle occupied by the gosub font texture strip. */
#define GOSUB_FONT_TEXTURE_X 0x140
#define GOSUB_FONT_TEXTURE_Y 0xF0
#define GOSUB_FONT_TEXTURE_WIDTH 0x10
#define GOSUB_FONT_TEXTURE_HEIGHT 0x10
#define GOSUB_FONT_TEXTURE_DATA_OFFSET 0x2C

/** @brief Dimensions and placement of one panel-corner sprite quadrant. */
#define GOSUB_PANEL_CORNER_SIZE 8
#define GOSUB_PANEL_CORNER_OUTSET 2
#define GOSUB_PANEL_CORNER_FAR_INSET 5
#define GOSUB_PANEL_CORNER_TEXTURE_V 0xF0

/** @brief Layout constants for a composite icon assembled from glyphs. */
#define GOSUB_COMPOSITE_ICON_PART_CAPACITY 19
#define GOSUB_COMPOSITE_ICON_BASE_GLYPH_OFFSET 0x13
#define GOSUB_COMPOSITE_ICON_BASE_CELL_SIZE 8
#define GOSUB_COMPOSITE_ICON_PART_CELL_SIZE 16
#define GOSUB_COMPOSITE_ICON_BASE_CLUT 9

/** @brief TIM flag indicating that a CLUT block precedes the pixel block. */
#define GOSUB_TIM_HAS_CLUT 0x8

/** @brief Lifecycle states used by a gosub UI element. */
typedef enum GosubElementState
{
    GOSUB_ELEMENT_STATE_INACTIVE = 0,
    GOSUB_ELEMENT_STATE_ENTERING = 1,
    GOSUB_ELEMENT_STATE_ACTIVE = 2,
    GOSUB_ELEMENT_STATE_EXITING = 3
} GosubElementState;

/**
 * @brief Animated panel or list element managed by the gosub renderer.
 */
typedef struct
{
    union
    {
        u32 word;
        struct
        {
            u32 state : 3;
            u32 transition_step : 4;
            u32 x : 9;
            u32 width_low : 8;
        } f;
    } attr;
    u32 width_high : 1;
    u32 y : 8;
    u32 reserved_9 : 23;
    void* draw_handler;
} GosubElement;

/**
 * @brief Display and selection metadata for one gosub list row.
 */
typedef struct
{
    u8* name;
    u8* desc;
    s16 value;
    s16 index;
    s32 detail_id : 8;
    s32 detail_variant : 8;
    s32 detail_group : 8;
    u32 text_color : 4;
    u32 equipment_kind : 4;
    u16 primary_value;
    u16 stats[4];
    u16 secondary_value;
    union
    {
        struct
        {
            u32 selection_blocked : 1;
            u32 selection_restricted : 1;
            u32 alternate_format : 1;
            u32 reserved_3 : 29;
        } f;
        u16 half;
        u32 word;
    } flags;
} GosubListRow;

/** @brief Sort keys offered for the packed logic-block list. */
typedef enum
{
    GOSUB_SORT_BY_TYPE = 0,
    GOSUB_SORT_BY_POWER = 1,
    GOSUB_SORT_BY_SHAPE = 2,
    GOSUB_SORT_KEY_COUNT = 3
} GosubSortKey;

/** @brief Screen-space position pair passed by address to the glyph writer. */
typedef struct
{
    s16 x;
    s16 y;
} GosubTextPosition;

/** @brief GPU tile-shaped packet with a volatile height field. */
typedef struct
{
    s32 tag;
    s32 color;
    s16 x;
    s16 y;
    s16 w;
    volatile u16 h;
} GosubGpuPacket;

/** @brief GPU tile packet used for cursor and selection highlights. */
struct GosubTilePacket
{
    s32 tag;
    s32 color;
    s16 x;
    s16 y;
    s16 w;
    u16 h;
};

/** @brief Render context fields used by the gosub element renderer. */
typedef struct
{
    s32 tag;
    u8 reserved_0004[0x40AE];
    s16 display_buffer_index;
    u8 reserved_40b4[4];
    GosubGpuPacket* packet_cursor;
} GosubRenderContext;

/** @brief Draw callback installed on a gosub element. */
typedef GosubGpuPacket* (*GosubElementDrawHandler)();

/** @brief Unconnected flat-line GPU packet. */
typedef struct
{
    u_long tag;  /* 0x00 P_TAG */
    u_char r0;   /* 0x04 */
    u_char g0;   /* 0x05 */
    u_char b0;   /* 0x06 */
    u_char code; /* 0x07 */
    s16 x0;      /* 0x08 */
    s16 y0;      /* 0x0A */
    s16 x1;      /* 0x0C */
    s16 y1;      /* 0x0E */
} GosubLinePacket; /* 0x10 */

/** @brief Packed four-byte record stored in the combination table. */
typedef struct
{
    u32 word;
} GosubPackedRecord;

/** @brief Stack workspace used to reorder logic-block records and their rows. */
typedef struct
{
    u8 row_order[GOSUB_SORT_ORDER_CAPACITY];
    GosubPackedRecord packed_records[GOSUB_SORT_ROW_CAPACITY];
    GosubListRow rows[GOSUB_SORT_ROW_CAPACITY];
} GosubSortWorkspace; /* 0x6A0 */

/** @brief One 0x40-byte equipment record in the table at g_pad_ctx + 0xCE0. */
typedef struct
{
    u8 name[0x14];
    union
    {
        u32 word;
        struct
        {
            u16 low;
            u16 material;
        } half;
    } attributes;
    u8 reserved_18[0xC];
    union
    {
        u16 kind0_value;
        u16 kind1_stats[4];
        struct
        {
            u8 group;
            u8 index;
            u8 value;
        } kind2;
    } data;
    u8 reserved_2c[0x14];
} GosubEquipmentRecord;

/** @brief Save-data prefix through the 100-record equipment table. */
typedef struct
{
    u8 save_prefix[0xCE0];
    GosubEquipmentRecord equipment[100];
} GosubSaveData;

/** @brief Per-row scratch text storage. */
typedef struct
{
    u8 text[0x50];
} GosubTextBuffer;

/** @brief Header of the text archive rooted at g_gosub_text_archive_offsets_0. */
typedef struct
{
    u32 block_offsets[13];
} GosubTextArchive;

/** @brief Three-entry lookup table indexed by gosub screen group. */
typedef struct
{
    s32 values[3];
} GosubGroupTable;

/**
 * @brief LINE_F4 packet used to draw an animated scroll marker.
 */
typedef struct
{
    u8 addr[3]; /* 0x00 P_TAG addr (24-bit, set via addPrim) */
    u8 len;     /* 0x03 P_TAG len */
    u8 r;       /* 0x04 */
    u8 g;       /* 0x05 */
    u8 b;       /* 0x06 */
    u8 code;    /* 0x07 */
    s16 x0;     /* 0x08 */
    s16 y0;     /* 0x0A */
    s16 x1;     /* 0x0C */
    s16 y1;     /* 0x0E */
    s16 x2;     /* 0x10 */
    s16 y2;     /* 0x12 */
    s16 x3;     /* 0x14 */
    s16 y3;     /* 0x16 */
    u32 mask;   /* 0x18 LINE_F4 pad word (0x55555555) */
} GosubScrollMarkerPacket; /* 0x1C */

/** @brief Offset tables at the head of the message archive. */
typedef struct
{
    u16 header_offsets[0x22];
    u16 text_offsets[16];
} GosubMessageArchiveHeader;

/** @brief VRAM upload position for an image and its CLUT. */
typedef struct
{
    u16 pixel_x;
    u16 pixel_y;
    u16 clut_x;
    u16 clut_y;
} GosubImageVramLayout;

/** @brief Glyph cell descriptor in the 8-byte g_gosub_glyph_metrics table. */
typedef struct
{
    u8 u0;    /* 0x00 texture u */
    u8 reserved_1;
    u8 v0;    /* 0x02 texture v */
    u8 reserved_3;
    u16 w;    /* 0x04 */
    u16 h;    /* 0x06 */
} GosubGlyphMetric; /* 0x08 */

/** @brief One positioned glyph in a composite icon layout. */
typedef struct
{
    s8 x;        /* 0x00, in 16-pixel cells */
    s8 y;        /* 0x01, in 16-pixel cells */
    s16 glyph_id; /* 0x02 */
} GosubCompositeIconPart; /* 0x04 */

/** @brief One 88-byte composite icon layout in D_800F1CD0. */
typedef struct
{
    u8 part_count; /* 0x00 */
    u8 reserved_01;
    u8 grid_width;  /* 0x02 */
    u8 grid_height; /* 0x03 */
    s16 origin_x;   /* 0x04, in 8-pixel cells */
    s16 origin_y;   /* 0x06, in 8-pixel cells */
    s8 base_x;      /* 0x08, in 8-pixel cells */
    s8 base_y;      /* 0x09, in 8-pixel cells */
    u8 reserved_0a[2];
    GosubCompositeIconPart parts[GOSUB_COMPOSITE_ICON_PART_CAPACITY]; /* 0x0C */
} GosubCompositeIconLayout; /* 0x58 */

/** @brief Byte and structured views of a composite icon table cursor. */
typedef union
{
    u8* bytes;
    GosubCompositeIconLayout* layout;
} GosubCompositeIconView;

/* External data. */

extern u8* g_pad_ctx;
extern s32 g_field_gosub_state;
extern s32 g_pad_input;
extern s32 g_frame_counter;
extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values[];
extern s32 g_gosub_message_archive_offset;
extern s32 g_gosub_text_archive_0;
extern u32 g_gosub_text_archive_offsets_0[];
extern u32 g_gosub_text_archive_offsets_1[];
extern u32 g_gosub_text_archive_offsets_2[];
extern u32 g_gosub_text_archive_offsets_3[];
extern s32 g_gosub_text_archive_offsets_5;
extern s32 g_gosub_text_archive_offsets_6;
extern u8 g_gosub_item_metadata[];
extern GosubGroupTable g_gosub_group_first_indices;
extern GosubGroupTable g_gosub_group_counts;
extern s32 g_gosub_portrait_archive[];
extern TimPrefix g_gosub_image_archive;
extern GosubGlyphMetric g_gosub_glyph_metrics[];
extern u8 g_gosub_font_texture[];
extern u8 D_800EC3DA[];
extern u8 D_800EC3E2[];
extern u8 D_800EC3EE[];
extern u8 D_800EC3F0[];
extern u8 D_800EC3F2[];
extern s32 D_800F2180[];
extern u8 D_800F1CD0[];

/* Typed access and helper macros. */

/**
 * @brief Set the low eight bits of an element's width.
 * @param element Element to update.
 * @param width Low width byte.
 */
#define SET_ELEMENT_WIDTH_LOW(element, width) ((element)->attr.word = ((element)->attr.word & 0x00FFFFFF) | ((u32)(width) << 24))

/**
 * @brief Resolve one entry of a text archive block.
 * @param blk Block offset word, e.g. g_gosub_text_archive_offsets_1[0] or g_gosub_text_archive_offsets_0[12].
 * @param idx Entry index within the block.
 * @return Pointer to the entry.
 */
#define ARCHIVE_ENTRY(blk, idx) ((u8*)g_gosub_text_archive_offsets_0 + (blk) + *(u16*)((u8*)g_gosub_text_archive_offsets_0 + (blk) + (idx) * 2))

/**
 * @brief Resolve a message-archive entry at byte offset @p off.
 * @param off Byte offset of the u16 entry index within the resolved block.
 * @return Pointer to the entry.
 */
#define GOSUB_MSG_PTR(off) ((u8*)&g_gosub_message_archive_offset - 0x20 + g_gosub_message_archive_offset + *(u16*)((u8*)&g_gosub_message_archive_offset + g_gosub_message_archive_offset + (off)))

/**
 * @brief Resolve the message-archive entry at @p off and open its dialog.
 * @param off Byte offset of the u16 entry index within the resolved block.
 */
#define GOSUB_MSG(off) gosub_open_message_dialog(GOSUB_MSG_PTR(off))

/** @brief Link a packet after explicitly constraining its address to 24 bits. */
#define ADD_PRIM_MASKED(ot, p) (setaddr(p, getaddr(ot) & 0xFFFFFF), setaddr(ot, p))

#define GOSUB_LOGIC_BLOCK_COUNT (g_pad_ctx[GOSUB_LOGIC_BLOCK_COUNT_OFFSET])
#define GOSUB_LOGIC_BLOCK_RECORDS ((GosubPackedRecord*)(g_pad_ctx + GOSUB_LOGIC_BLOCK_RECORDS_OFFSET))

#define GOSUB_EQUIPMENT_RECORD(ptr) (&((GosubSaveData*)(ptr))->equipment[0])
#define GOSUB_EQUIPMENT_BASE_FROM_INDEX(index) (g_pad_ctx + (index) * 0x40)
#define GOSUB_EQUIPMENT_FROM_INDEX(index) GOSUB_EQUIPMENT_RECORD((index) * 0x40 + (s32)g_pad_ctx)
#define GOSUB_EQUIPMENT_SOURCE_FROM_INDEX(index) ((u8*)((index) * 0x40 + (s32)g_pad_ctx))
#define GOSUB_EQUIPMENT_AT(index) ((GosubEquipmentRecord*)(g_pad_ctx + ((index) * 0x40 + 0xCE0)))
#define GOSUB_EQUIPMENT_AT_SHIFTED_INDEX(index) ((GosubEquipmentRecord*)(g_pad_ctx + ((index) << 6) + 0xCE0))
#define GOSUB_TEXT_BUFFER(index) (((GosubTextBuffer*)g_gosub_text_buffers)[index].text)
#define GOSUB_TEXT_ARCHIVE ((GosubTextArchive*)&g_gosub_text_archive_offsets_0)
#define GOSUB_EQUIPMENT_KIND(attributes) (((attributes) >> 8) & 3)
#define GOSUB_EQUIPMENT_CATEGORY(attributes) (((attributes) >> 10) & 0x3F)
#define GOSUB_EQUIPMENT_CATEGORY_OFFSET(attributes) (((attributes) >> 9) & 0x7E)
#define GOSUB_KIND2_ARCHIVE_ENTRY(attributes)                                                                                                                  \
    ((u8*)&g_gosub_text_archive_offsets_0 + g_gosub_text_archive_offsets_2[0] + *(u16*)((u8*)g_gosub_text_archive_offsets_2 + g_gosub_text_archive_offsets_2[0] + GOSUB_EQUIPMENT_CATEGORY_OFFSET(attributes) + 0x22))

/**
 * @brief Resolve a message pointer against a caller-held archive base.
 *
 * Same lookup as GOSUB_MSG_PTR, using an archive base already held by the
 * caller. ABS addresses the offset table symbol; REL addresses the base.
 *
 * @param base Archive base, i.e. (u8*)&g_gosub_message_archive_offset - 0x20.
 * @param off Byte offset of the u16 entry within the message block.
 * @return Pointer to the message text.
 */
#define GOSUB_MSG_ABS(base, off) ((base) + g_gosub_message_archive_offset + *(u16*)((u8*)&g_gosub_message_archive_offset + g_gosub_message_archive_offset + (off)))
#define GOSUB_MSG_REL(base, off) ((base) + g_gosub_message_archive_offset + *(u16*)((base) + g_gosub_message_archive_offset + (off)))

/** @brief Resolve message entries through a gosub_draw_item_list local archive base. */
#define MSG_HDR ((GosubMessageArchiveHeader*)((u8*)&g_gosub_message_archive_offset - -g_gosub_message_archive_offset))
#define MSG_HI(off) ((void*)(g_gosub_message_archive_offset + (MSG_HDR->header_offsets[(off) >> 1] + base)))
#define MSG_LO(off) ((void*)(g_gosub_message_archive_offset + (*(u16*)(base + g_gosub_message_archive_offset + (off)) + base)))

/** @brief Test whether a byte begins a two-byte encoded character. */
#define IS_DBCS_LEAD_BYTE(byte) (((byte) >= 0x19) && ((byte) <= 0x1F))

/* External and forward function declarations. */

void bcopy();
void field_set_default_fade_target();
void func_800A8B90();
void func_800AA02C();
s32 func_800A88A0(s32 prim, s32* ot, void* text, s32 color, s32 x, s32 y, s32 mode);
s32 func_800A8A78(s32* ot, s32 prim, s32 value, s32 color, GosubTextPosition* position, s32 mode);
void gosub_load_screen_sequence(s32*);
void gosub_build_screen_9_elements();
void gosub_build_screen_10_elements();
void gosub_build_category_screen_elements();
void gosub_build_list_screen_elements();
void gosub_build_screen_11_elements();
void gosub_build_compact_list_elements();
void gosub_build_screen_15_item_list();
void gosub_build_screen_19_item_list();
void gosub_build_screen_16_item_list();
void gosub_build_screen_1_item_list();
void gosub_build_screen_0_item_list();
void gosub_build_packed_record_list();
void gosub_build_roster_list();
s32 gosub_select_row_with_validation();
s32 gosub_select_row();
s32 gosub_validate_pending_pair_selection();
s32 gosub_commit_row_reorder();
s32 gosub_update_group_selection();
s32 gosub_publish_two_row_selection();
s32 gosub_handle_combination_dialog(s32 dialog_result);
s32 gosub_publish_group_selection(void);
s32 gosub_publish_selection(void);
s32 gosub_is_row_unselected(s32 row);
void gosub_build_equipment_list(u32 item_kind);
void gosub_build_grouped_option_list(s32 group);
void gosub_update_screen(s32 render_ctx);
void gosub_enter_screen();
s32 gosub_handle_input(s32 unused);
void gosub_scroll_to_cursor(void);
s32 gosub_toggle_cursor_selection(void);
s32 gosub_advance_screen_sequence(void);
s32 gosub_are_elements_idle(void);
void gosub_start_element_exit();
void gosub_clear_elements(void);
void gosub_render_elements();
void gosub_update_and_render_elements();
void gosub_open_message_dialog();
s32 gosub_draw_message_dialog();
s32 gosub_draw_detail_header();
s32 gosub_draw_two_line_header();
s32 gosub_draw_confirmation_prompt();
s32 gosub_draw_row_description();
void gosub_append_encoded_string();
void gosub_copy_encoded_string();
s32 gosub_encoded_string_length(const u8* text);
GosubElement* gosub_allocate_element();
void* gosub_emit_scroll_marker();
GosubGpuPacket* gosub_emit_panel();
GosubLinePacket* gosub_emit_panel_outline();
GosubGpuPacket* gosub_emit_panel_corners(SPRT*, s32*, s32, s32, s32, s32);
GosubTilePacket* gosub_draw_item_list();
s32 gosub_draw_portrait(s32 prim, s32* ot, s32 row, s32 x, s32 y, s32 count);
s32 gosub_draw_equipment_details(s32 prim, s32* ot, s32 x_off, s32 y_off);
s32 gosub_draw_composite_icon(s32 initial_packet, s32* ordering_table, s32 x, s32 y, s32 icon_id, s32 layout_index);
s32 gosub_draw_combination_preview();
s32 gosub_handle_backtrack_dialog();
s32 gosub_handle_delete_dialog(s32 dialog_result);
s32 gosub_draw_title();
s32 gosub_draw_two_option_dialog(s32* ot, s32 prim, s32 x_off, s32 y_off);
s32 gosub_draw_three_option_dialog(s32* ot, s32 prim, s32 x_off, s32 y_off);
void gosub_open_row_action_dialog(void);
void gosub_open_sort_dialog(void);
void gosub_upload_ui_image(void);
void gosub_upload_image_archive(GosubImageVramLayout* destinations, TimPrefix* tim);
inline void gosub_copy_packed_record(void* dst, void* src);
inline void gosub_copy_list_row(void* dst, void* src);
void gosub_delete_packed_record(s32 record_index);
void gosub_delete_list_row(s32 row);
s32 gosub_compare_rows(s32 mode, s32 left_row_index, s32 right_row_index);
void gosub_upload_font_texture(void);

/* Overlay BSS layout is address-sensitive; do not reorder these definitions. */

s32 g_gosub_frame_parity;
s32 g_gosub_finished;
s32 g_gosub_cursor_row;
s32 g_gosub_row_count;
s32 g_gosub_visible_row_count;
/** @brief Screen-sequence cursor stored in a four-byte BSS slot. */
u8 g_gosub_screen_sequence_index;
u8 g_gosub_screen_sequence_index_storage[4] __asm__("g_gosub_screen_sequence_index");
s32 g_gosub_scroll_frames_remaining;
s32 g_gosub_combination_variant;
s32 g_gosub_dialog_choice;
s32 g_gosub_combination_quantity;
s32 g_gosub_allow_duplicate_selection;
s32 g_gosub_dialog_text;
s32 (*g_gosub_finish_handler)();
u8 g_gosub_selection_mode;
/** @brief Required selection count stored in a three-byte BSS slot. */
u8 g_gosub_required_selection_count;
u8 g_gosub_required_selection_count_storage[3] __asm__("g_gosub_required_selection_count");
/** @brief Row-detail flag stored in an eight-byte BSS slot. */
s32 g_gosub_show_row_details;
u8 g_gosub_show_row_details_storage[8] __asm__("g_gosub_show_row_details");
s32 g_gosub_result_rows[16];
s32 g_gosub_dialog_accepting_input;
u8 g_gosub_selected_rows[4];
s32 g_gosub_window_height;
s32 (*g_gosub_select_handler)();
s32 g_gosub_window_width;
s32 g_gosub_suppress_dialog_sound;
u8 g_gosub_text_buffers[0x5000];
/** @brief Current selection count stored in an eight-byte BSS slot. */
u8 g_gosub_selection_count;
u8 g_gosub_selection_count_storage[8] __asm__("g_gosub_selection_count");
u8 g_gosub_screen_sequence[20];
s32 g_gosub_combination_result_id;
/** @brief List row height stored in an eight-byte BSS slot. */
s32 g_gosub_row_height;
u8 g_gosub_row_height_storage[8] __asm__("g_gosub_row_height");
s32 g_gosub_scroll_y;
/** @brief Whether the next confirmed sort uses ascending order. */
s32 g_gosub_sort_ascending;
s32 g_gosub_scroll_target_y;
u8* g_gosub_title_text;
GosubElement g_gosub_elements[1];
GosubElement g_gosub_dynamic_elements[GOSUB_ELEMENT_COUNT - 1];
GosubListRow g_gosub_rows[512];
s32 (*g_gosub_dialog_handler)(s32);

/**
 * @brief Open the gosub overlay for a sequence of screen ids.
 *
 * @param unused Unused loader argument.
 * @param screen_sequence Pointer to an s32 array terminated by
 *        @c GOSUB_SCREEN_SEQUENCE_END.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/qM81L
 */
void gosub_open_screen_sequence(s32 unused, s32 screen_sequence)
{
    field_set_default_fade_target();
    g_gosub_frame_parity = 0;
    g_gosub_finished = 0;
    func_800AA02C();
    gosub_load_screen_sequence(screen_sequence);
}

/**
 * @brief Run one frame of the gosub overlay and return its completion state.
 * @param render_ctx Active field rendering context.
 * @return Nonzero after the current gosub sequence finishes.
 * @see decomp.me (100%) https://decomp.me/scratch/ykfW4
 */
s32 gosub_update_frame(s32 render_ctx)
{
    s32* frame_parity;
    s32 finished;
    field_text_reset_scratch();
    gosub_update_screen(render_ctx);
    func_80063194();
    frame_parity = &g_gosub_frame_parity;
    finished = g_gosub_finished;
    *frame_parity ^= 1;
    return finished;
}

/**
 * @brief Copy and enter a GOSUB_SCREEN_SEQUENCE_END-terminated screen sequence.
 * @param screen_sequence Sequence of screen ids stored as s32 values.
 * @see decomp.me (100%) https://decomp.me/scratch/weBhP
 */
void gosub_load_screen_sequence(s32* screen_sequence)
{
    u8* sequence_cursor;
    s32 screen_count;
    u8 screen_id;
    s32 stack_pad[2];

    gosub_upload_ui_image();
    g_gosub_scroll_frames_remaining = 0;
    g_gosub_scroll_target_y = 0;
    g_gosub_scroll_y = 0;
    g_gosub_cursor_row = 0;
    gosub_clear_elements();
    g_gosub_sort_ascending = 0;
    g_gosub_screen_sequence_index = 0;
    g_gosub_result_count = 0;
    g_gosub_dialog_handler = (void*)gosub_handle_backtrack_dialog;
    screen_count = 0;
    if (*screen_sequence != GOSUB_SCREEN_SEQUENCE_END)
    {
        u8* sequence = g_gosub_screen_sequence;
        s32 sentinel = GOSUB_SCREEN_SEQUENCE_END;
        sequence_cursor = (u8*)screen_sequence;
        do
        {
            screen_id = *sequence_cursor;
            sequence_cursor += 4;
            *((u8*)(screen_count + (u32)sequence)) = screen_id;
            screen_count++;
        } while (*(s32*)sequence_cursor != sentinel);
    }
    g_gosub_screen_sequence[screen_count] = ((u8*)screen_sequence)[screen_count * 4];
    g_gosub_dialog_accepting_input = 0;
    gosub_enter_screen(g_gosub_screen_sequence[g_gosub_screen_sequence_index], screen_count);
    gosub_upload_font_texture();
}

/**
 * @brief Initialize a gosub sub-screen and install its selection callbacks.
 * @param screen_id Screen id, 0..19; anything else returns without touching state.
 * @param unused Unused by this function; passed by gosub_load_screen_sequence.
 * @see decomp.me (100%)
 */
void gosub_enter_screen(screen_id, unused) s32 screen_id;
s32 unused;
{
    g_gosub_scroll_frames_remaining = 0;
    g_gosub_scroll_target_y = 0;
    g_gosub_scroll_y = 0;
    g_gosub_cursor_row = 0;
    g_gosub_combination_variant = 0;
    g_gosub_combination_result_id = 0;
    g_gosub_combination_quantity = 0;
    g_gosub_allow_duplicate_selection = 0;
    g_gosub_show_row_details = 0;

    switch (screen_id)
    {
    case 0:
        gosub_start_element_exit();
        gosub_build_screen_0_item_list();
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = (void*)gosub_select_row;
        g_gosub_finish_handler = (void*)gosub_publish_selection;
        gosub_build_list_screen_elements(1);
        if (g_gosub_row_count == 0)
        {
            gosub_start_element_exit();
            GOSUB_MSG(8);
        }
        break;

    case 1:
        gosub_start_element_exit();
        gosub_build_screen_1_item_list();
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = (void*)gosub_select_row;
        g_gosub_finish_handler = (void*)gosub_publish_selection;
        gosub_build_list_screen_elements(1);
        if (g_gosub_row_count == 0)
        {
            gosub_start_element_exit();
            GOSUB_MSG(0xA);
        }
        break;

    case 2:
        gosub_start_element_exit();
        gosub_build_equipment_list(0);
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = (void*)gosub_select_row;
        g_gosub_finish_handler = (void*)gosub_publish_selection;
        gosub_build_category_screen_elements();
        if (g_gosub_row_count == 0)
        {
            gosub_start_element_exit();
            GOSUB_MSG(2);
        }
        break;

    case 3:
        gosub_start_element_exit();
        gosub_build_equipment_list(1);
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = (void*)gosub_select_row;
        g_gosub_finish_handler = (void*)gosub_publish_selection;
        gosub_build_category_screen_elements();
        if (g_gosub_row_count == 0)
        {
            gosub_start_element_exit();
            GOSUB_MSG(4);
        }
        break;

    case 4:
        gosub_start_element_exit();
        gosub_build_equipment_list(2);
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = (void*)gosub_select_row;
        g_gosub_finish_handler = (void*)gosub_publish_selection;
        gosub_build_category_screen_elements();
        if (g_gosub_row_count == 0)
        {
            gosub_start_element_exit();
            GOSUB_MSG(6);
        }
        break;

    case 5:
        gosub_start_element_exit();
        gosub_build_equipment_list(3);
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = (void*)gosub_select_row;
        g_gosub_finish_handler = (void*)gosub_publish_selection;
        gosub_build_category_screen_elements();
        if (g_gosub_row_count == 0)
        {
            gosub_start_element_exit();
            GOSUB_MSG(0);
        }
        break;

    case 6:
    case 7:
    case 8:
        gosub_start_element_exit();
        gosub_build_grouped_option_list(screen_id - 6);
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = (void*)gosub_select_row;
        g_gosub_finish_handler = (void*)gosub_publish_selection;
        gosub_build_list_screen_elements(1);
        break;

    case 9:
        gosub_start_element_exit();
        gosub_build_equipment_list(4);
        g_gosub_required_selection_count = 4;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = (void*)gosub_update_group_selection;
        g_gosub_finish_handler = (void*)gosub_publish_group_selection;
        gosub_build_screen_9_elements();
        if (g_gosub_row_count == 0)
        {
            gosub_start_element_exit();
            GOSUB_MSG(-2);
        }
        break;

    case 10:
        gosub_start_element_exit();
        gosub_build_equipment_list(3);
        g_gosub_show_row_details = 1;
        g_gosub_visible_row_count = 6;
        g_gosub_row_height = 0x10;
        g_gosub_window_width = 0xE8;
        g_gosub_window_height = 0x64;
        g_gosub_combination_variant = 0;
        g_gosub_combination_result_id = 0;
        g_gosub_combination_quantity = 0;
        g_gosub_required_selection_count = 2;
        g_gosub_selection_mode = 2;
        g_gosub_select_handler = (void*)gosub_validate_pending_pair_selection;
        g_gosub_finish_handler = (void*)gosub_publish_two_row_selection;
        g_gosub_dialog_handler = (void*)gosub_handle_combination_dialog;
        gosub_build_screen_10_elements();
        if (g_pad_ctx[0x29D6] >= 0x28)
        {
            gosub_start_element_exit();
            GOSUB_MSG(-4);
        }
        break;

    case 11:
        gosub_start_element_exit();
        gosub_build_packed_record_list();
        g_gosub_required_selection_count = 2;
        g_gosub_selection_mode = 2;
        g_gosub_select_handler = (void*)gosub_commit_row_reorder;
        g_gosub_finish_handler = (void*)gosub_publish_selection;
        g_gosub_allow_duplicate_selection = 1;
        gosub_build_screen_11_elements();
        if (g_pad_ctx[0x29D6] == 0)
        {
            gosub_start_element_exit();
            GOSUB_MSG(-6);
        }
        break;

    case 12:
        gosub_start_element_exit();
        gosub_build_roster_list(0);
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = (void*)gosub_select_row_with_validation;
        g_gosub_finish_handler = (void*)gosub_publish_selection;
        gosub_build_compact_list_elements();
        if (g_gosub_row_count == 0)
        {
            gosub_start_element_exit();
            GOSUB_MSG(0x32);
        }
        break;

    case 13:
        gosub_start_element_exit();
        gosub_build_roster_list(1);
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = (void*)gosub_select_row_with_validation;
        g_gosub_finish_handler = (void*)gosub_publish_selection;
        gosub_build_compact_list_elements();
        if (g_gosub_row_count == 0)
        {
            gosub_start_element_exit();
            GOSUB_MSG(0x34);
        }
        break;

    case 14:
        gosub_start_element_exit();
        gosub_build_roster_list(2);
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = (void*)gosub_select_row;
        g_gosub_finish_handler = (void*)gosub_publish_selection;
        gosub_build_compact_list_elements();
        if (g_gosub_row_count == 0)
        {
            gosub_start_element_exit();
            GOSUB_MSG(0x36);
        }
        break;

    case 15:
        gosub_start_element_exit();
        gosub_build_screen_15_item_list();
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = (void*)gosub_select_row;
        g_gosub_finish_handler = (void*)gosub_publish_selection;
        gosub_build_list_screen_elements(1);
        if (g_gosub_row_count == 0)
        {
            gosub_start_element_exit();
            GOSUB_MSG(0x3E);
        }
        break;

    case 16:
        gosub_start_element_exit();
        gosub_build_screen_16_item_list();
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = (void*)gosub_select_row;
        g_gosub_finish_handler = (void*)gosub_publish_selection;
        gosub_build_list_screen_elements(0);
        if (g_gosub_row_count == 0)
        {
            gosub_start_element_exit();
            GOSUB_MSG(0x3C);
        }
        break;

    case 17:
        gosub_start_element_exit();
        gosub_build_roster_list(0);
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = (void*)gosub_select_row;
        g_gosub_finish_handler = (void*)gosub_publish_selection;
        gosub_build_compact_list_elements();
        if (g_gosub_row_count == 0)
        {
            gosub_start_element_exit();
            GOSUB_MSG(0x32);
        }
        break;

    case 18:
        gosub_start_element_exit();
        gosub_build_roster_list(1);
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = (void*)gosub_select_row;
        g_gosub_finish_handler = (void*)gosub_publish_selection;
        gosub_build_compact_list_elements();
        if (g_gosub_row_count == 0)
        {
            gosub_start_element_exit();
            GOSUB_MSG(0x34);
        }
        break;

    case 19:
        gosub_start_element_exit();
        gosub_build_screen_19_item_list();
        g_gosub_required_selection_count = 1;
        g_gosub_selection_mode = 1;
        g_gosub_select_handler = (void*)gosub_select_row;
        g_gosub_finish_handler = (void*)gosub_publish_selection;
        gosub_build_list_screen_elements(1);
        if (g_gosub_row_count == 0)
        {
            gosub_start_element_exit();
            GOSUB_MSG(0x4C);
        }
        break;
    }
}

/**
 * @brief Build the list, header, and detail elements for screen 9.
 * @see decomp.me (100%)
 */
void gosub_build_screen_9_elements(void)
{
    GosubElement* element;

    element = gosub_allocate_element();
    element->draw_handler = (void*)gosub_draw_item_list;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0xA0 - g_gosub_window_width / 2;
    element->attr.f.width_low = 0x38;
    element->width_high = 0;
    element->y = g_gosub_row_height * g_gosub_visible_row_count + 4;
    SET_ELEMENT_WIDTH_LOW(element, 0xE8);
    g_gosub_selection_count = 0;

    element = gosub_allocate_element();
    element->draw_handler = (void*)&gosub_draw_two_line_header;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0x1C;
    element->attr.f.width_low = 0x10;
    element->width_high = 1;
    element->y = 0x24;
    SET_ELEMENT_WIDTH_LOW(element, 8);

    element = gosub_allocate_element();
    element->draw_handler = (void*)&gosub_draw_row_description;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0x1C;
    element->attr.f.width_low = 0xB0;
    element->width_high = 1;
    element->y = 0x24;
    SET_ELEMENT_WIDTH_LOW(element, 8);
}

/**
 * @brief Build the list, detail header, and preview elements for screen 10.
 * @see decomp.me (100%)
 */
void gosub_build_screen_10_elements(void)
{
    GosubElement* element;

    element = gosub_allocate_element();
    element->draw_handler = (void*)gosub_draw_item_list;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0xA0 - g_gosub_window_width / 2;
    element->attr.f.width_low = 0x48;
    element->width_high = 0;
    element->y = g_gosub_row_height * g_gosub_visible_row_count + 4;
    SET_ELEMENT_WIDTH_LOW(element, 0xE8);
    g_gosub_selection_count = 0;

    element = gosub_allocate_element();
    element->draw_handler = (void*)&gosub_draw_detail_header;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0x1C;
    element->attr.f.width_low = 0xB0;
    element->width_high = 1;
    element->y = 0x24;
    SET_ELEMENT_WIDTH_LOW(element, 8);

    element = gosub_allocate_element();
    element->draw_handler = (void*)gosub_draw_combination_preview;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0x1C;
    element->attr.f.width_low = 0x20;
    element->width_high = 1;
    element->y = 0x24;
    SET_ELEMENT_WIDTH_LOW(element, 8);
}

/**
 * @brief Configure reserved element 0 and reset the dialog choice.
 * @see decomp.me (100%)
 */
void gosub_initialize_fixed_element(void)
{
    GosubElement* element;

    element = &g_gosub_elements[0];
    element->draw_handler = (void*)&gosub_draw_confirmation_prompt;
    g_gosub_dialog_choice = 0;
    element->attr.f.state = GOSUB_ELEMENT_STATE_ENTERING;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0x20;
    element->attr.f.width_low = 0x70;
    element->width_high = 1;
    element->y = 0x24;
    SET_ELEMENT_WIDTH_LOW(element, 0);
}

/**
 * @brief Build the three elements of the gosub screens entered by arms 2 to 5.
 * @see decomp.me (100%)
 */
void gosub_build_category_screen_elements(void)
{
    GosubElement* element;

    element = gosub_allocate_element();
    element->draw_handler = (void*)gosub_draw_item_list;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0xA0 - g_gosub_window_width / 2;
    element->attr.f.width_low = 0x28;
    element->width_high = 0;
    element->y = g_gosub_row_height * g_gosub_visible_row_count + 4;
    SET_ELEMENT_WIDTH_LOW(element, 0xE8);
    g_gosub_selection_count = 0;

    element = gosub_allocate_element();
    element->draw_handler = (void*)&gosub_draw_row_description;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0x1C;
    element->attr.f.width_low = 0xB0;
    element->width_high = 1;
    element->y = 0x24;
    SET_ELEMENT_WIDTH_LOW(element, 8);

    element = gosub_allocate_element();
    element->draw_handler = (void*)gosub_draw_title;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0x1C;
    element->attr.f.width_low = 0x10;
    element->width_high = 1;
    element->y = 0x14;
    SET_ELEMENT_WIDTH_LOW(element, 8);
}

/**
 * @brief Build the elements of the gosub screens entered by arms 0, 1, 6-8,
 *        15, 16 and 19.
 * @param include_middle Non-zero to include the middle element, zero to skip it.
 * @see decomp.me (100%)
 */
void gosub_build_list_screen_elements(s32 include_middle)
{
    GosubElement* element;

    element = gosub_allocate_element();
    element->draw_handler = (void*)gosub_draw_item_list;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0xA0 - g_gosub_window_width / 2;
    element->attr.f.width_low = 0x28;
    element->width_high = 0;
    element->y = g_gosub_row_height * g_gosub_visible_row_count + 4;
    SET_ELEMENT_WIDTH_LOW(element, 0xE8);
    g_gosub_selection_count = 0;

    if (include_middle != 0)
    {
        element = gosub_allocate_element();
        element->draw_handler = (void*)&gosub_draw_row_description;
        element->attr.f.transition_step = 1;
        element->attr.f.x = 0x1C;
        element->attr.f.width_low = 0xB0;
        element->width_high = 1;
        element->y = 0x14;
        SET_ELEMENT_WIDTH_LOW(element, 8);
    }

    element = gosub_allocate_element();
    element->draw_handler = (void*)gosub_draw_title;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0x1C;
    element->attr.f.width_low = 0x10;
    element->width_high = 1;
    element->y = 0x14;
    SET_ELEMENT_WIDTH_LOW(element, 8);
}

/**
 * @brief Build the three elements of the gosub screen entered by arm 11.
 * @see decomp.me (100%)
 */
void gosub_build_screen_11_elements(void)
{
    GosubElement* element;

    element = gosub_allocate_element();
    element->draw_handler = (void*)gosub_draw_item_list;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0xA0 - g_gosub_window_width / 2;
    element->attr.f.width_low = 0x28;
    element->width_high = 1;
    element->y = g_gosub_row_height * g_gosub_visible_row_count + 4;
    SET_ELEMENT_WIDTH_LOW(element, 0x20);
    g_gosub_selection_count = 0;

    element = gosub_allocate_element();
    element->draw_handler = (void*)&gosub_draw_row_description;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0x1C;
    element->attr.f.width_low = 0xB0;
    element->width_high = 1;
    element->y = 0x14;
    SET_ELEMENT_WIDTH_LOW(element, 8);

    element = gosub_allocate_element();
    element->draw_handler = (void*)gosub_draw_title;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0x1C;
    element->attr.f.width_low = 0x10;
    element->width_high = 1;
    element->y = 0x14;
    SET_ELEMENT_WIDTH_LOW(element, 8);
}

/**
 * @brief Build the two elements of the gosub screens entered by arms 12-14 and
 *        17-18.
 * @see decomp.me (100%)
 */
void gosub_build_compact_list_elements(void)
{
    GosubElement* element;

    element = gosub_allocate_element();
    element->draw_handler = (void*)gosub_draw_item_list;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0xA0 - g_gosub_window_width / 2;
    element->attr.f.width_low = 0x30;
    element->width_high = 1;
    element->y = g_gosub_row_height * g_gosub_visible_row_count + 4;
    SET_ELEMENT_WIDTH_LOW(element, 0x18);
    g_gosub_selection_count = 0;

    element = gosub_allocate_element();
    element->draw_handler = (void*)gosub_draw_title;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0x1C;
    element->attr.f.width_low = 0x10;
    element->width_high = 1;
    element->y = 0x14;
    SET_ELEMENT_WIDTH_LOW(element, 8);
}

/**
 * @brief Build screen 15's rows from nonempty inventory slots 0x60-0x84.
 * @see decomp.me (100%)
 */
void gosub_build_screen_15_item_list(void)
{
    s32 i;
    s32 count;

    count = 0;
    for (i = 0x60; i < 0x85; i++)
    {
        if (*(g_pad_ctx + i + 0x25E0) != 0)
        {
            GosubListRow* row = &g_gosub_rows[count];
            row->name = ARCHIVE_ENTRY(g_gosub_text_archive_offsets_1[0], i);
            row->desc = ARCHIVE_ENTRY(g_gosub_text_archive_offsets_0[12], g_gosub_item_metadata[i]);
            row->value = *(g_pad_ctx + i + 0x25E0);
            row->text_color = 4;
            row->index = i;
            count++;
        }
    }
    g_gosub_row_count = count;
    g_gosub_visible_row_count = 8;
    g_gosub_row_height = 0x10;
    g_gosub_window_width = 0xE8;
    g_gosub_window_height = 0x84;
    g_gosub_title_text = GOSUB_MSG_PTR(0x3A);
}

/**
 * @brief Build screen 19's rows from nonempty inventory slots 0x60-0x8F.
 * @see decomp.me (100%)
 */
void gosub_build_screen_19_item_list(void)
{
    s32 i;
    s32 count;

    count = 0;
    for (i = 0x60; i < 0x90; i++)
    {
        if (*(g_pad_ctx + i + 0x25E0) != 0)
        {
            GosubListRow* row = &g_gosub_rows[count];
            row->name = ARCHIVE_ENTRY(g_gosub_text_archive_offsets_1[0], i);
            row->desc = ARCHIVE_ENTRY(g_gosub_text_archive_offsets_0[2], i);
            row->value = *(g_pad_ctx + i + 0x25E0);
            row->text_color = 4;
            row->index = i;
            count++;
        }
    }
    g_gosub_row_count = count;
    g_gosub_visible_row_count = 8;
    g_gosub_row_height = 0x10;
    g_gosub_window_width = 0xE8;
    g_gosub_window_height = 0x84;
    g_gosub_title_text = GOSUB_MSG_PTR(0x4A);
}

/**
 * @brief Build screen 16's rows from nonempty inventory slots 0x40-0x4F.
 * @see decomp.me (100%)
 */
void gosub_build_screen_16_item_list(void)
{
    s32 i;
    s32 count;

    count = 0;
    for (i = 0x40; i < 0x50; i++)
    {
        if (*(g_pad_ctx + i + 0x25E0) != 0)
        {
            GosubListRow* row = &g_gosub_rows[count];
            row->name = ARCHIVE_ENTRY(g_gosub_text_archive_offsets_1[0], i);
            row->value = *(g_pad_ctx + i + 0x25E0);
            row->text_color = 4;
            row->index = i;
            count++;
        }
    }
    g_gosub_row_count = count;
    g_gosub_visible_row_count = 8;
    g_gosub_row_height = 0x10;
    g_gosub_window_width = 0xE8;
    g_gosub_window_height = 0x84;
    g_gosub_title_text = GOSUB_MSG_PTR(0x38);
}

/**
 * @brief Build screen 1's rows from nonempty inventory slots 0x40-0xFE.
 * @see decomp.me (100%)
 */
void gosub_build_screen_1_item_list(void)
{
    s32 i;
    s32 count;

    count = 0;
    for (i = 0x40; i < 0xFF; i++)
    {
        if (*(g_pad_ctx + i + 0x25E0) != 0)
        {
            GosubListRow* row = &g_gosub_rows[count];
            row->name = ARCHIVE_ENTRY(g_gosub_text_archive_offsets_1[0], i);
            row->desc = ARCHIVE_ENTRY(g_gosub_text_archive_offsets_0[2], i);
            row->value = *(g_pad_ctx + i + 0x25E0);
            row->text_color = 4;
            row->index = i;
            count++;
        }
    }
    g_gosub_row_count = count;
    g_gosub_visible_row_count = 8;
    g_gosub_row_height = 0x10;
    g_gosub_window_width = 0xE8;
    g_gosub_window_height = 0x84;
    g_gosub_title_text = GOSUB_MSG_PTR(0x14);
}

/**
 * @brief Build screen 0's rows from nonempty inventory slots 0x00-0x3F.
 * @see decomp.me (100%)
 */
void gosub_build_screen_0_item_list(void)
{
    s32 i;
    s32 count;

    count = 0;
    for (i = 0; i < 0x40; i++)
    {
        if (*(g_pad_ctx + i + 0x25E0) != 0)
        {
            GosubListRow* row = &g_gosub_rows[count];
            row->name = ARCHIVE_ENTRY(g_gosub_text_archive_offsets_1[0], i);
            row->desc = ARCHIVE_ENTRY(g_gosub_text_archive_offsets_0[2], i);
            row->value = *(g_pad_ctx + i + 0x25E0);
            row->text_color = 4;
            row->index = i;
            count++;
        }
    }
    g_gosub_row_count = count;
    g_gosub_visible_row_count = 8;
    g_gosub_row_height = 0x10;
    g_gosub_window_width = 0xE8;
    g_gosub_window_height = 0x84;
    g_gosub_title_text = GOSUB_MSG_PTR(0x12);
}

/**
 * @brief Build the packed-record list for the gosub screen entered by arm 11.
 * @note Bit 2 of the flag word is cleared only for records that are both unflagged at
 *       bit 16 and have both low bits set; every other record sets it.
 * @see decomp.me (100%)
 */
void gosub_build_packed_record_list(void)
{
    s32 i;
    u8* row_name;
    u8 number_text[32];

    for (i = 0; i < *(g_pad_ctx + 0x29D6); i++)
    {
        g_gosub_rows[i].detail_group = *(g_pad_ctx + (i << 2) + 0x29DC) >> 2;
        g_gosub_rows[i].detail_id = (*(u32*)(g_pad_ctx + (i << 2) + 0x29DC) >> 8) & 0xF;
        row_name = g_gosub_text_buffers + i * 0x50;
        gosub_copy_encoded_string(row_name, ARCHIVE_ENTRY(g_gosub_text_archive_offsets_3[0], g_gosub_rows[i].detail_group));
        if (g_gosub_rows[i].detail_id != 0)
        {
            gosub_append_encoded_string(row_name, D_800EC3DA - 0x16 + D_800EC3DA[0] + (D_800EC3DA[1] << 8));
            func_800A8B90(number_text, g_gosub_rows[i].detail_id, 1);
            gosub_append_encoded_string(row_name, number_text);
        }
        g_gosub_rows[i].name = row_name;
        g_gosub_rows[i].desc = ARCHIVE_ENTRY(g_gosub_text_archive_offsets_0[7], g_gosub_rows[i].detail_group);
        g_gosub_rows[i].value = -2;
        g_gosub_rows[i].detail_variant = (*(u32*)(g_pad_ctx + (i << 2) + 0x29DC) >> 12) & 0xF;
        if (((*(u32*)(g_pad_ctx + (i << 2) + 0x29DC) >> 16) & 1) != 0 || (*(u32*)(g_pad_ctx + (i << 2) + 0x29DC) & 3) != 3)
        {
            g_gosub_rows[i].flags.word |= 4;
        }
        else
        {
            g_gosub_rows[i].flags.word &= ~4;
        }
        g_gosub_rows[i].index = i;
        g_gosub_rows[i].text_color = 4;
    }
    g_gosub_row_count = *(g_pad_ctx + 0x29D6);
    g_gosub_visible_row_count = 4;
    g_gosub_row_height = 0x20;
    g_gosub_window_width = 0x120;
    g_gosub_window_height = 0x84;
    g_gosub_title_text = GOSUB_MSG_PTR(0x18);
}

/**
 * @brief Build a roster list for gosub screens 12-14 and 17-18.
 * @param mode Which blocks to emit: 1 = second only, 2 = first only, otherwise
 *             both. Also picks the screen's title message.
 * @see decomp.me (100%)
 */
void gosub_build_roster_list(s32 mode)
{
    s32 large_ref_index;
    s32 stat_index;
    s32 row_count;
    s32 record_index;
    s32 record_offset;
    s32 slot;
    s32 record_offset_reload;
    u8 unused_name_buf[32];

    row_count = 0;
    if (mode != 1)
    {
        for (large_ref_index = 0; large_ref_index < 3; large_ref_index++)
        {
            record_index = *(g_pad_ctx + large_ref_index + 0x29D8);
            if (record_index < 3)
            {
                if (mode == 0)
                {
                    g_gosub_rows[row_count].index = record_index & 0x80;
                }
                else
                {
                    g_gosub_rows[row_count].index = record_index;
                }
                g_gosub_rows[row_count].value = -3;
                g_gosub_rows[row_count].flags.f.alternate_format = 0;
                if (*(s8*)(g_pad_ctx + 0x29D7) == record_index)
                {
                    g_gosub_rows[row_count].detail_group = 1;
                }
                else
                {
                    g_gosub_rows[row_count].detail_group = 0;
                }
                g_gosub_rows[row_count].flags.f.selection_blocked = 0;
                g_gosub_rows[row_count].flags.f.selection_restricted = 0;
                g_gosub_rows[row_count].text_color = 4;
                record_offset = record_index * 332;
                g_gosub_rows[row_count].name = g_pad_ctx + 0x2B0C + record_offset;
                g_gosub_rows[row_count].detail_id = *(g_pad_ctx + record_offset + 0x2B50) & 0xF;
                g_gosub_rows[row_count].detail_variant = *(g_pad_ctx + record_offset + 0x2B54);
                g_gosub_rows[row_count].primary_value = *(u16*)(g_pad_ctx + record_offset + 0x2B24);
                for (stat_index = 0; stat_index < 4; stat_index++)
                {
                    g_gosub_rows[row_count].stats[stat_index] = *(u16*)(g_pad_ctx + record_offset + 0x2B26 + stat_index * 2);
                }
                record_offset_reload = record_index * 332;
                g_gosub_rows[row_count].secondary_value = *(u16*)(g_pad_ctx + record_offset_reload + 0x2B22);
                row_count++;
            }
        }
    }
    if (mode != 2)
    {
        for (slot = 0; slot < 5; slot++)
        {
            record_offset = slot * 0x60;
            if (*(g_pad_ctx + record_offset + 0x2EF4) != 0)
            {
                g_gosub_rows[row_count].index = slot;
                g_gosub_rows[row_count].value = -3;
                g_gosub_rows[row_count].flags.f.alternate_format = 1;
                if (*(s32*)(g_pad_ctx + 0x2EF0) == slot)
                {
                    g_gosub_rows[row_count].detail_group = 1;
                }
                else
                {
                    g_gosub_rows[row_count].detail_group = 0;
                }
                g_gosub_rows[row_count].text_color = 4;
                g_gosub_rows[row_count].name = g_pad_ctx + 0x2EF4 + slot * 0x60;
                g_gosub_rows[row_count].detail_id = *(g_pad_ctx + record_offset + 0x2F09);
                g_gosub_rows[row_count].primary_value = *(u16*)(g_pad_ctx + record_offset + 0x2F12);
                g_gosub_rows[row_count].flags.f.selection_blocked = *(u32*)(g_pad_ctx + record_offset + 0x2F38) >> 31;
                g_gosub_rows[row_count].flags.f.selection_restricted = (*(u32*)(g_pad_ctx + record_offset + 0x2F38) >> 30) & 1;
                g_gosub_rows[row_count].detail_variant = *(g_pad_ctx + record_offset + 0x2F0C);
                if (g_gosub_rows[row_count].flags.half & 1)
                {
                    g_gosub_rows[row_count].detail_id = *(g_pad_ctx + record_offset + 0x2F0A) + 0x48;
                    if (*(u16*)(g_pad_ctx + record_offset + 0x2F36) < 6)
                    {
                        g_gosub_rows[row_count].detail_variant = 0;
                    }
                    else if (*(u16*)(g_pad_ctx + record_offset + 0x2F36) < 0x1F)
                    {
                        g_gosub_rows[row_count].detail_variant = 1;
                    }
                    else
                    {
                        g_gosub_rows[row_count].detail_variant = 2;
                    }
                }
                for (stat_index = 0; stat_index < 4; stat_index++)
                {
                    g_gosub_rows[row_count].stats[stat_index] = *(u16*)(g_pad_ctx + record_offset + 0x2F14 + stat_index * 2);
                }
                record_offset_reload = slot * 0x60;
                g_gosub_rows[row_count].secondary_value = *(u16*)(g_pad_ctx + record_offset_reload + 0x2F10);
                row_count++;
            }
        }
    }
    g_gosub_row_count = row_count;
    g_gosub_visible_row_count = 3;
    g_gosub_row_height = 0x30;
    g_gosub_window_width = 0x120;
    g_gosub_window_height = 0x94;
    g_gosub_title_text = GOSUB_MSG_PTR(mode * 2 + 0x2C);
}

/**
 * @brief Confirm the currently highlighted row of the gosub list.
 * @return 0 if the row was rejected by a flag, 1 otherwise. Note that 1 is also
 *         returned when g_gosub_selection_count is clear and nothing was appended.
 * @see decomp.me (100%)
 */
s32 gosub_select_row_with_validation(void)
{
    s32 row;
    GosubListRow* list;
    GosubListRow* row_entry;

    list = g_gosub_rows;
    row = g_gosub_cursor_row;
    row_entry = &list[row];
    if (row_entry->flags.half & 1)
    {
        GOSUB_MSG(0x42);
        g_gosub_selection_count = 0;
        g_gosub_suppress_dialog_sound = 1;
        return 0;
    }
    if (row_entry->flags.f.selection_restricted)
    {
        GOSUB_MSG(0x50);
        g_gosub_selection_count = 0;
        g_gosub_suppress_dialog_sound = 1;
        return 0;
    }
    if (g_gosub_selection_count != 0)
    {
        g_gosub_result_values[g_gosub_result_count] = row_entry->index;
        g_gosub_result_rows[g_gosub_result_count] = row;
        g_gosub_result_count++;
    }
    return 1;
}

/**
 * @brief Append the highlighted row to the selection, with no flag checks.
 * @return Always 1. Nothing is appended while g_gosub_selection_count is clear.
 * @see decomp.me (100%)
 */
s32 gosub_select_row(void)
{
    s32 n;

    if (g_gosub_selection_count != 0)
    {
        n = g_gosub_result_count;
        g_gosub_result_count = n + 1;
        g_gosub_result_values[n] = g_gosub_rows[g_gosub_cursor_row].index;
        g_gosub_result_rows[n] = g_gosub_cursor_row;
    }
    return 1;
}

/**
 * @brief Validate a pending two-row selection before publishing it.
 * @return gosub_publish_two_row_selection's result while g_gosub_combination_result_id is set, 0 on every other path.
 * @see decomp.me (100%)
 */
s32 gosub_validate_pending_pair_selection(void)
{
    if (g_gosub_selection_count != 0)
    {
        if (g_gosub_combination_result_id == 0)
        {
            if (g_gosub_selection_count == 2)
            {
                g_gosub_selection_count = 1;
            }
            return 0;
        }
        return gosub_publish_two_row_selection();
    }
    return 0;
}

/**
 * @brief Commit a pending row move by swapping the two marked rows.
 * @return Always 0.
 * @see decomp.me (100%)
 */
s32 gosub_commit_row_reorder(void)
{
    GosubListRow entry_tmp;
    u32 rec_tmp;
    s32 saved_index;

    if (g_gosub_selection_count == 0)
    {
        return 0;
    }
    if (g_gosub_selection_count != 2)
    {
        return 0;
    }
    if (g_gosub_selected_rows[0] != g_gosub_selected_rows[1])
    {
        gosub_copy_packed_record(&rec_tmp, g_pad_ctx + (g_gosub_rows[g_gosub_selected_rows[0]].index * 4 + 0x29DC));
        gosub_copy_packed_record(g_pad_ctx + (g_gosub_rows[g_gosub_selected_rows[0]].index * 4 + 0x29DC),
                      g_pad_ctx + (g_gosub_rows[g_gosub_selected_rows[1]].index * 4 + 0x29DC));
        gosub_copy_packed_record(g_pad_ctx + (g_gosub_rows[g_gosub_selected_rows[1]].index * 4 + 0x29DC), &rec_tmp);
        gosub_copy_list_row(&entry_tmp, &g_gosub_rows[g_gosub_selected_rows[0]]);
        gosub_copy_list_row(&g_gosub_rows[g_gosub_selected_rows[0]], &g_gosub_rows[g_gosub_selected_rows[1]]);
        gosub_copy_list_row(&g_gosub_rows[g_gosub_selected_rows[1]], &entry_tmp);
        saved_index = g_gosub_rows[g_gosub_selected_rows[0]].index;
        g_gosub_rows[g_gosub_selected_rows[0]].index = g_gosub_rows[g_gosub_selected_rows[1]].index;
        g_gosub_rows[g_gosub_selected_rows[1]].index = saved_index;
        g_gosub_selection_count = 0;
    }
    else
    {
        gosub_open_row_action_dialog();
        g_gosub_selection_count = 1;
    }
    return 0;
}

/**
 * @brief Update row colors for the current group selection.
 * @return 1 after publishing a complete mixed-group selection, otherwise 0.
 * @see decomp.me (100%)
 */
s32 gosub_update_group_selection(void)
{
    s32 i;
    s32 open_count;
    s32 marked_count;

    for (i = 0; i < g_gosub_row_count; i++)
    {
        g_gosub_rows[i].text_color = 4;
    }
    open_count = 0;
    marked_count = 0;
    for (i = 0; i < g_gosub_selection_count; i++)
    {
        if (g_gosub_rows[g_gosub_selected_rows[i]].equipment_kind == 0)
        {
            open_count++;
        }
        else
        {
            marked_count++;
        }
    }
    if (open_count != 0)
    {
        for (i = 0; i < g_gosub_row_count; i++)
        {
            if (g_gosub_rows[i].equipment_kind == 0 && gosub_is_row_unselected(i) != 0)
            {
                g_gosub_rows[i].text_color = 5;
            }
        }
    }
    if (marked_count == 3)
    {
        for (i = 0; i < g_gosub_row_count; i++)
        {
            if (g_gosub_rows[i].equipment_kind != 0 && gosub_is_row_unselected(i) != 0)
            {
                g_gosub_rows[i].text_color = 5;
            }
        }
    }
    if (open_count != 0)
    {
        if (marked_count == 3)
        {
            gosub_publish_selection();
            return 1;
        }
        return 0;
    }
    return 0;
}

/**
 * @brief Publish the picked rows' indices as the screen's result.
 * @return 1 if the result was published, 0 if the picker was not in state 2.
 * @see decomp.me (100%)
 */
s32 gosub_publish_two_row_selection(void)
{
    s32 i;

    if (g_gosub_selection_count == 2)
    {
        g_gosub_result_count = g_gosub_selection_count;
        for (i = 0; i < g_gosub_selection_count; i++)
        {
            g_gosub_result_values[i] = g_gosub_rows[g_gosub_selected_rows[i]].index;
        }
        return 1;
    }
    return 0;
}

/**
 * @brief Handle the confirmation dialog for creating a two-item combination.
 *
 * @param dialog_result Zero to confirm; nonzero to return to the selection.
 * @return 1 if confirming leaves no equipment rows, otherwise 0.
 * @see decomp.me (100%) https://decomp.me/scratch/2OzmD
 */
s32 gosub_handle_combination_dialog(s32 dialog_result)
{
    s32 combination_count;
    GosubPackedRecord* record;
    u32 packed;
    s32 result_id;
    s32 packed_result;
    s32 secondary_value;
    s32 clear_config_mask;
    u16 stored_word;

    if (dialog_result == 0 && (g_gosub_dialog_choice & 1) == 0)
    {
        clear_config_mask = ~0xFC;
        combination_count = *(g_pad_ctx + 0x29D6);
        if (combination_count < 0x28)
        {
            record = (GosubPackedRecord*)(g_pad_ctx + combination_count * 4 + 0x29DC);
            result_id = g_gosub_combination_result_id;
            packed = record->word & clear_config_mask;
            packed = packed | ((result_id & 0x3F) << 2);
            record->word = packed;
            secondary_value = g_gosub_combination_quantity;
            dialog_result = (packed & ~0xF00) | ((secondary_value & 0xF) << 8);
            record->word = dialog_result;
            packed_result = ((dialog_result & 0xFFFF0FFF) | ((g_gosub_combination_variant & 0xF) << 12) | 3) & 0xFFFF;
            stored_word = packed_result;
            record->word = stored_word;
            *(g_pad_ctx + 0x29D6) = *(g_pad_ctx + 0x29D6) + 1;
            GOSUB_EQUIPMENT_AT_SHIFTED_INDEX(g_gosub_result_values[0])->name[0] = 0;
            GOSUB_EQUIPMENT_AT_SHIFTED_INDEX(g_gosub_result_values[1])->name[0] = 0;
            func_800A8FB4();
        }
        if (*(g_pad_ctx + 0x29D6) >= 0x28)
        {
            gosub_start_element_exit();
            g_field_gosub_state = 0;
            GOSUB_MSG(-4);
            return 0;
        }
        g_gosub_scroll_frames_remaining = 0;
        g_gosub_scroll_target_y = 0;
        g_gosub_scroll_y = 0;
        g_gosub_cursor_row = 0;
        g_gosub_allow_duplicate_selection = 0;
        gosub_build_equipment_list(3);
        g_gosub_visible_row_count = 6;
        g_gosub_row_height = 0x10;
        g_gosub_window_width = 0xE8;
        g_gosub_window_height = 0x64;
        g_gosub_combination_variant = 0;
        g_gosub_combination_result_id = 0;
        g_gosub_combination_quantity = 0;
        g_gosub_required_selection_count = 2;
        g_gosub_selection_mode = 2;
        g_gosub_selection_count = 0;
        g_gosub_elements[0].attr.f.state = GOSUB_ELEMENT_STATE_INACTIVE;
        g_gosub_screen_sequence_index -= 1;
        if (g_gosub_row_count == 0)
        {
            g_field_gosub_state = 0;
            func_80067F28();
            gosub_start_element_exit();
            return 1;
        }
        return 0;
    }
    g_gosub_selection_count -= 1;
    g_gosub_screen_sequence_index -= 1;
    g_gosub_elements[0].attr.f.state = GOSUB_ELEMENT_STATE_INACTIVE;
    return 0;
}

/**
 * @brief Publish the selected group rows as result values.
 *
 * @return 1 when at least one row was published, otherwise 0.
 * @see decomp.me (100%) https://decomp.me/scratch/pOY6i
 */
s32 gosub_publish_group_selection(void)
{
    s32 i;

    if (g_gosub_selection_count == 0)
    {
        return 0;
    }

    g_gosub_result_count = g_gosub_selection_count;

    for (i = 0; i < g_gosub_selection_count; i++)
    {
        g_gosub_result_values[i] = g_gosub_rows[g_gosub_selected_rows[i]].index;
    }

    return 1;
}

/**
 * @brief Publish the selected rows' entry indices as result values.
 *
 * @return 1 when at least one row was published, otherwise 0.
 * @see decomp.me (100%) https://decomp.me/scratch/FN7DQ
 */
s32 gosub_publish_selection(void)
{
    s32 i;

    if (g_gosub_selection_count == 0)
    {
        return 0;
    }

    g_gosub_result_count = g_gosub_selection_count;

    for (i = 0; i < g_gosub_selection_count; i++)
    {
        g_gosub_result_values[i] = g_gosub_rows[g_gosub_selected_rows[i]].index;
    }

    return 1;
}

/**
 * @brief Test whether a row is absent from the current selection.
 *
 * @param row Row index to test.
 * @return 1 if the row is unselected, otherwise 0.
 * @see decomp.me (100%) https://decomp.me/scratch/lBIH9
 */
s32 gosub_is_row_unselected(s32 row)
{
    s32 i;
    s32 count = g_gosub_selection_count;

    for (i = 0; i < count; i++)
    {
        if (g_gosub_selected_rows[i] == row)
        {
            return 0;
        }
    }

    return 1;
}

/**
 * @brief Build list rows from nonempty equipment records of the requested kind.
 *
 * Kind 3 accepts every record; kind 4 accepts every record except kind 2.
 *
 * @param item_kind Equipment kind filter, or 3/4 for the aggregate filters.
 * @see decomp.me (100%) https://decomp.me/scratch/CJYqj
 */
void gosub_build_equipment_list(u32 item_kind)
{
    s32 item_index;
    s32 stat_index;
    s32 row_count;
    u8* item_base;
    u8* entry;
    u8* item;
    s32 separator_offset;
    GosubEquipmentRecord* record;
    u32 attributes;

    g_gosub_show_row_details = 1;
    row_count = 0;

    for (item_index = 0; item_index < 100; item_index++)
    {
        entry = GOSUB_EQUIPMENT_BASE_FROM_INDEX(item_index);
        if (GOSUB_EQUIPMENT_RECORD(entry)->name[0] != 0)
        {
            if ((item_kind == 3) || (GOSUB_EQUIPMENT_KIND(GOSUB_EQUIPMENT_RECORD(entry)->attributes.word) == item_kind) ||
                ((item_kind == 4) && (GOSUB_EQUIPMENT_KIND(GOSUB_EQUIPMENT_RECORD(entry)->attributes.word) != 2)))
            {

                g_gosub_rows[row_count].name = GOSUB_EQUIPMENT_AT(item_index)->name;

                gosub_copy_encoded_string(GOSUB_TEXT_BUFFER(row_count),
                              ARCHIVE_ENTRY(g_gosub_text_archive_offsets_1[0], GOSUB_EQUIPMENT_AT_SHIFTED_INDEX(item_index)->attributes.half.material & 0x3F));
                separator_offset = (s32)(D_800EC3E2 - 0x1E) + (D_800EC3E2[1] << 8);
                gosub_append_encoded_string(GOSUB_TEXT_BUFFER(row_count), D_800EC3E2[0] + separator_offset);

                item_base = GOSUB_EQUIPMENT_BASE_FROM_INDEX(item_index);
                g_gosub_rows[row_count].equipment_kind = GOSUB_EQUIPMENT_KIND(GOSUB_EQUIPMENT_RECORD(item_base)->attributes.word);
                attributes = GOSUB_EQUIPMENT_RECORD(item_base)->attributes.word;

                switch (GOSUB_EQUIPMENT_KIND(attributes))
                {
                case 0:
                    gosub_append_encoded_string(GOSUB_TEXT_BUFFER(row_count), ARCHIVE_ENTRY(GOSUB_TEXT_ARCHIVE->block_offsets[3], GOSUB_EQUIPMENT_CATEGORY(attributes)));
                    g_gosub_rows[row_count].primary_value = GOSUB_EQUIPMENT_AT(item_index)->data.kind0_value;
                    break;
                case 1:
                    gosub_append_encoded_string(GOSUB_TEXT_BUFFER(row_count),
                                  ARCHIVE_ENTRY(GOSUB_TEXT_ARCHIVE->block_offsets[3], GOSUB_EQUIPMENT_CATEGORY(attributes) + 0xB));
                    record = GOSUB_EQUIPMENT_FROM_INDEX(item_index);
                    for (stat_index = 0; stat_index < 4; stat_index++)
                    {
                        g_gosub_rows[row_count].stats[stat_index] = record->data.kind1_stats[stat_index];
                    }
                    break;
                default:
                    item = GOSUB_EQUIPMENT_SOURCE_FROM_INDEX(item_index);
                    record = GOSUB_EQUIPMENT_RECORD(item);
                    g_gosub_rows[row_count].primary_value = record->data.kind2.value;
                    g_gosub_rows[row_count].stats[0] = record->data.kind2.index + (record->data.kind2.group * 14);
                    gosub_append_encoded_string(GOSUB_TEXT_BUFFER(row_count), GOSUB_KIND2_ARCHIVE_ENTRY(GOSUB_EQUIPMENT_RECORD(item)->attributes.word));
                    break;
                }

                g_gosub_rows[row_count].desc = GOSUB_TEXT_BUFFER(row_count);
                g_gosub_rows[row_count].value = -1;
                g_gosub_rows[row_count].index = item_index;
                g_gosub_rows[row_count].text_color = 4;
                row_count++;
            }
        }
    }

    g_gosub_row_count = row_count;
    g_gosub_visible_row_count = 8;

    switch (item_kind)
    {
    case 0:
        g_gosub_title_text = GOSUB_MSG_PTR(0xC);
        break;
    case 1:
        g_gosub_title_text = GOSUB_MSG_PTR(0xE);
        break;
    case 2:
        g_gosub_title_text = GOSUB_MSG_PTR(0x10);
        break;
    case 3:
        g_gosub_title_text = GOSUB_MSG_PTR(0x16);
        break;
    case 4:
        g_gosub_visible_row_count = 7;
        g_gosub_title_text = GOSUB_MSG_PTR(0x16);
        break;
    }

    g_gosub_row_height = 0x10;
    g_gosub_window_width = 0xE8;
    g_gosub_window_height = (g_gosub_visible_row_count * 0x10) + 4;
}
/**
 * @brief Build one of the three grouped option lists from the text archive.
 *
 * @param group Option group index, from 0 through 2.
 * @see decomp.me (100%)
 */
void gosub_build_grouped_option_list(s32 group)
{
    s32 option_index;
    GosubGroupTable first_indices = g_gosub_group_first_indices;
    GosubGroupTable counts = g_gosub_group_counts;

    for (option_index = 0; option_index < counts.values[group]; option_index++)
    {
        GosubListRow* row = &g_gosub_rows[option_index];
        u8* text;

        row->name = ARCHIVE_ENTRY(g_gosub_text_archive_offsets_2[0], option_index + first_indices.values[group]);
        text = ARCHIVE_ENTRY(g_gosub_text_archive_offsets_2[0], option_index + first_indices.values[group]);
        row->name = text;
        row->value = -1;
        row->index = option_index;
        row->equipment_kind = 0;
        row->desc = text;
        row->text_color = 4;
    }

    g_gosub_row_count = counts.values[group];

    switch (group)
    {
    case 0:
        g_gosub_title_text = GOSUB_MSG_PTR(0x1A);
        break;
    case 1:
        g_gosub_title_text = GOSUB_MSG_PTR(0x1C);
        break;
    case 2:
        g_gosub_title_text = GOSUB_MSG_PTR(0x1E);
        break;
    }

    g_gosub_visible_row_count = 8;
    g_gosub_row_height = 0x10;
    g_gosub_window_width = 0xE8;
    g_gosub_window_height = 0x84;
}

/**
 * @brief Process input, advance scroll interpolation, and draw the active screen.
 *
 * @param render_ctx Rendering context forwarded to the input and draw handlers.
 * @see decomp.me (100%)
 */
void gosub_update_screen(s32 render_ctx)
{
    gosub_handle_input(render_ctx);

    if (g_gosub_finished == 0)
    {
        if (g_gosub_scroll_frames_remaining != 0)
        {
            g_gosub_scroll_y += (g_gosub_scroll_target_y - g_gosub_scroll_y) / g_gosub_scroll_frames_remaining;
            g_gosub_scroll_frames_remaining -= 1;
        }
        else
        {
            g_gosub_scroll_y = g_gosub_scroll_target_y;
        }

        gosub_render_elements(render_ctx);
    }
}

/**
 * @brief Handle dialog, navigation, selection, completion, and cancellation input.
 *
 * @param unused Unused rendering context.
 * @return Undefined; callers ignore the value.
 * @see decomp.me (100%)
 */
s32 gosub_handle_input(s32 unused)
{
    GosubElement* elements;
    s32 steps_remaining;
    s32 restored_row;
    s32 max_scroll_y;

    elements = g_gosub_elements;
    if ((elements[1].attr.word & 7) == 0 && (elements[0].attr.word & 7) == 0)
    {
        g_gosub_finished = 1;
        return;
    }

    if (gosub_are_elements_idle() == 0)
    {
        return;
    }

    if ((g_gosub_elements[0].attr.word & 7) == 2)
    {
        if (g_gosub_dialog_accepting_input != 0)
        {
            if ((g_pad_input & 0x260) == 0)
            {
                return;
            }
            if (g_gosub_suppress_dialog_sound == 0)
            {
                func_800A3938(0x7D, 0x80);
                func_80067F28();
                gosub_start_element_exit();
                return;
            }
            g_gosub_elements[0].attr.f.state = GOSUB_ELEMENT_STATE_INACTIVE;
            g_gosub_dialog_accepting_input = 0;
            return;
        }

        if (g_pad_input & 0x9000)
        {
            func_800A3938(0x7D, 0x80);
            g_gosub_dialog_choice -= 1;
            if (g_gosub_dialog_choice < 0)
            {
                g_gosub_dialog_choice = 0xB;
            }
            return;
        }

        if (g_pad_input & 0x6000)
        {
            func_800A3938(0x7D, 0x80);
            g_gosub_dialog_choice += 1;
            if (g_gosub_dialog_choice == 0xC)
            {
                g_gosub_dialog_choice = 0;
            }
            return;
        }

        if (g_pad_input & 0x220)
        {
            func_800A3938(0x7D, 0x80);
            if (g_gosub_dialog_handler == 0)
            {
                return;
            }
            if (g_gosub_dialog_handler(0) == 0)
            {
                return;
            }
            func_80067F28();
            gosub_start_element_exit();
            return;
        }

        if (g_pad_input & 0x40)
        {
            func_800A3938(0x7D, 0x80);
            if (g_gosub_dialog_handler == 0)
            {
                return;
            }
            if (g_gosub_dialog_handler(1) == 0)
            {
                return;
            }
            func_80067F28();
            gosub_start_element_exit();
            return;
        }

        return;
    }

    if (g_gosub_scroll_frames_remaining != 0)
    {
        return;
    }

    steps_remaining = 1;
    if (g_pad_input & 8)
    {
        steps_remaining = g_gosub_visible_row_count;
        g_pad_input = 0x4000;
    }
    if (g_pad_input & 4)
    {
        steps_remaining = g_gosub_visible_row_count;
        g_pad_input = 0x1000;
    }

    if (steps_remaining != 0)
    {
        do
        {
            if (g_pad_input & 0x1000)
            {
                g_gosub_cursor_row -= 1;
                if (g_gosub_cursor_row == 0)
                {
                    steps_remaining = 1;
                }
                if (g_gosub_cursor_row < 0)
                {
                    g_gosub_cursor_row = g_gosub_row_count - 1;
                    steps_remaining = 1;
                }
            }
            if (g_pad_input & 0x4000)
            {
                g_gosub_cursor_row += 1;
                if (g_gosub_cursor_row == g_gosub_row_count - 1)
                {
                    steps_remaining = 1;
                }
                if (g_gosub_cursor_row >= g_gosub_row_count)
                {
                    g_gosub_cursor_row = 0;
                    steps_remaining = 1;
                }
            }
            steps_remaining -= 1;
        } while (steps_remaining != 0);
    }

    if (g_pad_input & 0x5000)
    {
        func_800A3938(0x7D, 0x80);
        gosub_scroll_to_cursor();
        return;
    }

    if (g_pad_input & 0x220)
    {
        func_800A3938(0x7D, 0x80);
        if ((g_gosub_rows[g_gosub_cursor_row].text_color & 0xF) != 4)
        {
            return;
        }
        if (gosub_toggle_cursor_selection() != 0)
        {
            g_gosub_selected_rows[g_gosub_selection_count] = g_gosub_cursor_row;
            g_gosub_selection_count += 1;
            if (g_gosub_select_handler != 0)
            {
                if (g_gosub_select_handler() == 0)
                {
                    return;
                }
                if (gosub_advance_screen_sequence() == 0)
                {
                    return;
                }
                func_80067F28();
                gosub_start_element_exit();
                return;
            }
            if (g_gosub_selection_count != g_gosub_required_selection_count)
            {
                return;
            }
            if (g_gosub_finish_handler == 0)
            {
                return;
            }
            if (g_gosub_finish_handler() == 0)
            {
                return;
            }
            if (gosub_advance_screen_sequence() == 0)
            {
                return;
            }
            func_80067F28();
            gosub_start_element_exit();
            return;
        }
        if (g_gosub_select_handler == 0)
        {
            return;
        }
        if (g_gosub_select_handler() == 0)
        {
            return;
        }
        if (gosub_advance_screen_sequence() == 0)
        {
            return;
        }
        func_80067F28();
        gosub_start_element_exit();
        return;
    }

    if (g_pad_input & 0x800)
    {
        func_800A3938(0x7D, 0x80);
        if (g_gosub_finish_handler != 0)
        {
            if (g_gosub_finish_handler() == 0)
            {
                return;
            }
            if (gosub_advance_screen_sequence() == 0)
            {
                return;
            }
            func_80067F28();
            gosub_start_element_exit();
            return;
        }
        if (gosub_advance_screen_sequence() == 0)
        {
            return;
        }
        func_80067F28();
        gosub_start_element_exit();
        return;
    }

    if ((g_pad_input & 0x40) == 0)
    {
        return;
    }

    func_800A3938(0x7F, 0x80);

    if (g_gosub_selection_count != 0)
    {
        g_gosub_selection_count -= 1;
        g_gosub_cursor_row = g_gosub_selected_rows[g_gosub_selection_count];
        gosub_scroll_to_cursor();
        if (g_gosub_select_handler != 0)
        {
            g_gosub_select_handler();
        }
        return;
    }

    if (g_gosub_screen_sequence_index != 0)
    {
        g_gosub_screen_sequence_index -= 1;
        gosub_enter_screen(g_gosub_screen_sequence[g_gosub_screen_sequence_index]);
        g_gosub_result_count -= 1;
        restored_row = g_gosub_result_rows[g_gosub_result_count];
        g_gosub_cursor_row = restored_row;
        g_gosub_scroll_y = g_gosub_row_height * restored_row;
        max_scroll_y = (g_gosub_row_count * g_gosub_row_height) - g_gosub_window_height + 4;
        if (max_scroll_y < g_gosub_scroll_y)
        {
            g_gosub_scroll_y = max_scroll_y;
        }
        if (g_gosub_scroll_y < 0)
        {
            g_gosub_scroll_y = 0;
        }
        g_gosub_scroll_frames_remaining = 0;
        g_gosub_scroll_target_y = g_gosub_scroll_y;
        return;
    }

    g_gosub_result_count = 0;
    func_80067F28();
    gosub_start_element_exit();
}

/**
 * @brief Scroll the list viewport toward the cursor when it leaves view.
 *
 * @see decomp.me (100%)
 */
void gosub_scroll_to_cursor(void)
{
    s32 cursor_offset;
    s32 cursor_y;

    cursor_y = g_gosub_cursor_row * g_gosub_row_height;
    cursor_offset = cursor_y - g_gosub_scroll_y;

    if ((g_gosub_window_height - g_gosub_row_height) < cursor_offset)
    {
        g_gosub_scroll_frames_remaining = 4;
        g_gosub_scroll_target_y = (g_gosub_cursor_row - (g_gosub_visible_row_count - 1)) * g_gosub_row_height;
    }

    if (cursor_offset < 0)
    {
        g_gosub_scroll_target_y = cursor_y;
        g_gosub_scroll_frames_remaining = 4;
    }
}

/**
 * @brief Remove the cursor row if selected, or permit the caller to add it.
 *
 * @return 0 if the row was removed, otherwise 1.
 * @see decomp.me (100%)
 */
s32 gosub_toggle_cursor_selection(void)
{
    s32 selection_index;
    s32 shift_index;

    if (g_gosub_allow_duplicate_selection != 0)
    {
        return 1;
    }

    for (selection_index = 0; selection_index < g_gosub_selection_count; selection_index++)
    {
        if (g_gosub_selected_rows[selection_index] == g_gosub_cursor_row)
        {
            for (shift_index = selection_index; shift_index < 3; shift_index++)
            {
                g_gosub_selected_rows[shift_index] = g_gosub_selected_rows[shift_index + 1];
            }
            g_gosub_selection_count -= 1;
            return 0;
        }
    }

    return 1;
}

/**
 * @brief Advance to the next screen or open the sequence's final dialog.
 *
 * @return 1 at the sequence terminator, otherwise 0.
 * @see decomp.me (100%)
 */
s32 gosub_advance_screen_sequence(void)
{
    GosubElement* element;

    g_gosub_screen_sequence_index += 1;

    if (g_gosub_screen_sequence[g_gosub_screen_sequence_index] == GOSUB_SCREEN_SEQUENCE_END)
    {
        return 1;
    }

    if (g_gosub_screen_sequence[g_gosub_screen_sequence_index] == GOSUB_SCREEN_SEQUENCE_DIALOG)
    {
        element = &g_gosub_elements[0];
        element->draw_handler = (void*)&gosub_draw_confirmation_prompt;
        g_gosub_dialog_choice = 0;
        element->attr.f.state = GOSUB_ELEMENT_STATE_ENTERING;
        element->attr.f.transition_step = 1;
        element->attr.f.x = 0x20;
        element->attr.f.width_low = 0x70;
        element->width_high = 1;
        element->y = 0x24;
        SET_ELEMENT_WIDTH_LOW(element, 0);
    }
    else
    {
        gosub_enter_screen(g_gosub_screen_sequence[g_gosub_screen_sequence_index]);
    }

    return 0;
}

/**
 * @brief Test whether all fixed elements have finished transitioning.
 *
 * @return 1 when all elements are idle, otherwise 0.
 * @see decomp.me (100%)
 */
s32 gosub_are_elements_idle(void)
{
    GosubElement* element;
    s32 i;

    element = g_gosub_elements;

    for (i = 0; i < GOSUB_ELEMENT_COUNT; i++)
    {
        if (element->attr.f.state == GOSUB_ELEMENT_STATE_ENTERING || element->attr.f.state == GOSUB_ELEMENT_STATE_EXITING)
        {
            return 0;
        }
        element++;
    }

    return 1;
}

/**
 * @brief Start the exit transition for every allocated element.
 * @see decomp.me (100%) https://decomp.me/scratch/RsBVl
 */
void gosub_start_element_exit(void)
{
    s32 element_word;
    s32 element_index;
    s32* element_words;
    s32 state_word;

    element_words = &g_gosub_elements;
    element_index = 0;
    do
    {
        element_word = *element_words;
        if (element_word & 7)
        {
            state_word = element_word & ~7;
            *element_words = (state_word & ~0x78) | 0x40;
        }
        element_index += 1;
        element_words += 3;
    } while (element_index < 0x10);
}

/**
 * @brief Render and animate all allocated gosub elements.
 * @see decomp.me (100%) https://decomp.me/scratch/nVefu
 */
void gosub_render_elements(void)
{
    gosub_update_and_render_elements();
}

/**
 * @brief Mark every gosub element slot inactive.
 * @see decomp.me (100%) https://decomp.me/scratch/dib6Q
 */
void gosub_clear_elements(void)
{
    s32 element_index;
    s32* element_words;

    element_words = &g_gosub_elements;
    element_index = 0;
    do
    {
        element_index += 1;
        *element_words &= ~7;
        element_words += 3;
    } while (element_index < 0x10);
}

/**
 * @brief Allocate the first inactive dynamic element slot.
 * @return Allocated element, or element 0 when the pool is full.
 * @see decomp.me (100%) https://decomp.me/scratch/X1pXK
 */
GosubElement* gosub_allocate_element(void)
{
    s32 element_word;
    s32 element_index;
    s32* element_words;

    element_words = (s32*)g_gosub_dynamic_elements;

    for (element_index = 1; element_index < 0x10; element_index++, element_words += 3)
    {
        element_word = *element_words;
        if (!(element_word & 7))
        {
            *element_words = (element_word & ~7) | 1;
            return element_words;
        }
    }

    return &g_gosub_elements;
}

/**
 * @brief Animate, draw, frame, and link every allocated gosub element.
 * @param render_context Field render context and packet cursor.
 * @see decomp.me (100%) https://decomp.me/scratch/t79hi
 */
void gosub_update_and_render_elements(GosubRenderContext* render_context)
{
    GosubGpuPacket* packet_cursor;
    s32 animated_width;
    s32 animated_height;
    GosubRenderContext* ordering_table;
    u32* element_words;
    u32 address_mask;
    u32 tag_mask;
    s32 element_index;
    s32 draw_env[24];
    u32 geometry_word;
    s32 element_height;
    u16 visible_height;
    u32 geometry;
    u32 element_width;
    u32 element_word;
    s32 element_height_calc;
    s32 packet_address;
    s32 content_height;
    GosubGpuPacket* draw_cursor;
    GosubGpuPacket* unused_packet;
    u32 marker_word;
    u32 panel_word;
    u32 state_word;
    s32 working_word;
    s32 exit_transition_step;
    s32 exit_scaled_width;
    s32 exit_full_height;
    s32 exit_scaled_height;
    s32 exit_remaining_height;
    u32 entering_word;
    u32 updated_entering_word;
    s32 transition_step;
    s32 scaled_width;
    s32 full_height;
    s32 scaled_height;
    s32 remaining_height;
    u32 exiting_word;
    u32 updated_exiting_word;

    packet_cursor = render_context->packet_cursor;
    ordering_table = render_context;

    if (render_context->display_buffer_index != 0)
    {
        SetDefDrawEnv((DRAWENV*)draw_env, 0, 0xF0, 0x140, 0xE0);
    }
    else
    {
        SetDefDrawEnv((DRAWENV*)draw_env, 0, 8, 0x140, 0xE0);
    }

    element_words = &g_gosub_elements;
    element_index = 0;
    address_mask = 0x00FFFFFF;
    tag_mask = 0xFF000000;

    for (; element_index < 0x10; element_index++)
    {
        element_word = *element_words;
        if (element_word & 7)
        {
            draw_cursor = packet_cursor;

            if (*(GosubElementDrawHandler*)((u8*)element_words + 8) == (GosubElementDrawHandler)gosub_draw_item_list)
            {
                geometry_word = *(u32*)((u8*)element_words + 4);
                element_height = (geometry_word >> 1) & 0xFF;

                content_height = g_gosub_row_count * g_gosub_row_height;
                if ((g_gosub_scroll_y + element_height) < content_height)
                {
                    {
                        u32 field;
                        u32 high;
                        field = (element_word >> 7) & 0x1FF;
                        high = element_word >> 24;
                        packet_cursor = gosub_emit_scroll_marker(draw_cursor, ordering_table, (field + (((geometry_word & 1) << 8) | high)) - 0x10, (*((u8*)element_words + 2)) + element_height, 0);
                    }
                }
                if (g_gosub_scroll_y != 0)
                {
                    {
                        u32 field;
                        u32 high;
                        marker_word = *element_words;
                        field = (marker_word >> 7) & 0x1FF;
                        high = marker_word >> 24;
                        packet_cursor = gosub_emit_scroll_marker(packet_cursor, ordering_table, (field + (((*(u32*)((u8*)element_words + 4) & 1) << 8) | high)) - 0x10, (*((u8*)element_words + 2)), 1);
                    }
                }
                SetDrawEnv((DR_ENV*)packet_cursor, (DRAWENV*)draw_env);

                packet_cursor->tag = (packet_cursor->tag & tag_mask) | (ordering_table->tag & address_mask);
                ordering_table->tag = (s32)((ordering_table->tag & tag_mask) | ((s32)packet_cursor & address_mask));

                packet_cursor = (GosubGpuPacket*)((u8*)packet_cursor + 0x40);

                if (g_gosub_row_count != 0)
                {
                    packet_cursor->color = 0xFFFF00;
                    ((u8*)packet_cursor)[3] = 3;
                    ((u8*)packet_cursor)[7] = 0x60;
                    packet_cursor->w = 6;
                    element_height_calc = (*(u32*)((u8*)element_words + 4) >> 1) & 0xFF;
                    packet_cursor->h = (u16)((s32)(element_height_calc * (element_height_calc / g_gosub_row_height)) / g_gosub_row_count);
                    {
                        s32 clamp_h;
                        clamp_h = (s16)packet_cursor->h;
                        working_word = *(u32*)((u8*)element_words + 4);
                        visible_height = ((u32)working_word >> 1) & 0xFF;
                        if (clamp_h >= (s32)visible_height - 2)
                        {
                            packet_cursor->h = visible_height;
                        }
                    }
                    packet_cursor->x = 1;
                    packet_cursor->y = (s16)((s32)(((*(u32*)((u8*)element_words + 4) >> 1) & 0xFF) * (g_gosub_scroll_y / g_gosub_row_height)) / g_gosub_row_count);
                    packet_cursor->tag = (packet_cursor->tag & tag_mask) | (ordering_table->tag & address_mask);

                    packet_address = (s32)packet_cursor & address_mask;
                    packet_cursor = (GosubGpuPacket*)((u8*)packet_cursor + 0x10);
                    ordering_table->tag = (s32)((ordering_table->tag & tag_mask) | packet_address);
                }
                {
                    u32 field;
                    u32 high;
                    panel_word = *element_words;
                    field = (panel_word >> 7) & 0x1FF;
                    high = panel_word >> 24;
                    packet_cursor = gosub_emit_panel(packet_cursor, ordering_table, field + (((*(u32*)((u8*)element_words + 4) & 1) << 8) | high) + 3, (*((u8*)element_words + 2)), 0xA,
                                           (*(u32*)((u8*)element_words + 4) >> 1) & 0xFF, render_context->display_buffer_index);
                }
                draw_cursor = packet_cursor;
            }
            SetDrawEnv((DR_ENV*)draw_cursor, (DRAWENV*)draw_env);
            packet_cursor->tag = (packet_cursor->tag & tag_mask) | (ordering_table->tag & address_mask);
            ordering_table->tag = (s32)((ordering_table->tag & tag_mask) | ((s32)packet_cursor & address_mask));

            state_word = *element_words;
            working_word = state_word & 7;

            packet_cursor = (GosubGpuPacket*)((u8*)packet_cursor + 0x40);

            switch (working_word)
            {
            case 1:
                geometry = *(u32*)((u8*)element_words + 4);
                element_width = ((geometry & 1) << 8) | (state_word >> 24);
                exit_transition_step = (state_word >> 3) & 0xF;
                exit_scaled_width = element_width * exit_transition_step;
                if (exit_scaled_width < 0)
                {
                    exit_scaled_width += 7;
                }
                exit_full_height = (geometry >> 1) & 0xFF;
                exit_scaled_height = exit_full_height * exit_transition_step;
                animated_width = exit_scaled_width >> 3;
                if (exit_scaled_height < 0)
                {
                    exit_scaled_height += 7;
                }
                animated_height = exit_scaled_height >> 3;
                exit_remaining_height = (s32)(exit_full_height - animated_height);

                packet_cursor = (*(GosubElementDrawHandler*)((u8*)element_words + 8))(ordering_table, packet_cursor, (s32)(element_width - animated_width) / 2, exit_remaining_height / 2);
                {
                    u32 post_word;
                    u32 field;
                    u32 high;
                    post_word = *element_words;
                    field = (post_word >> 7) & 0x1FF;
                    high = post_word >> 24;
                    packet_cursor =
                        gosub_emit_panel(packet_cursor, ordering_table, field + (s32)((((*(u32*)((u8*)element_words + 4) & 1) << 8) | high) - animated_width) / 2,
                                      (*((u8*)element_words + 2)) + ((s32)((*(u32*)((u8*)element_words + 4) >> 1) & 0xFF) - animated_height) / 2, animated_width, animated_height, render_context->display_buffer_index);
                }
                entering_word = *element_words;
                updated_entering_word = entering_word & ~0x78;
                updated_entering_word |= (((((entering_word >> 3) & 0xF) + 1) & 0xF) * 8);
                *element_words = updated_entering_word;
                if (((updated_entering_word >> 3) & 0xF) == 8)
                {
                    *element_words = (updated_entering_word & ~7) | 2;
                }
                break;

            case 2:
                packet_cursor = (*(GosubElementDrawHandler*)((u8*)element_words + 8))(ordering_table, packet_cursor, 0, 0);
                {
                    u32 case_word;
                    u32 high;
                    case_word = *element_words;
                    high = case_word >> 24;
                    packet_cursor = gosub_emit_panel(packet_cursor, ordering_table, (case_word >> 7) & 0x1FF, (*((u8*)element_words + 2)), ((*(u32*)((u8*)element_words + 4) & 1) << 8) | high,
                                           (*(u32*)((u8*)element_words + 4) >> 1) & 0xFF, render_context->display_buffer_index);
                }
                break;

            case 3:
                geometry = *(u32*)((u8*)element_words + 4);
                element_width = ((geometry & 1) << 8) | (state_word >> 24);
                transition_step = (state_word >> 3) & 0xF;
                scaled_width = element_width * transition_step;
                if (scaled_width < 0)
                {
                    scaled_width += 7;
                }
                full_height = (geometry >> 1) & 0xFF;
                scaled_height = full_height * transition_step;
                animated_width = scaled_width >> 3;
                if (scaled_height < 0)
                {
                    scaled_height += 7;
                }
                animated_height = scaled_height >> 3;
                remaining_height = (s32)(full_height - animated_height);

                packet_cursor = (*(GosubElementDrawHandler*)((u8*)element_words + 8))(ordering_table, packet_cursor, (s32)(element_width - animated_width) / 2, remaining_height / 2);
                {
                    u32 post_word;
                    u32 field;
                    u32 high;
                    post_word = *element_words;
                    field = (post_word >> 7) & 0x1FF;
                    high = post_word >> 24;
                    packet_cursor =
                        gosub_emit_panel(packet_cursor, ordering_table, field + (s32)((((*(u32*)((u8*)element_words + 4) & 1) << 8) | high) - animated_width) / 2,
                                      (*((u8*)element_words + 2)) + ((s32)((*(u32*)((u8*)element_words + 4) >> 1) & 0xFF) - animated_height) / 2, animated_width, animated_height, render_context->display_buffer_index);
                }
                exiting_word = *element_words;
                updated_exiting_word = (exiting_word & ~0x78) | (((((exiting_word >> 3) & 0xF) - 1) & 0xF) * 8);
                *element_words = updated_exiting_word;
                if (!((updated_exiting_word >> 3) & 0xF))
                {
                    *element_words = updated_exiting_word & ~7;
                }
                g_gosub_dialog_accepting_input = 0;
                break;
            }
        }

        element_words += 3;
    }

    render_context->packet_cursor = packet_cursor;
}

/**
 * @brief Emit an animated scroll arrow and its fill packet.
 * @param prim Destination packet buffer.
 * @param ot   Ordering-table tag for both packets.
 * @param x    Center X coordinate.
 * @param y    Center Y coordinate.
 * @param flag Selects the up vs down vertex arrangement.
 * @return Pointer to the next free packet slot.
 * @see decomp.me (100%)
 */
void *gosub_emit_scroll_marker(GosubScrollMarkerPacket *prim, s32 *ot, s32 x, s32 y, s32 flag)
{
    s32 pulse_value;
    s32 working_y;
    u32 i;
    u32 addr_mask;
    u8 *source_bytes;
    GosubScrollMarkerPacket *fill_packet;

    prim->len = 6;
    prim->code = 0x4C;
    prim->mask = 0x55555555;
    if (g_frame_counter & 0x10)
    {
        pulse_value = g_frame_counter & 0xF;
    }
    else
    {
        pulse_value = (~g_frame_counter) & 0xF;
    }
    working_y = pulse_value * 4;
    pulse_value = working_y + 0x70;
    prim->b = pulse_value;
    prim->g = pulse_value;
    prim->r = pulse_value;
    if (flag != 0)
    {
        pulse_value = y - 8;
        prim->y3 = pulse_value;
        prim->y0 = pulse_value;
        pulse_value = x - 6;
        working_y = y + 4;
        prim->x1 = pulse_value;
        prim->x3 = x;
        prim->x0 = x;
        prim->y1 = working_y;
        prim->x2 = x + 6;
        prim->y2 = working_y;
    }
    else
    {
        pulse_value = y + 8;
        prim->y3 = pulse_value;
        prim->y0 = pulse_value;
        pulse_value = x - 6;
        working_y = y - 4;
        prim->x1 = pulse_value;
        prim->x3 = x;
        prim->x0 = x;
        prim->y1 = working_y;
        prim->x2 = x + 6;
        prim->y2 = working_y;
    }

    addr_mask = 0xFFFFFF;
    source_bytes = (u8 *)prim;
    fill_packet = (GosubScrollMarkerPacket *)(source_bytes + 0x1C);
    prim = fill_packet;
    *(u32 *)source_bytes = (*(u32 *)source_bytes & 0xFF000000) | (*ot & addr_mask);
    i = 0;
    *ot = (*ot & 0xFF000000) | ((u32)source_bytes & addr_mask);
    do
    {
        i += 1;
        *(u8 *)prim = *source_bytes;
        source_bytes += 1;
        prim = (GosubScrollMarkerPacket *)((u8 *)prim + 1);
    } while (i < 0x14U);

    fill_packet->len = 4;
    *(u32 *)&fill_packet->r = 0;
    fill_packet->code = 0x20;
    *(u32 *)fill_packet = (*(u32 *)fill_packet & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & 0xFF000000) | ((u32)fill_packet & 0xFFFFFF);
    return (u8 *)fill_packet + 0x14;
}

/**
 * @brief Emit a clipped, framed gosub panel.
 * @param prim Destination packet buffer.
 * @param ot   Ordering-table tag for the panel packets.
 * @param x    Panel left edge.
 * @param y    Panel top edge.
 * @param w    Panel width.
 * @param h    Panel height.
 * @param flag Non-zero selects the lower frame-buffer half.
 * @return Pointer to the next free packet slot.
 * @see decomp.me (100%)
 */
GosubGpuPacket* gosub_emit_panel(GosubGpuPacket* prim, s32* ot, s32 x, s32 y, s32 w, s32 h, s32 flag)
{
    GosubGpuPacket* packet_cursor;
    GosubGpuPacket* draw_mode_packet;
    GosubGpuPacket* draw_env_packet;
    s32 draw_env[24];
    s32 working_value;

    draw_env_packet = prim;
    if (flag != 0)
    {
        working_value = y + 0xF2;
        SetDefDrawEnv((DRAWENV*)draw_env, x + 2, working_value, w - 4, h - 4);
    }
    else
    {
        working_value = y + 0xA;
        SetDefDrawEnv((DRAWENV*)draw_env, x + 2, working_value, w - 4, h - 4);
    }
    SetDrawEnv((DR_ENV*)draw_env_packet, (DRAWENV*)draw_env);

    draw_env_packet->tag = (draw_env_packet->tag & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & 0xFF000000) | ((s32)draw_env_packet & 0xFFFFFF);

    draw_env_packet = (GosubGpuPacket*)((u8*)draw_env_packet + 0x40);
    packet_cursor = gosub_emit_panel_corners(draw_env_packet, ot, x, y, w, h);
    packet_cursor = (GosubGpuPacket*)gosub_emit_panel_outline((GosubLinePacket*)packet_cursor, ot, x, y, w, h, 0xFFFFFF);
    packet_cursor = (GosubGpuPacket*)gosub_emit_panel_outline((GosubLinePacket*)packet_cursor, ot, x + 1, y + 1, w - 2, h - 2, 0);
    packet_cursor = (GosubGpuPacket*)gosub_emit_panel_outline((GosubLinePacket*)packet_cursor, ot, x - 1, y - 1, w + 2, h + 2, 0);

    do { working_value = (s32)packet_cursor; } while (0);
    ((GosubGpuPacket*)working_value)->color = 0xC0C0C0;
    ((u8*)working_value)[3] = 3;
    ((u8*)working_value)[7] = 0x62;
    ((GosubGpuPacket*)working_value)->x = x;
    ((GosubGpuPacket*)working_value)->y = y;
    ((GosubGpuPacket*)working_value)->w = w;
    ((GosubGpuPacket*)working_value)->h = h;
    ((GosubGpuPacket*)working_value)->tag = (((GosubGpuPacket*)working_value)->tag & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & 0xFF000000) | (working_value & 0xFFFFFF);

    draw_mode_packet = (GosubGpuPacket*)(working_value + 0x10);
    ((u8*)draw_mode_packet)[3] = 1;
    draw_mode_packet->color = 0xE1000045;
    draw_mode_packet->tag = (draw_mode_packet->tag & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & 0xFF000000) | ((s32)draw_mode_packet & 0xFFFFFF);
    return (GosubGpuPacket*)((u8*)draw_mode_packet + 8);
}

/**
 * @brief Emit a rectangle outline as four flat lines linked into @p ot.
 * @param line  Destination packet buffer.
 * @param ot    Ordering-table tag all four lines are linked into.
 * @param x     Rectangle left edge.
 * @param y     Rectangle top edge.
 * @param w     Rectangle width.
 * @param h     Rectangle height.
 * @param color Packed 0x00BBGGRR colour written to every line.
 * @return Pointer to the next free packet slot.
 * @see decomp.me (100%)
 */
GosubLinePacket* gosub_emit_panel_outline(GosubLinePacket* line, s32* ot, s32 x, s32 y, s32 w, s32 h, s32 color)
{
    *(u32*)&line->r0 = color;
    setLineF2(line);
    line->x0 = x + 4;
    line->y0 = y;
    line->x1 = (x + w) - 4;
    line->y1 = y;
    addPrim(ot, line);
    line++;

    *(u32*)&line->r0 = color;
    setLineF2(line);
    line->x0 = x + w;
    line->y0 = y + 4;
    line->x1 = x + w;
    line->y1 = (y + h) - 4;
    addPrim(ot, line);
    line++;

    *(u32*)&line->r0 = color;
    setLineF2(line);
    line->x0 = (x + w) - 4;
    line->y0 = y + h;
    line->x1 = x + 4;
    line->y1 = y + h;
    addPrim(ot, line);
    line++;

    *(u32*)&line->r0 = color;
    setLineF2(line);
    line->x0 = x;
    line->y0 = y + 4;
    line->x1 = x;
    line->y1 = (y + h) - 4;
    addPrim(ot, line);
    return line + 1;
}

/**
 * @brief Draw the gosub item list: one packet run per row, then the cursor
 *        highlight and one highlight per selected row.
 *
 * Rows dispatch on @c value: -3 is a full equipment card (icon strip via
 * gosub_draw_portrait plus three text lines and up to one status glyph), -2 is a
 * combination header (gosub_draw_composite_icon frame plus a label), anything else is a
 * plain label with an optional trailing glyph. Each row is culled against
 * g_gosub_window_height before any packet is emitted. The tail appends a
 * 0xF080F0 TILE for the cursor row and a 0x808080 TILE per entry of
 * g_gosub_selected_rows.
 *
 * @param ot       Ordering-table tag every packet is linked into.
 * @param initial_prim Packet cursor; copied into the local @c prim, which is what
 *                 the body advances.
 * @param x_off    Horizontal offset subtracted from every column position.
 * @param y_off    Vertical scroll offset subtracted from every row position.
 * @return Packet cursor just past the last highlight tile.
 * @see decomp.me (100%)
 */
GosubTilePacket* gosub_draw_item_list(s32* ot, s32 initial_prim, s32 x_off, s32 y_off)
{
    s32 prim;
    s32 drawn_count;
    GosubTextPosition* pos_p;
    s32 row_offset;
    GosubTextPosition pos;
    s32 row;
    s32 y;
    s32 line_y;
    s32 y_top;
    s32 sel_mul;
    s32 msg_off;
    u8* msg_p;
    s32 y_top2;
    s32 y_top3;
    s32 line_y2;
    s32 line_y3;
    s32 label_x;
    s32 x_pad;
    s32 status_pad;
    s32* table;
    s32 base;
    s32 archive_block;
    s32 detail_block;
    u8* archive_data;
    GosubTilePacket* tile;
    GosubTilePacket* mark;
    s32* cursor_p;
    s32* height_p;
    s32* scroll_p;
    u32 cursor_color;
    u32 addr_mask;

    prim = initial_prim;
    row = 0;
    drawn_count = 0;
    if (g_gosub_row_count > 0)
    {
        label_x = 0x30 - x_off;
        pos_p = &pos;
        row_offset = 0;
        archive_data = (u8*)&g_gosub_text_archive_0;
        do
        {
            table = &g_gosub_message_archive_offset;
            base = (s32)table - 0x20;
            if (g_gosub_rows[row].value == -3)
            {
                y = ((row * 0x30) - y_off) - g_gosub_scroll_y;
                if (y >= -0x2F && y < g_gosub_window_height)
                {
                    prim =
                        func_800A88A0(gosub_draw_portrait(prim, ot, row, -x_off, y, drawn_count), ot, g_gosub_rows[row].name, g_gosub_rows[row].text_color, label_x, y, 0);
                    if (g_gosub_rows[row].flags.f.alternate_format)
                    {
                        if ((g_gosub_rows[row].flags.half & 1) == 0)
                        {
                            line_y = y + 0x10;
                            prim = func_800A88A0(prim, ot, MSG_HI(0x24), g_gosub_rows[row].text_color, label_x, line_y, 0);
                            pos.x = 0x54 - x_off;
                            pos.y = line_y;
                            prim = func_800A8A78(ot, prim, g_gosub_rows[row].detail_variant, g_gosub_rows[row].text_color, pos_p, 0);
                            detail_block = *(s32*)(base + 0x24);
                            prim = func_800A88A0(prim, ot, (void*)(detail_block + (*(u16*)((detail_block + g_gosub_rows[row].detail_id * 2) + base) + base)), g_gosub_rows[row].text_color,
                                                 0x84 - x_off, line_y, 0);
                        }
                        else
                        {
                            msg_off = *(u16*)((u8*)&g_gosub_message_archive_offset + g_gosub_message_archive_offset + g_gosub_rows[row].detail_variant * 2 + 0x44);
                            prim = func_800A88A0(prim, ot, (void*)(g_gosub_message_archive_offset + (msg_off + base)), g_gosub_rows[row].text_color, label_x, y + 0x10, 0);
                        }
                    }
                    else
                    {
                        archive_block = g_gosub_text_archive_offsets_5;
                        prim = func_800A88A0(prim, ot, (void*)(archive_block + (*(u16*)((archive_block + g_gosub_rows[row].detail_id * 2) + base) + base)), g_gosub_rows[row].text_color,
                                             label_x, y + 0x10, 0);
                    }
                    line_y2 = y + 0x20;
                    prim = func_800A88A0(prim, ot, MSG_HI(0x26), g_gosub_rows[row].text_color, label_x, line_y2, 0);
                    pos.x = 0x48 - x_off;
                    pos.y = line_y2;
                    prim = func_800A8A78(ot, prim, g_gosub_rows[row].secondary_value, g_gosub_rows[row].text_color, pos_p, 0);
                    msg_off = g_gosub_message_archive_offset - -(*(u16*)((s32)g_gosub_message_archive_offset - -(s32)archive_data) + base);
                    prim = func_800A88A0(prim, ot, (void*)msg_off, g_gosub_rows[row].text_color, 0x64 - x_off, line_y2, 0);
                    pos.x = 0xB0 - x_off;
                    pos.y = line_y2;
                    prim = func_800A8A78(ot, prim, g_gosub_rows[row].primary_value, g_gosub_rows[row].text_color, pos_p, 0);
                    if (g_gosub_rows[row].detail_group != 0)
                    {
                        s32 right_padding;
                        prim = func_800A88A0(prim, ot, MSG_LO(0x4A), g_gosub_rows[row].text_color, g_gosub_window_width - (right_padding = x_off, right_padding += 0xC), line_y2, 1);
                    }
                    else if (g_gosub_rows[row].flags.half & 1)
                    {
                        s32 right_padding;
                        prim = func_800A88A0(prim, ot, MSG_LO(0x60), g_gosub_rows[row].text_color, g_gosub_window_width - (right_padding = x_off, right_padding += 0xC), line_y2, 1);
                    }
                    else if (g_gosub_rows[row].flags.f.selection_restricted)
                    {
                        s32 right_padding;
                        prim = func_800A88A0(prim, ot, MSG_LO(0x6E), g_gosub_rows[row].text_color, g_gosub_window_width - (right_padding = x_off, right_padding += 0xC), line_y2, 1);
                    }
                    drawn_count += 1;
                }
            }
            else if (g_gosub_rows[row].value == -2)
            {
                y = (row_offset - y_off) - g_gosub_scroll_y;
                if (y >= -0x1F && y < g_gosub_window_height)
                {
                    line_y3 = y + 8;
                    prim = gosub_draw_composite_icon(prim, ot, 0xC - x_off, y, g_gosub_rows[row].detail_group, g_gosub_rows[row].detail_variant);
                    prim = func_800A88A0(prim, ot, g_gosub_rows[row].name, g_gosub_rows[row].text_color, 0x4C - x_off, line_y3, 0);
                    if (g_gosub_rows[row].flags.f.alternate_format)
                    {
                        prim = func_800A88A0(prim, ot, MSG_HI(0x20), g_gosub_rows[row].text_color, 0x110 - x_off, line_y3, 1);
                    }
                }
            }
            else
            {
                status_pad = row * g_gosub_row_height;
                y_top = y_off - 2;
                y = (status_pad - y_top) - g_gosub_scroll_y;
                if (-g_gosub_row_height < y && y < g_gosub_window_height)
                {
                    prim = func_800A88A0(prim, ot, g_gosub_rows[row].name, g_gosub_rows[row].text_color, 0xC - x_off, y, 0);
                    pos.y = y;
                    x_pad = x_off + 0xC;
                    pos.x = g_gosub_window_width - x_pad;
                    if (g_gosub_rows[row].value >= 0)
                    {
                        prim = func_800A8A78(ot, prim, g_gosub_rows[row].value, g_gosub_rows[row].text_color, pos_p, 1);
                    }
                }
            }
            row_offset += 0x20;
            row += 1;
        } while (row < g_gosub_row_count);
    }

    tile = (GosubTilePacket*)prim;
    cursor_color = 0xF080F0;
    addr_mask = 0xFFFFFF;
    for (;;)
    {
        cursor_p = &g_gosub_cursor_row;
        height_p = &g_gosub_row_height;
        scroll_p = &g_gosub_scroll_y;
        break;
    }
    mark = tile + 1;
    status_pad = *cursor_p * *height_p;
    y_top2 = y_off - 2;
    y = (status_pad - y_top2) - *scroll_p;
    ((u8*)tile)[3] = 3;
    tile->color = cursor_color;
    ((u8*)tile)[7] = 0x62;
    row = 0;
    tile->w = g_gosub_window_width;
    tile->y = y - 2;
    tile->x = 1;
    tile->h = g_gosub_row_height - 1;
    setaddr(tile, getaddr(ot) & addr_mask);
    setaddr(ot, tile);
    while (row < g_gosub_selection_count)
    {
        {
            mark->x = (row | 1) & 1;
            mark->color = 0x808080;
            ((u8*)mark)[3] = 3;
            ((u8*)mark)[7] = 0x62;
            sel_mul = g_gosub_selected_rows[row] * g_gosub_row_height;
            y_top3 = y_off - 2;
            y = (sel_mul - y_top3) - g_gosub_scroll_y;
            mark->w = g_gosub_window_width;
            mark->y = y - 2;
            mark->h = g_gosub_row_height - 1;
            row += 1;
            ADD_PRIM_MASKED(ot, mark);
            mark += 1;
        }
    }
    return mark;
}

/**
 * @brief Upload a row's portrait strip and CLUT, then emit its 48x48 sprite.
 *
 * The portrait index comes from the row's detail id (offset by 0x41 for the
 * alternate record format); the pixel and CLUT rectangles are double-buffered by
 * g_gosub_frame_parity. Draws nothing once five portraits are on screen.
 *
 * @param prim  Packet cursor.
 * @param ot    Ordering-table tag to link into.
 * @param row   g_gosub_rows index to portray.
 * @param x     Sprite left edge.
 * @param y     Sprite top edge.
 * @param count How many portraits were already emitted this frame.
 * @return Packet cursor past the sprite (gosub_finish_glyph_run's return), or prim
 *         when count is 5 or more.
 * @see decomp.me (100%)
 */
s32 gosub_draw_portrait(s32 prim, s32* ot, s32 row, s32 x, s32 y, s32 count)
{
    SPRT* sprt;
    RECT rect;
    s32 idx;
    s32 cell;
    s32 n;

    if (count >= 5)
    {
        return prim;
    }

    if (g_gosub_rows[row].flags.f.alternate_format)
    {
        idx = g_gosub_rows[row].detail_id;
    }
    else
    {
        idx = g_gosub_rows[row].detail_id + 0x41;
    }
    n = count;
    cell = n * 3;

    rect.x = cell * 4 + 0x140;
    rect.w = 0xC;
    rect.h = 0x30;
    rect.y = g_gosub_frame_parity * 0x30;
    LoadImage(&rect, (u8*)g_gosub_portrait_archive + g_gosub_portrait_archive[idx] + 0x1C);

    rect.y = 0x1F2;
    rect.w = 0x10;
    rect.h = 1;
    n = n * 0x10;
    rect.x = n + g_gosub_frame_parity * 0x50;
    LoadImage(&rect, (u8*)g_gosub_portrait_archive + g_gosub_portrait_archive[idx] - 4);

    sprt = (SPRT*)prim;
    SET_BGR0_PACKED(sprt, GPU_TINT_NEUTRAL);
    setSprt(sprt);
    sprt->u0 = cell * 0x10;
    sprt->x0 = x;
    sprt->v0 = g_gosub_frame_parity * 0x30;
    sprt->y0 = y;
    sprt->w = 0x30;
    sprt->h = 0x30;
    sprt->clut = (((n + g_gosub_frame_parity * 0x50) >> 4) & 0x3F) | 0x7C80;
    addPrim(ot, sprt);
    return gosub_finish_glyph_run(prim + 0x14, ot);
}

/**
 * @brief Draw the combination preview for the cursor row.
 *
 * When at least one row is selected and the cursor sits on a different row,
 * func_800CA480 is asked whether the two rows' item indices combine. It
 * returns the resulting item in g_gosub_combination_result_id and fills g_gosub_combination_quantity and
 * g_gosub_combination_variant; a zero result means the pair does not combine and nothing is
 * drawn. Otherwise a frame is emitted, then the result's archive name, with
 * g_gosub_combination_quantity's decimal form appended after a separator when it is nonzero.
 *
 * @param ot       Ordering-table tag every packet is linked into.
 * @param initial_prim Packet cursor.
 * @param x_off    Horizontal offset subtracted from every column position.
 * @param y_off    Vertical offset subtracted from every row position.
 * @return Packet cursor past the last packet, or the incoming cursor when
 *         there is no combination to show.
 *
 * @see decomp.me (100%)
 */
s32 gosub_draw_combination_preview(s32* ot, s32 initial_prim, s32 x_off, s32 y_off)
{
    s32 stack_pad[2];
    u8 result_name[0x50];
    u8 number_text[0x50];
    s32 pair[3];
    s32 base;
    u8* name_cursor;
    u8* archive;
    s32 block_offset;
    s32 prim;

    g_gosub_combination_result_id = 0;
    prim = initial_prim;
    if (g_gosub_selection_count != 0)
    {
        if (g_gosub_cursor_row != g_gosub_selected_rows[0])
        {
            pair[0] = g_gosub_rows[g_gosub_selected_rows[0]].index;
            pair[1] = g_gosub_rows[g_gosub_cursor_row].index;
            g_gosub_combination_result_id = func_800CA480(pair, &g_gosub_combination_quantity, &g_gosub_combination_variant);
        }
    }
    if (g_gosub_combination_result_id != 0)
    {
        prim = gosub_draw_composite_icon(prim, ot, 0xC - x_off, -y_off, g_gosub_combination_result_id, g_gosub_combination_variant);
        name_cursor = result_name;
        archive = (u8*)g_gosub_text_archive_offsets_3;
        base = (s32)archive;
        base -= 0x18;
        block_offset = g_gosub_text_archive_offsets_3[0];
        gosub_copy_encoded_string(name_cursor, block_offset + (*(u16*)(g_gosub_combination_result_id * 2 + block_offset + base) + base));
        if (g_gosub_combination_quantity != 0)
        {
            gosub_append_encoded_string(name_cursor, D_800EC3DA - 0x16 + D_800EC3DA[0] + (D_800EC3DA[1] << 8));
            func_800A8B90(number_text, g_gosub_combination_quantity, 1);
            gosub_append_encoded_string(name_cursor, number_text);
        }
        prim = func_800A88A0(prim, ot, name_cursor, 4, 0x4C - x_off, 0xA - y_off, 0);
    }
    return prim;
}

/**
 * @brief Dialog handler for the selected row's action prompt.
 *
 * A nonzero @p dialog_result cancels: the selection count is dropped and the
 * dialog element is deactivated. On confirm, bit 0 of g_gosub_dialog_choice
 * picks the path. When it is clear the work is handed to gosub_open_sort_dialog. When
 * it is set the first selected row decides: a row with flag2 set is rejected
 * with message 0x22 (the selection is dropped and g_gosub_suppress_dialog_sound is raised),
 * otherwise a follow-up dialog is opened with gosub_handle_delete_dialog as its handler.
 *
 * @param dialog_result Zero to confirm; nonzero to cancel.
 * @return Always 0.
 * @see decomp.me (100%)
 */
s32 gosub_handle_row_action_dialog(s32 dialog_result)
{
    GosubElement* element;

    if (dialog_result != 0)
    {
        g_gosub_selection_count = 0;
        g_gosub_elements[0].attr.f.state = GOSUB_ELEMENT_STATE_INACTIVE;
        return 0;
    }

    if (g_gosub_dialog_choice & 1)
    {
        if (g_gosub_rows[g_gosub_selected_rows[0]].flags.f.alternate_format)
        {
            GOSUB_MSG(0x22);
            g_gosub_selection_count = 0;
            g_gosub_suppress_dialog_sound = 1;
            return 0;
        }
        element = &g_gosub_elements[0];
        element->draw_handler = (void*)&gosub_draw_confirmation_prompt;
        g_gosub_dialog_handler = gosub_handle_delete_dialog;
        g_gosub_dialog_choice = 0;
        element->attr.f.state = GOSUB_ELEMENT_STATE_ENTERING;
        element->attr.f.transition_step = 1;
        element->attr.f.x = 0x20;
        element->attr.f.width_low = 0x70;
        element->width_high = 1;
        element->y = 0x24;
        SET_ELEMENT_WIDTH_LOW(element, 0);
    }
    else
    {
        gosub_open_sort_dialog();
    }
    return 0;
}

/**
 * @brief Dialog handler that drops the cursor row and re-clamps the list.
 *
 * Always deactivates the dialog element. A nonzero @p dialog_result, or bit 0
 * of g_gosub_dialog_choice, cancels: the selection count is set to 1 and
 * nothing else changes. On confirm the cursor row is handed to gosub_delete_packed_record
 * (by entry index) and to gosub_delete_list_row (by row), the selection is cleared,
 * and the viewport is re-clamped -- the cursor is pulled back to the last row
 * when it now sits past the end, and the scroll target is clamped to the
 * bottom of the shortened list over a 4-frame scroll.
 *
 * @param dialog_result Zero to confirm; nonzero to cancel.
 * @return 1 when no rows remain, otherwise 0.
 * @see decomp.me (100%)
 */
s32 gosub_handle_delete_dialog(s32 dialog_result)
{
    s32 scroll_y;
    s32 max_scroll;

    g_gosub_elements[0].attr.f.state = GOSUB_ELEMENT_STATE_INACTIVE;
    if (dialog_result == 0 && (g_gosub_dialog_choice & 1) == 0)
    {
        gosub_delete_packed_record(g_gosub_rows[g_gosub_cursor_row].index);
        gosub_delete_list_row(g_gosub_cursor_row);
        g_gosub_selection_count = 0;
        if (g_gosub_row_count == 0)
        {
            return 1;
        }
        if (g_gosub_cursor_row >= g_gosub_row_count)
        {
            g_gosub_cursor_row = g_gosub_row_count - 1;
        }
        scroll_y = g_gosub_scroll_y;
        max_scroll = ((g_gosub_row_count * g_gosub_row_height) - g_gosub_window_height) + 4;
        if (max_scroll < scroll_y)
        {
            scroll_y = max_scroll;
        }
        if (scroll_y < 0)
        {
            scroll_y = 0;
        }
        g_gosub_scroll_target_y = scroll_y;
        g_gosub_scroll_frames_remaining = 4;
        return 0;
    }
    g_gosub_selection_count = 1;
    return 0;
}

/**
 * @brief Dialog handler that backs the screen sequence out one step.
 *
 * Does nothing and reports 1 when the dialog was confirmed with bit 0 of
 * g_gosub_dialog_choice clear. Otherwise it releases one selection slot (only
 * when the selection is already full), steps the screen sequence back, runs
 * g_gosub_select_handler when one is installed, drops the nesting depth in
 * g_gosub_result_count, and puts element 0 into its exit animation.
 *
 * @param dialog_result Zero to confirm; nonzero to cancel.
 * @return 1 when the confirm path is taken, otherwise 0.
 *
 * @see decomp.me (100%)
 */
s32 gosub_handle_backtrack_dialog(s32 dialog_result)
{
    if (dialog_result == 0 && (g_gosub_dialog_choice & 1) == 0)
    {
        return 1;
    }

    if (g_gosub_required_selection_count == g_gosub_selection_count)
    {
        g_gosub_selection_count -= 1;
    }

    g_gosub_screen_sequence_index -= 1;
    if (g_gosub_select_handler != 0)
    {
        g_gosub_select_handler();
        g_gosub_result_count -= 1;
    }
    else
    {
        g_gosub_result_count -= 1;
    }

    g_gosub_elements[0].attr.f.state = GOSUB_ELEMENT_STATE_EXITING;
    g_gosub_elements[0].attr.f.transition_step = 8;
    return 0;
}

/**
 * @brief Apply the selected logic-block sort and toggle its direction.
 *
 * A confirmed sort uses the selected type, power, or shape key and the current
 * direction. The direction then flips for the next sort. Cancelling preserves
 * the rows and restores one pending selection.
 *
 * @param dialog_result Zero to confirm; nonzero to cancel.
 * @return Always 0.
 * @see decomp.me (100%)
 */
s32 gosub_handle_sort_dialog(s32 dialog_result)
{
    if (dialog_result == 0)
    {
        gosub_sort_rows((g_gosub_sort_ascending << GOSUB_SORT_ASCENDING_SHIFT) +
                        (g_gosub_dialog_choice % GOSUB_SORT_KEY_COUNT));
        g_gosub_selection_count = 0;
        g_gosub_sort_ascending ^= 1;
    }
    else
    {
        g_gosub_selection_count = 1;
    }

    g_gosub_elements[0].attr.f.state = GOSUB_ELEMENT_STATE_INACTIVE;
    return 0;
}

/**
 * @brief Open the wide confirmation dialog and hand it to gosub_handle_row_action_dialog.
 *
 * Installs gosub_draw_two_option_dialog as element 0's draw handler and gosub_handle_row_action_dialog as the
 * dialog's result handler, clears the pending choice, then starts the element
 * entering at x 0x80 / y 0x24 with code 0x80. func_800AA02C runs last.
 *
 * @see decomp.me (100%)
 */
void gosub_open_row_action_dialog(void)
{
    GosubElement* element;

    element = &g_gosub_elements[0];
    element->draw_handler = (void*)&gosub_draw_two_option_dialog;
    g_gosub_dialog_choice = 0;
    g_gosub_dialog_handler = gosub_handle_row_action_dialog;
    element->attr.f.state = GOSUB_ELEMENT_STATE_ENTERING;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0x80;
    element->attr.f.width_low = 0x70;
    element->width_high = 0;
    element->y = 0x24;
    SET_ELEMENT_WIDTH_LOW(element, 0x80);
    func_800AA02C();
}

/**
 * @brief Open the three-option row sorting dialog.
 * @see decomp.me (100%)
 */
void gosub_open_sort_dialog(void)
{
    GosubElement* element;

    element = &g_gosub_elements[0];
    element->draw_handler = (void*)&gosub_draw_three_option_dialog;
    g_gosub_dialog_handler = gosub_handle_sort_dialog;
    g_gosub_dialog_choice = 0;
    element->attr.f.state = GOSUB_ELEMENT_STATE_ENTERING;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0x80;
    element->attr.f.width_low = 0x70;
    element->width_high = 0;
    element->y = 0x34;
    SET_ELEMENT_WIDTH_LOW(element, 0x80);
    func_800AA02C();
}

/**
 * @brief Draw handler for element 0 of the wide confirmation dialog.
 *
 * Emits the two option labels through func_800A88A0, highlighting the one that
 * matches the current selection: the color toggles between 5 (highlighted) and
 * 4 (dim) with g_gosub_dialog_choice bit 0, inverted between the two rows. Both
 * labels sit at x 0x40 - x_off; the rows are at y 2 - y_off and 0x12 - y_off.
 * The labels come from message archive entries -0x12 and -0x10.
 *
 * @param ot    Ordering-table tag every packet links into.
 * @param prim  Packet cursor; threaded through both draws.
 * @param x_off Horizontal offset subtracted from the label column.
 * @param y_off Vertical offset subtracted from both row positions.
 * @return Packet cursor past the last emitted label.
 *
 * @see decomp.me (100%)
 */
s32 gosub_draw_two_option_dialog(s32* ot, s32 prim, s32 x_off, s32 y_off)
{
    s32* table;
    s32 base;
    void* first_option;
    void* second_option;
    s32 color;
    s32 first_color;
    s32 packet_cursor;
    s32 stack_pad[14];

    packet_cursor = prim;
    table = &g_gosub_message_archive_offset;
    base = (s32)table - 0x20;

    first_option = (void*)(g_gosub_message_archive_offset + (base + *(u16*)((u8*)&g_gosub_message_archive_offset + g_gosub_message_archive_offset - 0x12)));
    first_color = 5;
    if ((g_gosub_dialog_choice & 1) == 0)
    {
        first_color = 4;
    }
    packet_cursor = func_800A88A0(packet_cursor, ot, first_option, first_color, 0x40 - x_off, 2 - y_off, 2);

    second_option = (void*)(g_gosub_message_archive_offset + (base + *(u16*)((u8*)&g_gosub_message_archive_offset + g_gosub_message_archive_offset - 0x10)));
    color = 5;
    if ((g_gosub_dialog_choice & 1) != 0)
    {
        color = 4;
    }
    packet_cursor = func_800A88A0(packet_cursor, ot, second_option, color, 0x40 - x_off, 0x12 - y_off, 2);

    return packet_cursor;
}

/**
 * @brief Draw handler for element 0 of the three-option wide confirmation dialog.
 *
 * Emits the three option labels through func_800A88A0, dimming the one that
 * matches the current selection: the color is 5 (bright) unless the row index
 * equals g_gosub_dialog_choice % 3, in which case it is 4 (dim). All labels sit
 * at x 0x40 - x_off; the rows are at y 2 - y_off, 0x12 - y_off, and 0x22 - y_off.
 * The labels come from message archive entries -0xE, -0xC, and -0xA.
 *
 * @param ot    Ordering-table tag every packet links into.
 * @param prim  Packet cursor; threaded through all three draws.
 * @param x_off Horizontal offset subtracted from the label column.
 * @param y_off Vertical offset subtracted from every row position.
 * @return Packet cursor past the last emitted label.
 *
 * @see decomp.me (100%)
 */
s32 gosub_draw_three_option_dialog(s32* ot, s32 prim, s32 x_off, s32 y_off)
{
    s32* table;
    s32 base;
    void* first_option;
    void* second_option;
    void* third_option;
    s32 color;
    s32 first_color;
    s32 packet_cursor;
    s32 selected_option;
    s32 stack_pad[14];

    packet_cursor = prim;
    table = &g_gosub_message_archive_offset;
    base = (s32)table - 0x20;

    first_option = (void*)(g_gosub_message_archive_offset + (base + *(u16*)((u8*)&g_gosub_message_archive_offset + g_gosub_message_archive_offset - 0xE)));
    selected_option = g_gosub_dialog_choice;
    selected_option %= GOSUB_SORT_KEY_COUNT;
    first_color = 5;
    if (selected_option == 0)
    {
        first_color = 4;
    }
    packet_cursor = func_800A88A0(packet_cursor, ot, first_option, first_color, 0x40 - x_off, 2 - y_off, 2);

    second_option = (void*)(g_gosub_message_archive_offset + (base + *(u16*)((u8*)&g_gosub_message_archive_offset + g_gosub_message_archive_offset - 0xC)));
    color = 5;
    if (g_gosub_dialog_choice % GOSUB_SORT_KEY_COUNT == 1)
    {
        color = 4;
    }
    packet_cursor = func_800A88A0(packet_cursor, ot, second_option, color, 0x40 - x_off, 0x12 - y_off, 2);

    third_option = (void*)(g_gosub_message_archive_offset + (base + *(u16*)((u8*)&g_gosub_message_archive_offset + g_gosub_message_archive_offset - 0xA)));
    color = 5;
    if (g_gosub_dialog_choice % GOSUB_SORT_KEY_COUNT == 2)
    {
        color = 4;
    }
    packet_cursor = func_800A88A0(packet_cursor, ot, third_option, color, 0x40 - x_off, 0x22 - y_off, 2);

    return packet_cursor;
}

/**
 * @brief Open a modal dialog containing caller-provided text.
 * @param message_text Pointer to the encoded dialog text.
 * @see decomp.me (100%)
 */
void gosub_open_message_dialog(s32 message_text)
{
    GosubElement* element;

    g_gosub_dialog_text = message_text;
    element = &g_gosub_elements[0];
    element->draw_handler = (void*)&gosub_draw_message_dialog;
    g_gosub_dialog_choice = 0;
    g_gosub_dialog_accepting_input = 1;
    g_gosub_suppress_dialog_sound = 0;
    element->attr.f.state = GOSUB_ELEMENT_STATE_ENTERING;
    element->attr.f.transition_step = 1;
    element->attr.f.x = 0x20;
    element->attr.f.width_low = 0x70;
    element->width_high = 1;
    element->y = 0x14;
    SET_ELEMENT_WIDTH_LOW(element, 0);
    func_800AA02C();
    g_gosub_result_count = 0;
}

/**
 * @brief Draw the text stored by gosub_open_message_dialog.
 * @param ot Ordering-table tag to link into.
 * @param prim Packet cursor.
 * @param x_off Horizontal dialog animation offset.
 * @param y_off Vertical dialog animation offset.
 * @return Packet cursor after the text primitives.
 * @see decomp.me (100%)
 */
s32 gosub_draw_message_dialog(s32* ot, s32 prim, s32 x_off, s32 y_off)
{
    s32 stack_pad[14];

    prim = func_800A88A0(prim, ot, (void*)g_gosub_dialog_text, 4, 0x80 - x_off, 2 - y_off, 2);
    return prim;
}

/**
 * @brief Draw a fixed header and the current equipment row's details.
 * @param ot Ordering-table tag to link into.
 * @param prim Packet cursor.
 * @param x_off Horizontal dialog animation offset.
 * @param y_off Vertical dialog animation offset.
 * @return Packet cursor after the emitted text.
 * @see decomp.me (100%)
 */
s32 gosub_draw_detail_header(s32* ot, s32 prim, s32 x_off, s32 y_off)
{
    s32* table;
    s32 base;
    void* glyph;
    s32 stack_pad[14];

    table = &g_gosub_message_archive_offset;
    base = (s32)table - 0x20;

    glyph = (void*)(g_gosub_message_archive_offset + (base + *(u16*)((u8*)&g_gosub_message_archive_offset + g_gosub_message_archive_offset - 0x18)));
    prim = func_800A88A0(prim, ot, glyph, 4, 0x84 - x_off, 2 - y_off, 2);
    if (g_gosub_show_row_details != 0)
    {
        prim = gosub_draw_equipment_details(prim, ot, x_off, y_off);
    }
    return prim;
}

/**
 * @brief Draw the two fixed header lines used by the grouped selection screen.
 * @param ot Ordering-table tag to link into.
 * @param prim Packet cursor.
 * @param x_off Horizontal dialog animation offset.
 * @param y_off Vertical dialog animation offset.
 * @return Packet cursor after both lines.
 * @see decomp.me (100%)
 */
s32 gosub_draw_two_line_header(s32* ot, s32 prim, s32 x_off, s32 y_off)
{
    s32* table;
    s32 base;
    void* glyph;
    s32 stack_pad[14];

    table = &g_gosub_message_archive_offset;
    base = (s32)table - 0x20;

    glyph = (void*)(g_gosub_message_archive_offset + (base + *(u16*)((u8*)&g_gosub_message_archive_offset + g_gosub_message_archive_offset - 0x1C)));
    prim = func_800A88A0(prim, ot, glyph, 4, 0x84 - x_off, 2 - y_off, 2);

    glyph = (void*)(g_gosub_message_archive_offset + (base + *(u16*)((u8*)&g_gosub_message_archive_offset + g_gosub_message_archive_offset - 0x1A)));
    prim = func_800A88A0(prim, ot, glyph, 4, 0x84 - x_off, 0x12 - y_off, 2);

    return prim;
}

/**
 * @brief Draw a confirmation title and two highlighted choices.
 * @param ot Ordering-table tag to link into.
 * @param prim Packet cursor.
 * @param x_off Horizontal dialog animation offset.
 * @param y_off Vertical dialog animation offset.
 * @return Packet cursor after the prompt.
 * @see decomp.me (100%)
 */
s32 gosub_draw_confirmation_prompt(s32* ot, s32 prim, s32 x_off, s32 y_off)
{
    s32* table;
    s32 base;
    void* glyph;
    s32 color;
    s32 stack_pad[14];

    table = &g_gosub_message_archive_offset;
    base = (s32)table - 0x20;

    glyph = (void*)(g_gosub_message_archive_offset + (base + *(u16*)((u8*)&g_gosub_message_archive_offset + g_gosub_message_archive_offset - 0x14)));
    prim = func_800A88A0(prim, ot, glyph, 4, 0x80 - x_off, 2 - y_off, 2);

    glyph = (void*)(g_gosub_message_archive_offset + (base + *(u16*)((u8*)&g_gosub_message_archive_offset + g_gosub_message_archive_offset - 0x20)));
    color = 5;
    if ((g_gosub_dialog_choice & 1) == 0)
    {
        color = 4;
    }
    prim = func_800A88A0(prim, ot, glyph, color, 0x78 - x_off, 0x12 - y_off, 1);

    glyph = (void*)(g_gosub_message_archive_offset + (base + *(u16*)((u8*)&g_gosub_message_archive_offset + g_gosub_message_archive_offset - 0x1E)));
    color = 4;
    if ((g_gosub_dialog_choice & 1) == 0)
    {
        color = 5;
    }
    prim = func_800A88A0(prim, ot, glyph, color, 0x88 - x_off, 0x12 - y_off, 0);

    return prim;
}

/**
 * @brief Draw the current row's description and optional equipment details.
 * @param ot Ordering-table tag to link into.
 * @param prim Packet cursor.
 * @param x_off Horizontal dialog animation offset.
 * @param y_off Vertical dialog animation offset.
 * @return Packet cursor after the row description.
 * @see decomp.me (100%)
 */
s32 gosub_draw_row_description(s32* ot, s32 prim, s32 x_off, s32 y_off)
{
    s32 stack_pad[12];

    prim = func_800A88A0(prim, ot, g_gosub_rows[g_gosub_cursor_row].desc, 4, 0x84 - x_off, 2 - y_off, 2);
    if (g_gosub_show_row_details != 0)
    {
        prim = gosub_draw_equipment_details(prim, ot, x_off, y_off);
    }
    return prim;
}

/**
 * @brief Draw the current equipment row's type-specific detail values.
 * @param prim Packet cursor.
 * @param ot Ordering-table tag to link into.
 * @param x_off Horizontal dialog animation offset.
 * @param y_off Vertical dialog animation offset.
 * @return Packet cursor after the detail values.
 * @see decomp.me (100%)
 */
s32 gosub_draw_equipment_details(s32 prim, s32* ot, s32 x_off, s32 y_off)
{
    s32 kind;
    GosubTextPosition pos;
    u8* base;
    s32* table;
    s32 text_offset;

    kind = g_gosub_rows[g_gosub_cursor_row].equipment_kind;

    switch (kind)
    {
    case 0:
        prim = func_800A88A0(prim, ot, (void*)((u8*)D_800EC3EE - 0x2A + D_800EC3EE[0] + (D_800EC3EE[1] << 8)), 4, 0x10 - x_off, 0x12 - y_off, 0);
        pos.x = 0x68 - x_off;
        pos.y = (s16)(0x12 - y_off);
        prim = func_800A8A78(ot, prim, g_gosub_rows[g_gosub_cursor_row].primary_value, 4, &pos, 0);
        break;

    case 1:
        prim = func_800A88A0(prim, ot, (void*)((u8*)D_800EC3F0 - 0x2C + D_800EC3F0[0] + (D_800EC3F0[1] << 8)), 4, 0x10 - x_off, 0x12 - y_off, 0);
        pos.x = 0x60 - x_off;
        pos.y = (s16)(0x12 - y_off);
        prim = func_800A8A78(ot, prim,
                             g_gosub_rows[g_gosub_cursor_row].stats[0] + g_gosub_rows[g_gosub_cursor_row].stats[1] + g_gosub_rows[g_gosub_cursor_row].stats[2] +
                                 g_gosub_rows[g_gosub_cursor_row].stats[3],
                             4, &pos, 0);
        break;

    default:
        table = &g_gosub_text_archive_offsets_6;
        prim = func_800A88A0(prim, ot, (void*)((u8*)D_800EC3F2 - 0x2E + D_800EC3F2[0] + (D_800EC3F2[1] << 8)), 4, 0x10 - x_off, 0x12 - y_off, 0);
        pos.x = 0x38 - x_off;
        pos.y = (s16)(0x12 - y_off);
        prim = func_800A8A78(ot, prim, g_gosub_rows[g_gosub_cursor_row].primary_value, 4, &pos, 0);
        base = (u8*)table;
        base -= 0x2C;
        text_offset = base + *(u16*)(g_gosub_rows[g_gosub_cursor_row].stats[0] * 2 + g_gosub_text_archive_offsets_6 + base);
        prim = func_800A88A0(prim, ot, (void*)(g_gosub_text_archive_offsets_6 + text_offset), 4, 0x60 - x_off, 0x12 - y_off, 0);
        break;
    }
    return prim;
}

/**
 * @brief Draw the current gosub screen title.
 * @param ot Ordering-table tag to link into.
 * @param prim Packet cursor.
 * @param x_off Horizontal dialog animation offset.
 * @param y_off Vertical dialog animation offset.
 * @return Packet cursor after the title.
 * @see decomp.me (100%)
 */
s32 gosub_draw_title(s32* ot, s32 prim, s32 x_off, s32 y_off)
{
    s32 stack_pad[14];

    prim = func_800A88A0(prim, ot, g_gosub_title_text, 4, 0x84 - x_off, 2 - y_off, 2);
    return prim;
}

/**
 * @brief Append one encoded string to another.
 * @param dst Null-terminated destination buffer.
 * @param src Null-terminated source string.
 * @see decomp.me (100%)
 */
void gosub_append_encoded_string(u8* dst, u8* src)
{
    s32 len1;
    s32 len2;
    s32 i;

    len1 = gosub_encoded_string_length(dst);
    len2 = gosub_encoded_string_length(src);

    for (i = 0; i < len2; i++)
    {
        dst[len1 + i] = src[i];
    }

    dst[len1 + i] = 0;
}

/**
 * @brief Count bytes in a null-terminated encoded string.
 * @param text Encoded string to measure.
 * @return Byte count excluding the terminator.
 * @see decomp.me (100%)
 */
s32 gosub_encoded_string_length(const u8* text)
{
    const u8* scan_cursor;
    s32 byte_count;

    scan_cursor = text;
    byte_count = 0;

    while (*scan_cursor)
    {
        if (IS_DBCS_LEAD_BYTE(*scan_cursor))
        {
            scan_cursor += 2;
            byte_count += 2;
        }
        else
        {
            scan_cursor += 1;
            byte_count += 1;
        }
    }

    return byte_count;
}

/**
 * @brief Copy a null-terminated encoded string.
 * @param dst Destination buffer.
 * @param src Source string.
 * @see decomp.me (100%)
 */
void gosub_copy_encoded_string(u8* dst, u8* src)
{
    const u8* scan_cursor;
    s32 byte_count;
    s32 i;

    scan_cursor = src;
    byte_count = 0;

    while (*scan_cursor)
    {
        if (IS_DBCS_LEAD_BYTE(*scan_cursor))
        {
            scan_cursor += 2;
            byte_count += 2;
        }
        else
        {
            scan_cursor += 1;
            byte_count += 1;
        }
    }

    for (i = 0; i < byte_count; i++)
    {
        dst[i] = src[i];
    }

    dst[i] = 0;
}

/**
 * @brief Upload the gosub interface image and CLUT to their fixed VRAM slots.
 * @see decomp.me (100%)
 */
void gosub_upload_ui_image(void)
{
    GosubImageVramLayout destinations;

    destinations.pixel_x = 0x140;
    destinations.pixel_y = 0;
    destinations.clut_x = 0;
    destinations.clut_y = 0x1F2;
    gosub_upload_image_archive(&destinations, &g_gosub_image_archive);
}

/**
 * @brief Upload a TIM's optional CLUT and pixel data to selected VRAM positions.
 * @param destinations VRAM destinations for the pixel and CLUT blocks.
 * @param tim TIM resource to upload.
 * @see decomp.me (100%)
 */
void gosub_upload_image_archive(GosubImageVramLayout* destinations, TimPrefix* tim)
{
    RECT upload_rect;
    s32 flags;
    s32 clut_block_size;
    TimDimensions* pixel_dimensions;

    flags = tim->flags;
    clut_block_size = tim->clut_block.bnum;

    if (flags & GOSUB_TIM_HAS_CLUT)
    {
        setRECT(&upload_rect, destinations->clut_x, destinations->clut_y,
                CLUT_ENTRY_COUNT, 1);
        LoadImage(&upload_rect, (u_long*)tim->clut_data);
        pixel_dimensions = &((TimBlock*)(clut_block_size + (s32)tim + TIM_HEADER_SIZE))->dimensions;
    }
    else
    {
        pixel_dimensions = &tim->clut_block.dimensions;
    }

    setRECT(&upload_rect, destinations->pixel_x, destinations->pixel_y,
            pixel_dimensions->width, pixel_dimensions->height);
    LoadImage(&upload_rect,
              (u_long*)(((TimBlock*)(clut_block_size + (s32)tim + TIM_HEADER_SIZE)) + 1));
}

/**
 * @brief Draw a composite icon from its base glyph and positioned parts.
 * @param initial_packet Next free GPU packet.
 * @param ordering_table Ordering table to receive the glyph packets.
 * @param x Base screen x coordinate.
 * @param y Base screen y coordinate.
 * @param icon_id Icon identifier used for the base glyph and part CLUT.
 * @param layout_index Composite layout index.
 * @return Packet cursor after closing the glyph run.
 * @see decomp.me (100%)
 */
s32 gosub_draw_composite_icon(s32 initial_packet, s32* ordering_table, s32 x, s32 y, s32 icon_id, s32 layout_index)
{
    GosubCompositeIconView layout_view;
    s32 clut;
    s16 layout_x;
    s16 layout_y;
    s8 base_glyph_x;
    s8 base_glyph_y;
    s32 icon_x;
    s32 icon_y;
    GosubCompositeIconView part_view;
    s32 part_index;
    s32 packet_cursor;
    u8* table_bytes;

    clut = D_800F2180[icon_id];
    table_bytes = D_800F1CD0;
    layout_view.bytes = (u8*)(layout_index * (s32)sizeof(GosubCompositeIconLayout) +
                              (s32)table_bytes);
    layout_x = layout_view.layout->origin_x;
    layout_y = layout_view.layout->origin_y;
    base_glyph_x = layout_view.layout->base_x;
    base_glyph_y = layout_view.layout->base_y;
    icon_x = x + layout_x * GOSUB_COMPOSITE_ICON_BASE_CELL_SIZE;
    icon_y = y + layout_y * GOSUB_COMPOSITE_ICON_BASE_CELL_SIZE;
    packet_cursor = gosub_emit_glyph(initial_packet, ordering_table,
                                     icon_id + GOSUB_COMPOSITE_ICON_BASE_GLYPH_OFFSET,
                                     base_glyph_x * GOSUB_COMPOSITE_ICON_BASE_CELL_SIZE + icon_x,
                                     base_glyph_y * GOSUB_COMPOSITE_ICON_BASE_CELL_SIZE + icon_y,
                                     GOSUB_COMPOSITE_ICON_BASE_CLUT);

    /* The part count is byte zero of the packed layout. */
    layout_y = 0;
    for (part_index = layout_y; part_index < layout_view.bytes[layout_y]; part_index++)
    {
        u8* loop_base = &D_800F1CD0[layout_y];

        part_view.bytes = (u8*)(layout_index * (s32)sizeof(GosubCompositeIconLayout) +
                                part_index * (s32)sizeof(GosubCompositeIconPart) +
                                (s32)loop_base);
        /* The shifted layout view exposes the current tuple as parts[0]. */
        packet_cursor = gosub_emit_glyph(packet_cursor, ordering_table,
                                         part_view.layout->parts[0].glyph_id,
                                         part_view.layout->parts[0].x * GOSUB_COMPOSITE_ICON_PART_CELL_SIZE + icon_x,
                                         part_view.layout->parts[0].y * GOSUB_COMPOSITE_ICON_PART_CELL_SIZE + icon_y,
                                         clut);
    }
    return gosub_finish_glyph_run(packet_cursor, ordering_table);
}

/**
 * @brief Append the texture-page packet that closes a glyph run.
 *
 * @param packet_cursor Packet cursor.
 * @param ordering_table Ordering-table tag to link the packet into.
 * @return Packet cursor past the 8-byte draw-mode packet.
 * @see decomp.me (100%)
 */
s32 gosub_finish_glyph_run(s32 packet_cursor, s32* ordering_table)
{
    DR_TPAGE* draw_tpage;

    draw_tpage = (DR_TPAGE*)packet_cursor;
    setDrawTPage(draw_tpage, 0, 0, GOSUB_FONT_TPAGE);
    addPrim(ordering_table, draw_tpage);
    return packet_cursor + sizeof(DR_TPAGE);
}

/**
 * @brief Emit one glyph sprite described by the g_gosub_glyph_metrics cell table.
 *
 * @param packet_cursor Packet cursor.
 * @param ordering_table Ordering-table tag to link the sprite into.
 * @param glyph_id Index into g_gosub_glyph_metrics supplying the cell u/v and size.
 * @param x Sprite left edge.
 * @param y Sprite top edge.
 * @param clut_index CLUT slot on the glyph palette row.
 * @return Packet cursor past the 0x14-byte sprite.
 * @see decomp.me (100%)
 */
s32 gosub_emit_glyph(s32 packet_cursor, s32* ordering_table, s32 glyph_id, s32 x, s32 y, s32 clut_index)
{
    SPRT* sprite;

    sprite = (SPRT*)packet_cursor;
    SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
    setSprt(sprite);
    setXY0(sprite, x, y);
    setWH(sprite, g_gosub_glyph_metrics[glyph_id].w, g_gosub_glyph_metrics[glyph_id].h);
    setUV0(sprite, g_gosub_glyph_metrics[glyph_id].u0, g_gosub_glyph_metrics[glyph_id].v0);
    setClut(sprite, clut_index << GOSUB_GLYPH_CLUT_X_SHIFT, GOSUB_GLYPH_CLUT_Y);
    addPrim(ordering_table, sprite);
    return packet_cursor + sizeof(SPRT);
}

/**
 * @brief Delete one packed logic-block record and close the gap.
 *
 * @param record_index Index of the record to remove.
 * @see decomp.me (100%)
 */
void gosub_delete_packed_record(s32 record_index)
{
    s32 shift_index;

    for (shift_index = record_index; shift_index < GOSUB_LOGIC_BLOCK_COUNT - 1; shift_index++)
    {
        gosub_copy_packed_record(&GOSUB_LOGIC_BLOCK_RECORDS[shift_index],
                                 &GOSUB_LOGIC_BLOCK_RECORDS[shift_index + 1]);
    }
    GOSUB_LOGIC_BLOCK_COUNT--;
}

/**
 * @brief Delete one row from g_gosub_rows and renumber the rows above it.
 *
 * Rows above @p row are shifted down one slot and g_gosub_row_count is
 * decremented; every surviving row whose index still points past @p row has
 * that index pulled down by one so it keeps naming the same record.
 *
 * @param row Index of the row to remove.
 * @see decomp.me (100%)
 */
void gosub_delete_list_row(s32 row)
{
    s32 i;

    for (i = row; i < g_gosub_row_count - 1; i++)
    {
        gosub_copy_list_row(&g_gosub_rows[i], &g_gosub_rows[i + 1]);
    }
    g_gosub_row_count--;
    for (i = 0; i < g_gosub_row_count; i++)
    {
        if (row < g_gosub_rows[i].index)
        {
            g_gosub_rows[i].index--;
        }
    }
}

/**
 * @brief Copy one 4-byte record.
 *
 * @param dst Destination record.
 * @param src Source record.
 * @see decomp.me (100%)
 */
inline void gosub_copy_packed_record(void* dst, void* src)
{
    u8* dst_bytes;
    u8* src_bytes;
    u32 byte_index;

    dst_bytes = (u8*)dst;
    src_bytes = (u8*)src;
    for (byte_index = 0; byte_index < sizeof(GosubPackedRecord);)
    {
        byte_index++;
        *dst_bytes = *src_bytes;
        src_bytes += 1;
        dst_bytes += 1;
    }
}

/**
 * @brief Copy one 0x20-byte GosubListRow.
 *
 * @param dst Destination row.
 * @param src Source row.
 * @see decomp.me (100%)
 */
inline void gosub_copy_list_row(void* dst, void* src)
{
    u8* dst_bytes;
    u8* src_bytes;
    u32 byte_index;

    dst_bytes = (u8*)dst;
    src_bytes = (u8*)src;
    for (byte_index = 0; byte_index < sizeof(GosubListRow);)
    {
        byte_index++;
        *dst_bytes = *src_bytes;
        src_bytes += 1;
        dst_bytes += 1;
    }
}

/**
 * @brief Sort the gosub row list, carrying each row's backing record with it.
 *
 * An insertion sort first builds a row permutation. The packed records and
 * display rows are then snapshotted and rewritten through that permutation.
 * Rebuilding the list last refreshes its derived names and fields.
 *
 * @param sort_mode Encoded type, power, or shape key and sort direction.
 *
 * @see decomp.me (100%)
 */
void gosub_sort_rows(s32 sort_mode)
{
    GosubSortWorkspace workspace;
    s32 row_index;
    s32 insertion_index;
    s32 shift_index;

    for (row_index = 0; row_index < g_gosub_row_count; row_index++)
    {
        for (insertion_index = 0; insertion_index < row_index; insertion_index++)
        {
            if (gosub_compare_rows(sort_mode, row_index, workspace.row_order[insertion_index]) == 0)
            {
                break;
            }
        }
        if (insertion_index != row_index)
        {
            for (shift_index = row_index; shift_index > insertion_index; shift_index--)
            {
                workspace.row_order[shift_index] = workspace.row_order[shift_index - 1];
            }
        }
        workspace.row_order[insertion_index] = row_index;
    }

    bcopy(GOSUB_LOGIC_BLOCK_RECORDS, workspace.packed_records, sizeof(workspace.packed_records));
    bcopy(g_gosub_rows, workspace.rows, sizeof(workspace.rows));

    for (row_index = 0; row_index < g_gosub_row_count; row_index++)
    {
        gosub_copy_packed_record(&GOSUB_LOGIC_BLOCK_RECORDS[row_index],
                                 &workspace.packed_records[workspace.row_order[row_index]]);
        gosub_copy_list_row(&g_gosub_rows[row_index], &workspace.rows[workspace.row_order[row_index]]);
    }

    gosub_build_packed_record_list();
}

/**
 * @brief Compare two logic-block rows by type, power, or shape.
 *
 * @param mode Low nibble selects the key; a nonzero high nibble swaps the operands.
 * @param left_row_index  First row index before the optional direction swap.
 * @param right_row_index Second row index before the optional direction swap.
 * @return 1 when the left operand sorts before the right, otherwise 0.
 * @see decomp.me (100%)
 */
s32 gosub_compare_rows(s32 mode, s32 left_row_index, s32 right_row_index)
{
    s32 swapped_row_index;

    if (mode & GOSUB_SORT_ASCENDING_MASK)
    {
        swapped_row_index = left_row_index;
        left_row_index = right_row_index;
        right_row_index = swapped_row_index;
    }
    switch (mode & GOSUB_SORT_KEY_MASK)
    {
    case GOSUB_SORT_BY_TYPE:
        if (g_gosub_rows[left_row_index].detail_group < g_gosub_rows[right_row_index].detail_group)
        {
            return 1;
        }
        break;
    case GOSUB_SORT_BY_POWER:
        if (g_gosub_rows[left_row_index].detail_id < g_gosub_rows[right_row_index].detail_id)
        {
            return 1;
        }
        break;
    case GOSUB_SORT_BY_SHAPE:
        if (g_gosub_rows[left_row_index].detail_variant < g_gosub_rows[right_row_index].detail_variant)
        {
            return 1;
        }
        break;
    }
    return 0;
}

/**
 * @brief Upload the gosub font CLUT and texture strip to their fixed VRAM slots.
 *
 * @note The 0x200-byte texture transfer continues through the first 0x5C bytes
 *       of g_gosub_item_metadata; its live metadata begins at index 0x60.
 * @see decomp.me (100%)
 */
void gosub_upload_font_texture(void)
{
    RECT upload_rect;

    setRECT(&upload_rect, GOSUB_FONT_CLUT_X, GOSUB_FONT_CLUT_Y,
            GOSUB_FONT_CLUT_WIDTH, GOSUB_FONT_CLUT_HEIGHT);
    LoadImage(&upload_rect, (u_long*)g_gosub_font_texture);

    setRECT(&upload_rect, GOSUB_FONT_TEXTURE_X, GOSUB_FONT_TEXTURE_Y,
            GOSUB_FONT_TEXTURE_WIDTH, GOSUB_FONT_TEXTURE_HEIGHT);
    LoadImage(&upload_rect, (u_long*)(g_gosub_font_texture + GOSUB_FONT_TEXTURE_DATA_OFFSET));
    DrawSync(0);
}

/**
 * @brief Emit the four corner sprites of a gosub panel frame and close the run
 *        with a texture-page packet.
 *
 * The corners sit two pixels outside (@p x, @p y) and five pixels inside the
 * far edge; the top pair uses texture row 0xF0 and the bottom pair 0xF8.
 *
 * @param prim Packet cursor; four sprites and one draw-mode packet are written.
 * @param ot   Ordering-table tag every packet is linked into.
 * @param x    Panel left edge.
 * @param y    Panel top edge.
 * @param w    Panel width.
 * @param h    Panel height.
 * @return Packet cursor past the draw-mode packet.
 * @see decomp.me (100%)
 */
GosubGpuPacket* gosub_emit_panel_corners(SPRT* prim, s32* ot, s32 x, s32 y, s32 w, s32 h)
{
    DR_TPAGE* draw_tpage;
    s32 x0;
    s32 y0;
    s32 x1;
    s32 y1;

    x0 = x - GOSUB_PANEL_CORNER_OUTSET;
    y0 = y - GOSUB_PANEL_CORNER_OUTSET;

    SET_BGR0_PACKED(prim, GPU_TINT_NEUTRAL);
    setSprt(prim);
    setXY0(prim, x0, y0);
    setUV0(prim, 0, GOSUB_PANEL_CORNER_TEXTURE_V);
    setWH(prim, GOSUB_PANEL_CORNER_SIZE, GOSUB_PANEL_CORNER_SIZE);
    setClut(prim, GOSUB_FONT_CLUT_X, GOSUB_FONT_CLUT_Y);
    addPrim(ot, prim);
    prim += 1;

    x1 = x + w - GOSUB_PANEL_CORNER_FAR_INSET;
    y1 = y + h - GOSUB_PANEL_CORNER_FAR_INSET;

    SET_BGR0_PACKED(prim, GPU_TINT_NEUTRAL);
    setSprt(prim);
    setXY0(prim, x1, y0);
    setUV0(prim, GOSUB_PANEL_CORNER_SIZE, GOSUB_PANEL_CORNER_TEXTURE_V);
    setWH(prim, GOSUB_PANEL_CORNER_SIZE, GOSUB_PANEL_CORNER_SIZE);
    setClut(prim, GOSUB_FONT_CLUT_X, GOSUB_FONT_CLUT_Y);
    addPrim(ot, prim);
    prim += 1;

    SET_BGR0_PACKED(prim, GPU_TINT_NEUTRAL);
    setSprt(prim);
    setXY0(prim, x0, y1);
    setUV0(prim, 0, GOSUB_PANEL_CORNER_TEXTURE_V + GOSUB_PANEL_CORNER_SIZE);
    setWH(prim, GOSUB_PANEL_CORNER_SIZE, GOSUB_PANEL_CORNER_SIZE);
    setClut(prim, GOSUB_FONT_CLUT_X, GOSUB_FONT_CLUT_Y);
    addPrim(ot, prim);
    prim += 1;

    SET_BGR0_PACKED(prim, GPU_TINT_NEUTRAL);
    setSprt(prim);
    setXY0(prim, x1, y1);
    setUV0(prim, GOSUB_PANEL_CORNER_SIZE, GOSUB_PANEL_CORNER_TEXTURE_V + GOSUB_PANEL_CORNER_SIZE);
    setWH(prim, GOSUB_PANEL_CORNER_SIZE, GOSUB_PANEL_CORNER_SIZE);
    setClut(prim, GOSUB_FONT_CLUT_X, GOSUB_FONT_CLUT_Y);
    addPrim(ot, prim);
    prim += 1;

    draw_tpage = (DR_TPAGE*)prim;
    setDrawTPage(draw_tpage, 0, 0, GOSUB_FONT_TPAGE);
    addPrim(ot, draw_tpage);
    return (GosubGpuPacket*)(draw_tpage + 1);
}
