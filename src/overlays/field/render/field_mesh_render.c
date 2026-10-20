/** @file field_mesh_render.c
 * @brief Animate actor CLUT rows and emit textured and lit mesh primitives.
 *
 * A mesh's first face selects one primitive kind for all its faces: textured
 * flat (POLY_FT3), untextured flat (POLY_F3 plus a DR_TPAGE for its blend
 * mode) or textured Gouraud (POLY_GT3). Back faces are culled with the GTE
 * normal clip; each face goes into the ordering-table slot of the effect depth
 * plus its own depth offset.
 */

#include "common.h"
#include "main/display.h"
#include "main/field_runtime.h"
#include "../internal/field_effect_transform.h"
#include "../internal/field_effect_render_state.h"
#include "../internal/field_actor_palette.h"
#include "../internal/field_effect_types.h"
#include "../internal/field_mesh_render.h"
#include "../internal/field_mesh.h"
#include "../internal/field_mesh_transform.h"
#include <libgte.h>
#include <libgpu.h>
#include <libetc.h>
#include "common/gpu_packet.h"
#include <inline_c.h>
#include "sdk/gte_dmpsx_compat.h"

extern FieldMotionRecord g_field_effect_records[];
/* FieldActorSlot view of the slots field_actor_tables.h declares as FieldActorSlot. */

/** @brief Frame period bits of an animation's palette_animation word. */
#define FIELD_PALETTE_ANIMATION_PERIOD_MASK 0xF
/** @brief Mode bits (after >> 4) of an animation's palette_animation word. */
#define FIELD_PALETTE_ANIMATION_MODE_MASK 7
/** @brief Palette animation modes; the others do nothing. */
#define FIELD_PALETTE_ANIMATION_NONE 0
#define FIELD_PALETTE_ANIMATION_CYCLE_SUBPALETTE 2
#define FIELD_PALETTE_ANIMATION_CYCLE_CLUT 3
/** @brief Bit 7 of palette_animation: cycle the colours right instead of left. */
#define FIELD_PALETTE_ANIMATION_RIGHT(word) (((word) >> 7) & 1)
/** @brief Byte offset, and VRAM x, of the sub-palette in bits 8-11 of palette_animation. */
#define FIELD_PALETTE_ANIMATION_SUBPALETTE_OFFSET(word) (((word) >> 3) & 0x1E0)
#define FIELD_PALETTE_ANIMATION_SUBPALETTE_X(word) (((word) >> 4) & 0xF0)

/** @brief Store three packed screen vertices (x in the low half, y in the high half) in a triangle primitive. */
#define FIELD_MESH_SET_XY3(poly, xy) (*(s32*)&(poly)->x0 = (xy)[0], *(s32*)&(poly)->x1 = (xy)[1], *(s32*)&(poly)->x2 = (xy)[2])

/** @brief Move a triangle primitive's three vertices by a screen origin. */
#define FIELD_MESH_OFFSET_XY3(poly, origin)                                                                                                                    \
    ((poly)->x0 += (origin)->vx, (poly)->y0 += (origin)->vy, (poly)->x1 += (origin)->vx, (poly)->y1 += (origin)->vy, (poly)->x2 += (origin)->vx,               \
     (poly)->y2 += (origin)->vy)

/** @brief Store three packed texture coordinates (u in the low byte, v in the high byte) in a triangle primitive. */
#define FIELD_MESH_SET_UV3(poly, uv) (*(u16*)&(poly)->u0 = (uv)[0], *(u16*)&(poly)->u1 = (uv)[1], *(u16*)&(poly)->u2 = (uv)[2])

/** @brief Primitive kinds of a mesh (FIELD_MESH_FACE_KIND of its first face). */
#define FIELD_MESH_KIND_TEXTURED 0
#define FIELD_MESH_KIND_FLAT 1
#define FIELD_MESH_KIND_GOURAUD 2
/** @brief GPU codes of the mesh primitives and the semi-transparency bit. */
#define FIELD_MESH_F3_CODE 0x20
#define FIELD_MESH_FT3_CODE 0x24
#define FIELD_MESH_GT3_CODE 0x34
#define FIELD_MESH_SEMI_TRANSPARENT 0x02
/** @brief Texture mode (0 4-bit, 1 8-bit) of a part's mesh texture; its blend mode is FIELD_PART_BLEND_MODE. */
#define FIELD_PART_TEXTURE_MODE(part) (((part)->spawn_flags.word >> 22) & 1)
/** @brief Last ordering-table slot; deeper faces are clamped to it. */

