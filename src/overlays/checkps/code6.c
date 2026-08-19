#include "checkps.h"

const u32 g_hardwareModificationWarning[15] = {
    0xA790AD8B, 0xB997498F, 0xDC82B582, 0xBD82B582, 0x960A4281, 0x82CC917B, 0x91FC89AA, 0x82B382A2,
    0x82C482EA, 0x0AE982A2, 0xBB82A882, 0xAA82EA82, 0xE882A082, 0xB782DC82, 0x00004281,
};
void DrawHardwareCheckPattern(void)
{
    s32 quadPacketWords[6];
    s32 ringIndex;
    s32 quadrantIndex;
    s32 vertexIndex;

    quadPacketWords[0] = 0x05000000;
    quadPacketWords[1] = 0x280000FF;
    for (ringIndex = 0; ringIndex < 16; ringIndex++)
    {
        for (quadrantIndex = 0; quadrantIndex < 4; quadrantIndex++)
        {
            s8 quadrantSigns[][2] = {0x01, 0x01, 0xFF, 0x01, 0x01, 0xFF, 0xFF, 0xFF};

            for (vertexIndex = 0; vertexIndex < 4; vertexIndex++)
            {
                // Select width (0) or height (1); this yields 0,0,1,1 for vertices 0-3
                s32 dimensionIndex = (vertexIndex + 2) >> 2;
                // Alternate between the inner and outer size for successive vertices
                s32 sizeIndex = ringIndex + (vertexIndex & 1);
                // Pack PSX XY as x in the low halfword (center 160), y in the high halfword (center 120)
                quadPacketWords[vertexIndex + 2] =
                    (((quadrantSigns[quadrantIndex][1] * g_hardwarePatternSizeTable[16 - sizeIndex][dimensionIndex]) + 120)
                     << 16) |
                    ((quadrantSigns[quadrantIndex][0] * g_hardwarePatternSizeTable[sizeIndex][dimensionIndex]) + 160);
            }

            DrawPrim(quadPacketWords);
        }
    }
    // Draw the final fixed center quad.
    quadPacketWords[2] = 0x00500070;
    quadPacketWords[3] = 0x00480078;
    quadPacketWords[4] = 0x00A800C8;
    quadPacketWords[5] = 0x00A000D0;

    DrawPrim(quadPacketWords);
}

const u8 g_hardwarePatternVertexPadding[12] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
