#include "common.h"

extern void *g_field_resource_cursor;

void func_8009BDD4(s32 *src, s32 *end)
{
    s32 size;
    s32 words;
    s32 *dst;

    size = (char *) end - (char *) src + 3;
    words = size >> 2;
    dst = g_field_resource_cursor;
    g_field_resource_cursor = (char *) dst + (size & ~3);
    while (words != 0)
    {
        *dst = *src;
        dst++;
        src++;
        words--;
    }
}
