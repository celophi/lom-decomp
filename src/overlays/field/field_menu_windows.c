/** @file
 * @brief FIELD menu-window rendering: bordered frames, sprite tiling, element
 *        lifecycle and the eight-element update/draw loop.
 *
 * Consolidated translation unit (gcc272_cdk) for the related menu-window family
 * at 0x800AD850-0x800AE8A8. See docs/decompilation/field-boundaries/map.md
 * (candidate field_menu_windows). The shared record @c D_80122828 is viewed with
 * a different layout by several routines; each keeps its own block-scope extern
 * of the original type, and @c g_menu_element_counter is likewise read as u16 or
 * s32 per its original use, so every function reproduces its standalone codegen.
 */

#include "common.h"
#include "sdk/libgpu.h"

/** @brief Eight-element menu record as seen by the lifecycle reset. */
typedef struct
{
    u32 word;
    u8 pad[0x14 - 4];
} UnkEntry80122828;

/** @brief Eight-element menu record as seen by the state queries. */
typedef struct
{
    s32 flags; /* 0x00 */
    u8 pad4[0x10];
} RecADEEC;

/** @brief Eight-element menu record as seen by the allocation helper. */
typedef struct
{
    u32 flags;
    u32 state;
    u16 unk8;
    u16 unkA;
    u16 unkC;
    u8 padE[6];
} FieldADF84Rec;

typedef struct FieldMenuRenderContext FieldMenuRenderContext;
typedef struct FieldMenuElement FieldMenuElement;

/** @brief Render context with ordering-table head, buffer selector, and packet cursor. */
struct FieldMenuRenderContext
{
    u32 tag;
    u8 pad4[0x40B2 - 4];
    s16 buffer;
    u8 pad40b4[4];
    s32 *cursor;
};

/** @brief Twenty-byte menu element with packed geometry, scrolling, and draw callback. */
struct FieldMenuElement
{
    u32 attr;
    union
    {
        u32 word;
        struct
        {
            u16 bits;
            s16 content_height;
        } fields;
    } size;
    s16 scroll, scroll_target, scroll_ticks;
    u16 padE;
    s32 *(*draw)(FieldMenuRenderContext *, s32 *, s32, s32, s32, FieldMenuElement *);
};

extern u8 D_800EF1BC[];
extern s32 g_pad_input, g_frame_counter;

void func_800A3938(s32, s32);
void *func_800ADCD0(void *, u32 *, RECT *, RECT *);
u_long *func_800AE76C(u_long *, u_long *, s32, s32, s32);

/**
 * @brief Emit a bordered menu rectangle, fill tile and draw-mode command.
 * @param buffer First free primitive-buffer address.
 * @param ordering Ordering-table entry receiving the new primitive chain.
 * @param x Rectangle origin in screen coordinates.
 * @param y Rectangle origin in screen coordinates.
 * @param width Rectangle width in pixels.
 * @param height Rectangle height in pixels.
 * @param bottom_buffer Nonzero selects the lower framebuffer clipping region.
 * @param bright Nonzero selects the brighter fill color.
 * @return First free buffer address after all emitted primitives.
 */
