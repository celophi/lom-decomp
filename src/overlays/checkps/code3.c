#include "checkps.h"

/**
 * decomp.me link (100%) https://decomp.me/scratch/cjji6
 * PsyQ 4.3 / gcc 2.8.0
 */

void func_80051830(u32 arg0, void* arg1, s32 arg2)
{
    u32 end;
    int unk0;
    u32 new_var;
    s32 local_arg2 = arg2;

    //This is actually strlen((char*)arg0);
    end = arg0 + strlen();
    unk0 = ((arg1struct*)arg1)->unk0;
    if (arg0 < end)
    {
        u8 nl = 0x0A;
        (void)nl;

        new_var = end;
        do
        {
            if ((*((u8*)arg0)) == nl)
            {
                ((arg1struct*)arg1)->unk0 = unk0;
                ((arg1struct*)arg1)->unk2 += 0x12;
            }
            else
            {
                int temp_a0 = *((u8*)arg0);
                arg0++;
                func_80051908(arg1, (u8*)func_8001687C((*((u8*)arg0)) | (temp_a0 << 8)), local_arg2);
                ((arg1struct*)arg1)->unk0 = (end = (s16)(((0x11 * 0, (u16)((arg1struct*)arg1)->unk0)) + 0x11));
            }
            arg0++;
        } while (arg0 < new_var);
    }
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/PIDIi
 * matches under Psy-Q 4.3 / gcc 2.8.0
 */
void func_80051908(void* arg0, u8* arg1, s32 arg2)
{
    /* No intermediate pointer — reference arg0 directly so it gets s1 first */
    u8* var_s2 = arg1;     /* s2 */
    s16 local_arg2 = arg2; /* s4 */

    /* Force exact stack offsets: sp10=0x10(sp), sp14=0x14(sp),
       sp18=0x18(sp), sp1C=0x1C(sp), sp20=0x20(sp) */
    struct
    {
        s32 sp10;
        s32 sp14;
        s32 sp18;
        s32 sp1C;
        s16 sp20[16];
    } locals;

    s32 temp_s5; /* sign-extended, kept in s5 */
    s32 temp_s6; /* sign-extended, kept in s6 */
    s32 var_s3;  /* kept in s3, zeroed after loading arg->unk4 */
    s32 var_s0;
    s16* next;
    s16* curr;
    int bit;
    s16 val;

    /* Exact instruction order from target */
    temp_s5 = ((Arg0Struct*)arg0)->unk0; /* first use of arg0 → allocates s1 */
    temp_s6 = ((Arg0Struct*)arg0)->unk2;
    locals.sp10 = 0x0B000000; /* lui/sw -> 0x10(sp) */
    locals.sp14 = 0xA0000000; /* lui/sw -> 0x14(sp) */
    {
        s32 tmp = ((Arg0Struct*)arg0)->unk4; /* lw v0,4(s1) */
        var_s3 = 0;                          /* move s3,zero */
        locals.sp1C = tmp;                   /* sw v0,0x1c(sp) */
    }

    /* The two nested loops – identical to target */
    do
    {
        next = locals.sp20; /* addiu a2,sp,0x20 */
        var_s0 = 0;
        do
        {
            for (bit = 7; bit >= 0; bit--)
            {
                curr = next;     /* move a1,a2 */
                next = curr + 1; /* addiu a2,a1,2 */
                val = 0;
                if (((s32)(*var_s2) >> bit) & 1)
                    val = local_arg2;
                *curr = val; /* sh a0,0(a1) */
            }
            var_s0++;
            var_s2++;
        } while (var_s0 < 2);

        var_s0 = 0;
        do
        {
            locals.sp18 = *(s32*)&((Arg0Struct*)arg0)->unk0; /* lw v0,0(s1); sw v0,0x18(sp) */
            DrawPrim(&locals.sp10);                          /* addiu a0,sp,0x10; jal DrawPrim */
            var_s0++;
            ((Arg0Struct*)arg0)->unk0 = (s16)((u16)((Arg0Struct*)arg0)->unk0 + 1);
        } while (var_s0 < 2);

        var_s3++;
        ((Arg0Struct*)arg0)->unk0 = (s16)temp_s5;
        ((Arg0Struct*)arg0)->unk2 = (s16)((u16)((Arg0Struct*)arg0)->unk2 + 1);
    } while (var_s3 < 0xF);

    ((Arg0Struct*)arg0)->unk2 = (s16)temp_s6;
}

/**
 * decomp.me link (95.20%) https://decomp.me/scratch/taoth
 * This function might match gcc 2.8.0 Psy-Q 4.3
 */
void func_80051A24(void)
{
    s32 sp10[6];
    u8 stack_buffer[8];
    s32 outer_cnt;
    int new_var;
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
    new_var = 0x280000FF;
    outer_cnt = 0;
    sp10_base = sp10;
    stack_base = stack_buffer;
    const16 = 0x10;
    sp10[0] = 0x05000000;
    sp10[1] = new_var;
    table = D_8005CFF0;
    do
    {
        middle_cnt = 0;
        local_buf = stack_base;
        do
        {
            __builtin_memcpy(stack_buffer, D_8004FD00, 8);
            inner_cnt = 0;
            read_ptr = (s8*)local_buf;
            out_ptr = sp10_base;
            do
            {
                s32 a1 = (inner_cnt + 2) >> 2;
                s32 v1 = outer_cnt + (inner_cnt & 1);
                s32 a3 = ((s8)read_ptr[1]) * table[((const16 - v1) << 1) + a1];
                new_var = read_ptr[0];
                lo_val = ((s8)new_var) * table[(v1 << 1) + a1];
                temp_a3 = a3;
                out_ptr[2] = ((temp_a3 + 0x78) << 16) | (lo_val + 0xA0);
                out_ptr++;
                inner_cnt++;
            } while (inner_cnt < 4);
            DrawPrim(sp10);
            middle_cnt++;
            local_buf += 2;
        } while (middle_cnt < 4);
        outer_cnt++;
    } while (outer_cnt < 0x10);
    sp10[2] = 0x00500070;
    sp10[3] = 0x00480078;
    sp10[4] = 0x00A800C8;
    sp10[5] = 0x00A000D0;
    DrawPrim(sp10);
}