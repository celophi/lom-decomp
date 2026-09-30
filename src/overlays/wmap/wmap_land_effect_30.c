#include "wmap_main.h"
#include "wmap_land_effect_30.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_effect_primitives.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"

void wmap_land_effect_30_sequence_12_step_02(void);
void wmap_land_effect_30_sequence_13_step_02(void);
void wmap_land_effect_30_sequence_14_step_02(void);
void wmap_land_effect_30_wait_idle_02(void);
void wmap_land_effect_30_step_03(void);
s32 wmap_land_effect_30_run_timeline(s32 arg0);
void wmap_land_effect_30_wait_idle_04(void);
void wmap_land_effect_30_end(void);
s32 wmap_land_effect_30_run_sequence_1(s32 arg0);
s32 wmap_land_effect_30_run_sequence_3(s32 arg0);
s32 wmap_land_effect_30_run_sequence_6(s32 arg0);
s32 wmap_land_effect_30_run_sequence_7(s32 arg0);
s32 wmap_land_effect_30_run_sequence_8(s32 arg0);
s32 wmap_land_effect_30_run_sequence_9(s32 arg0);
s32 wmap_land_effect_30_run_sequence_4(s32 arg0);
s32 wmap_land_effect_30_run_sequence_12(s32 arg0);
s32 wmap_land_effect_30_run_sequence_14(s32 arg0);
s32 wmap_land_effect_30_run_sequence_13(s32 arg0);
s32 wmap_land_effect_30_run_sequence_10(s32 arg0);
s32 wmap_land_effect_30_run_sequence_2(s32 arg0);
s32 wmap_land_effect_30_run_sequence_5(s32 arg0);
s32 wmap_land_effect_30_run_sequence_11(s32 arg0);
void wmap_land_effect_30_sequence_1_step_02(void);
void wmap_land_effect_30_sequence_2_step_02(void);
void wmap_land_effect_30_sequence_5_step_02(void);
void wmap_land_effect_30_sequence_6_step_02(void);
void wmap_land_effect_30_sequence_6_step_04(void);
void wmap_land_effect_30_sequence_7_step_02(void);
void wmap_land_effect_30_sequence_7_step_04(void);
void wmap_land_effect_30_sequence_8_step_02(void);
void wmap_land_effect_30_sequence_8_step_04(void);
void wmap_land_effect_30_sequence_9_step_02(void);
void wmap_land_effect_30_sequence_9_step_04(void);
void wmap_land_effect_30_sequence_10_step_02(void);
void wmap_land_effect_30_sequence_11_step_02(void);
void wmap_land_effect_30_sequence_13_step_04(void);
void wmap_land_effect_30_sequence_13_step_06(void);

/** @brief Per-actor motion and animation parameters. */
typedef struct
{
    s16 state;
    s16 angle;
    s32 x;
    s32 z;
    s16 scale;
    s16 field_0E;
    s32 field_10;
} WmapMotion;

/** @brief Animation resource slot. */
typedef struct
{
    s32 field_00;
    void *resource;
} WmapResource;

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
    u32 value;
    u8 unknown_4[36];
} WmapValueRecord;

extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B280C;
extern u8* D_8011CF1C;
extern s32 D_801B2814;
extern s32 D_8011CF74;
extern s32 D_80139980;
extern s32 D_801B0FD0;
extern s32 D_801B2844;
extern s32 D_801B284C;
extern s32 D_80139240;
extern s32 D_80139250;
extern s32 D_80139264;
extern s32 D_80139268;
extern s32 D_8013926C;
extern s32 D_80139284;
extern s32 D_8013B264;
extern s32 D_8013B270;
extern s32 D_8013B278;
extern s32 D_801B2854;
extern WmapMotion D_801B0530[];
extern s32 rand(void);
extern WmapResource D_80139D48;
extern s32 D_800D9150;
extern s32 D_800DCEA8;
extern s32 D_801B25D8;
extern s32 D_801B285C;
extern WmapMotion D_801AFCFC[];
extern s32 D_801B2864;
extern WmapResource D_80139A00;
extern s32 D_801B27EC;
extern void (*D_800D56F0[])(void);
extern s32 D_8013B20C;
extern void wmap_land_effect_30_step_03(void);
extern void wmap_land_effect_30_end(void);
extern s32 D_801B27F4;
extern void (*D_800D5708[])(void);
extern s32 D_801ADAE0;
extern WmapValueRecord D_80139290[][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 D_801B27FC;
extern void (*D_800D5768[])(void);
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern void wmap_land_effect_30_sequence_1_step_02(void);
extern s32 D_801B2804;
extern void (*D_800D5778[])(void);
extern u8 D_80121538[];
extern u8* D_801399B4;
extern void wmap_land_effect_30_sequence_2_step_02(void);
extern void (*D_800D5788[])(void);
extern void (*D_800D5798[])(void);
extern s32 D_801B281C;
extern void (*D_800D57A8[])(void);
extern u8 *D_801399BC;
extern s32 D_801B2824;
extern void (*D_800D57B8[])(void);
extern u8 D_8011F538[];
extern u8* D_801399C4;
extern void wmap_land_effect_30_sequence_6_step_02(void);
extern void wmap_land_effect_30_sequence_6_step_04(void);
extern s32 D_801B282C;
extern void (*D_800D57D0[])(void);
extern u8* D_801399CC;
extern void wmap_land_effect_30_sequence_7_step_02(void);
extern void wmap_land_effect_30_sequence_7_step_04(void);
extern s32 D_801B2834;
extern void (*D_800D57E8[])(void);
extern void *D_801399D4;
extern void wmap_land_effect_30_sequence_8_step_04(void);
extern s32 D_801B283C;
extern void (*D_800D5800[])(void);
extern u8* D_801399DC;
extern void wmap_land_effect_30_sequence_9_step_04(void);
extern void (*D_800D5818[])(void);
extern void (*D_800D5830[])(void);
extern void (*D_800D5848[])(void);
extern void (*D_800D5858[])(void);
extern void (*D_800D5878[])(void);

/** @brief World-map actor configuration. */
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
} __attribute__((aligned(4))) WmapConfigA;

