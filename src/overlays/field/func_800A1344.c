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
    VECTOR rotated;
    VECTOR world;
    VECTOR unused;
    SVECTOR input;
    MATRIX matrices[5];
} FieldStripWorkspace;
extern s32 D_800F22A0, D_800F22A4, D_800F22A8;
extern s32 D_801178D8;
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
#define ADD_PACKET(depth) (*(s32 *)current_packet = (*(s32 *)current_packet & tag_mask) | (ordering_table[depth] & addr_mask), ordering_table[depth] = (ordering_table[depth] & tag_mask) | ((s32)current_packet & addr_mask))
u8 *func_800A1344(s32 *ordering_table, u8 *packet, VECTOR *position, s32 slope, s32 facing)
{
    s32 screen_x;
    s32 saved_mask;
    FieldStripWorkspace work;
    s32 strip_index;
    s32 angle;
    s32 first_xy;
    s32 second_xy;
    MATRIX *matrix;
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
    s32 addr_mask;
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
    matrix = &work.matrices[2];
    addr_mask = 0xFFFFFF;
    tag_mask = 0xFF000000;
    radius = D_801178D8;
    strip_code = current_packet + 7;
    strip_index = 0;
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
    RotMatrixY(angle, matrix);
    work.input.vx = (s16)radius;
    work.input.vy = 0;
    work.input.vz = 0;
    gte_SetRotMatrix(matrix);
    gte_ldv0(&work.input);
    gte_rtv0();
    gte_stlvnl(&work.rotated);
    if (facing != 0)
    {
        var_v1 = position->vx + (work.rotated.vx << 8);
    }
    else
    {
        var_v1 = position->vx - (work.rotated.vx << 8);
    }
    do {
    work.world.vx = var_v1;
    work.world.vy = position->vy + (work.rotated.vy << 8);
    work.world.vz = position->vz + (work.rotated.vz << 8);
    } while (0);
    screen_x = 160 + D_800F22A0 / 256 + ((volatile VECTOR *)&work.world)->vx / 256;
    var_a0 = D_800F22A4;
    *(s16 *)(strip_code + 1) = screen_x;
    if (var_a0 < 0) { var_a0 += 255; }
    *(s16 *)(strip_code + 3) = 112 + (var_a0 >> 8) + ((volatile VECTOR *)&work.world)->vy / 256 - ((volatile VECTOR *)&work.world)->vz / 512 - D_800F22A8 / 512;
    first_xy = *(s32 *)(strip_code + 0x1);
    work.input.vx = radius + 0x14;
    work.input.vy = 0;
    work.input.vz = 0;
    gte_SetRotMatrix(matrix);
    gte_ldv0(&work.input);
    gte_rtv0();
    gte_stlvnl(&work.rotated);
    if (facing != 0)
    {
        var_v1_4 = position->vx + (work.rotated.vx << 8);
    }
    else
    {
        var_v1_4 = position->vx - (work.rotated.vx << 8);
    }
    do {
    work.world.vx = var_v1_4;
    work.world.vy = position->vy + (work.rotated.vy << 8);
    work.world.vz = position->vz + (work.rotated.vz << 8);
    } while (0);
    screen_x = 160 + D_800F22A0 / 256 + ((volatile VECTOR *)&work.world)->vx / 256;
    var_a0 = D_800F22A4;
    *(s16 *)(strip_code + 9) = screen_x;
    if (var_a0 < 0) { var_a0 += 255; }
    *(s16 *)(strip_code + 11) = 112 + (var_a0 >> 8) + ((volatile VECTOR *)&work.world)->vy / 256 - ((volatile VECTOR *)&work.world)->vz / 512 - D_800F22A8 / 512;
    segment_index = 1;
    radius_sum = radius;
    segment_code = current_packet;
    second_xy = *(s32 *)(strip_code + 0x9);
do {
    RotMatrixX(-0x100, matrix);
    work.input.vx = radius + (radius_sum >> 5);
    work.input.vy = 0;
    work.input.vz = 0;
    gte_SetRotMatrix(matrix);
    gte_ldv0(&work.input);
    gte_rtv0();
    gte_stlvnl(&work.rotated);
    if (facing != 0)
    {
        var_v1_7 = position->vx + (work.rotated.vx << 8);
    }
    else
    {
        var_v1_7 = position->vx - (work.rotated.vx << 8);
    }
    do {
    work.world.vx = var_v1_7;
    work.world.vy = position->vy + (work.rotated.vy << 8);
    work.world.vz = position->vz + (work.rotated.vz << 8);
    } while (0);
    screen_x = 160 + D_800F22A0 / 256 + ((volatile VECTOR *)&work.world)->vx / 256;
    var_a0 = D_800F22A4;
    *(s16 *)(segment_code + 24) = screen_x;
    if (var_a0 < 0) { var_a0 += 255; }
    *(s16 *)(segment_code + 26) = 112 + (var_a0 >> 8) + ((volatile VECTOR *)&work.world)->vy / 256 - ((volatile VECTOR *)&work.world)->vz / 512 - D_800F22A8 / 512;
    *(s32 *)(segment_code + 44) = *(s32 *)(segment_code + 24);
    work.input.vx = radius + (radius_sum >> 5) + 0x14;
    work.input.vy = 0;
    work.input.vz = 0;
    gte_SetRotMatrix(matrix);
    gte_ldv0(&work.input);
    gte_rtv0();
    gte_stlvnl(&work.rotated);
    if (facing != 0)
    {
        var_v1_10 = position->vx + (work.rotated.vx << 8);
    }
    else
    {
        var_v1_10 = position->vx - (work.rotated.vx << 8);
    }
    do {
    work.world.vx = var_v1_10;
    work.world.vy = position->vy + (work.rotated.vy << 8);
    work.world.vz = position->vz + (work.rotated.vz << 8);
    } while (0);
    screen_x = 160 + D_800F22A0 / 256 + ((volatile VECTOR *)&work.world)->vx / 256;
    var_a0 = D_800F22A4;
    *(s16 *)(segment_code + 32) = screen_x;
    if (var_a0 < 0) { var_a0 += 255; }
    *(s16 *)(segment_code + 34) = 112 + (var_a0 >> 8) + ((volatile VECTOR *)&work.world)->vy / 256 - ((volatile VECTOR *)&work.world)->vz / 512 - D_800F22A8 / 512;
    *(s32 *)(segment_code + 4) = 0;
    *(s32 *)(segment_code + 12) = 0xA00000;
    *(s32 *)(segment_code + 20) = 0;
    *(s32 *)(segment_code + 28) = 0xA00000;
    *(s32 *)(segment_code + 52) = *(s32 *)(segment_code + 32);
    SetPolyG4((POLY_G4 *)current_packet);
    *(u8 *)(segment_code + 7) = (u8)(*(u8 *)(segment_code + 7) | 2);
    temp_v1 = position->vz >> 7;
    do {
    if (temp_v1 < 0)
    {
        ADD_PACKET(0);
        current_packet += 0x24;
        segment_code += 0x24;
        strip_code += 0x24;
    }
    else if (temp_v1 >= 0x1000)
    {
        ADD_PACKET(0xFFF);
        current_packet += 0x24;
        segment_code += 0x24;
        strip_code += 0x24;
    }
    else
    {
        ADD_PACKET(position->vz >> 7);
        current_packet += 0x24;
        segment_code += 0x24;
        strip_code += 0x24;
    }
    segment_index += 1;
    radius_sum += radius;
    } while (0); } while (segment_index < 9);
    *(s32 *)(strip_code + (17)) = first_xy;
    *(s32 *)(strip_code + (25)) = second_xy;
    *(s32 *)(strip_code + (-3)) = 0;
    *(s32 *)(strip_code + (5)) = 0xA000;
    *(s32 *)(strip_code + (13)) = 0;
    *(s32 *)(strip_code + (21)) = 0xA000;
    SetPolyG4((POLY_G4 *)current_packet);
    *(u8 *)(strip_code + (0)) = (u8)(*(u8 *)(strip_code + (0)) | 2);
    temp_v1_5 = position->vz >> 7;
    if (temp_v1_5 < 0)
    {
        ADD_PACKET(0);
        current_packet += 0x24;
        strip_code += 0x24;
    }
    else if (temp_v1_5 >= 0x1000)
    {
        ADD_PACKET(0xFFF);
        current_packet += 0x24;
        strip_code += 0x24;
    }
    else
    {
        ADD_PACKET(position->vz >> 7);
        current_packet += 0x24;
        strip_code += 0x24;
    }
    radius += 0x50;
    do {
    if (radius >= 0x140)
    {
        radius -= 0x140;
    }
    } while (0);
    strip_index++;
}
    if (strip_index < 4)
    {
        goto next_strip;
    }
    *(u8 *)(current_packet + (3)) = 1;
    *(s32 *)(current_packet + (4)) = 0xE1000025;
    temp_v1_9 = position->vz >> 7;
    if (temp_v1_9 < 0)
    {
        addPrim(&ordering_table[0], current_packet);
        current_packet += 8;
    }
    else if (temp_v1_9 >= 0x1000)
    {
        addPrim(&ordering_table[0xFFF], current_packet);
        current_packet += 8;
    }
    else
    {
        addPrim(&ordering_table[position->vz >> 7], current_packet);
        current_packet += 8;
    }
    return current_packet;
}
