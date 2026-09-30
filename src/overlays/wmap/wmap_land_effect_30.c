#include "wmap_main.h"
#include "wmap_land_effect_30.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_effect_primitives.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"

void func_80081508(void);
void func_80083A10(void);
void func_80081954(void);
void func_80081C74(void);
void func_80081CB0(void);
s32 func_80081D48(s32 arg0);
void func_80081CF4(void);
void func_80081D30(void);
s32 func_80082360(s32 arg0);
s32 func_800826A0(s32 arg0);
s32 func_80082B04(s32 arg0);
s32 func_80082D70(s32 arg0);
s32 func_80082FDC(s32 arg0);
s32 func_80083248(s32 arg0);
s32 func_800827F8(s32 arg0);
s32 func_800838D8(s32 arg0);
s32 func_80083CBC(s32 arg0);
s32 func_80083980(s32 arg0);
s32 func_800834B8(s32 arg0);
s32 func_80082500(s32 arg0);
s32 func_80082950(s32 arg0);
s32 func_800836C8(s32 arg0);
void func_8008246C(void);
void func_8008260C(void);
void func_80082A70(void);
void func_80082C14(void);
void func_80082CDC(void);
void func_80082E80(void);
void func_80082F48(void);
void func_800830EC(void);
void func_800831B4(void);
void func_8008335C(void);
void func_80083424(void);
void func_800835DC(void);
void func_800837EC(void);
void func_80083B04(void);
void func_80083BF4(void);

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
    void_ptr resource;
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
extern u8_ptr D_8011CF1C;
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
extern WmapStepHandlerSlot D_800D56F0[];
extern s32 D_8013B20C;
extern void func_80081CB0(void);
extern void func_80081D30(void);
extern s32 D_801B27F4;
extern WmapStepHandlerSlot D_800D5708[];
extern s32 D_801ADAE0;
extern WmapValueRecord D_80139290[][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 D_801B27FC;
extern WmapStepHandlerSlot D_800D5768[];
extern u8 D_8011D538[];
extern u8_ptr D_801399AC;
extern void func_8008246C(void);
extern s32 D_801B2804;
extern WmapStepHandlerSlot D_800D5778[];
extern u8 D_80121538[];
extern u8_ptr D_801399B4;
extern void func_8008260C(void);
extern WmapStepHandlerSlot D_800D5788[];
extern WmapStepHandlerSlot D_800D5798[];
extern s32 D_801B281C;
extern WmapStepHandlerSlot D_800D57A8[];
extern u8_ptr D_801399BC;
extern s32 D_801B2824;
extern WmapStepHandlerSlot D_800D57B8[];
extern u8 D_8011F538[];
extern u8_ptr D_801399C4;
extern void func_80082C14(void);
extern void func_80082CDC(void);
extern s32 D_801B282C;
extern WmapStepHandlerSlot D_800D57D0[];
extern u8_ptr D_801399CC;
extern void func_80082E80(void);
extern void func_80082F48(void);
extern s32 D_801B2834;
extern WmapStepHandlerSlot D_800D57E8[];
extern void_ptr D_801399D4;
extern void func_800831B4(void);
extern s32 D_801B283C;
extern WmapStepHandlerSlot D_800D5800[];
extern u8_ptr D_801399DC;
extern void func_80083424(void);
extern WmapStepHandlerSlot D_800D5818[];
extern WmapStepHandlerSlot D_800D5830[];
extern WmapStepHandlerSlot D_800D5848[];
extern WmapStepHandlerSlot D_800D5858[];
extern WmapStepHandlerSlot D_800D5878[];

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

extern s32_ptr D_80139280;

extern WmapMotion D_801AFBD0[];

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_80080FC8(void)
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
void func_800810C8(void)
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
void func_800811C8(void)
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
void func_80081294(void)
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
void func_80081360(void)
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
    func_80081508();
}

/** @brief Advance spaced effect actors and copy their trailing samples. */
void func_80081508(void)
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
void func_8008172C(void)
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
    func_80083A10();
}

/** @brief Initialize effect actors at regular angular intervals. */
void func_800817F0(void)
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
    func_80081954();
}

