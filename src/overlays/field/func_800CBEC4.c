#include "common.h"

extern u8 g_menuLayoutBuffer[];

typedef struct
{
    u8 pad[0x2A7F];
    u8 unk2A7F;
} FieldCBEC4MenuScan;

void func_800CBEC4(void *arg0)
{
    u8 *scan;
    u8 *base;
    s32 *clear;
    s32 count;
    s32 offset;
    s32 sentinel;
    s32 marker;
    u8 first;
    u8 second;
    void *out;

    out = arg0;
    count = 0x3B;
    clear = (s32 *)((u8 *)out + 0xEC);
    do
    {
        *clear = 0;
        count--;
        clear--;
    } while (count >= 0);

    count = 0;
    base = g_menuLayoutBuffer;
    sentinel = 0x63;
    marker = 0x4F;
    offset = 0x18;
    scan = base;
    do
    {
        first = ((FieldCBEC4MenuScan *)scan)->unk2A7F;
        if (first != sentinel)
        {
            second = ((FieldCBEC4MenuScan *)((u32)offset + (u32)base))->unk2A7F;
            if ((second != sentinel) && (first != second))
            {
                *(s32 *)((u8 *)out + 0x78) = marker;
            }
        }
        out = (u8 *)out + 4;
        offset += 4;
        count++;
        scan += 4;
    } while (count < 0x1E);
}
