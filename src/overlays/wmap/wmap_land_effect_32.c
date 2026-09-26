#include "wmap_land_effect_32.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"

void func_8007A32C(void);
void func_8007AF6C(void);
void func_80079A60(void);
void func_80079A9C(void);
s32 func_80079B34(s32 arg0);
void func_80079AE0(void);
void func_80079B1C(void);
s32 func_80079F90(s32 arg0);
s32 func_8007AA60(s32 arg0);
s32 func_8007ABB0(s32 arg0);
s32 func_8007A478(s32 arg0);
s32 func_8007A600(s32 arg0);
s32 func_8007A788(s32 arg0);
s32 func_8007A2A4(s32 arg0);
s32 func_8007A910(s32 arg0);
s32 func_8007AD48(s32 arg0);
s32 func_8007AEDC(s32 arg0);
void func_8007A09C(void);
void func_8007A150(void);
void func_8007A210(void);
void func_8007A3D0(void);
void func_8007ACB4(void);
void func_8007AE48(void);
void func_8007B020(void);

/** @brief Configure effect parameters and reset its animation resource slots. */
void func_80078E78(void)
{
/** @brief World-map 8-byte slot: only the +4 pointer field is written here. */
typedef struct
{
    s32 field_00;
    void *field_04;
} WmapSlot8;

/** @brief World-map 0x14-byte slot: only the leading halfword is cleared here. */
typedef struct
{
    s16 field_00;
    u8 pad_02[0x12];
} WmapSlot14;

extern s32 D_80139234;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_8013924C;
extern s32 D_80139250;
extern s32 D_80139260;
extern s32 D_80139264;
extern s32 D_80139268;
extern s32 D_8013926C;
extern s32 D_80139284;
extern s32 D_801B24B4;
extern s32 D_801B0FD0;
extern s32 D_801B2698;
extern s32 D_801B269C;
extern WmapSlot8 D_80139988[];
extern WmapSlot14 D_801AFBD0[];
extern u8 D_8011D538[];

    s32 i;

    D_801B0FD0 = 24;
    D_801B24B4 = 127;
    D_80139234 = 1;
    D_8013923C = 10;
    D_80139240 = 20;
    D_8013924C = 2;
    D_80139250 = 2;
    D_80139260 = 1500;
    D_80139264 = 100;
    D_80139268 = 19;
    D_8013926C = 0;
    D_80139284 = 1000;
    for (i = 0; i < 24; i++)
    {
        D_801AFBD0[i + D_80139264].field_00 = 0;
        D_80139988[i + 104].field_04 = D_8011D538;
    }
    D_801B269C = 40;
    D_801B2698++;
    func_8007A32C();
}

/** @brief World-map animated element: advance phase, draw, and tick refcount. */
void func_80078F8C(void)
{
extern s32 D_80139888;
extern u16 D_8013B240;
extern s32 D_8011CF28;
extern s32 D_80182DE4;
extern s32 D_801B26A0;
extern s32 D_801B26A4;
extern void PushMatrix(void);
extern void PopMatrix(void);

    s32 *p = &D_80139888;
    u16 *q;
    s32 v;
    s32 t;
    s32 c;

    v = p[2] - 0x5DC;
    p[2] = v;
    if (v <= 0x9C3F)
    {
        p[2] = 0x9C40;
    }
    PushMatrix();
    q = &D_8013B240;
    wmap_set_model_transform(p, q);
    wmap_draw_model_default(D_8011CF28, 0, 0xC, 0x35, 0x7800, 1, D_80182DE4);
    t = D_80182DE4 + 0x10;
    D_80182DE4 = t;
    if (t >= 0x82)
    {
        D_80182DE4 = 0x81;
    }
    *(u16 *)((u8 *)q + 4) += 2;
    PopMatrix();
    c = D_801B26A4 - 1;
    D_801B26A4 = c;
    if (c == 0)
    {
        D_801B26A0 += 1;
    }
}

/** @brief World-map animated element: advance phase, draw while active, then tick refcount. */
void func_80079088(void)
{
extern s32 D_80139888;
extern u16 D_8013B240;
extern s32 D_8011CF28;
extern s32 D_80182DE4;
extern s32 D_801B26A0;
extern s32 D_801B26A4;
extern void PushMatrix(void);
extern void PopMatrix(void);

    s32 *p = &D_80139888;
    u16 *q;
    s32 v;
    s32 t;
    s32 c;

    v = p[2] - 0x5DC;
    p[2] = v;
    if (v <= 0x9C3F)
    {
        p[2] = 0x9C40;
    }
    PushMatrix();
    q = &D_8013B240;
    wmap_set_model_transform(p, q);
    if (D_80182DE4 != 0)
    {
        wmap_draw_model_default(D_8011CF28, 0, 0xC, 0x35, 0x7800, 1, D_80182DE4);
        t = D_80182DE4 - 2;
        D_80182DE4 = t;
        if (t < 0)
        {
            D_80182DE4 = 0;
        }
        *(u16 *)((u8 *)q + 4) += 2;
    }
    PopMatrix();
    c = D_801B26A4 - 1;
    D_801B26A4 = c;
    if (c == 0)
    {
        D_801B26A0 += 1;
    }
}

