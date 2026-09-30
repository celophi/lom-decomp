#include "wmap_model_render.h"
#include "wmap_land_effect_24.h"
#include "wmap_sprite_render.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_main.h"
#include "wmap_effect_primitives.h"
#include "wmap_sequence_runtime.h"
#include "wmap_effect_resources.h"
#include "cdrom.h"
#include "sdk/libgte.h"

void func_800BAEF8(void);
void func_800BAF34(void);
void func_800BAF78(void);
void func_800BAFB4(void);
s32 func_800BAFCC(s32 reset);
s32 func_800BB610(s32 reset);
s32 func_800BB87C(s32 reset);
s32 func_800BBAE8(s32 reset);
void func_800BCDE0(void);
void func_800BCCF0(void);
void func_800BCAC8(void);
void func_800BC8C4(void);
void func_800BD258(void);
void func_800BD354(void);
s32 func_800BC01C(s32 reset);
s32 func_800BC1BC(s32 reset);
s32 func_800BC360(s32 reset);
s32 func_800BC4B8(s32 reset);
s32 func_800BC60C(s32 reset);
s32 func_800BC764(s32 reset);
s32 func_800BC964(s32 reset);
void func_800BC7F4(void);
void func_800BC9F4(void);
void func_800BCBFC(void);
s32 func_800BCB6C(s32 reset);
s32 func_800BCEA8(s32 reset);
s32 func_800BCFD4(s32 reset);
s32 func_800BD12C(s32 reset);
s32 func_800BBD54(s32 reset);
void func_800BB720(void);
void func_800BB7E8(void);
void func_800BB98C(void);
void func_800BBA54(void);
void func_800BBBF8(void);
void func_800BBCC0(void);
void func_800BBF88(void);
void func_800BC128(void);
void func_800BC2CC(void);
void func_800BBE9C(void);

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
    s32 field_00;
    s32 field_04;
    s32 field_08;
} WmapInt3;

typedef struct
{
    s32 value;
    u8 pad[0x24];
} WmapCell;

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

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    s16 field_00;
    u8 pad_02[0x12];
} WmapConfigEntry;

extern WmapStepHandlerSlot D_800D76F4[];
extern WmapStepHandlerSlot D_800D770C[];
extern WmapStepHandlerSlot D_800D7774[];
extern WmapStepHandlerSlot D_800D778C[];
extern WmapStepHandlerSlot D_800D77A4[];
extern WmapStepHandlerSlot D_800D77BC[];
extern WmapStepHandlerSlot D_800D77D4[];
extern WmapStepHandlerSlot D_800D77E4[];
extern WmapStepHandlerSlot D_800D77F4[];
extern WmapStepHandlerSlot D_800D7804[];
extern WmapStepHandlerSlot D_800D7814[];
extern WmapStepHandlerSlot D_800D7824[];
extern WmapStepHandlerSlot D_800D78A4[];
extern WmapStepHandlerSlot D_800D788C[];
extern WmapStepHandlerSlot D_800D7874[];
extern WmapStepHandlerSlot D_800D7854[];
extern WmapStepHandlerSlot D_800D783C[];
extern u8 D_800D95D8[];
extern u8 D_800DA028[];
extern s32 D_800DCEB0;
extern WmapShort3 D_800DCEB8;
extern s32 D_800DCF18[];
extern s32 D_800D9158;
extern s32_ptr D_8011CF1C;
extern s32_ptr D_8011CF24;
extern s32_ptr D_8011CF28;
extern s32_ptr D_8011CF2C;
extern s32 D_8011D4FC;
extern s32 D_8011D510;
extern s32 D_8011D530;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern WmapInt3 D_80139200;
extern WmapShort3 D_80139210;
extern s32 D_80139240;
extern s32 D_80139244;
extern s32 D_8013924C;
extern s32 D_80139250;
extern s32 D_80139268;
extern WmapCell D_80139290[][6];
extern WmapAnimationSlot D_80139A28[];
extern u8 D_80139C08[];
extern WmapInt3 D_80139968;
extern void_ptr D_801399AC;
extern void_ptr D_801399B4;
extern void_ptr D_801399BC;
extern void_ptr D_801399C4;
extern void_ptr D_801399D4;
extern void_ptr D_801399DC;
extern s32 D_8013B20C;
extern s32 D_8013B208;
extern s32 D_8013B29C;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern s32 D_801ADAE0;
extern s32 D_801B25D8;
extern s32 D_801B25DC;
extern s32 D_801B3128;
extern s32 D_801B312C;
extern s32 D_801B3130;
extern s32 D_801B3134;
extern s32 D_801B3138;
extern s32 D_801B313C;
extern s32 D_801B3140;
extern s32 D_801B3144;
extern s32 D_801B3148;
extern s32 D_801B314C;
extern s32 D_801B3150;
extern s32 D_801B3154;
extern s32 D_801B3158;
extern s32 D_801B315C;
extern s32 D_801B3160;
extern s32 D_801B3164;
extern s32 D_801B3168;
extern s32 D_801B316C;
extern s32 D_801B3170;
extern s32 D_801B3174;
extern s32 D_801B3178;
extern s32 D_801B317C;
extern s32 D_801B3180;
extern s32 D_801B3184;
extern s32 D_801B3188;
extern s32 D_801B318C;
extern s32 D_801B3190;
extern s32 D_801B3194;
extern s32 D_801B3198;
extern s32 D_801B319C;
extern s32 D_801B31A0;
extern s32 D_801B31A4;
extern s32 D_801B31A8;
extern s32 D_801B31AC;
extern s32 D_8011CF74;
extern void func_800BA580();
extern void func_800BA66C();
extern void func_800BA758();
extern void func_800BC7F4(void);
extern void func_800BAAB0();
extern void func_800BAB80();
extern void func_800BAC44();
extern void func_800BAD38();
extern void func_800BAEF8();
extern void func_800BAF34();
extern void func_800BAF78();
extern void func_800BAFB4();
extern s32 func_800BAFCC(s32);
extern s32 func_800BB610(s32);
extern void func_800BB720();
extern void func_800BB7E8();
extern s32 func_800BB87C(s32);
extern void func_800BB98C();
extern void func_800BBA54();
extern s32 func_800BBAE8(s32);
extern void func_800BBBF8();
extern void func_800BBCC0();
extern s32 func_800BBD54(s32);
extern void func_800BBF88();
extern s32 func_800BC01C(s32);
extern void func_800BC128();
extern s32 func_800BC1BC(s32);
extern void func_800BC2CC();
extern s32 func_800BC360(s32);
extern s32 func_800BC4B8(s32);
extern s32 func_800BC60C(s32);
extern s32 func_800BC764(s32);
extern void func_800BC8C4();
extern s32 func_800BC964(s32);
extern void func_800BC9F4(void);
extern void func_800BCAC8();
extern s32 func_800BCB6C(s32);
extern void func_800BCBFC(void);
extern void func_800BCCF0();
extern void func_800BCDE0();
extern s32 func_800BCEA8(s32);
extern s32 func_800BCFD4(s32);
extern s32 func_800BD12C(s32);
extern void func_800BD258();
extern void func_800BD354();

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

