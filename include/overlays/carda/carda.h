#ifndef OVERLAYS_CARDA_H
#define OVERLAYS_CARDA_H

#include "common.h"

/**
 * @file carda.h
 * @brief CARDA's entry point, called by FIELD once the overlay is loaded.
 */

/**
 * @brief Start CARDA.
 * @param work Work buffer FIELD passes.
 * @param mode CARDA mode (memory card save, load, or PocketStation transfer).
 */
void carda_init(void* work, s32 mode);

#endif
