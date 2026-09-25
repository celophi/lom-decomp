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

/**
 * @brief 0x18-byte mesh record in an actor's mesh table (FieldActorState.mesh_data).
 * @note faces holds 16-byte triangle records; bits 1..4 of byte 6 of the first
 *       face select the primitive kind of the whole mesh.
 */
typedef struct FieldMeshResource
{
    u16 face_count;
    u8 texture_part_count;
    u8 pad3;
    FieldMeshTexturePart *texture_parts;
    SVECTOR *vertices;
    SVECTOR *normals;
    SVECTOR *offsets;
    u8 *faces;
} FieldMeshResource;

/** @brief Mesh record @p index of @p actor's mesh table. */
#define FIELD_ACTOR_MESH(actor, index) (&((FieldMeshResource *)(actor)->mesh_data)[index])

/** @brief Vertices a mesh can transform at once (size of the shared mesh work buffers). */
#define FIELD_MESH_VERTEX_MAX 768

extern s16 *g_field_mesh_screen_vertices;
extern SVECTOR *g_field_mesh_transformed_normals;
extern s32 *g_field_mesh_depth_offsets;

#endif
