#ifndef FIELD_INTERACTION_START_H
#define FIELD_INTERACTION_START_H

#include "common.h"

#define FIELD_ACTION_SCRIPT_COUNT 16

/** @brief Packed actor/action definition loaded from a field resource. */
typedef struct
{
    union
    {
        s32 flags;
        struct
        {
            u8 kind_flags;
            u8 selector;
            u8 local_variable_count;
            u8 render_flags;
        } bytes;
    } control;
    struct
    {
        u16 variable;
        u8 minimum;
        u8 maximum;
    } condition;
    union
    {
        u32 word;
        struct
        {
            u16 x;
            u16 z;
        } halves;
    } position;
    union
    {
        u16 actor;
        s16 result_type;
    } source;
    u16 enabled_events;
    u16 scripts[FIELD_ACTION_SCRIPT_COUNT];
} FieldActionRequest;

void field_install_actor_action(FieldActionRequest* request, s32 request_index);

#endif