/** @brief World-map animated element: advance phase, draw, and tick refcount. */
void func_80079188(void)
{
extern s32 D_80139898;
extern u16 D_801B2670;
extern s32 D_8011CF2C;
extern s32 D_80182DE8;
extern s32 D_801B26A8;
extern s32 D_801B26AC;
extern void PushMatrix(void);
extern void PopMatrix(void);

    s32 *p = &D_80139898;
    u16 *q;
    s32 v;
    s32 t;
    s32 c;

    v = p[2] - 0x5DC;
    p[2] = v;
    if (v <= 0x9C3F)
    {
        p[2] = 0x9C40;
    }
    PushMatrix();
    q = &D_801B2670;
    wmap_set_model_transform(p, q);
    wmap_draw_model_default(D_8011CF2C, 0, 0xC, 0x35, 0x7800, 1, D_80182DE8);
    t = D_80182DE8 + 0x10;
    D_80182DE8 = t;
    if (t >= 0x82)
    {
        D_80182DE8 = 0x81;
    }
    *(u16 *)((u8 *)q + 4) -= 3;
    PopMatrix();
    c = D_801B26AC - 1;
    D_801B26AC = c;
    if (c == 0)
    {
        D_801B26A8 += 1;
    }
}

/** @brief World-map animated element: advance phase, draw while active, then tick refcount. */
void func_80079284(void)
{
extern s32 D_80139898;
extern u16 D_801B2670;
extern s32 D_8011CF2C;
extern s32 D_80182DE8;
extern s32 D_801B26A8;
extern s32 D_801B26AC;
extern void PushMatrix(void);
extern void PopMatrix(void);

    s32 *p = &D_80139898;
    u16 *q;
    s32 v;
    s32 t;
    s32 c;

    v = p[2] - 0x5DC;
    p[2] = v;
    if (v <= 0x9C3F)
    {
        p[2] = 0x9C40;
    }
    PushMatrix();
    q = &D_801B2670;
    wmap_set_model_transform(p, q);
    if (D_80182DE8 != 0)
    {
        wmap_draw_model_default(D_8011CF2C, 0, 0xC, 0x35, 0x7800, 1, D_80182DE8);
        t = D_80182DE8 - 2;
        D_80182DE8 = t;
        if (t < 0)
        {
            D_80182DE8 = 0;
        }
        *(u16 *)((u8 *)q + 4) -= 3;
    }
    PopMatrix();
    c = D_801B26AC - 1;
    D_801B26AC = c;
    if (c == 0)
    {
        D_801B26A8 += 1;
    }
}

/** @brief World-map animated element: advance phase, draw, and tick refcount. */
void func_80079384(void)
{
extern s32 D_801B2660;
extern u16 D_801B2678;
extern s32 D_8011CF30;
extern s32 D_80182DEC;
extern s32 D_801B26B0;
extern s32 D_801B26B4;
extern void PushMatrix(void);
extern void PopMatrix(void);

    s32 *p = &D_801B2660;
    u16 *q;
    s32 v;
    s32 t;
    s32 c;

    v = p[2] - 0x5DC;
    p[2] = v;
    if (v <= 0x9C3F)
    {
        p[2] = 0x9C40;
    }
    PushMatrix();
    q = &D_801B2678;
    wmap_set_model_transform(p, q);
    wmap_draw_model_default(D_8011CF30, 0, 0xC, 0x35, 0x7800, 1, D_80182DEC);
    t = D_80182DEC + 0x10;
    D_80182DEC = t;
    if (t >= 0x82)
    {
        D_80182DEC = 0x81;
    }
    *(u16 *)((u8 *)q + 4) += 4;
    PopMatrix();
    c = D_801B26B4 - 1;
    D_801B26B4 = c;
    if (c == 0)
    {
        D_801B26B0 += 1;
    }
}

