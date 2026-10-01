#ifndef ZUKAN_CATEGORY_H
#define ZUKAN_CATEGORY_H

#include "common.h"

/** @brief Encyclopedia categories, in the order of the UI archive's category names. */
#define ZUKAN_CATEGORY_LANDS 1
#define ZUKAN_CATEGORY_ARTIFACTS 2
#define ZUKAN_CATEGORY_EQUIPMENT 3
#define ZUKAN_CATEGORY_ITEMS 4
#define ZUKAN_CATEGORY_PRODUCE 5
#define ZUKAN_CATEGORY_MONSTERS 6
#define ZUKAN_CATEGORY_CHARACTERS 7
#define ZUKAN_CATEGORY_WORLD_HISTORY 8
#define ZUKAN_CATEGORY_CACTUS_DIARIES 9
#define ZUKAN_CATEGORY_GOLEMOLOGY 10
#define ZUKAN_CATEGORY_TECHNIQUES 11

/**
 * @brief Build and filter the encyclopedia entries for one category.
 * @param category Category index to process.
 * @param entry_values Output array of entry resource values.
 * @param entry_indices Output array of entry indices.
 * @return Number of entries remaining after filtering.
 */
s32 zukan_build_category_entries(s32 category, s32* entry_values, s32* entry_indices);

#endif
