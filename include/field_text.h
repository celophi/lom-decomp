#ifndef FIELD_TEXT_H
#define FIELD_TEXT_H

#include "common.h"
struct SPRT;

typedef struct FieldOrderingTags FieldOrderingTags;
typedef struct FieldTextConfig FieldTextConfig;

extern FieldTextConfig* g_field_text_saved_configs;

/**
 * @brief First byte the save-screen text helpers (ADDHERO, CARDA, NIKI, SHOP) count as a two-byte sequence.
 * @note In the US renderer only 0x19 (a two-byte character) and 0x1F (an extended dictionary entry)
 *       take a second byte; 0x1A-0x1E are one-byte dictionary entries.
 */
#define FIELD_TEXT_DOUBLE_BYTE_LEAD_FIRST 0x19

/** @brief Last byte the save-screen text helpers count as a two-byte sequence. */
#define FIELD_TEXT_DOUBLE_BYTE_LEAD_LAST 0x1F

/** @brief True when the save-screen text helpers treat @p code as the first of two bytes. */
#define FIELD_TEXT_IS_DOUBLE_BYTE_LEAD(code) ((code) >= FIELD_TEXT_DOUBLE_BYTE_LEAD_FIRST && (code) <= FIELD_TEXT_DOUBLE_BYTE_LEAD_LAST)

/** @brief field_draw_text / field_draw_number alignment, in the FIELD_TEXT_ALIGN_MASK bits of their flags. */
#define FIELD_TEXT_ALIGN_LEFT 0   /**< x is the left edge. */
#define FIELD_TEXT_ALIGN_RIGHT 1  /**< x is the right edge. */
#define FIELD_TEXT_ALIGN_CENTER 2 /**< x is the centre. */
#define FIELD_TEXT_ALIGN_MASK 0x7F

/** @brief field_draw_text / field_draw_number flag that adds the black glyph outline. */
#define FIELD_TEXT_SHADOW 0x80

/** @brief field_draw_text / field_draw_number text colours. */
#define FIELD_TEXT_COLOR_NORMAL 4
#define FIELD_TEXT_COLOR_DIM 5

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

#endif