s32 *func_800AD850(s32 *buffer, s32 *ordering, s32 x, s32 y, s32 width, s32 height,
                   s32 bottom_buffer, s32 bright)
{
    extern u16 g_menu_element_counter;
    s32 *ot = ordering;
    DR_ENV *draw_packet = (DR_ENV *)buffer;
    DRAWENV draw_env;
    RECT destination;
    RECT texture;
    s32 *cursor;
    u_long *packet;

    if (bottom_buffer != 0)
    {
        SetDefDrawEnv(&draw_env, x + 2, y + 0xF2, width - 4, height - 4);
    }
    else
    {
        SetDefDrawEnv(&draw_env, x + 2, y + 0xA, width - 4, height - 4);
    }
    SetDrawEnv(draw_packet, &draw_env);
    addPrim(ot, draw_packet);
    cursor = (s32 *)(draw_packet + 1);
    setRECT(&destination, x - 4, y - 4, 8, 8);
    setRECT(&texture, g_menu_element_counter + 0x40, 0xE0, 8, 8);
    cursor = func_800ADCD0(cursor, ot, &destination, &texture);
    setRECT(&destination, x + width - 4, y - 4, 8, 8);
    setRECT(&texture, g_menu_element_counter + 0x58, 0xE0, 8, 8);
    cursor = func_800ADCD0(cursor, ot, &destination, &texture);
    setRECT(&destination, x - 4, y + height - 4, 8, 8);
    setRECT(&texture, g_menu_element_counter + 0x40, 0xF8, 8, 8);
    cursor = func_800ADCD0(cursor, ot, &destination, &texture);
    setRECT(&destination, x + width - 4, y + height - 4, 8, 8);
    setRECT(&texture, g_menu_element_counter + 0x58, 0xF8, 8, 8);
    cursor = func_800ADCD0(cursor, ot, &destination, &texture);
    setRECT(&destination, x + 4, y - 4, width - 8, 8);
    setRECT(&texture, g_menu_element_counter + 0x48, 0xE0, 16, 8);
    cursor = func_800ADCD0(cursor, ot, &destination, &texture);
    setRECT(&destination, x + 4, y + height - 4, width - 8, 8);
    setRECT(&texture, g_menu_element_counter + 0x48, 0xF8, 16, 8);
    cursor = func_800ADCD0(cursor, ot, &destination, &texture);
    setRECT(&destination, x - 4, y + 4, 8, height - 8);
    setRECT(&texture, g_menu_element_counter + 0x40, 0xE8, 8, 16);
    cursor = func_800ADCD0(cursor, ot, &destination, &texture);
    setRECT(&destination, x + width - 4, y + 4, 8, height - 8);
    setRECT(&texture, g_menu_element_counter + 0x58, 0xE8, 8, 16);
    cursor = func_800ADCD0(cursor, ot, &destination, &texture);
    packet = (u_long *)cursor;
    if (bright != 0)
    {
        *(u32 *)&((TILE *)packet)->r0 = 0xA0A0A0;
    }
    else
    {
        *(u32 *)&((TILE *)packet)->r0 = 0x303030;
    }

    setTile((TILE *)packet);
    setSemiTrans((TILE *)packet, 1);
    setXY0((TILE *)packet, x, y);
    setWH((TILE *)packet, width, height);
    addPrim(ot, packet);
    packet += sizeof(TILE) / sizeof(*packet);
    setlen((DR_TPAGE *)packet, 1);
    ((DR_TPAGE *)packet)->code[0] = 0xE1000054;
    addPrim(ot, (DR_TPAGE *)packet);
    return (s32 *)((DR_TPAGE *)packet + 1);
}

/**
 * @brief Tile a rectangle with sprite primitives and link them into an ordering table.
 * @param packet_cursor Next free primitive-buffer address.
 * @param ordering_table Ordering-table entry receiving the primitive chain.
 * @param destination Destination rectangle position and dimensions.
 * @param texture Texture origin and maximum tile dimensions.
 * @return The first free buffer address after the emitted primitives.
 */
