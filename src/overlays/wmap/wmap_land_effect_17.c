#include "wmap_main.h"
#include "wmap_land_effect_17.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_resource_support.h"
#include "wmap_view_effects.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"

/** @brief World-map tile and its cached neighboring layout data. */
typedef struct
{
    s32 tile;
    s16 field_04;
    s16 field_06;
    u8 neighbors[32];
} WmapTile;

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_8008A864(void)
{
extern s32 D_801B29A0;
extern s32 D_801B2650[];
extern SVECTOR D_801B24A0;
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B29CC;
extern s32 D_801B29C8;

    MATRIX m;
    s32 x;

    x = D_801B2650[2] - 0xDAC;
    D_801B2650[2] = x;
    if (x < 0x2710)
    {
        D_801B2650[2] = 0x2710;
    }

    PushMatrix();
    RotMatrix(&D_801B24A0, &m);
    TransMatrix(&m, &D_8011CF60);
    SetRotMatrix(&m);
    SetTransMatrix(&m);

    if (D_80182DE8 != 0)
    {
        func_8006CD98((s32)D_800DCF18, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DE8);
        D_80182DE8 -= 0x4;
        if (D_80182DE8 < 0)
        {
            D_80182DE8 = 0;
        }
    }

    PopMatrix();
    if (--D_801B29CC == 0)
    {
        D_801B29C8 += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_8008A964(void)
{
extern s32 D_801B2478[];
extern SVECTOR D_801B24A8;
extern VECTOR D_8011CF60;
extern s32 D_80182DEC;
extern s32 D_8011CF1C;
extern s32 D_801B29D4;
extern s32 D_801B29D0;

    MATRIX m;
    s32 x;

    x = D_801B2478[2] - 0xDAC;
    D_801B2478[2] = x;
    if (x < 0x2710)
    {
        D_801B2478[2] = 0x2710;
    }

    PushMatrix();
    RotMatrix(&D_801B24A8, &m);
    TransMatrix(&m, &D_8011CF60);
    SetRotMatrix(&m);
    SetTransMatrix(&m);

    if (D_80182DEC != 0)
    {
        func_8006CD98(D_8011CF1C, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DEC);
        D_80182DEC -= 1;
        if (D_80182DEC < 0)
        {
            D_80182DEC = 0;
        }
    }

    PopMatrix();
    if (--D_801B29D4 == 0)
    {
        D_801B29D0 += 1;
    }
}

/** @brief Initialize the effect actors, resources, and evenly spaced angles. */
void func_8008AA64(void)
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

extern WmapConfigA D_800D95D8[];
extern WmapMotion D_801AFD60[];
extern WmapResource D_80139988[];
extern s32 *D_80139280;
extern s32 D_801B0FD0;
extern u8 D_8011D538[];
extern s32 D_801B29D8;
extern s32 D_801B29DC;
extern void func_8008C008__for_func_8008AA64(void) __asm__("func_8008C008");

    s32 i;

    i = 0;
    D_801B0FD0 = 3;
    D_80139280[0x15] = 20;
    D_80139280[0x16] = 20;
    D_80139280[0x17] = 128;
    D_80139280[0x18] = 0;
    D_80139280[0x19] = -1;
    D_80139280[0x1A] = 1000;
    D_80139280[0x1B] = 20;
    D_80139280[0x1C] = 15;
    D_80139280[0x1D] = 1;
    D_80139280[0x1E] = 12000;
    do
    {
        D_801AFD60[i].state = 1;
        D_801AFD60[i].angle = i * 1365;
        D_801AFD60[i].scale = 128;
        D_801AFD60[i].z = 12000;
        D_801AFD60[i].x = 0;
        D_801AFD60[i].field_0E = 0;
        D_80139988[i + 20].resource = D_8011D538;
        D_800D95D8[i].field_06 = 15;
        D_800D95D8[i].field_10 = -1;
        D_800D95D8[i].field_26 = 2;
        D_800D95D8[i].field_02 = 0;
        D_800D95D8[i].field_0E = 1;
        D_800D95D8[i].field_22 = 0;
        D_800D95D8[i].field_24 = 127;
        i++;
    } while (i < 3);
    D_801B29DC = 32;
    D_801B29D8++;
    func_8008C008__for_func_8008AA64();
}

/** @brief Initialize the effect actors, resources, and evenly spaced angles. */
void func_8008AB94(void)
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

extern WmapConfigA D_800D9B00[];
extern WmapMotion D_801AFFB8[];
extern WmapResource D_80139988[];
extern s32 *D_80139280;
extern s32 D_801B0FD0;
extern u8 D_8011D538[];
extern s32 D_801B29E0;
extern s32 D_801B29E4;
extern void func_8008C134__for_func_8008AB94(void) __asm__("func_8008C134");

    s32 i;

    i = 0;
    D_801B0FD0 = 3;
    D_80139280[0xB] = 4;
    D_80139280[0xC] = 4;
    D_80139280[0xD] = 128;
    D_80139280[0xE] = 0;
    D_80139280[0xF] = -1;
    D_80139280[0x10] = 100;
    D_80139280[0x11] = 50;
    D_80139280[0x12] = 15;
    D_80139280[0x13] = 2;
    D_80139280[0x14] = 12000;
    do
    {
        D_801AFFB8[i].state = 1;
        D_801AFFB8[i].angle = i * 1365;
        D_801AFFB8[i].scale = 128;
        D_801AFFB8[i].z = 12000;
        D_801AFFB8[i].x = 0;
        D_801AFFB8[i].field_0E = 0;
        D_80139988[i + 50].resource = D_8011D538;
        D_800D9B00[i].field_06 = 15;
        D_800D9B00[i].field_10 = -1;
        D_800D9B00[i].field_02 = 0;
        D_800D9B00[i].field_0E = 2;
        D_800D9B00[i].field_26 = 2;
        D_800D9B00[i].field_22 = 0;
        D_800D9B00[i].field_24 = 127;
        i++;
    } while (i < 3);
    D_801B29E4 = 64;
    D_801B29E0++;
    func_8008C134__for_func_8008AB94();
}

