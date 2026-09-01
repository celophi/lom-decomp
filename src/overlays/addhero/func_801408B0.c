#include "common.h"

typedef struct AddheroElement {
    union {
        u32 word;
        struct { u32 state:3; u32 unk0_3:4; u32 x:9; u32 unk0_16:8; } f;
    } attr;
    u32 unk4_0:1;
    u32 y:8;
    u32 unk4_9:23;
    void *draw_handler;
    s32 unkC;
} AddheroElement;

typedef struct AddheroRecord {
    u8 pad0[0x17];
    u8 unk17;
    u8 pad18[0xCF - 0x18];
    u8 unkCF;
    u8 padD0[4];
    u16 unkD4;
    u16 unkD6;
} AddheroRecord;

typedef struct { u8 data[0x28]; } AddheroEntry28;

extern AddheroElement D_80160940;
extern s32 D_801609A0;
extern s32 D_8016092C;
extern s32 D_801609A4;
extern s32 D_80164A60;
extern s32 D_80164A40;
extern u8 *D_80165488;
extern s32 D_8016093C;
extern s32 D_80122988;
extern s32 D_80122718;
extern s32 D_801609AC;
extern s32 D_801609A8;
extern char D_800ECF7C[];
extern AddheroEntry28 D_80164B60[][20];
extern AddheroRecord D_80165388;
extern AddheroRecord *D_8012271C;
extern s32 D_8003EC9C;

void func_80140C94(void);
void func_80140C18(void);
void func_80145E14(void);
void func_80140CFC(void);
s32 func_8001714C();
AddheroElement *func_80141EAC(void);
void func_80144008(void);
void func_80145824(void);
s32 func_80142618(s32 *ot, s32 prim, s32 arg2, s32 arg3);

#define SET_ELEM_CODE(e,c) ((e)->attr.word = ((e)->attr.word & 0x00FFFFFF) | ((u32)(c) << 24))

s32 func_801408B0(void)
{
    s32 pending;
    s32 status;
    s32 count;
    s32 term1;
    s32 term2;
    AddheroElement *p;

    if ((D_80160940.unkC & 7) == 0) {
        D_801609A0 = D_8016092C;
        return;
    }
    if (D_801609A0 != 0) {
        return;
    }
    if ((D_80160940.unkC & 7) >= 3) {
        return;
    }
    if ((D_80160940.attr.word & 7) != 0) {
        return;
    }
    pending = D_801609A4;
    if (pending == 0xFF) {
        return;
    }
    if (D_80164A60 != 0) {
        return;
    }
    if (D_80164A40 != 0) {
        return;
    }
    if ((u32)(*D_80165488 - 6) < 2U) {
        return;
    }
    if (D_8016093C != 0) {
        return;
    }

    status = D_80122988;
    if (status & 0x40) {
        D_80122718 = 3;
        func_800A3938(0x78, 0x80);
        func_80140C94();
        return;
    }
    if (status & 0xA100) {
        func_800A3938(0x7D, 0x80);
        func_80140C18();
        return;
    }
    if (pending >= 0x10) {
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
            D_801609AC -= 1;
            if (D_801609AC < 0) {
                D_801609AC = D_801609A4 - 1;
            }
        }
        if (D_80122988 & 0x4000) {
            D_801609AC += 1;
            if (D_801609AC >= D_801609A4) {
                D_801609AC = 0;
            }
        }
        count -= 1;
    }

    if (D_80122988 & 0x5000) {
        func_80145E14();
        func_800A3938(0x7D, 0x80);
        func_80140CFC();
        return;
    }

    if (D_80122988 & 0x220) {
        term1 = D_801609A8 * 0x320;
        term2 = (D_801609AC * 0x28) + (s32)D_80164B60;
        if (func_8001714C(D_800ECF7C, (char *)(term1 + term2), 0xC) == 0) {
            if ((D_80165388.unkD4 != D_8012271C->unkD4) &&
                ((D_8003EC9C == 0xFF) || (D_80165388.unkCF == D_8003EC9C))) {
                p = func_80141EAC();
                p->attr.f.unk0_3 = 1;
                p->attr.f.x = 0x10;
                p->attr.f.unk0_16 = 0x61;
                p->unk4_0 = 1;
                p->y = 0x1E;
                SET_ELEM_CODE(p, 0x20);
                func_80144008();
                p->draw_handler = func_80142618;
                func_80145824();
                func_800A3938(0x7E, 0x80);
                return;
            }
        }
        func_800A3938(0x78, 0x80);
    }
}
