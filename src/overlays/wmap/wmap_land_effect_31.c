#include "wmap_model_render.h"
#include "wmap_land_effect_31.h"
#include "wmap_sprite_render.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_main.h"
#include "wmap_effect_primitives.h"
#include "wmap_sequence_runtime.h"
#include "wmap_effect_resources.h"
#include "cdrom.h"
#include "sdk/libgte.h"

void wmap_land_effect_31_sequence_20_step_02(void);
void wmap_land_effect_31_sequence_20_step_04(void);
void wmap_land_effect_31_sequence_19_step_02(void);
void wmap_land_effect_31_sequence_19_step_04(void);
void wmap_land_effect_31_sequence_5_step_04(void);
void wmap_land_effect_31_sequence_6_step_04(void);
void wmap_land_effect_31_sequence_15_step_02(void);
void wmap_land_effect_31_sequence_15_step_04(void);
void wmap_land_effect_31_sequence_16_step_02(void);
void wmap_land_effect_31_sequence_16_step_04(void);
void wmap_land_effect_31_sequence_17_step_02(void);
void wmap_land_effect_31_sequence_18_step_04(void);
s32 wmap_land_effect_31_run_timeline(s32 reset);
s32 wmap_land_effect_31_run_sequence_1(s32 reset);
s32 wmap_land_effect_31_run_sequence_2(s32 reset);
s32 wmap_land_effect_31_run_sequence_3(s32 reset);
s32 wmap_land_effect_31_run_sequence_8(s32 reset);
s32 wmap_land_effect_31_run_sequence_9(s32 reset);
s32 wmap_land_effect_31_run_sequence_10(s32 reset);
s32 wmap_land_effect_31_run_sequence_11(s32 reset);
s32 wmap_land_effect_31_run_sequence_12(s32 reset);
s32 wmap_land_effect_31_run_sequence_13(s32 reset);
s32 wmap_land_effect_31_run_sequence_17(s32 reset);
s32 wmap_land_effect_31_run_sequence_18(s32 reset);
s32 wmap_land_effect_31_run_sequence_20(s32 reset);
s32 wmap_land_effect_31_run_sequence_4(s32 reset);
s32 wmap_land_effect_31_run_sequence_5(s32 reset);
s32 wmap_land_effect_31_run_sequence_6(s32 reset);
s32 wmap_land_effect_31_run_sequence_7(s32 reset);
s32 wmap_land_effect_31_run_sequence_14(s32 reset);
s32 wmap_land_effect_31_run_sequence_15(s32 reset);
s32 wmap_land_effect_31_run_sequence_16(s32 reset);
s32 wmap_land_effect_31_run_sequence_19(s32 reset);
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);
void wmap_land_effect_31_sequence_1_step_02(void);
void wmap_land_effect_31_sequence_5_step_02(void);
void wmap_land_effect_31_sequence_6_step_02(void);

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s16 field_04;
} WmapShort3;

typedef struct
{
    s16 field_00;
    s16 field_02;
    u16 field_04;
    s16 field_06;
} WmapShort4;

typedef struct
{
    s32 field_00;
    s32 field_04;
    s32 field_08;
} WmapInt3;

typedef struct
{
    s32 words[9];
} WmapBlock36;

typedef struct
{
    s32 value;
    u8 pad[0x24];
} WmapCell;

typedef struct
{
    s32 field_00;
    s32 field_04;
    s32 field_08;
    s32 field_0C;
    s32 field_10;
    s32 field_14;
    s32 field_18;
    s32 field_1C;
    s32 field_20;
    s32 state_24;
    s32 field_28;
    s32 field_2C;
    s32 field_30;
    s32 field_34;
    s32 field_38;
    s32 field_3C;
    s32 field_40;
    s32 field_44;
    s32 field_48;
    s32 tail_state;
} WmapState;

typedef void (*WmapHandler)(void);

/** @brief World-map 0x14-byte slot: only the leading halfword is cleared here. */
typedef struct
{
    s16 field_00;
    u8 pad_02[0x12];
} WmapSlot14;

extern WmapHandler D_800D7474[];
extern WmapHandler D_800D748C[];
extern WmapHandler D_800D753C[];
extern WmapHandler D_800D7554[];
extern WmapHandler D_800D7564[];
extern WmapHandler D_800D757C[];
extern WmapHandler D_800D7594[];
extern WmapHandler D_800D75AC[];
extern WmapHandler D_800D75C4[];
extern WmapHandler D_800D75D4[];
extern WmapHandler D_800D75E4[];
extern WmapHandler D_800D75F4[];
extern WmapHandler D_800D760C[];
extern WmapHandler D_800D7624[];
extern WmapHandler D_800D763C[];
extern WmapHandler D_800D7654[];
extern WmapHandler D_800D766C[];
extern WmapHandler D_800D7684[];
extern WmapHandler D_800D769C[];
extern WmapHandler D_800D76AC[];
extern WmapHandler D_800D76C4[];
extern WmapHandler D_800D76DC[];
extern u8 D_800D95D8[];
extern u8 D_800D9D68[];
extern WmapBlock36 D_800D06BC;
extern WmapBlock36 D_800D9240;
extern WmapShort3 D_800DCEB8;
extern s32 D_800DCF18[];
extern s32* D_8011CF1C;
extern s32* D_8011CF24;
extern s32* D_8011CF28;
extern s32* D_8011CF2C;
extern s32* D_8011CF30;
extern s32* D_8011CF34;
extern s32* D_8011CF38;
extern s32* D_8011CF3C;
extern s32 D_8011D4FC;
extern s32 D_8011D510;
extern s32 D_8011D530;
extern u8 D_8011D538;
extern u8 D_80121538;
extern s32 D_80139234;
extern WmapInt3 D_80139200;
extern WmapShort3 D_80139210;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139244;
extern s32 D_8013924C;
extern s32 D_80139260;
extern s32 D_80139264;
extern s32 D_80139268;
extern s32 D_8013926C;
extern WmapCell D_80139290[][6];
extern s32 g_wmap_view_mode;
extern u8 D_80139B88[];
extern WmapInt3 D_80139968;
extern void* D_801399B4;
extern void* D_801399BC;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern s32 D_8013B208;
extern s32 D_8013B29C;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern s32 D_80182DE4;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern s32 D_801ADAE0;
extern s32 D_801B24B4;
extern s32 D_801B25D8;
extern s32 D_801B25DC;
extern s32 D_801B25E0;
extern s32 g_wmap_land_effect_31_step;
extern s32 g_wmap_land_effect_31_timer;
extern s32 g_wmap_land_effect_31_timeline_step;
extern s32 g_wmap_land_effect_31_timeline_timer;
extern s32 g_wmap_land_effect_31_sequence_1_step;
extern s32 g_wmap_land_effect_31_sequence_1_timer;
extern s32 g_wmap_land_effect_31_sequence_2_step;
extern s32 g_wmap_land_effect_31_sequence_2_timer;
extern s32 g_wmap_land_effect_31_sequence_3_step;
extern s32 g_wmap_land_effect_31_sequence_3_timer;
extern s32 g_wmap_land_effect_31_sequence_4_step;
extern s32 g_wmap_land_effect_31_sequence_4_timer;
extern s32 g_wmap_land_effect_31_sequence_5_step;
extern s32 g_wmap_land_effect_31_sequence_5_timer;
extern s32 g_wmap_land_effect_31_sequence_6_step;
extern s32 g_wmap_land_effect_31_sequence_6_timer;
extern s32 g_wmap_land_effect_31_sequence_7_step;
extern s32 g_wmap_land_effect_31_sequence_7_timer;
extern s32 g_wmap_land_effect_31_sequence_8_step;
extern s32 g_wmap_land_effect_31_sequence_8_timer;
extern s32 g_wmap_land_effect_31_sequence_9_step;
extern s32 g_wmap_land_effect_31_sequence_9_timer;
extern s32 g_wmap_land_effect_31_sequence_10_step;
extern s32 g_wmap_land_effect_31_sequence_10_timer;
extern s32 g_wmap_land_effect_31_sequence_11_step;
extern s32 g_wmap_land_effect_31_sequence_11_timer;
extern s32 g_wmap_land_effect_31_sequence_12_step;
extern s32 g_wmap_land_effect_31_sequence_12_timer;
extern s32 g_wmap_land_effect_31_sequence_13_step;
extern s32 g_wmap_land_effect_31_sequence_13_timer;
extern s32 g_wmap_land_effect_31_sequence_14_step;
extern s32 g_wmap_land_effect_31_sequence_14_timer;
extern s32 g_wmap_land_effect_31_sequence_15_step;
extern s32 g_wmap_land_effect_31_sequence_15_timer;
extern s32 g_wmap_land_effect_31_sequence_16_step;
extern s32 g_wmap_land_effect_31_sequence_16_timer;
extern s32 g_wmap_land_effect_31_sequence_17_step;
extern s32 g_wmap_land_effect_31_sequence_17_timer;
extern s32 g_wmap_land_effect_31_sequence_18_step;
extern s32 g_wmap_land_effect_31_sequence_18_timer;
extern s32 g_wmap_land_effect_31_sequence_19_step;
extern s32 g_wmap_land_effect_31_sequence_19_timer;
extern s32 g_wmap_land_effect_31_sequence_20_step;
extern s32 g_wmap_land_effect_31_sequence_20_timer;
extern s32 D_801B0FD0;
extern void wmap_land_effect_31_sequence_4_step_02(void);
extern void wmap_land_effect_31_sequence_4_step_04(void);
extern void wmap_land_effect_31_sequence_7_step_02(void);
extern void wmap_land_effect_31_sequence_8_step_02(void);
extern void wmap_land_effect_31_sequence_9_step_02(void);
extern void wmap_land_effect_31_sequence_10_step_02(void);
extern void wmap_land_effect_31_sequence_10_step_04(void);
extern void wmap_land_effect_31_sequence_11_step_02(void);
extern void wmap_land_effect_31_sequence_11_step_04(void);
extern void wmap_land_effect_31_sequence_12_step_02(void);
extern void wmap_land_effect_31_sequence_12_step_04(void);
extern void wmap_land_effect_31_sequence_13_step_02(void);
extern void wmap_land_effect_31_sequence_13_step_04(void);
extern s32* D_8011CF40;
extern s32 wmap_land_effect_31_run_timeline(s32);
extern s32 wmap_land_effect_31_run_sequence_1(s32);
extern s32 wmap_land_effect_31_run_sequence_2(s32);
extern s32 wmap_land_effect_31_run_sequence_3(s32);
extern s32 wmap_land_effect_31_run_sequence_4(s32);
extern s32 wmap_land_effect_31_run_sequence_5(s32);
extern void wmap_land_effect_31_sequence_5_step_04(void);
extern s32 wmap_land_effect_31_run_sequence_6(s32);
extern void wmap_land_effect_31_sequence_6_step_04(void);
extern s32 wmap_land_effect_31_run_sequence_7(s32);
extern s32 wmap_land_effect_31_run_sequence_8(s32);
extern s32 wmap_land_effect_31_run_sequence_9(s32);
extern s32 wmap_land_effect_31_run_sequence_10(s32);
extern s32 wmap_land_effect_31_run_sequence_11(s32);
extern s32 wmap_land_effect_31_run_sequence_12(s32);
extern s32 wmap_land_effect_31_run_sequence_13(s32);
extern s32 wmap_land_effect_31_run_sequence_14(s32);
extern s32 wmap_land_effect_31_run_sequence_15(s32);
extern void wmap_land_effect_31_sequence_15_step_02(void);
extern void wmap_land_effect_31_sequence_15_step_04(void);
extern s32 wmap_land_effect_31_run_sequence_16(s32);
extern void wmap_land_effect_31_sequence_16_step_02(void);
extern void wmap_land_effect_31_sequence_16_step_04(void);
extern s32 wmap_land_effect_31_run_sequence_17(s32);
extern void wmap_land_effect_31_sequence_17_step_02(void);
extern s32 wmap_land_effect_31_run_sequence_18(s32);
extern void wmap_land_effect_31_sequence_18_step_02(void);
extern void wmap_land_effect_31_sequence_18_step_04(void);
extern s32 wmap_land_effect_31_run_sequence_19(s32);
extern void wmap_land_effect_31_sequence_19_step_02();
extern void wmap_land_effect_31_sequence_19_step_04();
extern s32 wmap_land_effect_31_run_sequence_20(s32);
extern void wmap_land_effect_31_sequence_20_step_02(void);
extern void wmap_land_effect_31_sequence_20_step_04();

