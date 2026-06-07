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