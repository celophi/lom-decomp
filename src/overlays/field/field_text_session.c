/** @file field_text_session.c
 * @brief Reset and tear down the field text session.
 */

#include "field_text.h"
#include "common.h"
#include "field_calls.h"
#include "field_menu_element.h"

extern s32 D_80122714;
extern s32 g_field_item_list_count;
extern s32 D_80122980;
extern s32 g_field_item_list_cursor;

/** @brief Clear active text-session state and associated counters. */
void func_800AF8C4(void)
{
    D_80122980 = 0;
    g_field_item_list_count = 0;
    D_80122714 = 0;
    g_field_item_list_cursor = 0;
}

/**
 * @brief Run one frame of the text session while one is active.
 *
 * Resets the text scratch state, updates and draws the menu elements
 * (field_draw_menu_elements) and uploads the immediate glyph cache. If the session ended
 * during the update, the text windows are reset as well.
 *
 * @param context Menu render context passed to field_draw_menu_elements.
 */
void func_800AF8E8(s32 context)
{
    if (D_80122714 != 0)
    {
        field_text_reset_scratch();
        field_draw_menu_elements((FieldRenderHalf*)context);
        field_text_upload_immediate_cache();
        if (D_80122714 == 0)
        {
            field_text_reset_windows();
        }
    }
}
