#ifndef FIELD_MENU_VARS_H
#define FIELD_MENU_VARS_H

/**
 * @file field_menu_vars.h
 * @brief FIELD view of the script temporary variables at D_80122C00.
 */

#include "common.h"

/**
 * @brief Script temporary-variable words at D_80122C00, as the menu and golem programs use them.
 *
 * Field scripts address this area as bitfield variables; each menu program
 * gives the bytes its own meaning, so each program has its own view.
 */
typedef union
{
    u8 bytes[0x20];
    s32 words[8];
    /** @brief Golem slot menus. */
    struct
    {
        s32 slot; /**< Selected golem slot, an index into large_history_order. */
        s16 state;
        s16 slot_status[3];
        u8 unk0C[4];
        s16 result;
        u8 unk12[6];
        s16 unk18;
        s16 unk1A;
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

extern FieldMenuVars D_80122C00;

#endif /* FIELD_MENU_VARS_H */
