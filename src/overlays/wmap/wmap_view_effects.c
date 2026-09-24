#include "wmap_frame_render.h"
#include "wmap_main.h"
#include "wmap_sprite_render.h"
#include "wmap_view_effects.h"
#include "common.h"
#include "sdk/libgpu.h"
#include "wmap_resource_support.h"
#include "wmap_effect_primitives.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
#include "wmap_map_labels.h"
#include "wmap_sequence_runtime.h"
#include "wmap_effect_backdrop.h"

#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8*)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;

/** @brief Packed color and GPU command byte. */
typedef union
{
    u32 packed;
    struct
    {
        u8 r, g, b, code;
    } channels;
} WmapColor;

/** @brief Byte-aligned source vertex record. */
typedef struct
{
    u8 bytes[8];
} WmapVertex;
/** @brief Textured quad with packed coordinate words. */
typedef struct
{
    u32 tag, color;
    s32 xy0;
    u32 uv0;
    s32 xy1;
    u32 uv1;
    s32 xy2;
    u32 uv2;
    s32 xy3;
    u32 uv3;
} WmapQuad;

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
    void* resource;
} WmapResource;

/** @brief Pointer slot in the world-map display table. */
typedef struct
{
    s32 unknown_0;
    u8* data;
} WmapPointerSlot;

/** @brief Screen-space position for a map scale and layout. */
typedef struct
{
    s16 x, y;
} WmapPoint;

/** @brief Map translation and projection scale. */
typedef struct
{
    s32 x, y, scale, pad;
} WmapTransform;

extern u32 D_801AFBA0;
extern s32 D_801AFBA4;
extern void (*D_800D0458[])(void);
extern s32 D_80182D88;


extern s32 D_801ADAE8;
extern CVECTOR D_8011D50C;
extern WmapColor D_80129548;
extern s32 D_801398B0;
extern u8 D_8013B24C;
extern u8 D_80054934;
extern s32 D_800DCEEC;
extern s32 D_800DCEF0;
extern s32 D_800DCF04;
extern s32 D_8011CF18;
extern s32 D_8011CF44;
extern s32 D_8011CF7C;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapTransform D_80139950;
extern s32 D_80139954;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern WmapVertex D_80051D4C[];
extern s16 D_80053414[];
extern s16 D_800D928A;
extern s32 D_800DBE70;
extern s32 D_800DBE78;
extern u8 D_800DCEC8;
extern s32 D_8011D4FC;
extern u8 D_801AFBA8;
extern u8 D_801AFBB8;
extern s32 D_80139288;
extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D9370[];
extern WmapMotion D_801AFBD0[];
extern WmapResource D_801399B8[];
extern s32 D_801AFBC8;
extern s32 D_801B0FD0;
extern u32 rand(void);
extern u8 D_8011D538[];
extern u8 D_80139988[];
extern WmapPoint D_80054944[];
extern SVECTOR D_80139278;
extern VECTOR D_80182DC0;
extern s32 D_8013B20C;
extern s32 D_80182234;
extern s32 D_8018223C;
extern s32 D_801B1098;
extern s32 D_801B109C;
extern s32 D_8011D510;
extern s32 D_8011D530;
extern s32 D_8011CF4C;

