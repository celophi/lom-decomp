#include "common.h"

/** @brief Eight packed lookup indices for coordinate adjustments. */
typedef struct
{
    u8 pad[0x1C];
    u32 n0 : 4;
    u32 n1 : 4;
    u32 n2 : 4;
    u32 n3 : 4;
    u32 n4 : 4;
    u32 n5 : 4;
    u32 n6 : 4;
    u32 n7 : 4;
} Source;
/** @brief Two seven-bit fields embedded in a packed result word. */
typedef struct
{
    u32 low : 9;
    u32 x : 7;
    u32 middle : 9;
    u32 y : 7;
} Pair;
/** @brief Four packed words adjusted by the source indices. */
typedef struct
{
    u8 pad[0x30];
    Pair pairs[4];
} Result;
extern volatile u8 D_800F0C38[];
/**
 * @brief Apply signed table adjustments to eight packed result fields.
 * @param source Packed four-bit indices into the adjustment table.
 * @param result Destination fields updated modulo 128.
 * @note Volatile table access preserves unsigned loads before sign extension.
 */
void func_800B7D10(Source *source, Result *result)
{
    u32 y;
    result->pairs[0].x = (s8)D_800F0C38[source->n0] + result->pairs[0].x;
    y = result->pairs[0].y;
    y += (s8)D_800F0C38[source->n1];
    result->pairs[0].y = y;
    result->pairs[1].x = (s8)D_800F0C38[source->n2] + result->pairs[1].x;
    y = result->pairs[1].y;
    y += (s8)D_800F0C38[source->n3];
    result->pairs[1].y = y;
    result->pairs[2].x = (s8)D_800F0C38[source->n4] + result->pairs[2].x;
    y = result->pairs[2].y;
    y += (s8)D_800F0C38[source->n5];
    result->pairs[2].y = y;
    result->pairs[3].x = (s8)D_800F0C38[source->n6] + result->pairs[3].x;
    y = result->pairs[3].y;
    y += (s8)D_800F0C38[source->n7];
    result->pairs[3].y = y;
}
