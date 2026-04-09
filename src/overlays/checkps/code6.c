#include "checkps.h"

/**
 * decomp.me link (95.20%) https://decomp.me/scratch/taoth
 * Matches 100% with GNU AS
 */
void func_80051A24(void)
{
    s32 sp10[6];
    u8 stack_buffer[8];
    s32 outer_cnt;
    s32 middle_cnt;
    s32 inner_cnt;
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
    sp10_base = sp10;
    stack_base = stack_buffer;
    const16 = 0x10;
    sp10[0] = 0x05000000;
    sp10[1] = new_var;
    do
    {
        middle_cnt = 0;
        reg_local_buf = stack_base;
        do
        {
            reg_local_buf++;
            reg_local_buf--;
            __builtin_memcpy(stack_buffer, D_8004FD00, 8);
            inner_cnt = 0;
            reg_read_ptr = (s8*)reg_local_buf;
            out_ptr = sp10_base;
            do
            {
                s32 a1 = (inner_cnt + 2) >> 2;
                s32 v1 = outer_cnt + (inner_cnt & 1);
                u8* ptr1 = D_8005CFF0 + ((const16 - v1) << 1);
                u8* ptr2 = D_8005CFF0 + (v1 << 1);
                s32 a3 = ((s8)reg_read_ptr[1]) * ptr1[a1];
                s32 b3 = ((s8)reg_read_ptr[0]) * ptr2[a1];
                out_ptr[2] = ((a3 + 0x78) << 16) | (b3 + 0xA0);
                out_ptr++;
                inner_cnt++;
            } while (inner_cnt < 4);
            DrawPrim(sp10);
            middle_cnt++;
            reg_local_buf += 2;
        } while (middle_cnt < 4);
        outer_cnt++;
    } while (outer_cnt < 0x10);
    sp10[2] = 0x00500070;
    sp10[3] = 0x00480078;
    sp10[4] = 0x00A800C8;
    sp10[5] = 0x00A000D0;
    DrawPrim(sp10);
}