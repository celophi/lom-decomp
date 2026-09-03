#include "common.h"
#include "sdk/libgpu.h"
typedef struct { u8 _pad0[4]; u16 x1,y1,x2,y2; u8 _pad1[0x18-0xC]; } WselTexEntry;
extern WselTexEntry D_800C6720[];
extern int func_80019A34(RECT*, u_long*);
void func_800521D0(u8* res, s32 index)
{
    RECT rect;
    u8 *base;
    WselTexEntry *entry;
    s16 x1, y1, x2, y2;
    s32 block_len;
    int skip;
    base = (u8 *)D_800C6720;
    entry = (WselTexEntry *)(base + index * 0x18);
    x1 = entry->x1; y1 = entry->y1; x2 = entry->x2; y2 = entry->y2;
    skip = 8;
    if (res[4] & skip)
    {
        block_len = *(s32*)(res + skip);
        rect.w = *(u16*)(res + 0x10) * *(u16*)(res + 0x12);
        rect.x = x2;
        rect.y = y2;
        rect.h = 1;
        func_80019A34(&rect, (u_long*)(res + 0x14));
        res = (res + skip) + block_len;
    }
    else
    {
        res = res + 8;
    }
    rect.x = x1; rect.y = y1; rect.w = *(u16*)(res + 8); rect.h = *(u16*)(res + 0xA);
    func_80019A34(&rect, (u_long*)(res + 0xC));
}
