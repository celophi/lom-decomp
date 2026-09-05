#include "common.h"

typedef struct CardaElement {
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
    void *draw_handler;
    s32 unkC;
} CardaElement;

extern CardaElement D_80165F80;
extern s32 D_80165FE0;
extern s32 D_80165FEC;
extern s32 D_80165FF4;
extern s32 D_80166000;
extern s32 D_80166078;
extern s32 D_801660A0;
extern s32 D_80166AE0;
extern u8 *D_801663A0;
extern u8 D_8016636F;
extern u8 D_80166440[];
extern s32 D_80122988;
extern s32 D_8012299C;
extern s32 D_8003EC9C;
extern char D_800ECF7C[];
extern char D_800ECFC4[];

CardaElement *func_80142614();
void func_80142E10();
void func_80144F18();
void func_801495E4();
void func_80141164();
void func_801410E4();
void func_801411CC();
extern s32 func_8014344C(s32 *, s32, s32, s32);
extern s32 func_801439B4(s32 *, s32, s32, s32);
extern s32 func_80143BD4(s32 *, s32, s32, s32);

#define SET_ELEM_CODE(e,c) ((e)->attr.word = ((e)->attr.word & 0x00FFFFFFU) | ((u32)(c) << 24))

s32 func_80140BAC(void)
{
    s32 pending;
    s32 status;
    s32 count;
    CardaElement *p;

    if ((D_80165F80.unkC & 7) == 0) {
        D_80165FE0 = 1;
        return;
    }
    if (D_80165FE0 != 0) {
        return;
    }
    if ((D_80165F80.unkC & 7) >= 3) {
        return;
    }
    if ((D_80165F80.attr.word & 7) != 0) {
        return;
    }
    if ((u32)(D_80166078 - 2) < 2U) {
        return;
    }
    pending = D_80165FEC;
    if (pending == 0xFF) {
        return;
    }
    if (D_80166AE0 != 0) {
        return;
    }
    if (D_80166000 != 0) {
        return;
    }
    if ((u32)(*D_801663A0 - 6) < 2U) {
        return;
    }

    status = D_80122988;
    if (status & 0x40) {
        D_8012299C = 3;
        func_800A3938(0x78, 0x80);
        func_80141164();
        return;
    }
    if (status & 0xA100) {
        func_800A3938(0x7D, 0x80);
        func_801410E4();
        return;
    }
    if (pending >= 0x12) {
        return;
    }

    count = 1;
    if (status & 8) {
        D_80122988 = 0x4000;
        count = 1;
    }
    if (D_80122988 & 4) {
        D_80122988 = 0x1000;
        count = 1;
    }

    while (count != 0) {
        if (D_80122988 & 0x1000) {
            D_80165FF4 -= 1;
            if (D_80165FF4 < 0) {
                D_80165FF4 = D_80165FEC - 1;
            }
        }
        if (D_80122988 & 0x4000) {
            D_80165FF4 += 1;
            if (D_80165FF4 >= D_80165FEC) {
                D_80165FF4 = 0;
            }
        }
        count -= 1;
    }

    if (D_80122988 & 0x5000) {
        func_80149DF4();
        func_800A3938(0x7D, 0x80);
        func_801411CC();
        return;
    }

    if (D_80122988 & 0x220) {
        if (D_80166078 == 1) {
            s32 term1;
            s32 term2;
            term1 = D_801660A0 * 0x320;
            term2 = (D_80165FF4 * 0x28) + (s32)D_80166440;
            if (func_8001714C(D_800ECF7C, (void *)(term1 + term2), 0xC) == 0) {
                if (D_8003EC9C == 0xFF || D_8016636F == D_8003EC9C) {
                    p = func_80142614();
                    p->attr.f.unk0_3 = 1;
                    p->attr.f.x = 0x10;
                    p->attr.f.unk0_16 = 0x5A;
                    p->unk4_0 = 1;
                    p->y = 0x2C;
                    SET_ELEM_CODE(p, 0x20);
                    func_80142E10();
                    func_80144F18();
                    p->draw_handler = (void *)func_8014344C;
                    func_801495E4();
                    func_800A3938(0x7E, 0x80);
                    return;
                }
            }
        } else {
            {
                s32 term1;
                s32 term2;
                term1 = D_801660A0 * 0x320;
                term2 = (D_80165FF4 * 0x28) + (s32)D_80166440;
                if (func_8001714C(D_800ECFC4, (void *)(term1 + term2), 8) == 0) {
                    p = func_80142614();
                    p->attr.f.unk0_3 = 1;
                    p->attr.f.x = 0x10;
                    p->attr.f.unk0_16 = 0x5A;
                    p->unk4_0 = 1;
                    p->y = 0x2C;
                    SET_ELEM_CODE(p, 0x20);
                    func_80142E10();
                    func_80144F18();
                    p->draw_handler = (void *)func_801439B4;
                    func_801495E4();
                    func_800A3938(0x7E, 0x80);
                    return;
                }
            }
            {
                s32 term1;
                s32 term2;
                term1 = D_801660A0 * 0x320;
                term2 = (D_80165FF4 * 0x28) + (s32)D_80166440;
                if (func_8001714C(D_800ECF7C, (void *)(term1 + term2), 0xC) == 0) {
                    p = func_80142614();
                    p->attr.f.unk0_3 = 1;
                    p->attr.f.x = 0x10;
                    p->attr.f.unk0_16 = 0x5A;
                    p->unk4_0 = 1;
                    p->y = 0x2C;
                    SET_ELEM_CODE(p, 0x20);
                    func_80142E10();
                    func_80144F18();
                    p->draw_handler = (void *)func_80143BD4;
                    func_801495E4();
                    func_800A3938(0x7E, 0x80);
                    return;
                }
            }
        }
        func_800A3938(0x78, 0x80);
    }
}