/** @brief Light sources of a lit mesh part, and the selector value for an unused one (8 and up). */
#define FIELD_PART_LIGHT_COUNT 3
#define FIELD_PART_LIGHT_NONE 8

/**
 * @brief Part index of light source @p i (0..2) of an actor part.
 * @note The three selector bytes start at rotation_extent's last byte and run into effect_flags.
 */
#define FIELD_PART_LIGHT_SOURCE(part, i) ((&(part)->rotation_extent.fields.linked_part)[i])

/**
 * @brief CLUT buffer of actor palette owner @p owner (0 or 1).
 * @note g_field_actor_clut_buffers is declared as a flat byte array; the
 *       original indexes it as a [2][0x400] array.
 */
#define FIELD_ACTOR_CLUT_BUFFER(owner) (((u8(*)[FIELD_ACTOR_CLUT_BUFFER_SIZE])g_field_actor_clut_buffers)[owner])

static void field_rotate_palette_row(u16* row, s32 count, s32 rotate_right);

/**
 * @brief Write the CLUT and texture-page halfwords of a textured mesh packet.
 * @param actor Actor owning the mesh; owners 0 and 1 have their own CLUT rows and texture pages.
 * @param part Part supplying the palette, texture mode and blend mode.
 * @param clut Lvalue receiving the CLUT.
 * @param tpage Lvalue receiving the texture page.
 * @note Track flag bit 21 moves the CLUT to the start of the next VRAM row.
 */
#define FIELD_MESH_SET_CLUT_TPAGE(actor, part, clut, tpage)                                                                                                    \
    {                                                                                                                                                          \
        if ((actor)->owner_object_index < FIELD_ACTOR_PALETTE_OWNERS)                                                                                          \
        {                                                                                                                                                      \
            s32 owner_index;                                                                                                                                   \
            s32 palette_index;                                                                                                                                 \
            s16 page;                                                                                                                                          \
            owner_index = (actor)->owner_object_index;                                                                                                         \
            palette_index = (part)->appearance.fields.palette_selector;                                                                                        \
            (clut) = getClut(palette_index * 16, FIELD_ACTOR_CLUT_VRAM_Y + owner_index * 2);                                                                   \
            page = getTPage(FIELD_PART_TEXTURE_MODE(part), FIELD_PART_BLEND_MODE(part),                                                                        \
                            FIELD_ACTOR_TEXTURE_VRAM_X + (actor)->owner_object_index * FIELD_ACTOR_TEXTURE_VRAM_WIDTH, FIELD_ACTOR_TEXTURE_VRAM_Y);            \
            (tpage) = page;                                                                                                                                    \
        }                                                                                                                                                      \
        else                                                                                                                                                   \
        {                                                                                                                                                      \
            (clut) = getClut((part)->appearance.fields.palette_selector * 16, VRAM_CLUT_Y);                                                                    \
            (tpage) = getTPage(FIELD_PART_TEXTURE_MODE(part), FIELD_PART_BLEND_MODE(part), FIELD_SHARED_TEXTURE_VRAM_X, FIELD_SHARED_TEXTURE_VRAM_Y);          \
        }                                                                                                                                                      \
        if (((part)->track_flags.word >> 21) & 1)                                                                                                              \
        {                                                                                                                                                      \
            (clut) = ((clut) & 0xFFC0) + getClut(0, 1);                                                                                                        \
        }                                                                                                                                                      \
    }

/**
 * @brief Advance an actor's palette animation and upload the changed CLUT row to VRAM.
 * @param actor Actor whose current animation carries the palette animation word.
 * @return Nothing meaningful; callers ignore it.
 */
