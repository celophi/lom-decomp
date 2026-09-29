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

void func_800BA108(void);
void func_800BA1BC(void);
void func_800B9F1C(void);
void func_800B9FE4(void);
void func_800B8794(void);
void func_800B8988(void);
void func_800B95CC(void);
void func_800B96B8(void);
void func_800B9850(void);
void func_800B9938(void);
void func_800B9B08(void);
void func_800B9D74(void);
s32 func_800B7438(s32 reset);
s32 func_800B7E7C(s32 reset);
s32 func_800B8084(s32 reset);
s32 func_800B822C(s32 reset);
s32 func_800B8B80(s32 reset);
s32 func_800B8CD8(s32 reset);
s32 func_800B8E30(s32 reset);
s32 func_800B8F8C(s32 reset);
s32 func_800B90E8(s32 reset);
s32 func_800B9244(s32 reset);
s32 func_800B99F8(s32 reset);
s32 func_800B9B9C(s32 reset);
s32 func_800BA078(s32 reset);
s32 func_800B84D0(s32 reset);
s32 func_800B862C(s32 reset);
s32 func_800B8824(s32 reset);
s32 func_800B8A2C(s32 reset);
s32 func_800B93A0(s32 reset);
s32 func_800B94FC(s32 reset);
s32 func_800B977C(s32 reset);
s32 func_800B9E08(s32 reset);
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);
void func_800B7F0C(void);
void func_800B86BC(void);
void func_800B88B4(void);

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
extern s32 D_8013B20C;
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
extern s32 D_801B3048;
extern s32 D_801B304C;
extern s32 D_801B3050;
extern s32 D_801B3054;
extern s32 D_801B3058;
extern s32 D_801B305C;
extern s32 D_801B3060;
extern s32 D_801B3064;
extern s32 D_801B3068;
extern s32 D_801B306C;
extern s32 D_801B3070;
extern s32 D_801B3074;
extern s32 D_801B3078;
extern s32 D_801B307C;
extern s32 D_801B3080;
extern s32 D_801B3084;
extern s32 D_801B3088;
extern s32 D_801B308C;
extern s32 D_801B3090;
extern s32 D_801B3094;
extern s32 D_801B3098;
extern s32 D_801B309C;
extern s32 D_801B30A0;
extern s32 D_801B30A4;
extern s32 D_801B30A8;
extern s32 D_801B30AC;
extern s32 D_801B30B0;
extern s32 D_801B30B4;
extern s32 D_801B30B8;
extern s32 D_801B30BC;
extern s32 D_801B30C0;
extern s32 D_801B30C4;
extern s32 D_801B30C8;
extern s32 D_801B30CC;
extern s32 D_801B30D0;
extern s32 D_801B30D4;
extern s32 D_801B30D8;
extern s32 D_801B30DC;
extern s32 D_801B30E0;
extern s32 D_801B30E4;
extern s32 D_801B30E8;
extern s32 D_801B30EC;
extern s32 D_801B30F0;
extern s32 D_801B30F4;
extern s32 D_801B0FD0;
extern void func_800B619C(void);
extern void func_800B6284(void);
extern void func_800B6558(void);
extern void func_800B6644(void);
extern void func_800B6730(void);
extern void func_800B681C(void);
extern void func_800B6918(void);
extern void func_800B6A10(void);
extern void func_800B6B0C(void);
extern void func_800B6C04(void);
extern void func_800B6D00(void);
extern void func_800B6DF8(void);
extern void func_800B6EF4(void);
extern s32* D_8011CF40;
extern s32 func_800B7438(s32);
extern s32 func_800B7E7C(s32);
extern s32 func_800B8084(s32);
extern s32 func_800B822C(s32);
extern s32 func_800B84D0(s32);
extern s32 func_800B862C(s32);
extern void func_800B8794(void);
extern s32 func_800B8824(s32);
extern void func_800B8988(void);
extern s32 func_800B8A2C(s32);
extern s32 func_800B8B80(s32);
extern s32 func_800B8CD8(s32);
extern s32 func_800B8E30(s32);
extern s32 func_800B8F8C(s32);
extern s32 func_800B90E8(s32);
extern s32 func_800B9244(s32);
extern s32 func_800B93A0(s32);
extern s32 func_800B94FC(s32);
extern void func_800B95CC(void);
extern void func_800B96B8(void);
extern s32 func_800B977C(s32);
extern void func_800B9850(void);
extern void func_800B9938(void);
extern s32 func_800B99F8(s32);
extern void func_800B9B08(void);
extern s32 func_800B9B9C(s32);
extern void func_800B9CAC(void);
extern void func_800B9D74(void);
extern s32 func_800B9E08(s32);
extern void func_800B9F1C();
extern void func_800B9FE4();
extern s32 func_800BA078(s32);
extern void func_800BA108(void);
extern void func_800BA1BC();

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

void func_800B5D0C(void)
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
    D_801B3054 = 0x38;
    D_801B3050++;
}

