#ifndef GOSUB_INTERNAL_H
#define GOSUB_INTERNAL_H

#include "common.h"
#include "gpu_packet.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "sdk/memory.h"
#include "tim.h"
#include "main.h"
#include "field_ui_text.h"
#include "encoded_text.h"
#include "display.h"
#include "pad.h"
#include "golem_shape.h"

typedef struct GosubTilePacket GosubTilePacket;

/* Overlay constants. */

/** @brief Number of entries in the gosub UI element pool. */
#define GOSUB_ELEMENT_COUNT 16

/** @brief Panel sizes and positions shared by the element builders. */
#define GOSUB_LIST_PANEL_WIDTH 232
#define GOSUB_LOGIC_BLOCK_PANEL_WIDTH 288
#define GOSUB_COMPANION_PANEL_WIDTH 280
#define GOSUB_TEXT_PANEL_X 28
#define GOSUB_TEXT_PANEL_WIDTH 264
#define GOSUB_TITLE_PANEL_Y 16
#define GOSUB_DESCRIPTION_PANEL_Y 176
#define GOSUB_ONE_LINE_PANEL_HEIGHT 20
#define GOSUB_TWO_LINE_PANEL_HEIGHT 36
#define GOSUB_THREE_LINE_PANEL_HEIGHT 52
#define GOSUB_DIALOG_PANEL_Y 112
#define GOSUB_MESSAGE_PANEL_X 32
#define GOSUB_MESSAGE_PANEL_WIDTH 256
#define GOSUB_CHOICE_PANEL_X 128
#define GOSUB_CHOICE_PANEL_WIDTH 128

/** @brief Space between a list's last row and the bottom of its panel. */
#define GOSUB_LIST_PANEL_PADDING 4

/** @brief Menu sound effects and their volume. */
#define GOSUB_SFX_CURSOR 0x7D
#define GOSUB_SFX_CANCEL 0x7F
#define GOSUB_SFX_VOLUME 0x80

/** @brief Buttons that confirm, cancel, and move the cursor. */
#define GOSUB_BUTTONS_CONFIRM (PAD_BTN_CROSS | PAD_BTN_L3)
#define GOSUB_BUTTONS_DISMISS (GOSUB_BUTTONS_CONFIRM | PAD_BTN_CIRCLE)
#define GOSUB_BUTTON_CANCEL PAD_BTN_CIRCLE
#define GOSUB_BUTTON_FINISH PAD_BTN_START
#define GOSUB_BUTTONS_PREVIOUS (PAD_BTN_UP | PAD_BTN_LEFT)
#define GOSUB_BUTTONS_NEXT (PAD_BTN_DOWN | PAD_BTN_RIGHT)
#define GOSUB_BUTTON_PAGE_UP PAD_BTN_L1
#define GOSUB_BUTTON_PAGE_DOWN PAD_BTN_R1

/** @brief Frames a list takes to scroll to its new position. */
#define GOSUB_SCROLL_FRAMES 4

/** @brief Number of choices the dialog cursor cycles through. */
#define GOSUB_DIALOG_CHOICE_COUNT 12

/** @brief Item kind ranges (SavedGameLayout::item_counts) listed by the item screens. */
#define GOSUB_PRIMARY_MATERIAL_FIRST 0x00
#define GOSUB_PRIMARY_MATERIAL_END 0x40
#define GOSUB_SECONDARY_MATERIAL_FIRST 0x40
#define GOSUB_SECONDARY_MATERIAL_END 0xFF
#define GOSUB_ELEMENTAL_COIN_FIRST 0x40
#define GOSUB_ELEMENTAL_COIN_END 0x50
#define GOSUB_COLOR_MATERIAL_FIRST 0x60
#define GOSUB_COLOR_MATERIAL_END 0x85
#define GOSUB_PRODUCE_FIRST 0x60
#define GOSUB_PRODUCE_END 0x90

/** @brief GosubListRow::value of rows that show no count, and of the special row layouts. */
#define GOSUB_ROW_NO_COUNT (-1)
#define GOSUB_ROW_LOGIC_BLOCK (-2)
#define GOSUB_ROW_COMPANION (-3)

/** @brief Row and window layout of the plain item lists. */
#define GOSUB_ITEM_ROW_HEIGHT 16
#define GOSUB_ITEM_VISIBLE_ROWS 8
#define GOSUB_ITEM_WINDOW_HEIGHT (GOSUB_ITEM_ROW_HEIGHT * GOSUB_ITEM_VISIBLE_ROWS + GOSUB_LIST_PANEL_PADDING)

