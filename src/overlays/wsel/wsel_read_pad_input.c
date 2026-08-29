#include "common.h"
#include "pad.h"

extern u8 D_801ED600[];
extern s32 D_800CA8D4;
extern s32 D_800CA8A0;
extern s32 D_800CA8A4;

void func_80052510(void)
{
    SCDRegs *base = SCD_REGS;
    s32 state;
    u32 buttons;
    s16 axis;

    D_800CA8D4 = 0;
    if (D_801ED600[0] >= 254)
    {
        state = 0;
    }
    else
    {
        buttons = ((base->held_buttons >> 8) & 0xFF) | (base->held_buttons << 8);
        buttons = PAD_REMAP_FACE_BITS(buttons);
        if (base->device_type != 0)
        {
            axis = base->axis_x.signed_value;
            if (axis < (-1))
            {
                buttons |= PAD_BTN_LEFT;
            }
            else if (axis >= 2)
            {
                buttons |= PAD_BTN_RIGHT;
            }
            axis = base->axis_y.signed_value;
            if (axis < (-1))
            {
                buttons |= PAD_BTN_UP;
            }
            else if (axis >= 2)
            {
                buttons |= PAD_BTN_DOWN;
            }
        }
        state = buttons;
    }
    D_800CA8A0 = state;
    D_800CA8A4 = 15;
}
