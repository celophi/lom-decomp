#include "common.h"

extern s32 D_801227C8;
extern s32 D_80122984;

void akao_cmd_98_9a_9c_9e(s32 arg0);
void field_text_reset_scratch(void);
void field_text_reset_windows(void);
void func_80063194(void);
void func_800A3904(s32 arg0, s32 arg1, s32 arg2);
void func_800A92CC(s32 arg0);
void func_800A939C(s32 arg0);
void func_800A9B88(void);

/**
 * @brief Tear down or advance the active field text window on a gated event.
 *
 * When the text subsystem is active (@c D_801227C8), runs func_800A9B88 and, if
 * still active, resets the scratch buffer and dispatches the per-mode advance
 * (@c func_800A92CC / @c func_800A939C selected by @c D_80122984). If the pass
 * cleared the active flag it instead resets the windows, stops the sequence
 * (@c akao_cmd_98_9a_9c_9e), and kicks off the close fade.
 *
 * @param arg0 Mode parameter forwarded to the advance dispatch.
 * @see decomp.me (100%) TODO
 */
void func_800AA858(s32 arg0)
{
    if (D_801227C8 != 0)
    {
        func_800A9B88();
        if (D_801227C8 != 0)
        {
            field_text_reset_scratch();
            if (D_80122984 != 0)
            {
                func_800A92CC(arg0);
            }
            else
            {
                func_800A939C(arg0);
            }
            func_80063194();
            return;
        }
        field_text_reset_windows();
        akao_cmd_98_9a_9c_9e(2);
        func_800A3904(0, 0x3C, 0x7F);
    }
}