/** @brief Scratch text buffers for rows whose text is assembled at run time. */
#define GOSUB_TEXT_BUFFER_COUNT 256
#define GOSUB_TEXT_BUFFER_SIZE 0x50

/** @brief Companion portraits: 48x48 4-bit images uploaded to one of five slots per frame buffer. */
#define GOSUB_PORTRAIT_SIZE 48
#define GOSUB_PORTRAIT_SLOTS 5
#define GOSUB_PORTRAIT_VRAM_X 0x140
#define GOSUB_PORTRAIT_VRAM_WIDTH (GOSUB_PORTRAIT_SIZE / 4)
#define GOSUB_PORTRAIT_CLUT_SIZE 16

/** @brief Offsets of a portrait's pixels and CLUT from its g_gosub_portrait_archive entry. */
#define GOSUB_PORTRAIT_PIXEL_OFFSET 0x1C
#define GOSUB_PORTRAIT_CLUT_OFFSET (-4)

/** @brief Portrait index of the first egg portrait; an egg adds its species. */
#define GOSUB_EGG_PORTRAIT_FIRST 0x48

/** @brief Portrait index of the first golem portrait; a golem adds its type. */
#define GOSUB_GOLEM_PORTRAIT_FIRST 0x41

/** @brief Egg hatch counter values below which an egg hatches any time, or is almost ready. */
#define GOSUB_EGG_ANY_TIME_BELOW 6
#define GOSUB_EGG_ALMOST_READY_BELOW 31

/** @brief Egg hatch states stored in a row's detail_variant; they follow GOSUB_MSG_HATCH_ANY_TIME. */
#define GOSUB_EGG_HATCH_ANY_TIME 0
#define GOSUB_EGG_HATCH_ALMOST_READY 1
#define GOSUB_EGG_HATCH_NEEDS_TIME 2

/** @brief Control entries embedded in a gosub screen sequence. */
#define GOSUB_SCREEN_SEQUENCE_END 0xFE
#define GOSUB_SCREEN_SEQUENCE_DIALOG 0xFF

/** @brief Fields in the encoded sort mode passed to gosub_sort_logic_blocks. */
#define GOSUB_SORT_KEY_MASK 0x0F
#define GOSUB_SORT_ASCENDING_MASK 0xF0
#define GOSUB_SORT_ASCENDING_SHIFT 7
#define GOSUB_SORT_ORDER_CAPACITY 0x100
#define GOSUB_SORT_ROW_CAPACITY 0x28

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
#define GOSUB_COMPOSITE_ICON_BASE_GLYPH_OFFSET 0x13
#define GOSUB_COMPOSITE_ICON_BASE_CELL_SIZE 8
#define GOSUB_COMPOSITE_ICON_PART_CELL_SIZE 16
#define GOSUB_COMPOSITE_ICON_BASE_CLUT 9

/** @brief VRAM column of the UI image (panel corners, glyphs and portrait slots). */
#define GOSUB_UI_IMAGE_X 0x140

/**
 * @brief Positions and color used by the equipment detail line.
 * @note JP moves the weapon and instrument power columns.
 */
#define GOSUB_EQUIPMENT_DETAIL_LABEL_X 0x10
#define GOSUB_EQUIPMENT_DETAIL_Y 0x12
#if defined(VERSION_JP)
#define GOSUB_WEAPON_POWER_X 0x40
#else
#define GOSUB_WEAPON_POWER_X 0x68
#endif
#define GOSUB_ARMOR_DEFENSE_X 0x60
#if defined(VERSION_JP)
#define GOSUB_INSTRUMENT_POWER_X 0x30
#else
#define GOSUB_INSTRUMENT_POWER_X 0x38
#endif
#define GOSUB_INSTRUMENT_EFFECT_X 0x60

/**
 * @brief Columns of the third line of an equipment card row in the item list:
 *        secondary value, the label after it, then the primary value.
 * @note JP lays the line out differently.
 */
#if defined(VERSION_JP)
#define GOSUB_CARD_SECONDARY_VALUE_X 0x54
#define GOSUB_CARD_VALUE_LABEL_X 0x78
#define GOSUB_CARD_PRIMARY_VALUE_X 0x9C
#else
#define GOSUB_CARD_SECONDARY_VALUE_X 0x48
#define GOSUB_CARD_VALUE_LABEL_X 0x64
#define GOSUB_CARD_PRIMARY_VALUE_X 0xB0
#endif

