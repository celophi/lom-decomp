#include "common.h"
#include "pad.h"

extern u8 D_801ED600[];
extern s32 D_800CA8B0;
extern s32 D_800CA8A0;
extern s32 D_800CA8D4;
extern s32 D_800CA8A4;

void func_80052384(void)
{
    u8* ptr = (u8*)0x801ED600;
    u8 device_status = D_801ED600[0];
    u16 raw_buttons;
    u16 unused;
    u32 buttons;
    s16 axis;
    s32 state;
    if (device_status >= 0xFE)
    {
        state = 0;
    }
    else
    {
        raw_buttons = *((u16*)(ptr + 2));

        buttons = (raw_buttons >> 8) | (*((u16*)(2 + ptr)) << 8);
        buttons = PAD_REMAP_FACE_BITS(buttons);
        if ((*ptr) != 0)
        {
            axis = *((s16*)(ptr + 0x2C));
            if (axis < (-1))
            {
                buttons |= PAD_BTN_LEFT;
            }
            else if (axis >= 2)
            {
                buttons |= PAD_BTN_RIGHT;
            }
            axis = *((volatile s16*)(ptr + 0x2E));
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
    {
        s32 current_state = state;
        D_800CA8B0 = current_state;
        D_800CA8D4 = 0;

        if (((current_state == D_800CA8A0) || ((D_800CA8A0 != 0) && (current_state & (D_800CA8A0 | 0xB6F)))) && current_state != 0)
    {
        u32 dpad = current_state & (PAD_BTN_UP | PAD_BTN_RIGHT | PAD_BTN_DOWN | PAD_BTN_LEFT);
        if (dpad != 0)
        {
            current_state = dpad;
        }
        if (D_800CA8A4 == 0)
        {
            D_800CA8D4 = current_state;
            D_800CA8A4 = 2;
        }
        else
        {
            D_800CA8A4--;
            D_800CA8D4 = 0;
        }
        return;
    }
        else if (current_state == 0)
    {
        (void)(&D_800CA8D4);
        *((s32*)(&D_800CA8A4)) = 0;
        *((s32*)(&D_800CA8A0)) = 0;
    }
    else
    {
            D_800CA8D4 = current_state;
            D_800CA8A0 = current_state;
        D_800CA8A4 = 15;
    }
    }
}
