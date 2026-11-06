#ifndef FIELD_SCRIPT_LOCALS_H
#define FIELD_SCRIPT_LOCALS_H

/**
 * @file field_script_locals.h
 * @brief The field scripts' local variables, as the menu and golem programs use them.
 */

#include "common.h"

/**
 * @brief The 0x20 bytes of script local variables at the start of FIELD's runtime context.
 *
 * Field scripts address this area as bitfield variables; each menu program
 * gives the bytes its own meaning, so each program has its own view.
 */
typedef union
{
    u8 bytes[0x20];
    s32 words[8];
    /** @brief While scripts are being set up: the first free local variable slot. */
    s32 local_variable_base;
    /** @brief Golem slot menus. */
    struct
    {
        s32 slot; /**< Selected golem slot, an index into golem_order. */
        s16 state;
        s16 slot_status[3];
        u8 unk0C[4];
        s16 result;
        u8 unk12[6];
        s16 unk18;
        s16 joined_group; /**< Group of the joined golem after a reorder. */
        s16 order_position;
        s16 detail;
    } golem;
    /** @brief Mystic Card slot menus. */
    struct
    {
        s16 card_ids[3];
    } cards;
    /** @brief Menu action item menus. */
    struct
    {
        u8 item_counts[8]; /**< Owned count of each action item. */
        u16 item_mask;     /**< Bit set for each action item that is not owned. */
        u8 owned_count;
        u8 used_slots; /**< Item slots in use in the active group. */
        u8 group_full; /**< Set when fewer than three item slots stay free. */
        u8 unk0D[7];
        s16 selected_item; /**< Chosen action item, 0-7. */
    } actions;
} FieldMenuVars;

/**
 * @brief GOLEM's name for the script locals.
 * @note FIELD reaches the same bytes as g_field_runtime_context.locals.
 */
extern FieldMenuVars g_field_script_locals;

#endif /* FIELD_SCRIPT_LOCALS_H */