/** @brief Position of the centered current-row description. */
#define GOSUB_ROW_DESCRIPTION_X 0x84
#define GOSUB_ROW_DESCRIPTION_Y 2

/** @brief Position of caller-provided text within a message dialog. */
#define GOSUB_MESSAGE_DIALOG_TEXT_X 0x80
#define GOSUB_MESSAGE_DIALOG_TEXT_Y 2

/** @brief Position of the equipment detail header. */
#define GOSUB_DETAIL_HEADER_X 0x84
#define GOSUB_DETAIL_HEADER_Y 2

/** @brief Layout of the fixed two-line header. */
#define GOSUB_TWO_LINE_HEADER_X 0x84
#define GOSUB_TWO_LINE_HEADER_FIRST_Y 2
#define GOSUB_TWO_LINE_HEADER_SECOND_Y 0x12

/** @brief Layout of the two-choice confirmation prompt. */
#define GOSUB_CONFIRMATION_CHOICE_MASK 1
#define GOSUB_CONFIRMATION_TITLE_X 0x80
#define GOSUB_CONFIRMATION_FIRST_CHOICE_X 0x78
#define GOSUB_CONFIRMATION_SECOND_CHOICE_X 0x88
#define GOSUB_CONFIRMATION_TITLE_Y 2
#define GOSUB_CONFIRMATION_CHOICE_Y 0x12

/** @brief Layout of the sort-key dialog. */
#define GOSUB_SORT_DIALOG_X 0x40
#define GOSUB_SORT_DIALOG_TYPE_Y 2
#define GOSUB_SORT_DIALOG_POWER_Y 0x12
#define GOSUB_SORT_DIALOG_SHAPE_Y 0x22

/** @brief Layout of the row-action dialog. */
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
            u32 y : 8;
            u32 width_low : 8;
        } f;
    } attr;
    union
    {
        u32 word;
        struct
        {
            u32 width_high : 1;
            u32 height : 8;
            u32 reserved_9 : 23;
        } f;
    } geometry;
    void* draw_handler;
} GosubElement;

/**
 * @brief Display and selection data for one list row.
 * @note The detail fields depend on the list: a logic block keeps its id in
 *       detail_group, its level in detail_id and its shape in detail_variant;
 *       a pet or golem its species or type (the portrait) in detail_id, its
 *       level, palette or hatch state in detail_variant, and whether it is on
 *       the field in detail_group.
 */
typedef struct
{
    u8* name;
    u8* desc;
    /** @brief Count shown at the right of the row, or GOSUB_ROW_NO_COUNT / a special row layout. */
    s16 value;
    /** @brief Item kind, inventory index, block index or record index the row stands for. */
    s16 index;
    s32 detail_id : 8;
    s32 detail_variant : 8;
    s32 detail_group : 8;
    u32 text_color : 4;
    /** @brief GosubEquipmentKind of an equipment row. */
    u32 equipment_kind : 4;
    /** @brief Attack power (weapon, golem), instrument power or pet HP value shown by the row. */
    u16 primary_value;
    /** @brief Armor defense values; an instrument's spell index in stats[0]. */
    u16 stats[4];
    u16 secondary_value;
    union
    {
        /** @brief Companion list rows. */
        struct
        {
            u32 egg : 1;     /**< The pet has not hatched; also tested through @c half. */
            u32 grazing : 1; /**< The pet is grazing and cannot be taken along. */
            u32 pet : 1;     /**< A pet row; clear for a golem row. */
            u32 reserved_3 : 29;
        } companion;
        /** @brief Logic-block list rows. */
        struct
        {
            u32 reserved_0 : 2;
            u32 in_use : 1; /**< The block is placed or owned by a golem and cannot be discarded. */
            u32 reserved_3 : 29;
        } block;
        u16 half;
        u32 word;
    } flags;
} GosubListRow;

/**
 * @brief Equipment categories (InventoryAttributes bits 9:8), as stored in a
 *        row's equipment_kind field and passed to gosub_build_equipment_list.
 */
