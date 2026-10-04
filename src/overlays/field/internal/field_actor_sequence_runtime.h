#ifndef FIELD_ACTOR_SEQUENCE_RUNTIME_H
#define FIELD_ACTOR_SEQUENCE_RUNTIME_H

#include "field_effect_types.h"

/** @brief Weapon-specific technique banks stored in the actor sequence resource. */
#define FIELD_SEQUENCE_BANK_COUNT 11
/** @brief Technique sequence rows in each weapon bank. */
#define FIELD_SEQUENCE_ROW_COUNT 24
/** @brief Bytes reserved for one technique sequence. */
#define FIELD_SEQUENCE_ROW_SIZE 32
/** @brief Bytes in one weapon's technique sequence bank. */
#define FIELD_SEQUENCE_BANK_SIZE (FIELD_SEQUENCE_ROW_COUNT * FIELD_SEQUENCE_ROW_SIZE)

/** @brief Actor sequence bytecode, arranged by weapon bank, row and byte. */
extern u8 g_field_actor_sequence_data[FIELD_SEQUENCE_BANK_COUNT * FIELD_SEQUENCE_BANK_SIZE];

struct FieldActor;

/** @brief Binding of an object owner to a temporary animation actor. */
typedef struct
{
    s32 state;
    u8 pad_0x4[8];
    s32 owner_object_index;
    u8 pad_0x10[8];
    s32 actor_index;
} FieldSequenceBinding;

extern FieldSequenceBinding g_field_actor_bindings[];

void field_apply_sequence_displacement(struct FieldActor* actor, s32 direction_x, s32 vertical_step, s32 direction_z);
void field_update_sequence_actor_binding(struct FieldActor* actor, s32 release_actor);
s32 field_execute_actor_sequence(struct FieldActor* actor, s32 script_index);
s32 field_allocate_sequence_actor(s32 index, s32 flags);
void field_restart_sequence_animation(struct FieldActor* actor);
void field_update_object_tints(void);

#endif
