#include "wmap_model_render.h"
#include "wmap_main.h"
#include "wmap_map_display.h"
#include "wmap_land_event_17.h"
#include "wmap_sequence_runtime.h"
#include "wmap_view_effects.h"
#include "wmap_map_labels.h"
#include "cdrom.h"
#include "sdk/libgte.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"

void func_800AB8E0(void);
void func_800AC9AC(void);
void func_800ACAD0(void);
void func_800ACBF4(void);
void func_800ACD18(void);
void func_800ACE3C(void);
void func_800ACF60(void);
void func_800AD084(void);
void func_800AD1A8(void);
void func_800AD2CC(void);
void func_800AD7F8(void);
void func_800AB920(void);
s32 func_800AB9C8(s32 arg0);
void func_800AB964(void);
void func_800AB9A0(void);
s32 func_800AD23C(s32 arg0);
s32 func_800AD360(s32 arg0);
s32 func_800AC91C(s32 arg0);
s32 func_800ACED0(s32 arg0);
s32 func_800AD610(s32 arg0);
s32 func_800AD768(s32 arg0);
s32 func_800ACC88(s32 arg0);
s32 func_800ACA40(s32 arg0);
s32 func_800ACB64(s32 arg0);
s32 func_800AD118(s32 arg0);
s32 func_800AD4B8(s32 arg0);
s32 func_800ACDAC(s32 arg0);
s32 func_800ACFF4(s32 arg0);
void func_800AD8EC(void);
void func_800AD9DC(void);

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

extern u8 D_800DEF18[];
extern s32 D_8013B258;
extern s32 g_wmap_vehicle_cell_x;
extern s32 g_wmap_vehicle_cell_y;
extern s32 g_wmap_view_scroll_mode;
extern s32 g_wmap_scroll_remaining_x;
extern s32 g_wmap_scroll_remaining_y;
extern u8 D_800DCF18[];
extern s32 D_8013B208;
extern u8 D_80182E40[];
extern u8 D_8018B240[];
extern u8 D_80193640[];
extern s32 D_801ADAE0;
extern s32 D_801B2E84;
extern u8* D_801399AC;
extern s32 D_801B2E8C;
extern u8* D_801399B4;
extern s32 D_801B2E94;
extern u8* D_801399BC;
extern s32 D_801B2E9C;
extern u8* D_801399D4;
extern u8 D_8011F538[];
extern s32 D_801B2EA4;
extern u8* D_801399CC;
extern s32 D_801B2EAC;
extern u8* D_801399C4;
extern s32 D_801B2EB4;
extern u8* D_801399DC;
extern u8 D_80121538[];
extern s32 D_801B2EBC;
extern u8* D_801399E4;
extern s32 D_801B2EC4;
extern u8* D_801399EC;
extern u8 D_800D9478[];
extern s32 D_801B2ECC;
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern s32 D_801B2ED4;
extern s32 D_80182DEC;
extern s32 D_801B2EDC;
extern s32 D_80182DF0;
extern s32 D_801B2EE4;
extern s32 D_800D9154;
extern s32 D_800DCEAC;
extern s32 D_801B25D8;
extern s32 D_801B2EEC;
extern s32 D_801B2E7C;
extern void (*D_800D6DEC[])(void);
extern s32 D_8013B20C;
extern void func_800AB920(void);
extern s32 D_8013B294;
extern s32 D_80139228;
extern void (*D_800D6E04[])(void);
extern void (*D_800D6EFC[])(void);
extern void (*D_800D6F0C[])(void);
extern void (*D_800D6F1C[])(void);
extern void (*D_800D6F2C[])(void);
extern void (*D_800D6F3C[])(void);
extern void (*D_800D6F4C[])(void);
extern void (*D_800D6F5C[])(void);
extern void (*D_800D6F6C[])(void);
extern void (*D_800D6F7C[])(void);
extern void (*D_800D6F8C[])(void);
extern void (*D_800D6F9C[])(void);
extern void (*D_800D6FAC[])(void);
extern void (*D_800D6FBC[])(void);
extern u8 *D_8011CF1C;
extern u8 *D_8011CF24;
extern u32 D_801B2E78;
extern u8 D_8011D538[];
extern u32 D_801B2E80;
extern u32 D_801B2E88;
extern u32 D_801B2E90;
extern u32 D_801B2E98;
extern u32 D_801B2EA0;
extern u32 D_801B2EA8;
extern u32 D_801B2EB0;
extern u32 D_801B2EB8;
extern u32 D_801B2EC0;
extern u32 D_801B2EC8;
extern u32 D_801B2ED0;
extern u32 D_801B2ED8;
extern u32 D_801B2EE0;
extern u32 D_801B2EE8;