typedef enum
{
    GOSUB_EQUIPMENT_KIND_WEAPON = 0,
    GOSUB_EQUIPMENT_KIND_ARMOR = 1,
    GOSUB_EQUIPMENT_KIND_INSTRUMENT = 2,
    /** @brief gosub_build_equipment_list filter: every category. */
    GOSUB_EQUIPMENT_KIND_ANY = 3,
    /** @brief gosub_build_equipment_list filter: weapons and armor (golem parts). */
    GOSUB_EQUIPMENT_KIND_GOLEM_PARTS = 4
} GosubEquipmentKind;

/** @brief Golem parts: one weapon and this many pieces of armor. */
#define GOSUB_GOLEM_ARMOR_PARTS 3

/** @brief First entry of each equipment category in GOSUB_TEXT_EQUIPMENT_TYPES. */
#define GOSUB_WEAPON_TYPE_FIRST 0
#define GOSUB_ARMOR_TYPE_FIRST 11
#define GOSUB_INSTRUMENT_TYPE_FIRST 23

/** @brief Which companions gosub_build_companion_list lists. */
typedef enum
{
    GOSUB_COMPANIONS_ALL = 0,
    GOSUB_COMPANIONS_PETS = 1,
    GOSUB_COMPANIONS_GOLEMS = 2
} GosubCompanionFilter;

/** @brief Screens a GOSUB screen sequence can name. */
typedef enum
{
    GOSUB_SCREEN_PRIMARY_MATERIAL = 0,
    GOSUB_SCREEN_SECONDARY_MATERIAL = 1,
    GOSUB_SCREEN_WEAPON = 2,
    GOSUB_SCREEN_ARMOR = 3,
    GOSUB_SCREEN_INSTRUMENT = 4,
    GOSUB_SCREEN_EQUIPMENT = 5,
    GOSUB_SCREEN_WEAPON_TYPE = 6,
    GOSUB_SCREEN_ARMOR_TYPE = 7,
    GOSUB_SCREEN_INSTRUMENT_TYPE = 8,
    GOSUB_SCREEN_GOLEM_PARTS = 9,
    GOSUB_SCREEN_BLOCK_COMPONENTS = 10,
    GOSUB_SCREEN_LOGIC_BLOCKS = 11,
    /** @brief Pet or golem to take along; eggs and grazing pets are refused. */
    GOSUB_SCREEN_COMPANION_TO_TAKE = 12,
    /** @brief Pet to take along; eggs and grazing pets are refused. */
    GOSUB_SCREEN_PET_TO_TAKE = 13,
    GOSUB_SCREEN_GOLEM = 14,
    GOSUB_SCREEN_COLOR_MATERIAL = 15,
    GOSUB_SCREEN_ELEMENTAL_COIN = 16,
    GOSUB_SCREEN_COMPANION = 17,
    GOSUB_SCREEN_PET = 18,
    GOSUB_SCREEN_PRODUCE = 19
} GosubScreen;

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

/**
 * @brief TILE-shaped GPU packet: the element renderer's packet cursor type,
 *        also used for the scroll bar and the cursor and selection highlights.
 */
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
    GosubTilePacket* packet_cursor;
} GosubRenderContext;

/** @brief Draw callback installed on a gosub element. */
typedef GosubTilePacket* (*GosubElementDrawHandler)();

/** @brief Unconnected flat-line GPU packet. */
typedef struct
{
    u_long tag;
    u_char r0;
    u_char g0;
    u_char b0;
    u_char code;
    s16 x0;
    s16 y0;
    s16 x1;
    s16 y1;
} GosubLinePacket;

/** @brief Stack workspace used to reorder the logic blocks and their rows. */
typedef struct
{
    u8 row_order[GOSUB_SORT_ORDER_CAPACITY];
    LogicBlock blocks[GOSUB_SORT_ROW_CAPACITY];
    GosubListRow rows[GOSUB_SORT_ROW_CAPACITY];
} GosubSortWorkspace;

/** @brief Sections of the overlay text archive (g_gosub_text_archive). */
typedef enum
{
    GOSUB_TEXT_ITEM_NAMES = 0,
    GOSUB_TEXT_ITEM_DESCRIPTIONS = 1,
    GOSUB_TEXT_EQUIPMENT_TYPES = 2,
    GOSUB_TEXT_SPIRIT_NAMES = 3,
    GOSUB_TEXT_SPIRIT_DESCRIPTIONS = 4,
    GOSUB_TEXT_LOGIC_BLOCK_NAMES = 5,
    GOSUB_TEXT_LOGIC_BLOCK_DESCRIPTIONS = 6,
    GOSUB_TEXT_MESSAGES = 7,
    GOSUB_TEXT_PET_SPECIES = 8,
    GOSUB_TEXT_GOLEM_TYPES = 9,
    GOSUB_TEXT_INSTRUMENT_SPELLS = 10,
    GOSUB_TEXT_COLOR_NAMES = 11,
    GOSUB_TEXT_SECTION_COUNT = 12
} GosubTextSection;

