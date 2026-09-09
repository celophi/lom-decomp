#include "common.h"

typedef struct
{
    u8 pad0[0x25C];
    u8 unk25C;
    u8 unk25D;
    u8 pad25E[0x268 - 0x25E];
} FieldEntry268;

extern s32 D_801227EC;
extern FieldEntry268 D_800FD818[];
extern u8 *g_pad_ctx;
extern u8 D_800EC3D6[];
extern s32 D_8011F420;

s32 func_800B0888(void *arg0);
void *func_800A88A0(void *sprite_cursor, void *ot, u8 *text, s32 color, s32 x, s32 y, s32 flags);
void *func_800A8660(void *ot, void *cursor, s32 x, s32 y);
void *func_800A8524(void *ot, void *cursor, s32 x, s32 y);
void *func_800A8A78(void *ot, void *cursor, s32 value, s32 color, s16 *position, s32 flags);

void func_800A788C(void *ot, void *cursor, s32 x_offset, s32 y_offset)
{
    s32 i;
    s32 total_x;
    s32 total_y;
    volatile u8 *entry;
    u8 *pad;
    volatile u8 *text_base;
    void *handle;
    void *first_cursor;
    s16 position[2];
    s16 y;

    if (D_801227EC != 0)
    {
        first_cursor = cursor;
        if ((x_offset | y_offset) != 0)
        {
            goto loop_setup;
        }
        D_801227EC--;
        if (D_801227EC != 0)
        {
            goto loop_setup;
        }
        if (func_800B0888(first_cursor) != 0)
        {
            D_801227EC = 1;
        }
    }

    first_cursor = cursor;
loop_setup:
    i = 0;
    total_x = i;
    total_y = i;
    entry = (u8 *)D_800FD818;
    pad = g_pad_ctx;
    do
    {
        if (pad[0x5F0] != 0)
        {
            total_x += entry[0x25C];
            total_y += entry[0x25D];
        }
        entry += 0x268;
        i++;
        pad += 0x250;
    } while (i < 3);

    text_base = D_800EC3D6 - 0x12;
    handle = func_800A88A0(first_cursor, ot,
        D_800EC3D6[0] + ((D_800EC3D6[1] << 8) + text_base),
        4, 0x10 - x_offset, -y_offset, 0);

    y = 0x10 - y_offset;
    position[1] = y;
    handle = func_800A8660(ot, handle, 0x18 - x_offset, y - 3);

    position[0] = 0x28 - x_offset;
    handle = func_800A8A78(ot, handle, total_x, 4, position, 0);
    handle = func_800A88A0(handle, ot,
        text_base[0x16] + ((text_base[0x17] << 8) + text_base),
        4, 0x40 - x_offset, position[1], 0);
    handle = func_800A8524(ot, handle, 0x50 - x_offset, position[1] + 2);

    position[0] = 0x60 - x_offset;
    handle = func_800A8A78(ot, handle, total_y, 4, position, 0);
    handle = func_800A88A0(handle, ot,
        text_base[0x1A] + ((text_base[0x1B] << 8) + text_base),
        4, 0x80 - x_offset, position[1], 0);

    position[0] = 0xD0 - x_offset;
    handle = func_800A8A78(ot, handle, *(s32 *)(g_pad_ctx + 0x2C) - D_8011F420, 4, position, 1);
    func_800A88A0(handle, ot,
        text_base[0] + ((text_base[1] << 8) + text_base),
        4, 0xF0 - x_offset, position[1], 1);
}
