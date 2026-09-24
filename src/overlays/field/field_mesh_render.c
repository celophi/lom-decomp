/** @file field_mesh_render.c
 * @brief Animate actor CLUT rows and emit textured and lit mesh primitives.
 */

#include "common.h"
#include "field_effect_transform.h"
#include "field_effect_render_state.h"
#include "field_actor_palette.h"
#include "field_effect_types.h"
#include "field_mesh_render.h"
#include "field_mesh.h"
#include "field_mesh_transform.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

extern FieldMotionRecord g_field_effect_records[];

extern FieldActorState g_field_actor_slots[80];
#define FIELD_PALETTE_ANIMATION_PERIOD_MASK 0xF
#define FIELD_PALETTE_ANIMATION_MODE_MASK 7
#define FIELD_PALETTE_ANIMATION_MODE_COUNT 6

#define FIELD_MESH_SCREEN_CENTER_X 160
#define FIELD_MESH_SCREEN_CENTER_Y 112
#define FIELD_MESH_SCREEN_ORIGIN ((s16 *)0x1F800000)
#define FIELD_MESH_FACE_STRIDE 16
#define FIELD_MESH_OT_MAX_DEPTH 0xFFF
#define FIELD_OT_ADDRESS_MASK 0x00FFFFFF
#define FIELD_OT_TAG_MASK 0xFF000000

/**
 * @brief Part index of light source @p i (0..2) of an actor part.
 * @note The three selector bytes start at rotation_extent's last byte and run into effect_flags.
 */
#define FIELD_PART_LIGHT_SOURCE(part, i) ((&(part)->rotation_extent.fields.unknown_0x23)[i])

/** @brief Bytes of CLUT data owned by each of the two actor palette owners. */
#define FIELD_ACTOR_CLUT_BUFFER_SIZE 0x400

/**
 * @brief CLUT buffer of actor palette owner @p owner (0 or 1).
 * @note g_field_actor_clut_buffers is declared as a flat byte array; the
 *       original indexes it as a [2][0x400] array.
 */
#define FIELD_ACTOR_CLUT_BUFFER(owner) (((u8(*)[FIELD_ACTOR_CLUT_BUFFER_SIZE])g_field_actor_clut_buffers)[owner])

void field_rotate_palette_row(u16 *row, s32 count, s32 rotate_right);



#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

/**
 * @brief Advance an actor's palette animation and upload the changed CLUT row to VRAM.
 * @param actor Owning actor state selecting the animation definition and slot.
 * @return Return value is unused by callers.
 * @see decomp.me (100.00%)
 */
s32 field_update_actor_palette_animation(FieldActorState *actor)
{
    RECT rect;
    u8 *buf;
    u16 animation;

    animation = actor->animation->palette_animation;
    if (!(animation & FIELD_PALETTE_ANIMATION_PERIOD_MASK))
    {
        return;
    }
    if (((u32)actor->track_ages[0] % (u32)((u8)actor->animation->palette_animation & FIELD_PALETTE_ANIMATION_PERIOD_MASK)) != 0)
    {
        return;
    }
    switch ((animation >> 4) & FIELD_PALETTE_ANIMATION_MODE_MASK)
    {
    case 0:
    case 1:
    case 4:
    case 5:
        return;

    case 2:
        if (actor->owner_object_index < 2)
        {
            u16 animation_word;
            s32 owner = actor->owner_object_index;

            animation_word = actor->animation->palette_animation;
            buf = &FIELD_ACTOR_CLUT_BUFFER(owner)[(animation_word >> 3) & 0x1E0];
            field_rotate_palette_row((u16 *)(buf + 2), 15, (animation_word >> 7) & 1);
            rect.x = (actor->animation->palette_animation >> 4) & 0xF0;
            rect.y = (actor->owner_object_index * 2) + 0x1EE;
            rect.w = 0x10;
            rect.h = 1;
        }
        else
        {
            u16 animation_word = actor->animation->palette_animation;
            buf = &g_field_shared_clut_buffer[(animation_word >> 3) & 0x1E0];
            field_rotate_palette_row((u16 *)(buf + 2), 15, (animation_word >> 7) & 1);
            rect.x = (actor->animation->palette_animation >> 4) & 0xF0;
            rect.y = 0x1F2;
            rect.w = 0x10;
            rect.h = 1;
        }
        break;

    case 3:
        if (actor->owner_object_index < 2)
        {
            buf = FIELD_ACTOR_CLUT_BUFFER(actor->owner_object_index);
            field_rotate_palette_row((u16 *)(buf + 2), 255, (actor->animation->palette_animation >> 7) & 1);
            rect.x = 0;
            rect.y = (actor->owner_object_index * 2) + 0x1EE;
            rect.w = 0x100;
            rect.h = 1;
        }
        else
        {
            buf = g_field_shared_clut_buffer;
            field_rotate_palette_row((u16 *)(buf + 2), 255, (actor->animation->palette_animation >> 7) & 1);
            rect.y = 0x1F2;
            rect.w = 0x100;
            rect.x = 0;
            rect.h = 1;
        }
        break;

    default:
        return;
    }
    LoadImage(&rect, (u_long *)buf);
}

/**
 * @brief Rotate a row of palette entries by one element.
 * @param row Palette row to rotate.
 * @param count Number of cells in the row.
 * @param rotate_right Nonzero to rotate right; zero to rotate left.
 * @see decomp.me (100%) https://decomp.me/scratch (func_800801F8)
 */
void field_rotate_palette_row(u16 *row, s32 count, s32 rotate_right)
{
    u16 temp;
    s32 i;

    if (rotate_right)
    {
        temp = row[count - 1];
        for (i = count - 1; i > 0; i--)
        {
            row[i] = row[i - 1];
        }
        row[0] = temp;
    }
    else
    {
        temp = row[0];
        for (i = 0; i < count - 1; i++)
        {
            row[i] = row[i + 1];
        }
        row[i] = temp;
    }
}

