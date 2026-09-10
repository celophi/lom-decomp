#include "common.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
/** @brief Byte access in a partially recovered field record. */
#define U8_AT(p, o) (*(u8 *)((s32)(p) + (o)))
/** @brief Halfword access in the text-offset tables or controller sample. */
#define U16_AT(p, o) (*(u16 *)((s32)(p) + (o)))
/** @brief Word access in the render context, actor slot, or position record. */
#define S32_AT(p, o) (*(s32 *)((s32)(p) + (o)))
void *func_80086184(SPRT *, u32 *, s32, u32 *);
s32 func_800A88A0(s32, s32, void *, s32, s32, s32, s32);
void *func_800AD208(s32 *, void *, s32, s32, u16 *, s32);
void *func_800AD524(u8 *, s32 *, s32, s32 *, s32);
s32 func_800AE864(u8 *);
extern u8 D_800EC2FC[], D_800EC3C4[], D_800EC3E0[], D_800EC3E6[], D_800EC3E8[];
extern u8 D_800ED064[], D_800EDBE4[], D_800FD818[], D_800FDF58[], D_800FE3A0[], D_80105AE0[],
    D_8010A028[], D_8010A038[], D_801226E0[], D_801228D0[], D_801228E0[];
extern s32 D_800F22A0, D_800F22A4, D_800F22A8, g_pad_ctx;
extern u8 D_8011F3D2, D_801227D0;
/** @brief Packed screen coordinates consumed by the sprite builders. */
typedef struct
{
    s16 x;
    s16 y;
} FieldLabelPosition;

/**
 * @brief Draw controller action hints and labels for the selectable field actors.
 * @param context Rendering context with the primitive cursor at offset 0x40B8.
 * @note Action labels use the first active button among eight mapped controls.
 * @note Selected actors receive a raised ordering-table entry and highlight palette.
 * @note Access widths and record strides are retained for partially known layouts.
 */