s32 field_update_actor_palette_animation(FieldActorSlot* actor)
{
    RECT rect;
    u8* buf;
    u16 animation;

    animation = actor->animation->palette_animation;

    if (!(animation & FIELD_PALETTE_ANIMATION_PERIOD_MASK))
    {
        return;
    }

    if (((u32)actor->track_frames[0] % (u32)((u8)actor->animation->palette_animation & FIELD_PALETTE_ANIMATION_PERIOD_MASK)) != 0)
    {
        return;
    }

    switch ((animation >> 4) & FIELD_PALETTE_ANIMATION_MODE_MASK)
    {
    case FIELD_PALETTE_ANIMATION_NONE:
    case 1:
    case 4:
    case 5:
        return;

    case FIELD_PALETTE_ANIMATION_CYCLE_SUBPALETTE:
        /* Cycle colours 1-15 of one 16-colour sub-palette. */
        if (actor->owner_object_index < FIELD_ACTOR_PALETTE_OWNERS)
        {
            u16 animation_word;
            s32 owner = actor->owner_object_index;

            animation_word = actor->animation->palette_animation;
            buf = &FIELD_ACTOR_CLUT_BUFFER(owner)[FIELD_PALETTE_ANIMATION_SUBPALETTE_OFFSET(animation_word)];
            field_rotate_palette_row((u16*)buf + 1, GPU_CLUT_4BIT_COLORS - 1, FIELD_PALETTE_ANIMATION_RIGHT(animation_word));
            rect.x = FIELD_PALETTE_ANIMATION_SUBPALETTE_X(actor->animation->palette_animation);
            rect.y = (actor->owner_object_index * 2) + FIELD_ACTOR_CLUT_VRAM_Y;
            rect.w = GPU_CLUT_4BIT_COLORS;
            rect.h = 1;
        }
        else
        {
            u16 animation_word = actor->animation->palette_animation;

            buf = &g_field_shared_clut_buffer[FIELD_PALETTE_ANIMATION_SUBPALETTE_OFFSET(animation_word)];
            field_rotate_palette_row((u16*)buf + 1, GPU_CLUT_4BIT_COLORS - 1, FIELD_PALETTE_ANIMATION_RIGHT(animation_word));
            rect.x = FIELD_PALETTE_ANIMATION_SUBPALETTE_X(actor->animation->palette_animation);
            rect.y = VRAM_CLUT_Y;
            rect.w = GPU_CLUT_4BIT_COLORS;
            rect.h = 1;
        }

        break;

    case FIELD_PALETTE_ANIMATION_CYCLE_CLUT:
        /* Cycle colours 1-255 of the whole CLUT row. */
        if (actor->owner_object_index < FIELD_ACTOR_PALETTE_OWNERS)
        {
            buf = FIELD_ACTOR_CLUT_BUFFER(actor->owner_object_index);
            field_rotate_palette_row((u16*)buf + 1, GPU_CLUT_8BIT_COLORS - 1, FIELD_PALETTE_ANIMATION_RIGHT(actor->animation->palette_animation));
            rect.x = 0;
            rect.y = (actor->owner_object_index * 2) + FIELD_ACTOR_CLUT_VRAM_Y;
            rect.w = GPU_CLUT_8BIT_COLORS;
            rect.h = 1;
        }
        else
        {
            buf = g_field_shared_clut_buffer;
            field_rotate_palette_row((u16*)buf + 1, GPU_CLUT_8BIT_COLORS - 1, FIELD_PALETTE_ANIMATION_RIGHT(actor->animation->palette_animation));
            rect.y = VRAM_CLUT_Y;
            rect.w = GPU_CLUT_8BIT_COLORS;
            rect.x = 0;
            rect.h = 1;
        }

        break;

    default:
        return;
    }

    LoadImage(&rect, (u_long*)buf);
}

/**
 * @brief Rotate a row of palette entries by one element.
 * @param row Palette row to rotate.
 * @param count Number of cells in the row.
 * @param rotate_right Nonzero to rotate right; zero to rotate left.
 */
static void field_rotate_palette_row(u16* row, s32 count, s32 rotate_right)
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
 * @brief Tint a face colour channel by the part colour and clamp it to 0..255.
 * @param color Face colour channel.
 * @param tint Part colour channel; FIELD_TINT_NEUTRAL leaves @p color unchanged.
 * @return Tinted channel, limited to 0..255.
 */
