#include "common.h"
#include "cd_resources.h"

/* Define the structure layout for the memory at 0x801ED600 */
typedef struct
{
    u8 pad0[0x91];
    u8 unk91;
    u8 unk92;
    u8 pad93[0xAC]; /* 0x13F - 0x093 = 0xAC bytes of padding */
    u8 unk13F;
    u8 unk140;
} UnkStruct_801ED600;

s32 cdrom_stream(s32 resourceIndex, u32 destination);
void cdrom_wait_queue_empty(void);
extern void func_80084240(void); /* Fixed prototype */
void func_80140004(s32 cdLoadAddr, s32 imageResourceIndex, s32 musicResourceIndex, s32 audioClipIndex);
void func_800A74E8();
void func_800AA02C();

typedef struct
{
    char pad_00[0x1EC];
    u16 unk1EC[35]; /* 0x1EC - Array size bound by 0x232 offset */
    u8 unk232;      /* 0x232 */
    char pad_233;   /* 0x233 */
    u16 unk234;     /* 0x234 */
    u16 unk236;     /* 0x236 */
    u16 unk238;     /* 0x238 */
    u8 unk23A;      /* 0x23A */
} MainStruct2;

typedef struct
{
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
} Struct_D_800F2268;

typedef struct
{
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
} Struct_D_800F2270;

typedef struct
{
    u8 pad[0x1EC];
    u16 unk1EC;
} Struct_80068494;

typedef struct
{
    u8 unk0;
    u8 unk1;
    u8 pad[4];
} Struct_Unk4;
typedef struct
{
    u8 pad[4];
    Struct_Unk4* unk4;
    u16* unk8;
} Struct_arg0;
typedef struct
{
    u8 pad[0x1EC];
    u16 unk1EC;
} Struct_1EC;

typedef struct
{
    u8 unk0;
    u8 unk1;
    s16 unk2;
    s16 unk4;
} Struct_Unk3;
typedef struct
{
    u8 pad[4];
    Struct_Unk3* unk4;
    u16* unk8;
} Struct_Arg0;

typedef struct Struct_C
{
    u8 pad0[0xC];
    u16 unkC;
    u8 padE[0x18 - 0x0E];
    u16 unk18;
} Struct_C;

typedef struct
{
    char pad_00[0x32];
    u8 unk32;
    char pad_33[0x48 - 0x33];
} SubStruct;
typedef struct
{
    SubStruct* sub_array;
    char pad_04[0x21];
    u8 unk25;
    char pad_26[0x5];
    u8 unk2B[9];
    char pad_34[7];
    u8 inner1[9][16];
    char pad_CB[1];
    u16 inner2[9][16];
    u16 unk1EC[9];
    char pad_1FE[0x36];
    u16 unk234;
    u16 unk236;
} MainStruct;

typedef struct
{
    u8 pad0[0xC];
    Struct_C* unkC;
    u8 pad10[0x24 - 0x10];
    u8 unk24;
    u8 pad25[0x2A - 0x25];
    u8 unk2A;
    u8 pad2B[0x1EC - 0x2B];
    u16 unk1EC[9];
    u8 pad1FE[0x222 - 0x1FE];
    u16 unk222;
    u32 unk224;
    u8 unk228;
    u8 unk229[9];
    u8 unk232;
    u8 unk233;
    u8 pad234[0x23A - 0x234];
    u8 unk23A;
    u8 unk23B;
    u8 pad23C[0x244 - 0x23C];
} func_80068970_Arg0;

typedef struct
{
    u8 pad0[0x25];
    u8 unk25;
    u8 pad26[0x2A - 0x26];
    s16 unk2A;
    u8 pad2C[0x54 - 0x2C];
} Struct_D800FDF58;

typedef struct
{
    u8 pad0[0xC];
    u32 unkC;
    u8 pad10[0x178 - 0x10];
    union
    {
        u32 unk178;
        struct
        {
            u8 pad[2];
            u8 unk17A;
            u8 pad2;
        } b;
    } u;
    u8 pad17C[0x23C - 0x17C];
} Struct_D80105AE0;

typedef struct
{
    u8 unk0;
    u8 unk1;
    s16 unk2;
    s16 unk4;
} TableEntry;

typedef struct
{
    u8 pad[4];
    TableEntry* unk4;
    u16* unk8;
} DataStruct;

