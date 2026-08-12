/**
 * @file gname_data.c
 * @brief Panel-character / glyph layout data for the name-entry (GNAME) overlay.
 *
 * This translation unit builds the overlay's entire .data region from C. Each
 * table corresponds to a symbol in config/symbols/gname_symbol_addrs.txt and is
 * declared with the record type gname.c reads it through (see gname_types.h).
 * Tables appear in ascending-address order so the linker lays them out
 * contiguously at their fixed addresses (0x80142C98 onward).
 *
 * The initializer values live in gitignored, generated fragments under gen/,
 * produced by tools/gen_gname_data.py from the original disc image, so no
 * copyrighted data is committed. If a record type's layout is wrong, the
 * compiled bytes diverge from the original and the overlay stops matching.
 *
 * @note All tables are non-const so they land in the writable .data section.
 * @note Regenerate the fragments with: python tools/gen_gname_data.py
 */

#include "common.h"
#include "gname_types.h"

/**
 * @brief Record-index boundaries for the non-kanji character panels.
 * @note gname.c reads index 3 as g_kanji_cat_names_offset and index 4 as the
 *       following boundary word, both of which live in the next symbol.
 */
u32 g_panel_char_offsets[] = {
#include "gen/g_panel_char_offsets.inc"
};

/**
 * @brief First record-offset index of the kanji category name records.
 * @note gname.c declares this as a scalar s32; the symbol actually spans two
 *       words - the offset [0] and the boundary word [1] read via
 *       g_panel_char_offsets[4].
 */
s32 g_kanji_cat_names_offset[] = {
#include "gen/g_kanji_cat_names_offset.inc"
};

/** @brief Kanji category entry index table ([cat] -> sub-index; 0xFF = empty). */
u32 g_kanji_cat_entries[] = {
#include "gen/g_kanji_cat_entries.inc"
};

/** @brief Glyph metrics table indexed by character id. */
GlyphInfo g_glyph_table[] = {
#include "gen/g_glyph_table.inc"
};

/** @brief Scroll-up [0] and scroll-down [1] indicator entries. */
TabCursorEntry g_tab_cursor_pos[] = {
#include "gen/g_tab_cursor_pos.inc"
};

/** @brief Cursor targets and glyphs for the 11 action/panel selection entries. */
TabCursorEntry g_tab_cursor_entries[] = {
#include "gen/g_tab_cursor_entries.inc"
};

/** @brief Kanji sub-index to glyph offset lookup table. */
u32 g_kanji_entry_offsets[] = {
#include "gen/g_kanji_entry_offsets.inc"
};

/* --- Character-panel data blob header (see PanelDataHeader in gname.c) -------
 * These five u32 byte-offset fields stay as separate contiguous symbols; the
 * code derives the blob base from each field's own address at runtime. */

/** @brief Blob + 0x00: stored value 4; purpose unknown. */
u32 g_panel_data_base =
#include "gen/g_panel_data_base.inc"
;

/** @brief Blob + 0x04: offset (0x14) of the u16 record-offset table. */
u32 g_panel_tbl_off =
#include "gen/g_panel_tbl_off.inc"
;

/** @brief Blob + 0x08: offset (0x2A0) of the kanji panel glyph data. */
u32 g_kanji_panel_offset =
#include "gen/g_kanji_panel_offset.inc"
;

/** @brief Blob + 0x0C: offset (0x3754) of the history name list. */
u32 g_history_names_off =
#include "gen/g_history_names_off.inc"
;

/** @brief Blob + 0x10: offset (0x3C9C) of the random name pool. */
u32 g_random_names_off =
#include "gen/g_random_names_off.inc"
;

/**
 * @brief Blob + 0x14: u16 record-offset table followed by the packed panel
 *        records (glyph lists, category labels, tab sprites, kanji names).
 * @note The first 138 entries are the offset table; the remainder is the
 *       record data the offsets point into. Kept as u16[] pending record RE.
 */
u16 g_panel_record_offsets[] = {
#include "gen/g_panel_record_offsets.inc"
};

/**
 * @brief Embedded TIM image for the name-entry screen artwork.
 * @note gname.c reads this as a Tim (header + CLUT, pixel block follows); kept
 *       as a byte blob here since the pixel data has no further structure.
 */
u8 g_name_entry_tim[] = {
#include "gen/g_name_entry_tim.inc"
};

/** @brief Layout sprite draw sequence (20 entries walked each frame). */
GlyphSeqEntry g_layout_sprite_sequence[] = {
#include "gen/g_layout_sprite_sequence.inc"
};

/** @brief Glyph append animation frames (7 frames, frame 0 = resting). */
GlyphAppendAnimFrame g_glyph_append_anim_frames[] = {
#include "gen/g_glyph_append_anim_frames.inc"
};

/**
 * @brief Four bytes trailing the animation frames, before .bss.
 * @note Purpose unknown; kept as its own symbol so the .data section length
 *       matches the original. External linkage ensures it is always emitted.
 */
u8 gname_anim_frames_tail[] = {
#include "gen/g_glyph_append_anim_frames_tail.inc"
};