/** @brief Strings of the GOSUB_TEXT_MESSAGES section. */
typedef enum
{
    GOSUB_MSG_YES = 0,
    GOSUB_MSG_NO = 1,
    GOSUB_MSG_SELECT_GOLEM_PARTS = 2,
    GOSUB_MSG_PRESS_START_TO_SELECT = 3,
    GOSUB_MSG_CHOOSE_BLOCK_COMPONENTS = 4,
    GOSUB_MSG_MAKE_BLOCK = 5,
    GOSUB_MSG_IS_THIS_OKAY = 6,
    GOSUB_MSG_SORT = 7,
    GOSUB_MSG_DISCARD = 8,
    GOSUB_MSG_SORT_BY_TYPE = 9,
    GOSUB_MSG_SORT_BY_POWER = 10,
    GOSUB_MSG_SORT_BY_SHAPE = 11,
    GOSUB_MSG_ASSIGN_BLOCK = 12,
    GOSUB_MSG_NO_LOGIC_BLOCKS = 13,
    GOSUB_MSG_LOGIC_BLOCKS_FULL = 14,
    GOSUB_MSG_NO_GOLEM_PARTS = 15,
    GOSUB_MSG_NO_EQUIPMENT = 16,
    GOSUB_MSG_NO_WEAPONS = 17,
    GOSUB_MSG_NO_ARMOR = 18,
    GOSUB_MSG_NO_INSTRUMENTS = 19,
    GOSUB_MSG_NO_PRIMARY_MATERIAL = 20,
    GOSUB_MSG_NO_SECONDARY_MATERIAL = 21,
    GOSUB_MSG_CHOOSE_WEAPON = 22,
    GOSUB_MSG_CHOOSE_ARMOR = 23,
    GOSUB_MSG_CHOOSE_INSTRUMENT = 24,
    GOSUB_MSG_CHOOSE_PRIMARY_MATERIAL = 25,
    GOSUB_MSG_CHOOSE_SECONDARY_MATERIAL = 26,
    GOSUB_MSG_CHOOSE_EQUIPMENT = 27,
    GOSUB_MSG_GOLEM_LOGIC_BLOCKS = 28,
    GOSUB_MSG_CHOOSE_WEAPON_TYPE = 29,
    GOSUB_MSG_CHOOSE_ARMOR_TYPE = 30,
    GOSUB_MSG_CHOOSE_INSTRUMENT_TYPE = 31,
    GOSUB_MSG_IN_USE = 32,
    GOSUB_MSG_IN_USE_CANNOT_DISCARD = 33,
    GOSUB_MSG_LEVEL = 34,
    GOSUB_MSG_HP = 35,
    GOSUB_MSG_ATTACK_POWER = 36,
    GOSUB_MSG_ON_FIELD = 37,
    GOSUB_MSG_CHOOSE_PET_OR_GOLEM = 38,
    GOSUB_MSG_CHOOSE_PET = 39,
    GOSUB_MSG_CHOOSE_GOLEM = 40,
    GOSUB_MSG_NO_PETS_OR_GOLEMS = 41,
    GOSUB_MSG_NO_PETS = 42,
    GOSUB_MSG_NO_GOLEMS = 43,
    GOSUB_MSG_CHOOSE_ELEMENTAL_COIN = 44,
    GOSUB_MSG_COLOR_MATERIAL = 45,
    GOSUB_MSG_NO_ELEMENTAL_COINS = 46,
    GOSUB_MSG_NO_COLOR_MATERIAL = 47,
    GOSUB_MSG_MONSTER_EGG = 48,
    GOSUB_MSG_EGG_CANNOT_LEAVE = 49,
    GOSUB_MSG_HATCH_ANY_TIME = 50,
    GOSUB_MSG_HATCH_ALMOST_READY = 51,
    GOSUB_MSG_HATCH_NEEDS_TIME = 52,
    GOSUB_MSG_CHOOSE_PRODUCE = 53,
    GOSUB_MSG_NO_PRODUCE = 54,
    GOSUB_MSG_GRAZING = 55,
    GOSUB_MSG_CANNOT_LEAVE = 56
} GosubMessage;