/** @brief Clamp @p expr into 0..0xFF and store it into @p dst. */
#define CLAMP_COLOR_CHANNEL(dst, expr) do { \
    s32 _v = (expr); \
    s32 _out; \
    if (_v >= 0) { \
        _out = 0xFF; \
        if (_v < 0x100) { \
            _out = _v; \
        } \
    } else { \
        _out = 0; \
    } \
    (dst) = _out; \
} while (0)

/**
 * @brief Emit ordering-table primitives for one mesh of an actor part.
 * @param effect Effect record driving the mesh transform.
 * @param mesh_index Mesh resource index to render.
 * @param packet_cursor Current primitive-buffer write cursor.
 * @param ordering_table Ordering table receiving the generated primitives.
 * @return Advanced primitive-buffer cursor.
 * @see decomp.me (100%) https://decomp.me/scratch (field_render_effect_mesh)
 */
s32 *field_render_effect_mesh(FieldMotionRecord *effect, s32 mesh_index, s32 *packet_cursor, s32 *ordering_table)
{
    s32 unused[2]; /* never used; the original stack frame reserves it */
    MATRIX transform;
    MATRIX base_matrix;
    MATRIX *transform_matrix;
    CVECTOR base_color;
    s32 triangle_area;
    FieldActorState *actor;
    FieldActorPartDef *part;
    u8 *face_data;
    s32 *screen_vertices;
    s32 *depth_offsets;
    s16 *screen_origin;
    s32 primitive_kind;
    s32 packed_color;
    s32 *ordering_table_base;

    transform_matrix = &base_matrix;
    ordering_table_base = ordering_table;
    transform_matrix = &transform;
    part = &g_field_actor_slots[effect->actor_index].parts[effect->part_index];
    actor = &g_field_actor_slots[effect->actor_index];

    func_80082C90(actor, effect, part, transform_matrix, &base_matrix);
    transform.t[2] = 0;
    transform.t[1] = 0;
    transform.t[0] = 0;
    field_resolve_effect_part_color(actor, effect, part, (FieldPrimitiveColor*)&base_color);
    screen_origin = FIELD_MESH_SCREEN_ORIGIN;
    gte_SetRotMatrix(transform_matrix);
    gte_SetTransMatrix(transform_matrix);
    func_800822A4(actor, effect, part, mesh_index);

    screen_vertices = (s32 *)g_field_mesh_screen_vertices;
    depth_offsets = g_field_mesh_depth_offsets;
    face_data = FIELD_ACTOR_MESH(actor, mesh_index)->faces;
    screen_origin[0] = FIELD_MESH_SCREEN_CENTER_X + g_field_view_offset_x / 256 + effect->x / 256;
    screen_origin[1] = FIELD_MESH_SCREEN_CENTER_Y + g_field_view_offset_y / 256 + effect->y / 256 - effect->z / 512 - g_field_view_offset_z / 512;
    primitive_kind = (face_data[6] >> 1) & 0xF;

    switch (primitive_kind)
    {
    case 0:
    {
        s32 face_count;
        volatile u8 *packet;
        s32 primitive_code;
        s32 address_mask;
        s32 tag_mask;
        face_count = FIELD_ACTOR_MESH(actor, mesh_index)->face_count;
        if (face_count != 0)
        {
            packet = (u8 *)packet_cursor + 0xE;
            primitive_code = 0x24;
            do
            {
                address_mask = FIELD_OT_ADDRESS_MASK;
            } while (0);
            do
            {
                do
                {
                    do
                    {
                        tag_mask = FIELD_OT_TAG_MASK;
                    } while (0);
                } while (0);
            } while (0);
            do
            {
                gte_ldsxy3(screen_vertices[0], screen_vertices[1], screen_vertices[2]);
                gte_nclip();
                gte_stopz(&triangle_area);
                if (triangle_area > 0)
                {
                    packed_color = *(s32*)&base_color;
                    packet[-11] = 7;
                    *(volatile s32 *)(packet - 10) = packed_color;
                    packet[-7] = primitive_code;
                    if (effect->flags & FIELD_EFFECT_SEMITRANSPARENT)
                    {
                        packet[-7] = 0x26;
                    }
                    *(volatile s32 *)(packet - 6) = screen_vertices[0];
                    *(volatile s32 *)(packet + 2) = screen_vertices[1];
                    *(volatile s32 *)(packet + 10) = screen_vertices[2];
                    *(volatile u16 *)(packet - 6) += (u16)screen_origin[0];
                    *(volatile u16 *)(packet - 4) += (u16)screen_origin[1];
                    *(volatile u16 *)(packet + 2) += (u16)screen_origin[0];
                    *(volatile u16 *)(packet + 4) += (u16)screen_origin[1];
                    *(volatile u16 *)(packet + 10) += (u16)screen_origin[0];
                    *(volatile u16 *)(packet + 12) += (u16)screen_origin[1];
                    *(volatile u16 *)(packet - 2) = *(u16 *)face_data;
                    do
            {
                    *(volatile u16 *)(packet + 6) = *(u16 *)(face_data + 2);
                    *(volatile u16 *)(packet + 14) = *(u16 *)(face_data + 4);
                    } while (0);
                    if (actor->owner_object_index < 2)
                    {
                        {
                            s32 owner_index;
                            s32 palette_index;
                            owner_index = actor->owner_object_index;
                            palette_index = part->appearance.fields.palette_selector;
                            *(volatile u16 *)(packet + 0) = ((owner_index << 7) + 0x7B80) | (palette_index & 0x3F);
                        }
                        *(s16 *)((u8 *)packet_cursor + (packet - (volatile u8 *)packet_cursor) + 8) =
                            ((part->spawn_flags.word >> 15) & 0x80) | ((part->behavior_flags.word >> 17) & 0x60) | 0x10 |
                            ((((actor->owner_object_index << 6) + 0x340) & 0x3FF) >> 6);
                    }
                    else
                    {
                        *(volatile u16 *)(packet + 0) = (part->appearance.fields.palette_selector & 0x3F) | 0x7C80;
                        *(volatile s16 *)(packet + 8) = ((part->spawn_flags.word >> 15) & 0x80) | ((part->behavior_flags.word >> 17) & 0x60) | 5;
                    }
                    if ((part->track_flags.word >> 21) & 1)
                    {
                        *(volatile u16 *)packet = (*(volatile u16 *)packet & 0xFFC0) + 0x40;
                    }
                    {
                        s32 base_depth = effect->z >> 7;
                        s32 depth_offset = *depth_offsets;
                        s32 depth_index = base_depth + depth_offset;
                        if (depth_index < 0)
                        {
                            s32 old_tag;
                            s32 packet_address;
                            packet += 0x20;
                            old_tag = *packet_cursor;
                            *packet_cursor = (old_tag & tag_mask) | (ordering_table_base[0] & address_mask);
                            packet_address = (s32)packet_cursor & address_mask;
                            packet_cursor = (s32 *)((u8 *)packet_cursor + 0x20);
                            ordering_table_base[0] = (ordering_table_base[0] & tag_mask) | packet_address;
                        }
                        else if (depth_index > FIELD_MESH_OT_MAX_DEPTH)
                        {
                            s32 old_tag;
                            s32 packet_address;
                            packet += 0x20;
                            old_tag = *packet_cursor;
                            *packet_cursor = (old_tag & tag_mask) | (ordering_table_base[FIELD_MESH_OT_MAX_DEPTH] & address_mask);
                            packet_address = (s32)packet_cursor & address_mask;
                            packet_cursor = (s32 *)((u8 *)packet_cursor + 0x20);
                            ordering_table_base[FIELD_MESH_OT_MAX_DEPTH] = (ordering_table_base[FIELD_MESH_OT_MAX_DEPTH] & tag_mask) | packet_address;
                        }
                        else
                        {
                            s32 packet_address;
                            s32 *ordering_entry;
                            packet += 0x20;
                            packet_address = (s32)packet_cursor & address_mask;
                            *packet_cursor = (*packet_cursor & tag_mask) |
                                             (*((s32 *)((depth_offset << 2) + ((base_depth << 2) + (s32)ordering_table_base))) & address_mask);
                            {
                                s32 record_depth = effect->z >> 7;
                                s32 face_depth_offset = *depth_offsets;
                                ordering_entry = (s32 *)((face_depth_offset << 2) + ((record_depth << 2) + (s32)ordering_table_base));
                            }
                            packet_cursor = (s32 *)((u8 *)packet_cursor + 0x20);
                            *ordering_entry = (*ordering_entry & tag_mask) | packet_address;
                        }
                    }
                }
                face_data += FIELD_MESH_FACE_STRIDE;
                face_count--;
                screen_vertices += 3;
                depth_offsets++;
            } while (face_count != 0);
        }
        return packet_cursor;
    }
    case 1:
    {
        s32 face_count;
        s32 *packet = packet_cursor;
        u8 *face_color;
        s32 base_depth, depth_offset, depth_index;
        s32 primitive_code;
        s32 address_mask;
        s32 tag_mask;
        face_count = FIELD_ACTOR_MESH(actor, mesh_index)->face_count;
        if (face_count != 0)
        {
            primitive_code = 0x20;
            address_mask = FIELD_OT_ADDRESS_MASK;
            tag_mask = FIELD_OT_TAG_MASK;
            do
            {
                gte_ldsxy3(screen_vertices[0], screen_vertices[1], screen_vertices[2]);
                gte_nclip();
                gte_stopz(&triangle_area);
                if (triangle_area > 0)
                {
                    face_color = face_data + 2;
                    ((u8 *)packet)[4] = face_data[0] + base_color.r - 0x80;
                    ((u8 *)packet)[5] = face_color[-1] + base_color.g - 0x80;
                    ((u8 *)packet)[6] = face_color[0] + base_color.b - 0x80;
                    *(s32 *)((u8 *)packet + 8) = screen_vertices[0];
                    *(s32 *)((u8 *)packet + 12) = screen_vertices[1];
                    *(s32 *)((u8 *)packet + 16) = screen_vertices[2];
                    *(u16 *)((u8 *)packet + 8) += *(u16 *)screen_origin;
                    *(u16 *)((u8 *)packet + 10) += *(u16 *)((u8 *)screen_origin + 2);
                    *(u16 *)((u8 *)packet + 12) += *(u16 *)screen_origin;
                    *(u16 *)((u8 *)packet + 14) += *(u16 *)((u8 *)screen_origin + 2);
                    *(u16 *)((u8 *)packet + 16) += *(u16 *)screen_origin;
                    *(u16 *)((u8 *)packet + 18) += *(u16 *)((u8 *)screen_origin + 2);
                    ((u8 *)packet)[3] = 4;
                    ((u8 *)packet)[7] = primitive_code;
                    if (effect->flags & FIELD_EFFECT_SEMITRANSPARENT)
                    {
                        ((u8 *)packet)[7] = 0x22;
                    }

                    depth_offset = *depth_offsets;
                    base_depth = effect->z >> 7;
                    depth_index = base_depth + depth_offset;
                    if (depth_index < 0)
                    {
                        addPrim(ordering_table_base, packet);
                        packet += 5;
                    }
                    else if (depth_index > FIELD_MESH_OT_MAX_DEPTH)
                    {
                        addPrim(&ordering_table_base[FIELD_MESH_OT_MAX_DEPTH], packet);
                        packet += 5;
                    }
                    else
                    {
                        addPrim(ordering_table_base + (effect->z >> 7) + *depth_offsets, packet);
                        packet += 5;
                    }

                    ((u8 *)packet)[3] = 1;
                    {
                        s32 draw_mode_bits = part->spawn_flags.word >> 15;
                        s32 texture_mode_bits = part->behavior_flags.word >> 17;
                        texture_mode_bits &= 0x60;
                        draw_mode_bits &= 0x80;
                        draw_mode_bits |= texture_mode_bits;
                        draw_mode_bits |= 0xE1000000;
                        *(s32 *)((u8 *)packet + 4) = draw_mode_bits;
                    }
                    depth_offset = *depth_offsets;
                    base_depth = effect->z >> 7;
                    depth_index = base_depth + depth_offset;
                    {
                        s32 *next_packet;
                        if (depth_index < 0)
                        {
                            next_packet = packet + 2;
                            addPrim(ordering_table_base, packet);
                        }
                        else if (depth_index > FIELD_MESH_OT_MAX_DEPTH)
                        {
                            next_packet = packet + 2;
                            addPrim(&ordering_table_base[FIELD_MESH_OT_MAX_DEPTH], packet);
                        }
                        else
                        {
                            next_packet = packet + 2;
                            addPrim(ordering_table_base + (effect->z >> 7) + *depth_offsets, packet);
                        }
                        packet = next_packet;
                    }
                }
                face_data += FIELD_MESH_FACE_STRIDE;
                do
                {
                    face_count--;
                } while (0);
                screen_vertices += 3;
                depth_offsets++;
            } while (face_count != 0);
        }
        packet_cursor = packet;
        break;
    }
    case 2:
    {
        s32 face_count;
        u8 *packet_bytes;
        s32 address_mask;
        s32 tag_mask;
        u8 primitive_code;
        face_count = FIELD_ACTOR_MESH(actor, mesh_index)->face_count;
        do
        {
            do
            {
                do
                {
                    do
                    {
                        do
                        {
                            packet_bytes = (u8 *)packet_cursor;
                        } while (0);
                    } while (0);
                } while (0);
            } while (0);
        } while (0);
        if (face_count != 0)
        {
            do
            {
                do
                {
                    do
                    {
                        address_mask = FIELD_OT_ADDRESS_MASK;
                    } while (0);
                } while (0);
            } while (0);
            do
            {
                do
                {
                    do
                    {
                        do
                        {
                            do
                            {
                                do
                                {
                                    tag_mask = FIELD_OT_TAG_MASK;
                                } while (0);
                            } while (0);
                        } while (0);
                    } while (0);
                } while (0);
            } while (0);
            do
            {
                s32 screen_vertex0 = screen_vertices[0], screen_vertex1 = screen_vertices[1], screen_vertex2 = screen_vertices[2];
                gte_ldsxy3(screen_vertex0, screen_vertex1, screen_vertex2);
                gte_nclip();
                gte_stopz(&triangle_area);
                if (triangle_area > 0)
                {
                    CLAMP_COLOR_CHANNEL(packet_bytes[4], face_data[7] + base_color.r - 0x80);
                    CLAMP_COLOR_CHANNEL(packet_bytes[5], face_data[8] + base_color.g - 0x80);
                    CLAMP_COLOR_CHANNEL(packet_bytes[6], face_data[9] + base_color.b - 0x80);
                    CLAMP_COLOR_CHANNEL(packet_bytes[16], face_data[10] + base_color.r - 0x80);
                    CLAMP_COLOR_CHANNEL(packet_bytes[17], face_data[11] + base_color.g - 0x80);
                    CLAMP_COLOR_CHANNEL(packet_bytes[18], face_data[12] + base_color.b - 0x80);
                    CLAMP_COLOR_CHANNEL(packet_bytes[28], face_data[13] + base_color.r - 0x80);
                    CLAMP_COLOR_CHANNEL(packet_bytes[29], face_data[14] + base_color.g - 0x80);
                    CLAMP_COLOR_CHANNEL(packet_bytes[(packet_bytes[3] = 9, primitive_code = 0x34, 30)], face_data[15] + base_color.b - 0x80);
                    packet_bytes[7] = primitive_code;
                    setSemiTrans(packet_bytes, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);
                    do
                    {
                        *(s32 *)(packet_bytes + 8) = screen_vertices[0];
                        *(s32 *)(packet_bytes + 20) = screen_vertices[1];
                        *(s32 *)(packet_bytes + 32) = screen_vertices[2];
                    } while (0);
                    *(u16 *)(packet_bytes + 8) += (u16)screen_origin[0];
                    *(u16 *)(packet_bytes + 10) += (u16)screen_origin[1];
                    *(u16 *)(packet_bytes + 20) += (u16)screen_origin[0];
                    *(u16 *)(packet_bytes + 22) += (u16)screen_origin[1];
                    *(u16 *)(packet_bytes + 32) += (u16)screen_origin[0];
                    *(u16 *)(packet_bytes + 34) += (u16)screen_origin[1];
                    *(u16 *)(packet_bytes + 12) = *(u16 *)face_data;
                    *(u16 *)(packet_bytes + 24) = *(u16 *)(face_data + 2);
                    *(u16 *)(packet_bytes + 36) = *(u16 *)(face_data + 4);
                    if (actor->owner_object_index < 2)
                    {
                        {
                            s32 owner_index;
                            s32 palette_index;
                            owner_index = actor->owner_object_index;
                            palette_index = part->appearance.fields.palette_selector;
                            *(u16 *)(packet_bytes + 14) = ((owner_index << 7) + 0x7B80) | (palette_index & 0x3F);
                        }
                        *(s16 *)(packet_bytes + 26) = ((part->spawn_flags.word >> 15) & 0x80) | ((part->behavior_flags.word >> 17) & 0x60) | 0x10 |
                                                        ((((actor->owner_object_index << 6) + 0x340) & 0x3FF) >> 6);
                    }
                    else
                    {
                        *(u16 *)(packet_bytes + 14) = (part->appearance.fields.palette_selector & 0x3F) | 0x7C80;
                        *(s16 *)(packet_bytes + 26) = ((part->spawn_flags.word >> 15) & 0x80) |
                                                        ((part->behavior_flags.word >> 17) & 0x60) | 5;
                    }
                    if ((part->track_flags.word >> 21) & 1)
                    {
                        *(u16 *)(packet_bytes + 14) = (*(u16 *)(packet_bytes + 14) & 0xFFC0) + 0x40;
                    }
                    {
                        s32 depth_offset = *depth_offsets;
                        s32 base_depth = effect->z >> 7;
                        s32 depth_index = base_depth + depth_offset;
                        if (depth_index < 0)
                        {
                            s32 packet_address;
                            *(s32 *)packet_bytes = (*(s32 *)packet_bytes & tag_mask) | (ordering_table_base[0] & address_mask);
                            packet_address = (s32)packet_bytes & address_mask;
                            packet_bytes += 0x28;
                            ordering_table_base[0] = (ordering_table_base[0] & tag_mask) | packet_address;
                        }
                        else if (depth_index > FIELD_MESH_OT_MAX_DEPTH)
                        {
                            s32 packet_address;
                            *(s32 *)packet_bytes = (*(s32 *)packet_bytes & tag_mask) | (ordering_table_base[FIELD_MESH_OT_MAX_DEPTH] & address_mask);
                            packet_address = (s32)packet_bytes & address_mask;
                            packet_bytes += 0x28;
                            ordering_table_base[FIELD_MESH_OT_MAX_DEPTH] = (ordering_table_base[FIELD_MESH_OT_MAX_DEPTH] & tag_mask) | packet_address;
                        }
                        else
                        {
                            s32 packet_address;
                            s32 *ordering_entry;
                            packet_address = (s32)packet_bytes & address_mask;
                            *(s32 *)packet_bytes = (*(s32 *)packet_bytes & tag_mask) |
                                                   (*((s32 *)((depth_offset << 2) + ((base_depth << 2) + (s32)ordering_table_base))) & address_mask);
                            {
                                s32 record_depth;
                                s32 face_depth_offset;
                                record_depth = effect->z >> 7;
                                face_depth_offset = *depth_offsets;
                                ordering_entry = (s32 *)((face_depth_offset << 2) + ((record_depth << 2) + (s32)ordering_table_base));
                            }
                            packet_bytes += 0x28;
                            *ordering_entry = (*ordering_entry & tag_mask) | packet_address;
                        }
                    }
                }
                face_data += FIELD_MESH_FACE_STRIDE;
                do
                {
                    do
                    {
                        do
                        {
                            face_count--;
                        } while (0);
                    } while (0);
                } while (0);
                screen_vertices += 3;
                depth_offsets++;
            } while (face_count != 0);
        }
        packet_cursor = (s32 *)packet_bytes;
        break;
    }
    default:
        return packet_cursor;
    }
    return packet_cursor;
}

