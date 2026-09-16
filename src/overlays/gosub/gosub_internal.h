#ifndef GOSUB_INTERNAL_H
#define GOSUB_INTERNAL_H

#include "common.h"
#include "gpu_packet.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
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

/** @brief Positions and color used by the equipment detail line. */
#define GOSUB_EQUIPMENT_DETAIL_LABEL_X 0x10
#define GOSUB_EQUIPMENT_DETAIL_Y 0x12
#define GOSUB_WEAPON_POWER_X 0x68
#define GOSUB_ARMOR_DEFENSE_X 0x60
#define GOSUB_INSTRUMENT_POWER_X 0x38
#define GOSUB_INSTRUMENT_EFFECT_X 0x60

/** @brief Position of the centered current-row description. */
#define GOSUB_ROW_DESCRIPTION_X 0x84
#define GOSUB_ROW_DESCRIPTION_Y 2

/** @brief Position of caller-provided text within a message dialog. */
#define GOSUB_MESSAGE_DIALOG_TEXT_X 0x80
#define GOSUB_MESSAGE_DIALOG_TEXT_Y 2

/** @brief Message offset and position of the equipment detail header. */
#define GOSUB_DETAIL_HEADER_MESSAGE_OFFSET (-0x18)
#define GOSUB_DETAIL_HEADER_X 0x84
#define GOSUB_DETAIL_HEADER_Y 2

/** @brief Message offsets and layout of the fixed two-line header. */
#define GOSUB_TWO_LINE_HEADER_FIRST_MESSAGE_OFFSET (-0x1C)
#define GOSUB_TWO_LINE_HEADER_SECOND_MESSAGE_OFFSET (-0x1A)
#define GOSUB_TWO_LINE_HEADER_X 0x84
#define GOSUB_TWO_LINE_HEADER_FIRST_Y 2
#define GOSUB_TWO_LINE_HEADER_SECOND_Y 0x12

/** @brief Message offsets and layout of the two-choice confirmation prompt. */
#define GOSUB_CONFIRMATION_TITLE_MESSAGE_OFFSET (-0x14)
#define GOSUB_CONFIRMATION_FIRST_CHOICE_MESSAGE_OFFSET (-0x20)
#define GOSUB_CONFIRMATION_SECOND_CHOICE_MESSAGE_OFFSET (-0x1E)
#define GOSUB_CONFIRMATION_CHOICE_MASK 1
#define GOSUB_CONFIRMATION_TITLE_X 0x80
#define GOSUB_CONFIRMATION_FIRST_CHOICE_X 0x78
#define GOSUB_CONFIRMATION_SECOND_CHOICE_X 0x88
#define GOSUB_CONFIRMATION_TITLE_Y 2
#define GOSUB_CONFIRMATION_CHOICE_Y 0x12

/** @brief Message offsets and layout of the sort-key dialog. */
#define GOSUB_SORT_DIALOG_TYPE_MESSAGE_OFFSET (-0xE)
#define GOSUB_SORT_DIALOG_POWER_MESSAGE_OFFSET (-0xC)
#define GOSUB_SORT_DIALOG_SHAPE_MESSAGE_OFFSET (-0xA)
#define GOSUB_SORT_DIALOG_X 0x40
#define GOSUB_SORT_DIALOG_TYPE_Y 2
#define GOSUB_SORT_DIALOG_POWER_Y 0x12
#define GOSUB_SORT_DIALOG_SHAPE_Y 0x22

/** @brief Message offsets and layout of the row-action dialog. */
#define GOSUB_ROW_ACTION_SORT_MESSAGE_OFFSET (-0x12)
#define GOSUB_ROW_ACTION_DELETE_MESSAGE_OFFSET (-0x10)
#define GOSUB_ROW_ACTION_CHOICE_MASK 1
#define GOSUB_ROW_ACTION_DIALOG_X 0x40
#define GOSUB_ROW_ACTION_SORT_Y 2
#define GOSUB_ROW_ACTION_DELETE_Y 0x12

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

/** @brief Equipment categories encoded in the row's equipment_kind field. */
typedef enum
{
    GOSUB_EQUIPMENT_KIND_WEAPON = 0,
    GOSUB_EQUIPMENT_KIND_ARMOR = 1,
    GOSUB_EQUIPMENT_KIND_INSTRUMENT = 2
} GosubEquipmentKind;