extern u32 D_801B2808;
extern u32 D_801B2810;
extern u32 D_801B2840;
extern u32 D_801B2848;
extern u32 D_801B2850;
extern WmapConfigA D_800DA708[];
extern u8 D_80123538[];
extern u32 D_801B2858;
extern WmapConfigA D_800D94FC[];
extern u8 D_80125538[];
extern u32 D_801B2860;
extern u32 D_801B27E8;
extern u32 D_801B27F0;
extern u32 D_801B27F8;
extern u32 D_801B2800;
extern u32 D_801B2818;
extern u32 D_801B2820;
extern u32 D_801B2828;
extern u8 D_80127538[];
extern u32 D_801B2830;
extern u32 D_801B2838;

extern VECTOR g_wmap_camera_translation;
extern VECTOR D_801B2650;

extern SVECTOR D_80139258;
extern SVECTOR D_801B24A0;

extern WmapSpriteActor D_800D9268[];
extern WmapSpriteActor D_800D9318;
extern WmapSpriteActor D_800D9344;
extern WmapSpriteActor D_800D9370;
extern WmapSpriteActor D_800D939C;
extern WmapSpriteActor D_800D93C8;
extern WmapSpriteActor D_800D93F4;
extern WmapSpriteActor D_800D9420;

extern WmapAnimationSlot D_80139988[];
extern WmapAnimationSlot D_801399A8;
extern WmapAnimationSlot D_801399B0;
extern WmapAnimationSlot D_801399B8;
extern WmapAnimationSlot D_801399C0;
extern WmapAnimationSlot D_801399C8;
extern WmapAnimationSlot D_801399D0;
extern WmapAnimationSlot D_801399D8;

extern WmapScreenPosition g_wmap_focus_screen_position;
extern WmapScreenPosition D_80182D58;

extern s32* D_80139280;