typedef struct
{
    s16 field_00;
    s16 field_02;
    u16 field_04;
    s16 field_06;
} WmapPair;

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad_04[2];
    u8 field_06;
    u8 pad_07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad_12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad_28[4];
} WmapConfigA;

extern WmapConfigA D_800DB4C8[];
extern u8 D_8011F538[];
extern WmapPair D_801B3118;

extern VECTOR g_wmap_camera_translation;
extern VECTOR D_801B2650;
extern VECTOR D_801B2478;
extern VECTOR D_80139870;

extern WmapSpriteActor D_800D9344;
extern WmapSpriteActor D_800D9370;
extern WmapSpriteActor D_800D939C;
extern WmapSpriteActor D_800D93C8;
extern WmapSpriteActor D_800D93F4;

extern WmapAnimationSlot D_80139988[];
extern WmapAnimationSlot D_801399B0;
extern WmapAnimationSlot D_801399B8;
extern WmapAnimationSlot D_801399C0;
extern WmapAnimationSlot D_801399C8;
extern WmapAnimationSlot D_801399D0;

extern WmapScreenPosition g_wmap_focus_screen_position;

extern s32* D_80139280;

extern WmapSlot14 D_801AFBD0[];

extern SVECTOR D_80139258;
extern SVECTOR D_801B2498;
extern SVECTOR D_8013B240;
extern SVECTOR D_801B24A0;
extern SVECTOR D_8013B238;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B2670;
extern SVECTOR D_801B24A8;
extern SVECTOR D_801B2678;
extern SVECTOR D_801B3120;

void wmap_land_effect_31_timeline_step_17(void)
{
    g_wmap_view_mode = -1;
    g_wmap_backdrop_target_level = 0;
    D_80182D74.field_00 = 0xB4;
    D_80182D74.field_01 = 0;
    D_80182D74.field_02 = 0x96;
    D_80182D80.field_00 = 0xB4;
    D_80182D80.field_01 = 0;
    D_80182D80.field_02 = 0x96;
    D_80182D8C.field_00 = 0xB4;
    D_80182D8C.field_01 = 0;
    D_80182D8C.field_02 = 0x96;
    D_80182D94.field_00 = 0xB4;
    D_80182D94.field_01 = 0;
    D_80182D94.field_02 = 0x96;
    g_wmap_land_effect_31_timeline_timer = 0x38;
    g_wmap_land_effect_31_timeline_step++;
}

void wmap_land_effect_31_timeline_step_19(void)
{
    D_80139244 = 1;
    wmap_start_sequence(wmap_land_effect_31_run_sequence_7);
    g_wmap_backdrop_target_level = 0;
    D_80182D74.field_00 = 0xC8;
    D_80182D74.field_01 = 0xC8;
    D_80182D74.field_02 = 0xC8;
    D_80182D80.field_00 = 0xC8;
    D_80182D80.field_01 = 0xC8;
    D_80182D80.field_02 = 0xC8;
    D_80182D8C.field_00 = 0xC8;
    D_80182D8C.field_01 = 0xC8;
    D_80182D8C.field_02 = 0xC8;
    D_80182D94.field_00 = 0xC8;
    D_80182D94.field_01 = 0xC8;
    D_80182D94.field_02 = 0xC8;
    g_wmap_land_effect_31_timeline_timer = 0xF;
    g_wmap_land_effect_31_timeline_step++;
}

void wmap_land_effect_31_timeline_step_23(void)
{
    wmap_start_sequence(wmap_land_effect_31_run_sequence_15);
    wmap_start_sequence(wmap_land_effect_31_run_sequence_16);
    wmap_start_sequence(wmap_land_effect_31_run_sequence_14);
    wmap_start_sequence(wmap_land_effect_31_run_sequence_5);
    wmap_start_sequence(wmap_land_effect_31_run_sequence_4);
    g_wmap_backdrop_target_level = 0;
    D_80182D74.field_00 = 0x60;
    D_80182D74.field_01 = 0xC;
    D_80182D74.field_02 = 0xC8;
    D_80182D80.field_00 = 0x60;
    D_80182D80.field_01 = 0xC;
    D_80182D80.field_02 = 0xC8;
    D_80182D8C.field_00 = 0xA0;
    D_80182D8C.field_01 = 0xA0;
    D_80182D8C.field_02 = 0xA0;
    D_80182D94.field_00 = 0xA0;
    D_80182D94.field_01 = 0xA0;
    D_80182D94.field_02 = 0xA0;
    g_wmap_land_effect_31_timeline_timer = 0x14;
    g_wmap_land_effect_31_timeline_step++;
}

void wmap_land_effect_31_timeline_step_37(void)
{
    g_wmap_backdrop_target_level = 0;
    D_80182D74.field_00 = 0;
    D_80182D74.field_01 = 0x64;
    D_80182D74.field_02 = 0x78;
    D_80182D80.field_00 = 0;
    D_80182D80.field_01 = 0x64;
    D_80182D80.field_02 = 0x78;
    D_80182D8C.field_00 = 0;
    D_80182D8C.field_01 = 0x64;
    D_80182D8C.field_02 = 0x78;
    D_80182D94.field_00 = 0;
    D_80182D94.field_01 = 0x64;
    D_80182D94.field_02 = 0x78;
    D_8013B29C = 0;
    wmap_start_map_tint(0x701050);
    g_wmap_view_mode = 0;
    g_wmap_backdrop_target_level = 0;
    wmap_start_sequence(wmap_land_effect_31_run_sequence_19);
    g_wmap_land_effect_31_timeline_timer = 0x58;
    g_wmap_land_effect_31_timeline_step++;
}

void wmap_land_effect_31_timeline_step_41(void)
{
    wmap_start_sequence(wmap_land_effect_31_run_sequence_6);
    D_80182D74.field_00 = 0;
    D_80182D74.field_01 = 0;
    D_80182D74.field_02 = 0;
    D_80182D80.field_00 = 0;
    D_80182D80.field_01 = 0;
    D_80182D80.field_02 = 0;
    D_80182D8C.field_00 = 0;
    D_80182D8C.field_01 = 0;
    D_80182D8C.field_02 = 0;
    D_80182D94.field_00 = 0;
    D_80182D94.field_01 = 0;
    D_80182D94.field_02 = 0;
    D_800D9240 = D_800D06BC;
    g_wmap_land_effect_31_timeline_timer = 0x78;
    g_wmap_land_effect_31_timeline_step++;
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_31_sequence_1_step_01(void)
{
    s32 i;

    D_801B0FD0 = 40;
    D_80139280[0x1F] = -8;
    D_80139280[0x20] = 20;
    D_80139280[0x21] = 20;
    D_80139280[0x22] = 30;
    D_80139280[0x23] = 1;
    D_80139280[0x24] = 3900;
    D_80139280[0x25] = 60;
    D_80139280[0x26] = 15;
    D_80139280[0x27] = 2;
    D_80139280[0x28] = 10000;
    for (i = 0; i < 40; i++)
    {
        D_801AFBD0[i + D_80139280[0x25]].field_00 = 0;
        D_80139988[i + 64].data = D_8011F538;
    }
    g_wmap_land_effect_31_sequence_1_timer = 40;
    g_wmap_land_effect_31_sequence_1_step++;
    wmap_land_effect_31_sequence_1_step_02();
}

void wmap_land_effect_31_sequence_4_step_02(void)
{
    s32 value;
    s32 timer;

    func_8006AEE0();
    wmap_draw_model(D_8011CF1C, (D_80139234 / 0x10) & 3, 0xA, 0x36, 0x7940, 0x1001, D_801B24B4, 0, 0, -1);
    D_80139234 += 0x10;
    value = D_801B24B4 + 2;
    D_801B24B4 = value;
    if (value >= 0x82)
    {
        D_801B24B4 = 0x81;
    }
    ((WmapShort4*)&D_801B2490)->field_04 += 0x40;
    timer = g_wmap_land_effect_31_sequence_4_timer - 1;
    g_wmap_land_effect_31_sequence_4_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_4_step++;
    }
}

