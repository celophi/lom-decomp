#include "common.h"
#include "sdk/libgpu.h"

typedef struct { s16 x, y; } WselPoint;

extern u8 D_800C32F0[];
extern u8 D_800C345C[];
extern u8 D_800C3480[];
extern u8 D_800C3708[];
extern u8 D_800C6720[];
extern u8 *D_800C6870;
extern s32 D_800CA898;
extern s32 D_800CA8B0;
extern WselPoint D_800CA8B8;
extern WselPoint D_800CA8BC;

extern void func_8001A5D4(void *, void *);
extern void func_8001C56C(void *, s32, s32, s32, s32);
extern void *func_800513D0();

void *func_80050F0C(void *prim, u_long *ot)
{
    DRAWENV draw_env;
    s32 col0;
    s32 cell_index;
    s32 dx;
    s32 dy;
    s32 qx;
    s32 qy;
    s32 qy2;
    s32 remx;
    s32 remy;
    s32 row;
    s32 col;
    volatile s32 saved_index;
    u8 *p = prim;

    dx = -D_800CA8B8.x;
    dx += D_800CA8BC.x;
    qx = dx - 0x10;
    if (qx < 0)
        qx = dx - 1;
    qx >>= 4;
    col0 = qx;

    dy = D_800CA8B8.y * -1 + D_800CA8BC.y;
    {
        s32 yoff = dy - 0x10;
        if (yoff < 0)
            yoff = dy - 1;
        qy = yoff >> 4;
    }

    {
        u8 *occupancy = D_800C32F0;
        cell_index = qy * 0x13;
        if (occupancy[cell_index + col0] != 0) {
        for (row = 0; row < 6; row++) {
            for (col = 0; col < 6; col++) {
                p = func_800513D0(p, ot,
                    *(u16 *)(D_800C6720 + 0x44) + col * 0x10 + 0xB,
                    *(u16 *)(D_800C6720 + 0x46) + row * 0x10 + 0xB,
                    0xA0);
            }
        }
        goto done;
        }
    }

    func_8001A5D4(p, D_800C6870 + ((D_800CA898 ^ 1) * 0x80CC) + 0x4054);
    addPrim(ot, p);
    p += 0x40;

    dx = -D_800CA8B8.x;
    dx += D_800CA8BC.x;
    remx = dx - 0x10;
    qx = remx;
    if (remx < 0)
        qx = dx - 1;
    remx -= (qx >> 4) * 0x10;

    dy = D_800CA8B8.y * -1 + D_800CA8BC.y;
    remy = dy - 0x10;
    qy2 = remy;
    if (remy < 0)
        qy2 = dy - 1;
    remy -= (qy2 >> 4) * 0x10;

    if (D_800CA8B0 & 0x10) {
        saved_index = cell_index;
        for (row = 0; row < 6; row++) {
            s32 base_cell = saved_index + col0;
            u8 * const grid = D_800C345C;
            for (col = 0; col < 6; col++) {
                s32 off = base_cell << 3;
                s32 pos;
                u8 *cellp;
                off = (off + base_cell) << 2;
                cellp = grid + off;
                pos = row * 6 + col;
                if (cellp[pos] == 0) {
                    p = func_800513D0(p, ot, col * 0x10 - remx,
                                      row * 0x10 - remy, 0x130);
                }
            }
        }

        for (col = 0; col < 6; col++) {
            if (D_800C3708[(qy * 0x13 + col0) * 0x24 + col + 0x1E] == 0) {
                p = func_800513D0(p, ot, col * 0x10 - remx,
                                  0x60 - remy, 0x130);
            }
        }

        for (row = 0; row < 6; row++) {
            if (D_800C3480[(qy * 0x13 + col0) * 0x24 + row * 6 + 5] == 0) {
                p = func_800513D0(p, ot, 0x60 - remx,
                                  row * 0x10 - remy, 0x130);
            }
        }

        if (D_800C345C[(col0 + ((qy + 1) * 0x13) + 1) * 0x24 + 0x23] == 0) {
            p = func_800513D0(p, ot, 0x60 - remx, 0x60 - remy, 0x130);
        }
    }

    {
        u8 *ctx = D_800C6720;
        s32 draw_y_base = *(u16 *)(ctx + 0x46);
        s32 draw_x = *(u16 *)(ctx + 0x44) + 0xC;
        s32 draw_y = draw_y_base + 0x14;
        if (D_800CA898 != 0)
            draw_y = draw_y_base + 0xFC;
        func_8001C56C(&draw_env, draw_x, draw_y, 0x60, 0x60);
    }
    func_8001A5D4(p, &draw_env);
    addPrim(ot, p);
    p += 0x40;
done:
    return p;
}