extern WmapMotion D_801AFBD0[];

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_30_sequence_3_step_02(void)
{
    MATRIX m;
    s32 x;

    x = D_801B2650.vz - 0xDAC;
    D_801B2650.vz = x;
    if (x < 0x2710)
    {
        D_801B2650.vz = 0x2710;
    }

    PushMatrix();
    RotMatrix(&D_801B24A0, &m);
    TransMatrix(&m, &D_8011CF60);
    SetRotMatrix(&m);
    SetTransMatrix(&m);

    if (D_80182DE8 != 0)
    {
        wmap_draw_model_default((s32)D_800DCF18, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DE8);
        D_80182DE8 -= 0x5;
        if (D_80182DE8 < 0)
        {
            D_80182DE8 = 0;
        }
    }

    PopMatrix();
    if (--D_801B280C == 0)
    {
        D_801B2808 += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_30_sequence_4_step_02(void)
{
    MATRIX m;
    s32 x;

    x = D_801B2650.vz - 0xDAC;
    D_801B2650.vz = x;
    if (x < 0x2710)
    {
        D_801B2650.vz = 0x2710;
    }

    PushMatrix();
    RotMatrix(&D_801B24A0, &m);
    TransMatrix(&m, &D_8011CF60);
    SetRotMatrix(&m);
    SetTransMatrix(&m);

    if (D_80182DE8 != 0)
    {
        wmap_draw_model_default(D_8011CF1C, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DE8);
        D_80182DE8 -= 0x2;
        if (D_80182DE8 < 0)
        {
            D_80182DE8 = 0;
        }
    }

    PopMatrix();
    if (--D_801B2814 == 0)
    {
        D_801B2810 += 1;
    }
}

/** @brief Reduce the effect parameters, draw it, and advance its countdown. */
void wmap_land_effect_30_sequence_10_step_04(void)
{
    s32 value;
    s32 remaining;

    value = D_80139980 - 1;
    D_80139980 = value;
    if (value < 0)
    {
        D_80139980 = 0;
    }
    if (!(D_8011CF74 & 3))
    {
        D_801B0FD0 -= 1;
    }
    if (D_801B0FD0 < 0)
    {
        D_801B0FD0 = 0;
    }
    func_8006AFAC(0x13, 0x45, 0x13, 0x20, 0xFD0, 4, 0x20, 1);
    remaining = D_801B2844 - 1;
    D_801B2844 = remaining;
    if (remaining == 0)
    {
        D_801B2840 += 1;
    }
}

/** @brief Reduce the effect parameters, draw it, and advance its countdown. */
void wmap_land_effect_30_sequence_11_step_04(void)
{
    s32 value;
    s32 remaining;

    value = D_80139980 - 1;
    D_80139980 = value;
    if (value < 0)
    {
        D_80139980 = 0;
    }
    if (!(D_8011CF74 & 3))
    {
        D_801B0FD0 -= 1;
    }
    if (D_801B0FD0 < 0)
    {
        D_801B0FD0 = 0;
    }
    func_8006AFAC(0x54, 0x5C, 0x13, 0x20, 0x1000, 0x20, 8, 2);
    remaining = D_801B284C - 1;
    D_801B284C = remaining;
    if (remaining == 0)
    {
        D_801B2848 += 1;
    }
}

/** @brief Initialize spaced actors with randomized animation choices. */
void wmap_land_effect_30_sequence_12_step_01(void)
{
    s32 i;
    WmapConfigA *actor;

    D_80139264 = 120;
    D_80139268 = 19;
    D_8013926C = 0;
    for (i = 0; i < 60; i++)
    {
        D_801AFBD0[i + 120].state = 0;
        D_80139988[i + 120].data = D_80123538;
    }
    D_80139240 = 0;
    for (i = 0; i < 60; i += 5)
    {
        actor = &D_800DA708[i];
        actor->field_02 = 0;
        actor->field_06 = 15;
        actor->field_0E = rand() % 3;
        actor->field_10 = -1;
        actor->field_26 = 2;
        actor->field_22 = 129;
        actor->field_24 = 8;
        D_801B0530[i].state = 1;
        D_801B0530[i].z = 0;
        D_801B0530[i].scale = 60;
        D_801B0530[i].angle = D_80139240;
        D_80139240 += 341;
        D_801B0530[i].field_0E = 0;
    }
    D_8013B264 = 60;
    D_8013B270 = 4;
    D_8013B278 = 5000;
    D_80139284 = 0;
    D_80139250 = -1;
    D_801B2854 = 60;
    D_801B2850++;
    wmap_land_effect_30_sequence_12_step_02();
}

/** @brief Advance spaced effect actors and copy their trailing samples. */
void wmap_land_effect_30_sequence_12_step_02(void)
{
    s32 next;
    s32 i;
    s32 destination;
    s32 remaining;

    func_8006D014(&D_800DA708, &D_80139D48, 60, 128, 128, 8, 3);
    for (i = 120; i < 180; i += 5)
    {
        if (D_801AFBD0[i].scale != 0)
        {
            D_801AFBD0[i].scale--;
        }
        D_801AFBD0[i].z += 5000;
    }
    if ((D_8011CF74 & 1) == 0)
    {
        for (i = 120; i < 180; i += 5)
        {
            next = i + 1;
            destination = D_80139284 + next;
            D_801AFBD0[destination] = D_801AFBD0[i];
            D_800D9268[destination] = D_800D9268[i];
            D_80139988[destination] = D_80139988[i];
            D_800D9268[destination].target_shade = 0;
        }
        D_80139284 = (D_80139284 + 1) % 4;
    }
    remaining = D_801B2854 - 1;
    D_801B2854 = remaining;
    if (remaining == 0)
    {
        D_801B2850++;
    }
}

/**
 * @brief Initialize a range of world-map per-entry records and schedule the next step.
 */
void wmap_land_effect_30_sequence_13_step_01(void)
{
    s32 i;
    WmapD94Entry *entry;

    i = 200;
    D_801B25D8 = 1;
    D_800DCEA8 = 1;

    do
    {
        D_801AFBD0[i].state = 0;
        D_80139988[i].data = D_80123538;
        entry = &D_800D9268[i];
        entry->unk2 = 0;
        entry->unk6 = 0xF;
        entry->unkE = 3;
        entry->unk10 = -1;
        i++;
    } while (i < 240);

    D_800D9150 = 2;
    D_801B285C = 0x10;
    D_801B2858 += 1;
    wmap_land_effect_30_sequence_13_step_02();
}

/** @brief Initialize effect actors at regular angular intervals. */
void wmap_land_effect_30_sequence_14_step_01(void)
{
    s32 i;
    s16 actor_field_value;

    D_80139280[0x25] = 15;
    D_80139280[0x26] = 19;
    D_80139280[0x27] = 0;
    for (i = 0; i < 60; i++)
    {
        D_801AFBD0[i + 15].state = 0;
        D_80139988[i + 15].data = D_80125538;
    }
    D_80139280[0x21] = 256;
    for (i = 0; i < 60; i += 5)
    {
        actor_field_value = 2;
        D_800D94FC[i].field_26 = actor_field_value;
        actor_field_value = 129;
        D_800D94FC[i].field_22 = actor_field_value;
        actor_field_value = 8;
        D_800D94FC[i].field_24 = actor_field_value;
        D_800D94FC[i].field_02 = 0;
        D_800D94FC[i].field_06 = 15;
        D_800D94FC[i].field_0E = 0;
        D_800D94FC[i].field_10 = -1;
        D_801AFCFC[i].state = 1;
        D_801AFCFC[i].z = 0;
        D_801AFCFC[i].scale = 60;
        D_801AFCFC[i].angle = D_80139280[0x21];
        D_80139280[0x21] += 341;
        D_801AFCFC[i].field_0E = 0;
    }
    D_8013B264 = 60;
    D_8013B270 = 4;
    D_8013B278 = 5000;
    D_80139280[0x23] = -1;
    D_80139280[0x28] = 0;
    D_801B2864 = 60;
    D_801B2860++;
    wmap_land_effect_30_sequence_14_step_02();
}

/** @brief Advance spaced effect actors and copy their trailing samples. */
void wmap_land_effect_30_sequence_14_step_02(void)
{
    s32 next;
    s32 i;
    s32 destination;
    s32 remaining;

    func_8006A2FC(&D_800D94FC, &D_80139A00, 60, 128, 128, 8, 3, &D_80139280[30]);
    for (i = 15; i < 75; i += 5)
    {
        if (D_801AFBD0[i].scale != 0)
        {
            D_801AFBD0[i].scale--;
        }
        D_801AFBD0[i].z += 5000;
        D_801AFBD0[i].angle += 128;
    }
    if ((D_8011CF74 & 1) == 0)
    {
        for (i = 15; i < 75; i += 5)
        {
            next = i + 1;
            destination = D_80139280[40] + next;
            D_801AFBD0[destination] = D_801AFBD0[i];
            D_800D9268[destination] = D_800D9268[i];
            D_80139988[destination] = D_80139988[i];
            D_800D9268[destination].target_shade = 0;
        }
        D_80139280[40] = (D_80139280[40] + 1) % 4;
    }
    remaining = D_801B2864 - 1;
    D_801B2864 = remaining;
    if (remaining == 0)
    {
        D_801B2860++;
    }
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_30_run(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B27E8 = 1;
        D_801B27EC = 1;
        return 1;
    }

    if (D_801B27E8 < 0x6)
    {
        D_800D56F0[D_801B27E8]();
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
void wmap_land_effect_30_reset(void)
{
    D_801B27E8 = 1;
    D_801B27EC = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void wmap_land_effect_30_step_01(void)
{
    wmap_start_sequence(wmap_run_land_focus);
    D_8013B20C = 1;
    D_801B27E8 += 1;
    wmap_land_effect_30_wait_idle_02();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void wmap_land_effect_30_wait_idle_02(void)
{
    if (D_8013B20C == 0)
    {
        D_801B27E8 += 1;
        wmap_land_effect_30_step_03();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void wmap_land_effect_30_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_30_run_timeline);
    D_8013B20C = 1;
    D_801B27E8 += 1;
    wmap_land_effect_30_wait_idle_04();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void wmap_land_effect_30_wait_idle_04(void)
{
    if (D_8013B20C == 0)
    {
        D_801B27E8 += 1;
        wmap_land_effect_30_end();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_30_end(void)
{
    D_801B27E8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_30_run_timeline(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B27F0 = 1;
        D_801B27F4 = 1;
        return 1;
    }

    if (D_801B27F0 < 0x18)
    {
        D_800D5708[D_801B27F0]();
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
void wmap_land_effect_30_timeline_reset(void)
{
    D_801B27F0 = 1;
    D_801B27F4 = 1;
}

/** @brief Set world-map color, play sound 30, register two callbacks, and begin an eight-tick delay. */
void wmap_land_effect_30_timeline_step_01(void)
{
    wmap_start_map_tint(0x122840);
    g_wmap_backdrop_target_level = 4;
    wmap_play_sound(0x1E, 0x80);
    wmap_start_sequence(&wmap_land_effect_30_run_sequence_1);
    wmap_start_sequence(&wmap_land_effect_30_run_sequence_3);
    D_801B27F4 = 8;
    D_801B27F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_30_timeline_wait_02(void)
{
    if (--D_801B27F4 == 0)
    {
        D_801B27F0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_30_timeline_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_30_run_sequence_6);
    D_801B27F4 = 0x4;
    D_801B27F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_30_timeline_wait_04(void)
{
    if (--D_801B27F4 == 0)
    {
        D_801B27F0 += 1;
    }
}

/** @brief World-map step handler: register the next callback and advance the counter. */
void wmap_land_effect_30_timeline_step_05(void)
{
    wmap_start_sequence(wmap_land_effect_30_run_sequence_7);
    D_801ADAE0 = 1;
    D_801B27F4 = 4;
    D_801B27F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_30_timeline_wait_06(void)
{
    if (--D_801B27F4 == 0)
    {
        D_801B27F0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_30_timeline_step_07(void)
{
    wmap_start_sequence(wmap_land_effect_30_run_sequence_8);
    D_801B27F4 = 0x4;
    D_801B27F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_30_timeline_wait_08(void)
{
    if (--D_801B27F4 == 0)
    {
        D_801B27F0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_30_timeline_step_09(void)
{
    wmap_start_sequence(wmap_land_effect_30_run_sequence_9);
    D_801B27F4 = 0x24;
    D_801B27F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_30_timeline_wait_10(void)
{
    if (--D_801B27F4 == 0)
    {
        D_801B27F0 += 1;
    }
}

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void wmap_land_effect_30_timeline_step_11(void)
{
    wmap_start_sequence(wmap_land_effect_30_run_sequence_4);
    wmap_start_sequence(wmap_land_effect_30_run_sequence_12);
    wmap_start_sequence(wmap_land_effect_30_run_sequence_14);
    D_801B27F4 = 0x38;
    D_801B27F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_30_timeline_wait_12(void)
{
    if (--D_801B27F4 == 0)
    {
        D_801B27F0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_30_timeline_step_13(void)
{
    wmap_start_sequence(wmap_land_effect_30_run_sequence_13);
    D_801B27F4 = 0xA;
    D_801B27F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_30_timeline_wait_14(void)
{
    if (--D_801B27F4 == 0)
    {
        D_801B27F0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_30_timeline_step_15(void)
{
    wmap_start_sequence(wmap_land_effect_30_run_sequence_10);
    D_801B27F4 = 0x10;
    D_801B27F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_30_timeline_wait_16(void)
{
    if (--D_801B27F4 == 0)
    {
        D_801B27F0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_30_timeline_step_17(void)
{
    wmap_start_sequence(wmap_land_effect_30_run_sequence_2);
    D_801B27F4 = 0x30;
    D_801B27F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_30_timeline_wait_18(void)
{
    if (--D_801B27F4 == 0)
    {
        D_801B27F0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_30_timeline_step_19(void)
{
    wmap_start_sequence(wmap_land_effect_30_run_sequence_5);
    D_801B27F4 = 0x2;
    D_801B27F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_30_timeline_wait_20(void)
{
    if (--D_801B27F4 == 0)
    {
        D_801B27F0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_30_timeline_step_21(void)
{
    wmap_start_sequence(wmap_land_effect_30_run_sequence_11);
    D_801B27F4 = 0x28;
    D_801B27F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_30_timeline_wait_22(void)
{
    if (--D_801B27F4 == 0)
    {
        D_801B27F0 += 1;
    }
}

/** @brief Update the selected world-map land record and advance the sequence step. */
void wmap_land_effect_30_timeline_finish(void)
{
    D_8013B20C = 0;
    D_80139290[D_8011D510][D_8011D530].value = D_8011D4FC | 0x100;
    D_801B27F0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_30_run_sequence_1(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B27F8 = 1;
        D_801B27FC = 1;
        return 1;
    }

    if (D_801B27F8 < 0x4)
    {
        D_800D5768[D_801B27F8]();
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
void wmap_land_effect_30_sequence_1_reset(void)
{
    D_801B27F8 = 1;
    D_801B27FC = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_30_sequence_1_step_01(void)
{
    D_801399AC = D_8011D538;
    D_800D9318.scale_index = 0xF;
    D_800D9318.previous_sequence = -1;
    D_800D9318.shade_step = 8;
    D_800D9318.resource_index = 0;
    D_800D9318.sequence = 0;
    D_800D9318.target_shade = 0x80;
    D_800D9318.shade = 0;
    D_801B27FC = 0x3C;
    D_801B27F8 += 1;
    wmap_land_effect_30_sequence_1_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_30_sequence_1_step_02(void)
{
    wmap_step_actor_animation(&D_800D9318, &D_801399A8);
    wmap_draw_actor_sprite(&D_800D9318, g_wmap_focus_screen_position.packed, 0x13, 0x14, 0);
    if (--D_801B27FC == 0)
    {
        D_801B27F8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_30_sequence_1_end(void)
{
    D_801B27F8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_30_run_sequence_2(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2800 = 1;
        D_801B2804 = 1;
        return 1;
    }

    if (D_801B2800 < 0x4)
    {
        D_800D5778[D_801B2800]();
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
void wmap_land_effect_30_sequence_2_reset(void)
{
    D_801B2800 = 1;
    D_801B2804 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_30_sequence_2_step_01(void)
{
    D_801399B4 = D_80121538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.previous_sequence = -1;
    D_800D9344.shade_step = 8;
    D_800D9344.resource_index = 0;
    D_800D9344.sequence = 0;
    D_800D9344.target_shade = 0x80;
    D_800D9344.shade = 0x80;
    D_801B2804 = 0x5D;
    D_801B2800 += 1;
    wmap_land_effect_30_sequence_2_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_30_sequence_2_step_02(void)
{
    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0x18, 0x33, 0);
    if (--D_801B2804 == 0)
    {
        D_801B2800 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_30_sequence_2_end(void)
{
    D_801B2800 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_30_run_sequence_3(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2808 = 1;
        D_801B280C = 1;
        return 1;
    }

    if (D_801B2808 < 0x4)
    {
        D_800D5788[D_801B2808]();
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
void wmap_land_effect_30_sequence_3_reset(void)
{
    D_801B2808 = 1;
    D_801B280C = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_30_sequence_3_step_01(void)
{
    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    D_801B280C = 0x14;
    D_801B2808 += 1;
    wmap_land_effect_30_sequence_3_step_02();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_30_sequence_3_end(void)
{
    D_801B2808 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_30_run_sequence_4(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2810 = 1;
        D_801B2814 = 1;
        return 1;
    }

    if (D_801B2810 < 0x4)
    {
        D_800D5798[D_801B2810]();
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
void wmap_land_effect_30_sequence_4_reset(void)
{
    D_801B2810 = 1;
    D_801B2814 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_30_sequence_4_step_01(void)
{
    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    D_801B2814 = 0x40;
    D_801B2810 += 1;
    wmap_land_effect_30_sequence_4_step_02();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_30_sequence_4_end(void)
{
    D_801B2810 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_30_run_sequence_5(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2818 = 1;
        D_801B281C = 1;
        return 1;
    }

    if (D_801B2818 < 0x4)
    {
        D_800D57A8[D_801B2818]();
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
void wmap_land_effect_30_sequence_5_reset(void)
{
    D_801B2818 = 1;
    D_801B281C = 1;
}

/** @brief Configure the actor, save its screen position, and begin a 44-tick delay. */
void wmap_land_effect_30_sequence_5_step_01(void)
{
    D_801399BC = D_80121538;
    D_800D9370.scale_index = 0xF;
    D_800D9370.sequence = 1;
    D_800D9370.previous_sequence = -1;
    D_800D9370.shade_step = 8;
    D_800D9370.target_shade = 0x80;
    D_800D9370.shade = 0x80;
    D_800D9370.resource_index = 0;
    D_801B281C = 0x2C;
    D_80182D58.packed = g_wmap_focus_screen_position.packed;
    D_801B2818 += 1;
    wmap_land_effect_30_sequence_5_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_30_sequence_5_step_02(void)
{
    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, g_wmap_focus_screen_position.packed, 0x18, 0x22, 0);
    if (--D_801B281C == 0)
    {
        D_801B2818 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_30_sequence_5_end(void)
{
    D_801B2818 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_30_run_sequence_6(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2820 = 1;
        D_801B2824 = 1;
        return 1;
    }

    if (D_801B2820 < 0x6)
    {
        D_800D57B8[D_801B2820]();
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
void wmap_land_effect_30_sequence_6_reset(void)
{
    D_801B2820 = 1;
    D_801B2824 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_30_sequence_6_step_01(void)
{
    D_801399C4 = D_8011F538;
    D_800D939C.scale_index = 0xF;
    D_800D939C.previous_sequence = -1;
    D_800D939C.shade_step = 2;
    D_800D939C.target_shade = 0x81;
    D_800D939C.resource_index = 0;
    D_800D939C.sequence = 0;
    D_800D939C.shade = 1;
    D_801B2824 = 0x30;
    D_801B2820 += 1;
    wmap_land_effect_30_sequence_6_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_30_sequence_6_step_02(void)
{
    wmap_step_actor_animation(&D_800D939C, &D_801399C0);
    wmap_draw_actor_sprite(&D_800D939C, g_wmap_focus_screen_position.packed, 0x8, 0xA, 0);
    if (--D_801B2824 == 0)
    {
        D_801B2820 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_30_sequence_6_step_03(void)
{
    D_800D939C.shade_step = 2;
    D_800D939C.target_shade = 0;
    D_801B2824 = 0x8;
    D_801B2820 += 1;
    wmap_land_effect_30_sequence_6_step_04();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_30_sequence_6_step_04(void)
{
    wmap_step_actor_animation(&D_800D939C, &D_801399C0);
    wmap_draw_actor_sprite(&D_800D939C, g_wmap_focus_screen_position.packed, 0x8, 0xA, 0);
    if (--D_801B2824 == 0)
    {
        D_801B2820 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_30_sequence_6_end(void)
{
    D_801B2820 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_30_run_sequence_7(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2828 = 1;
        D_801B282C = 1;
        return 1;
    }

    if (D_801B2828 < 0x6)
    {
        D_800D57D0[D_801B2828]();
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
void wmap_land_effect_30_sequence_7_reset(void)
{
    D_801B2828 = 1;
    D_801B282C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_30_sequence_7_step_01(void)
{
    D_801399CC = D_80127538;
    D_800D93C8.scale_index = 0xF;
    D_800D93C8.previous_sequence = -1;
    D_800D93C8.shade_step = 8;
    D_800D93C8.target_shade = 0x81;
    D_800D93C8.resource_index = 0;
    D_800D93C8.sequence = 0;
    D_800D93C8.shade = 1;
    D_801B282C = 0x20;
    D_801B2828 += 1;
    wmap_land_effect_30_sequence_7_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_30_sequence_7_step_02(void)
{
    wmap_step_actor_animation(&D_800D93C8, &D_801399C8);
    wmap_draw_actor_sprite(&D_800D93C8, g_wmap_focus_screen_position.packed, 0x8, 0xA, 0);
    if (--D_801B282C == 0)
    {
        D_801B2828 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_30_sequence_7_step_03(void)
{
    D_800D93C8.shade_step = 8;
    D_800D93C8.target_shade = 0;
    D_801B282C = 0x8;
    D_801B2828 += 1;
    wmap_land_effect_30_sequence_7_step_04();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_30_sequence_7_step_04(void)
{
    wmap_step_actor_animation(&D_800D93C8, &D_801399C8);
    wmap_draw_actor_sprite(&D_800D93C8, g_wmap_focus_screen_position.packed, 0x8, 0xA, 0);
    if (--D_801B282C == 0)
    {
        D_801B2828 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_30_sequence_7_end(void)
{
    D_801B2828 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_30_run_sequence_8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2830 = 1;
        D_801B2834 = 1;
        return 1;
    }

    if (D_801B2830 < 0x6)
    {
        D_800D57E8[D_801B2830]();
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
void wmap_land_effect_30_sequence_8_reset(void)
{
    D_801B2830 = 1;
    D_801B2834 = 1;
}

/** @brief Configure the world-map actor and advance to its draw step. */
void wmap_land_effect_30_sequence_8_step_01(void)
{
    D_801399D4 = &D_80127538;
    D_800D93F4.scale_index = 0xF;
    D_800D93F4.sequence = 1;
    D_800D93F4.previous_sequence = -1;
    D_800D93F4.shade_step = 8;
    D_800D93F4.resource_index = 0;
    D_800D93F4.target_shade = 0x81;
    D_800D93F4.shade = 1;
    D_801B2834 = 0x1C;
    D_801B2830 += 1;
    wmap_land_effect_30_sequence_8_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_30_sequence_8_step_02(void)
{
    wmap_step_actor_animation(&D_800D93F4, &D_801399D0);
    wmap_draw_actor_sprite(&D_800D93F4, g_wmap_focus_screen_position.packed, 0x8, 0xA, 0);
    if (--D_801B2834 == 0)
    {
        D_801B2830 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_30_sequence_8_step_03(void)
{
    D_800D93F4.shade_step = 8;
    D_800D93F4.target_shade = 0;
    D_801B2834 = 0x8;
    D_801B2830 += 1;
    wmap_land_effect_30_sequence_8_step_04();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_30_sequence_8_step_04(void)
{
    wmap_step_actor_animation(&D_800D93F4, &D_801399D0);
    wmap_draw_actor_sprite(&D_800D93F4, g_wmap_focus_screen_position.packed, 0x8, 0xA, 0);
    if (--D_801B2834 == 0)
    {
        D_801B2830 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_30_sequence_8_end(void)
{
    D_801B2830 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_30_run_sequence_9(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2838 = 1;
        D_801B283C = 1;
        return 1;
    }

    if (D_801B2838 < 0x6)
    {
        D_800D5800[D_801B2838]();
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
void wmap_land_effect_30_sequence_9_reset(void)
{
    D_801B2838 = 1;
    D_801B283C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_30_sequence_9_step_01(void)
{
    D_801399DC = D_80127538;
    D_800D9420.scale_index = 0xF;
    D_800D9420.sequence = 2;
    D_800D9420.previous_sequence = -1;
    D_800D9420.shade_step = 8;
    D_800D9420.target_shade = 0x81;
    D_800D9420.resource_index = 0;
    D_800D9420.shade = 1;
    D_801B283C = 0x18;
    D_801B2838 += 1;
    wmap_land_effect_30_sequence_9_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_30_sequence_9_step_02(void)
{
    wmap_step_actor_animation(&D_800D9420, &D_801399D8);
    wmap_draw_actor_sprite(&D_800D9420, g_wmap_focus_screen_position.packed, 0x8, 0xA, 0);
    if (--D_801B283C == 0)
    {
        D_801B2838 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_30_sequence_9_step_03(void)
{
    D_800D9420.shade_step = 8;
    D_800D9420.target_shade = 0;
    D_801B283C = 0x8;
    D_801B2838 += 1;
    wmap_land_effect_30_sequence_9_step_04();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_30_sequence_9_step_04(void)
{
    wmap_step_actor_animation(&D_800D9420, &D_801399D8);
    wmap_draw_actor_sprite(&D_800D9420, g_wmap_focus_screen_position.packed, 0x8, 0xA, 0);
    if (--D_801B283C == 0)
    {
        D_801B2838 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_30_sequence_9_end(void)
{
    D_801B2838 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_30_run_sequence_10(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2840 = 1;
        D_801B2844 = 1;
        return 1;
    }

    if (D_801B2840 < 0x6)
    {
        D_800D5818[D_801B2840]();
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
void wmap_land_effect_30_sequence_10_reset(void)
{
    D_801B2840 = 1;
    D_801B2844 = 1;
}

/** @brief World-map step: clear a run of per-entry slots, then schedule the next step. */
void wmap_land_effect_30_sequence_10_step_01(void)
{
    s32 i;

    D_801B0FD0 = 0;
    D_80139980 = 0x7F;
    for (i = 0; i < 0x32; i++)
    {
        D_801AFBD0[i + 0x13].state = 0;
        D_80139988[i + 0x13].data = &D_80125538;
    }
    D_801B2844 = 0x32;
    D_801B2840 += 1;
    wmap_land_effect_30_sequence_10_step_02();
}

/** @brief Advance the effect on odd frames, draw it, and update the countdown. */
void wmap_land_effect_30_sequence_10_step_02(void)
{
    s32 remaining_ticks;

    if (D_8011CF74 & 1)
    {
        D_801B0FD0 += 1;
    }
    func_8006AFAC(0x13, 0x45, 0x13, 0x20, 0xFD0, 4, 0x20, 1);
    remaining_ticks = D_801B2844 - 1;
    D_801B2844 = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B2840 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_30_sequence_10_step_03(void)
{
    D_801B2844 = 0x20;
    D_801B2840 += 1;
    wmap_land_effect_30_sequence_10_step_04();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_30_sequence_10_end(void)
{
    D_801B2840 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_30_run_sequence_11(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2848 = 1;
        D_801B284C = 1;
        return 1;
    }

    if (D_801B2848 < 0x6)
    {
        D_800D5830[D_801B2848]();
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
void wmap_land_effect_30_sequence_11_reset(void)
{
    D_801B2848 = 1;
    D_801B284C = 1;
}

/** @brief World-map step: clear a run of per-entry slots, then schedule the next step. */
void wmap_land_effect_30_sequence_11_step_01(void)
{
    s32 i;

    D_801B0FD0 = 0;
    D_80139980 = 0x7F;
    for (i = 0; i < 0x8; i++)
    {
        D_801AFBD0[i + 0x54].state = 0;
        D_80139988[i + 0x54].data = &D_80125538;
    }
    D_801B284C = 2;
    D_801B2848 += 1;
    wmap_land_effect_30_sequence_11_step_02();
}

/** @brief Advance the effect on odd frames, draw it, and update the countdown. */
void wmap_land_effect_30_sequence_11_step_02(void)
{
    s32 remaining_ticks;

    if (D_8011CF74 & 1)
    {
        D_801B0FD0 += 1;
    }
    func_8006AFAC(0x54, 0x5C, 0x13, 0x20, 0x1000, 0x20, 8, 2);
    remaining_ticks = D_801B284C - 1;
    D_801B284C = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B2848 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_30_sequence_11_step_03(void)
{
    D_801B284C = 0x18;
    D_801B2848 += 1;
    wmap_land_effect_30_sequence_11_step_04();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_30_sequence_11_end(void)
{
    D_801B2848 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_30_run_sequence_12(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2850 = 1;
        D_801B2854 = 1;
        return 1;
    }

    if (D_801B2850 < 0x4)
    {
        D_800D5848[D_801B2850]();
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
void wmap_land_effect_30_sequence_12_reset(void)
{
    D_801B2850 = 1;
    D_801B2854 = 1;
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_30_sequence_12_end(void)
{
    D_801B2850 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_30_run_sequence_13(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2858 = 1;
        D_801B285C = 1;
        return 1;
    }

    if (D_801B2858 < 0x8)
    {
        D_800D5858[D_801B2858]();
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
void wmap_land_effect_30_sequence_13_reset(void)
{
    D_801B2858 = 1;
    D_801B285C = 1;
}

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void wmap_land_effect_30_sequence_13_step_02(void)
{
    s32 remaining;

    func_8006B328(0xC8, 0xF0, 2, -1, 0x12, 4, 0x168, 0x13, -0x32, 0x64, -0x32, 0x64, 0xB4, 0x81, 0x81, 8, 0);
    D_801B25D8 += 8;
    remaining = D_801B285C - 1;
    D_801B285C = remaining;
    if (remaining == 0)
    {
        D_801B2858 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_30_sequence_13_step_03(void)
{
    D_801B285C = 0x18;
    D_801B2858 += 1;
    wmap_land_effect_30_sequence_13_step_04();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void wmap_land_effect_30_sequence_13_step_04(void)
{
    func_8006B328(0xC8, 0xF0, 2, -1, 0x12, 4, 0x168, 0x13, -0x32, 0x64, -0x32, 0x64, 0xB4, 0x81, 0x81, 8, 0);
    if (--D_801B285C == 0)
    {
        D_801B2858 += 1;
    }
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_30_sequence_13_step_05(void)
{
    D_800DCEA8 = 0;
    D_801B285C = 0x18;
    D_801B2858 += 1;
    wmap_land_effect_30_sequence_13_step_06();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void wmap_land_effect_30_sequence_13_step_06(void)
{
    func_8006B328(0xC8, 0xF0, 2, -1, 0x12, 4, 0x168, 0x13, -0x32, 0x64, -0x32, 0x64, 0xB4, 0x81, 0x81, 8, 0);
    if (--D_801B285C == 0)
    {
        D_801B2858 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_30_sequence_13_end(void)
{
    D_801B2858 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_30_run_sequence_14(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2860 = 1;
        D_801B2864 = 1;
        return 1;
    }

    if (D_801B2860 < 0x4)
    {
        D_800D5878[D_801B2860]();
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
void wmap_land_effect_30_sequence_14_reset(void)
{
    D_801B2860 = 1;
    D_801B2864 = 1;
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_30_sequence_14_end(void)
{
    D_801B2860 += 1;
}
