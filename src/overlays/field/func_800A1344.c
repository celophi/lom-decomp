#include "common.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
/**
 * @brief Vector and matrix scratch area used by the GTE strip renderer.
 * @note The input is at workspace+0x30 and the active matrix at+0x78;
 *       unused vector/matrix slots preserve the original scratch layout.
 */
typedef struct
{
    volatile VECTOR vectors[3];
    SVECTOR input;
    MATRIX matrices[5];
} FieldStripWorkspace;
extern s32 D_800F22A0, D_800F22A4, D_800F22A8, D_801178D8;
/**
 * @brief Draw four rotating strips of Gouraud-shaded quads around a position.
 * @param ordering_table Depth ordering table with 4096 entries.
 * @param packet First free primitive packet.
 * @param position Fixed-point world-space origin of the effect.
 * @param slope Direction parameter converted into a Y rotation angle.
 * @param facing Selects addition or subtraction of the rotated X offset.
 * @return First free packet following all quads and the draw-page command.
 * @note Byte cursors point at POLY_G4's code byte; full word copies carry packed
 *       X/Y pairs into the next segment and close the strip.
 */
u8 *func_800A1344(s32 *ordering_table, u8 *packet, VECTOR *position, s32 slope, s32 facing)
{
    s32 saved_mask;
    MATRIX *matrix;
    FieldStripWorkspace work;
    s32 second_xy;
    s32 first_xy;
    s32 angle;
    s32 strip_index;
    s32 *temp_v0;
    s32 *temp_v1_4;
    s32 *temp_v1_8;
    u8 *next_packet;
    u8 *current_packet;
    s32 temp_a0;
    s32 temp_a0_2;
    s32 temp_a1;
    s32 temp_t0;
    s32 temp_v1;
    s32 temp_v1_2;
    s32 temp_v1_3;
    s32 temp_v1_5;
    s32 temp_v1_6;
    s32 temp_v1_7;
    s32 temp_v1_9;
    s32 var_a0;
    s32 var_a0_2;
    s32 var_a0_3;
    s32 var_a0_4;
    s32 var_a0_5;
    s32 var_a0_6;
    s32 var_a0_7;
    s32 var_a0_8;
    s32 outer_mask;
    s32 tag_mask;
    s32 segment_index;
    s32 radius;
    s32 radius_sum;
    s32 var_v0;
    s32 var_v0_2;
    s32 var_v0_3;
    s32 var_v0_4;
    s32 var_v0_5;
    s32 var_v0_6;
    s32 var_v0_7;
    s32 var_v0_8;
    s32 var_v1;
    s32 var_v1_10;
    s32 var_v1_11;
    s32 var_v1_12;
    s32 var_v1_2;
    s32 var_v1_3;
    s32 var_v1_4;
    s32 var_v1_5;
    s32 var_v1_6;
    s32 var_v1_7;
    s32 var_v1_8;
    s32 var_v1_9;
    u8 *segment_code;
    u8 *strip_code;

    current_packet = packet;
    angle = ratan2(slope, 0x64);
    outer_mask = 0xFF000000;
    radius = D_801178D8;
    strip_code = current_packet + 7;
    strip_index = 0;
    matrix = &work.matrices[2];
next_strip:
{
    ((s32 *)&work.matrices[2])[4] = 0x1000;
    ((s32 *)&work.matrices[2])[2] = 0x1000;
    ((s32 *)&work.matrices[2])[0] = 0x1000;
    ((s32 *)&work.matrices[2])[7] = 0;
    ((s32 *)&work.matrices[2])[6] = 0;
    ((s32 *)&work.matrices[2])[5] = 0;
    ((s32 *)&work.matrices[2])[3] = 0;
    ((s32 *)&work.matrices[2])[1] = 0;
    saved_mask = outer_mask;
    RotMatrixY(angle, matrix);
    work.input.vx = (s16)radius;
    work.input.vy = 0;
    work.input.vz = 0;
    gte_SetRotMatrix(matrix);
    gte_ldv0(&work.input);
    gte_rtv0();
    gte_stlvnl(&work.vectors[0]);
    tag_mask = outer_mask;
    if (facing != 0)
    {
        var_v1 = position->vx + (work.vectors[0].vx << 8);
    }
    else
    {
        var_v1 = position->vx - (work.vectors[0].vx << 8);
    }
    work.vectors[1].vx = var_v1;
    work.vectors[1].vy = position->vy + (work.vectors[0].vy << 8);
    work.vectors[1].vz = position->vz + (work.vectors[0].vz << 8);
    var_v0 = D_800F22A0;
    if (var_v0 < 0)
    {
        var_v0 += 0xFF;
    }
    var_v0 >>= 8;
    var_v1_2 = work.vectors[1].vx;
    if (var_v1_2 < 0)
    {
        var_v1_2 += 0xFF;
    }
    var_v1_2 >>= 8;
    var_a0 = D_800F22A4;
    *(s16 *)(strip_code + (1)) = (s16)((var_v0) + ((var_v1_2) + 0xA0));
    if (var_a0 < 0)
    {
        var_a0 += 0xFF;
    }
    var_a0 >>= 8;
    var_v0_2 = work.vectors[1].vy;
    if (var_v0_2 < 0)
    {
        var_v0_2 += 0xFF;
    }
    var_v0_2 >>= 8;
    var_a0_2 = work.vectors[1].vz;
    if (var_a0_2 < 0)
    {
        var_a0_2 += 0x1FF;
    }
    var_a0_2 >>= 9;
    var_v1_3 = D_800F22A8;
    if (var_v1_3 < 0)
    {
        var_v1_3 += 0x1FF;
    }
    var_v1_3 >>= 9;
    *(s16 *)(strip_code + (3)) = (s16)((((var_a0) + ((var_v0_2) + 0x70)) - (var_a0_2)) - (var_v1_3));
    work.input.vx = radius + 0x14;
    work.input.vy = 0;
    work.input.vz = 0;
    first_xy = *(s32 *)(strip_code + 0x1);
    gte_SetRotMatrix(matrix);
    gte_ldv0(&work.input);
    gte_rtv0();
    gte_stlvnl(&work.vectors[0]);
    if (facing != 0)
    {
        var_v1_4 = position->vx + (work.vectors[0].vx << 8);
    }
    else
    {
        var_v1_4 = position->vx - (work.vectors[0].vx << 8);
    }
    work.vectors[1].vx = var_v1_4;
    work.vectors[1].vy = position->vy + (work.vectors[0].vy << 8);
    work.vectors[1].vz = position->vz + (work.vectors[0].vz << 8);
    var_v0_3 = D_800F22A0;
    if (var_v0_3 < 0)
    {
        var_v0_3 += 0xFF;
    }
    var_v0_3 >>= 8;
    var_v1_5 = work.vectors[1].vx;
    if (var_v1_5 < 0)
    {
        var_v1_5 += 0xFF;
    }
    var_v1_5 >>= 8;
    var_a0_3 = D_800F22A4;
    *(s16 *)(strip_code + (9)) = (s16)((var_v0_3) + ((var_v1_5) + 0xA0));
    if (var_a0_3 < 0)
    {
        var_a0_3 += 0xFF;
    }
    var_a0_3 >>= 8;
    var_v0_4 = work.vectors[1].vy;
    if (var_v0_4 < 0)
    {
        var_v0_4 += 0xFF;
    }
    var_v0_4 >>= 8;
    var_v1_6 = work.vectors[1].vz;
    if (var_v1_6 < 0)
    {
        var_v1_6 += 0x1FF;
    }
    var_v1_6 >>= 9;
    var_a0_4 = D_800F22A8;
    if (var_a0_4 < 0)
    {
        var_a0_4 += 0x1FF;
    }
    var_a0_4 >>= 9;
    segment_index = 1;
    radius_sum = radius;
    segment_code = current_packet + 7;
    *(s16 *)(strip_code + (11)) = (s16)((((var_a0_3) + ((var_v0_4) + 0x70)) - (var_v1_6)) - (var_a0_4));
    second_xy = *(s32 *)(strip_code + 0x9);
loop_32:
    saved_mask = tag_mask;
    RotMatrixX(-0x100, matrix);
    work.input.vx = radius + (radius_sum >> 5);
    work.input.vy = 0;
    work.input.vz = 0;
    gte_SetRotMatrix(matrix);
    gte_ldv0(&work.input);
    gte_rtv0();
    gte_stlvnl(&work.vectors[0]);
    if (facing != 0)
    {
        var_v1_7 = position->vx + (work.vectors[0].vx << 8);
    }
    else
    {
        var_v1_7 = position->vx - (work.vectors[0].vx << 8);
    }
    work.vectors[1].vx = var_v1_7;
    work.vectors[1].vy = position->vy + (work.vectors[0].vy << 8);
    work.vectors[1].vz = position->vz + (work.vectors[0].vz << 8);
    var_v0_5 = D_800F22A0;
    if (var_v0_5 < 0)
    {
        var_v0_5 += 0xFF;
    }
    var_v0_5 >>= 8;
    var_v1_8 = work.vectors[1].vx;
    if (var_v1_8 < 0)
    {
        var_v1_8 += 0xFF;
    }
    var_v1_8 >>= 8;
    var_a0_5 = D_800F22A4;
    *(s16 *)(segment_code + (17)) = (s16)((var_v0_5) + ((var_v1_8) + 0xA0));
    if (var_a0_5 < 0)
    {
        var_a0_5 += 0xFF;
    }
    var_a0_5 >>= 8;
    var_v0_6 = work.vectors[1].vy;
    if (var_v0_6 < 0)
    {
        var_v0_6 += 0xFF;
    }
    var_v0_6 >>= 8;
    var_a0_6 = work.vectors[1].vz;
    if (var_a0_6 < 0)
    {
        var_a0_6 += 0x1FF;
    }
    var_a0_6 >>= 9;
    var_v1_9 = D_800F22A8;
    if (var_v1_9 < 0)
    {
        var_v1_9 += 0x1FF;
    }
    var_v1_9 >>= 9;
    *(s16 *)(segment_code + (19)) = (s16)((((var_a0_5) + ((var_v0_6) + 0x70)) - (var_a0_6)) - (var_v1_9));
    *(s32 *)(segment_code + (37)) = *(s32 *)(segment_code + 0x11);
    work.input.vx = radius + (radius_sum >> 5) + 0x14;
    work.input.vy = 0;
    work.input.vz = 0;
    gte_SetRotMatrix(matrix);
    gte_ldv0(&work.input);
    gte_rtv0();
    gte_stlvnl(&work.vectors[0]);
    if (facing != 0)
    {
        var_v1_10 = position->vx + (work.vectors[0].vx << 8);
    }
    else
    {
        var_v1_10 = position->vx - (work.vectors[0].vx << 8);
    }
    work.vectors[1].vx = var_v1_10;
    work.vectors[1].vy = position->vy + (work.vectors[0].vy << 8);
    work.vectors[1].vz = position->vz + (work.vectors[0].vz << 8);
    var_v0_7 = D_800F22A0;
    if (var_v0_7 < 0)
    {
        var_v0_7 += 0xFF;
    }
    var_v0_7 >>= 8;
    var_v1_11 = work.vectors[1].vx;
    if (var_v1_11 < 0)
    {
        var_v1_11 += 0xFF;
    }
    var_v1_11 >>= 8;
    var_a0_7 = D_800F22A4;
    *(s16 *)(segment_code + (25)) = (s16)((var_v0_7) + ((var_v1_11) + 0xA0));
    if (var_a0_7 < 0)
    {
        var_a0_7 += 0xFF;
    }
    var_a0_7 >>= 8;
    var_v0_8 = work.vectors[1].vy;
    if (var_v0_8 < 0)
    {
        var_v0_8 += 0xFF;
    }
    var_v0_8 >>= 8;
    var_a0_8 = work.vectors[1].vz;
    if (var_a0_8 < 0)
    {
        var_a0_8 += 0x1FF;
    }
    var_a0_8 >>= 9;
    var_v1_12 = D_800F22A8;
    temp_a1 = ((var_a0_7) + ((var_v0_8) + 0x70)) - (var_a0_8);
    if (var_v1_12 < 0)
    {
        var_v1_12 += 0x1FF;
    }
    var_v1_12 >>= 9;
    *(s16 *)(segment_code + (27)) = (s16)(temp_a1 - (var_v1_12));
    *(s32 *)(segment_code + (-3)) = 0;
    *(s32 *)(segment_code + (5)) = 0xA00000;
    *(s32 *)(segment_code + (13)) = 0;
    *(s32 *)(segment_code + (21)) = 0xA00000;
    *(s32 *)(segment_code + (45)) = *(s32 *)(segment_code + 0x19);
    saved_mask = tag_mask;
    SetPolyG4((POLY_G4 *)current_packet);
    *(u8 *)(segment_code + (0)) = (u8)(*(u8 *)(segment_code + (0)) | 2);
    temp_v1 = (s32)position->vz >> 7;
    if (temp_v1 < 0)
    {
        segment_code += 0x24;
        strip_code += 0x24;
        temp_v1_2 = (s32)current_packet & 0xFFFFFF;
        *(s32 *)current_packet = (*(s32 *)current_packet & tag_mask) | (ordering_table[0] & 0xFFFFFF);
        current_packet += 0x24;
        ordering_table[0] = (s32)((ordering_table[0] & tag_mask) | temp_v1_2);
    }
    else if (temp_v1 >= 0x1000)
    {
        segment_code += 0x24;
        strip_code += 0x24;
        temp_v1_3 = (s32)current_packet & 0xFFFFFF;
        *(s32 *)current_packet = (*(s32 *)current_packet & tag_mask) | (ordering_table[0xFFF] & 0xFFFFFF);
        current_packet += 0x24;
        ordering_table[0xFFF] = (s32)((ordering_table[0xFFF] & tag_mask) | temp_v1_3);
    }
    else
    {
        segment_code += 0x24;
        strip_code += 0x24;
        *(s32 *)current_packet = (*(s32 *)current_packet & tag_mask) | (ordering_table[temp_v1] & 0xFFFFFF);
        temp_a0 = (s32)current_packet & 0xFFFFFF;
        temp_v1_4 = ordering_table + ((s32)position->vz >> 7);
        current_packet += 0x24;
        *temp_v1_4 = (*temp_v1_4 & tag_mask) | temp_a0;
    }
    segment_index += 1;
    radius_sum += radius;
    if (segment_index < 9)
    {
        goto loop_32;
    }
    *(s32 *)(strip_code + (17)) = first_xy;
    *(s32 *)(strip_code + (25)) = second_xy;
    *(s32 *)(strip_code + (-3)) = 0;
    *(s32 *)(strip_code + (5)) = 0xA000;
    *(s32 *)(strip_code + (13)) = 0;
    *(s32 *)(strip_code + (21)) = 0xA000;
    saved_mask = tag_mask;
    SetPolyG4((POLY_G4 *)current_packet);
    *(u8 *)(strip_code + (0)) = (u8)(*(u8 *)(strip_code + (0)) | 2);
    outer_mask = tag_mask;
    temp_v1_5 = (s32)position->vz >> 7;
    if (temp_v1_5 < 0)
    {
        strip_code += 0x24;
        *(s32 *)current_packet = (*(s32 *)current_packet & outer_mask) | (ordering_table[0] & 0xFFFFFF);
        temp_v1_6 = (s32)current_packet & 0xFFFFFF;
        current_packet += 0x24;
        ordering_table[0] = (s32)((ordering_table[0] & outer_mask) | temp_v1_6);
    }
    else if (temp_v1_5 >= 0x1000)
    {
        strip_code += 0x24;
        *(s32 *)current_packet = (*(s32 *)current_packet & outer_mask) | (ordering_table[0xFFF] & 0xFFFFFF);
        temp_v1_7 = (s32)current_packet & 0xFFFFFF;
        current_packet += 0x24;
        ordering_table[0xFFF] = (s32)((ordering_table[0xFFF] & outer_mask) | temp_v1_7);
    }
    else
    {
        strip_code += 0x24;
        *(s32 *)current_packet = (*(s32 *)current_packet & outer_mask) | (ordering_table[temp_v1_5] & 0xFFFFFF);
        temp_a0_2 = (s32)current_packet & 0xFFFFFF;
        temp_v1_8 = ordering_table + ((s32)position->vz >> 7);
        current_packet += 0x24;
        *temp_v1_8 = (*temp_v1_8 & outer_mask) | temp_a0_2;
    }
    radius += 0x50;
    if (radius >= 0x140)
    {
        radius -= 0x140;
    }
    temp_t0 = strip_index + 1;
    strip_index = temp_t0;
}
    if (temp_t0 < 4)
    {
        goto next_strip;
    }
    *(u8 *)(current_packet + (3)) = 1;
    *(s32 *)(current_packet + (4)) = 0xE1000025;
    temp_v1_9 = (s32)position->vz >> 7;
    if (temp_v1_9 < 0)
    {
        *(s32 *)(current_packet + (0)) = (*(s32 *)(current_packet + (0)) & 0xFF000000) | (ordering_table[0] & 0xFFFFFF);
        next_packet = current_packet + 8;
        ordering_table[0] = (s32)((ordering_table[0] & 0xFF000000) | ((s32)current_packet & 0xFFFFFF));
    }
    else if (temp_v1_9 >= 0x1000)
    {
        *(s32 *)(current_packet + (0)) =
            (*(s32 *)(current_packet + (0)) & 0xFF000000) | (ordering_table[0xFFF] & 0xFFFFFF);
        next_packet = current_packet + 8;
        ordering_table[0xFFF] = (s32)((ordering_table[0xFFF] & 0xFF000000) | ((s32)current_packet & 0xFFFFFF));
    }
    else
    {
        *(s32 *)(current_packet + (0)) =
            (*(s32 *)(current_packet + (0)) & 0xFF000000) | (ordering_table[temp_v1_9] & 0xFFFFFF);
        temp_v0 = ordering_table + ((s32)position->vz >> 7);
        next_packet = current_packet + 8;
        *temp_v0 = (*temp_v0 & 0xFF000000) | ((s32)current_packet & 0xFFFFFF);
    }
    return next_packet;
}
