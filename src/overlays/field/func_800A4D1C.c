#include "common.h"
#include "sdk/libgpu.h"
#include "sdk/libgte.h"

void func_80086F48(POLY_FT4 *, s32);

extern s32 D_8011F340;
extern s32 D_8011F344;
extern s32 D_8011F348;
extern s32 D_8011F34C;
extern s32 D_8011F350;
extern s32 D_8011F378;
extern u8 D_8011F388[];
extern s32 D_8011F3B4;
extern s32 D_8011F3B8;
extern s32 D_8011F3BC;
extern s32 D_8011F3C0;
extern s32 g_frame_counter;

/**
 * @brief Draw the selectable textured quads arranged around the rotating menu ring.
 * @param render_context Rendering context with an ordering table at 0x40 and packet cursor at
 * 0x40B8.
 * @note Ring angle controls position, brightness, size, and ordering depth. The selected
 * item grows by one eighth every eighth frame. Only submitted quads advance the packet cursor.
 * @note WIP: 97.341730% gcc272_cdk, with allocation and scheduling differences.
 */
void func_800A4D1C(u8 *render_context)
{
    u16 top;
    s16 right;
    s32 bottom;
    s32 *bucket;
    POLY_FT4 *prim;
    u32 address_mask;
    u32 tag_mask;
    s32 columns;
    s32 vertical_product;
    s32 u_or_angle;
    s32 final_cosine;
    s32 initial_count;
    s32 depth_cosine;
    s32 depth;
    s32 final_depth;
    s32 v_or_width;
    s32 height;
    s32 index;
    s32 visible_depth;
    s32 depth_numerator;
    s32 right_u;
    s32 bottom_v;
    s32 brightness;
    s32 packet_code;
    u16 left;
    u8 *entry;
    u8 tile;
    u32 *ordering_table;

    initial_count = D_8011F3B8;
    index = 0;
    prim = *(POLY_FT4 **)(render_context + 0x40B8);
    ordering_table = (u32 *)(render_context + 0x40);
    if (initial_count > 0)
    {
        address_mask = 0xFFFFFF;
        entry = D_8011F388;
        do
        {
            /* Keep packet initialization separate from the texture calculations. */
            do
            {
                setPolyFT4(prim);
                packet_code = 0x2E;
                if (D_8011F3B4 == 0)
                {
                    packet_code = 0x2C;
                }
                prim->code = packet_code;
            } while (0);
            columns = 0x100 / (s32)D_8011F348;
            tile = *entry;
            u_or_angle = ((s32)tile % columns) * D_8011F348;
            v_or_width = ((s32)tile / columns) * D_8011F34C;
            prim->u2 = u_or_angle;
            prim->u0 = u_or_angle;
            right_u = ((u8)D_8011F348 + u_or_angle) - 1;
            prim->u3 = right_u;
            prim->u1 = right_u;
            prim->v1 = v_or_width;
            prim->v0 = v_or_width;
            bottom_v = ((u8)D_8011F34C + v_or_width) - 1;
            prim->v3 = bottom_v;
            prim->v2 = bottom_v;
            u_or_angle = ((s32)(index << 0xC) / (s32)D_8011F3B8) + D_8011F3C0;
            brightness = (u8)D_8011F3BC +
                         ((s32)(((s32)D_8011F3BC >> 1) * (rcos(u_or_angle) - 0x1000)) >> 0xD);
            prim->b0 = brightness;
            prim->g0 = brightness;
            prim->r0 = brightness;
            v_or_width =
                D_8011F348 + ((s32)(((s32)D_8011F348 >> 1) * (rcos(u_or_angle) - 0x1000)) >> 0xD);
            height =
                D_8011F34C + ((s32)(((s32)D_8011F34C >> 1) * (rcos(u_or_angle) - 0x1000)) >> 0xD);
            if ((index == D_8011F378) && !(g_frame_counter & 7))
            {
                v_or_width = v_or_width * 9 / 8;
                height = height * 9 / 8;
            }
            left = ((u16)D_8011F340 + ((s32)(D_8011F350 * rsin(u_or_angle)) >> 0xC)) -
                   (v_or_width >> 1);
            prim->x2 = left;
            prim->x0 = left;
            vertical_product = D_8011F350 * rcos(u_or_angle);
            prim->tpage = 0x25;
            top = ((u16)D_8011F344 + (vertical_product >> 0xE)) - (height >> 1);
            prim->y0 = top;
            /* Preserve the coordinate copy before completing the opposite corners. */
            do
            {
                bottom = top;
                right = (u16)prim->x0;
                bottom += height;
                prim->y1 = top;
                prim->y3 = bottom;
                prim->y2 = bottom;
            } while (0);
            right += v_or_width;
            prim->x3 = right;
            prim->x1 = right;
            prim->clut = (s16)((*entry & 0x3F) | 0x7C80);
            visible_depth = (rcos(u_or_angle) - 0x1000) / 64;
            if (visible_depth <= 0)
            {
                depth_cosine = rcos(u_or_angle);
                depth_numerator = depth_cosine - 0x1000;
                if (depth_numerator < 0)
                {
                    depth_numerator = depth_cosine - 0xFC1;
                }
                /* Preserve the GPU length byte while linking the 24-bit packet address. */
                tag_mask = 0xFF000000;
                *(u32 *)prim = (*(u32 *)prim & tag_mask) |
                               (*(ordering_table - (depth_numerator >> 6)) & address_mask);
                depth = (rcos(u_or_angle) - 0x1000) / 64;
                bucket = (s32 *)(ordering_table - depth);
                *bucket = (*bucket & tag_mask) | ((s32)prim & address_mask);
                final_cosine = rcos(u_or_angle);
                final_depth = final_cosine - 0x1000;
                if (final_depth < 0)
                {
                    final_depth = final_cosine - 0xFC1;
                }
                func_80086F48(prim, -(final_depth >> 6) + 0x10);
                prim++;
            }
            index += 1;
            entry += 1;
        } while (index < D_8011F3B8);
    }
    *(POLY_FT4 **)(render_context + 0x40B8) = prim;
}