extern VECTOR g_wmap_camera_translation;
extern VECTOR D_801B2650;
extern VECTOR D_801B2478;
extern VECTOR D_80139870;

extern SVECTOR D_80139258;
extern SVECTOR D_801B24A0;
extern SVECTOR D_8013B238;
extern SVECTOR D_801B2670;
extern SVECTOR D_801B24A8;

extern WmapSpriteActor D_800D9268[];
extern WmapSpriteActor D_800D9318;
extern WmapSpriteActor D_800D9344;
extern WmapSpriteActor D_800D9370;
extern WmapSpriteActor D_800D939C;
extern WmapSpriteActor D_800D93F4;
extern WmapSpriteActor D_800D9420;

extern WmapAnimationSlot D_80139988[];
extern WmapAnimationSlot D_801399A8;
extern WmapAnimationSlot D_801399B0;
extern WmapAnimationSlot D_801399B8;
extern WmapAnimationSlot D_801399C0;
extern WmapAnimationSlot D_801399D0;
extern WmapAnimationSlot D_801399D8;

extern WmapScreenPosition g_wmap_focus_screen_position;
extern WmapScreenPosition D_80182D58;

extern s32_ptr D_80139280;

extern WmapConfigEntry D_801AFBD0[];

void func_800BA228(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    wmap_start_sequence(func_800BC360);
    D_80139244 = 1;
    wmap_start_sequence(func_800BB87C);
    wmap_start_sequence(func_800BB610);
    wmap_start_sequence(func_800BBAE8);
    wmap_start_map_tint(0x701040);
    g_wmap_backdrop_target_level = 0;
    D_80182D74.field_00 = 0x38;
    D_80182D74.field_01 = 0;
    D_80182D74.field_02 = 0x18;
    D_80182D80.field_00 = 0x38;
    D_80182D80.field_01 = 0;
    D_80182D80.field_02 = 0x18;
    D_80182D8C.field_00 = 0x38;
    D_80182D8C.field_01 = 0;
    D_80182D8C.field_02 = 0x18;
    D_80182D94.field_00 = 0x38;
    D_80182D94.field_01 = 0;
    D_80182D94.field_02 = 0x18;
    D_801B3134 = 8;
    D_801B3130++;
}

void func_800BA300(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    wmap_start_sequence(func_800BCB6C);
    wmap_install_callback(func_8006C0EC);
    D_80139210.field_00 = 5;
    D_80139210.field_02 = 0;
    D_80139210.field_04 = 0;
    D_80139968.field_00 = 0;
    D_80139968.field_04 = 2;
    D_80139968.field_08 = 0;
    D_800DCEB8.field_00 = 0xFA;
    D_800DCEB8.field_02 = 0;
    D_800DCEB8.field_04 = 0;
    D_80139200.field_00 = 0;
    D_80139200.field_04 = 0x64;
    D_80139200.field_08 = 0;
    g_wmap_backdrop_target_level = 0;
    D_80182D74.field_00 = 0x38;
    D_80182D74.field_01 = 0;
    D_80182D74.field_02 = 0x18;
    D_80182D80.field_00 = 0x38;
    D_80182D80.field_01 = 0;
    D_80182D80.field_02 = 0x18;
    D_80182D8C.field_00 = 0;
    D_80182D8C.field_01 = 0;
    D_80182D8C.field_02 = 0;
    D_80182D94.field_00 = 0;
    D_80182D94.field_01 = 0;
    D_80182D94.field_02 = 0;
    D_801B3134 = 0x70;
    D_801B3130++;
}

