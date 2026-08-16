#include "checkps.h"

const u32 D_8004FCC4[15] = {
    0xA790AD8B, 0xB997498F, 0xDC82B582, 0xBD82B582, 0x960A4281, 0x82CC917B, 0x91FC89AA, 0x82B382A2,
    0x82C482EA, 0x0AE982A2, 0xBB82A882, 0xAA82EA82, 0xE882A082, 0xB782DC82, 0x00004281,
};

/**
 * decomp.me link (95.20%) https://decomp.me/scratch/taoth
 * Matches 100% with GNU AS
 */
void DrawSymmetricTestPattern(void)
{
    s32 polyF4[6];
    s32 patternCount;
    s32 quadCount;
    s32 vertexCount;
    s32* polyF4Ref;
    u8* stack_base;
    s32* out_ptr;
    u8* table;
    s32 tableSizePairs;
    s32 color;
    u8* reg_local_buf;
    s8* reg_read_ptr;

    polyF4[0] = 0x05000000;
    polyF4[1] = 0x280000FF;

    for (patternCount = 0; patternCount < 16; patternCount++)
    {
        for (quadCount = 0; quadCount < 4; quadCount++)
        {
            s8 g_testPatternVertexTable[][2] = {0x01, 0x01, 0xFF, 0x01, 0x01, 0xFF, 0xFF, 0xFF};

            for (vertexCount = 0; vertexCount < 4; vertexCount++)
            {
                // Calculate table index; results in 0,0,1,1 for vertices 0-3
                s32 tableIndex = (vertexCount + 2) >> 2;

                // Alternate between current outer loop index and the next one (patternCount, patternCount + 1)
                s32 tableOffset = patternCount + (vertexCount & 1);

                // Center the vertex on screen (X+120, Y+160) and pack as [Short X][Short Y] into a 32-bit word
                polyF4[vertexCount + 2] =
                    (((g_testPatternVertexTable[quadCount][1] * g_testPatternSizeTable[16 - tableOffset][tableIndex]) + 120)
                     << 16) |
                    ((g_testPatternVertexTable[quadCount][0] * g_testPatternSizeTable[tableOffset][tableIndex]) + 160);
            }

            DrawPrim(polyF4);
        }
    }

    // Draw final static center quad
    polyF4[2] = 0x00500070;
    polyF4[3] = 0x00480078;
    polyF4[4] = 0x00A800C8;
    polyF4[5] = 0x00A000D0;

    DrawPrim(polyF4);
}

const u8 g_testPatternVertexTablePadding[12] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