/** @brief Initialize the actor group and its animation resources, then advance. */
void func_8008ACC8(void)
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
    s16 field_00;
    s16 angle;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    s16 field_10;
    s16 pad_12;
} WmapMotion;

/** @brief Animation resource slot. */
typedef struct
{
    s32 field_00;
    void *data;
} WmapResource;

extern WmapConfigA D_800D9268[];
extern WmapMotion D_801AFBD0[];
extern WmapResource D_80139988[];
extern u8 D_80121538[];
extern s32 D_801B0FD0;
extern s32 D_801B29E8;
extern s32 D_801B29EC;
extern void func_8008C260__for_func_8008ACC8(void) __asm__("func_8008C260");

    s32 i;
    s16 angle;
    WmapConfigA *actor;

    D_801B0FD0 = 8;
    angle = (s16)0x27FD8;
    for (i = 80; i < 88; i++)
    {
        actor = &D_800D9268[i];
        D_80139988[i].data = D_80121538;
        actor->field_02 = 0;
        actor->field_06 = 15;
        actor->field_0E = 2;
        actor->field_10 = -1;
        actor->field_22 = 129;
        actor->field_24 = 1;
        actor->field_26 = 8;
        D_801AFBD0[i].field_00 = 1;
        D_801AFBD0[i].angle = i << 9;
        D_801AFBD0[i].field_04 = -4000;
        D_801AFBD0[i].field_08 = 480000;
        D_801AFBD0[i].field_0C = 9999;
        D_801AFBD0[i].field_0E = 0;
        D_801AFBD0[i].field_10 = 144;
    }
    D_801B29EC = 42;
    D_801B29E8++;
    func_8008C260__for_func_8008ACC8();
}

/** @brief Initialize the effect descriptor and actors with randomized angles. */
void func_8008ADC0(void)
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

extern WmapConfigA D_800D9B00[];
extern WmapMotion D_801AFFB8[];
extern WmapResource D_80139988[];
extern s32 *D_80139280;
extern u8 D_80121538[];
extern s32 D_801B0FD0;
extern s32 D_801B29F0;
extern s32 D_801B29F4;
extern s32 rand(void);
extern void func_8008C42C__for_func_8008ADC0(void) __asm__("func_8008C42C");

    s32 i;
    s32 angle;
    WmapConfigA *actor;
    s32 actor_offset;
    s32 resource_offset;
    WmapResource *resource;
    WmapMotion *motion;

    i = 0;
    D_801B0FD0 = 20;
    D_80139280[0xB] = 64;
    D_80139280[0xC] = 20;
    D_80139280[0xD] = 128;
    D_80139280[0xE] = 0;
    D_80139280[0xF] = -1;
    D_80139280[0x10] = 2000;
    D_80139280[0x11] = 50;
    D_80139280[0x12] = 8;
    D_80139280[0x13] = 1;
    D_80139280[0x14] = 10;
    do
    {
        WmapResource *resources;
        u8 *resource_data;

        motion = &D_801AFFB8[i];
        motion->state = 1;
        angle = rand();
        resource_offset = (i + 50) * 8;
        actor_offset = i * 44;
        resources = D_80139988;
        resource_data = D_80121538;
        actor = (WmapConfigA *)((u8 *)D_800D9B00 + actor_offset);
        resource = (WmapResource *)((u8 *)resources + resource_offset);
        motion->angle = angle & 4095;
        motion->scale = 128;
        motion->z = 10;
        motion->x = 0;
        motion->field_0E = 0;
        resource->resource = resource_data;
        actor->field_06 = 15;
        actor->field_10 = -1;
        actor->field_26 = 2;
        actor->field_24 = 127;
        actor->field_02 = 0;
        actor->field_0E = 1;
        actor->field_22 = 0;
        i++;
    } while (i < 20);
    D_801B29F4 = 64;
    D_801B29F0++;
    func_8008C42C__for_func_8008ADC0();
}

