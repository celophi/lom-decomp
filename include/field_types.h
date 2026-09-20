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

#endif
