#include "common.h"

typedef struct { s16 x; s16 y; s16 w; s16 h; } RECT;

typedef struct {
    union {
        u32 word;
        struct {
            u32 state : 3;
            u32 unk0_3 : 4;
            u32 x : 9;
            u32 unk0_16 : 8;
        } f;
    } attr;
    u32 unk4_0 : 1;
    u32 y : 8;
    u32 unk4_9 : 23;
    void (*draw_handler)();
} CardaPacket;

extern s32 D_80042FB4;
extern u8 *D_8012271C;
extern u16 D_8014B06A;
extern CardaPacket D_80165F80;
extern u8 *D_80165FF0;
extern s32 D_80166070;

s32 func_80143414(u8 *data);
s32 func_8014385C(s32 result, s32 *ot);
void func_801447DC(s32 arg);

s32 func_8014366C(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;
    u8 *base;
    u8 *resource;
    CardaPacket *p;
    CardaPacket *cursor;
    s32 result;
    s32 x;
    s32 i;
    s32 valid;
    s32 checksum;

    x = -arg2 + 0x90;
    result = func_800A88A0(prim, ot, (void *)((s32)&D_8014B06A - 0x32 + D_8014B06A), 4, x, -arg3, 2);
    base = (u8 *)&D_8014B06A - 0x32;
    result = func_800A88A0(result, ot, base + *(u16 *)(base + 0x1E), 4, x, 0xE - arg3, 2);
    result = func_800A88A0(result, ot, base + *(u16 *)(base + 0xB2), 4, x, 0x1C - arg3, 2);
    result = func_8014385C(result, ot);

    if (D_80166070 == 0)
    {
        resource = D_80165FF0;
        p = (CardaPacket *)&D_80165F80;
        p->attr.f.state = 0;
        checksum = func_80143414(resource);
        valid = 0;
        if (*(s32 *)(resource + 0x33E0) == checksum)
        {
            valid = *(s32 *)(resource + 0x33E4) == 0x414E41;
        }
        if (valid == 0)
        {
            func_801447DC(4);
            return result;
        }

        func_800A3938(0x7B, 0x80);
        func_80016E7C(resource + 0x180, D_8012271C, 0x3268);
        D_80042FB4 = func_8002054C(-1);
        func_80067F28();

        cursor = p;
        for (i = 0; i < 8; i++, cursor++)
        {
            if (cursor->attr.f.state != 0)
            {
                cursor->attr.f.state = 3;
                cursor->attr.f.unk0_3 = 8;
            }
        }
        func_80067EB4(0, 0, 0, 8);
    }

    return result;
}
