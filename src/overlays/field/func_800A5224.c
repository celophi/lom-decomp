#include "common.h"

/** @brief Ordering-table head and primitive allocation cursor. */
typedef struct
{
    u32 tag;
    u8 pad4[0x40B8 - 4];
    u8 *cursor;
} RenderContext;

/** @brief Textured quad packet with word views for tag and color writes. */
typedef struct
{
    union
    {
        u32 word;
        struct
        {
            u8 link[3];
            u8 length;
        } bytes;
    } tag;
    union
    {
        u32 word;
        struct
        {
            u8 r, g, b, code;
        } bytes;
    } color;
    s16 x0, y0;
    u8 u0, v0;
    s16 clut;
    s16 x1, y1;
    u8 u1, v1;
    s16 tpage;
    s16 x2, y2;
    u8 u2, v2;
    s16 pad2;
    s16 x3, y3;
    u8 u3, v3;
    s16 pad3;
} TexturedQuad;

extern s32 D_8011F3C8;

/**
 * @brief Queue five textured strips using the selected field rendering layout.
 * @param context Render context containing the ordering table and packet cursor.
 * @param layout Zero selects two quads per strip; nonzero selects one quad.
 * @note Partial match; loop-invariant scheduling and register differences remain.
 */
void func_800A5224(RenderContext *context, s32 layout)
{
    u32 *head;
    s16 split_left_x;
    s16 full_left_x;
    s16 full_right_x;
    s16 split_right_x;
    u8 *packet;
    s32 split_page;
    s32 full_page;
    s32 strip_index;
    TexturedQuad *full_quad;
    TexturedQuad *split_quad;

    if (D_8011F3C8 != 0)
    {
        packet = context->cursor;
        head = &context->tag;
        if (layout == 0)
        {
            strip_index = 0;
            split_right_x = 0x10;
            split_quad = (TexturedQuad *)packet;
            do
            {
                split_left_x = strip_index * 0x10;
                split_page = strip_index & 0xF;
                split_quad->color.word = 0x808080;
                split_quad->tag.bytes.length = 9;
                split_quad->color.bytes.code = 0x2C;
                split_quad->x0 = split_left_x;
                split_quad->y0 = 0;
                split_quad->x1 = split_right_x;
                split_quad->y1 = 0;
                split_quad->x2 = split_left_x;
                split_quad->y2 = 4;
                split_quad->x3 = split_right_x;
                split_quad->y3 = 4;
                split_quad->u0 = 0;
                split_quad->v0 = 0xF0;
                split_quad->u1 = 0x40;
                split_quad->v1 = 0xF0;
                split_quad->u2 = 0;
                split_quad->v2 = 0xFF;
                split_quad->u3 = 0x40;
                split_quad->v3 = 0xFF;
                split_quad->clut = 0;
                split_quad->tpage = (s16) (split_page | 0x120);
                split_quad++;
                strip_index += 1;
                ((TexturedQuad *)packet)->tag.word = (*head & 0xFFFFFF) | (((TexturedQuad *)packet)->tag.word & 0xFF000000);
                *head = (s32) ((*head & 0xFF000000) | ((s32) packet & 0xFFFFFF));
                packet += 0x28;
                split_quad->color.word = 0x808080;
                split_quad->tag.bytes.length = 9;
                split_quad->color.bytes.code = 0x2C;
                split_quad->x0 = split_left_x;
                split_quad->y0 = 4;
                split_quad->x1 = split_right_x;
                split_quad->y1 = 4;
                split_quad->x2 = split_left_x;
                split_quad->x3 = split_right_x;
                split_right_x += 0x10;
                split_quad->y2 = 0x38;
                split_quad->y3 = 0x38;
                split_quad->u0 = 0;
                split_quad->v0 = 0;
                split_quad->u1 = 0x40;
                split_quad->v1 = 0;
                split_quad->u2 = 0;
                split_quad->v2 = 0xD0;
                split_quad->u3 = 0x40;
                split_quad->v3 = 0xD0;
                split_quad->clut = 0;
                split_quad->tpage = (s16) (split_page | 0x130);
                split_quad += 2;
                ((TexturedQuad *)packet)->tag.word = (s32) ((*head & 0xFFFFFF) | (((TexturedQuad *)packet)->tag.word & 0xFF000000));
                *head = (s32) ((*head & 0xFF000000) | ((s32) packet & 0xFFFFFF));
                packet += 0x50;
            } while (strip_index < 5);
            context->cursor = packet;
            return;
        }
        strip_index = 0;
        full_right_x = 0x10;
        full_quad = (TexturedQuad *)packet;
        do
        {
            full_quad->x1 = full_right_x;
            full_quad->x3 = full_right_x;
            full_right_x += 0x10;
            full_left_x = strip_index * 0x10;
            full_page = strip_index & 0xF;
            strip_index += 1;
            full_quad->color.word = 0x808080;
            full_quad->tag.bytes.length = 9;
            full_quad->color.bytes.code = 0x2C;
            full_quad->x0 = full_left_x;
            full_quad->y0 = 0;
            full_quad->y1 = 0;
            full_quad->x2 = full_left_x;
            full_quad->y2 = 0x38;
            full_quad->y3 = 0x38;
            full_quad->u0 = 0;
            full_quad->v0 = 8;
            full_quad->u1 = 0x40;
            full_quad->v1 = 8;
            full_quad->u2 = 0;
            full_quad->v2 = 0xE8;
            full_quad->u3 = 0x40;
            full_quad->v3 = 0xE8;
            full_quad->clut = 0;
            full_quad->tpage = (s16) (full_page | 0x120);
            full_quad += 2;
            ((TexturedQuad *)packet)->tag.word = (*head & 0xFFFFFF) | (((TexturedQuad *)packet)->tag.word & 0xFF000000);
            *head = (s32) ((*head & 0xFF000000) | ((s32) packet & 0xFFFFFF));
            packet += 0x50;
        } while (strip_index < 5);
        context->cursor = packet;
    }
}