/** @brief Update active effects and initialize the next available config slot. */
void func_800BA408(void)
{
    s32 index;
    s32 config_offset;
    u16 value;
    u8* config_base;
    WmapConfigA* config;
    void* entry;

    index = 0xAA;

    do
    {
        config_offset = index * 0x2C + 0xB0;
        config_base = (u8*)D_800D9268;
        entry = (void*)(config_offset + (u32)config_base);
        config = &D_800D9268[index + 4];
        if (*(s16*)(entry + 2) == 0)
        {
            wmap_draw_actor_sprite((void*)config, g_wmap_focus_screen_position.packed, 0xB, 2, 0);
            value = config->field_24 - 5;
            config->field_24 = value;
            if ((s32)(value << 0x10) <= 0)
            {
                *(s16*)(entry + 2) = (value = -1);
            }
        }
        index++;
    } while (index < 0xB9);

    index = 0xAA;
    if ((D_8011CF74 & 1) == 0)
    {
        void* entry;
        u8* copy_end;
        s32 config_offset;
        s32 screen_offset;
        u8* scan_base;
        u8* screen_base;

        scan_base = (u8*)D_800D9268;
        copy_end = scan_base + 0x154;
        screen_base = D_80139988;
        config_offset = 0x1DE8;
        screen_offset = 0x570;
        do
        {
            entry = (void*)(config_offset + (u32)scan_base);
            if (((WmapConfigA*)entry)->field_02 != 0)
            {
                typedef struct
                {
                    s32 words[4];
                } Chunk;
                typedef struct
                {
                    s32 words[3];
                } Tail;
                u8* source;
                source = scan_base + 0x134;
                do
                {
                    *(Chunk*)entry = *(Chunk*)source;
                    source += 16;
                    entry += 16;
                } while (source != copy_end);
                *(Tail*)entry = *(Tail*)source;
                *(WmapAlignedPair*)(screen_offset + (u32)screen_base) = *(WmapAlignedPair*)(screen_base + 0x38);
                ((WmapConfigA*)(config_offset + (u32)scan_base))->field_22 = 0;
                return;
            }
            config_offset += 0x2C;
            index++;
            screen_offset += 8;
        } while (index < 0xB9);
    }
}

void func_800BA580(void)
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
    timer = D_801B316C - 1;
    D_801B316C = timer;
    if (timer == 0)
    {
        D_801B3168++;
    }
}

void func_800BA66C(void)
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
        wmap_draw_model(D_8011CF1C, 0, 4, 0x35, 0x7800, 1, D_80182DEC, 0, 0, -1);
        fade = D_80182DEC - 1;
        D_80182DEC = fade;
        if (fade < 0)
        {
            D_80182DEC = 0;
        }
    }
    PopMatrix();
    timer = D_801B3174 - 1;
    D_801B3174 = timer;
    if (timer == 0)
    {
        D_801B3170++;
    }
}

void func_800BA758(void)
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
        wmap_draw_model(D_8011CF24, 0, 4, 0x35, 0x7800, 1, D_80182DF0, 0, 0, -1);
        fade = D_80182DF0 - 8;
        D_80182DF0 = fade;
        if (fade < 0)
        {
            D_80182DF0 = 0;
        }
    }
    PopMatrix();
    timer = D_801B317C - 1;
    D_801B317C = timer;
    if (timer == 0)
    {
        D_801B3178++;
    }
}

void func_800BA844(void)
{
    s32 i;

    D_80139280[1] = 0;
    D_80139280[2] = 0;
    D_80139280[3] = 0x80;
    D_80139280[4] = 0;
    D_80139280[5] = 3;
    D_80139280[6] = 0x384;
    D_80139280[7] = 0x14;
    D_80139280[8] = 8;
    D_80139280[9] = 1;
    D_80139280[10] = 0x1F40;
    for (i = 0; i < 60; i++)
    {
        D_801AFBD0[i + 20].field_00 = 0;
        D_80139988[i + 20].data = &D_80121538;
    }
    D_801B3184 = 0xB4;
    D_801B3180++;
    func_800BC7F4();
}

void func_800BA910(void)
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
    screen_offset = 0x280;
    config_offset = 0x640;
    D_80139280[11] = 1;
    D_80139280[12] = 4;
    D_80139280[13] = 0x40;
    D_80139280[15] = 3;
    D_80139280[16] = -0x1C2;
    D_80139280[17] = 0x50;
    D_80139280[18] = 8;
    D_80139280[19] = 2;
    D_80139280[14] = 0;
    D_80139280[20] = 0x61A8;

    do
    {
        screen_entry = (u8*)(screen_offset + (s32)screen_base);
        screen_offset += 8;
        config_entry = (s16*)(config_offset + (s32)config_base);
        config_offset += 0x14;
        index++;
        *config_entry = 0;
        *(u8**)(screen_entry + 4) = resource;
    } while (index < 0x1E);

    D_801B318C = 0x5A;
    D_801B3188++;
    func_800BC9F4();
}

void func_800BA9E4(void)
{
    s32 i;

    D_801B25DC = 1;
    D_800DCEB0 = 12;
    for (i = 120; i < 156; i++)
    {
        D_801AFBD0[i].field_00 = 0;
        D_80139988[i].data = &D_8011D538;
        D_800D9268[i].sequence = (i & 1) + 2;
        D_800D9268[i].resource_index = 0;
        D_800D9268[i].scale_index = 15;
        D_800D9268[i].previous_sequence = -1;
    }

    D_800D9158 = 2;
    D_801B3194 = 0x10;
    D_801B3190++;
    func_800BCBFC();
}

void func_800BAAB0(void)
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
    s32 timer;

    func_8006AEE0();
    wmap_draw_model(D_8011CF28, (D_80139240 >> 4) & 7, 0xA, 0x35, 0x7800, 1, D_80182DF4, 0, 0, -1);
    value = D_80182DF4 + 8;
    D_80182DF4 = value;
    if (value >= 0x82)
    {
        D_80182DF4 = 0x81;
    }
    D_80139240 += 0x10;
    timer = D_801B319C - 1;
    D_801B319C = timer;
    if (timer == 0)
    {
        D_801B3198++;
    }
}

