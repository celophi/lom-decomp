#include "wmap_model_render.h"
#include "wmap_land_effect_33.h"
#include "wmap_sprite_render.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_main.h"
#include "wmap_effect_primitives.h"
#include "wmap_sequence_runtime.h"
#include "wmap_effect_resources.h"
#include "cdrom.h"
#include "sdk/libgte.h"

/** @brief Activity flag in a world-map motion slot. */
typedef struct
{
    s16 active;
    u8 unknown_02[0x12];
} WmapEffectMotionSlot;

void func_800BEBE0(void);
void func_800BEDE0(void);
s32 func_800BD998(s32 reset);
void func_800BD934(void);
void func_800BD970(void);
s32 func_800BE1A8(s32 reset);
s32 func_800BE40C(s32 reset);
s32 func_800BE678(s32 reset);
s32 func_800BE8E4(s32 reset);
s32 func_800BEB50(s32 reset);
s32 func_800BED50(s32 reset);
s32 func_800BEF58(s32 reset);
s32 func_800BF160(s32 reset);
s32 func_800BF368(s32 reset);
s32 func_800BDF40(s32 reset);
void func_800BE04C(void);
void func_800BE114(void);
void func_800BE2B0(void);
void func_800BE378(void);
void func_800BE51C(void);
void func_800BE5E4(void);
void func_800BE788(void);
void func_800BE850(void);
void func_800BE9F4(void);
void func_800BEABC(void);
void func_800BECB0(void);
void func_800BEEB4(void);
void func_800BF0BC(void);
void func_800BF1F0(void);
void func_800BF2C4(void);
void func_800BEFE8(void);

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

extern s32 D_801B31E8;
extern s32 D_801B31EC;
extern s32 D_801B31F0;
extern s32 D_801B31F4;
extern WmapHandler D_800D78BC[];
extern WmapHandler D_800D78CC[];
extern WmapHandler D_800D7924[];
extern WmapHandler D_800D793C[];
extern WmapHandler D_800D7954[];
extern WmapHandler D_800D796C[];
extern WmapHandler D_800D7984[];
extern WmapHandler D_800D799C[];
extern WmapHandler D_800D79B4[];
extern WmapHandler D_800D79CC[];
extern WmapHandler D_800D79E4[];
extern WmapHandler D_800D79FC[];
extern u8 D_800D95D8[];
extern u8 D_800DA028[];
extern u8 D_800DAA78[];
extern u8 D_800DB310[];
extern s32 D_800DBE70;
extern s32 D_800DCF18[];
extern s32 D_8011D510;
extern s32 D_8011D530;
extern u8 D_80123538;
extern u8 D_80125538;
extern u8 D_80127538;
extern s32 D_80139228;
extern WmapCell D_80139290[][6];
extern WmapAnimationSlot D_80139A28[];
extern u8 D_80139C08[];
extern u8 D_80139DE8[];
extern u8 D_80139F78[];
extern s32 D_80139978;
extern void* D_801399B4;
extern void* D_801399BC;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern s32 D_8013B20C;
extern s32 D_8013B208;
extern s32 D_8013B294;
extern s32 D_80182DE8;
extern s32 D_801ADAE0;
extern s32 D_801B31B0;
extern s32 D_801B31B4;
extern s32 D_801B31B8;
extern s32 D_801B31BC;
extern s32 D_801B31C0;
extern s32 D_801B31C4;
extern s32 D_801B31C8;
extern s32 D_801B31CC;
extern s32 D_801B31D0;
extern s32 D_801B31D4;
extern s32 D_801B31D8;
extern s32 D_801B31DC;
extern s32 D_801B31E0;
extern s32 D_801B31E4;
extern s32 D_801B31F8;
extern s32 D_801B31FC;
extern s32 D_801B3200;
extern s32 D_801B3204;
extern s32 D_801B3208;
extern s32 D_801B320C;
extern void akao_fade_all_sfx_volume(s32, s32);
extern void func_800BD750(void);
extern void func_800BD934(void);
extern void func_800BD970(void);
extern s32 func_800BD998(s32);
extern s32 func_800BDF40(s32);
extern void func_800BE04C(void);
extern void func_800BE114(void);
extern s32 func_800BE1A8(s32);
extern void func_800BE2B0(void);
extern void func_800BE378(void);
extern s32 func_800BE40C(s32);
extern void func_800BE51C(void);
extern void func_800BE5E4(void);
extern s32 func_800BE678(s32);
extern void func_800BE788(void);
extern void func_800BE850(void);
extern s32 func_800BE8E4(s32);
extern void func_800BE9F4(void);
extern void func_800BEABC(void);
extern s32 func_800BEB50(s32);
extern void func_800BECB0(void);
extern s32 func_800BED50(s32);
extern void func_800BEEB4(void);
extern s32 func_800BEF58(s32);
extern void func_800BEFE8(void);
extern void func_800BF0BC(void);
extern s32 func_800BF160(s32);
extern void func_800BF1F0(void);
extern void func_800BF2C4(void);
extern s32 func_800BF368(s32);
extern u8 D_80121538[];

