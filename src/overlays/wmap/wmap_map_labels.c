#include "wmap_frame_render.h"
#include "wmap_land_layout.h"
#include "wmap_party_travel.h"
#include "wmap_resource_support.h"
#include "wmap_map_labels.h"
#include "sdk/libgpu.h"
#include "gpu_packet.h"

/** @brief Screen position of a map label at a particular map scale. */
typedef struct
{
    s16 x, y;
} WmapPoint;

extern u8 D_800515F4;
extern u8 D_80051A58;

extern SPRT D_800DBE80;
extern s32 D_800DBE84;
extern s32 D_8011CF80;
extern SPRT D_801391E8;
extern SPRT D_801398D8;
extern u8 D_801398DC;

extern SPRT D_80182DA0;
extern s32 D_80182DA4;
extern s32 D_80182E34;
extern WmapPoint D_800519E4[];
extern SPRT D_80051A08;
extern SPRT D_80051A1C;
extern s32 D_800D0230[];
extern s32 D_800D0254[];
extern s32 D_800D0368;
extern u16 D_800D036C[];
extern s32 g_wmap_cursor_column;
extern s32 g_wmap_cursor_row;
extern u16 D_8011CF78;
extern SPRT D_8011D518;
extern s32 g_wmap_view_scroll_mode;
extern u16 D_80182DD0;
extern s32 D_80182E04;
extern SPRT D_80182E08;
extern s32 D_801ADAE4;
extern s32 D_80051A6C[];
extern u8 D_800D03EC;
extern u8 D_800D040C;
extern s32 D_801398BC;
extern s32 D_8013B258;
extern s32 D_8013B28C;
extern SPRT D_80051A30;
extern SPRT D_80051A44;
extern s8 D_80182E0C;

void func_80060230(void);

/** @brief Gouraud-shaded triangle packet (libgpu WmapShadeTriangle layout). */
typedef struct
{
    u_long tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 r1, g1, b1, pad1;
    s16 x1, y1;
    u8 r2, g2, b2, pad2;
    s16 x2, y2;
} WmapShadeTriangle;

/** @brief Current map render context viewed through its ordering-table layout. */

/**
 * @brief Append map label and decorative packets with selection fades.
 */
