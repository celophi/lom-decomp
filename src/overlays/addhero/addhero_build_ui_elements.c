#include "common.h"

typedef struct AddheroElement {
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
} AddheroElement;

extern s32 D_801609BC;
extern s32 D_80160938;
extern s32 D_80160928;
extern s32 D_801609AC;
extern s32 D_801609B8;
extern s32 D_80160924;
extern void *D_8012271C;
extern s32 D_801609B4;
extern s32 D_8016093C;
extern AddheroElement D_80160940;

void func_80141E54();
AddheroElement *func_80141EAC();
s32 func_80143044(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_80140D80(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_80141430(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_801414DC(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_801415B8(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_80141694(s32 *ot, s32 prim, s32 arg2, s32 arg3);

#define SET_ELEM_CODE(e,c) ((e)->attr.word = ((e)->attr.word & 0x00FFFFFF) | ((u32)(c) << 24))

void func_8014028C(void)
{
    AddheroElement *p;
    D_801609BC = 0;
    D_80160938 = 0;
    D_80160928 = 0;
    D_801609AC = 0;
    D_801609B8 = 0;
    D_80160924 = (s32)D_8012271C + 0xCE0;
    if (0) func_80141E54(0,0,0,0,0);
    func_80141E54();
    D_801609B4 = 0;
    if (D_8016093C != 0)
    {
        D_80160940.attr.f.state = 1;
        p = func_80141EAC();
        p->draw_handler = (void *)func_80143044;
        p->attr.f.unk0_3 = 1;
        p->attr.f.x = 0x10;
        p->attr.f.unk0_16 = 0x61;
        p->unk4_0 = 1;
        p->y = 0x2C;
        SET_ELEM_CODE(p, 0x20);

        p = func_80141EAC();
        p->draw_handler = (void *)func_801414DC;
        p->attr.f.unk0_3 = 1;
        p->attr.f.x = 0x18;
        p->attr.f.unk0_16 = 0x4D;
        p->unk4_0 = 0;
        p->y = 0x10;
        SET_ELEM_CODE(p, 0x80);

        p = func_80141EAC();
        p->draw_handler = (void *)func_801415B8;
        p->attr.f.unk0_3 = 1;
        p->attr.f.x = 0xA0;
        p->attr.f.unk0_16 = 0x4D;
        p->unk4_0 = 0;
        p->y = 0x10;
        SET_ELEM_CODE(p, 0x80);
        D_80160940.attr.f.state = 0;
        return;
    }

    D_80160940.attr.f.state = 1;
    p = func_80141EAC();
    p->draw_handler = (void *)func_80140D80;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x1C;
    p->attr.f.unk0_16 = 0x32;
    p->unk4_0 = 1;
    p->y = 0x58;
    SET_ELEM_CODE(p, 8);
    /* ADDHERO-specific flag */
    *(u32 *)((u8 *)p + 4) |= 0x200;

    p = func_80141EAC();
    p->draw_handler = (void *)func_80141430;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x24;
    p->attr.f.unk0_16 = 0x0A;
    p->unk4_0 = 0;
    p->y = 0x10;
    SET_ELEM_CODE(p, 0xF0);

    p = func_80141EAC();
    p->draw_handler = (void *)func_801414DC;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x18;
    p->attr.f.unk0_16 = 0x1E;
    p->unk4_0 = 0;
    p->y = 0x10;
    SET_ELEM_CODE(p, 0x80);

    p = func_80141EAC();
    p->draw_handler = (void *)func_801415B8;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0xA0;
    p->attr.f.unk0_16 = 0x1E;
    p->unk4_0 = 0;
    p->y = 0x10;
    SET_ELEM_CODE(p, 0x80);

    p = func_80141EAC();
    p->draw_handler = (void *)func_80141694;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x1E;
    p->attr.f.unk0_16 = 0x8E;
    p->unk4_0 = 1;
    p->y = 0x34;
    SET_ELEM_CODE(p, 4);
    D_80160940.attr.f.state = 0;
}
