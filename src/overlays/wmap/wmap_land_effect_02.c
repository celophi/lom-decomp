#include "wmap_land_effect_02.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_sprite_render.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_effect_primitives.h"

/**
 * @brief Initialize world-map globals and four per-entry state records.
 */
void func_8007D428(void)
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
    s16 unk0;
    s16 unk2;
    s32 unk4;
    s32 unk8;
    s16 unkC;
    s16 unkE;
    s32 unk10;
} WmapAfcEntry;

typedef struct
{
    s32 unk0;
    void *unk4;
} WmapPair;

extern WmapD94Entry D_800D94D0[];
extern s32 D_8011D538[];
extern s32 D_80139240;
extern s32 D_8013924C;
extern s32 D_80139250;
extern s32 D_80139260;
extern s32 D_80139264;
extern s32 D_80139268;
extern s32 D_8013926C;
extern s32 D_80139284;
extern WmapPair D_80139988[];
extern s32 D_8013B264;
extern s32 D_8013B270;
extern s32 D_8013B278;
extern s32 D_8013B280;
extern s32 D_80182DEC;
extern WmapAfcEntry D_801AFC98[];
extern s32 D_801B0FD0;
extern s32 D_801B2760;
extern s32 D_801B2764;

extern void func_8007E510__for_func_8007D428(void) __asm__("func_8007E510");
extern s32 D_801B2738;
extern s32 D_80139888[];
extern SVECTOR D_8013B240;
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B276C;
extern s32 D_801B2768;

    s32 i;
    s16 shift;
    WmapD94Entry *entry;

    i = 0;
    D_801B0FD0 = 4;
    D_80182DEC = 0x7F;
    D_80139240 = 0x50;
    D_8013924C = 2;
    D_80139250 = -1;
    D_80139260 = 0x7D0;
    D_80139264 = 0xA;
    D_80139268 = 8;
    D_8013926C = 1;
    D_80139284 = 0;
    D_8013B264 = 0x28;
    D_8013B270 = 0x50;
    D_8013B278 = 0x14;
    D_8013B280 = 1;

    do
    {
        entry = &D_800D94D0[i];
        D_80139988[i + 14].unk4 = D_8011D538;
        shift = i << 10;
        entry->unk6 = 0xF;
        entry->unk10 = -1;
        entry->unk26 = 4;
        entry->unk2 = 0;
        entry->unk22 = 0x81;
        entry->unk24 = 0x81;
        entry->unkE = D_8013926C;
        D_801AFC98[i].unk0 = 1;
        D_801AFC98[i].unk8 = D_80139284;
        D_801AFC98[i].unkC = 0x50;
        D_801AFC98[i].unk2 = shift;
        D_801AFC98[i].unk4 = 0;
        D_801AFC98[i].unkE = 0;
        i++;
    } while (i < 4);

    D_801B2764 = 0x3C;
    D_801B2760++;
    func_8007E510__for_func_8007D428();
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_8007D5A4(void)
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
    s16 unk0;
    s16 unk2;
    s32 unk4;
    s32 unk8;
    s16 unkC;
    s16 unkE;
    s32 unk10;
} WmapAfcEntry;

typedef struct
{
    s32 unk0;
    void *unk4;
} WmapPair;

extern WmapD94Entry D_800D94D0[];
extern s32 D_8011D538[];
extern s32 D_80139240;
extern s32 D_8013924C;
extern s32 D_80139250;
extern s32 D_80139260;
extern s32 D_80139264;
extern s32 D_80139268;
extern s32 D_8013926C;
extern s32 D_80139284;
extern WmapPair D_80139988[];
extern s32 D_8013B264;
extern s32 D_8013B270;
extern s32 D_8013B278;
extern s32 D_8013B280;
extern s32 D_80182DEC;
extern WmapAfcEntry D_801AFC98[];
extern s32 D_801B0FD0;
extern s32 D_801B2760;
extern s32 D_801B2764;

