/**
 * @file gname_data.c
 * @brief Panel-character / glyph layout data for the name-entry (GNAME) overlay.
 *
 * This translation unit builds the overlay's entire .data region from C. Each
 * named table below corresponds to a symbol in config/symbols/gname_symbol_addrs.txt
 * and must appear in ascending-address order so the linker lays them out
 * contiguously at their fixed addresses (0x80142C98 onward). The initializer
 * values live in gitignored, generated fragments under gen/, produced by
 * tools/gen_gname_data.py from the original disc image, so no copyrighted data
 * is committed. If a table's size or order is wrong, the compiled bytes diverge
 * from the original and the overlay no longer matches.
 *
 * @note All tables are non-const so they land in the writable .data section,
 *       and are plain byte arrays for now; the internal record layouts are
 *       still being reverse-engineered. Refining a table into a typed struct is
 *       a follow-up that must preserve the byte-for-byte match.
 * @note Regenerate the fragments with: python tools/gen_gname_data.py
 */

/** @brief Panel character offset header (index/offset table). */
unsigned char g_panel_char_offsets[] = {
#include "gen/g_panel_char_offsets.inc"
};

/** @brief Offset to the kanji-category name strings. */
unsigned char g_kanji_cat_names_offset[] = {
#include "gen/g_kanji_cat_names_offset.inc"
};

/** @brief Kanji-category entry table. */
unsigned char g_kanji_cat_entries[] = {
#include "gen/g_kanji_cat_entries.inc"
};

/** @brief Glyph metrics/layout table. */
unsigned char g_glyph_table[] = {
#include "gen/g_glyph_table.inc"
};

/** @brief Tab cursor position pair. */
unsigned char g_tab_cursor_pos[] = {
#include "gen/g_tab_cursor_pos.inc"
};

/** @brief Tab cursor entry table. */
unsigned char g_tab_cursor_entries[] = {
#include "gen/g_tab_cursor_entries.inc"
};

/** @brief Kanji entry offset table. */
unsigned char g_kanji_entry_offsets[] = {
#include "gen/g_kanji_entry_offsets.inc"
};

/** @brief Base offset for the panel data block. */
unsigned char g_panel_data_base[] = {
#include "gen/g_panel_data_base.inc"
};

/** @brief Offset into the panel table. */
unsigned char g_panel_tbl_off[] = {
#include "gen/g_panel_tbl_off.inc"
};

/** @brief Offset to the kanji panel data. */
unsigned char g_kanji_panel_offset[] = {
#include "gen/g_kanji_panel_offset.inc"
};

/** @brief Offset to the name-history strings. */
unsigned char g_history_names_off[] = {
#include "gen/g_history_names_off.inc"
};

/** @brief Offset to the random-name strings. */
unsigned char g_random_names_off[] = {
#include "gen/g_random_names_off.inc"
};

/**
 * @brief Panel record offset table.
 * @note TODO: currently absorbs the unnamed data that follows it (up to
 *       g_name_entry_tim); the true record count/layout is still being
 *       reverse-engineered.
 */
unsigned char g_panel_record_offsets[] = {
#include "gen/g_panel_record_offsets.inc"
};

/** @brief Embedded TIM image for the name-entry screen artwork. */
unsigned char g_name_entry_tim[] = {
#include "gen/g_name_entry_tim.inc"
};

/** @brief Layout sprite draw sequence. */
unsigned char g_layout_sprite_sequence[] = {
#include "gen/g_layout_sprite_sequence.inc"
};

/** @brief Glyph append animation frame table. */
unsigned char g_glyph_append_anim_frames[] = {
#include "gen/g_glyph_append_anim_frames.inc"
};
