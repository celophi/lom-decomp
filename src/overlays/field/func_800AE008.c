#include "common.h"
#include "sdk/libgpu.h"
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
extern FieldMenuElement D_80122828[];
extern s32 g_pad_input, g_frame_counter;
void func_800A3938(s32, s32);
s32 *func_800AE76C(s32 *, FieldMenuRenderContext *, s32, s32, s32);
s32 *func_800AD850(s32 *, FieldMenuRenderContext *, s32, s32, s32, s32, s32, s32);
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