void func_800B5D98(void)
{
    D_80139244 = 1;
    wmap_start_sequence(func_800B8A2C);
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
    D_801B3054 = 0xF;
    D_801B3050++;
}

void func_800B5E3C(void)
{
    wmap_start_sequence(func_800B94FC);
    wmap_start_sequence(func_800B977C);
    wmap_start_sequence(func_800B93A0);
    wmap_start_sequence(func_800B862C);
    wmap_start_sequence(func_800B84D0);
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
    D_801B3054 = 0x14;
    D_801B3050++;
}

void func_800B5F10(void)
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
    wmap_start_sequence(func_800B9E08);
    D_801B3054 = 0x58;
    D_801B3050++;
}

void func_800B5FD4(void)
{
    wmap_start_sequence(func_800B8824);
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
    D_801B3054 = 0x78;
    D_801B3050++;
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_800B60AC(void)
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
    D_801B305C = 40;
    D_801B3058++;
    func_800B7F0C();
}

void func_800B619C(void)
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
    timer = D_801B3074 - 1;
    D_801B3074 = timer;
    if (timer == 0)
    {
        D_801B3070++;
    }
}

void func_800B6284(void)
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
    timer = D_801B3074 - 1;
    D_801B3074 = timer;
    position->field_04 += 0x40;
    if (timer == 0)
    {
        D_801B3070++;
    }
}

/** @brief Configure the effect and its resource slots, then advance the sequence. */
void func_800B6368(void)
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
    D_801B307C = 204;
    D_801B3078++;
    func_800B86BC();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_800B6468(void)
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
    D_801B3084 = 32;
    D_801B3080++;
    func_800B88B4();
}

void func_800B6558(void)
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
    timer = D_801B308C - 1;
    D_801B308C = timer;
    if (timer == 0)
    {
        D_801B3088++;
    }
}

void func_800B6644(void)
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
    timer = D_801B3094 - 1;
    D_801B3094 = timer;
    if (timer == 0)
    {
        D_801B3090++;
    }
}

void func_800B6730(void)
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
    timer = D_801B309C - 1;
    D_801B309C = timer;
    if (timer == 0)
    {
        D_801B3098++;
    }
}

void func_800B681C(void)
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
    timer = D_801B30A4 - 1;
    D_801B30A4 = timer;
    if (timer == 0)
    {
        D_801B30A0++;
    }
}

void func_800B6918(void)
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
    timer = D_801B30A4 - 1;
    D_801B30A4 = timer;
    D_801B2498.vz += 0x10;
    if (timer == 0)
    {
        D_801B30A0++;
    }
}

void func_800B6A10(void)
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
    timer = D_801B30AC - 1;
    D_801B30AC = timer;
    if (timer == 0)
    {
        D_801B30A8++;
    }
}

void func_800B6B0C(void)
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
    timer = D_801B30AC - 1;
    D_801B30AC = timer;
    ((u16*)&D_801B3118)[2] += 0x10;
    if (timer == 0)
    {
        D_801B30A8++;
    }
}

void func_800B6C04(void)
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
    timer = D_801B30B4 - 1;
    D_801B30B4 = timer;
    if (timer == 0)
    {
        D_801B30B0++;
    }
}

void func_800B6D00(void)
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
    timer = D_801B30B4 - 1;
    D_801B30B4 = timer;
    D_8013B240.vz += 0x90;
    if (timer == 0)
    {
        D_801B30B0++;
    }
}

void func_800B6DF8(void)
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
    timer = D_801B30BC - 1;
    D_801B30BC = timer;
    if (timer == 0)
    {
        D_801B30B8++;
    }
}

void func_800B6EF4(void)
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
    timer = D_801B30BC - 1;
    D_801B30BC = timer;
    D_801B2670.vz += 0x60;
    if (timer == 0)
    {
        D_801B30B8++;
    }
}

void func_800B6FEC(void)
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
    timer = D_801B30C4 - 1;
    D_801B30C4 = timer;
    if (timer == 0)
    {
        D_801B30C0++;
    }
}

void func_800B70CC(void)
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
    timer = D_801B30C4 - 1;
    D_801B30C4 = timer;
    if (timer == 0)
    {
        D_801B30C0++;
    }
}

void func_800B71A8(void)
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

    D_801B30F4 = 0x64;
    D_801B30F0++;
    func_800BA108();
}

s32 func_800B7290(s32 reset)
{
    if (reset != 0)
    {
        D_801B3048 = 1;
        D_801B304C = 1;
        return 1;
    }

    if ((u32)D_801B3048 >= 6)
    {
        return 0;
    }

    D_800D7474[D_801B3048]();
    return 1;
}

void func_800B7308(void)
{
    D_801B3048 = 1;
    D_801B304C = 1;
}

void func_800B7320(void)
{
    wmap_start_sequence(wmap_run_land_focus);
    D_8013B20C = 1;
    D_801B3048++;
    func_800B7364();
}

