#ifndef FIELD_MESH_TRANSFORM_H
#define FIELD_MESH_TRANSFORM_H

#include "common.h"
#include "sdk/libgte.h"
#include "field_effect_types.h"

void field_transform_mesh_vertices(FieldActorState *actor, FieldMotionRecord *record, FieldActorPartDef *part, s32 index);
void field_transform_mesh_normals(FieldActorState *actor, FieldMotionRecord *record, FieldActorPartDef *part, s32 index, MATRIX *matrix);
s32 field_build_part_matrix(FieldActorState *actor, FieldMotionRecord *record, FieldActorPartDef *part, MATRIX *matrix, MATRIX *base_matrix);
void field_copy_matrix_rotation(MATRIX *dst, MATRIX *src);
void field_animate_mesh_textures(FieldActorState *actor, FieldActorPartDef *parts, s32 part_count);

#endif
