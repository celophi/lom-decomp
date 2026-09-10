#include "common.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;
extern s32 D_801178D8;

/**
 * @brief Draw animated curved quad strips extending from a fixed-point position.
 * @param ordering_table Ordering table with 0x1000 depth entries.
 * @param primitive_buffer Destination for the generated GPU packets.
 * @param position World position in signed fixed-point coordinates.
 * @param extent Maximum horizontal extent, tested after each completed strip.
 * @param forward Nonzero extends toward positive X; zero extends toward negative X.
 * @return First byte after the emitted primitives and draw-page command.
 * @note Emits at least one strip and at most four, with nine quads per strip.
 * @note Unused local workspace storage preserves the recovered frame layout.
 */
u8 *func_800A0B0C(s32 *ordering_table, u8 *primitive_buffer, VECTOR *position, s32 extent, s32 forward)
{
    s32 step;
    s32 strip_index;
    s32 first_xy;
    s32 second_xy;
    /* Only element 1 is accessed; other recovered workspace roles are unknown. */
    volatile VECTOR projection_workspace[13];
    SVECTOR reserved_workspace;
    s32 *temp_v0;
    s32 *temp_v1_4;
    s32 *temp_v1_8;
    u8 *next_primitive;
    u8 *primitive;
    s32 temp_a0;
    s32 temp_a0_2;
    s32 temp_a1;
    s32 temp_a1_2;
    s32 temp_t0;
    s32 temp_t0_2;
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
    s32 angle;
    s32 offset;
    s32 var_v0;
    s32 var_v0_10;
    s32 var_v0_11;
    s32 var_v0_12;
    s32 var_v0_2;
    s32 var_v0_3;
    s32 var_v0_4;
    s32 var_v0_5;
    s32 var_v0_6;
    s32 var_v0_7;
    s32 var_v0_8;
    s32 var_v0_9;
    s32 var_v1;
    s32 var_v1_2;
    s32 var_v1_3;
    s32 var_v1_4;
    s32 var_v1_5;
    s32 var_v1_6;
    s32 var_v1_7;
    s32 var_v1_8;
    u8 *strip_code;
    u8 *outer_code;

    primitive = primitive_buffer;
    outer_code = primitive + 7;
    strip_index = 0;
    offset = (D_801178D8 % 40) << 8;
next_strip:
    /* Save both initial packed XY values to close the strip after eight steps. */
    if (forward != 0)
    {
        var_v0 = position->vx + offset;
    }
    else
    {
        var_v0 = position->vx - offset;
    }
    projection_workspace[1].vx = var_v0;
    projection_workspace[1].vy = position->vy;
    projection_workspace[1].vz = position->vz + 0x2000;
    *(s16 *)(outer_code + 1) = 160 + projection_workspace[1].vx / 256 + D_800F22A0 / 256;
    *(s16 *)(outer_code + 3) = 112 + projection_workspace[1].vy / 256 + D_800F22A4 / 256 - projection_workspace[1].vz / 512 - D_800F22A8 / 512;
    first_xy = (s32) *(s32 *)(outer_code + (1));
    if (forward != 0)
    {
        var_v0_4 = position->vx + offset + 0x1400;
    }
    else
    {
        var_v0_4 = (position->vx - offset) - 0x1400;
    }
    projection_workspace[1].vx = var_v0_4;
    projection_workspace[1].vy = position->vy;
    projection_workspace[1].vz = position->vz + 0x2000;
    *(s16 *)(outer_code + 9) = 160 + projection_workspace[1].vx / 256 + D_800F22A0 / 256;
    *(s16 *)(outer_code + 11) = 112 + projection_workspace[1].vy / 256 + D_800F22A4 / 256 - projection_workspace[1].vz / 512 - D_800F22A8 / 512;
    angle = 0x100;
    strip_code = primitive + 7;
    step = 1;
    second_xy = (s32) *(s32 *)(outer_code + (9));
    do
    {
        if (forward != 0)
        {
            var_v0_7 = position->vx + offset + angle;
        }
        else
        {
            var_v0_7 = (position->vx - offset) - angle;
        }
        projection_workspace[1].vx = var_v0_7;
        projection_workspace[1].vy = position->vy - (rsin(angle) * 2);
        projection_workspace[1].vz = position->vz + (rcos(angle) * 2);
        *(s16 *)(strip_code + 17) = 160 + projection_workspace[1].vx / 256 + D_800F22A0 / 256;
        *(s16 *)(strip_code + 19) = 112 + projection_workspace[1].vy / 256 + D_800F22A4 / 256 - projection_workspace[1].vz / 512 - D_800F22A8 / 512;
        *(s32 *)(strip_code + (37)) = (s32) *(s32 *)(strip_code + (17));
        if (forward != 0)
        {
            var_v0_10 = position->vx + offset + angle + 0x1400;
        }
        else
        {
            var_v0_10 = ((position->vx - offset) - angle) - 0x1400;
        }
        projection_workspace[1].vx = var_v0_10;
        projection_workspace[1].vy = position->vy - (rsin(angle) * 2);
        projection_workspace[1].vz = position->vz + (rcos(angle) * 2);
        *(s16 *)(strip_code + 25) = 160 + projection_workspace[1].vx / 256 + D_800F22A0 / 256;
        *(s16 *)(strip_code + 27) = 112 + projection_workspace[1].vy / 256 + D_800F22A4 / 256 - projection_workspace[1].vz / 512 - D_800F22A8 / 512;
        /* Fade the curved strip from black to blue. */
        *(s32 *)(strip_code + (-3)) = 0;
        *(s32 *)(strip_code + (5)) = 0xA00000;
        *(s32 *)(strip_code + (13)) = 0;
        *(s32 *)(strip_code + (21)) = 0xA00000;
        *(s32 *)(strip_code + (45)) = (s32) *(s32 *)(strip_code + (25));
        SetPolyG4((POLY_G4 *)primitive);
        *(u8 *)(strip_code + (0)) = (u8) (*(u8 *)(strip_code + (0)) | 2);
        temp_v1 = (s32) position->vz >> 7;
        if (temp_v1 < 0)
        {
            strip_code += 0x24;
            outer_code += 0x24;
            temp_v1_2 = (s32) primitive & 0xFFFFFF;
            *(s32 *)primitive = (*(s32 *)primitive & 0xFF000000) | (ordering_table[0] & 0xFFFFFF);
            primitive += 0x24;
            ordering_table[0] = (s32) ((ordering_table[0] & 0xFF000000) | temp_v1_2);
        }
        else if (temp_v1 >= 0x1000)
        {
            strip_code += 0x24;
            outer_code += 0x24;
            temp_v1_3 = (s32) primitive & 0xFFFFFF;
            *(s32 *)primitive = (*(s32 *)primitive & 0xFF000000) | (ordering_table[0xFFF] & 0xFFFFFF);
            primitive += 0x24;
            ordering_table[0xFFF] = (s32) ((ordering_table[0xFFF] & 0xFF000000) | temp_v1_3);
        }
        else
        {
            strip_code += 0x24;
            outer_code += 0x24;
            *(s32 *)primitive = (*(s32 *)primitive & 0xFF000000) | (ordering_table[temp_v1] & 0xFFFFFF);
            temp_a0 = (s32) primitive & 0xFFFFFF;
            temp_v1_4 = &ordering_table[(s32) position->vz >> 7];
            primitive += 0x24;
            *temp_v1_4 = (*temp_v1_4 & 0xFF000000) | temp_a0;
        }
        angle += 0x100;
        temp_t0 = step + 1;
        step = temp_t0;
        } while (temp_t0 < 9);
        *(s32 *)(outer_code + (17)) = first_xy;
        *(s32 *)(outer_code + (25)) = second_xy;
        *(s32 *)(outer_code + (-3)) = 0;
        *(s32 *)(outer_code + (5)) = 0xA000;
        *(s32 *)(outer_code + (13)) = 0;
        *(s32 *)(outer_code + (21)) = 0xA000;
        SetPolyG4((POLY_G4 *)primitive);
        *(u8 *)(outer_code + (0)) = (u8) (*(u8 *)(outer_code + (0)) | 2);
        temp_v1_5 = (s32) position->vz >> 7;
        if (temp_v1_5 < 0)
        {
            outer_code += 0x24;
            *(s32 *)primitive = (*(s32 *)primitive & 0xFF000000) | (ordering_table[0] & 0xFFFFFF);
            temp_v1_6 = (s32) primitive & 0xFFFFFF;
            primitive += 0x24;
            ordering_table[0] = (s32) ((ordering_table[0] & 0xFF000000) | temp_v1_6);
        }
        else if (temp_v1_5 >= 0x1000)
        {
            outer_code += 0x24;
            *(s32 *)primitive = (*(s32 *)primitive & 0xFF000000) | (ordering_table[0xFFF] & 0xFFFFFF);
            temp_v1_7 = (s32) primitive & 0xFFFFFF;
            primitive += 0x24;
            ordering_table[0xFFF] = (s32) ((ordering_table[0xFFF] & 0xFF000000) | temp_v1_7);
        }
        else
        {
            outer_code += 0x24;
            *(s32 *)primitive = (*(s32 *)primitive & 0xFF000000) | (ordering_table[temp_v1_5] & 0xFFFFFF);
            temp_a0_2 = (s32) primitive & 0xFFFFFF;
            temp_v1_8 = &ordering_table[(s32) position->vz >> 7];
            primitive += 0x24;
            *temp_v1_8 = (*temp_v1_8 & 0xFF000000) | temp_a0_2;
        }
        offset += 0x2800;
        if (offset < (extent << 8) && ++strip_index < 4)
        {
            goto next_strip;
        }
        /* Draw-page command follows all polygons at the same clamped depth. */
        *(u8 *)(primitive + (3)) = 1;
        *(s32 *)(primitive + (4)) = 0xE1000025;
        temp_v1_9 = (s32) position->vz >> 7;
        if (temp_v1_9 < 0)
        {
            *(s32 *)(primitive + (0)) = (*(s32 *)(primitive + (0)) & 0xFF000000) | (ordering_table[0] & 0xFFFFFF);
            next_primitive = primitive + 8;
            ordering_table[0] = (s32) ((ordering_table[0] & 0xFF000000) | ((s32) primitive & 0xFFFFFF));
        }
        else if (temp_v1_9 >= 0x1000)
        {
            *(s32 *)(primitive + (0)) = (*(s32 *)(primitive + (0)) & 0xFF000000) | (ordering_table[0xFFF] & 0xFFFFFF);
            next_primitive = primitive + 8;
            ordering_table[0xFFF] = (s32) ((ordering_table[0xFFF] & 0xFF000000) | ((s32) primitive & 0xFFFFFF));
        }
        else
        {
            *(s32 *)(primitive + (0)) = (*(s32 *)(primitive + (0)) & 0xFF000000) | (ordering_table[temp_v1_9] & 0xFFFFFF);
            temp_v0 = &ordering_table[(s32) position->vz >> 7];
            next_primitive = primitive + 8;
            *temp_v0 = (*temp_v0 & 0xFF000000) | ((s32) primitive & 0xFFFFFF);
        }
        return next_primitive;
    }