void func_800B7364(void)
{
void func_800B7364(void);

    if (D_8013B20C == 0)
    {
        D_801B3048++;
        func_800B73A0();
    }
}

void func_800B73A0(void)
{
void func_800B73A0(void);

    wmap_start_sequence(func_800B7438);
    D_8013B20C = 1;
    D_801B3048++;
    func_800B73E4();
}

void func_800B73E4(void)
{
void func_800B73E4(void);

    if (D_8013B20C == 0)
    {
        D_801B3048++;
        func_800B7420();
    }
}

void func_800B7420(void)
{
void func_800B7420(void);

    D_801B3048++;
}

s32 func_800B7438(s32 reset)
{
    if (reset != 0)
    {
        D_801B3050 = 1;
        D_801B3054 = 1;
        return 1;
    }

    if ((u32)D_801B3050 >= 0x2C)
    {
        return 0;
    }

    D_800D748C[D_801B3050]();
    return 1;
}

void func_800B74B0(void)
{
    D_801B3050 = 1;
    D_801B3054 = 1;
}

void func_800B74C8(void)
{
    wmap_play_sound(0x34, 0x80);
    D_8013B208 = 1;
    wmap_start_sequence(func_800B9B9C);
    D_801B3054 = 0xF;
    D_801B3050++;
}

