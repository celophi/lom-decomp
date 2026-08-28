#include "common.h"

extern void func_800A7384(void);
extern void func_800A764C(void);
extern void func_800A7724(void);
extern s32 func_800ADEEC(void);
extern s32 D_800F229C;
extern s32 D_801227EC;
extern s32 D_80122908;
extern s32 g_pad_input;

/**
 * @brief Route a confirm/cancel pad press to the active field sub-dialog.
 *
 * When no modal is blocking (func_800ADEEC returns 0) and a confirm or cancel
 * button is held (@c g_pad_input & 0x220), dispatches on the current dialog
 * mode @c D_800F229C: mode 1 advances via func_800A7384 (gated by
 * @c D_801227EC); mode 3 backs out via func_800A764C when @c D_80122908 is set,
 * else falls through to the mode-2 handler func_800A7724.
 *
 * @see decomp.me (100%) TODO
 */
void func_800A710C(void)
{
    if ((func_800ADEEC() == 0) && (g_pad_input & 0x220))
    {
        switch (D_800F229C)
        {
        case 1:
            if (D_801227EC == 0)
            {
                func_800A7384();
                return;
            }
            break;
        case 3:
            if (D_80122908 != 0)
            {
                func_800A764C();
                return;
            }
            /* fallthrough */
        case 2:
            func_800A7724();
            break;
        }
    }
}
