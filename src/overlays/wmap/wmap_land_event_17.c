#include "wmap_land_event_17.h"
#include "wmap_sequence_runtime.h"
#include "wmap_view_effects.h"
#include "wmap_map_labels.h"
#include "cdrom.h"
#include "sdk/libgte.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"

/** @brief Set the effect resources and map-relative position, then advance. */
void func_800AAA2C(void)
{
/** @brief Four-word world-map projection state. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
    s32 pad;
} WmapTransform;

extern u8 D_800DEF18[];
extern u8 *D_8011CF1C;
extern u8 *D_8011CF24;
extern s16 D_8011CF4C[];
extern s32 D_8013B258;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_801398D0;
extern WmapTransform D_80139950;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern s32 D_801B2E78;
extern void func_800AB8E0(void);

    D_8011CF1C = D_800DEF18;
    D_8011CF24 = D_800DEF18 + 0x2000;
    D_8011CF4C[0] = 0x94;
    D_8011CF4C[1] = 0x31;
    D_8013B258 = 1;
    func_8006D0F0(0x11, &D_800DCEF8, &D_800DCF00);
    D_801398D0 = 2;
    D_80182D68 = ((D_800DCEF8 - 1) * 0x30) - D_80139950.x;
    D_80182D78 = ((D_800DCF00 - 1) * 0x30) - D_80139950.y;
    D_801B2E78++;
    func_800AB8E0();
}

/** @brief Set transition controls, queue resources, and start the loading countdown. */
void func_800AAB18(void)
{
extern u8 D_800DCF18[];
extern u8 *D_8011CF1C;
extern u8 *D_8011CF24;
extern u8 D_8011D538[];
extern s32 D_8013B208;
extern u8 D_80182E40[];
extern u8 D_8018B240[];
extern u8 D_80193640[];
extern s32 D_801ADAE0;
extern s32 D_801ADAEC;
extern s32 D_801ADAF4;
extern s32 D_801B2E80;
extern s32 D_801B2E84;

    D_8013B208 = 1;
    func_8006683C(0x301020);
    D_801ADAF4 = 4;
    D_801ADAE0 = 1;
    D_801ADAEC = 0;
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
extern u8* D_801399AC;
extern u8 D_8011D538[];
extern u8 D_800D9318[];
extern u16 D_8011CF4C[];
extern s16 D_80182D58[];
extern s32 D_801B2E8C;
extern s32 D_801B2E88;
extern void func_800AC9AC(void);

    D_801399AC = D_8011D538;
    D_800D9318[0x6] = 0xF;
    *(s16*)&D_800D9318[0x10] = -1;
    *(s16*)&D_800D9318[0x26] = 0x2;
    *(s16*)&D_800D9318[0x2] = 0;
    *(s16*)&D_800D9318[0xE] = 0;
    *(s16*)&D_800D9318[0x22] = 0x81;
    *(s16*)&D_800D9318[0x24] = 0x81;
    D_80182D58[0] = D_8011CF4C[0] + ((rand() * 50) >> 15) - 10;
    D_80182D58[1] = D_8011CF4C[1] + ((rand() * 30) >> 15) + 15;
    D_801B2E8C = 0x24;
    D_801B2E88 += 1;
    func_800AC9AC();
}

/**
 * @brief Spawn a world-map wandering actor and randomize its start position.
 */
void func_800AAD04(void)
{
extern u8* D_801399B4;
extern u8 D_8011D538[];
extern u8 D_800D9344[];
extern u16 D_8011CF4C[];
extern s16 D_80182D60[];
extern s32 D_801B2E94;
extern s32 D_801B2E90;
extern void func_800ACAD0(void);

    D_801399B4 = D_8011D538;
    D_800D9344[0x6] = 0xF;
    *(s16*)&D_800D9344[0x10] = -1;
    *(s16*)&D_800D9344[0x26] = 0x2;
    *(s16*)&D_800D9344[0x2] = 0;
    *(s16*)&D_800D9344[0xE] = 0;
    *(s16*)&D_800D9344[0x22] = 0x81;
    *(s16*)&D_800D9344[0x24] = 0x81;
    D_80182D60[0] = D_8011CF4C[0] + ((rand() * 50) >> 15) - 10;
    D_80182D60[1] = D_8011CF4C[1] + ((rand() * 30) >> 15) + 15;
    D_801B2E94 = 0x24;
    D_801B2E90 += 1;
    func_800ACAD0();
}

/**
 * @brief Spawn a world-map wandering actor and randomize its start position.
 */
void func_800AADEC(void)
{
extern u8* D_801399BC;
extern u8 D_8011D538[];
extern u8 D_800D9370[];
extern u16 D_8011CF4C[];
extern s16 D_80182D64[];
extern s32 D_801B2E9C;
extern s32 D_801B2E98;
extern void func_800ACBF4(void);

    D_801399BC = D_8011D538;
    D_800D9370[0x6] = 0xF;
    *(s16*)&D_800D9370[0x10] = -1;
    *(s16*)&D_800D9370[0x26] = 0x2;
    *(s16*)&D_800D9370[0x2] = 0;
    *(s16*)&D_800D9370[0xE] = 0;
    *(s16*)&D_800D9370[0x22] = 0x81;
    *(s16*)&D_800D9370[0x24] = 0x81;
    D_80182D64[0] = D_8011CF4C[0] + ((rand() * 50) >> 15) - 10;
    D_80182D64[1] = D_8011CF4C[1] + ((rand() * 30) >> 15) + 15;
    D_801B2E9C = 0x24;
    D_801B2E98 += 1;
    func_800ACBF4();
}

/**
 * @brief Spawn a world-map wandering actor and randomize its start position.
 */
void func_800AAED4(void)
{
extern u8* D_801399D4;
extern u8 D_8011F538[];
extern u8 D_800D93F4[];
extern u16 D_8011CF4C[];
extern s16 D_80182D6C[];
extern s32 D_801B2EA4;
extern s32 D_801B2EA0;
extern void func_800ACD18(void);

    D_801399D4 = D_8011F538;
    D_800D93F4[0x6] = 0xF;
    *(s16*)&D_800D93F4[0x10] = -1;
    *(s16*)&D_800D93F4[0x26] = 0x2;
    *(s16*)&D_800D93F4[0x2] = 0;
    *(s16*)&D_800D93F4[0xE] = 0;
    *(s16*)&D_800D93F4[0x22] = 0x81;
    *(s16*)&D_800D93F4[0x24] = 0x81;
    D_80182D6C[0] = D_8011CF4C[0] + ((rand() * 50) >> 15) - 10;
    D_80182D6C[1] = D_8011CF4C[1] + ((rand() * 30) >> 15) + 15;
    D_801B2EA4 = 0x24;
    D_801B2EA0 += 1;
    func_800ACD18();
}

/**
 * @brief Spawn a world-map wandering actor and randomize its start position.
 */
void func_800AAFBC(void)
{
extern u8* D_801399CC;
extern u8 D_8011F538[];
extern u8 D_800D93C8[];
extern u16 D_8011CF4C[];
extern s16 D_80182D7C[];
extern s32 D_801B2EAC;
extern s32 D_801B2EA8;
extern void func_800ACE3C(void);

    D_801399CC = D_8011F538;
    D_800D93C8[0x6] = 0xF;
    *(s16*)&D_800D93C8[0x10] = -1;
    *(s16*)&D_800D93C8[0x26] = 0x2;
    *(s16*)&D_800D93C8[0x2] = 0;
    *(s16*)&D_800D93C8[0xE] = 0;
    *(s16*)&D_800D93C8[0x22] = 0x81;
    *(s16*)&D_800D93C8[0x24] = 0x81;
    D_80182D7C[0] = D_8011CF4C[0] + ((rand() * 50) >> 15) - 10;
    D_80182D7C[1] = D_8011CF4C[1] + ((rand() * 30) >> 15) + 15;
    D_801B2EAC = 0x24;
    D_801B2EA8 += 1;
    func_800ACE3C();
}

/**
 * @brief Spawn a world-map wandering actor and randomize its start position.
 */
void func_800AB0A4(void)
{
extern u8* D_801399C4;
extern u8 D_8011F538[];
extern u8 D_800D939C[];
extern u16 D_8011CF4C[];
extern s16 D_80182D84[];
extern s32 D_801B2EB4;
extern s32 D_801B2EB0;
extern void func_800ACF60(void);

    D_801399C4 = D_8011F538;
    D_800D939C[0x6] = 0xF;
    *(s16*)&D_800D939C[0x10] = -1;
    *(s16*)&D_800D939C[0x26] = 0x2;
    *(s16*)&D_800D939C[0x2] = 0;
    *(s16*)&D_800D939C[0xE] = 0;
    *(s16*)&D_800D939C[0x22] = 0x81;
    *(s16*)&D_800D939C[0x24] = 0x81;
    D_80182D84[0] = D_8011CF4C[0] + ((rand() * 50) >> 15) - 10;
    D_80182D84[1] = D_8011CF4C[1] + ((rand() * 30) >> 15) + 15;
    D_801B2EB4 = 0x24;
    D_801B2EB0 += 1;
    func_800ACF60();
}

/**
 * @brief Spawn a world-map wandering actor and randomize its start position.
 */
void func_800AB18C(void)
{
extern u8* D_801399DC;
extern u8 D_80121538[];
extern u8 D_800D9420[];
extern u16 D_8011CF4C[];
extern s16 D_80182D90[];
extern s32 D_801B2EBC;
extern s32 D_801B2EB8;
extern void func_800AD084(void);

    D_801399DC = D_80121538;
    D_800D9420[0x6] = 0xF;
    *(s16*)&D_800D9420[0x10] = -1;
    *(s16*)&D_800D9420[0x26] = 0x2;
    *(s16*)&D_800D9420[0x2] = 0;
    *(s16*)&D_800D9420[0xE] = 0;
    *(s16*)&D_800D9420[0x22] = 0x81;
    *(s16*)&D_800D9420[0x24] = 0x81;
    D_80182D90[0] = D_8011CF4C[0] + ((rand() * 50) >> 15) - 10;
    D_80182D90[1] = D_8011CF4C[1] + ((rand() * 30) >> 15) + 15;
    D_801B2EBC = 0x24;
    D_801B2EB8 += 1;
    func_800AD084();
}

/**
 * @brief Spawn a world-map wandering actor and randomize its start position.
 */
void func_800AB274(void)
{
extern u8* D_801399E4;
extern u8 D_80121538[];
extern u8 D_800D944C[];
extern u16 D_8011CF4C[];
extern s16 D_80182D98[];
extern s32 D_801B2EC4;
extern s32 D_801B2EC0;
extern void func_800AD1A8(void);

    D_801399E4 = D_80121538;
    D_800D944C[0x6] = 0xF;
    *(s16*)&D_800D944C[0x10] = -1;
    *(s16*)&D_800D944C[0x26] = 0x2;
    *(s16*)&D_800D944C[0x2] = 0;
    *(s16*)&D_800D944C[0xE] = 0;
    *(s16*)&D_800D944C[0x22] = 0x81;
    *(s16*)&D_800D944C[0x24] = 0x81;
    D_80182D98[0] = D_8011CF4C[0] + ((rand() * 50) >> 15) - 10;
    D_80182D98[1] = D_8011CF4C[1] + ((rand() * 30) >> 15) + 15;
    D_801B2EC4 = 0x24;
    D_801B2EC0 += 1;
    func_800AD1A8();
}

/**
 * @brief Spawn a world-map wandering actor and randomize its start position.
 */
void func_800AB35C(void)
{
extern u8* D_801399EC;
extern u8 D_80121538[];
extern u8 D_800D9478[];
extern u16 D_8011CF4C[];
extern s16 D_80182DB8[];
extern s32 D_801B2ECC;
extern s32 D_801B2EC8;
extern void func_800AD2CC(void);

    D_801399EC = D_80121538;
    D_800D9478[0x6] = 0xF;
    *(s16*)&D_800D9478[0x10] = -1;
    *(s16*)&D_800D9478[0x26] = 0x2;
    *(s16*)&D_800D9478[0x2] = 0;
    *(s16*)&D_800D9478[0xE] = 0;
    *(s16*)&D_800D9478[0x22] = 0x81;
    *(s16*)&D_800D9478[0x24] = 0x81;
    D_80182DB8[0] = D_8011CF4C[0] + ((rand() * 50) >> 15) - 10;
    D_80182DB8[1] = D_8011CF4C[1] + ((rand() * 30) >> 15) + 15;
    D_801B2ECC = 0x24;
    D_801B2EC8 += 1;
    func_800AD2CC();
}

/** @brief Approach the effect depth, draw its fading layer, and advance the countdown. */
void func_800AB444(void)
{
extern u8 D_800DCF18[];
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern SVECTOR D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B2ED0;
extern s32 D_801B2ED4;
extern void func_800675F0(void *, s32, s32, s32, s32, s32, s32, s32, s32, s32);

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
        func_800675F0(D_800DCF18, 0, 4, 53, 0x7800, 1, D_80182DE8, 50, -20, -1);
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
extern void func_800675F0(s32* a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7, s32 a8, s32 a9);
extern s32 D_801B2478[];
extern SVECTOR D_801B24A8;
extern VECTOR D_8011CF60;
extern s32* D_8011CF1C;
extern s32 D_80182DEC;
extern s32 D_801B2ED8;
extern s32 D_801B2EDC;

    MATRIX m;
    s32 x;
    s32 timer;

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
        func_800675F0(D_8011CF1C, 0, 4, 0x35, 0x7800, 1, D_80182DEC, -0x19, -0x32, -1);
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
extern void func_800675F0(s32* a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7, s32 a8, s32 a9);
extern s32 D_80139870[];
extern SVECTOR D_8013B238;
extern VECTOR D_8011CF60;
extern s32* D_8011CF24;
extern s32 D_80182DF0;
extern s32 D_801B2EE0;
extern s32 D_801B2EE4;

    MATRIX m;
    s32 x;
    s32 timer;

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
        func_800675F0(D_8011CF24, 0, 4, 0x35, 0x7800, 1, D_80182DF0, 0xF, -0x37, -1);
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
extern s32 D_8011D538[];
extern WmapPair D_80139988[];
extern WmapAfcEntry D_801AFBD0[];
extern s32 D_800D9154;
extern s32 D_800DCEAC;
extern s32 D_801B25D8;
extern s32 D_801B2EE8;
extern s32 D_801B2EEC;

extern void func_800AD7F8(void);

    s32 i;
    WmapD94Entry *entry;

    i = 80;
    D_801B25D8 = 1;
    D_800DCEAC = 1;

    do
    {
        D_801AFBD0[i].unk0 = 0;
        D_80139988[i].unk4 = D_8011D538;
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
extern u32 D_801B2E78;
extern s32 D_801B2E7C;
extern void (*D_800D6DEC[])(void);
extern s32 D_801398D0;
extern void func_800AB920(void);
extern void func_800AB9C8(void);
extern void func_800AB964(void);
extern s32 D_8013B20C;

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
extern u32 D_801B2E78;
extern s32 D_801B2E7C;
extern void (*D_800D6DEC[])(void);
extern s32 D_801398D0;
extern void func_800AB920(void);
extern void func_800AB9C8(void);
extern void func_800AB964(void);
extern s32 D_8013B20C;

    D_801B2E78 = 1;
    D_801B2E7C = 1;
}

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void func_800AB8E0(void)
{
extern u32 D_801B2E78;
extern s32 D_801B2E7C;
extern void (*D_800D6DEC[])(void);
extern s32 D_801398D0;
extern void func_800AB920(void);
extern void func_800AB9C8(void);
extern void func_800AB964(void);
extern s32 D_8013B20C;

    if (D_801398D0 != 2)
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
extern u32 D_801B2E78;
extern s32 D_801B2E7C;
extern void (*D_800D6DEC[])(void);
extern s32 D_801398D0;
extern void func_800AB920(void);
extern void func_800AB9C8(void);
extern void func_800AB964(void);
extern s32 D_8013B20C;

    func_8006CAC0(func_800AB9C8);
    D_8013B20C = 1;
    D_801B2E78 += 1;
    func_800AB964();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800AB964(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2E78;
extern void func_800AB9A0(void);

    if (D_8013B20C == 0)
    {
        D_801B2E78 += 1;
        func_800AB9A0();
    }
}

/** @brief World-map trigger: set two flags and bump a counter. */
void func_800AB9A0(void)
{
extern s32 D_8013B294;
extern s32 D_80139228;
extern s32 D_801B2E78;

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
extern u32 D_801B2E80;
extern s32 D_801B2E84;
extern void (*D_800D6E04[])(void);

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
extern u32 D_801B2E80;
extern s32 D_801B2E84;
extern void (*D_800D6E04[])(void);

    D_801B2E80 = 1;
    D_801B2E84 = 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800ABA58(void)
{
extern u32 D_801B2E80;
extern s32 D_801B2E84;
extern void (*D_800D6E04[])(void);

    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/** @brief Wait for queued CD work, initialize three buffers, and register the next callback. */
void func_800ABA8C(void)
{
extern void cdrom_wait_queue_empty(void);
extern u8 D_80182E40[];
extern u8 D_8018B240[];
extern u8 D_80193640[];
extern s32 D_801B2E80;
extern s32 D_801B2E84;
extern void func_800AD23C(void);

    cdrom_wait_queue_empty();
    func_800652A8(0x2D, 0x80);
    func_800651B4(&D_80182E40);
    func_800651B4(&D_8018B240);
    func_800651B4(&D_80193640);
    func_8006CAC0(&func_800AD23C);
    D_801B2E84 = 0x12;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800ABB00(void)
{
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800AD360(void);

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
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800AD360(void);

    func_8006CAC0(func_800AD360);
    D_801B2E84 = 0xC;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800ABB70(void)
{
extern s32 D_801B2E84;
extern s32 D_801B2E80;

    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/** @brief World-map step: register the four draw callbacks and tick the frame counter. */
void func_800ABBA4(void)
{
extern void func_800AC91C(void);
extern void func_800ACED0(void);
extern void func_800AD610(void);
extern void func_800AD768(void);
extern s32 D_801B2E80;
extern s32 D_801B2E84;

    func_8006CAC0(func_800AC91C);
    func_8006CAC0(func_800ACED0);
    func_8006CAC0(func_800AD610);
    func_8006CAC0(func_800AD768);
    D_801B2E84 = 4;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800ABC04(void)
{
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800ACC88(void);
extern void func_800ACA40(void);

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
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800ACC88(void);
extern void func_800ACA40(void);

    func_8006CAC0(func_800ACC88);
    func_8006CAC0(func_800ACA40);
    D_801B2E84 = 0x14;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800ABC80(void)
{
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800ACB64(void);
extern void func_800AD118(void);

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
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800ACB64(void);
extern void func_800AD118(void);

    func_8006CAC0(func_800ACB64);
    func_8006CAC0(func_800AD118);
    D_801B2E84 = 0x4;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800ABCFC(void)
{
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800AD23C(void);

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
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800AD23C(void);

    func_8006CAC0(func_800AD23C);
    D_801B2E84 = 0x8;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800ABD6C(void)
{
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800AD4B8(void);
extern void func_800ACDAC(void);

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
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800AD4B8(void);
extern void func_800ACDAC(void);

    func_8006CAC0(func_800AD4B8);
    func_8006CAC0(func_800ACDAC);
    D_801B2E84 = 0xE;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800ABDE8(void)
{
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800ACFF4(void);
extern void func_800ACED0(void);

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
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800ACFF4(void);
extern void func_800ACED0(void);

    func_8006CAC0(func_800ACFF4);
    func_8006CAC0(func_800ACED0);
    D_801B2E84 = 0x10;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800ABE64(void)
{
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800AD610(void);
extern void func_800AD118(void);
extern void func_800ACC88(void);

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
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800AD610(void);
extern void func_800AD118(void);
extern void func_800ACC88(void);

    func_8006CAC0(func_800AD610);
    func_8006CAC0(func_800AD118);
    func_8006CAC0(func_800ACC88);
    D_801B2E84 = 0x18;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800ABEEC(void)
{
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800AD23C(void);
extern void func_800ACB64(void);

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
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800AD23C(void);
extern void func_800ACB64(void);

    func_8006CAC0(func_800AD23C);
    func_8006CAC0(func_800ACB64);
    D_801B2E84 = 0x8;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800ABF68(void)
{
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800ACDAC(void);
extern void func_800ACA40(void);

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
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800ACDAC(void);
extern void func_800ACA40(void);

    func_8006CAC0(func_800ACDAC);
    func_8006CAC0(func_800ACA40);
    D_801B2E84 = 0xC;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800ABFE4(void)
{
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800AC91C(void);
extern void func_800ACED0(void);
extern void func_800AD610(void);

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
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800AC91C(void);
extern void func_800ACED0(void);
extern void func_800AD610(void);

    func_8006CAC0(func_800AC91C);
    func_8006CAC0(func_800ACED0);
    func_8006CAC0(func_800AD610);
    D_801B2E84 = 0x4;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC06C(void)
{
extern s32 D_801B2E84;
extern s32 D_801B2E80;

    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/** @brief Register a sequence callback, play sound 45, and start a 20-tick delay. */
void func_800AC0A0(void)
{
extern s32 D_801B2E80;
extern s32 D_801B2E84;
extern void func_800ACC88(void);

    func_8006CAC0(&func_800ACC88);
    func_800652A8(0x2D, 0x80);
    D_801B2E84 = 0x14;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC0E8(void)
{
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800ACB64(void);
extern void func_800AD118(void);

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
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800ACB64(void);
extern void func_800AD118(void);

    func_8006CAC0(func_800ACB64);
    func_8006CAC0(func_800AD118);
    D_801B2E84 = 0x4;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC164(void)
{
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800AD23C(void);

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
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800AD23C(void);

    func_8006CAC0(func_800AD23C);
    D_801B2E84 = 0x8;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC1D4(void)
{
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800AD4B8(void);
extern void func_800ACDAC(void);

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
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800AD4B8(void);
extern void func_800ACDAC(void);

    func_8006CAC0(func_800AD4B8);
    func_8006CAC0(func_800ACDAC);
    D_801B2E84 = 0xE;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC250(void)
{
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800ACFF4(void);
extern void func_800ACED0(void);

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
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800ACFF4(void);
extern void func_800ACED0(void);

    func_8006CAC0(func_800ACFF4);
    func_8006CAC0(func_800ACED0);
    D_801B2E84 = 0x10;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC2CC(void)
{
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800AD610(void);
extern void func_800AD118(void);
extern void func_800ACC88(void);

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
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800AD610(void);
extern void func_800AD118(void);
extern void func_800ACC88(void);

    func_8006CAC0(func_800AD610);
    func_8006CAC0(func_800AD118);
    func_8006CAC0(func_800ACC88);
    D_801B2E84 = 0x18;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC354(void)
{
extern s32 D_801B2E84;
extern s32 D_801B2E80;

    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/** @brief Play sound 45, register two callbacks, and begin an eight-tick delay. */
void func_800AC388(void)
{
extern s32 D_801B2E80;
extern s32 D_801B2E84;
extern void func_800ACB64(void);
extern void func_800AD23C(void);

    func_800652A8(0x2D, 0x80);
    func_8006CAC0(&func_800AD23C);
    func_8006CAC0(&func_800ACB64);
    D_801B2E84 = 8;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC3DC(void)
{
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800ACDAC(void);
extern void func_800ACA40(void);

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
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800ACDAC(void);
extern void func_800ACA40(void);

    func_8006CAC0(func_800ACDAC);
    func_8006CAC0(func_800ACA40);
    D_801B2E84 = 0xC;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC458(void)
{
extern s32 D_801B2E84;
extern s32 D_801B2E80;

    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/** @brief World-map step: register the four draw callbacks and tick the frame counter. */
void func_800AC48C(void)
{
extern void func_800AC91C(void);
extern void func_800ACED0(void);
extern void func_800AD610(void);
extern void func_800AD768(void);
extern s32 D_801B2E80;
extern s32 D_801B2E84;

    func_8006CAC0(func_800AC91C);
    func_8006CAC0(func_800ACED0);
    func_8006CAC0(func_800AD610);
    func_8006CAC0(func_800AD768);
    D_801B2E84 = 4;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC4EC(void)
{
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800ACC88(void);

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
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800ACC88(void);

    func_8006CAC0(func_800ACC88);
    D_801B2E84 = 0x14;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC55C(void)
{
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800ACB64(void);
extern void func_800AD118(void);

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
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800ACB64(void);
extern void func_800AD118(void);

    func_8006CAC0(func_800ACB64);
    func_8006CAC0(func_800AD118);
    D_801B2E84 = 0x4;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC5D8(void)
{
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800AD23C(void);

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
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800AD23C(void);

    func_8006CAC0(func_800AD23C);
    D_801B2E84 = 0x8;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC648(void)
{
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800AD4B8(void);
extern void func_800ACDAC(void);

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
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800AD4B8(void);
extern void func_800ACDAC(void);

    func_8006CAC0(func_800AD4B8);
    func_8006CAC0(func_800ACDAC);
    D_801B2E84 = 0xE;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC6C4(void)
{
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800ACFF4(void);
extern void func_800ACED0(void);

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
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800ACFF4(void);
extern void func_800ACED0(void);

    func_8006CAC0(func_800ACFF4);
    func_8006CAC0(func_800ACED0);
    D_801B2E84 = 0x10;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC740(void)
{
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800AD610(void);
extern void func_800AD118(void);
extern void func_800ACC88(void);

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
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800AD610(void);
extern void func_800AD118(void);
extern void func_800ACC88(void);

    func_8006CAC0(func_800AD610);
    func_8006CAC0(func_800AD118);
    func_8006CAC0(func_800ACC88);
    D_801B2E84 = 0x18;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC7C8(void)
{
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800AD23C(void);
extern void func_800AD360(void);
extern void func_800ACB64(void);

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
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800AD23C(void);
extern void func_800AD360(void);
extern void func_800ACB64(void);

    func_8006CAC0(func_800AD23C);
    func_8006CAC0(func_800AD360);
    func_8006CAC0(func_800ACB64);
    D_801B2E84 = 0x8;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC850(void)
{
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800ACDAC(void);
extern void func_800ACA40(void);

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
extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800ACDAC(void);
extern void func_800ACA40(void);

    func_8006CAC0(func_800ACDAC);
    func_8006CAC0(func_800ACA40);
    D_801B2E84 = 0x54;
    D_801B2E80 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AC8CC(void)
{
extern s32 D_801B2E84;
extern s32 D_801B2E80;

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
extern s32 D_801B2E80;
extern s32 D_8013B20C;

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
extern u32 D_801B2E88;
extern s32 D_801B2E8C;
extern void (*D_800D6EFC[])(void);
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_80182D58;

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
extern u32 D_801B2E88;
extern s32 D_801B2E8C;
extern void (*D_800D6EFC[])(void);
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_80182D58;

    D_801B2E88 = 1;
    D_801B2E8C = 1;
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800AC9AC(void)
{
extern u32 D_801B2E88;
extern s32 D_801B2E8C;
extern void (*D_800D6EFC[])(void);
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_80182D58;

    func_8006CC4C(D_800D9318, D_801399A8);
    func_80066F9C(D_800D9318, D_80182D58, 0x8, 0x2, 0);
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
extern s32 D_801B2E88;

    D_801B2E88 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800ACA40(s32 arg0)
{
extern u32 D_801B2E90;
extern s32 D_801B2E94;
extern void (*D_800D6F0C[])(void);
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_80182D60;

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
extern u32 D_801B2E90;
extern s32 D_801B2E94;
extern void (*D_800D6F0C[])(void);
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_80182D60;

    D_801B2E90 = 1;
    D_801B2E94 = 1;
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800ACAD0(void)
{
extern u32 D_801B2E90;
extern s32 D_801B2E94;
extern void (*D_800D6F0C[])(void);
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_80182D60;

    func_8006CC4C(D_800D9344, D_801399B0);
    func_80066F9C(D_800D9344, D_80182D60, 0x20, 0x2, 0);
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
extern s32 D_801B2E90;

    D_801B2E90 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800ACB64(s32 arg0)
{
extern u32 D_801B2E98;
extern s32 D_801B2E9C;
extern void (*D_800D6F1C[])(void);
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_80182D64;

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
extern u32 D_801B2E98;
extern s32 D_801B2E9C;
extern void (*D_800D6F1C[])(void);
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_80182D64;

    D_801B2E98 = 1;
    D_801B2E9C = 1;
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800ACBF4(void)
{
extern u32 D_801B2E98;
extern s32 D_801B2E9C;
extern void (*D_800D6F1C[])(void);
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_80182D64;

    func_8006CC4C(D_800D9370, D_801399B8);
    func_80066F9C(D_800D9370, D_80182D64, 0x21, 0x2, 0);
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
extern s32 D_801B2E98;

    D_801B2E98 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800ACC88(s32 arg0)
{
extern u32 D_801B2EA0;
extern s32 D_801B2EA4;
extern void (*D_800D6F2C[])(void);
extern u8 D_800D93F4[];
extern u8 D_801399D0[];
extern s32 D_80182D6C;

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
extern u32 D_801B2EA0;
extern s32 D_801B2EA4;
extern void (*D_800D6F2C[])(void);
extern u8 D_800D93F4[];
extern u8 D_801399D0[];
extern s32 D_80182D6C;

    D_801B2EA0 = 1;
    D_801B2EA4 = 1;
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800ACD18(void)
{
extern u32 D_801B2EA0;
extern s32 D_801B2EA4;
extern void (*D_800D6F2C[])(void);
extern u8 D_800D93F4[];
extern u8 D_801399D0[];
extern s32 D_80182D6C;

    func_8006CC4C(D_800D93F4, D_801399D0);
    func_80066F9C(D_800D93F4, D_80182D6C, 0x24, 0x2, 0);
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
extern s32 D_801B2EA0;

    D_801B2EA0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800ACDAC(s32 arg0)
{
extern u32 D_801B2EA8;
extern s32 D_801B2EAC;
extern void (*D_800D6F3C[])(void);
extern u8 D_800D93C8[];
extern u8 D_801399C8[];
extern s32 D_80182D7C;

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
extern u32 D_801B2EA8;
extern s32 D_801B2EAC;
extern void (*D_800D6F3C[])(void);
extern u8 D_800D93C8[];
extern u8 D_801399C8[];
extern s32 D_80182D7C;

    D_801B2EA8 = 1;
    D_801B2EAC = 1;
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800ACE3C(void)
{
extern u32 D_801B2EA8;
extern s32 D_801B2EAC;
extern void (*D_800D6F3C[])(void);
extern u8 D_800D93C8[];
extern u8 D_801399C8[];
extern s32 D_80182D7C;

    func_8006CC4C(D_800D93C8, D_801399C8);
    func_80066F9C(D_800D93C8, D_80182D7C, 0x11, 0x2, 0);
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
extern s32 D_801B2EA8;

    D_801B2EA8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800ACED0(s32 arg0)
{
extern u32 D_801B2EB0;
extern s32 D_801B2EB4;
extern void (*D_800D6F4C[])(void);
extern u8 D_800D939C[];
extern u8 D_801399C0[];
extern s32 D_80182D84;

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
extern u32 D_801B2EB0;
extern s32 D_801B2EB4;
extern void (*D_800D6F4C[])(void);
extern u8 D_800D939C[];
extern u8 D_801399C0[];
extern s32 D_80182D84;

    D_801B2EB0 = 1;
    D_801B2EB4 = 1;
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800ACF60(void)
{
extern u32 D_801B2EB0;
extern s32 D_801B2EB4;
extern void (*D_800D6F4C[])(void);
extern u8 D_800D939C[];
extern u8 D_801399C0[];
extern s32 D_80182D84;

    func_8006CC4C(D_800D939C, D_801399C0);
    func_80066F9C(D_800D939C, D_80182D84, 0x23, 0x2, 0);
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
extern s32 D_801B2EB0;

    D_801B2EB0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800ACFF4(s32 arg0)
{
extern u32 D_801B2EB8;
extern s32 D_801B2EBC;
extern void (*D_800D6F5C[])(void);
extern u8 D_800D9420[];
extern u8 D_801399D8[];
extern s32 D_80182D90;

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
extern u32 D_801B2EB8;
extern s32 D_801B2EBC;
extern void (*D_800D6F5C[])(void);
extern u8 D_800D9420[];
extern u8 D_801399D8[];
extern s32 D_80182D90;

    D_801B2EB8 = 1;
    D_801B2EBC = 1;
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800AD084(void)
{
extern u32 D_801B2EB8;
extern s32 D_801B2EBC;
extern void (*D_800D6F5C[])(void);
extern u8 D_800D9420[];
extern u8 D_801399D8[];
extern s32 D_80182D90;

    func_8006CC4C(D_800D9420, D_801399D8);
    func_80066F9C(D_800D9420, D_80182D90, 0x25, 0x2, 0);
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
extern s32 D_801B2EB8;

    D_801B2EB8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800AD118(s32 arg0)
{
extern u32 D_801B2EC0;
extern s32 D_801B2EC4;
extern void (*D_800D6F6C[])(void);
extern u8 D_800D944C[];
extern u8 D_801399E0[];
extern s32 D_80182D98;

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
extern u32 D_801B2EC0;
extern s32 D_801B2EC4;
extern void (*D_800D6F6C[])(void);
extern u8 D_800D944C[];
extern u8 D_801399E0[];
extern s32 D_80182D98;

    D_801B2EC0 = 1;
    D_801B2EC4 = 1;
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800AD1A8(void)
{
extern u32 D_801B2EC0;
extern s32 D_801B2EC4;
extern void (*D_800D6F6C[])(void);
extern u8 D_800D944C[];
extern u8 D_801399E0[];
extern s32 D_80182D98;

    func_8006CC4C(D_800D944C, D_801399E0);
    func_80066F9C(D_800D944C, D_80182D98, 0x26, 0x2, 0);
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
extern s32 D_801B2EC0;

    D_801B2EC0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800AD23C(s32 arg0)
{
extern u32 D_801B2EC8;
extern s32 D_801B2ECC;
extern void (*D_800D6F7C[])(void);
extern u8 D_800D9478[];
extern u8 D_801399E8[];
extern s32 D_80182DB8;

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
extern u32 D_801B2EC8;
extern s32 D_801B2ECC;
extern void (*D_800D6F7C[])(void);
extern u8 D_800D9478[];
extern u8 D_801399E8[];
extern s32 D_80182DB8;

    D_801B2EC8 = 1;
    D_801B2ECC = 1;
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800AD2CC(void)
{
extern u32 D_801B2EC8;
extern s32 D_801B2ECC;
extern void (*D_800D6F7C[])(void);
extern u8 D_800D9478[];
extern u8 D_801399E8[];
extern s32 D_80182DB8;

    func_8006CC4C(D_800D9478, D_801399E8);
    func_80066F9C(D_800D9478, D_80182DB8, 0x27, 0x2, 0);
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
extern s32 D_801B2EC8;

    D_801B2EC8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800AD360(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2ED0;
extern s32 D_801B2ED4;
extern void (*D_800D6F8C[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_800AB444(void);

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
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2ED0;
extern s32 D_801B2ED4;
extern void (*D_800D6F8C[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_800AB444(void);

    D_801B2ED0 = 1;
    D_801B2ED4 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_800AD3F0(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2ED0;
extern s32 D_801B2ED4;
extern void (*D_800D6F8C[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_800AB444(void);

    D_801B24A0 = D_80139258;
    D_801B2650 = D_80182DC0;
    D_80182DE8 = 0x80;
    D_801B2650.w[2] = 0xAFC8;
    D_801B2ED4 = 0x20;
    D_801B2ED0 += 1;
    func_800AB444();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800AD4A0(void)
{
extern s32 D_801B2ED0;

    D_801B2ED0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800AD4B8(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2ED8;
extern s32 D_801B2EDC;
extern void (*D_800D6F9C[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern void func_800AB55C(void);

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
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2ED8;
extern s32 D_801B2EDC;
extern void (*D_800D6F9C[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern void func_800AB55C(void);

    D_801B2ED8 = 1;
    D_801B2EDC = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_800AD548(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2ED8;
extern s32 D_801B2EDC;
extern void (*D_800D6F9C[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern void func_800AB55C(void);

    D_801B24A8 = D_80139258;
    D_801B2478 = D_80182DC0;
    D_80182DEC = 0x80;
    D_801B2478.w[2] = 0xAFC8;
    D_801B2EDC = 0x10;
    D_801B2ED8 += 1;
    func_800AB55C();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800AD5F8(void)
{
extern s32 D_801B2ED8;

    D_801B2ED8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800AD610(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2EE0;
extern s32 D_801B2EE4;
extern void (*D_800D6FAC[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B238;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_80139870;
extern s32 D_80182DF0;
extern void func_800AB674(void);

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
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2EE0;
extern s32 D_801B2EE4;
extern void (*D_800D6FAC[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B238;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_80139870;
extern s32 D_80182DF0;
extern void func_800AB674(void);

    D_801B2EE0 = 1;
    D_801B2EE4 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_800AD6A0(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2EE0;
extern s32 D_801B2EE4;
extern void (*D_800D6FAC[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B238;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_80139870;
extern s32 D_80182DF0;
extern void func_800AB674(void);

    D_8013B238 = D_80139258;
    D_80139870 = D_80182DC0;
    D_80182DF0 = 0x80;
    D_80139870.w[2] = 0xAFC8;
    D_801B2EE4 = 0x10;
    D_801B2EE0 += 1;
    func_800AB674();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800AD750(void)
{
extern s32 D_801B2EE0;

    D_801B2EE0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800AD768(s32 arg0)
{
extern u32 D_801B2EE8;
extern s32 D_801B2EEC;
extern void (*D_800D6FBC[])(void);

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
extern u32 D_801B2EE8;
extern s32 D_801B2EEC;
extern void (*D_800D6FBC[])(void);

    D_801B2EE8 = 1;
    D_801B2EEC = 1;
}

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void func_800AD7F8(void)
{
extern s32 D_801B25D8;
extern s32 D_801B2EE8;
extern s32 D_801B2EEC;

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
extern void func_800AD8EC(void);
extern s32 D_801B2EEC;
extern s32 D_801B2EE8;

    D_801B2EEC = 0x58;
    D_801B2EE8 += 1;
    func_800AD8EC();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void func_800AD8EC(void)
{
extern s32 D_801B2EEC;
extern s32 D_801B2EE8;

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
extern void func_800AD9DC(void);
extern s32 D_800DCEAC;
extern s32 D_801B2EEC;
extern s32 D_801B2EE8;

    D_800DCEAC = 0;
    D_801B2EEC = 0x64;
    D_801B2EE8 += 1;
    func_800AD9DC();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void func_800AD9DC(void)
{
extern s32 D_801B2EEC;
extern s32 D_801B2EE8;

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
extern s32 D_801B2EE8;

    D_801B2EE8 += 1;
}
