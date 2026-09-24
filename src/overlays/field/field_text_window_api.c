/** @file field_text_window_api.c
 * @brief Script-facing helpers that configure, open and fill field text windows.
 */

#include "common.h"
#include "field_calls.h"
#include "field_text.h"

/** @brief Pending text-window configuration block consumed by field_text_apply_config. */
#define FIELD_TEXT_PENDING_CONFIG ((FieldTextConfig*)0x801ED408)

/** @brief Number of window slots per layout in the geometry table. */
#define FIELD_TEXT_WINDOW_SLOTS 4

/** @brief Layout index of the one-shot timed window. */
#define FIELD_TEXT_LAYOUT_TIMED 4

/** @brief First layout opened as a packed window; lower layouts are fixed windows. */
#define FIELD_TEXT_LAYOUT_FIRST_PACKED 5

/** @brief Layout whose window uses text style 1. */
#define FIELD_TEXT_LAYOUT_STYLED 6

/** @brief Size of one portrait image in a portrait bank. */
#define FIELD_TEXT_PORTRAIT_SIZE 0x4A0

/** @brief Size of one scene's portrait block. */
#define FIELD_TEXT_SCENE_PORTRAIT_BLOCK_SIZE 0x12360

/** @brief Portrait selector bit that picks the shared portrait bank. */
#define FIELD_TEXT_PORTRAIT_SHARED 0x80

/** @brief Portrait selector bits that hold the portrait index. */
#define FIELD_TEXT_PORTRAIT_INDEX_MASK 0x3F

/**
 * @brief Address of scene string @p index, through the u16 offset table at the start of the scene strings.
 * @note Summed as integers, table first, which is how the original indexes the table.
 */
#define FIELD_SCENE_STRING(index) ((u8*)(g_field_scene_strings + *(u16*)((index) * 2 + g_field_scene_strings)))

/** @brief Window-geometry table element used by the packed/fixed openers. */
typedef struct
{
    u16 x;
    u16 y;
    u16 width;
    u16 height;
} FieldWindowGeometry;

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
extern s32 g_field_scene_strings;
extern u8 g_prim_rect_buf[];
extern FieldWindowGeometry D_800EF64C[][FIELD_TEXT_WINDOW_SLOTS];

/**
 * @brief Configure and open a field text window.
 * @param window_slot Window slot index.
 * @param layout_index Window layout/style selector.
 * @param unused Unused.
 * @param portrait_selector Portrait selector: -1 for none, bit 0x80 for the shared bank, low six bits the portrait.
 */
void func_8009C620(s32 window_slot, s32 layout_index, s32 unused, s32 portrait_selector)
{
    FieldTextConfig* cfg;
    FieldWindowGeometry* geometry;
    FieldWindowGeometry* geometry_table;
    s32 geometry_offset;
    u32 flags;

    cfg = FIELD_TEXT_PENDING_CONFIG;
    cfg->anchor.pos.x = 0;
    cfg->anchor.pos.y = 0;
    if ((portrait_selector == -1) || (layout_index >= 2 && layout_index < 5))
    {
        cfg->portrait = NULL;
    }
    else if (portrait_selector & FIELD_TEXT_PORTRAIT_SHARED)
    {
        cfg->portrait = g_prim_rect_buf + (portrait_selector & FIELD_TEXT_PORTRAIT_INDEX_MASK) * FIELD_TEXT_PORTRAIT_SIZE;
    }
    else
    {
        cfg->portrait = g_field_scene_portraits + (portrait_selector & FIELD_TEXT_PORTRAIT_INDEX_MASK) * FIELD_TEXT_PORTRAIT_SIZE;
    }

    cfg->flags.b.low = 0;
    if (layout_index == FIELD_TEXT_LAYOUT_STYLED)
    {
        cfg->flags.word = (cfg->flags.word & ~0x300) | 0x100;
    }
    else
    {
        cfg->flags.word &= ~0x300;
    }

    flags = cfg->flags.word;
    geometry_table = D_800EF64C[0];
    flags &= ~0xC00;
    flags |= (portrait_selector << 4) & 0x400;
    flags &= ~0x7000;
    geometry_offset = window_slot << 3;
    cfg->flags.word = flags;
    geometry = (FieldWindowGeometry*)((u8*)geometry_table + (geometry_offset + (layout_index << 5)));
    cfg->x = geometry->x;
    cfg->y = geometry->y;
    cfg->width = geometry->width;
    cfg->height = geometry->height;

    if (layout_index >= FIELD_TEXT_LAYOUT_FIRST_PACKED)
    {
        field_text_open_packed_window(window_slot);
        return;
    }
    field_text_open_fixed_window(window_slot);
}