/** @brief Advance spaced effect actors and copy their trailing samples. */
void func_80081954(void)
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
s32 func_80081BA0(s32 arg0)
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
        PS1_CALL(D_800D56F0[D_801B27E8])();
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
void func_80081C18(void)
{
    D_801B27E8 = 1;
    D_801B27EC = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_80081C30(void)
{
    wmap_start_sequence(wmap_run_land_focus);
    D_8013B20C = 1;
    D_801B27E8 += 1;
    func_80081C74();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80081C74(void)
{
    if (D_8013B20C == 0)
    {
        D_801B27E8 += 1;
        func_80081CB0();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_80081CB0(void)
{
    wmap_start_sequence(func_80081D48);
    D_8013B20C = 1;
    D_801B27E8 += 1;
    func_80081CF4();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80081CF4(void)
{
    if (D_8013B20C == 0)
    {
        D_801B27E8 += 1;
        func_80081D30();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80081D30(void)
{
    D_801B27E8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80081D48(s32 arg0)
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
        PS1_CALL(D_800D5708[D_801B27F0])();
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
void func_80081DC0(void)
{
    D_801B27F0 = 1;
    D_801B27F4 = 1;
}

/** @brief Set world-map color, play sound 30, register two callbacks, and begin an eight-tick delay. */
void func_80081DD8(void)
{
    wmap_start_map_tint(0x122840);
    g_wmap_backdrop_target_level = 4;
    wmap_play_sound(0x1E, 0x80);
    wmap_start_sequence(&func_80082360);
    wmap_start_sequence(&func_800826A0);
    D_801B27F4 = 8;
    D_801B27F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80081E44(void)
{
    if (--D_801B27F4 == 0)
    {
        D_801B27F0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80081E78(void)
{
    wmap_start_sequence(func_80082B04);
    D_801B27F4 = 0x4;
    D_801B27F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80081EB4(void)
{
    if (--D_801B27F4 == 0)
    {
        D_801B27F0 += 1;
    }
}

/** @brief World-map step handler: register the next callback and advance the counter. */
void func_80081EE8(void)
{
    wmap_start_sequence(func_80082D70);
    D_801ADAE0 = 1;
    D_801B27F4 = 4;
    D_801B27F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80081F30(void)
{
    if (--D_801B27F4 == 0)
    {
        D_801B27F0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80081F64(void)
{
    wmap_start_sequence(func_80082FDC);
    D_801B27F4 = 0x4;
    D_801B27F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80081FA0(void)
{
    if (--D_801B27F4 == 0)
    {
        D_801B27F0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80081FD4(void)
{
    wmap_start_sequence(func_80083248);
    D_801B27F4 = 0x24;
    D_801B27F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80082010(void)
{
    if (--D_801B27F4 == 0)
    {
        D_801B27F0 += 1;
    }
}

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void func_80082044(void)
{
    wmap_start_sequence(func_800827F8);
    wmap_start_sequence(func_800838D8);
    wmap_start_sequence(func_80083CBC);
    D_801B27F4 = 0x38;
    D_801B27F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80082098(void)
{
    if (--D_801B27F4 == 0)
    {
        D_801B27F0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800820CC(void)
{
    wmap_start_sequence(func_80083980);
    D_801B27F4 = 0xA;
    D_801B27F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80082108(void)
{
    if (--D_801B27F4 == 0)
    {
        D_801B27F0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8008213C(void)
{
    wmap_start_sequence(func_800834B8);
    D_801B27F4 = 0x10;
    D_801B27F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80082178(void)
{
    if (--D_801B27F4 == 0)
    {
        D_801B27F0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800821AC(void)
{
    wmap_start_sequence(func_80082500);
    D_801B27F4 = 0x30;
    D_801B27F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800821E8(void)
{
    if (--D_801B27F4 == 0)
    {
        D_801B27F0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8008221C(void)
{
    wmap_start_sequence(func_80082950);
    D_801B27F4 = 0x2;
    D_801B27F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80082258(void)
{
    if (--D_801B27F4 == 0)
    {
        D_801B27F0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8008228C(void)
{
    wmap_start_sequence(func_800836C8);
    D_801B27F4 = 0x28;
    D_801B27F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800822C8(void)
{
    if (--D_801B27F4 == 0)
    {
        D_801B27F0 += 1;
    }
}

/** @brief Update the selected world-map land record and advance the sequence step. */
void func_800822FC(void)
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
s32 func_80082360(s32 arg0)
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
        PS1_CALL(D_800D5768[D_801B27F8])();
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
void func_800823D8(void)
{
    D_801B27F8 = 1;
    D_801B27FC = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_800823F0(void)
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
    func_8008246C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8008246C(void)
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
void func_800824E8(void)
{
    D_801B27F8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80082500(s32 arg0)
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
        PS1_CALL(D_800D5778[D_801B2800])();
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
void func_80082578(void)
{
    D_801B2800 = 1;
    D_801B2804 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80082590(void)
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
    func_8008260C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8008260C(void)
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
void func_80082688(void)
{
    D_801B2800 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800826A0(s32 arg0)
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
        PS1_CALL(D_800D5788[D_801B2808])();
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
void func_80082718(void)
{
    D_801B2808 = 1;
    D_801B280C = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_80082730(void)
{
    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    D_801B280C = 0x14;
    D_801B2808 += 1;
    func_80080FC8();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800827E0(void)
{
    D_801B2808 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800827F8(s32 arg0)
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
        PS1_CALL(D_800D5798[D_801B2810])();
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
void func_80082870(void)
{
    D_801B2810 = 1;
    D_801B2814 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_80082888(void)
{
    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    D_801B2814 = 0x40;
    D_801B2810 += 1;
    func_800810C8();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80082938(void)
{
    D_801B2810 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80082950(s32 arg0)
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
        PS1_CALL(D_800D57A8[D_801B2818])();
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
void func_800829C8(void)
{
    D_801B2818 = 1;
    D_801B281C = 1;
}

/** @brief Configure the actor, save its screen position, and begin a 44-tick delay. */
void func_800829E0(void)
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
    func_80082A70();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80082A70(void)
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
void func_80082AEC(void)
{
    D_801B2818 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80082B04(s32 arg0)
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
        PS1_CALL(D_800D57B8[D_801B2820])();
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
void func_80082B7C(void)
{
    D_801B2820 = 1;
    D_801B2824 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80082B94(void)
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
    func_80082C14();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80082C14(void)
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
void func_80082C90(void)
{
    D_800D939C.shade_step = 2;
    D_800D939C.target_shade = 0;
    D_801B2824 = 0x8;
    D_801B2820 += 1;
    func_80082CDC();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80082CDC(void)
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
void func_80082D58(void)
{
    D_801B2820 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80082D70(s32 arg0)
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
        PS1_CALL(D_800D57D0[D_801B2828])();
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
void func_80082DE8(void)
{
    D_801B2828 = 1;
    D_801B282C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80082E00(void)
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
    func_80082E80();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80082E80(void)
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
void func_80082EFC(void)
{
    D_800D93C8.shade_step = 8;
    D_800D93C8.target_shade = 0;
    D_801B282C = 0x8;
    D_801B2828 += 1;
    func_80082F48();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80082F48(void)
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
void func_80082FC4(void)
{
    D_801B2828 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80082FDC(s32 arg0)
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
        PS1_CALL(D_800D57E8[D_801B2830])();
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
void func_80083054(void)
{
    D_801B2830 = 1;
    D_801B2834 = 1;
}

/** @brief Configure the world-map actor and advance to its draw step. */
void func_8008306C(void)
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
    func_800830EC();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800830EC(void)
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
void func_80083168(void)
{
    D_800D93F4.shade_step = 8;
    D_800D93F4.target_shade = 0;
    D_801B2834 = 0x8;
    D_801B2830 += 1;
    func_800831B4();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800831B4(void)
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
void func_80083230(void)
{
    D_801B2830 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80083248(s32 arg0)
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
        PS1_CALL(D_800D5800[D_801B2838])();
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
void func_800832C0(void)
{
    D_801B2838 = 1;
    D_801B283C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_800832D8(void)
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
    func_8008335C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8008335C(void)
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
void func_800833D8(void)
{
    D_800D9420.shade_step = 8;
    D_800D9420.target_shade = 0;
    D_801B283C = 0x8;
    D_801B2838 += 1;
    func_80083424();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80083424(void)
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
void func_800834A0(void)
{
    D_801B2838 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800834B8(s32 arg0)
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
        PS1_CALL(D_800D5818[D_801B2840])();
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
void func_80083530(void)
{
    D_801B2840 = 1;
    D_801B2844 = 1;
}

/** @brief World-map step: clear a run of per-entry slots, then schedule the next step. */
void func_80083548(void)
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
    func_800835DC();
}

/** @brief Advance the effect on odd frames, draw it, and update the countdown. */
void func_800835DC(void)
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
void func_80083678(void)
{
    D_801B2844 = 0x20;
    D_801B2840 += 1;
    func_800811C8();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800836B0(void)
{
    D_801B2840 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800836C8(s32 arg0)
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
        PS1_CALL(D_800D5830[D_801B2848])();
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
void func_80083740(void)
{
    D_801B2848 = 1;
    D_801B284C = 1;
}

/** @brief World-map step: clear a run of per-entry slots, then schedule the next step. */
void func_80083758(void)
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
    func_800837EC();
}

/** @brief Advance the effect on odd frames, draw it, and update the countdown. */
void func_800837EC(void)
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
void func_80083888(void)
{
    D_801B284C = 0x18;
    D_801B2848 += 1;
    func_80081294();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800838C0(void)
{
    D_801B2848 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800838D8(s32 arg0)
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
        PS1_CALL(D_800D5848[D_801B2850])();
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
void func_80083950(void)
{
    D_801B2850 = 1;
    D_801B2854 = 1;
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80083968(void)
{
    D_801B2850 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80083980(s32 arg0)
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
        PS1_CALL(D_800D5858[D_801B2858])();
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
void func_800839F8(void)
{
    D_801B2858 = 1;
    D_801B285C = 1;
}

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void func_80083A10(void)
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
void func_80083ACC(void)
{
    D_801B285C = 0x18;
    D_801B2858 += 1;
    func_80083B04();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void func_80083B04(void)
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
void func_80083BB4(void)
{
    D_800DCEA8 = 0;
    D_801B285C = 0x18;
    D_801B2858 += 1;
    func_80083BF4();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void func_80083BF4(void)
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
void func_80083CA4(void)
{
    D_801B2858 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80083CBC(s32 arg0)
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
        PS1_CALL(D_800D5878[D_801B2860])();
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
void func_80083D34(void)
{
    D_801B2860 = 1;
    D_801B2864 = 1;
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80083D4C(void)
{
    D_801B2860 += 1;
}