extern VECTOR g_wmap_camera_translation;
extern VECTOR D_801B2650;

extern SVECTOR D_80139258;
extern SVECTOR D_801B24A0;

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

extern WmapEffectMotionSlot D_801AFBD0[];

/** @brief Configure the first world-map effect state and reset its fifty resource slots. */
void func_800BD41C(void)
{
    s32 i;

    D_80139280[1] = 0;
    D_80139280[2] = 3;
    D_80139280[3] = 0x20;
    D_80139280[4] = 0;
    D_80139280[5] = 2;
    D_80139280[6] = 0x190;
    D_80139280[7] = 0x14;
    D_80139280[8] = 0x2C;
    D_80139280[9] = 3;
    D_80139280[10] = 0x32C8;

    for (i = 0; i < 50; i++)
    {
        D_801AFBD0[i + 20].active = 0;
        D_80139988[i + 20].data = D_80121538;
    }

    D_801B31EC = 100;
    D_801B31E8++;
    func_800BEBE0();
}

/** @brief Configure the second world-map effect state and reset its forty resource slots. */
void func_800BD4E8(void)
{
    s32 i;

    D_80139280[11] = 1;
    D_80139280[12] = 2;
    D_80139280[13] = 0x80;
    D_80139280[14] = 0;
    D_80139280[15] = 1;
    D_80139280[16] = 0x190;
    D_80139280[17] = 0x50;
    D_80139280[18] = 0x2C;
    D_80139280[19] = 4;
    D_80139280[20] = 0x4650;

    for (i = 0; i < 40; i++)
    {
        D_801AFBD0[i + 80].active = 0;
        D_80139988[i + 80].data = D_80121538;
    }

    D_801B31F4 = 40;
    D_801B31F0++;
    func_800BEDE0();
}

void func_800BD5B8(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    s32 index;
    s32 screen_offset;
    s32 config_offset;
    u8* config_base;
    u8* screen_base;
    u8* resource;
    u8* screen_entry;
    s16* config_entry;

    index = 0;
    config_base = D_801AFBD0;
    screen_base = D_80139988;
    resource = &D_80125538;
    screen_offset = 0x460;
    config_offset = 0xAF0;
    D_80139280[21] = 0;
    D_80139280[22] = 0;
    D_80139280[23] = 0x80;
    D_80139280[24] = 0;
    D_80139280[25] = 3;
    D_80139280[26] = 0x3E8;
    D_80139280[27] = 0x8C;
    D_80139280[28] = 0x13;
    D_80139280[29] = 0;
    D_80139280[30] = 0x32C8;

    do
    {
        screen_entry = (u8*)(screen_offset + (s32)screen_base);
        screen_offset += 8;
        config_entry = (s16*)(config_offset + (s32)config_base);
        config_offset += 0x14;
        index++;
        *config_entry = 0;
        *(u8**)(screen_entry + 4) = resource;
    } while (index < 34);

    D_801B31FC = 102;
    D_801B31F8++;
    func_800BEFE8();
}

