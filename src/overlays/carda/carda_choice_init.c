#include "common.h"

typedef struct CardaChoiceElement {
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
} CardaChoiceElement;

extern CardaChoiceElement D_80165F8C;
extern s32 D_80165FE4;
extern s32 D_80165FEC;
extern s32 D_80166000;
extern s32 D_80166070;
extern s32 D_80166078;
extern s32 D_801660FC;
extern s32 D_80166118;
extern void *D_801663A0;
extern void *jtbl_8014018C[];

void func_80146AF0(void);
void func_80147C5C(void);

#define SET_ELEM_CODE(e, c) \
    ((e)->attr.word = ((e)->attr.word & 0x00FFFFFF) | ((u32)(c) << 24))

/**
 * @brief Reset the CARDA choice state and initialize its mode-specific element.
 *
 * The zero switch and label-address array preserve the original six-entry
 * jump-table dispatch.  CARDA.BIN.yaml links this function's .rodata at the
 * original jtbl_8014018C location, so the array is the target jump table rather
 * than duplicate data.
 * @see matching: 100.00%
 */
void func_8014697C(s32 arg0)
{
    func_800A3938(0x78, 0x80);
    func_800AA02C();
    D_80166118 = 0;
    D_80166070 = 0;
    D_801660FC = 0;
    D_80166000 = 0;
    func_80147C5C();
    D_801663A0 = 0;
    D_80165FE4 = arg0;

    if ((u32)(D_80166078 - 2) < 2)
    {
        static void *const keep[] = {
            &&case0, &&case1, &&case2, &&case3, &&case4, &&case5
        };
        switch (0)
        {
        case 0:
            if ((u32)arg0 >= 6)
                goto done;
            goto *jtbl_8014018C[arg0];
        case0:
            D_80165FEC = 0xF0;
            goto done;
        case1:
            D_80165FEC = 0xEF;
            goto done;
        case2:
            D_80165FEC = 0xEE;
            goto done;
        case3:
            D_80165FEC = 0xED;
            goto done;
        case4:
            D_80165FEC = 0xEC;
            goto done;
        case5:
            D_80165FEC = 0xEB;
        done:
            D_801663A0 = 0;
            return;
        }
    }

    D_80165F8C.draw_handler = (void *)func_80146AF0;
    D_80165F8C.attr.f.unk0_3 = 1;
    D_80165F8C.attr.f.state = 1;
    D_80165F8C.attr.f.x = 0x20;
    D_80165F8C.attr.f.unk0_16 = 0x70;
    D_80165F8C.unk4_0 = 1;
    D_80165F8C.y = 0x14;
    SET_ELEM_CODE(&D_80165F8C, 0);
}
