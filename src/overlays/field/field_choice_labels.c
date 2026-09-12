#include "common.h"

s32 func_800A88A0(void* arg0, void* arg1, void* arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6);
extern u8 D_800EC3EA[];
extern s32 D_801226D8;

/**
 * @brief Draw the two return-to-title choice labels with the active choice highlighted.
 * @param arg0 Ordering-table context passed to the label renderer.
 * @param arg1 Initial packet handle.
 * @param arg2 Horizontal label offset.
 * @param arg3 Vertical label offset.
 */
void func_800AED20(void* arg0, s32 arg1, s32 arg2, s32 arg3)
{
    u8* text_base;
    void* first_text;
    void* second_text;
    s32 first_color;
    s32 second_color;
    s32 draw_x;
    s32 second_y_offset;
    s32 text_base_offset;
    s32 handle;
    s32 pad[2];
    void* draw_ctx;

    draw_ctx = arg0;
    handle = arg1;
    text_base_offset = 0x26;
    first_text = (void*)(D_800EC3EA[0] + ((D_800EC3EA[1] << 8) + (s32)(text_base = D_800EC3EA - text_base_offset)));
    first_color = 5;
    if (D_801226D8 == 0)
    {
        first_color = 4;
    }
    draw_x = 0x50 - arg2;
    handle = func_800A88A0((void*)handle, draw_ctx, first_text, first_color, draw_x, 1 - arg3, 2);
    second_y_offset = arg3;
    second_text = (void*)(text_base[0x28] + ((text_base[0x29] << 8) + (s32)text_base));
    second_color = 5;
    if (D_801226D8 == 1)
    {
        second_color = 4;
    }
    func_800A88A0((void*)handle, draw_ctx, second_text, second_color, draw_x, 0x11 - second_y_offset, 2);
}
