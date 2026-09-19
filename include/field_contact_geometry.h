#ifndef FIELD_CONTACT_GEOMETRY_H
#define FIELD_CONTACT_GEOMETRY_H

#include "field_effect_types.h"
#include "vector.h"

/** @brief Packed screen point with signed 16-bit coordinates. */
typedef union
{
    s32 packed;
    Vec2s coord;
} FieldContactPoint;

/** @brief Actor index and packed point returned by a contact scan. */
typedef struct
{
    s32 actor;
    s32 point;
} FieldContactResult;

void func_800970B0(void);
s32 func_80097150(FieldContactPoint* quad, FieldMotionRecord* record, FieldContactResult* contact);
s32 func_800978AC(Vec2s* first_start, Vec2s* first_end, Vec2s* second_start, Vec2s* second_end);
s32 func_80097FA0(FieldMotionRecord* actor, s32* position, s32 mode);
s32 func_800987DC(FieldMotionRecord* record, s32* position, s32 filter_group);
void func_80098C7C(FieldMotionRecord* record, s32 actor_index);
void func_80098DD4(FieldMotionRecord* entry);
s32 func_8009980C(s32* reference_position, s32 distance_limit, FieldActorState* source_actor, s32 opposing_group);
void func_80099A48(FieldActorState* actor, FieldActorPartDef* part);
s32 func_8009A204(FieldVector* a, FieldVector* b);

#endif