/**
 * @brief Set a field text-window string chosen through the scene string table.
 * @param window_slot Window slot index.
 * @param string_index Scene string index.
 * @param options Option flags forwarded to field_text_set_string.
 */
void func_8009C77C(s32 window_slot, s32 string_index, s32 options)
{
    field_text_set_string(window_slot, FIELD_SCENE_STRING(string_index), options);
}

/**
 * @brief Configure, open, and populate a field text window.
 * @param string_index Scene string index.
 * @param window_slot Window slot index.
 * @param layout_index Window layout/style selector.
 * @param unused Unused.
 * @param portrait_selector Portrait selector: -1 for none, bit 0x80 for the shared bank, otherwise a scene block.
 */
void func_8009C7B0(s32 string_index, s32 window_slot, s32 layout_index, s32 unused, s32 portrait_selector)
{
    FieldTextConfig* cfg;
    FieldWindowGeometry* geometry;
    FieldWindowGeometry* geometry_table;
    s32 geometry_offset;
    u32 flags;

    cfg = FIELD_TEXT_PENDING_CONFIG;
    cfg->anchor.pos.x = 0;
    cfg->anchor.pos.y = 0;
    if ((portrait_selector == -1) || (layout_index >= 2 && layout_index < 5))
    {
        cfg->portrait = NULL;
    }
    else if (portrait_selector & FIELD_TEXT_PORTRAIT_SHARED)
    {
        cfg->portrait = g_prim_rect_buf + (portrait_selector & FIELD_TEXT_PORTRAIT_INDEX_MASK) * FIELD_TEXT_PORTRAIT_SIZE;
    }
    else
    {
        cfg->portrait = g_field_scene_portraits + portrait_selector * FIELD_TEXT_SCENE_PORTRAIT_BLOCK_SIZE;
    }

    cfg->flags.b.low = 0;
    if (layout_index == FIELD_TEXT_LAYOUT_STYLED)
    {
        cfg->flags.word = (cfg->flags.word & ~0x300) | 0x100;
    }
    else
    {
        cfg->flags.word &= ~0x300;
    }

    flags = cfg->flags.word;
    geometry_table = D_800EF64C[0];
    flags &= ~0xC00;
    flags |= (portrait_selector << 4) & 0x400;
    flags &= ~0x7000;
    geometry_offset = window_slot << 3;
    cfg->flags.word = flags;
    geometry = (FieldWindowGeometry*)((u8*)geometry_table + (geometry_offset + (layout_index << 5)));
    cfg->x = geometry->x;
    cfg->y = geometry->y;
    cfg->width = geometry->width;
    cfg->height = geometry->height;

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
void func_8009C954(s32 window_slot)
{
    field_text_close_window(window_slot);
}

/**
 * @brief Reset the pending field text-window config and open a timed window.
 *
 * Clears the portrait, anchor and style bits of the pending config, takes the
 * geometry of the timed-window layout, then opens a timed text window showing
 * scene string @p string_index.
 *
 * @param string_index Scene string index.
 */
void func_8009C974(s32 string_index)
{
    FieldTextConfig* cfg = FIELD_TEXT_PENDING_CONFIG;

    cfg->flags.b.low = 0;
    cfg->anchor.pos.x = 0;
    cfg->anchor.pos.y = 0;
    cfg->portrait = 0;
    cfg->flags.word &= ~0x300;
    cfg->flags.word &= ~0xC00;
    cfg->flags.word &= ~0x7000;
    cfg->x = D_800EF64C[FIELD_TEXT_LAYOUT_TIMED][0].x;
    cfg->y = D_800EF64C[FIELD_TEXT_LAYOUT_TIMED][0].y;
    cfg->width = D_800EF64C[FIELD_TEXT_LAYOUT_TIMED][0].width;
    cfg->height = D_800EF64C[FIELD_TEXT_LAYOUT_TIMED][0].height;
    field_text_start_timed_window(FIELD_SCENE_STRING(string_index));
}
