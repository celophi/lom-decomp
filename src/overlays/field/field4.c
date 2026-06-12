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

typedef struct Struct_C
{
    u8 unk0[2];
    u8 pad2[0xC - 2];
    u16 unkC;
    u16 unkE;
    u16 unk10;
    u8 pad10[0x14 - 0x12];
    u8 unk14;
    u8 unk15;
    u8 pad16[0x18 - 0x16];
    u16 unk18;
} Struct_C;
typedef struct Struct_D
{
    u8 pad0[0xD];
    u8 unkD;
    u8 padE[0x31 - 0xE];
    u8 unk31;
    u8 unk32;
    u8 pad33[1];
    u32 unk34;
    u8 pad38[0x48 - 0x38];
} Struct_D;

typedef struct
{
    u8 pad0[0x91];
    u8 unk91;
    u8 unk92;
    u8 pad93[0x13F - 0x93];
    u8 unk13F;
    u8 unk140;
} Struct_801ED600;

typedef struct
{
    u8 unk0;
    u8 pad1;
    u8 unk2;
    u8 unk3[16];
    u8 unk13[9][16];
    u8 padA3;
    u16 unkA4[9][16];
    u16 unk1C4[9];
    u8 pad1D6[0x1FA - 0x1D6];
    u16 unk1FA;
    union
    {
        u32 unk1FC;
        struct
        {
            u16 lo;
            u16 unk1FE;
        } h;
    } u1FC;
    u8 unk200;
    u8 unk201[9];
    u8 unk20A;
    u8 unk20B;
    u16 unk20C;
    u16 unk20E;
    u16 unk210;
    u8 unk212;
    u8 unk213;
    u8 pad214[0x21C - 0x214];
} Struct_Unk28;

