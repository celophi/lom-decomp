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
extern s32 D_8011D538[];
extern WmapPair D_80139988[];
extern WmapAfcEntry D_801AFBD0[];
extern s32 D_800D9150;
extern s32 D_800DCEA8;
extern s32 D_801B25D8;
extern s32 D_801B2778;
extern s32 D_801B277C;

extern void func_8007E9C4(void);

/**
 * @brief Initialize a range of world-map per-entry records and schedule the next step.
 */
void func_8007DB50(void)
{
    s32 i;
    WmapD94Entry *entry;

    i = 100;
    D_801B25D8 = 1;
    D_800DCEA8 = 1;

    do
    {
        entry = &D_800D9268[i];
        D_80139988[i].unk4 = D_8011D538;
        entry->unkE = 2;
        entry->unk2 = 0;
        entry->unk6 = 0xF;
        entry->unk10 = -1;
        D_801AFBD0[i].unk0 = 0;
        i++;
    } while (i < 200);

    D_800D9150 = 2;
    D_801B277C = 0x10;
    D_801B2778 += 1;
    func_8007E9C4();
}
