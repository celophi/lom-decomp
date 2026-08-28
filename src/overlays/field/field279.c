#include "common.h"
#include "psyq_compat/libgte.h"
#include "psyq_compat/libgpu.h"

extern u8 g_field_resource_buffer[];

void func_8008396C(void)
{
    RECT rect;
    u8 *buf;
    u8 *base;
    u8 *data;
    u32 off;
    s32 w;
    s32 h;

    buf = g_field_resource_buffer;
    base = buf + 0x14;
    rect.x = 0;
    rect.y = 0x1EA;
    rect.w = 0x100;
    rect.h = 4;
    off = *(u32 *)(buf + 8);
    LoadImage(&rect, (u_long *)base);
    data = base + off;
    w = *(u16 *)(data - 4);
    h = *(u16 *)(data - 2);
    rect.w = w;
    rect.h = h;
    rect.x = 0x180;
    rect.y = 0;
    LoadImage(&rect, (u_long *)data);
}
