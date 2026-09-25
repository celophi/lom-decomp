#ifndef _FIELD_TYPES_H
#define _FIELD_TYPES_H

#include "common.h"
#include "vector.h"
#include "sdk/libgte.h"

/** @brief Integer-coordinate work vector used by actor movement calculations. */
typedef struct
{
    s32 vx;
    s32 vy;
    s32 vz;
    s32 pad;
} FieldVector;

/** @brief Compact field entity position/state record. */
typedef struct
{
    s32 vx;
    s32 vy;
    s32 vz;
    u8 unk0C[0x19];
    u8 unk25;
} FieldEntity;

/** @brief Angle units of the GTE rotation helpers: 0x1000 is a full turn. */
#define FIELD_ANGLE_TURN 0x1000
#define FIELD_ANGLE_HALF_TURN 0x800
#define FIELD_ANGLE_QUARTER_TURN 0x400
#define FIELD_ANGLE_MASK 0xFFF

#endif
