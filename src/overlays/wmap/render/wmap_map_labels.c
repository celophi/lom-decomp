#include "common.h"
#include <libgpu.h>

#include "../internal/wmap_frame_render.h"
#include "../internal/wmap_land_layout.h"
#include "../internal/wmap_main.h"
#include "../internal/wmap_map_labels.h"
#include "../internal/wmap_party_travel.h"
#include "../internal/wmap_resource_support.h"
#include "../internal/wmap_sprite_render.h"
#include "../internal/wmap_view_effects.h"
#include "common/gpu_packet.h"

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

extern POLY_G3 g_wmap_status_panel_shades[];
extern SPRT g_wmap_status_panel_template;
extern SPRT g_wmap_day_icon_template;
extern SPRT g_wmap_day_name_template;
extern SPRT g_wmap_land_label_template;
extern SPRT g_wmap_artifact_label_template;
extern WmapPoint g_wmap_label_positions[];
extern s32 g_wmap_label_width_scales[];
extern s32 g_wmap_land_label_widths[];
extern s32 g_wmap_label_clut_row;
extern u16 g_wmap_land_label_cells[];
extern WmapLabelGlyph g_wmap_auxiliary_label_glyphs[];
extern WmapLabelChar g_wmap_auxiliary_label_chars[];
extern s32 g_wmap_auxiliary_label_starts[];

/** @brief Day shown by the status panel; -1 forces a refresh. */
extern s32 g_wmap_status_panel_day;
extern SPRT g_wmap_day_icon_sprite;
extern SPRT g_wmap_outgoing_day_icon_sprite;
extern SPRT g_wmap_day_name_sprite;
extern SPRT g_wmap_outgoing_day_name_sprite;

/** @brief Label selection: a land id, an artifact id plus 64, or -1 for none. */
extern s32 g_wmap_label_selection;
extern s32 g_wmap_outgoing_label_selection;
extern SPRT g_wmap_label_sprite;
extern SPRT g_wmap_outgoing_label_sprite;
extern u16 g_wmap_label_texture_page;
extern u16 g_wmap_outgoing_label_texture_page;
extern s8 D_80182E0C;

/**
 * @brief Append map label and decorative packets with selection fades.
 */