static inline s32 tint_color_channel(s32 color, s32 tint)
{
    s32 value = color + tint - FIELD_TINT_NEUTRAL;
    s32 clamped;

    if (value >= 0)
    {
        clamped = 255;

        if (value < 256)
        {
            clamped = value;
        }
    }
    else
    {
        clamped = 0;
    }

    return clamped;
}

/**
 * @brief Link a mesh primitive into the ordering-table slot of the current face.
 * @param ot Ordering table base.
 * @param effect Effect record whose z (>> 7) is the base depth.
 * @param depth_offsets Cursor into the per-face depth offsets.
 * @param prim Primitive to link.
 * @param size Size of @p prim in bytes.
 * @return The packet address after @p prim.
 * @note Depths below 0 go to slot 0 and depths past FIELD_WORLD_OT_LAST to the last slot.
 */
static inline void* add_mesh_prim(s32* ot, FieldMotionRecord* effect, s32* depth_offsets, void* prim, s32 size)
{
    s32 depth_offset = *depth_offsets;
    s32 base_depth = effect->z >> 7;
    s32 depth_index = base_depth + depth_offset;

    if (depth_index < 0)
    {
        addPrim(ot, prim);
        return (u8*)prim + size;
    }
    else if (depth_index > FIELD_WORLD_OT_LAST)
    {
        addPrim(&ot[FIELD_WORLD_OT_LAST], prim);
        return (u8*)prim + size;
    }
    else
    {
        addPrim(ot + (effect->z >> 7) + *depth_offsets, prim);
        return (u8*)prim + size;
    }
}

/**
 * @brief Link a DR_TPAGE packet into the ordering table at the same depth as the face before it.
 * @param ot Ordering table base.
 * @param effect Effect record whose z (>> 7) is the base depth.
 * @param depth_offsets Cursor into the per-face depth offsets.
 * @param packet DR_TPAGE packet to link.
 * @return The packet address after the DR_TPAGE.
 */
