#ifndef _VECTOR_H
#define _VECTOR_H

#include "common.h"

/** @brief Two-component signed 16-bit vector. */
typedef struct
{
    s16 x;
    s16 y;
} Vec2s;

/** @brief Three-component signed 32-bit vector. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
} Vec3i;

#endif