extern SVECTOR D_800FF668;

#define CLAMP_LIT_COLOR_CHANNEL(dst, expr) { \
    s32 _v = (expr); \
    s32 _out; \
    if (_v >= 0) { \
        _out = 0xFF; \
        if (_v < 0x100) { \
            _out = _v; \
        } \
    } else { \
        _out = 0; \
    } \
    (dst) = _out; \
}

/**
 * @brief Emit lit ordering-table primitives for one mesh of an actor part.
 *
 * Builds the part light and color matrices, transforms its normals, and emits
 * the mesh's lit triangle primitives into the ordering table.
 *
 * @param effect Effect record driving the mesh transform.
 * @param mesh_index Mesh resource index to render.
 * @param packet_cursor Current primitive-buffer write cursor.
 * @param ordering_table Ordering table receiving the generated primitives.
 * @return Advanced primitive-buffer cursor.
 * @see decomp.me (100%) https://decomp.me/scratch/F2Z4z
 */
s32 *field_render_lit_effect_mesh(FieldMotionRecord *effect, s32 mesh_index, s32 *packet_cursor, s32 *ordering_table)
{
    s32 unused[2]; /* never used; the original stack frame reserves it */
    s32 *write_cursor;
    MATRIX transform;
    MATRIX base_matrix;
    CVECTOR base_color;
    MATRIX light_matrix;
    MATRIX color_matrix;
    SVECTOR light_direction;
    SVECTOR transformed_light;
    SVECTOR *effect_rotation_base;
    s32 triangle_area;
    FieldActorState *actor;
    FieldActorPartDef *part;
    FieldMotionRecord *light_effect;
    s32 light_index;
    s32 effect_offset;
    s32 *screen_vertices;
    SVECTOR *transformed_normals;
    s32 *depth_offsets;
    s16 *screen_origin;
    u8 *face_data;
    s32 count;
    s32 primitive_kind;

    write_cursor = packet_cursor;
    part = &g_field_actor_slots[effect->actor_index].parts[effect->part_index];
    actor = &g_field_actor_slots[effect->actor_index];

    func_80082C90(actor, effect, part, &transform, &base_matrix);
    transform.t[2] = 0;
    transform.t[1] = 0;
    transform.t[0] = 0;
    field_resolve_effect_part_color(actor, effect, part, (FieldPrimitiveColor*)&base_color);
    screen_origin = FIELD_MESH_SCREEN_ORIGIN;
    gte_SetRotMatrix(&transform);
    gte_SetTransMatrix(&transform);
    func_800822A4(actor, effect, part, mesh_index);
    func_800829A0(actor, effect, part, mesh_index, &base_matrix);

    light_index = 0;
    effect_rotation_base = &D_800FF668;

    do
    {
        count = 0;
        if (FIELD_PART_LIGHT_SOURCE(part, light_index) < 8)
        {
            do
            {
                light_effect = &g_field_effect_records[count];
                effect_offset = count * 0x54;
                if (FIELD_PART_LIGHT_SOURCE(part, light_index) == light_effect->part_index && effect->actor_index == light_effect->actor_index)
                {
                    RotMatrix_gte((SVECTOR *)(effect_offset + (s32)effect_rotation_base), &base_matrix);
                    RotMatrixZ(effect->rotation_z_16 * 0x10, &base_matrix);
                    RotMatrixY(effect->rotation_y_16 * 0x10, &base_matrix);
                    light_direction.vz = 0;
                    light_direction.vx = 0;
                    light_direction.vy = -0x1000;
                    gte_SetRotMatrix(&base_matrix);
                    gte_ldv0(&light_direction);
                    gte_rtv0();
                    gte_stsv(&transformed_light);
                    light_matrix.m[light_index][0] = transformed_light.vx;
                    light_matrix.m[light_index][1] = transformed_light.vy;
                    light_matrix.m[light_index][2] = transformed_light.vz;
                    color_matrix.m[0][light_index] = g_field_actor_slots[effect->actor_index].parts[light_effect->part_index].red_or_track * 0x10;
                    color_matrix.m[1][light_index] = g_field_actor_slots[effect->actor_index].parts[light_effect->part_index].green_or_track * 0x10;
                    color_matrix.m[2][light_index] = g_field_actor_slots[effect->actor_index].parts[light_effect->part_index].blue_or_track * 0x10;
                    break;
                }
                count++;
            } while (count < FIELD_EFFECT_ACTIVE_RECORD_COUNT);
            if (count == FIELD_EFFECT_ACTIVE_RECORD_COUNT)
            {
                color_matrix.m[2][light_index] = 0;
                color_matrix.m[1][light_index] = 0;
                color_matrix.m[0][light_index] = 0;
                light_matrix.m[light_index][0] = 0;
            }
        }
        else
        {
            color_matrix.m[2][light_index] = 0;
            color_matrix.m[1][light_index] = 0;
            color_matrix.m[0][light_index] = 0;
            light_matrix.m[light_index][0] = 0;
        }
        light_index++;
    } while (light_index < 3);

    gte_SetLightMatrix(&light_matrix);
    gte_SetColorMatrix(&color_matrix);
    screen_vertices = (s32 *)g_field_mesh_screen_vertices;
    transformed_normals = g_field_mesh_transformed_normals;
    depth_offsets = g_field_mesh_depth_offsets;
    face_data = FIELD_ACTOR_MESH(actor, mesh_index)->faces;
    screen_origin[0] = FIELD_MESH_SCREEN_CENTER_X + g_field_view_offset_x / 256 + effect->x / 256;
    screen_origin[1] = FIELD_MESH_SCREEN_CENTER_Y + g_field_view_offset_y / 256 + effect->y / 256 - effect->z / 512 - g_field_view_offset_z / 512;
    primitive_kind = (face_data[6] >> 1) & 0xF;

    switch (primitive_kind)
    {
    case 0:
    {
        u8 *packet;
        s32 primitive_code;
        s32 base_depth, depth_offset, depth_index;

        gte_SetBackColor(base_color.r, base_color.g, base_color.b);
        if (actor->owner_object_index < 2)
        {
            {
                s32 owner_index;
                s32 palette_index;
                owner_index = actor->owner_object_index;
                palette_index = part->appearance.fields.palette_selector;
                *(u16 *)((u8 *)write_cursor + 0xE) = ((owner_index << 7) + 0x7B80) | (palette_index & 0x3F);
            }
            *(s16 *)((u8 *)write_cursor + 0x16) =
                ((part->spawn_flags.word >> 15) & 0x80) | ((part->behavior_flags.word >> 17) & 0x60) | 0x10 |
                ((((actor->owner_object_index << 6) + 0x340) & 0x3FF) >> 6);
        }
        else
        {
            *(u16 *)((u8 *)write_cursor + 0xE) = (part->appearance.fields.palette_selector & 0x3F) | 0x7C80;
            *(s16 *)((u8 *)write_cursor + 0x16) = ((part->spawn_flags.word >> 15) & 0x80) | ((part->behavior_flags.word >> 17) & 0x60) | 5;
        }
        if ((part->track_flags.word >> 21) & 1)
        {
            *(u16 *)((u8 *)write_cursor + 0xE) = (*(u16 *)((u8 *)write_cursor + 0xE) & 0xFFC0) + 0x40;
        }

        count = FIELD_ACTOR_MESH(actor, mesh_index)->face_count;
        if (count != 0)
        {
            primitive_code = 0x24;
            do
            {
                packet = (u8 *)write_cursor + 0x36;
                gte_ldsxy3(screen_vertices[0], screen_vertices[1], screen_vertices[2]);
                gte_nclip();
                gte_stopz(&triangle_area);
                if (triangle_area > 0)
                {
                    gte_ldv0(transformed_normals);
                    gte_ncs();
                    gte_strgb((u8 *)write_cursor + 4);
                    *(s32 *)(packet - 0x2E) = screen_vertices[0];
                    *(s32 *)(packet - 0x26) = screen_vertices[1];
                    *(s32 *)(packet - 0x1E) = screen_vertices[2];
                    *(u16 *)(packet - 0x2E) += (u16)screen_origin[0];
                    *(u16 *)(packet - 0x2C) += (u16)screen_origin[1];
                    *(u16 *)(packet - 0x26) += (u16)screen_origin[0];
                    *(u16 *)(packet - 0x24) += (u16)screen_origin[1];
                    *(u16 *)(packet - 0x1E) += (u16)screen_origin[0];
                    *(u16 *)(packet - 0x1C) += (u16)screen_origin[1];
                    packet[-0x33] = 7;
                    packet[-0x2F] = primitive_code;
                    if (effect->flags & FIELD_EFFECT_SEMITRANSPARENT)
                    {
                        packet[-0x2F] = 0x26;
                    }
                    else
                    {
                        packet[-0x2F] = primitive_code;
                    }
                    *(u16 *)(packet - 0x2A) = *(u16 *)face_data;
                    *(u16 *)(packet - 0x22) = *(u16 *)(face_data + 2);
                    *(u16 *)(packet - 0x1A) = *(u16 *)(face_data + 4);
                    *(u16 *)(packet - 8) = *(u16 *)(packet - 0x28);
                    *(u16 *)((u8 *)write_cursor + 0x36) = *(u16 *)(packet - 0x20);

                    depth_offset = *depth_offsets;
                    base_depth = effect->z >> 7;
                    depth_index = base_depth + depth_offset;
                    if (depth_index < 0)
                    {
                        addPrim(ordering_table, write_cursor);
                        write_cursor += 8;
                    }
                    else if (depth_index > FIELD_MESH_OT_MAX_DEPTH)
                    {
                        addPrim(&ordering_table[FIELD_MESH_OT_MAX_DEPTH], write_cursor);
                        write_cursor += 8;
                    }
                    else
                    {
                        addPrim(ordering_table + (effect->z >> 7) + *depth_offsets, write_cursor);
                        write_cursor += 8;
                    }
                }
                face_data += FIELD_MESH_FACE_STRIDE;
                count--;
                screen_vertices += 3;
                transformed_normals++;
                depth_offsets++;
            } while (count != 0);
        }
        return write_cursor;
    }
    case 2:
    {
        u8 *packet_bytes;
        count = FIELD_ACTOR_MESH(actor, mesh_index)->face_count;
        packet_bytes = (u8 *)write_cursor;
        if (count != 0)
        {
            do
            {
                gte_ldsxy3(screen_vertices[0], screen_vertices[1], screen_vertices[2]);
                gte_nclip();
                gte_stopz(&triangle_area);
                if (triangle_area > 0)
                {
                    CLAMP_LIT_COLOR_CHANNEL(packet_bytes[4], face_data[7] + base_color.r - 0x80);
                    CLAMP_LIT_COLOR_CHANNEL(packet_bytes[5], face_data[8] + base_color.g - 0x80);
                    CLAMP_LIT_COLOR_CHANNEL(packet_bytes[6], face_data[9] + base_color.b - 0x80);
                    CLAMP_LIT_COLOR_CHANNEL(packet_bytes[16], face_data[10] + base_color.r - 0x80);
                    CLAMP_LIT_COLOR_CHANNEL(packet_bytes[17], face_data[11] + base_color.g - 0x80);
                    CLAMP_LIT_COLOR_CHANNEL(packet_bytes[18], face_data[12] + base_color.b - 0x80);
                    CLAMP_LIT_COLOR_CHANNEL(packet_bytes[28], face_data[13] + base_color.r - 0x80);
                    CLAMP_LIT_COLOR_CHANNEL(packet_bytes[29], face_data[14] + base_color.g - 0x80);
                    {
                        s32 blue_channel;
                        CLAMP_LIT_COLOR_CHANNEL(blue_channel, face_data[15] + base_color.b - 0x80);
                        setlen(packet_bytes, 9);
                        packet_bytes[30] = blue_channel;
                    }
                    setcode(packet_bytes, 0x34);
                    setSemiTrans(packet_bytes, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);
                    *(s32 *)(packet_bytes + 8) = screen_vertices[0];
                    *(s32 *)(packet_bytes + 20) = screen_vertices[1];
                    *(s32 *)(packet_bytes + 32) = screen_vertices[2];
                    *(u16 *)(packet_bytes + 8) += (u16)screen_origin[0];
                    *(u16 *)(packet_bytes + 10) += (u16)screen_origin[1];
                    *(u16 *)(packet_bytes + 20) += (u16)screen_origin[0];
                    *(u16 *)(packet_bytes + 22) += (u16)screen_origin[1];
                    *(u16 *)(packet_bytes + 32) += (u16)screen_origin[0];
                    *(u16 *)(packet_bytes + 34) += (u16)screen_origin[1];
                    *(u16 *)(packet_bytes + 12) = *(u16 *)face_data;
                    *(u16 *)(packet_bytes + 24) = *(u16 *)(face_data + 2);
                    *(u16 *)(packet_bytes + 36) = *(u16 *)(face_data + 4);
                    if (actor->owner_object_index < 2)
                    {
                        {
                            s32 owner_index;
                            s32 palette_index;
                            owner_index = actor->owner_object_index;
                            palette_index = part->appearance.fields.palette_selector;
                            *(u16 *)(packet_bytes + 14) = ((owner_index << 7) + 0x7B80) | (palette_index & 0x3F);
                        }
                        *(s16 *)(packet_bytes + 26) = ((part->spawn_flags.word >> 15) & 0x80) | ((part->behavior_flags.word >> 17) & 0x60) | 0x10 |
                                                        ((((actor->owner_object_index << 6) + 0x340) & 0x3FF) >> 6);
                    }
                    else
                    {
                        *(u16 *)(packet_bytes + 14) = (part->appearance.fields.palette_selector & 0x3F) | 0x7C80;
                        *(s16 *)(packet_bytes + 26) = ((part->spawn_flags.word >> 15) & 0x80) |
                                                        ((part->behavior_flags.word >> 17) & 0x60) | 5;
                    }
                    if ((part->track_flags.word >> 21) & 1)
                    {
                        *(u16 *)(packet_bytes + 14) = (*(u16 *)(packet_bytes + 14) & 0xFFC0) + 0x40;
                    }
                    {
                        s32 depth_offset = *depth_offsets;
                        s32 base_depth = effect->z >> 7;
                        s32 depth_index = base_depth + depth_offset;
                        if (depth_index < 0)
                        {
                            addPrim(ordering_table, packet_bytes);
                            packet_bytes += 0x28;
                        }
                        else
                        {
                            if (depth_index > FIELD_MESH_OT_MAX_DEPTH)
                            {
                                addPrim(&ordering_table[FIELD_MESH_OT_MAX_DEPTH], packet_bytes);
                                packet_bytes += 0x28;
                            }
                            else
                            {
                                {
                                    s32 record_depth;
                                    s32 face_depth_offset;
                                    addPrim((record_depth = effect->z >> 7, face_depth_offset = *depth_offsets,
                                             (s32 *)((face_depth_offset << 2) + ((record_depth << 2) + (s32)ordering_table))),
                                            packet_bytes);
                                }
                                packet_bytes += 0x28;
                            }
                        }
                    }
                }
                face_data += FIELD_MESH_FACE_STRIDE;
                count--;
                screen_vertices += 3;
                depth_offsets++;
            } while (count != 0);
        }
        write_cursor = (s32 *)packet_bytes;
        break;
    }
    case 1:
    {
        s32 *packet = write_cursor;
        u8 *face_color;
        s32 base_depth, depth_offset, depth_index;
        s32 primitive_code;
        count = FIELD_ACTOR_MESH(actor, mesh_index)->face_count;
        if (count != 0)
        {
            primitive_code = 0x20;
            do
            {
                gte_ldsxy3(screen_vertices[0], screen_vertices[1], screen_vertices[2]);
                gte_nclip();
                gte_stopz(&triangle_area);
                if (triangle_area > 0)
                {
                    face_color = face_data + 2;
                    gte_SetBackColor(face_data[0] + base_color.r - 0x80,
                                     face_color[-1] + base_color.g - 0x80,
                                     face_color[0] + base_color.b - 0x80);
                    gte_ldv0(transformed_normals);
                    gte_ncs();
                    gte_strgb((u8 *)packet + 4);
                    *(s32 *)((u8 *)packet + 8) = screen_vertices[0];
                    *(s32 *)((u8 *)packet + 12) = screen_vertices[1];
                    *(s32 *)((u8 *)packet + 16) = screen_vertices[2];
                    *(u16 *)((u8 *)packet + 8) += *(u16 *)screen_origin;
                    *(u16 *)((u8 *)packet + 10) += *(u16 *)((u8 *)screen_origin + 2);
                    *(u16 *)((u8 *)packet + 12) += *(u16 *)screen_origin;
                    *(u16 *)((u8 *)packet + 14) += *(u16 *)((u8 *)screen_origin + 2);
                    *(u16 *)((u8 *)packet + 16) += *(u16 *)screen_origin;
                    *(u16 *)((u8 *)packet + 18) += *(u16 *)((u8 *)screen_origin + 2);
                    ((u8 *)packet)[3] = 4;
                    ((u8 *)packet)[7] = primitive_code;
                    if (effect->flags & FIELD_EFFECT_SEMITRANSPARENT)
                    {
                        ((u8 *)packet)[7] = 0x22;
                    }
                    else
                    {
                        ((u8 *)packet)[7] = primitive_code;
                    }

                    depth_offset = *depth_offsets;
                    base_depth = effect->z >> 7;
                    depth_index = base_depth + depth_offset;
                    if (depth_index < 0)
                    {
                        addPrim(ordering_table, packet);
                        packet += 5;
                    }
                    else if (depth_index > FIELD_MESH_OT_MAX_DEPTH)
                    {
                        addPrim(&ordering_table[FIELD_MESH_OT_MAX_DEPTH], packet);
                        packet += 5;
                    }
                    else
                    {
                        addPrim(ordering_table + (effect->z >> 7) + *depth_offsets, packet);
                        packet += 5;
                    }

                    ((u8 *)packet)[3] = 1;
                    {
                        s32 draw_mode_bits = part->spawn_flags.word >> 15;
                        s32 texture_mode_bits = part->behavior_flags.word >> 17;
                        texture_mode_bits &= 0x60;
                        draw_mode_bits &= 0x80;
                        draw_mode_bits |= texture_mode_bits;
                        draw_mode_bits |= 0xE1000000;
                        *(s32 *)((u8 *)packet + 4) = draw_mode_bits;
                    }
                    depth_offset = *depth_offsets;
                    base_depth = effect->z >> 7;
                    depth_index = base_depth + depth_offset;
                    {
                        s32 *next_packet;
                        if (depth_index < 0)
                        {
                            next_packet = packet + 2;
                            addPrim(ordering_table, packet);
                        }
                        else if (depth_index > FIELD_MESH_OT_MAX_DEPTH)
                        {
                            next_packet = packet + 2;
                            addPrim(&ordering_table[FIELD_MESH_OT_MAX_DEPTH], packet);
                        }
                        else
                        {
                            next_packet = packet + 2;
                            addPrim(ordering_table + (effect->z >> 7) + *depth_offsets, packet);
                        }
                        packet = next_packet;
                    }
                }
                face_data += FIELD_MESH_FACE_STRIDE;
                count--;
                screen_vertices += 3;
                transformed_normals++;
                depth_offsets++;
            } while (count != 0);
        }
        write_cursor = packet;
        break;
    }
    default:
        return write_cursor;
    }
    return write_cursor;
}