void wmap_land_effect_31_sequence_4_step_04(void)
{
    s32 value;
    s32 timer;
    WmapShort4* position;

    func_8006AEE0();
    wmap_draw_model(D_8011CF1C, (D_80139234 / 0x10) & 3, 0xA, 0x36, 0x7940, 0x1001, D_801B24B4, 0, 0, -1);
    value = D_801B24B4 - 2;
    D_801B24B4 = value;
    if (value < 0)
    {
        D_801B24B4 = 0;
    }
    position = (WmapShort4*)&D_801B2490;
    D_80139234 += 0x10;
    timer = g_wmap_land_effect_31_sequence_4_timer - 1;
    g_wmap_land_effect_31_sequence_4_timer = timer;
    position->field_04 += 0x40;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_4_step++;
    }
}

/** @brief Configure the effect and its resource slots, then advance the sequence. */
void wmap_land_effect_31_sequence_5_step_01(void)
{
    s32 i;

    D_80139280[0] = 40;
    D_80139280[1] = 20;
    D_80139280[2] = -210;
    D_80139280[3] = 420;
    D_80139280[4] = -210;
    D_80139280[5] = 420;
    D_80139280[6] = 0;
    D_80139280[7] = 80;
    D_80139280[8] = 129;
    D_80139280[9] = 1;
    D_80139280[10] = 8;
    D_80139280[11] = 80;
    D_80139280[12] = 15;
    D_80139280[13] = 0;
    D_801B3120 = D_80139258;
    for (i = 0; i < 40; i++)
    {
        D_80139988[i + 20].data = D_8011F538;
    }
    g_wmap_land_effect_31_sequence_5_timer = 204;
    g_wmap_land_effect_31_sequence_5_step++;
    wmap_land_effect_31_sequence_5_step_02();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_31_sequence_6_step_01(void)
{
    s32 i;

    D_801B0FD0 = 32;
    D_80139280[0x15] = -3;
    D_80139280[0x16] = 8;
    D_80139280[0x17] = 20;
    D_80139280[0x18] = 30;
    D_80139280[0x19] = 1;
    D_80139280[0x1A] = 1500;
    D_80139280[0x1B] = 60;
    D_80139280[0x1C] = 15;
    D_80139280[0x1D] = 1;
    D_80139280[0x1E] = 10000;
    for (i = 0; i < 32; i++)
    {
        D_801AFBD0[i + D_80139280[0x1B]].field_00 = 0;
        D_80139988[i + 64].data = D_8011F538;
    }
    g_wmap_land_effect_31_sequence_6_timer = 32;
    g_wmap_land_effect_31_sequence_6_step++;
    wmap_land_effect_31_sequence_6_step_02();
}

void wmap_land_effect_31_sequence_7_step_02(void)
{
    s32 value;
    s32 fade;
    s32 timer;

    value = D_801B2650.vz - 0xDAC;
    D_801B2650.vz = value;
    if (value < 0x2710)
    {
        D_801B2650.vz = 0x2710;
    }
    PushMatrix();
    wmap_set_map_rotation(&D_801B24A0);
    if (D_80182DE8 != 0)
    {
        wmap_draw_model(D_8011CF34, 0, 4, 0x35, 0x7800, 1, D_80182DE8, 0, 0, -1);
        fade = D_80182DE8 - 1;
        D_80182DE8 = fade;
        if (fade < 0)
        {
            D_80182DE8 = 0;
        }
    }
    PopMatrix();
    timer = g_wmap_land_effect_31_sequence_7_timer - 1;
    g_wmap_land_effect_31_sequence_7_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_7_step++;
    }
}

void wmap_land_effect_31_sequence_8_step_02(void)
{
    s32 value;
    s32 fade;
    s32 timer;

    value = D_801B2478.vz - 0xDAC;
    D_801B2478.vz = value;
    if (value < 0x2710)
    {
        D_801B2478.vz = 0x2710;
    }
    PushMatrix();
    wmap_set_map_rotation(&D_801B24A8);
    if (D_80182DEC != 0)
    {
        wmap_draw_model(D_8011CF3C, 0, 4, 0x35, 0x7800, 1, D_80182DEC, 0, 0, -1);
        fade = D_80182DEC - 4;
        D_80182DEC = fade;
        if (fade < 0)
        {
            D_80182DEC = 0;
        }
    }
    PopMatrix();
    timer = g_wmap_land_effect_31_sequence_8_timer - 1;
    g_wmap_land_effect_31_sequence_8_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_8_step++;
    }
}

void wmap_land_effect_31_sequence_9_step_02(void)
{
    s32 value;
    s32 fade;
    s32 timer;

    value = D_80139870.vz - 0xDAC;
    D_80139870.vz = value;
    if (value < 0x2710)
    {
        D_80139870.vz = 0x2710;
    }
    PushMatrix();
    wmap_set_map_rotation(&D_8013B238);
    if (D_80182DF0 != 0)
    {
        wmap_draw_model(D_8011CF2C, 0, 4, 0x35, 0x7800, 1, D_80182DF0, 0, 0, -1);
        fade = D_80182DF0 - 2;
        D_80182DF0 = fade;
        if (fade < 0)
        {
            D_80182DF0 = 0;
        }
    }
    PopMatrix();
    timer = g_wmap_land_effect_31_sequence_9_timer - 1;
    g_wmap_land_effect_31_sequence_9_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_9_step++;
    }
}

void wmap_land_effect_31_sequence_10_step_02(void)
{
    s32 value;
    s32 timer;

    wmap_set_map_rotation(&D_801B2498);
    wmap_draw_model(D_800DCF18, (D_8013923C / 0x10) & 7, 0xA, 0x36, 0x7880, 0x1001, D_80182DE4, 6, -0x18, -1);
    D_8013923C += 0x10;
    value = D_80182DE4 + 4;
    D_80182DE4 = value;
    if (value >= 0x82)
    {
        D_80182DE4 = 0x81;
    }
    D_801B2498.vz += 0x10;
    timer = g_wmap_land_effect_31_sequence_10_timer - 1;
    g_wmap_land_effect_31_sequence_10_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_10_step++;
    }
}

void wmap_land_effect_31_sequence_10_step_04(void)
{
    s32 value;
    s32 timer;

    wmap_set_map_rotation(&D_801B2498);
    wmap_draw_model(D_800DCF18, (D_8013923C / 0x10) & 7, 0xA, 0x36, 0x7880, 0x1001, D_80182DE4, 6, -0x18, -1);
    value = D_80182DE4 - 0x10;
    D_80182DE4 = value;
    if (value < 0)
    {
        D_80182DE4 = 0;
    }
    D_8013923C += 0x10;
    timer = g_wmap_land_effect_31_sequence_10_timer - 1;
    g_wmap_land_effect_31_sequence_10_timer = timer;
    D_801B2498.vz += 0x10;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_10_step++;
    }
}

void wmap_land_effect_31_sequence_11_step_02(void)
{
    s32 value;
    s32 timer;

    wmap_set_map_rotation(&D_801B3118);
    wmap_draw_model(D_800DCF18, (D_8013926C / 0x10) & 7, 0xA, 0x36, 0x7880, 0x1001, D_801B25E0, 4, -0x14, -1);
    D_8013926C -= 0x10;
    value = D_801B25E0 + 2;
    D_801B25E0 = value;
    if (value >= 0x82)
    {
        D_801B25E0 = 0x81;
    }
    ((u16*)&D_801B3118)[2] += 0x10;
    timer = g_wmap_land_effect_31_sequence_11_timer - 1;
    g_wmap_land_effect_31_sequence_11_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_11_step++;
    }
}

void wmap_land_effect_31_sequence_11_step_04(void)
{
    s32 value;
    s32 timer;

    wmap_set_map_rotation(&D_801B3118);
    wmap_draw_model(D_800DCF18, (D_8013926C / 0x10) & 7, 0xA, 0x36, 0x7880, 0x1001, D_801B25E0, 4, -0x14, -1);
    value = D_801B25E0 - 0x80;
    D_801B25E0 = value;
    if (value < 0)
    {
        D_801B25E0 = 0;
    }
    D_8013926C -= 0x10;
    timer = g_wmap_land_effect_31_sequence_11_timer - 1;
    g_wmap_land_effect_31_sequence_11_timer = timer;
    ((u16*)&D_801B3118)[2] += 0x10;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_11_step++;
    }
}

void wmap_land_effect_31_sequence_12_step_02(void)
{
    s32 value;
    s32 timer;

    wmap_set_map_rotation(&D_8013B240);
    wmap_draw_model(D_8011CF28, (D_80139260 / 0x10) & 3, 0xA, 0x36, 0x7900, 0x1001, D_80182DF4, 4, -0x14, -1);
    D_80139260 -= 0x20;
    value = D_80182DF4 + 2;
    D_80182DF4 = value;
    if (value >= 0x82)
    {
        D_80182DF4 = 0x81;
    }
    D_8013B240.vz += 0x90;
    timer = g_wmap_land_effect_31_sequence_12_timer - 1;
    g_wmap_land_effect_31_sequence_12_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_12_step++;
    }
}

void wmap_land_effect_31_sequence_12_step_04(void)
{
    s32 value;
    s32 timer;

    wmap_set_map_rotation(&D_8013B240);
    wmap_draw_model(D_8011CF28, (D_80139260 / 0x10) & 3, 0xA, 0x36, 0x7900, 0x1001, D_80182DF4, 4, -0x14, -1);
    value = D_80182DF4 - 8;
    D_80182DF4 = value;
    if (value < 0)
    {
        D_80182DF4 = 0;
    }
    D_80139260 -= 0x20;
    timer = g_wmap_land_effect_31_sequence_12_timer - 1;
    g_wmap_land_effect_31_sequence_12_timer = timer;
    D_8013B240.vz += 0x90;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_12_step++;
    }
}

void wmap_land_effect_31_sequence_13_step_02(void)
{
    s32 value;
    s32 timer;

    wmap_set_map_rotation(&D_801B2670);
    wmap_draw_model(D_8011CF38, (D_80139264 / 0x10) & 3, 0xA, 0x36, 0x7900, 0x1001, D_801B25D8, 6, -0x18, -1);
    D_80139264 += 0x10;
    value = D_801B25D8 + 2;
    D_801B25D8 = value;
    if (value >= 0x82)
    {
        D_801B25D8 = 0x81;
    }
    D_801B2670.vz += 0x60;
    timer = g_wmap_land_effect_31_sequence_13_timer - 1;
    g_wmap_land_effect_31_sequence_13_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_13_step++;
    }
}