void func_800BAB80(void)
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
    s32 timer;

    func_8006AEE0();
    wmap_draw_model(D_8011CF28, (D_80139240 >> 4) & 7, 0xA, 0x35, 0x7800, 1, D_80182DF4, 0, 0, -1);
    value = D_80182DF4 - 4;
    D_80182DF4 = value;
    if (value < 0)
    {
        D_80182DF4 = 0;
    }
    D_80139240 += 0x10;
    timer = D_801B319C - 1;
    D_801B319C = timer;
    if (timer == 0)
    {
        D_801B3198++;
    }
}

void func_800BAC44(void)
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
    s32 timer;

    wmap_set_map_rotation(&D_801B2670);
    wmap_draw_model(D_8011CF2C, (D_8013924C >> 4) & 7, 0xB0, 0x35, 0x7800, 1, D_801B25D8, 4, -0x44, -1);
    value = D_801B25D8 + 2;
    D_801B25D8 = value;
    if (value >= 0x81)
    {
        D_801B25D8 = 0x80;
    }
    D_8013924C += 0x18;
    timer = D_801B31A4 - 1;
    D_801B31A4 = timer;
    D_801B2670.vz += 0x28;
    if (timer == 0)
    {
        D_801B31A0++;
    }
}

void func_800BAD38(void)
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
    s32 timer;

    wmap_set_map_rotation(&D_801B2670);
    wmap_draw_model(D_8011CF2C, (D_8013924C >> 4) & 7, 0xB0, 0x35, 0x7800, 1, D_801B25D8, 4, -0x44, -1);
    value = D_801B25D8 - 0x10;
    D_801B25D8 = value;
    if (value < 0)
    {
        D_801B25D8 = 0;
    }
    D_8013924C += 0x18;
    timer = D_801B31A4 - 1;
    D_801B31A4 = timer;
    D_801B2670.vz += 0x28;
    if (timer == 0)
    {
        D_801B31A0++;
    }
}

s32 func_800BAE24(s32 reset)
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
        D_801B3128 = 1;
        D_801B312C = 1;
        return 1;
    }

    if ((u32)D_801B3128 >= 6)
    {
        return 0;
    }

    PS1_CALL(D_800D76F4[D_801B3128])();
    return 1;
}

void func_800BAE9C(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3128 = 1;
    D_801B312C = 1;
}

void func_800BAEB4(void)
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

    wmap_start_sequence(wmap_run_land_focus);
    D_8013B20C = value = 1;
    D_801B3128 += value;
    func_800BAEF8();
}

void func_800BAEF8(void)
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
        D_801B3128++;
        func_800BAF34();
    }
}

void func_800BAF34(void)
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

    wmap_start_sequence(func_800BAFCC);
    D_8013B20C = value = 1;
    D_801B3128 += value;
    func_800BAF78();
}

void func_800BAF78(void)
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
        D_801B3128++;
        func_800BAFB4();
    }
}

void func_800BAFB4(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3128++;
}

s32 func_800BAFCC(s32 reset)
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
        D_801B3130 = 1;
        D_801B3134 = 1;
        return 1;
    }

    if ((u32)D_801B3130 >= 0x1A)
    {
        return 0;
    }

    PS1_CALL(D_800D770C[D_801B3130])();
    return 1;
}

void func_800BB044(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3130 = 1;
    D_801B3134 = 1;
}

void func_800BB05C(void)
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

    D_8013B208 = value = 1;
    wmap_play_sound(0x35, 0x80);
    D_8013B29C = value;
    D_801B3134 = 4;
    D_801B3130 += value;
}

void func_800BB0B4(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    s32 timer;

    timer = D_801B3134 - 1;
    D_801B3134 = timer;
    if (timer == 0)
    {
        D_801B3130++;
    }
}

void func_800BB0E8(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    wmap_start_sequence(func_800BBD54);
    wmap_start_map_tint(0x605060);
    g_wmap_backdrop_target_level = 0xA;
    D_801B3134 = 0x12;
    D_801B3130++;
}

void func_800BB13C(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    s32 timer;

    timer = D_801B3134 - 1;
    D_801B3134 = timer;
    if (timer == 0)
    {
        D_801B3130++;
    }
}

void func_800BB170(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801ADAE0 = 1;
    g_wmap_backdrop_target_level = 5;
    D_801B3134 = 0xE;
    D_801B3130++;
}

void func_800BB1A8(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    s32 timer;

    timer = D_801B3134 - 1;
    D_801B3134 = timer;
    if (timer == 0)
    {
        D_801B3130++;
    }
}

void func_800BB1DC(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    s32 timer;

    timer = D_801B3134 - 1;
    D_801B3134 = timer;
    if (timer == 0)
    {
        D_801B3130++;
    }
}

void func_800BB210(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    wmap_start_sequence(func_800BC764);
    D_801B3134 = 4;
    D_801B3130++;
}

void func_800BB24C(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    s32 timer;

    timer = D_801B3134 - 1;
    D_801B3134 = timer;
    if (timer == 0)
    {
        D_801B3130++;
    }
}

void func_800BB280(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    wmap_start_sequence(func_800BCEA8);
    D_801B3134 = 0x5C;
    D_801B3130++;
}

void func_800BB2BC(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    s32 timer;

    timer = D_801B3134 - 1;
    D_801B3134 = timer;
    if (timer == 0)
    {
        D_801B3130++;
    }
}

void func_800BB2F0(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    wmap_start_sequence(func_800BD12C);
    D_801B3134 = 0x1E;
    D_801B3130++;
}

