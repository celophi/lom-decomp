#include "common/pad_repeat.h"
#include "main/controller_internal.h"

/**
 * @brief Sample the controller buttons, folding analog stick deflection into the D-pad bits.
 * @return Button mask in PAD_BTN_* order, or 0 when no controller is available.
 * @note Unlike read_pad_buttons, availability is tested through g_controller_device_type.
 * @note JP reports the face buttons as read (no PAD_REMAP_FACE_BITS swap).
 */
static inline s32 sample_pad_buttons(void)
{
    SCDRegs* regs = SCD_REGS;
    u32 buttons;
    s32 axis;

    if (g_controller_device_type >= CONTROLLER_DEVICE_CONFIGURING)
    {
        return 0;
    }

    buttons = (regs->held_buttons >> 8) | (regs->held_buttons << 8);
#if !defined(VERSION_JP)
    buttons = PAD_REMAP_FACE_BITS(buttons);
#endif
    if (regs->device_type != CONTROLLER_DEVICE_DIGITAL)
    {
        axis = regs->axis_x.signed_value;
        if (axis < -PAD_STICK_THRESHOLD)
        {
            buttons |= PAD_BTN_LEFT;
        }
        else if (axis > PAD_STICK_THRESHOLD)
        {
            buttons |= PAD_BTN_RIGHT;
        }

        axis = regs->axis_y.signed_value;
        if (axis < -PAD_STICK_THRESHOLD)
        {
            buttons |= PAD_BTN_UP;
        }
        else if (axis > PAD_STICK_THRESHOLD)
        {
            buttons |= PAD_BTN_DOWN;
        }
    }
    return buttons;
}
