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
#define ADD_PACKET(depth)                                                                                                                                      \
    (*(s32*)current_packet = (*(s32*)current_packet & tag_mask) | (ordering_table[depth] & addr_mask),                                                         \
     ordering_table[depth] = (ordering_table[depth] & tag_mask) | ((s32)current_packet & addr_mask))

/**
 * @brief Draw four rotating strips of Gouraud-shaded quads around a position.
 * @param ordering_table Depth ordering table with 4096 entries.
 * @param packet First free primitive packet.
 * @param position Fixed-point world-space origin of the effect.
 * @param slope Direction parameter converted into a Y rotation angle.
 * @param facing Selects addition or subtraction of the rotated X offset.
 * @return First free packet following all quads and the draw-page command.
 * @note Full word copies carry packed X/Y pairs into the next segment and
 *       close the strip. The strip cursor points at the primitive code byte.
 * @see decomp.me (100%)
 */
u8* func_800A1344(s32* ordering_table, u8* packet, VECTOR* position, s32 slope, s32 facing)
{
    s32 screen_x;
    s32 packet_addr;
    FieldStripWorkspace work;
    s32 strip_index;
    s32 angle;
    s32 first_xy;
    s32 second_xy;
    MATRIX* matrix;
    u8* current_packet;
    s32 segment_depth;
    s32 closing_depth;
    s32 drawpage_depth;
    s32 camera_y;
    s32 addr_mask;
    s32 tag_mask;
    s32 segment_index;
    s32 radius;
    s32 radius_sum;
    s32 first_x;
    s32 segment_outer_x;
    s32 second_x;
    s32 segment_inner_x;
    u8* segment_packet;
    u8* strip_code;

    current_packet = packet;
    angle = ratan2(slope, 0x64);
    strip_index = 0;
    matrix = &work.matrices[2];
    addr_mask = 0xFFFFFF;
    tag_mask = 0xFF000000;
    radius = D_801178D8;
    strip_code = current_packet + 7;
next_strip:
    {
        ((s32*)&work.matrices[2])[4] = 0x1000;
        ((s32*)&work.matrices[2])[2] = 0x1000;
        ((s32*)&work.matrices[2])[0] = 0x1000;
        ((s32*)&work.matrices[2])[7] = 0;
        ((s32*)&work.matrices[2])[6] = 0;
        ((s32*)&work.matrices[2])[5] = 0;
        ((s32*)&work.matrices[2])[3] = 0;
        ((s32*)&work.matrices[2])[1] = 0;
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
            first_x = position->vx + (work.rotated.vx << 8);
        }
        else
        {
            first_x = position->vx - (work.rotated.vx << 8);
        }
        do
        {
            work.world.vx = first_x;
            work.world.vy = position->vy + (work.rotated.vy << 8);
            work.world.vz = position->vz + (work.rotated.vz << 8);
        } while (0);
        screen_x = 160 + D_800F22A0 / 256 + ((volatile VECTOR*)&work.world)->vx / 256;
        camera_y = D_800F22A4;
        *(s16*)(strip_code + 1) = screen_x;
        if (camera_y < 0)
        {
            camera_y += 255;
        }
        *(s16*)(strip_code + 3) = 112 + (camera_y >> 8) + ((volatile VECTOR*)&work.world)->vy / 256 - ((volatile VECTOR*)&work.world)->vz / 512 - D_800F22A8 / 512;
        first_xy = *(s32*)(strip_code + 0x1);
        work.input.vx = radius + 0x14;
        work.input.vy = 0;
        work.input.vz = 0;
        gte_SetRotMatrix(matrix);
        gte_ldv0(&work.input);
        gte_rtv0();
        gte_stlvnl(&work.rotated);
        if (facing != 0)
        {
            second_x = position->vx + (work.rotated.vx << 8);
        }
        else
        {
            second_x = position->vx - (work.rotated.vx << 8);
        }
        do
        {
            work.world.vx = second_x;
            work.world.vy = position->vy + (work.rotated.vy << 8);
            work.world.vz = position->vz + (work.rotated.vz << 8);
        } while (0);
        screen_x = 160 + D_800F22A0 / 256 + ((volatile VECTOR*)&work.world)->vx / 256;
        camera_y = D_800F22A4;
        *(s16*)(strip_code + 9) = screen_x;
        if (camera_y < 0)
        {
            camera_y += 255;
        }
        *(s16*)(strip_code + 11) = 112 + (camera_y >> 8) + ((volatile VECTOR*)&work.world)->vy / 256 - ((volatile VECTOR*)&work.world)->vz / 512 - D_800F22A8 / 512;
        segment_index = 1;
        radius_sum = radius;
        segment_packet = current_packet;
        second_xy = *(s32*)(strip_code + 0x9);
        do
        {
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
                segment_inner_x = position->vx + (work.rotated.vx << 8);
            }
            else
            {
                segment_inner_x = position->vx - (work.rotated.vx << 8);
            }
            do
            {
                work.world.vx = segment_inner_x;
                work.world.vy = position->vy + (work.rotated.vy << 8);
                work.world.vz = position->vz + (work.rotated.vz << 8);
            } while (0);
            screen_x = 160 + D_800F22A0 / 256 + ((volatile VECTOR*)&work.world)->vx / 256;
            camera_y = D_800F22A4;
            *(s16*)(segment_packet + 24) = screen_x;
            if (camera_y < 0)
            {
                camera_y += 255;
            }
            *(s16*)(segment_packet + 26) =
                112 + (camera_y >> 8) + ((volatile VECTOR*)&work.world)->vy / 256 - ((volatile VECTOR*)&work.world)->vz / 512 - D_800F22A8 / 512;
            *(s32*)(segment_packet + 44) = *(s32*)(segment_packet + 24);
            work.input.vx = radius + (radius_sum >> 5) + 0x14;
            work.input.vy = 0;
            work.input.vz = 0;
            gte_SetRotMatrix(matrix);
            gte_ldv0(&work.input);
            gte_rtv0();
            gte_stlvnl(&work.rotated);
            if (facing != 0)
            {
                segment_outer_x = position->vx + (work.rotated.vx << 8);
            }
            else
            {
                segment_outer_x = position->vx - (work.rotated.vx << 8);
            }
            do
            {
                work.world.vx = segment_outer_x;
                work.world.vy = position->vy + (work.rotated.vy << 8);
                work.world.vz = position->vz + (work.rotated.vz << 8);
            } while (0);
            screen_x = 160 + D_800F22A0 / 256 + ((volatile VECTOR*)&work.world)->vx / 256;
            camera_y = D_800F22A4;
            *(s16*)(segment_packet + 32) = screen_x;
            if (camera_y < 0)
            {
                camera_y += 255;
            }
            *(s16*)(segment_packet + 34) =
                112 + (camera_y >> 8) + ((volatile VECTOR*)&work.world)->vy / 256 - ((volatile VECTOR*)&work.world)->vz / 512 - D_800F22A8 / 512;
            *(s32*)(segment_packet + 4) = 0;
            *(s32*)(segment_packet + 12) = 0xA00000;
            *(s32*)(segment_packet + 20) = 0;
            *(s32*)(segment_packet + 28) = 0xA00000;
            *(s32*)(segment_packet + 52) = *(s32*)(segment_packet + 32);
            SetPolyG4((POLY_G4*)current_packet);
            do
            {
                *(u8*)(segment_packet + 7) = (u8)(*(u8*)(segment_packet + 7) | 2);
                segment_depth = position->vz >> 7;
                if (segment_depth < 0)
                {
                    ADD_PACKET(0);
                    current_packet += 0x24;
                    segment_packet += 0x24;
                    strip_code += 0x24;
                }
                else if (segment_depth >= 0x1000)
                {
                    ADD_PACKET(0xFFF);
                    current_packet += 0x24;
                    segment_packet += 0x24;
                    strip_code += 0x24;
                }
                else
                {
                    ADD_PACKET(position->vz >> 7);
                    current_packet += 0x24;
                    segment_packet += 0x24;
                    strip_code += 0x24;
                }
                segment_index += 1;
                radius_sum += radius;
            } while (0);
        } while (segment_index < 9);
        *(s32*)(strip_code + (17)) = first_xy;
        *(s32*)(strip_code + (25)) = second_xy;
        *(s32*)(strip_code + (-3)) = 0;
        *(s32*)(strip_code + (5)) = 0xA000;
        *(s32*)(strip_code + (13)) = 0;
        *(s32*)(strip_code + (21)) = 0xA000;
        SetPolyG4((POLY_G4*)current_packet);
        *(u8*)(strip_code + (0)) = (u8)(*(u8*)(strip_code + (0)) | 2);
        closing_depth = position->vz >> 7;
        if (closing_depth < 0)
        {
            strip_code += 0x24;
            do
            {
                *(s32*)current_packet = (*(s32*)current_packet & tag_mask) | (ordering_table[0] & addr_mask);
                packet_addr = (s32)current_packet & addr_mask;
            } while (0);
            ordering_table[0] = (ordering_table[0] & tag_mask) | packet_addr;
            current_packet += 0x24;
        }
        else if (closing_depth >= 0x1000)
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
        do
        {
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
    *(u8*)(current_packet + (3)) = 1;
    *(s32*)(current_packet + (4)) = 0xE1000025;
    drawpage_depth = position->vz >> 7;
    if (drawpage_depth < 0)
    {
        addPrim(&ordering_table[0], current_packet);
        current_packet += 8;
    }
    else if (drawpage_depth >= 0x1000)
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

#undef ADD_PACKET