void wmap_land_effect_31_sequence_13_step_04(void)
{
    s32 value;
    s32 timer;

    wmap_set_map_rotation(&D_801B2670);
    wmap_draw_model(D_8011CF38, (D_80139264 / 0x10) & 3, 0xA, 0x36, 0x7900, 0x1001, D_801B25D8, 6, -0x18, -1);
    value = D_801B25D8 - 0x10;
    D_801B25D8 = value;
    if (value < 0)
    {
        D_801B25D8 = 0;
    }
    D_80139264 += 0x10;
    timer = g_wmap_land_effect_31_sequence_13_timer - 1;
    g_wmap_land_effect_31_sequence_13_timer = timer;
    D_801B2670.vz += 0x60;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_13_step++;
    }
}

void wmap_land_effect_31_sequence_14_step_02(void)
{
    s32 value;
    s32 timer;

    func_8006AEE0();
    wmap_draw_model(D_8011CF40, (D_80139268 / 0x10) & 3, 0xA, 0x36, 0x7880, 1, D_801B25DC, 0, 0, -1);
    D_80139268 += 8;
    value = D_801B25DC + 2;
    D_801B25DC = value;
    if (value >= 0x82)
    {
        D_801B25DC = 0x81;
    }
    D_801B2678.vz += 0;
    timer = g_wmap_land_effect_31_sequence_14_timer - 1;
    g_wmap_land_effect_31_sequence_14_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_14_step++;
    }
}

void wmap_land_effect_31_sequence_14_step_04(void)
{
    s32 value;
    s32 timer;

    func_8006AEE0();
    wmap_draw_model(D_8011CF40, (D_80139268 / 0x10) & 3, 0xA, 0x36, 0x7880, 1, D_801B25DC, 0, 0, -1);
    value = D_801B25DC - 4;
    D_801B25DC = value;
    if (value < 0)
    {
        D_801B25DC = 0;
    }
    D_80139268 += 8;
    D_801B2678.vz += 0;
    timer = g_wmap_land_effect_31_sequence_14_timer - 1;
    g_wmap_land_effect_31_sequence_14_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_14_step++;
    }
}

void wmap_land_effect_31_sequence_20_step_01(void)
{
    s32 index;
    s32 config_offset;
    s32 screen_offset;
    u8* config_base;
    u8* screen_base;
    u8* resource;
    u8* screen_entry;
    s16* config_entry;

    index = 0;
    config_base = D_801AFBD0;
    screen_base = (u8*)D_80139988;
    resource = &D_80121538;
    screen_offset = 0x640;
    config_offset = 0xFA0;
    D_80139280[40] = 0x28;
    D_80139280[41] = 0xC8;
    D_80139280[42] = -0x136;
    D_80139280[43] = 0x28;
    D_80139280[44] = -0x64;
    D_80139280[45] = 0x28;
    D_80139280[46] = 0;
    D_80139280[47] = 0x2D;
    D_80139280[48] = 0x81;
    D_80139280[49] = 1;
    D_80139280[50] = 0x20;
    D_80139280[51] = 4;
    D_80139280[52] = 0xF;
    D_80139280[53] = 0;

    do
    {
        screen_entry = (u8*)(screen_offset + (s32)screen_base);
        screen_offset += 8;
        config_entry = (s16*)(config_offset + (s32)config_base);
        config_offset += 0x14;
        index++;
        *config_entry = 0;
        *(u8**)(screen_entry + 4) = resource;
    } while (index < 0x28);

    g_wmap_land_effect_31_sequence_20_timer = 0x64;
    g_wmap_land_effect_31_sequence_20_step++;
    wmap_land_effect_31_sequence_20_step_02();
}

s32 wmap_land_effect_31_run(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_land_effect_31_step = 1;
        g_wmap_land_effect_31_timer = 1;
        return 1;
    }

    if ((u32)g_wmap_land_effect_31_step >= 6)
    {
        return 0;
    }

    D_800D7474[g_wmap_land_effect_31_step]();
    return 1;
}

void wmap_land_effect_31_reset(void)
{
    g_wmap_land_effect_31_step = 1;
    g_wmap_land_effect_31_timer = 1;
}

void wmap_land_effect_31_step_01(void)
{
    wmap_start_sequence(wmap_run_land_focus);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_31_step++;
    wmap_land_effect_31_step_02();
}

void wmap_land_effect_31_step_02(void)
{
void wmap_land_effect_31_step_02(void);

    if (g_wmap_sequence_busy == 0)
    {
        g_wmap_land_effect_31_step++;
        wmap_land_effect_31_step_03();
    }
}

void wmap_land_effect_31_step_03(void)
{
void wmap_land_effect_31_step_03(void);

    wmap_start_sequence(wmap_land_effect_31_run_timeline);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_31_step++;
    wmap_land_effect_31_step_04();
}

void wmap_land_effect_31_step_04(void)
{
void wmap_land_effect_31_step_04(void);

    if (g_wmap_sequence_busy == 0)
    {
        g_wmap_land_effect_31_step++;
        wmap_land_effect_31_step_05();
    }
}

void wmap_land_effect_31_step_05(void)
{
void wmap_land_effect_31_step_05(void);

    g_wmap_land_effect_31_step++;
}

s32 wmap_land_effect_31_run_timeline(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_land_effect_31_timeline_step = 1;
        g_wmap_land_effect_31_timeline_timer = 1;
        return 1;
    }

    if ((u32)g_wmap_land_effect_31_timeline_step >= 0x2C)
    {
        return 0;
    }

    D_800D748C[g_wmap_land_effect_31_timeline_step]();
    return 1;
}

void wmap_land_effect_31_timeline_reset(void)
{
    g_wmap_land_effect_31_timeline_step = 1;
    g_wmap_land_effect_31_timeline_timer = 1;
}

void wmap_land_effect_31_timeline_step_01(void)
{
    wmap_play_sound(0x34, 0x80);
    D_8013B208 = 1;
    wmap_start_sequence(wmap_land_effect_31_run_sequence_18);
    g_wmap_land_effect_31_timeline_timer = 0xF;
    g_wmap_land_effect_31_timeline_step++;
}

void wmap_land_effect_31_timeline_step_02(void)
{
    s32 timer;

    timer = g_wmap_land_effect_31_timeline_timer - 1;
    g_wmap_land_effect_31_timeline_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_timeline_step++;
    }
}

void wmap_land_effect_31_timeline_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_31_run_sequence_20);
    g_wmap_land_effect_31_timeline_timer = 0xF;
    g_wmap_land_effect_31_timeline_step++;
}

void wmap_land_effect_31_timeline_step_04(void)
{
    s32 timer;

    timer = g_wmap_land_effect_31_timeline_timer - 1;
    g_wmap_land_effect_31_timeline_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_timeline_step++;
    }
}

void wmap_land_effect_31_timeline_step_05(void)
{
    wmap_start_sequence(wmap_land_effect_31_run_sequence_2);
    g_wmap_land_effect_31_timeline_timer = 0x16;
    g_wmap_land_effect_31_timeline_step++;
}

void wmap_land_effect_31_timeline_step_06(void)
{
    s32 timer;

    timer = g_wmap_land_effect_31_timeline_timer - 1;
    g_wmap_land_effect_31_timeline_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_timeline_step++;
    }
}

void wmap_land_effect_31_timeline_step_07(void)
{
    wmap_start_sequence(wmap_land_effect_31_run_sequence_13);
    g_wmap_backdrop_target_level = 0xC;
    wmap_start_map_tint(0x704060);
    g_wmap_land_effect_31_timeline_timer = 0xB;
    g_wmap_land_effect_31_timeline_step++;
}

void wmap_land_effect_31_timeline_step_08(void)
{
    s32 timer;

    timer = g_wmap_land_effect_31_timeline_timer - 1;
    g_wmap_land_effect_31_timeline_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_timeline_step++;
    }
}

void wmap_land_effect_31_timeline_step_09(void)
{
    D_801ADAE0 = 1;
    g_wmap_land_effect_31_timeline_timer = 0x28;
    g_wmap_land_effect_31_timeline_step++;
}

void wmap_land_effect_31_timeline_step_10(void)
{
    s32 timer;

    timer = g_wmap_land_effect_31_timeline_timer - 1;
    g_wmap_land_effect_31_timeline_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_timeline_step++;
    }
}

void wmap_land_effect_31_timeline_step_11(void)
{
    wmap_start_sequence(wmap_land_effect_31_run_sequence_17);
    wmap_start_map_tint(0x352030);
    wmap_start_sequence(wmap_land_effect_31_run_sequence_10);
    g_wmap_land_effect_31_timeline_timer = 0xA;
    g_wmap_land_effect_31_timeline_step++;
}

void wmap_land_effect_31_timeline_step_12(void)
{
    s32 timer;

    timer = g_wmap_land_effect_31_timeline_timer - 1;
    g_wmap_land_effect_31_timeline_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_timeline_step++;
    }
}

void wmap_land_effect_31_timeline_step_13(void)
{
    g_wmap_screen_fade_mode = 2;
    D_8013B29C = 1;
    g_wmap_land_effect_31_timeline_timer = 4;
    g_wmap_land_effect_31_timeline_step++;
}

void wmap_land_effect_31_timeline_step_14(void)
{
    s32 timer;

    timer = g_wmap_land_effect_31_timeline_timer - 1;
    g_wmap_land_effect_31_timeline_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_timeline_step++;
    }
}

void wmap_land_effect_31_timeline_step_15(void)
{
    wmap_start_map_tint(0);
    g_wmap_backdrop_target_level = 1;
    g_wmap_land_effect_31_timeline_timer = 0xF;
    g_wmap_land_effect_31_timeline_step++;
}

void wmap_land_effect_31_timeline_step_16(void)
{
    s32 timer;

    timer = g_wmap_land_effect_31_timeline_timer - 1;
    g_wmap_land_effect_31_timeline_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_timeline_step++;
    }
}

void wmap_land_effect_31_timeline_step_18(void)
{
    s32 timer;

    timer = g_wmap_land_effect_31_timeline_timer - 1;
    g_wmap_land_effect_31_timeline_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_timeline_step++;
    }
}

void wmap_land_effect_31_timeline_step_20(void)
{
    s32 timer;

    timer = g_wmap_land_effect_31_timeline_timer - 1;
    g_wmap_land_effect_31_timeline_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_timeline_step++;
    }
}

