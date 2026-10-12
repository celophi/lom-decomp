#ifndef CLOAD_H
#define CLOAD_H

#include "common.h"

/** @brief cload_main result: a save was loaded. */
#define CLOAD_RESULT_LOADED 0

/** @brief cload_main result: the player backed out to the title screen. */
#define CLOAD_RESULT_CANCELLED 1

/**
 * @brief Run the memory-card load menu.
 * @return CLOAD_RESULT_LOADED or CLOAD_RESULT_CANCELLED.
 */
s32 cload_main(void);

#endif
