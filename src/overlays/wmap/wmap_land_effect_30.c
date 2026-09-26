#include "wmap_main.h"
#include "wmap_land_effect_30.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_effect_primitives.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_80080FC8(void)
{
extern s32 D_801B27E0;
extern void func_8007F2D0(void);
extern s32 D_801B27E4;
extern s32 D_801B2650[];
extern SVECTOR D_801B24A0;
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B280C;
extern s32 D_801B2808;

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
extern s32 D_801B2650[];
extern SVECTOR D_801B24A0;
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern s32 D_8011CF1C;
extern s32 D_801B2814;
extern s32 D_801B2810;

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
        func_8006CD98(D_8011CF1C, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DE8);
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
extern s32 D_8011CF74;
extern s32 D_80139980;
extern s32 D_801B0FD0;
extern s32 D_801B2840;
extern s32 D_801B2844;

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
extern s32 D_8011CF74;
extern s32 D_80139980;
extern s32 D_801B0FD0;
extern s32 D_801B2848;
extern s32 D_801B284C;

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

extern s32 D_80139240;
extern s32 D_80139250;
extern s32 D_80139264;
extern s32 D_80139268;
extern s32 D_8013926C;
extern s32 D_80139284;
extern s32 D_8013B264;
extern s32 D_8013B270;
extern s32 D_8013B278;
extern s32 D_801B2850;
extern s32 D_801B2854;
extern WmapConfigA D_800DA708[];
extern WmapMotion D_801AFBD0[];
extern WmapMotion D_801B0530[];
extern WmapResource D_80139988[];
extern u8 D_80123538[];
extern s32 rand(void);
extern void func_80081508__for_func_80081360(void) __asm__("func_80081508");

    s32 i;
    WmapConfigA *actor;

    D_80139264 = 120;
    D_80139268 = 19;
    D_8013926C = 0;
    for (i = 0; i < 60; i++)
    {
        D_801AFBD0[i + 120].state = 0;
        D_80139988[i + 120].resource = D_80123538;
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
    func_80081508__for_func_80081360();
}

/** @brief Advance spaced effect actors and copy their trailing samples. */
void func_80081508(void)
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
} __attribute__((aligned(4))) WmapConfigA;

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

extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800DA708;
extern WmapMotion D_801AFBD0[];
extern WmapResource D_80139988[];
extern WmapResource D_80139D48;
extern s32 D_8011CF74;
extern s32 D_80139284;
extern s32 D_801B2850;
extern s32 D_801B2854;

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
            D_800D9268[destination].field_22 = 0;
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
    s32 unk0;
    void *unk4;
} WmapPair;

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

extern WmapD94Entry D_800D9268[];
extern s32 D_80123538[];
extern WmapPair D_80139988[];
extern WmapAfcEntry D_801AFBD0[];
extern s32 D_800D9150;
extern s32 D_800DCEA8;
extern s32 D_801B25D8;
extern s32 D_801B2858;
extern s32 D_801B285C;

extern void func_80083A10__for_func_8008172C(void) __asm__("func_80083A10");

    s32 i;
    WmapD94Entry *entry;

    i = 200;
    D_801B25D8 = 1;
    D_800DCEA8 = 1;

    do
    {
        D_801AFBD0[i].unk0 = 0;
        D_80139988[i].unk4 = D_80123538;
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
    func_80083A10__for_func_8008172C();
}

/** @brief Initialize effect actors at regular angular intervals. */
void func_800817F0(void)
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

extern WmapConfigA D_800D94FC[];
extern WmapMotion D_801AFBD0[];
extern WmapMotion D_801AFCFC[];
extern WmapResource D_80139988[];
extern s32 *D_80139280;
extern u8 D_80125538[];
extern s32 D_8013B264;
extern s32 D_8013B270;
extern s32 D_8013B278;
extern s32 D_801B2860;
extern s32 D_801B2864;
extern void func_80081954__for_func_800817F0(void) __asm__("func_80081954");

    s32 i;
    s16 actor_field_value;

    D_80139280[0x25] = 15;
    D_80139280[0x26] = 19;
    D_80139280[0x27] = 0;
    for (i = 0; i < 60; i++)
    {
        D_801AFBD0[i + 15].state = 0;
        D_80139988[i + 15].resource = D_80125538;
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
    func_80081954__for_func_800817F0();
}

/** @brief Advance spaced effect actors and copy their trailing samples. */
void func_80081954(void)
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
} __attribute__((aligned(4))) WmapConfigA;

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

extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D94FC;
extern WmapMotion D_801AFBD0[];
extern WmapResource D_80139988[];
extern WmapResource D_80139A00;
extern s32 D_8011CF74;
extern s32 *D_80139280;
extern s32 D_801B2860;
extern s32 D_801B2864;

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
            D_800D9268[destination].field_22 = 0;
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
extern u32 D_801B27E8;
extern s32 D_801B27EC;
extern void (*D_800D56F0[])(void);
extern void func_80081C74__for_func_80081BA0(void) __asm__("func_80081C74");
extern s32 D_8013B20C;

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
void func_80081C18(void)
{
extern u32 D_801B27E8;
extern s32 D_801B27EC;
extern void (*D_800D56F0[])(void);
extern void func_80081C74__for_func_80081C18(void) __asm__("func_80081C74");
extern s32 D_8013B20C;

    D_801B27E8 = 1;
    D_801B27EC = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_80081C30(void)
{
extern u32 D_801B27E8;
extern s32 D_801B27EC;
extern void (*D_800D56F0[])(void);
extern void func_80081C74__for_func_80081C30(void) __asm__("func_80081C74");
extern s32 D_8013B20C;

    func_8006CAC0(func_8006C81C);
    D_8013B20C = 1;
    D_801B27E8 += 1;
    func_80081C74__for_func_80081C30();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80081C74(void)
{
extern s32 D_8013B20C;
extern s32 D_801B27E8;
extern void func_80081CB0__for_func_80081C74(void) __asm__("func_80081CB0");
extern void func_80081D48__for_func_80081C74(void) __asm__("func_80081D48");
extern void func_80081CF4__for_func_80081C74(void) __asm__("func_80081CF4");

    if (D_8013B20C == 0)
    {
        D_801B27E8 += 1;
        func_80081CB0__for_func_80081C74();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_80081CB0(void)
{
extern s32 D_8013B20C;
extern s32 D_801B27E8;
extern void func_80081CB0(void);
extern void func_80081D48__for_func_80081CB0(void) __asm__("func_80081D48");
extern void func_80081CF4__for_func_80081CB0(void) __asm__("func_80081CF4");

    func_8006CAC0(func_80081D48__for_func_80081CB0);
    D_8013B20C = 1;
    D_801B27E8 += 1;
    func_80081CF4__for_func_80081CB0();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80081CF4(void)
{
extern s32 D_801B27E8;
extern s32 D_8013B20C;
extern void func_80081D30__for_func_80081CF4(void) __asm__("func_80081D30");

    if (D_8013B20C == 0)
    {
        D_801B27E8 += 1;
        func_80081D30__for_func_80081CF4();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80081D30(void)
{
extern s32 D_801B27E8;
extern s32 D_8013B20C;
extern void func_80081D30(void);

    D_801B27E8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80081D48(s32 arg0)
{
extern u32 D_801B27F0;
extern s32 D_801B27F4;
extern void (*D_800D5708[])(void);

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
void func_80081DC0(void)
{
extern u32 D_801B27F0;
extern s32 D_801B27F4;
extern void (*D_800D5708[])(void);

    D_801B27F0 = 1;
    D_801B27F4 = 1;
}

/** @brief Set world-map color, play sound 30, register two callbacks, and begin an eight-tick delay. */
void func_80081DD8(void)
{
extern s32 D_801B27F0;
extern s32 D_801B27F4;
extern void func_80082360__for_func_80081DD8(void) __asm__("func_80082360");
extern void func_800826A0__for_func_80081DD8(void) __asm__("func_800826A0");

    wmap_start_map_tint(0x122840);
    g_wmap_backdrop_target_level = 4;
    wmap_play_sound(0x1E, 0x80);
    func_8006CAC0(&func_80082360__for_func_80081DD8);
    func_8006CAC0(&func_800826A0__for_func_80081DD8);
    D_801B27F4 = 8;
    D_801B27F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80081E44(void)
{
extern s32 D_801B27F4;
extern s32 D_801B27F0;
extern void func_80082B04__for_func_80081E44(void) __asm__("func_80082B04");

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
extern s32 D_801B27F4;
extern s32 D_801B27F0;
extern void func_80082B04__for_func_80081E78(void) __asm__("func_80082B04");

    func_8006CAC0(func_80082B04__for_func_80081E78);
    D_801B27F4 = 0x4;
    D_801B27F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80081EB4(void)
{
extern s32 D_801B27F4;
extern s32 D_801B27F0;

    if (--D_801B27F4 == 0)
    {
        D_801B27F0 += 1;
    }
}

/** @brief World-map step handler: register the next callback and advance the counter. */
void func_80081EE8(void)
{
extern void func_80082D70__for_func_80081EE8(void) __asm__("func_80082D70");
extern s32 D_801ADAE0;
extern s32 D_801B27F4;
extern s32 D_801B27F0;

    func_8006CAC0(func_80082D70__for_func_80081EE8);
    D_801ADAE0 = 1;
    D_801B27F4 = 4;
    D_801B27F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80081F30(void)
{
extern s32 D_801B27F4;
extern s32 D_801B27F0;
extern void func_80082FDC__for_func_80081F30(void) __asm__("func_80082FDC");

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
extern s32 D_801B27F4;
extern s32 D_801B27F0;
extern void func_80082FDC__for_func_80081F64(void) __asm__("func_80082FDC");

    func_8006CAC0(func_80082FDC__for_func_80081F64);
    D_801B27F4 = 0x4;
    D_801B27F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80081FA0(void)
{
extern s32 D_801B27F4;
extern s32 D_801B27F0;
extern void func_80083248__for_func_80081FA0(void) __asm__("func_80083248");

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
extern s32 D_801B27F4;
extern s32 D_801B27F0;
extern void func_80083248__for_func_80081FD4(void) __asm__("func_80083248");

    func_8006CAC0(func_80083248__for_func_80081FD4);
    D_801B27F4 = 0x24;
    D_801B27F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80082010(void)
{
extern s32 D_801B27F4;
extern s32 D_801B27F0;
extern void func_800827F8__for_func_80082010(void) __asm__("func_800827F8");
extern void func_800838D8__for_func_80082010(void) __asm__("func_800838D8");
extern void func_80083CBC__for_func_80082010(void) __asm__("func_80083CBC");

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
extern s32 D_801B27F4;
extern s32 D_801B27F0;
extern void func_800827F8__for_func_80082044(void) __asm__("func_800827F8");
extern void func_800838D8__for_func_80082044(void) __asm__("func_800838D8");
extern void func_80083CBC__for_func_80082044(void) __asm__("func_80083CBC");

    func_8006CAC0(func_800827F8__for_func_80082044);
    func_8006CAC0(func_800838D8__for_func_80082044);
    func_8006CAC0(func_80083CBC__for_func_80082044);
    D_801B27F4 = 0x38;
    D_801B27F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80082098(void)
{
extern s32 D_801B27F4;
extern s32 D_801B27F0;
extern void func_80083980__for_func_80082098(void) __asm__("func_80083980");

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
extern s32 D_801B27F4;
extern s32 D_801B27F0;
extern void func_80083980__for_func_800820CC(void) __asm__("func_80083980");

    func_8006CAC0(func_80083980__for_func_800820CC);
    D_801B27F4 = 0xA;
    D_801B27F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80082108(void)
{
extern s32 D_801B27F4;
extern s32 D_801B27F0;
extern void func_800834B8__for_func_80082108(void) __asm__("func_800834B8");

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
extern s32 D_801B27F4;
extern s32 D_801B27F0;
extern void func_800834B8__for_func_8008213C(void) __asm__("func_800834B8");

    func_8006CAC0(func_800834B8__for_func_8008213C);
    D_801B27F4 = 0x10;
    D_801B27F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80082178(void)
{
extern s32 D_801B27F4;
extern s32 D_801B27F0;
extern void func_80082500__for_func_80082178(void) __asm__("func_80082500");

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
extern s32 D_801B27F4;
extern s32 D_801B27F0;
extern void func_80082500__for_func_800821AC(void) __asm__("func_80082500");

    func_8006CAC0(func_80082500__for_func_800821AC);
    D_801B27F4 = 0x30;
    D_801B27F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800821E8(void)
{
extern s32 D_801B27F4;
extern s32 D_801B27F0;
extern void func_80082950__for_func_800821E8(void) __asm__("func_80082950");

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
extern s32 D_801B27F4;
extern s32 D_801B27F0;
extern void func_80082950__for_func_8008221C(void) __asm__("func_80082950");

    func_8006CAC0(func_80082950__for_func_8008221C);
    D_801B27F4 = 0x2;
    D_801B27F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80082258(void)
{
extern s32 D_801B27F4;
extern s32 D_801B27F0;
extern void func_800836C8__for_func_80082258(void) __asm__("func_800836C8");

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
extern s32 D_801B27F4;
extern s32 D_801B27F0;
extern void func_800836C8__for_func_8008228C(void) __asm__("func_800836C8");

    func_8006CAC0(func_800836C8__for_func_8008228C);
    D_801B27F4 = 0x28;
    D_801B27F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800822C8(void)
{
extern s32 D_801B27F4;
extern s32 D_801B27F0;

    if (--D_801B27F4 == 0)
    {
        D_801B27F0 += 1;
    }
}

/** @brief Update the selected world-map land record and advance the sequence step. */
void func_800822FC(void)
{
typedef struct
{
    u32 value;
    u8 unknown_4[36];
} WmapValueRecord;

extern s32 D_8013B20C;
extern WmapValueRecord D_80139290[][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 D_801B27F0;

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
extern u32 D_801B27F8;
extern s32 D_801B27FC;
extern void (*D_800D5768[])(void);
extern void func_8008246C__for_func_80082360(void) __asm__("func_8008246C");
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

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
void func_800823D8(void)
{
extern u32 D_801B27F8;
extern s32 D_801B27FC;
extern void (*D_800D5768[])(void);
extern void func_8008246C__for_func_800823D8(void) __asm__("func_8008246C");
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

    D_801B27F8 = 1;
    D_801B27FC = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_800823F0(void)
{
extern u32 D_801B27F8;
extern s32 D_801B27FC;
extern void (*D_800D5768[])(void);
extern void func_8008246C__for_func_800823F0(void) __asm__("func_8008246C");
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

    D_801399AC = D_8011D538;
    D_800D9318[0x6] = 0xF;
    *(s16*)&D_800D9318[0x10] = -1;
    *(s16*)&D_800D9318[0x26] = 8;
    *(s16*)&D_800D9318[0x2] = 0;
    *(s16*)&D_800D9318[0xE] = 0;
    *(s16*)&D_800D9318[0x22] = 0x80;
    *(s16*)&D_800D9318[0x24] = 0;
    D_801B27FC = 0x3C;
    D_801B27F8 += 1;
    func_8008246C__for_func_800823F0();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8008246C(void)
{
extern u32 D_801B27F8;
extern s32 D_801B27FC;
extern void (*D_800D5768[])(void);
extern void func_8008246C(void);
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

    wmap_step_actor_animation(D_800D9318, D_801399A8);
    wmap_draw_actor_sprite(D_800D9318, D_8011CF4C, 0x13, 0x14, 0);
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
extern s32 D_801B27F8;

    D_801B27F8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80082500(s32 arg0)
{
extern u32 D_801B2800;
extern s32 D_801B2804;
extern void (*D_800D5778[])(void);
extern void func_8008260C__for_func_80082500(void) __asm__("func_8008260C");
extern u8 D_80121538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

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
void func_80082578(void)
{
extern u32 D_801B2800;
extern s32 D_801B2804;
extern void (*D_800D5778[])(void);
extern void func_8008260C__for_func_80082578(void) __asm__("func_8008260C");
extern u8 D_80121538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    D_801B2800 = 1;
    D_801B2804 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80082590(void)
{
extern u32 D_801B2800;
extern s32 D_801B2804;
extern void (*D_800D5778[])(void);
extern void func_8008260C__for_func_80082590(void) __asm__("func_8008260C");
extern u8 D_80121538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    D_801399B4 = D_80121538;
    D_800D9344[0x6] = 0xF;
    *(s16*)&D_800D9344[0x10] = -1;
    *(s16*)&D_800D9344[0x26] = 8;
    *(s16*)&D_800D9344[0x2] = 0;
    *(s16*)&D_800D9344[0xE] = 0;
    *(s16*)&D_800D9344[0x22] = 0x80;
    *(s16*)&D_800D9344[0x24] = 0x80;
    D_801B2804 = 0x5D;
    D_801B2800 += 1;
    func_8008260C__for_func_80082590();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8008260C(void)
{
extern u32 D_801B2800;
extern s32 D_801B2804;
extern void (*D_800D5778[])(void);
extern void func_8008260C(void);
extern u8 D_80121538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    wmap_step_actor_animation(D_800D9344, D_801399B0);
    wmap_draw_actor_sprite(D_800D9344, D_8011CF4C, 0x18, 0x33, 0);
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
extern s32 D_801B2800;

    D_801B2800 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800826A0(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2808;
extern s32 D_801B280C;
extern void (*D_800D5788[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_80080FC8__for_func_800826A0(void) __asm__("func_80080FC8");

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
void func_80082718(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2808;
extern s32 D_801B280C;
extern void (*D_800D5788[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_80080FC8__for_func_80082718(void) __asm__("func_80080FC8");

    D_801B2808 = 1;
    D_801B280C = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_80082730(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2808;
extern s32 D_801B280C;
extern void (*D_800D5788[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_80080FC8__for_func_80082730(void) __asm__("func_80080FC8");

    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.w[2] = 0xAFC8;
    D_801B280C = 0x14;
    D_801B2808 += 1;
    func_80080FC8__for_func_80082730();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800827E0(void)
{
extern s32 D_801B2808;

    D_801B2808 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800827F8(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2810;
extern s32 D_801B2814;
extern void (*D_800D5798[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_800810C8__for_func_800827F8(void) __asm__("func_800810C8");

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
void func_80082870(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2810;
extern s32 D_801B2814;
extern void (*D_800D5798[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_800810C8__for_func_80082870(void) __asm__("func_800810C8");

    D_801B2810 = 1;
    D_801B2814 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_80082888(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2810;
extern s32 D_801B2814;
extern void (*D_800D5798[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_800810C8__for_func_80082888(void) __asm__("func_800810C8");

    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.w[2] = 0xAFC8;
    D_801B2814 = 0x40;
    D_801B2810 += 1;
    func_800810C8__for_func_80082888();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80082938(void)
{
extern s32 D_801B2810;

    D_801B2810 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80082950(s32 arg0)
{
extern u32 D_801B2818;
extern s32 D_801B281C;
extern void (*D_800D57A8[])(void);

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
void func_800829C8(void)
{
extern u32 D_801B2818;
extern s32 D_801B281C;
extern void (*D_800D57A8[])(void);

    D_801B2818 = 1;
    D_801B281C = 1;
}

/** @brief Configure the actor, save its screen position, and begin a 44-tick delay. */
void func_800829E0(void)
{
/** @brief World-map actor configuration with its original field layout. */
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

extern void func_80082A70__for_func_800829E0(void) __asm__("func_80082A70");
extern WmapConfigA D_800D9370;
extern s32 D_8011CF4C;
extern u8 D_80121538[];
extern u8 *D_801399BC;
extern s32 D_80182D58;
extern s32 D_801B2818;
extern s32 D_801B281C;

    D_801399BC = D_80121538;
    D_800D9370.field_06 = 0xF;
    D_800D9370.field_0E = 1;
    D_800D9370.field_10 = -1;
    D_800D9370.field_26 = 8;
    D_800D9370.field_22 = 0x80;
    D_800D9370.field_24 = 0x80;
    D_800D9370.field_02 = 0;
    D_801B281C = 0x2C;
    D_80182D58 = D_8011CF4C;
    D_801B2818 += 1;
    func_80082A70__for_func_800829E0();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80082A70(void)
{
extern s32 D_801B2818;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;
extern s32 D_801B281C;

    wmap_step_actor_animation(D_800D9370, D_801399B8);
    wmap_draw_actor_sprite(D_800D9370, D_8011CF4C, 0x18, 0x22, 0);
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
extern s32 D_801B2818;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;
extern s32 D_801B281C;

    D_801B2818 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80082B04(s32 arg0)
{
extern u32 D_801B2820;
extern s32 D_801B2824;
extern void (*D_800D57B8[])(void);
extern void func_80082C14__for_func_80082B04(void) __asm__("func_80082C14");
extern u8 D_8011F538[];
extern u8* D_801399C4;
extern u8 D_800D939C[];
extern u8 D_801399C0[];
extern s32 D_8011CF4C;

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
void func_80082B7C(void)
{
extern u32 D_801B2820;
extern s32 D_801B2824;
extern void (*D_800D57B8[])(void);
extern void func_80082C14__for_func_80082B7C(void) __asm__("func_80082C14");
extern u8 D_8011F538[];
extern u8* D_801399C4;
extern u8 D_800D939C[];
extern u8 D_801399C0[];
extern s32 D_8011CF4C;

    D_801B2820 = 1;
    D_801B2824 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80082B94(void)
{
extern u32 D_801B2820;
extern s32 D_801B2824;
extern void (*D_800D57B8[])(void);
extern void func_80082C14__for_func_80082B94(void) __asm__("func_80082C14");
extern u8 D_8011F538[];
extern u8* D_801399C4;
extern u8 D_800D939C[];
extern u8 D_801399C0[];
extern s32 D_8011CF4C;

    D_801399C4 = D_8011F538;
    D_800D939C[0x6] = 0xF;
    *(s16*)&D_800D939C[0x10] = -1;
    *(s16*)&D_800D939C[0x26] = 2;
    *(s16*)&D_800D939C[0x22] = 0x81;
    *(s16*)&D_800D939C[0x2] = 0;
    *(s16*)&D_800D939C[0xE] = 0;
    *(s16*)&D_800D939C[0x24] = 1;
    D_801B2824 = 0x30;
    D_801B2820 += 1;
    func_80082C14__for_func_80082B94();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80082C14(void)
{
extern u32 D_801B2820;
extern s32 D_801B2824;
extern void (*D_800D57B8[])(void);
extern void func_80082C14(void);
extern u8 D_8011F538[];
extern u8* D_801399C4;
extern u8 D_800D939C[];
extern u8 D_801399C0[];
extern s32 D_8011CF4C;

    wmap_step_actor_animation(D_800D939C, D_801399C0);
    wmap_draw_actor_sprite(D_800D939C, D_8011CF4C, 0x8, 0xA, 0);
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
extern void func_80082CDC__for_func_80082C90(void) __asm__("func_80082CDC");
extern s16 D_800D939C[];
extern s32 D_801B2824;
extern s32 D_801B2820;
extern u8 D_801399C0[];
extern s32 D_8011CF4C;

    D_800D939C[19] = 2;
    D_800D939C[17] = 0;
    D_801B2824 = 0x8;
    D_801B2820 += 1;
    func_80082CDC__for_func_80082C90();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80082CDC(void)
{
extern void func_80082CDC(void);
extern s16 D_800D939C[];
extern s32 D_801B2824;
extern s32 D_801B2820;
extern u8 D_801399C0[];
extern s32 D_8011CF4C;

    wmap_step_actor_animation(D_800D939C, D_801399C0);
    wmap_draw_actor_sprite(D_800D939C, D_8011CF4C, 0x8, 0xA, 0);
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
extern s32 D_801B2820;

    D_801B2820 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80082D70(s32 arg0)
{
extern u32 D_801B2828;
extern s32 D_801B282C;
extern void (*D_800D57D0[])(void);
extern void func_80082E80__for_func_80082D70(void) __asm__("func_80082E80");
extern u8 D_80127538[];
extern u8* D_801399CC;
extern u8 D_800D93C8[];
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

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
void func_80082DE8(void)
{
extern u32 D_801B2828;
extern s32 D_801B282C;
extern void (*D_800D57D0[])(void);
extern void func_80082E80__for_func_80082DE8(void) __asm__("func_80082E80");
extern u8 D_80127538[];
extern u8* D_801399CC;
extern u8 D_800D93C8[];
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

    D_801B2828 = 1;
    D_801B282C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80082E00(void)
{
extern u32 D_801B2828;
extern s32 D_801B282C;
extern void (*D_800D57D0[])(void);
extern void func_80082E80__for_func_80082E00(void) __asm__("func_80082E80");
extern u8 D_80127538[];
extern u8* D_801399CC;
extern u8 D_800D93C8[];
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

    D_801399CC = D_80127538;
    D_800D93C8[0x6] = 0xF;
    *(s16*)&D_800D93C8[0x10] = -1;
    *(s16*)&D_800D93C8[0x26] = 8;
    *(s16*)&D_800D93C8[0x22] = 0x81;
    *(s16*)&D_800D93C8[0x2] = 0;
    *(s16*)&D_800D93C8[0xE] = 0;
    *(s16*)&D_800D93C8[0x24] = 1;
    D_801B282C = 0x20;
    D_801B2828 += 1;
    func_80082E80__for_func_80082E00();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80082E80(void)
{
extern u32 D_801B2828;
extern s32 D_801B282C;
extern void (*D_800D57D0[])(void);
extern void func_80082E80(void);
extern u8 D_80127538[];
extern u8* D_801399CC;
extern u8 D_800D93C8[];
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

    wmap_step_actor_animation(D_800D93C8, D_801399C8);
    wmap_draw_actor_sprite(D_800D93C8, D_8011CF4C, 0x8, 0xA, 0);
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
extern void func_80082F48__for_func_80082EFC(void) __asm__("func_80082F48");
extern s16 D_800D93C8[];
extern s32 D_801B282C;
extern s32 D_801B2828;
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

    D_800D93C8[19] = 8;
    D_800D93C8[17] = 0;
    D_801B282C = 0x8;
    D_801B2828 += 1;
    func_80082F48__for_func_80082EFC();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80082F48(void)
{
extern void func_80082F48(void);
extern s16 D_800D93C8[];
extern s32 D_801B282C;
extern s32 D_801B2828;
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

    wmap_step_actor_animation(D_800D93C8, D_801399C8);
    wmap_draw_actor_sprite(D_800D93C8, D_8011CF4C, 0x8, 0xA, 0);
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
extern s32 D_801B2828;

    D_801B2828 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80082FDC(s32 arg0)
{
extern u32 D_801B2830;
extern s32 D_801B2834;
extern void (*D_800D57E8[])(void);

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
void func_80083054(void)
{
extern u32 D_801B2830;
extern s32 D_801B2834;
extern void (*D_800D57E8[])(void);

    D_801B2830 = 1;
    D_801B2834 = 1;
}

/** @brief Configure the world-map actor and advance to its draw step. */
void func_8008306C(void)
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
} __attribute__((aligned(4))) WmapConfigA;


extern WmapConfigA D_800D93F4;
extern u8 D_80127538;
extern void *D_801399D4;
extern s32 D_801B2830;
extern s32 D_801B2834;
extern void func_800830EC__for_func_8008306C(void) __asm__("func_800830EC");

    D_801399D4 = &D_80127538;
    D_800D93F4.field_06 = 0xF;
    D_800D93F4.field_0E = 1;
    D_800D93F4.field_10 = -1;
    D_800D93F4.field_26 = 8;
    D_800D93F4.field_02 = 0;
    D_800D93F4.field_22 = 0x81;
    D_800D93F4.field_24 = 1;
    D_801B2834 = 0x1C;
    D_801B2830 += 1;
    func_800830EC__for_func_8008306C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800830EC(void)
{
extern void func_800831B4__for_func_800830EC(void) __asm__("func_800831B4");
extern s16 D_800D93F4[];
extern s32 D_801B2834;
extern s32 D_801B2830;
extern u8 D_801399D0[];
extern s32 D_8011CF4C;

    wmap_step_actor_animation(D_800D93F4, D_801399D0);
    wmap_draw_actor_sprite(D_800D93F4, D_8011CF4C, 0x8, 0xA, 0);
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
extern void func_800831B4__for_func_80083168(void) __asm__("func_800831B4");
extern s16 D_800D93F4[];
extern s32 D_801B2834;
extern s32 D_801B2830;
extern u8 D_801399D0[];
extern s32 D_8011CF4C;

    D_800D93F4[19] = 8;
    D_800D93F4[17] = 0;
    D_801B2834 = 0x8;
    D_801B2830 += 1;
    func_800831B4__for_func_80083168();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800831B4(void)
{
extern void func_800831B4(void);
extern s16 D_800D93F4[];
extern s32 D_801B2834;
extern s32 D_801B2830;
extern u8 D_801399D0[];
extern s32 D_8011CF4C;

    wmap_step_actor_animation(D_800D93F4, D_801399D0);
    wmap_draw_actor_sprite(D_800D93F4, D_8011CF4C, 0x8, 0xA, 0);
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
extern s32 D_801B2830;

    D_801B2830 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80083248(s32 arg0)
{
extern u32 D_801B2838;
extern s32 D_801B283C;
extern void (*D_800D5800[])(void);

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
void func_800832C0(void)
{
extern u32 D_801B2838;
extern s32 D_801B283C;
extern void (*D_800D5800[])(void);

    D_801B2838 = 1;
    D_801B283C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_800832D8(void)
{
extern u8* D_801399DC;
extern u8 D_80127538[];
extern u8 D_800D9420[];
extern s32 D_801B2838;
extern s32 D_801B283C;
extern void func_8008335C__for_func_800832D8(void) __asm__("func_8008335C");

    D_801399DC = D_80127538;
    D_800D9420[0x6] = 0xF;
    *(s16*)&D_800D9420[0xE] = 2;
    *(s16*)&D_800D9420[0x10] = -1;
    *(s16*)&D_800D9420[0x26] = 8;
    *(s16*)&D_800D9420[0x22] = 0x81;
    *(s16*)&D_800D9420[0x2] = 0;
    *(s16*)&D_800D9420[0x24] = 1;
    D_801B283C = 0x18;
    D_801B2838 += 1;
    func_8008335C__for_func_800832D8();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8008335C(void)
{
extern void func_80083424__for_func_8008335C(void) __asm__("func_80083424");
extern s16 D_800D9420[];
extern s32 D_801B283C;
extern s32 D_801B2838;
extern u8 D_801399D8[];
extern s32 D_8011CF4C;

    wmap_step_actor_animation(D_800D9420, D_801399D8);
    wmap_draw_actor_sprite(D_800D9420, D_8011CF4C, 0x8, 0xA, 0);
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
extern void func_80083424__for_func_800833D8(void) __asm__("func_80083424");
extern s16 D_800D9420[];
extern s32 D_801B283C;
extern s32 D_801B2838;
extern u8 D_801399D8[];
extern s32 D_8011CF4C;

    D_800D9420[19] = 8;
    D_800D9420[17] = 0;
    D_801B283C = 0x8;
    D_801B2838 += 1;
    func_80083424__for_func_800833D8();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80083424(void)
{
extern void func_80083424(void);
extern s16 D_800D9420[];
extern s32 D_801B283C;
extern s32 D_801B2838;
extern u8 D_801399D8[];
extern s32 D_8011CF4C;

    wmap_step_actor_animation(D_800D9420, D_801399D8);
    wmap_draw_actor_sprite(D_800D9420, D_8011CF4C, 0x8, 0xA, 0);
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
extern s32 D_801B2838;

    D_801B2838 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800834B8(s32 arg0)
{
extern u32 D_801B2840;
extern s32 D_801B2844;
extern void (*D_800D5818[])(void);

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
void func_80083530(void)
{
extern u32 D_801B2840;
extern s32 D_801B2844;
extern void (*D_800D5818[])(void);

    D_801B2840 = 1;
    D_801B2844 = 1;
}

/** @brief World-map step: clear a run of per-entry slots, then schedule the next step. */
void func_80083548(void)
{
/** @brief World-map 0x14-byte slot: only the leading halfword is cleared here. */
typedef struct
{
    s16 field_00;
    u8 pad_02[0x12];
} WmapSlot14;

/** @brief World-map 8-byte slot: only the +4 pointer field is written here. */
typedef struct
{
    s32 field_00;
    void *field_04;
} WmapSlot8;

extern s32 D_801B0FD0;
extern s32 D_80139980;
extern WmapSlot14 D_801AFBD0[];
extern WmapSlot8 D_80139988[];
extern s32 D_80125538;
extern s32 D_801B2840;
extern s32 D_801B2844;
extern void func_800835DC__for_func_80083548(void) __asm__("func_800835DC");

    s32 i;

    D_801B0FD0 = 0;
    D_80139980 = 0x7F;
    for (i = 0; i < 0x32; i++)
    {
        D_801AFBD0[i + 0x13].field_00 = 0;
        D_80139988[i + 0x13].field_04 = &D_80125538;
    }
    D_801B2844 = 0x32;
    D_801B2840 += 1;
    func_800835DC__for_func_80083548();
}

/** @brief Advance the effect on odd frames, draw it, and update the countdown. */
void func_800835DC(void)
{
extern s32 D_8011CF74;
extern s32 D_801B0FD0;
extern s32 D_801B2840;
extern s32 D_801B2844;

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
extern s32 D_801B2840;
extern void func_800811C8__for_func_80083678(void) __asm__("func_800811C8");
extern s32 D_801B2844;

    D_801B2844 = 0x20;
    D_801B2840 += 1;
    func_800811C8__for_func_80083678();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800836B0(void)
{
extern s32 D_801B2840;
extern void func_800811C8__for_func_800836B0(void) __asm__("func_800811C8");
extern s32 D_801B2844;

    D_801B2840 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800836C8(s32 arg0)
{
extern u32 D_801B2848;
extern s32 D_801B284C;
extern void (*D_800D5830[])(void);

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
void func_80083740(void)
{
extern u32 D_801B2848;
extern s32 D_801B284C;
extern void (*D_800D5830[])(void);

    D_801B2848 = 1;
    D_801B284C = 1;
}

/** @brief World-map step: clear a run of per-entry slots, then schedule the next step. */
void func_80083758(void)
{
/** @brief World-map 0x14-byte slot: only the leading halfword is cleared here. */
typedef struct
{
    s16 field_00;
    u8 pad_02[0x12];
} WmapSlot14;

/** @brief World-map 8-byte slot: only the +4 pointer field is written here. */
typedef struct
{
    s32 field_00;
    void *field_04;
} WmapSlot8;

extern s32 D_801B0FD0;
extern s32 D_80139980;
extern WmapSlot14 D_801AFBD0[];
extern WmapSlot8 D_80139988[];
extern s32 D_80125538;
extern s32 D_801B2848;
extern s32 D_801B284C;
extern void func_800837EC__for_func_80083758(void) __asm__("func_800837EC");

    s32 i;

    D_801B0FD0 = 0;
    D_80139980 = 0x7F;
    for (i = 0; i < 0x8; i++)
    {
        D_801AFBD0[i + 0x54].field_00 = 0;
        D_80139988[i + 0x54].field_04 = &D_80125538;
    }
    D_801B284C = 2;
    D_801B2848 += 1;
    func_800837EC__for_func_80083758();
}

/** @brief Advance the effect on odd frames, draw it, and update the countdown. */
void func_800837EC(void)
{
extern s32 D_8011CF74;
extern s32 D_801B0FD0;
extern s32 D_801B2848;
extern s32 D_801B284C;

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
extern s32 D_801B2848;
extern void func_80081294__for_func_80083888(void) __asm__("func_80081294");
extern s32 D_801B284C;

    D_801B284C = 0x18;
    D_801B2848 += 1;
    func_80081294__for_func_80083888();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800838C0(void)
{
extern s32 D_801B2848;
extern void func_80081294__for_func_800838C0(void) __asm__("func_80081294");
extern s32 D_801B284C;

    D_801B2848 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800838D8(s32 arg0)
{
extern u32 D_801B2850;
extern s32 D_801B2854;
extern void (*D_800D5848[])(void);

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
void func_80083950(void)
{
extern u32 D_801B2850;
extern s32 D_801B2854;
extern void (*D_800D5848[])(void);

    D_801B2850 = 1;
    D_801B2854 = 1;
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80083968(void)
{
extern u32 D_801B2850;
extern s32 D_801B2854;
extern void (*D_800D5848[])(void);

    D_801B2850 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80083980(s32 arg0)
{
extern u32 D_801B2858;
extern s32 D_801B285C;
extern void (*D_800D5858[])(void);

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
void func_800839F8(void)
{
extern u32 D_801B2858;
extern s32 D_801B285C;
extern void (*D_800D5858[])(void);

    D_801B2858 = 1;
    D_801B285C = 1;
}

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void func_80083A10(void)
{
extern s32 D_801B25D8;
extern s32 D_801B2858;
extern s32 D_801B285C;

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
extern void func_80083B04__for_func_80083ACC(void) __asm__("func_80083B04");
extern s32 D_801B285C;
extern s32 D_801B2858;

    D_801B285C = 0x18;
    D_801B2858 += 1;
    func_80083B04__for_func_80083ACC();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void func_80083B04(void)
{
extern s32 D_801B285C;
extern s32 D_801B2858;

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
extern void func_80083BF4__for_func_80083BB4(void) __asm__("func_80083BF4");
extern s32 D_800DCEA8;
extern s32 D_801B285C;
extern s32 D_801B2858;

    D_800DCEA8 = 0;
    D_801B285C = 0x18;
    D_801B2858 += 1;
    func_80083BF4__for_func_80083BB4();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void func_80083BF4(void)
{
extern s32 D_801B285C;
extern s32 D_801B2858;

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
extern s32 D_801B2858;

    D_801B2858 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80083CBC(s32 arg0)
{
extern u32 D_801B2860;
extern s32 D_801B2864;
extern void (*D_800D5878[])(void);

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
void func_80083D34(void)
{
extern u32 D_801B2860;
extern s32 D_801B2864;
extern void (*D_800D5878[])(void);

    D_801B2860 = 1;
    D_801B2864 = 1;
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80083D4C(void)
{
extern u32 D_801B2860;
extern s32 D_801B2864;
extern void (*D_800D5878[])(void);

    D_801B2860 += 1;
}