void func_800BB32C(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    s32 timer;

    timer = D_801B3134 - 1;
    D_801B3134 = timer;
    if (timer == 0)
    {
        D_801B3130++;
    }
}

void func_800BB360(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    s32 timer;

    timer = D_801B3134 - 1;
    D_801B3134 = timer;
    if (timer == 0)
    {
        D_801B3130++;
    }
}

void func_800BB394(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    wmap_start_sequence(func_800BCFD4);
    D_801B3134 = 0x5A;
    D_801B3130++;
}

void func_800BB3D0(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    s32 timer;

    timer = D_801B3134 - 1;
    D_801B3134 = timer;
    if (timer == 0)
    {
        D_801B3130++;
    }
}

void func_800BB404(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    wmap_start_sequence(func_800BC4B8);
    g_wmap_backdrop_target_level = 7;
    D_80139244 = 0;
    wmap_start_map_tint(0x703080);
    D_8013B29C = 0;
    wmap_start_sequence(func_800BC01C);
    wmap_start_sequence(func_800BC1BC);
    D_801B3134 = 4;
    D_801B3130++;
}

void func_800BB480(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    s32 timer;

    timer = D_801B3134 - 1;
    D_801B3134 = timer;
    if (timer == 0)
    {
        D_801B3130++;
    }
}

void func_800BB4B4(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    wmap_start_sequence(func_800BC964);
    D_801B3134 = 0xB7;
    D_801B3130++;
}

void func_800BB4F0(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    s32 timer;

    timer = D_801B3134 - 1;
    D_801B3134 = timer;
    if (timer == 0)
    {
        D_801B3130++;
    }
}

void func_800BB524(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    wmap_start_sequence(func_800BC60C);
    wmap_start_map_tint(0x808080);
    g_wmap_backdrop_target_level = 0x10;
    D_801B3134 = 0x3C;
    D_80139290[D_8011D510][D_8011D530].value = D_8011D4FC | 0x100;
    D_801B3130++;
}

void func_800BB5C0(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    s32 timer;

    timer = D_801B3134 - 1;
    D_801B3134 = timer;
    if (timer == 0)
    {
        D_801B3130++;
    }
}

void func_800BB5F4(void)
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
    D_801B3130++;
}

s32 func_800BB610(s32 reset)
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
        D_801B3138 = 1;
        D_801B313C = 1;
        return 1;
    }

    if ((u32)D_801B3138 >= 6)
    {
        return 0;
    }

    PS1_CALL(D_800D7774[D_801B3138])();
    return 1;
}

void func_800BB688(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3138 = 1;
    D_801B313C = 1;
}

void func_800BB6A0(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801399AC = &D_8011F538;
    D_800D9318.scale_index = 0xF;
    D_800D9318.previous_sequence = -1;
    D_800D9318.shade_step = 4;
    D_800D9318.target_shade = 0x81;
    D_800D9318.resource_index = 0;
    D_800D9318.sequence = 0;
    D_800D9318.shade = 1;
    D_801B313C = 0x88;
    D_801B3138++;
    func_800BB720();
}

void func_800BB720(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    s32 timer;

    wmap_step_actor_animation(&D_800D9318, &D_801399A8);
    wmap_draw_actor_sprite(&D_800D9318, g_wmap_focus_screen_position.packed, 0xB, 2, 0);
    timer = D_801B313C - 1;
    D_801B313C = timer;
    if (timer == 0)
    {
        D_801B3138++;
    }
}

void func_800BB79C(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_800D9318.target_shade = 0;
    D_800D9318.shade_step = 8;
    D_801B313C = 0x10;
    D_801B3138++;
    func_800BB7E8();
}

void func_800BB7E8(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    s32 timer;

    wmap_step_actor_animation(&D_800D9318, &D_801399A8);
    wmap_draw_actor_sprite(&D_800D9318, g_wmap_focus_screen_position.packed, 0xB, 2, 0);
    timer = D_801B313C - 1;
    D_801B313C = timer;
    if (timer == 0)
    {
        D_801B3138++;
    }
}

void func_800BB864(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3138++;
}

s32 func_800BB87C(s32 reset)
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
        D_801B3140 = 1;
        D_801B3144 = 1;
        return 1;
    }

    if ((u32)D_801B3140 >= 6)
    {
        return 0;
    }

    PS1_CALL(D_800D778C[D_801B3140])();
    return 1;
}

void func_800BB8F4(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3140 = 1;
    D_801B3144 = 1;
}

void func_800BB90C(void)
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

    D_801399B4 = &D_8011F538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.sequence = value = 1;
    D_800D9344.previous_sequence = -value;
    D_800D9344.shade_step = 8;
    D_800D9344.shade = value;
    D_800D9344.resource_index = 0;
    D_800D9344.target_shade = 0x81;
    D_801B3144 = 0x88;
    D_801B3140++;
    func_800BB98C();
}

void func_800BB98C(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    s32 timer;

    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0xB, 2, 0);
    timer = D_801B3144 - 1;
    D_801B3144 = timer;
    if (timer == 0)
    {
        D_801B3140++;
    }
}

void func_800BBA08(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_800D9344.shade_step = 8;
    D_800D9344.target_shade = 0;
    D_801B3144 = 0x10;
    D_801B3140++;
    func_800BBA54();
}

void func_800BBA54(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    s32 timer;

    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0xB, 2, 0);
    timer = D_801B3144 - 1;
    D_801B3144 = timer;
    if (timer == 0)
    {
        D_801B3140++;
    }
}

void func_800BBAD0(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3140++;
}

