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
extern s32 D_80121538[];
extern WmapPair D_80139988[];
extern WmapAfcEntry D_801AFBD0[];
extern s32 D_800D9150;
extern s32 D_800DCEA8;
extern s32 D_80182DF0;
extern s32 D_801B2BC0;
extern s32 D_801B2BC4;

extern void func_80096DC8(void);

/**
 * @brief Initialize a range of world-map per-entry records and schedule the next step.
 */
void func_80095BAC(void)
{
    s32 i;
    WmapD94Entry *entry;

    i = 150;
    D_80182DF0 = 1;
    D_800DCEA8 = 1;

    do
    {
        D_801AFBD0[i].unk0 = 0;
        D_80139988[i].unk4 = D_80121538;
        entry = &D_800D9268[i];
        entry->unk2 = 0;
        entry->unk6 = 0xF;
        entry->unkE = 1;
        entry->unk10 = -1;
        i++;
    } while (i < 158);

    D_800D9150 = 3;
    D_801B2BC4 = 0x10;
    D_801B2BC0 += 1;
    func_80096DC8();
}
