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
#include "display.h"
#include "field_effect_transform.h"
#include "field_effect_render_state.h"
#include "field_actor_palette.h"
#include "field_effect_types.h"
#include "field_mesh_render.h"
#include "field_mesh.h"
#include "field_mesh_transform.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "sdk/libetc.h"
#include "gpu_packet.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

extern FieldMotionRecord g_field_effect_records[];
/* FieldActorState view of the slots field_actor_tables.h declares as FieldActorSlot. */
extern FieldActorState g_field_actor_slots[80];

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
/** @brief Colours of a CLUT row and of one of its sub-palettes. */
#define FIELD_CLUT_COLORS 256
#define FIELD_CLUT_SUBPALETTE_COLORS 16

/** @brief Screen-space centre added to every projected mesh vertex. */
#define FIELD_MESH_SCREEN_CENTER_X (SCREEN_WIDTH / 2)
#define FIELD_MESH_SCREEN_CENTER_Y (VRAM_DRAW_HEIGHT / 2)
/** @brief Colour channel value that leaves a tinted colour unchanged. */
#define FIELD_COLOR_NEUTRAL 0x80
/** @brief Primitive kinds of a mesh (FIELD_MESH_FACE_KIND of its first face). */
#define FIELD_MESH_KIND_TEXTURED 0
#define FIELD_MESH_KIND_FLAT 1
#define FIELD_MESH_KIND_GOURAUD 2
/** @brief GPU codes of the mesh primitives and the semi-transparency bit. */
#define FIELD_MESH_F3_CODE 0x20
#define FIELD_MESH_FT3_CODE 0x24
#define FIELD_MESH_GT3_CODE 0x34
#define FIELD_MESH_SEMI_TRANSPARENT 0x02
/** @brief Texture mode (0 4-bit, 1 8-bit) and blend mode of a part's mesh texture. */
#define FIELD_PART_TEXTURE_MODE(part) (((part)->spawn_flags.word >> 22) & 1)
#define FIELD_PART_BLEND_MODE(part) (((part)->behavior_flags.word >> 22) & 3)
/** @brief Last ordering-table slot; deeper faces are clamped to it. */
#define FIELD_MESH_OT_MAX_DEPTH 0xFFF

/** @brief Light sources of a lit mesh part, and the selector value for an unused one (8 and up). */
#define FIELD_PART_LIGHT_COUNT 3
#define FIELD_PART_LIGHT_NONE 8

/**
 * @brief Part index of light source @p i (0..2) of an actor part.
 * @note The three selector bytes start at rotation_extent's last byte and run into effect_flags.
 */
#define FIELD_PART_LIGHT_SOURCE(part, i) ((&(part)->rotation_extent.fields.unknown_0x23)[i])

/**
 * @brief CLUT buffer of actor palette owner @p owner (0 or 1).
 * @note g_field_actor_clut_buffers is declared as a flat byte array; the
 *       original indexes it as a [2][0x400] array.
 */
#define FIELD_ACTOR_CLUT_BUFFER(owner) (((u8(*)[FIELD_ACTOR_CLUT_BUFFER_SIZE])g_field_actor_clut_buffers)[owner])

static void field_rotate_palette_row(u16 *row, s32 count, s32 rotate_right);

