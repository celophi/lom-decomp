#include "common.h"

/**
 * Lookup table for test pattern vertex offsets,
 * used in DrawSymmetricTestPattern. Contains 4 pairs of X/Y sign values.
 */
const u8 g_testPatternVertexTable[8] = {
    0x01, 0x01, 0xFF, 0x01, 0x01, 0xFF, 0xFF, 0xFF,
};

/**
 * Additional padding to align the next data in memory,
 * since the vertex table is only 8 bytes but the next item starts at an offset of 20 bytes.
 */
const u8 g_testPatternVertexTablePadding[12] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};