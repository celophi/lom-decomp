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
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

extern FieldMotionRecord g_field_effect_records[];
extern FieldActorState g_field_actor_slots[80];

/** @brief Rotation words of g_field_effect_records (the record base + 0x10). */
extern SVECTOR D_800FF668;

/** @brief Frame period bits of an animation's palette_animation word. */
#define FIELD_PALETTE_ANIMATION_PERIOD_MASK 0xF
/** @brief Mode bits (after >> 4) of an animation's palette_animation word. */
#define FIELD_PALETTE_ANIMATION_MODE_MASK 7

/** @brief Screen-space centre added to every projected mesh vertex. */
#define FIELD_MESH_SCREEN_CENTER_X 160
#define FIELD_MESH_SCREEN_CENTER_Y 112
/** @brief Scratchpad pair (x, y) holding the per-mesh screen offset. */
#define FIELD_MESH_SCREEN_ORIGIN ((s16 *)0x1F800000)
/** @brief Bytes per face record in a mesh's face table. */
#define FIELD_MESH_FACE_STRIDE 16
/** @brief Last ordering-table slot; deeper faces are clamped to it. */
#define FIELD_MESH_OT_MAX_DEPTH 0xFFF

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

/**
 * @brief Clamp @p expr into 0..0xFF and store it into @p dst.
 * @param dst Lvalue receiving the clamped channel.
 * @param expr Signed channel value.
 * @note A plain block, not do/while(0): the extra loop nesting changes register allocation.
 */
#define CLAMP_COLOR_CHANNEL(dst, expr) { \
    s32 _v = (expr); \
    s32 _out; \
    if (_v >= 0) \
    { \
        _out = 0xFF; \
        if (_v < 0x100) \
        { \
            _out = _v; \
        } \
    } \
    else \
    { \
        _out = 0; \
    } \
    (dst) = _out; \
}

/**
 * @brief Link @p prim into the ordering-table slot of the current face, then run @p advance.
 * @param ot Ordering table base.
 * @param effect Effect record whose z (>> 7) is the base depth.
 * @param depth_offsets Cursor into the per-face depth offsets.
 * @param prim Primitive to link.
 * @param advance Statement that moves the packet cursor past @p prim.
 * @note Depths below 0 go to slot 0 and depths past FIELD_MESH_OT_MAX_DEPTH to the last slot.
 */
#define FIELD_MESH_ADD_PRIM(ot, effect, depth_offsets, prim, advance) { \
    s32 depth_offset = *(depth_offsets); \
    s32 base_depth = (effect)->z >> 7; \
    s32 depth_index = base_depth + depth_offset; \
    if (depth_index < 0) \
    { \
        addPrim((ot), (prim)); \
        advance; \
    } \
    else if (depth_index > FIELD_MESH_OT_MAX_DEPTH) \
    { \
        addPrim(&(ot)[FIELD_MESH_OT_MAX_DEPTH], (prim)); \
        advance; \
    } \
    else \
    { \
        addPrim((ot) + ((effect)->z >> 7) + *(depth_offsets), (prim)); \
        advance; \
    } \
}

/**
 * @brief Write the CLUT and texture-page halfwords of a textured mesh packet.
 * @param actor Actor owning the mesh; owners 0 and 1 have their own CLUT rows and pages.
 * @param part Part supplying the palette selector and the draw and texture mode bits.
 * @param clut Lvalue receiving the CLUT halfword.
 * @param tpage Lvalue receiving the texture-page halfword.
 * @note Other owners share CLUT row 0x1F2 and texture page 5; track flag bit 21
 *       moves the CLUT one 16-color block to the right.
 */
#define FIELD_MESH_SET_CLUT_TPAGE(actor, part, clut, tpage) { \
    if ((actor)->owner_object_index < 2) \
    { \
        s32 owner_index; \
        s32 palette_index; \
        owner_index = (actor)->owner_object_index; \
        palette_index = (part)->appearance.fields.palette_selector; \
        (clut) = ((owner_index << 7) + 0x7B80) | (palette_index & 0x3F); \
        (tpage) = (((part)->spawn_flags.word >> 15) & 0x80) | (((part)->behavior_flags.word >> 17) & 0x60) | 0x10 | \
                  (((((actor)->owner_object_index << 6) + 0x340) & 0x3FF) >> 6); \
    } \
    else \
    { \
        (clut) = ((part)->appearance.fields.palette_selector & 0x3F) | 0x7C80; \
        (tpage) = (((part)->spawn_flags.word >> 15) & 0x80) | (((part)->behavior_flags.word >> 17) & 0x60) | 5; \
    } \
    if (((part)->track_flags.word >> 21) & 1) \
    { \
        (clut) = ((clut) & 0xFFC0) + 0x40; \
    } \
}