void func_8005F9BC(void)
{
    SPRT* sprite;
    SPRT* shadow;
    WmapShadeTriangle* shade;
    s32 i;

    if (D_80182E34 != 3)
    {
        sprite = (SPRT*)g_wmap_current_frame->packet_cursor;
        *sprite = *(SPRT*)&D_80051A58;
        addPrim(&g_wmap_current_frame->ordering_table[7], sprite);
        if (g_wmap_packet_bytes < 0x7D00)
        {
            g_wmap_packet_bytes += sizeof(SPRT);
            g_wmap_current_frame->packet_cursor += sizeof(SPRT);
        }
        if (g_wmap_travel_day != D_8011CF80)
        {
            D_8011CF80 = g_wmap_travel_day;
            D_801391E8 = D_80182DA0;
            D_80182DA0.r0 = 0;
            D_800DBE80 = D_801398D8;
            D_801398D8.u0 = 0;
            setSemiTrans(&D_801391E8, 1);
            D_80182DA0.u0 = ((g_wmap_travel_day & 1) * 0x30) + 8;
            D_80182DA0.v0 = ((g_wmap_travel_day / 2) * 0x38) + 0xE;
            setSemiTrans(&D_800DBE80, 1);
            D_801398D8.v0 = *(volatile s32*)&g_wmap_travel_day << 5;
        }
        D_80182DA0.g0 = D_80182DA0.r0;
        D_80182DA0.b0 = D_80182DA0.r0;
        sprite = (SPRT*)g_wmap_current_frame->packet_cursor;
        *sprite = D_80182DA0;
        if ((s8)D_80182DA0.r0 >= 0)
        {
            D_80182DA0.r0 += 8;
            setSemiTrans(sprite, 1);
        }
        addPrim(&g_wmap_current_frame->ordering_table[7], g_wmap_current_frame->packet_cursor);
        if (g_wmap_packet_bytes < 0x7D00)
        {
            g_wmap_packet_bytes += sizeof(SPRT);
            g_wmap_current_frame->packet_cursor += sizeof(SPRT);
        }
        if (D_801391E8.r0 != 0)
        {
            D_801391E8.g0 = D_801391E8.r0;
            D_801391E8.b0 = D_801391E8.r0;
            sprite = (SPRT*)g_wmap_current_frame->packet_cursor;
            *sprite = D_801391E8;
            addPrim(&g_wmap_current_frame->ordering_table[7], sprite);
            D_800DBE84 = *(s32*)&D_801391E8.r0;
            if (g_wmap_packet_bytes < 0x7D00)
            {
                g_wmap_packet_bytes += sizeof(SPRT);
                g_wmap_current_frame->packet_cursor += sizeof(SPRT);
            }
            D_801391E8.r0 -= 8;
        }
        func_8006534C(0xBC, 7);
        func_80060230();
        sprite = (SPRT*)g_wmap_current_frame->packet_cursor;
        *(s32*)&D_801398DC = D_80182DA4;
        shadow = (SPRT*)(&D_801398DC - 4);
        setSemiTrans(shadow, 1);
        *sprite = *shadow;
        addPrim(&g_wmap_current_frame->ordering_table[7], g_wmap_current_frame->packet_cursor);
        if (g_wmap_packet_bytes < 0x7D00)
        {
            g_wmap_packet_bytes += sizeof(SPRT);
            g_wmap_current_frame->packet_cursor += sizeof(SPRT);
        }
        if (D_800DBE80.r0 >= 9)
        {
            sprite = (SPRT*)g_wmap_current_frame->packet_cursor;
            *sprite = D_800DBE80;
            addPrim(&g_wmap_current_frame->ordering_table[7], g_wmap_current_frame->packet_cursor);
            if (g_wmap_packet_bytes < 0x7D00)
            {
                g_wmap_packet_bytes += sizeof(SPRT);
                g_wmap_current_frame->packet_cursor += sizeof(SPRT);
            }
        }
        func_8006534C(0x3E, 7);
        for (i = 0; i < 0x24; i++)
        {
            shade = (WmapShadeTriangle*)g_wmap_current_frame->packet_cursor;
            *shade = ((WmapShadeTriangle*)&D_800515F4)[i];
            setlen(shade, 6);
            setcode(shade, 0x32);
            addPrim(&g_wmap_current_frame->ordering_table[8], shade);
            if (g_wmap_packet_bytes < 0x7D00)
            {
                g_wmap_packet_bytes += sizeof(WmapShadeTriangle);
                g_wmap_current_frame->packet_cursor += sizeof(WmapShadeTriangle);
            }
        }
        func_8006534C(0x20, 8);
    }
}

/**
 * @brief Change the map label sprite while preserving its outgoing fade.
 * @param selection Label identifier, or -1 to hide the current label.
 */
void func_8005FF88(s32 selection)
{
    s32 position;
    s32 previous_selection;
    s32 width;
    s32* scale;

    if (selection == 33)
    {
        selection = 24;
    }
    if (g_wmap_view_scroll_mode != 0)
    {
        selection = -1;
    }
    previous_selection = D_801ADAE4;
    if (previous_selection != selection && (D_80182E08.r0 == 0 || selection == -1))
    {
        D_80182E04 = previous_selection;
        D_80182E08 = D_8011D518;
        D_801ADAE4 = selection;
        D_80182DD0 = D_8011CF78;
        D_80182E08.code |= 2;
        D_80182E08.clut = (D_800D0368 << 6) | 47;
        if (selection == -1)
        {
            D_8011D518.r0 = 0;
            return;
        }
        position = g_wmap_cursor_column + g_wmap_cursor_row * 3;
        if (selection >= 64)
        {
            D_8011CF78 = 45;
            D_8011D518 = D_80051A08;
            D_800D0368 = 506;
        }
        else
        {
            D_8011CF78 = 43;
            D_8011D518 = D_80051A1C;
            if (wmap_is_land_active(selection) == 0)
            {
                D_800D0368 = 508;
            }
            else
            {
                D_800D0368 = 506;
            }
            scale = &D_800D0230[position];
            width = *scale * D_800D0254[selection];
            D_8011D518.y0 = D_800519E4[position].y;
            D_8011D518.x0 = D_800519E4[position].x - width / 2;
        }
        selection &= 63;
        selection = (s16)D_800D036C[selection];
        D_8011D518.r0 = 8;
        D_8011D518.u0 = (selection / 14) * 112;
        D_8011D518.v0 = (selection % 14) * 16;
    }
}


/** @brief Glyph texture rectangle in the auxiliary label font table. */
typedef struct
{
    u8 u;
    u8 pad_01;
    u8 v;
    u8 pad_03;
    u16 w;
    u16 h;
} WmapLabelGlyph;