/**
 * @brief Advance an actor's palette animation and upload the changed CLUT row to VRAM.
 * @param actor Actor whose current animation carries the palette animation word.
 * @return Nothing meaningful; callers ignore it.
 * @note Declared int with bare returns; as void it compiles differently (v0 is dead at the returns).
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
            field_rotate_palette_row((u16 *)buf + 1, FIELD_CLUT_SUBPALETTE_COLORS - 1, FIELD_PALETTE_ANIMATION_RIGHT(animation_word));
            rect.x = FIELD_PALETTE_ANIMATION_SUBPALETTE_X(actor->animation->palette_animation);
            rect.y = (actor->owner_object_index * 2) + FIELD_ACTOR_CLUT_VRAM_Y;
            rect.w = FIELD_CLUT_SUBPALETTE_COLORS;
            rect.h = 1;
        }
        else
        {
            u16 animation_word = actor->animation->palette_animation;

            buf = &g_field_shared_clut_buffer[FIELD_PALETTE_ANIMATION_SUBPALETTE_OFFSET(animation_word)];
            field_rotate_palette_row((u16 *)buf + 1, FIELD_CLUT_SUBPALETTE_COLORS - 1, FIELD_PALETTE_ANIMATION_RIGHT(animation_word));
            rect.x = FIELD_PALETTE_ANIMATION_SUBPALETTE_X(actor->animation->palette_animation);
            rect.y = FIELD_SHARED_CLUT_VRAM_Y;
            rect.w = FIELD_CLUT_SUBPALETTE_COLORS;
            rect.h = 1;
        }
        break;

    case FIELD_PALETTE_ANIMATION_CYCLE_CLUT:
        /* Cycle colours 1-255 of the whole CLUT row. */
        if (actor->owner_object_index < FIELD_ACTOR_PALETTE_OWNERS)
        {
            buf = FIELD_ACTOR_CLUT_BUFFER(actor->owner_object_index);
            field_rotate_palette_row((u16 *)buf + 1, FIELD_CLUT_COLORS - 1, FIELD_PALETTE_ANIMATION_RIGHT(actor->animation->palette_animation));
            rect.x = 0;
            rect.y = (actor->owner_object_index * 2) + FIELD_ACTOR_CLUT_VRAM_Y;
            rect.w = FIELD_CLUT_COLORS;
            rect.h = 1;
        }
        else
        {
            buf = g_field_shared_clut_buffer;
            field_rotate_palette_row((u16 *)buf + 1, FIELD_CLUT_COLORS - 1, FIELD_PALETTE_ANIMATION_RIGHT(actor->animation->palette_animation));
            rect.y = FIELD_SHARED_CLUT_VRAM_Y;
            rect.w = FIELD_CLUT_COLORS;
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
static void field_rotate_palette_row(u16 *row, s32 count, s32 rotate_right)
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
 * @param tint Part colour channel; FIELD_COLOR_NEUTRAL leaves @p color unchanged.
 * @return Tinted channel, limited to 0..255.
 */
static inline s32 tint_color_channel(s32 color, s32 tint)
{
    s32 value = color + tint - FIELD_COLOR_NEUTRAL;
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
 * @note Depths below 0 go to slot 0 and depths past FIELD_MESH_OT_MAX_DEPTH to the last slot.
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
    else if (depth_index > FIELD_MESH_OT_MAX_DEPTH)
    {
        addPrim(&ot[FIELD_MESH_OT_MAX_DEPTH], prim);
        return (u8*)prim + size;
    }
    else
    {
        addPrim(ot + (effect->z >> 7) + *depth_offsets, prim);
        return (u8*)prim + size;
    }
}

/**
 * @brief Write the CLUT and texture-page halfwords of a textured mesh packet.
 * @param actor Actor owning the mesh; owners 0 and 1 have their own CLUT rows and texture pages.
 * @param part Part supplying the palette, texture mode and blend mode.
 * @param clut Lvalue receiving the CLUT.
 * @param tpage Lvalue receiving the texture page.
 * @note Track flag bit 21 moves the CLUT to the start of the next VRAM row.
 */
#define FIELD_MESH_SET_CLUT_TPAGE(actor, part, clut, tpage) \
    { \
        if ((actor)->owner_object_index < FIELD_ACTOR_PALETTE_OWNERS) \
        { \
            s32 owner_index; \
            s32 palette_index; \
            s16 page; \
            owner_index = (actor)->owner_object_index; \
            palette_index = (part)->appearance.fields.palette_selector; \
            (clut) = getClut(palette_index * 16, FIELD_ACTOR_CLUT_VRAM_Y + owner_index * 2); \
            page = getTPage(FIELD_PART_TEXTURE_MODE(part), FIELD_PART_BLEND_MODE(part), \
                            FIELD_ACTOR_TEXTURE_VRAM_X + (actor)->owner_object_index * FIELD_ACTOR_TEXTURE_VRAM_WIDTH, FIELD_ACTOR_TEXTURE_VRAM_Y); \
            (tpage) = page; \
        } \
        else \
        { \
            (clut) = getClut((part)->appearance.fields.palette_selector * 16, FIELD_SHARED_CLUT_VRAM_Y); \
            (tpage) = getTPage(FIELD_PART_TEXTURE_MODE(part), FIELD_PART_BLEND_MODE(part), FIELD_SHARED_TEXTURE_VRAM_X, FIELD_SHARED_TEXTURE_VRAM_Y); \
        } \
        if (((part)->track_flags.word >> 21) & 1) \
        { \
            (clut) = ((clut) & 0xFFC0) + getClut(0, 1); \
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
    FieldMeshFace *face;
    s32 *screen_vertices;
    s32 *depth_offsets;
    DVECTOR *screen_origin;
    s32 primitive_kind;
    s32 *ordering_table_base;
    s32 face_count;

    screen_origin = (DVECTOR *)getScratchAddr(0);
    ordering_table_base = ordering_table;
    part = &g_field_actor_slots[effect->actor_index].parts[effect->part_index];
    actor = &g_field_actor_slots[effect->actor_index];

    field_build_part_matrix(actor, effect, part, &transform, &base_matrix);
    transform.t[2] = 0;
    transform.t[1] = 0;
    transform.t[0] = 0;
    field_resolve_effect_part_color(actor, effect, part, (FieldPrimitiveColor *)&base_color);
    gte_SetRotMatrix(&transform);
    gte_SetTransMatrix(&transform);
    field_transform_mesh_vertices(actor, effect, part, mesh_index);

    screen_vertices = (s32 *)g_field_mesh_screen_vertices;
    depth_offsets = g_field_mesh_depth_offsets;
    face = FIELD_ACTOR_MESH(actor, mesh_index)->faces;
    screen_origin->vx = FIELD_MESH_SCREEN_CENTER_X + g_field_view_offset_x / 256 + effect->x / 256;
    screen_origin->vy = FIELD_MESH_SCREEN_CENTER_Y + g_field_view_offset_y / 256 + effect->y / 256 - effect->z / 512 - g_field_view_offset_z / 512;
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
            do
            {
                gte_ldsxy3(screen_vertices[0], screen_vertices[1], screen_vertices[2]);
                gte_nclip();
                gte_stopz(&triangle_area);
                if (triangle_area > 0)
                {
                    u32 color = *(u32 *)&base_color;

                    setlen((POLY_FT3 *)packet_cursor, 7);
                    SET_BGR0_PACKED((POLY_FT3 *)packet_cursor, color);
                    setcode((POLY_FT3 *)packet_cursor, code);
                    if (effect->flags & FIELD_EFFECT_SEMITRANSPARENT)
                    {
                        setcode((POLY_FT3 *)packet_cursor, FIELD_MESH_FT3_CODE | FIELD_MESH_SEMI_TRANSPARENT);
                    }
                    *(s32 *)&((POLY_FT3 *)packet_cursor)->x0 = screen_vertices[0];
                    *(s32 *)&((POLY_FT3 *)packet_cursor)->x1 = screen_vertices[1];
                    *(s32 *)&((POLY_FT3 *)packet_cursor)->x2 = screen_vertices[2];
                    ((POLY_FT3 *)packet_cursor)->x0 += screen_origin->vx;
                    ((POLY_FT3 *)packet_cursor)->y0 += screen_origin->vy;
                    ((POLY_FT3 *)packet_cursor)->x1 += screen_origin->vx;
                    ((POLY_FT3 *)packet_cursor)->y1 += screen_origin->vy;
                    ((POLY_FT3 *)packet_cursor)->x2 += screen_origin->vx;
                    ((POLY_FT3 *)packet_cursor)->y2 += screen_origin->vy;
                    *(u16 *)&((POLY_FT3 *)packet_cursor)->u0 = face->texture.uv[0];
                    *(u16 *)&((POLY_FT3 *)packet_cursor)->u1 = face->texture.uv[1];
                    *(u16 *)&((POLY_FT3 *)packet_cursor)->u2 = face->texture.uv[2];
                    FIELD_MESH_SET_CLUT_TPAGE(actor, part, ((POLY_FT3 *)packet_cursor)->clut, ((POLY_FT3 *)packet_cursor)->tpage);
                    packet_cursor = add_mesh_prim(ordering_table_base, effect, depth_offsets, packet_cursor, sizeof(POLY_FT3));
                }
                face++;
                face_count--;
                screen_vertices += 3;
                depth_offsets++;
            } while (face_count != 0);
        }
        return packet_cursor;
    }
    case FIELD_MESH_KIND_FLAT:
    {
        /* A POLY_F3 and then a DR_TPAGE selecting its blend mode. */
        u_long *packet = (u_long *)packet_cursor;
        s32 code;

        face_count = FIELD_ACTOR_MESH(actor, mesh_index)->face_count;
        if (face_count != 0)
        {
            code = FIELD_MESH_F3_CODE;
            do
            {
                gte_ldsxy3(screen_vertices[0], screen_vertices[1], screen_vertices[2]);
                gte_nclip();
                gte_stopz(&triangle_area);
                if (triangle_area > 0)
                {
                    ((POLY_F3 *)packet)->r0 = face->texture.color[0] + base_color.r - FIELD_COLOR_NEUTRAL;
                    ((POLY_F3 *)packet)->g0 = face->texture.color[1] + base_color.g - FIELD_COLOR_NEUTRAL;
                    ((POLY_F3 *)packet)->b0 = face->texture.color[2] + base_color.b - FIELD_COLOR_NEUTRAL;
                    *(s32 *)&((POLY_F3 *)packet)->x0 = screen_vertices[0];
                    *(s32 *)&((POLY_F3 *)packet)->x1 = screen_vertices[1];
                    *(s32 *)&((POLY_F3 *)packet)->x2 = screen_vertices[2];
                    ((POLY_F3 *)packet)->x0 += screen_origin->vx;
                    ((POLY_F3 *)packet)->y0 += screen_origin->vy;
                    ((POLY_F3 *)packet)->x1 += screen_origin->vx;
                    ((POLY_F3 *)packet)->y1 += screen_origin->vy;
                    ((POLY_F3 *)packet)->x2 += screen_origin->vx;
                    ((POLY_F3 *)packet)->y2 += screen_origin->vy;
                    setlen((POLY_F3 *)packet, 4);
                    setcode((POLY_F3 *)packet, code);
                    if (effect->flags & FIELD_EFFECT_SEMITRANSPARENT)
                    {
                        setcode((POLY_F3 *)packet, FIELD_MESH_F3_CODE | FIELD_MESH_SEMI_TRANSPARENT);
                    }

                    packet = add_mesh_prim(ordering_table_base, effect, depth_offsets, packet, sizeof(POLY_F3));

                    setDrawTPage((DR_TPAGE *)packet, 0, 0, getTPage(FIELD_PART_TEXTURE_MODE(part), FIELD_PART_BLEND_MODE(part), 0, 0));
                    {
                        s32 depth_offset = *depth_offsets;
                        s32 base_depth = effect->z >> 7;
                        s32 depth_index = base_depth + depth_offset;
                        u_long *next_packet;

                        if (depth_index < 0)
                        {
                            next_packet = (u_long *)((DR_TPAGE *)packet + 1);
                            addPrim(ordering_table_base, packet);
                        }
                        else if (depth_index > FIELD_MESH_OT_MAX_DEPTH)
                        {
                            next_packet = (u_long *)((DR_TPAGE *)packet + 1);
                            addPrim(&ordering_table_base[FIELD_MESH_OT_MAX_DEPTH], packet);
                        }
                        else
                        {
                            next_packet = (u_long *)((DR_TPAGE *)packet + 1);
                            addPrim(ordering_table_base + (effect->z >> 7) + *depth_offsets, packet);
                        }
                        packet = next_packet;
                    }
                }
                face++;
                face_count--;
                screen_vertices += 3;
                depth_offsets++;
            } while (face_count != 0);
        }
        packet_cursor = (s32 *)packet;
        break;
    }
    case FIELD_MESH_KIND_GOURAUD:
    {
        POLY_GT3 *poly;

        face_count = FIELD_ACTOR_MESH(actor, mesh_index)->face_count;
        poly = (POLY_GT3 *)packet_cursor;
        if (face_count != 0)
        {
            do
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
                    *(s32 *)&poly->x0 = screen_vertices[0];
                    *(s32 *)&poly->x1 = screen_vertices[1];
                    *(s32 *)&poly->x2 = screen_vertices[2];
                    poly->x0 += screen_origin->vx;
                    poly->y0 += screen_origin->vy;
                    poly->x1 += screen_origin->vx;
                    poly->y1 += screen_origin->vy;
                    poly->x2 += screen_origin->vx;
                    poly->y2 += screen_origin->vy;
                    *(u16 *)&poly->u0 = face->texture.uv[0];
                    *(u16 *)&poly->u1 = face->texture.uv[1];
                    *(u16 *)&poly->u2 = face->texture.uv[2];
                    FIELD_MESH_SET_CLUT_TPAGE(actor, part, poly->clut, poly->tpage);
                    poly = add_mesh_prim(ordering_table_base, effect, depth_offsets, poly, sizeof(POLY_GT3));
                }
                face++;
                face_count--;
                screen_vertices += 3;
                depth_offsets++;
            } while (face_count != 0);
        }
        packet_cursor = (s32 *)poly;
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
s32 *field_render_lit_effect_mesh(FieldMotionRecord *effect, s32 mesh_index, s32 *buffer, s32 *ordering_table)
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
    s32 triangle_area;
    FieldActorState *actor;
    FieldActorPartDef *part;
    FieldMotionRecord *light_effect;
    s32 light_index;
    s32 *screen_vertices;
    SVECTOR *transformed_normals;
    s32 *depth_offsets;
    DVECTOR *screen_origin;
    FieldMeshFace *face;
    s32 count;
    s32 primitive_kind;

    write_cursor = buffer;
    part = &g_field_actor_slots[effect->actor_index].parts[effect->part_index];
    actor = &g_field_actor_slots[effect->actor_index];

    field_build_part_matrix(actor, effect, part, &transform, &base_matrix);
    transform.t[2] = 0;
    transform.t[1] = 0;
    transform.t[0] = 0;
    field_resolve_effect_part_color(actor, effect, part, (FieldPrimitiveColor *)&base_color);
    screen_origin = (DVECTOR *)getScratchAddr(0);
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
                    RotMatrix_gte((SVECTOR *)&g_field_effect_records[count].rotation_x, &base_matrix);
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
                    color_matrix.m[0][light_index] = g_field_actor_slots[effect->actor_index].parts[light_effect->part_index].red_or_track * 16;
                    color_matrix.m[1][light_index] = g_field_actor_slots[effect->actor_index].parts[light_effect->part_index].green_or_track * 16;
                    color_matrix.m[2][light_index] = g_field_actor_slots[effect->actor_index].parts[light_effect->part_index].blue_or_track * 16;
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
    screen_vertices = (s32 *)g_field_mesh_screen_vertices;
    transformed_normals = g_field_mesh_transformed_normals;
    depth_offsets = g_field_mesh_depth_offsets;
    face = FIELD_ACTOR_MESH(actor, mesh_index)->faces;
    screen_origin->vx = FIELD_MESH_SCREEN_CENTER_X + g_field_view_offset_x / 256 + effect->x / 256;
    screen_origin->vy = FIELD_MESH_SCREEN_CENTER_Y + g_field_view_offset_y / 256 + effect->y / 256 - effect->z / 512 - g_field_view_offset_z / 512;
    primitive_kind = FIELD_MESH_FACE_KIND(face);

    switch (primitive_kind)
    {
    case FIELD_MESH_KIND_TEXTURED:
    {
        /* The CLUT and texture page are written into the first packet and carried forward. */
        s32 code;

        gte_SetBackColor(base_color.r, base_color.g, base_color.b);
        FIELD_MESH_SET_CLUT_TPAGE(actor, part, ((POLY_FT3 *)write_cursor)->clut, ((POLY_FT3 *)write_cursor)->tpage);

        count = FIELD_ACTOR_MESH(actor, mesh_index)->face_count;
        if (count != 0)
        {
            code = FIELD_MESH_FT3_CODE;
            do
            {
                gte_ldsxy3(screen_vertices[0], screen_vertices[1], screen_vertices[2]);
                gte_nclip();
                gte_stopz(&triangle_area);
                if (triangle_area > 0)
                {
                    gte_ldv0(transformed_normals);
                    gte_ncs();
                    gte_strgb(&((POLY_FT3 *)write_cursor)->r0);
                    *(s32 *)&((POLY_FT3 *)write_cursor)->x0 = screen_vertices[0];
                    *(s32 *)&((POLY_FT3 *)write_cursor)->x1 = screen_vertices[1];
                    *(s32 *)&((POLY_FT3 *)write_cursor)->x2 = screen_vertices[2];
                    ((POLY_FT3 *)write_cursor)->x0 += screen_origin->vx;
                    ((POLY_FT3 *)write_cursor)->y0 += screen_origin->vy;
                    ((POLY_FT3 *)write_cursor)->x1 += screen_origin->vx;
                    ((POLY_FT3 *)write_cursor)->y1 += screen_origin->vy;
                    ((POLY_FT3 *)write_cursor)->x2 += screen_origin->vx;
                    ((POLY_FT3 *)write_cursor)->y2 += screen_origin->vy;
                    setlen((POLY_FT3 *)write_cursor, 7);
                    setcode((POLY_FT3 *)write_cursor, code);
                    if (effect->flags & FIELD_EFFECT_SEMITRANSPARENT)
                    {
                        setcode((POLY_FT3 *)write_cursor, FIELD_MESH_FT3_CODE | FIELD_MESH_SEMI_TRANSPARENT);
                    }
                    else
                    {
                        setcode((POLY_FT3 *)write_cursor, code);
                    }
                    *(u16 *)&((POLY_FT3 *)write_cursor)->u0 = face->texture.uv[0];
                    *(u16 *)&((POLY_FT3 *)write_cursor)->u1 = face->texture.uv[1];
                    *(u16 *)&((POLY_FT3 *)write_cursor)->u2 = face->texture.uv[2];
                    ((POLY_FT3 *)write_cursor)[1].clut = ((POLY_FT3 *)write_cursor)->clut;
                    ((POLY_FT3 *)write_cursor)[1].tpage = ((POLY_FT3 *)write_cursor)->tpage;

                    write_cursor = add_mesh_prim(ordering_table, effect, depth_offsets, write_cursor, sizeof(POLY_FT3));
                }
                face++;
                count--;
                screen_vertices += 3;
                transformed_normals++;
                depth_offsets++;
            } while (count != 0);
        }
        return write_cursor;
    }
    case FIELD_MESH_KIND_GOURAUD:
    {
        POLY_GT3 *poly;

        count = FIELD_ACTOR_MESH(actor, mesh_index)->face_count;
        poly = (POLY_GT3 *)write_cursor;
        if (count != 0)
        {
            do
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
                    *(s32 *)&poly->x0 = screen_vertices[0];
                    *(s32 *)&poly->x1 = screen_vertices[1];
                    *(s32 *)&poly->x2 = screen_vertices[2];
                    poly->x0 += screen_origin->vx;
                    poly->y0 += screen_origin->vy;
                    poly->x1 += screen_origin->vx;
                    poly->y1 += screen_origin->vy;
                    poly->x2 += screen_origin->vx;
                    poly->y2 += screen_origin->vy;
                    *(u16 *)&poly->u0 = face->texture.uv[0];
                    *(u16 *)&poly->u1 = face->texture.uv[1];
                    *(u16 *)&poly->u2 = face->texture.uv[2];
                    FIELD_MESH_SET_CLUT_TPAGE(actor, part, poly->clut, poly->tpage);
                    poly = add_mesh_prim(ordering_table, effect, depth_offsets, poly, sizeof(POLY_GT3));
                }
                face++;
                count--;
                screen_vertices += 3;
                depth_offsets++;
            } while (count != 0);
        }
        write_cursor = (s32 *)poly;
        break;
    }
    case FIELD_MESH_KIND_FLAT:
    {
        /* A lit POLY_F3 and then a DR_TPAGE selecting its blend mode. */
        u_long *packet = (u_long *)write_cursor;
        s32 code;

        count = FIELD_ACTOR_MESH(actor, mesh_index)->face_count;
        if (count != 0)
        {
            code = FIELD_MESH_F3_CODE;
            do
            {
                gte_ldsxy3(screen_vertices[0], screen_vertices[1], screen_vertices[2]);
                gte_nclip();
                gte_stopz(&triangle_area);
                if (triangle_area > 0)
                {
                    gte_SetBackColor(face->texture.color[0] + base_color.r - FIELD_COLOR_NEUTRAL, face->texture.color[1] + base_color.g - FIELD_COLOR_NEUTRAL,
                                     face->texture.color[2] + base_color.b - FIELD_COLOR_NEUTRAL);
                    gte_ldv0(transformed_normals);
                    gte_ncs();
                    gte_strgb(&((POLY_F3 *)packet)->r0);
                    *(s32 *)&((POLY_F3 *)packet)->x0 = screen_vertices[0];
                    *(s32 *)&((POLY_F3 *)packet)->x1 = screen_vertices[1];
                    *(s32 *)&((POLY_F3 *)packet)->x2 = screen_vertices[2];
                    ((POLY_F3 *)packet)->x0 += screen_origin->vx;
                    ((POLY_F3 *)packet)->y0 += screen_origin->vy;
                    ((POLY_F3 *)packet)->x1 += screen_origin->vx;
                    ((POLY_F3 *)packet)->y1 += screen_origin->vy;
                    ((POLY_F3 *)packet)->x2 += screen_origin->vx;
                    ((POLY_F3 *)packet)->y2 += screen_origin->vy;
                    setlen((POLY_F3 *)packet, 4);
                    setcode((POLY_F3 *)packet, code);
                    if (effect->flags & FIELD_EFFECT_SEMITRANSPARENT)
                    {
                        setcode((POLY_F3 *)packet, FIELD_MESH_F3_CODE | FIELD_MESH_SEMI_TRANSPARENT);
                    }
                    else
                    {
                        setcode((POLY_F3 *)packet, code);
                    }

                    packet = add_mesh_prim(ordering_table, effect, depth_offsets, packet, sizeof(POLY_F3));

                    setDrawTPage((DR_TPAGE *)packet, 0, 0, getTPage(FIELD_PART_TEXTURE_MODE(part), FIELD_PART_BLEND_MODE(part), 0, 0));
                    {
                        s32 depth_offset = *depth_offsets;
                        s32 base_depth = effect->z >> 7;
                        s32 depth_index = base_depth + depth_offset;
                        u_long *next_packet;

                        if (depth_index < 0)
                        {
                            next_packet = (u_long *)((DR_TPAGE *)packet + 1);
                            addPrim(ordering_table, packet);
                        }
                        else if (depth_index > FIELD_MESH_OT_MAX_DEPTH)
                        {
                            next_packet = (u_long *)((DR_TPAGE *)packet + 1);
                            addPrim(&ordering_table[FIELD_MESH_OT_MAX_DEPTH], packet);
                        }
                        else
                        {
                            next_packet = (u_long *)((DR_TPAGE *)packet + 1);
                            addPrim(ordering_table + (effect->z >> 7) + *depth_offsets, packet);
                        }
                        packet = next_packet;
                    }
                }
                face++;
                count--;
                screen_vertices += 3;
                transformed_normals++;
                depth_offsets++;
            } while (count != 0);
        }
        write_cursor = (s32 *)packet;
        break;
    }
    default:
        return write_cursor;
    }
    return write_cursor;
}
