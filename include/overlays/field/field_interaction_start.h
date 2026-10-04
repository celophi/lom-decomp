#ifndef FIELD_INTERACTION_START_H
#define FIELD_INTERACTION_START_H

#include "common.h"

#define FIELD_ACTION_SCRIPT_COUNT 16

/** @brief Scene layout record describing a conditional actor or action. */
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
        struct
        {
            /** @brief Copied to the installed record's trigger group. */
            u32 trigger_group : 4;
            /** @brief FieldActionKind. */
            u32 kind : 4;
            /** @brief Menu actions: bit 7 menu slot group, bits 0-2 slot; events: event group. */
            u32 selector : 8;
            u32 local_variable_count : 8;
            /** @brief CLUT column of the actor's palette (FieldActorControl::palette). */
            u32 palette : 4;
            /** @brief Group actors: actor group plus one; zero takes the layout default. */
            u32 group : 2;
            /** @brief The actor starts hidden (FIELD_ACTOR_HIDDEN). */
            u32 hidden : 1;
            /** @brief Set while the action is installed and active. */
            u32 active : 1;
        } bits;
    } control;
    struct
    {
        /** @brief Encoded script-variable reference tested against the inclusive range. */
        u16 variable_ref;
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
        struct
        {
            u32 x : 16;
            u32 z : 11;
            u32 unk27 : 3;
            u32 y : 2;
        } bits;
    } position;
    union
    {
        u16 actor;
        s16 result_type;
    } source;
    u16 enabled_events;
    /**
     * @brief Event scripts and parameters interpreted by the actor's scripts.
     * Common chests use [4] for the item ID and [5] for the collection-variable
     * reference, with bit 15 selecting the alternate facing.
     */
    u16 scripts[FIELD_ACTION_SCRIPT_COUNT];
} FieldLayoutRecord;

void field_install_actor_action(FieldLayoutRecord* layout_record, s32 record_index);

#endif