extern void func_8007E510__for_func_8007D5A4(void) __asm__("func_8007E510");
extern s32 D_801B2738;
extern s32 D_80139888[];
extern SVECTOR D_8013B240;
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B276C;
extern s32 D_801B2768;

    MATRIX m;
    s32 x;

    x = D_80139888[2] - 0xDAC;
    D_80139888[2] = x;
    if (x < 0x2710)
    {
        D_80139888[2] = 0x2710;
    }

    PushMatrix();
    RotMatrix(&D_8013B240, &m);
    TransMatrix(&m, &D_8011CF60);
    SetRotMatrix(&m);
    SetTransMatrix(&m);

    if (D_80182DE8 != 0)
    {
        func_8006CD98((s32)D_800DCF18, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DE8);
        D_80182DE8 -= 0x2;
        if (D_80182DE8 < 0)
        {
            D_80182DE8 = 0;
        }
    }

    PopMatrix();
    if (--D_801B276C == 0)
    {
        D_801B2768 += 1;
    }
}

/** @brief Initialize four equally spaced actors and start the effect. */
void func_8007D6A4(void)
{
/* Partial WMAP decompilation: 94.976746% (gcc280_g0). */

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

/** @brief Unaligned eight-byte configuration copy. */
typedef struct
{
    u8 bytes[8];
} WmapConfigBytes;
extern WmapConfigA D_800D9268[];
extern WmapMotion D_801AFBD0[];
extern WmapResource D_80139988[];
extern WmapConfigBytes D_80139258;
extern WmapConfigBytes D_801B2490;
extern u8 D_8011D538[];
extern s32 D_80139234;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_801B2770;
extern s32 D_801B2774;
extern void func_8007D7FC__for_func_8007D6A4(void) __asm__("func_8007D7FC");

    s32 i;

    D_801B2490 = D_80139258;
    D_80139234 = 1;
    D_8013923C = 2;
    D_80139240 = 0;
    for (i = 20; i < 80; i++)
    {
        D_801AFBD0[i].state = 0;
    }
    for (i = 20; i < 80; i += 15)
    {
        D_80139988[i].resource = D_8011D538;
        D_800D9268[i].field_06 = 15;
        D_800D9268[i].field_10 = -1;
        D_800D9268[i].field_26 = 2;
        D_800D9268[i].field_02 = 0;
        D_800D9268[i].field_0E = 0;
        D_800D9268[i].field_22 = 1;
        D_800D9268[i].field_24 = 129;
        D_801AFBD0[i].state = 1;
        D_801AFBD0[i].z = 150000;
        D_801AFBD0[i].field_0E = 0;
        D_801AFBD0[i].angle = D_80139240;
        D_80139240 += 1024;
    }
    D_801B2774 = 32;
    D_801B2770++;
    func_8007D7FC__for_func_8007D6A4();
}

void func_8007D7FC(void)
{
/* Partial WMAP decompilation: 67.098595% (gcc280_g0). */

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))