void wmap_draw_status_panel(void)
{
    SPRT* sprite;
    SPRT* shadow;
    POLY_G3* shade;
    s32 i;

    if (g_wmap_status_panel_mode != 3)
    {
        sprite = g_wmap_current_frame->packet_cursor;
        *sprite = g_wmap_status_panel_template;
        addPrim(&g_wmap_current_frame->ordering_table[7], sprite);

        if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
        {
            g_wmap_packet_bytes += sizeof(SPRT);
            g_wmap_current_frame->packet_cursor += sizeof(SPRT);
        }

        if (g_wmap_travel_day != g_wmap_status_panel_day)
        {
            g_wmap_status_panel_day = g_wmap_travel_day;
            g_wmap_outgoing_day_icon_sprite = g_wmap_day_icon_sprite;
            g_wmap_day_icon_sprite.r0 = 0;
            setSemiTrans(&g_wmap_outgoing_day_icon_sprite, 1);
            g_wmap_day_icon_sprite.u0 = ((g_wmap_travel_day & 1) * 0x30) + 8;
            g_wmap_day_icon_sprite.v0 = ((g_wmap_travel_day / 2) * 0x38) + 0xE;
            g_wmap_outgoing_day_name_sprite = g_wmap_day_name_sprite;
            g_wmap_day_name_sprite.u0 = 0;
            setSemiTrans(&g_wmap_outgoing_day_name_sprite, 1);
            g_wmap_day_name_sprite.v0 = g_wmap_travel_day * 32;
        }

        g_wmap_day_icon_sprite.g0 = g_wmap_day_icon_sprite.r0;
        g_wmap_day_icon_sprite.b0 = g_wmap_day_icon_sprite.r0;
        sprite = g_wmap_current_frame->packet_cursor;

        *sprite = g_wmap_day_icon_sprite;
        if ((s8)g_wmap_day_icon_sprite.r0 >= 0)
        {
            g_wmap_day_icon_sprite.r0 += 8;
            setSemiTrans(sprite, 1);
        }

        addPrim(&g_wmap_current_frame->ordering_table[7], g_wmap_current_frame->packet_cursor);

        if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
        {
            g_wmap_packet_bytes += sizeof(SPRT);
            g_wmap_current_frame->packet_cursor += sizeof(SPRT);
        }

        if (g_wmap_outgoing_day_icon_sprite.r0 != 0)
        {
            g_wmap_outgoing_day_icon_sprite.g0 = g_wmap_outgoing_day_icon_sprite.r0;
            g_wmap_outgoing_day_icon_sprite.b0 = g_wmap_outgoing_day_icon_sprite.r0;
            sprite = g_wmap_current_frame->packet_cursor;
            *sprite = g_wmap_outgoing_day_icon_sprite;
            addPrim(&g_wmap_current_frame->ordering_table[7], sprite);
            SET_BGR0_PACKED(&g_wmap_outgoing_day_name_sprite, *(u32*)&g_wmap_outgoing_day_icon_sprite.r0);

            if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
            {
                g_wmap_packet_bytes += sizeof(SPRT);
                g_wmap_current_frame->packet_cursor += sizeof(SPRT);
            }

            g_wmap_outgoing_day_icon_sprite.r0 -= 8;
        }

        wmap_queue_texture_page(0xBC, 7);
        wmap_draw_map_labels();
        sprite = g_wmap_current_frame->packet_cursor;
        SET_BGR0_PACKED(&g_wmap_day_name_sprite, *(u32*)&g_wmap_day_icon_sprite.r0);
        shadow = &g_wmap_day_name_sprite;
        setSemiTrans(shadow, 1);
        *sprite = *shadow;
        addPrim(&g_wmap_current_frame->ordering_table[7], g_wmap_current_frame->packet_cursor);

        if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
        {
            g_wmap_packet_bytes += sizeof(SPRT);
            g_wmap_current_frame->packet_cursor += sizeof(SPRT);
        }

        if (g_wmap_outgoing_day_name_sprite.r0 >= 9)
        {
            sprite = g_wmap_current_frame->packet_cursor;
            *sprite = g_wmap_outgoing_day_name_sprite;
            addPrim(&g_wmap_current_frame->ordering_table[7], g_wmap_current_frame->packet_cursor);

            if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
            {
                g_wmap_packet_bytes += sizeof(SPRT);
                g_wmap_current_frame->packet_cursor += sizeof(SPRT);
            }
        }

        wmap_queue_texture_page(0x3E, 7);

        for (i = 0; i < 0x24; i++)
        {
            shade = g_wmap_current_frame->packet_cursor;
            *shade = g_wmap_status_panel_shades[i];
            setlen(shade, 6);
            setcode(shade, 0x32);
            addPrim(&g_wmap_current_frame->ordering_table[8], shade);

            if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
            {
                g_wmap_packet_bytes += sizeof(POLY_G3);
                g_wmap_current_frame->packet_cursor += sizeof(POLY_G3);
            }
        }

        wmap_queue_texture_page(0x20, 8);
    }
}

/**
 * @brief Change the map label sprite while preserving its outgoing fade.
 * @param selection Label identifier, or -1 to hide the current label.
 */
void wmap_set_map_label(s32 selection)
{
    s32 position;
    s32 previous_selection;
    s32 width;
    s32* scale;

    if (selection == 33)
    {
        selection = 24;
    }

    if (g_wmap_view_scroll_mode != WMAP_SCROLL_NONE)
    {
        selection = -1;
    }

    previous_selection = g_wmap_label_selection;

    if (previous_selection != selection && (g_wmap_outgoing_label_sprite.r0 == 0 || selection == -1))
    {
        g_wmap_outgoing_label_selection = previous_selection;
        g_wmap_outgoing_label_sprite = g_wmap_label_sprite;
        g_wmap_label_selection = selection;
        g_wmap_outgoing_label_texture_page = g_wmap_label_texture_page;
        g_wmap_outgoing_label_sprite.code |= 2;
        g_wmap_outgoing_label_sprite.clut = (g_wmap_label_clut_row << 6) | 47;

        if (selection == -1)
        {
            g_wmap_label_sprite.r0 = 0;
            return;
        }

        position = g_wmap_cursor_column + g_wmap_cursor_row * 3;

        if (selection >= 64)
        {
            g_wmap_label_texture_page = 45;
            g_wmap_label_sprite = g_wmap_artifact_label_template;
            g_wmap_label_clut_row = 506;
        }
        else
        {
            g_wmap_label_texture_page = 43;
            g_wmap_label_sprite = g_wmap_land_label_template;

            if (wmap_is_land_active(selection) == 0)
            {
                g_wmap_label_clut_row = 508;
            }
            else
            {
                g_wmap_label_clut_row = 506;
            }

            scale = &g_wmap_label_width_scales[position];
            width = *scale * g_wmap_land_label_widths[selection];
            g_wmap_label_sprite.y0 = g_wmap_label_positions[position].y;
            g_wmap_label_sprite.x0 = g_wmap_label_positions[position].x - width / 2;
        }

        selection &= 63;
        selection = (s16)g_wmap_land_label_cells[selection];
        g_wmap_label_sprite.r0 = 8;
        g_wmap_label_sprite.u0 = (selection / 14) * 112;
        g_wmap_label_sprite.v0 = (selection % 14) * 16;
    }
}

