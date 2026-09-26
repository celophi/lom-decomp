/** @file field_choice_labels.c
 * @brief Draw the two choices of the return-to-title prompt.
 */

#include "common.h"
#include "sdk/libgpu.h"
#include "field_calls.h"
#include "field_ui_text.h"

/** @brief UI string index of the first return-to-title choice; the second follows it. */
#define FIELD_TITLE_CHOICE_FIRST_TEXT 19

/** @brief g_field_return_to_title_choice value: continue from the saved state. */
#define FIELD_TITLE_CHOICE_CONTINUE 0

/** @brief g_field_return_to_title_choice value: return to the title screen. */
#define FIELD_TITLE_CHOICE_QUIT 1

/** @brief Text color of the highlighted choice. */
#define FIELD_CHOICE_COLOR_ACTIVE 4

/** @brief Text color of the other choice. */
#define FIELD_CHOICE_COLOR_INACTIVE 5

/** @brief field_draw_text() flag that centers the text on x. */
#define FIELD_TEXT_ALIGN_CENTER 2

/** @brief Horizontal center of the choices inside the prompt window. */
#define FIELD_CHOICE_CENTER_X 80

/** @brief Top of the first choice inside the prompt window. */
#define FIELD_CHOICE_FIRST_Y 1

/** @brief Top of the second choice inside the prompt window. */
#define FIELD_CHOICE_SECOND_Y 17

void* field_draw_text(SPRT* sprite_cursor, s32* ot, u8* text, s32 color, s32 x, s32 y, s32 flags);

/** @brief Offset entry of UI string FIELD_TITLE_CHOICE_FIRST_TEXT (each entry is its own symbol). */
extern u8 g_field_title_choice_text_entry[];

/**
 * @brief Draw both return-to-title choices, highlighting the selected one.
 * @param ot Ordering table the text is added to.
 * @param prim Primitive buffer the text sprites are written to.
 * @param scroll_x Horizontal scroll of the prompt window, subtracted from the text position.
 * @param scroll_y Vertical scroll of the prompt window, subtracted from the text position.
 */
void field_draw_return_to_title_choices(s32* ot, void* prim, s32 scroll_x, s32 scroll_y)
{
    u8* text_table;
    u8* first_text;
    u8* second_text;
    s32 first_color;
    s32 second_color;
    s32 x;
    void* cursor;
    s32 unused[2]; /* never used, but the stack frame only matches the original with it */

    cursor = prim;
    first_text = FIELD_UI_TEXT_AT(g_field_title_choice_text_entry, FIELD_TITLE_CHOICE_FIRST_TEXT);
    text_table = g_field_title_choice_text_entry - FIELD_TITLE_CHOICE_FIRST_TEXT * 2;
    first_color = (g_field_return_to_title_choice == FIELD_TITLE_CHOICE_CONTINUE) ? FIELD_CHOICE_COLOR_ACTIVE : FIELD_CHOICE_COLOR_INACTIVE;
    x = FIELD_CHOICE_CENTER_X - scroll_x;
    cursor = field_draw_text(cursor, ot, first_text, first_color, x, FIELD_CHOICE_FIRST_Y - scroll_y, FIELD_TEXT_ALIGN_CENTER);

    second_text = FIELD_UI_TEXT(text_table, FIELD_TITLE_CHOICE_FIRST_TEXT + 1);
    second_color = (g_field_return_to_title_choice == FIELD_TITLE_CHOICE_QUIT) ? FIELD_CHOICE_COLOR_ACTIVE : FIELD_CHOICE_COLOR_INACTIVE;
    field_draw_text(cursor, ot, second_text, second_color, x, FIELD_CHOICE_SECOND_Y - scroll_y, FIELD_TEXT_ALIGN_CENTER);
}
