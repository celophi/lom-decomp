#include "wmap_main.h"
#include "wmap_land_effect_09.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_resource_support.h"
#include "wmap_view_effects.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"

void func_80096BD0(void);
void func_80096DC8(void);
void func_800970F8(void);
void func_800972C4(void);
void func_800974BC(void);
void func_80096008(void);
void func_80096044(void);
s32 func_800960DC(s32 arg0);
void func_80096088(void);
void func_800960C4(void);
s32 func_80097068(s32 arg0);
s32 func_80096554(s32 arg0);
s32 func_80096890(s32 arg0);
s32 func_80096B40(s32 arg0);
s32 func_80097234(s32 arg0);
s32 func_80096D38(s32 arg0);
s32 func_8009742C(s32 arg0);
s32 func_800969E8(s32 arg0);
s32 func_800966F0(s32 arg0);
void func_8009665C(void);
void func_800967FC(void);
void func_80096C9C(void);
void func_80096EB8(void);
void func_80096FA4(void);
void func_800971BC(void);
void func_80097390(void);
void func_80097588(void);

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_800958C8(void)
{
extern s32 D_801B2B80;
extern s32 D_801B2650[];
extern SVECTOR D_801B24A0;
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B2BAC;
extern s32 D_801B2BA8;

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
        wmap_draw_model_default((s32)D_800DCF18, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DE8);
        D_80182DE8 -= 0x2;
        if (D_80182DE8 < 0)
        {
            D_80182DE8 = 0;
        }
    }

    PopMatrix();
    if (--D_801B2BAC == 0)
    {
        D_801B2BA8 += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_800959C8(void)
{
extern s32 D_801B2478[];
extern SVECTOR D_801B24A8;
extern VECTOR D_8011CF60;
extern s32 D_80182DEC;
extern s32 D_8011CF1C;
extern s32 D_801B2BB4;
extern s32 D_801B2BB0;

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
        wmap_draw_model_default(D_8011CF1C, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DEC);
        D_80182DEC -= 0x2;
        if (D_80182DEC < 0)
        {
            D_80182DEC = 0;
        }
    }

    PopMatrix();
    if (--D_801B2BB4 == 0)
    {
        D_801B2BB0 += 1;
    }
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_80095AC8(void)
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

extern s32 *D_80139280;
extern WmapSlot8 D_80139988[];
extern WmapSlot14 D_801AFBD0[];
extern s32 D_801B0FD0;
extern s32 D_801B2BB8;
extern s32 D_801B2BBC;
extern u8 D_80121538[];

    s32 i;

    D_801B0FD0 = 50;
    D_80139280[0x1] = 0;
    D_80139280[0x2] = 0;
    D_80139280[0x3] = 255;
    D_80139280[0x4] = 0;
    D_80139280[0x5] = 2;
    D_80139280[0x6] = 1000;
    D_80139280[0x7] = 200;
    D_80139280[0x8] = 8;
    D_80139280[0x9] = 0;
    D_80139280[0xA] = 18200;
    for (i = 0; i < 50; i++)
    {
        D_801AFBD0[i + D_80139280[0x7]].field_00 = 0;
        D_80139988[i + 204].field_04 = D_80121538;
    }
    D_801B2BBC = 100;
    D_801B2BB8++;
    func_80096BD0();
}

/**
 * @brief Initialize a range of world-map per-entry records and schedule the next step.
 */
void func_80095BAC(void)
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
extern s32 D_80121538[];
extern WmapPair D_80139988[];
extern WmapAfcEntry D_801AFBD0[];
extern s32 D_800D9150;
extern s32 D_800DCEA8;
extern s32 D_80182DF0;
extern s32 D_801B2BC0;
extern s32 D_801B2BC4;

    s32 i;
    WmapD94Entry *entry;

    i = 150;
    D_80182DF0 = 1;
    D_800DCEA8 = 1;

    do
    {
        D_801AFBD0[i].unk0 = 0;
        D_80139988[i].unk4 = D_80121538;
        entry = &D_800D9268[i];
        entry->unk2 = 0;
        entry->unk6 = 0xF;
        entry->unkE = 1;
        entry->unk10 = -1;
        i++;
    } while (i < 158);

    D_800D9150 = 3;
    D_801B2BC4 = 0x10;
    D_801B2BC0 += 1;
    func_80096DC8();
}

