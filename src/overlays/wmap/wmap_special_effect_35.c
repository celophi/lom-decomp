#include "wmap_special_effect_35.h"
#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_effect_resources.h"
#include "cdrom.h"

void func_800C0CA4(WmapStateHead* state)
{
/* Partial WMAP decompilation: 93.679344% (gcc280_g0). */


typedef struct
{
    s16 field_00, field_02; s32 field_04, field_08; s16 field_0C, field_0E;
    u8 pad_10[4];
} WmapConfigB;
typedef struct
{
    s16 field_00, field_02; u8 pad_04[2]; u8 field_06; u8 pad_07[7];
    s16 field_0E, field_10; u8 pad_12[0x10]; s16 field_22, field_24, field_26;
    u8 pad_28[4];
} WmapConfigA;
extern u8 D_800D9268[];
extern u8 D_80139988[];
extern u8 D_801AFBD0[];
extern s32 rand(void);

    SVECTOR position;
    s32 screen_position;
    s32 index = state->field_00;
    WmapConfigB* config;
    s32 display_offset;
    s32 screen_offset;
    WmapConfigA* display;
    s32 draw_type;
    s32 radius;
    s32 base_radius;

    if (index < state->field_04)
    {
        u8* config_base;

        config_base = D_801AFBD0;
        config = (WmapConfigB*)((index * sizeof(WmapConfigB)) + (s32)config_base);
        display_offset = index * sizeof(WmapConfigA);
        do
        {
            if (config->field_00 != 0)
            {
                u8* display_base;

                display_base = D_800D9268;
                display = (WmapConfigA*)(display_offset + (s32)display_base);
                position.vx = (config->field_08 * (ccos(config->field_02) >> 5)) >> 0xC;
                position.vy = (config->field_08 * (csin(config->field_02) >> 5)) >> 0xC;
                position.vz = config->field_0E;
                gte_ldv0(&position);
                gte_rtps();
                screen_offset = index * 8;
                display_base = D_80139988;
                func_8006CC4C(display, (void*)(screen_offset + (s32)display_base));
                gte_stsxy(&screen_position);
                if (index < 0x78)
                {
                    func_80066F9C(display, screen_position, 0x16, 8, 0);
                }
                else
                {
                    func_80066F9C(display, screen_position, 0x19, 8, 0);
                }

                if (config->field_0C != 0)
                {
                    config->field_0C--;
                }
                else if (config->field_08 >= 0x579)
                {
                    config->field_08 -= config->field_04;
                }
                else if (display->field_24 >= 5)
                {
                    display->field_22 = 0;
                }
                else
                {
                    config->field_00 = 0;
                }
            }
            config++;
            index++;
            display_offset += sizeof(WmapConfigA);
        } while (index < state->field_04);
    }
    if (state->field_24 != 0)
    {
        index = state->field_00;
        if (index < state->field_04)
        {
            u8* free_base;

            free_base = D_800D9268;
            display = (WmapConfigA*)((index * sizeof(WmapConfigA)) + (s32)free_base);
            free_base = D_801AFBD0;
            config = (WmapConfigB*)((index * sizeof(WmapConfigB)) + (s32)free_base);
            do
            {
                index++;
                if (config->field_00 == 0)
                {
                    config->field_00 = 1;
                    display->field_06 = 0xF;
                    display->field_0E = 2;
                    display->field_02 = 0;
                    display->field_10 = -1;
                    display->field_26 = state->field_08;
                    display->field_22 = state->field_0C;
                    display->field_24 = state->field_10;
                    config->field_08 = state->field_14;
                    config->field_02 = rand() & 0xFFF;
                    radius = rand() * state->field_1C;
                    base_radius = state->field_18;
                    config->field_0E = 0;
                    config->field_04 = (radius >> 0xF) + base_radius;
                    config->field_0C = state->field_20;
                    return;
                }
                display++;
                config++;
            } while (index < state->field_04);
        }
    }
}

void func_800C0F84(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801ADAF4 = 0;
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
    D_801B3254 = 8;
    D_801B3250++;
}

void func_800C1000(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    func_8006CAC0(func_800C3188);
    func_8006CAC0(func_800C3660);
    func_8006CAC0(func_800C33F4);
    D_801ADAF4 = 0;
    D_80182D74.field_00 = 0x40;
    D_80182D74.field_01 = 0x10;
    D_80182D74.field_02 = 0x20;
    D_80182D80.field_00 = 0x40;
    D_80182D80.field_01 = 0x10;
    D_80182D80.field_02 = 0x20;
    D_80182D8C.field_00 = 0x40;
    D_80182D8C.field_01 = 0x10;
    D_80182D8C.field_02 = 0x20;
    D_80182D94.field_00 = 0x40;
    D_80182D94.field_01 = 0x10;
    D_80182D94.field_02 = 0x20;
    D_801B3254 = 0x18;
    D_801B3250++;
}

void func_800C10B8(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    s32 value = D_801B2650.vz - 0xDAC;

    D_801B2650.vz = value;
    if (value < 0x2710)
    {
        D_801B2650.vz = 0x2710;
    }
    PushMatrix();
    func_8006D150(&D_80139258);
    if (D_80182DE8 != 0)
    {
        func_800675F0(D_8011CF1C, 0, 4, 0x35, 0x7800, 1,
                      D_80182DE8, 0, 0, -1);
        value = D_80182DE8 - 4;
        D_80182DE8 = value;
        if (value < 0)
        {
            D_80182DE8 = 0;
        }
    }
    PopMatrix();
    if (--D_801B325C == 0)
    {
        D_801B3258++;
    }
}

void func_800C11A4(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    s32 value = D_801B2478.vz - 0xDAC;

    D_801B2478.vz = value;
    if (value < 0x2710)
    {
        D_801B2478.vz = 0x2710;
    }
    PushMatrix();
    func_8006D150(&D_80139258);
    if (D_80182DEC != 0)
    {
        func_800675F0(D_800DCF18, 0, 4, 0x35, 0x7800, 1,
                      D_80182DEC, 0, 0, -1);
        value = D_80182DEC - 1;
        D_80182DEC = value;
        if (value < 0)
        {
            D_80182DEC = 0;
        }
    }
    PopMatrix();
    if (--D_801B3264 == 0)
    {
        D_801B3260++;
    }
}