extern VECTOR g_wmap_camera_translation;
extern VECTOR D_801B2650;
extern VECTOR D_801B2478;
extern VECTOR D_80139870;

/** @brief Map scroll position (map units) and projection scale. */
typedef struct
{
    s32 x;
    s32 y;
    s32 projection_scale;
    s32 unknown_0c;
} WmapView;

extern WmapView g_wmap_view;

extern SVECTOR D_80139258;
extern SVECTOR D_801B24A0;
extern SVECTOR D_8013B238;
extern SVECTOR D_801B24A8;

extern WmapSpriteActor D_800D9268[];
extern WmapSpriteActor D_800D9318;
extern WmapSpriteActor D_800D9344;
extern WmapSpriteActor D_800D9370;
extern WmapSpriteActor D_800D939C;
extern WmapSpriteActor D_800D93C8;
extern WmapSpriteActor D_800D93F4;
extern WmapSpriteActor D_800D9420;
extern WmapSpriteActor D_800D944C;

extern WmapAnimationSlot D_80139988[];
extern WmapAnimationSlot D_801399A8;
extern WmapAnimationSlot D_801399B0;
extern WmapAnimationSlot D_801399B8;
extern WmapAnimationSlot D_801399C0;
extern WmapAnimationSlot D_801399C8;
extern WmapAnimationSlot D_801399D0;
extern WmapAnimationSlot D_801399D8;
extern WmapAnimationSlot D_801399E0;
extern WmapAnimationSlot D_801399E8;

extern WmapScreenPosition g_wmap_focus_screen_position;
extern WmapScreenPosition D_80182D58;
extern WmapScreenPosition D_80182D60;
extern WmapScreenPosition D_80182D64;
extern WmapScreenPosition D_80182D6C;
extern WmapScreenPosition D_80182D7C;
extern WmapScreenPosition D_80182D84;
extern WmapScreenPosition D_80182D90;
extern WmapScreenPosition D_80182D98;
extern WmapScreenPosition D_80182DB8;

extern WmapAfcEntry D_801AFBD0[];