void func_800B751C(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B7550(void)
{
    wmap_start_sequence(func_800BA078);
    D_801B3054 = 0xF;
    D_801B3050++;
}

void func_800B758C(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B75C0(void)
{
    wmap_start_sequence(func_800B8084);
    D_801B3054 = 0x16;
    D_801B3050++;
}

void func_800B75FC(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B7630(void)
{
    wmap_start_sequence(func_800B9244);
    g_wmap_backdrop_target_level = 0xC;
    wmap_start_map_tint(0x704060);
    D_801B3054 = 0xB;
    D_801B3050++;
}

void func_800B7684(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B76B8(void)
{
    D_801ADAE0 = 1;
    D_801B3054 = 0x28;
    D_801B3050++;
}

void func_800B76E4(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B7718(void)
{
    wmap_start_sequence(func_800B99F8);
    wmap_start_map_tint(0x352030);
    wmap_start_sequence(func_800B8E30);
    D_801B3054 = 0xA;
    D_801B3050++;
}

void func_800B776C(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B77A0(void)
{
    g_wmap_screen_fade_mode = 2;
    D_8013B29C = 1;
    D_801B3054 = 4;
    D_801B3050++;
}

void func_800B77D8(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B780C(void)
{
    wmap_start_map_tint(0);
    g_wmap_backdrop_target_level = 1;
    D_801B3054 = 0xF;
    D_801B3050++;
}

void func_800B7850(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B7884(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B78B8(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B78EC(void)
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
    D_801B3054 = 0xA;
    D_801B3050++;
}

void func_800B7988(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B79BC(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B79F0(void)
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
    D_801B3054 = 0x1E;
    D_801B3050++;
}

void func_800B7A64(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B7A98(void)
{
    wmap_start_sequence(func_800B822C);
    D_801B3054 = 0xA0;
    D_801B3050++;
}

void func_800B7AD4(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B7B08(void)
{
    wmap_start_sequence(func_800B8B80);
    D_801B3054 = 7;
    D_801B3050++;
}

void func_800B7B44(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B7B78(void)
{
    wmap_start_sequence(func_800B8F8C);
    D_801B3054 = 0x14;
    D_801B3050++;
}

void func_800B7BB4(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B7BE8(void)
{
    wmap_start_sequence(func_800B7E7C);
    D_801B3054 = 0x28;
    D_801B3050++;
}

void func_800B7C24(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B7C58(void)
{
    wmap_start_sequence(func_800B90E8);
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
    D_801B3054 = 0x1E;
    D_801B3050++;
}

void func_800B7CEC(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B7D20(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B7D54(void)
{
    wmap_start_sequence(func_800B8CD8);
    wmap_start_map_tint(0x808080);
    g_wmap_backdrop_target_level = 0x10;
    D_80139244 = 0;
    D_801B3054 = 0x1C;
    D_80139290[D_8011D510][D_8011D530].value = D_8011D4FC | 0x100;
    D_801B3050++;
}

void func_800B7DF8(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B7E2C(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B7E60(void)
{
    D_8013B20C = 0;
    D_801B3050++;
}

s32 func_800B7E7C(s32 reset)
{
    if (reset != 0)
    {
        D_801B3058 = 1;
        D_801B305C = 1;
        return 1;
    }

    if ((u32)D_801B3058 >= 6)
    {
        return 0;
    }

    D_800D753C[D_801B3058]();
    return 1;
}

void func_800B7EF4(void)
{
    D_801B3058 = 1;
    D_801B305C = 1;
}

void func_800B7F0C(void)
{
    s32 timer;

    func_8006AEE0();
    func_8006A2FC(D_800D9D68, D_80139B88, 0x28, 0, 0x7F, 4, 0, (u8*)D_80139280 + 0x78);
    timer = D_801B305C - 1;
    D_801B305C = timer;
    if (timer == 0)
    {
        D_801B3058++;
    }
}

void func_800B7F98(void)
{
    D_801B305C = 0x20;
    D_80139280[35] = -1;
    D_801B3058++;
    func_800B7FE0();
}

void func_800B7FE0(void)
{
void func_800B7FE0(void);

    s32 timer;

    func_8006AEE0();
    func_8006A2FC(D_800D9D68, D_80139B88, 0x28, 0, 0x7F, 4, 0, (u8*)D_80139280 + 0x78);
    timer = D_801B305C - 1;
    D_801B305C = timer;
    if (timer == 0)
    {
        D_801B3058++;
    }
}

void func_800B806C(void)
{
    D_801B3058++;
}

s32 func_800B8084(s32 reset)
{
    if (reset != 0)
    {
        D_801B3060 = 1;
        D_801B3064 = 1;
        return 1;
    }

    if ((u32)D_801B3060 >= 4)
    {
        return 0;
    }

    D_800D7554[D_801B3060]();
    return 1;
}

void func_800B80FC(void)
{
    D_801B3060 = 1;
    D_801B3064 = 1;
}

void func_800B8114(void)
{
    D_801399B4 = &D_8011F538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.sequence = 3;
    D_800D9344.previous_sequence = -1;
    D_800D9344.shade_step = 8;
    D_800D9344.target_shade = 0x81;
    D_800D9344.unknown_02 = 0;
    D_800D9344.shade = 1;
    D_801B3064 = 0x64;
    D_801B3060++;
    func_800B8198();
}

void func_800B8198(void)
{
void func_800B8198(void);

    s32 timer;

    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0xF, 2, 0);
    timer = D_801B3064 - 1;
    D_801B3064 = timer;
    if (timer == 0)
    {
        D_801B3060++;
    }
}

void func_800B8214(void)
{
    D_801B3060++;
}

s32 func_800B822C(s32 reset)
{
    if (reset != 0)
    {
        D_801B3068 = 1;
        D_801B306C = 1;
        return 1;
    }

    if ((u32)D_801B3068 >= 6)
    {
        return 0;
    }

    D_800D7564[D_801B3068]();
    return 1;
}

void func_800B82A4(void)
{
    D_801B3068 = 1;
    D_801B306C = 1;
}

void func_800B82BC(void)
{
    D_801399BC = &D_8011D538;
    D_800D9370.previous_sequence = -1;
    D_800D9370.shade_step = 2;
    D_800D9370.target_shade = 0x81;
    D_800D9370.unknown_02 = 0;
    D_800D9370.scale_index = 0;
    D_800D9370.sequence = 0;
    D_800D9370.shade = 1;
    D_801B306C = 0x10E;
    D_801B3068++;
    func_800B8338();
}

void func_800B8338(void)
{
void func_800B8338(void);

    s32 timer;

    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, g_wmap_focus_screen_position.packed, 0x10, 8, 1);
    timer = D_801B306C - 1;
    D_801B306C = timer;
    if (timer == 0)
    {
        D_801B3068++;
    }
}

void func_800B83B8(void)
{
    D_800D9370.shade_step = 2;
    D_800D9370.target_shade = 0;
    D_80139268 = 0;
    D_801B306C = 0x40;
    D_801B3068++;
    func_800B840C();
}

void func_800B840C(void)
{
void func_800B840C(void);

    s32 timer;

    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, g_wmap_focus_screen_position.packed, 0x10, 8, 1);
    D_800D9370.scale_index = D_80139268 >> 4;
    D_80139268 += 0x10;
    if (D_80139268 >= 0xF1)
    {
        D_80139268 = 0xF0;
    }
    timer = D_801B306C - 1;
    D_801B306C = timer;
    if (timer == 0)
    {
        D_801B3068++;
    }
}

void func_800B84B8(void)
{
    D_801B3068++;
}

s32 func_800B84D0(s32 reset)
{
    if (reset != 0)
    {
        D_801B3070 = 1;
        D_801B3074 = 1;
        return 1;
    }

    if ((u32)D_801B3070 >= 6)
    {
        return 0;
    }

    D_800D757C[D_801B3070]();
    return 1;
}

void func_800B8548(void)
{
    D_801B3070 = 1;
    D_801B3074 = 1;
}

void func_800B8560(void)
{
    D_801B24B4 = 1;
    D_801B2490 = D_80139258;
    D_80139234 = 0;
    D_801B3074 = 0x92;
    D_801B3070++;
    func_800B619C();
}

void func_800B85DC(void)
{
    D_801B3074 = 0x40;
    D_801B3070++;
    func_800B6284();
}

void func_800B8614(void)
{
    D_801B3070++;
}

s32 func_800B862C(s32 reset)
{
    if (reset != 0)
    {
        D_801B3078 = 1;
        D_801B307C = 1;
        return 1;
    }

    if ((u32)D_801B3078 >= 6)
    {
        return 0;
    }

    D_800D7594[D_801B3078]();
    return 1;
}

void func_800B86A4(void)
{
    D_801B3078 = 1;
    D_801B307C = 1;
}

void func_800B86BC(void)
{
    s32 timer;

    func_8006ADD0(&g_wmap_camera_translation, &D_801B3120);
    func_8006C448((WmapState*)D_80139280);
    D_801B3120.vz += 0x18;
    timer = D_801B307C - 1;
    D_801B307C = timer;
    if (timer == 0)
    {
        D_801B3078++;
    }
}

void func_800B8734(void)
{
    s32 i;
    WmapConfigA* configs;

    configs = (WmapConfigA*)D_800D95D8;
    for (i = 0; i < 0x28; i++)
    {
        configs[i].field_22 = 0;
        configs[i].field_26 = 8;
    }
    D_801B307C = 0x10;
    D_801B3078++;
    func_800B8794();
}

void func_800B8794(void)
{
    s32 timer;

    func_8006ADD0(&g_wmap_camera_translation, &D_801B3120);
    func_8006C448((WmapState*)D_80139280);
    D_801B3120.vz += 0x18;
    timer = D_801B307C - 1;
    D_801B307C = timer;
    if (timer == 0)
    {
        D_801B3078++;
    }
}

void func_800B880C(void)
{
    D_801B3078++;
}

s32 func_800B8824(s32 reset)
{
    if (reset != 0)
    {
        D_801B3080 = 1;
        D_801B3084 = 1;
        return 1;
    }

    if ((u32)D_801B3080 >= 6)
    {
        return 0;
    }

    D_800D75AC[D_801B3080]();
    return 1;
}

void func_800B889C(void)
{
    D_801B3080 = 1;
    D_801B3084 = 1;
}

void func_800B88B4(void)
{
    s32 timer;

    func_8006AEE0();
    func_8006A2FC(D_800D9D68, D_80139B88, 0x20, 0, 0x7F, 2, 0, (WmapState*)D_80139280 + 1);
    timer = D_801B3084 - 1;
    D_801B3084 = timer;
    if (timer == 0)
    {
        D_801B3080++;
    }
}

void func_800B8940(void)
{
    D_801B3084 = 0x20;
    D_80139280[25] = -1;
    D_801B3080++;
    func_800B8988();
}

void func_800B8988(void)
{
    s32 timer;

    func_8006AEE0();
    func_8006A2FC(D_800D9D68, D_80139B88, 0x20, 0, 0x7F, 2, 0, (WmapState*)D_80139280 + 1);
    timer = D_801B3084 - 1;
    D_801B3084 = timer;
    if (timer == 0)
    {
        D_801B3080++;
    }
}

void func_800B8A14(void)
{
    D_801B3080++;
}

s32 func_800B8A2C(s32 reset)
{
    if (reset != 0)
    {
        D_801B3088 = 1;
        D_801B308C = 1;
        return 1;
    }

    if ((u32)D_801B3088 >= 4)
    {
        return 0;
    }

    D_800D75C4[D_801B3088]();
    return 1;
}

void func_800B8AA4(void)
{
    D_801B3088 = 1;
    D_801B308C = 1;
}

void func_800B8ABC(void)
{
    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    D_801B308C = 0x80;
    D_801B3088++;
    func_800B6558();
}

void func_800B8B68(void)
{
    D_801B3088++;
}

s32 func_800B8B80(s32 reset)
{
    if (reset != 0)
    {
        D_801B3090 = 1;
        D_801B3094 = 1;
        return 1;
    }

    if ((u32)D_801B3090 >= 4)
    {
        return 0;
    }

    D_800D75D4[D_801B3090]();
    return 1;
}

void func_800B8BF8(void)
{
    D_801B3090 = 1;
    D_801B3094 = 1;
}

void func_800B8C10(void)
{
    D_801B24A8 = D_80139258;
    D_801B2478 = g_wmap_camera_translation;
    D_80182DEC = 0x80;
    D_801B2478.vz = 0xAFC8;
    D_801B3094 = 0x20;
    D_801B3090++;
    func_800B6644();
}

void func_800B8CC0(void)
{
    D_801B3090++;
}

s32 func_800B8CD8(s32 reset)
{
    if (reset != 0)
    {
        D_801B3098 = 1;
        D_801B309C = 1;
        return 1;
    }

    if ((u32)D_801B3098 >= 4)
    {
        return 0;
    }

    D_800D75E4[D_801B3098]();
    return 1;
}

void func_800B8D50(void)
{
    D_801B3098 = 1;
    D_801B309C = 1;
}

void func_800B8D68(void)
{
    D_8013B238 = D_80139258;
    D_80139870 = g_wmap_camera_translation;
    D_80182DF0 = 0x80;
    D_80139870.vz = 0xAFC8;
    D_801B309C = 0x40;
    D_801B3098++;
    func_800B6730();
}

void func_800B8E18(void)
{
    D_801B3098++;
}

s32 func_800B8E30(s32 reset)
{
    if (reset != 0)
    {
        D_801B30A0 = 1;
        D_801B30A4 = 1;
        return 1;
    }

    if ((u32)D_801B30A0 >= 6)
    {
        return 0;
    }

    D_800D75F4[D_801B30A0]();
    return 1;
}

void func_800B8EA8(void)
{
    D_801B30A0 = 1;
    D_801B30A4 = 1;
}

void func_800B8EC0(void)
{
    D_80182DE4 = 1;
    D_801B2498 = D_80139258;
    D_8013923C = 0;
    D_801B30A4 = 0x5A;
    D_801B30A0++;
    func_800B681C();
}

void func_800B8F3C(void)
{
    D_801B30A4 = 8;
    D_801B30A0++;
    func_800B6918();
}

void func_800B8F74(void)
{
    D_801B30A0++;
}

s32 func_800B8F8C(s32 reset)
{
    if (reset != 0)
    {
        D_801B30A8 = 1;
        D_801B30AC = 1;
        return 1;
    }

    if ((u32)D_801B30A8 >= 6)
    {
        return 0;
    }

    D_800D760C[D_801B30A8]();
    return 1;
}

void func_800B9004(void)
{
    D_801B30A8 = 1;
    D_801B30AC = 1;
}

void func_800B901C(void)
{
    D_801B25E0 = 1;
    D_801B3118 = *(WmapPair*)&D_80139258;
    D_8013926C = 0;
    D_801B30AC = 0xB6;
    D_801B30A8++;
    func_800B6A10();
}

void func_800B9098(void)
{
    D_801B30AC = 1;
    D_801B30A8++;
    func_800B6B0C();
}

void func_800B90D0(void)
{
    D_801B30A8++;
}

s32 func_800B90E8(s32 reset)
{
    if (reset != 0)
    {
        D_801B30B0 = 1;
        D_801B30B4 = 1;
        return 1;
    }

    if ((u32)D_801B30B0 >= 6)
    {
        return 0;
    }

    D_800D7624[D_801B30B0]();
    return 1;
}

void func_800B9160(void)
{
    D_801B30B0 = 1;
    D_801B30B4 = 1;
}

void func_800B9178(void)
{
    D_80182DF4 = 0x81;
    D_8013B240 = D_80139258;
    D_80139260 = 0;
    D_801B30B4 = 0x68;
    D_801B30B0++;
    func_800B6C04();
}

void func_800B91F4(void)
{
    D_801B30B4 = 0x10;
    D_801B30B0++;
    func_800B6D00();
}

void func_800B922C(void)
{
    D_801B30B0++;
}

s32 func_800B9244(s32 reset)
{
    if (reset != 0)
    {
        D_801B30B8 = 1;
        D_801B30BC = 1;
        return 1;
    }

    if ((u32)D_801B30B8 >= 6)
    {
        return 0;
    }

    D_800D763C[D_801B30B8]();
    return 1;
}

void func_800B92BC(void)
{
    D_801B30B8 = 1;
    D_801B30BC = 1;
}

void func_800B92D4(void)
{
    D_801B25D8 = 1;
    D_801B2670 = D_80139258;
    D_80139264 = 0;
    D_801B30BC = 0x87;
    D_801B30B8++;
    func_800B6DF8();
}

void func_800B9350(void)
{
    D_801B30BC = 8;
    D_801B30B8++;
    func_800B6EF4();
}

void func_800B9388(void)
{
    D_801B30B8++;
}

s32 func_800B93A0(s32 reset)
{
    if (reset != 0)
    {
        D_801B30C0 = 1;
        D_801B30C4 = 1;
        return 1;
    }

    if ((u32)D_801B30C0 >= 6)
    {
        return 0;
    }

    D_800D7654[D_801B30C0]();
    return 1;
}

void func_800B9418(void)
{
    D_801B30C0 = 1;
    D_801B30C4 = 1;
}

void func_800B9430(void)
{
    D_801B25DC = 1;
    D_801B2678 = D_80139258;
    D_80139268 = 0;
    D_801B30C4 = 0xF0;
    D_801B30C0++;
    func_800B6FEC();
}

void func_800B94AC(void)
{
    D_801B30C4 = 0x20;
    D_801B30C0++;
    func_800B70CC();
}

void func_800B94E4(void)
{
    D_801B30C0++;
}

s32 func_800B94FC(s32 reset)
{
    if (reset != 0)
    {
        D_801B30C8 = 1;
        D_801B30CC = 1;
        return 1;
    }

    if ((u32)D_801B30C8 >= 6)
    {
        return 0;
    }

    D_800D766C[D_801B30C8]();
    return 1;
}

void func_800B9574(void)
{
    D_801B30C8 = 1;
    D_801B30CC = 1;
}

void func_800B958C(void)
{
    D_80139240 = 0;
    D_801B30CC = 0xF8;
    D_801B30C8++;
    func_800B95CC();
}

void func_800B95CC(void)
{
    s32 timer;

    func_8006AEE0();
    wmap_draw_model(D_8011CF30, 0, 0xB0, 0x35, 0x7800, 1, D_80139240, -0x32, 0, -1);
    D_80139240 += 8;
    if (D_80139240 >= 0x81)
    {
        D_80139240 = 0x80;
    }

    timer = D_801B30CC - 1;
    D_801B30CC = timer;
    if (timer == 0)
    {
        D_801B30C8++;
    }
}

void func_800B9680(void)
{
    D_801B30CC = 0x10;
    D_801B30C8++;
    func_800B96B8();
}

void func_800B96B8(void)
{
    s32 timer;

    func_8006AEE0();
    wmap_draw_model(D_8011CF30, 0, 0xB0, 0x35, 0x7800, 1, D_80139240, -0x32, 0, -1);
    D_80139240 -= 8;
    if (D_80139240 < 0)
    {
        D_80139240 = 0;
    }

    timer = D_801B30CC - 1;
    D_801B30CC = timer;
    if (timer == 0)
    {
        D_801B30C8++;
    }
}

void func_800B9764(void)
{
    D_801B30C8++;
}

s32 func_800B977C(s32 reset)
{
    if (reset != 0)
    {
        D_801B30D0 = 1;
        D_801B30D4 = 1;
        return 1;
    }

    if ((u32)D_801B30D0 >= 6)
    {
        return 0;
    }

    D_800D7684[D_801B30D0]();
    return 1;
}

void func_800B97F4(void)
{
    D_801B30D0 = 1;
    D_801B30D4 = 1;
}

void func_800B980C(void)
{
    D_8013924C = 1;
    D_801B30D4 = 0x70;
    D_801B30D0++;
    func_800B9850();
}

void func_800B9850(void)
{
    s32 timer;

    func_8006AEE0();
    wmap_draw_model(D_8011CF24, 0, 0xA, 0x35, 0x7840, 1, D_8013924C, 0, 0, -1);
    D_8013924C += 8;
    if (D_8013924C >= 0x82)
    {
        D_8013924C = 0x81;
    }

    timer = D_801B30D4 - 1;
    D_801B30D4 = timer;
    if (timer == 0)
    {
        D_801B30D0++;
    }
}

void func_800B9900(void)
{
    D_801B30D4 = 0x20;
    D_801B30D0++;
    func_800B9938();
}

void func_800B9938(void)
{
    s32 timer;

    func_8006AEE0();
    wmap_draw_model(D_8011CF24, 0, 0xA, 0x35, 0x7840, 1, D_8013924C, 0, 0, -1);
    D_8013924C -= 4;
    if (D_8013924C < 0)
    {
        D_8013924C = 0;
    }

    timer = D_801B30D4 - 1;
    D_801B30D4 = timer;
    if (timer == 0)
    {
        D_801B30D0++;
    }
}

void func_800B99E0(void)
{
    D_801B30D0++;
}

s32 func_800B99F8(s32 reset)
{
    if (reset != 0)
    {
        D_801B30D8 = 1;
        D_801B30DC = 1;
        return 1;
    }

    if ((u32)D_801B30D8 >= 4)
    {
        return 0;
    }

    D_800D769C[D_801B30D8]();
    return 1;
}

void func_800B9A70(void)
{
    D_801B30D8 = 1;
    D_801B30DC = 1;
}

void func_800B9A88(void)
{
    D_801399C4 = &D_8011D538;
    D_800D939C.scale_index = 0xF;
    D_800D939C.sequence = 1;
    D_800D939C.previous_sequence = -1;
    D_800D939C.shade_step = 4;
    D_800D939C.unknown_02 = 0;
    D_800D939C.target_shade = 0x61;
    D_800D939C.shade = 1;
    D_801B30DC = 0x64;
    D_801B30D8++;
    func_800B9B08();
}

void func_800B9B08(void)
{
    s32 timer;
    WmapConfigA* config;

    config = &D_800D939C;
    wmap_step_actor_animation(config, &D_801399C0);
    wmap_draw_actor_sprite(config, g_wmap_focus_screen_position.packed, 0x2B, 8, 0);
    timer = D_801B30DC - 1;
    D_801B30DC = timer;
    if (timer == 0)
    {
        D_801B30D8++;
    }
}

void func_800B9B84(void)
{
    D_801B30D8++;
}

s32 func_800B9B9C(s32 reset)
{
    if (reset != 0)
    {
        D_801B30E0 = 1;
        D_801B30E4 = 1;
        return 1;
    }

    if ((u32)D_801B30E0 >= 6)
    {
        return 0;
    }

    D_800D76AC[D_801B30E0]();
    return 1;
}

void func_800B9C14(void)
{
    D_801B30E0 = 1;
    D_801B30E4 = 1;
}

void func_800B9C2C(void)
{
    D_801399CC = &D_8011D538;
    D_800D93C8.scale_index = 0xF;
    D_800D93C8.sequence = 2;
    D_800D93C8.previous_sequence = -1;
    D_800D93C8.shade_step = 2;
    D_800D93C8.unknown_02 = 0;
    D_800D93C8.target_shade = 0x81;
    D_800D93C8.shade = 1;
    D_801B30E4 = 0x6B;
    D_801B30E0++;
    func_800B9CAC();
}

void func_800B9CAC(void)
{
    s32 timer;
    WmapConfigA* config;

    config = &D_800D93C8;
    wmap_step_actor_animation(config, &D_801399C8);
    wmap_draw_actor_sprite(config, g_wmap_focus_screen_position.packed, 0x10, 8, 0);
    timer = D_801B30E4 - 1;
    D_801B30E4 = timer;
    if (timer == 0)
    {
        D_801B30E0++;
    }
}

void func_800B9D28(void)
{
    D_800D93C8.shade_step = 8;
    D_800D93C8.target_shade = 0;
    D_801B30E4 = 0x10;
    D_801B30E0++;
    func_800B9D74();
}

void func_800B9D74(void)
{
    s32 timer;
    WmapConfigA* config;

    config = &D_800D93C8;
    wmap_step_actor_animation(config, &D_801399C8);
    wmap_draw_actor_sprite(config, g_wmap_focus_screen_position.packed, 0x10, 8, 0);
    timer = D_801B30E4 - 1;
    D_801B30E4 = timer;
    if (timer == 0)
    {
        D_801B30E0++;
    }
}

void func_800B9DF0(void)
{
    D_801B30E0++;
}

s32 func_800B9E08(s32 reset)
{
    if (reset != 0)
    {
        D_801B30E8 = 1;
        D_801B30EC = 1;
        return 1;
    }

    if ((u32)D_801B30E8 >= 6)
    {
        return 0;
    }

    D_800D76C4[D_801B30E8]();
    return 1;
}

void func_800B9E80(void)
{
    D_801B30E8 = 1;
    D_801B30EC = 1;
}

void func_800B9E98(void)
{
    D_801399D4 = &D_8011D538;
    D_800D93F4.scale_index = 0xF;
    D_800D93F4.sequence = 3;
    D_800D93F4.previous_sequence = -1;
    D_800D93F4.shade_step = 2;
    D_800D93F4.target_shade = 0x81;
    D_800D93F4.unknown_02 = 0;
    D_800D93F4.shade = 1;
    D_801B30EC = 0xB4;
    D_801B30E8++;
    func_800B9F1C();
}

void func_800B9F1C(void)
{
    s32 timer;
    WmapConfigA* config;

    config = &D_800D93F4;
    wmap_step_actor_animation(config, &D_801399D0);
    wmap_draw_actor_sprite(config, g_wmap_focus_screen_position.packed, 0x10, 8, 0);
    timer = D_801B30EC - 1;
    D_801B30EC = timer;
    if (timer == 0)
    {
        D_801B30E8++;
    }
}

void func_800B9F98(void)
{
    D_800D93F4.shade_step = 4;
    D_800D93F4.target_shade = 0;
    D_801B30EC = 0x20;
    D_801B30E8++;
    func_800B9FE4();
}

void func_800B9FE4(void)
{
    s32 timer;
    WmapConfigA* config;

    config = &D_800D93F4;
    wmap_step_actor_animation(config, &D_801399D0);
    wmap_draw_actor_sprite(config, g_wmap_focus_screen_position.packed, 0x10, 8, 0);
    timer = D_801B30EC - 1;
    D_801B30EC = timer;
    if (timer == 0)
    {
        D_801B30E8++;
    }
}

void func_800BA060(void)
{
    D_801B30E8++;
}

s32 func_800BA078(s32 reset)
{
    if (reset != 0)
    {
        D_801B30F0 = 1;
        D_801B30F4 = 1;
        return 1;
    }

    if ((u32)D_801B30F0 >= 6)
    {
        return 0;
    }

    D_800D76DC[D_801B30F0]();
    return 1;
}

void func_800BA0F0(void)
{
    D_801B30F0 = 1;
    D_801B30F4 = 1;
}

void func_800BA108(void)
{
    s32 timer;

    func_8006C448(&D_80139280[40]);
    timer = D_801B30F4 - 1;
    D_801B30F4 = timer;
    if (timer == 0)
    {
        D_801B30F0++;
    }
}

void func_800BA15C(void)
{
    s32 i;

    for (i = 0; i < 0x28; i++)
    {
        D_800DB4C8[i].field_22 = 0;
        D_800DB4C8[i].field_26 = 0x20;
    }
    D_801B30F4 = 4;
    D_801B30F0++;
    func_800BA1BC();
}

void func_800BA1BC(void)
{
    s32 timer;

    func_8006C448(&D_80139280[40]);
    timer = D_801B30F4 - 1;
    D_801B30F4 = timer;
    if (timer == 0)
    {
        D_801B30F0++;
    }
}

void func_800BA210(void)
{
    D_801B30F0++;
}