void func_800C1290(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    s32 value;

    PushMatrix();
    func_8006CFA8(&D_80182DC0, &D_8013B238);
    func_800675F0(D_8011CF24, (D_8013923C >> 4) & 3, 0xA, 0x35,
                  0x7800, 0x1001, D_80182DF0, 0, 0, -1);
    value = D_80182DF0 + 2;
    D_80182DF0 = value;
    if (value >= 0x82)
    {
        D_80182DF0 = 0x81;
    }
    D_8013923C += 8;
    ((WmapPair16*)&D_8013B238)->field_04 += 0xA;
    PopMatrix();
    if (--D_801B3284 == 0)
    {
        D_801B3280++;
    }
}

void func_800C1390(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    s32 value;

    PushMatrix();
    func_8006CFA8(&D_80182DC0, &D_8013B238);
    func_800675F0(D_8011CF24, (D_8013923C >> 4) & 3, 0xA, 0x35,
                  0x7800, 0x1001, D_80182DF0, 0, 0, -1);
    value = D_80182DF0 - 4;
    D_80182DF0 = value;
    if (value < 0)
    {
        D_80182DF0 = 0;
    }
    D_8013923C += 8;
    ((WmapPair16*)&D_8013B238)->field_04 += 0xA;
    PopMatrix();
    if (--D_801B3284 == 0)
    {
        D_801B3280++;
    }
}

void func_800C1488(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    s32 value;

    PushMatrix();
    func_8006CFA8(&D_80182DC0, &D_8013B240);
    func_800675F0(D_8011CF28, (D_80139260 >> 4) & 7, 0xA, 0x36,
                  0x7880, 0x1001, D_80182DF4, 0, 0, -1);
    value = D_80182DF4 + 2;
    D_80182DF4 = value;
    if (value >= 0x62)
    {
        D_80182DF4 = 0x61;
    }
    D_80139260 -= 0x10;
    ((WmapPair16*)&D_8013B240)->field_04 += 4;
    PopMatrix();
    if (--D_801B328C == 0)
    {
        D_801B3288++;
    }
}

void func_800C1588(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    s32 value;

    PushMatrix();
    func_8006CFA8(&D_80182DC0, &D_8013B240);
    func_800675F0(D_8011CF28, (D_80139260 >> 4) & 7, 0xA, 0x36,
                  0x7880, 0x1001, D_80182DF4, 0, 0, -1);
    value = D_80182DF4 - 8;
    D_80182DF4 = value;
    if (value < 0)
    {
        D_80182DF4 = 0;
    }
    D_80139260 -= 0x10;
    ((WmapPair16*)&D_8013B240)->field_04 += 4;
    PopMatrix();
    if (--D_801B328C == 0)
    {
        D_801B3288++;
    }
}

/**
 * @brief Project, draw, and update the world-map entry backed by D_801AFC70.
 */
void func_800C1680(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    SVECTOR position;
    s32 screen_position;
    WmapConfigA* display = &D_800D93C8;
    u8* screen_entry;
    position.vx = (D_801AFC70.field_08 * (ccos(D_801AFC70.field_02) >> 5)) >> 0xC;
    position.vy = (D_801AFC70.field_08 * (csin(D_801AFC70.field_02) >> 5)) >> 0xC;
    position.vz = D_801AFC70.field_0E;
    gte_ldv0(&position);
    gte_rtps();
    screen_entry = D_80139988;
    func_8006CC4C(display, screen_entry + 0x40);
    gte_stsxy(&screen_position);
    func_80066F9C(display, screen_position, 0x16, 8, 0);
    if (D_801AFC70.field_0C != 0)
    {
        D_801AFC70.field_0C--;
    }
    else if (D_801AFC70.field_08 >= 0x579)
    {
        D_801AFC70.field_08 -= D_801AFC70.field_04;
    }
    else if (display->field_24 >= 5)
    {
        display->field_22 = 0;
    }
    else
    {
        D_801AFC70.field_00 = 0;
    }
    if (--D_801B3294 == 0)
    {
        D_801B3290++;
    }
}

/**
 * @brief Project, draw, and update the world-map entry backed by D_801AFC84.
 */
void func_800C17E0(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    SVECTOR position;
    s32 screen_position;
    WmapConfigA* display = &D_800D93F4;
    u8* screen_entry;
    position.vx = (D_801AFC84.field_08 * (ccos(D_801AFC84.field_02) >> 5)) >> 0xC;
    position.vy = (D_801AFC84.field_08 * (csin(D_801AFC84.field_02) >> 5)) >> 0xC;
    position.vz = D_801AFC84.field_0E;
    gte_ldv0(&position);
    gte_rtps();
    screen_entry = D_80139988;
    func_8006CC4C(display, screen_entry + 0x48);
    gte_stsxy(&screen_position);
    func_80066F9C(display, screen_position, 0x16, 8, 0);
    if (D_801AFC84.field_0C != 0)
    {
        D_801AFC84.field_0C--;
    }
    else if (D_801AFC84.field_08 >= 0x579)
    {
        D_801AFC84.field_08 -= D_801AFC84.field_04;
    }
    else if (display->field_24 >= 5)
    {
        display->field_22 = 0;
    }
    else
    {
        D_801AFC84.field_00 = 0;
    }
    if (--D_801B329C == 0)
    {
        D_801B3298++;
    }
}

/**
 * @brief Project, draw, and update the world-map entry backed by D_801AFC98.
 */
void func_800C1940(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    SVECTOR position;
    s32 screen_position;
    WmapConfigA* display = &D_800D9420;
    u8* screen_entry;
    position.vx = (D_801AFC98.field_08 * (ccos(D_801AFC98.field_02) >> 5)) >> 0xC;
    position.vy = (D_801AFC98.field_08 * (csin(D_801AFC98.field_02) >> 5)) >> 0xC;
    position.vz = D_801AFC98.field_0E;
    gte_ldv0(&position);
    gte_rtps();
    screen_entry = D_80139988;
    func_8006CC4C(display, screen_entry + 0x50);
    gte_stsxy(&screen_position);
    func_80066F9C(display, screen_position, 0x16, 8, 0);
    if (D_801AFC98.field_0C != 0)
    {
        D_801AFC98.field_0C--;
    }
    else if (D_801AFC98.field_08 >= 0x579)
    {
        D_801AFC98.field_08 -= D_801AFC98.field_04;
    }
    else if (display->field_24 >= 5)
    {
        display->field_22 = 0;
    }
    else
    {
        D_801AFC98.field_00 = 0;
    }
    if (--D_801B32A4 == 0)
    {
        D_801B32A0++;
    }
}