void wmap_land_effect_31_timeline_step_21(void)
{
    wmap_install_callback(func_8006C0EC);
    D_80139210.field_00 = 0x132;
    D_80139210.field_02 = 0;
    D_80139210.field_04 = 0x1C2;
    D_80139968.field_00 = 0x26C;
    D_80139968.field_04 = 0;
    D_80139968.field_08 = 0;
    D_800DCEB8.field_00 = 0x132;
    D_800DCEB8.field_02 = 0;
    D_800DCEB8.field_04 = -0x1C2;
    D_80139200.field_00 = 0x26C;
    D_80139200.field_04 = 0;
    D_80139200.field_08 = 0;
    g_wmap_land_effect_31_timeline_timer = 0xA;
    g_wmap_land_effect_31_timeline_step++;
}

void wmap_land_effect_31_timeline_step_22(void)
{
    s32 timer;

    timer = g_wmap_land_effect_31_timeline_timer - 1;
    g_wmap_land_effect_31_timeline_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_timeline_step++;
    }
}

void wmap_land_effect_31_timeline_step_24(void)
{
    s32 timer;

    timer = g_wmap_land_effect_31_timeline_timer - 1;
    g_wmap_land_effect_31_timeline_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_timeline_step++;
    }
}

void wmap_land_effect_31_timeline_step_25(void)
{
    D_80139210.field_00 = 0;
    D_80139210.field_02 = 0;
    D_80139210.field_04 = 0;
    D_80139968.field_00 = 5;
    D_80139968.field_04 = 0;
    D_80139968.field_08 = 0;
    D_800DCEB8.field_00 = 0;
    D_800DCEB8.field_02 = 0;
    D_800DCEB8.field_04 = 0;
    D_80139200.field_00 = 0;
    D_80139200.field_04 = 0;
    D_80139200.field_08 = 0;
    g_wmap_land_effect_31_timeline_timer = 0x1E;
    g_wmap_land_effect_31_timeline_step++;
}

void wmap_land_effect_31_timeline_step_26(void)
{
    s32 timer;

    timer = g_wmap_land_effect_31_timeline_timer - 1;
    g_wmap_land_effect_31_timeline_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_timeline_step++;
    }
}

void wmap_land_effect_31_timeline_step_27(void)
{
    wmap_start_sequence(wmap_land_effect_31_run_sequence_3);
    g_wmap_land_effect_31_timeline_timer = 0xA0;
    g_wmap_land_effect_31_timeline_step++;
}

void wmap_land_effect_31_timeline_step_28(void)
{
    s32 timer;

    timer = g_wmap_land_effect_31_timeline_timer - 1;
    g_wmap_land_effect_31_timeline_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_timeline_step++;
    }
}

void wmap_land_effect_31_timeline_step_29(void)
{
    wmap_start_sequence(wmap_land_effect_31_run_sequence_8);
    g_wmap_land_effect_31_timeline_timer = 7;
    g_wmap_land_effect_31_timeline_step++;
}

void wmap_land_effect_31_timeline_step_30(void)
{
    s32 timer;

    timer = g_wmap_land_effect_31_timeline_timer - 1;
    g_wmap_land_effect_31_timeline_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_timeline_step++;
    }
}

void wmap_land_effect_31_timeline_step_31(void)
{
    wmap_start_sequence(wmap_land_effect_31_run_sequence_11);
    g_wmap_land_effect_31_timeline_timer = 0x14;
    g_wmap_land_effect_31_timeline_step++;
}

void wmap_land_effect_31_timeline_step_32(void)
{
    s32 timer;

    timer = g_wmap_land_effect_31_timeline_timer - 1;
    g_wmap_land_effect_31_timeline_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_timeline_step++;
    }
}

void wmap_land_effect_31_timeline_step_33(void)
{
    wmap_start_sequence(wmap_land_effect_31_run_sequence_1);
    g_wmap_land_effect_31_timeline_timer = 0x28;
    g_wmap_land_effect_31_timeline_step++;
}

void wmap_land_effect_31_timeline_step_34(void)
{
    s32 timer;

    timer = g_wmap_land_effect_31_timeline_timer - 1;
    g_wmap_land_effect_31_timeline_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_timeline_step++;
    }
}

void wmap_land_effect_31_timeline_step_35(void)
{
    wmap_start_sequence(wmap_land_effect_31_run_sequence_12);
    g_wmap_backdrop_target_level = 0;
    D_80182D74.field_00 = 0;
    D_80182D74.field_01 = 0;
    D_80182D74.field_02 = 0;
    D_80182D80.field_00 = 0;
    D_80182D80.field_01 = 0;
    D_80182D80.field_02 = 0;
    D_80182D8C.field_00 = 0;
    D_80182D8C.field_01 = 0;
    D_80182D8C.field_02 = 0;
    D_80182D94.field_00 = 0;
    D_80182D94.field_01 = 0;
    D_80182D94.field_02 = 0;
    g_wmap_land_effect_31_timeline_timer = 0x1E;
    g_wmap_land_effect_31_timeline_step++;
}

void wmap_land_effect_31_timeline_step_36(void)
{
    s32 timer;

    timer = g_wmap_land_effect_31_timeline_timer - 1;
    g_wmap_land_effect_31_timeline_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_timeline_step++;
    }
}

void wmap_land_effect_31_timeline_step_38(void)
{
    s32 timer;

    timer = g_wmap_land_effect_31_timeline_timer - 1;
    g_wmap_land_effect_31_timeline_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_timeline_step++;
    }
}

void wmap_land_effect_31_timeline_step_39(void)
{
    wmap_start_sequence(wmap_land_effect_31_run_sequence_9);
    wmap_start_map_tint(0x808080);
    g_wmap_backdrop_target_level = 0x10;
    D_80139244 = 0;
    g_wmap_land_effect_31_timeline_timer = 0x1C;
    D_80139290[D_8011D510][D_8011D530].value = D_8011D4FC | 0x100;
    g_wmap_land_effect_31_timeline_step++;
}

void wmap_land_effect_31_timeline_step_40(void)
{
    s32 timer;

    timer = g_wmap_land_effect_31_timeline_timer - 1;
    g_wmap_land_effect_31_timeline_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_timeline_step++;
    }
}

void wmap_land_effect_31_timeline_step_42(void)
{
    s32 timer;

    timer = g_wmap_land_effect_31_timeline_timer - 1;
    g_wmap_land_effect_31_timeline_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_timeline_step++;
    }
}

void wmap_land_effect_31_timeline_finish(void)
{
    g_wmap_sequence_busy = 0;
    g_wmap_land_effect_31_timeline_step++;
}

s32 wmap_land_effect_31_run_sequence_1(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_land_effect_31_sequence_1_step = 1;
        g_wmap_land_effect_31_sequence_1_timer = 1;
        return 1;
    }

    if ((u32)g_wmap_land_effect_31_sequence_1_step >= 6)
    {
        return 0;
    }

    D_800D753C[g_wmap_land_effect_31_sequence_1_step]();
    return 1;
}

void wmap_land_effect_31_sequence_1_reset(void)
{
    g_wmap_land_effect_31_sequence_1_step = 1;
    g_wmap_land_effect_31_sequence_1_timer = 1;
}

void wmap_land_effect_31_sequence_1_step_02(void)
{
    s32 timer;

    func_8006AEE0();
    func_8006A2FC(D_800D9D68, D_80139B88, 0x28, 0, 0x7F, 4, 0, (u8*)D_80139280 + 0x78);
    timer = g_wmap_land_effect_31_sequence_1_timer - 1;
    g_wmap_land_effect_31_sequence_1_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_1_step++;
    }
}

void wmap_land_effect_31_sequence_1_step_03(void)
{
    g_wmap_land_effect_31_sequence_1_timer = 0x20;
    D_80139280[35] = -1;
    g_wmap_land_effect_31_sequence_1_step++;
    wmap_land_effect_31_sequence_1_step_04();
}

void wmap_land_effect_31_sequence_1_step_04(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);

    s32 timer;

    func_8006AEE0();
    func_8006A2FC(D_800D9D68, D_80139B88, 0x28, 0, 0x7F, 4, 0, (u8*)D_80139280 + 0x78);
    timer = g_wmap_land_effect_31_sequence_1_timer - 1;
    g_wmap_land_effect_31_sequence_1_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_1_step++;
    }
}

void wmap_land_effect_31_sequence_1_end(void)
{
    g_wmap_land_effect_31_sequence_1_step++;
}

s32 wmap_land_effect_31_run_sequence_2(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_land_effect_31_sequence_2_step = 1;
        g_wmap_land_effect_31_sequence_2_timer = 1;
        return 1;
    }

    if ((u32)g_wmap_land_effect_31_sequence_2_step >= 4)
    {
        return 0;
    }

    D_800D7554[g_wmap_land_effect_31_sequence_2_step]();
    return 1;
}

void wmap_land_effect_31_sequence_2_reset(void)
{
    g_wmap_land_effect_31_sequence_2_step = 1;
    g_wmap_land_effect_31_sequence_2_timer = 1;
}

void wmap_land_effect_31_sequence_2_step_01(void)
{
    D_801399B4 = &D_8011F538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.sequence = 3;
    D_800D9344.previous_sequence = -1;
    D_800D9344.shade_step = 8;
    D_800D9344.target_shade = 0x81;
    D_800D9344.resource_index = 0;
    D_800D9344.shade = 1;
    g_wmap_land_effect_31_sequence_2_timer = 0x64;
    g_wmap_land_effect_31_sequence_2_step++;
    wmap_land_effect_31_sequence_2_step_02();
}

void wmap_land_effect_31_sequence_2_step_02(void)
{
void wmap_land_effect_31_sequence_2_step_02(void);

    s32 timer;

    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0xF, 2, 0);
    timer = g_wmap_land_effect_31_sequence_2_timer - 1;
    g_wmap_land_effect_31_sequence_2_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_2_step++;
    }
}

void wmap_land_effect_31_sequence_2_end(void)
{
    g_wmap_land_effect_31_sequence_2_step++;
}

s32 wmap_land_effect_31_run_sequence_3(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_land_effect_31_sequence_3_step = 1;
        g_wmap_land_effect_31_sequence_3_timer = 1;
        return 1;
    }

    if ((u32)g_wmap_land_effect_31_sequence_3_step >= 6)
    {
        return 0;
    }

    D_800D7564[g_wmap_land_effect_31_sequence_3_step]();
    return 1;
}