void func_800BD680(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    s32 index;
    s32 screen_offset;
    s32 config_offset;
    u8* config_base;
    u8* screen_base;
    u8* resource;
    u8* screen_entry;
    s16* config_entry;

    index = 0;
    config_base = D_801AFBD0;
    screen_base = D_80139988;
    resource = &D_80121538;
    screen_offset = 0x5F0;
    config_offset = 0xED8;
    D_80139280[41] = 1;
    D_80139280[42] = 3;
    D_80139280[43] = 0x40;
    D_80139280[44] = 0;
    D_80139280[45] = 3;
    D_80139280[46] = -0x1C2;
    D_80139280[47] = 0xBE;
    D_80139280[48] = 0x2C;
    D_80139280[49] = 5;
    D_80139280[50] = 0x6D60;

    do
    {
        screen_entry = (u8*)(screen_offset + (s32)screen_base);
        screen_offset += 8;
        config_entry = (s16*)(config_offset + (s32)config_base);
        config_offset += 0x14;
        index++;
        *config_entry = 0;
        *(u8**)(screen_entry + 4) = resource;
    } while (index < 40);

    D_801B3204 = 120;
    D_801B3200++;
    func_800BF1F0();
}

void func_800BD750(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

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
        wmap_draw_model(D_800DCF18, 0, 4, 0x35, 0x7800, 1, D_80182DE8, 0, 0, -1);
        fade = D_80182DE8 - 2;
        D_80182DE8 = fade;
        if (fade < 0)
        {
            D_80182DE8 = 0;
        }
    }
    PopMatrix();

    timer = D_801B320C - 1;
    D_801B320C = timer;
    if (timer == 0)
    {
        D_801B3208++;
    }
}

s32 func_800BD83C(s32 reset)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    if (reset != 0)
    {
        D_801B31B0 = 1;
        D_801B31B4 = 1;
        return 1;
    }

    if ((u32)D_801B31B0 >= 4)
    {
        return 0;
    }

    D_800D78BC[D_801B31B0]();
    return 1;
}

void func_800BD8B4(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B31B0 = 1;
    D_801B31B4 = 1;
}

void func_800BD8CC(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_800DBE70 = 0;
    wmap_reset_focus_screen_position();
    g_wmap_focus_screen_position.point.x = 0xA4;
    g_wmap_focus_screen_position.point.y = 0x69;
    wmap_start_sequence(func_800BD998);
    D_8013B20C = 1;
    D_801B31B0++;
    func_800BD934();
}

void func_800BD934(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    if (D_8013B20C == 0)
    {
        D_801B31B0++;
        func_800BD970();
    }
}

void func_800BD970(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_8013B294 = 1;
    D_80139228 = 1;
    D_801B31B0++;
}

s32 func_800BD998(s32 reset)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    if (reset != 0)
    {
        D_801B31B8 = 1;
        D_801B31BC = 1;
        return 1;
    }

    if ((u32)D_801B31B8 >= 0x16)
    {
        return 0;
    }

    D_800D78CC[D_801B31B8]();
    return 1;
}

void func_800BDA10(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B31B8 = 1;
    D_801B31BC = 1;
}

void func_800BDA28(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_8013B208 = 1;
    D_801ADAE0 = 1;
    wmap_play_sound(0x36, 0x80);
    D_801B31BC = 0xF;
    D_801B31B8++;
}

void func_800BDA78(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    if (--D_801B31BC == 0)
    {
        D_801B31B8++;
    }
}

void func_800BDAAC(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    wmap_start_sequence(func_800BF160);
    D_801B31BC = 0x23;
    D_801B31B8++;
}

void func_800BDAE8(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    if (--D_801B31BC == 0)
    {
        D_801B31B8++;
    }
}

void func_800BDB1C(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    wmap_start_sequence(func_800BF368);
    wmap_start_map_tint(0x703040);
    g_wmap_backdrop_target_level = 0xA;
    D_801B31BC = 0x19;
    D_801B31B8++;
}

