#ifndef WMAP_MAP_LABELS_H
#define WMAP_MAP_LABELS_H

#include "common.h"

void wmap_draw_status_panel(void);
void wmap_draw_map_labels(void);
void wmap_set_map_label(s32 selection);
void wmap_init_label_sprites(void);

/** @brief Auxiliary label set selected by the map and artifact controls. */
extern s32 g_wmap_auxiliary_label_mode;

/** @brief Auxiliary label mode currently used to select the glyph range. */
extern s32 g_wmap_auxiliary_label_draw_mode;

/** @brief Nonzero to update land labels during map-cell selection. */
extern s32 g_wmap_land_label_updates_enabled;

/** @brief Label texture of each land. */
extern s16 g_wmap_land_label_cells[];

#endif
