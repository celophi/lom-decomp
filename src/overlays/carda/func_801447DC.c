#include "common.h"

typedef struct CardaElement {
    u32 attr;
    u32 size_flags;
    void *draw_handler;
} CardaElement;

extern CardaElement D_80165F80;
extern u32 D_80165FE4;
extern s32 D_80165FEC;
extern s32 D_80166000;
extern s32 D_80166070;
extern s32 D_80166078;
extern s32 D_801660A0;
extern s32 D_801660FC;
extern s32 D_80166118;
extern void *D_801663A0;
extern u8 D_80165B70;
extern void *jtbl_80140094[];

void func_80144A24(void);
void func_80147C5C(void);
void func_8001729C(s32);
void func_800A3938(s32, s32);
void func_800AA02C(void);

/**
 * @brief Select and initialize the CARDA status dialog for the requested state.
 * @param arg0 Status-dialog state index.
 */
void func_801447DC(u32 arg0)
{
    s32 state;
    CardaElement *p;
    static void *const keep[] = { &&case0, &&case1, &&case2, &&case3, &&case4, &&case5 };

    if (D_80165FE4 != arg0 || !(D_80165F80.attr & 7) ||
        D_80165F80.draw_handler != (void *)func_80144A24) {
        D_80166118 = 0;
        D_80166070 = 0;
        D_801660FC = 0;
        D_80166000 = 0;
        func_80147C5C();
        D_80165FE4 = arg0;
        func_8001729C(D_801660A0);
        func_800A3938(0x78, 0x80);

        if ((u32)(D_80166078 - 2) < 2) {
            state = D_80165FE4;
            if ((u32)state < 6)
                goto *jtbl_80140094[state];
            goto after_switch;
case0:
            D_80165FEC = 0xF0;
            goto after_switch;
case1:
            D_80165FEC = 0xEF;
            goto after_switch;
case2:
            D_80165FEC = 0xEE;
            goto after_switch;
case3:
            D_80165FEC = 0xED;
            goto after_switch;
case4:
            D_80165FEC = 0xEC;
            goto after_switch;
case5:
            D_80165FEC = 0xEB;
after_switch:
            D_801663A0 = 0;
            return;
        }

        D_80165F80.attr = (((((((D_80165F80.attr & ~0x78U) | 8) & ~7U) | 1) & 0xFFFF007FU) | 0x1000) & 0xFFFFFFU);
        p = &D_80165F80;
        p->size_flags |= 1;
        if ((s32)arg0 < 2 || arg0 == 4 || arg0 == 5) {
            ((u8 *)p)[2] = 0x60;
            p->size_flags = (p->size_flags & ~0x1FEU) | 0x48;
        } else {
            p->size_flags = (p->size_flags & ~0x1FEU) | 0x28;
            ((u8 *)p)[2] = 0x70;
        }
        p->draw_handler = (void *)func_80144A24;
        func_800AA02C();
        D_80166118 = 0;
        D_80166070 = 0;
        D_801660FC = 0;
        D_80166000 = 0;
        D_80165FEC = 0xFF;
        func_80147C5C();
        D_801663A0 = &D_80165B70;
        D_80165FE4 = arg0;
        func_8001729C(D_801660A0);
    }
}
