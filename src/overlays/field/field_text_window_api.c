/** @file field_text_window_api.c
 * @brief Script-facing helpers that configure, open and fill field text windows.
 */

#include "common.h"
#include "field_calls.h"
#include "field_text.h"
#include "main.h"

/** @brief Pending text-window configuration block (fixed RAM) consumed by field_text_apply_config. */
#define FIELD_TEXT_PENDING_CONFIG ((FieldTextConfig*)0x801ED408)

/** @brief Window layouts in g_field_text_window_layouts. */
#define FIELD_TEXT_LAYOUT_NO_PORTRAIT 2 /**< First layout that never shows a portrait. */
#define FIELD_TEXT_LAYOUT_TIMED 4       /**< One-shot timed window. */
#define FIELD_TEXT_LAYOUT_FIRST_PACKED 5 /**< First packed-window layout; lower layouts are fixed windows. */
#define FIELD_TEXT_LAYOUT_BOLD 6        /**< Layout drawn in the bold text style. */

/** @brief Size of one portrait (16-color palette plus 48 by 48 4bpp pixels). */
#define FIELD_TEXT_PORTRAIT_SIZE (16 * 2 + 48 * 48 / 2)

/** @brief Portraits in one scene portrait block. */
#define FIELD_TEXT_SCENE_BLOCK_PORTRAITS 63

/** @brief Portrait selector bits (-1 selects no portrait). */
#define FIELD_TEXT_PORTRAIT_INDEX_MASK 0x3F /**< Portrait index. */
#define FIELD_TEXT_PORTRAIT_SELECT_RIGHT 0x40 /**< Show the portrait on the right. */
#define FIELD_TEXT_PORTRAIT_SELECT_SHARED 0x80 /**< Take the portrait from the shared bank in g_prim_rect_buf. */

/** @brief Pending-config flag bits (field_text_apply_config moves them into the window flags). */
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

/** @brief Selector bit FIELD_TEXT_PORTRAIT_SELECT_RIGHT shifted to FIELD_TEXT_CONFIG_PORTRAIT_RIGHT. */
#define FIELD_TEXT_PORTRAIT_SELECT_SHIFT 4

/** @brief Scene string @p index; the scene strings start with a table of u16 offsets to each string. */
#define FIELD_SCENE_STRING(index) (g_field_scene_strings + ((u16*)g_field_scene_strings)[index])

/** @brief Screen rectangle of one text window. */
typedef struct
{
    u16 x;
    u16 y;
    u16 width;
    u16 height;
} FieldTextWindowRect;

/** @brief Byte view of a field text flags word. */
typedef struct
{
    u8 low;
    u8 byte1;
    u8 byte2;
    u8 byte3;
} FieldTextFlagBytes;

/** @brief Field text flags word, addressable whole or by byte. */
typedef union
{
    u32 word;
    FieldTextFlagBytes b;
} FieldTextFlags;

/** @brief Transition anchor of a text window. */
typedef struct
{
    u16 x;
    u16 y;
} FieldTextAnchor;

/** @brief Transition anchor, addressable whole or by coordinate. */
typedef union
{
    u32 word;
    FieldTextAnchor pos;
} FieldTextAnchorWord;

/** @brief Pending configuration copied into a field text-window state. */
struct FieldTextConfig
{
    u8* portrait;
    u16 x;
    u16 y;
    u16 width;
    u16 height;
    FieldTextAnchorWord anchor;
    FieldTextFlags flags;
    u8* text;
};

extern u8* g_field_scene_portraits;
extern u8* g_field_scene_strings;
extern FieldTextWindowRect g_field_text_window_layouts[][FIELD_TEXT_WINDOW_SLOTS];

/**
 * @brief Configure and open a field text window.
 * @param window_slot Window slot index.
 * @param layout_index Window layout.
 * @param unused Unused.
 * @param portrait_selector Portrait selector: -1 for none, otherwise FIELD_TEXT_PORTRAIT_* bits and an index.
 */
void field_open_text_window(s32 window_slot, s32 layout_index, s32 unused, s32 portrait_selector)
{
    FieldTextConfig* cfg;

    cfg = FIELD_TEXT_PENDING_CONFIG;
    cfg->anchor.pos.x = 0;
    cfg->anchor.pos.y = 0;
    if ((portrait_selector == -1) || (layout_index >= FIELD_TEXT_LAYOUT_NO_PORTRAIT && layout_index < FIELD_TEXT_LAYOUT_FIRST_PACKED))
    {
        cfg->portrait = NULL;
    }
    else if (portrait_selector & FIELD_TEXT_PORTRAIT_SELECT_SHARED)
    {
        cfg->portrait = g_prim_rect_buf + (portrait_selector & FIELD_TEXT_PORTRAIT_INDEX_MASK) * FIELD_TEXT_PORTRAIT_SIZE;
    }
    else
    {
        cfg->portrait = g_field_scene_portraits + (portrait_selector & FIELD_TEXT_PORTRAIT_INDEX_MASK) * FIELD_TEXT_PORTRAIT_SIZE;
    }

    cfg->flags.b.low = 0;
    if (layout_index == FIELD_TEXT_LAYOUT_BOLD)
    {
        cfg->flags.word = (cfg->flags.word & ~FIELD_TEXT_CONFIG_STYLE_MASK) | FIELD_TEXT_CONFIG_STYLE_BOLD;
    }
    else
    {
        cfg->flags.word &= ~FIELD_TEXT_CONFIG_STYLE_MASK;
    }
    cfg->flags.word = ((cfg->flags.word & ~FIELD_TEXT_CONFIG_PORTRAIT_MASK) | ((portrait_selector << FIELD_TEXT_PORTRAIT_SELECT_SHIFT) & FIELD_TEXT_CONFIG_PORTRAIT_RIGHT)) &
                      ~FIELD_TEXT_CONFIG_MODE_MASK;

    cfg->x = g_field_text_window_layouts[layout_index][window_slot].x;
    cfg->y = g_field_text_window_layouts[layout_index][window_slot].y;
    cfg->width = g_field_text_window_layouts[layout_index][window_slot].width;
    cfg->height = g_field_text_window_layouts[layout_index][window_slot].height;

    if (layout_index >= FIELD_TEXT_LAYOUT_FIRST_PACKED)
    {
        field_text_open_packed_window(window_slot);
    }
    else
    {
        field_text_open_fixed_window(window_slot);
    }
}