extern s32 D_801227C8;
extern s32 D_800F22B8;
extern s32 D_800F22BC;
extern s32 D_800F22C0;
extern s32 D_800F22C4;
extern s32 D_800F2284;
extern s32 D_800F22B4;
extern s32 D_8012291C;
extern s32 D_8003EC98;
extern s32 D_80105788;
extern s32 D_8011588C;
extern Struct_D_800F2268 D_800F2268;
extern Struct_D_800F2270 D_800F2270;
extern s32 D_800F2278;
extern s32 D_800F227C;
extern s32 D_800F2280;
extern func_80068970_Arg0 D_800F22C8[80];
extern Struct_D800FDF58 D_800FDF58[];
extern Struct_D80105AE0 D_80105AE0[];

/**
 * decomp.me (100%) https://decomp.me/scratch/9Ady0
 */
void field_func_8006828C(void)
{
    UnkStruct_801ED600* ptr = (UnkStruct_801ED600*)0x801ED600;

    if (D_800F22C0 == 0)
    {
        return;
    }

    if (--D_800F22C0 == 0)
    {
        ptr->unk140 = 0;
        ptr->unk92 = 0;
        ptr->unk13F = 0;
        ptr->unk91 = 0;
        cdrom_stream(CD_RES_GOVER_BIN, 0x80140000);
        cdrom_wait_queue_empty();
        func_80140004(0x80160000, D_800F22B8, D_800F22BC, D_800F22C4);
        func_80084240();
    }
}

/**
 * decomp.me (100%) https://decomp.me/scratch/Kws0l
 */
void func_80068310(s32 arg0)
{

    if (D_800F22B4 != 0)
    {

        if (D_800F2284 != 0)
        {
            D_800F2284--;
            if (D_800F2284 == 0)
            {
                D_800F2268.unk0 = 0xC0;
                D_800F2268.unk2 = 0xC0;
                D_800F2268.unk4 = 0xC0;
                D_800F2268.unk6 = 5;
            }
        }
        else
        {
            func_800A6F1C();
            if (D_800F22B4 != 0)
            {
                func_8006441C();
                if (D_800F22B4 != 0)
                {
                    func_800A8880(arg0);
                }
                func_80063194();
            }
        }
    }
}

/**
 * decomp.me (100%) https://decomp.me/scratch/doJjR
 */
void func_800683C8(void)
{
    func_800AA02C();
    func_800A74E8();
}

/**
 * decomp.me (100%) https://decomp.me/scratch/b8yys
 */
