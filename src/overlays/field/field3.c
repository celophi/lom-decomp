#include "common.h"

struct S1;
struct S2;
struct Block;
struct T;
typedef struct S1
{
    u8 pad_00[4];
    union
    {
        u32 word;
        struct
        {
            u8 pad_04[2];
            u8 field_06;
            u8 field_07;
        } bytes;
    } u;
    struct S1* next;
    u8 field_0C;
    u8 field_0D;
    u8 field_0E;
    u8 field_0F;
    struct S2* ptr_s2;
    struct Block* blocks;
} S1;
typedef struct S2
{
    u8 pad_00[8];
    union
    {
        u32 word;
        struct
        {
            u8 pad_08[2];
            u8 field_0A;
            u8 field_0B;
        } bytes;
    } u;
} S2;
typedef struct Block
{
    u8 b0;
    u8 b1;
    u8 b2;
    u8 b3;
} Block;
typedef struct T
{
    u8 pad_00[8];
    struct T* next;
    s32* data;
    u8 pad_10[8];
    u32 field_18;
    u32 field_1C;
} T;
extern T* func_8005ABD8(void* arg0, int arg1);

/**
 * decomp.me (95.80%) https://decomp.me/scratch/Kkiiv
 */
void field_validate_and_rasterize_quads(void* arg0, s32 arg1)
{
    S1* s1 = (S1*)arg0;
    u32 var_t6 = 0;
    u32 var_s7 = 0;
    u32 var_s8 = 0;
    u32 var_s6 = 0;
    s32 flag_a3;
    s32 flag_a2;
    int new_var;
    S1* s0;
    S2* temp_s2;
    T* var_t4;
    Block* var_t3;
    Block* var_t2;
    s32 var_t1;
    s32 var_a1;
    s32 var_a1_2;
    s32* var_a2;
    s32 var_t0;
    s32 var_a0;
    s32 var_a3_val;
    if (s1 != 0)
    {
        do
        {
            s1->u.bytes.field_07 = arg1;
            if (((arg1 == 0) && ((s1->u.word & 7U) < 2U)) || (arg1 == 3))
            {
                s0 = s1;

                temp_s2 = s1->ptr_s2;
                var_t4 = func_8005ABD8(temp_s2, 0);
                if (var_t4->next != 0)
                {
                    var_t4 = var_t4->next;
                }
                if ((s1->u.word & 7U) == 1)
                {
                    if ((temp_s2->u.word & 0xF00U) == 0x100U)
                    {
                        s1->field_0E = 1;
                        s1->field_0F = 1;
                    }
                    else
                    {
                        s1->field_0E = temp_s2->u.bytes.field_0A;
                        s1->field_0F = temp_s2->u.bytes.field_0B;
                    }
                }
                if (var_t4->field_18 != 0)
                {
                    u32 temp = var_t4->field_18 - 1;
                    flag_a3 = 1;
                    var_s6 = temp >> 4;
                    var_t6 = temp & 0xF;
                }
                else
                {
                    flag_a3 = 0;
                }
                if (var_t4->field_1C != 0)
                {
                    u32 temp = var_t4->field_1C - 1;
                    flag_a2 = 1;
                    var_s8 = temp >> 9;
                    var_s7 = temp & 0xFF;
                }
                else
                {
                    flag_a2 = 0;
                }
                if ((flag_a3 != 0) || (flag_a2 != 0))
                {
                    var_t1 = s1->u.bytes.field_06 - 1;
                    var_t3 = s0->blocks;
                    if (var_t1 != (-1))
                    {
                        do
                        {
                            var_a1 = ((var_t1 = s0->field_0E) * s0->field_0F) - 1;
                            if (var_a1 != (-1))
                            {
                                do
                                {
                                    if (var_t3->b0 & 0x80)
                                    {
                                        if (flag_a3 == 1)
                                        {
                                            u8 b1_val = var_t3->b1;
                                            if ((var_t6 != (b1_val & 0xF)) || (var_s6 != ((b1_val >> 4) & 3)))
                                            {
                                                flag_a3 = 2;
                                            }
                                        }
                                        if (flag_a2 == 1)
                                        {
                                            u8 b3_val = var_t3->b3;
                                            u8 b1_val = var_t3->b1;
                                            if ((var_s7 != b3_val) || (var_s8 != ((b1_val >> 6) & 1)))
                                            {
                                                flag_a2 = 2;
                                            }
                                        }
                                    }
                                    var_t3++;
                                    var_a1--;
                                } while (var_a1 != (-1));
                            }
                            var_t1--;
                        } while (var_t1 != (-1));
                    }
                    if (flag_a3 != 1)
                    {
                        var_t4->field_18 = 0;
                    }
                    if (flag_a2 != 1)
                    {
                        var_t4->field_1C = 0;
                    }
                }

                if (((arg1 == 0) && ((s1->u.word & 7U) == 0)) || (arg1 == 3))
                {
                    var_t1 = s1->u.bytes.field_06 - 1;
                    var_t3 = s0->blocks;
                    if (var_t1 != (-1))
                    {
                        do
                        {
                            var_t2 = var_t3;
                            var_a1_2 = 1;
                            var_a2 = var_t4->data;
                            var_a3_val = *var_a2;
                            if (temp_s2->u.bytes.field_0B != 0)
                            {
                                var_t0 = 0;
                                do
                                {
                                    if (var_t0 < s0->field_0D)
                                    {
                                        new_var = -1;
                                        var_a0 = temp_s2->u.bytes.field_0A - 1;
                                        if (var_a0 != new_var)
                                        {
                                            do
                                            {
                                                var_a1_2 <<= 1;
                                                if (var_a1_2 == 0)
                                                {
                                                    *var_a2 = var_a3_val;
                                                    var_a2++;
                                                    var_a1_2 = 1;
                                                    var_a3_val = *var_a2;
                                                }
                                                var_a0--;
                                            } while (var_a0 != (-1));
                                        }
                                    }
                                    else if (var_t0 < (s0->field_0D + s0->field_0F))
                                    {
                                        if (temp_s2->u.bytes.field_0A != 0)
                                        {
                                            var_a0 = 0;
                                            do
                                            {
                                                if ((var_a0 >= s0->field_0C) &&
                                                    (var_a0 < (s0->field_0C + s0->field_0E)))
                                                {
                                                    if (var_t2->b0 & 0x80)
                                                    {
                                                        var_a3_val |= var_a1_2;
                                                    }
                                                    var_t2++;
                                                }
                                                var_a1_2 <<= 1;
                                                if (var_a1_2 == 0)
                                                {
                                                    *var_a2 = var_a3_val;
                                                    var_a2++;
                                                    var_a1_2 = 1;
                                                    var_a3_val = *var_a2;
                                                }
                                                var_a0++;
                                            } while (var_a0 != temp_s2->u.bytes.field_0A);
                                        }
                                    }
                                    else
                                    {
                                        break;
                                    }
                                    var_t0++;
                                } while (var_t0 != temp_s2->u.bytes.field_0B);
                            }
                            if (var_a1_2 != 1)
                            {
                                *var_a2 = var_a3_val;
                            }
                            var_t1--;
                            var_t3 += s0->field_0E * s0->field_0F;
                        } while (var_t1 != (-1));
                    }
                }
            }
            s1 = s1->next;
        } while (s1 != 0);
    }
}