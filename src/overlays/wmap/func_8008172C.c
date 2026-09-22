#include "common.h"

typedef struct
{
    s16 unk0;
    s16 unk2;
    u8 unk4[2];
    u8 unk6;
    u8 unk7[7];
    s16 unkE;
    s16 unk10;
    u8 unk12[0x10];
    s16 unk22;
    s16 unk24;
    s16 unk26;
    u8 unk28[4];
} WmapD94Entry;

typedef struct
{
    s32 unk0;
    void *unk4;
} WmapPair;

typedef struct
{
    s16 unk0;
    s16 unk2;
    s32 unk4;
    s32 unk8;
    s16 unkC;
    s16 unkE;
    s32 unk10;
} WmapAfcEntry;

extern WmapD94Entry D_800D9268[];
extern s32 D_80123538[];
extern WmapPair D_80139988[];
extern WmapAfcEntry D_801AFBD0[];
extern s32 D_800D9150;
extern s32 D_800DCEA8;
extern s32 D_801B25D8;
extern s32 D_801B2858;
extern s32 D_801B285C;

extern void func_80083A10(void);

/**
 * @brief Initialize a range of world-map per-entry records and schedule the next step.
 */
void func_8008172C(void)
{
    s32 i;
    WmapD94Entry *entry;

    i = 200;
    D_801B25D8 = 1;
    D_800DCEA8 = 1;

    do
    {
        D_801AFBD0[i].unk0 = 0;
        D_80139988[i].unk4 = D_80123538;
        entry = &D_800D9268[i];
        entry->unk2 = 0;
        entry->unk6 = 0xF;
        entry->unkE = 3;
        entry->unk10 = -1;
        i++;
    } while (i < 240);

    D_800D9150 = 2;
    D_801B285C = 0x10;
    D_801B2858 += 1;
    func_80083A10();
}