/**
 * @brief Project, draw, and update the world-map entry backed by D_801AFCE8.
 */
void func_800C1AA0(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    SVECTOR position;
    s32 screen_position;
    WmapConfigA* display = &D_800D94D0;
    u8* screen_entry;
    position.vx = (D_801AFCE8.field_08 * (ccos(D_801AFCE8.field_02) >> 5)) >> 0xC;
    position.vy = (D_801AFCE8.field_08 * (csin(D_801AFCE8.field_02) >> 5)) >> 0xC;
    position.vz = D_801AFCE8.field_0E;
    gte_ldv0(&position);
    gte_rtps();
    screen_entry = D_80139988;
    func_8006CC4C(display, screen_entry + 0x70);
    gte_stsxy(&screen_position);
    func_80066F9C(display, screen_position, 0x16, 8, 0);
    if (D_801AFCE8.field_0C != 0)
    {
        D_801AFCE8.field_0C--;
    }
    else if (D_801AFCE8.field_08 >= 0x579)
    {
        D_801AFCE8.field_08 -= D_801AFCE8.field_04;
    }
    else if (display->field_24 >= 5)
    {
        display->field_22 = 0;
    }
    else
    {
        D_801AFCE8.field_00 = 0;
    }
    if (--D_801B32AC == 0)
    {
        D_801B32A8++;
    }
}

/**
 * @brief Project, draw, and update the world-map entry backed by D_801AFCFC.
 */
void func_800C1C00(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    SVECTOR position;
    s32 screen_position;
    WmapConfigA* display = &D_800D94FC;
    u8* screen_entry;
    position.vx = (D_801AFCFC.field_08 * (ccos(D_801AFCFC.field_02) >> 5)) >> 0xC;
    position.vy = (D_801AFCFC.field_08 * (csin(D_801AFCFC.field_02) >> 5)) >> 0xC;
    position.vz = D_801AFCFC.field_0E;
    gte_ldv0(&position);
    gte_rtps();
    screen_entry = D_80139988;
    func_8006CC4C(display, screen_entry + 0x78);
    gte_stsxy(&screen_position);
    func_80066F9C(display, screen_position, 0x16, 8, 0);
    if (D_801AFCFC.field_0C != 0)
    {
        D_801AFCFC.field_0C--;
    }
    else if (D_801AFCFC.field_08 >= 0x579)
    {
        D_801AFCFC.field_08 -= D_801AFCFC.field_04;
    }
    else if (display->field_24 >= 5)
    {
        display->field_22 = 0;
    }
    else
    {
        D_801AFCFC.field_00 = 0;
    }
    if (--D_801B32B4 == 0)
    {
        D_801B32B0++;
    }
}

/**
 * @brief Project, draw, and update the world-map entry backed by D_801AFD10.
 */
void func_800C1D60(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    SVECTOR position;
    s32 screen_position;
    WmapConfigA* display = &D_800D9528;
    u8* screen_entry;
    position.vx = (D_801AFD10.field_08 * (ccos(D_801AFD10.field_02) >> 5)) >> 0xC;
    position.vy = (D_801AFD10.field_08 * (csin(D_801AFD10.field_02) >> 5)) >> 0xC;
    position.vz = D_801AFD10.field_0E;
    gte_ldv0(&position);
    gte_rtps();
    screen_entry = D_80139988;
    func_8006CC4C(display, screen_entry + 0x80);
    gte_stsxy(&screen_position);
    func_80066F9C(display, screen_position, 0x16, 8, 0);
    if (D_801AFD10.field_0C != 0)
    {
        D_801AFD10.field_0C--;
    }
    else if (D_801AFD10.field_08 >= 0x579)
    {
        D_801AFD10.field_08 -= D_801AFD10.field_04;
    }
    else if (display->field_24 >= 5)
    {
        display->field_22 = 0;
    }
    else
    {
        D_801AFD10.field_00 = 0;
    }
    if (--D_801B32BC == 0)
    {
        D_801B32B8++;
    }
}

/**
 * @brief Project, draw, and update the world-map entry backed by D_801AFD24.
 */
void func_800C1EC0(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    SVECTOR position;
    s32 screen_position;
    WmapConfigA* display = &D_800D9554;
    u8* screen_entry;

    position.vx = (D_801AFD24.field_08 * (ccos(D_801AFD24.field_02) >> 5)) >> 0xC;
    position.vy = (D_801AFD24.field_08 * (csin(D_801AFD24.field_02) >> 5)) >> 0xC;
    position.vz = D_801AFD24.field_0E;
    gte_ldv0(&position);
    gte_rtps();
    screen_entry = D_80139988;
    func_8006CC4C(display, screen_entry + 0x88);
    gte_stsxy(&screen_position);
    func_80066F9C(display, screen_position, 0x16, 8, 0);
    if (D_801AFD24.field_0C != 0)
    {
        D_801AFD24.field_0C--;
    }
    else if (D_801AFD24.field_08 >= 0x579)
    {
        D_801AFD24.field_08 -= D_801AFD24.field_04;
    }
    else if (display->field_24 >= 5)
    {
        display->field_22 = 0;
    }
    else
    {
        D_801AFD24.field_00 = 0;
    }
    if (--D_801B32C4 == 0)
    {
        D_801B32C0++;
    }
}

void func_800C2020(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    s32 index = 0x14;
    u8* resource = &D_8011D538;
    u8* base;
    WmapConfigB* config;
    WmapPointerPair* entry;
    WmapState* state;
    s32 next_state;
    register s32 one __asm__("$6");

    base = (u8*)D_801AFBD0;
    config = (WmapConfigB*)(base + 0x190);
    base = D_80139988;
    entry = (WmapPointerPair*)(base + 0xA0);
    do
    {
        entry->field_04 = resource;
        config->field_00 = 0;
        config++;
        index++;
        entry++;
    } while (index < 0x32);
    *(s32*)((u8*)D_80139280 + 0x00) = 0x14;
    one = 1;
    D_801B32CC = 0x90;
    state = D_80139280;
    state->field_04 = 0x32;
    state->field_08 = 0x10;
    state->field_0C = 0x81;
    state->field_14 = 0x2EE0;
    state->field_18 = 0x64;
    state->field_1C = 0x64;
    next_state = D_801B32C8 + one;
    state->field_10 = one;
    state->field_20 = 2;
    state->state_24 = one;
    D_801B32C8 = next_state;
    func_800C42F0();
}