/** @brief Initialize the actor group and its animation resources, then advance. */
void func_80095C70(void)
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
extern u8 D_80123538[];
extern s32 D_801B0FD0;
extern s32 D_801B2BC8;
extern s32 D_801B2BCC;

    s32 i;
    WmapConfigA* config;

    D_801B0FD0 = 4;
    for (i = 140; i < 144; i++)
    {
        config = &D_800D9268[i];
        D_80139988[i].data = D_80123538;
        config->field_02 = 0;
        config->field_06 = 15;
        config->field_0E = 1;
        config->field_10 = -1;
        config->field_22 = 129;
        config->field_24 = 129;
        config->field_26 = 8;
        D_801AFBD0[i].field_00 = 1;
        D_801AFBD0[i].angle = i << 10;
        D_801AFBD0[i].field_04 = -20000;
        D_801AFBD0[i].field_08 = 990000;
        D_801AFBD0[i].field_0C = 9999;
        D_801AFBD0[i].field_0E = 240;
        D_801AFBD0[i].field_10 = 160;
    }
    D_801B2BCC = 28;
    D_801B2BC8++;
    func_800970F8();
}

/** @brief World-map step: fill a spawn descriptor, clear its slot run, then advance. */
void func_80095D68(void)
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

extern s32 *D_80139280;
extern WmapSlot8 D_80139988[];
extern WmapSlot14 D_801AFBD0[];
extern s32 D_801B0FD0;
extern s32 D_801B2BD0;
extern s32 D_801B2BD4;
extern u8 D_80123538[];

    s32 i;

    D_801B0FD0 = 12;
    D_80139280[0x1F] = 1;
    D_80139280[0x20] = 3;
    D_80139280[0x21] = 64;
    D_80139280[0x22] = 30;
    D_80139280[0x23] = -2;
    D_80139280[0x24] = 1000;
    D_80139280[0x25] = 20;
    D_80139280[0x26] = 15;
    D_80139280[0x27] = 0;
    D_80139280[0x28] = 10000;
    for (i = 0; i < 12; i++)
    {
        D_801AFBD0[i + D_80139280[0x25]].field_00 = 0;
        D_80139988[i + 24].field_04 = D_80123538;
    }
    D_801B2BD4 = 64;
    D_801B2BD0++;
    func_800972C4();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_80095E58(void)
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