void func_800BDB70(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    if (--D_801B31BC == 0)
    {
        D_801B31B8++;
    }
}

void func_800BDBA4(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    wmap_start_sequence(func_800BDF40);
    wmap_start_sequence(func_800BED50);
    D_801B31BC = 0x20;
    D_801B31B8++;
}

void func_800BDBEC(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    if (--D_801B31BC == 0)
    {
        D_801B31B8++;
    }
}

void func_800BDC20(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_80139978 = -1;
    wmap_start_sequence(func_800BE40C);
    D_801B31BC = 0x36;
    D_801B31B8++;
}

void func_800BDC68(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    if (--D_801B31BC == 0)
    {
        D_801B31B8++;
    }
}

void func_800BDC9C(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    wmap_start_sequence(func_800BEB50);
    D_801B31BC = 0x45;
    D_801B31B8++;
}

void func_800BDCD8(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    if (--D_801B31BC == 0)
    {
        D_801B31B8++;
    }
}

void func_800BDD0C(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    wmap_start_sequence(func_800BE1A8);
    D_801B31BC = 0x12;
    D_801B31B8++;
}

void func_800BDD48(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    if (--D_801B31BC == 0)
    {
        D_801B31B8++;
    }
}

void func_800BDD7C(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    wmap_start_sequence(func_800BEF58);
    D_801B31BC = 0x2B;
    D_801B31B8++;
}

void func_800BDDB8(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    if (--D_801B31BC == 0)
    {
        D_801B31B8++;
    }
}

void func_800BDDEC(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    wmap_start_sequence(func_800BE8E4);
    wmap_start_sequence(func_800BE678);
    wmap_start_sequence(func_800BF368);
    D_801B31BC = 0x64;
    D_801B31B8++;
}

void func_800BDE40(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    if (--D_801B31BC == 0)
    {
        D_801B31B8++;
    }
}

void func_800BDE74(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_80139290[D_8011D510][D_8011D530].value = 0x121;
    akao_fade_all_sfx_volume(0x3C, 0);
    D_801B31BC = 0x5A;
    D_801B31B8++;
}

void func_800BDEF0(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    if (--D_801B31BC == 0)
    {
        D_801B31B8++;
    }
}

void func_800BDF24(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_8013B20C = 0;
    D_801B31B8++;
}

s32 func_800BDF40(s32 reset)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    if (reset != 0)
    {
        D_801B31C0 = 1;
        D_801B31C4 = 1;
        return 1;
    }

    if ((u32)D_801B31C0 >= 6)
    {
        return 0;
    }

    D_800D7924[D_801B31C0]();
    return 1;
}

void func_800BDFB8(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B31C0 = 1;
    D_801B31C4 = 1;
}

void func_800BDFD0(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801399B4 = &D_80127538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.previous_sequence = -1;
    D_800D9344.shade_step = 4;
    D_800D9344.resource_index = 0;
    D_800D9344.sequence = 0;
    D_800D9344.target_shade = 0x80;
    D_800D9344.shade = 0;
    D_801B31C4 = 0xA0;
    D_801B31C0++;
    func_800BE04C();
}

void func_800BE04C(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0x2C, 8, 0);
    if (--D_801B31C4 == 0)
    {
        D_801B31C0++;
    }
}

void func_800BE0C8(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_800D9344.shade_step = 0x80;
    D_800D9344.target_shade = 0;
    D_801B31C4 = 1;
    D_801B31C0++;
    func_800BE114();
}

void func_800BE114(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0x2C, 8, 0);
    if (--D_801B31C4 == 0)
    {
        D_801B31C0++;
    }
}

void func_800BE190(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B31C0++;
}

s32 func_800BE1A8(s32 reset)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    if (reset != 0)
    {
        D_801B31C8 = 1;
        D_801B31CC = 1;
        return 1;
    }

    if ((u32)D_801B31C8 >= 6)
    {
        return 0;
    }

    D_800D793C[D_801B31C8]();
    return 1;
}

