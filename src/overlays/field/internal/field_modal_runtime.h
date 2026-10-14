#ifndef FIELD_MODAL_RUNTIME_H
#define FIELD_MODAL_RUNTIME_H

#include "common.h"

/** @brief Modal operation serviced by field_update_modal. */
typedef enum FieldModalState
{
    FIELD_MODAL_NONE = 0,
    FIELD_MODAL_SHOP = 1,
    FIELD_MODAL_GOSUB = 2,
    FIELD_MODAL_CARDA = 3,
    FIELD_MODAL_NIKI = 4,
    FIELD_MODAL_ADDHERO = 5,
    FIELD_MODAL_EMPTY_SHOP = 6,
    FIELD_MODAL_DUEL_INTRO = 7,
    FIELD_MODAL_DUEL_RESULT = 8
} FieldModalState;

struct FieldRenderHalf;

void field_process_input(struct FieldRenderHalf* render);
void field_reset_text_session(void);
void field_rebuild_party_actions(s32 refresh_only);
void field_update_modal(struct FieldRenderHalf* render);
void field_begin_duel_intro(void);
void field_begin_duel_result(void);

/** @brief Set from battle entry until the dialog or text session closes; while set, the menu button opens the text session. */
extern s32 g_field_menu_locked;

/** @brief Non-zero while a text session (message or label window) runs. */
extern s32 g_field_text_session_active;

/** @brief Non-zero while a timed panel is shown. */
extern s32 g_field_timed_panel_active;

/** @brief State of the modal runtime; 0 when no modal window is open. */
extern s32 g_field_modal_state;

/** @brief Non-zero while the item drop menu is open. */
extern s32 g_field_item_drop_menu_open;

#endif
