/** @file field_text_session.c
 * @brief Reset and tear down the field text session.
 */

#include "field_text.h"
#include "common.h"

extern s32 D_80122714;
extern s32 D_80122734;
extern s32 D_80122980;
extern s32 D_80122A00;

void func_800AE008(s32 context);

/** @brief Clear active text-session state and associated counters. */
void func_800AF8C4(void)
{
    D_80122980 = 0;
    D_80122734 = 0;
    D_80122714 = 0;
    D_80122A00 = 0;
}

/**
 * @brief Run one frame of the text session while one is active.
 *
 * Resets the text scratch state, updates and draws the menu elements
 * (func_800AE008) and uploads the immediate glyph cache. If the session ended
 * during the update, the text windows are reset as well.
 *
 * @param context Menu render context passed to func_800AE008.
 */
void func_800AF8E8(s32 context)
{
    if (D_80122714 != 0)
    {
        field_text_reset_scratch();
        func_800AE008(context);
        field_text_upload_immediate_cache();
        if (D_80122714 == 0)
        {
            field_text_reset_windows();
        }
    }
}
