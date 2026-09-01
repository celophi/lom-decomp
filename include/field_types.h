#ifndef _FIELD_TYPES_H
#define _FIELD_TYPES_H

#include "common.h"

/** @brief Two-component signed field-space vector. */
typedef struct
{
    s16 x;
    s16 y;
} Vec2s;

/** @brief Three-component signed field vector with a trailing pad word. */
typedef struct
{
    s32 vx;
    s32 vy;
    s32 vz;
    s32 pad;
} FieldVector;

/** @brief Field-space 3x3 transform matrix with translation vector. */
typedef struct
{
    s16 m[3][3];
    s32 t[3];
} FieldMatrix;

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