/**
 * @brief Append the current and outgoing label sprites and auxiliary labels.
 */
void wmap_draw_map_labels(void)
{
    SPRT* sprite;
    WmapLabelChar* label_char;
    WmapLabelGlyph* glyph;
    s32 char_index;
    u16 clut_y;
    u8 fade;
    SPRT* label;

    label = &g_wmap_label_sprite;

    if (label->r0 != 0)
    {
        label->b0 = label->r0;
        label->g0 = label->r0;
        sprite = g_wmap_current_frame->packet_cursor;

        if ((s8)label->r0 >= 0)
        {
            clut_y = *(u16*)&g_wmap_label_clut_row;
            label->r0 += 8;
            label->clut = (clut_y << 6) | 0x2F;
        }
        else
        {
            clut_y = *(u16*)&g_wmap_label_clut_row + 1;
            label->clut = (clut_y << 6) | 0x2F;
        }

        *sprite = g_wmap_label_sprite;
        setSemiTrans(sprite, 1);
        addPrim(&g_wmap_current_frame->ordering_table[3], sprite);

        if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
        {
            g_wmap_packet_bytes += sizeof(SPRT);
            g_wmap_current_frame->packet_cursor += sizeof(SPRT);
        }

        wmap_queue_texture_page(g_wmap_label_texture_page, 3);
    }

    fade = g_wmap_outgoing_label_sprite.r0;

    if (fade != 0)
    {
        sprite = g_wmap_current_frame->packet_cursor;
        g_wmap_outgoing_label_sprite.r0 = fade - 8;
        g_wmap_outgoing_label_sprite.b0 = fade;
        g_wmap_outgoing_label_sprite.g0 = fade;

        *sprite = g_wmap_outgoing_label_sprite;
        if (g_wmap_outgoing_label_selection != -1)
        {
            addPrim(&g_wmap_current_frame->ordering_table[3], sprite);

            if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
            {
                g_wmap_packet_bytes += sizeof(SPRT);
                g_wmap_current_frame->packet_cursor += sizeof(SPRT);
            }

            wmap_queue_texture_page(g_wmap_outgoing_label_texture_page, 3);
        }
    }

    if (g_wmap_auxiliary_label_draw_mode != g_wmap_auxiliary_label_mode)
    {
        g_wmap_auxiliary_label_draw_mode = g_wmap_auxiliary_label_mode;
    }

    if (g_wmap_auxiliary_labels_hidden == 0)
    {
        char_index = g_wmap_auxiliary_label_starts[g_wmap_auxiliary_label_draw_mode];

        do
        {
            SPRT* glyph_sprite;

            label_char = &g_wmap_auxiliary_label_chars[char_index];
            glyph_sprite = g_wmap_current_frame->packet_cursor;
            glyph = &g_wmap_auxiliary_label_glyphs[label_char->glyph];
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

            if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
            {
                g_wmap_packet_bytes += sizeof(SPRT);
                g_wmap_current_frame->packet_cursor += sizeof(SPRT);
            }

            char_index++;
        } while (g_wmap_auxiliary_label_starts[g_wmap_auxiliary_label_draw_mode + 1] != char_index);
        wmap_queue_texture_page(0xB, 1);
    }
}

/**
 * @brief Restore map sprite templates and reset the display selection.
 */
void wmap_init_label_sprites(void)
{
    g_wmap_label_sprite = g_wmap_land_label_template;
    g_wmap_day_icon_sprite = g_wmap_day_icon_template;
    g_wmap_outgoing_day_icon_sprite = g_wmap_day_icon_template;
    g_wmap_day_name_sprite = g_wmap_day_name_template;
    g_wmap_outgoing_day_name_sprite = g_wmap_day_name_template;
    g_wmap_outgoing_day_icon_sprite.b0 = 0;
    g_wmap_outgoing_day_icon_sprite.g0 = 0;
    g_wmap_outgoing_day_icon_sprite.r0 = 0;
    g_wmap_outgoing_day_name_sprite.b0 = 0;
    g_wmap_outgoing_day_name_sprite.g0 = 0;
    g_wmap_outgoing_day_name_sprite.r0 = 0;
    D_80182E0C = 0;
    g_wmap_label_sprite.r0 = 0;
    g_wmap_label_selection = -1;
    g_wmap_status_panel_day = -1;
}
