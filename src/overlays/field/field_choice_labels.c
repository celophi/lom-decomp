/** @file field_choice_labels.c
 * @brief Draw the two return-to-title choice labels.
 */

#include "common.h"
#include "field_ui_text.h"

/** @brief UI string index of the first return-to-title choice. */
#define FIELD_TITLE_CHOICE_FIRST_TEXT 19

/** @brief Text color of a highlighted choice. */
#define FIELD_CHOICE_COLOR_ACTIVE 4

/** @brief Text color of an inactive choice. */
#define FIELD_CHOICE_COLOR_INACTIVE 5

s32 func_800A88A0(void* prim, void* ot, void* text, s32 color, s32 x, s32 y, s32 align);
extern u8 D_800EC3EA[];
extern s32 D_801226D8;

/**
 * @brief Draw the two return-to-title choice labels with the active choice highlighted.
 * @param ot Ordering-table context passed to the label renderer.
 * @param prim Primitive-buffer cursor the labels are written from.
 * @param x_offset Horizontal offset subtracted from the label position.
 * @param y_offset Vertical offset subtracted from the label position.
 */
void func_800AED20(void* ot, void* prim, s32 x_offset, s32 y_offset)
{
    u8* text_table;
    u8* first_text;
    u8* second_text;
    s32 first_color;
    s32 second_color;
    s32 x;
    void* cursor;
    s32 unused[2]; /* never used; the original stack frame reserves it */

    cursor = prim;
    first_text = FIELD_UI_TEXT_AT(D_800EC3EA, FIELD_TITLE_CHOICE_FIRST_TEXT);
    text_table = D_800EC3EA - FIELD_TITLE_CHOICE_FIRST_TEXT * 2;
    first_color = FIELD_CHOICE_COLOR_INACTIVE;
    if (D_801226D8 == 0)
    {
        first_color = FIELD_CHOICE_COLOR_ACTIVE;
    }
    x = 0x50 - x_offset;
    cursor = (void*)func_800A88A0(cursor, ot, first_text, first_color, x, 1 - y_offset, 2);

    second_text = FIELD_UI_TEXT(text_table, FIELD_TITLE_CHOICE_FIRST_TEXT + 1);
    second_color = FIELD_CHOICE_COLOR_INACTIVE;
    if (D_801226D8 == 1)
    {
        second_color = FIELD_CHOICE_COLOR_ACTIVE;
    }
    func_800A88A0(cursor, ot, second_text, second_color, x, 0x11 - y_offset, 2);
}