/** @brief Initialize the effect descriptor and actors with randomized angles. */
void func_8008AF30(void)
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

extern WmapConfigA D_800D95D8[];
extern WmapMotion D_801AFD60[];
extern WmapResource D_80139988[];
extern s32 *D_80139280;
extern u8 D_80121538[];
extern s32 D_801B0FD0;
extern s32 D_801B29F8;
extern s32 D_801B29FC;
extern s32 rand(void);
extern void func_8008C558__for_func_8008AF30(void) __asm__("func_8008C558");

    s32 i;
    s32 angle;
    WmapConfigA *actor;
    s32 actor_offset;
    s32 resource_offset;
    WmapResource *resource;
    WmapMotion *motion;

    i = 0;
    D_801B0FD0 = 30;
    D_80139280[0x15] = 192;
    D_80139280[0x16] = 16;
    D_80139280[0x17] = 128;
    D_80139280[0x18] = 0;
    D_80139280[0x19] = -1;
    D_80139280[0x1A] = 4000;
    D_80139280[0x1B] = 20;
    D_80139280[0x1C] = 8;
    D_80139280[0x1D] = 2;
    D_80139280[0x1E] = 10;
    do
    {
        WmapResource *resources;
        u8 *resource_data;

        motion = &D_801AFD60[i];
        motion->state = 1;
        angle = rand();
        resource_offset = (i + 20) * 8;
        actor_offset = i * 44;
        resources = D_80139988;
        resource_data = D_80121538;
        actor = (WmapConfigA *)((u8 *)D_800D95D8 + actor_offset);
        resource = (WmapResource *)((u8 *)resources + resource_offset);
        motion->angle = angle & 4095;
        motion->scale = 128;
        motion->z = 10;
        motion->x = 0;
        motion->field_0E = 0;
        resource->resource = resource_data;
        actor->field_06 = 15;
        actor->field_0E = 2;
        actor->field_10 = -1;
        actor->field_26 = 4;
        actor->field_24 = 127;
        actor->field_02 = 0;
        actor->field_22 = 0;
        i++;
    } while (i < 30);
    D_801B29FC = 64;
    D_801B29F8++;
    func_8008C558__for_func_8008AF30();
}

/** @brief Initialize particle positions, velocities and animation resources. */
void func_8008B0A0(void)
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