void func_800C20EC(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    s32 index = 0x78;
    u8* resource = &D_8011F538;
    u8* base;
    WmapConfigB* config;
    WmapPointerPair* entry;
    s32 next_state;

    base = (u8*)D_801AFBD0;
    config = (WmapConfigB*)(base + 0x960);
    base = D_80139988;
    entry = (WmapPointerPair*)(base + 0x3C0);
    do
    {
        entry->field_04 = resource;
        config->field_00 = 0;
        config++;
        index++;
        entry++;
    } while (index < 0xDC);
    index = 0x78;
    do
    {
        D_800D9268[index].field_06 = 0xF;
        D_800D9268[index].field_0E = 2;
        D_800D9268[index].field_10 = -1;
        D_800D9268[index].field_26 = 0x10;
        D_800D9268[index].field_22 = 0x81;
        D_800D9268[index].field_02 = 0;
        D_800D9268[index].field_24 = 1;
        D_801AFBD0[index].field_08 = 0x2710;
        D_801AFBD0[index].field_02 = rand() & 0xFFF;
        D_801AFBD0[index].field_04 = ((rand() * 0x32) >> 0xF) + 0x32;
        D_801AFBD0[index].field_0E = 0;
        D_801AFBD0[index].field_0C = 0x26;
        index++;
    } while (index < 0x82);
    D_801B32D4 = 0xF0;
    D_80139280->field_28 = 0x78;
    D_80139280->field_2C = 0xDC;
    D_80139280->field_30 = 0x10;
    D_80139280->field_34 = 0x81;
    D_80139280->field_40 = 0x32;
    D_80139280->field_44 = 0x32;
    next_state = D_801B32D0 + 1;
    D_80139280->field_38 = 1;
    D_80139280->field_3C = 0x2710;
    D_80139280->field_48 = 2;
    D_80139280->tail_state = 1;
    D_801B32D0 = next_state;
    func_800C4484();
}

s32 func_800C2274(s32 reset)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (reset != 0)
    {
        D_801B3248 = 1;
        D_801B324C = 1;
        return 1;
    }
    if ((u32)D_801B3248 >= 4)
    {
        return 0;
    }
    D_800D7ADC[D_801B3248]();
    return 1;
}

void func_800C22EC(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B3248 = 1;
    D_801B324C = 1;
}

void func_800C2304(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_80139978 = 0x1F;
    func_800A89DC(0x23);
    cdrom_wait_queue_empty();
    func_800651B4(&D_80182E40);
    func_800651B4(&D_8018B240);
    func_800651B4(&D_80193640);
    func_8006D190();
    func_8006CAC0(func_800C23F8);
    D_8013B20C = 1;
    D_801B3248++;
    func_800C2390();
}

void func_800C2390(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (D_8013B20C == 0)
    {
        D_801B3248++;
        func_800C23CC();
    }
}

void func_800C23CC(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_8013B294 = 1;
    D_80139228 = 2;
    D_801B3248++;
}

s32 func_800C23F8(s32 reset)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (reset != 0)
    {
        D_801B3250 = 1;
        D_801B3254 = 1;
        return 1;
    }

    if ((u32)D_801B3250 >= 0x26)
    {
        return 0;
    }

    D_800D7AEC[D_801B3250]();
    return 1;
}

void func_800C2470(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B3250 = 1;
    D_801B3254 = 1;
}

void func_800C2488(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_8013B208 = 1;
    D_801ADAE0 = 1;
    func_8006D0F0(0x1F, &D_800DCEF8, &D_800DCF00);
    D_801398D0 = 2;
    D_80182D68 = ((D_800DCEF8 - 1) * 0x30) - D_80139950.field_00;
    D_80182D78 = ((D_800DCF00 - 1) * 0x30) - D_80139950.field_04;
    D_801B3250++;
    func_800C2548();
}

void func_800C2548(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (D_801398D0 != 2)
    {
        D_801B3250++;
        func_800C2588();
    }
}

void func_800C2588(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    func_800652A8(0x3A, 0x80);
    func_8006CAC0(func_800C2F1C);
    D_801B3254 = 0x14;
    D_801B3250++;
}

void func_800C25D0(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (--D_801B3254 == 0)
    {
        D_801B3250++;
    }
}

void func_800C2604(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_800DBE70 = 1;
    D_801B3254 = 0x2D;
    D_801B3250++;
}

void func_800C2630(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (--D_801B3254 == 0)
    {
        D_801B3250++;
    }
}

void func_800C2664(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    func_8006CAC0(func_800C2CD0);
    D_800DBE70 = 0;
    D_801ADAF4 = 3;
    func_8006683C(0x801530);
    D_801B3254 = 0x64;
    D_801B3250++;
}

void func_800C26C0(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (--D_801B3254 == 0)
    {
        D_801B3250++;
    }
}

void func_800C26F4(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    func_8006CAC0(func_800C3918);
    D_801B3254 = 0x64;
    D_801B3250++;
}

void func_800C2730(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (--D_801B3254 == 0)
    {
        D_801B3250++;
    }
}

void func_800C2764(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    func_8006CAC0(func_800C3A68);
    D_801B3254 = 0x14;
    D_801B3250++;
}

void func_800C27A0(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (--D_801B3254 == 0)
    {
        D_801B3250++;
    }
}

void func_800C27D4(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    func_8006CAC0(func_800C3BBC);
    D_801B3254 = 0xF;
    D_801B3250++;
}

void func_800C2810(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (--D_801B3254 == 0)
    {
        D_801B3250++;
    }
}

void func_800C2844(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    func_8006CAC0(func_800C3D10);
    D_801B3254 = 0xF;
    D_801B3250++;
}

void func_800C2880(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (--D_801B3254 == 0)
    {
        D_801B3250++;
    }
}

