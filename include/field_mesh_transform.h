#ifndef FIELD_MESH_TRANSFORM_H
#define FIELD_MESH_TRANSFORM_H

#include "common.h"
#include "sdk/libgte.h"
#include "field_effect_types.h"

void func_800822A4(FieldActorState *actor, FieldMotionRecord *record, FieldActorPartDef *part, s32 index);
void func_800829A0(FieldActorState *actor, FieldMotionRecord *record, FieldActorPartDef *part, s32 index, MATRIX *matrix);
s32 func_80082C90(FieldActorState *actor, FieldMotionRecord *record, FieldActorPartDef *part, MATRIX *matrix, MATRIX *base_matrix);
void func_800832F0(MATRIX *dst, MATRIX *src);

#endif