s32 func_800BBAE8(s32 reset)
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
        D_801B3148 = 1;
        D_801B314C = 1;
        return 1;
    }

    if ((u32)D_801B3148 >= 6)
    {
        return 0;
    }

    PS1_CALL(D_800D77A4[D_801B3148])();
    return 1;
}

void func_800BBB60(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3148 = 1;
    D_801B314C = 1;
}

void func_800BBB78(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801399BC = &D_8011F538;
    D_800D9370.scale_index = 0xF;
    D_800D9370.sequence = 2;
    D_800D9370.previous_sequence = -1;
    D_800D9370.shade_step = 8;
    D_800D9370.resource_index = 0;
    D_800D9370.target_shade = 0x81;
    D_800D9370.shade = 0x81;
    D_801B314C = 0x88;
    D_801B3148++;
    func_800BBBF8();
}

void func_800BBBF8(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    s32 timer;

    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, g_wmap_focus_screen_position.packed, 0xB, 2, 0);
    timer = D_801B314C - 1;
    D_801B314C = timer;
    if (timer == 0)
    {
        D_801B3148++;
    }
}

void func_800BBC74(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_800D9370.shade_step = 2;
    D_800D9370.target_shade = 0;
    D_801B314C = 0x40;
    D_801B3148++;
    func_800BBCC0();
}

void func_800BBCC0(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    s32 timer;

    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, g_wmap_focus_screen_position.packed, 0xB, 2, 0);
    timer = D_801B314C - 1;
    D_801B314C = timer;
    if (timer == 0)
    {
        D_801B3148++;
    }
}

void func_800BBD3C(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3148++;
}

s32 func_800BBD54(s32 reset)
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
        D_801B3150 = 1;
        D_801B3154 = 1;
        return 1;
    }

    if ((u32)D_801B3150 >= 6)
    {
        return 0;
    }

    PS1_CALL(D_800D77BC[D_801B3150])();
    return 1;
}

void func_800BBDCC(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3150 = 1;
    D_801B3154 = 1;
}

void func_800BBDE4(void)
{
    s32 i;
    s32 value;
    s32 config_index;
    s32 loop_value;
    WmapConfigA* config;
    WmapConfigA* configs;

    i = 0xAA;
    value = -1;
    config = &D_800D939C;
    configs = (WmapConfigA*)((u8*)config - 0x134);
    D_801399C4 = &D_8011F538;
    D_80139268 = 0x1E;
    config->field_06 = 0xF;
    config->field_0E = 3;
    config->field_10 = value;
    config->field_26 = 8;
    config->field_22 = 0x81;
    loop_value = value;
    config->field_02 = 0;
    config->field_24 = 1;
    for (i = 0xAA; i < 0xB9; i++)
    {
        config_index = i + 4;
        configs[config_index].field_02 = loop_value;
    }
    D_801B3154 = 0xAA;
    D_801B3150++;
    func_800BBE9C();
}

void func_800BBE9C(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    s32 timer;

    wmap_step_actor_animation(&D_800D939C, &D_801399C0);
    wmap_draw_actor_sprite(&D_800D939C, g_wmap_focus_screen_position.packed, 0xB, 2, 0);
    if (D_80139268 > 0)
    {
        func_800BA408();
    }
    timer = D_801B3154 - 1;
    D_80139268--;
    D_801B3154 = timer;
    if (timer == 0)
    {
        D_801B3150++;
    }
}

void func_800BBF3C(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_800D939C.shade_step = 8;
    D_800D939C.target_shade = 0;
    D_801B3154 = 0x10;
    D_801B3150++;
    func_800BBF88();
}

void func_800BBF88(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    s32 timer;

    wmap_step_actor_animation(&D_800D939C, &D_801399C0);
    wmap_draw_actor_sprite(&D_800D939C, g_wmap_focus_screen_position.packed, 0xB, 2, 0);
    timer = D_801B3154 - 1;
    D_801B3154 = timer;
    if (timer == 0)
    {
        D_801B3150++;
    }
}

void func_800BC004(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3150++;
}

s32 func_800BC01C(s32 reset)
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
        D_801B3158 = 1;
        D_801B315C = 1;
        return 1;
    }

    if ((u32)D_801B3158 >= 4)
    {
        return 0;
    }

    PS1_CALL(D_800D77D4[D_801B3158])();
    return 1;
}

void func_800BC094(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3158 = 1;
    D_801B315C = 1;
}

void func_800BC0AC(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801399B4 = &D_8011D538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.previous_sequence = -1;
    D_800D9344.shade_step = 8;
    D_800D9344.resource_index = 0;
    D_800D9344.sequence = 0;
    D_800D9344.target_shade = 0x80;
    D_800D9344.shade = 0;
    D_801B315C = 0xDC;
    D_801B3158++;
    func_800BC128();
}

void func_800BC128(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    s32 timer;

    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0x1F, 0xC, 0);
    timer = D_801B315C - 1;
    D_801B315C = timer;
    if (timer == 0)
    {
        D_801B3158++;
    }
}

void func_800BC1A4(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3158++;
}

s32 func_800BC1BC(s32 reset)
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
        D_801B3160 = 1;
        D_801B3164 = 1;
        return 1;
    }

    if ((u32)D_801B3160 >= 4)
    {
        return 0;
    }

    PS1_CALL(D_800D77E4[D_801B3160])();
    return 1;
}

void func_800BC234(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3160 = 1;
    D_801B3164 = 1;
}

