#include "common.h"

typedef struct { s16 x, y; } WselPoint;
typedef struct { s16 x0, y0, x1, y1; } WselRect4;

extern u8 D_80042FD8[];
extern u32 D_80043000;
extern u8 D_800C32F0[];
extern u8 D_800C6720[];
extern s32 D_800CA89C;
extern WselRect4 D_800CA8A8;
extern s32 D_800CA8B0;
extern s32 D_800CA8B4;
extern WselPoint D_800CA8B8;
extern WselPoint D_800CA8BC;
extern WselRect4 D_800CA8C0;
extern WselPoint D_800CA8C8;
extern WselPoint D_800CA8CC;
extern s16 D_800CA8CE;
extern s32 D_800CA8D0;
extern s32 D_800CA8D4;
extern s32 D_800CA8D8;
extern s32 D_800CA8DC;
extern s32 D_800CA8E0;

void func_800517BC(void)
{
    s32 temp_a0;
    s32 var_a2;
    s32 temp_v0;
    s32 temp_v1;
    s32 var_a0;
    s32 var_a1;

    func_80052384();
    switch (D_800CA8D8) {
    case 0:
        if (D_800CA8D4 & 0x220) {
            func_80050080(0x7E, 0x80);
            func_80052154();
            D_800CA8B4 = 0x12;
            D_800CA8A8.x0 = 0x60;
            D_800CA8A8.x1 = 0xE0;
            D_800CA8C0.x0 = -0x2E;
            D_800CA8C0.x1 = 0x152;
            D_800CA8A8.y0 = 0x30;
            D_800CA8A8.y1 = 0xA8;
            D_800CA8C0.y0 = -0x5A;
            D_800CA8C0.y1 = 0x11E;
            D_800CA8D8 = 2;
            return;
        }
        if ((D_800CA8D4 & 0x40) && !((D_80043000 >> 3) & 1)) {
            func_80050080(0x7F, 0x80);
            D_800CA89C = 3;
        }
        return;

    case 1:
        if ((D_800CA8E0 == 0) && (D_800CA8DC == 0)) {
            if (D_800CA8D4 & 0x40) {
                func_80050080(0x7F, 0x80);
                D_800CA8B4 = 0x12;
                D_800CA8C0.x0 = 0x60;
                D_800CA8C0.x1 = 0xE0;
                D_800CA8A8.x0 = -0x2E;
                D_800CA8A8.x1 = 0x152;
                D_800CA8C0.y0 = 0x30;
                D_800CA8C0.y1 = 0xA8;
                D_800CA8A8.y0 = -0x5A;
                D_800CA8A8.y1 = 0x11E;
                D_800CA8D8 = 4;
                goto after_action;
            }
            if (D_800CA8D4 & 0x220) {
                D_800CA8B4 = 0;
                D_800C6720[0x33] = 0x80;
                D_800C6720[3] = 0x80;
                D_800C6720[2] = 0;
                temp_a0 = -D_800CA8B8.x;
                temp_a0 += D_800CA8BC.x;
                var_a1 = temp_a0 - 0x10;
                D_800C6720[0x32] = 1;
                if (var_a1 < 0) {
                    var_a1 = temp_a0 - 1;
                }
                temp_v1 = D_800CA8B8.y * -1 + D_800CA8BC.y;
                temp_v0 = temp_v1 - 0x10;
                if (temp_v0 < 0) {
                    temp_v0 = temp_v1 - 1;
                }
                var_a2 = var_a1 >> 4;
                var_a1 = temp_v0 >> 4;
                var_a0 = 0x78;
                if (D_800C32F0[var_a1 * 0x13 + var_a2] == 0) {
                    func_80050080(0x7E, 0x80);
                    D_800CA8D8 = 3;
                    D_800CA8D0 = 0;
                    return;
                }
                goto play_move_sound;
            }

after_action:
            if (D_800CA8B0 & 0x1000) {
                if (D_800CA8CE < 0x41) {
                    if (D_800CA8C8.y < 0) {
                        D_800CA8C8.y = (u16)D_800CA8C8.y + 0x10;
                        D_800CA8DC = 4;
                    } else {
                        goto up_second;
                    }
                } else {
up_second:
                    if (D_800CA8CC.y >= 0x11) {
                        D_800CA8CC.y = (u16)D_800CA8CC.y - 0x10;
                        D_800CA8E0 = 4;
                    }
                }
            }
            if (D_800CA8B0 & 0x4000) {
                if ((D_800CA8CE >= 0x40) && (D_800CA8C8.y >= -0xBF)) {
                    D_800CA8C8.y = (u16)D_800CA8C8.y - 0x10;
                    D_800CA8DC = 4;
                } else if (D_800CA8CC.y < 0x70) {
                    D_800CA8CC.y = (u16)D_800CA8CC.y + 0x10;
                    D_800CA8E0 = 4;
                }
            }
            if (D_800CA8B0 & 0x8000) {
                if ((D_800CA8CC.x < 0x71) && (D_800CA8C8.x < 0)) {
                    D_800CA8C8.x = (u16)D_800CA8C8.x + 0x10;
                    D_800CA8DC = 4;
                } else if (D_800CA8CC.x >= 0x11) {
                    D_800CA8CC.x = (u16)D_800CA8CC.x - 0x10;
                    D_800CA8E0 = 4;
                }
            }
            if (D_800CA8B0 & 0x2000) {
                if (D_800CA8CC.x >= 0x70) {
                    if (D_800CA8C8.x >= -0x5F) {
                        D_800CA8C8.x = (u16)D_800CA8C8.x - 0x10;
                        D_800CA8DC = 4;
                    } else {
                        goto right_second;
                    }
                } else {
right_second:
                    if (D_800CA8CC.x < 0xD0) {
                        D_800CA8CC.x = (u16)D_800CA8CC.x + 0x10;
                        D_800CA8E0 = 4;
                    }
                }
            }
            if (D_800CA8B0 & 0xF000) {
                var_a0 = 0x7D;
                if ((D_800CA8E0 != 0) || (D_800CA8DC != 0)) {
play_move_sound:
                    func_80050080(var_a0, 0x80);
                    return;
                }
            }
        }
        break;

    case 3:
        if (D_800CA8D4 & 0xA20) {
            func_80050080(0x7E, 0x80);
            temp_v0 = -D_800CA8B8.x;
            temp_v0 += D_800CA8BC.x;
            var_a2 = temp_v0 - 0x10;
            if (var_a2 < 0) {
                var_a2 = temp_v0 - 1;
            }
            temp_v1 = D_800CA8B8.y * -1 + D_800CA8BC.y;
            temp_v0 = temp_v1 - 0x10;
            var_a2 >>= 4;
            if (temp_v0 < 0) {
                temp_v0 = temp_v1 - 1;
            }
            var_a1 = temp_v0 >> 4;
            {
                u8 *base = D_80042FD8;
                *(s32 *)(base + 0xE0) = var_a2 + (var_a1 * 0x13);
                D_800CA89C = 1;
                *(u32 *)(base + 0x28) &= ~8;
            }
            return;
        }
        if (D_800CA8D4 & 0x40) {
            func_80050080(0x7D, 0x80);
            D_800CA8D8 = 1;
        }
        break;
    }
}
