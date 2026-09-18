#ifndef ZUKAN_CATEGORY_H
#define ZUKAN_CATEGORY_H

#include "common.h"

/**
 * @brief Build and filter the encyclopedia entries for one category.
 * @param category Category index to process.
 * @param entry_values Output array of entry resource values.
 * @param entry_indices Output array of entry indices.
 * @return Number of entries remaining after filtering.
 */
s32 zukan_build_category_entries(s32 category, s32* entry_values, s32* entry_indices);

#endif