void func_800BC24C(void)
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

    D_801399D4 = &D_8011D538;
    D_800D93F4.scale_index = 0xF;
    D_800D93F4.sequence = value = 1;
    D_800D93F4.previous_sequence = -value;
    D_800D93F4.shade_step = 8;
    D_800D93F4.shade = value;
    D_800D93F4.resource_index = 0;
    D_800D93F4.target_shade = 0x81;
    D_801B3164 = 0xBD;
    D_801B3160++;
    func_800BC2CC();
}

void func_800BC2CC(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    s32 timer;

    wmap_step_actor_animation(&D_800D93F4, &D_801399D0);
    wmap_draw_actor_sprite(&D_800D93F4, g_wmap_focus_screen_position.packed, 0x2D, 0x1E, 0);
    timer = D_801B3164 - 1;
    D_801B3164 = timer;
    if (timer == 0)
    {
        D_801B3160++;
    }
}

void func_800BC348(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3160++;
}

s32 func_800BC360(s32 reset)
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
        D_801B3168 = 1;
        D_801B316C = 1;
        return 1;
    }

    if ((u32)D_801B3168 >= 4)
    {
        return 0;
    }

    PS1_CALL(D_800D77F4[D_801B3168])();
    return 1;
}

void func_800BC3D8(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3168 = 1;
    D_801B316C = 1;
}

void func_800BC3F0(void)
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
    D_801B316C = 0x40;
    D_801B3168++;
    func_800BA580();
}

void func_800BC4A0(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3168++;
}

s32 func_800BC4B8(s32 reset)
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
        D_801B3170 = 1;
        D_801B3174 = 1;
        return 1;
    }

    if ((u32)D_801B3170 >= 4)
    {
        return 0;
    }

    PS1_CALL(D_800D7804[D_801B3170])();
    return 1;
}

void func_800BC530(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3170 = 1;
    D_801B3174 = 1;
}

void func_800BC548(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B24A8 = D_80139258;
    D_801B2478 = g_wmap_camera_translation;
    D_80182DEC = 0x80;
    D_801B2478.vz = 0xAFC8;
    D_801B3174 = 0x80;
    D_801B3170++;
    func_800BA66C();
}

void func_800BC5F4(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3170++;
}

s32 func_800BC60C(s32 reset)
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
        D_801B3178 = 1;
        D_801B317C = 1;
        return 1;
    }

    if ((u32)D_801B3178 >= 4)
    {
        return 0;
    }

    PS1_CALL(D_800D7814[D_801B3178])();
    return 1;
}

void func_800BC684(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3178 = 1;
    D_801B317C = 1;
}

void func_800BC69C(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_8013B238 = D_80139258;
    D_80139870 = g_wmap_camera_translation;
    D_80182DF0 = 0x80;
    D_80139870.vz = 0xAFC8;
    D_801B317C = 0x10;
    D_801B3178++;
    func_800BA758();
}

void func_800BC74C(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3178++;
}

s32 func_800BC764(s32 reset)
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
        D_801B3180 = 1;
        D_801B3184 = 1;
        return 1;
    }

    if ((u32)D_801B3180 >= 6)
    {
        return 0;
    }

    PS1_CALL(D_800D7824[D_801B3180])();
    return 1;
}

void func_800BC7DC(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3180 = 1;
    D_801B3184 = 1;
}

void func_800BC7F4(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    s32 timer;

    func_8006AEE0();
    func_8006A2FC(D_800D95D8, D_80139A28, 0x3C, 0xFF, 1, 2, 0, (WmapState*)D_80139280);
    timer = D_801B3184 - 1;
    D_801B3184 = timer;
    if (timer == 0)
    {
        D_801B3180++;
    }
}

void func_800BC87C(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3184 = 0x80;
    D_80139280[5] = -1;
    D_801B3180++;
    func_800BC8C4();
}

void func_800BC8C4(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    s32 timer;

    func_8006AEE0();
    func_8006A2FC(D_800D95D8, D_80139A28, 0x3C, 0xFF, 1, 2, 0, (WmapState*)D_80139280);
    timer = D_801B3184 - 1;
    D_801B3184 = timer;
    if (timer == 0)
    {
        D_801B3180++;
    }
}

void func_800BC94C(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3180++;
}

s32 func_800BC964(s32 reset)
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
        D_801B3188 = 1;
        D_801B318C = 1;
        return 1;
    }

    if ((u32)D_801B3188 >= 6)
    {
        return 0;
    }

    PS1_CALL(D_800D783C[D_801B3188])();
    return 1;
}

void func_800BC9DC(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3188 = 1;
    D_801B318C = 1;
}

void func_800BC9F4(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    s32 timer;

    func_8006AEE0();
    func_8006A2FC(D_800DA028, D_80139C08, 0x1E, 0xFF, 1, 4, 0, (u8*)D_80139280 + 0x28);
    timer = D_801B318C - 1;
    D_801B318C = timer;
    if (timer == 0)
    {
        D_801B3188++;
    }
}

void func_800BCA80(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B318C = 0x40;
    D_80139280[15] = -1;
    D_801B3188++;
    func_800BCAC8();
}

void func_800BCAC8(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    s32 timer;

    func_8006AEE0();
    func_8006A2FC(D_800DA028, D_80139C08, 0x1E, 0xFF, 1, 4, 0, (u8*)D_80139280 + 0x28);
    timer = D_801B318C - 1;
    D_801B318C = timer;
    if (timer == 0)
    {
        D_801B3188++;
    }
}

void func_800BCB54(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3188++;
}

s32 func_800BCB6C(s32 reset)
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
        D_801B3190 = 1;
        D_801B3194 = 1;
        return 1;
    }

    if ((u32)D_801B3190 >= 8)
    {
        return 0;
    }

    PS1_CALL(D_800D7854[D_801B3190])();
    return 1;
}