void func_800683F0(void)
{
    D_800F2268.unk0 = 0;
    D_800F2270.unk0 = 0;
    D_800F2268.unk2 = 0;
    D_800F2270.unk2 = 0;
    D_800F2268.unk4 = 0;
    D_800F2270.unk4 = 0;
    D_800F2268.unk6 = 8;
    func_800643E0();
    D_8012291C = 0;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/KDXt0
 */
void func_80068440(void)
{
    s32 temp_v0;

    if (D_8003EC98 != 0)
    {
        temp_v0 = D_8003EC98 - 1;
        D_8003EC98 = temp_v0;
        if (temp_v0 == 0)
        {
            FUN_8001160c();
            func_800A380C();
            func_800A3904(0, 1, D_8011588C);
        }
    }
}

/**
 * decomp.me (100%) https://decomp.me/scratch/Xn30r
 */
s32 func_80068494(s32 arg0, s32 arg1)
{
    return ((Struct_80068494*)((u8*)arg0 + D_80105788 * 2))->unk1EC % arg1;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/X9uyL
 */
void func_800684E4(Struct_arg0* arg0, s32 arg1, s32 arg2, s16* arg3)
{
    s32 var_t9 = 0;
    s32 var_t1 = 4;
    s16* var_t7;
    s32 var_t6;
    s32 new_var;
    Struct_Unk4* temp_v0;
    s32 var_t0 = (&arg0->unk4[arg1 & 0xF])->unk0 & 0x7F;
    u16* var_t5 = &arg0->unk8[(&arg0->unk4[arg1 & 0xF])->unk1];
    new_var = arg1;
    if (var_t0 != 0)
    {
        do
        {
            u16 temp = *var_t5;
            s32 temp_v1 = var_t9 + (temp & 0x3FF);
            if (((Struct_1EC*)(((u8*)arg0) + (D_80105788 * 2)))->unk1EC < temp_v1)
            {
                break;
            }
            var_t9 = temp_v1;
            var_t5++;
            var_t1 += 4;
            var_t0--;
            if (var_t1 == 16)
            {
                var_t1 = 4;
            }
        } while (var_t0 != 0);
    }
    if (var_t0 != 0)
    {
        s32 temp_a1 = new_var & 0xFFFF;
        u16* var_t8 = (u16*)(arg2 + (((temp_a1 >> var_t1) & 0xF) << 5));
        u16* var_t4;
        s32 temp_v1_2;
        if ((var_t1 + 4) != 16)
        {
            var_t4 = (u16*)(arg2 + (((temp_a1 >> (var_t1 + 4)) & 0xF) << 5));
        }
        else
        {
            var_t4 = (u16*)(arg2 + ((temp_a1 << 1) & 0x1E0));
        }
        var_t7 = arg3;
        var_t6 = 0;
        do
        {
            u16 a = *var_t8;
            u16 b = *var_t4;
            s32 low_a = a & 0x1F;
            s32 low_b = b & 0x1F;
            s32 diff0 = low_b - low_a;
            s32 temp_t0 = ((Struct_1EC*)(((u8*)arg0) + (D_80105788 * 2)))->unk1EC - var_t9;
            s32 temp_a1_2 = (*var_t5) & 0x3FF;
            s32 temp_t3 = (diff0 * temp_t0) / temp_a1_2;
            s32 mid_a = (a >> 5) & 0x1F;
            s32 mid_b = (b >> 5) & 0x1F;
            s32 diff1 = mid_b - mid_a;
            s32 temp_t2 = (diff1 * temp_t0) / temp_a1_2;
            s32 high_a = (a >> 10) & 0x1F;
            s32 high_b = (b >> 10) & 0x1F;
            s32 diff2 = high_b - high_a;
            s32 temp_a1_3 = (diff2 * temp_t0) / temp_a1_2;
            *var_t7 = (((a & 0x8000) | (low_a + temp_t3)) | ((mid_a + temp_t2) << 5)) | ((high_a + temp_a1_3) << 10);
            var_t7++;
            var_t6++;
            var_t8++;
            var_t4++;
        } while (var_t6 < 16);
    }
}

/**
 * decomp.me (100%) https://decomp.me/scratch/t5bIj
 */
s32 func_80068734(Struct_Arg0* arg0, s32 arg1, u16 arg2)
{
    s32 var_s4;
    Struct_Unk3* temp_s3;
    s32 var_s2;
    s32 temp_s1;
    u16* var_s0;
    s32 var_a1;
    u16 var_a2;
    u16 temp_a0;
    s32 temp_v1_2;
    u16 temp_v1_3;
    s32 rand_val;
    var_s4 = 0;
    temp_s3 = arg0->unk4 + arg1;
    var_s2 = var_s4;
    var_a1 = temp_s3->unk0 & 0x7F;
    var_s0 = arg0->unk8 + temp_s3->unk1;
    if (var_a1 != 0)
    {
        var_a2 = *((u16*)((((u8*)arg0) + (D_80105788 * 2)) + 0x1EC));
        do
        {
            if (!arg0)
            {
            }
            temp_a0 = *var_s0;
            temp_v1_2 = var_s4 + (temp_a0 & 0x3FF);
            if (((s32)var_a2) < temp_v1_2)
            {
                break;
            }
            var_s4 = temp_v1_2;
            var_s2 = temp_a0 >> 10;
            var_a1--;
            var_s0++;
        } while (var_a1 != 0);
    }
    temp_s1 = temp_s3->unk2 - temp_s3->unk4;
    var_s2 = (temp_s1 * var_s2) >> 5;
    if (var_a1 == 0)
    {
        return temp_s3->unk4;
    }
    if (((*((u16*)temp_s3)) & 0x80) && (D_801227C8 == 0))
    {
        rand_val = rand();
        return temp_s3->unk4 +
               (((var_s2 + (((((temp_s1 * ((*var_s0) >> 10)) >> 5) - var_s2) * ((*((u16*)((((u8*)arg0) + (D_80105788 * 2)) + 0x1EC))) - var_s4)) /
                            ((*var_s0) & 0x3FF))) *
                 rand_val) >>
                15);
    }
    else
    {
        temp_v1_3 = *var_s0;
        return (temp_s3->unk4 + var_s2) +
               (((((temp_s1 * (temp_v1_3 >> 10)) >> 5) - var_s2) * ((*((u16*)((((u8*)arg0) + (D_80105788 * 2)) + 0x1EC))) - var_s4)) / (temp_v1_3 & 0x3FF));
    }
}

/**
 * decomp.me (100%) https://decomp.me/scratch/MCyYP
 */
s32 func_80068970(func_80068970_Arg0* arg0)
{
    s32 found;
    u8 temp_a0;
    s32 i;
    func_80068970_Arg0* var;
    s16 temp_v1;
    if (arg0->unk222 == arg0->unk1EC[0])
    {
        if (arg0->unkC->unk18 & 0x20)
        {
            for (i = 8; i >= 0; i--)
            {
                arg0->unk1EC[i] = 0;
            }

            return 0;
        }
        arg0->unk23A = 0;
    }
    if (arg0->unk23A == 0)
    {
        arg0->unk23B = 0;
        if (arg0->unkC->unk18 & 2)
        {
            if (D_80105AE0[arg0->unk228].u.b.unk17A == arg0->unk233)
            {
                temp_v1 = D_800FDF58[arg0->unk228].unk2A;
                if (((temp_v1 != 0x90) && (temp_v1 != 0x94)) || (D_80105AE0[arg0->unk228].unkC & 0x200))
                {
                    D_800FDF58[arg0->unk228].unk25 = 0;
                }
                D_80105AE0[arg0->unk228].u.unk178 &= ~1;
            }
        }
        if (arg0->unkC->unk18 & 4)
        {
            for (found = 0; found < ((s32)arg0->unk232); found++)
            {
                if (arg0->unk229[found] != 0xFF)
                {
                    temp_a0 = D_80105AE0[arg0->unk229[found]].u.unk178;
                    if ((temp_a0 & 1) && (D_80105AE0[arg0->unk229[found]].u.b.unk17A == arg0->unk233))
                    {
                        D_800FDF58[arg0->unk229[found]].unk25 = 0;
                        D_80105AE0[arg0->unk229[found]].u.unk178 &= ~1;
                    }
                }
            }
        }
        if (arg0->unkC->unkC & 0x1000)
        {
            D_800F2280 = 0;
            D_800F227C = 0;
            D_800F2278 = 0;
        }
        if ((arg0->unkC->unkC >> 8) & 4)
        {
            s32 j;
            var = D_800F22C8;
            for (j = 0; j < 80; j++, var++)
            {
                found = 0;
                if (((arg0 != var) && (var->unk24 != 0)) && ((var->unkC->unkC >> 8) & 4))
                {
                    found = 1;
                    break;
                }
            }

            if (found == 0)
            {
                func_8006A240(0x100, 0x100, 0x100);
            }
        }
        if ((arg0->unk224 & 0xFFFF0001) != 0xC0000)
        {
            func_8006D21C(arg0);
        }
        if (!(arg0->unkC->unkC & 0x800))
        {
            arg0->unk222 = 0;
            arg0->unk24 = 0;
            if (arg0->unk224 & 1)
            {
                func_80084424(arg0->unk228);
            }
            return 1;
        }
        temp_a0 = arg0->unk2A;
        if (temp_a0 != 0)
        {
            arg0->unk222 = 0;
            arg0->unk24 = 0;
            func_80084424(arg0->unk228);
            for (i = 0; i < 80; i++)
            {
                if (((D_800F22C8[i].unk24 != 0) && (D_800F22C8[i].unk228 == arg0->unk228)) && ((temp_a0 = D_800F22C8[i].unk224) & 1))
                {
                    D_800F22C8[i].unk23A = 0;
                    func_80068970(&D_800F22C8[i]);
                }
            }

            arg0->unk2A = 0;
        }
        else
        {
            arg0->unk222 = 0;
            arg0->unk24 = 0;
        }
        return 1;
    }
    return 0;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/7d7kv
 */
unsigned int func_80068DA8(DataStruct* arg0, s32 arg1, s32 arg2)
{
    TableEntry* entry;
    int new_var2;
    u16* data_ptr;
    s32 count;
    s32 accumulated_val;
    s32 v1_reg;
    s32 diff;
    u16 val;
    int new_var3;
    s32 fraction;
    int new_var;
    s32 result;
    s32 local_arg2;

    accumulated_val = 0;
    v1_reg = 0;
    local_arg2 = arg2;
    entry = &arg0->unk4[arg1];

    count = entry->unk0 & 0x7F;
    data_ptr = &arg0->unk8[entry->unk1];

    if (count != 0)
    {
        do
        {
            u16 current = *data_ptr;
            s32 sum = accumulated_val + (current & 0x3FF);

            if (local_arg2 < sum)
            {
                break;
            }

            accumulated_val = sum;
            v1_reg = current >> 10;
            data_ptr++;
        } while ((--count) != 0);
    }

    diff = entry->unk2 - entry->unk4;
    new_var2 = 0x3FF;
    v1_reg = (diff * v1_reg) >> 5;
    new_var = new_var2;

    if (count == 0)
    {
        return entry->unk4;
    }

    if (((*((u16*)entry)) & 0x80) && (D_801227C8 == 0))
    {
        s32 rand_val = rand();
        val = *data_ptr;
        fraction = ((((diff * (val >> 10)) >> 5) - v1_reg) * (local_arg2 - accumulated_val)) / (val & new_var);
        new_var3 = entry->unk4 + (((v1_reg + fraction) * rand_val) >> 15);
        result = ((v1_reg + fraction) * rand_val) >> 15;
        return new_var3;
    }
    else
    {
        val = *data_ptr;
        fraction = ((((diff * (val >> 10)) >> 5) - v1_reg) * (local_arg2 - accumulated_val)) / (val & new_var2);
        result = fraction;
        return (entry->unk4 + v1_reg) + result;
    }
}

/**
 * decomp.me (100%) https://decomp.me/scratch/eRVUu
 */
void func_80068F94(MainStruct* arg0)
{
    s32 i;
    s32 j;

    /* Changing this to use 'j' binds 'j' to register a1, */
    /* which forces 'i' to take register a3 later on.     */
    for (j = 8; j >= 0; j--)
    {
        arg0->unk1EC[j] = 0;
    }

    arg0->unk236 = 0;
    arg0->unk234 = 0;

    for (i = 0; i < arg0->unk25; i++)
    {
        arg0->sub_array[i].unk32 = i;
        for (j = 0; j < 9; j++)
        {
            arg0->unk2B[i] = (arg0->inner2[j][i] = (arg0->inner1[j][i] = 0));
        }
    }
}

/**
 * decomp.me (100%) https://decomp.me/scratch/PjqLA
 */
void func_80069028(MainStruct2* arg0)
{
    s32 var_a0;

    if (func_80068970() == 0)
    {
        /* Removed the redundant 'if (arg0->unk232 != 0)' wrapper */
        for (var_a0 = 0; var_a0 < arg0->unk232; var_a0++)
        {
            if (((s32)arg0->unk23A >> var_a0) & 1)
            {
                arg0->unk1EC[var_a0]++;
            }
        }

        /* Incremented before checking conditions to match delay slot scheduling */
        arg0->unk236++;

        if (arg0->unk238 != 0 && arg0->unk232 != 0)
        {
            if ((arg0->unk236 % arg0->unk238) == 0)
            {
                s32 temp_a0 = arg0->unk236 / arg0->unk238;
                if (temp_a0 < arg0->unk232)
                {
                    arg0->unk23A |= (1 << temp_a0);
                }
            }
        }
    }
}

/**
 * decomp.me (100%) https://decomp.me/scratch/3KrRM
 */
void func_8006911C(void)
{
    s32 var_a0;
    s32 var_a0_2;
    func_80068970_Arg0* new_var;
    volatile u8* var_v1_2;
    var_a0 = 0;
    do
    {
        D_80105AE0[var_a0].u.unk178 &= ~1;
        var_a0++;
    } while (var_a0 < 0xD);
    var_a0_2 = 0;
    new_var = D_800F22C8;
    var_v1_2 = ((u8*)new_var) + 0x238;
    do
    {
        var_v1_2[-5] = var_a0_2;
        var_a0_2++;
        var_v1_2[-0x213] = 0;
        var_v1_2[-0x214] = 0;
        *((volatile u16*)(var_v1_2 - 0x16)) = 0;
        *((volatile u16*)var_v1_2) = 0;
        var_v1_2 += 0x244;
    } while (var_a0_2 < 0x50);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/0JqYo
 */
void func_80069184(void)
{
    s32 var_a0 = 0;
    func_80068970_Arg0* var_v1 = D_800F22C8;

    do
    {
        var_a0 += 1;
        var_v1->unk24 = 0;
        var_v1->unk232 = 0;
        var_v1->unk23A = 0;
        var_v1->unk23B = 0;
        var_v1->pad25[2] = 0;
        var_v1->pad25[3] = 0;
        var_v1++;
    } while (var_a0 < 0x50);
}