/**
 * @brief Set the string of a field text window to a scene string.
 * @param window_slot Window slot index.
 * @param string_index Scene string index.
 * @param options Option flags forwarded to field_text_set_string.
 */
void field_set_text_window_string(s32 window_slot, s32 string_index, s32 options)
{
    field_text_set_string(window_slot, FIELD_SCENE_STRING(string_index), options);
}

/**
 * @brief Configure and open a field text window and show a scene string in it.
 * @param string_index Scene string index.
 * @param window_slot Window slot index.
 * @param layout_index Window layout.
 * @param unused Unused.
 * @param portrait_selector Portrait selector: -1 for none, otherwise FIELD_TEXT_PORTRAIT_* bits and an index.
 * @note Without FIELD_TEXT_PORTRAIT_SELECT_SHARED the whole selector picks a scene portrait block of
 *       FIELD_TEXT_SCENE_BLOCK_PORTRAITS portraits, not a single portrait.
 */
void field_open_text_window_with_string(s32 string_index, s32 window_slot, s32 layout_index, s32 unused, s32 portrait_selector)
{
    FieldTextConfig* cfg;

    cfg = FIELD_TEXT_PENDING_CONFIG;
    cfg->anchor.pos.x = 0;
    cfg->anchor.pos.y = 0;
    if ((portrait_selector == -1) || (layout_index >= FIELD_TEXT_LAYOUT_NO_PORTRAIT && layout_index < FIELD_TEXT_LAYOUT_FIRST_PACKED))
    {
        cfg->portrait = NULL;
    }
    else if (portrait_selector & FIELD_TEXT_PORTRAIT_SELECT_SHARED)
    {
        cfg->portrait = g_prim_rect_buf + (portrait_selector & FIELD_TEXT_PORTRAIT_INDEX_MASK) * FIELD_TEXT_PORTRAIT_SIZE;
    }
    else
    {
        cfg->portrait = g_field_scene_portraits + portrait_selector * FIELD_TEXT_SCENE_BLOCK_PORTRAITS * FIELD_TEXT_PORTRAIT_SIZE;
    }

    cfg->flags.b.low = 0;
    if (layout_index == FIELD_TEXT_LAYOUT_BOLD)
    {
        cfg->flags.word = (cfg->flags.word & ~FIELD_TEXT_CONFIG_STYLE_MASK) | FIELD_TEXT_CONFIG_STYLE_BOLD;
    }
    else
    {
        cfg->flags.word &= ~FIELD_TEXT_CONFIG_STYLE_MASK;
    }
    cfg->flags.word = ((cfg->flags.word & ~FIELD_TEXT_CONFIG_PORTRAIT_MASK) | ((portrait_selector << FIELD_TEXT_PORTRAIT_SELECT_SHIFT) & FIELD_TEXT_CONFIG_PORTRAIT_RIGHT)) &
                      ~FIELD_TEXT_CONFIG_MODE_MASK;

    cfg->x = g_field_text_window_layouts[layout_index][window_slot].x;
    cfg->y = g_field_text_window_layouts[layout_index][window_slot].y;
    cfg->width = g_field_text_window_layouts[layout_index][window_slot].width;
    cfg->height = g_field_text_window_layouts[layout_index][window_slot].height;

    if (layout_index >= FIELD_TEXT_LAYOUT_FIRST_PACKED)
    {
        field_text_open_packed_window(window_slot);
    }
    else
    {
        field_text_open_fixed_window(window_slot);
    }
    field_text_set_string(window_slot, FIELD_SCENE_STRING(string_index), 1);
}

/**
 * @brief Close a field text window.
 * @param window_slot Window slot index.
 */
void field_close_text_window(s32 window_slot)
{
    field_text_close_window(window_slot);
}

/**
 * @brief Open the timed text window on a scene string.
 *
 * Clears the portrait, anchor and flag bits of the pending config and takes
 * the rectangle of the timed-window layout before opening the window.
 *
 * @param string_index Scene string index.
 */
void field_show_timed_text(s32 string_index)
{
    FieldTextConfig* cfg = FIELD_TEXT_PENDING_CONFIG;

    cfg->flags.b.low = 0;
    cfg->anchor.pos.x = 0;
    cfg->anchor.pos.y = 0;
    cfg->portrait = NULL;
    cfg->flags.word &= ~FIELD_TEXT_CONFIG_STYLE_MASK;
    cfg->flags.word &= ~FIELD_TEXT_CONFIG_PORTRAIT_MASK;
    cfg->flags.word &= ~FIELD_TEXT_CONFIG_MODE_MASK;
    cfg->x = g_field_text_window_layouts[FIELD_TEXT_LAYOUT_TIMED][0].x;
    cfg->y = g_field_text_window_layouts[FIELD_TEXT_LAYOUT_TIMED][0].y;
    cfg->width = g_field_text_window_layouts[FIELD_TEXT_LAYOUT_TIMED][0].width;
    cfg->height = g_field_text_window_layouts[FIELD_TEXT_LAYOUT_TIMED][0].height;
    field_text_start_timed_window(FIELD_SCENE_STRING(string_index));
}
