#include "checkps_internal.h"

#include "display.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"

#define CHECKPS_PATTERN_RING_COUNT 16
#define CHECKPS_PATTERN_SIZE_COUNT (CHECKPS_PATTERN_RING_COUNT + 1)
#define CHECKPS_PATTERN_QUADRANT_COUNT 4
#define CHECKPS_PATTERN_VERTEX_COUNT 4
#define CHECKPS_PATTERN_PACKET_WORD_COUNT 6
#define CHECKPS_PATTERN_VERTEX_WORD_BASE 2
#define CHECKPS_PATTERN_PACKET_TAG 0x05000000
#define CHECKPS_PATTERN_RED_POLY_F4_COMMAND 0x280000FF
#define CHECKPS_PATTERN_FIXED_INNER_OFFSET 40
#define CHECKPS_PATTERN_FIXED_OUTER_OFFSET 48
#define CHECKPS_PATTERN_VERTEX_PADDING_SIZE 12
#define CHECKPS_PATTERN_PACK_XY(x, y) (((y) << 16) | (x))

/** @brief POLY_F4 packet with indexed access to its six 32-bit GPU words. */
typedef union
{
    POLY_F4 polygon;
    u32 words[CHECKPS_PATTERN_PACKET_WORD_COUNT];
} CheckPSPatternPacket;

/** @brief X/Y signs selecting each screen quadrant. */
typedef struct
{
    s8 values[CHECKPS_PATTERN_QUADRANT_COUNT][2];
} CheckPSPatternVertexSigns;

extern u8 g_hardware_pattern_size_table[CHECKPS_PATTERN_SIZE_COUNT][2];

/**
 * @brief Shift-JIS hardware-modification warning shown before termination.
 *
 * Decoded meaning:
 *   "Execution was forcibly terminated."
 *   "The console may have been modified."
 *
 * The original message is split across three display lines. It remains packed
 * as 15 little-endian words to preserve its exact 60-byte layout, including
 * two trailing NUL bytes.
 */
const u32 g_hardware_modification_warning[CHECKPS_HARDWARE_WARNING_WORD_COUNT] = {
    0xA790AD8B, 0xB997498F, 0xDC82B582, 0xBD82B582, 0x960A4281, 0x82CC917B, 0x91FC89AA, 0x82B382A2,
    0x82C482EA, 0x0AE982A2, 0xBB82A882, 0xAA82EA82, 0xE882A082, 0xB782DC82, 0x00004281,
};

/** Signs used to reflect one ring segment into all four quadrants. */
const CheckPSPatternVertexSigns g_hardware_pattern_vertex_signs = {
    {
        {1, 1},
        {-1, 1},
        {1, -1},
        {-1, -1},
    },
};

/**
 * @brief Draw the concentric diagnostic pattern used by the failure screen.
 */
void draw_hardware_check_pattern(void)
{
    CheckPSPatternPacket packet;
    s32 ring_index;
    s32 quadrant_index;
    s32 vertex_index;

    packet.words[0] = CHECKPS_PATTERN_PACKET_TAG;
    packet.words[1] = CHECKPS_PATTERN_RED_POLY_F4_COMMAND;
    for (ring_index = 0; ring_index < CHECKPS_PATTERN_RING_COUNT; ring_index++)
    {
        for (quadrant_index = 0; quadrant_index < CHECKPS_PATTERN_QUADRANT_COUNT; quadrant_index++)
        {
            CheckPSPatternVertexSigns quadrant_signs = g_hardware_pattern_vertex_signs;

            for (vertex_index = 0; vertex_index < CHECKPS_PATTERN_VERTEX_COUNT; vertex_index++)
            {
                /* Select width or height: vertices 0-1 use width, 2-3 use height. */
                s32 dimension_index = (vertex_index + 2) >> 2;
                /* Alternate between the inner and outer edge of each ring. */
                s32 size_index = ring_index + (vertex_index & 1);

                packet.words[vertex_index + CHECKPS_PATTERN_VERTEX_WORD_BASE] =
                    CHECKPS_PATTERN_PACK_XY(
                        (quadrant_signs.values[quadrant_index][0] * g_hardware_pattern_size_table[size_index][dimension_index]) + (SCREEN_WIDTH / 2),
                        (quadrant_signs.values[quadrant_index][1] * g_hardware_pattern_size_table[CHECKPS_PATTERN_RING_COUNT - size_index][dimension_index]) + (SCREEN_HEIGHT / 2));
            }

            DrawPrim(&packet.polygon);
        }
    }

    /* Draw the fixed inner quadrilateral around the screen center. */
    packet.words[2] = CHECKPS_PATTERN_PACK_XY(
        (SCREEN_WIDTH / 2) - CHECKPS_PATTERN_FIXED_OUTER_OFFSET,
        (SCREEN_HEIGHT / 2) - CHECKPS_PATTERN_FIXED_INNER_OFFSET);
    packet.words[3] = CHECKPS_PATTERN_PACK_XY(
        (SCREEN_WIDTH / 2) - CHECKPS_PATTERN_FIXED_INNER_OFFSET,
        (SCREEN_HEIGHT / 2) - CHECKPS_PATTERN_FIXED_OUTER_OFFSET);
    packet.words[4] = CHECKPS_PATTERN_PACK_XY(
        (SCREEN_WIDTH / 2) + CHECKPS_PATTERN_FIXED_INNER_OFFSET,
        (SCREEN_HEIGHT / 2) + CHECKPS_PATTERN_FIXED_OUTER_OFFSET);
    packet.words[5] = CHECKPS_PATTERN_PACK_XY(
        (SCREEN_WIDTH / 2) + CHECKPS_PATTERN_FIXED_OUTER_OFFSET,
        (SCREEN_HEIGHT / 2) + CHECKPS_PATTERN_FIXED_INNER_OFFSET);

    DrawPrim(&packet.polygon);
}

/** Zero-filled padding that preserves the following fixed symbol address. */
const u8 g_hardware_pattern_vertex_padding[CHECKPS_PATTERN_VERTEX_PADDING_SIZE] = {
    0,
};