void *func_800ADCD0(void *packet_cursor, u32 *ordering_table, RECT *destination, RECT *texture)
{
    extern s32 g_menu_element_counter;
    s32 remaining_height;
    s32 remaining_width;
    s32 y_offset;
    s32 x_offset;
    s32 tile_height;
    s32 tile_width;
    u32 primitive_addr;
    s32 tag_length;
    s32 command;
    s16 clut;
    s32 color_word;
    s32 tag_length_mask;

    if (destination->w > 0)
    {
        if (destination->h > 0)
        {
            y_offset = 0;
            remaining_height = destination->h;
            color_word = 0x808080;
            tag_length = 4;
            command = 0x64;
            do
            {
                x_offset = 0;
                tile_height = remaining_height;
                if (texture->h < remaining_height)
                {
                    tile_height = texture->h;
                }
                remaining_width = destination->w;
                tag_length_mask = 0xFF000000;
                do
                {
                    tile_width = remaining_width;
                    if (texture->w < remaining_width)
                    {
                        tile_width = texture->w;
                    }

                    *(u32 *)&((SPRT *)packet_cursor)->r0 = color_word;
                    setlen((SPRT *)packet_cursor, tag_length);
                    setcode((SPRT *)packet_cursor, command);
                    ((SPRT *)packet_cursor)->x0 = (s16)((u16)destination->x + x_offset);
                    ((SPRT *)packet_cursor)->y0 = (s16)((u16)destination->y + y_offset);
                    ((SPRT *)packet_cursor)->u0 = (u8)texture->x;
                    ((SPRT *)packet_cursor)->v0 = (u8)texture->y;
                    ((SPRT *)packet_cursor)->w = tile_width;
                    ((SPRT *)packet_cursor)->h = tile_height;
                    clut = 0x7CD0;
                    if (g_menu_element_counter != 0)
                    {
                        clut = 0x7D10;
                    }
                    ((SPRT *)packet_cursor)->clut = clut;

                    ((SPRT *)packet_cursor)->tag = (((SPRT *)packet_cursor)->tag & tag_length_mask) | (*ordering_table & 0xFFFFFF);
                    primitive_addr = (u32)packet_cursor & 0xFFFFFF;
                    *ordering_table = (*ordering_table & tag_length_mask) | (primitive_addr & 0xFFFFFF);
                    packet_cursor = (u8 *)packet_cursor + sizeof(SPRT);

                    x_offset += tile_width;
                    remaining_width -= tile_width;
                } while (remaining_width != 0);

                remaining_height -= tile_height;
                y_offset += tile_height;
            } while (remaining_height != 0);
        }
        return packet_cursor;
    }
    return packet_cursor;
}

/**
 * @brief Load two fixed sub-images into VRAM from @c D_800EF1BC.
 *
 * Transfers a 0x10x2 block to VRAM (0x100, 0x1F3) from the base image data and
 * a 0x10x0x20 block to (0x110, 0x1E0) from the data 0x40 bytes further in.
 *
 * @see decomp.me (100%) TODO
 */
void func_800ADE2C(void)
{
    RECT rect;

    rect.x = 0x100;
    rect.y = 0x1F3;
    rect.w = 0x10;
    rect.h = 2;
    LoadImage(&rect, (u_long *)D_800EF1BC);
    rect.x = 0x110;
    rect.y = 0x1E0;
    rect.w = 0x10;
    rect.h = 0x20;
    LoadImage(&rect, (u_long *)(D_800EF1BC + 0x40));
}

/**
 * @brief Clear the menu-element counter and the low tag bits of all eight records.
 */
void func_800ADEB0(void)
{
    extern s32 g_menu_element_counter;
    extern UnkEntry80122828 D_80122828[];
    UnkEntry80122828 *p;
    s32 i;

    g_menu_element_counter = 0;
    p = D_80122828;
    for (i = 0; i < 8; i++)
    {
        p->word &= ~7;
        p++;
    }
}

/**
 * @brief Report whether any menu record is in a non-idle, non-active state.
 * @return 1 when a record has a set state other than 2, otherwise 0.
 */
s32 func_800ADEEC(void)
{
    extern RecADEEC D_80122828[];
    RecADEEC *p;
    s32 i;
    s32 x;

    p = D_80122828;
    for (i = 0; i < 8; i++, p++)
    {
        x = p->flags & 0x7;
        if (x != 0)
        {
            if (x != 2)
            {
                return 1;
            }
        }
    }
    return 0;
}

/**
 * @brief Force every active menu record into the closing state.
 */
void func_800ADF34(void)
{
    extern RecADEEC D_80122828[];
    RecADEEC *p;
    s32 i;
    u32 x;

    p = D_80122828;
    for (i = 0; i < 8; i++, p++) {
        x = p->flags;
        if (x & 7) {
            p->flags = (((x & ~7) | 3) & ~0x78) | 0x40;
        }
    }
}

/**
 * @brief Allocate the first idle menu record and initialize it to the opening state.
 * @return The claimed record, or the first record when none are free.
 */
FieldADF84Rec *func_800ADF84(void)
{
    extern FieldADF84Rec D_80122828[];
    FieldADF84Rec *rec;
    s32 i;

    rec = D_80122828;
    for (i = 0; i < 8; i++, rec++)
    {
        if ((rec->flags & 7) == 0)
        {
            rec->flags = (rec->flags & ~7) | 1;
            rec->unk8 = 0;
            rec->unkA = 0;
            rec->state &= ~0x200;
            rec->state &= ~0xC00;
            ((u16 *)&rec->state)[1] = 0;
            rec->unkC = 0;
            return rec;
        }
    }
    return D_80122828;
}

