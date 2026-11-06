#ifndef OVERLAYS_NIKI_H
#define OVERLAYS_NIKI_H

#include "common.h"

/**
 * @file niki.h
 * @brief NIKI's entry point, called by FIELD once the overlay is loaded.
 */

/**
 * @brief Start NIKI.
 * @param work Work buffer FIELD passes.
 * @param mode Mode FIELD opened NIKI in.
 */
void niki_init(void* work, s32 mode);

#endif