/** @brief World-map animated element: advance phase, draw while active, then tick refcount. */
void func_80079480(void)
{
extern s32 D_801B2660;
extern u16 D_801B2678;
extern s32 D_8011CF30;
extern s32 D_80182DEC;
extern s32 D_801B26B0;
extern s32 D_801B26B4;
extern void PushMatrix(void);
extern void PopMatrix(void);

    s32 *p = &D_801B2660;
    u16 *q;
    s32 v;
    s32 t;
    s32 c;

    v = p[2] - 0x5DC;
    p[2] = v;
    if (v <= 0x9C3F)
    {
        p[2] = 0x9C40;
    }
    PushMatrix();
    q = &D_801B2678;
    wmap_set_model_transform(p, q);
    if (D_80182DEC != 0)
    {
        wmap_draw_model_default(D_8011CF30, 0, 0xC, 0x35, 0x7800, 1, D_80182DEC);
        t = D_80182DEC - 2;
        D_80182DEC = t;
        if (t < 0)
        {
            D_80182DEC = 0;
        }
        *(u16 *)((u8 *)q + 4) += 4;
    }
    PopMatrix();
    c = D_801B26B4 - 1;
    D_801B26B4 = c;
    if (c == 0)
    {
        D_801B26B0 += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_80079580(void)
{
extern s32 D_80139870[];
extern SVECTOR D_8013B238;
extern VECTOR D_8011CF60;
extern s32 D_80182DF0;
extern u8 D_800DCF18[];
extern s32 D_801B26BC;
extern s32 D_801B26B8;

    MATRIX m;
    s32 x;

    x = D_80139870[2] - 0xDAC;
    D_80139870[2] = x;
    if (x < 0x2710)
    {
        D_80139870[2] = 0x2710;
    }

    PushMatrix();
    RotMatrix(&D_8013B238, &m);
    TransMatrix(&m, &D_8011CF60);
    SetRotMatrix(&m);
    SetTransMatrix(&m);

    if (D_80182DF0 != 0)
    {
        wmap_draw_model_default((s32)D_800DCF18, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DF0);
        D_80182DF0 -= 0x5;
        if (D_80182DF0 < 0)
        {
            D_80182DF0 = 0;
        }
    }

    PopMatrix();
    if (--D_801B26BC == 0)
    {
        D_801B26B8 += 1;
    }
}

/** @brief Draw and fade the transformed effect, then advance its countdown. */
void func_80079680(void)
{
extern void *D_8011CF1C;
extern VECTOR D_8011CF60;
extern SVECTOR g_wmap_camera_rotation;
extern s32 D_80182DF4;
extern VECTOR D_801B2478;
extern SVECTOR D_801B24A8;
extern s32 D_801B26C0;
extern s32 D_801B26C4;

    MATRIX base;
    MATRIX effect;
    s32 depth;
    s32 intensity;
    s32 remaining;

    depth = D_801B2478.vz - 0x7D0;
    D_801B2478.vz = depth;
    if (depth < 0x1F40)
    {
        D_801B2478.vz = 0x1F40;
    }
    PushMatrix();
    RotMatrix(&g_wmap_camera_rotation, &base);
    TransMatrix(&base, &D_801B2478);
    SetRotMatrix(&base);
    SetTransMatrix(&base);
    RotMatrix(&D_801B24A8, &effect);
    TransMatrix(&effect, &D_8011CF60);
    CompMatrix(&base, &effect, &effect);
    SetRotMatrix(&effect);
    SetTransMatrix(&effect);
    if (D_80182DF4 != 0)
    {
        wmap_draw_model_default(D_8011CF1C, 0, 0xC, 0x35, 0x7800, 1, D_80182DF4);
        intensity = D_80182DF4 - 6;
        D_80182DF4 = intensity;
        if (intensity < 0)
        {
            D_80182DF4 = 0;
        }
    }
    PopMatrix();
    remaining = D_801B26C4 - 1;
    D_801B26C4 = remaining;
    if (remaining == 0)
    {
        D_801B26C0 += 1;
    }
}

/** @brief Initialize the effect actors and randomized motion angles. */
void func_800797C4(void)
{
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
} WmapConfigA;

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

extern s32 D_801B0FD0;
extern s32 D_801B25D8;
extern s32 D_80139234;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_8013924C;
extern s32 D_80139250;
extern s32 D_80139260;
extern s32 D_80139264;
extern s32 D_80139268;
extern s32 D_8013926C;
extern s32 D_80139284;
extern s32 D_8013B264;
extern s32 D_8013B270;
extern s32 D_8013B278;
extern s32 D_8013B280;
extern s32 D_8013B284;
extern s32 D_801B26D8;
extern s32 D_801B26DC;
extern WmapConfigA D_800DB578[];
extern WmapMotion D_801B0B70[];
extern WmapResource D_80139988[];
extern u8 D_8011D538[];
extern s32 rand(void);

    s32 i;
    WmapConfigA *actor;
    WmapMotion *motion;

    D_801B0FD0 = 40;
    D_801B25D8 = 256;
    D_80139234 = 48;
    D_8013923C = 10;
    D_80139240 = 60;
    D_8013924C = 2;
    D_80139250 = -1;
    D_80139260 = 8000;
    D_80139264 = 200;
    D_80139268 = 19;
    D_8013926C = 1;
    D_80139284 = 200;
    D_8013B264 = 54;
    D_8013B270 = 1;
    D_8013B278 = 30;
    D_8013B280 = 1;
    D_8013B284 = 3;
    for (i = 0; i < 40; i++)
    {
        actor = &D_800DB578[i];
        motion = &D_801B0B70[i];
        D_80139988[i + 204].resource = D_8011D538;
        actor->field_06 = 15;
        actor->field_10 = -1;
        actor->field_22 = 128;
        actor->field_24 = 128;
        actor->field_02 = 0;
        actor->field_26 = 0;
        actor->field_0E = D_8013926C;
        motion->state = 1;
        motion->angle = rand() & 0xFFF;
        motion->scale = 60;
        motion->field_0E = 50;
        motion->x = 0;
        motion->z = D_80139284;
    }
    D_801B26DC = 90;
    D_801B26D8++;
    func_8007AF6C();
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8007998C(s32 arg0)
{
extern u32 D_801B2680;
extern s32 D_801B2684;
extern void (*D_800D5280[])(void);
extern s32 D_8013B20C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2680 = 1;
        D_801B2684 = 1;
        return 1;
    }

    if (D_801B2680 < 0x6)
    {
        D_800D5280[D_801B2680]();
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
void func_80079A04(void)
{
extern u32 D_801B2680;
extern s32 D_801B2684;
extern void (*D_800D5280[])(void);
extern s32 D_8013B20C;

    D_801B2680 = 1;
    D_801B2684 = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_80079A1C(void)
{
extern u32 D_801B2680;
extern s32 D_801B2684;
extern void (*D_800D5280[])(void);
extern s32 D_8013B20C;

    wmap_start_sequence(wmap_run_land_focus);
    D_8013B20C = 1;
    D_801B2680 += 1;
    func_80079A60();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80079A60(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2680;

    if (D_8013B20C == 0)
    {
        D_801B2680 += 1;
        func_80079A9C();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_80079A9C(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2680;
extern void func_80079A9C(void);

    wmap_start_sequence(func_80079B34);
    D_8013B20C = 1;
    D_801B2680 += 1;
    func_80079AE0();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80079AE0(void)
{
extern s32 D_801B2680;
extern s32 D_8013B20C;

    if (D_8013B20C == 0)
    {
        D_801B2680 += 1;
        func_80079B1C();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80079B1C(void)
{
extern s32 D_801B2680;
extern s32 D_8013B20C;
extern void func_80079B1C(void);

    D_801B2680 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80079B34(s32 arg0)
{
extern u32 D_801B2688;
extern s32 D_801B268C;
extern void (*D_800D5298[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2688 = 1;
        D_801B268C = 1;
        return 1;
    }

    if (D_801B2688 < 0x10)
    {
        D_800D5298[D_801B2688]();
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
void func_80079BAC(void)
{
extern u32 D_801B2688;
extern s32 D_801B268C;
extern void (*D_800D5298[])(void);

    D_801B2688 = 1;
    D_801B268C = 1;
}

/** @brief Set color and flags, play sound 26, and register three sequence callbacks. */
void func_80079BC4(void)
{
extern s32 D_8013B208;
extern s32 D_801ADAE0;
extern s32 D_801B2688;
extern s32 D_801B268C;

    D_8013B208 = 1;
    wmap_start_map_tint(0x202020);
    wmap_play_sound(0x1A, 0x80);
    wmap_start_sequence(&func_80079F90);
    D_801ADAE0 = 1;
    wmap_start_sequence(&func_8007ABB0);
    wmap_start_sequence(&func_8007AA60);
    D_801B268C = 8;
    D_801B2688 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80079C4C(void)
{
extern s32 D_801B268C;
extern s32 D_801B2688;

    if (--D_801B268C == 0)
    {
        D_801B2688 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80079C80(void)
{
extern s32 D_801B268C;
extern s32 D_801B2688;

    wmap_start_sequence(func_8007A478);
    D_801B268C = 0x4;
    D_801B2688 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80079CBC(void)
{
extern s32 D_801B268C;
extern s32 D_801B2688;

    if (--D_801B268C == 0)
    {
        D_801B2688 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80079CF0(void)
{
extern s32 D_801B268C;
extern s32 D_801B2688;

    wmap_start_sequence(func_8007A600);
    D_801B268C = 0x4;
    D_801B2688 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80079D2C(void)
{
extern s32 D_801B268C;
extern s32 D_801B2688;

    if (--D_801B268C == 0)
    {
        D_801B2688 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80079D60(void)
{
extern s32 D_801B268C;
extern s32 D_801B2688;

    wmap_start_sequence(func_8007A788);
    D_801B268C = 0xF;
    D_801B2688 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80079D9C(void)
{
extern s32 D_801B268C;
extern s32 D_801B2688;

    if (--D_801B268C == 0)
    {
        D_801B2688 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80079DD0(void)
{
extern s32 D_801B268C;
extern s32 D_801B2688;

    wmap_start_sequence(func_8007A2A4);
    D_801B268C = 0x30;
    D_801B2688 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80079E0C(void)
{
extern s32 D_801B268C;
extern s32 D_801B2688;

    if (--D_801B268C == 0)
    {
        D_801B2688 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_80079E40(void)
{
extern s32 D_801B268C;
extern s32 D_801B2688;

    wmap_start_sequence(func_8007A910);
    wmap_start_sequence(func_8007AD48);
    D_801B268C = 0x42;
    D_801B2688 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80079E88(void)
{
extern s32 D_801B268C;
extern s32 D_801B2688;

    if (--D_801B268C == 0)
    {
        D_801B2688 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80079EBC(void)
{
extern s32 D_801B268C;
extern s32 D_801B2688;

    wmap_start_sequence(func_8007AEDC);
    D_801B268C = 0x83;
    D_801B2688 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80079EF8(void)
{
extern s32 D_801B268C;
extern s32 D_801B2688;

    if (--D_801B268C == 0)
    {
        D_801B2688 += 1;
    }
}

void func_80079F2C(void)
{
/** @brief First word of a 40-byte world-map cell. */
typedef struct
{
    s32 value;
    u8 unknown_04[36];
} WmapValueRecord;

extern s32 D_8013B20C;
extern WmapValueRecord D_80139290[][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 D_801B2688;

    D_8013B20C = 0;
    D_80139290[D_8011D510][D_8011D530].value = D_8011D4FC | 0x100;
    D_801B2688 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_80079F90(s32 arg0)
{
extern u32 D_801B2690;
extern s32 D_801B2694;
extern void (*D_800D52D8[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2690 = 1;
        D_801B2694 = 1;
    }

    if (D_801B2690 < 0x8)
    {
        D_800D52D8[D_801B2690]();
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
void func_8007A000(void)
{
extern u32 D_801B2690;
extern s32 D_801B2694;
extern void (*D_800D52D8[])(void);

    D_801B2690 = 1;
    D_801B2694 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_8007A018(void)
{
extern u8* D_801399B4;
extern u8 D_8011D538[];
extern u8 D_800D9344[];
extern s32 D_801B2690;
extern s32 D_801B2694;

    D_801399B4 = D_8011D538;
    D_800D9344[0x6] = 0xF;
    *(s16*)&D_800D9344[0xE] = 2;
    *(s16*)&D_800D9344[0x10] = -1;
    *(s16*)&D_800D9344[0x26] = 0x10;
    *(s16*)&D_800D9344[0x22] = 0x81;
    *(s16*)&D_800D9344[0x2] = 0;
    *(s16*)&D_800D9344[0x24] = 1;
    D_801B2694 = 0x10;
    D_801B2690 += 1;
    func_8007A09C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8007A09C(void)
{
extern s32 D_801B2694;
extern s32 D_801B2690;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 g_wmap_focus_screen_position;

    wmap_step_actor_animation(D_800D9344, D_801399B0);
    wmap_draw_actor_sprite(D_800D9344, g_wmap_focus_screen_position, 0x13, 0xB, 0);
    if (--D_801B2694 == 0)
    {
        D_801B2690 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007A118(void)
{
extern s32 D_801B2694;
extern s32 D_801B2690;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 g_wmap_focus_screen_position;

    D_801B2694 = 0x8;
    D_801B2690 += 1;
    func_8007A150();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8007A150(void)
{
extern void func_8007A150(void);
extern s32 D_801B2694;
extern s32 D_801B2690;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 g_wmap_focus_screen_position;

    wmap_step_actor_animation(D_800D9344, D_801399B0);
    wmap_draw_actor_sprite(D_800D9344, g_wmap_focus_screen_position, 0x13, 0xB, 0);
    if (--D_801B2694 == 0)
    {
        D_801B2690 += 1;
    }
}

/**
 * @brief Kick off a world-map step: arm a flag and wait, bump the index, run it.
 */
void func_8007A1CC(void)
{
extern s16 D_800D9366;
extern s32 D_801B2690;
extern s32 D_801B2694;

    D_800D9366 = 1;
    D_801B2694 = 0x10;
    D_801B2690 += 1;
    func_8007A210();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8007A210(void)
{
extern s32 D_801B2690;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 g_wmap_focus_screen_position;
extern s32 D_801B2694;

    wmap_step_actor_animation(D_800D9344, D_801399B0);
    wmap_draw_actor_sprite(D_800D9344, g_wmap_focus_screen_position, 0x13, 0xB, 0);
    if (--D_801B2694 == 0)
    {
        D_801B2690 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007A28C(void)
{
extern s32 D_801B2690;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 g_wmap_focus_screen_position;
extern s32 D_801B2694;

    D_801B2690 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_8007A2A4(s32 arg0)
{
extern u32 D_801B2698;
extern s32 D_801B269C;
extern void (*D_800D52F8[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2698 = 1;
        D_801B269C = 1;
    }

    if (D_801B2698 < 0x6)
    {
        D_800D52F8[D_801B2698]();
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
void func_8007A314(void)
{
extern u32 D_801B2698;
extern s32 D_801B269C;
extern void (*D_800D52F8[])(void);

    D_801B2698 = 1;
    D_801B269C = 1;
}

/** @brief World-map step handler: spawn a sub-object and expire the step counter. */
void func_8007A32C(void)
{
extern u8 D_800DA448[];
extern u8 D_80139CC8[];
extern s32 D_801B24B4;
extern s32 D_801B2698;
extern s32 D_801B269C;

    func_8006CFE4(D_800DA448, D_80139CC8, 0x18, D_801B24B4, D_801B24B4, 0);
    if (--D_801B269C == 0)
    {
        D_801B2698 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007A398(void)
{
extern s32 D_801B269C;
extern s32 D_801B2698;

    D_801B269C = 0x10;
    D_801B2698 += 1;
    func_8007A3D0();
}

/** @brief Draw and fade the effect, then advance when its countdown expires. */
void func_8007A3D0(void)
{
extern u8 D_800DA448[];
extern u8 D_80139CC8[];
extern s32 D_801B24B4;
extern s32 D_801B2698;
extern s32 D_801B269C;

    s32 value;
    s32 remaining_ticks;

    if (D_801B24B4 != 0)
    {
        func_8006CFE4((s32)D_800DA448, (s32)D_80139CC8, 0x18, D_801B24B4, D_801B24B4, 0);
        value = D_801B24B4 - 8;
        D_801B24B4 = value;
        if (value < 0)
        {
            D_801B24B4 = 0;
        }
    }
    remaining_ticks = D_801B269C - 1;
    D_801B269C = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B2698 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007A460(void)
{
extern s32 D_801B2698;

    D_801B2698 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_8007A478(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B26A0;
extern s32 D_801B26A4;
extern void (*D_800D5310[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B240;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_80139888;
extern s32 D_80182DE4;

    s32 result;

    if (arg0 != 0)
    {
        D_801B26A0 = 1;
        D_801B26A4 = 1;
    }

    if (D_801B26A0 < 0x6)
    {
        D_800D5310[D_801B26A0]();
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
void func_8007A4E8(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B26A0;
extern s32 D_801B26A4;
extern void (*D_800D5310[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B240;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_80139888;
extern s32 D_80182DE4;

    D_801B26A0 = 1;
    D_801B26A4 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_8007A500(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B26A0;
extern s32 D_801B26A4;
extern void (*D_800D5310[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B240;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_80139888;
extern s32 D_80182DE4;

    D_8013B240 = D_80139258;
    D_80139888 = g_wmap_camera_translation;
    D_80182DE4 = 0x1;
    D_80139888.w[2] = 0xA410;
    D_801B26A4 = 0x42;
    D_801B26A0 += 1;
    func_80078F8C();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007A5B0(void)
{
extern s32 D_801B26A0;
extern s32 D_801B26A4;

    D_801B26A4 = 0x40;
    D_801B26A0 += 1;
    func_80079088();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007A5E8(void)
{
extern s32 D_801B26A0;
extern s32 D_801B26A4;

    D_801B26A0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_8007A600(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B26A8;
extern s32 D_801B26AC;
extern void (*D_800D5328[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B2670;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_80139898;
extern s32 D_80182DE8;

    s32 result;

    if (arg0 != 0)
    {
        D_801B26A8 = 1;
        D_801B26AC = 1;
    }

    if (D_801B26A8 < 0x6)
    {
        D_800D5328[D_801B26A8]();
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
void func_8007A670(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B26A8;
extern s32 D_801B26AC;
extern void (*D_800D5328[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B2670;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_80139898;
extern s32 D_80182DE8;

    D_801B26A8 = 1;
    D_801B26AC = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_8007A688(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B26A8;
extern s32 D_801B26AC;
extern void (*D_800D5328[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B2670;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_80139898;
extern s32 D_80182DE8;

    D_801B2670 = D_80139258;
    D_80139898 = g_wmap_camera_translation;
    D_80182DE8 = 0x1;
    D_80139898.w[2] = 0xA410;
    D_801B26AC = 0x3E;
    D_801B26A8 += 1;
    func_80079188();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007A738(void)
{
extern s32 D_801B26A8;
extern s32 D_801B26AC;

    D_801B26AC = 0x40;
    D_801B26A8 += 1;
    func_80079284();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007A770(void)
{
extern s32 D_801B26A8;
extern s32 D_801B26AC;

    D_801B26A8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_8007A788(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B26B0;
extern s32 D_801B26B4;
extern void (*D_800D5340[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B2678;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2660;
extern s32 D_80182DEC;

    s32 result;

    if (arg0 != 0)
    {
        D_801B26B0 = 1;
        D_801B26B4 = 1;
    }

    if (D_801B26B0 < 0x6)
    {
        D_800D5340[D_801B26B0]();
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
void func_8007A7F8(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B26B0;
extern s32 D_801B26B4;
extern void (*D_800D5340[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B2678;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2660;
extern s32 D_80182DEC;

    D_801B26B0 = 1;
    D_801B26B4 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_8007A810(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B26B0;
extern s32 D_801B26B4;
extern void (*D_800D5340[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B2678;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2660;
extern s32 D_80182DEC;

    D_801B2678 = D_80139258;
    D_801B2660 = g_wmap_camera_translation;
    D_80182DEC = 0x1;
    D_801B2660.w[2] = 0xA410;
    D_801B26B4 = 0x3A;
    D_801B26B0 += 1;
    func_80079384();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007A8C0(void)
{
extern s32 D_801B26B0;
extern s32 D_801B26B4;

    D_801B26B4 = 0x40;
    D_801B26B0 += 1;
    func_80079480();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007A8F8(void)
{
extern s32 D_801B26B0;
extern s32 D_801B26B4;

    D_801B26B0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_8007A910(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B26B8;
extern s32 D_801B26BC;
extern void (*D_800D5358[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B238;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_80139870;
extern s32 D_80182DF0;

    s32 result;

    if (arg0 != 0)
    {
        D_801B26B8 = 1;
        D_801B26BC = 1;
    }

    if (D_801B26B8 < 0x4)
    {
        D_800D5358[D_801B26B8]();
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
void func_8007A980(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B26B8;
extern s32 D_801B26BC;
extern void (*D_800D5358[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B238;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_80139870;
extern s32 D_80182DF0;

    D_801B26B8 = 1;
    D_801B26BC = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_8007A998(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B26B8;
extern s32 D_801B26BC;
extern void (*D_800D5358[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B238;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_80139870;
extern s32 D_80182DF0;

    D_8013B238 = D_80139258;
    D_80139870 = g_wmap_camera_translation;
    D_80182DF0 = 0x80;
    D_80139870.w[2] = 0xAFC8;
    D_801B26BC = 0x28;
    D_801B26B8 += 1;
    func_80079580();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007AA48(void)
{
extern s32 D_801B26B8;

    D_801B26B8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_8007AA60(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B26C0;
extern s32 D_801B26C4;
extern void (*D_800D5368[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DF4;

    s32 result;

    if (arg0 != 0)
    {
        D_801B26C0 = 1;
        D_801B26C4 = 1;
    }

    if (D_801B26C0 < 0x4)
    {
        D_800D5368[D_801B26C0]();
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
void func_8007AAD0(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B26C0;
extern s32 D_801B26C4;
extern void (*D_800D5368[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DF4;

    D_801B26C0 = 1;
    D_801B26C4 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_8007AAE8(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B26C0;
extern s32 D_801B26C4;
extern void (*D_800D5368[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DF4;

    D_801B24A8 = D_80139258;
    D_801B2478 = g_wmap_camera_translation;
    D_80182DF4 = 0x80;
    D_801B2478.w[2] = 0xAFC8;
    D_801B26C4 = 0x28;
    D_801B26C0 += 1;
    func_80079680();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007AB98(void)
{
extern s32 D_801B26C0;

    D_801B26C0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_8007ABB0(s32 arg0)
{
extern u32 D_801B26C8;
extern s32 D_801B26CC;
extern void (*D_800D5378[])(void);
extern u8 D_8011D538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 g_wmap_focus_screen_position;

    s32 result;

    if (arg0 != 0)
    {
        D_801B26C8 = 1;
        D_801B26CC = 1;
    }

    if (D_801B26C8 < 0x4)
    {
        D_800D5378[D_801B26C8]();
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
void func_8007AC20(void)
{
extern u32 D_801B26C8;
extern s32 D_801B26CC;
extern void (*D_800D5378[])(void);
extern u8 D_8011D538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 g_wmap_focus_screen_position;

    D_801B26C8 = 1;
    D_801B26CC = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_8007AC38(void)
{
extern u32 D_801B26C8;
extern s32 D_801B26CC;
extern void (*D_800D5378[])(void);
extern u8 D_8011D538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 g_wmap_focus_screen_position;

    D_801399BC = D_8011D538;
    D_800D9370[0x6] = 0xF;
    *(s16*)&D_800D9370[0xE] = 3;
    *(s16*)&D_800D9370[0x10] = -1;
    *(s16*)&D_800D9370[0x2] = 0;
    *(s16*)&D_800D9370[0x26] = 0;
    *(s16*)&D_800D9370[0x22] = 0x81;
    *(s16*)&D_800D9370[0x24] = 0x81;
    D_801B26CC = 0x4D;
    D_801B26C8 += 1;
    func_8007ACB4();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8007ACB4(void)
{
extern u32 D_801B26C8;
extern s32 D_801B26CC;
extern void (*D_800D5378[])(void);
extern void func_8007ACB4(void);
extern u8 D_8011D538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 g_wmap_focus_screen_position;

    wmap_step_actor_animation(D_800D9370, D_801399B8);
    wmap_draw_actor_sprite(D_800D9370, g_wmap_focus_screen_position, 0x13, 0xB, 0);
    if (--D_801B26CC == 0)
    {
        D_801B26C8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007AD30(void)
{
extern s32 D_801B26C8;

    D_801B26C8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_8007AD48(s32 arg0)
{
extern u32 D_801B26D0;
extern s32 D_801B26D4;
extern void (*D_800D5388[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B26D0 = 1;
        D_801B26D4 = 1;
    }

    if (D_801B26D0 < 0x4)
    {
        D_800D5388[D_801B26D0]();
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
void func_8007ADB8(void)
{
extern u32 D_801B26D0;
extern s32 D_801B26D4;
extern void (*D_800D5388[])(void);

    D_801B26D0 = 1;
    D_801B26D4 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its next step.
 */
void func_8007ADD0(void)
{
extern u8* D_801399AC;
extern u8 D_8011F538[];
extern u8 D_800D9318[];
extern s32 D_801B26D0;
extern s32 D_801B26D4;

    D_801399AC = D_8011F538;
    D_800D9318[0x6] = 0xF;
    *(s16*)&D_800D9318[0x10] = -1;
    *(s16*)&D_800D9318[0x2] = 0;
    *(s16*)&D_800D9318[0xE] = 0;
    *(s16*)&D_800D9318[0x26] = 0;
    *(s16*)&D_800D9318[0x22] = 0x80;
    *(s16*)&D_800D9318[0x24] = 0x80;
    D_801B26D4 = 0xC8;
    D_801B26D0 += 1;
    func_8007AE48();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8007AE48(void)
{
extern s32 D_801B26D0;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 g_wmap_focus_screen_position;
extern s32 D_801B26D4;

    wmap_step_actor_animation(D_800D9318, D_801399A8);
    wmap_draw_actor_sprite(D_800D9318, g_wmap_focus_screen_position, 0x14, 0xB, 0);
    if (--D_801B26D4 == 0)
    {
        D_801B26D0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007AEC4(void)
{
extern s32 D_801B26D0;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 g_wmap_focus_screen_position;
extern s32 D_801B26D4;

    D_801B26D0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8007AEDC(s32 arg0)
{
extern u32 D_801B26D8;
extern s32 D_801B26DC;
extern void (*D_800D5398[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B26D8 = 1;
        D_801B26DC = 1;
        return 1;
    }

    if (D_801B26D8 < 0x6)
    {
        D_800D5398[D_801B26D8]();
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
void func_8007AF54(void)
{
extern u32 D_801B26D8;
extern s32 D_801B26DC;
extern void (*D_800D5398[])(void);

    D_801B26D8 = 1;
    D_801B26DC = 1;
}

/** @brief Draw the sequence effect and advance when its countdown expires. */
void func_8007AF6C(void)
{
extern u8 D_800DB578[];
extern u8 D_80139FE8[];
extern s32 D_801B25D8;
extern s32 D_801B26D8;
extern s32 D_801B26DC;

    s32 value;

    func_8006D014((s32)D_800DB578, (s32)D_80139FE8, 0x28, 0, D_801B25D8, 8, 1);
    value = D_801B26DC - 1;
    D_801B26DC = value;
    if (value == 0)
    {
        D_801B26D8 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007AFE8(void)
{
extern s32 D_801B26DC;
extern s32 D_801B26D8;

    D_801B26DC = 0x10;
    D_801B26D8 += 1;
    func_8007B020();
}

/** @brief Draw and fade the effect, then advance when its countdown expires. */
void func_8007B020(void)
{
extern u8 D_800DB578[];
extern u8 D_80139FE8[];
extern s32 D_801B25D8;
extern s32 D_801B26D8;
extern s32 D_801B26DC;

    s32 value;
    s32 remaining_ticks;

    if (D_801B25D8 != 0)
    {
        func_8006D014((s32)D_800DB578, (s32)D_80139FE8, 0x28, 1, D_801B25D8, 8, 1);
        value = D_801B25D8 - 8;
        D_801B25D8 = value;
        if (value < 0)
        {
            D_801B25D8 = 0;
        }
    }
    remaining_ticks = D_801B26DC - 1;
    D_801B26DC = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B26D8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007B0C0(void)
{
extern s32 D_801B26D8;

    D_801B26D8 += 1;
}
