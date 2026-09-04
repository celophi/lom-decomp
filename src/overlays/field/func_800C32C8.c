#include "common.h"

extern u8 *D_80122B74;

void func_800C1EC8(void *dst, void *src, s32 n);

void func_800C32C8(void)
{
    s32 i;
    s32 dst_off;
    u8 *src;
    u8 *p;

    if ((u32)*(s32 *)(D_80122B74 + 0x2EF0) < 5)
    {
        i = 0;
        do
        {
            dst_off = i + *(s32 *)(D_80122B74 + 0x2EF0) * 0x60;
            src = D_80122B74 + i;
            i += 1;
            *(u8 *)(D_80122B74 + dst_off + 0x2EF4) = *(u8 *)(src + 0xA90);
        } while (i < 0x15);

        {
            u8 *b; s32 idx; s32 off; u32 val;
            b = D_80122B74;
            idx = *(s32 *)(b + 0x2EF0);
            off = idx * 0x60;
            val = *(u8 *)(b + 0xAB0);
            b += off;
            *(u8 *)(b + 0x2F0C) = val;
        }
        p = D_80122B74 + *(s32 *)(D_80122B74 + 0x2EF0) * 0x60;
        {
            u32 word = *(u32 *)(D_80122B74 + 0xAB0);
            u32 low = *(u8 *)(p + 0x2F0C);
            low |= (word >> 8) << 8;
            *(u32 *)(p + 0x2F0C) = low;
        }
        {
            u8 *b = D_80122B74;
            s32 off = *(s32 *)(b + 0x2EF0) * 0x60;
            u16 val = *(u16 *)(b + 0xAB4);
            b += off;
            *(u16 *)(b + 0x2F10) = val;
        }
        {
            u8 *b = D_80122B74;
            s32 off = *(s32 *)(b + 0x2EF0) * 0x60;
            off += (s32)b;
            func_800C1EC8(b + 0xAC0, (void *)(off + 0x2F1C), 0x10);
        }
    }
}