void func_800BE220(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B31C8 = 1;
    D_801B31CC = 1;
}

void func_800BE238(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801399BC = &D_80123538;
    D_800D9370.scale_index = 0xF;
    D_800D9370.previous_sequence = -1;
    D_800D9370.resource_index = 0;
    D_800D9370.sequence = 0;
    D_800D9370.shade_step = 0x80;
    D_800D9370.target_shade = 0x80;
    D_800D9370.shade = 0;
    D_801B31CC = 0x3E;
    D_801B31C8++;
    func_800BE2B0();
}

void func_800BE2B0(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, g_wmap_focus_screen_position.packed, 0x2C, 8, 0);
    if (--D_801B31CC == 0)
    {
        D_801B31C8++;
    }
}

void func_800BE32C(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_800D9370.shade_step = 0x80;
    D_800D9370.target_shade = 0;
    D_801B31CC = 1;
    D_801B31C8++;
    func_800BE378();
}

void func_800BE378(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, g_wmap_focus_screen_position.packed, 0x2C, 8, 0);
    if (--D_801B31CC == 0)
    {
        D_801B31C8++;
    }
}

void func_800BE3F4(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B31C8++;
}

s32 func_800BE40C(s32 reset)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    if (reset != 0)
    {
        D_801B31D0 = 1;
        D_801B31D4 = 1;
        return 1;
    }

    if ((u32)D_801B31D0 >= 6)
    {
        return 0;
    }

    D_800D7954[D_801B31D0]();
    return 1;
}

void func_800BE484(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B31D0 = 1;
    D_801B31D4 = 1;
}

void func_800BE49C(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801399C4 = &D_80121538;
    D_800D939C.scale_index = 0xF;
    D_800D939C.previous_sequence = -1;
    D_800D939C.shade_step = 0x80;
    D_800D939C.target_shade = 0x81;
    D_800D939C.resource_index = 0;
    D_800D939C.sequence = 0;
    D_800D939C.shade = 1;
    D_801B31D4 = 0xBC;
    D_801B31D0++;
    func_800BE51C();
}

void func_800BE51C(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    wmap_step_actor_animation(&D_800D939C, &D_801399C0);
    wmap_draw_actor_sprite(&D_800D939C, g_wmap_focus_screen_position.packed, 0x2E, 0xC, 0);
    if (--D_801B31D4 == 0)
    {
        D_801B31D0++;
    }
}

void func_800BE598(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_800D939C.shade_step = 0x80;
    D_800D939C.target_shade = 0;
    D_801B31D4 = 1;
    D_801B31D0++;
    func_800BE5E4();
}

void func_800BE5E4(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    wmap_step_actor_animation(&D_800D939C, &D_801399C0);
    wmap_draw_actor_sprite(&D_800D939C, g_wmap_focus_screen_position.packed, 0x2E, 0xC, 0);
    if (--D_801B31D4 == 0)
    {
        D_801B31D0++;
    }
}

void func_800BE660(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B31D0++;
}

s32 func_800BE678(s32 reset)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    if (reset != 0)
    {
        D_801B31D8 = 1;
        D_801B31DC = 1;
        return 1;
    }

    if ((u32)D_801B31D8 >= 6)
    {
        return 0;
    }

    D_800D796C[D_801B31D8]();
    return 1;
}

void func_800BE6F0(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B31D8 = 1;
    D_801B31DC = 1;
}

void func_800BE708(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    s32 value;

    D_801399CC = &D_80121538;
    D_800D93C8.scale_index = 0xF;
    D_800D93C8.sequence = value = 1;
    D_800D93C8.previous_sequence = -value;
    D_800D93C8.shade_step = 8;
    D_800D93C8.shade = value;
    D_800D93C8.resource_index = 0;
    D_800D93C8.target_shade = 0x81;
    D_801B31DC = 0x65;
    D_801B31D8++;
    func_800BE788();
}

