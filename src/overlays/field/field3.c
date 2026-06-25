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

extern void *g_field_scene;
void *func_80059224(void *, u8, u8 *);
void func_80057E88(void *, void *, s32);
void *func_8005B31C(void *);
void func_8005AC50(void *, u16, s32 *);
void func_8005AD20(u8, u16, s8 *);
void func_8005477C(s32, void *, s32, s32);
void func_80054904(s32, void *, s32, s32);
void func_80057CA4(void *, void *, s32);

typedef struct Struct80053C7C_1
{
    u8 unk0;
    u8 unk1;
    u8 pad2[2];
    u32 unk4;
    u8 unk8;
    u8 unk9;
    u8 unkA;
    u8 unkB;
    u8 unkC;
    u8 unkD;
    u8 unkE;
    u8 unkF;
    void *unk10;
    void *unk14;
} Struct80053C7C_1;

typedef struct
{
    u16 unk0;
    u16 unk2;
} Struct80053C7C_2;

/**
 * decomp.me (88.26%) https://decomp.me/scratch/0nNOC
 */
void func_80053C7C(Struct80053C7C_1 *arg0, void **arg1, void *arg2)
{
    s32 sp10[3];
    u8 sp20;
    void *sp24;
    s8 sp28;
    void *sp2C;
    Struct80053C7C_1 *sp30;
    u16 sp38;
    s32 sp40;
    u16 sp48;
    s32 temp_s3;
    s32 var_s0;
    s32 *new_var5;
    s32 var_s1;
    s32 var_s3_2;
    s32 var_s3_3;
    s32 var_s6;
    s32 var_s7;
    int new_var6;
    void *new_var;
    s32 var_t1;
    s32 var_t2;
    short new_var7;
    s32 var_t4;
    s32 temp_flags;
    Struct80053C7C_1 *new_var3;
    s32 *temp_s2;
    s32 *var_s2;
    u16 temp_v1;
    u16 var_v0;
    u16 *temp_a0;
    u8 temp_v0;
    u8 temp_v0_5;
    unsigned char temp_v1_2;
    u8 temp_v1_6;
    u8 temp_v1_7;
    u8 temp_v1_8;
    u8 temp_v1_9;
    int var_s3;
    Struct80053C7C_2 *temp_v1_5;
    void **new_var4;
    void *temp_a1;
    void *temp_a1_2;
    Struct80053C7C_1 *temp_s4_5;
    void *temp_s5;
    void *var_fp;
    Struct80053C7C_1 *var_t0;
    Struct80053C7C_2 *temp_v0_2;
    int new_var2;
    void *var_t3;
    void *temp_v0_3;
    void *temp_v0_4;
    void *temp_v0_7;
    var_t0 = arg0;
    var_t3 = (void *) 0;
    sp30 = (void *) 0;
    sp40 = 0;
    sp48 = 0;
    sp38 = 1;
    sp24 = (void *) 0;
    sp2C = g_field_scene;
    if (var_t0 != ((void *) 0))
    {
        do
        {
            temp_s5 = *arg1;
            *arg1 = (void *) (((u8 *) temp_s5) + 0x30);
            ((void **) arg2)[0] = temp_s5;
            arg2 = temp_s5;
            ((void **) temp_s5)[1] = var_t0;
            if (!((*((u32 *) var_t0)) & 0x7F))
            {
                ((s32 *) arg2)[9] = ((s32 *) arg2)[9] & (~0x40);
            }
            else
            {
                ((s32 *) arg2)[9] = (((s32 *) arg2)[9] & (~0x40)) | ((((u8) var_t0->unk4) >> 7) << 6);
            }
            ((u8 *) temp_s5)[0x28] = 0;
            temp_flags = (((s32 *) temp_s5)[9] & (~1)) | ((var_t0->unk4 >> 3) & 1);
            temp_flags &= ~2;
            temp_flags &= ~4;
            temp_flags &= ~8;
            temp_flags &= ~0x10;
            temp_flags &= ~0x20;
            ((s32 *) temp_s5)[9] = temp_flags;
            ((u8 *) temp_s5)[0x27] = 0;
            if (var_t0->unk4 & 0x40)
            {
                ((u8 *) temp_s5)[0x26] = 0;
                ((u8 *) temp_s5)[0x25] = var_t0->unk1;
            }
            else
            {
                temp_v0 = var_t0->unk1;
                ((u8 *) temp_s5)[0x25] = temp_v0;
                ((u8 *) temp_s5)[0x26] = temp_v0;
            }
            if (((u8 *) var_t0)[7] == 3)
            {
                ((u16 *) temp_s5)[0x15] = 1;
            }
            else
            {
                temp_v0_2 = func_80059224(var_t0, ((u8 *) temp_s5)[0x26], &sp20);
                if (var_t0->unk4 & 0x20)
                {
                    ((u16 *) temp_s5)[0x15] = temp_v0_2->unk2;
                }
                else
                {
                    temp_v1 = temp_v0_2->unk2;
                    if (temp_v1 < sp38)
                    {
                        ((u16 *) temp_s5)[0x15] = temp_v1;
                        sp38 = 1;
                    }
                    else
                    {
                        var_v0 = sp38;
                        sp38 = 1 + var_v0;
                        ((u16 *) temp_s5)[0x15] = var_v0;
                    }
                }
            }
            temp_v1_2 = ((u8 *) var_t0)[7];
            new_var4 = &var_t0->unk14;
            switch (temp_v1_2)
            {
                case 0:
                    switch (((u8 *) var_t0)[4] & 7)
                    {
                        case 0:
                            new_var2 = 0xFF;

                        case 1:
                            sp30 = (Struct80053C7C_1 *) var_t0->unk10;
                            var_t3 = func_8005ABD8(var_t0->unk10, &sp24);
                            ((void **) temp_s5)[3] = var_t3;
                            break;

                        case 2:
                            sp30 = (Struct80053C7C_1 *) var_t0->unk10;
                            var_t3 = func_8005ABD8(var_t0->unk10, &sp24);
                            ((void **) temp_s5)[3] = var_t3;
                            if ((((s32 *) temp_s5)[9] & 0x40) && ((sp20 = 0, var_s3 = ((u8 *) var_t0)[5], var_s3 != new_var2)))
                            {
                                do
                                {
                                    var_s3 -= 1;
                                    ((s8 *) var_t3)[0x20] = sp20 == ((u8 *) temp_s5)[0x25];
                                    var_t3 = *((void **) var_t3);
                                    sp20 += 1;
                                }
                                while (var_s3 != 0xFF);
                            }
                            break;

                        case 3:
                            if ((((s32 *) temp_s5)[9] & 0x40) && (((u16 *) temp_s5)[0x15] != 1))
                            {
                                ((s32 *) temp_s5)[9] |= 0x20;
                            }
                            break;

                        case 4:
                            sp30 = (Struct80053C7C_1 *) var_t0->unk10;
                            var_t3 = func_8005ABD8(var_t0->unk10, &sp24);
                            ((void **) temp_s5)[3] = var_t3;
                            ((s32 *) sp2C)[0xE] = 1;
                            break;

                        case 5:
                            sp30 = (Struct80053C7C_1 *) var_t0->unk10;
                            temp_v0_3 = func_8005ABD8(var_t0->unk10, &sp24);
                            ((void **) temp_s5)[3] = temp_v0_3;
                            func_80057E88(var_t0, temp_s5, 0);
                            var_t3 = temp_v0_3;
                            break;
                            if ((var_t0 && var_t0) && var_t0)
                            {
                            }

                        case 6:
                            new_var3 = var_t0;
                            temp_v0_4 = func_8005B31C(new_var3->unk10);
                            sp24 = temp_v0_4;
                            ((void **) temp_s5)[3] = temp_v0_4;
                            func_80057E88(var_t0, temp_s5, 0);
                            break;

                        case 7:

                        default:
                            sp30 = (Struct80053C7C_1 *) new_var3->unk10;
                            var_t3 = func_8005ABD8(var_t0->unk10, &sp24);
                            ((void **) temp_s5)[3] = var_t3;
                            ((void **) temp_s5)[4] = sp24;
                            temp_v1_5 = (Struct80053C7C_2 *) (*new_var4);
                            temp_v0_5 = temp_v1_5->unk0;
                            if (((temp_v0_5 & 7) == 1) && (temp_v1_5->unk2 & 0x8000))
                            {
                                ((u16 *) temp_s5)[0x15] = 1;
                                ((s32 *) temp_s5)[9] |= 8;
                            }
                            break;
                    }
                    break;

                case 1:
                    switch (((u8 *) new_var3)[4] & 7)
                    {
                        sp30 = (Struct80053C7C_1 *) (*new_var4);
                        case 0:
                            var_t3 = func_8005ABD8(*new_var4, &sp24);
                            ((void **) temp_s5)[3] = var_t3;
                            break;

                        case 1:
                            temp_v0_7 = func_8005B31C(*new_var4);
                            sp24 = temp_v0_7;
                            ((void **) temp_s5)[3] = temp_v0_7;
                            break;
                    }
                    break;

                case 2:
                    var_s3 = new_var3->unk4;
                    switch (var_s3 & 7)
                    {
                        case 0:
                            sp30 = (Struct80053C7C_1 *) new_var3->unk10;
                            var_t3 = func_8005ABD8(new_var = var_t0->unk10, &sp24);
                            ((void **) temp_s5)[3] = var_t3;
                            ((void **) temp_s5)[4] = sp24;
                            break;

                        case 1:
                            temp_v0_7 = func_8005B31C(new_var3->unk10);
                            sp24 = temp_v0_7;
                            ((void **) temp_s5)[3] = temp_v0_7;
                            break;
                    }
                    break;

                default:
                    sp30 = (Struct80053C7C_1 *) new_var3->unk10;
                    var_t3 = func_8005ABD8(var_t0->unk10, &sp24);
                    ((void **) temp_s5)[3] = var_t3;
                    break;
            }

            if (((var_t0->unk4 & 0xFF000007) < 2) || (((u8 *) new_var3)[7] == 3))
            {
                sp10[0] = ((u16 *) sp24)[8] << 8;
                sp10[1] = ((u16 *) sp24)[9] << 8;
                sp10[2] = ((u16 *) sp24)[10] << 8;
                temp_a0 = (u16 *) ((void **) (*((void **) (((u8 *) sp24) + 4))))[1];
                func_8005AC50(temp_a0 + 2, temp_a0[0], sp10);
                sp28 = 0;
                func_8005AD20(((u8 *) var_t3)[0x21], *((u16 *) ((void **) (*((void **) (((u8 *) sp24) + 4))))[1]), &sp28);
                ((void **) temp_s5)[8] = *arg1;
                var_fp = *arg1;
                temp_v1_6 = ((u8 *) var_t3)[0x21];
                switch (temp_v1_6)
                {
                    case 0:
                        sp40 = 0xC;
                        break;

                    case 1:
                        break;

                    case 2:

                    case 3:

                    case 4:

                    case 5:
                        sp40 = 0xC;
                        break;

                    case 6:
                        break;
                }

                var_t2 = 1;
                if (((s32 *) var_t3)[7] != 0)
                {
                    sp40 -= 4;
                }
                else
                {
                    var_t2 = 0;
                }
                if ((new_var5 = (s32 *) var_t3)[6] != 0)
                {
                    var_t2 |= 2;
                    sp40 -= 4;
                }
                var_t4 = (s32) ((long) new_var3->unk14);
                if ((var_t0->unk4 & 0xFF000007) == 1)
                {
                    ((void **) temp_s5)[4] = (void *) new_var5[4];
                    temp_s3 = ((u8 *) var_t0)[6] - 1;
                    if (temp_s3 != (-1))
                    {
                        var_s3_2 = temp_s3 - 1;
                        new_var6 = 1;
                        if (var_s3_2 != (-new_var6))
                        {
                            new_var7 = 1;
                            do
                            {
                                var_s3_2 -= 1;
                            }
                            while (var_s3_2 != (-new_var7));
                        }
                    }
                }
                else
                {
                    sp48 = 0;
                    temp_s4_5 = var_t0;
                    var_s3_3 = ((u8 *) var_t0)[6] - 1;
                    if (var_s3_3 != (-1))
                    {
                        do
                        {
                            var_s7 = var_t4;
                            var_s1 = 1;
                            var_s6 = 0;
                            temp_s2 = (s32 *) new_var5[3];
                            var_t1 = *temp_s2;
                            var_s2 = temp_s2 + 1;
                            if (sp30->unkB != 0)
                            {
                                while (1)
                                {
                                    temp_v1_7 = temp_s4_5->unkD;
                                    if (var_s6 < temp_v1_7)
                                    {
                                        var_s0 = 1;
                                        var_s0 = sp30->unkA;
                                        var_s0 = var_s0 - var_s0;
                                        if (var_s0 != (-1))
                                        {
                                            do
                                            {
                                                var_s1 *= 2;
                                                if (var_s1 == 0)
                                                {
                                                    var_t1 = *var_s2;
                                                    var_s2 += 1;
                                                    var_s1 = 1;
                                                }
                                                var_s0 -= 1;
                                            }
                                            while (var_s0 != (-1));
                                        }
                                    }
                                    else if (var_s6 < (temp_v1_7 + temp_s4_5->unkF))
                                    {
                                        var_s0 = 0;
                                        if (sp30->unkA != 0)
                                        {
                                            do
                                            {
                                                temp_v1_8 = temp_s4_5->unkC;
                                                if ((var_s0 >= temp_v1_8) && (var_s0 < (temp_v1_8 + temp_s4_5->unkE)))
                                                {
                                                    if (var_t1 & var_s1)
                                                    {
                                                        temp_v1_9 = ((u8 *) var_t3)[0x21];
                                                        switch (temp_v1_9)
                                                        {
                                                            case 0:
                                                                temp_a1 = var_fp;
                                                                var_fp = (void *) (((u8 *) var_fp) + (var_s3_3 = sp40));
                                                                func_8005477C(var_s7, temp_a1, ((*((u32 *) (((u8 *) sp30) + 8))) >> 4) & 3, var_t2);
                                                                break;

                                                            case 1:
                                                                break;

                                                            case 2:

                                                            case 3:

                                                            case 4:

                                                            case 5:
                                                                temp_a1_2 = var_fp;
                                                                var_fp = (void *) (((u8 *) var_fp) + sp40);
                                                                func_80054904(var_s7, temp_a1_2, ((*((u32 *) (((u8 *) sp30) + 8))) >> 4) & 3, var_t2);
                                                                break;

                                                            case 6:
                                                                break;
                                                        }

                                                        sp48 += 1;
                                                    }
                                                    var_s7 += 4;
                                                }
                                                var_s1 *= 2;
                                                if (var_s1 == 0)
                                                {
                                                    var_t1 = *var_s2;
                                                    var_s2 += 1;
                                                    var_s1 = 1;
                                                }
                                                var_s0 += 1;
                                            }
                                            while (var_s0 != sp30->unkA);
                                        }
                                    }
                                    var_s6 += 1;
                                    if (var_s6 == sp30->unkB)
                                    {
                                        break;
                                    }
                                }
                            }
                            var_s3_3 -= 1;
                            var_t4 += (temp_s4_5->unkE * temp_s4_5->unkF) * 4;
                        }
                        while (var_s3_3 != (-1));
                    }
                }
                ((u16 *) temp_s5)[0x16] = sp48;
                if (((u8 *) var_t0)[7] == 3)
                {
                    if (new_var3->unk4 & 0x20)
                    {
                        func_80057CA4(var_t0, temp_s5, 0);
                    }
                }
                else if (((s32 *) temp_s5)[9] & 0x40)
                {
                    func_80057CA4(var_t0, temp_s5, 0);
                }
                *arg1 = var_fp;
            }
            if ((((var_t0->unk4 & 0xFF000007) - 3) < 2) || ((((u8 *) new_var3)[7] == 1) && ((var_t0->unk4 & 7) >= 2)))
            {
                if ((var_t0->unk4 & 0xFF000007) == 0x01000002)
                {
                    if (var_t0->unkC == 0)
                    {
                        *arg1 = (void *) (((u8 *) (*arg1)) + 0x50);
                    }
                    else
                    {
                        *arg1 = (void *) (((u8 *) (*arg1)) + 0x410);
                    }
                }
                else if ((var_t0->unk4 & 0xFF000007) == 0x01000005)
                {
                    if (new_var3->unkC == 0)
                    {
                        *arg1 = (void *) ((((u8 *) (*arg1)) + (((s32) ((long) new_var3->unk10)) << 6)) + 0x10);
                    }
                    else
                    {
                        *arg1 = (void *) ((((u8 *) (*arg1)) + (((s32) ((long) new_var3->unk10)) << 0xA)) + 0x10);
                    }
                }
                else
                {
                    *arg1 = (void *) (((u8 *) (*arg1)) + 0x10);
                }
            }
            var_t0 = *((Struct80053C7C_1 **) (&var_t0->unk8));
        }
        while (var_t0 != ((void *) 0));
    }
    ((void **) arg2)[0] = (void *) 0;
}