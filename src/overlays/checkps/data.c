#include "common.h"

/**
 * Lookup table for test pattern vertex offsets,
 * used in DrawSymmetricTestPattern. Contains 4 pairs of X/Y sign values.
 */
const s8 g_testPatternVertexTable[8] = {
    1, 1, -1, 1, 1, -1, -1, -1,
};

/**
 * Additional padding to align the next data in memory,
 * since the vertex table is only 8 bytes but the next item starts at an offset of 20 bytes.
 */
const u8 g_testPatternVertexTablePadding[12] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};