#include "wmap_effect_backdrop.h"
#include "common.h"

typedef struct
{
    u8 pad_00[0x26];
    s16 display_mode;
    u8 pad_28[4];
} WmapActor;

typedef struct
{
    s16 state;
    u8 pad_02[0x12];
} WmapMotion;

extern WmapActor D_800D9268[];
extern WmapMotion D_801AFBD0[];
extern void func_80063F38(void);
extern void func_8005909C(void);
extern void akao_cmd_c2(s32, s32, s32, s32);
extern s32 D_800D9210;
extern s32 D_800D9220;
extern s32 D_800DBE70;
extern s32 D_800DCEA0;
extern s32 D_800DCEE0;
extern s32 D_8011CF50;
extern s32 D_8011D4FC;
extern s32 D_8011D52C;
extern s32 D_80129550;
extern s32 D_801398C4;
extern s32 D_801398FC;
extern s32 D_80139978;
extern s32 D_8013B24C;
extern s32 D_8013B258;
extern s32 D_80182DE0;
extern s32 D_80182E24;
extern s32 D_80182E30;
extern s32 D_80182E3C;
extern s32 D_801ADAE0;
extern s32 D_801ADAEC;
extern s32 D_801ADAF4;
extern s32 D_801ADAFC;

/** @brief Reset map actors, layout, effect selection, and display state. */
void func_80064094(void)
{
    s32 i;

    D_80139978 = -1;
    for (i = 0; i < 256; i++)
    {
        D_800D9268[i].display_mode = 16;
        D_801AFBD0[i].state = 0;
    }
    func_80063F38();
    D_8011CF50 = 0;
    D_8013B258 = 0;
    D_80182E24 = 0;
    D_80129550 = 0;
    D_8011D4FC = -1;
    D_80182E30 = 0;
    D_800DCEE0 = 0;
    func_8005909C();
    D_8011D52C = 0;
    func_8006D870(0);
    D_800DBE70 = 2;
    D_801ADAE0 = 0;
    D_801ADAEC = 0x80;
    D_801ADAF4 = 0x10;
    D_80182DE0 = 0;
    D_801398FC = 0;
    D_801398C4 = 0;
    D_800D9210 = 1;
    D_800DCEA0 = -1;
    D_800D9220 = -1;
    akao_cmd_c2(0, 0x1E, 1, 0x7F);
    D_801ADAFC = 1;
    D_8013B24C = 4;
    D_80182E3C = -1;
}
