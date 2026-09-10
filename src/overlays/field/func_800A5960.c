#include "common.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
void func_80086F48(const void *, s16);
s32 rcos(s32);
s32 rsin(s32);
/** @brief Position, texture coordinates, dimensions and effect flags of one quad. */
typedef struct
{
    u16 x, y;
    u8 u, v;
    u16 width, height, flags;
} FieldTransitionQuad;
extern u8 D_800EF124[];
extern u16 D_8011F3D0;
extern u16 D_80122904;

/**
 * @brief Append up to four textured quads for a field transition effect.
 * @param packet Next available primitive packet.
 * @param ordering_table Ordering-table entry receiving each generated primitive.
 * @param index Selects a group of four transition descriptors.
 * @return First unused packet after the generated textured quads.
 * @note Effect flags select expansion, translation, rotation or brightness;
 *       disabled descriptors have X equal to 0xFFFF.
 */
s32 *func_800A5960(s32 *packet, s32 *ordering_table, s32 index)
{
    u32 half_width;
    u16 *alpha;
    s32 quad_index;
    s16 temp_v0_2;
    s16 temp_v0_5;
    s16 temp_v0_8;
    s16 var_v0_3;
    s32 temp_a0;
    s32 temp_a0_2;
    s32 temp_a0_3;
    s32 temp_a0_4;
    s32 temp_a2;
    s32 temp_a2_2;
    s32 temp_a2_3;
    s32 temp_a2_4;
    s32 temp_a3;
    s32 temp_lo;
    s32 temp_lo_2;
    s32 temp_lo_3;
    s32 temp_s0;
    s32 temp_s2;
    s32 expansion;
    s32 angle;
    s32 negative_width;
    s32 temp_v1_3;
    s32 temp_v1_4;
    s32 temp_v1_6;
    s32 var_a0;
    s32 var_a0_2;
    s32 var_a0_3;
    s32 var_a0_4;
    s32 var_a0_5;
    s32 var_a1;
    s32 var_v0;
    s32 var_v0_2;
    s32 var_v0_4;
    s32 var_v0_5;
    s32 var_v0_6;
    s32 var_v0_7;
    s32 var_v0_8;
    s32 var_v1_2;
    s32 var_v1_3;
    s32 var_v1_4;
    s32 var_v1_5;
    s32 var_v1_6;
    s32 var_v1_7;
    s32 var_v1_8;
    s32 var_v1_9;
    s8 temp_v0_11;
    s8 temp_v0_13;
    u16 *quad_x;
    u16 temp_v0;
    u16 temp_v0_3;
    u16 temp_v0_4;
    u16 temp_v0_6;
    u16 temp_v0_7;
    u16 temp_v0_9;
    u16 temp_v1_2;
    u16 temp_v1_5;
    u16 var_v1;
    u32 temp_a3_2;
    u32 width;
    u32 temp_v1;
    u8 temp_v0_10;
    u8 temp_v0_12;
    u8 var_v0_9;
    u8 *packet_tpage;
    u8 *quad_flags;

    quad_x = (u16 *)((index * 0x30) + D_800EF124);
    quad_flags = (u8 *)quad_x + 0xA;
    packet_tpage = (u8 *)packet + 0x16;
    quad_index = 0;
draw_next:
{
    if (*quad_x != 0xFFFF)
    {
        var_v1 = *(u16 *)(quad_flags + (-4));
        if (*(u16 *)(quad_flags + (0)) & 0xF)
        {
            var_v1 = (u16)(var_v1 >> 1);
        }
        *(u8 *)(packet_tpage + (-19)) = 9;
        *(u8 *)(packet_tpage + (-15)) = 0x2CU;
        width = var_v1 & 0xFFFF;
        alpha = &D_8011F3D0;
        *(u8 *)(packet_tpage + (-18)) = *(u8 *)(packet_tpage + (-17)) = *(u8 *)(packet_tpage + (-16)) = *(u8 *)alpha;
        if ((*alpha != 0x80) || (*(u16 *)(quad_flags + (0)) & 0x100))
        {
            *(u8 *)(packet_tpage + (-15)) = (u8)(*(u8 *)(packet_tpage + (-15)) | 2);
        }
        expansion = 0x80 - *alpha;
        temp_v1 = ((u16) * (u16 *)(quad_flags + (0)) >> 0xB) & 0xF;
        switch (temp_v1)
        {
        case 0:
            temp_lo = width * expansion;
            var_a0 = temp_lo;
            if (temp_lo < 0)
            {
                var_a0 = temp_lo + 0x7F;
            }
            var_a0_2 = var_a0 >> 7;
            goto block_52;
        case 1:
            if ((u16)D_80122904 < 0x25U)
            {
                temp_lo_2 = width * expansion;
                var_v1_2 = temp_lo_2;
                if (temp_lo_2 < 0)
                {
                    var_v1_2 = temp_lo_2 + 0x7F;
                }
                temp_v1_3 = var_v1_2 >> 7;
                temp_v0_3 = *quad_x - temp_v1_3;
                *(s16 *)(packet_tpage + (2)) = temp_v0_3;
                *(s16 *)(packet_tpage + (-14)) = temp_v0_3;
                var_v0_4 = (*quad_x + width) - temp_v1_3;
            }
            else
            {
                temp_lo_2 = width * expansion;
                var_v1_3 = temp_lo_2;
                if (temp_lo_2 < 0)
                {
                    var_v1_3 = temp_lo_2 + 0x7F;
                }
                temp_v1_4 = var_v1_3 >> 7;
                temp_v0_4 = *quad_x + temp_v1_4;
                *(s16 *)(packet_tpage + (2)) = temp_v0_4;
                *(s16 *)(packet_tpage + (-14)) = temp_v0_4;
                var_v0_4 = *quad_x + width + temp_v1_4;
            }
            temp_v0_5 = var_v0_4 - 1;
            *(s16 *)(packet_tpage + (10)) = temp_v0_5;
            *(s16 *)(packet_tpage + (-6)) = temp_v0_5;
            temp_v0_6 = *(u16 *)(quad_flags + (-8));
            *(s16 *)(packet_tpage + (-4)) = temp_v0_6;
            *(s16 *)(packet_tpage + (-12)) = temp_v0_6;
            var_v0_2 = *(u16 *)(quad_flags + (-2)) + *(u16 *)(quad_flags + (-8));
            goto block_57;
        case 2:
            angle = expansion * 0x32;
            temp_s0 = rcos(angle);
            temp_a0 = rsin(angle);
            var_v1_4 = width * temp_s0;
            if (var_v1_4 < 0)
            {
                var_v1_4 += 0x1FFF;
            }
            var_v1_4 >>= 13;
            var_v0_5 = *(u16 *)(quad_flags + (-2)) * temp_a0;
            if (var_v0_5 < 0)
            {
                var_v0_5 += 0x1FFF;
            }
            temp_s2 = (var_v1_4) + (var_v0_5 >> 0xD);
            temp_s0 = rsin(angle);
            temp_a0_2 = rcos(angle);
            var_v1_5 = width * temp_s0;
            if (var_v1_5 < 0)
            {
                var_v1_5 += 0x1FFF;
            }
            var_v1_5 >>= 13;
            var_v0_6 = *(u16 *)(quad_flags + (-2)) * temp_a0_2;
            if (var_v0_6 < 0)
            {
                var_v0_6 += 0x1FFF;
            }
            var_a0_4 = expansion * temp_s2;
            temp_a2 = (var_v1_5) - (var_v0_6 >> 0xD);
            if (var_a0_4 < 0)
            {
                var_a0_4 += 0x7F;
            }
            var_v1_6 = expansion * temp_a2;
            temp_s2 = temp_s2 + (var_a0_4 >> 7);
            if (var_v1_6 < 0)
            {
                var_v1_6 += 0x7F;
            }
            temp_a2_2 = temp_a2 + (var_v1_6 >> 7);
            temp_a3_2 = width >> 1;
            half_width = temp_a3_2;
            *(s16 *)(packet_tpage + (-14)) = (u16)((*quad_x + temp_a3_2) - temp_s2);
            *(s16 *)(packet_tpage + (10)) = (s16)(*quad_x + temp_a3_2 + temp_s2);
            *(s16 *)(packet_tpage + (-12)) =
                (u16)(*(u16 *)(quad_flags + (-8)) + ((u16) * (u16 *)(quad_flags + (-2)) >> 1) + temp_a2_2);
            *(s16 *)(packet_tpage + (12)) =
                (s16)((*(u16 *)(quad_flags + (-8)) + ((u16) * (u16 *)(quad_flags + (-2)) >> 1)) - temp_a2_2);
            temp_s0 = rcos(angle);
            negative_width = -(s32)width;
            temp_a0_3 = rsin(angle);
            var_v1_7 = negative_width * temp_s0;
            if (var_v1_7 < 0)
            {
                var_v1_7 += 0x1FFF;
            }
            var_v1_7 >>= 13;
            var_v0_7 = *(u16 *)(quad_flags + (-2)) * temp_a0_3;
            if (var_v0_7 < 0)
            {
                var_v0_7 += 0x1FFF;
            }
            temp_s2 = (var_v1_7) + (var_v0_7 >> 0xD);
            temp_s0 = rsin(angle);
            temp_a0_4 = rcos(angle);
            var_v1_8 = negative_width * temp_s0;
            if (var_v1_8 < 0)
            {
                var_v1_8 += 0x1FFF;
            }
            var_v1_8 >>= 13;
            var_v0_8 = *(u16 *)(quad_flags + (-2)) * temp_a0_4;
            if (var_v0_8 < 0)
            {
                var_v0_8 += 0x1FFF;
            }
            var_a1 = expansion * temp_s2;
            temp_a2_3 = (var_v1_8) - (var_v0_8 >> 0xD);
            if (var_a1 < 0)
            {
                var_a1 += 0x7F;
            }
            var_v1_9 = expansion * temp_a2_3;
            temp_s2 = temp_s2 + (var_a1 >> 7);
            if (var_v1_9 < 0)
            {
                var_v1_9 += 0x7F;
            }
            *(s16 *)(packet_tpage + (-6)) = (s16)((*quad_x + half_width) - temp_s2);
            temp_a2_4 = temp_a2_3 + (var_v1_9 >> 7);
            *(s16 *)(packet_tpage + (2)) = (u16)(*quad_x + half_width + temp_s2);
            *(s16 *)(packet_tpage + (-4)) =
                (u16)(*(u16 *)(quad_flags + (-8)) + ((u16) * (u16 *)(quad_flags + (-2)) >> 1) + temp_a2_4);
            var_v0_3 = (*(u16 *)(quad_flags + (-8)) + ((u16) * (u16 *)(quad_flags + (-2)) >> 1)) - temp_a2_4;
            goto block_58;
        case 3:
            if (*alpha < 0x40U)
            {
                var_v0_9 = *alpha * 4;
            }
            else
            {
                var_v0_9 = ~((*alpha - 0x40) * 2);
            }
            *(u8 *)(packet_tpage + (-16)) = var_v0_9;
            *(u8 *)(packet_tpage + (-17)) = var_v0_9;
            *(u8 *)(packet_tpage + (-18)) = var_v0_9;
            temp_v0_7 = *quad_x;
            *(s16 *)(packet_tpage + (2)) = temp_v0_7;
            *(s16 *)(packet_tpage + (-14)) = temp_v0_7;
            temp_v0_8 = (*quad_x + width) - 1;
            *(s16 *)(packet_tpage + (10)) = temp_v0_8;
            *(s16 *)(packet_tpage + (-6)) = temp_v0_8;
            temp_v0_9 = *(u16 *)(quad_flags + (-8));
            *(s16 *)(packet_tpage + (-4)) = temp_v0_9;
            *(s16 *)(packet_tpage + (-12)) = temp_v0_9;
            var_v0_2 = *(u16 *)(quad_flags + (-2)) + *(u16 *)(quad_flags + (-8));
            goto block_57;
        case 4:
            temp_lo_3 = width * expansion;
            var_a0_5 = temp_lo_3;
            if (temp_lo_3 < 0)
            {
                var_a0_5 = temp_lo_3 + 0x3F;
            }
            var_a0_2 = var_a0_5 >> 6;
        block_52:
            temp_v0 = *quad_x - var_a0_2;
            *(s16 *)(packet_tpage + (2)) = temp_v0;
            *(s16 *)(packet_tpage + (-14)) = temp_v0;
            temp_v0_2 = (*quad_x + width + var_a0_2) - 1;
            *(s16 *)(packet_tpage + (10)) = temp_v0_2;
            *(s16 *)(packet_tpage + (-6)) = temp_v0_2;
            var_v0 = *(u16 *)(quad_flags + (-2)) * expansion;
            if (var_v0 < 0)
            {
                var_v0 += 0x7F;
            }
            temp_v1_2 = *(u16 *)(quad_flags + (-8)) - (var_v0 >> 7);
            *(s16 *)(packet_tpage + (-4)) = temp_v1_2;
            *(s16 *)(packet_tpage + (-12)) = temp_v1_2;
            var_a0_3 = *(u16 *)(quad_flags + (-2)) * expansion;
            if (var_a0_3 < 0)
            {
                var_a0_3 += 0x7F;
            }
            var_v0_2 = *(u16 *)(quad_flags + (-8)) + *(u16 *)(quad_flags + (-2)) + (var_a0_3 >> 7);
        block_57:
            var_v0_3 = var_v0_2 - 1;
            *(s16 *)(packet_tpage + (12)) = var_v0_3;
        block_58:
            *(s16 *)(packet_tpage + (4)) = var_v0_3;
            break;
        }
        temp_v0_10 = *(u8 *)(quad_flags + (-6));
        *(u8 *)(packet_tpage + (6)) = temp_v0_10;
        *(u8 *)(packet_tpage + (-10)) = temp_v0_10;
        temp_v0_11 = (*(u8 *)(quad_flags + (-6)) + width) - 1;
        *(u8 *)(packet_tpage + (14)) = temp_v0_11;
        *(u8 *)(packet_tpage + (-2)) = temp_v0_11;
        temp_v0_12 = *(u8 *)(quad_flags + (-5));
        *(u8 *)(packet_tpage + (-1)) = temp_v0_12;
        *(u8 *)(packet_tpage + (-9)) = temp_v0_12;
        temp_v0_13 = (*(u8 *)(quad_flags + (-5)) + (u8) * (u16 *)(quad_flags + (-2))) - 1;
        *(u8 *)(packet_tpage + (15)) = temp_v0_13;
        *(u8 *)(packet_tpage + (7)) = temp_v0_13;
        *(s16 *)(packet_tpage + (-8)) = (s16)(((u32)(*(u16 *)(quad_flags + (0)) & 0xF0) >> 4) | 0x7C80);
        temp_v1_5 = *(u16 *)(quad_flags + (0));
        *(s16 *)(packet_tpage + (0)) = (s16)(((temp_v1_5 & 3) << 7) | ((temp_v1_5 >> 4) & 0x60) | 5);
        *packet = (*packet & 0xFF000000) | (*ordering_table & 0xFFFFFF);
        *ordering_table = (*ordering_table & 0xFF000000) | ((s32)packet & 0xFFFFFF);
        if ((*alpha < 0x7CU) && ((temp_v1_6 = ((u16) * (u16 *)(quad_flags + (0)) >> 0xB) & 0xF, (temp_v1_6 == 1)) ||
                                 ((temp_v1_6 >= 2) && (temp_v1_6 == 2))))
        {
            func_80086F48(packet, 0);
        }
        packet_tpage += 0x28;
        packet += 10;
    }
    quad_flags += 0xC;
    quad_x += 6;
    temp_a3 = quad_index + 1;
    quad_index = temp_a3;
}
    if (temp_a3 < 4)
    {
        goto draw_next;
    }
    return packet;
}
