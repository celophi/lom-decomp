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

/**
 * decomp.me (100%) https://decomp.me/scratch/9Ady0
 */
void func_8006828C(void)
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