void func_800A939C(void *context)
{
    s32 pad_offset;
    s32 custom_text_offset;
    FieldLabelPosition point;
    u8 *object_record;
    s32 text_style;
    s32 swapped_buttons;
    s32 buttons;
    s32 text_address;
    s32 text_base;
    s32 local_pad_offset;
    s32 local_text_offset;
    s32 screen_y;
    s32 camera_x_pixels;
    s32 label_ot;
    s32 label_half_width;
    s32 text_index;
    s32 actor_x;
    s32 camera_height;
    s32 number_ot;
    s32 camera_x;
    s32 camera_y;
    s32 text_ot;
    s32 left_glyph_ot;
    s32 right_glyph_ot;
    s32 button_index;
    s32 button_mask_or_y_offset;
    s32 index;
    s32 primitive;
    s32 action_offset;
    s32 actor_y;
    s32 actor_height;
    u16 raw_buttons;
    u8 *default_label_offset;
    s32 action_code;
    s32 actor_id;
    s32 action_slot;
    s32 secondary_action;
    s32 secondary_action_alt;
    s32 text_low_or_base;
    s32 pad_record;
    void *actor_slot;
    void *highlight_motion;
    void *actor_position;
    void *normal_motion;
    void *pad_sample;
    s32 text_high_or_offset;

    label_ot = (s32)context + 0x3C;
    index = 0;
    default_label_offset = D_800EC3E0;
    object_record = D_800FD818;
    action_offset = index;
    primitive = S32_AT(context, 0x40B8);
    pad_sample = (void *)0x801ED600;
    custom_text_offset = 0x5F0;
    pad_offset = 0;
    do
    {
        if ((U8_AT(object_record, 0x0) & 1) && ((u8)U8_AT(pad_sample, 0x0) < 0xFEU))
        {
            text_base = (s32)D_800EC3C4;
            button_mask_or_y_offset = 1;
            button_index = 0;
            local_pad_offset = pad_offset;
            raw_buttons = U16_AT(pad_sample, 0x2);
            local_text_offset = custom_text_offset;
            swapped_buttons = ((raw_buttons << 8) & 0xFF00) | (raw_buttons >> 8);
            buttons = (((u32)(swapped_buttons & 0x40) >> 1) | ((swapped_buttons & 0x20) * 2) |
                       ((u32)(swapped_buttons & 0x80) >> 3) | ((swapped_buttons & 0x10) * 8) |
                       (swapped_buttons & 0xFF0F));
        scan_buttons:
            if (buttons & button_mask_or_y_offset)
            {
                pad_record = g_pad_ctx + local_pad_offset;
                action_slot = U8_AT(pad_record + D_800EC2FC[button_index], 0x638);
                do
                {
                    switch (action_slot)
                    {
                    case 2:
                    case 3:
                        text_low_or_base = (s32)D_800EDBE4;
                        text_index = U16_AT(D_8010A028, action_offset + action_slot * 8) & 0x7FFF;
                        goto read_text_offset;

                    case 0:
                        if (((U8_AT(pad_record, 0x608) & 0x7F) == 2) &&
                            ((secondary_action = U8_AT(pad_record, 0x609),
                              (secondary_action == 5)) ||
                             (secondary_action == 8)))
                        {
                            text_high_or_offset = (default_label_offset[1] << 8) + text_base;
                            text_low_or_base = D_800EC3E0[0];
                        }
                        else
                        {
                            text_high_or_offset = (D_800EC3E6[1] << 8) + text_base;
                            text_low_or_base = D_800EC3E6[0];
                        }

                        break;
                    case 1:
                        if (((U8_AT(pad_record, 0x608) & 0x7F) == 2) &&
                            ((secondary_action_alt = U8_AT(pad_record, 0x609),
                              (secondary_action_alt == 5)) ||
                             (secondary_action_alt == 8)))
                        {
                            text_high_or_offset = (default_label_offset[1] << 8) + text_base;
                            text_low_or_base = D_800EC3E0[0];
                        }
                        else
                        {
                            text_high_or_offset = (D_800EC3E8[1] << 8) + text_base;
                            text_low_or_base = D_800EC3E8[0];
                        }
                        break;
                    default:
                        action_code = U8_AT(g_pad_ctx + local_pad_offset + action_slot, 0x608);
                        if (action_code == 0xFF)
                        {
                            text_high_or_offset = (default_label_offset[1] << 8) + text_base;
                            text_low_or_base = D_800EC3E0[0];
                        }
                        else if (action_code & 0x80)
                        {
                            text_low_or_base = g_pad_ctx + local_text_offset;
                            text_high_or_offset = ((action_code & 0xFF7F) << 6) + 0x150;
                        }
                        else
                        {
                            text_index =
                                (U16_AT(D_8010A038, action_offset + action_slot * 8) & 0x7FFF) +
                                (U8_AT(object_record, 0x1) * 0x18);
                            text_low_or_base = (s32)D_800ED064;
                        read_text_offset:
                            text_high_or_offset = U16_AT(text_low_or_base, text_index * 2);
                        }

                        break;
                    }
                    text_address = text_high_or_offset + text_low_or_base;
                } while (0);
                button_mask_or_y_offset = index << 5;
                point.x = 0x60;
                point.y = button_mask_or_y_offset + 0x3C;

                primitive = func_800A88A0(
                    (s32)func_80086184((SPRT *)primitive, (u32 *)label_ot, index, (u32 *)&point),
                    label_ot, (void *)text_address, 4, 0x80, button_mask_or_y_offset + 0x40, 0x80);
            }
            else
            {
                button_index += 1;
                button_mask_or_y_offset *= 2;
                if (button_index < 8)
                {
                    goto scan_buttons;
                }
            }
        }
        object_record += 0x268;
        action_offset += 0x190;
        pad_sample += 0xAE;
        index += 1;
        custom_text_offset += 0x250;
        pad_offset += 0x250;
    } while (index < 2);
    index = 0;
    if (D_801227D0 != 0)
    {
        do
        {
            actor_id = D_801226E0[index];
            camera_x = D_800F22A0;
            actor_slot = (void *)((actor_id * 0x23C) + (s32)D_80105AE0);
            actor_position = (void *)((actor_id * 0x54) + (s32)D_800FDF58);
            if (camera_x < 0)
            {
                camera_x += 0xFF;
            }
            actor_x = S32_AT(actor_position, 0x0);
            camera_x_pixels = camera_x >> 8;
            if (actor_x < 0)
            {
                actor_x += 0xFF;
            }
            camera_y = D_800F22A4;
            point.x = camera_x_pixels + ((actor_x >> 8) + 0xA0);
            if (camera_y < 0)
            {
                camera_y += 0xFF;
            }
            actor_y = S32_AT(actor_position, 0x4);
            if (actor_y < 0)
            {
                actor_y += 0xFF;
            }
            actor_height = S32_AT(actor_position, 0x8);
            screen_y = (camera_y >> 8) + ((actor_y >> 8) + 0x70);
            if (actor_height < 0)
            {
                actor_height += 0x1FF;
            }
            camera_height = D_800F22A8;
            if (camera_height < 0)
            {
                camera_height += 0x1FF;
            }
            point.y = (screen_y - (actor_height >> 9)) - (camera_height >> 9);
            label_half_width = func_800AE864((u8 *)S32_AT(actor_slot, 0x64)) * 6;
            if ((point.x + label_half_width) >= 0x141)
            {
                point.x = 0x140 - label_half_width;
            }
            if (((point.x - label_half_width) - 8) < 0)
            {
                point.x = label_half_width + 8;
            }
            if (point.y >= 0xB1)
            {
                point.y = 0xB0;
            }
            if (point.y < 0x32)
            {
                point.y = 0x32;
            }
            text_ot = label_ot;
            if (D_8011F3D2 == index)
            {
                text_ot = label_ot - 4;
            }
            text_style = 5;
            if (D_8011F3D2 == index)
            {
                text_style = 4;
            }
            primitive = func_800A88A0(primitive, text_ot, (void *)S32_AT(actor_slot, 0x64),
                                      text_style, (s32)point.x, (s32)point.y, 0x82);
            left_glyph_ot = label_ot;
            point.y = (u16)point.y - 8;
            if (D_8011F3D2 == index)
            {
                left_glyph_ot = label_ot - 4;
            }
            primitive = (s32)func_800AD524((u8 *)primitive, (s32 *)left_glyph_ot, 0xC,
                                           (s32 *)&point, D_8011F3D2 == index ? 0x81 : 0x82);
            right_glyph_ot = label_ot;
            point.x = (u16)point.x + 8;
            if (D_8011F3D2 == index)
            {
                right_glyph_ot = label_ot - 4;
            }
            primitive = (s32)func_800AD524((u8 *)primitive, (s32 *)right_glyph_ot, 0xD,
                                           (s32 *)&point, D_8011F3D2 == index ? 0x81 : 0x82);
            number_ot = label_ot;
            point.x = (u16)point.x + 8;
            if (D_8011F3D2 == index)
            {
                number_ot -= 4;
            }
            if (D_8011F3D2 == index)
            {
            }
            primitive = (s32)func_800AD208((s32 *)number_ot, (void *)primitive,
                                           (u8)U8_AT(actor_slot, 0x4C) >> 1, 2, (u16 *)&point,
                                           D_8011F3D2 == index ? 0x81 : 0x82);
            if ((D_8011F3D2 == index) && (S32_AT(actor_slot, 0x8) >= 0))
            {
                highlight_motion = (void *)((actor_id * 0x48) + (s32)D_800FE3A0);
                U8_AT(highlight_motion, 0x2E) = 0x80;
                U8_AT(highlight_motion, 0x33) = 0x80;
            }
            else
            {
                normal_motion = (void *)((actor_id * 0x48) + (s32)D_800FE3A0);
                U8_AT(normal_motion, 0x2E) = (u8)D_801228D0[index];
                U8_AT(normal_motion, 0x33) = (u8)D_801228E0[index];
            }
            index += 1;
        } while (index < (s32)D_801227D0);
    }
    S32_AT(context, 0x40B8) = primitive;
}