void func_800C28B4(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    func_8006CAC0(func_800C3E64);
    D_801B3254 = 0xA;
    D_801B3250++;
}

void func_800C28F0(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (--D_801B3254 == 0)
    {
        D_801B3250++;
    }
}

void func_800C2924(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    func_8006CAC0(func_800C3FB8);
    D_801B3254 = 5;
    D_801B3250++;
}

void func_800C2960(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (--D_801B3254 == 0)
    {
        D_801B3250++;
    }
}

void func_800C2994(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    func_8006CAC0(func_800C410C);
    D_801ADAF4 = 0;
    D_801B3254 = 8;
    D_801B3250++;
}

void func_800C29D8(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (--D_801B3254 == 0)
    {
        D_801B3250++;
    }
}

void func_800C2A0C(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    func_8006CAC0(func_800C4260);
    D_801B3254 = 0x90;
    D_801B3250++;
}

void func_800C2A48(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (--D_801B3254 == 0)
    {
        D_801B3250++;
    }
}

void func_800C2A7C(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (--D_801B3254 == 0)
    {
        D_801B3250++;
    }
}

void func_800C2AB0(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    func_8006CAC0(func_800C2DF8);
    D_80139244 = 1;
    D_80139978 = -1;
    D_8013986C = -1;
    D_801B3254 = 2;
    D_801B3250++;
}

void func_800C2B0C(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (--D_801B3254 == 0)
    {
        D_801B3250++;
    }
}

void func_800C2B40(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    func_8006CAC0(func_800C43F4);
    D_801B3254 = 0xD;
    D_801B3250++;
}

void func_800C2B7C(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (--D_801B3254 == 0)
    {
        D_801B3250++;
    }
}

void func_800C2BB0(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (--D_801B3254 == 0)
    {
        D_801B3250++;
    }
}

void func_800C2BE4(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    func_8006CAC0(func_800C37BC);
    D_801B3254 = 0x7C;
    D_801B3250++;
}

void func_800C2C20(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (--D_801B3254 == 0)
    {
        D_801B3250++;
    }
}

void func_800C2C54(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_8011D500 = 0xFE;
    D_801B3254 = 0x80;
    D_801B3250++;
}

void func_800C2C80(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (--D_801B3254 == 0)
    {
        D_801B3250++;
    }
}

void func_800C2CB4(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_8013B20C = 0;
    D_801B3250++;
}

s32 func_800C2CD0(s32 reset)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (reset != 0)
    {
        D_801B3258 = 1;
        D_801B325C = 1;
        return 1;
    }

    if ((u32)D_801B3258 >= 4)
    {
        return 0;
    }

    D_800D7B84[D_801B3258]();
    return 1;
}

void func_800C2D48(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B3258 = 1;
    D_801B325C = 1;
}

void func_800C2D60(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B2650 = D_80182DC0;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    D_801B325C = 0x20;
    D_801B3258++;
    func_800C10B8();
}

void func_800C2DE0(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B3258++;
}

s32 func_800C2DF8(s32 reset)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (reset != 0)
    {
        D_801B3260 = 1;
        D_801B3264 = 1;
        return 1;
    }

    if ((u32)D_801B3260 >= 4)
    {
        return 0;
    }

    D_800D7B94[D_801B3260]();
    return 1;
}

void func_800C2E70(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B3260 = 1;
    D_801B3264 = 1;
}

void func_800C2E88(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B2478 = D_80182DC0;
    D_80182DEC = 0x80;
    D_801B2478.vz = 0xAFC8;
    D_801B3264 = 0x80;
    D_801B3260++;
    func_800C11A4();
}

void func_800C2F04(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B3260++;
}

s32 func_800C2F1C(s32 reset)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (reset != 0)
    {
        D_801B3268 = 1;
        D_801B326C = 1;
        return 1;
    }

    if ((u32)D_801B3268 >= 6)
    {
        return 0;
    }

    D_800D7BA4[D_801B3268]();
    return 1;
}

void func_800C2F94(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B3268 = 1;
    D_801B326C = 1;
}

void func_800C2FAC(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801399B4 = &D_80121538;
    D_800D9344.field_06 = 0xF;
    D_800D9344.field_10 = -1;
    D_800D9344.field_26 = 2;
    D_800D9344.field_22 = 0x81;
    D_800D9344.field_02 = 0;
    D_800D9344.field_0E = 0;
    D_800D9344.field_24 = 1;
    D_801B326C = 0x1F6;
    D_801B3268++;
    func_800C302C();
}

void func_800C302C(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    func_8006CC4C(&D_800D9344, &D_801399B0);
    func_80066F9C(&D_800D9344, D_8011CF4C, 8, 2, 0);
    if (--D_801B326C == 0)
    {
        D_801B3268++;
    }
}

void func_800C30A8(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_800D9344.field_26 = 8;
    D_800D9344.field_22 = 0;
    D_801B326C = 0x10;
    D_801B3268++;
    func_800C30F4();
}

void func_800C30F4(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    func_8006CC4C(&D_800D9344, &D_801399B0);
    func_80066F9C(&D_800D9344, D_8011CF4C, 8, 2, 0);
    if (--D_801B326C == 0)
    {
        D_801B3268++;
    }
}

void func_800C3170(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B3268++;
}

s32 func_800C3188(s32 reset)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (reset != 0)
    {
        D_801B3270 = 1;
        D_801B3274 = 1;
        return 1;
    }

    if ((u32)D_801B3270 >= 6)
    {
        return 0;
    }

    D_800D7BBC[D_801B3270]();
    return 1;
}

void func_800C3200(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B3270 = 1;
    D_801B3274 = 1;
}

void func_800C3218(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    s32 value;

    D_801399BC = &D_8011F538;
    D_800D9370.field_06 = 0xF;
    D_800D9370.field_0E = value = 1;
    D_800D9370.field_10 = -value;
    D_800D9370.field_26 = 0x80;
    D_800D9370.field_24 = value;
    D_800D9370.field_02 = 0;
    D_800D9370.field_22 = 0x81;
    D_801B3274 = 0x118;
    D_801B3270++;
    func_800C3298();
}

void func_800C3298(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    func_8006CC4C(&D_800D9370, &D_801399B8);
    func_80066F9C(&D_800D9370, D_8011CF4C, 0x19, 7, 0);
    if (--D_801B3274 == 0)
    {
        D_801B3270++;
    }
}

