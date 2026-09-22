#ifndef WMAP_LAND_LAYOUT_H
#define WMAP_LAND_LAYOUT_H

#include "common.h"

/** @brief Per-land field scene identifiers from the current layout file. */
extern u16* g_wmap_land_scenes;
/** @brief Per-land field entry flags from the current layout file. */
extern u8* g_wmap_land_entry_flags;
/** @brief Packed saved status of the replacement land. */
extern u32 g_wmap_replacement_land_status;

void wmap_update_travel_growth(void);
s32 wmap_can_place_land(u32 x, s32 y, s32 land);
void wmap_place_land(s32 x, s32 y, s32 index);
void wmap_get_proposed_spirit_sprites(u32 x, s32 y, u32 land, u32 proposed_x, s32 proposed_y, s32* output);
s32 wmap_load_land_layout(void);
s32 wmap_next_land_event(void);
void wmap_scroll_artifact_list(s32 forward, s32* visible_entries);
s32 wmap_build_artifact_list(s32 selection);
void wmap_build_placement_labels(s32 selected_cell, u32 x, s32 y, s32* groups, s32* output, s32 land);
s32 wmap_get_day(void);
s32 wmap_get_land_count_tier(void);
s32 wmap_is_land_active(s32 record_index);
s32 wmap_is_other_land_cell(u32 x, s32 y);
s32 wmap_get_remaining_land_count(void);
s32 wmap_get_land_at_cell(u32 column, s32 row);
s32 wmap_get_cell_spirit_sprites(u32 column, s32 row, s32* output);
void wmap_get_artifact_spirit_sprites(s32 table_index, s32* output);
s32 wmap_is_artifact_unplaced(s32 record_index);
s32 wmap_get_starting_cell(s32* x, s32* y);
s32 wmap_get_selected_artifact(void);
s32 wmap_get_spirit_sprite(s32 row, s32 column);
s32 wmap_has_growth_blocking_event(void);
s32 wmap_has_persistent_land_event(void);
void wmap_add_artifact_influence(s32 table_row, s32 output_row, s32 output_address);
void wmap_add_saved_land_influence(s32 record_index, s32 other_index, s32 unused, s32* values);

void func_8005D46C(void);
void func_8005D474(void);
void func_8005D47C(void);
void func_8005D484(void);
void func_8005D48C(void);

#endif