/**
 * @brief Update scrolling and emit drawing packets for the eight field menu elements.
 *
 * Active elements receive scroll markers and their draw-environment packet.
 * Each element callback emits its contents, followed by an opening, steady,
 * blinking, or closing border. The updated packet cursor returns to the context.
 *
 * @param context Ordering-table and primitive-buffer state for the current frame.
 */
void func_800AE008(FieldMenuRenderContext *context)
{
    extern FieldMenuElement D_80122828[];
    DRAWENV env;
    u32 marker_x, marker_width_low, opening_x, opening_width_low, active_width_low, closing_x,
        closing_width_low;
    u32 marker_attr;
    u32 marker_size;
    u32 input_size;
    s32 clamp_height;
    s16 scroll_ticks;
    u32 state_attr;
    u32 opening_geometry;
    s32 opening_height;
    s32 opening_width;
    s32 opening_step;
    s32 opening_width_product;
    s32 opening_height_product;
    u32 opening_updated_attr;
    u32 opening_previous_attr;
    u32 opening_draw_attr;
    u32 opening_draw_geometry;
    u32 active_geometry;
    s32 active_inset;
    u32 active_attr;
    u32 active_blink_attr;
    u32 closing_geometry;
    s32 closing_height;
    s32 closing_width;
    s32 closing_step;
    s32 closing_width_product;
    s32 closing_height_product;
    u32 closing_updated_attr;
    u32 closing_previous_attr;
    u32 closing_draw_attr;
    u32 closing_draw_geometry;
    s32 *cursor;
    FieldMenuRenderContext *ordering;
    FieldMenuElement *element;
    s32 element_index;
    u32 entry_attr, scroll_size;
    s32 mode;
    s32 visible_height, animated_width, animated_height;
    u16 scroll_target;
    s16 next_scroll_target;
    s32 movement;
    cursor = context->cursor;
    ordering = context;
    if (context->buffer)
    {
        SetDefDrawEnv(&env, 0, 0xF0, 0x140, 0xE0);
    }
    else
    {
        SetDefDrawEnv(&env, 0, 8, 0x140, 0xE0);
    }
    element = D_80122828;
    element_index = 0;
    for (; element_index < 8; element_index++, element++)
    {
        entry_attr = element->attr;
        if (entry_attr & 7)
        {
            scroll_size = element->size.word;
            if ((scroll_size >> 10) & 3)
            {
                if (element->scroll != 0)
                {
                    marker_x = (entry_attr >> 7) & 0x1ff;
                    marker_width_low = entry_attr >> 24;
                    cursor =
                        func_800AE76C(cursor, ordering,
                                      marker_x + (((scroll_size & 1) << 8) | marker_width_low) - 16,
                                      ((u8 *)element)[2] + 8, 1);
                }
                marker_size = element->size.word;
                visible_height = (marker_size >> 1) & 255;
                if (element->scroll + visible_height < element->size.fields.content_height)
                {
                    marker_attr = element->attr;
                    cursor =
                        func_800AE76C(cursor, ordering,
                                      ((marker_attr >> 7) & 0x1ff) +
                                          (((marker_size & 1) << 8) | (marker_attr >> 24)) - 16,
                                      ((u8 *)element)[2] + visible_height - 8, 0);
                }
                scroll_ticks = element->scroll_ticks;
                if (scroll_ticks != 0)
                {
                    movement = (element->scroll_target - element->scroll) / scroll_ticks;
                    /* Retain the original fresh halfword read after division. */
                    element->scroll_ticks = *(volatile u16 *)&element->scroll_ticks - 1;
                    element->scroll = (u16)element->scroll + movement;
                }
                else
                {
                    input_size = element->size.word;
                    scroll_target = element->scroll_target;
                    element->scroll = scroll_target;
                    if (((input_size >> 10) & 3) == 1)
                    {
                        if ((g_pad_input & 0x4000) &&
                            (s16)scroll_target + (s32)((input_size >> 1) & 255) <
                                element->size.fields.content_height)
                        {
                            func_800A3938(0x7D, 0x80);
                            next_scroll_target = (u16)element->scroll_target + 16;
                            /* Keep the scroll-target write observable before clamping. */
                            *(volatile s16 *)&element->scroll_target = next_scroll_target;
                            clamp_height = (element->size.word >> 1) & 255;
                            if (element->size.fields.content_height - clamp_height <
                                next_scroll_target)
                            {
                                element->scroll_target =
                                    element->size.fields.content_height - clamp_height;
                            }
                            element->scroll_ticks = 4;
                        }
                        else if ((g_pad_input & 0x1000) && element->scroll > 0)
                        {
                            func_800A3938(0x7D, 0x80);
                            next_scroll_target = (u16)element->scroll_target - 16;
                            /* Keep the scroll-target write observable before clamping. */
                            *(volatile s16 *)&element->scroll_target = next_scroll_target;
                            if (next_scroll_target < 0)
                            {
                                element->scroll_target = 0;
                            }
                            element->scroll_ticks = 4;
                        }
                    }
                }
            }
            SetDrawEnv((DR_ENV *)cursor, &env);
            *cursor = (*cursor & 0xFF000000) | (ordering->tag & 0xFFFFFF);
            ordering->tag = (ordering->tag & 0xFF000000) | ((s32)cursor & 0xFFFFFF);
            state_attr = element->attr;
            mode = state_attr & 7;
            cursor = (s32 *)((u8 *)cursor + 0x40);
            switch (mode)
            {
            case 1:
                opening_geometry = element->size.word;
                opening_width = ((opening_geometry & 1) << 8) | (state_attr >> 24);
                opening_step = (state_attr >> 3) & 15;
                opening_width_product = opening_width * opening_step;
                if (opening_width_product < 0)
                {
                    opening_width_product += 7;
                }
                opening_height = (opening_geometry >> 1) & 255;
                opening_height_product = opening_height * opening_step;
                animated_width = opening_width_product >> 3;
                if (opening_height_product < 0)
                {
                    opening_height_product += 7;
                }
                animated_height = opening_height_product >> 3;
                cursor = element->draw(ordering, cursor, (opening_width - animated_width) / 2,
                                       (opening_height - animated_height) / 2 + element->scroll,
                                       opening_height, element);
                opening_draw_attr = element->attr;
                opening_draw_geometry = element->size.word;
                opening_x = (opening_draw_attr >> 7) & 0x1ff;
                opening_width_low = opening_draw_attr >> 24;
                cursor = func_800AD850(
                    cursor, ordering,
                    opening_x + (s32)((((opening_draw_geometry & 1) << 8) | opening_width_low) -
                                      animated_width) /
                                    2,
                    ((u8 *)element)[2] +
                        (s32)(((opening_draw_geometry >> 1) & 255) - animated_height) / 2,
                    animated_width, animated_height, context->buffer, 0);
                opening_previous_attr = element->attr;
                opening_updated_attr = (opening_previous_attr & ~0x78) |
                                       (((((opening_previous_attr >> 3) & 15) + 1) & 15) * 8);
                element->attr = opening_updated_attr;
                if (((opening_updated_attr >> 3) & 15) == 8)
                {
                    element->attr = (opening_updated_attr & ~7) | 2;
                }
                break;
            case 2:
                active_geometry = element->size.word;
                if ((active_geometry >> 9) & 1)
                {
                    active_inset = 0;
                    if (!(g_frame_counter & 4))
                    {
                        active_inset = -2;
                    }
                    cursor = element->draw(ordering, cursor, active_inset,
                                           active_inset + element->scroll,
                                           (active_geometry >> 1) & 255, element);
                    active_blink_attr = element->attr;
                    active_width_low = active_blink_attr >> 24;
                    cursor = func_800AD850(
                        cursor, ordering, ((active_blink_attr >> 7) & 0x1ff) + active_inset,
                        ((u8 *)element)[2] + active_inset,
                        (((element->size.word & 1) << 8) | active_width_low) - active_inset * 2,
                        ((element->size.word >> 1) & 255) - active_inset * 2, context->buffer, 0);
                }
                else
                {
                    cursor = element->draw(ordering, cursor, 0, element->scroll,
                                           (active_geometry >> 1) & 255, element);
                    active_attr = element->attr;
                    active_width_low = active_attr >> 24;
                    cursor = func_800AD850(cursor, ordering, (active_attr >> 7) & 0x1ff,
                                           ((u8 *)element)[2],
                                           ((element->size.word & 1) << 8) | active_width_low,
                                           (element->size.word >> 1) & 255, context->buffer, 0);
                }
                break;
            case 3:
                closing_geometry = element->size.word;
                closing_width = ((closing_geometry & 1) << 8) | (state_attr >> 24);
                closing_step = (state_attr >> 3) & 15;
                closing_width_product = closing_width * closing_step;
                if (closing_width_product < 0)
                {
                    closing_width_product += 7;
                }
                closing_height = (closing_geometry >> 1) & 255;
                closing_height_product = closing_height * closing_step;
                animated_width = closing_width_product >> 3;
                if (closing_height_product < 0)
                {
                    closing_height_product += 7;
                }
                animated_height = closing_height_product >> 3;
                cursor = element->draw(ordering, cursor, (closing_width - animated_width) / 2,
                                       (closing_height - animated_height) / 2 + element->scroll,
                                       closing_height, element);
                closing_draw_attr = element->attr;
                closing_draw_geometry = element->size.word;
                closing_x = (closing_draw_attr >> 7) & 0x1ff;
                closing_width_low = closing_draw_attr >> 24;
                cursor = func_800AD850(
                    cursor, ordering,
                    closing_x + (s32)((((closing_draw_geometry & 1) << 8) | closing_width_low) -
                                      animated_width) /
                                    2,
                    ((u8 *)element)[2] +
                        (s32)(((closing_draw_geometry >> 1) & 255) - animated_height) / 2,
                    animated_width, animated_height, context->buffer, 0);
                closing_previous_attr = element->attr;
                closing_updated_attr = (closing_previous_attr & ~0x78) |
                                       (((((closing_previous_attr >> 3) & 15) - 1) & 15) * 8);
                element->attr = closing_updated_attr;
                if (((closing_updated_attr >> 3) & 15) == 0)
                {
                    element->attr = closing_updated_attr & ~7;
                }
                break;
            }
        }
    }
    context->cursor = cursor;
}

