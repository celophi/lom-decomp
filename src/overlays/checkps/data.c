#include "common.h"

const u32 D_8004FCC4[15] = {
    0xA790AD8B, 0xB997498F, 0xDC82B582, 0xBD82B582, 0x960A4281, 0x82CC917B, 0x91FC89AA, 0x82B382A2,
    0x82C482EA, 0x0AE982A2, 0xBB82A882, 0xAA82EA82, 0xE882A082, 0xB782DC82, 0x00004281,
};

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