/** @brief Text palette values used by the gosub UI. */
typedef enum
{
    GOSUB_TEXT_COLOR_NORMAL = 4,
    GOSUB_TEXT_COLOR_DISABLED = 5
} GosubTextColor;

/** @brief Horizontal alignment modes accepted by the text renderer. */
typedef enum
{
    GOSUB_TEXT_ALIGN_LEFT = 0,
    GOSUB_TEXT_ALIGN_RIGHT = 1,
    GOSUB_TEXT_ALIGN_CENTER = 2
} GosubTextAlignment;

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

/** @brief Little-endian text offset stored as two independently loaded bytes. */
typedef struct
{
    u8 low;
    u8 high;
} GosubEncodedTextOffset;

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
extern GosubEncodedTextOffset D_800EC3EE;
extern GosubEncodedTextOffset D_800EC3F0;
extern GosubEncodedTextOffset D_800EC3F2;
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
#define GOSUB_MSG_ABS(base, off)                                                                                                                               \
    ((void*)(g_gosub_message_archive_offset + ((base) + *(u16*)((u8*)&g_gosub_message_archive_offset + g_gosub_message_archive_offset + (off)))))
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
void gosub_open_message_dialog(u8* message_text);
s32 gosub_draw_message_dialog(s32* ordering_table, s32 packet_cursor, s32 x_offset, s32 y_offset);
s32 gosub_draw_detail_header(s32* ordering_table, s32 packet_cursor, s32 x_offset, s32 y_offset);
s32 gosub_draw_two_line_header(s32* ordering_table, s32 packet_cursor, s32 x_offset, s32 y_offset);
s32 gosub_draw_confirmation_prompt(s32* ordering_table, s32 packet_cursor, s32 x_offset, s32 y_offset);
s32 gosub_draw_row_description(s32* ordering_table, s32 packet_cursor, s32 x_offset, s32 y_offset);
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
s32 gosub_draw_equipment_details(s32 packet_cursor, s32* ordering_table, s32 x_offset, s32 y_offset);
s32 gosub_draw_composite_icon(s32 initial_packet, s32* ordering_table, s32 x, s32 y, s32 icon_id, s32 layout_index);
s32 gosub_draw_combination_preview();
s32 gosub_handle_backtrack_dialog();
s32 gosub_handle_delete_dialog(s32 dialog_result);
s32 gosub_draw_title();
s32 gosub_draw_two_option_dialog(s32* ordering_table, s32 initial_packet, s32 x_offset, s32 y_offset);
s32 gosub_draw_three_option_dialog(s32* ordering_table, s32 initial_packet, s32 x_offset, s32 y_offset);
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


/* Overlay state shared across the recovered translation units. */
extern s32 g_gosub_frame_parity;
extern s32 g_gosub_finished;
extern s32 g_gosub_cursor_row;
extern s32 g_gosub_row_count;
extern s32 g_gosub_visible_row_count;
extern u8 g_gosub_screen_sequence_index;
extern s32 g_gosub_scroll_frames_remaining;
extern s32 g_gosub_combination_variant;
extern s32 g_gosub_dialog_choice;
extern s32 g_gosub_combination_quantity;
extern s32 g_gosub_allow_duplicate_selection;
extern u8* g_gosub_dialog_text;
extern s32 (*g_gosub_finish_handler)();
extern u8 g_gosub_selection_mode;
extern u8 g_gosub_required_selection_count;
extern s32 g_gosub_show_row_details;
extern s32 g_gosub_result_rows[16];
extern s32 g_gosub_dialog_accepting_input;
extern u8 g_gosub_selected_rows[4];
extern s32 g_gosub_window_height;
extern s32 (*g_gosub_select_handler)();
extern s32 g_gosub_window_width;
extern s32 g_gosub_suppress_dialog_sound;
extern u8 g_gosub_text_buffers[0x5000];
extern u8 g_gosub_selection_count;
extern u8 g_gosub_screen_sequence[20];
extern s32 g_gosub_combination_result_id;
extern s32 g_gosub_row_height;
extern s32 g_gosub_scroll_y;
extern s32 g_gosub_sort_ascending;
extern s32 g_gosub_scroll_target_y;
extern u8* g_gosub_title_text;
extern GosubElement g_gosub_elements[1];
extern GosubElement g_gosub_dynamic_elements[GOSUB_ELEMENT_COUNT - 1];
extern GosubListRow g_gosub_rows[512];
extern s32 (*g_gosub_dialog_handler)(s32);

#endif
