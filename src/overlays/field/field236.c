#include "common.h"
extern u8 *D_80123FB0;
void *func_800B2B08(void)
{
    s32 offset;
    s32 i;
    u8 *p;
    u8 *base;
    i = 3;
    base = D_80123FB0;
    offset = 0x160;
    p = base + 0x138;
    do {
        i++;
        if ((*(u32 *)(p + 0x2C) >> 8) & 1) {
            offset += 0x68;
            p += 0x68;
        } else {
            return base + offset;
        }
    } while (i < 0xB);
    return 0;
}