extern u8 D_800D9268;
extern s32 D_80139234;
extern s32 D_8013923C;
extern u8 D_80139988;
extern u8 D_801AFBD0;
extern s32 D_801B2770;
extern s32 D_801B2774;

    SVECTOR position;
    u32 sp20;
    s32 temp_v0_3;
    s32 temp_v0_4;
    s32 var_s2;
    s32 var_s2_2;
    void *temp_v0;
    void *temp_v0_2;
    void *temp_v1;
    void *var_a0;
    void *var_s0;
    void *var_s0_2;
    void *var_s1;
    void *var_s1_2;
    void *var_s3;
    void *var_s3_2;
    void *var_v0;

    var_s2 = 0x14;
    var_s3 = (u8 *)&D_80139988 + 0xA0;
    var_s0 = (u8 *)&D_801AFBD0 + 0x190;
    var_s1 = (u8 *)&D_800D9268 + 0x370;
    D_8013923C -= 1;
    do
    {
        position.vx = (s16) ((s32) (((s32) M2C_FIELD(var_s0, s32 *, 8) >> 3) * (ccos(M2C_FIELD(var_s0, s16 *, 2)) >> 6)) >> 0xC);
        position.vy = (s16) ((s32) (((s32) M2C_FIELD(var_s0, s32 *, 8) >> 3) * (csin(M2C_FIELD(var_s0, s16 *, 2)) >> 6)) >> 0xC);
        position.vz = M2C_FIELD(var_s0, u16 *, 0xE);
        gte_ldv0(&position);
                gte_rtps();
        M2C_FIELD(var_s0, s16 *, 2) = (s16) ((u16) M2C_FIELD(var_s0, s16 *, 2) + 0x60);
        M2C_FIELD(var_s0, s32 *, 8) = (s32) (M2C_FIELD(var_s0, s32 *, 8) - 0xBB8);
        gte_stsxy(&sp20);
        M2C_FIELD(var_s0, u16 *, 0x10) = sp20;
        M2C_FIELD(var_s0, u16 *, 0x12) = (u16) M2C_FIELD(&sp20, u16 *, 2);
        if (D_8013923C == 0)
        {
            var_a0 = var_s1;
            var_v0 = ((var_s2 + D_80139234) * 0x2C) + (u8 *)&D_800D9268;
            do
            {
                M2C_FIELD(var_v0, s32 *, 0) = (s32) M2C_FIELD(var_a0, s32 *, 0);
                M2C_FIELD(var_v0, s32 *, 4) = (s32) M2C_FIELD(var_a0, s32 *, 4);
                M2C_FIELD(var_v0, s32 *, 8) = (s32) M2C_FIELD(var_a0, s32 *, 8);
                M2C_FIELD(var_v0, s32 *, 0xC) = (s32) M2C_FIELD(var_a0, s32 *, 0xC);
                var_a0 += 0x10;
                var_v0 += 0x10;
            } while (var_a0 != (var_s1 + 0x20));
            M2C_FIELD(var_v0, s32 *, 0) = (s32) M2C_FIELD(var_a0, s32 *, 0);
            M2C_FIELD(var_v0, s32 *, 4) = (s32) M2C_FIELD(var_a0, s32 *, 4);
            M2C_FIELD(var_v0, s32 *, 8) = (s32) M2C_FIELD(var_a0, s32 *, 8);
            temp_v0 = ((var_s2 + D_80139234) * 0x14) + (u8 *)&D_801AFBD0;
            M2C_FIELD(temp_v0, s32 *, 0) = (s32) M2C_FIELD(var_s0, s32 *, 0);
            M2C_FIELD(temp_v0, s32 *, 4) = (s32) M2C_FIELD(var_s0, s32 *, 4);
            M2C_FIELD(temp_v0, s32 *, 8) = (s32) M2C_FIELD(var_s0, s32 *, 8);
            M2C_FIELD(temp_v0, s32 *, 0xC) = (s32) M2C_FIELD(var_s0, s32 *, 0xC);
            M2C_FIELD(temp_v0, s32 *, 0x10) = (s32) M2C_FIELD(var_s0, s32 *, 0x10);
            temp_v0_2 = ((var_s2 + D_80139234) * 0x2C) + (u8 *)&D_800D9268;
            temp_v1 = ((var_s2 + D_80139234) * 8) + (u8 *)&D_80139988;
            M2C_FIELD(temp_v1, s32 *, 0) = (s32) M2C_FIELD(var_s3, s32 *, 0);
            M2C_FIELD(temp_v1, s32 *, 4) = (s32) M2C_FIELD(var_s3, s32 *, 4);
            M2C_FIELD(temp_v0_2, s16 *, 0x26) = 8;
            M2C_FIELD(temp_v0_2, s16 *, 0x22) = 1;
        }
        var_s3 += 0x78;
        var_s0 += 0x12C;
        var_s2 += 0xF;
        var_s1 += 0x294;
    } while (var_s2 < 0x50);
    var_s2_2 = 0x14;
    if (D_8013923C == 0)
    {
        D_8013923C = 2;
        temp_v0_3 = D_80139234 + 1;
        D_80139234 = temp_v0_3;
        if (temp_v0_3 >= 0xF)
        {
            D_80139234 = 1;
            var_s2_2 = 0x14;
        }
    }
    var_s1_2 = (u8 *)&D_801AFBD0 + 0x190;
    var_s3_2 = (u8 *)&D_80139988 + 0xA0;
    var_s0_2 = (u8 *)&D_800D9268 + 0x370;
    do
    {
        if (M2C_FIELD(var_s1_2, s16 *, 0) != 0)
        {
            func_8006CC4C(var_s0_2, var_s3_2);
            func_80066F9C(var_s0_2, M2C_FIELD(var_s1_2, s32 *, 0x10), 8, 8, 0);
            if (M2C_FIELD(var_s0_2, s16 *, 0x24) < 5)
            {
                M2C_FIELD(var_s1_2, s16 *, 0) = 0;
            }
        }
        var_s1_2 += 0x14;
        var_s3_2 += 8;
        var_s2_2 += 1;
        var_s0_2 += 0x2C;
    } while (var_s2_2 < 0x50);
    temp_v0_4 = D_801B2774 - 1;
    D_801B2774 = temp_v0_4;
    if (temp_v0_4 == 0)
    {
        D_801B2770 += 1;
    }
}
#undef M2C_FIELD
#undef M2C_UNALIGNED32
#undef M2C_BITWISE

