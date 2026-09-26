#ifndef FIELD_PATH_INTERPOLATION_H
#define FIELD_PATH_INTERPOLATION_H

#include "common.h"
#include "field_effect_types.h"

/** @brief Path groups; path-mode effects take the next one in turn. */
#define FIELD_PATH_GROUP_COUNT 32

void field_update_path_position(u8* time, FieldMotionRecord* record, s32 group);
void field_init_path(FieldMotionRecord* center, s32 radius, s32 randomize, s32 group);

#endif