/**
 * @brief Build a clipped menu primitive and append its draw-mode packet to the ordering table.
 * @param packet_buffer Current GPU packet cursor.
 * @param ordering_table Ordering-table entry to link the packet into.
 * @param x Source rectangle x coordinate before the four-pixel inset.
 * @param y Source rectangle y coordinate before the eight-pixel inset.
 * @param alternate Select the menu-counter offset used for the destination rectangle.
 * @return GPU packet cursor advanced past the emitted draw-mode packet.
 */
u_long *func_800AE76C(u_long *packet_buffer, u_long *ordering_table, s32 x, s32 y, s32 alternate)
{
    extern u16 g_menu_element_counter;
    u_long *packet;
    RECT source;
    RECT destination;

    packet = packet_buffer;
    setRECT(&source, x - 4, y - 8, 8, 0x10);

    if (alternate != 0)
    {
        setRECT(&destination, g_menu_element_counter + 0x48, 0xE8, 8, 0x10);
    }
    else
    {
        setRECT(&destination, g_menu_element_counter + 0x50, 0xE8, 8, 0x10);
    }

    packet = func_800ADCD0(packet, ordering_table, &source, &destination);
    setDrawTPage((DR_TPAGE *)packet, 0, 0, 0x14);
    addPrim(ordering_table, packet);
    return packet + 2;
}

/**
 * @brief Count printable glyphs in a menu string, treating a wide-glyph range as two bytes.
 * @param str NUL-terminated menu string.
 * @return Glyph count.
 */
s32 func_800AE864(u8 *str)
{
    s32 count;
    s32 c;

    count = 0;
    c = str[0];
    if (c != 0)
    {
        do
        {
            if ((u32) (c - 0x19) < 7)
            {
                str += 2;
            }
            else
            {
                str += 1;
            }
            c = str[0];
            count++;
        } while (c != 0);
    }
    return count;
}
