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
} CardaElement;

extern s32 D_8012298C;
extern s32 D_80165F38;
extern CardaElement D_80165F80;
extern s32 D_80165FFC;
extern s32 D_80166104;
extern void func_80146E80(void);
extern void func_80146EDC(void);

#define SET_ELEM_CODE(e,c) ((e)->attr.word = ((e)->attr.word & 0x00FFFFFF) | ((u32)(c) << 24))

static __inline__ CardaElement *claim_element(void)
{
    CardaElement *p;
    s32 i;
    p = &D_80165F80;
    for (i = 0; i < 8; i++, p++) {
        if ((p->attr.word & 7) == 0) {
            p->attr.word = (p->attr.word & ~7) | 1;
            return p;
        }
    }
    return &D_80165F80;
}

void func_80146CA4(void)
{
    CardaElement *slot;
    D_8012298C = 0x20;
    {
        CardaElement *clear_p;
        s32 clear_i;
        clear_p = &D_80165F80;
        clear_i = 0;
        do {
            clear_i++;
            clear_p->attr.word &= ~7;
            clear_p++;
        } while (clear_i < 8);
    }

    D_80165F80.attr.f.state = 1;
    slot = claim_element();

    slot->draw_handler = (void *)func_80146EDC;
    slot->attr.f.unk0_3 = 2;
    slot->attr.f.x = 0x20;
    slot->attr.f.unk0_16 = 0x36;
    slot->unk4_0 = 1;
    slot->y = 0x90;
    SET_ELEM_CODE(slot, 0);

    slot = claim_element();
    slot->draw_handler = (void *)func_80146E80;
    slot->attr.f.unk0_3 = 2;
    slot->attr.f.x = 0x20;
    slot->attr.f.unk0_16 = 0x1A;
    slot->unk4_0 = 1;
    slot->y = 0x10;
    D_80165FFC = 0;
    D_80165F38 = 0;
    SET_ELEM_CODE(slot, 0);
    D_80166104 = 0;
    D_80165F80.attr.f.state = 0;
}
