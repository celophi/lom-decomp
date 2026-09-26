#ifndef FIELD_MESH_H
#define FIELD_MESH_H

#include "common.h"
#include "sdk/libgte.h"

/** @brief Texture part of an actor mesh: a VRAM tile and its pixel data. */
typedef struct FieldMeshTexturePart
{
    u8 x;
    u8 y;
    u8 width;
    u8 height;
    u16 *pixels;
} FieldMeshTexturePart;

/** @brief 16-byte triangle record of a mesh. */
typedef struct FieldMeshFace
{
    union
    {
        u16 uv[3];   /**< Textured faces: packed u, v of each vertex. */
        u8 color[3]; /**< Flat faces: face colour. */
    } texture;
    u8 kind;                  /**< Bits 1..4: primitive kind (the first face's kind applies to the whole mesh). */
    u8 vertex_colors[3][3];   /**< Gouraud faces: colour of each vertex. */
} FieldMeshFace;

/** @brief Primitive kind of a mesh face (FieldMeshFace kind). */
#define FIELD_MESH_FACE_KIND(face) (((face)->kind >> 1) & 0xF)

/** @brief 0x18-byte mesh record in an actor's mesh table (FieldActorState.mesh_data). */
typedef struct FieldMeshResource
{
    u16 face_count;
    u8 texture_part_count;
    u8 pad3;
    FieldMeshTexturePart *texture_parts;
    SVECTOR *vertices;
    SVECTOR *normals;
    SVECTOR *offsets;
    FieldMeshFace *faces;
} FieldMeshResource;

/** @brief Mesh record @p index of @p actor's mesh table. */
#define FIELD_ACTOR_MESH(actor, index) (&((FieldMeshResource *)(actor)->mesh_data)[index])

/** @brief Vertices a mesh can transform at once (size of the shared mesh work buffers). */
#define FIELD_MESH_VERTEX_MAX 768

extern s16 *g_field_mesh_screen_vertices;
extern SVECTOR *g_field_mesh_transformed_normals;
extern s32 *g_field_mesh_depth_offsets;

#endif