void func_800C3314(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_800D9370.field_26 = 0x80;
    D_800D9370.field_22 = 0;
    D_801B3274 = 1;
    D_801B3270++;
    func_800C3360();
}

/**
 * @see decomp.me (100%)
 */
void func_800C3360(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    func_8006CC4C(&D_800D9370, &D_801399B8);
    func_80066F9C(&D_800D9370, D_8011CF4C, 0x19, 7, 0);
    if (--D_801B3274 == 0)
    {
        D_801B3270++;
    }
}

/**
 * @see decomp.me (100%)
 */
void func_800C33DC(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B3270++;
}

/**
 * @see decomp.me (100%)
 */
s32 func_800C33F4(s32 reset)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (reset != 0)
    {
        D_801B3278 = 1;
        D_801B327C = 1;
        return 1;
    }

    if ((u32)D_801B3278 >= 6)
    {
        return 0;
    }

    D_800D7BD4[D_801B3278]();
    return 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C346C(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B3278 = 1;
    D_801B327C = 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C3484(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801399C4 = &D_8011F538;
    D_800D939C.field_06 = 0xF;
    D_800D939C.field_10 = -1;
    D_800D939C.field_26 = 2;
    D_800D939C.field_22 = 0x81;
    D_800D939C.field_02 = 0;
    D_800D939C.field_0E = 0;
    D_800D939C.field_24 = 1;
    D_801B327C = 0xF0;
    D_801B3278++;
    func_800C3504();
}

/**
 * @see decomp.me (100%)
 */
void func_800C3504(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    func_8006CC4C(&D_800D939C, &D_801399C0);
    func_80066F9C(&D_800D939C, D_8011CF4C, 0x19, 8, 0);
    if (--D_801B327C == 0)
    {
        D_801B3278++;
    }
}

/**
 * @see decomp.me (100%)
 */
void func_800C3580(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_800D939C.field_26 = 4;
    D_800D939C.field_22 = 0;
    D_801B327C = 0x20;
    D_801B3278++;
    func_800C35CC();
}

/**
 * @see decomp.me (100%)
 */
void func_800C35CC(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    func_8006CC4C(&D_800D939C, &D_801399C0);
    func_80066F9C(&D_800D939C, D_8011CF4C, 0x19, 8, 0);
    if (--D_801B327C == 0)
    {
        D_801B3278++;
    }
}

/**
 * @see decomp.me (100%)
 */
void func_800C3648(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B3278++;
}

/**
 * @see decomp.me (100%)
 */
s32 func_800C3660(s32 reset)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (reset != 0)
    {
        D_801B3280 = 1;
        D_801B3284 = 1;
        return 1;
    }

    if ((u32)D_801B3280 >= 6)
    {
        return 0;
    }

    D_800D7BEC[D_801B3280]();
    return 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C36D8(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B3280 = 1;
    D_801B3284 = 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C36F0(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_80182DF0 = 1;
    D_8013B238 = D_80139258;
    D_8013923C = 0;
    D_801B3284 = 0xF0;
    D_801B3280++;
    func_800C1290();
}

/**
 * @see decomp.me (100%)
 */
void func_800C376C(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B3284 = 0x20;
    D_801B3280++;
    func_800C1390();
}

/**
 * @see decomp.me (100%)
 */
void func_800C37A4(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B3280++;
}

/**
 * @see decomp.me (100%)
 */
s32 func_800C37BC(s32 reset)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (reset != 0)
    {
        D_801B3288 = 1;
        D_801B328C = 1;
        return 1;
    }

    if ((u32)D_801B3288 >= 6)
    {
        return 0;
    }

    D_800D7C04[D_801B3288]();
    return 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C3834(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B3288 = 1;
    D_801B328C = 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C384C(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_80182DF4 = 1;
    D_8013B240 = D_80139258;
    D_80139260 = 0;
    D_801B328C = 0xD0;
    D_801B3288++;
    func_800C1488();
}

/**
 * @see decomp.me (100%)
 */
void func_800C38C8(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B328C = 0x10;
    D_801B3288++;
    func_800C1588();
}

/**
 * @see decomp.me (100%)
 */
void func_800C3900(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B3288++;
}

/**
 * @see decomp.me (100%)
 */
s32 func_800C3918(s32 reset)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (reset != 0)
    {
        D_801B3290 = 1;
        D_801B3294 = 1;
        return 1;
    }

    if ((u32)D_801B3290 >= 4)
    {
        return 0;
    }

    D_800D7C1C[D_801B3290]();
    return 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C3990(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B3290 = 1;
    D_801B3294 = 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C39A8(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801399CC = &D_8011D538;
    D_800D93C8.field_06 = 0xF;
    D_800D93C8.field_10 = -1;
    D_800D93C8.field_26 = 0x10;
    D_800D93C8.field_02 = 0;
    D_800D93C8.field_0E = 0;
    D_800D93C8.field_22 = 0x80;
    D_800D93C8.field_24 = 0;
    D_801AFC70.field_08 = 0x1964;
    D_801AFC70.field_02 = 0xBB8;
    D_801AFC70.field_04 = 0x64;
    D_801AFC70.field_0E = 0;
    D_801AFC70.field_0C = 0x5A;
    D_801B3294 = 0xBF;
    D_801B3290++;
    func_800C1680();
}

/**
 * @see decomp.me (100%)
 */
void func_800C3A50(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B3290++;
}

/**
 * @see decomp.me (100%)
 */
s32 func_800C3A68(s32 reset)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (reset != 0)
    {
        D_801B3298 = 1;
        D_801B329C = 1;
        return 1;
    }

    if ((u32)D_801B3298 >= 4)
    {
        return 0;
    }

    D_800D7C2C[D_801B3298]();
    return 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C3AE0(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B3298 = 1;
    D_801B329C = 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C3AF8(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801399D4 = &D_8011D538;
    D_800D93F4.field_06 = 0xF;
    D_800D93F4.field_0E = 1;
    D_800D93F4.field_10 = -1;
    D_800D93F4.field_26 = 0x10;
    D_800D93F4.field_02 = 0;
    D_800D93F4.field_22 = 0x80;
    D_800D93F4.field_24 = 0;
    D_801AFC84.field_08 = 0x2710;
    D_801AFC84.field_02 = 0x200;
    D_801AFC84.field_04 = 0x64;
    D_801AFC84.field_0E = 0;
    D_801AFC84.field_0C = 0x3D;
    D_801B329C = 0xBF;
    D_801B3298++;
    func_800C17E0();
}

/**
 * @see decomp.me (100%)
 */
void func_800C3BA4(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B3298++;
}

/**
 * @see decomp.me (100%)
 */
s32 func_800C3BBC(s32 reset)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (reset != 0)
    {
        D_801B32A0 = 1;
        D_801B32A4 = 1;
        return 1;
    }

    if ((u32)D_801B32A0 >= 4)
    {
        return 0;
    }

    D_800D7C3C[D_801B32A0]();
    return 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C3C34(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B32A0 = 1;
    D_801B32A4 = 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C3C4C(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801399DC = &D_8011D538;
    D_800D9420.field_06 = 0xF;
    D_800D9420.field_0E = 1;
    D_800D9420.field_10 = -1;
    D_800D9420.field_26 = 0x10;
    D_800D9420.field_02 = 0;
    D_800D9420.field_22 = 0x80;
    D_800D9420.field_24 = 0;
    D_801AFC98.field_08 = 0x2710;
    D_801AFC98.field_02 = 0x898;
    D_801AFC98.field_04 = 0x64;
    D_801AFC98.field_0E = 0;
    D_801AFC98.field_0C = 0x3D;
    D_801B32A4 = 0xBF;
    D_801B32A0++;
    func_800C1940();
}

/**
 * @see decomp.me (100%)
 */
void func_800C3CF8(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B32A0++;
}

/**
 * @see decomp.me (100%)
 */
s32 func_800C3D10(s32 reset)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (reset != 0)
    {
        D_801B32A8 = 1;
        D_801B32AC = 1;
        return 1;
    }

    if ((u32)D_801B32A8 >= 4)
    {
        return 0;
    }

    D_800D7C4C[D_801B32A8]();
    return 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C3D88(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B32A8 = 1;
    D_801B32AC = 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C3DA0(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801399FC = &D_8011D538;
    D_800D94D0.field_06 = 0xF;
    D_800D94D0.field_0E = 2;
    D_800D94D0.field_10 = -1;
    D_800D94D0.field_26 = 0x10;
    D_800D94D0.field_02 = 0;
    D_800D94D0.field_22 = 0x80;
    D_800D94D0.field_24 = 0;
    D_801AFCE8.field_08 = 0x2710;
    D_801AFCE8.field_02 = 0x50;
    D_801AFCE8.field_04 = 0x64;
    D_801AFCE8.field_0E = 0;
    D_801AFCE8.field_0C = 0x26;
    D_801B32AC = 0xBF;
    D_801B32A8++;
    func_800C1AA0();
}

/**
 * @see decomp.me (100%)
 */
void func_800C3E4C(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B32A8++;
}

/**
 * @see decomp.me (100%)
 */
s32 func_800C3E64(s32 reset)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (reset != 0)
    {
        D_801B32B0 = 1;
        D_801B32B4 = 1;
        return 1;
    }

    if ((u32)D_801B32B0 >= 4)
    {
        return 0;
    }

    D_800D7C5C[D_801B32B0]();
    return 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C3EDC(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B32B0 = 1;
    D_801B32B4 = 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C3EF4(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_80139A04 = &D_8011D538;
    D_800D94FC.field_06 = 0xF;
    D_800D94FC.field_0E = 2;
    D_800D94FC.field_10 = -1;
    D_800D94FC.field_26 = 0x10;
    D_800D94FC.field_02 = 0;
    D_800D94FC.field_22 = 0x80;
    D_800D94FC.field_24 = 0;
    D_801AFCFC.field_08 = 0x1388;
    D_801AFCFC.field_02 = 0x6D6;
    D_801AFCFC.field_04 = 0x64;
    D_801AFCFC.field_0E = 0;
    D_801AFCFC.field_0C = 0x26;
    D_801B32B4 = 0xBF;
    D_801B32B0++;
    func_800C1C00();
}

/**
 * @see decomp.me (100%)
 */
void func_800C3FA0(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B32B0++;
}

/**
 * @see decomp.me (100%)
 */
s32 func_800C3FB8(s32 reset)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (reset != 0)
    {
        D_801B32B8 = 1;
        D_801B32BC = 1;
        return 1;
    }

    if ((u32)D_801B32B8 >= 4)
    {
        return 0;
    }

    D_800D7C6C[D_801B32B8]();
    return 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C4030(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B32B8 = 1;
    D_801B32BC = 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C4048(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_80139A0C = &D_8011D538;
    D_800D9528.field_06 = 0xF;
    D_800D9528.field_0E = 2;
    D_800D9528.field_10 = -1;
    D_800D9528.field_26 = 0x10;
    D_800D9528.field_02 = 0;
    D_800D9528.field_22 = 0x80;
    D_800D9528.field_24 = 0;
    D_801AFD10.field_08 = 0x2710;
    D_801AFD10.field_02 = 0xA8C;
    D_801AFD10.field_04 = 0x64;
    D_801AFD10.field_0E = 0;
    D_801AFD10.field_0C = 0x26;
    D_801B32BC = 0xBF;
    D_801B32B8++;
    func_800C1D60();
}

/**
 * @see decomp.me (100%)
 */
void func_800C40F4(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B32B8++;
}

/**
 * @see decomp.me (100%)
 */
s32 func_800C410C(s32 reset)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (reset != 0)
    {
        D_801B32C0 = 1;
        D_801B32C4 = 1;
        return 1;
    }

    if ((u32)D_801B32C0 >= 4)
    {
        return 0;
    }

    D_800D7C7C[D_801B32C0]();
    return 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C4184(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B32C0 = 1;
    D_801B32C4 = 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C419C(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_80139A14 = &D_8011D538;
    D_800D9554.field_06 = 0xF;
    D_800D9554.field_0E = 2;
    D_800D9554.field_10 = -1;
    D_800D9554.field_26 = 0x10;
    D_800D9554.field_02 = 0;
    D_800D9554.field_22 = 0x80;
    D_800D9554.field_24 = 0;
    D_801AFD24.field_08 = 0x2710;
    D_801AFD24.field_02 = 0x320;
    D_801AFD24.field_04 = 0x64;
    D_801AFD24.field_0E = 0;
    D_801AFD24.field_0C = 0x26;
    D_801B32C4 = 0xBF;
    D_801B32C0++;
    func_800C1EC0();
}

/**
 * @see decomp.me (100%)
 */
void func_800C4248(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B32C0++;
}

/**
 * @see decomp.me (100%)
 */
s32 func_800C4260(s32 reset)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (reset != 0)
    {
        D_801B32C8 = 1;
        D_801B32CC = 1;
        return 1;
    }

    if ((u32)D_801B32C8 >= 6)
    {
        return 0;
    }

    D_800D7C8C[D_801B32C8]();
    return 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C42D8(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B32C8 = 1;
    D_801B32CC = 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C42F0(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    func_800C0CA4(D_80139280);
    if (--D_801B32CC == 0)
    {
        D_801B32C8++;
    }
}

/**
 * @see decomp.me (100%)
 */
void func_800C4344(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_80139280->state_24 = 0;
    D_801B32CC = 0x10;
    D_801B32C8++;
    func_800C4388();
}

/**
 * @see decomp.me (100%)
 */
void func_800C4388(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    func_800C0CA4(D_80139280);
    if (--D_801B32CC == 0)
    {
        D_801B32C8++;
    }
}

/**
 * @see decomp.me (100%)
 */
void func_800C43DC(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B32C8++;
}

/**
 * @see decomp.me (100%)
 */
s32 func_800C43F4(s32 reset)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    if (reset != 0)
    {
        D_801B32D0 = 1;
        D_801B32D4 = 1;
        return 1;
    }

    if ((u32)D_801B32D0 >= 6)
    {
        return 0;
    }

    D_800D7CA4[D_801B32D0]();
    return 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C446C(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B32D0 = 1;
    D_801B32D4 = 1;
}

/**
 * @see decomp.me (100%)
 */
void func_800C4484(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    func_800C0CA4((u8*)D_80139280 + 0x28);
    if (--D_801B32D4 == 0)
    {
        D_801B32D0++;
    }
}

/**
 * @see decomp.me (100%)
 */
void func_800C44D8(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_80139280->tail_state = 0;
    D_801B32D4 = 0x20;
    D_801B32D0++;
    func_800C451C();
}

/**
 * @see decomp.me (100%)
 */
void func_800C451C(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    func_800C0CA4((u8*)D_80139280 + 0x28);
    if (--D_801B32D4 == 0)
    {
        D_801B32D0++;
    }
}

/**
 * @see decomp.me (100%)
 */
void func_800C4570(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad04[2];
    u8 field_06;
    u8 pad07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad28[4];
} WmapConfigA;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    u8 pad10[4];
} WmapConfigB;

typedef struct
{
    s32 field_00;
    void* field_04;
} WmapPointerPair;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 field_00;
    u16 field_04;
    u16 pad06;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef void (*WmapHandler)(void);

extern s32 D_800DBE70;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapHandler D_800D7B84[];
extern WmapHandler D_800D7B94[];
extern WmapHandler D_800D7BA4[];
extern WmapHandler D_800D7BBC[];
extern WmapHandler D_800D7BD4[];
extern WmapHandler D_800D7BEC[];
extern WmapHandler D_800D7C04[];
extern WmapHandler D_800D7C1C[];
extern WmapHandler D_800D7C2C[];
extern WmapHandler D_800D7C3C[];
extern WmapHandler D_800D7C4C[];
extern WmapHandler D_800D7C5C[];
extern WmapHandler D_800D7C6C[];
extern WmapHandler D_800D7C7C[];
extern WmapHandler D_800D7C8C[];
extern WmapHandler D_800D7CA4[];
extern WmapHandler D_800D7AEC[];
extern WmapHandler D_800D7ADC[];
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern WmapConfigA D_800D94D0;
extern WmapConfigA D_800D94FC;
extern WmapConfigA D_800D9528;
extern WmapConfigA D_800D9554;
extern s32 D_8011CF4C;
extern s32 D_8011D500;
extern s32 D_8011CF1C;
extern s32 D_8011CF24;
extern s32 D_8011CF28;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_8013923C;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern WmapState* D_80139280;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern u8 D_80139988[];
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern void* D_801399CC;
extern void* D_801399D4;
extern void* D_801399DC;
extern void* D_801399FC;
extern s32 D_80139978;
extern void* D_80139A04;
extern void* D_80139A0C;
extern void* D_80139A14;
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern WmapConfigB D_801AFC70;
extern WmapConfigB D_801AFC84;
extern WmapConfigB D_801AFC98;
extern WmapConfigB D_801AFCE8;
extern WmapConfigB D_801AFCFC;
extern WmapConfigB D_801AFD10;
extern WmapConfigB D_801AFD24;
extern WmapConfigB D_801AFBD0[];
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern VECTOR D_80182DC0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern u8 D_80182E40;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern u8 D_8018B240;
extern u8 D_80193640;
extern VECTOR D_801B2478;
extern VECTOR D_801B2650;
extern s32 D_801B3248;
extern s32 D_801B324C;
extern s32 D_801B3250;
extern s32 D_801B3254;
extern s32 D_801B3258;
extern s32 D_801B325C;
extern s32 D_801B3260;
extern s32 D_801B3264;
extern s32 D_801B3268;
extern s32 D_801B326C;
extern s32 D_801B3270;
extern s32 D_801B3274;
extern s32 D_801B3278;
extern s32 D_801B327C;
extern s32 D_801B3280;
extern s32 D_801B3284;
extern s32 D_801B3288;
extern s32 D_801B328C;
extern s32 D_801B3290;
extern s32 D_801B3294;
extern s32 D_801B3298;
extern s32 D_801B329C;
extern s32 D_801B32A0;
extern s32 D_801B32A4;
extern s32 D_801B32A8;
extern s32 D_801B32AC;
extern s32 D_801B32B0;
extern s32 D_801B32B4;
extern s32 D_801B32B8;
extern s32 D_801B32BC;
extern s32 D_801B32C0;
extern s32 D_801B32C4;
extern s32 D_801B32C8;
extern s32 D_801B32CC;
extern s32 D_801B32D0;
extern s32 D_801B32D4;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    D_801B32D0++;
}