void wmap_land_effect_31_sequence_3_reset(void)
{
    g_wmap_land_effect_31_sequence_3_step = 1;
    g_wmap_land_effect_31_sequence_3_timer = 1;
}

void wmap_land_effect_31_sequence_3_step_01(void)
{
    D_801399BC = &D_8011D538;
    D_800D9370.previous_sequence = -1;
    D_800D9370.shade_step = 2;
    D_800D9370.target_shade = 0x81;
    D_800D9370.resource_index = 0;
    D_800D9370.scale_index = 0;
    D_800D9370.sequence = 0;
    D_800D9370.shade = 1;
    g_wmap_land_effect_31_sequence_3_timer = 0x10E;
    g_wmap_land_effect_31_sequence_3_step++;
    wmap_land_effect_31_sequence_3_step_02();
}

void wmap_land_effect_31_sequence_3_step_02(void)
{
void wmap_land_effect_31_sequence_3_step_02(void);

    s32 timer;

    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, g_wmap_focus_screen_position.packed, 0x10, 8, 1);
    timer = g_wmap_land_effect_31_sequence_3_timer - 1;
    g_wmap_land_effect_31_sequence_3_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_3_step++;
    }
}

void wmap_land_effect_31_sequence_3_step_03(void)
{
    D_800D9370.shade_step = 2;
    D_800D9370.target_shade = 0;
    D_80139268 = 0;
    g_wmap_land_effect_31_sequence_3_timer = 0x40;
    g_wmap_land_effect_31_sequence_3_step++;
    wmap_land_effect_31_sequence_3_step_04();
}

void wmap_land_effect_31_sequence_3_step_04(void)
{
void wmap_land_effect_31_sequence_3_step_04(void);

    s32 timer;

    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, g_wmap_focus_screen_position.packed, 0x10, 8, 1);
    D_800D9370.scale_index = D_80139268 >> 4;
    D_80139268 += 0x10;
    if (D_80139268 >= 0xF1)
    {
        D_80139268 = 0xF0;
    }
    timer = g_wmap_land_effect_31_sequence_3_timer - 1;
    g_wmap_land_effect_31_sequence_3_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_3_step++;
    }
}

void wmap_land_effect_31_sequence_3_end(void)
{
    g_wmap_land_effect_31_sequence_3_step++;
}

s32 wmap_land_effect_31_run_sequence_4(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_land_effect_31_sequence_4_step = 1;
        g_wmap_land_effect_31_sequence_4_timer = 1;
        return 1;
    }

    if ((u32)g_wmap_land_effect_31_sequence_4_step >= 6)
    {
        return 0;
    }

    D_800D757C[g_wmap_land_effect_31_sequence_4_step]();
    return 1;
}

void wmap_land_effect_31_sequence_4_reset(void)
{
    g_wmap_land_effect_31_sequence_4_step = 1;
    g_wmap_land_effect_31_sequence_4_timer = 1;
}

void wmap_land_effect_31_sequence_4_step_01(void)
{
    D_801B24B4 = 1;
    D_801B2490 = D_80139258;
    D_80139234 = 0;
    g_wmap_land_effect_31_sequence_4_timer = 0x92;
    g_wmap_land_effect_31_sequence_4_step++;
    wmap_land_effect_31_sequence_4_step_02();
}

void wmap_land_effect_31_sequence_4_step_03(void)
{
    g_wmap_land_effect_31_sequence_4_timer = 0x40;
    g_wmap_land_effect_31_sequence_4_step++;
    wmap_land_effect_31_sequence_4_step_04();
}

void wmap_land_effect_31_sequence_4_end(void)
{
    g_wmap_land_effect_31_sequence_4_step++;
}

s32 wmap_land_effect_31_run_sequence_5(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_land_effect_31_sequence_5_step = 1;
        g_wmap_land_effect_31_sequence_5_timer = 1;
        return 1;
    }

    if ((u32)g_wmap_land_effect_31_sequence_5_step >= 6)
    {
        return 0;
    }

    D_800D7594[g_wmap_land_effect_31_sequence_5_step]();
    return 1;
}

void wmap_land_effect_31_sequence_5_reset(void)
{
    g_wmap_land_effect_31_sequence_5_step = 1;
    g_wmap_land_effect_31_sequence_5_timer = 1;
}

void wmap_land_effect_31_sequence_5_step_02(void)
{
    s32 timer;

    func_8006ADD0(&g_wmap_camera_translation, &D_801B3120);
    func_8006C448((WmapState*)D_80139280);
    D_801B3120.vz += 0x18;
    timer = g_wmap_land_effect_31_sequence_5_timer - 1;
    g_wmap_land_effect_31_sequence_5_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_5_step++;
    }
}

void wmap_land_effect_31_sequence_5_step_03(void)
{
    s32 i;
    WmapConfigA* configs;

    configs = (WmapConfigA*)D_800D95D8;
    for (i = 0; i < 0x28; i++)
    {
        configs[i].field_22 = 0;
        configs[i].field_26 = 8;
    }
    g_wmap_land_effect_31_sequence_5_timer = 0x10;
    g_wmap_land_effect_31_sequence_5_step++;
    wmap_land_effect_31_sequence_5_step_04();
}

void wmap_land_effect_31_sequence_5_step_04(void)
{
    s32 timer;

    func_8006ADD0(&g_wmap_camera_translation, &D_801B3120);
    func_8006C448((WmapState*)D_80139280);
    D_801B3120.vz += 0x18;
    timer = g_wmap_land_effect_31_sequence_5_timer - 1;
    g_wmap_land_effect_31_sequence_5_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_5_step++;
    }
}

void wmap_land_effect_31_sequence_5_end(void)
{
    g_wmap_land_effect_31_sequence_5_step++;
}

s32 wmap_land_effect_31_run_sequence_6(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_land_effect_31_sequence_6_step = 1;
        g_wmap_land_effect_31_sequence_6_timer = 1;
        return 1;
    }

    if ((u32)g_wmap_land_effect_31_sequence_6_step >= 6)
    {
        return 0;
    }

    D_800D75AC[g_wmap_land_effect_31_sequence_6_step]();
    return 1;
}

void wmap_land_effect_31_sequence_6_reset(void)
{
    g_wmap_land_effect_31_sequence_6_step = 1;
    g_wmap_land_effect_31_sequence_6_timer = 1;
}

void wmap_land_effect_31_sequence_6_step_02(void)
{
    s32 timer;

    func_8006AEE0();
    func_8006A2FC(D_800D9D68, D_80139B88, 0x20, 0, 0x7F, 2, 0, (WmapState*)D_80139280 + 1);
    timer = g_wmap_land_effect_31_sequence_6_timer - 1;
    g_wmap_land_effect_31_sequence_6_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_6_step++;
    }
}

void wmap_land_effect_31_sequence_6_step_03(void)
{
    g_wmap_land_effect_31_sequence_6_timer = 0x20;
    D_80139280[25] = -1;
    g_wmap_land_effect_31_sequence_6_step++;
    wmap_land_effect_31_sequence_6_step_04();
}

void wmap_land_effect_31_sequence_6_step_04(void)
{
    s32 timer;

    func_8006AEE0();
    func_8006A2FC(D_800D9D68, D_80139B88, 0x20, 0, 0x7F, 2, 0, (WmapState*)D_80139280 + 1);
    timer = g_wmap_land_effect_31_sequence_6_timer - 1;
    g_wmap_land_effect_31_sequence_6_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_6_step++;
    }
}

void wmap_land_effect_31_sequence_6_end(void)
{
    g_wmap_land_effect_31_sequence_6_step++;
}

s32 wmap_land_effect_31_run_sequence_7(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_land_effect_31_sequence_7_step = 1;
        g_wmap_land_effect_31_sequence_7_timer = 1;
        return 1;
    }

    if ((u32)g_wmap_land_effect_31_sequence_7_step >= 4)
    {
        return 0;
    }

    D_800D75C4[g_wmap_land_effect_31_sequence_7_step]();
    return 1;
}

void wmap_land_effect_31_sequence_7_reset(void)
{
    g_wmap_land_effect_31_sequence_7_step = 1;
    g_wmap_land_effect_31_sequence_7_timer = 1;
}

void wmap_land_effect_31_sequence_7_step_01(void)
{
    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    g_wmap_land_effect_31_sequence_7_timer = 0x80;
    g_wmap_land_effect_31_sequence_7_step++;
    wmap_land_effect_31_sequence_7_step_02();
}

void wmap_land_effect_31_sequence_7_end(void)
{
    g_wmap_land_effect_31_sequence_7_step++;
}

s32 wmap_land_effect_31_run_sequence_8(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_land_effect_31_sequence_8_step = 1;
        g_wmap_land_effect_31_sequence_8_timer = 1;
        return 1;
    }

    if ((u32)g_wmap_land_effect_31_sequence_8_step >= 4)
    {
        return 0;
    }

    D_800D75D4[g_wmap_land_effect_31_sequence_8_step]();
    return 1;
}

void wmap_land_effect_31_sequence_8_reset(void)
{
    g_wmap_land_effect_31_sequence_8_step = 1;
    g_wmap_land_effect_31_sequence_8_timer = 1;
}

void wmap_land_effect_31_sequence_8_step_01(void)
{
    D_801B24A8 = D_80139258;
    D_801B2478 = g_wmap_camera_translation;
    D_80182DEC = 0x80;
    D_801B2478.vz = 0xAFC8;
    g_wmap_land_effect_31_sequence_8_timer = 0x20;
    g_wmap_land_effect_31_sequence_8_step++;
    wmap_land_effect_31_sequence_8_step_02();
}

void wmap_land_effect_31_sequence_8_end(void)
{
    g_wmap_land_effect_31_sequence_8_step++;
}

s32 wmap_land_effect_31_run_sequence_9(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_land_effect_31_sequence_9_step = 1;
        g_wmap_land_effect_31_sequence_9_timer = 1;
        return 1;
    }

    if ((u32)g_wmap_land_effect_31_sequence_9_step >= 4)
    {
        return 0;
    }

    D_800D75E4[g_wmap_land_effect_31_sequence_9_step]();
    return 1;
}

void wmap_land_effect_31_sequence_9_reset(void)
{
    g_wmap_land_effect_31_sequence_9_step = 1;
    g_wmap_land_effect_31_sequence_9_timer = 1;
}

