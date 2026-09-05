#include "common.h"

typedef struct CardaElement40918 {
    u32 attr;
    u32 size_flags;
    void *draw_handler;
} CardaElement40918;

extern s32 D_80122988;
extern u8 D_80165B70[];
extern u8 D_80165B7C[];
extern u8 D_80165B84[];
extern CardaElement40918 D_80165F80;
extern s32 D_80165FEC;
extern s32 D_80166078;
extern s32 D_801660F8;
extern u8 *D_801663A0;
extern void func_80144050(void);

/* Deliberately no prototypes for the old-style callees. */

s32 func_80140918(void)
{
    s32 result;
    s32 repeat;
    CardaElement40918 *p;

    if ((u32)(D_80166078 - 2) < 2) {
        repeat = 3;
        if (D_801663A0 != 0) {
            goto load_loop;
        }
        switch (D_80165FEC) {
        case 0xE9:
        case 0xEB:
        case 0xEC:
        case 0xED:
        case 0xEE:
        case 0xEF:
        case 0xF0:
        case 0xF1:
        case 0xF6:
        case 0xF8:
        case 0xF9:
        case 0xFA:
        case 0xFB:
        case 0xFC:
        case 0xFD:
        case 0xFF:
            return;
        default:
            D_801663A0 = D_80165B70;
            goto set_repeat;
        }
    } else {
        repeat = 3;
        if (D_80165FEC < 0x12) {
            goto load_loop;
        }
        if (D_801663A0 != 0) {
            goto load_loop;
        }
        if (D_80165FEC == 0xF1) {
            goto load_loop;
        }
        D_801663A0 = D_80165B70;
    }
set_repeat:
    repeat = 3;
load_loop:
    do {
        result = func_80147F4C();
    } while (result == repeat);

    if (D_801660F8 != 0 && (D_80122988 & 0x220)) {
        D_80165FEC = 0xF9;
        D_801660F8 = 0;

        p = &D_80165F80;
        D_80165F80.attr = (((((D_80165F80.attr & ~7U) | 1U) & ~0x78U) | 8U) & 0xFFFF007FU) | 0x800U;
        ((u8 *)p)[2] = 0x4C;
        D_80165F80.attr = (D_80165F80.attr & 0x00FFFFFFU) | 0x20000000U;
        p->size_flags = ((p->size_flags | 1U) & ~0x1FEU) | 0x90U;
        func_80144F18();
        D_80165F80.draw_handler = (void *)func_80144050;
        func_801495E4();
        return;
    }

    switch (result) {
    case 0:
        break;
    case 2:
        D_801663A0 = D_80165B84;
        break;
    case 4:
        if ((u32)(D_80166078 - 2) < 2) {
            D_801663A0 = 0;
        } else {
            D_801663A0 = D_80165B7C;
        }
        D_801660F8 = 0;
        break;
    case 5:
        if (D_80166078 == 1 || D_80166078 == 3) {
            D_80165FEC = 0xF9;
            if (D_80166078 == 1) {
                D_801663A0 = D_80165B70;
            }
        } else {
            D_80165FEC = 0xF9;
            D_801660F8 = 0;
            p = &D_80165F80;
            D_80165F80.attr = (((((D_80165F80.attr & ~7U) | 1U) & ~0x78U) | 8U) & 0xFFFF007FU) | 0x800U;
            ((u8 *)p)[2] = 0x4C;
            D_80165F80.attr = (D_80165F80.attr & 0x00FFFFFFU) | 0x20000000U;
            p->size_flags = ((p->size_flags | 1U) & ~0x1FEU) | 0x90U;
            func_80144F18();
            D_80165F80.draw_handler = (void *)func_80144050;
            func_801495E4();
        }
        break;
    }
}
