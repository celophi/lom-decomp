#ifndef SHOP_INTERNAL_H
#define SHOP_INTERNAL_H

#include "common.h"
#include "main/main.h"
#include "overlays/shop/shop.h"
#include "common/pad.h"
#include "overlays/field/field_menu_window.h"
#include "overlays/field/field_ui_text.h"
#include "common/vector.h"
#include "overlays/field/field_fade.h"
#include "overlays/field/field_input.h"

#define SHOP_WINDOW_COUNT 8
/** @brief Window slot reserved for the notice and quantity-prompt popups. */
#define SHOP_POPUP_WINDOW 0
/** @brief Window slot holding the money display; the shop exits once it has closed. */
#define SHOP_MONEY_WINDOW 1
/** @brief Frames an opening or closing window takes to reach its final size. */
#define SHOP_WINDOW_ANIM_STEPS 8

/** @brief Entry id of a sold-out entry. */
#define SHOP_ENTRY_EMPTY 0xFFFF

#define SHOP_MAX_ITEM_COUNT 99
#define SHOP_MAX_MONEY 10000000
#define SHOP_ROW_HEIGHT 16
#define SHOP_ROW_HEIGHT_SHIFT 4

#define SHOP_CONFIRM_BUTTONS (PAD_BTN_CROSS | PAD_BTN_L3)
#define SHOP_CANCEL_BUTTONS PAD_BTN_CIRCLE
#define SHOP_DPAD_BUTTONS (PAD_BTN_UP | PAD_BTN_RIGHT | PAD_BTN_DOWN | PAD_BTN_LEFT)

#define SHOP_SFX_CURSOR 0x7D
#define SHOP_SFX_ERROR 0x78
#define SHOP_SFX_CANCEL 0x7F
#define SHOP_SFX_TRADE 0xB4

#define SHOP_TEXT_COLOR_NORMAL 4
#define SHOP_TEXT_COLOR_DISABLED 5
#define SHOP_TEXT_COLOR_NOTICE 6

#define SHOP_NOTICE_WINDOW_X 32
#define SHOP_NOTICE_WINDOW_Y 112
#define SHOP_NOTICE_WINDOW_WIDTH 256
#define SHOP_NOTICE_WINDOW_HEIGHT 16

/** @brief Notice shown when the player cannot carry any more of the selected item. */
#define SHOP_NOTICE_CANNOT_CARRY 0
/** @brief Notice shown when a purchase had to be reduced to fit the player's inventory. */
#define SHOP_NOTICE_QUANTITY_REDUCED 1

/** @brief Lifecycle of a shop window slot. */
enum ShopWindowState
{
    SHOP_WINDOW_FREE,
    SHOP_WINDOW_OPENING,
    SHOP_WINDOW_OPEN,
    SHOP_WINDOW_CLOSING
};

/** @brief Draw callback installed on a shop window. */
typedef u8* (*ShopDrawFunc)(u_long* ot, u8* prim, s32 x_inset, s32 y_inset);

/** @brief Value of ShopWindow.extent.bits.width_high for a window @p width pixels wide. */
#define SHOP_WINDOW_WIDTH_HIGH(width) ((width) >> 8)
/** @brief ShopWindow.frame.word bits holding the low byte of a window @p width pixels wide. */
#define SHOP_WINDOW_WIDTH_LOW(width) ((u32)((width) & 0xFF) << 24)

/**
 * @brief One animated shop window.
 * @note The 9-bit width is split across both words: its low byte sits at the
 *       top of @c frame and its high bit at the bottom of @c extent.
 */
typedef struct
{
    union
    {
        u32 word;
        struct
        {
            unsigned state : 3;
            unsigned anim_step : 4;
            unsigned x : 9;
            unsigned y : 8;
            unsigned width_low : 8;
        } bits;
    } frame;
    union
    {
        u32 word;
        struct
        {
            unsigned width_high : 1;
            unsigned height : 8;
            unsigned unused : 23;
        } bits;
    } extent;
    ShopDrawFunc draw;
} ShopWindow;

/** @brief Sections of the overlay text archive (g_shop_text_archive). */
typedef enum
{
    SHOP_TEXT_ITEM_DESCRIPTIONS = 0, /**< Class and ingredient role of each item kind; the detail title of a plain item. */
    SHOP_TEXT_ITEM_NAMES = 1,        /**< Name of each item kind; the first 64 double as equipment materials. */
    SHOP_TEXT_EQUIPMENT_TYPES = 2,   /**< Weapon, armor and instrument type names. */
    SHOP_TEXT_INSTRUMENT_SPELLS = 3, /**< Instrument spell names, one row per spirit. */
    SHOP_TEXT_SECTION_COUNT = 4
} ShopTextSection;

/** @brief Header of the overlay's text archive; each section is a table of u16 string offsets. */
typedef struct
{
    u16 section_count;
    u16 unused;
    u32 section_offsets[SHOP_TEXT_SECTION_COUNT];
} ShopTextArchive;

/**
 * @brief The list entry under the cursor.
 */
#define SHOP_SELECTED_ENTRY() ((ShopEntry*)(g_shop_cursor * sizeof(ShopEntry) + (uintptr_t)g_shop_entries))

extern ShopTextArchive g_shop_text_archive;
extern u16 g_shop_item_sell_prices[FIELD_ITEM_KIND_COUNT];

extern s32 g_shop_title_text_id;
extern s32 g_shop_frame_index;
extern u8* g_shop_work_end;
extern s32 g_shop_is_buying;
extern s32 g_shop_finished;
extern ShopPacketBuffer* g_shop_work_buffer;
extern s32 g_shop_confirm_choice;
extern s32 g_shop_prompt_active;
extern ShopWindow g_shop_windows[SHOP_WINDOW_COUNT];
extern s32 g_shop_notice_active;
extern s32 g_shop_notice_id;
extern s32 g_shop_quantity;
extern FieldItemRecord* g_shop_item_records;
extern s32 g_shop_scroll_y;
extern s32 g_shop_scroll_target;
extern ShopEntry* g_shop_entries;
extern ShopEntry g_shop_entry_buffer[];
extern s32 g_shop_entry_count;
extern s32 g_shop_cursor;
extern s32 g_shop_scroll_frames;

/*
 * FIELD exports with no project header. FIELD stays resident while SHOP runs.
 */

/*
 * FIELD's UI string archive starts with a table of unaligned little-endian u16
 * offsets. Each entry SHOP reads is its own symbol; the string for entry N of
 * the table starting at D_800EC3C4 lies at D_800EC3C4 + offset.
 */
extern u8 D_800EC3C4[];
extern u8 g_field_ui_text_cant_hold_more[];
extern u8 D_800EC3DC[];
extern u8 D_800EC3E0[];
extern u8 D_800EC3E2[];
extern u8 D_800EC3F2[];
extern u8 D_800EC3F4[];
extern u8 D_800EC3F8[];
extern u8 D_800EC3FE[];

void play_menu_sfx(s32 sfx_id, s32 volume);
void field_compact_inventory(void);
FieldItemRecord* field_find_free_inventory_record(void);
void field_copy_inventory_record(FieldItemRecord* dst, FieldItemRecord* src);
u8* func_800AD850(u8* prim, u_long* ot, s32 x, s32 y, s32 width, s32 height, s32 display_buffer_index, s32 is_popup);
u8* func_800AE76C(u8* prim, u_long* ot, s32 x, s32 y, s32 direction);

#endif