void wmap_land_effect_31_sequence_9_step_01(void)
{
    D_8013B238 = D_80139258;
    D_80139870 = g_wmap_camera_translation;
    D_80182DF0 = 0x80;
    D_80139870.vz = 0xAFC8;
    g_wmap_land_effect_31_sequence_9_timer = 0x40;
    g_wmap_land_effect_31_sequence_9_step++;
    wmap_land_effect_31_sequence_9_step_02();
}

void wmap_land_effect_31_sequence_9_end(void)
{
    g_wmap_land_effect_31_sequence_9_step++;
}

s32 wmap_land_effect_31_run_sequence_10(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_land_effect_31_sequence_10_step = 1;
        g_wmap_land_effect_31_sequence_10_timer = 1;
        return 1;
    }

    if ((u32)g_wmap_land_effect_31_sequence_10_step >= 6)
    {
        return 0;
    }

    D_800D75F4[g_wmap_land_effect_31_sequence_10_step]();
    return 1;
}

void wmap_land_effect_31_sequence_10_reset(void)
{
    g_wmap_land_effect_31_sequence_10_step = 1;
    g_wmap_land_effect_31_sequence_10_timer = 1;
}

void wmap_land_effect_31_sequence_10_step_01(void)
{
    D_80182DE4 = 1;
    D_801B2498 = D_80139258;
    D_8013923C = 0;
    g_wmap_land_effect_31_sequence_10_timer = 0x5A;
    g_wmap_land_effect_31_sequence_10_step++;
    wmap_land_effect_31_sequence_10_step_02();
}

void wmap_land_effect_31_sequence_10_step_03(void)
{
    g_wmap_land_effect_31_sequence_10_timer = 8;
    g_wmap_land_effect_31_sequence_10_step++;
    wmap_land_effect_31_sequence_10_step_04();
}

void wmap_land_effect_31_sequence_10_end(void)
{
    g_wmap_land_effect_31_sequence_10_step++;
}

s32 wmap_land_effect_31_run_sequence_11(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_land_effect_31_sequence_11_step = 1;
        g_wmap_land_effect_31_sequence_11_timer = 1;
        return 1;
    }

    if ((u32)g_wmap_land_effect_31_sequence_11_step >= 6)
    {
        return 0;
    }

    D_800D760C[g_wmap_land_effect_31_sequence_11_step]();
    return 1;
}

void wmap_land_effect_31_sequence_11_reset(void)
{
    g_wmap_land_effect_31_sequence_11_step = 1;
    g_wmap_land_effect_31_sequence_11_timer = 1;
}

void wmap_land_effect_31_sequence_11_step_01(void)
{
    D_801B25E0 = 1;
    D_801B3118 = *(WmapPair*)&D_80139258;
    D_8013926C = 0;
    g_wmap_land_effect_31_sequence_11_timer = 0xB6;
    g_wmap_land_effect_31_sequence_11_step++;
    wmap_land_effect_31_sequence_11_step_02();
}

void wmap_land_effect_31_sequence_11_step_03(void)
{
    g_wmap_land_effect_31_sequence_11_timer = 1;
    g_wmap_land_effect_31_sequence_11_step++;
    wmap_land_effect_31_sequence_11_step_04();
}

void wmap_land_effect_31_sequence_11_end(void)
{
    g_wmap_land_effect_31_sequence_11_step++;
}

s32 wmap_land_effect_31_run_sequence_12(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_land_effect_31_sequence_12_step = 1;
        g_wmap_land_effect_31_sequence_12_timer = 1;
        return 1;
    }

    if ((u32)g_wmap_land_effect_31_sequence_12_step >= 6)
    {
        return 0;
    }

    D_800D7624[g_wmap_land_effect_31_sequence_12_step]();
    return 1;
}

void wmap_land_effect_31_sequence_12_reset(void)
{
    g_wmap_land_effect_31_sequence_12_step = 1;
    g_wmap_land_effect_31_sequence_12_timer = 1;
}

void wmap_land_effect_31_sequence_12_step_01(void)
{
    D_80182DF4 = 0x81;
    D_8013B240 = D_80139258;
    D_80139260 = 0;
    g_wmap_land_effect_31_sequence_12_timer = 0x68;
    g_wmap_land_effect_31_sequence_12_step++;
    wmap_land_effect_31_sequence_12_step_02();
}

void wmap_land_effect_31_sequence_12_step_03(void)
{
    g_wmap_land_effect_31_sequence_12_timer = 0x10;
    g_wmap_land_effect_31_sequence_12_step++;
    wmap_land_effect_31_sequence_12_step_04();
}

void wmap_land_effect_31_sequence_12_end(void)
{
    g_wmap_land_effect_31_sequence_12_step++;
}

s32 wmap_land_effect_31_run_sequence_13(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_land_effect_31_sequence_13_step = 1;
        g_wmap_land_effect_31_sequence_13_timer = 1;
        return 1;
    }

    if ((u32)g_wmap_land_effect_31_sequence_13_step >= 6)
    {
        return 0;
    }

    D_800D763C[g_wmap_land_effect_31_sequence_13_step]();
    return 1;
}

void wmap_land_effect_31_sequence_13_reset(void)
{
    g_wmap_land_effect_31_sequence_13_step = 1;
    g_wmap_land_effect_31_sequence_13_timer = 1;
}

void wmap_land_effect_31_sequence_13_step_01(void)
{
    D_801B25D8 = 1;
    D_801B2670 = D_80139258;
    D_80139264 = 0;
    g_wmap_land_effect_31_sequence_13_timer = 0x87;
    g_wmap_land_effect_31_sequence_13_step++;
    wmap_land_effect_31_sequence_13_step_02();
}

void wmap_land_effect_31_sequence_13_step_03(void)
{
    g_wmap_land_effect_31_sequence_13_timer = 8;
    g_wmap_land_effect_31_sequence_13_step++;
    wmap_land_effect_31_sequence_13_step_04();
}

void wmap_land_effect_31_sequence_13_end(void)
{
    g_wmap_land_effect_31_sequence_13_step++;
}

s32 wmap_land_effect_31_run_sequence_14(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_land_effect_31_sequence_14_step = 1;
        g_wmap_land_effect_31_sequence_14_timer = 1;
        return 1;
    }

    if ((u32)g_wmap_land_effect_31_sequence_14_step >= 6)
    {
        return 0;
    }

    D_800D7654[g_wmap_land_effect_31_sequence_14_step]();
    return 1;
}

void wmap_land_effect_31_sequence_14_reset(void)
{
    g_wmap_land_effect_31_sequence_14_step = 1;
    g_wmap_land_effect_31_sequence_14_timer = 1;
}

void wmap_land_effect_31_sequence_14_step_01(void)
{
    D_801B25DC = 1;
    D_801B2678 = D_80139258;
    D_80139268 = 0;
    g_wmap_land_effect_31_sequence_14_timer = 0xF0;
    g_wmap_land_effect_31_sequence_14_step++;
    wmap_land_effect_31_sequence_14_step_02();
}

void wmap_land_effect_31_sequence_14_step_03(void)
{
    g_wmap_land_effect_31_sequence_14_timer = 0x20;
    g_wmap_land_effect_31_sequence_14_step++;
    wmap_land_effect_31_sequence_14_step_04();
}

void wmap_land_effect_31_sequence_14_end(void)
{
    g_wmap_land_effect_31_sequence_14_step++;
}

s32 wmap_land_effect_31_run_sequence_15(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_land_effect_31_sequence_15_step = 1;
        g_wmap_land_effect_31_sequence_15_timer = 1;
        return 1;
    }

    if ((u32)g_wmap_land_effect_31_sequence_15_step >= 6)
    {
        return 0;
    }

    D_800D766C[g_wmap_land_effect_31_sequence_15_step]();
    return 1;
}

void wmap_land_effect_31_sequence_15_reset(void)
{
    g_wmap_land_effect_31_sequence_15_step = 1;
    g_wmap_land_effect_31_sequence_15_timer = 1;
}

void wmap_land_effect_31_sequence_15_step_01(void)
{
    D_80139240 = 0;
    g_wmap_land_effect_31_sequence_15_timer = 0xF8;
    g_wmap_land_effect_31_sequence_15_step++;
    wmap_land_effect_31_sequence_15_step_02();
}

void wmap_land_effect_31_sequence_15_step_02(void)
{
    s32 timer;

    func_8006AEE0();
    wmap_draw_model(D_8011CF30, 0, 0xB0, 0x35, 0x7800, 1, D_80139240, -0x32, 0, -1);
    D_80139240 += 8;
    if (D_80139240 >= 0x81)
    {
        D_80139240 = 0x80;
    }

    timer = g_wmap_land_effect_31_sequence_15_timer - 1;
    g_wmap_land_effect_31_sequence_15_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_15_step++;
    }
}

void wmap_land_effect_31_sequence_15_step_03(void)
{
    g_wmap_land_effect_31_sequence_15_timer = 0x10;
    g_wmap_land_effect_31_sequence_15_step++;
    wmap_land_effect_31_sequence_15_step_04();
}

void wmap_land_effect_31_sequence_15_step_04(void)
{
    s32 timer;

    func_8006AEE0();
    wmap_draw_model(D_8011CF30, 0, 0xB0, 0x35, 0x7800, 1, D_80139240, -0x32, 0, -1);
    D_80139240 -= 8;
    if (D_80139240 < 0)
    {
        D_80139240 = 0;
    }

    timer = g_wmap_land_effect_31_sequence_15_timer - 1;
    g_wmap_land_effect_31_sequence_15_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_15_step++;
    }
}

void wmap_land_effect_31_sequence_15_end(void)
{
    g_wmap_land_effect_31_sequence_15_step++;
}

s32 wmap_land_effect_31_run_sequence_16(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_land_effect_31_sequence_16_step = 1;
        g_wmap_land_effect_31_sequence_16_timer = 1;
        return 1;
    }

    if ((u32)g_wmap_land_effect_31_sequence_16_step >= 6)
    {
        return 0;
    }

    D_800D7684[g_wmap_land_effect_31_sequence_16_step]();
    return 1;
}

void wmap_land_effect_31_sequence_16_reset(void)
{
    g_wmap_land_effect_31_sequence_16_step = 1;
    g_wmap_land_effect_31_sequence_16_timer = 1;
}

void wmap_land_effect_31_sequence_16_step_01(void)
{
    D_8013924C = 1;
    g_wmap_land_effect_31_sequence_16_timer = 0x70;
    g_wmap_land_effect_31_sequence_16_step++;
    wmap_land_effect_31_sequence_16_step_02();
}