/** @brief Set the effect resources and map-relative position, then advance. */
void func_800AAA2C(void)
{
    D_8011CF1C = D_800DEF18;
    D_8011CF24 = D_800DEF18 + 0x2000;
    g_wmap_focus_screen_position.point.x = 0x94;
    g_wmap_focus_screen_position.point.y = 0x31;
    D_8013B258 = 1;
    wmap_find_land_cell(0x11, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = ((g_wmap_vehicle_cell_x - 1) * 0x30) - g_wmap_view.x;
    g_wmap_scroll_remaining_y = ((g_wmap_vehicle_cell_y - 1) * 0x30) - g_wmap_view.y;
    D_801B2E78++;
    func_800AB8E0();
}

/** @brief Set transition controls, queue resources, and start the loading countdown. */
void func_800AAB18(void)
{
    D_8013B208 = 1;
    wmap_start_map_tint(0x301020);
    g_wmap_backdrop_target_level = 4;
    D_801ADAE0 = 1;
    g_wmap_spirit_target_brightness = 0;
    func_8005FF88(-1);
    cdrom_wait_queue_empty();
    cdrom_queue_read(0x1205, D_80182E40);
    cdrom_queue_read(0x1206, D_8018B240);
    cdrom_queue_read(0x1207, D_80193640);
    cdrom_queue_read(0x1208, D_8011D538);
    cdrom_queue_read(0x1209, D_8011D538 + 0x2000);
    cdrom_queue_read(0x120A, D_8011D538 + 0x4000);
    cdrom_queue_read(0x120B, D_800DCF18);
    cdrom_queue_read(0x120C, D_8011CF1C);
    cdrom_queue_read(0x120D, D_8011CF24);
    D_801B2E84 = 0x1E;
    D_801B2E80 += 1;
}

/**
 * @brief Spawn a world-map wandering actor and randomize its start position.
 */
void func_800AAC1C(void)
{
    D_801399AC = D_8011D538;
    D_800D9318.scale_index = 0xF;
    D_800D9318.previous_sequence = -1;
    D_800D9318.shade_step = 0x2;
    D_800D9318.unknown_02 = 0;
    D_800D9318.sequence = 0;
    D_800D9318.target_shade = 0x81;
    D_800D9318.shade = 0x81;
    D_80182D58.point.x = g_wmap_focus_screen_position.point.x + ((rand() * 50) >> 15) - 10;
    D_80182D58.point.y = g_wmap_focus_screen_position.point.y + ((rand() * 30) >> 15) + 15;
    D_801B2E8C = 0x24;
    D_801B2E88 += 1;
    func_800AC9AC();
}

/**
 * @brief Spawn a world-map wandering actor and randomize its start position.
 */
void func_800AAD04(void)
{
    D_801399B4 = D_8011D538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.previous_sequence = -1;
    D_800D9344.shade_step = 0x2;
    D_800D9344.unknown_02 = 0;
    D_800D9344.sequence = 0;
    D_800D9344.target_shade = 0x81;
    D_800D9344.shade = 0x81;
    D_80182D60.point.x = g_wmap_focus_screen_position.point.x + ((rand() * 50) >> 15) - 10;
    D_80182D60.point.y = g_wmap_focus_screen_position.point.y + ((rand() * 30) >> 15) + 15;
    D_801B2E94 = 0x24;
    D_801B2E90 += 1;
    func_800ACAD0();
}

/**
 * @brief Spawn a world-map wandering actor and randomize its start position.
 */
void func_800AADEC(void)
{
    D_801399BC = D_8011D538;
    D_800D9370.scale_index = 0xF;
    D_800D9370.previous_sequence = -1;
    D_800D9370.shade_step = 0x2;
    D_800D9370.unknown_02 = 0;
    D_800D9370.sequence = 0;
    D_800D9370.target_shade = 0x81;
    D_800D9370.shade = 0x81;
    D_80182D64.point.x = g_wmap_focus_screen_position.point.x + ((rand() * 50) >> 15) - 10;
    D_80182D64.point.y = g_wmap_focus_screen_position.point.y + ((rand() * 30) >> 15) + 15;
    D_801B2E9C = 0x24;
    D_801B2E98 += 1;
    func_800ACBF4();
}

/**
 * @brief Spawn a world-map wandering actor and randomize its start position.
 */
void func_800AAED4(void)
{
    D_801399D4 = D_8011F538;
    D_800D93F4.scale_index = 0xF;
    D_800D93F4.previous_sequence = -1;
    D_800D93F4.shade_step = 0x2;
    D_800D93F4.unknown_02 = 0;
    D_800D93F4.sequence = 0;
    D_800D93F4.target_shade = 0x81;
    D_800D93F4.shade = 0x81;
    D_80182D6C.point.x = g_wmap_focus_screen_position.point.x + ((rand() * 50) >> 15) - 10;
    D_80182D6C.point.y = g_wmap_focus_screen_position.point.y + ((rand() * 30) >> 15) + 15;
    D_801B2EA4 = 0x24;
    D_801B2EA0 += 1;
    func_800ACD18();
}

/**
 * @brief Spawn a world-map wandering actor and randomize its start position.
 */
void func_800AAFBC(void)
{
    D_801399CC = D_8011F538;
    D_800D93C8.scale_index = 0xF;
    D_800D93C8.previous_sequence = -1;
    D_800D93C8.shade_step = 0x2;
    D_800D93C8.unknown_02 = 0;
    D_800D93C8.sequence = 0;
    D_800D93C8.target_shade = 0x81;
    D_800D93C8.shade = 0x81;
    D_80182D7C.point.x = g_wmap_focus_screen_position.point.x + ((rand() * 50) >> 15) - 10;
    D_80182D7C.point.y = g_wmap_focus_screen_position.point.y + ((rand() * 30) >> 15) + 15;
    D_801B2EAC = 0x24;
    D_801B2EA8 += 1;
    func_800ACE3C();
}

/**
 * @brief Spawn a world-map wandering actor and randomize its start position.
 */
void func_800AB0A4(void)
{
    D_801399C4 = D_8011F538;
    D_800D939C.scale_index = 0xF;
    D_800D939C.previous_sequence = -1;
    D_800D939C.shade_step = 0x2;
    D_800D939C.unknown_02 = 0;
    D_800D939C.sequence = 0;
    D_800D939C.target_shade = 0x81;
    D_800D939C.shade = 0x81;
    D_80182D84.point.x = g_wmap_focus_screen_position.point.x + ((rand() * 50) >> 15) - 10;
    D_80182D84.point.y = g_wmap_focus_screen_position.point.y + ((rand() * 30) >> 15) + 15;
    D_801B2EB4 = 0x24;
    D_801B2EB0 += 1;
    func_800ACF60();
}

/**
 * @brief Spawn a world-map wandering actor and randomize its start position.
 */
void func_800AB18C(void)
{
    D_801399DC = D_80121538;
    D_800D9420.scale_index = 0xF;
    D_800D9420.previous_sequence = -1;
    D_800D9420.shade_step = 0x2;
    D_800D9420.unknown_02 = 0;
    D_800D9420.sequence = 0;
    D_800D9420.target_shade = 0x81;
    D_800D9420.shade = 0x81;
    D_80182D90.point.x = g_wmap_focus_screen_position.point.x + ((rand() * 50) >> 15) - 10;
    D_80182D90.point.y = g_wmap_focus_screen_position.point.y + ((rand() * 30) >> 15) + 15;
    D_801B2EBC = 0x24;
    D_801B2EB8 += 1;
    func_800AD084();
}

/**
 * @brief Spawn a world-map wandering actor and randomize its start position.
 */
void func_800AB274(void)
{
    D_801399E4 = D_80121538;
    D_800D944C.scale_index = 0xF;
    D_800D944C.previous_sequence = -1;
    D_800D944C.shade_step = 0x2;
    D_800D944C.unknown_02 = 0;
    D_800D944C.sequence = 0;
    D_800D944C.target_shade = 0x81;
    D_800D944C.shade = 0x81;
    D_80182D98.point.x = g_wmap_focus_screen_position.point.x + ((rand() * 50) >> 15) - 10;
    D_80182D98.point.y = g_wmap_focus_screen_position.point.y + ((rand() * 30) >> 15) + 15;
    D_801B2EC4 = 0x24;
    D_801B2EC0 += 1;
    func_800AD1A8();
}

/**
 * @brief Spawn a world-map wandering actor and randomize its start position.
 */
void func_800AB35C(void)
{
    D_801399EC = D_80121538;
    D_800D9478[0x6] = 0xF;
    *(s16*)&D_800D9478[0x10] = -1;
    *(s16*)&D_800D9478[0x26] = 0x2;
    *(s16*)&D_800D9478[0x2] = 0;
    *(s16*)&D_800D9478[0xE] = 0;
    *(s16*)&D_800D9478[0x22] = 0x81;
    *(s16*)&D_800D9478[0x24] = 0x81;
    D_80182DB8.point.x = g_wmap_focus_screen_position.point.x + ((rand() * 50) >> 15) - 10;
    D_80182DB8.point.y = g_wmap_focus_screen_position.point.y + ((rand() * 30) >> 15) + 15;
    D_801B2ECC = 0x24;
    D_801B2EC8 += 1;
    func_800AD2CC();
}

/** @brief Approach the effect depth, draw its fading layer, and advance the countdown. */
void func_800AB444(void)
{
    MATRIX transform;
    s32 depth;
    s32 intensity;
    s32 remaining;

    depth = D_801B2650.vz - 3500;
    D_801B2650.vz = depth;
    if (depth < 10000)
    {
        D_801B2650.vz = 10000;
    }
    PushMatrix();
    RotMatrix(&D_801B24A0, &transform);
    TransMatrix(&transform, &D_8011CF60);
    SetRotMatrix(&transform);
    SetTransMatrix(&transform);
    if (D_80182DE8 != 0)
    {
        wmap_draw_model(D_800DCF18, 0, 4, 53, 0x7800, 1, D_80182DE8, 50, -20, -1);
        intensity = D_80182DE8 - 4;
        D_80182DE8 = intensity;
        if (intensity < 0)
        {
            D_80182DE8 = 0;
        }
    }
    PopMatrix();
    remaining = D_801B2ED4 - 1;
    D_801B2ED4 = remaining;
    if (remaining == 0)
    {
        D_801B2ED0++;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_800AB55C(void)
{
    MATRIX m;
    s32 x;
    s32 timer;

    x = D_801B2478.vz - 0xDAC;
    D_801B2478.vz = x;
    if (x < 0x2710)
    {
        D_801B2478.vz = 0x2710;
    }

    PushMatrix();
    RotMatrix(&D_801B24A8, &m);
    TransMatrix(&m, &D_8011CF60);
    SetRotMatrix(&m);
    SetTransMatrix(&m);

    if (D_80182DEC != 0)
    {
        wmap_draw_model(D_8011CF1C, 0, 4, 0x35, 0x7800, 1, D_80182DEC, -0x19, -0x32, -1);
        D_80182DEC -= 8;
        if (D_80182DEC < 0)
        {
            D_80182DEC = 0;
        }
    }

    PopMatrix();
    timer = D_801B2EDC - 1;
    D_801B2EDC = timer;
    if (timer == 0)
    {
        D_801B2ED8 += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_800AB674(void)
{
    MATRIX m;
    s32 x;
    s32 timer;

    x = D_80139870.vz - 0xDAC;
    D_80139870.vz = x;
    if (x < 0x2710)
    {
        D_80139870.vz = 0x2710;
    }

    PushMatrix();
    RotMatrix(&D_8013B238, &m);
    TransMatrix(&m, &D_8011CF60);
    SetRotMatrix(&m);
    SetTransMatrix(&m);

    if (D_80182DF0 != 0)
    {
        wmap_draw_model(D_8011CF24, 0, 4, 0x35, 0x7800, 1, D_80182DF0, 0xF, -0x37, -1);
        D_80182DF0 -= 8;
        if (D_80182DF0 < 0)
        {
            D_80182DF0 = 0;
        }
    }

    PopMatrix();
    timer = D_801B2EE4 - 1;
    D_801B2EE4 = timer;
    if (timer == 0)
    {
        D_801B2EE0 += 1;
    }
}

/**
 * @brief Initialize a range of world-map per-entry records and schedule the next step.
 */
void func_800AB78C(void)
{
    s32 i;
    WmapD94Entry *entry;

    i = 80;
    D_801B25D8 = 1;
    D_800DCEAC = 1;

    do
    {
        D_801AFBD0[i].unk0 = 0;
        D_80139988[i].data = D_8011D538;
        entry = &D_800D9268[i];
        entry->unk2 = 0;
        entry->unk6 = 0xF;
        entry->unkE = 1;
        entry->unk10 = -1;
        i++;
    } while (i < 124);

    D_800D9154 = 2;
    D_801B2EEC = 0x10;
    D_801B2EE8 += 1;
    func_800AD7F8();
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800AB850(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2E78 = 1;
        D_801B2E7C = 1;
        return 1;
    }

    if (D_801B2E78 < 0x6)
    {
        D_800D6DEC[D_801B2E78]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800AB8C8(void)
{
    D_801B2E78 = 1;
    D_801B2E7C = 1;
}

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void func_800AB8E0(void)
{
    if (g_wmap_view_scroll_mode != 2)
    {
        D_801B2E78 += 1;
        func_800AB920();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_800AB920(void)
{
    wmap_start_sequence(func_800AB9C8);
    D_8013B20C = 1;
    D_801B2E78 += 1;
    func_800AB964();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800AB964(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2E78 += 1;
        func_800AB9A0();
    }
}

/** @brief World-map trigger: set two flags and bump a counter. */
void func_800AB9A0(void)
{
    D_8013B294 = 1;
    D_80139228 = 1;
    D_801B2E78 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800AB9C8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2E80 = 1;
        D_801B2E84 = 1;
        return 1;
    }

    if (D_801B2E80 < 0x3E)
    {
        D_800D6E04[D_801B2E80]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800ABA40(void)
{
    D_801B2E80 = 1;
    D_801B2E84 = 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800ABA58(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/** @brief Wait for queued CD work, initialize three buffers, and register the next callback. */
void func_800ABA8C(void)
{
    cdrom_wait_queue_empty();
    wmap_play_sound(0x2D, 0x80);
    func_800651B4(&D_80182E40);
    func_800651B4(&D_8018B240);
    func_800651B4(&D_80193640);
    wmap_start_sequence(&func_800AD23C);
    D_801B2E84 = 0x12;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800ABB00(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800ABB34(void)
{
    wmap_start_sequence(func_800AD360);
    D_801B2E84 = 0xC;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800ABB70(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/** @brief World-map step: register the four draw callbacks and tick the frame counter. */
void func_800ABBA4(void)
{
    wmap_start_sequence(func_800AC91C);
    wmap_start_sequence(func_800ACED0);
    wmap_start_sequence(func_800AD610);
    wmap_start_sequence(func_800AD768);
    D_801B2E84 = 4;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800ABC04(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800ABC38(void)
{
    wmap_start_sequence(func_800ACC88);
    wmap_start_sequence(func_800ACA40);
    D_801B2E84 = 0x14;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800ABC80(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800ABCB4(void)
{
    wmap_start_sequence(func_800ACB64);
    wmap_start_sequence(func_800AD118);
    D_801B2E84 = 0x4;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800ABCFC(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800ABD30(void)
{
    wmap_start_sequence(func_800AD23C);
    D_801B2E84 = 0x8;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800ABD6C(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800ABDA0(void)
{
    wmap_start_sequence(func_800AD4B8);
    wmap_start_sequence(func_800ACDAC);
    D_801B2E84 = 0xE;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800ABDE8(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800ABE1C(void)
{
    wmap_start_sequence(func_800ACFF4);
    wmap_start_sequence(func_800ACED0);
    D_801B2E84 = 0x10;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800ABE64(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void func_800ABE98(void)
{
    wmap_start_sequence(func_800AD610);
    wmap_start_sequence(func_800AD118);
    wmap_start_sequence(func_800ACC88);
    D_801B2E84 = 0x18;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800ABEEC(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800ABF20(void)
{
    wmap_start_sequence(func_800AD23C);
    wmap_start_sequence(func_800ACB64);
    D_801B2E84 = 0x8;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800ABF68(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800ABF9C(void)
{
    wmap_start_sequence(func_800ACDAC);
    wmap_start_sequence(func_800ACA40);
    D_801B2E84 = 0xC;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800ABFE4(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void func_800AC018(void)
{
    wmap_start_sequence(func_800AC91C);
    wmap_start_sequence(func_800ACED0);
    wmap_start_sequence(func_800AD610);
    D_801B2E84 = 0x4;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC06C(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/** @brief Register a sequence callback, play sound 45, and start a 20-tick delay. */
void func_800AC0A0(void)
{
    wmap_start_sequence(&func_800ACC88);
    wmap_play_sound(0x2D, 0x80);
    D_801B2E84 = 0x14;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC0E8(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800AC11C(void)
{
    wmap_start_sequence(func_800ACB64);
    wmap_start_sequence(func_800AD118);
    D_801B2E84 = 0x4;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC164(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800AC198(void)
{
    wmap_start_sequence(func_800AD23C);
    D_801B2E84 = 0x8;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC1D4(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800AC208(void)
{
    wmap_start_sequence(func_800AD4B8);
    wmap_start_sequence(func_800ACDAC);
    D_801B2E84 = 0xE;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC250(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800AC284(void)
{
    wmap_start_sequence(func_800ACFF4);
    wmap_start_sequence(func_800ACED0);
    D_801B2E84 = 0x10;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC2CC(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void func_800AC300(void)
{
    wmap_start_sequence(func_800AD610);
    wmap_start_sequence(func_800AD118);
    wmap_start_sequence(func_800ACC88);
    D_801B2E84 = 0x18;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC354(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/** @brief Play sound 45, register two callbacks, and begin an eight-tick delay. */
void func_800AC388(void)
{
    wmap_play_sound(0x2D, 0x80);
    wmap_start_sequence(&func_800AD23C);
    wmap_start_sequence(&func_800ACB64);
    D_801B2E84 = 8;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC3DC(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800AC410(void)
{
    wmap_start_sequence(func_800ACDAC);
    wmap_start_sequence(func_800ACA40);
    D_801B2E84 = 0xC;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC458(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/** @brief World-map step: register the four draw callbacks and tick the frame counter. */
void func_800AC48C(void)
{
    wmap_start_sequence(func_800AC91C);
    wmap_start_sequence(func_800ACED0);
    wmap_start_sequence(func_800AD610);
    wmap_start_sequence(func_800AD768);
    D_801B2E84 = 4;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC4EC(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800AC520(void)
{
    wmap_start_sequence(func_800ACC88);
    D_801B2E84 = 0x14;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC55C(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800AC590(void)
{
    wmap_start_sequence(func_800ACB64);
    wmap_start_sequence(func_800AD118);
    D_801B2E84 = 0x4;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC5D8(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800AC60C(void)
{
    wmap_start_sequence(func_800AD23C);
    D_801B2E84 = 0x8;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC648(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800AC67C(void)
{
    wmap_start_sequence(func_800AD4B8);
    wmap_start_sequence(func_800ACDAC);
    D_801B2E84 = 0xE;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC6C4(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800AC6F8(void)
{
    wmap_start_sequence(func_800ACFF4);
    wmap_start_sequence(func_800ACED0);
    D_801B2E84 = 0x10;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC740(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void func_800AC774(void)
{
    wmap_start_sequence(func_800AD610);
    wmap_start_sequence(func_800AD118);
    wmap_start_sequence(func_800ACC88);
    D_801B2E84 = 0x18;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC7C8(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void func_800AC7FC(void)
{
    wmap_start_sequence(func_800AD23C);
    wmap_start_sequence(func_800AD360);
    wmap_start_sequence(func_800ACB64);
    D_801B2E84 = 0x8;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC850(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800AC884(void)
{
    wmap_start_sequence(func_800ACDAC);
    wmap_start_sequence(func_800ACA40);
    D_801B2E84 = 0x54;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC8CC(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief World-map step handler: clear the shared flag and advance the step counter.
 */
void func_800AC900(void)
{
    D_8013B20C = 0;
    D_801B2E80 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800AC91C(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2E88 = 1;
        D_801B2E8C = 1;
        return 1;
    }

    if (D_801B2E88 < 0x4)
    {
        D_800D6EFC[D_801B2E88]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800AC994(void)
{
    D_801B2E88 = 1;
    D_801B2E8C = 1;
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800AC9AC(void)
{
    wmap_step_actor_animation(&D_800D9318, &D_801399A8);
    wmap_draw_actor_sprite(&D_800D9318, D_80182D58.packed, 0x8, 0x2, 0);
    if (--D_801B2E8C == 0)
    {
        D_801B2E88 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800ACA28(void)
{
    D_801B2E88 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800ACA40(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2E90 = 1;
        D_801B2E94 = 1;
        return 1;
    }

    if (D_801B2E90 < 0x4)
    {
        D_800D6F0C[D_801B2E90]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800ACAB8(void)
{
    D_801B2E90 = 1;
    D_801B2E94 = 1;
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800ACAD0(void)
{
    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, D_80182D60.packed, 0x20, 0x2, 0);
    if (--D_801B2E94 == 0)
    {
        D_801B2E90 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800ACB4C(void)
{
    D_801B2E90 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800ACB64(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2E98 = 1;
        D_801B2E9C = 1;
        return 1;
    }

    if (D_801B2E98 < 0x4)
    {
        D_800D6F1C[D_801B2E98]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800ACBDC(void)
{
    D_801B2E98 = 1;
    D_801B2E9C = 1;
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800ACBF4(void)
{
    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, D_80182D64.packed, 0x21, 0x2, 0);
    if (--D_801B2E9C == 0)
    {
        D_801B2E98 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800ACC70(void)
{
    D_801B2E98 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800ACC88(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2EA0 = 1;
        D_801B2EA4 = 1;
        return 1;
    }

    if (D_801B2EA0 < 0x4)
    {
        D_800D6F2C[D_801B2EA0]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800ACD00(void)
{
    D_801B2EA0 = 1;
    D_801B2EA4 = 1;
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800ACD18(void)
{
    wmap_step_actor_animation(&D_800D93F4, &D_801399D0);
    wmap_draw_actor_sprite(&D_800D93F4, D_80182D6C.packed, 0x24, 0x2, 0);
    if (--D_801B2EA4 == 0)
    {
        D_801B2EA0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800ACD94(void)
{
    D_801B2EA0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800ACDAC(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2EA8 = 1;
        D_801B2EAC = 1;
        return 1;
    }

    if (D_801B2EA8 < 0x4)
    {
        D_800D6F3C[D_801B2EA8]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800ACE24(void)
{
    D_801B2EA8 = 1;
    D_801B2EAC = 1;
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800ACE3C(void)
{
    wmap_step_actor_animation(&D_800D93C8, &D_801399C8);
    wmap_draw_actor_sprite(&D_800D93C8, D_80182D7C.packed, 0x11, 0x2, 0);
    if (--D_801B2EAC == 0)
    {
        D_801B2EA8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800ACEB8(void)
{
    D_801B2EA8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800ACED0(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2EB0 = 1;
        D_801B2EB4 = 1;
        return 1;
    }

    if (D_801B2EB0 < 0x4)
    {
        D_800D6F4C[D_801B2EB0]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800ACF48(void)
{
    D_801B2EB0 = 1;
    D_801B2EB4 = 1;
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800ACF60(void)
{
    wmap_step_actor_animation(&D_800D939C, &D_801399C0);
    wmap_draw_actor_sprite(&D_800D939C, D_80182D84.packed, 0x23, 0x2, 0);
    if (--D_801B2EB4 == 0)
    {
        D_801B2EB0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800ACFDC(void)
{
    D_801B2EB0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800ACFF4(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2EB8 = 1;
        D_801B2EBC = 1;
        return 1;
    }

    if (D_801B2EB8 < 0x4)
    {
        D_800D6F5C[D_801B2EB8]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800AD06C(void)
{
    D_801B2EB8 = 1;
    D_801B2EBC = 1;
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800AD084(void)
{
    wmap_step_actor_animation(&D_800D9420, &D_801399D8);
    wmap_draw_actor_sprite(&D_800D9420, D_80182D90.packed, 0x25, 0x2, 0);
    if (--D_801B2EBC == 0)
    {
        D_801B2EB8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800AD100(void)
{
    D_801B2EB8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800AD118(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2EC0 = 1;
        D_801B2EC4 = 1;
        return 1;
    }

    if (D_801B2EC0 < 0x4)
    {
        D_800D6F6C[D_801B2EC0]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800AD190(void)
{
    D_801B2EC0 = 1;
    D_801B2EC4 = 1;
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800AD1A8(void)
{
    wmap_step_actor_animation(&D_800D944C, &D_801399E0);
    wmap_draw_actor_sprite(&D_800D944C, D_80182D98.packed, 0x26, 0x2, 0);
    if (--D_801B2EC4 == 0)
    {
        D_801B2EC0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800AD224(void)
{
    D_801B2EC0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800AD23C(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2EC8 = 1;
        D_801B2ECC = 1;
        return 1;
    }

    if (D_801B2EC8 < 0x4)
    {
        D_800D6F7C[D_801B2EC8]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800AD2B4(void)
{
    D_801B2EC8 = 1;
    D_801B2ECC = 1;
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800AD2CC(void)
{
    wmap_step_actor_animation(D_800D9478, &D_801399E8);
    wmap_draw_actor_sprite(D_800D9478, D_80182DB8.packed, 0x27, 0x2, 0);
    if (--D_801B2ECC == 0)
    {
        D_801B2EC8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800AD348(void)
{
    D_801B2EC8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800AD360(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2ED0 = 1;
        D_801B2ED4 = 1;
        return 1;
    }

    if (D_801B2ED0 < 0x4)
    {
        D_800D6F8C[D_801B2ED0]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800AD3D8(void)
{
    D_801B2ED0 = 1;
    D_801B2ED4 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_800AD3F0(void)
{
    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    D_801B2ED4 = 0x20;
    D_801B2ED0 += 1;
    func_800AB444();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800AD4A0(void)
{
    D_801B2ED0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800AD4B8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2ED8 = 1;
        D_801B2EDC = 1;
        return 1;
    }

    if (D_801B2ED8 < 0x4)
    {
        D_800D6F9C[D_801B2ED8]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800AD530(void)
{
    D_801B2ED8 = 1;
    D_801B2EDC = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_800AD548(void)
{
    D_801B24A8 = D_80139258;
    D_801B2478 = g_wmap_camera_translation;
    D_80182DEC = 0x80;
    D_801B2478.vz = 0xAFC8;
    D_801B2EDC = 0x10;
    D_801B2ED8 += 1;
    func_800AB55C();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800AD5F8(void)
{
    D_801B2ED8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800AD610(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2EE0 = 1;
        D_801B2EE4 = 1;
        return 1;
    }

    if (D_801B2EE0 < 0x4)
    {
        D_800D6FAC[D_801B2EE0]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800AD688(void)
{
    D_801B2EE0 = 1;
    D_801B2EE4 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_800AD6A0(void)
{
    D_8013B238 = D_80139258;
    D_80139870 = g_wmap_camera_translation;
    D_80182DF0 = 0x80;
    D_80139870.vz = 0xAFC8;
    D_801B2EE4 = 0x10;
    D_801B2EE0 += 1;
    func_800AB674();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800AD750(void)
{
    D_801B2EE0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800AD768(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2EE8 = 1;
        D_801B2EEC = 1;
        return 1;
    }

    if (D_801B2EE8 < 0x8)
    {
        D_800D6FBC[D_801B2EE8]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800AD7E0(void)
{
    D_801B2EE8 = 1;
    D_801B2EEC = 1;
}

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void func_800AD7F8(void)
{
    s32 remaining;

    func_8006B328(0x50, 0x7C, 2, -1, 1, 2, 0x78, 8, -0x32, 0x64, -0x28, 0x50, 0x64, 0, 0x81, 2, 1);
    D_801B25D8 += 8;
    remaining = D_801B2EEC - 1;
    D_801B2EEC = remaining;
    if (remaining == 0)
    {
        D_801B2EE8 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800AD8B4(void)
{
    D_801B2EEC = 0x58;
    D_801B2EE8 += 1;
    func_800AD8EC();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void func_800AD8EC(void)
{
    func_8006B328(0x50, 0x7C, 2, -1, 1, 2, 0x78, 8, -0x32, 0x64, -0x28, 0x50, 0x64, 0, 0x81, 2, 1);
    if (--D_801B2EEC == 0)
    {
        D_801B2EE8 += 1;
    }
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void func_800AD99C(void)
{
    D_800DCEAC = 0;
    D_801B2EEC = 0x64;
    D_801B2EE8 += 1;
    func_800AD9DC();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void func_800AD9DC(void)
{
    func_8006B328(0x50, 0x7C, 2, -1, 1, 2, 0x78, 8, -0x32, 0x64, -0x28, 0x50, 0x64, 0, 0x81, 2, 1);
    if (--D_801B2EEC == 0)
    {
        D_801B2EE8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800ADA8C(void)
{
    D_801B2EE8 += 1;
}