/** @brief Position and velocity halfwords for an effect particle. */
typedef struct
{
    s16 x, y, z, pad_06;
    s16 vx, vy, vz, pad_0E;
} WmapParticle;
extern WmapConfigA D_800D9268[];
extern WmapParticle D_800E4F18[];
extern WmapResource D_80139988[];
extern u8 D_80121538[];
extern s32 D_801B2A00;
extern s32 D_801B2A04;
extern s32 rand(void);
extern void func_8008C684__for_func_8008B0A0(void) __asm__("func_8008C684");

    s32 i;
    WmapParticle *particles = D_800E4F18;

    for (i = 100; i < 124; i++)
    {
        particles[i].x = ((rand() * 200) >> 15) - 100;
        particles[i].y = ((rand() * 200) >> 15) - 100;
        particles[i].z = ((rand() * 150) >> 15) - 150;
        particles[i].vx = ((rand() << 6) >> 15) - 20;
        particles[i].vy = ((rand() << 6) >> 15) - 20;
        particles[i].vz = ((rand() << 6) >> 15) - 20;
        D_800D9268[i].field_02 = 0;
        D_800D9268[i].field_06 = 15;
        D_800D9268[i].field_0E = (rand() * 3) >> 15;
        D_800D9268[i].field_10 = -1;
        D_800D9268[i].field_26 = 4;
        D_800D9268[i].field_22 = 129;
        D_800D9268[i].field_24 = 1;
        D_80139988[i].resource = D_80121538;
    }
    particles[i].x = -100;
    particles[i].y = -100;
    particles[i].z = -150;
    particles[i].vx = 300;
    particles[i].vy = 300;
    particles[i].vz = 300;
    D_801B2A04 = 112;
    D_801B2A00++;
    func_8008C684__for_func_8008B0A0();
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008B25C(s32 arg0)
{
extern u32 D_801B29A8;
extern s32 D_801B29AC;
extern void (*D_800D5C78[])(void);
extern void func_8008B330__for_func_8008B25C(void) __asm__("func_8008B330");
extern s32 D_8013B20C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B29A8 = 1;
        D_801B29AC = 1;
        return 1;
    }

    if (D_801B29A8 < 0x6)
    {
        D_800D5C78[D_801B29A8]();
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
void func_8008B2D4(void)
{
extern u32 D_801B29A8;
extern s32 D_801B29AC;
extern void (*D_800D5C78[])(void);
extern void func_8008B330__for_func_8008B2D4(void) __asm__("func_8008B330");
extern s32 D_8013B20C;

    D_801B29A8 = 1;
    D_801B29AC = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_8008B2EC(void)
{
extern u32 D_801B29A8;
extern s32 D_801B29AC;
extern void (*D_800D5C78[])(void);
extern void func_8008B330__for_func_8008B2EC(void) __asm__("func_8008B330");
extern s32 D_8013B20C;

    func_8006CAC0(func_8006C81C);
    D_8013B20C = 1;
    D_801B29A8 += 1;
    func_8008B330__for_func_8008B2EC();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8008B330(void)
{
extern s32 D_8013B20C;
extern s32 D_801B29A8;
extern void func_8008B36C__for_func_8008B330(void) __asm__("func_8008B36C");
extern void func_8008B404__for_func_8008B330(void) __asm__("func_8008B404");
extern void func_8008B3B0__for_func_8008B330(void) __asm__("func_8008B3B0");

    if (D_8013B20C == 0)
    {
        D_801B29A8 += 1;
        func_8008B36C__for_func_8008B330();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_8008B36C(void)
{
extern s32 D_8013B20C;
extern s32 D_801B29A8;
extern void func_8008B36C(void);
extern void func_8008B404__for_func_8008B36C(void) __asm__("func_8008B404");
extern void func_8008B3B0__for_func_8008B36C(void) __asm__("func_8008B3B0");

    func_8006CAC0(func_8008B404__for_func_8008B36C);
    D_8013B20C = 1;
    D_801B29A8 += 1;
    func_8008B3B0__for_func_8008B36C();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8008B3B0(void)
{
extern s32 D_801B29A8;
extern s32 D_8013B20C;
extern void func_8008B3EC__for_func_8008B3B0(void) __asm__("func_8008B3EC");

    if (D_8013B20C == 0)
    {
        D_801B29A8 += 1;
        func_8008B3EC__for_func_8008B3B0();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008B3EC(void)
{
extern s32 D_801B29A8;
extern s32 D_8013B20C;
extern void func_8008B3EC(void);

    D_801B29A8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008B404(s32 arg0)
{
extern u32 D_801B29B0;
extern s32 D_801B29B4;
extern void (*D_800D5C90[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B29B0 = 1;
        D_801B29B4 = 1;
        return 1;
    }

    if (D_801B29B0 < 0x16)
    {
        D_800D5C90[D_801B29B0]();
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
void func_8008B47C(void)
{
extern u32 D_801B29B0;
extern s32 D_801B29B4;
extern void (*D_800D5C90[])(void);

    D_801B29B0 = 1;
    D_801B29B4 = 1;
}

/** @brief Set two flags, play sound 35, and register a callback before a four-tick delay. */
void func_8008B494(void)
{
extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_801B29B0;
extern s32 D_801B29B4;
extern void func_8008BCC8__for_func_8008B494(void) __asm__("func_8008BCC8");

    D_8013B20C = 1;
    D_8013B208 = 1;
    wmap_play_sound(0x23, 0x80);
    func_8006CAC0(&func_8008BCC8__for_func_8008B494);
    D_801B29B4 = 4;
    D_801B29B0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008B4F0(void)
{
extern s32 D_801B29B4;
extern s32 D_801B29B0;

    if (--D_801B29B4 == 0)
    {
        D_801B29B0 += 1;
    }
}

/** @brief Register two callbacks, set world-map color and state, and begin an eight-tick delay. */
void func_8008B524(void)
{
extern s32 D_801B29B0;
extern s32 D_801B29B4;
extern void func_8008C39C__for_func_8008B524(void) __asm__("func_8008C39C");
extern void func_8008C4C8__for_func_8008B524(void) __asm__("func_8008C4C8");

    func_8006CAC0(&func_8008C4C8__for_func_8008B524);
    func_8006CAC0(&func_8008C39C__for_func_8008B524);
    wmap_start_map_tint(0x103056);
    g_wmap_backdrop_target_level = 4;
    D_801B29B4 = 8;
    D_801B29B0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008B584(void)
{
extern s32 D_801B29B4;
extern s32 D_801B29B0;
extern void func_8008B98C__for_func_8008B584(void) __asm__("func_8008B98C");

    if (--D_801B29B4 == 0)
    {
        D_801B29B0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8008B5B8(void)
{
extern s32 D_801B29B4;
extern s32 D_801B29B0;
extern void func_8008B98C__for_func_8008B5B8(void) __asm__("func_8008B98C");

    func_8006CAC0(func_8008B98C__for_func_8008B5B8);
    D_801B29B4 = 0x2;
    D_801B29B0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008B5F4(void)
{
extern s32 D_801B29B4;
extern s32 D_801B29B0;

    if (--D_801B29B4 == 0)
    {
        D_801B29B0 += 1;
    }
}

/**
 * @brief World-map step handler: set flags and advance the step counter.
 */
void func_8008B628(void)
{
extern s32 D_801ADAE0;
extern s32 D_801B29B0;
extern s32 D_801B29B4;

    D_801ADAE0 = 1;
    D_801B29B4 = 0xC;
    D_801B29B0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008B654(void)
{
extern s32 D_801B29B4;
extern s32 D_801B29B0;
extern void func_8008C1D0__for_func_8008B654(void) __asm__("func_8008C1D0");

    if (--D_801B29B4 == 0)
    {
        D_801B29B0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8008B688(void)
{
extern s32 D_801B29B4;
extern s32 D_801B29B0;
extern void func_8008C1D0__for_func_8008B688(void) __asm__("func_8008C1D0");

    func_8006CAC0(func_8008C1D0__for_func_8008B688);
    D_801B29B4 = 0x2;
    D_801B29B0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008B6C4(void)
{
extern s32 D_801B29B4;
extern s32 D_801B29B0;
extern void func_8008C5F4__for_func_8008B6C4(void) __asm__("func_8008C5F4");

    if (--D_801B29B4 == 0)
    {
        D_801B29B0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8008B6F8(void)
{
extern s32 D_801B29B4;
extern s32 D_801B29B0;
extern void func_8008C5F4__for_func_8008B6F8(void) __asm__("func_8008C5F4");

    func_8006CAC0(func_8008C5F4__for_func_8008B6F8);
    D_801B29B4 = 0x32;
    D_801B29B0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008B734(void)
{
extern s32 D_801B29B4;
extern s32 D_801B29B0;
extern void func_8008BE20__for_func_8008B734(void) __asm__("func_8008BE20");

    if (--D_801B29B4 == 0)
    {
        D_801B29B0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8008B768(void)
{
extern s32 D_801B29B4;
extern s32 D_801B29B0;
extern void func_8008BE20__for_func_8008B768(void) __asm__("func_8008BE20");

    func_8006CAC0(func_8008BE20__for_func_8008B768);
    D_801B29B4 = 0x2;
    D_801B29B0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008B7A4(void)
{
extern s32 D_801B29B4;
extern s32 D_801B29B0;
extern void func_8008BB28__for_func_8008B7A4(void) __asm__("func_8008BB28");

    if (--D_801B29B4 == 0)
    {
        D_801B29B0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8008B7D8(void)
{
extern s32 D_801B29B4;
extern s32 D_801B29B0;
extern void func_8008BB28__for_func_8008B7D8(void) __asm__("func_8008BB28");

    func_8006CAC0(func_8008BB28__for_func_8008B7D8);
    D_801B29B4 = 0x7A;
    D_801B29B0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008B814(void)
{
extern s32 D_801B29B4;
extern s32 D_801B29B0;
extern void func_8008BF78__for_func_8008B814(void) __asm__("func_8008BF78");

    if (--D_801B29B4 == 0)
    {
        D_801B29B0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8008B848(void)
{
extern s32 D_801B29B4;
extern s32 D_801B29B0;
extern void func_8008BF78__for_func_8008B848(void) __asm__("func_8008BF78");

    func_8006CAC0(func_8008BF78__for_func_8008B848);
    D_801B29B4 = 0x10;
    D_801B29B0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008B884(void)
{
extern s32 D_801B29B4;
extern s32 D_801B29B0;
extern void func_8008C0A4__for_func_8008B884(void) __asm__("func_8008C0A4");

    if (--D_801B29B4 == 0)
    {
        D_801B29B0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8008B8B8(void)
{
extern s32 D_801B29B4;
extern s32 D_801B29B0;
extern void func_8008C0A4__for_func_8008B8B8(void) __asm__("func_8008C0A4");

    func_8006CAC0(func_8008C0A4__for_func_8008B8B8);
    D_801B29B4 = 0x68;
    D_801B29B0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008B8F4(void)
{
extern s32 D_801B29B4;
extern s32 D_801B29B0;

    if (--D_801B29B4 == 0)
    {
        D_801B29B0 += 1;
    }
}

/** @brief Mark the current world-map tile and advance the sequence step. */
void func_8008B928(void)
{
extern s32 D_8013B20C;
extern WmapTile D_80139290[6][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 D_801B29B0;

    D_8013B20C = 0;
    D_80139290[D_8011D510][D_8011D530].tile = D_8011D4FC | 0x100;
    D_801B29B0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008B98C(s32 arg0)
{
extern u32 D_801B29B8;
extern s32 D_801B29BC;
extern void (*D_800D5CE8[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B29B8 = 1;
        D_801B29BC = 1;
        return 1;
    }

    if (D_801B29B8 < 0x4)
    {
        D_800D5CE8[D_801B29B8]();
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
void func_8008BA04(void)
{
extern u32 D_801B29B8;
extern s32 D_801B29BC;
extern void (*D_800D5CE8[])(void);

    D_801B29B8 = 1;
    D_801B29BC = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its next step.
 */
void func_8008BA1C(void)
{
extern u8* D_801399AC;
extern u8 D_8011D538[];
extern u8 D_800D9318[];
extern s32 D_801B29B8;
extern s32 D_801B29BC;
extern void func_8008BA94__for_func_8008BA1C(void) __asm__("func_8008BA94");

    D_801399AC = D_8011D538;
    D_800D9318[0x6] = 0xF;
    *(s16*)&D_800D9318[0x10] = -1;
    *(s16*)&D_800D9318[0x2] = 0;
    *(s16*)&D_800D9318[0xE] = 0;
    *(s16*)&D_800D9318[0x26] = 0;
    *(s16*)&D_800D9318[0x22] = 0x80;
    *(s16*)&D_800D9318[0x24] = 0x80;
    D_801B29BC = 0x48;
    D_801B29B8 += 1;
    func_8008BA94__for_func_8008BA1C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8008BA94(void)
{
extern s32 D_801B29B8;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;
extern s32 D_801B29BC;

    wmap_step_actor_animation(D_800D9318, D_801399A8);
    wmap_draw_actor_sprite(D_800D9318, D_8011CF4C, 0x1B, 0x1E, 0);
    if (--D_801B29BC == 0)
    {
        D_801B29B8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008BB10(void)
{
extern s32 D_801B29B8;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;
extern s32 D_801B29BC;

    D_801B29B8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008BB28(s32 arg0)
{
extern u32 D_801B29C0;
extern s32 D_801B29C4;
extern void (*D_800D5CF8[])(void);
extern void func_8008BC34__for_func_8008BB28(void) __asm__("func_8008BC34");
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B29C0 = 1;
        D_801B29C4 = 1;
        return 1;
    }

    if (D_801B29C0 < 0x4)
    {
        D_800D5CF8[D_801B29C0]();
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
void func_8008BBA0(void)
{
extern u32 D_801B29C0;
extern s32 D_801B29C4;
extern void (*D_800D5CF8[])(void);
extern void func_8008BC34__for_func_8008BBA0(void) __asm__("func_8008BC34");
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    D_801B29C0 = 1;
    D_801B29C4 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_8008BBB8(void)
{
extern u32 D_801B29C0;
extern s32 D_801B29C4;
extern void (*D_800D5CF8[])(void);
extern void func_8008BC34__for_func_8008BBB8(void) __asm__("func_8008BC34");
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    D_801399B4 = D_8011F538;
    D_800D9344[0x6] = 0xF;
    *(s16*)&D_800D9344[0x10] = -1;
    *(s16*)&D_800D9344[0x26] = 8;
    *(s16*)&D_800D9344[0x2] = 0;
    *(s16*)&D_800D9344[0xE] = 0;
    *(s16*)&D_800D9344[0x22] = 0x80;
    *(s16*)&D_800D9344[0x24] = 0;
    D_801B29C4 = 0xF5;
    D_801B29C0 += 1;
    func_8008BC34__for_func_8008BBB8();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8008BC34(void)
{
extern u32 D_801B29C0;
extern s32 D_801B29C4;
extern void (*D_800D5CF8[])(void);
extern void func_8008BC34(void);
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    wmap_step_actor_animation(D_800D9344, D_801399B0);
    wmap_draw_actor_sprite(D_800D9344, D_8011CF4C, 0x1A, 0x1E, 0);
    if (--D_801B29C4 == 0)
    {
        D_801B29C0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008BCB0(void)
{
extern s32 D_801B29C0;

    D_801B29C0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008BCC8(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B29C8;
extern s32 D_801B29CC;
extern void (*D_800D5D08[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_8008A864__for_func_8008BCC8(void) __asm__("func_8008A864");

    s32 result;

    if (arg0 != 0)
    {
        D_801B29C8 = 1;
        D_801B29CC = 1;
        return 1;
    }

    if (D_801B29C8 < 0x4)
    {
        D_800D5D08[D_801B29C8]();
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
void func_8008BD40(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B29C8;
extern s32 D_801B29CC;
extern void (*D_800D5D08[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_8008A864__for_func_8008BD40(void) __asm__("func_8008A864");

    D_801B29C8 = 1;
    D_801B29CC = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_8008BD58(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B29C8;
extern s32 D_801B29CC;
extern void (*D_800D5D08[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_8008A864__for_func_8008BD58(void) __asm__("func_8008A864");

    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.w[2] = 0xAFC8;
    D_801B29CC = 0x40;
    D_801B29C8 += 1;
    func_8008A864__for_func_8008BD58();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008BE08(void)
{
extern s32 D_801B29C8;

    D_801B29C8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008BE20(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B29D0;
extern s32 D_801B29D4;
extern void (*D_800D5D18[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern void func_8008A964__for_func_8008BE20(void) __asm__("func_8008A964");

    s32 result;

    if (arg0 != 0)
    {
        D_801B29D0 = 1;
        D_801B29D4 = 1;
        return 1;
    }

    if (D_801B29D0 < 0x4)
    {
        D_800D5D18[D_801B29D0]();
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
void func_8008BE98(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B29D0;
extern s32 D_801B29D4;
extern void (*D_800D5D18[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern void func_8008A964__for_func_8008BE98(void) __asm__("func_8008A964");

    D_801B29D0 = 1;
    D_801B29D4 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_8008BEB0(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B29D0;
extern s32 D_801B29D4;
extern void (*D_800D5D18[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern void func_8008A964__for_func_8008BEB0(void) __asm__("func_8008A964");

    D_801B24A8 = D_80139258;
    D_801B2478 = g_wmap_camera_translation;
    D_80182DEC = 0x80;
    D_801B2478.w[2] = 0xAFC8;
    D_801B29D4 = 0x100;
    D_801B29D0 += 1;
    func_8008A964__for_func_8008BEB0();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008BF60(void)
{
extern s32 D_801B29D0;

    D_801B29D0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008BF78(s32 arg0)
{
extern u32 D_801B29D8;
extern s32 D_801B29DC;
extern void (*D_800D5D28[])(void);
extern u8 D_800D95D8[];
extern u8 D_80139A28[];
extern s32 D_80139280;

    s32 result;

    if (arg0 != 0)
    {
        D_801B29D8 = 1;
        D_801B29DC = 1;
        return 1;
    }

    if (D_801B29D8 < 0x4)
    {
        D_800D5D28[D_801B29D8]();
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
void func_8008BFF0(void)
{
extern u32 D_801B29D8;
extern s32 D_801B29DC;
extern void (*D_800D5D28[])(void);
extern u8 D_800D95D8[];
extern u8 D_80139A28[];
extern s32 D_80139280;

    D_801B29D8 = 1;
    D_801B29DC = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_8008C008(void)
{
extern u32 D_801B29D8;
extern s32 D_801B29DC;
extern void (*D_800D5D28[])(void);
extern u8 D_800D95D8[];
extern u8 D_80139A28[];
extern s32 D_80139280;

    func_8006A2FC(D_800D95D8, D_80139A28, 0x3, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x50));
    if (--D_801B29DC == 0)
    {
        D_801B29D8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008C08C(void)
{
extern s32 D_801B29D8;

    D_801B29D8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008C0A4(s32 arg0)
{
extern u32 D_801B29E0;
extern s32 D_801B29E4;
extern void (*D_800D5D38[])(void);
extern u8 D_800D9B00[];
extern u8 D_80139B18[];
extern s32 D_80139280;

    s32 result;

    if (arg0 != 0)
    {
        D_801B29E0 = 1;
        D_801B29E4 = 1;
        return 1;
    }

    if (D_801B29E0 < 0x4)
    {
        D_800D5D38[D_801B29E0]();
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
void func_8008C11C(void)
{
extern u32 D_801B29E0;
extern s32 D_801B29E4;
extern void (*D_800D5D38[])(void);
extern u8 D_800D9B00[];
extern u8 D_80139B18[];
extern s32 D_80139280;

    D_801B29E0 = 1;
    D_801B29E4 = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_8008C134(void)
{
extern u32 D_801B29E0;
extern s32 D_801B29E4;
extern void (*D_800D5D38[])(void);
extern u8 D_800D9B00[];
extern u8 D_80139B18[];
extern s32 D_80139280;

    func_8006A2FC(D_800D9B00, D_80139B18, 0x3, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B29E4 == 0)
    {
        D_801B29E0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008C1B8(void)
{
extern s32 D_801B29E0;

    D_801B29E0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008C1D0(s32 arg0)
{
extern u32 D_801B29E8;
extern s32 D_801B29EC;
extern void (*D_800D5D48[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B29E8 = 1;
        D_801B29EC = 1;
        return 1;
    }

    if (D_801B29E8 < 0x6)
    {
        D_800D5D48[D_801B29E8]();
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
void func_8008C248(void)
{
extern u32 D_801B29E8;
extern s32 D_801B29EC;
extern void (*D_800D5D48[])(void);

    D_801B29E8 = 1;
    D_801B29EC = 1;
}

/**
 * @brief Draw the world-map cursor via func_8006B6EC, then advance after the wait expires.
 */
void func_8008C260(void)
{
extern s32 D_801B29E8;
extern s32 D_801B29EC;

    func_8006B6EC(0x50, 0x58, 0x8, 0, 0x1C);
    if (--D_801B29EC == 0)
    {
        D_801B29E8 += 1;
    }
}

/**
 * @brief Reset a range of world-map actor configs, arm the timer, and advance the step.
 */
void func_8008C2C0(void)
{
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

extern WmapConfigA D_800D9268[];
extern s32 D_801B29E8;
extern s32 D_801B29EC;
extern void func_8008C324__for_func_8008C2C0(void) __asm__("func_8008C324");

    s32 i;

    for (i = 0x50; i < 0x58; i++)
    {
        D_800D9268[i].field_22 = 0;
        D_800D9268[i].field_26 = 8;
    }
    D_801B29EC = 0x10;
    D_801B29E8 += 1;
    func_8008C324__for_func_8008C2C0();
}

/**
 * @brief Draw the world-map cursor via func_8006B6EC, then advance after the wait expires.
 */
void func_8008C324(void)
{
extern s32 D_801B29E8;
extern s32 D_801B29EC;

    func_8006B6EC(0x50, 0x58, 0x8, 0, 0x1C);
    if (--D_801B29EC == 0)
    {
        D_801B29E8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008C384(void)
{
extern s32 D_801B29E8;

    D_801B29E8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008C39C(s32 arg0)
{
extern u32 D_801B29F0;
extern s32 D_801B29F4;
extern void (*D_800D5D60[])(void);
extern u8 D_800D9B00[];
extern u8 D_80139B18[];
extern s32 D_80139280;

    s32 result;

    if (arg0 != 0)
    {
        D_801B29F0 = 1;
        D_801B29F4 = 1;
        return 1;
    }

    if (D_801B29F0 < 0x4)
    {
        D_800D5D60[D_801B29F0]();
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
void func_8008C414(void)
{
extern u32 D_801B29F0;
extern s32 D_801B29F4;
extern void (*D_800D5D60[])(void);
extern u8 D_800D9B00[];
extern u8 D_80139B18[];
extern s32 D_80139280;

    D_801B29F0 = 1;
    D_801B29F4 = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_8008C42C(void)
{
extern u32 D_801B29F0;
extern s32 D_801B29F4;
extern void (*D_800D5D60[])(void);
extern u8 D_800D9B00[];
extern u8 D_80139B18[];
extern s32 D_80139280;

    func_8006A2FC(D_800D9B00, D_80139B18, 0x14, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B29F4 == 0)
    {
        D_801B29F0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008C4B0(void)
{
extern s32 D_801B29F0;

    D_801B29F0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008C4C8(s32 arg0)
{
extern u32 D_801B29F8;
extern s32 D_801B29FC;
extern void (*D_800D5D70[])(void);
extern u8 D_800D95D8[];
extern u8 D_80139A28[];
extern s32 D_80139280;

    s32 result;

    if (arg0 != 0)
    {
        D_801B29F8 = 1;
        D_801B29FC = 1;
        return 1;
    }

    if (D_801B29F8 < 0x4)
    {
        D_800D5D70[D_801B29F8]();
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
void func_8008C540(void)
{
extern u32 D_801B29F8;
extern s32 D_801B29FC;
extern void (*D_800D5D70[])(void);
extern u8 D_800D95D8[];
extern u8 D_80139A28[];
extern s32 D_80139280;

    D_801B29F8 = 1;
    D_801B29FC = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_8008C558(void)
{
extern u32 D_801B29F8;
extern s32 D_801B29FC;
extern void (*D_800D5D70[])(void);
extern u8 D_800D95D8[];
extern u8 D_80139A28[];
extern s32 D_80139280;

    func_8006A2FC(D_800D95D8, D_80139A28, 0x1E, 0, 0x7F, 0x4, 0, (s32)((u8*)D_80139280 + 0x50));
    if (--D_801B29FC == 0)
    {
        D_801B29F8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008C5DC(void)
{
extern s32 D_801B29F8;

    D_801B29F8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008C5F4(s32 arg0)
{
extern u32 D_801B2A00;
extern s32 D_801B2A04;
extern void (*D_800D5D80[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2A00 = 1;
        D_801B2A04 = 1;
        return 1;
    }

    if (D_801B2A00 < 0x6)
    {
        D_800D5D80[D_801B2A00]();
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
void func_8008C66C(void)
{
extern u32 D_801B2A00;
extern s32 D_801B2A04;
extern void (*D_800D5D80[])(void);

    D_801B2A00 = 1;
    D_801B2A04 = 1;
}

/** @brief Draw the sequence effect and advance when its countdown expires. */
void func_8008C684(void)
{
extern u8 D_800E4F18[];
extern s32 D_801B2A00;
extern s32 D_801B2A04;

    s32 remaining_ticks;

    func_8006B998(0x64, 0x7C, D_800E4F18, 8, 0xA);
    remaining_ticks = D_801B2A04 - 1;
    D_801B2A04 = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B2A00 += 1;
    }
}

/** @brief Set particle display parameters and begin their countdown. */
void func_8008C6E8(void)
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

extern WmapConfigA D_800D9268[];
extern s32 D_801B2A00;
extern s32 D_801B2A04;
extern void func_8008C750__for_func_8008C6E8(void) __asm__("func_8008C750");

    s32 i;

    for (i = 100; i < 124; i++)
    {
        D_800D9268[i].field_26 = 4;
        D_800D9268[i].field_22 = 1;
    }
    D_801B2A04 = 64;
    D_801B2A00++;
    func_8008C750__for_func_8008C6E8();
}

/** @brief Draw the particle range and advance when its countdown expires. */
void func_8008C750(void)
{
extern u8 D_800E4F18[];
extern s32 D_801B2A00;
extern s32 D_801B2A04;

    s32 remaining;

    func_8006B998(100, 124, D_800E4F18, 8, 10);
    remaining = D_801B2A04 - 1;
    D_801B2A04 = remaining;
    if (remaining == 0)
    {
        D_801B2A00++;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008C7B4(void)
{
extern s32 D_801B2A00;

    D_801B2A00 += 1;
}
