#include "common.h"
#include "sdk/libgpu.h"

typedef struct { s16 x, y; } WselPoint;
typedef struct {
    s16 x0, y0;
    s16 x1, y1;
    s16 x2, y2;
    s16 x3, y3;
} WselQuadCoords;

extern u8 D_800435E0;
extern u8 D_800C6720[];
extern WselPoint D_800CA8A8[2];
extern s32 D_800CA8B0;
extern s32 D_800CA8B4;
extern WselPoint D_800CA8C0[2];
extern s32 D_800CA8D8;
extern void *jtbl_8004FC74[];

extern void func_80050080(s32, s32);
extern void func_80050944(void);
extern void *func_80050B40(u8 *, s32 *);
extern void *func_80050DB0(POLY_FT4 *, u_long *, WselQuadCoords *, s32, s32);
extern void *func_80050F0C(void *, u_long *);
extern void *func_800514D8(void *, s32 *, s32);
extern POLY_FT4 *func_80051D78(POLY_FT4 *, u_long *, s32);

void func_800503F0(void *arg0)
{
    WselQuadCoords q;
    s32 i;
    s32 state;
    u8 *prim;
    s32 *ot;

    static void *const keep[] = { &&case0, &&case1, &&case2, &&case3, &&case4, &&case5 };

    prim = *(u8 **)((u8 *)arg0 + 0x80B8);
    ot = (s32 *)((u8 *)arg0 + 0x40);

    state = D_800CA8D8;
    if ((u32)state < 6) {
        goto *jtbl_8004FC74[state];
    }
    goto finish;

case1:
        {
            s32 n = D_800CA8B4;
            if (n != 0) {
                u8 *state;
                n--;
                D_800CA8B4 = n;
                state = D_800C6720;
                state[0x33] = state[3] = (0x10 - n) << 3;
                if (n == 0) {
                    state[2] = 0;
                    state[0x32] = 1;
                } else {
                    state[2] = 1;
                    state[1] = 1;
                    state[0x32] = 1;
                    state[0x31] = 1;
                }
            }
        }
        prim = func_800514D8(prim, ot, 2);
        prim = func_80050F0C(prim, (u_long *)ot);
        if (D_800CA8B4 != 0) {
            q.x0 = q.x2 = D_800CA8A8[0].x;
            q.x1 = q.x3 = D_800CA8A8[1].x;
            q.y0 = q.y1 = D_800CA8A8[0].y;
            q.y2 = q.y3 = D_800CA8A8[1].y;
            prim = func_80050DB0((POLY_FT4 *)prim, (u_long *)ot, &q, 1, D_800CA8B4 << 3);
        }
        prim = func_800514D8(prim, ot, 0);
        func_80050944();
        goto finish;

case2:
        for (i = 0; i < 2; i++) {
            D_800CA8A8[i].x = (u16)D_800CA8A8[i].x + (D_800CA8C0[i].x - D_800CA8A8[i].x) / D_800CA8B4;
            D_800CA8A8[i].y = (u16)D_800CA8A8[i].y + (D_800CA8C0[i].y - D_800CA8A8[i].y) / D_800CA8B4;
        }
        q.x0 = q.x2 = D_800CA8A8[0].x;
        q.x1 = q.x3 = D_800CA8A8[1].x;
        q.y0 = q.y1 = D_800CA8A8[0].y;
        q.y2 = q.y3 = D_800CA8A8[1].y;
        prim = func_80050DB0((POLY_FT4 *)prim, (u_long *)ot, &q, 0, 0x80);
        if (--D_800CA8B4 == 0) {
            D_800CA8D8 = 5;
            D_800CA8B4 = 0x10;
        }
        goto draw_tail;

case3:
        prim = func_800514D8(prim, ot, 2);
        prim = func_80050F0C(prim, (u_long *)ot);
        prim = func_80050B40(prim, ot);
        prim = func_800514D8(prim, ot, 0);
        func_80050944();
        goto finish;

case4:
        for (i = 0; i < 2; i++) {
            D_800CA8A8[i].x = (u16)D_800CA8A8[i].x + (D_800CA8C0[i].x - D_800CA8A8[i].x) / D_800CA8B4;
            D_800CA8A8[i].y = (u16)D_800CA8A8[i].y + (D_800CA8C0[i].y - D_800CA8A8[i].y) / D_800CA8B4;
        }
        q.x0 = q.x2 = D_800CA8A8[0].x;
        q.x1 = q.x3 = D_800CA8A8[1].x;
        q.y0 = q.y1 = D_800CA8A8[0].y;
        q.y2 = q.y3 = D_800CA8A8[1].y;
        prim = func_80050DB0((POLY_FT4 *)prim, (u_long *)ot, &q, 0, 0x80);
        if (--D_800CA8B4 == 0) {
            D_800CA8D8 = 0;
            D_800CA8B4 = 0;
        }
        goto draw_tail;

case5:
        for (i = 0; i < 2; i++) {
            D_800CA8A8[i].x = D_800CA8C0[i].x;
            D_800CA8A8[i].y = D_800CA8C0[i].y;
        }
        q.x0 = q.x2 = D_800CA8A8[0].x;
        q.x1 = q.x3 = D_800CA8A8[1].x;
        q.y0 = q.y1 = D_800CA8A8[0].y;
        q.y2 = q.y3 = D_800CA8A8[1].y;
        prim = func_800514D8(prim, ot, 7);
        prim = func_80050DB0((POLY_FT4 *)prim, (u_long *)ot, &q, 0, 0x80);
        if (D_800CA8B0 & 0x220) {
            func_80050080(0x7E, 0x80);
            D_800CA8D8 = 1;
            D_800CA8B4 = 0x10;
        }
        goto draw_tail;

case0:
draw_tail:
        prim = func_800514D8(prim, ot, 3);
        prim = (u8 *)func_80051D78((POLY_FT4 *)prim, (u_long *)ot, D_800435E0 & 0x7F);
        prim = func_800514D8(prim, ot, 1);

finish:
    *(u8 **)((u8 *)arg0 + 0x80B8) = prim;
}
