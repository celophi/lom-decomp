#include "checkps.h"

/**
 * decomp.me link (95.20%) https://decomp.me/scratch/taoth
 * Matches 100% with GNU AS
 */
void DrawSymmetricTestPattern(void)
{
    s32 polyF4[6];
    u8 mirrorSignBuffer[8];
    s32 patternCount;
    s32 quadCount;
    s32 vertexCount;
    s32* polyF4Base;
    u8* signBufferBase;
    s32* vertexCursor;
    u8* table;
    s32 tableSizePairs;
    s32 color;
    u8* currentSignPtr;
    s8* currentSignValue;

    currentSignValue = (s8*)currentSignPtr;
    color = 0x280000FF;
    patternCount = 0;
    polyF4Base = polyF4;
    signBufferBase = mirrorSignBuffer;
    tableSizePairs = 16;

    polyF4[0] = 0x05000000;
    polyF4[1] = color;

    while (patternCount < 16)
    {
        quadCount = 0;
        currentSignPtr = signBufferBase;

        while (quadCount < 4)
        {
            __builtin_memcpy(mirrorSignBuffer, g_testPatternVertexTable, 8);
            vertexCount = 0;
            currentSignValue = (s8*)currentSignPtr;
            vertexCursor = polyF4Base;

            while (vertexCount < 4)
            {
                // Calculate table index; results in 0,0,1,1 for vertices 0-3
                s32 tableIndex = (vertexCount + 2) >> 2;

                // Alternate between current outer loop index and the next one (patternCount, patternCount + 1)
                s32 tableOffset = patternCount + (vertexCount & 1);

                // Locate base coordinate offsets from the data table (g_testPatternSizeTable)
                // ptr_mirror picks from the end of the table, ptr_base picks from the start
                u8* ptr1 = g_testPatternSizeTable + ((tableSizePairs - tableOffset) << 1);
                u8* ptr2 = g_testPatternSizeTable + (tableOffset << 1);

                // Scale the base offsets by the sign pairs (1 or -1) from g_testPatternVertexTable
                // currentSignValue[1] is the Y-scale, currentSignValue[0] is the X-scale
                s32 offsetX = ((s8)currentSignValue[1]) * ptr1[tableIndex];
                s32 offsetY = ((s8)currentSignValue[0]) * ptr2[tableIndex];

                // Center the vertex on screen (X+120, Y+160) and pack as [Short X][Short Y] into a 32-bit word
                vertexCursor[2] = ((offsetX + 120) << 16) | (offsetY + 160);

                // Move to the next vertex slot in the POLY_F4 structure
                vertexCursor++;
                vertexCount++;
            }

            DrawPrim(polyF4);
            quadCount++;
            currentSignPtr += 2;
        }

        patternCount++;
    }

    // Draw final static center quad
    polyF4[2] = 0x00500070;
    polyF4[3] = 0x00480078;
    polyF4[4] = 0x00A800C8;
    polyF4[5] = 0x00A000D0;

    DrawPrim(polyF4);
}