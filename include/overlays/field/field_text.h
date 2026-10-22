#ifndef FIELD_TEXT_H
#define FIELD_TEXT_H

#include "common.h"

#include "common/vector.h"
#include "overlays/field/field_portrait.h"
struct SPRT;

typedef struct FieldOrderingTags FieldOrderingTags;
typedef struct FieldTextConfig FieldTextConfig;

extern FieldTextConfig* g_field_text_saved_configs;

/** @brief field_draw_text / field_draw_number alignment, in the FIELD_TEXT_ALIGN_MASK bits of their flags. */
#define FIELD_TEXT_ALIGN_LEFT 0   /**< x is the left edge. */
#define FIELD_TEXT_ALIGN_RIGHT 1  /**< x is the right edge. */
#define FIELD_TEXT_ALIGN_CENTER 2 /**< x is the centre. */
#define FIELD_TEXT_ALIGN_MASK 0x7F

/** @brief field_draw_text / field_draw_number flag that adds the black glyph outline. */
#define FIELD_TEXT_SHADOW 0x80

/** @brief field_draw_text / field_draw_number text colours (palette pairs of the text CLUT). */
#define FIELD_TEXT_COLOR_MENU 1     /**< Body text of the main menu and of the title screen's load menu. */
#define FIELD_TEXT_COLOR_MENU_DIM 3 /**< Dimmed FIELD_TEXT_COLOR_MENU text, such as the inactive card slot label. */
#define FIELD_TEXT_COLOR_NORMAL 4   /**< Body text of field windows and the field-launched card menus. */
#define FIELD_TEXT_COLOR_DIM 5

/** @brief Glyph height of the field font, and the line advance of text drawn with it. */
#define FIELD_TEXT_LINE_HEIGHT 12
#define FIELD_TEXT_LINE_SPACING 16

/** @brief Number of text window slots. */
#define FIELD_TEXT_WINDOW_SLOTS 4

/** @brief Replacement text and its unsigned character budget. */
typedef struct
{
    u8 character_limit;
    u8 _pad1[3];
    u8* text;
} FieldTextMacro;

extern FieldTextMacro g_field_text_macros[];

void field_text_upload_immediate_cache(void);
void field_text_init(void);
void field_text_reset_windows(void);
void field_text_reset_scratch(void);
s32 field_text_build_sprites(struct SPRT* prim, u8* text, s32 text_style);
void field_text_open_packed_window(s32 window_index);
void field_text_open_fixed_window(s32 window_index);
void field_text_update(u8** packet_cursor, FieldOrderingTags* ot, s32 draw_count);
void field_text_set_string(s32 window_index, u8* text, s32 text_options);
void field_text_start_timed_window(u8* text);
void field_text_set_position(s32 slot, s16 x, s16 y);
void field_text_close_window(s32 slot);
/** @brief field_text_get_status results. */
#define FIELD_TEXT_STATUS_CLOSED (-1) /**< The window is not in dialogue mode. */
#define FIELD_TEXT_STATUS_DONE 0      /**< All text has been shown. */
#define FIELD_TEXT_STATUS_BUSY 1      /**< Text remains to be shown. */
#define FIELD_TEXT_STATUS_PROMPT 2    /**< The window waits at a prompt. */

s32 field_text_get_status(s32 slot);
s32 field_text_get_choice(s32 slot);
void field_text_format_number(s32 window_index, u32 value, u8 digits);

/**
 * @brief Draw encoded text as glyph sprites followed by a draw-mode packet.
 * @param sprite_cursor First free sprite in the primitive arena.
 * @param ot Ordering-table tag receiving the packets.
 * @param text Encoded text to draw.
 * @param text_color Text colour passed to the glyph builder.
 * @param x Horizontal position interpreted by the alignment flags.
 * @param y Top edge of the text.
 * @param flags Alignment and optional glyph outline flags.
 * @return First free primitive after the text and its draw-mode packet.
 */
void* field_draw_text(struct SPRT* sprite_cursor, u_long* ot, u8* text, s32 text_color, s32 x, s32 y, s32 flags);

/**
 * @brief Draw a signed decimal number as glyph sprites.
 * @param ot Ordering-table tag receiving the packets.
 * @param sprite_cursor First free sprite in the primitive arena.
 * @param value Number to draw.
 * @param text_color Text colour.
 * @param position Screen coordinates of the text.
 * @param flags Alignment and optional glyph outline flags.
 * @return First free primitive after the number.
 */
