#ifndef FIELD_ACTOR_ROUTES_H
#define FIELD_ACTOR_ROUTES_H

#include "common.h"

/* Defined in src/overlays/field/field_actor_tables.h. */
struct FieldActor;

void func_8008C728(void);
void field_reset_leader_position_history(void);
void field_refresh_party_routes(void);
void field_record_actor_position(struct FieldActor* record);
void field_follow_leader_route(struct FieldActor* actor, s32 follower_index);

#endif