extern s32 *D_80139280;
extern WmapSlot8 D_80139988[];
extern WmapSlot14 D_801AFBD0[];
extern s32 D_801B0FD0;
extern s32 D_801B2BD8;
extern s32 D_801B2BDC;
extern u8 D_80123538[];

    s32 i;

    D_801B0FD0 = 20;
    D_80139280[0xB] = 0;
    D_80139280[0xC] = 0;
    D_80139280[0xF] = 1;
    D_80139280[0x10] = 120;
    D_80139280[0x11] = 100;
    D_80139280[0x12] = 15;
    D_80139280[0x13] = 2;
    D_80139280[0x14] = 18500;
    for (i = 0; i < 20; i++)
    {
        D_801AFBD0[i + D_80139280[0x11]].field_00 = 0;
        D_80139988[i + 104].field_04 = D_80123538;
    }
    D_801B2BDC = 20;
    D_801B2BD8++;
    func_800974BC();
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80095F34(s32 arg0)
{
extern u32 D_801B2B88;
extern s32 D_801B2B8C;
extern void (*D_800D62A0[])(void);
extern s32 D_8013B20C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2B88 = 1;
        D_801B2B8C = 1;
        return 1;
    }

    if (D_801B2B88 < 0x6)
    {
        D_800D62A0[D_801B2B88]();
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
void func_80095FAC(void)
{
extern u32 D_801B2B88;
extern s32 D_801B2B8C;
extern void (*D_800D62A0[])(void);
extern s32 D_8013B20C;

    D_801B2B88 = 1;
    D_801B2B8C = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_80095FC4(void)
{
extern u32 D_801B2B88;
extern s32 D_801B2B8C;
extern void (*D_800D62A0[])(void);
extern s32 D_8013B20C;

    wmap_start_sequence(wmap_run_land_focus);
    D_8013B20C = 1;
    D_801B2B88 += 1;
    func_80096008();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80096008(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2B88;

    if (D_8013B20C == 0)
    {
        D_801B2B88 += 1;
        func_80096044();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_80096044(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2B88;
extern void func_80096044(void);

    wmap_start_sequence(func_800960DC);
    D_8013B20C = 1;
    D_801B2B88 += 1;
    func_80096088();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80096088(void)
{
extern s32 D_801B2B88;
extern s32 D_8013B20C;

    if (D_8013B20C == 0)
    {
        D_801B2B88 += 1;
        func_800960C4();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800960C4(void)
{
extern s32 D_801B2B88;
extern s32 D_8013B20C;
extern void func_800960C4(void);

    D_801B2B88 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800960DC(s32 arg0)
{
extern u32 D_801B2B90;
extern s32 D_801B2B94;
extern void (*D_800D62B8[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2B90 = 1;
        D_801B2B94 = 1;
        return 1;
    }

    if (D_801B2B90 < 0x10)
    {
        D_800D62B8[D_801B2B90]();
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
void func_80096154(void)
{
extern u32 D_801B2B90;
extern s32 D_801B2B94;
extern void (*D_800D62B8[])(void);

    D_801B2B90 = 1;
    D_801B2B94 = 1;
}

/**
 * @brief Set up a world-map sub-scene: request assets and register its handler.
 */
void func_8009616C(void)
{
extern s32 D_8013B208;
extern s32 D_801B2B90;
extern s32 D_801B2B94;

    D_8013B208 = 1;
    wmap_play_sound(0x30, 0x80);
    wmap_start_sequence(func_80097068);
    D_801B2B94 = 0x28;
    D_801B2B90 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800961C0(void)
{
extern s32 D_801B2B94;
extern s32 D_801B2B90;

    if (--D_801B2B94 == 0)
    {
        D_801B2B90 += 1;
    }
}

/** @brief Register four callbacks, set effect color and flags, and begin a one-tick delay. */
void func_800961F4(void)
{
extern s32 D_80139244;
extern s32 D_801B2B90;
extern s32 D_801B2B94;

    wmap_start_sequence(&func_80096890);
    wmap_start_sequence(&func_80096B40);
    wmap_start_sequence(&func_80097234);
    D_80139244 = 1;
    wmap_start_sequence(&func_80096554);
    wmap_start_map_tint(0x701040);
    g_wmap_backdrop_target_level = 4;
    D_801B2B94 = 1;
    D_801B2B90 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009627C(void)
{
extern s32 D_801B2B94;
extern s32 D_801B2B90;

    if (--D_801B2B94 == 0)
    {
        D_801B2B90 += 1;
    }
}

/**
 * @brief World-map step handler: set flags and advance the step counter.
 */
void func_800962B0(void)
{
extern s32 D_801ADAE0;
extern s32 D_801B2B90;
extern s32 D_801B2B94;

    D_801ADAE0 = 1;
    D_801B2B94 = 0x23;
    D_801B2B90 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800962DC(void)
{
extern s32 D_801B2B94;
extern s32 D_801B2B90;

    if (--D_801B2B94 == 0)
    {
        D_801B2B90 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80096310(void)
{
extern s32 D_801B2B94;
extern s32 D_801B2B90;

    wmap_start_sequence(func_80096D38);
    D_801B2B94 = 0x8;
    D_801B2B90 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009634C(void)
{
extern s32 D_801B2B94;
extern s32 D_801B2B90;

    if (--D_801B2B94 == 0)
    {
        D_801B2B90 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80096380(void)
{
extern s32 D_801B2B94;
extern s32 D_801B2B90;

    wmap_start_sequence(func_8009742C);
    D_801B2B94 = 0x32;
    D_801B2B90 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800963BC(void)
{
extern s32 D_801B2B94;
extern s32 D_801B2B90;

    if (--D_801B2B94 == 0)
    {
        D_801B2B90 += 1;
    }
}

/** @brief Register a callback, set world-map state and color, and begin a two-tick delay. */
void func_800963F0(void)
{
extern s32 D_80139244;
extern s32 D_801B2B90;
extern s32 D_801B2B94;

    wmap_start_sequence(&func_800969E8);
    D_80139244 = 0;
    g_wmap_backdrop_target_level = 0x10;
    wmap_start_map_tint(0x808080);
    D_801B2B94 = 2;
    D_801B2B90 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009644C(void)
{
extern s32 D_801B2B94;
extern s32 D_801B2B90;

    if (--D_801B2B94 == 0)
    {
        D_801B2B90 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80096480(void)
{
extern s32 D_801B2B94;
extern s32 D_801B2B90;

    wmap_start_sequence(func_800966F0);
    D_801B2B94 = 0xAB;
    D_801B2B90 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800964BC(void)
{
extern s32 D_801B2B94;
extern s32 D_801B2B90;

    if (--D_801B2B94 == 0)
    {
        D_801B2B90 += 1;
    }
}

void func_800964F0(void)
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
extern s32 D_801B2B90;

    D_8013B20C = 0;
    D_80139290[D_8011D510][D_8011D530].value = D_8011D4FC | 0x100;
    D_801B2B90 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80096554(s32 arg0)
{
extern u32 D_801B2B98;
extern s32 D_801B2B9C;
extern void (*D_800D62F8[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2B98 = 1;
        D_801B2B9C = 1;
        return 1;
    }

    if (D_801B2B98 < 0x4)
    {
        D_800D62F8[D_801B2B98]();
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
void func_800965CC(void)
{
extern u32 D_801B2B98;
extern s32 D_801B2B9C;
extern void (*D_800D62F8[])(void);

    D_801B2B98 = 1;
    D_801B2B9C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its next step.
 */
void func_800965E4(void)
{
extern u8* D_801399AC;
extern u8 D_8011F538[];
extern u8 D_800D9318[];
extern s32 D_801B2B98;
extern s32 D_801B2B9C;

    D_801399AC = D_8011F538;
    D_800D9318[0x6] = 0xF;
    *(s16*)&D_800D9318[0x10] = -1;
    *(s16*)&D_800D9318[0x2] = 0;
    *(s16*)&D_800D9318[0xE] = 0;
    *(s16*)&D_800D9318[0x26] = 0;
    *(s16*)&D_800D9318[0x22] = 0x81;
    *(s16*)&D_800D9318[0x24] = 0x81;
    D_801B2B9C = 0x4C;
    D_801B2B98 += 1;
    func_8009665C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8009665C(void)
{
extern s32 D_801B2B98;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 g_wmap_focus_screen_position;
extern s32 D_801B2B9C;

    wmap_step_actor_animation(D_800D9318, D_801399A8);
    wmap_draw_actor_sprite(D_800D9318, g_wmap_focus_screen_position, 0xF, 0x4, 0);
    if (--D_801B2B9C == 0)
    {
        D_801B2B98 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800966D8(void)
{
extern s32 D_801B2B98;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 g_wmap_focus_screen_position;
extern s32 D_801B2B9C;

    D_801B2B98 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800966F0(s32 arg0)
{
extern u32 D_801B2BA0;
extern s32 D_801B2BA4;
extern void (*D_800D6308[])(void);
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 g_wmap_focus_screen_position;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2BA0 = 1;
        D_801B2BA4 = 1;
        return 1;
    }

    if (D_801B2BA0 < 0x4)
    {
        D_800D6308[D_801B2BA0]();
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
void func_80096768(void)
{
extern u32 D_801B2BA0;
extern s32 D_801B2BA4;
extern void (*D_800D6308[])(void);
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 g_wmap_focus_screen_position;

    D_801B2BA0 = 1;
    D_801B2BA4 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80096780(void)
{
extern u32 D_801B2BA0;
extern s32 D_801B2BA4;
extern void (*D_800D6308[])(void);
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 g_wmap_focus_screen_position;

    D_801399B4 = D_8011D538;
    D_800D9344[0x6] = 0xF;
    *(s16*)&D_800D9344[0x10] = -1;
    *(s16*)&D_800D9344[0x26] = 8;
    *(s16*)&D_800D9344[0x2] = 0;
    *(s16*)&D_800D9344[0xE] = 0;
    *(s16*)&D_800D9344[0x22] = 0x80;
    *(s16*)&D_800D9344[0x24] = 0;
    D_801B2BA4 = 0xAC;
    D_801B2BA0 += 1;
    func_800967FC();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800967FC(void)
{
extern u32 D_801B2BA0;
extern s32 D_801B2BA4;
extern void (*D_800D6308[])(void);
extern void func_800967FC(void);
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 g_wmap_focus_screen_position;

    wmap_step_actor_animation(D_800D9344, D_801399B0);
    wmap_draw_actor_sprite(D_800D9344, g_wmap_focus_screen_position, 0x17, 0x8, 0);
    if (--D_801B2BA4 == 0)
    {
        D_801B2BA0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80096878(void)
{
extern s32 D_801B2BA0;

    D_801B2BA0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80096890(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2BA8;
extern s32 D_801B2BAC;
extern void (*D_800D6318[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2BA8 = 1;
        D_801B2BAC = 1;
        return 1;
    }

    if (D_801B2BA8 < 0x4)
    {
        D_800D6318[D_801B2BA8]();
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
void func_80096908(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2BA8;
extern s32 D_801B2BAC;
extern void (*D_800D6318[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;

    D_801B2BA8 = 1;
    D_801B2BAC = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_80096920(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2BA8;
extern s32 D_801B2BAC;
extern void (*D_800D6318[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;

    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.w[2] = 0xAFC8;
    D_801B2BAC = 0x40;
    D_801B2BA8 += 1;
    func_800958C8();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800969D0(void)
{
extern s32 D_801B2BA8;

    D_801B2BA8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800969E8(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2BB0;
extern s32 D_801B2BB4;
extern void (*D_800D6328[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2BB0 = 1;
        D_801B2BB4 = 1;
        return 1;
    }

    if (D_801B2BB0 < 0x4)
    {
        D_800D6328[D_801B2BB0]();
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
void func_80096A60(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2BB0;
extern s32 D_801B2BB4;
extern void (*D_800D6328[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;

    D_801B2BB0 = 1;
    D_801B2BB4 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_80096A78(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2BB0;
extern s32 D_801B2BB4;
extern void (*D_800D6328[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;

    D_801B24A8 = D_80139258;
    D_801B2478 = g_wmap_camera_translation;
    D_80182DEC = 0x80;
    D_801B2478.w[2] = 0xAFC8;
    D_801B2BB4 = 0x40;
    D_801B2BB0 += 1;
    func_800959C8();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80096B28(void)
{
extern s32 D_801B2BB0;

    D_801B2BB0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80096B40(s32 arg0)
{
extern u32 D_801B2BB8;
extern s32 D_801B2BBC;
extern void (*D_800D6338[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2BB8 = 1;
        D_801B2BBC = 1;
        return 1;
    }

    if (D_801B2BB8 < 0x6)
    {
        D_800D6338[D_801B2BB8]();
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
void func_80096BB8(void)
{
extern u32 D_801B2BB8;
extern s32 D_801B2BBC;
extern void (*D_800D6338[])(void);

    D_801B2BB8 = 1;
    D_801B2BBC = 1;
}

/** @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires. */
void func_80096BD0(void)
{
extern u8 D_800DB578[];
extern u8 D_80139FE8[];
extern s32 D_80139280;
extern s32 D_801B2BBC;
extern s32 D_801B2BB8;

    func_8006A2FC(D_800DB578, D_80139FE8, 0x32, 0x7F, 1, 4, 5, D_80139280);
    if (--D_801B2BBC == 0)
    {
        D_801B2BB8 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_80096C54(void)
{
extern s32* D_80139280;
extern s32 D_801B2BBC;
extern s32 D_801B2BB8;

    D_801B2BBC = 0x40;
    D_80139280[5] = -1;
    D_801B2BB8 += 1;
    func_80096C9C();
}

/** @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires. */
void func_80096C9C(void)
{
extern u8 D_800DB578[];
extern u8 D_80139FE8[];
extern s32 D_80139280;
extern s32 D_801B2BBC;
extern s32 D_801B2BB8;

    func_8006A2FC(D_800DB578, D_80139FE8, 0x32, 0x7F, 1, 4, 5, D_80139280);
    if (--D_801B2BBC == 0)
    {
        D_801B2BB8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80096D20(void)
{
extern s32 D_801B2BB8;

    D_801B2BB8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80096D38(s32 arg0)
{
extern u32 D_801B2BC0;
extern s32 D_801B2BC4;
extern void (*D_800D6350[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2BC0 = 1;
        D_801B2BC4 = 1;
        return 1;
    }

    if (D_801B2BC0 < 0x8)
    {
        D_800D6350[D_801B2BC0]();
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
void func_80096DB0(void)
{
extern u32 D_801B2BC0;
extern s32 D_801B2BC4;
extern void (*D_800D6350[])(void);

    D_801B2BC0 = 1;
    D_801B2BC4 = 1;
}

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void func_80096DC8(void)
{
extern s32 D_80182DF0;
extern s32 D_801B2BC0;
extern s32 D_801B2BC4;

    s32 remaining;

    func_8006B328(0x96, 0x9E, 3, -1, -1, -5, 0, 8, -0x50, 0xA0, -0x50, 0xA0, 1, 0x7F, 1, 4, 0);
    D_80182DF0 += 8;
    remaining = D_801B2BC4 - 1;
    D_801B2BC4 = remaining;
    if (remaining == 0)
    {
        D_801B2BC0 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80096E80(void)
{
extern s32 D_801B2BC4;
extern s32 D_801B2BC0;

    D_801B2BC4 = 0x18;
    D_801B2BC0 += 1;
    func_80096EB8();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void func_80096EB8(void)
{
extern s32 D_801B2BC4;
extern s32 D_801B2BC0;

    func_8006B328(0x96, 0x9E, 3, -1, -1, -5, 0, 8, -0x50, 0xA0, -0x50, 0xA0, 1, 0x7F, 1, 4, 0);
    if (--D_801B2BC4 == 0)
    {
        D_801B2BC0 += 1;
    }
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void func_80096F64(void)
{
extern s32 D_800DCEA8;
extern s32 D_801B2BC4;
extern s32 D_801B2BC0;

    D_800DCEA8 = 0;
    D_801B2BC4 = 0x14;
    D_801B2BC0 += 1;
    func_80096FA4();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void func_80096FA4(void)
{
extern s32 D_801B2BC4;
extern s32 D_801B2BC0;

    func_8006B328(0x96, 0x9E, 3, -1, -1, -5, 0, 8, -0x50, 0xA0, -0x50, 0xA0, 1, 0x7F, 1, 4, 0);
    if (--D_801B2BC4 == 0)
    {
        D_801B2BC0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80097050(void)
{
extern s32 D_801B2BC0;

    D_801B2BC0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80097068(s32 arg0)
{
extern u32 D_801B2BC8;
extern s32 D_801B2BCC;
extern void (*D_800D6370[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2BC8 = 1;
        D_801B2BCC = 1;
        return 1;
    }

    if (D_801B2BC8 < 0x6)
    {
        D_800D6370[D_801B2BC8]();
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
void func_800970E0(void)
{
extern u32 D_801B2BC8;
extern s32 D_801B2BCC;
extern void (*D_800D6370[])(void);

    D_801B2BC8 = 1;
    D_801B2BCC = 1;
}

/** @brief World-map step handler: draw the sprite element, then countdown-advance the step. */
void func_800970F8(void)
{
extern s32 D_801B2BC8;
extern s32 D_801B2BCC;

    func_8006B6EC(0x8C, 0x90, 0xF, -6, 4);
    if (--D_801B2BCC == 0)
    {
        D_801B2BC8 += 1;
    }
}

/**
 * @brief Reset a range of world-map actor configs, arm the timer, and advance the step.
 */
void func_80097158(void)
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
extern s32 D_801B2BC8;
extern s32 D_801B2BCC;

    s32 i;

    for (i = 0x8C; i < 0x90; i++)
    {
        D_800D9268[i].field_22 = 0;
        D_800D9268[i].field_26 = 8;
    }
    D_801B2BCC = 0x10;
    D_801B2BC8 += 1;
    func_800971BC();
}

/** @brief World-map step handler: draw the sprite element, then countdown-advance the step. */
void func_800971BC(void)
{
extern s32 D_801B2BC8;
extern s32 D_801B2BCC;

    func_8006B6EC(0x8C, 0x90, 0xF, -6, 4);
    if (--D_801B2BCC == 0)
    {
        D_801B2BC8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009721C(void)
{
extern s32 D_801B2BC8;

    D_801B2BC8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80097234(s32 arg0)
{
extern u32 D_801B2BD0;
extern s32 D_801B2BD4;
extern void (*D_800D6388[])(void);
extern u8 D_800D9688[];
extern u8 D_80139A48[];
extern s32 D_80139280;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2BD0 = 1;
        D_801B2BD4 = 1;
        return 1;
    }

    if (D_801B2BD0 < 0x6)
    {
        D_800D6388[D_801B2BD0]();
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
void func_800972AC(void)
{
extern u32 D_801B2BD0;
extern s32 D_801B2BD4;
extern void (*D_800D6388[])(void);
extern u8 D_800D9688[];
extern u8 D_80139A48[];
extern s32 D_80139280;

    D_801B2BD0 = 1;
    D_801B2BD4 = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_800972C4(void)
{
extern u32 D_801B2BD0;
extern s32 D_801B2BD4;
extern void (*D_800D6388[])(void);
extern u8 D_800D9688[];
extern u8 D_80139A48[];
extern s32 D_80139280;

    func_8006A2FC(D_800D9688, D_80139A48, 0xC, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--D_801B2BD4 == 0)
    {
        D_801B2BD0 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_80097348(void)
{
extern s32* D_80139280;
extern s32 D_801B2BD4;
extern s32 D_801B2BD0;
extern u8 D_800D9688[];
extern u8 D_80139A48[];

    D_801B2BD4 = 0x20;
    D_80139280[35] = -1;
    D_801B2BD0 += 1;
    func_80097390();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_80097390(void)
{
extern void func_80097390(void);
extern s32* D_80139280;
extern s32 D_801B2BD4;
extern s32 D_801B2BD0;
extern u8 D_800D9688[];
extern u8 D_80139A48[];

    func_8006A2FC(D_800D9688, D_80139A48, 0xC, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--D_801B2BD4 == 0)
    {
        D_801B2BD0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80097414(void)
{
extern s32 D_801B2BD0;

    D_801B2BD0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009742C(s32 arg0)
{
extern u32 D_801B2BD8;
extern s32 D_801B2BDC;
extern void (*D_800D63A0[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2BD8 = 1;
        D_801B2BDC = 1;
        return 1;
    }

    if (D_801B2BD8 < 0x6)
    {
        D_800D63A0[D_801B2BD8]();
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
void func_800974A4(void)
{
extern u32 D_801B2BD8;
extern s32 D_801B2BDC;
extern void (*D_800D63A0[])(void);

    D_801B2BD8 = 1;
    D_801B2BDC = 1;
}

/**
 * @brief World-map step handler: submit a batched sprite draw, then advance the
 *        sequence once its frame counter expires.
 */
void func_800974BC(void)
{
extern u8 D_800DA448[];
extern u8 D_80139CC8[];
extern s32* D_80139280;
extern s32 D_801B2BD8;
extern s32 D_801B2BDC;

    func_8006A2FC(D_800DA448, D_80139CC8, 0x14, 0x7F, 0x7F, 0x4, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B2BDC == 0)
    {
        D_801B2BD8 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_80097540(void)
{
extern s32* D_80139280;
extern s32 D_801B2BDC;
extern s32 D_801B2BD8;

    D_801B2BDC = 0x20;
    D_80139280[15] = -1;
    D_801B2BD8 += 1;
    func_80097588();
}

/**
 * @brief World-map step handler: submit a batched sprite draw, then advance the
 *        sequence once its frame counter expires.
 */
void func_80097588(void)
{
extern u8 D_800DA448[];
extern u8 D_80139CC8[];
extern s32* D_80139280;
extern s32 D_801B2BD8;
extern s32 D_801B2BDC;

    func_8006A2FC(D_800DA448, D_80139CC8, 0x14, 0x7F, 0x7F, 0x4, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B2BDC == 0)
    {
        D_801B2BD8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009760C(void)
{
extern s32 D_801B2BD8;
extern s32 D_801B2650[];
extern SVECTOR D_801B24A0;
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B2C0C;
extern s32 D_801B2C08;

    D_801B2BD8 += 1;
}