/** @brief One placed glyph of an auxiliary label string. */
typedef struct
{
    u8 glyph;
    u8 pad_01;
    u16 x;
    u8 y;
    u8 pad_05;
} WmapLabelChar;

/**
 * @brief Append the current and outgoing label sprites and auxiliary labels.
 */
void func_80060230(void)
{
    SPRT* sprite;
    WmapLabelChar* label_char;
    WmapLabelGlyph* glyph;
    s32 char_index;
    u16 clut_y;
    u8 fade;
    SPRT* label;

    label = &D_8011D518;
    if (label->r0 != 0)
    {
        label->b0 = label->r0;
        label->g0 = label->r0;
        sprite = (SPRT*)g_wmap_current_frame->packet_cursor;
        if ((s8)label->r0 >= 0)
        {
            clut_y = *(u16*)&D_800D0368;
            label->r0 += 8;
            label->clut = (clut_y << 6) | 0x2F;
        }
        else
        {
            clut_y = *(u16*)&D_800D0368 + 1;
            label->clut = (clut_y << 6) | 0x2F;
        }
        *sprite = D_8011D518;
        setSemiTrans(sprite, 1);
        addPrim(&g_wmap_current_frame->ordering_table[3], sprite);
        if (g_wmap_packet_bytes < 0x7D00)
        {
            g_wmap_packet_bytes += sizeof(SPRT);
            g_wmap_current_frame->packet_cursor += sizeof(SPRT);
        }
        func_8006534C(D_8011CF78, 3);
    }
    fade = D_80182E08.r0;
    if (fade != 0)
    {
        sprite = (SPRT*)g_wmap_current_frame->packet_cursor;
        D_80182E08.r0 = fade - 8;
        D_80182E08.b0 = fade;
        D_80182E08.g0 = fade;
        *sprite = D_80182E08;
        if (D_80182E04 != -1)
        {
            addPrim(&g_wmap_current_frame->ordering_table[3], sprite);
            if (g_wmap_packet_bytes < 0x7D00)
            {
                g_wmap_packet_bytes += sizeof(SPRT);
                g_wmap_current_frame->packet_cursor += sizeof(SPRT);
            }
            func_8006534C(D_80182DD0, 3);
        }
    }
    if (D_8013B28C != D_801398BC)
    {
        D_8013B28C = D_801398BC;
    }
    if (D_8013B258 == 0)
    {
        char_index = D_80051A6C[D_8013B28C];
        do
        {
            SPRT* glyph_sprite;

            label_char = &((WmapLabelChar*)&D_800D040C)[char_index];
            glyph_sprite = (SPRT*)g_wmap_current_frame->packet_cursor;
            glyph = &((WmapLabelGlyph*)&D_800D03EC)[label_char->glyph];
            glyph_sprite->u0 = glyph->u;
            glyph_sprite->v0 = glyph->v - 0x20;
            glyph_sprite->w = glyph->w;
            glyph_sprite->h = glyph->h;
            glyph_sprite->x0 = label_char->x;
            glyph_sprite->y0 = label_char->y;
            glyph_sprite->clut = 0x7F2E;
            SET_BGR0_PACKED(glyph_sprite, 0x808080);
            setSprt(glyph_sprite);
            addPrim(&g_wmap_current_frame->ordering_table[1], glyph_sprite);
            if (g_wmap_packet_bytes < 0x7D00)
            {
                g_wmap_packet_bytes += sizeof(SPRT);
                g_wmap_current_frame->packet_cursor += sizeof(SPRT);
            }
            char_index += 1;
        } while (D_80051A6C[D_8013B28C + 1] != char_index);
        func_8006534C(0xB, 1);
    }
}

/**
 * @brief Restore map sprite templates and reset the display selection.
 */
void func_800605B4(void)
{
    D_8011D518 = D_80051A1C;
    D_80182DA0 = D_80051A44;
    D_801391E8 = D_80051A44;
    D_801398D8 = D_80051A30;
    D_800DBE80 = D_80051A30;
    D_801391E8.b0 = 0;
    D_801391E8.g0 = 0;
    D_801391E8.r0 = 0;
    D_800DBE80.b0 = 0;
    D_800DBE80.g0 = 0;
    D_800DBE80.r0 = 0;
    D_80182E0C = 0;
    D_8011D518.r0 = 0;
    D_801ADAE4 = -1;
    D_8011CF80 = -1;
}
