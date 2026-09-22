#include "wmap_sequence_runtime.h"
#include "common.h"
#include "sdk/libgte.h"

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
    s16 unk0;
    s16 unk2;
    s32 unk4;
    s32 unk8;
    s16 unkC;
    s16 unkE;
    s32 unk10;
} WmapAfcEntry;

typedef struct
{
    s32 unk0;
    void *unk4;
} WmapPair;

extern WmapD94Entry D_800D94D0[];
extern s32 D_8011D538[];
extern s32 D_80139240;
extern s32 D_8013924C;
extern s32 D_80139250;
extern s32 D_80139260;
extern s32 D_80139264;
extern s32 D_80139268;
extern s32 D_8013926C;
extern s32 D_80139284;
extern WmapPair D_80139988[];
extern s32 D_8013B264;
extern s32 D_8013B270;
extern s32 D_8013B278;
extern s32 D_8013B280;
extern s32 D_80182DEC;
extern WmapAfcEntry D_801AFC98[];
extern s32 D_801B0FD0;
extern s32 D_801B2760;
extern s32 D_801B2764;

extern void func_8007E510(void);
extern s32 D_801B2738;
extern s32 D_80139888[];
extern SVECTOR D_8013B240;
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B276C;
extern s32 D_801B2768;

/**
 * @brief Increment a world-map state counter.
 */
void func_8007D410(void)
{
    D_801B2738 += 1;
}

/**
 * @brief Initialize world-map globals and four per-entry state records.
 */
void func_8007D428(void)
{
    s32 i;
    s16 shift;
    WmapD94Entry *entry;

    i = 0;
    D_801B0FD0 = 4;
    D_80182DEC = 0x7F;
    D_80139240 = 0x50;
    D_8013924C = 2;
    D_80139250 = -1;
    D_80139260 = 0x7D0;
    D_80139264 = 0xA;
    D_80139268 = 8;
    D_8013926C = 1;
    D_80139284 = 0;
    D_8013B264 = 0x28;
    D_8013B270 = 0x50;
    D_8013B278 = 0x14;
    D_8013B280 = 1;

    do
    {
        entry = &D_800D94D0[i];
        D_80139988[i + 14].unk4 = D_8011D538;
        shift = i << 10;
        entry->unk6 = 0xF;
        entry->unk10 = -1;
        entry->unk26 = 4;
        entry->unk2 = 0;
        entry->unk22 = 0x81;
        entry->unk24 = 0x81;
        entry->unkE = D_8013926C;
        D_801AFC98[i].unk0 = 1;
        D_801AFC98[i].unk8 = D_80139284;
        D_801AFC98[i].unkC = 0x50;
        D_801AFC98[i].unk2 = shift;
        D_801AFC98[i].unk4 = 0;
        D_801AFC98[i].unkE = 0;
        i++;
    } while (i < 4);

    D_801B2764 = 0x3C;
    D_801B2760++;
    func_8007E510();
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_8007D5A4(void)
{
    MATRIX m;
    s32 x;

    x = D_80139888[2] - 0xDAC;
    D_80139888[2] = x;
    if (x < 0x2710)
    {
        D_80139888[2] = 0x2710;
    }

    PushMatrix();
    RotMatrix(&D_8013B240, &m);
    TransMatrix(&m, &D_8011CF60);
    SetRotMatrix(&m);
    SetTransMatrix(&m);

    if (D_80182DE8 != 0)
    {
        func_8006CD98((s32)D_800DCF18, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DE8);
        D_80182DE8 -= 0x2;
        if (D_80182DE8 < 0)
        {
            D_80182DE8 = 0;
        }
    }

    PopMatrix();
    if (--D_801B276C == 0)
    {
        D_801B2768 += 1;
    }
}