static inline u_long* add_mesh_tpage(s32* ot, FieldMotionRecord* effect, s32* depth_offsets, u_long* packet)
{
    s32 depth_offset = *depth_offsets;
    s32 base_depth = effect->z >> 7;
    s32 depth_index = base_depth + depth_offset;
    u_long* next_packet;

    if (depth_index < 0)
    {
        next_packet = (u_long*)((DR_TPAGE*)packet + 1);
        addPrim(ot, packet);
    }
    else if (depth_index > FIELD_WORLD_OT_LAST)
    {
        next_packet = (u_long*)((DR_TPAGE*)packet + 1);
        addPrim(&ot[FIELD_WORLD_OT_LAST], packet);
    }
    else
    {
        next_packet = (u_long*)((DR_TPAGE*)packet + 1);
        addPrim(ot + (effect->z >> 7) + *depth_offsets, packet);
    }

    return next_packet;
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
s32* field_render_effect_mesh(FieldMotionRecord* effect, s32 mesh_index, s32* packet_cursor, s32* ordering_table)
{
    s32 unused[2];
    MATRIX transform;
    MATRIX base_matrix;
    CVECTOR base_color;
    s32 triangle_area;
    FieldActorSlot* actor;
    FieldObjectPart* part;
    FieldMeshFace* face;
    s32* screen_vertices;
    s32* depth_offsets;
    DVECTOR* screen_origin;
    s32 primitive_kind;
    s32* ordering_table_base;
    s32 face_count;

    screen_origin = (DVECTOR*)getScratchAddr(0);
    ordering_table_base = ordering_table;
    part = &g_field_actor_slots[effect->actor_index].parts[effect->part_index];
    actor = &g_field_actor_slots[effect->actor_index];

    field_build_part_matrix(actor, effect, part, &transform, &base_matrix);
    transform.t[2] = 0;
    transform.t[1] = 0;
    transform.t[0] = 0;
    field_resolve_effect_part_color(actor, effect, part, (FieldPrimitiveColor*)&base_color);
    gte_SetRotMatrix(&transform);
    gte_SetTransMatrix(&transform);
    field_transform_mesh_vertices(actor, effect, part, mesh_index);

    screen_vertices = (s32*)g_field_mesh_screen_vertices;
    depth_offsets = g_field_mesh_depth_offsets;
    face = FIELD_ACTOR_MESH(actor, mesh_index)->faces;
    screen_origin->vx = FIELD_SCREEN_X(effect->x);
    screen_origin->vy = FIELD_SCREEN_Y(effect->y, effect->z);
    primitive_kind = FIELD_MESH_FACE_KIND(face);

    switch (primitive_kind)
    {
    case FIELD_MESH_KIND_TEXTURED:
    {
        s32 code;

        face_count = FIELD_ACTOR_MESH(actor, mesh_index)->face_count;

        if (face_count != 0)
        {
            code = FIELD_MESH_FT3_CODE;

            for (; face_count != 0; face++, face_count--, screen_vertices += 3, depth_offsets++)
            {
                gte_ldsxy3(screen_vertices[0], screen_vertices[1], screen_vertices[2]);
                gte_nclip();
                gte_stopz(&triangle_area);

                if (triangle_area > 0)
                {
                    u32 color = *(u32*)&base_color;

                    setlen((POLY_FT3*)packet_cursor, 7);
                    SET_BGR0_PACKED((POLY_FT3*)packet_cursor, color);
                    setcode((POLY_FT3*)packet_cursor, code);

                    if (effect->flags & FIELD_EFFECT_SEMITRANSPARENT)
                    {
                        setcode((POLY_FT3*)packet_cursor, FIELD_MESH_FT3_CODE | FIELD_MESH_SEMI_TRANSPARENT);
                    }

                    FIELD_MESH_SET_XY3((POLY_FT3*)packet_cursor, screen_vertices);
                    FIELD_MESH_OFFSET_XY3((POLY_FT3*)packet_cursor, screen_origin);
                    FIELD_MESH_SET_UV3((POLY_FT3*)packet_cursor, face->texture.uv);
                    FIELD_MESH_SET_CLUT_TPAGE(actor, part, ((POLY_FT3*)packet_cursor)->clut, ((POLY_FT3*)packet_cursor)->tpage);
                    packet_cursor = add_mesh_prim(ordering_table_base, effect, depth_offsets, packet_cursor, sizeof(POLY_FT3));
                }
            }
        }

        return packet_cursor;
    }
    case FIELD_MESH_KIND_FLAT:
    {
        /* A POLY_F3 and then a DR_TPAGE selecting its blend mode. */
        u_long* packet = (u_long*)packet_cursor;
        s32 code;

        face_count = FIELD_ACTOR_MESH(actor, mesh_index)->face_count;

        if (face_count != 0)
        {
            code = FIELD_MESH_F3_CODE;

            while (face_count != 0)
            {
                gte_ldsxy3(screen_vertices[0], screen_vertices[1], screen_vertices[2]);
                gte_nclip();
                gte_stopz(&triangle_area);

                if (triangle_area > 0)
                {
                    POLY_F3* poly = (POLY_F3*)packet;

                    poly->r0 = face->texture.color[0] + base_color.r - FIELD_TINT_NEUTRAL;
                    poly->g0 = face->texture.color[1] + base_color.g - FIELD_TINT_NEUTRAL;
                    poly->b0 = face->texture.color[2] + base_color.b - FIELD_TINT_NEUTRAL;
                    FIELD_MESH_SET_XY3(poly, screen_vertices);
                    FIELD_MESH_OFFSET_XY3(poly, screen_origin);
                    setlen(poly, 4);
                    setcode(poly, code);

                    if (effect->flags & FIELD_EFFECT_SEMITRANSPARENT)
                    {
                        setcode(poly, FIELD_MESH_F3_CODE | FIELD_MESH_SEMI_TRANSPARENT);
                    }

                    packet = add_mesh_prim(ordering_table_base, effect, depth_offsets, packet, sizeof(POLY_F3));

                    setDrawTPage((DR_TPAGE*)packet, 0, 0, getTPage(FIELD_PART_TEXTURE_MODE(part), FIELD_PART_BLEND_MODE(part), 0, 0));
                    packet = add_mesh_tpage(ordering_table_base, effect, depth_offsets, packet);
                }

                face++;
                face_count--;
                screen_vertices += 3;
                depth_offsets++;
            }
        }

        packet_cursor = (s32*)packet;
        break;
    }
    case FIELD_MESH_KIND_GOURAUD:
    {
        POLY_GT3* poly;

        face_count = FIELD_ACTOR_MESH(actor, mesh_index)->face_count;
        poly = (POLY_GT3*)packet_cursor;

        for (; face_count != 0; face++, face_count--, screen_vertices += 3, depth_offsets++)
        {
            gte_ldsxy3(screen_vertices[0], screen_vertices[1], screen_vertices[2]);
            gte_nclip();
            gte_stopz(&triangle_area);

            if (triangle_area > 0)
            {
                s32 blue;

                poly->r0 = tint_color_channel(face->vertex_colors[0][0], base_color.r);
                poly->g0 = tint_color_channel(face->vertex_colors[0][1], base_color.g);
                poly->b0 = tint_color_channel(face->vertex_colors[0][2], base_color.b);
                poly->r1 = tint_color_channel(face->vertex_colors[1][0], base_color.r);
                poly->g1 = tint_color_channel(face->vertex_colors[1][1], base_color.g);
                poly->b1 = tint_color_channel(face->vertex_colors[1][2], base_color.b);
                poly->r2 = tint_color_channel(face->vertex_colors[2][0], base_color.r);
                poly->g2 = tint_color_channel(face->vertex_colors[2][1], base_color.g);
                blue = tint_color_channel(face->vertex_colors[2][2], base_color.b);
                setlen(poly, 9);
                poly->b2 = blue;
                setcode(poly, FIELD_MESH_GT3_CODE);
                setSemiTrans(poly, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);
                FIELD_MESH_SET_XY3(poly, screen_vertices);
                FIELD_MESH_OFFSET_XY3(poly, screen_origin);
                FIELD_MESH_SET_UV3(poly, face->texture.uv);
                FIELD_MESH_SET_CLUT_TPAGE(actor, part, poly->clut, poly->tpage);
                poly = add_mesh_prim(ordering_table_base, effect, depth_offsets, poly, sizeof(POLY_GT3));
            }
        }

        packet_cursor = (s32*)poly;
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
 * @param buffer Current primitive-buffer write cursor.
 * @param ordering_table Ordering table receiving the generated primitives.
 * @return Advanced primitive-buffer cursor.
 * @see decomp.me (100%) https://decomp.me/scratch/F2Z4z
 */
s32* field_render_lit_effect_mesh(FieldMotionRecord* effect, s32 mesh_index, s32* buffer, s32* ordering_table)
{
    s32 unused[2];
    s32* write_cursor;
    MATRIX transform;
    MATRIX base_matrix;
    CVECTOR base_color;
    MATRIX light_matrix;
    MATRIX color_matrix;
    SVECTOR light_direction;
    SVECTOR transformed_light;
    s32 triangle_area;
    FieldActorSlot* actor;
    FieldObjectPart* part;
    FieldMotionRecord* light_effect;
    s32 light_index;
    s32* screen_vertices;
    SVECTOR* transformed_normals;
    s32* depth_offsets;
    DVECTOR* screen_origin;
    FieldMeshFace* face;
    s32 count;
    s32 primitive_kind;

    write_cursor = buffer;
    part = &g_field_actor_slots[effect->actor_index].parts[effect->part_index];
    actor = &g_field_actor_slots[effect->actor_index];

    field_build_part_matrix(actor, effect, part, &transform, &base_matrix);
    transform.t[2] = 0;
    transform.t[1] = 0;
    transform.t[0] = 0;
    field_resolve_effect_part_color(actor, effect, part, (FieldPrimitiveColor*)&base_color);
    screen_origin = (DVECTOR*)getScratchAddr(0);
    gte_SetRotMatrix(&transform);
    gte_SetTransMatrix(&transform);
    field_transform_mesh_vertices(actor, effect, part, mesh_index);
    field_transform_mesh_normals(actor, effect, part, mesh_index, &base_matrix);

    for (light_index = 0; light_index < FIELD_PART_LIGHT_COUNT; light_index++)
    {
        if (FIELD_PART_LIGHT_SOURCE(part, light_index) < FIELD_PART_LIGHT_NONE)
        {
            for (count = 0; count < FIELD_EFFECT_ACTIVE_RECORD_COUNT; count++)
            {
                light_effect = &g_field_effect_records[count];

                if (FIELD_PART_LIGHT_SOURCE(part, light_index) == light_effect->part_index && effect->actor_index == light_effect->actor_index)
                {
                    RotMatrix_gte((SVECTOR*)&g_field_effect_records[count].rotation_x, &base_matrix);
                    RotMatrixZ(effect->rotation_z_16 * 16, &base_matrix);
                    RotMatrixY(effect->rotation_y_16 * 16, &base_matrix);
                    light_direction.vz = 0;
                    light_direction.vx = 0;
                    light_direction.vy = -ONE;
                    gte_SetRotMatrix(&base_matrix);
                    gte_ldv0(&light_direction);
                    gte_rtv0();
                    gte_stsv(&transformed_light);
                    light_matrix.m[light_index][0] = transformed_light.vx;
                    light_matrix.m[light_index][1] = transformed_light.vy;
                    light_matrix.m[light_index][2] = transformed_light.vz;
                    color_matrix.m[0][light_index] = g_field_actor_slots[effect->actor_index].parts[light_effect->part_index].tint_red * 16;
                    color_matrix.m[1][light_index] = g_field_actor_slots[effect->actor_index].parts[light_effect->part_index].tint_green * 16;
                    color_matrix.m[2][light_index] = g_field_actor_slots[effect->actor_index].parts[light_effect->part_index].tint_blue * 16;
                    break;
                }
            }

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
    }

    gte_SetLightMatrix(&light_matrix);
    gte_SetColorMatrix(&color_matrix);
    screen_vertices = (s32*)g_field_mesh_screen_vertices;
    transformed_normals = g_field_mesh_transformed_normals;
    depth_offsets = g_field_mesh_depth_offsets;
    face = FIELD_ACTOR_MESH(actor, mesh_index)->faces;
    screen_origin->vx = FIELD_SCREEN_X(effect->x);
    screen_origin->vy = FIELD_SCREEN_Y(effect->y, effect->z);
    primitive_kind = FIELD_MESH_FACE_KIND(face);

    switch (primitive_kind)
    {
    case FIELD_MESH_KIND_TEXTURED:
    {
        /* The CLUT and texture page are written into the first packet and carried forward. */
        s32 code;

        gte_SetBackColor(base_color.r, base_color.g, base_color.b);
        FIELD_MESH_SET_CLUT_TPAGE(actor, part, ((POLY_FT3*)write_cursor)->clut, ((POLY_FT3*)write_cursor)->tpage);

        count = FIELD_ACTOR_MESH(actor, mesh_index)->face_count;

        if (count != 0)
        {
            code = FIELD_MESH_FT3_CODE;

            for (; count != 0; face++, count--, screen_vertices += 3, transformed_normals++, depth_offsets++)
            {
                gte_ldsxy3(screen_vertices[0], screen_vertices[1], screen_vertices[2]);
                gte_nclip();
                gte_stopz(&triangle_area);

                if (triangle_area > 0)
                {
                    gte_ldv0(transformed_normals);
                    gte_ncs();
                    gte_strgb(&((POLY_FT3*)write_cursor)->r0);
                    FIELD_MESH_SET_XY3((POLY_FT3*)write_cursor, screen_vertices);
                    FIELD_MESH_OFFSET_XY3((POLY_FT3*)write_cursor, screen_origin);
                    setlen((POLY_FT3*)write_cursor, 7);
                    setcode((POLY_FT3*)write_cursor, code);

                    if (effect->flags & FIELD_EFFECT_SEMITRANSPARENT)
                    {
                        setcode((POLY_FT3*)write_cursor, FIELD_MESH_FT3_CODE | FIELD_MESH_SEMI_TRANSPARENT);
                    }
                    else
                    {
                        setcode((POLY_FT3*)write_cursor, code);
                    }

                    FIELD_MESH_SET_UV3((POLY_FT3*)write_cursor, face->texture.uv);
                    ((POLY_FT3*)write_cursor)[1].clut = ((POLY_FT3*)write_cursor)->clut;
                    ((POLY_FT3*)write_cursor)[1].tpage = ((POLY_FT3*)write_cursor)->tpage;

                    write_cursor = add_mesh_prim(ordering_table, effect, depth_offsets, write_cursor, sizeof(POLY_FT3));
                }
            }
        }

        return write_cursor;
    }
    case FIELD_MESH_KIND_GOURAUD:
    {
        POLY_GT3* poly;

        count = FIELD_ACTOR_MESH(actor, mesh_index)->face_count;
        poly = (POLY_GT3*)write_cursor;

        for (; count != 0; face++, count--, screen_vertices += 3, depth_offsets++)
        {
            gte_ldsxy3(screen_vertices[0], screen_vertices[1], screen_vertices[2]);
            gte_nclip();
            gte_stopz(&triangle_area);

            if (triangle_area > 0)
            {
                s32 blue;

                poly->r0 = tint_color_channel(face->vertex_colors[0][0], base_color.r);
                poly->g0 = tint_color_channel(face->vertex_colors[0][1], base_color.g);
                poly->b0 = tint_color_channel(face->vertex_colors[0][2], base_color.b);
                poly->r1 = tint_color_channel(face->vertex_colors[1][0], base_color.r);
                poly->g1 = tint_color_channel(face->vertex_colors[1][1], base_color.g);
                poly->b1 = tint_color_channel(face->vertex_colors[1][2], base_color.b);
                poly->r2 = tint_color_channel(face->vertex_colors[2][0], base_color.r);
                poly->g2 = tint_color_channel(face->vertex_colors[2][1], base_color.g);
                blue = tint_color_channel(face->vertex_colors[2][2], base_color.b);
                setlen(poly, 9);
                poly->b2 = blue;
                setcode(poly, FIELD_MESH_GT3_CODE);
                setSemiTrans(poly, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);
                FIELD_MESH_SET_XY3(poly, screen_vertices);
                FIELD_MESH_OFFSET_XY3(poly, screen_origin);
                FIELD_MESH_SET_UV3(poly, face->texture.uv);
                FIELD_MESH_SET_CLUT_TPAGE(actor, part, poly->clut, poly->tpage);
                poly = add_mesh_prim(ordering_table, effect, depth_offsets, poly, sizeof(POLY_GT3));
            }
        }

        write_cursor = (s32*)poly;
        break;
    }
    case FIELD_MESH_KIND_FLAT:
    {
        /* A lit POLY_F3 and then a DR_TPAGE selecting its blend mode. */
        u_long* packet = (u_long*)write_cursor;
        s32 code;

        count = FIELD_ACTOR_MESH(actor, mesh_index)->face_count;

        if (count != 0)
        {
            code = FIELD_MESH_F3_CODE;

            while (count != 0)
            {
                gte_ldsxy3(screen_vertices[0], screen_vertices[1], screen_vertices[2]);
                gte_nclip();
                gte_stopz(&triangle_area);

                if (triangle_area > 0)
                {
                    POLY_F3* poly = (POLY_F3*)packet;

                    gte_SetBackColor(face->texture.color[0] + base_color.r - FIELD_TINT_NEUTRAL, face->texture.color[1] + base_color.g - FIELD_TINT_NEUTRAL,
                                     face->texture.color[2] + base_color.b - FIELD_TINT_NEUTRAL);
                    gte_ldv0(transformed_normals);
                    gte_ncs();
                    gte_strgb(&poly->r0);
                    FIELD_MESH_SET_XY3(poly, screen_vertices);
                    FIELD_MESH_OFFSET_XY3(poly, screen_origin);
                    setlen(poly, 4);
                    setcode(poly, code);

                    if (effect->flags & FIELD_EFFECT_SEMITRANSPARENT)
                    {
                        setcode(poly, FIELD_MESH_F3_CODE | FIELD_MESH_SEMI_TRANSPARENT);
                    }
                    else
                    {
                        setcode(poly, code);
                    }

                    packet = add_mesh_prim(ordering_table, effect, depth_offsets, packet, sizeof(POLY_F3));

                    setDrawTPage((DR_TPAGE*)packet, 0, 0, getTPage(FIELD_PART_TEXTURE_MODE(part), FIELD_PART_BLEND_MODE(part), 0, 0));
                    packet = add_mesh_tpage(ordering_table, effect, depth_offsets, packet);
                }

                face++;
                count--;
                screen_vertices += 3;
                transformed_normals++;
                depth_offsets++;
            }
        }

        write_cursor = (s32*)packet;
        break;
    }
    default:
        return write_cursor;
    }

    return write_cursor;
}
