#include "common.h"

extern s32 D_80122714;

void field_text_reset_scratch(void);
void func_800AE008(s32 arg0);
void func_80063194(void);
void field_text_reset_windows(void);

/**
 * @brief Runs the field text teardown sequence when a text session is active.
 *
 * When @c D_80122714 is set, resets the text scratch state, runs the
 * @p arg0-specific teardown (func_800AE008) and func_80063194, and - if that
 * teardown cleared @c D_80122714 - also resets the text windows.
 *
 * @param arg0 Passed to func_800AE008.
 */
void func_800AF8E8(s32 arg0)
{
    if (D_80122714 != 0)
    {
        field_text_reset_scratch();
        func_800AE008(arg0);
        func_80063194();
        if (D_80122714 == 0)
        {
            field_text_reset_windows();
        }
    }
}
