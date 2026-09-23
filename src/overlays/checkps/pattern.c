#include "checkps_internal.h"

#include "display.h"
#include "gpu_packet.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

#define CHECKPS_PATTERN_RING_COUNT 16
#define CHECKPS_PATTERN_SIZE_COUNT (CHECKPS_PATTERN_RING_COUNT + 1)
#define CHECKPS_PATTERN_QUADRANT_COUNT 4
#define CHECKPS_PATTERN_VERTEX_COUNT 4
/* POLY_F4 packet length in words after the tag, and its GPU command code. */
#define CHECKPS_PATTERN_PACKET_LENGTH 5
#define CHECKPS_PATTERN_POLY_F4_CODE 0x28
#define CHECKPS_PATTERN_FIXED_INNER_OFFSET 40
#define CHECKPS_PATTERN_FIXED_OUTER_OFFSET 48
#define CHECKPS_PATTERN_PACK_XY(x, y) (((y) << 16) | (x))

/**
 * @brief Flat-shaded quadrilateral packet (POLY_F4 layout) built from whole words.
 */
typedef struct
{
    u32 tag;
    u32 color_code;                       /* RGB in the low 24 bits, GPU command code in the top byte. */
    u32 xy[CHECKPS_PATTERN_VERTEX_COUNT]; /* Packed vertices, y in the high half. */
} CheckPSPatternPacket;

/** @brief X/Y signs selecting each screen quadrant. */
typedef struct
{
    s8 values[CHECKPS_PATTERN_QUADRANT_COUNT][2];
} CheckPSPatternVertexSigns;

extern u8 g_hardware_pattern_size_table[CHECKPS_PATTERN_SIZE_COUNT][2];

/**
 * @brief Signs used to reflect one ring segment into all four quadrants.
 */
extern const CheckPSPatternVertexSigns g_hardware_pattern_vertex_signs;

/**
 * @brief Draw the concentric diagnostic pattern used by the failure screen.
 */
void draw_hardware_check_pattern(void)
{
    CheckPSPatternPacket packet;
    s32 ring_index;
    s32 quadrant_index;
    s32 vertex_index;

    packet.tag = CHECKPS_PATTERN_PACKET_LENGTH << 24;
    packet.color_code = (CHECKPS_PATTERN_POLY_F4_CODE << 24) | GPU_COLOR_WORD(0xFF, 0, 0);
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

                packet.xy[vertex_index] = CHECKPS_PATTERN_PACK_XY(
                    (quadrant_signs.values[quadrant_index][0] * g_hardware_pattern_size_table[size_index][dimension_index]) + (SCREEN_WIDTH / 2),
                    (quadrant_signs.values[quadrant_index][1] * g_hardware_pattern_size_table[CHECKPS_PATTERN_RING_COUNT - size_index][dimension_index]) +
                        (SCREEN_HEIGHT / 2));
            }

            DrawPrim(&packet);
        }
    }

    /* Draw the fixed inner quadrilateral around the screen center. */
    packet.xy[0] = CHECKPS_PATTERN_PACK_XY((SCREEN_WIDTH / 2) - CHECKPS_PATTERN_FIXED_OUTER_OFFSET, (SCREEN_HEIGHT / 2) - CHECKPS_PATTERN_FIXED_INNER_OFFSET);
    packet.xy[1] = CHECKPS_PATTERN_PACK_XY((SCREEN_WIDTH / 2) - CHECKPS_PATTERN_FIXED_INNER_OFFSET, (SCREEN_HEIGHT / 2) - CHECKPS_PATTERN_FIXED_OUTER_OFFSET);
    packet.xy[2] = CHECKPS_PATTERN_PACK_XY((SCREEN_WIDTH / 2) + CHECKPS_PATTERN_FIXED_INNER_OFFSET, (SCREEN_HEIGHT / 2) + CHECKPS_PATTERN_FIXED_OUTER_OFFSET);
    packet.xy[3] = CHECKPS_PATTERN_PACK_XY((SCREEN_WIDTH / 2) + CHECKPS_PATTERN_FIXED_OUTER_OFFSET, (SCREEN_HEIGHT / 2) + CHECKPS_PATTERN_FIXED_INNER_OFFSET);

    DrawPrim(&packet);
}
