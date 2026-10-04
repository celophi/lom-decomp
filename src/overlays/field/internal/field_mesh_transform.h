#ifndef FIELD_MESH_TRANSFORM_H
#define FIELD_MESH_TRANSFORM_H

#include "common.h"
#include <libgte.h>
#include "field_effect_types.h"

void field_transform_mesh_vertices(FieldActorSlot *actor, FieldMotionRecord *record, FieldObjectPart *part, s32 index);
void field_transform_mesh_normals(FieldActorSlot *actor, FieldMotionRecord *record, FieldObjectPart *part, s32 index, MATRIX *matrix);
s32 field_build_part_matrix(FieldActorSlot *actor, FieldMotionRecord *record, FieldObjectPart *part, MATRIX *matrix, MATRIX *base_matrix);
void field_copy_matrix_rotation(MATRIX *dst, MATRIX *src);
void field_animate_mesh_textures(FieldActorSlot *actor, FieldObjectPart *parts, s32 part_count);

#endif
