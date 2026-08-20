#include "checkps_internal.h"

const u32 g_hardware_modification_warning[15] = {
    0xA790AD8B, 0xB997498F, 0xDC82B582, 0xBD82B582, 0x960A4281, 0x82CC917B, 0x91FC89AA, 0x82B382A2,
    0x82C482EA, 0x0AE982A2, 0xBB82A882, 0xAA82EA82, 0xE882A082, 0xB782DC82, 0x00004281,
};
/**
 * @brief Draw the concentric diagnostic pattern used by the failure screen.
 */
void draw_hardware_check_pattern(void)
{
    s32 quad_packet_words[6];
    s32 ring_index;
    s32 quadrant_index;
    s32 vertex_index;

    quad_packet_words[0] = CHECKPS_PATTERN_GPU_TAG;
    quad_packet_words[1] = CHECKPS_PATTERN_GPU_POLY_F4;
    for (ring_index = 0; ring_index < CHECKPS_PATTERN_RING_COUNT; ring_index++)
    {
        for (quadrant_index = 0; quadrant_index < CHECKPS_PATTERN_QUADRANT_COUNT; quadrant_index++)
        {
            s8 quadrant_signs[][2] = {0x01, 0x01, 0xFF, 0x01, 0x01, 0xFF, 0xFF, 0xFF};

            for (vertex_index = 0; vertex_index < 4; vertex_index++)
            {
                // Select width (0) or height (1); this yields 0,0,1,1 for vertices 0-3
                s32 dimension_index = (vertex_index + 2) >> 2;
                // Alternate between the inner and outer size for successive vertices
                s32 size_index = ring_index + (vertex_index & 1);
                // Pack PSX XY as x in the low halfword (center 160), y in the high halfword (center 120)
                quad_packet_words[vertex_index + 2] =
                    (((quadrant_signs[quadrant_index][1] * g_hardware_pattern_size_table[CHECKPS_PATTERN_RING_COUNT - size_index][dimension_index]) + (SCREEN_HEIGHT / 2))
                     << 16) |
                    ((quadrant_signs[quadrant_index][0] * g_hardware_pattern_size_table[size_index][dimension_index]) + (SCREEN_WIDTH / 2));
            }

            DrawPrim(quad_packet_words);
        }
    }
    // Draw the final fixed center quad.
    quad_packet_words[2] = 0x00500070;
    quad_packet_words[3] = 0x00480078;
    quad_packet_words[4] = 0x00A800C8;
    quad_packet_words[5] = 0x00A000D0;

    DrawPrim(quad_packet_words);
}

const u8 g_hardware_pattern_vertex_padding[12] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
