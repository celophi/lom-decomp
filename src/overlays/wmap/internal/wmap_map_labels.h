#ifndef WMAP_MAP_LABELS_H
#define WMAP_MAP_LABELS_H

#include "common.h"

void func_8005F9BC(void);
void func_8005FF88(s32 selection);
void wmap_init_label_sprites(void);

/** @brief Auxiliary label set selected by the map and artifact controls. */
extern s32 g_wmap_auxiliary_label_mode;

/** @brief Auxiliary label mode currently used to select the glyph range. */
extern s32 g_wmap_auxiliary_label_draw_mode;

/** @brief Nonzero to update land labels during map-cell selection. */
extern s32 g_wmap_land_label_updates_enabled;

#endif