void func_800BCBE4(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3190 = 1;
    D_801B3194 = 1;
}

void func_800BCBFC(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    s32 timer;

    func_8006B328(0x78, 0x9C, 2, -1, -3, -8, 0, 0x1F, -0xB4, 0x190, -0xA0, 0x190, 0, 0x80, 0, 8, 2);
    D_801B25DC += 8;
    timer = D_801B3194 - 1;
    D_801B3194 = timer;
    if (timer == 0)
    {
        D_801B3190++;
    }
}

void func_800BCCB8(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3194 = 0x12;
    D_801B3190++;
    func_800BCCF0();
}

void func_800BCCF0(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    s32 timer;

    func_8006B328(0x78, 0x9C, 2, -1, -3, -8, 0, 0x1F, -0xB4, 0x190, -0xA0, 0x190, 0, 0x80, 0, 8, 2);
    timer = D_801B3194 - 1;
    D_801B3194 = timer;
    if (timer == 0)
    {
        D_801B3190++;
    }
}

void func_800BCDA0(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_800DCEB0 = 0;
    D_801B3194 = 0x64;
    D_801B3190++;
    func_800BCDE0();
}

void func_800BCDE0(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    s32 timer;

    func_8006B328(0x78, 0x9C, 2, -1, -3, -8, 0, 0x1F, -0xB4, 0x190, -0xA0, 0x190, 0, 0x80, 0, 8, 2);
    timer = D_801B3194 - 1;
    D_801B3194 = timer;
    if (timer == 0)
    {
        D_801B3190++;
    }
}

void func_800BCE90(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3190++;
}

s32 func_800BCEA8(s32 reset)
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
        D_801B3198 = 1;
        D_801B319C = 1;
        return 1;
    }

    if ((u32)D_801B3198 >= 6)
    {
        return 0;
    }

    PS1_CALL(D_800D7874[D_801B3198])();
    return 1;
}

void func_800BCF20(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3198 = 1;
    D_801B319C = 1;
}

void func_800BCF38(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_80139240 = 0;
    D_80182DF4 = 1;
    D_801B319C = 0x5A;
    D_801B3198++;
    func_800BAAB0();
}

void func_800BCF84(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B319C = 0x20;
    D_801B3198++;
    func_800BAB80();
}

void func_800BCFBC(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3198++;
}

s32 func_800BCFD4(s32 reset)
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
        D_801B31A0 = 1;
        D_801B31A4 = 1;
        return 1;
    }

    if ((u32)D_801B31A0 >= 6)
    {
        return 0;
    }

    PS1_CALL(D_800D788C[D_801B31A0])();
    return 1;
}

void func_800BD04C(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B31A0 = 1;
    D_801B31A4 = 1;
}

void func_800BD064(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_8013924C = 0;
    D_801B2670 = D_80139258;
    D_801B25D8 = 0;
    D_801B31A4 = 0x58;
    D_801B31A0++;
    func_800BAC44();
}

void func_800BD0DC(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B31A4 = 8;
    D_801B31A0++;
    func_800BAD38();
}

void func_800BD114(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B31A0++;
}

s32 func_800BD12C(s32 reset)
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
        D_801B31A8 = 1;
        D_801B31AC = 1;
        return 1;
    }

    if ((u32)D_801B31A8 >= 6)
    {
        return 0;
    }

    PS1_CALL(D_800D78A4[D_801B31A8])();
    return 1;
}

void func_800BD1A4(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B31A8 = 1;
    D_801B31AC = 1;
}

void func_800BD1BC(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801399DC = &D_80121538;
    D_80139250 = 0x780;
    D_800D9420.scale_index = 0xF;
    D_800D9420.previous_sequence = -1;
    D_800D9420.shade_step = 4;
    D_800D9420.target_shade = 0x81;
    D_800D9420.shade = 1;
    D_800D9420.resource_index = 0;
    D_800D9420.sequence = 0;
    D_801B31AC = 0xE1;
    D_80182D58.packed = g_wmap_focus_screen_position.packed;
    D_801B31A8++;
    func_800BD258();
}

void func_800BD258(void)
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
    s32 adjusted;
    s32 timer;

    wmap_step_actor_animation(&D_800D9420, &D_801399D8);
    wmap_draw_actor_sprite(&D_800D9420, D_80182D58.packed, 8, 0x14, 0);

    value = D_80139250;
    adjusted = value;
    if (value < 0)
    {
        adjusted = value + 0xF;
    }
    D_80182D58.point.y = adjusted >> 4;
    if (value >= 0x321)
    {
        D_80139250 = value - 0x10;
    }

    timer = D_801B31AC - 1;
    D_801B31AC = timer;
    if (timer == 0)
    {
        D_801B31A8++;
    }
}

void func_800BD308(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_800D9420.shade_step = 8;
    D_800D9420.target_shade = 0;
    D_801B31AC = 0x10;
    D_801B31A8++;
    func_800BD354();
}

void func_800BD354(void)
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
    s32 adjusted;
    s32 timer;

    wmap_step_actor_animation(&D_800D9420, &D_801399D8);
    wmap_draw_actor_sprite(&D_800D9420, D_80182D58.packed, 8, 0x14, 0);

    value = D_80139250;
    adjusted = value;
    if (value < 0)
    {
        adjusted = value + 0xF;
    }
    D_80182D58.point.y = adjusted >> 4;
    if (value >= 0x321)
    {
        D_80139250 = value - 0x10;
    }

    timer = D_801B31AC - 1;
    D_801B31AC = timer;
    if (timer == 0)
    {
        D_801B31A8++;
    }
}

void func_800BD404(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B31A8++;
}