/** @brief One value per equipment category (weapon, armor, instrument). */
typedef struct
{
    s32 values[3];
} GosubCategoryTable;

/**
 * @brief LINE_F4 packet used to draw an animated scroll marker.
 */
typedef struct
{
    u8 addr[3];
    u8 len;
    u8 r;
    u8 g;
    u8 b;
    u8 code;
    s16 x0;
    s16 y0;
    s16 x1;
    s16 y1;
    s16 x2;
    s16 y2;
    s16 x3;
    s16 y3;
    u32 mask;
} GosubScrollMarkerPacket;

/** @brief Flat triangle packet that fills a scroll marker's arrow head. */
typedef struct
{
    u8 addr[3];
    u8 len;
    u8 r;
    u8 g;
    u8 b;
    u8 code;
    s16 x0;
    s16 y0;
    s16 x1;
    s16 y1;
    s16 x2;
    s16 y2;
} GosubScrollFillPacket;

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
    u8 u0;
    u8 reserved_1;
    u8 v0;
    u8 reserved_3;
    u16 w;
    u16 h;
} GosubGlyphMetric;

/* External data. */

extern s32 g_field_gosub_state;
extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values[];
/**
 * @brief Overlay text archive: a section count word, one offset word per
 *        GosubTextSection, then the sections. Section offsets are relative
 *        to the archive start; each section begins with u16 string offsets
 *        relative to the section, followed by the encoded strings.
 */
extern u8 g_gosub_text_archive[];
/** @brief GOSUB_TEXT_COLOR_NAMES entry of each item kind; only the color materials use it. */
extern u8 g_gosub_item_colors[];
/** @brief First GOSUB_TEXT_EQUIPMENT_TYPES entry and number of types of each category. */
extern GosubCategoryTable g_gosub_equipment_type_firsts;
extern GosubCategoryTable g_gosub_equipment_type_counts;
extern s32 g_gosub_portrait_archive[];
extern TimPrefix g_gosub_image_archive;
extern GosubGlyphMetric g_gosub_glyph_metrics[];
extern u8 g_gosub_font_texture[];
/* FIELD UI string offset entries (see field_ui_text.h); each entry GOSUB reads is its own symbol. */
extern u8 D_800EC3DA[];
extern u8 D_800EC3E2[];
extern u8 D_800EC3EE[];
extern u8 D_800EC3F0[];
extern u8 D_800EC3F2[];
/** @brief Icon CLUT of each logic-block id; GOSUB reads each entry as one word. */
extern u32 g_golem_logic_block_icons[];

/* Typed access and helper macros. */

/** @brief Header word of text archive section @p section: its offset from the archive start. */
#define GOSUB_SECTION_OFFSET(section) (((u32*)g_gosub_text_archive)[1 + (section)])

/** @brief Address of string @p index of text archive section @p section. */
#define GOSUB_TEXT(section, index)                                                                                                                             \
    ((u8*)g_gosub_text_archive + GOSUB_SECTION_OFFSET(section) + *(u16*)((u8*)g_gosub_text_archive + GOSUB_SECTION_OFFSET(section) + (index) * 2))

/** @brief Address of UI message @p message (a GosubMessage). */
#define GOSUB_MESSAGE(message) GOSUB_TEXT(GOSUB_TEXT_MESSAGES, message)

/**
 * @brief Bit 8 of a 9-bit element width, for GosubElement::geometry.f.width_high.
 * @param width Element width.
 */
#define GOSUB_ELEMENT_WIDTH_HIGH(width) ((width) >> 8)

/**
 * @brief Store the low eight bits of a 9-bit element width in the top byte of GosubElement::attr.
 * @param element Element to update.
 * @param width Element width; bit 8 is stored separately with GOSUB_ELEMENT_WIDTH_HIGH.
 */
#define GOSUB_SET_ELEMENT_WIDTH_LOW(element, width) ((element)->attr.word = ((element)->attr.word & 0x00FFFFFF) | ((u32)((width) & 0xFF) << 24))

/* External and forward function declarations. */

