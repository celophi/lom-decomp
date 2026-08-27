#include "common.h"

/**
 * @brief Field record fields used by the scaled position query.
 */
typedef struct Record94690
{
    u8 pad0[0x20];
    u8 scale;
    u8 pad21[0x2A - 0x21];
    s16 value;
} Record94690;

/**
 * @brief Three-component scratch vector stored in scratchpad RAM.
 */
typedef struct Vector94690
{
    s32 x;
    s32 y;
    s32 z;
} Vector94690;

s32 func_80097FA0(Record94690 *, Vector94690 *, s32, s32);

/**
 * @brief Tests a scaled X/Z displacement for a field record.
 *
 * Builds a scratchpad vector from the supplied X/Z components and the byte
 * scale at record offset 0x20. A failed query clears the record halfword at
 * offset 0x2A.
 *
 * @param record Record supplying the scale and result halfword.
 * @param x X displacement before scaling.
 * @param z Z displacement before scaling.
 * @note 99.629630% match with two allocator-only register differences.
 */
void func_80094690(Record94690 *record, s32 x, s32 z)
{
    Vector94690 *vector;
    s32 scaled_x;
    s32 scaled_z;

    scaled_x = x * record->scale;
    vector = (Vector94690 *)0x1F800000;
    vector->y = 0;
    vector->x = scaled_x;
    scaled_z = z * record->scale;
    vector->z = scaled_z;
    if (func_80097FA0(record, vector, 0, scaled_z) == 0)
    {
        record->value = 0;
    }
}