typedef struct
{
    Struct_D* unk0;
    u8 pad4[0xC - 4];
    Struct_C* unkC;
    u8 pad10[0x24 - 0x10];
    u8 unk24;
    u8 unk25;
    u8 unk26;
    u8 unk27;
    u8 unk28;
    u8 pad29;
    u8 unk2A;
    u8 unk2B[16];
    u8 unk3B[9][16];
    u8 padCB;
    u16 unkCC[9][16];
    u16 unk1EC[9];
    u8 pad1FE[0x222 - 0x1FE];
    u16 unk222;
    union
    {
        u32 unk224;
        struct
        {
            u16 lo;
            u16 unk226;
        } h;
    } u224;
    u8 unk228;
    u8 unk229[9];
    u8 unk232;
    u8 unk233;
    u16 unk234;
    u16 unk236;
    u8 pad238[2];
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

typedef struct
{
    Struct_D* unk0;
    u8 pad4[0xC - 4];
    Struct_C* unkC;
    u8 pad10[0x24 - 0x10];
    u8 unk24;
    u8 unk25;
    u8 unk26;
    u8 unk27;
    Struct_Unk28 unk28;
} func_80068970_Arg2;

typedef struct
{
    u8 pad000[0x232];
    u8 unk232;
    u8 pad233[0x5];
    u16 unk238;
    u8 unk23A;
    u8 unk23B;
} ContextStruct;

typedef struct
{
  u8 pad[0x40];
  u32 unk40;
  u8 pad44[0x40B8 - 0x44];
  void *unk40B8;
} Struct_Arg4;

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
extern u8 D_800FF59C;

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
        if ((*(u32*)&arg0->u224.unk224 & 0xFFFF0001) != 0xC0000)
        {
            func_8006D21C(arg0);
        }
        if (!(arg0->unkC->unkC & 0x800))
        {
            arg0->unk222 = 0;
            arg0->unk24 = 0;
            if (*(u32*)(&arg0->u224.unk224) & 1)
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
                if (((D_800F22C8[i].unk24 != 0) && (D_800F22C8[i].unk228 == arg0->unk228)) && ((temp_a0 = D_800F22C8[i].u224.unk224) & 1))
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
        var_v1->unk27 = 0;
        var_v1->unk28 = 0;
        var_v1++;
    } while (var_a0 < 0x50);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/BUS6C
 */
void func_800691C4(s32 arg0, int arg1, u8* arg2)
{
    s32 i;
    s32 k;
    s32 j;
    s32 m;
    func_80068970_Arg0* temp_s0;
    u8* src;
    m = arg0;
    temp_s0 = &D_800F22C8[m];
    temp_s0->unk28 = 0;
    temp_s0->unk27 = 0;
    if (temp_s0->unk25 == 0)
    {
        return;
    }
    temp_s0->unkC->unk14 &= 0x7F;
    temp_s0->unk23A = 0;
    temp_s0->unk23B = 1;
    temp_s0->unk229[0] = 0;
    temp_s0->unk232 = arg1;
    ((u8*)&temp_s0->u224.h.lo)[1] = 0;
    if (arg1 != 0)
    {
        src = arg2;
        j = 0;
        i = 0;
        if (arg1 > 0)
        {
            do
            {
                temp_s0->unk229[j] = *src;
                j++;
                src += 4;
                i++;
            } while (i < arg1);
        }
        if (j != 0)
        {
            temp_s0->unk232 = j;
        }
        else
        {
            func_80068970(temp_s0);
            return;
        }
    }
    else
    {
        temp_s0->unk232 = 1;
        temp_s0->unk229[0] = 0xFF;
    }
    func_8006D21C(temp_s0);
    for (i = 8; i >= 0; i--)
    {
        temp_s0->unk1EC[i] = 0;
    }

    temp_s0->unk236 = 0;
    temp_s0->unk234 = 0;
    for (k = 0; k < temp_s0->unk25; k++)
    {
        temp_s0->unk0[k].unk32 = k;
        for (m = 0; m < 9; m++)
        {
            temp_s0->unk2B[k] = (temp_s0->unkCC[m][k] = (temp_s0->unk3B[m][k] = 0));
        }
    }

    func_800693B4(temp_s0, 1, 0);
    for (i = 0; i < temp_s0->unk25; i++)
    {
        temp_s0->unk0[i].unk32 = i;
        for (j = 0; j < 9; j++)
        {
            temp_s0->unk2B[i] = (temp_s0->unkCC[j][i] = (temp_s0->unk3B[j][i] = 0));
        }
    }
}

/**
 * decomp.me (100%) https://decomp.me/scratch/XDOcQ
 */
void func_800693B4(void* arg0, s32 arg1, s32 arg2)
{
    u8* ptr;
    Struct_D800FDF58* global_table;
    s32 i;
    s32 idx;
    u8* table;
    u16 val;
    s32 temp;
    u8* new_var;
    Struct_D800FDF58* entry;
    u8 d;

    for (i = 0; i < 2; i++)
    {
        ptr = (u8*)arg0 + i;
        idx = i << 1;
        if (ptr[0x27] == 0)
        {
            table = *((u8**)((u8*)arg0 + 0x0C));
            if ((*((u16*)(idx + (u32)table + 4))) == arg1)
            {
                if (arg1 != 1)
                {
                    if ((*((table + i) + 2)) != arg2)
                    {
                        continue;
                    }
                }
                temp = func_8006CE70(((u8*)arg0)[0x228]);
                table = (new_var = *((u8**)((u8*)arg0 + 0x0C)));
                val = *((u16*)((table + idx) + 8));
                switch (val >> 10)
                {
                case 0:
                    func_800A3938(val & 0x3FF, temp);
                    break;

                case 1:
                    if (((u8*)arg0)[0x228] < 2)
                    {
                        func_800A3A90(val & 0x3FF, temp, ((u8*)arg0)[0x228]);
                    }
                    else
                    {
                        global_table = D_800FDF58;
                        entry = &global_table[((u8*)arg0)[0x228]];
                        d = entry->pad2C[0xF];
                        if (((u32)(d - 3)) < 3)
                        {
                            if (D_800FF59C != 0)
                            {
                                func_800A39A8(val & 0x3FF, temp, 0, ((u8*)arg0)[0x228]);
                            }
                            else
                            {
                                func_800A39A8(val & 0x3FF, temp, entry->pad2C[0xF] - 3, ((u8*)arg0)[0x228]);
                            }
                        }
                    }
                    break;

                case 2:
                    if ((val & 0x3FF) < 2)
                    {
                        s32* field = (s32*)(((val & 0x3FF) << 2) + (u32)arg0 + 0x1C);
                        func_800A3E10(*field, temp, ((u8*)arg0)[0x228]);
                    }
                    break;
                }

                if (arg1 == 5)
                {
                    ptr[0x27] |= 0x80;
                }
                ((u8*)arg0 + i)[0x27] |= 1;
            }
        }
    }
}

/**
 * decomp.me (100%) https://decomp.me/scratch/tXpD1
 */
s32 func_800695D4(s32 arg0)
{
    func_80068970_Arg0* temp_v1;

    temp_v1 = &D_800F22C8[arg0];
    return (temp_v1->unk23A | temp_v1->unk23B) != 0;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/8lvHC
 */
s32 func_8006960C(void)
{
    int new_var3;
    func_80068970_Arg0* ptr;
    s32 i;
    int new_var2;
    u16 tmp;
    ptr = D_800F22C8;
    for (i = 0; i < 80; i++)
    {
        new_var2 = 0x1F;
        if ((ptr->unk24 != 0) && (!(ptr->u224.unk224 & 1)))
        {
            tmp = tmp >> 16;
            new_var3 = 0x24;
            tmp = ptr->u224.h.unk226;
            if ((tmp < new_var3) && (tmp >= new_var2))
            {
                return ptr->unk228 | 0x200;
            }
        }
        ptr++;
    }

    return 0;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/etUW8
 */
void func_80069684(void)
{
    Struct_801ED600* ptr_801ED600 = (Struct_801ED600*)0x801ED600;
    func_80068970_Arg2* var_s3;
    Struct_Unk28* var_s1;
    Struct_C* temp_v1_3;
    s32 var_s0;
    s32 var_s6;
    u8* new_var;
    s32 i;
    s32 tmp;
    u16 temp_a0_4;
    u32 div_num;
    u32 div_den;
    var_s3 = D_800F22C8;
    var_s6 = 0;
    ptr_801ED600->unk140 = 0U;
    ptr_801ED600->unk92 = 0U;
    do
    {
        var_s1 = &var_s3->unk28;
        if ((&var_s3->unk28)->unk1FA != 0)
        {
            if ((&var_s3->unk28)->unk213 != 0)
            {
                func_80069AF8(var_s3);
            }
            if ((&var_s3->unk28)->unk212 != (var_s3->unkC->unk15 * 0))
            {
                func_8007100C(var_s3);
                i = var_s3->unkC->unk14;
                if (((unsigned char)i) == 2)
                {
                    if ((&var_s3->unk0[var_s3->unkC->unk15])->unk34 & 0x04000000)
                    {
                        for (var_s0 = 0; var_s0 < (&var_s3->unk28)->unk20A; var_s0++)
                        {
                            u16 unk1ec = (&var_s3->unk28)->unk1C4[var_s0];
                            if (((&var_s3->unk0[var_s3->unkC->unk15])->unk31 < unk1ec) &&
                                ((&var_s3->unk28)->unk1C4[var_s0] < ((&var_s3->unk0[var_s3->unkC->unk15])->unk31 + (&var_s3->unk0[var_s3->unkC->unk15])->unkD)))
                            {
                                D_80105788 = var_s0;
                                func_80099A48(var_s3, &var_s3->unk0[var_s3->unkC->unk15]);
                            }
                        }
                    }
                    else if ((&var_s3->unk0[var_s3->unkC->unk15])->unk31 == 0xFF)
                    {
                        for (var_s0 = 0; var_s0 < (&var_s3->unk28)->unk20A; var_s0++)
                        {
                            if ((&var_s3->unk0[var_s3->unkC->unk15])->unkD > (&var_s3->unk28)->unk1C4[var_s0])
                            {
                                D_80105788 = var_s0;
                                func_80099A48(var_s3, &var_s3->unk0[var_s3->unkC->unk15]);
                            }
                        }
                    }
                    else if (((&var_s3->unk0[var_s3->unkC->unk15])->unk31 < (&var_s3->unk28)->unk1C4[0]) &&
                             ((&var_s3->unk28)->unk1C4[0] < ((&var_s3->unk0[var_s3->unkC->unk15])->unk31 + (&var_s3->unk0[var_s3->unkC->unk15])->unkD)))
                    {
                        D_80105788 = 0;
                        func_80099A48(var_s3, &var_s3->unk0[var_s3->unkC->unk15]);
                    }
                }
                temp_v1_3 = var_s3->unkC;
                temp_a0_4 = temp_v1_3->unkE;
                if ((temp_a0_4 & 0x8000) && (((u8*)temp_v1_3)[0xE] == (&var_s3->unk28)->unk1C4[0]))
                {
                    func_8005A67C((temp_a0_4 >> 8) & 0x7F, 0);
                }
                func_8006D270(var_s3);
                if (func_80068970(var_s3) == 0)
                {
                    for (i = 0; i < (&var_s3->unk28)->unk20A; i++)
                    {
                        if (((&var_s3->unk28)->unk212 >> i) & 1)
                        {
                            (&var_s3->unk28)->unk1C4[i] += 1;
                        }
                    }

                    (&var_s3->unk28)->unk20E += 1;
                    if (((&var_s3->unk28)->unk210 != 0) && ((&var_s3->unk28)->unk20A != 0))
                    {
                        div_num = var_s1->unk20E;
                        div_den = var_s1->unk210;
                        if ((div_num % div_den) == 0)
                        {
                            i = (div_num / div_den) & 0xFFFF;
                            if (i < (&var_s3->unk28)->unk20A)
                            {
                                (&var_s3->unk28)->unk212 |= 1 << i;
                            }
                        }
                    }
                }
                D_80105788 = 0;
                for (var_s0 = 0; var_s0 < 2; var_s0++)
                {
                    u8 temp_a1 = (new_var = var_s3->unkC->unk0)[var_s0];
                    if ((new_var[var_s0] != 0xFF) && (temp_a1 < 0x10U))
                    {
                        if (var_s0 != 0)
                        {
                            u8 temp_v1_6 = func_80068734((Struct_Arg0*)var_s3, temp_a1 & 0xF) | ptr_801ED600->unk92;
                            ptr_801ED600->unk92 = temp_v1_6;
                            ptr_801ED600->unk140 = temp_v1_6;
                        }
                        else
                        {
                            u8 temp_v0_2 = func_80068734((Struct_Arg0*)var_s3, (tmp = new_var[0]) & 0xF);
                            ptr_801ED600->unk91 = (ptr_801ED600->unk13F = temp_v0_2);
                        }
                    }
                }
            }
            var_s3->unk27 &= 0xFE;
            (&var_s3->unk28)->unk0 &= 0xFE;
        }
        var_s6 += 1;
        var_s3 += 1;
    } while (var_s6 < 0x50);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/VZWgF
 */
void func_80069AF8(ContextStruct* arg0)
{
    s32 i;
    u16 check238 = arg0->unk238;

    // hack
    check238++;
    check238--;

    arg0->unk23A = 1;
    if (check238 == 0)
    {
        arg0->unk23A = 0;
        for (i = 0; i < arg0->unk232; i++)
        {
            arg0->unk23A |= 1 << i;
        }
    }
    arg0->unk23B = 0;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/hvTSS
 */
void func_80069B44(s32 arg0, s32 arg1)
{
    func_80074D7C();
    func_80069B84(arg0, arg1);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/Sgd61
 */
void func_80069B84(void *arg0)
{
  u32 new_var;
  Struct_Arg4 *arg0_1 = (Struct_Arg4 *) arg0;
  s32 new_var4;
  u32 *var_s7 = &arg0_1->unk40;
  u32 *p_s4 = (u32 *) arg0_1->unk40B8;
  func_80068970_Arg0 *var_s2 = D_800F22C8;
  s32 var_s5 = 0;
  u32 sp_arr_word;
  int new_var6;
  int new_var3;
  int new_var2;
  do
  {
    new_var3 = var_s2->unk23A != 0;
    if (new_var3)
    {
      u16 temp_v1 = var_s2->unkC->unk18;
      if ((temp_v1 & 2) && (!(var_s2->unkC->unk18 & 8)))
      {
        u8 var_a1;
        s32 cond;
        D_80105788 = 0;
        cond = func_80068734(var_s2, (var_s2->unkC->unk18 >> 8) & 0xF);
        var_a1 = 0;
        if (cond != 0)
        {
          var_a1 = 0xFE;
        }
        if (var_a1 != 0)
        {
          D_800FDF58[var_s2->unk228].unk25 = var_a1;
          D_80105AE0[var_s2->unk228].u.unk178 |= 1;
          D_80105AE0[var_s2->unk228].u.b.unk17A = var_s2->unk233;
        }
        else
        {
          u8 temp_v1_2 = var_s2->unk228;
          s16 temp_a0 = D_800FDF58[temp_v1_2].unk2A;
          if ((temp_a0 == 0x90) || (temp_a0 == 0x94))
          {
            if (D_80105AE0[temp_v1_2].unkC & 0x200)
            {
              D_800FDF58[var_s2->unk228].unk25 = var_a1;
            }
          }
          else
          {
            D_800FDF58[var_s2->unk228].unk25 = var_a1;
          }
        }
      }
      new_var2 = var_s2->unkC->unk18 & 4;
      if (new_var2 && (!(var_s2->unkC->unk18 & 0x10)))
      {
        s32 var_s1 = 0;
        if (var_s2->unk232 != 0)
        {
          do
          {
            u8 var_a1_2;
            s32 cond2;
            D_80105788 = var_s1;
            cond2 = func_80068734(var_s2, var_s2->unkC->unk18 >> 0xC);
            var_a1_2 = 0;
            if (cond2 != 0)
            {
              var_a1_2 = 0xFE;
            }
            if (var_s2->unk229[var_s1] != 0xFF)
            {
              if (var_a1_2 != 0)
              {
                D_800FDF58[var_s2->unk229[var_s1]].unk25 = var_a1_2;
                do
                {
                  D_80105AE0[var_s2->unk229[var_s1]].u.unk178 |= 1;
                  D_80105AE0[var_s2->unk229[var_s1]].u.b.unk17A = var_s2->unk233;
                }
                while (0);
                ((u8 *) var_s2)[0x225] = 1;
              }
              else
              {
                new_var4 = var_s1;
                D_800FDF58[var_s2->unk229[new_var4]].unk25 = 0;
              }
            }
            var_s1++;
          }
          while (var_s1 < var_s2->unk232);
        }
      }
    }
    var_s5++;
    var_s2++;
  }
  while (var_s5 < 0x50);
  D_80105788 = 0;
  var_s2 = D_800F22C8;
  var_s5 = 0;
  do
  {
    if (var_s2->unk23A != 0)
    {
      Struct_C *temp_a0_2 = var_s2->unkC;
      if (temp_a0_2->unkC & 0x1000)
      {
        s32 new_var33 = temp_a0_2->unkC >> 0xD;
        switch (new_var33 & 3)
        {
          case 0:
            D_800F2278 = func_80068734(var_s2, temp_a0_2->unk10 >> 0xC);
            break;

          case 1:
            D_800F227C = func_80068734(var_s2, temp_a0_2->unk10 >> 0xC);
            break;

          case 2:
            D_800F227C = (D_800F2278 = func_80068734(var_s2, temp_a0_2->unk10 >> 0xC));
            break;

          case 3:
            D_800F2278 = func_80068734(var_s2, temp_a0_2->unk10 >> 0xC);
            D_800F227C = func_80068734(var_s2, ((var_s2->unkC->unk10 >> 0xC) + 1) & 0xF);
            break;

        }

      }
    }
    var_s5++;
    var_s2++;
  }
  while (var_s5 < 0x50);
  var_s2 = D_800F22C8;
  var_s5 = 0;
  do
  {
    new_var3 = 0x00FFFFFF;
    if ((var_s2->unk23A != 0) && (var_s2->unk24 != 0))
    {
      Struct_C *temp_v1_7 = var_s2->unkC;
      u8 temp_a1 = (u8) temp_v1_7->unkC;
      if (((u8) temp_v1_7->unkC) < 0x10)
      {
        if (((temp_v1_7->unkC >> 8) & 1) != 0)
        {
          do
          {
            ((u8 *) (&sp_arr_word))[0] = func_80068734(var_s2, (u8) temp_v1_7->unkC);
          }
          while (0);
          ((u8 *) (&sp_arr_word))[1] = func_80068734(var_s2, (((u8) var_s2->unkC->unkC) + 1) & 0xF);
          ((u8 *) (&sp_arr_word))[2] = func_80068734(var_s2, (((u8) var_s2->unkC->unkC) + 2) & 0xF);
        }
        else
        {
          ((u8 *) (&sp_arr_word))[0] = (((u8 *) (&sp_arr_word))[1] = (((u8 *) (&sp_arr_word))[2] = func_80068734(var_s2, temp_a1 & 0xF)));
        }
        if ((var_s2->unkC->unkC >> 8) & 4)
        {
          func_8006A240(((u8 *) (&sp_arr_word))[0] * 2, ((u8 *) (&sp_arr_word))[1] * 2, ((u8 *) (&sp_arr_word))[2] * 2);
        }
        else
        {
          u32 var_a0;
          if ((var_s2->unkC->unkC >> 8) & 1)
          {
            if (((u8 *) (&sp_arr_word))[0] == 0)
            {
              if (((u8 *) (&sp_arr_word))[1] == 0)
              {
                if (((u8 *) (&sp_arr_word))[2] == 0)
                {
                  goto block_59;
                }
              }
            }
          }
          else
            if (((u8 *) (&sp_arr_word))[0] == 0)
          {
            goto block_59;
          }
          var_a0 = 0xE1000005;
          {
            u8 *p_s0 = ((u8 *) p_s4) + 4;
            new_var = sp_arr_word;
            p_s0[-1] = 3;
            *((u16 *) (p_s0 + 8)) = 0x140;
            *((u16 *) (p_s0 + 0xA)) = 0xF0;
            *((u16 *) (p_s0 + 6)) = 0;
            *((u16 *) (p_s0 + 4)) = 0;
            *((u32 *) p_s0) = new_var;
            p_s0[3] = 0x62;
            p_s4[0] = (p_s4[0 ^ 0] & 0xFF000000) | ((*var_s7) & new_var3);
            *var_s7 = ((*var_s7) & 0xFF000000) | (((u32) p_s4) & 0x00FFFFFF);
            new_var6 = (((var_s2->unkC->unkC >> 9) & 3) + 1) & 3;
            p_s4 += 4;
            {
              u8 *p_s0_2 = ((u8 *) p_s4) + 4;
              p_s0_2[-1] = 1;
              *((u32 *) p_s0_2) = (new_var6 << 5) | var_a0;
            }
            var_a0 = 0x00FFFFFF;
            p_s4[0] = (p_s4[0] & 0xFF000000) | ((*var_s7) & var_a0);
            *var_s7 = ((*var_s7) & 0xFF000000) | (((u32) p_s4) & 0x00FFFFFF);
            p_s4 += 2;
          }
        }
      }
    }
    block_59:
    var_s5++;

    var_s2++;
  }
  while (var_s5 < 0x50);
  func_8006A258();
  arg0_1->unk40B8 = p_s4;
}