void func_800BE788(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    wmap_step_actor_animation(&D_800D93C8, &D_801399C8);
    wmap_draw_actor_sprite(&D_800D93C8, g_wmap_focus_screen_position.packed, 0x2E, 0xC, 0);
    if (--D_801B31DC == 0)
    {
        D_801B31D8++;
    }
}

void func_800BE804(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_800D93C8.shade_step = 0x80;
    D_800D93C8.target_shade = 0;
    D_801B31DC = 1;
    D_801B31D8++;
    func_800BE850();
}

void func_800BE850(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    wmap_step_actor_animation(&D_800D93C8, &D_801399C8);
    wmap_draw_actor_sprite(&D_800D93C8, g_wmap_focus_screen_position.packed, 0x2E, 0xC, 0);
    if (--D_801B31DC == 0)
    {
        D_801B31D8++;
    }
}

void func_800BE8CC(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B31D8++;
}

s32 func_800BE8E4(s32 reset)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    if (reset != 0)
    {
        D_801B31E0 = 1;
        D_801B31E4 = 1;
        return 1;
    }

    if ((u32)D_801B31E0 >= 6)
    {
        return 0;
    }

    D_800D7984[D_801B31E0]();
    return 1;
}

void func_800BE95C(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B31E0 = 1;
    D_801B31E4 = 1;
}

void func_800BE974(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801399D4 = &D_80121538;
    D_800D93F4.scale_index = 0xF;
    D_800D93F4.sequence = 2;
    D_800D93F4.previous_sequence = -1;
    D_800D93F4.shade_step = 4;
    D_800D93F4.resource_index = 0;
    D_800D93F4.target_shade = 0x80;
    D_800D93F4.shade = 0;
    D_801B31E4 = 0x66;
    D_801B31E0++;
    func_800BE9F4();
}

void func_800BE9F4(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    wmap_step_actor_animation(&D_800D93F4, &D_801399D0);
    wmap_draw_actor_sprite(&D_800D93F4, g_wmap_focus_screen_position.packed, 0x2C, 8, 0);
    if (--D_801B31E4 == 0)
    {
        D_801B31E0++;
    }
}

void func_800BEA70(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_800D93F4.shade_step = 2;
    D_800D93F4.target_shade = 0;
    D_801B31E4 = 0x40;
    D_801B31E0++;
    func_800BEABC();
}

void func_800BEABC(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    wmap_step_actor_animation(&D_800D93F4, &D_801399D0);
    wmap_draw_actor_sprite(&D_800D93F4, g_wmap_focus_screen_position.packed, 0x2C, 8, 0);
    if (--D_801B31E4 == 0)
    {
        D_801B31E0++;
    }
}

void func_800BEB38(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B31E0++;
}

s32 func_800BEB50(s32 reset)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    if (reset != 0)
    {
        D_801B31E8 = 1;
        D_801B31EC = 1;
        return 1;
    }

    if ((u32)D_801B31E8 >= 6)
    {
        return 0;
    }

    D_800D799C[D_801B31E8]();
    return 1;
}

void func_800BEBC8(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B31E8 = 1;
    D_801B31EC = 1;
}

void func_800BEBE0(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    func_8006AEE0();
    func_8006A2FC(D_800D95D8, D_80139A28, 0x32, 0xFF, 1, 8, 0, (WmapState*)D_80139280);
    if (--D_801B31EC == 0)
    {
        D_801B31E8++;
    }
}

void func_800BEC68(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B31EC = 0x20;
    D_80139280[5] = -1;
    D_801B31E8++;
    func_800BECB0();
}

void func_800BECB0(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    func_8006AEE0();
    func_8006A2FC(D_800D95D8, D_80139A28, 0x32, 0xFF, 1, 8, 0, (WmapState*)D_80139280);
    if (--D_801B31EC == 0)
    {
        D_801B31E8++;
    }
}

void func_800BED38(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B31E8++;
}