/**
 * @brief Emit ordering-table primitives for one mesh of an actor part.
 *
 * Builds the part matrix and color, projects the mesh vertices, and emits one
 * primitive per front-facing face; the first face's kind selects the packet type.
 *
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
    CVECTOR base_color;
    s32 triangle_area;
    FieldActorState *actor;
    FieldActorPartDef *part;
    u8 *face_data;
    s32 *screen_vertices;
    s32 *depth_offsets;
    s16 *screen_origin;
    s32 primitive_kind;
    s32 *ordering_table_base;
    s32 face_count;

    screen_origin = FIELD_MESH_SCREEN_ORIGIN;
    ordering_table_base = ordering_table;
    part = &g_field_actor_slots[effect->actor_index].parts[effect->part_index];
    actor = &g_field_actor_slots[effect->actor_index];

    func_80082C90(actor, effect, part, &transform, &base_matrix);
    transform.t[2] = 0;
    transform.t[1] = 0;
    transform.t[0] = 0;
    field_resolve_effect_part_color(actor, effect, part, (FieldPrimitiveColor *)&base_color);
    gte_SetRotMatrix(&transform);
    gte_SetTransMatrix(&transform);
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
        /* Textured flat triangles: one 0x20-byte POLY_FT3 per face. */
        s32 primitive_code;
        face_count = FIELD_ACTOR_MESH(actor, mesh_index)->face_count;
        if (face_count != 0)
        {
            primitive_code = 0x24;
            do
            {
                gte_ldsxy3(screen_vertices[0], screen_vertices[1], screen_vertices[2]);
                gte_nclip();
                gte_stopz(&triangle_area);
                if (triangle_area > 0)
                {
                    /* r, g, b and a placeholder code byte in one word store */
                    s32 packed_color = *(s32 *)&base_color;

                    setlen(packet_cursor, 7);
                    *(s32 *)((u8 *)packet_cursor + 4) = packed_color;
                    setcode(packet_cursor, primitive_code);
                    if (effect->flags & FIELD_EFFECT_SEMITRANSPARENT)
                    {
                        setcode(packet_cursor, 0x26);
                    }
                    *(s32 *)((u8 *)packet_cursor + 8) = screen_vertices[0];
                    *(s32 *)((u8 *)packet_cursor + 16) = screen_vertices[1];
                    *(s32 *)((u8 *)packet_cursor + 24) = screen_vertices[2];
                    *(u16 *)((u8 *)packet_cursor + 8) += (u16)screen_origin[0];
                    *(u16 *)((u8 *)packet_cursor + 10) += (u16)screen_origin[1];
                    *(u16 *)((u8 *)packet_cursor + 16) += (u16)screen_origin[0];
                    *(u16 *)((u8 *)packet_cursor + 18) += (u16)screen_origin[1];
                    *(u16 *)((u8 *)packet_cursor + 24) += (u16)screen_origin[0];
                    *(u16 *)((u8 *)packet_cursor + 26) += (u16)screen_origin[1];
                    *(u16 *)((u8 *)packet_cursor + 12) = *(u16 *)face_data;
                    *(u16 *)((u8 *)packet_cursor + 20) = *(u16 *)(face_data + 2);
                    *(u16 *)((u8 *)packet_cursor + 28) = *(u16 *)(face_data + 4);
                    FIELD_MESH_SET_CLUT_TPAGE(actor, part, *(u16 *)((u8 *)packet_cursor + 14), *(s16 *)((u8 *)packet_cursor + 22));
                    FIELD_MESH_ADD_PRIM(ordering_table_base, effect, depth_offsets, packet_cursor, packet_cursor += 8);
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
        /* Untextured flat triangles: a POLY_F3 (5 words) plus a draw-mode DR_TPAGE (2 words) per face. */
        s32 *packet = packet_cursor;
        u8 *face_color;
        s32 base_depth, depth_offset, depth_index;
        s32 primitive_code;
        face_count = FIELD_ACTOR_MESH(actor, mesh_index)->face_count;
        if (face_count != 0)
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
                    ((u8 *)packet)[4] = face_data[0] + base_color.r - 0x80;
                    ((u8 *)packet)[5] = face_color[-1] + base_color.g - 0x80;
                    ((u8 *)packet)[6] = face_color[0] + base_color.b - 0x80;
                    *(s32 *)((u8 *)packet + 8) = screen_vertices[0];
                    *(s32 *)((u8 *)packet + 12) = screen_vertices[1];
                    *(s32 *)((u8 *)packet + 16) = screen_vertices[2];
                    *(u16 *)((u8 *)packet + 8) += (u16)screen_origin[0];
                    *(u16 *)((u8 *)packet + 10) += (u16)screen_origin[1];
                    *(u16 *)((u8 *)packet + 12) += (u16)screen_origin[0];
                    *(u16 *)((u8 *)packet + 14) += (u16)screen_origin[1];
                    *(u16 *)((u8 *)packet + 16) += (u16)screen_origin[0];
                    *(u16 *)((u8 *)packet + 18) += (u16)screen_origin[1];
                    setlen(packet, 4);
                    setcode(packet, primitive_code);
                    if (effect->flags & FIELD_EFFECT_SEMITRANSPARENT)
                    {
                        setcode(packet, 0x22);
                    }

                    FIELD_MESH_ADD_PRIM(ordering_table_base, effect, depth_offsets, packet, packet += 5);

                    setlen(packet, 1);
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
                face_count--;
                screen_vertices += 3;
                depth_offsets++;
            } while (face_count != 0);
        }
        packet_cursor = packet;
        break;
    }
    case 2:
    {
        /* Textured Gouraud triangles: one 0x28-byte POLY_GT3 per face, colors clamped to 0..0xFF. */
        u8 *packet_bytes;
        face_count = FIELD_ACTOR_MESH(actor, mesh_index)->face_count;
        packet_bytes = (u8 *)packet_cursor;
        if (face_count != 0)
        {
            do
            {
                gte_ldsxy3(screen_vertices[0], screen_vertices[1], screen_vertices[2]);
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
                    {
                        s32 blue_channel;
                        CLAMP_COLOR_CHANNEL(blue_channel, face_data[15] + base_color.b - 0x80);
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
                    FIELD_MESH_SET_CLUT_TPAGE(actor, part, *(u16 *)(packet_bytes + 14), *(s16 *)(packet_bytes + 26));
                    FIELD_MESH_ADD_PRIM(ordering_table_base, effect, depth_offsets, packet_bytes, packet_bytes += 0x28);
                }
                face_data += FIELD_MESH_FACE_STRIDE;
                face_count--;
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
    field_resolve_effect_part_color(actor, effect, part, (FieldPrimitiveColor *)&base_color);
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
                effect_offset = count * sizeof(FieldMotionRecord);
                if (FIELD_PART_LIGHT_SOURCE(part, light_index) == light_effect->part_index && effect->actor_index == light_effect->actor_index)
                {
                    /* offset + base as an int sum: a pointer sum emits the base first */
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
        /* Lit textured triangles: POLY_FT3 packets whose CLUT and tpage are written once up front. */
        s32 primitive_code;

        gte_SetBackColor(base_color.r, base_color.g, base_color.b);
        FIELD_MESH_SET_CLUT_TPAGE(actor, part, *(u16 *)((u8 *)write_cursor + 0xE), *(s16 *)((u8 *)write_cursor + 0x16));

        count = FIELD_ACTOR_MESH(actor, mesh_index)->face_count;
        if (count != 0)
        {
            primitive_code = 0x24;
            do
            {
                gte_ldsxy3(screen_vertices[0], screen_vertices[1], screen_vertices[2]);
                gte_nclip();
                gte_stopz(&triangle_area);
                if (triangle_area > 0)
                {
                    gte_ldv0(transformed_normals);
                    gte_ncs();
                    gte_strgb((u8 *)write_cursor + 4);
                    *(s32 *)((u8 *)write_cursor + 0x8) = screen_vertices[0];
                    *(s32 *)((u8 *)write_cursor + 0x10) = screen_vertices[1];
                    *(s32 *)((u8 *)write_cursor + 0x18) = screen_vertices[2];
                    *(u16 *)((u8 *)write_cursor + 0x8) += (u16)screen_origin[0];
                    *(u16 *)((u8 *)write_cursor + 0xA) += (u16)screen_origin[1];
                    *(u16 *)((u8 *)write_cursor + 0x10) += (u16)screen_origin[0];
                    *(u16 *)((u8 *)write_cursor + 0x12) += (u16)screen_origin[1];
                    *(u16 *)((u8 *)write_cursor + 0x18) += (u16)screen_origin[0];
                    *(u16 *)((u8 *)write_cursor + 0x1A) += (u16)screen_origin[1];
                    setlen(write_cursor, 7);
                    setcode(write_cursor, primitive_code);
                    if (effect->flags & FIELD_EFFECT_SEMITRANSPARENT)
                    {
                        setcode(write_cursor, 0x26);
                    }
                    else
                    {
                        setcode(write_cursor, primitive_code);
                    }
                    *(u16 *)((u8 *)write_cursor + 0xC) = *(u16 *)face_data;
                    *(u16 *)((u8 *)write_cursor + 0x14) = *(u16 *)(face_data + 2);
                    *(u16 *)((u8 *)write_cursor + 0x1C) = *(u16 *)(face_data + 4);
                    /* Carry the CLUT and tpage forward into the next packet. */
                    *(u16 *)((u8 *)write_cursor + 0x2E) = *(u16 *)((u8 *)write_cursor + 0xE);
                    *(u16 *)((u8 *)write_cursor + 0x36) = *(u16 *)((u8 *)write_cursor + 0x16);

                    FIELD_MESH_ADD_PRIM(ordering_table, effect, depth_offsets, write_cursor, write_cursor += 8);
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
        /* Textured Gouraud triangles: one 0x28-byte POLY_GT3 per face, colors clamped to 0..0xFF. */
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
                    CLAMP_COLOR_CHANNEL(packet_bytes[4], face_data[7] + base_color.r - 0x80);
                    CLAMP_COLOR_CHANNEL(packet_bytes[5], face_data[8] + base_color.g - 0x80);
                    CLAMP_COLOR_CHANNEL(packet_bytes[6], face_data[9] + base_color.b - 0x80);
                    CLAMP_COLOR_CHANNEL(packet_bytes[16], face_data[10] + base_color.r - 0x80);
                    CLAMP_COLOR_CHANNEL(packet_bytes[17], face_data[11] + base_color.g - 0x80);
                    CLAMP_COLOR_CHANNEL(packet_bytes[18], face_data[12] + base_color.b - 0x80);
                    CLAMP_COLOR_CHANNEL(packet_bytes[28], face_data[13] + base_color.r - 0x80);
                    CLAMP_COLOR_CHANNEL(packet_bytes[29], face_data[14] + base_color.g - 0x80);
                    {
                        s32 blue_channel;
                        CLAMP_COLOR_CHANNEL(blue_channel, face_data[15] + base_color.b - 0x80);
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
                    FIELD_MESH_SET_CLUT_TPAGE(actor, part, *(u16 *)(packet_bytes + 14), *(s16 *)(packet_bytes + 26));
                    FIELD_MESH_ADD_PRIM(ordering_table, effect, depth_offsets, packet_bytes, packet_bytes += 0x28);
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
        /* Lit flat triangles: a POLY_F3 (5 words) plus a draw-mode DR_TPAGE (2 words) per face. */
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
                    *(u16 *)((u8 *)packet + 8) += (u16)screen_origin[0];
                    *(u16 *)((u8 *)packet + 10) += (u16)screen_origin[1];
                    *(u16 *)((u8 *)packet + 12) += (u16)screen_origin[0];
                    *(u16 *)((u8 *)packet + 14) += (u16)screen_origin[1];
                    *(u16 *)((u8 *)packet + 16) += (u16)screen_origin[0];
                    *(u16 *)((u8 *)packet + 18) += (u16)screen_origin[1];
                    setlen(packet, 4);
                    setcode(packet, primitive_code);
                    if (effect->flags & FIELD_EFFECT_SEMITRANSPARENT)
                    {
                        setcode(packet, 0x22);
                    }
                    else
                    {
                        setcode(packet, primitive_code);
                    }

                    FIELD_MESH_ADD_PRIM(ordering_table, effect, depth_offsets, packet, packet += 5);

                    setlen(packet, 1);
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
