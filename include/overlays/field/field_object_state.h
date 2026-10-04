#ifndef FIELD_OBJECT_STATE_H
#define FIELD_OBJECT_STATE_H

#include "overlays/field/field_types.h"
#include "overlays/field/field_interaction_start.h"
#include "overlays/field/field_actor_records.h"

#define FIELD_OBJECT_EFFECT_SCALE_MASK 0x3FF
/** @brief Two-bit movement mode in FieldMovementWord (0, 0x800 or 0x1000). */
#define FIELD_OBJECT_MOVEMENT_MODE_MASK 0x1800
#define FIELD_OBJECT_FOOTPRINT_STRENGTH_RANGE 256

#endif