static s32 func_80065620(s32 initialize);
void func_80065E20(void);
void func_80065F54(void);
void func_800660BC(void);
void func_800667E8(s32 value);

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param initialize Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_8006544C(s32 initialize)
{
    s32 result;

    if (initialize != 0)
    {
        D_801AFBA0 = 1;
        D_801AFBA4 = 1;
    }

    if (D_801AFBA0 < 0x2)
    {
        D_800D0458[D_801AFBA0]();
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
void func_800654BC(void)
{
    D_801AFBA0 = 1;
    D_801AFBA4 = 1;
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800654D4(void)
{
    D_801AFBA0 += 1;
}

/**
 * @brief Clear the world-map state value at D_80182D88.
 */
void func_800654EC(void)
{
    D_80182D88 = 0;
}

/** @brief Initialize both map polygon buffers and update the fade packet colors. */
void func_800654F8(void)
{
    s32 buffer_index;
    s32 row;
    s32 column;
    WmapFrame* buffer;
    POLY_FT4* tile;
    POLY_F4* fade;

    for (buffer_index = 0; buffer_index < 2; buffer_index++)
    {
        buffer = &g_wmap_frames[buffer_index];
        *(s16*)(buffer->tail + 0x16) = 0x45;
        g_wmap_current_frame = buffer;
        *(s32*)(buffer->tail + 0x18) = 0;
        *(s32*)(buffer->tail + 0x10) = 0;
        *(s32*)(buffer->tail + 8) = 0;
        *(s32*)(buffer->tail + 4) = 0;
        buffer->tail[3] = 7;
        g_wmap_current_frame->tail[7] = 0x24;
        for (row = 0; row < 26; row++)
        {
            for (column = 0; column < 26; column++)
            {
                tile = &g_wmap_current_frame->tiles.flat[row * 26 + column];
                *(s32*)&tile->r0 = 0;
                tile->tpage = 9;
                tile->clut = 0x6024;
                ((u8*)tile)[3] = 9;
                tile->code = 0x2C;
            }
        }
        if (D_801ADAE8 != 0)
        {
            if (D_801ADAE8 < 64)
            {
                D_801ADAE8++;
            }
            for (column = 0; column < 184; column++)
            {
                fade = &g_wmap_current_frame->fade[column];
                fade->r0 = D_801ADAE8;
                fade->g0 = D_801ADAE8;
                fade->b0 = D_801ADAE8;
                ((u8*)fade)[3] = 5;
                fade->code = 0x2A;
            }
        }
    }
}

/** @brief Step the map tint toward its target color.
 * @return Nonzero if a color channel was adjusted.

 * @param initialize Sequence event selector, unused by this callback.
 */
static s32 func_80065620(s32 initialize)
{
    s32 packed_color;
    s32 changed;

    changed = 0;
    if (D_8011D50C.r > D_80129548.channels.r)
    {
        changed = 1;
        D_80129548.channels.r = (u8)(D_80129548.channels.r + D_8013B24C);
    }
    if ((u8)D_8011D50C.r < (u8)D_80129548.channels.r)
    {
        changed = 1;
        D_80129548.channels.r = (u8)(D_80129548.channels.r - D_8013B24C);
    }
    if ((u8)D_80129548.channels.g < (u8)D_8011D50C.g)
    {
        changed = 1;
        D_80129548.channels.g = (u8)(D_80129548.channels.g + D_8013B24C);
    }
    if ((u8)D_8011D50C.g < (u8)D_80129548.channels.g)
    {
        changed = 1;
        D_80129548.channels.g = (u8)(D_80129548.channels.g - D_8013B24C);
    }
    if ((u8)D_80129548.channels.b < (u8)D_8011D50C.b)
    {
        changed = 1;
        D_80129548.channels.b = (u8)(D_80129548.channels.b + D_8013B24C);
    }
    if ((u8)D_8011D50C.b < (u8)D_80129548.channels.b)
    {
        changed = 1;
        D_80129548.channels.b = (u8)(D_80129548.channels.b - D_8013B24C);
    }
    if (D_801398B0 != 0)
    {
        packed_color = D_80129548.packed & 0xFFFFFF;
        if (packed_color == 0x808080)
        {
            *(u32*)&D_8011D50C = packed_color;
            D_80129548.packed = packed_color;
            D_80129548.channels.code = 0x2C;
            D_801398B0 = 0;
        }
        else
        {
            D_80129548.channels.code = 0x2E;
        }
    }
    else
    {
        D_80129548.channels.code = 0x2C;
    }
    func_800667E8(D_80129548.packed);
    return changed;
}

/** @brief Update the map view and submit the grid and fade packets.
 * @param initialize Sequence event selector, unused by this callback.
 * @return Always one.
 */
s32 func_8006579C(s32 initialize)
{
    s32* temp_a0;
    s32 temp_a1;
    s32 temp_a1_2;
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v0_3;
    s32 temp_v0_4;
    s32 temp_v0_5;
    s32 temp_v1;
    s32 temp_v1_2;
    s32 temp_v1_3;
    s32 var_a0;
    s32 var_a0_2;
    s32 var_a2;
    s32 var_a3;
    s32 var_a3_2;
    s32 var_t1;
    s32 var_t3;
    s32 var_v0;
    void* temp_a1_3;

    if (D_8013986C == -1)
    {
        func_8006AEE0();
        return 1;
    }
    if ((D_8013986C == 0) && (D_801398D0 == 0) && (D_8011CF18 == 0))
    {
        if (g_wmap_buttons_repeat & 0x8000)
        {
            temp_a1 = ((D_800DCEEC - 2) * 0x18) + 0x80;
            D_800DCEEC -= 1;
            func_800652A8(3, temp_a1);
            if (D_800DCEEC < 0)
            {
                D_800DCEEC = 0;
                if (M2C_FIELD(&D_80139950, s32*, 0) > 0)
                {
                    D_80182D68 = -0x30;
                    if (D_8011CF7C != 0)
                    {
                        D_801398D0 = 1;
                    }
                }
            }
        }
        if (g_wmap_buttons_repeat & 0x2000)
        {
            temp_a1_2 = (D_800DCEEC * 0x18) + 0x80;
            D_800DCEEC += 1;
            func_800652A8(3, temp_a1_2);
            temp_v1 = *((D_8013986C * 4) + (u8*)&D_80054934);
            if (temp_v1 < D_800DCEEC)
            {
                D_800DCEEC = temp_v1;
                if (M2C_FIELD(&D_80139950, s32*, 0) < 0x90)
                {
                    D_80182D68 = 0x30;
                    if (D_8011CF7C != 0)
                    {
                        D_801398D0 = 1;
                    }
                }
            }
        }
        if (g_wmap_buttons_repeat & 0x1000)
        {
            func_800652A8(3, ((D_800DCEEC - 1) * 0x18) + 0x80);
            temp_v0 = D_800DCEF0 - 1;
            D_800DCEF0 = temp_v0;
            if (temp_v0 < 0)
            {
                D_800DCEF0 = 0;
                if (D_80139954 > 0)
                {
                    D_80182D78 = -0x30;
                    if (D_8011CF7C != 0)
                    {
                        D_801398D0 = 1;
                    }
                }
            }
        }
        if (g_wmap_buttons_repeat & 0x4000)
        {
            func_800652A8(3, ((D_800DCEEC - 1) * 0x18) + 0x80);
            temp_v1_2 = *((D_8013986C * 4) + (u8*)&D_80054934);
            temp_v0_2 = D_800DCEF0 + 1;
            D_800DCEF0 = temp_v0_2;
            if (temp_v1_2 < temp_v0_2)
            {
                D_800DCEF0 = temp_v1_2;
                if (D_80139954 < 0x90)
                {
                    D_80182D78 = 0x30;
                    if (D_8011CF7C != 0)
                    {
                        D_801398D0 = 1;
                    }
                }
            }
        }
    }
    if (D_8013986C == 1)
    {
        if (g_wmap_buttons_repeat & 0x8000)
        {
            temp_v0_3 = D_800DCF04 - 1;
            D_800DCF04 = temp_v0_3;
            if (temp_v0_3 < 0)
            {
                D_800DCF04 = 8;
            }
        }
        if (g_wmap_buttons_repeat & 0x2000)
        {
            D_800DCF04 = (D_800DCF04 + 1) % 9;
        }
    }
    if (D_801398D0 != 0)
    {
        if (D_8013986C == 1)
        {
            D_80182D78 = 0;
            D_801398D0 = 0;
            D_80182D68 = 0;
        }
        else
        {
            g_wmap_buttons_repeat = 0;
            if ((D_80182D78 | D_80182D68) == 0)
            {
                D_801398D0 = 0;
                if (D_8011CF44 == 0)
                {
                    g_wmap_input_locked = 0;
                }
            }
            if (D_80182D78 != 0)
            {
                var_a0 = -4;
                if (D_8011CF7C != 0)
                {
                    g_wmap_input_locked = 1;
                    g_wmap_buttons_held = 0;
                    g_wmap_buttons_repeat = 0;
                    if (D_80182D78 > 0)
                    {
                        var_a0 = 4;
                    }
                    D_80182D78 -= var_a0;
                    temp_v0_4 = M2C_FIELD(&D_80139950, s32*, 4) + var_a0;
                    M2C_FIELD(&D_80139950, s32*, 4) = temp_v0_4;
                    if (D_801398D0 == 1)
                    {
                        if (temp_v0_4 < 0)
                        {
                            M2C_FIELD(&D_80139950, s32*, 4) = 0;
                            D_80182D78 = 0;
                            g_wmap_buttons_repeat = 0;
                            D_801398D0 = 0;
                        }
                        if (M2C_FIELD(&D_80139950, s32*, 4) >= 0x91)
                        {
                            M2C_FIELD(&D_80139950, s32*, 4) = 0x90;
                            D_80182D78 = 0;
                            D_801398D0 = 0;
                            g_wmap_buttons_repeat &= ~0x5000;
                        }
                    }
                }
            }
            if (D_80182D68 != 0)
            {
                var_a0_2 = -4;
                if (D_8011CF7C != 0)
                {
                    g_wmap_input_locked = 1;
                    g_wmap_buttons_held = 0;
                    g_wmap_buttons_repeat = 0;
                    if (D_80182D68 > 0)
                    {
                        var_a0_2 = 4;
                    }
                    D_80182D68 -= var_a0_2;
                    temp_v0_5 = M2C_FIELD(&D_80139950, s32*, 0) + var_a0_2;
                    M2C_FIELD(&D_80139950, s32*, 0) = temp_v0_5;
                    if (D_801398D0 == 1)
                    {
                        if (temp_v0_5 < 0)
                        {
                            M2C_FIELD(&D_80139950, s32*, 0) = 0;
                            D_80182D68 = 0;
                            g_wmap_buttons_repeat = 0;
                            D_801398D0 = 0;
                        }
                        if (M2C_FIELD(&D_80139950, s32*, 0) >= 0x91)
                        {
                            M2C_FIELD(&D_80139950, s32*, 0) = 0x90;
                            D_80182D68 = 0;
                            D_801398D0 = 0;
                            g_wmap_buttons_repeat &= 0xFFFF5FFF;
                        }
                    }
                }
            }
        }
    }
    func_800660BC();
    func_80065E20();
    var_t3 = 1;
    var_t1 = 0x1A;
    do
    {
        var_a3 = 1;
        var_v0 = var_t1 + 1;
    loop_60:
        temp_v1_3 = var_v0 * 0x28;
        temp_a1_3 = temp_v1_3 + ((u8*)g_wmap_current_frame);
        var_a3 += 1;
        M2C_FIELD(temp_a1_3, s32*, 0x340) = (s32)((M2C_FIELD(temp_a1_3, s32*, 0x340) & 0xFF000000) | (M2C_FIELD(((u8*)g_wmap_current_frame), s32*, 0x32C) & 0xFFFFFF));
        M2C_FIELD(((u8*)g_wmap_current_frame), s32*, 0x32C) =
            (s32)((M2C_FIELD(((u8*)g_wmap_current_frame), s32*, 0x32C) & 0xFF000000) | ((s32)(((u8*)g_wmap_current_frame) + (temp_v1_3 + 0x340)) & 0xFFFFFF));
        var_v0 = var_t1 + var_a3;
        if (var_a3 < 0x19)
        {
            goto loop_60;
        }
        var_t3 += 1;
        var_t1 += 0x1A;
    } while (var_t3 < 0x19);
    var_a3_2 = 0;
    var_a2 = 0x6CE0;
    do
    {
        temp_a0 = ((u8*)g_wmap_current_frame) + var_a2;
        var_a3_2 += 1;
        *temp_a0 = (*temp_a0 & 0xFF000000) | (M2C_FIELD(((u8*)g_wmap_current_frame), s32*, 0x32C) & 0xFFFFFF);
        M2C_FIELD(((u8*)g_wmap_current_frame), s32*, 0x32C) = (s32)((M2C_FIELD(((u8*)g_wmap_current_frame), s32*, 0x32C) & 0xFF000000) | ((s32)temp_a0 & 0xFFFFFF));
        var_a2 += 0x18;
    } while (var_a3_2 < 0xB8);
    M2C_FIELD(((u8*)g_wmap_current_frame), s32*, 0x7E20) =
        (s32)((M2C_FIELD(((u8*)g_wmap_current_frame), s32*, 0x7E20) & 0xFF000000) | (M2C_FIELD(((u8*)g_wmap_current_frame), s32*, 0x32C) & 0xFFFFFF));
    M2C_FIELD(((u8*)g_wmap_current_frame), s32*, 0x32C) =
        (s32)((M2C_FIELD(((u8*)g_wmap_current_frame), s32*, 0x32C) & 0xFF000000) | ((s32)(((u8*)g_wmap_current_frame) + 0x7E20) & 0xFFFFFF));
    return 1;
}

/**
 * @brief Project grid vertices into the four adjacent textured quads.
 * @see decomp.me (100%)
 */
void func_80065E20(void)
{
    WmapVertex position;
    s32 screen_position;
    s32 row;
    s32 column;
    s32 vertex_index;
    WmapQuad* top_left;
    WmapQuad* top_right;
    WmapQuad* bottom_left;
    WmapQuad* bottom_right;

    func_8006AEE0();
    for (row = 0; row < 25; row++)
    {
        top_left = (WmapQuad*)&g_wmap_current_frame->tiles.rows[row][0];
        top_right = top_left + 1;
        bottom_left = top_left + 26;
        bottom_right = top_left + 27;
        vertex_index = (row + 1) * 4;
        for (column = 0; column < 25;)
        {
            position = D_80051D4C[D_80053414[vertex_index]];
            gte_ldv0(&position);
            gte_rtps();
            vertex_index = (column + 1) * 104 + (row + 1) * 4;
            column++;
            gte_stsxy(&screen_position);
            bottom_right->xy0 = screen_position;
            bottom_left->xy1 = screen_position;
            top_right->xy2 = screen_position;
            top_left->xy3 = screen_position;
            top_left++;
            top_right++;
            bottom_left++;
            bottom_right++;
        }
    }
    func_80065F54();
}

/**
 * @brief Copy map edge coordinates into the fade polygons with a vertical offset.
 * @see decomp.me (100%)
 */
void func_80065F54(void)
{
    s32 row;
    s32 column;
    POLY_FT4* tile;
    POLY_F4* fade;

    fade = g_wmap_current_frame->fade;
    for (row = 22; row < 25; row++)
    {
        tile = &g_wmap_current_frame->tiles.flat[row * 26 + 1];
        for (column = 1; column < 25; column++)
        {
            fade->x0 = tile->x0;
            fade->y0 = tile->y0 + 10;
            fade->x1 = tile->x1;
            fade->y1 = tile->y1 + 10;
            fade->x2 = tile->x2;
            fade->y2 = tile->y2 + 10;
            fade->x3 = tile->x3;
            fade->y3 = tile->y3 + 10;
            fade++;
            tile++;
        }
    }
    for (column = 23; column < 25; column++)
    {
        tile = &g_wmap_current_frame->tiles.flat[column + 3 * 26];
        for (row = 3; row < 23; row++)
        {
            fade->x0 = tile->x0;
            fade->y0 = tile->y0 + 10;
            fade->x1 = tile->x1;
            fade->y1 = tile->y1 + 10;
            fade->x2 = tile->x2;
            fade->y2 = tile->y2 + 10;
            fade->x3 = tile->x3;
            fade->y3 = tile->y3 + 10;
            fade++;
            tile += 26;
        }
    }
}

/** @brief Update texture coordinates across the map grid. */
void func_800660BC(void)
{
    s32 temp_v0;
    s32 temp_v0_10;
    s32 temp_v0_2;
    s32 temp_v0_5;
    s32 temp_v0_7;
    s32 var_a0;
    s32 var_a0_2;
    s32 var_a1;
    s32 var_a1_2;
    s32 var_a2;
    s32 var_a2_2;
    s32 var_a2_3;
    s32 var_a3;
    s32 var_a3_2;
    s32 var_a3_3;
    s32 var_t0;
    s32 var_t0_2;
    s32 var_t0_3;
    s32 var_t1;
    s32 var_t1_2;
    s32 var_t2;
    s32 var_t3;
    s32 var_t3_2;
    s32 var_t5;
    s32 var_t6;
    s8 temp_v0_11;
    s8 temp_v0_3;
    s8 temp_v0_4;
    s8 temp_v0_6;
    s8 temp_v0_8;
    s8 temp_v0_9;
    s8 temp_v1;
    s32 temp_v1_2;
    s32 temp_v1_3;
    s8 temp_v1_4;
    s32 temp_v1_5;
    s32 temp_v1_6;
    u16 temp_a0_5;
    void* temp_a0;
    void* temp_a0_2;
    void* temp_a0_3;
    void* temp_a0_4;
    void* temp_a1;
    void* temp_a1_2;
    void* temp_a1_3;
    void* temp_a1_4;
    void* temp_a1_5;

    var_t3 = 0;
    var_t0 = 0;
    var_t1 = 0;
    var_t5 = 0x340;
    do
    {
        var_a1 = M2C_FIELD(&D_80139950, s32*, 8);
        temp_a0 = ((s32)g_wmap_current_frame) + var_t5;
        if (var_a1 < 0)
        {
            var_a1 += 0xFFF;
        }
        var_a3 = 0;
        temp_v1 = M2C_FIELD(&D_80139950, u8*, 0) + (var_a1 >> 0xC);
        M2C_FIELD(temp_a0, s8*, 0x1C) = temp_v1;
        M2C_FIELD(temp_a0, s8*, 0xC) = temp_v1;
    loop_4:
        temp_v0 = (var_t1 + var_a3) * 0x28;
        temp_a1 = ((s32)g_wmap_current_frame) + (temp_v0 + 0x340);
        var_a2 = var_a3 * M2C_FIELD(&D_80139950, s32*, 8);
        temp_a0_2 = ((s32)g_wmap_current_frame) + (temp_v0 + 0x368);
        if (var_a2 < 0)
        {
            var_a2 += 0xFFF;
        }
        temp_v0_2 = (var_a2 >> 0xC) + (s32)M2C_FIELD(&D_80139950, u8*, 0);
        temp_v1_2 = temp_v0_2 + 0x30;
        if (var_t3 != 0)
        {
            var_t3 = 0;
            temp_v0_3 = temp_v0_2 + 0x10;
            M2C_FIELD(temp_a1, s8*, 0x24) = temp_v0_3;
            M2C_FIELD(temp_a1, s8*, 0x14) = temp_v0_3;
            M2C_FIELD(temp_a1, s16*, 0x26) = 0x280;
        }
        else
        {
            if (temp_v1_2 < 0x100)
            {
                M2C_FIELD(temp_a1, s16*, 0x26) = 0x240;
            }
            else
            {
                M2C_FIELD(temp_a1, s16*, 0x26) = 0x280;
            }
            M2C_FIELD(temp_a1, s8*, 0x24) = temp_v1_2;
            M2C_FIELD(temp_a1, s8*, 0x14) = temp_v1_2;
        }
        if (temp_v1_2 < 0xF0)
        {
            M2C_FIELD(temp_a0_2, s16*, 0x26) = 0x240;
            goto block_15;
        }
        M2C_FIELD(temp_a0_2, s16*, 0x26) = 0x280;
        if (temp_v1_2 >= 0x100)
        {
        block_15:
            M2C_FIELD(temp_a0_2, s8*, 0x1C) = temp_v1_2;
            M2C_FIELD(temp_a0_2, s8*, 0xC) = temp_v1_2;
        }
        else
        {
            var_t3 = 1;
            temp_v0_4 = temp_v1_2 - 0x20;
            M2C_FIELD(temp_a0_2, s8*, 0x1C) = temp_v0_4;
            M2C_FIELD(temp_a0_2, s8*, 0xC) = temp_v0_4;
        }
        var_a3 += 1;
        if (var_a3 < 0x19)
        {
            goto loop_4;
        }
        var_a0 = var_a3 * M2C_FIELD(&D_80139950, s32*, 8);
        temp_a1_2 = ((s32)g_wmap_current_frame) + (((var_t1 + var_a3) * 0x28) + 0x340);
        if (var_a0 < 0)
        {
            var_a0 += 0xFFF;
        }
        temp_v0_5 = (var_a0 >> 0xC) + (s32)M2C_FIELD(&D_80139950, u8*, 0);
        temp_v1_3 = temp_v0_5 + 0x30;
        if (var_t3 != 0)
        {
            var_t3 = 0;
            temp_v0_6 = temp_v0_5 + 0x10;
            M2C_FIELD(temp_a1_2, s8*, 0x24) = temp_v0_6;
            M2C_FIELD(temp_a1_2, s8*, 0x14) = temp_v0_6;
        }
        else
        {
            if (temp_v1_3 < 0x100)
            {
                M2C_FIELD(temp_a1_2, s16*, 0x26) = 0x240;
            }
            else
            {
                M2C_FIELD(temp_a1_2, s16*, 0x26) = 0x280;
            }
            M2C_FIELD(temp_a1_2, s8*, 0x24) = temp_v1_3;
            M2C_FIELD(temp_a1_2, s8*, 0x14) = temp_v1_3;
        }
        var_t1 += 0x1A;
        var_t0 += 1;
        var_t5 += 0x410;
    } while (var_t0 < 0x19);
    var_t3_2 = 0;
    var_a3_2 = 0;
    var_t6 = 0x340;
    do
    {
        var_a1_2 = M2C_FIELD(&D_80139950, s32*, 8);
        temp_a0_3 = ((s32)g_wmap_current_frame) + var_t6;
        if (var_a1_2 < 0)
        {
            var_a1_2 += 0xFFF;
        }
        var_t0_2 = 0;
        var_t2 = 0x1A;
        var_t1_2 = var_a3_2;
        temp_v1_4 = M2C_FIELD(&D_80139950, u8*, 4) + (var_a1_2 >> 0xC);
        M2C_FIELD(temp_a0_3, s8*, 0x15) = temp_v1_4;
        M2C_FIELD(temp_a0_3, s8*, 0xD) = temp_v1_4;
    loop_31:
        temp_a1_3 = ((s32)g_wmap_current_frame) + ((var_t1_2 * 0x28) + 0x340);
        var_a2_2 = var_t0_2 * M2C_FIELD(&D_80139950, s32*, 8);
        temp_a0_4 = ((s32)g_wmap_current_frame) + (((var_t2 + var_a3_2) * 0x28) + 0x340);
        if (var_a2_2 < 0)
        {
            var_a2_2 += 0xFFF;
        }
        temp_v0_7 = (var_a2_2 >> 0xC) + (s32)M2C_FIELD(&D_80139950, u8*, 4);
        temp_v1_5 = temp_v0_7 + 0x30;
        if (var_t3_2 != 0)
        {
            var_t3_2 = 0;
            temp_v0_8 = temp_v0_7 + 0x10;
            M2C_FIELD(temp_a1_3, s8*, 0x25) = temp_v0_8;
            M2C_FIELD(temp_a1_3, s8*, 0x1D) = temp_v0_8;
            M2C_FIELD(temp_a1_3, s16*, 0x1E) = 0x100;
        }
        else
        {
            if (temp_v1_5 < 0x100)
            {
                M2C_FIELD(temp_a1_3, s16*, 0x1E) = 0;
            }
            else
            {
                M2C_FIELD(temp_a1_3, s16*, 0x1E) = 0x100;
            }
            M2C_FIELD(temp_a1_3, s8*, 0x25) = temp_v1_5;
            M2C_FIELD(temp_a1_3, s8*, 0x1D) = temp_v1_5;
        }
        if (temp_v1_5 < 0xF0)
        {
            M2C_FIELD(temp_a0_4, s16*, 0x1E) = 0x100;
            goto block_42;
        }
        M2C_FIELD(temp_a0_4, s16*, 0x1E) = 0x100;
        if (temp_v1_5 >= 0x100)
        {
        block_42:
            M2C_FIELD(temp_a0_4, s8*, 0x15) = temp_v1_5;
            M2C_FIELD(temp_a0_4, s8*, 0xD) = temp_v1_5;
        }
        else
        {
            var_t3_2 = 1;
            temp_v0_9 = temp_v1_5 - 0x20;
            M2C_FIELD(temp_a0_4, s8*, 0x15) = temp_v0_9;
            M2C_FIELD(temp_a0_4, s8*, 0xD) = temp_v0_9;
        }
        var_t2 += 0x1A;
        var_t0_2 += 1;
        var_t1_2 += 0x1A;
        if (var_t0_2 < 0x19)
        {
            goto loop_31;
        }
        var_a0_2 = var_t0_2 * M2C_FIELD(&D_80139950, s32*, 8);
        temp_a1_4 = ((s32)g_wmap_current_frame) + ((((var_t0_2 * 0x1A) + var_a3_2) * 0x28) + 0x340);
        if (var_a0_2 < 0)
        {
            var_a0_2 += 0xFFF;
        }
        temp_v0_10 = (var_a0_2 >> 0xC) + (s32)M2C_FIELD(&D_80139950, u8*, 4);
        temp_v1_6 = temp_v0_10 + 0x30;
        if (var_t3_2 != 0)
        {
            var_t3_2 = 0;
            temp_v0_11 = temp_v0_10 + 0x10;
            M2C_FIELD(temp_a1_4, s8*, 0x25) = temp_v0_11;
            M2C_FIELD(temp_a1_4, s8*, 0x1D) = temp_v0_11;
        }
        else
        {
            if (temp_v1_6 < 0x100)
            {
                M2C_FIELD(temp_a1_4, s16*, 0x1E) = 0;
            }
            else
            {
                M2C_FIELD(temp_a1_4, s16*, 0x1E) = 0x100;
            }
            M2C_FIELD(temp_a1_4, s8*, 0x25) = temp_v1_6;
            M2C_FIELD(temp_a1_4, s8*, 0x1D) = temp_v1_6;
        }
        var_a3_2 += 1;
        var_t6 += 0x410;
    } while (var_a3_2 < 0x19);
    var_t0_3 = 1;
    var_a2_3 = 0x1A;
    do
    {
        var_a3_3 = 1;
    loop_56:
        temp_a1_5 = ((s32)g_wmap_current_frame) + ((var_a2_3 + var_a3_3) * 0x28);
        var_a3_3 += 1;
        temp_a0_5 = M2C_FIELD(temp_a1_5, u16*, 0x35E);
        M2C_FIELD(temp_a1_5, s16*, 0x356) =
            (s16)(((u32)(temp_a0_5 & 0x100) >> 4) | ((u32)(M2C_FIELD(temp_a1_5, u16*, 0x366) & 0x3FF) >> 6) | ((temp_a0_5 & 0x200) * 4));
        if (var_a3_3 < 0x19)
        {
            goto loop_56;
        }
        var_t0_3 += 1;
        var_a2_3 += 0x1A;
    } while (var_t0_3 < 0x19);
}

/** @brief Advance the map selection transition and its view state. */
void func_800664B8(void)
{
    s32 temp_a2;
    s32 temp_a2_2;
    s32 temp_a3;
    s32 temp_a3_2;
    s32 temp_a3_3;
    s32 temp_t1;
    s32 var_t0;
    s32 var_t0_2;
    s32 var_v0;
    s32 var_v0_2;
    s32 var_v0_3;
    s32 var_v0_4;
    s32 var_v0_5;

    temp_t1 = D_8013986C;
    switch (temp_t1) /* irregular */
    {
    case 0:
        if ((g_wmap_buttons_repeat & 0x10) && (D_8011CF18 == 0) && (D_8011D4FC == -1))
        {
            D_8013986C = 3;
            func_8005FF88(-1);
            M2C_FIELD(&D_800DCEC8, s32*, 0) = (s32)M2C_FIELD(&D_80139950, s32*, 0);
            M2C_FIELD(&D_800DCEC8, s32*, 4) = (s32)M2C_FIELD(&D_80139950, s32*, 4);
            M2C_FIELD(&D_800DCEC8, s32*, 8) = (s32)M2C_FIELD(&D_80139950, s32*, 8);
            M2C_FIELD(&D_800DCEC8, s32*, 0xC) = (s32)M2C_FIELD(&D_80139950, s32*, 0xC);
            M2C_FIELD(&D_801AFBB8, s32*, 0) = (s32)(M2C_FIELD(&D_80139950, s32*, 0) << 8);
            M2C_FIELD(&D_801AFBB8, s32*, 4) = (s32)(M2C_FIELD(&D_80139950, s32*, 4) << 8);
            M2C_FIELD(&D_801AFBB8, s32*, 8) = (s32)(M2C_FIELD(&D_80139950, s32*, 8) << 8);
            M2C_FIELD(&D_801AFBA8, s32*, 0) = (s32) - (M2C_FIELD(&D_80139950, s32*, 0) * 0x10);
            temp_a3 = -(M2C_FIELD(&D_80139950, s32*, 4) * 0x10);
            M2C_FIELD(&D_801AFBA8, s32*, 4) = temp_a3;
            M2C_FIELD(&D_801AFBA8, s32*, 8) = (s32)(0xC0000 - (M2C_FIELD(&D_80139950, s32*, 8) * 0x10));
            func_800652A8(1, 0x80);
            var_v0 = 0xA130;
        block_13:
            g_wmap_map_button_mask = var_v0;
            g_wmap_buttons_held = 0;
            g_wmap_buttons_repeat = 0;
            return;
        }
        return;
    case 1:
        if (g_wmap_buttons_repeat & 0x30)
        {
            D_8013986C = 2;
            D_800D928A = 0x80;
            func_800652A8(2, 0x80);
            var_v0 = -1;
            goto block_13;
        }
        break;
    case 2:
        g_wmap_buttons_repeat = 0;
        var_t0 = M2C_FIELD(&D_801AFBB8, s32*, 0) - M2C_FIELD(&D_801AFBA8, s32*, 0);
        M2C_FIELD(&D_801AFBB8, s32*, 0) = var_t0;
        temp_a2 = M2C_FIELD(&D_801AFBB8, s32*, 4) - M2C_FIELD(&D_801AFBA8, s32*, 4);
        temp_a3_2 = M2C_FIELD(&D_801AFBB8, s32*, 8) - M2C_FIELD(&D_801AFBA8, s32*, 8);
        M2C_FIELD(&D_801AFBB8, s32*, 4) = temp_a2;
        M2C_FIELD(&D_801AFBB8, s32*, 8) = temp_a3_2;
        if (var_t0 < 0)
        {
            var_t0 += 0xFF;
        }
        M2C_FIELD(&D_80139950, s32*, 0) = (s32)(var_t0 >> 8);
        var_v0_2 = temp_a2;
        if (var_v0_2 < 0)
        {
            var_v0_2 += 0xFF;
        }
        M2C_FIELD(&D_80139950, s32*, 4) = (s32)(var_v0_2 >> 8);
        var_v0_3 = temp_a3_2;
        if (temp_a3_2 < 0)
        {
            var_v0_3 = temp_a3_2 + 0xFF;
        }
        M2C_FIELD(&D_80139950, s32*, 8) = (s32)(var_v0_3 >> 8);
        if (temp_a3_2 == 0x600000)
        {
            M2C_FIELD(&D_80139950, s32*, 0) = (s32)M2C_FIELD(&D_800DCEC8, s32*, 0);
            M2C_FIELD(&D_80139950, s32*, 4) = (s32)M2C_FIELD(&D_800DCEC8, s32*, 4);
            M2C_FIELD(&D_80139950, s32*, 8) = (s32)M2C_FIELD(&D_800DCEC8, s32*, 8);
            M2C_FIELD(&D_80139950, s32*, 0xC) = (s32)M2C_FIELD(&D_800DCEC8, s32*, 0xC);
            D_800DBE78 = 0;
            D_8013986C = 0;
            D_800DBE70 = temp_t1;
            return;
        }
        break;
    case 3:
        g_wmap_buttons_repeat = 0;
        var_t0_2 = M2C_FIELD(&D_801AFBB8, s32*, 0) + M2C_FIELD(&D_801AFBA8, s32*, 0);
        M2C_FIELD(&D_801AFBB8, s32*, 0) = var_t0_2;
        temp_a2_2 = M2C_FIELD(&D_801AFBB8, s32*, 4) + M2C_FIELD(&D_801AFBA8, s32*, 4);
        temp_a3_3 = M2C_FIELD(&D_801AFBB8, s32*, 8) + M2C_FIELD(&D_801AFBA8, s32*, 8);
        M2C_FIELD(&D_801AFBB8, s32*, 4) = temp_a2_2;
        M2C_FIELD(&D_801AFBB8, s32*, 8) = temp_a3_3;
        if (var_t0_2 < 0)
        {
            var_t0_2 += 0xFF;
        }
        M2C_FIELD(&D_80139950, s32*, 0) = (s32)(var_t0_2 >> 8);
        var_v0_4 = temp_a2_2;
        if (var_v0_4 < 0)
        {
            var_v0_4 += 0xFF;
        }
        M2C_FIELD(&D_80139950, s32*, 4) = (s32)(var_v0_4 >> 8);
        var_v0_5 = temp_a3_3;
        if (temp_a3_3 < 0)
        {
            var_v0_5 = temp_a3_3 + 0xFF;
        }
        M2C_FIELD(&D_80139950, s32*, 8) = (s32)(var_v0_5 >> 8);
        if (temp_a3_3 == 0xC00000)
        {
            M2C_FIELD(&D_80139950, s32*, 0) = 0;
            M2C_FIELD(&D_80139950, s32*, 8) = 0xC000;
            D_800DBE78 = 0;
            D_8013986C = 1;
            M2C_FIELD(&D_80139950, s32*, 4) = 0;
            D_800DBE70 = 1;
        }
        break;
    }
}

/**
 * @brief Fill the leading value of each interior world-map grid cell.
 * @param value Value assigned to the 24-by-24 interior.
 */
void func_800667E8(s32 value)
{
    s32 row;
    s32 column;
    for (row = 1; row < 25; row++)
    {
        for (column = 1; column < 25; column++)
        {
            *(s32*)((s32)g_wmap_current_frame + (row * 26 + column) * 0x28 + 0x344) = value;
        }
    }
}

/**
 * @brief Store a drawing color and register its update callback.
 * @param color Packed color bytes.
 * @return Always one.
 */
s32 func_8006683C(s32 color)
{
    D_8011D50C = *(CVECTOR*)&color;
    D_80139288 = 1;
    func_8006CBD8(func_80065620);
    return 1;
}

/**
 * @brief Spawn radial particles, advance their animation, and count active slots.
 * @return Number of active particle slots.
 */
s32 func_8006688C(void)
{
    s32 i;
    s32 actor_offset;
    s32 active;
    s32 remaining;
    s32 random_value;
    union
    {
        struct
        {
            s16 x, y;
        } point;
        s32 packed;
    } screen;
    WmapConfigA* actor;
    WmapConfigA* actor_base;
    WmapMotion* motion;

    active = 0;
    remaining = D_801B0FD0 * 3;
    for (i = 0, actor_offset = 264; i < D_801B0FD0 * 70; i++, actor_offset += 44)
    {
        motion = &D_801AFBD0[i];
        if (motion->state == 0)
        {
            actor = (WmapConfigA*)((u8*)D_800D9268 + actor_offset);
            if (D_801AFBC8 != 0)
            {
                actor->field_02 = 0;
                actor->field_06 = 15;
                actor->field_0E = ((s32)(rand() * D_801B0FD0) >> 15) + 1;
                actor->field_10 = -1;
                actor->field_22 = 129;
                actor->field_24 = 129;
                motion->state = 1;
                motion->angle = rand() >> 3;
                motion->z = 0;
                motion->x = rand() * D_801B0FD0;
                random_value = rand();
                motion->scale = ((s32)(random_value * (D_801B0FD0 * 20)) >> 15) + 5;
                if (--remaining == 0)
                {
                    break;
                }
            }
        }
    }
    actor_base = D_800D9370 - 6;
    for (i = 0, actor_offset = 264; i < 110; i++, actor_offset += 44)
    {
        motion = &D_801AFBD0[i];
        actor = &D_800D9370[i];
        if (motion->state != 0)
        {
            motion->z += motion->x;
            screen.point.x = ((motion->z * ccos(motion->angle)) >> 24) + 160;
            screen.point.y = ((motion->z * csin(motion->angle)) >> 24) + 120;
            func_8006CC4C(actor, &D_801399B8[i]);
            if (((WmapConfigA*)((u8*)actor_base + actor_offset))->field_0E == 3)
            {
                func_80066F9C(actor, screen.packed, 6, 4, 0);
            }
            else
            {
                func_80066F9C(actor, screen.packed, 7, 4, 0);
            }
            motion->scale--;
            if (motion->scale == 0)
            {
                motion->state = 0;
            }
            active++;
        }
    }
    return active;
}

/** @brief Reset 110 display states, install their default pointers, and enable processing. */
void func_80066B4C(void)
{
    s32 index = 0;
    u8* base = D_80139988;
    u8* data = D_8011D538;
    s32 offset = 0x30;
    WmapPointerSlot* slot;
    do
    {
        slot = (WmapPointerSlot*)(offset + (s32)base);
        offset += 8;
        D_801AFBD0[index].state = 0;
        slot->data = data;
        index++;
    } while (index < 110);
    D_801B0FD0 = 1;
    D_801AFBC8 = 1;
}

/** @brief Initialize the map effect and project its initial screen position. */
void func_80066BA4(void)
{
    SVECTOR position;
    MATRIX matrix;
    u32 screen;

    D_8013B20C = 1;
    func_8006D8F0(1);
    func_8006D870(1);
    D_801398D0 = 2;
    D_80182D68 = D_80054944[D_800DCEF0 * 3 + D_800DCEEC].x;
    D_80182D78 = D_80054944[D_800DCEF0 * 3 + D_800DCEEC].y;
    RotMatrix(&D_80139278, &matrix);
    TransMatrix(&matrix, &D_80182DC0);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
    position.vz = 0;
    position.vx = ((D_80139950.x * 0x14000 / D_80139950.scale) * 0x6000) / D_80139950.scale;
    position.vy = ((D_80139950.y * 0x14000 / D_80139950.scale) * 0x6000) / D_80139950.scale;
    gte_ldv0(&position);
    gte_rtps();
    gte_stsxy(&screen);
    D_800DBE70 = 1;
    D_800D928A = 0;
    D_801B109C = 4;
    D_80182234 = (s16)screen;
    D_8018223C = (s16)(screen >> 16);
    D_801B1098++;
}

/** @brief Set the map transform, project its position, and advance the effect. */
void func_80066DD8(void)
{
    MATRIX matrix;
    SVECTOR position;

    RotMatrix(&D_80139278, &matrix);
    TransMatrix(&matrix, &D_80182DC0);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
    position.vz = 0;
    position.vx = (((D_8011D510 - 1) * 160 - D_80139950.x * 0x14000 / D_80139950.scale) * 0x6000) / D_80139950.scale;
    position.vy = (((D_8011D530 - 1) * 160 - D_80139950.y * 0x14000 / D_80139950.scale) * 0x6000) / D_80139950.scale;
    gte_ldv0(&position);
    gte_rtps();
    gte_stsxy(&D_8011CF4C);
    D_8013B20C = 0;
    D_801B1098++;
}