void wmap_land_effect_31_sequence_16_step_02(void)
{
    s32 timer;

    func_8006AEE0();
    wmap_draw_model(D_8011CF24, 0, 0xA, 0x35, 0x7840, 1, D_8013924C, 0, 0, -1);
    D_8013924C += 8;
    if (D_8013924C >= 0x82)
    {
        D_8013924C = 0x81;
    }

    timer = g_wmap_land_effect_31_sequence_16_timer - 1;
    g_wmap_land_effect_31_sequence_16_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_16_step++;
    }
}

void wmap_land_effect_31_sequence_16_step_03(void)
{
    g_wmap_land_effect_31_sequence_16_timer = 0x20;
    g_wmap_land_effect_31_sequence_16_step++;
    wmap_land_effect_31_sequence_16_step_04();
}

void wmap_land_effect_31_sequence_16_step_04(void)
{
    s32 timer;

    func_8006AEE0();
    wmap_draw_model(D_8011CF24, 0, 0xA, 0x35, 0x7840, 1, D_8013924C, 0, 0, -1);
    D_8013924C -= 4;
    if (D_8013924C < 0)
    {
        D_8013924C = 0;
    }

    timer = g_wmap_land_effect_31_sequence_16_timer - 1;
    g_wmap_land_effect_31_sequence_16_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_16_step++;
    }
}

void wmap_land_effect_31_sequence_16_end(void)
{
    g_wmap_land_effect_31_sequence_16_step++;
}

s32 wmap_land_effect_31_run_sequence_17(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_land_effect_31_sequence_17_step = 1;
        g_wmap_land_effect_31_sequence_17_timer = 1;
        return 1;
    }

    if ((u32)g_wmap_land_effect_31_sequence_17_step >= 4)
    {
        return 0;
    }

    D_800D769C[g_wmap_land_effect_31_sequence_17_step]();
    return 1;
}

void wmap_land_effect_31_sequence_17_reset(void)
{
    g_wmap_land_effect_31_sequence_17_step = 1;
    g_wmap_land_effect_31_sequence_17_timer = 1;
}

void wmap_land_effect_31_sequence_17_step_01(void)
{
    D_801399C4 = &D_8011D538;
    D_800D939C.scale_index = 0xF;
    D_800D939C.sequence = 1;
    D_800D939C.previous_sequence = -1;
    D_800D939C.shade_step = 4;
    D_800D939C.resource_index = 0;
    D_800D939C.target_shade = 0x61;
    D_800D939C.shade = 1;
    g_wmap_land_effect_31_sequence_17_timer = 0x64;
    g_wmap_land_effect_31_sequence_17_step++;
    wmap_land_effect_31_sequence_17_step_02();
}

void wmap_land_effect_31_sequence_17_step_02(void)
{
    s32 timer;
    WmapConfigA* config;

    config = &D_800D939C;
    wmap_step_actor_animation(config, &D_801399C0);
    wmap_draw_actor_sprite(config, g_wmap_focus_screen_position.packed, 0x2B, 8, 0);
    timer = g_wmap_land_effect_31_sequence_17_timer - 1;
    g_wmap_land_effect_31_sequence_17_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_17_step++;
    }
}

void wmap_land_effect_31_sequence_17_end(void)
{
    g_wmap_land_effect_31_sequence_17_step++;
}

s32 wmap_land_effect_31_run_sequence_18(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_land_effect_31_sequence_18_step = 1;
        g_wmap_land_effect_31_sequence_18_timer = 1;
        return 1;
    }

    if ((u32)g_wmap_land_effect_31_sequence_18_step >= 6)
    {
        return 0;
    }

    D_800D76AC[g_wmap_land_effect_31_sequence_18_step]();
    return 1;
}

void wmap_land_effect_31_sequence_18_reset(void)
{
    g_wmap_land_effect_31_sequence_18_step = 1;
    g_wmap_land_effect_31_sequence_18_timer = 1;
}

void wmap_land_effect_31_sequence_18_step_01(void)
{
    D_801399CC = &D_8011D538;
    D_800D93C8.scale_index = 0xF;
    D_800D93C8.sequence = 2;
    D_800D93C8.previous_sequence = -1;
    D_800D93C8.shade_step = 2;
    D_800D93C8.resource_index = 0;
    D_800D93C8.target_shade = 0x81;
    D_800D93C8.shade = 1;
    g_wmap_land_effect_31_sequence_18_timer = 0x6B;
    g_wmap_land_effect_31_sequence_18_step++;
    wmap_land_effect_31_sequence_18_step_02();
}

void wmap_land_effect_31_sequence_18_step_02(void)
{
    s32 timer;
    WmapConfigA* config;

    config = &D_800D93C8;
    wmap_step_actor_animation(config, &D_801399C8);
    wmap_draw_actor_sprite(config, g_wmap_focus_screen_position.packed, 0x10, 8, 0);
    timer = g_wmap_land_effect_31_sequence_18_timer - 1;
    g_wmap_land_effect_31_sequence_18_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_18_step++;
    }
}

void wmap_land_effect_31_sequence_18_step_03(void)
{
    D_800D93C8.shade_step = 8;
    D_800D93C8.target_shade = 0;
    g_wmap_land_effect_31_sequence_18_timer = 0x10;
    g_wmap_land_effect_31_sequence_18_step++;
    wmap_land_effect_31_sequence_18_step_04();
}

void wmap_land_effect_31_sequence_18_step_04(void)
{
    s32 timer;
    WmapConfigA* config;

    config = &D_800D93C8;
    wmap_step_actor_animation(config, &D_801399C8);
    wmap_draw_actor_sprite(config, g_wmap_focus_screen_position.packed, 0x10, 8, 0);
    timer = g_wmap_land_effect_31_sequence_18_timer - 1;
    g_wmap_land_effect_31_sequence_18_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_18_step++;
    }
}

void wmap_land_effect_31_sequence_18_end(void)
{
    g_wmap_land_effect_31_sequence_18_step++;
}

s32 wmap_land_effect_31_run_sequence_19(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_land_effect_31_sequence_19_step = 1;
        g_wmap_land_effect_31_sequence_19_timer = 1;
        return 1;
    }

    if ((u32)g_wmap_land_effect_31_sequence_19_step >= 6)
    {
        return 0;
    }

    D_800D76C4[g_wmap_land_effect_31_sequence_19_step]();
    return 1;
}

void wmap_land_effect_31_sequence_19_reset(void)
{
    g_wmap_land_effect_31_sequence_19_step = 1;
    g_wmap_land_effect_31_sequence_19_timer = 1;
}

void wmap_land_effect_31_sequence_19_step_01(void)
{
    D_801399D4 = &D_8011D538;
    D_800D93F4.scale_index = 0xF;
    D_800D93F4.sequence = 3;
    D_800D93F4.previous_sequence = -1;
    D_800D93F4.shade_step = 2;
    D_800D93F4.target_shade = 0x81;
    D_800D93F4.resource_index = 0;
    D_800D93F4.shade = 1;
    g_wmap_land_effect_31_sequence_19_timer = 0xB4;
    g_wmap_land_effect_31_sequence_19_step++;
    wmap_land_effect_31_sequence_19_step_02();
}

void wmap_land_effect_31_sequence_19_step_02(void)
{
    s32 timer;
    WmapConfigA* config;

    config = &D_800D93F4;
    wmap_step_actor_animation(config, &D_801399D0);
    wmap_draw_actor_sprite(config, g_wmap_focus_screen_position.packed, 0x10, 8, 0);
    timer = g_wmap_land_effect_31_sequence_19_timer - 1;
    g_wmap_land_effect_31_sequence_19_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_19_step++;
    }
}

void wmap_land_effect_31_sequence_19_step_03(void)
{
    D_800D93F4.shade_step = 4;
    D_800D93F4.target_shade = 0;
    g_wmap_land_effect_31_sequence_19_timer = 0x20;
    g_wmap_land_effect_31_sequence_19_step++;
    wmap_land_effect_31_sequence_19_step_04();
}

void wmap_land_effect_31_sequence_19_step_04(void)
{
    s32 timer;
    WmapConfigA* config;

    config = &D_800D93F4;
    wmap_step_actor_animation(config, &D_801399D0);
    wmap_draw_actor_sprite(config, g_wmap_focus_screen_position.packed, 0x10, 8, 0);
    timer = g_wmap_land_effect_31_sequence_19_timer - 1;
    g_wmap_land_effect_31_sequence_19_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_19_step++;
    }
}

void wmap_land_effect_31_sequence_19_end(void)
{
    g_wmap_land_effect_31_sequence_19_step++;
}

s32 wmap_land_effect_31_run_sequence_20(s32 reset)
{
    if (reset != 0)
    {
        g_wmap_land_effect_31_sequence_20_step = 1;
        g_wmap_land_effect_31_sequence_20_timer = 1;
        return 1;
    }

    if ((u32)g_wmap_land_effect_31_sequence_20_step >= 6)
    {
        return 0;
    }

    D_800D76DC[g_wmap_land_effect_31_sequence_20_step]();
    return 1;
}

void wmap_land_effect_31_sequence_20_reset(void)
{
    g_wmap_land_effect_31_sequence_20_step = 1;
    g_wmap_land_effect_31_sequence_20_timer = 1;
}

void wmap_land_effect_31_sequence_20_step_02(void)
{
    s32 timer;

    func_8006C448(&D_80139280[40]);
    timer = g_wmap_land_effect_31_sequence_20_timer - 1;
    g_wmap_land_effect_31_sequence_20_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_20_step++;
    }
}

void wmap_land_effect_31_sequence_20_step_03(void)
{
    s32 i;

    for (i = 0; i < 0x28; i++)
    {
        D_800DB4C8[i].field_22 = 0;
        D_800DB4C8[i].field_26 = 0x20;
    }
    g_wmap_land_effect_31_sequence_20_timer = 4;
    g_wmap_land_effect_31_sequence_20_step++;
    wmap_land_effect_31_sequence_20_step_04();
}

void wmap_land_effect_31_sequence_20_step_04(void)
{
    s32 timer;

    func_8006C448(&D_80139280[40]);
    timer = g_wmap_land_effect_31_sequence_20_timer - 1;
    g_wmap_land_effect_31_sequence_20_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_20_step++;
    }
}

void wmap_land_effect_31_sequence_20_end(void)
{
    g_wmap_land_effect_31_sequence_20_step++;
}