/* FIELD exports with no project header. FIELD stays resident while GOSUB runs. */
void field_set_default_fade_target(void);
void field_restore_fade_target(void);
void field_reset_input_repeat(void);
void field_compact_inventory(void);
void play_menu_sfx(s32 sfx_id, s32 volume);
void field_format_number(u8* text, s32 value, s32 wide);
s32 equipment_combination_find(s32* record_indices, s32* quantity, s32* variant);
s32 field_draw_text(s32 prim, s32* ot, void* text, s32 color, s32 x, s32 y, s32 mode);
s32 field_draw_number(s32* ot, s32 prim, s32 value, s32 color, GosubTextPosition* position, s32 mode);
void gosub_load_screen_sequence(s32* screen_sequence);
void gosub_build_golem_parts_elements(void);
void gosub_build_block_components_elements(void);
void gosub_build_equipment_screen_elements(void);
void gosub_build_list_screen_elements(s32 include_middle);
void gosub_build_logic_block_list_elements(void);
void gosub_build_companion_list_elements(void);
void gosub_build_color_material_list(void);
void gosub_build_produce_list(void);
void gosub_build_elemental_coin_list(void);
void gosub_build_secondary_material_list(void);
void gosub_build_primary_material_list(void);
void gosub_build_logic_block_list(void);
void gosub_build_companion_list(s32 mode);
s32 gosub_select_companion_to_take(void);
s32 gosub_select_row(void);
s32 gosub_select_block_component(void);
s32 gosub_select_logic_block(void);
s32 gosub_select_golem_part(void);
s32 gosub_publish_block_components(void);
s32 gosub_handle_make_block_dialog(s32 dialog_result);
s32 gosub_publish_golem_parts(void);
s32 gosub_publish_selection(void);
s32 gosub_is_row_unselected(s32 row);
void gosub_build_equipment_list(u32 item_kind);
void gosub_build_equipment_type_list(s32 group);
void gosub_update_screen(GosubRenderContext* render_context);
void gosub_enter_screen(s32 screen_id);
s32 gosub_handle_input(void);
void gosub_scroll_to_cursor(void);
s32 gosub_toggle_cursor_selection(void);
s32 gosub_advance_screen_sequence(void);
s32 gosub_are_elements_idle(void);
void gosub_close_elements(void);
void gosub_clear_elements(void);
void gosub_render_elements(GosubRenderContext* render_context);
void gosub_update_and_render_elements(GosubRenderContext* render_context);
void gosub_open_message_dialog(u8* message_text);
s32 gosub_draw_message_dialog(s32* ordering_table, s32 packet_cursor, s32 x_offset, s32 y_offset);
s32 gosub_draw_block_components_header(s32* ordering_table, s32 packet_cursor, s32 x_offset, s32 y_offset);
s32 gosub_draw_golem_parts_header(s32* ordering_table, s32 packet_cursor, s32 x_offset, s32 y_offset);
s32 gosub_draw_confirmation_prompt(s32* ordering_table, s32 packet_cursor, s32 x_offset, s32 y_offset);
s32 gosub_draw_row_description(s32* ordering_table, s32 packet_cursor, s32 x_offset, s32 y_offset);
GosubElement* gosub_allocate_element(void);
void* gosub_emit_scroll_marker(GosubScrollMarkerPacket* prim, s32* ot, s32 x, s32 y, s32 flag);
GosubTilePacket* gosub_emit_panel(GosubTilePacket* prim, s32* ot, s32 x, s32 y, s32 w, s32 h, s32 flag);
GosubLinePacket* gosub_emit_panel_outline(GosubLinePacket* line, s32* ot, s32 x, s32 y, s32 w, s32 h, s32 color);
GosubTilePacket* gosub_emit_panel_corners(SPRT* prim, s32* ot, s32 x, s32 y, s32 w, s32 h);
GosubTilePacket* gosub_draw_item_list(s32* ot, s32 initial_prim, s32 x_off, s32 y_off);
s32 gosub_draw_portrait(s32 prim, s32* ot, s32 row, s32 x, s32 y, s32 count);
s32 gosub_draw_equipment_details(s32 packet_cursor, s32* ordering_table, s32 x_offset, s32 y_offset);
s32 gosub_draw_composite_icon(s32 packet, s32* ot, s32 x, s32 y, s32 block_id, s32 shape);
s32 gosub_draw_block_preview(s32* ot, s32 initial_prim, s32 x_off, s32 y_off);
s32 gosub_handle_block_action_dialog(s32 dialog_result);
s32 gosub_handle_sort_dialog(s32 dialog_result);
s32 gosub_handle_backtrack_dialog(s32 dialog_result);
s32 gosub_handle_discard_dialog(s32 dialog_result);
s32 gosub_draw_title(s32* ordering_table, s32 packet_cursor, s32 x_offset, s32 y_offset);
s32 gosub_draw_block_action_dialog(s32* ordering_table, s32 initial_packet, s32 x_offset, s32 y_offset);
s32 gosub_draw_sort_dialog(s32* ordering_table, s32 initial_packet, s32 x_offset, s32 y_offset);
void gosub_open_block_action_dialog(void);
void gosub_open_sort_dialog(void);
void gosub_upload_ui_image(void);
void gosub_upload_image_archive(GosubImageVramLayout* destinations, TimPrefix* tim);
inline void gosub_copy_logic_block(void* dst, void* src);
inline void gosub_copy_list_row(void* dst, void* src);
s32 gosub_emit_glyph(s32 packet_cursor, s32* ordering_table, s32 glyph_id, s32 x, s32 y, s32 clut_index);
s32 gosub_finish_glyph_run(s32 packet_cursor, s32* ordering_table);
void gosub_delete_logic_block(s32 record_index);
void gosub_delete_list_row(s32 row);
s32 gosub_compare_logic_blocks(s32 mode, s32 left_row_index, s32 right_row_index);
void gosub_upload_font_texture(void);

