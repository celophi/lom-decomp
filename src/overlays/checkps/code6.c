#include "checkps.h"

/**
 * decomp.me link (95.20%) https://decomp.me/scratch/taoth
 * Matches 100% with GNU AS
 */
void DrawSymmetricTestPattern(void)
{
    s32 polyF4[6];
    u8 stack_buffer[8];
    s32 outer_cnt;
    s32 middle_cnt;
    s32 vertexCount;
    s32* sp10_base;
    u8* stack_base;
    u8* local_buf;
    s32* out_ptr;
    s8* read_ptr;
    u8* table;
    s32 const16;
    s32 temp_a3;
    s32 lo_val;
    s32 new_var;
    u8* reg_local_buf;
    s8* reg_read_ptr;
    new_var = 0x280000FF;
    outer_cnt = 0;
    sp10_base = polyF4;
    stack_base = stack_buffer;
    const16 = 0x10;
    polyF4[0] = 0x05000000;
    polyF4[1] = new_var;
    
    while (outer_cnt < 16)
    {
        middle_cnt = 0;
        reg_local_buf = stack_base;
        
        while (middle_cnt < 4)
        {
            reg_local_buf++;
            reg_local_buf--;
            __builtin_memcpy(stack_buffer, g_testPatternVertexTable, 8);
            vertexCount = 0;
            reg_read_ptr = (s8*)reg_local_buf;
            out_ptr = sp10_base;

            while (vertexCount < 4)
            {
                // Calculate table index; results in 0,0,1,1 for vertices 0-3
                s32 tableIndex = (vertexCount + 2) >> 2;

                // Alternate between current outer loop index and the next one (outer_cnt, outer_cnt + 1)
                s32 tableOffset = outer_cnt + (vertexCount & 1);

                // Locate base coordinate offsets from the data table (g_testPatternSizeTable)
                // ptr_mirror picks from the end of the table, ptr_base picks from the start
                u8* ptr1 = g_testPatternSizeTable + ((const16 - tableOffset) << 1);
                u8* ptr2 = g_testPatternSizeTable + (tableOffset << 1);

                // Scale the base offsets by the sign pairs (1 or -1) from g_testPatternVertexTable
                // reg_read_ptr[1] is the Y-scale, reg_read_ptr[0] is the X-scale
                s32 offsetX = ((s8)reg_read_ptr[1]) * ptr1[tableIndex];
                s32 offsetY = ((s8)reg_read_ptr[0]) * ptr2[tableIndex];

                // Center the vertex on screen (X+120, Y+160) and pack as [Short X][Short Y] into a 32-bit word
                out_ptr[2] = ((offsetX + 0x78) << 16) | (offsetY + 0xA0);

                // Move to the next vertex slot in the POLY_F4 structure
                out_ptr++;
                vertexCount++;
            }

            DrawPrim(polyF4);
            middle_cnt++;
            reg_local_buf += 2;
        }

        outer_cnt++;
    }

    polyF4[2] = 0x00500070;
    polyF4[3] = 0x00480078;
    polyF4[4] = 0x00A800C8;
    polyF4[5] = 0x00A000D0;
    DrawPrim(polyF4);
}