/**
 * @brief Initialize a range of world-map per-entry records and schedule the next step.
 */
void func_8007DB50(void)
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
extern s32 D_800D9150;
extern s32 D_800DCEA8;
extern s32 D_801B25D8;
extern s32 D_801B2778;
extern s32 D_801B277C;

extern void func_8007E9C4__for_func_8007DB50(void) __asm__("func_8007E9C4");

    s32 i;
    WmapD94Entry *entry;

    i = 100;
    D_801B25D8 = 1;
    D_800DCEA8 = 1;

    do
    {
        D_801AFBD0[i].unk0 = 0;
        D_80139988[i].unk4 = D_8011D538;
        entry = &D_800D9268[i];
        entry->unk2 = 0;
        entry->unk6 = 0xF;
        entry->unkE = 2;
        entry->unk10 = -1;
        i++;
    } while (i < 200);

    D_800D9150 = 2;
    D_801B277C = 0x10;
    D_801B2778 += 1;
    func_8007E9C4__for_func_8007DB50();
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8007DC14(s32 arg0)
{
extern u32 D_801B2740;
extern s32 D_801B2744;
extern void (*D_800D54F0[])(void);
extern void func_8007DCE8__for_func_8007DC14(void) __asm__("func_8007DCE8");
extern s32 D_8013B20C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2740 = 1;
        D_801B2744 = 1;
        return 1;
    }

    if (D_801B2740 < 0x6)
    {
        D_800D54F0[D_801B2740]();
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
void func_8007DC8C(void)
{
extern u32 D_801B2740;
extern s32 D_801B2744;
extern void (*D_800D54F0[])(void);
extern void func_8007DCE8__for_func_8007DC8C(void) __asm__("func_8007DCE8");
extern s32 D_8013B20C;

    D_801B2740 = 1;
    D_801B2744 = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_8007DCA4(void)
{
extern u32 D_801B2740;
extern s32 D_801B2744;
extern void (*D_800D54F0[])(void);
extern void func_8007DCE8__for_func_8007DCA4(void) __asm__("func_8007DCE8");
extern s32 D_8013B20C;

    func_8006CAC0(func_8006C81C);
    D_8013B20C = 1;
    D_801B2740 += 1;
    func_8007DCE8__for_func_8007DCA4();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8007DCE8(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2740;
extern void func_8007DD24__for_func_8007DCE8(void) __asm__("func_8007DD24");
extern void func_8007DDBC__for_func_8007DCE8(void) __asm__("func_8007DDBC");
extern void func_8007DD68__for_func_8007DCE8(void) __asm__("func_8007DD68");

    if (D_8013B20C == 0)
    {
        D_801B2740 += 1;
        func_8007DD24__for_func_8007DCE8();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_8007DD24(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2740;
extern void func_8007DD24(void);
extern void func_8007DDBC__for_func_8007DD24(void) __asm__("func_8007DDBC");
extern void func_8007DD68__for_func_8007DD24(void) __asm__("func_8007DD68");

    func_8006CAC0(func_8007DDBC__for_func_8007DD24);
    D_8013B20C = 1;
    D_801B2740 += 1;
    func_8007DD68__for_func_8007DD24();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8007DD68(void)
{
extern s32 D_801B2740;
extern s32 D_8013B20C;
extern void func_8007DDA4__for_func_8007DD68(void) __asm__("func_8007DDA4");

    if (D_8013B20C == 0)
    {
        D_801B2740 += 1;
        func_8007DDA4__for_func_8007DD68();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007DDA4(void)
{
extern s32 D_801B2740;
extern s32 D_8013B20C;
extern void func_8007DDA4(void);

    D_801B2740 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8007DDBC(s32 arg0)
{
extern u32 D_801B2748;
extern s32 D_801B274C;
extern void (*D_800D5508[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2748 = 1;
        D_801B274C = 1;
        return 1;
    }

    if (D_801B2748 < 0xC)
    {
        D_800D5508[D_801B2748]();
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
void func_8007DE34(void)
{
extern u32 D_801B2748;
extern s32 D_801B274C;
extern void (*D_800D5508[])(void);

    D_801B2748 = 1;
    D_801B274C = 1;
}

/**
 * @brief Set up a tinted world-map sub-scene and register its handler.
 */
void func_8007DE4C(void)
{
extern s32 D_8013B208;
extern s32 D_801ADAF4;
extern s32 D_801B2748;
extern s32 D_801B274C;
extern void func_8007E2DC__for_func_8007DE4C(void) __asm__("func_8007E2DC");

    D_8013B208 = 1;
    func_8006683C(0x102045);
    D_801ADAF4 = 4;
    func_800652A8(0x1C, 0x80);
    func_8006CAC0(func_8007E2DC__for_func_8007DE4C);
    D_801B274C = 0x18;
    D_801B2748 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007DEB8(void)
{
extern s32 D_801B274C;
extern s32 D_801B2748;
extern void func_8007E480__for_func_8007DEB8(void) __asm__("func_8007E480");

    if (--D_801B274C == 0)
    {
        D_801B2748 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8007DEEC(void)
{
extern s32 D_801B274C;
extern s32 D_801B2748;
extern void func_8007E480__for_func_8007DEEC(void) __asm__("func_8007E480");

    func_8006CAC0(func_8007E480__for_func_8007DEEC);
    D_801B274C = 0x50;
    D_801B2748 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007DF28(void)
{
extern s32 D_801B274C;
extern s32 D_801B2748;
extern void func_8007E88C__for_func_8007DF28(void) __asm__("func_8007E88C");

    if (--D_801B274C == 0)
    {
        D_801B2748 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8007DF5C(void)
{
extern s32 D_801B274C;
extern s32 D_801B2748;
extern void func_8007E88C__for_func_8007DF5C(void) __asm__("func_8007E88C");

    func_8006CAC0(func_8007E88C__for_func_8007DF5C);
    D_801B274C = 0x1E;
    D_801B2748 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007DF98(void)
{
extern s32 D_801B274C;
extern s32 D_801B2748;

    if (--D_801B274C == 0)
    {
        D_801B2748 += 1;
    }
}

/** @brief Register a callback, set world-map state and color, and begin a two-tick delay. */
void func_8007DFCC(void)
{
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern s32 D_801B2748;
extern s32 D_801B274C;
extern void func_8007E734__for_func_8007DFCC(void) __asm__("func_8007E734");

    func_8006CAC0(&func_8007E734__for_func_8007DFCC);
    D_801ADAE0 = 1;
    D_801ADAF4 = 8;
    func_8006683C(0x203050);
    D_801B274C = 2;
    D_801B2748 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007E02C(void)
{
extern s32 D_801B274C;
extern s32 D_801B2748;
extern void func_8007E934__for_func_8007E02C(void) __asm__("func_8007E934");
extern void func_8007E140__for_func_8007E02C(void) __asm__("func_8007E140");

    if (--D_801B274C == 0)
    {
        D_801B2748 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_8007E060(void)
{
extern s32 D_801B274C;
extern s32 D_801B2748;
extern void func_8007E934__for_func_8007E060(void) __asm__("func_8007E934");
extern void func_8007E140__for_func_8007E060(void) __asm__("func_8007E140");

    func_8006CAC0(func_8007E934__for_func_8007E060);
    func_8006CAC0(func_8007E140__for_func_8007E060);
    D_801B274C = 0x7F;
    D_801B2748 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007E0A8(void)
{
extern s32 D_801B274C;
extern s32 D_801B2748;

    if (--D_801B274C == 0)
    {
        D_801B2748 += 1;
    }
}

void func_8007E0DC(void)
{
/* Partial WMAP decompilation: 95.600000% (gcc280_g0). */

extern s32 D_8013B20C;
extern u32 D_80139290[];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 D_801B2748;

    D_8013B20C = 0;
    D_80139290[D_8011D530 * 10 + D_8011D510 * 60] = D_8011D4FC | 0x100;
    D_801B2748 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8007E140(s32 arg0)
{
extern u32 D_801B2750;
extern s32 D_801B2754;
extern void (*D_800D5538[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2750 = 1;
        D_801B2754 = 1;
        return 1;
    }

    if (D_801B2750 < 0x4)
    {
        D_800D5538[D_801B2750]();
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
void func_8007E1B8(void)
{
extern u32 D_801B2750;
extern s32 D_801B2754;
extern void (*D_800D5538[])(void);

    D_801B2750 = 1;
    D_801B2754 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its next step.
 */
void func_8007E1D0(void)
{
extern u8* D_801399AC;
extern u8 D_8011F538[];
extern u8 D_800D9318[];
extern s32 D_801B2750;
extern s32 D_801B2754;
extern void func_8007E248__for_func_8007E1D0(void) __asm__("func_8007E248");

    D_801399AC = D_8011F538;
    D_800D9318[0x6] = 0xF;
    *(s16*)&D_800D9318[0x10] = -1;
    *(s16*)&D_800D9318[0x2] = 0;
    *(s16*)&D_800D9318[0xE] = 0;
    *(s16*)&D_800D9318[0x26] = 0;
    *(s16*)&D_800D9318[0x22] = 0x80;
    *(s16*)&D_800D9318[0x24] = 0x80;
    D_801B2754 = 0x82;
    D_801B2750 += 1;
    func_8007E248__for_func_8007E1D0();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8007E248(void)
{
extern s32 D_801B2750;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;
extern s32 D_801B2754;

    func_8006CC4C(D_800D9318, D_801399A8);
    func_80066F9C(D_800D9318, D_8011CF4C, 0xF, 0x1E, 0);
    if (--D_801B2754 == 0)
    {
        D_801B2750 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007E2C4(void)
{
extern s32 D_801B2750;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;
extern s32 D_801B2754;

    D_801B2750 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8007E2DC(s32 arg0)
{
extern u32 D_801B2758;
extern s32 D_801B275C;
extern void (*D_800D5548[])(void);
extern void func_8007E3EC__for_func_8007E2DC(void) __asm__("func_8007E3EC");
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2758 = 1;
        D_801B275C = 1;
        return 1;
    }

    if (D_801B2758 < 0x4)
    {
        D_800D5548[D_801B2758]();
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
void func_8007E354(void)
{
extern u32 D_801B2758;
extern s32 D_801B275C;
extern void (*D_800D5548[])(void);
extern void func_8007E3EC__for_func_8007E354(void) __asm__("func_8007E3EC");
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    D_801B2758 = 1;
    D_801B275C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_8007E36C(void)
{
extern u32 D_801B2758;
extern s32 D_801B275C;
extern void (*D_800D5548[])(void);
extern void func_8007E3EC__for_func_8007E36C(void) __asm__("func_8007E3EC");
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    D_801399B4 = D_8011D538;
    D_800D9344[0x6] = 0xF;
    *(s16*)&D_800D9344[0x10] = -1;
    *(s16*)&D_800D9344[0x26] = 4;
    *(s16*)&D_800D9344[0x22] = 0x81;
    *(s16*)&D_800D9344[0x2] = 0;
    *(s16*)&D_800D9344[0xE] = 0;
    *(s16*)&D_800D9344[0x24] = 1;
    D_801B275C = 0x8E;
    D_801B2758 += 1;
    func_8007E3EC__for_func_8007E36C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8007E3EC(void)
{
extern u32 D_801B2758;
extern s32 D_801B275C;
extern void (*D_800D5548[])(void);
extern void func_8007E3EC(void);
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D9344, D_801399B0);
    func_80066F9C(D_800D9344, D_8011CF4C, 0x8, 0x1, 0);
    if (--D_801B275C == 0)
    {
        D_801B2758 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007E468(void)
{
extern s32 D_801B2758;

    D_801B2758 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8007E480(s32 arg0)
{
extern u32 D_801B2760;
extern s32 D_801B2764;
extern void (*D_800D5558[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2760 = 1;
        D_801B2764 = 1;
        return 1;
    }

    if (D_801B2760 < 0x8)
    {
        D_800D5558[D_801B2760]();
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
void func_8007E4F8(void)
{
extern u32 D_801B2760;
extern s32 D_801B2764;
extern void (*D_800D5558[])(void);

    D_801B2760 = 1;
    D_801B2764 = 1;
}

/**
 * @brief Draw a world-map element, then advance the step when its wait expires.
 */
void func_8007E510(void)
{
extern u8 D_800D94D0[];
extern u8 D_801399F8[];
extern s32 D_80182DEC;
extern s32 D_801B2760;
extern s32 D_801B2764;

    func_8006D014(D_800D94D0, D_801399F8, 4, 1, D_80182DEC, 8, 2);
    if (--D_801B2764 == 0)
    {
        D_801B2760 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007E58C(void)
{
extern void func_8007E5C4__for_func_8007E58C(void) __asm__("func_8007E5C4");
extern s32 D_801B2764;
extern s32 D_801B2760;

    D_801B2764 = 0x14;
    D_801B2760 += 1;
    func_8007E5C4__for_func_8007E58C();
}

/**
 * @brief Draw a world-map element, then advance the step when its wait expires.
 */
void func_8007E5C4(void)
{
extern u8 D_800D94D0[];
extern u8 D_801399F8[];
extern s32 D_80182DEC;
extern s32 D_801B2760;
extern s32 D_801B2764;

    func_8006D014(D_800D94D0, D_801399F8, 4, 1, D_80182DEC, 8, 2);
    if (--D_801B2764 == 0)
    {
        D_801B2760 += 1;
    }
}

/** @brief World-map step handler: clear a small entry table, set the timer, advance the step. */
void func_8007E640(void)
{
/* Partial WMAP decompilation: 87.208336% (gcc280_g0). */

typedef struct
{
    u8 pad0[0x22];
    s16 f22;
    u8 pad1[0x2];
    s16 f26;
    u8 pad2[0x4];
} WmapEntry;

extern u8 D_800D94D0;
extern s32 D_801B2760;
extern s32 D_801B2764;
extern void func_8007E6A0__for_func_8007E640(void) __asm__("func_8007E6A0");

    WmapEntry *p;
    s32 i;

    i = 0;
    p = (WmapEntry *)&D_800D94D0;
    do
    {
        p->f22 = 0;
        p->f26 = 8;
        i += 1;
        p += 1;
    } while (i < 4);

    D_801B2764 = 0x10;
    D_801B2760 += 1;
    func_8007E6A0__for_func_8007E640();
}

/**
 * @brief Draw a world-map element, then advance the step when its wait expires.
 */
void func_8007E6A0(void)
{
extern u8 D_800D94D0[];
extern u8 D_801399F8[];
extern s32 D_80182DEC;
extern s32 D_801B2760;
extern s32 D_801B2764;

    func_8006D014(D_800D94D0, D_801399F8, 4, 1, D_80182DEC, 8, 2);
    if (--D_801B2764 == 0)
    {
        D_801B2760 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007E71C(void)
{
extern s32 D_801B2760;

    D_801B2760 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8007E734(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2768;
extern s32 D_801B276C;
extern void (*D_800D5578[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B240;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_80139888;
extern s32 D_80182DE8;
extern void func_8007D5A4__for_func_8007E734(void) __asm__("func_8007D5A4");

    s32 result;

    if (arg0 != 0)
    {
        D_801B2768 = 1;
        D_801B276C = 1;
        return 1;
    }

    if (D_801B2768 < 0x4)
    {
        D_800D5578[D_801B2768]();
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
void func_8007E7AC(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2768;
extern s32 D_801B276C;
extern void (*D_800D5578[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B240;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_80139888;
extern s32 D_80182DE8;
extern void func_8007D5A4__for_func_8007E7AC(void) __asm__("func_8007D5A4");

    D_801B2768 = 1;
    D_801B276C = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_8007E7C4(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2768;
extern s32 D_801B276C;
extern void (*D_800D5578[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B240;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_80139888;
extern s32 D_80182DE8;
extern void func_8007D5A4__for_func_8007E7C4(void) __asm__("func_8007D5A4");

    D_8013B240 = D_80139258;
    D_80139888 = D_80182DC0;
    D_80182DE8 = 0x80;
    D_80139888.w[2] = 0xAFC8;
    D_801B276C = 0x40;
    D_801B2768 += 1;
    func_8007D5A4__for_func_8007E7C4();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007E874(void)
{
extern s32 D_801B2768;

    D_801B2768 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8007E88C(s32 arg0)
{
extern u32 D_801B2770;
extern s32 D_801B2774;
extern void (*D_800D5588[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2770 = 1;
        D_801B2774 = 1;
        return 1;
    }

    if (D_801B2770 < 0x4)
    {
        D_800D5588[D_801B2770]();
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
void func_8007E904(void)
{
extern u32 D_801B2770;
extern s32 D_801B2774;
extern void (*D_800D5588[])(void);

    D_801B2770 = 1;
    D_801B2774 = 1;
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007E91C(void)
{
extern u32 D_801B2770;
extern s32 D_801B2774;
extern void (*D_800D5588[])(void);

    D_801B2770 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8007E934(s32 arg0)
{
extern u32 D_801B2778;
extern s32 D_801B277C;
extern void (*D_800D5598[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2778 = 1;
        D_801B277C = 1;
        return 1;
    }

    if (D_801B2778 < 0x8)
    {
        D_800D5598[D_801B2778]();
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
void func_8007E9AC(void)
{
extern u32 D_801B2778;
extern s32 D_801B277C;
extern void (*D_800D5598[])(void);

    D_801B2778 = 1;
    D_801B277C = 1;
}

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void func_8007E9C4(void)
{
extern s32 D_801B25D8;
extern s32 D_801B2778;
extern s32 D_801B277C;

    s32 remaining;

    func_8006B328(0x64, 0xC8, 2, -1, 5, 5, 0x15E, 8, -0xC8, 0x190, -0xFA, 0x1F4, 0x32, 0x81, 0x81, 8, 0);
    D_801B25D8 += 8;
    remaining = D_801B277C - 1;
    D_801B277C = remaining;
    if (remaining == 0)
    {
        D_801B2778 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007EA80(void)
{
extern void func_8007EAB8__for_func_8007EA80(void) __asm__("func_8007EAB8");
extern s32 D_801B277C;
extern s32 D_801B2778;

    D_801B277C = 0x40;
    D_801B2778 += 1;
    func_8007EAB8__for_func_8007EA80();
}

/** @brief World-map step: spawn an effect object then count down a timer. */
void func_8007EAB8(void)
{
extern s32 D_801B2778;
extern s32 D_801B277C;

    func_8006B328(0x64, 0xC8, 2, -1, 5, 5, 0x15E, 8, -0xC8, 0x190, -0xFA,
                  0x1F4, 0x32, 0x81, 0x81, 8, 0);
    if (--D_801B277C == 0)
    {
        D_801B2778 += 1;
    }
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007EB68(void)
{
extern void func_8007EBA8__for_func_8007EB68(void) __asm__("func_8007EBA8");
extern s32 D_800DCEA8;
extern s32 D_801B277C;
extern s32 D_801B2778;

    D_800DCEA8 = 0;
    D_801B277C = 0x40;
    D_801B2778 += 1;
    func_8007EBA8__for_func_8007EB68();
}

/** @brief World-map step: spawn an effect object then count down a timer. */
void func_8007EBA8(void)
{
extern s32 D_801B2778;
extern s32 D_801B277C;

    func_8006B328(0x64, 0xC8, 2, -1, 5, 5, 0x15E, 8, -0xC8, 0x190, -0xFA,
                  0x1F4, 0x32, 0x81, 0x81, 8, 0);
    if (--D_801B277C == 0)
    {
        D_801B2778 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007EC58(void)
{
extern s32 D_801B2778;

    D_801B2778 += 1;
}