s32 func_800BED50(s32 reset)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    if (reset != 0)
    {
        D_801B31F0 = 1;
        D_801B31F4 = 1;
        return 1;
    }

    if ((u32)D_801B31F0 >= 6)
    {
        return 0;
    }

    D_800D79B4[D_801B31F0]();
    return 1;
}

void func_800BEDC8(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B31F0 = 1;
    D_801B31F4 = 1;
}

void func_800BEDE0(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    func_8006AEE0();
    func_8006A2FC(D_800DA028, D_80139C08, 0x28, 0xFF, 1, 2, 0, &D_80139280[10]);
    if (--D_801B31F4 == 0)
    {
        D_801B31F0++;
    }
}

void func_800BEE6C(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B31F4 = 0x80;
    D_80139280[15] = -1;
    D_801B31F0++;
    func_800BEEB4();
}

void func_800BEEB4(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    func_8006AEE0();
    func_8006A2FC(D_800DA028, D_80139C08, 0x28, 0xFF, 1, 2, 0, &D_80139280[10]);
    if (--D_801B31F4 == 0)
    {
        D_801B31F0++;
    }
}

void func_800BEF40(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B31F0++;
}

s32 func_800BEF58(s32 reset)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    if (reset != 0)
    {
        D_801B31F8 = 1;
        D_801B31FC = 1;
        return 1;
    }

    if ((u32)D_801B31F8 >= 6)
    {
        return 0;
    }

    D_800D79CC[D_801B31F8]();
    return 1;
}

void func_800BEFD0(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B31F8 = 1;
    D_801B31FC = 1;
}

void func_800BEFE8(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    func_8006AEE0();
    func_8006A2FC(D_800DAA78, D_80139DE8, 0x22, 0xFF, 1, 2, 0, (WmapState*)D_80139280 + 1);
    if (--D_801B31FC == 0)
    {
        D_801B31F8++;
    }
}

void func_800BF074(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B31FC = 0x80;
    D_80139280[25] = -1;
    D_801B31F8++;
    func_800BF0BC();
}

void func_800BF0BC(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    func_8006AEE0();
    func_8006A2FC(D_800DAA78, D_80139DE8, 0x22, 0xFF, 1, 2, 0, (WmapState*)D_80139280 + 1);
    if (--D_801B31FC == 0)
    {
        D_801B31F8++;
    }
}

void func_800BF148(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B31F8++;
}

s32 func_800BF160(s32 reset)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    if (reset != 0)
    {
        D_801B3200 = 1;
        D_801B3204 = 1;
        return 1;
    }

    if ((u32)D_801B3200 >= 6)
    {
        return 0;
    }

    D_800D79E4[D_801B3200]();
    return 1;
}

void func_800BF1D8(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3200 = 1;
    D_801B3204 = 1;
}

void func_800BF1F0(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    func_8006AEE0();
    func_8006A2FC(D_800DB310, D_80139F78, 0x28, 0xFF, 1, 4, 0, (WmapState*)D_80139280 + 2);
    if (--D_801B3204 == 0)
    {
        D_801B3200++;
    }
}

void func_800BF27C(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3204 = 0x40;
    D_80139280[45] = -1;
    D_801B3200++;
    func_800BF2C4();
}

void func_800BF2C4(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    func_8006AEE0();
    func_8006A2FC(D_800DB310, D_80139F78, 0x28, 0xFF, 1, 4, 0, (WmapState*)D_80139280 + 2);
    if (--D_801B3204 == 0)
    {
        D_801B3200++;
    }
}

void func_800BF350(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3200++;
}

s32 func_800BF368(s32 reset)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    if (reset != 0)
    {
        D_801B3208 = 1;
        D_801B320C = 1;
        return 1;
    }

    if ((u32)D_801B3208 >= 4)
    {
        return 0;
    }

    D_800D79FC[D_801B3208]();
    return 1;
}

void func_800BF3E0(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3208 = 1;
    D_801B320C = 1;
}

void func_800BF3F8(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    D_801B320C = 0x40;
    D_801B3208++;
    func_800BD750();
}

void func_800BF4A8(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3208++;
}