void* field_draw_number(u_long* ot, struct SPRT* sprite_cursor, s32 value, s32 text_color, Vec2s* position, s32 flags);

/**
 * @brief Draw a signed decimal number, requesting double-byte digits in JP.
 * @param ot Ordering-table tag receiving the packets.
 * @param sprite_cursor First free sprite in the primitive arena.
 * @param value Number to draw.
 * @param text_color Text colour.
 * @param position Screen coordinates of the text.
 * @param flags Alignment and optional glyph outline flags.
 * @return First free primitive after the number.
 */
void* field_draw_number_wide(u_long* ot, struct SPRT* sprite_cursor, s32 value, s32 text_color, Vec2s* position, s32 flags);

/**
 * @brief Format a signed decimal number of up to eight digits as encoded text.
 * @param text Buffer receiving the digits, optional minus sign and terminator.
 * @param number Number to format; negative values use the FIELD text bank's minus sign.
 * @param wide_request Non-zero requests double-byte digits in JP; ignored in US.
 */
void field_format_number(u8* text, s32 number, s32 wide_request);

/** @brief Pending-config flag bits (field_text_apply_config moves them into the window flags). */
#define FIELD_TEXT_CONFIG_LOW_MASK 0xFF
#define FIELD_TEXT_CONFIG_STYLE_BOLD 0x100
#define FIELD_TEXT_CONFIG_STYLE_PLAIN 0x200
#define FIELD_TEXT_CONFIG_STYLE_MASK (FIELD_TEXT_CONFIG_STYLE_BOLD | FIELD_TEXT_CONFIG_STYLE_PLAIN)
#define FIELD_TEXT_CONFIG_PORTRAIT_RIGHT 0x400
#define FIELD_TEXT_CONFIG_PORTRAIT_OUTSIDE 0x800
#define FIELD_TEXT_CONFIG_PORTRAIT_MASK (FIELD_TEXT_CONFIG_PORTRAIT_RIGHT | FIELD_TEXT_CONFIG_PORTRAIT_OUTSIDE)
#define FIELD_TEXT_CONFIG_FLAG_1000 0x1000
#define FIELD_TEXT_CONFIG_FLAG_2000 0x2000
#define FIELD_TEXT_CONFIG_FLAG_4000 0x4000
#define FIELD_TEXT_CONFIG_MODE_MASK (FIELD_TEXT_CONFIG_FLAG_1000 | FIELD_TEXT_CONFIG_FLAG_2000 | FIELD_TEXT_CONFIG_FLAG_4000)

/** @brief field_text_set_string option that closes the window when its text ends. */
#define FIELD_TEXT_OPTION_AUTO_CLOSE 1

/** @brief Screen point that an opening or closing window grows from or shrinks to. */
typedef struct
{
    u16 x;
    u16 y;
} FieldTextAnchor;

/** @brief Transition anchor accessed as a word (zero means none) or as coordinates. */
typedef union
{
    u32 word;
    FieldTextAnchor pos;
} FieldTextAnchorWord;

/** @brief Pending-config flags: FIELD_TEXT_CONFIG_* bits, also accessed by byte. */
typedef union
{
    u32 word;
    struct
    {
        u8 low;
        u8 layout;
        u8 text_options;
        u8 unused;
    } b;
} FieldTextConfigFlags;

/** @brief Configuration of a text window, copied into its state when the window opens. */
struct FieldTextConfig
{
    FieldPortrait* portrait;
    u16 x;
    u16 y;
    u16 width;
    u16 height;
    FieldTextAnchorWord anchor;
    FieldTextConfigFlags flags;
    u8* text;
};

/** @brief Pending text-window configuration block (fixed RAM) consumed by field_text_apply_config. */
#define FIELD_TEXT_PENDING_CONFIG ((FieldTextConfig*)TEXT_CONFIG_ADDRESS)

/** @brief The text cache sits at VRAM x 960 inside the 4bpp texture page whose top row is 256. */
#define FIELD_TEXT_CACHE_VRAM_X 960
#define FIELD_TEXT_TPAGE_VRAM_Y 256

/** @brief 4bpp texels per 16-bit VRAM word. */
#define FIELD_TEXT_PIXELS_PER_WORD 4

#endif