/* Overlay state shared across the recovered translation units. */
extern s32 g_gosub_frame_parity;
extern s32 g_gosub_finished;
extern s32 g_gosub_cursor_row;
extern s32 g_gosub_row_count;
extern s32 g_gosub_visible_row_count;
extern u8 g_gosub_screen_sequence_index;
extern u8 D_8016B8DD[3];
extern s32 g_gosub_scroll_frames_remaining;
extern s32 g_gosub_block_shape;
extern s32 g_gosub_dialog_choice;
extern s32 g_gosub_block_level;
extern s32 g_gosub_allow_duplicate_selection;
extern u8* g_gosub_dialog_text;
extern s32 (*g_gosub_finish_handler)(void);
extern u8 g_gosub_selection_mode;
extern u8 g_gosub_required_selection_count;
extern u8 D_8016B8FE[2];
extern s32 g_gosub_show_row_details;
extern s32 D_8016B904;
extern s32 g_gosub_result_rows[16];
extern s32 g_gosub_dialog_accepting_input;
extern u8 g_gosub_selected_rows[4];
extern s32 g_gosub_window_height;
extern s32 (*g_gosub_select_handler)(void);
extern s32 g_gosub_window_width;
extern s32 g_gosub_suppress_dialog_sound;
extern u8 g_gosub_text_buffers[GOSUB_TEXT_BUFFER_COUNT][GOSUB_TEXT_BUFFER_SIZE];
extern u8 g_gosub_selection_count;
extern u8 D_80170961[7];
extern u8 g_gosub_screen_sequence[20];
extern s32 g_gosub_block_id;
extern s32 g_gosub_row_height;
extern s32 D_80170984;
extern s32 g_gosub_scroll_y;
extern s32 g_gosub_sort_ascending;
extern s32 g_gosub_scroll_target_y;
extern u8* g_gosub_title_text;
extern GosubElement g_gosub_elements[GOSUB_ELEMENT_COUNT];
extern GosubListRow g_gosub_rows[512];
extern s32 (*g_gosub_dialog_handler)(s32);

/**
 * @brief Open the Yes/No confirmation dialog in reserved element 0.
 * @note Defined out of line in gosub_elements.c; other callers inline this copy.
 */
extern inline void gosub_open_confirmation_dialog(void)
{
    GosubElement* element;

    element = &g_gosub_elements[0];
    element->draw_handler = (void*)&gosub_draw_confirmation_prompt;
    g_gosub_dialog_choice = 0;
    element->attr.f.state = GOSUB_ELEMENT_STATE_ENTERING;
    element->attr.f.transition_step = 1;
    element->attr.f.x = GOSUB_MESSAGE_PANEL_X;
    element->attr.f.y = GOSUB_DIALOG_PANEL_Y;
    element->geometry.f.width_high = GOSUB_ELEMENT_WIDTH_HIGH(GOSUB_MESSAGE_PANEL_WIDTH);
    element->geometry.f.height = GOSUB_TWO_LINE_PANEL_HEIGHT;
    GOSUB_SET_ELEMENT_WIDTH_LOW(element, GOSUB_MESSAGE_PANEL_WIDTH);
}

#endif
